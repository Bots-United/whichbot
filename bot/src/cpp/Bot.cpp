//
// $Id: Bot.cpp,v 1.54 2005/12/14 19:23:53 clamatius Exp $

// Copyright (c) 2003, WhichBot project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the WhichBot Project nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Bot.h"
#include "NSConstants.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "extern/metamod/meta_api.h"
#include "combat/CombatNavMethod.h"
#include "navigation/WaypointNavMethod.h"
#include "worldstate/HiveManager.h"
#include "strategy/HiveMind.h"
#include "BotManager.h"
#include "worldstate/WorldStateUtil.h"
#include "config/Config.h"
#include "config/TranslationManager.h"


Log Bot::_log(__FILE__);

const float DEFAULT_REWARD_FACTOR = 1.0;

// Sometimes it doesn't seem to change weapon, for some reason (maybe timeouts
// stopped us from switching)
const float WEAPON_CHANGE_TIME = 0.3;

bool pathValidator(void* pRock, int upstreamNodeId, int thisNodeId)
{
	WaypointManager* pWptMgr = (WaypointManager*)pRock;
	return pWptMgr->isPathValid(upstreamNodeId, thisNodeId);
}

Bot::Bot(int botIdx) :
	_pName(NULL),
	_entity(NULL),
	_pSensor(NULL),
	_lastEvolution(0),
	_pNavEngine(NULL),
	_evolution(kSkulk),
	_desiredEvolution(kSkulk),
	_pathMgr(DEFAULT_REWARD_FACTOR),
	_dead(true),
    _currentWeaponId(-1),
    _pMovement(NULL),
	_pTarget(NULL),
    _maxArmour(0),
    _statusReporter(),
	_role(Config::getInstance().getBotRole(botIdx))
{
	//empty
	setProperty(kLastLifeTime, 100);
	// path validators disabled till they work properly
	//_pathMgr.setPathValidatorFn(pathValidator, &gpBotManager->getWaypointManager());
}


Bot::~Bot()
{
    delete _pName;
    _pName = NULL;
    delete _pSensor;
    _pSensor = NULL;
    delete _pNavEngine;
    _pNavEngine = NULL;
    delete _pMovement;
    _pMovement = NULL;
    delete _pTarget;
    _pTarget = NULL;
}


//
// Processed every TICK_TIME.  Does all the reasoning, etc. for this bot.
//
int Bot::tick()
{
    gpCurrentThinkingBot = this;
    if (!_entity.isNull()) {
		// temp hack here - apparently the iuser3 values are different on steam vs. won
        if (_entity.getEdict()->v.iuser3 == PLAYERCLASS_ALIVE_GESTATING ||
			_entity.getEdict()->v.iuser3 == PLAYERCLASS_ALIVE_DIGESTING)
		{
            evolve();

        } else {
            bool normalMove = false;
            
            // sanity check - sometimes we can become another evolution unexpectedly
            if (_evolution != EVOLUTIONS[_entity.getEdict()->v.iuser3] &&
                _entity.getEdict()->v.iuser3 >= PLAYERCLASS_ALIVE_LEVEL1 &&
                _entity.getEdict()->v.iuser3 <= PLAYERCLASS_ALIVE_LEVEL5)
            {
                _evolution = EVOLUTIONS[_entity.getEdict()->v.iuser3];
            }

            if (_entity.getEdict()->v.team == NO_TEAM) {
                if ((_entity.getEdict()->v.flags & FL_SPECTATOR) != 0) {
                    // Uh-oh, we blundered into the spectator entrance somehow
                    readyroom();
                    
                } else {
                    startGame();
                }
                
            } else if (_entity.getEdict()->v.team == MARINE_TEAM) {
                readyroom();
                
            } else {
                
                // if the bot is dead, we need to respawn
                if ((_entity.getHealth() < 1) || (_entity.getEdict()->v.deadflag != DEAD_NO)) {
                    if (!_dead) {
                        setProperty(kLastLifeTime, gpGlobals->time - getProperty(kLastSpawnTime));
                        respawnBot();
                        _pSensor->clear();
                    }
                    _dead = true;
                    
                } else {
                    bool targetSeen = _pSensor->targetSeen();
                    bool threatSeen = _pSensor->threatSeen();
                    if (_dead) {
                        // force the sensor to scan, we just spawned and someone might be trying to kill us
                        _pSensor->scan(true);
                        
                    } else {
                        _pSensor->scan();
                    }
                    _dead = false;
                    
                    // Let's make it so gorges don't attack buildings unless they have 2 hives
                    int numHives = HiveManager::getActiveHiveCount();
                    bool newTargetSeen = 
                        (!targetSeen && _pSensor->targetSeen()) || 
                        (!threatSeen && _pSensor->threatSeen());
                    
                    if (newTargetSeen &&
                        (_evolution != kGorge || numHives >= 2 || _pSensor->threatSeen() || _pSensor->healableFriendSeen()))
                    {
                        // ok, we see a new target.  Let's switch into combat mode
                        _pNavEngine->setNextMethod(new CombatNavMethod(*this));
                    }
                    
                    if (_entity.getEdict()->v.armorvalue > _maxArmour) {
                        _maxArmour = _entity.getEdict()->v.armorvalue;
                    }
                    
                    
					checkCheating();
                    checkEvolution();
					checkPackEvolution();
                    normalMove = true;				
                }
            }
            
            if (normalMove && _pNavEngine != NULL) {
                if (gpGlobals->time < getProperty(kWeaponFireTillTime)) {
                    _entity.getEdict()->v.button |= IN_ATTACK;
					//setWeaponImpulse();
                    
                } else {
                    setProperty(kWeaponFireTillTime, 0.0);
                }
                _pNavEngine->navigate();
                
            } else {
                if (_pMovement != NULL) {
                    _pMovement->nullMove();
                }
            }
        }
    }
	
	setProperty(kLastFrameTime);
	return 0;
}


void Bot::evolve()
{
    if (_lastEvolution < FORM_CHANGE) {
        if (!hasTrait(_lastEvolution)) {
            _traits.push_back(_lastEvolution);
        }
        
    } else {
        int lastEvolutionImpulse = _lastEvolution - FORM_CHANGE;
        _evolution = kSkulk;
        for (int ii = 0; ii < NUM_EVOLUTIONS; ii++) {
            if (lastEvolutionImpulse == Config::getInstance().getImpulse(EVOLUTION_IMPULSES[ii])) {
                _evolution = (tEvolution)ii;
            }
        }
    }
    if (_pMovement != NULL) {
        // we can't do anything right now, so just wait till we hatch
        _pMovement->nullMove();
    }
}

void Bot::userInfoChanged(const char* infoBuffer)
{
	int infoBufLen = strlen(infoBuffer);
	if (strstr(infoBuffer, "model\\gestate") != NULL) {

	}
	
	if (strstr(infoBuffer, "undefinedteam")) {
		_dead = true;
		resetBot();
		
	} else if (_dead) {
		_dead = false;
		setProperty(kLastSpawnTime);
	}
	
	char* namePos = strstr(infoBuffer, "name\\");
	if (namePos != NULL) {	
		namePos += strlen("name\\");
		char* nameEnd = namePos;
		for (; (nameEnd - infoBuffer) < infoBufLen; nameEnd++) {
			if (*nameEnd == '\\') {
				break;
			}
		}
		char namebuf[128];
		strncpy(namebuf, namePos, nameEnd - namePos);
		namebuf[nameEnd - namePos] = '\0';
		delete _pName;
		_pName = new string(namebuf);
	}
}


void Bot::selectDesiredEvolution()
{
	_desiredEvolution = HiveMind::getDesiredEvolution(*this);
}


void Bot::checkCheating()
{
	// If cheats are enabled, let's cheat!
	if (gpBotManager->areCheatsEnabled() && 
		((gpBotManager->inCombatMode()) || getResources() < 100) &&
		gpGlobals->time > getProperty(kLastCheatCheckTime) + 1.0)
	{
        giveCheatPoints();

		setProperty(kLastCheatCheckTime);
	}
}


void Bot::giveCheatPoints()
{
	if (gpBotManager->inCombatMode()) {
		fakeClientCommand("givexp");
		
	} else {
		fakeClientCommand("givepoints");
	}
}


void Bot::checkEvolution()
{
	// Only evolve if we can't see an enemy
	if (!_pSensor->targetSeen()) {
		// and only check for evolution every so often and not if we died immediately last spawn
		if ((gpGlobals->time > getProperty(kLastEvolveCheckTime) + 1.0) && (getProperty(kLastLifeTime) > 30.0))
		{
			bool evolutionShift = false;

			if (_evolution != _desiredEvolution) {				
				tNodeId nearestId = gpBotManager->getWaypointManager().
					getNearestWaypoint(MIN_WALK_EVOLUTION, *(_entity.getOrigin()), _entity.getEdict());
				
				if (nearestId >= 0) {
					// Only evolve lifeform if we're near a walkable waypoint
					if ((gpBotManager->getWaypointManager().getFlags(nearestId) & W_FL_WALKABLE) != 0) {
						int impulse = 
							gpBotManager->inCombatMode() ? 
							getCombatEvolutionImpulse() : 
                            Config::getInstance().getImpulse(EVOLUTION_IMPULSES[_desiredEvolution]);

						if (impulse != 0) {
							_entity.getEdict()->v.impulse = impulse;
							_lastEvolution = FORM_CHANGE + impulse;
							evolutionShift = true;
						}
					}
				}				
			}
			
			// If we didn't change lifeforms, check for upgrade evolutions
			if (!evolutionShift) {
                int desiredUpgradeImpulse = HiveMind::getDesiredUpgrade(*this);
                if (desiredUpgradeImpulse != 0) {
                    _entity.getEdict()->v.impulse = desiredUpgradeImpulse;
                    _lastEvolution = desiredUpgradeImpulse;
                }
			}
			
			setProperty(kLastEvolveCheckTime);
		}
	}
}


byte Bot::getCombatEvolutionImpulse()
{
    byte impulse = 0;
    const int defaultMinResources[] = { 0, 480, 300, 720, 720 };
	
	int resources = getResources();
	int minResources = (int)Config::getInstance().getArrayTweak("game/resources/combat_min_resources", _desiredEvolution, defaultMinResources[_desiredEvolution]);
	if (_desiredEvolution != kSkulk) {
		if (resources >= minResources) {
	        impulse = Config::getInstance().getImpulse(EVOLUTION_IMPULSES[_desiredEvolution]);
		} else {
			// if we don't have a better idea, let's lerk in the meantime
			if (!gpBotManager->areCheatsEnabled() && resources >= Config::getInstance().getArrayTweak("game/resources/combat_min_resources", kLerk, defaultMinResources[kLerk])) {
				return Config::getInstance().getImpulse(EVOLUTION_IMPULSES[kLerk]);
			}
		}
    }
    return impulse;
}


int botNameIdx = 0;
const char* botNames[] = {
	"[wb]WhichOne", "[wb]Who", "[wb]What", "[wb]That", "[wb]It", "[wb]He", "[wb]They", "[wb]TheOther", NULL
};

const char* gorgeName = "[wb]TheFatOne";

const char* getNewBotName()
{
	int numConfiguredNames = Config::getInstance().getArraySize("admin/bot_names");

	if (numConfiguredNames > 0) {
		int nameIdx = botNameIdx++ % numConfiguredNames;
		return Config::getInstance().getArrayTweak("admin/bot_names", nameIdx, "[wb]Bot");	

	} else {

		if (gpBotManager->getNumBots() == 1) {
			return gorgeName;
		}
		
		const char* name = botNames[botNameIdx];
		if (name == NULL) {
			botNameIdx = 0;
			name = botNames[0];
		}
		botNameIdx++;
		return name;
	}
}


void Bot::create()
{
	delete _pName;
	_pName = new string(getNewBotName());
	_entity = WorldStateUtil::createPlayer(*_pName);
	if (!_entity.isNull()) {
        delete _pMovement;
        _pMovement = new BotMovement(*this);
		resetBot();
	}
}


int Bot::resetBot()
{
	_properties.clear();

	if (_pNavEngine == NULL) {
		_pNavEngine = new NavigationEngine(*this);
	}
	
	// Make sure we always set the nav method just in case we're in another type of nav
	// before (like combat nav).  Also, this allows us to clear out the history and any
	// wpt state information that the previous nav method may be holding which could
	// cause the bot to try to go to a wpt that's much too far away at the time the bot
	// was reseted.
	_pNavEngine->setNextMethod(new WaypointNavMethod(*this, true));
	
	if (_pSensor == NULL) {
		_pSensor = new BotSensor(*this);
	}
	_evolution = kSkulk;
    if (gpBotManager->inCombatMode() && !_entity.isNull()) {
        _evolution = getEvolution(_entity.getEdict());
    }
	if (getProperty(kLastSpawnTime) == 0.0) {
		setProperty(kLastLifeTime, 100);
	}
	setProperty(kLastSpawnTime);

	selectDesiredEvolution();
	
	HiveMind::unsetStrategies(*this);
	HiveMind::setStrategies(*this);
    _lastWeaponFireTimes.clear();
    _lastWeaponFireTimes.resize(WEAPON_MAX+1);
    _maxArmour = 0;
    setProperty(kWeaponFireTillTime, 0);
    _buildingBlacklist.clear();

    for (vector<float>::iterator ii = _lastWeaponFireTimes.begin(); ii != _lastWeaponFireTimes.end(); ii++) {
        *ii = 0.0f;
    }

    return 0;
}


void Bot::startGame()
{
	WorldStateUtil::joinTeam(_entity.getEdict(), ALIEN_TEAM);
}


void Bot::selectItem(const string& item_name)
{
	fakeClientCommand(item_name);
}


extern vector<string> g_fakeArgs;
void Bot::fakeClientCommand(const string& cmd)
{
	// allow the MOD DLL to execute the ClientCommand...
	g_fakeArgs.push_back(cmd);
	MDLL_ClientCommand(_entity.getEdict());
	g_fakeArgs.clear();
}


void Bot::respawnBot()
{
	resetBot();
	// TODO
	
	setProperty(kLastSpawnTime);
}


void Bot::sayToTeam(const string& msg)
{
	if (gpBotManager->isBotChatEnabled()) {
		g_fakeArgs.push_back("say_team");
		fakeClientCommand(msg);
	}
}


tEvolution Bot::getEvolution(edict_t* pEdict)
{
    if (pEdict != NULL) {
        switch (pEdict->v.iuser3) {
        case PLAYERCLASS_ALIVE_LEVEL1:
            return kSkulk;
        case PLAYERCLASS_ALIVE_LEVEL2:
            return kGorge;
        case PLAYERCLASS_ALIVE_LEVEL3:
            return kLerk;
        case PLAYERCLASS_ALIVE_LEVEL4:
            return kFade;
        case PLAYERCLASS_ALIVE_LEVEL5:
            return kOnos;
        }
    }
    // if they're currently evolving, we're going to say they're a skulk.  at some point perhaps we should
    // retrofit to cope with them being an egg.
    return kSkulk;
}


bool Bot::isEvolving(edict_t* pEdict)
{
    return pEdict != NULL ? (pEdict->v.iuser3 == PLAYERCLASS_ALIVE_GESTATING) : false;
}


Vector Bot::getGroundLevelOrigin ()
{
	return getGroundLevelOrigin(_entity.getEdict(), _evolution);
}


Vector Bot::getGroundLevelOrigin (const edict_t* pEdict, tEvolution evolution)
{
	return pEdict->v.origin - Vector(0, 0, g_CreatureOriginHeights[evolution]);
}


bool Bot::selectWeapon(Target* pTarget, float range, bool shouldUseMeleeAttack)
{
	int wpnIdx = 0;

    // Some weapons take time to fire, because we jump first
    int activeHiveCount = 1;        
    if (gpBotManager->inCombatMode()) {
        if (hasTrait(Config::getInstance().getImpulse(IMPULSE_HIVE2))) {
            activeHiveCount++;
        }
        if (hasTrait(Config::getInstance().getImpulse(IMPULSE_HIVE3))) {
            activeHiveCount++;
        }
    } else {
        activeHiveCount = HiveManager::getActiveHiveCount();
    }

	while (gWeaponSelect[wpnIdx].iId > 0) {
        tBotWeaponSelect& wpn = gWeaponSelect[wpnIdx];
		
		bool selectThisWpn = false;
		// TODO - check energy status
		if ((_evolution == wpn.evolution) &&
			(activeHiveCount >= wpn.min_hives))
		{
			// if we're using a melee attack, we're not attacking the target, just biting something on the way
            if ((pTarget->getEntity().getEdict()->v.team == ALIEN_TEAM) && ((wpn.purpose & HELPS_FRIENDLY) != 0)) {
                selectThisWpn = true;

            } else if (shouldUseMeleeAttack && ((wpn.purpose & MELEE_ATTACK) != 0)) {
				selectThisWpn = true;
				
			} else if ((range >= wpn.min_distance) && (range <= wpn.max_distance)) {
				
				if ((pTarget == NULL) ||
					(pTarget->getInfo()->isBuilding() && ((wpn.purpose & ANTI_BUILDING) != 0)) ||
					(!pTarget->getInfo()->isBuilding() && ((wpn.purpose & ANTI_PLAYER) != 0) && pTarget->getEntity().getEdict()->v.team != ALIEN_TEAM))
				{
                    int sz = _lastWeaponFireTimes.size();
                    float lastTimeWeaponFired = _lastWeaponFireTimes[wpn.iId];
                    if (gpGlobals->time > lastTimeWeaponFired + wpn.min_fire_delay) {
                        float currentEnergy = getEnergy();
                        if (currentEnergy >= wpn.min_energy) {
                            //_log.Debug("Using weapon %s", wpn.weapon_name);
        					selectThisWpn = true;
                        }
                    }
				}
			}
		}
		
		if (selectThisWpn) {
            selectWeapon(wpn.iId);
			return true;
		}
		
		wpnIdx++;
	}
	
	return false;
}


bool Bot::selectWeapon(int wpnId)
{
    int wpnIdx = 0;
    tBotWeaponSelect& wpn = gWeaponSelect[0];
	while (wpn.iId != wpnId) {
        wpn = gWeaponSelect[++wpnIdx];
    }

    if (_currentWeaponId != wpnId || 
        gpGlobals->time > getProperty(kLastWeaponChangeTime) + WEAPON_CHANGE_TIME)
    {
        //_log.Debug("Selected weapon %s", wpn.weapon_name);
        selectItem(wpn.weapon_name);
        _currentWeaponId = wpn.iId;
        setProperty(kLastWeaponChangeTime);
        return true;
    }
    return false;
}


void Bot::fireWeapon()
{
    int wpnIdx = 0;
    tBotWeaponSelect& wpn = gWeaponSelect[0];
    while (wpn.iId != _currentWeaponId) {
        wpn = gWeaponSelect[++wpnIdx];
    }
    
    if (!_entity.isNull()) {
        if (_currentWeaponId > 0) {
            _lastWeaponFireTimes[_currentWeaponId] = gpGlobals->time;
        }
        _entity.getEdict()->v.button |= IN_ATTACK;
        setProperty(kWeaponFireTillTime,
            (wpn.fire_time != 0) ? wpn.fire_time + gpGlobals->time : 0);

		setWeaponImpulse();
    }
}


void Bot::setWeaponImpulse()
{
    switch (_currentWeaponId) {
    case WEAPON_LEAP:
        _entity.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_LEAP);
        break;
    case WEAPON_BLINK:
        _entity.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_LEAP);
        break;
    case WEAPON_CHARGE:
        _entity.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_CHARGE);
        break;
    default:
        break;
    }
}


void Bot::kill()
{
	if (!_entity.isNull()) {
		MDLL_ClientKill(_entity.getEdict());
	}
}


short Bot::getResources()
{
	if (!_entity.isNull()) {
		return (short)(_entity.getEdict()->v.vuser4.z / 100);

	} else {
		return 0;
	}
}


bool Bot::isCloaked()
{
	if (!_entity.isNull()) {
		return ((_entity.getEdict()->v.iuser4 & MASK_ALIEN_CLOAKED) != 0u);

	} else {
		return false;
	}
}


Target* Bot::getTarget()
{ 
    if ((_pTarget != NULL) && _pTarget->getEntity().isNull()) {
        setTarget(NULL);
    }
    return _pTarget;
}


void Bot::setTarget(Target* newTarget)
{ 
    if (_pTarget != NULL) {
        delete _pTarget;
    }
    if (newTarget == NULL) {
        _pTarget = NULL;

    } else {
        _pTarget = new Target(*newTarget);
    }
}


bool Bot::needsHealing()
{
    return (!_entity.isNull() && 
            (_entity.getEdict()->v.health < _entity.getEdict()->v.max_health) ||
            (_entity.getEdict()->v.armorvalue < WorldStateUtil::getMaxArmour(_entity.getEdict())));
}


void Bot::failedToBuildAt(tNodeId waypointId)
{
    if (find(_buildingBlacklist.begin(), _buildingBlacklist.end(), waypointId) == _buildingBlacklist.end()) {
        _buildingBlacklist.push_back(waypointId);
    }
}


bool Bot::hasFailedToBuildAt(tNodeId waypointId)
{
    return find(_buildingBlacklist.begin(), _buildingBlacklist.end(), waypointId) != _buildingBlacklist.end();
}


void Bot::generateStatusReport()
{
	_statusReporter.generateStatusReport(*this);
}


float Bot::getProperty(eBotProperty property, float defaultVal)
{
	tBotPropertyMap::iterator found = _properties.find((int)property);
	if (found != _properties.end()) {
		return found->second;

    } else { 
		return defaultVal;
	}
}


void Bot::setProperty(eBotProperty property, float val)
{
	_properties[(int)property] = val;
}


void Bot::checkPackEvolution()
{
	PackInfo* pInfo = HiveMind::getPackManager().getPackInfo(*this);
	if (pInfo != NULL) {
		Bot* pLeader = pInfo->getLeader();
		if (pLeader != NULL && this != pLeader && _evolution > pLeader->getEvolution()) {
			_log.Debug("Bot evolving into pack leader");
			HiveMind::getPackManager().promoteBot(*this);
		}
	}
}


void Bot::handleDamage(byte armourDamage, byte healthDamage, const Vector& dmgOrigin)
{
	_log.Debug("Handling damage [armour=%d, health=%d]", armourDamage, healthDamage);
	// ok, let's see if we can find the damage dealer
	for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
		CBaseEntity* pPlayerBaseEntity = UTIL_PlayerByIndex(ii);

		if (pPlayerBaseEntity != NULL) {
			edict_t* pPlayerEdict = INDEXENT(ii);
			
            if (WorldStateUtil::isPlayerPresent(pPlayerEdict) && (pPlayerEdict->v.skin >= 0)) {
				float range = (pPlayerEdict->v.origin - dmgOrigin).Length();
				if (range < 10) {
					// consider this the damage dealer, we'll short cut and stop here
					_pSensor->forceScanEntity(pPlayerEdict);
					break;
				}
			}
		}
	}
}


bool Bot::isInjured()
{
	return _entity.getEdict()->v.health < _entity.getEdict()->v.max_health ||
		_entity.getEdict()->v.armorvalue < WorldStateUtil::getMaxArmour(_entity.getEdict());
}
