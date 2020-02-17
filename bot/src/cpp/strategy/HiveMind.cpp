//
// $Id: HiveMind.cpp,v 1.85 2005/12/12 06:57:02 clamatius Exp $

// Copyright (c) 2003, WhichBot Project
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

#include "BotTypedefs.h"
#include "config/Config.h"
#include "config/EntityInfo.h"
#include "combat/CombatStrategy.h"
#include "strategy/HiveMind.h"
#include "strategy/AmbushStrategy.h"
#include "strategy/AttackStrategy.h"
#include "strategy/BuilderStrategy.h"
#include "strategy/FleeStrategy.h"
#include "strategy/RescueStrategy.h"
#include "strategy/ScoutStrategy.h"
#include "BotManager.h"
#include "worldstate/HiveManager.h"
#include "worldstate/WorldStateUtil.h"

const float RANDOM_VISIT_TIME_VARIANCE = 300;
vector<float> HiveMind::_visitTimes;
EntityRegistry HiveMind::_entities;
Log HiveMind::_log("strategy/HiveMind");
PackManager HiveMind::_wolfPackMgr;
tEvolution HiveMind::_lifeformOverride = kInvalidEvolution;

static int CFG_MAX_NUM_GORGES = 1;
static float CFG_MAX_FRACTION_FADE = 0.49;
static float CFG_MAX_FRACTION_ONOS = 0.2;

void HiveMind::reset()
{
    _visitTimes.resize(gpBotManager->getWaypointManager().getNumWaypoints());

    addInfluencingEntities("team_command");
    addInfluencingEntities("resourcetower");
    addInfluencingEntities("func_resource");
    addInfluencingEntities("alienresourcetower");
    addInfluencingEntities("team_hive");
	//_wolfPackMgr.clear();

    CFG_MAX_NUM_GORGES = Config::getInstance().getTweak("admin/max_num_gorges", CFG_MAX_NUM_GORGES);
    CFG_MAX_FRACTION_FADE = Config::getInstance().getTweak("admin/max_fraction_fade", CFG_MAX_FRACTION_FADE);
    CFG_MAX_FRACTION_ONOS = Config::getInstance().getTweak("admin/max_fraction_onos", CFG_MAX_FRACTION_ONOS);
}


/// This should be called every frame.
void HiveMind::tick()
{
}


bool HiveMind::entityEntryIsInvalid(edict_t* pEdict)
{
	EntityRegistry::Entry* pEntry = _entities.getEntry(pEdict);
    if (pEntry != NULL) {
        HiveMindEntityInfo* pEntityInfo = pEntry->pExtraData;
        if (pEntityInfo != NULL) {
            // If the entity has become null since last time we looked, get rid of it
            return pEntry->entity.isNull();

        } else {
            return true;
        }

    } else {
        // we didn't even find it in the container, which seems very odd. let's kill it.
        return true;
    }
}


void HiveMind::cleanupEntities()
{
	_entities.clear();
}


void HiveMind::visitedWaypoint(tNodeId wptId)
{
    if (wptId >= 0 && wptId < _visitTimes.size()) {
        _visitTimes[wptId] = gpGlobals->time;
    }
}


void HiveMind::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	// Currently we're dropping evolution on the floor.  May be useful later for an
	// ambush strategy.
	visitedWaypoint(wptId);
}


float HiveMind::getVisitTime(tNodeId wptId)
{
    if (wptId >= 0 && wptId < _visitTimes.size()) {
        return _visitTimes[wptId];

    } else {
        return 0.0;
    }
}


void HiveMind::entitySeen(tEvolution evolution, edict_t* pEdict)
{
	if (pEdict != NULL) {
		tNodeId nearestWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(evolution, pEdict->v.origin, pEdict);
		EntityRegistry::Entry* pEntry = _entities.getEntry(pEdict);
		if (pEntry != NULL) {
			pEntry->pExtraData->currentNearestWpId = nearestWaypointId;

		} else {
			_entities.addEntry(pEdict, new HiveMindEntityInfo(nearestWaypointId));
		}
	}
}


tNodeId HiveMind::getSeenWaypoint(edict_t* pSeenEdict)
{
	EntityRegistry::Entry* pEntry = _entities.getEntry(pSeenEdict);
	if (pEntry != NULL) {
		return pEntry->pExtraData->currentNearestWpId;

    } else {
        return INVALID_NODE_ID;
    }
}


void HiveMind::entityDestroyed(edict_t* pEdict)
{
	_entities.removeEntry(pEdict);
}



vector<EntityReference> HiveMind::getEntitiesSeen()
{
	return _entities.getAllEntities();
}


tEvolution HiveMind::getDesiredEvolution(Bot& thisBot)
{
    if (_lifeformOverride != kInvalidEvolution) {
        return _lifeformOverride;
    }
    return gpBotManager->inCombatMode() ? getDesiredCombatEvolution(thisBot) : getDesiredClassicEvolution(thisBot);
}


tEvolution HiveMind::getDesiredClassicEvolution(Bot& thisBot)
{
	if (thisBot.getRole() == kGorgeRole) {
		return kGorge;
	}

    // For the sake of argument, currently the first bot is a gorge unless it's the only bot and we have 2 hives
    // already or another player is already gorging
    int numBots = 0;
	int futureCount[5];
	for (int ii = 0; ii < 5; ii++) {
		futureCount[ii] = 0;
	}

    for (vector<Bot*>::iterator jj = gpBotManager->begin(); jj != gpBotManager->end(); jj++) {
        Bot* pBot = *jj;
        if ((pBot != NULL) && (&thisBot != pBot)) {
			futureCount[pBot->getDesiredEvolution()]++;
        }
        numBots++;
    }

    assert(numBots > 0);

    if (numBots > 0) {
        tEvolution advancedEvolution = ((futureCount[kOnos]+1.0)/numBots < CFG_MAX_FRACTION_ONOS) ? kOnos : kFade;
        
		tEvolution alternative = (futureCount[kLerk] == 0 && HiveManager::getActiveHiveCount() >= 2) ? kLerk : kSkulk;

        return ((futureCount[kFade]+1.0)/numBots < CFG_MAX_FRACTION_FADE) ? advancedEvolution : alternative;

    } else {
        return kSkulk;
    }
}


tEvolution HiveMind::getDesiredCombatEvolution(Bot& thisBot)
{
	if (thisBot.getRole() == kGorgeRole) {
		return kGorge;
	}

    int numBots = 0;
    int numFutureFades = 0;
    int numFutureLerks = 0;

    for (vector<Bot*>::iterator ii = gpBotManager->begin(); ii != gpBotManager->end(); ii++) {
        Bot* pBot = *ii;
        if ((pBot != NULL) && (&thisBot != pBot)) {
            if (pBot->getDesiredEvolution() == kFade) {
                numFutureFades++;
            }
            if (pBot->getDesiredEvolution() == kLerk) {
                numFutureLerks++;
            }
        }
        numBots++;
    }

    if (numBots > 0) {
        if (((float)numFutureFades) / numBots < 0.4) {
            return kFade;
        }
        if (((float)numFutureLerks) / numBots < 0.2) {
            return kLerk;
        }
    }

    return kSkulk;
}


void HiveMind::setStrategies (Bot& bot)
{
	switch (bot.getRole()) {
	case kGorgeRole:
		setGorgeStrategies(bot);
		break;
	case kScoutRole:
		setStandardStrategies(bot);
		break;
	case kPack1Role:
	case kPack2Role:
	case kPack3Role:
	case kPack4Role:
	case kPack5Role:
	case kPack6Role:
	case kPack7Role:
	case kPack8Role:
		_wolfPackMgr.setPackStrategies(bot);
		break;
	}
}

int HiveMind::getDesiredUpgrade(Bot& thisBot)
{
    return gpBotManager->inCombatMode() ? getDesiredCombatUpgrade(thisBot) : getDesiredClassicUpgrade(thisBot);
}

int HiveMind::getDesiredClassicUpgrade(Bot& thisBot)
{
    if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_CARAPACE)) && 
        !thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_REGENERATION)) && 
        !thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_REDEMPTION)) && 
        HiveManager::isTraitAvailable(DEFENSE_TRAIT) && 
        HiveManager::getTraitLevel(DEFENSE_TRAIT) == 3)
    {
        if (thisBot.getDesiredEvolution() == kGorge) {
            return Config::getInstance().getImpulse(IMPULSE_REDEMPTION);
            
        } else if (thisBot.getDesiredEvolution() != kSkulk) {
            return Config::getInstance().getImpulse(IMPULSE_REGENERATION);
            
        } else {
            return Config::getInstance().getImpulse(IMPULSE_CARAPACE);
        }
        
    } else if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_SILENCE)) && 
               !thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_ADRENALINE)) && 
               !thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_CELERITY)) && 
               HiveManager::getTraitLevel(MOVEMENT_TRAIT) == 3)
    {
        if (thisBot.getDesiredEvolution() == kLerk) {
            return Config::getInstance().getImpulse(IMPULSE_ADRENALINE);
        }
        if (thisBot.getDesiredEvolution() == kSkulk ||
            ((thisBot.getDesiredEvolution() == kOnos || 
              thisBot.getDesiredEvolution() == kFade) && 
              thisBot.getResources() < 20))
        {
            // Let's grab silence sometimes if we're going to be a skulk for a good while
            if (RANDOM_FLOAT(0, 100) < 50) {
                return Config::getInstance().getImpulse(IMPULSE_SILENCE);
                
            } else {
                return Config::getInstance().getImpulse(IMPULSE_CELERITY);
            }
            
        } else {
            return Config::getInstance().getImpulse(IMPULSE_CELERITY);
        }

    } else if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_FOCUS)) &&
        HiveManager::getTraitLevel(SENSORY_TRAIT) == 3) {
        if (thisBot.getDesiredEvolution() != kOnos) {
            return Config::getInstance().getImpulse(IMPULSE_FOCUS);
        } else {
            return Config::getInstance().getImpulse(IMPULSE_CLOAK);
        }
    }
    return 0;
}


vector<int> getDesiredSkulkUpgrades()
{
    vector<int> desiredSkulkUpgrades;
    desiredSkulkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CARAPACE));
    desiredSkulkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CELERITY));
    desiredSkulkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REGENERATION));
    desiredSkulkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_SILENCE));
    desiredSkulkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REDEMPTION));
    return desiredSkulkUpgrades;
}


vector<int> getDesiredLerkUpgrades()
{
    vector<int> desiredLerkUpgrades;
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CARAPACE));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CELERITY));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_ADRENALINE));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REGENERATION));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_SILENCE));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_FOCUS));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REDEMPTION));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_HIVE2));
    desiredLerkUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_HIVE3));
    return desiredLerkUpgrades;
}


vector<int> getDesiredFadeUpgrades()
{
    vector<int> desiredFadeUpgrades;
    desiredFadeUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CARAPACE));
    desiredFadeUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CELERITY));
    desiredFadeUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_SILENCE));
    desiredFadeUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REDEMPTION));
    return desiredFadeUpgrades;
}


vector<int> getDesiredOnosUpgrades()
{
    vector<int> desiredOnosUpgrades;
    desiredOnosUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_CELERITY));
    desiredOnosUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_ADRENALINE));
    desiredOnosUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REGENERATION));
    desiredOnosUpgrades.push_back(Config::getInstance().getImpulse(IMPULSE_REDEMPTION));
    return desiredOnosUpgrades;
}


int HiveMind::getDesiredCombatUpgrade(Bot& thisBot)
{
    if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_HIVE2)) && thisBot.getDesiredEvolution() == kSkulk) {
        return Config::getInstance().getImpulse(IMPULSE_HIVE2);
    }

    // celerity > almost all
    if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_CELERITY))) {
        return Config::getInstance().getImpulse(IMPULSE_CELERITY);
    }

    if (thisBot.getEvolution() != thisBot.getDesiredEvolution()) {
        return 0;
    }

    // lucky dip, pick one.  if we've already got it, we'll pick another in a second or two
    // clearly not very elegant but it should do the job. :)
    vector<int>& desiredUpgrades = getDesiredSkulkUpgrades();

    switch (thisBot.getDesiredEvolution()) {
    case kSkulk:
        if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_FOCUS))) {
            return Config::getInstance().getImpulse(IMPULSE_FOCUS);
        }
        break;
    case kLerk:
        if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_ADRENALINE))) {
            return Config::getInstance().getImpulse(IMPULSE_ADRENALINE);
        }
        desiredUpgrades = getDesiredLerkUpgrades();
        break;
    case kFade:
		if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_REGENERATION))) {
			return Config::getInstance().getImpulse(IMPULSE_REGENERATION);
		}
		if (!thisBot.hasTrait(Config::getInstance().getImpulse(IMPULSE_HIVE2))) {
			return Config::getInstance().getImpulse(IMPULSE_HIVE2);
		}
        desiredUpgrades = getDesiredFadeUpgrades();
        break;
    case kOnos:
        desiredUpgrades = getDesiredOnosUpgrades();
        break;
    }

    int randomUpgradeIdx = RANDOM_LONG(0, desiredUpgrades.size() - 1);
    return desiredUpgrades[randomUpgradeIdx];
}

void HiveMind::unsetStrategies (Bot& bot)
{
	_wolfPackMgr.unsetPackStrategies(bot);
}


PackManager& HiveMind::getPackManager ()
{
	return _wolfPackMgr;
}


void HiveMind::entityUnderAttack (edict_t* pEdict)
{
	tNodeId nearestWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(kSkulk, pEdict->v.origin, pEdict);
	EntityRegistry::Entry* pEntry = _entities.getEntry(pEdict);
    if (pEntry != NULL) {
        HiveMindEntityInfo* pEntityInfo = pEntry->pExtraData;
        if (pEntityInfo != NULL) {
            pEntityInfo->lastAttacked = gpGlobals->time;
        }

    } else {
		// Haven't seen this entity yet.  Now we know something about it.
        HiveMindEntityInfo* pEntityInfo = new HiveMindEntityInfo(nearestWaypointId);
		pEntityInfo->lastAttacked = gpGlobals->time;
		_entities.addEntry(pEdict, pEntityInfo);
    }
}


void HiveMind::entityUnderAttack(const Vector& loc)
{
    CBaseEntity* pEntity = NULL;
    while ((pEntity = UTIL_FindEntityInSphere(pEntity, loc, 10)) != NULL) {
    	const char* classname = STRING(pEntity->edict()->v.classname);
        if ((strncmp(classname, "alienresourcetower", strlen("alienresourcetower")) == 0) ||
            (strncmp(classname, "defensechamber", strlen("defensechamber")) == 0) ||
            (strncmp(classname, "movementchamber", strlen("movementchamber")) == 0) ||
            (strncmp(classname, "sensorychamber", strlen("sensorychamber")) == 0))
        {
            entityUnderAttack(pEntity->edict());
        }
    }
}


// TODO:  Read this from a config file.
static const float ATTACK_TIMEOUT = 60.0;  // seconds
bool HiveMind::isEntityUnderAttack (edict_t* pEdict)
{
	bool result = false;
	EntityRegistry::Entry* pEntry = _entities.getEntry(pEdict);
	if (pEntry != NULL) {
		float currentTime = gpGlobals->time;
        HiveMindEntityInfo* pEntityInfo = pEntry->pExtraData;

		if (pEntityInfo != NULL && pEntityInfo->lastAttacked > 0) {
			// Ok, the entity (still) needs help.  Is it an entity we care about?
			EntityInfo* pInfo = Config::getInstance().getEntityInfo(STRING(pEdict->v.classname));
			if ((pInfo != NULL) && ((pInfo->getTeam() == ALIEN_TEAM) || (pInfo->getTeam() == NO_TEAM))) {
				// TODO:  Prioritization of entities that we care to try to rescue.
				if ((currentTime - pEntityInfo->lastAttacked) < ATTACK_TIMEOUT) {
					result = true;
					
				} else {
					// The attack status is stale, let's expire it.
					pEntityInfo->lastAttacked = 0;
					pEntityInfo->numRescuers = 0;
				}
			}
		}
	}
	return result;
}


// TODO:  Read these from a config file.
static const int MAX_RESCUERS = 2;  // Try to get 2 bots to respond to the attack.

EntityReference* HiveMind::getEntityToRescue ()
{
    Config& config = Config::getInstance();
	float currentTime = gpGlobals->time;
    for (EntityRegistry::iterator ii = _entities.begin(); ii != _entities.end(); ii++) {
		edict_t* pCandidate = ii->first;
		EntityRegistry::Entry* pEntry = ii->second;

		if (pEntry != NULL && !pEntry->entity.isNull()) {
			HiveMindEntityInfo* pEntityInfo = ii->second->pExtraData;
			// Is the entity under attack?
			if (isEntityUnderAttack(pCandidate)) {
				// Attack is still recent enough, let's try to save it.
				if (pEntityInfo->numRescuers < MAX_RESCUERS) {
					pEntityInfo->numRescuers++;
					return &pEntry->entity;
				}
			}
		}
	}

	// Nothing to rescue at the moment.  Check back later.
	return NULL;
}

// ---------
// PROTECTED
// ---------
void HiveMind::addInfluencingEntities(const char* classname)
{
	CBaseEntity* pEntity = NULL;
	do {
		pEntity = UTIL_FindEntityByClassname(pEntity, classname);
		if (pEntity != NULL) {
            edict_t* pEdict = pEntity->edict();
            entitySeen(kSkulk, pEdict);
        }
    } while (pEntity != NULL);
}


void HiveMind::setStandardStrategies(Bot& bot)
{
	StrategyManager& stratMgr = bot.getStrategyManager();
	PathManager& pathMgr = bot.getPathManager();

	// Let's clear out any existing strategies that the bot may have.
	stratMgr.clear();
	stratMgr.addStrategy(new AttackStrategy(bot), 0.10);
	stratMgr.addStrategy(new ScoutStrategy(bot), 0.10);
    stratMgr.addStrategy(new RescueStrategy(bot), 0.80);
	// Combat is really important when happening, so it gets a very high weight
	stratMgr.addStrategy(new CombatStrategy(bot), 100.0);
	stratMgr.addStrategy(new FleeStrategy(bot), 100.0);
	stratMgr.addStrategy(new AmbushStrategy(bot), 0.10);
}


void HiveMind::setGorgeStrategies(Bot& bot)
{
	StrategyManager& stratMgr = bot.getStrategyManager();
	PathManager& pathMgr = bot.getPathManager();

	// Let's clear out any existing strategies that the bot may have.
	stratMgr.clear();
	// Add a negatively weighted attack strategy, so the bot tends not to go near marine buildings
	if (HiveManager::getActiveHiveCount() == 1) {
		stratMgr.addStrategy(new AttackStrategy(bot), -0.01);
	}
	// and of course a strategy to boldly go where no gorge has gone before
	stratMgr.addStrategy(new BuilderStrategy(bot), 1.0);
	// Combat is really important when happening, so it gets a very high weight
	stratMgr.addStrategy(new CombatStrategy(bot), 100.0);
	stratMgr.addStrategy(new FleeStrategy(bot), 100.0);
}


bool HiveMind::resourceNodeIsTaken(Vector& nodeLoc, int nodeMask)
{
	const char* alienResClassname = "alienresourcetower";
	const char* marineResClassname = "resourcetower";

    CBaseEntity* pTower = NULL;

    if ((nodeMask & ALIEN_NODE_MASK) != 0) {
        while ((pTower = UTIL_FindEntityByClassname(pTower, alienResClassname)) != NULL) {
            float range = (nodeLoc - pTower->pev->origin).Length();
            if (range <= 1.0) {
                return true;
            }
        }
    }

    if ((nodeMask & MARINE_NODE_MASK) != 0) {
        while ((pTower = UTIL_FindEntityByClassname(pTower, marineResClassname)) != NULL) {
            float range = (nodeLoc - pTower->pev->origin).Length();
            if (range <= 1.0) {
                return true;
            }
        }
    }

    return false;
}


tNodeId HiveMind::getNearestWaypoint(tEvolution forEvolution, edict_t* pEdict)
{
    if (FNullEnt(pEdict)) {
        return -1;
    }

	EntityRegistry::Entry* pEntry = _entities.getEntry(pEdict);
    tNodeId nearestWptId = -1;

    if (pEntry != NULL) {
        HiveMindEntityInfo* pInfo = pEntry->pExtraData;
        if (pInfo != NULL) {
            nearestWptId = pInfo->currentNearestWpId;
        }
    }

    if (nearestWptId >= 0) {
        return nearestWptId;

    } else {
        return gpBotManager->getWaypointManager().getNearestWaypoint(forEvolution, pEdict->v.origin, NULL);
    }
}


int HiveMind::getNumEntitiesSeen(const string& className)
{
	EntityRegistry::tEntrySet* pEntrySet = _entities.getEntries(className);
	return pEntrySet != NULL ? pEntrySet->size() : 0;
}


void HiveMind::setLifeformOverride(tEvolution lifeform)
{
    _lifeformOverride = lifeform;
}
