//
// $Id: BotManager.cpp,v 1.21 2005/12/14 19:23:03 clamatius Exp $

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

#include "BotManager.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "extern/halflifesdk/cbase.h"
#include "worldstate/AreaManager.h"
#include "worldstate/HiveManager.h"
#include "strategy/HiveMind.h"
#include "NSConstants.h"
#include "config/TranslationManager.h"
#include "extern/metamod/meta_api.h"
#include "linkage/version.h"
#include "config/Config.h"
#include "worldstate/WorldStateUtil.h"
#include "sensory/AuditoryManager.h"

BotManager* gpBotManager = NULL;
Bot* gpCurrentThinkingBot = NULL;

Log BotManager::s_log(__FILE__);
int gLightningSpriteTexture;

const char* lifeformNames[] = { "skulk", "lerk", "gorge", "fade", "onos" };


static int CFG_MAX_NUM_BOTS = 64;

static cvar_t whichbotCVars[] = {
	{"whichbot_version", WHICHBOT_VERSION_STR, FCVAR_EXTDLL | FCVAR_SERVER | FCVAR_USERINFO, 0, NULL},
	
	{NULL,NULL,0,0,NULL},
};


#ifdef _WIN32
extern HINSTANCE h_Library;
#else
extern void* h_Library;
#endif

//
// Called once at startup time.
//
void BotManager::init()
{
   const char* gameDllPath = GET_GAME_INFO(PLID, GINFO_DLL_FULLPATH);

    #ifndef __linux__
        h_Library = LoadLibrary(gameDllPath);
    #else
        h_Library = dlopen(gameDllPath, RTLD_NOW);
    #endif

    _lastFrameTime = 0;
  	_cmdDispatcher.registerHandlers();
	_lastTeamBalanceCheckTime = -1;
	_teamBalanceEnabled = false;
    _noAutoDefenses = false;

    TranslationManager::loadDefaultTranslations();

    for (int ii = 0; whichbotCVars[ii].name != NULL; ii++) {
        CVAR_REGISTER(&whichbotCVars[ii]);
        (*g_engfuncs.pfnCVarSetString)(whichbotCVars[ii].name, whichbotCVars[ii].string);
    }
}

//
// Called once at the start of a new level.
//
void BotManager::newLevel()
{
    s_log.Debug("Starting new level");

    const char* mapName = STRING(gpGlobals->mapname);
    string mapNameStr(mapName);
    _inCombatMode = (mapNameStr.find("co_") == 0);

    _waypointManager.loadWaypoints(mapName);

    TranslationManager::loadMapTranslations(mapName);

	_levelInitialised = false;
   
    gLightningSpriteTexture = PRECACHE_MODEL("sprites/lgtning.spr");

    _lastBotAddedTime = gpGlobals->time;
	if (_lastTeamBalanceCheckTime >= 0) {
		_lastTeamBalanceCheckTime = gpGlobals->time;
	}
    _numBotsToAdd = 0;
    _lastReportingBot = 0;
    _statusReportTime = 0;

    // reset player enslavement
    HiveMind::getPackManager().enslavePacksToPlayer(NULL);

    CFG_MAX_NUM_BOTS = Config::getInstance().getTweak("admin/max_num_bots", CFG_MAX_NUM_BOTS);
	LOG_CONSOLE(PLID, "Maximum # of WhichBots set to %d", CFG_MAX_NUM_BOTS);
}

//
// Processed at the start of every frame.
//
// FIXME: CONFIG
const float TIME_TO_ADD_BOT = 2.0;
void BotManager::tick()
{
	_messageDispatcher.registerHandlers();
	if (!_levelInitialised) {
        while (_bots.size() != 0) {
            delete _bots.back();
            _bots.pop_back();
        }

        _waypointManager.initLevel();
		AreaManager::reset();
		HiveManager::reset();
		HiveMind::reset();
		_levelInitialised = true;
        startGame();
    }

    if (_lastFrameTime > gpGlobals->time + 0.1) {
        // a new game must have started
        startGame();
    }

    HiveMind::tick();

    for_each(_bots.begin(), _bots.end(), mem_fun(&Bot::tick));

	gpCurrentThinkingBot = NULL;

	checkBotsToAdd();

	checkTeamBalance();

    checkStatusReport();

    _waypointManager.tick();
	_lastFrameTime = gpGlobals->time;
}


// This will typically get called several times at the start of the game
void BotManager::startGame()
{
	HiveManager::reset();
    HiveMind::reset();

	AuditoryManager::getInstance().markAllMarinesHeard();
    //for_each(_bots.begin(), _bots.end(), mem_fun(&Bot::resetBot));
}


int BotManager::getTargetNumBots(int numNonWBPlayers)
{
	const char* keyName = inCombatMode() ? "admin/combat_team_balance" : "admin/classic_team_balance";
	int numBalanceSettings = Config::getInstance().getArraySize(keyName);
	int target = 0;
	if (numBalanceSettings > 0) {
		if (numNonWBPlayers < 0 || numNonWBPlayers > numBalanceSettings) {
			target = (int)Config::getInstance().getArrayTweak(keyName, numBalanceSettings - 1, 0.0);

		} else {
			target = (int)Config::getInstance().getArrayTweak(keyName, numNonWBPlayers, 0.0);
		}
	}
	return target;
}


void BotManager::checkBotsToAdd()
{
	if ((gpGlobals->time > _lastBotAddedTime + TIME_TO_ADD_BOT)) {
        _lastBotAddedTime = gpGlobals->time;

        if (_numBotsToAdd > 0 && _bots.size() + 1 < CFG_MAX_NUM_BOTS) {
            
            _numBotsToAdd--;
            Bot* newBot = new Bot(_bots.size());
            addBot(newBot);
            newBot->selectDesiredEvolution();
            
            s_log.Debug("Creating bot");
            newBot->create();
        }
    }
}


void BotManager::addBot()
{
    if (_waypointManager.getNumWaypoints() == 0) {
        LOG_CONSOLE(PLID, "No waypoints defined!  Not adding bot.");
        LOG_ERROR(PLID, "No waypoints defined!  Not adding bot.");

    } else if (!Config::getInstance().isValid()) {
		LOG_CONSOLE(PLID, "Config file missing, unreadable or corrupted.  Not adding bot.");
        LOG_ERROR(PLID, "Config file missing, unreadable or corrupted.  Not adding bot.");

	} else {
        _numBotsToAdd++;
    }
}


Bot* BotManager::getBot(edict_t* pEdict)
{
    for (tBotIterator ii = _bots.begin(); ii != _bots.end(); ii++) {
        Bot* bot = *ii;
        if (bot->getEdict() == pEdict) {
            // our bot got kicked by someone
            return bot;
        }
    }
    return NULL;
}


void BotManager::clientDisconnected(edict_t* pEdict)
{
    Bot* pBot = getBot(pEdict);
    if (pBot != NULL) {
		HiveMind::unsetStrategies(*pBot);
        removeBot(pBot);
		delete pBot;
    }
}


void BotManager::checkTeamBalance()
{
	if (_waypointManager.getNumWaypoints() != 0) {
		if (_lastTeamBalanceCheckTime >= 0) {
			if (gpGlobals->time > _lastTeamBalanceCheckTime + TIME_TO_ADD_BOT) {
				_lastTeamBalanceCheckTime = gpGlobals->time;
				
				int numNonWBPlayers = 0;
				int playerCount = 0;
                bool countMarinesOnly = Config::getInstance().getTweak("admin/balance_marines_only", 0) > 0;
				for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
					CBaseEntity* pPlayerBaseEntity = UTIL_PlayerByIndex(ii);

					if (pPlayerBaseEntity != NULL) {
						edict_t* pPlayerEdict = INDEXENT(ii);
						
                        if (WorldStateUtil::isPlayerPresent(pPlayerEdict) && (pPlayerEdict->v.skin >= 0)) {
							if (getBot(pPlayerEdict) == NULL) {
								// Don't count spectators and people in the ready room as players
								if (pPlayerEdict->v.team == MARINE_TEAM || (!countMarinesOnly && pPlayerEdict->v.team == ALIEN_TEAM)) {
									numNonWBPlayers++;
								}
							}
							playerCount++;
						}
					}
				}

				updateBotBalance(numNonWBPlayers, playerCount);
			}
		}
	}
}


bool BotManager::updateBotBalance(int numNonWBPlayers, int playerCount)
{
	bool cheatsEnabled = areCheatsEnabled();
	int totalPotentialBots = _bots.size() + _numBotsToAdd;
	int targetNumBots = getTargetNumBots(numNonWBPlayers);
	
	if ((_bots.size() > 0) && (totalPotentialBots > targetNumBots))
	{
		if (_numBotsToAdd > 0) {
			_numBotsToAdd--;
			
		} else {
			kickLastBot();
			return true;
		}
	}
	
	if ((totalPotentialBots < targetNumBots) && (playerCount+1 < gpGlobals->maxClients))
	{
		_numBotsToAdd++;
		return true;
	}

	return false;
}


void BotManager::kick(const char* name)
{
	char cmd[80];
	sprintf(cmd, "kick \"%s\"\n", name);
	SERVER_COMMAND(cmd);
}


void BotManager::kickLastBot()
{
	if (_bots.size() > 0) {
		Bot* pBot = _bots.front();

		if (pBot->getName() != NULL) {
			kick(pBot->getName()->c_str());
		}
	}
}


void BotManager::enableTeamBalance(bool enable)
{
	if (enable) {
		if (_waypointManager.getNumWaypoints() == 0) {
			LOG_ERROR(PLID, "No waypoints defined!  Not changing team balance.");
			LOG_CONSOLE(PLID, "No waypoints defined!  Not changing team balance.");
			
		} else if (!Config::getInstance().isValid()) {
			LOG_CONSOLE(PLID, "Config file missing, unreadable or corrupted.  Not changing team balance.");
		    LOG_ERROR(PLID, "Config file missing, unreadable or corrupted.  Not changing team balance.");

		} else {
			LOG_CONSOLE(PLID, "Team balance enabled.");
			_lastTeamBalanceCheckTime = gpGlobals->time;
			_teamBalanceEnabled = enable;
		}
        
	} else {
		LOG_CONSOLE(PLID, "Team balance disabled.");
		_lastTeamBalanceCheckTime = -1;
    }
}


void BotManager::terrainUpdated(tEvolution forEvolution, int startWptId, int endWptId, float costDiff)
{
    for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ii++) {
        Bot* thisBot = *ii;
        assert(thisBot != NULL);

        if ((thisBot != NULL) && (thisBot->getEvolution() == forEvolution)) {
            thisBot->getPathManager().addCorrectionToApply(costDiff);
        }
    }
}


bool BotManager::areCheatsEnabled()
{
	float cheats = CVAR_GET_FLOAT("sv_cheats");
	return cheats == 1;
}



void BotManager::addBot(Bot* newBot)
{
    _bots.push_back(newBot);
}


vector<Bot*>::iterator BotManager::begin()
{
    return _bots.begin();
}


vector<Bot*>::iterator BotManager::end()
{
    return _bots.end();
}


void BotManager::removeBot(Bot* oldBot)
{
    vector<Bot*>::iterator found = find(_bots.begin(), _bots.end(), oldBot);
    if (found != _bots.end()) {
        _bots.erase(found);
    }
}


bool BotManager::inCombatMode()
{
    return _inCombatMode;
}


int BotManager::getBotIndex(Bot& bot)
{
    vector<Bot*>::iterator found = find(_bots.begin(), _bots.end(), &bot);
    if (found != _bots.end()) {
        return found - _bots.begin();

    } else {
        return -1;
    }
}


void BotManager::triggerStatusReports()
{
    _statusReportTime = gpGlobals->time;
}


void BotManager::checkStatusReport()
{
    if (_statusReportTime > 0 && gpGlobals->time >= _statusReportTime) {
        if (_lastReportingBot < _bots.size()) {
            Bot* pBot = _bots[_lastReportingBot];
            if (pBot != NULL) {
                pBot->generateStatusReport();
            }

            _lastReportingBot++;
            _statusReportTime += 0.3;

        } else {
            _lastReportingBot = 0;
            _statusReportTime = 0;
        }
    }
}


void BotManager::toggleCheats(bool enable)
{
    (*g_engfuncs.pfnCVarSetFloat)("sv_cheats", enable ? 1 : 0);
}
