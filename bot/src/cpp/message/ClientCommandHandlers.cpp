//
// $Id: ClientCommandHandlers.cpp,v 1.14 2005/02/25 23:38:51 clamatius Exp $

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

#include "framework/Log.h"
#include "message/ClientCommandDispatcher.h"
#include "BotManager.h"
#include "extern/metamod/meta_api.h"
#include "config/TranslationManager.h"
#include "worldstate/AreaManager.h"
#include "strategy/HiveMind.h"

static Log _log(__FILE__);

void handleBotAdd(edict_t* pEdict, vector<string>& args)
{
	gpBotManager->addBot();
}


void handleBotRemove(edict_t* pEdict, vector<string>& args)
{
    gpBotManager->kickLastBot();
}


void handlePlayerInfo(edict_t* pEdict, vector<string>& args)
{
	CBaseEntity* pPlayer = UTIL_PlayerByIndex(1);

	if (pPlayer != NULL) {
		LOG_CONSOLE(PLID, "Player origin: %f %f %f", pPlayer->pev->origin.x, pPlayer->pev->origin.y, pPlayer->pev->origin.z);

	} else {
		LOG_CONSOLE(PLID, "Player not found for pinfo command");
	}
}


void handleTeamBalance(edict_t* pEdict, vector<string>& args)
{
    if (args.size() == 2) {
		string enableArg = args[1];
        if (enableArg.compare("on") == 0) {
            gpBotManager->enableTeamBalance(true);
			return;
		}
    }
    gpBotManager->enableTeamBalance(false);
}


void handleLogMessage(edict_t* pEdict, vector<string>& args)
{
    if (args.size() == 2) {
        string msg = args[1];
		_log.FileLog(msg.c_str());
    }
}


void handleDisableNetMessage(edict_t* pEdict, vector<string>& args)
{
    if (args.size() == 2) {
        int msgType = atoi(args[1].c_str());
		gpBotManager->getMessageDispatcher().disable(msgType);
    }
}


void handleEnableNetMessage(edict_t* pEdict, vector<string>& args)
{
    if (args.size() == 2) {
        int msgType = atoi(args[1].c_str());
		gpBotManager->getMessageDispatcher().enable(msgType);
    }
}


void handleWaypointDebug(edict_t* pEdict, vector<string>& args)
{
    gpBotManager->getWaypointManager().toggleDebugger();
}


void handleWaypointSave(edict_t* pEdict, vector<string>& args)
{
    gpBotManager->getWaypointManager().saveWaypoints(STRING(gpGlobals->mapname));
}


void handleWaypointAdd(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().addWaypoint(pPlayerEdict->v.origin);

    } else {
        LOG_ERROR(PLID, "Couldn't add waypoint - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't add waypoint - more than one player logged in?");
    }
}


void handleWaypointPathStartAdd(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().
            startWaypointPathAdd(pPlayerEdict->v.origin, pPlayerEdict);

    } else {
        LOG_ERROR(PLID, "Couldn't mark waypoint path add - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't mark waypoint path add - more than one player logged in?");
    }
}


void handleWaypointPathEndAdd(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().
            endWaypointPathAdd(pPlayerEdict->v.origin, pPlayerEdict);

    } else {
        LOG_ERROR(PLID, "Couldn't end waypoint path add - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't end waypoint path add - more than one player logged in?");
    }
}


void handleWaypointPathStartDelete(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().
            startWaypointPathDelete(pPlayerEdict->v.origin, pPlayerEdict);

    } else {
        LOG_ERROR(PLID, "Couldn't mark waypoint path deletion - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't mark waypoint path deletion - more than one player logged in?");
    }
}


void handleWaypointPathEndDelete(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().
            endWaypointPathDelete(pPlayerEdict->v.origin, pPlayerEdict);

    } else {
        LOG_ERROR(PLID, "Couldn't end waypoint path deletion - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't end waypoint path deletion - more than one player logged in?");
    }
}


void handleWaypointDelete(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().
            deleteWaypoint(pPlayerEdict->v.origin, pPlayerEdict);

    } else {
        LOG_ERROR(PLID, "Couldn't do waypoint deletion - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't do waypoint deletion - more than one player logged in?");
    }
}


void toggleWaypointFlag(int flag)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
    if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().toggleWaypointFlag(pPlayerEdict->v.origin, pPlayerEdict, flag);

    } else {
        LOG_ERROR(PLID, "Couldn't toggle waypoint flag - more than one player logged in?");
        LOG_CONSOLE(PLID, "Couldn't toggle waypoint flag - more than one player logged in?");
    }
}


void handleWaypointDoor(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_DOOR);
}


void handleWaypointLadder(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_LADDER);
}


void handleWaypointLift(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_LIFT);
}


void handleWaypointLiftSwitch(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_LIFT_SWITCH);
}


void handleWaypointLiftWait(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_LIFT_WAIT);
}


void handleWaypointForcedWalkable(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_FORCED_WALKABLE);
}


void handleWaypointJump(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_JUMP);
}


void handleWaypointAmbush(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_AMBUSH);
}


void handleWaypointPathDropStart(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_DROP_TOP);
}



void handleWaypointPathDropEnd(edict_t* pEdict, vector<string>& args)
{
    toggleWaypointFlag(W_FL_DROP_BOTTOM);
}


void handleNetDebug(edict_t* pEdict, vector<string>& args)
{
    gpBotManager->getMessageDispatcher().toggleDebug();
}


void handleEntityScan(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
	if (pPlayerEdict != NULL) {
		CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, pPlayerEdict->v.origin, 100)) != NULL) {
			const char* classname = STRING(pEntity->edict()->v.classname);
			LOG_CONSOLE(PLID, "Entity: %s", classname);
        }
	}
}


void handleWaypointWalkability(edict_t* pEdict, vector<string>& args)
{
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
	if (pPlayerEdict != NULL) {
        gpBotManager->getWaypointManager().calculateWalkability();
        LOG_CONSOLE(PLID, "Walkable nodes recalculated");
    }
}


int waypointCheck(const char* classname, float minRange)
{
    int numBeams = 0;
    edict_t* pPlayerEdict = WaypointDebugger::getEditingPlayer();
	if (pPlayerEdict != NULL) {
        CBaseEntity* pEntity = NULL;
        do {
            pEntity = UTIL_FindEntityByClassname(pEntity, classname);
            if (pEntity != NULL) {
                edict_t* pEdict = pEntity->edict();
                if (!FNullEnt(pEdict)) {
                    
                    int nearestWptId = gpBotManager->getWaypointManager().getNearestWaypoint(kGorge, pEdict->v.origin, NULL);
                    float range = 1000000.0;
                    if (nearestWptId >= 0) {
                        Vector rel(pEdict->v.origin - gpBotManager->getWaypointManager().getOrigin(nearestWptId));
                        if (rel.Length() > minRange || 
                            (gpBotManager->getWaypointManager().getFlags(nearestWptId) & W_FL_WALKABLE) == 0)
                        {
                            if (++numBeams > 3) {
                                return numBeams;
                            }
                            string areaName = TranslationManager::getTranslation(AreaManager::getAreaName(pEdict->v.origin));
                            LOG_CONSOLE(PLID, "Found unwalkable %s in %s", classname, areaName.c_str());
                            WaypointDebugger::drawDebugBeam(pPlayerEdict->v.origin, pEdict->v.origin, 255, 0, 0);
                        }
                    }
                }
            }
        } while (pEntity != NULL);
    }
    return numBeams;
}


void handleWaypointCheck(edict_t* pEdict, vector<string>& args)
{
    LOG_CONSOLE(PLID, "Checking walkability of hives and resource nozzles");

    int numBeams = waypointCheck("team_hive", 600);
    numBeams += waypointCheck("func_resource", 200);

    if (numBeams == 0) {
        LOG_CONSOLE(PLID, "Hives and resource nozzles appear walkable");
    }
}


void handleNoAutoDefenses(edict_t* pEdict, vector<string>& args)
{
    if ((args.size() == 3) && (strncmp(args[1].c_str(), "on", 2) == 0)) {
        gpBotManager->setNoAutoDefenses(true);
        LOG_CONSOLE(PLID, "No auto defenses mode enabled - sentry turrets and mines are no longer buildable");

    } else {
        gpBotManager->setNoAutoDefenses(false);
        LOG_CONSOLE(PLID, "No auto defenses mode disabled");
    }
}


void handleStatus(edict_t* pEdict, vector<string>& args)
{
    gpBotManager->triggerStatusReports();
}


void handleLead(edict_t* pEdict, vector<string>& args)
{
    if (pEdict != NULL) {
        HiveMind::getPackManager().enslavePacksToPlayer(pEdict);
    }
}


void handleResume(edict_t* pEdict, vector<string>& args)
{
	HiveMind::getPackManager().enslavePacksToPlayer(NULL);
}


void handleBoost(edict_t* pEdict, vector<string>& args)
{
    int argsSize = args.size();
    bool boosted = false;
    if (args.size() == 2) {
        int boostAmount = atoi(args[1].c_str());
        if (boostAmount > 0 && boostAmount < 15) {
            bool cheatsEnabled = gpBotManager->areCheatsEnabled();
            if (!cheatsEnabled) {
                gpBotManager->toggleCheats(true);
            }

            for (int ii = 0; ii < boostAmount; ii++) {
                for (tBotIterator jj = gpBotManager->begin(); jj != gpBotManager->end(); jj++) {
                    Bot* pBot = *jj;
                    if (pBot != NULL) {
                        pBot->giveCheatPoints();
                    }
                }
            }
            LOG_CONSOLE(PLID, "WhichBots boosted!");

            if (!cheatsEnabled) {
                gpBotManager->toggleCheats(false);
            }
            boosted = true;
        }
    }
    if (!boosted) {
        LOG_ERROR(PLID, "Boost syntax incorrect.  Usage: wb boost <number>");
        LOG_CONSOLE(PLID, "Boost syntax incorrect.  Usage: wb boost <number>");
    }
}


void handleEvolveOverride(edict_t* pEdict, vector<string>& args)
{
    if (args.size() == 2) {
        string specifiedLifeform(args[1]);
        for (int ii = 0; ii < NUM_EVOLUTIONS; ii++) {
            if (specifiedLifeform.compare(lifeformNames[ii]) == 0) {
                HiveMind::setLifeformOverride((tEvolution)ii);
                LOG_CONSOLE(PLID, "Set WhichBot lifeform override to %s", lifeformNames[ii]);
                return;
            }
        }
        HiveMind::setLifeformOverride(kInvalidEvolution);
        LOG_CONSOLE(PLID, "Disabled WhichBot lifeform override");
    }
}


void ClientCommandDispatcher::registerHandlers()
{
	if (_handlers.size() == 0) {
		registerHandler("add", handleBotAdd);
        registerHandler("remove", handleBotRemove);
		registerHandler("pinfo", handlePlayerInfo);
		registerHandler("balance", handleTeamBalance);
		registerHandler("log", handleLogMessage);
		registerHandler("disable", handleDisableNetMessage);
		registerHandler("enable", handleEnableNetMessage);
        registerHandler("wpdebug", handleWaypointDebug);
        registerHandler("wpsave", handleWaypointSave);
        registerHandler("wpadd", handleWaypointAdd);
        registerHandler("wpdel", handleWaypointDelete);
        registerHandler("wppathadd1", handleWaypointPathStartAdd);
        registerHandler("wppathadd2", handleWaypointPathEndAdd);
        registerHandler("wppathdel1", handleWaypointPathStartDelete);
        registerHandler("wppathdel2", handleWaypointPathEndDelete);
        registerHandler("wpdoor", handleWaypointDoor);
        registerHandler("wpladder", handleWaypointLadder);
		registerHandler("wpjump", handleWaypointJump);
		registerHandler("wplift", handleWaypointLift);
		registerHandler("wpliftswitch", handleWaypointLiftSwitch);
		registerHandler("wpliftwait", handleWaypointLiftWait);
        registerHandler("wpwalkability", handleWaypointWalkability);
        registerHandler("wpforcewalkable", handleWaypointForcedWalkable);
		registerHandler("wppathdrop1", handleWaypointPathDropStart);
		registerHandler("wppathdrop2", handleWaypointPathDropEnd);
        registerHandler("wpambush", handleWaypointAmbush);
        registerHandler("wpcheck", handleWaypointCheck);
        registerHandler("netdebug", handleNetDebug);
		registerHandler("entscan", handleEntityScan);
        registerHandler("noautodefenses", handleNoAutoDefenses);
        registerHandler("status", handleStatus);
        registerHandler("lead", handleLead);
		registerHandler("resume", handleResume);
        registerHandler("boost", handleBoost);
        registerHandler("evolve", handleEvolveOverride);
    }
}
