//
// $Id: BotSensor.cpp,v 1.8 2005/01/04 01:18:06 clamatius Exp $

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
#include "sensory/BotSensor.h"
#include "BotManager.h"
#include "Bot.h"
#include "config/TranslationManager.h"
#include "worldstate/AreaManager.h"
#include "strategy/HiveMind.h"
#include "config/Config.h"
#include "worldstate/WorldStateUtil.h"
#include "sensory/AuditoryManager.h"

const float LISTEN_RANGE = 1000.0;
Log BotSensor::_log(__FILE__);

float BotSensor::_lastGlobalScanTime = 0.0f;

BotSensor::BotSensor(Bot& bot) :
    _bot(bot),
    _lastScanTime(0),
    _scanPeriod(0.5),
    _scanRange(1200),
	_threatSeen(false),
    _healableSeen(false)
{
}


void BotSensor::scan(bool force)
{
    if (force || 
		((gpGlobals->time - _lastScanTime > _scanPeriod) &&
		(gpGlobals->time - _lastGlobalScanTime > 
		 (_scanPeriod / ((float)gpBotManager->getNumBots())))))
	{
		clear();
        _lastScanTime = _lastGlobalScanTime = gpGlobals->time;

        CBaseEntity* pEntity = NULL;
		WorldStateUtil::checkVector(_bot.getEdict()->v.origin);
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, _bot.getEdict()->v.origin, _scanRange)) != NULL) {
            scanEntity(pEntity->edict());
        }

        AuditoryManager::getInstance().listenForSounds(_bot.getEdict()->v.origin, LISTEN_RANGE);
    }
}


/**
 * Can the observerEdict see the toPoint if its viewpoint is at fromPt?
 */
bool BotSensor::isPointVisible(const Vector& fromPt, const Vector& toPt, edict_t* observerEdict)
{
    TraceResult tr;
    // Can we see the target (ignore ourselves)?
	Vector checkedToPt(toPt);
	Vector checkedFromPt(fromPt);
	WorldStateUtil::checkVector(checkedToPt);
	WorldStateUtil::checkVector(checkedFromPt);
    if (observerEdict != NULL) {
		Vector start(fromPt + observerEdict->v.view_ofs);
		WorldStateUtil::checkVector(start);
     	UTIL_TraceLine(start, checkedToPt, ignore_monsters, observerEdict->v.pContainingEntity, &tr);
    } else {
        UTIL_TraceLine(checkedFromPt, checkedToPt, ignore_monsters, NULL, &tr);
    }
    return (tr.flFraction == 1.0);
}


bool BotSensor::entityIsVisible(edict_t* pEntity)
{
	return isPointVisible(_bot.getEdict()->v.origin, pEntity->v.origin + pEntity->v.view_ofs, _bot.getEdict());
}


void BotSensor::scanEntity(edict_t* pEntity)
{
	_scanRange = 1000;
	if (FNullEnt(pEntity)) {
		return;
	}
    const char* classname = STRING(pEntity->v.classname);
	
	// hard-code some checks for speed
	if ((strncmp(classname, "weapon_", 7) == 0) || (strncmp(classname, "info_", 5) == 0) ||
		(strncmp(classname, "ambient_", 8) == 0) || (strncmp(classname, "env_", 4) == 0) ||
		(strncmp(classname, "func_", 5) == 0) || (strncmp(classname, "target_", 7) == 0))
	{
		return;
	}


    if (pEntity->v.health > 0 && !FNullEnt(pEntity)) {
        if (entityIsVisible(pEntity)) {
			forceScanEntity(pEntity);
		}
	}
}
 

void BotSensor::forceScanEntity(edict_t* pEntity)
{
    Config& config = Config::getInstance();
    EntityInfo* pInfo = config.getEntityInfo(STRING(pEntity->v.classname));
	if (pInfo != NULL) {
		int team = pInfo->getTeam();

		switch (pInfo->getType()) {
		case kPlayer:
			if ((pEntity->v.deadflag != DEAD_NO) || (pEntity->v.health <= 0) || (pEntity == _bot.getEdict())) {
				return;
			}
			team = pEntity->v.team;
			if (team == MARINE_TEAM) {
				_threatSeen = true;
			}
			break;
    
		case kTurret:
			_threatSeen = true;
			addInfluencingEntity(pEntity, team);
			break;
    
		default:
			if ((team == MARINE_TEAM) || (team == ALIEN_TEAM)) {
				addInfluencingEntity(pEntity, team);
			}
			break;
		}

		if (team == MARINE_TEAM) {
			// Skulks just totally ignore electrified things now
			if (_bot.getEvolution() != kSkulk || !isElectrified(pEntity)) {
				Target target(pEntity, pInfo);
				_targets.push_back(target);
			}

		} else if (team == ALIEN_TEAM) {
			if (pInfo->isBuilding() && looksBuildable(pEntity)) {
				Target target(pEntity, pInfo);
				_buildables.push_back(target);
				_log.Debug("Saw buildable target %s", pInfo->getClassname().c_str());

			} else if (_bot.getEvolution() == kGorge && !pInfo->isBuilding() && 
				((pEntity->v.health < pEntity->v.max_health) || 
				 (pEntity->v.armorvalue < WorldStateUtil::getMaxArmour(pEntity))))
			{
				Target target(pEntity, pInfo);
				_targets.push_back(target);
				_log.Debug("Saw healable target");
				_healableSeen = true;
			}
		}
	}
}


bool BotSensor::looksBuildable(edict_t* pEntity)
{
	if (FNullEnt(pEntity)) {
		return false;
	}
	return ((pEntity->v.iuser4 & MASK_BUILDABLE) != 0) && (strcmp(STRING(pEntity->v.classname), "team_hive") != 0);
}


bool BotSensor::isElectrified(edict_t* pEntity)
{
    if (FNullEnt(pEntity)) {
        return false;
    }
    return ((pEntity->v.iuser4 & MASK_UPGRADE_11) != 0);
}

void BotSensor::addInfluencingEntity(edict_t* pEntity, int team)
{
    if (team != NO_TEAM) {
        tNodeId wptId = HiveMind::getSeenWaypoint(pEntity);
        if (wptId == INVALID_NODE_ID) {
            Bot* pBot = gpBotManager->getBot(_bot.getEdict());
            if (pBot != NULL) {
                if (team == MARINE_TEAM && strcmp(STRING(pEntity->v.classname), "mine") < 0) {
                    pBot->sayToTeam(
                        TranslationManager::getTranslation("hivemind_say_prefix") +
                        TranslationManager::getTranslation("seen_new") + " " +
                        TranslationManager::getTranslation(STRING(pEntity->v.classname)) + " " + 
                        TranslationManager::getTranslation("in") + " " +
                        TranslationManager::getTranslation(AreaManager::getAreaName(pEntity->v.origin)));
                }
            }
            HiveMind::entitySeen(pBot->getEvolution(), pEntity);
        }
    }
}


void BotSensor::clear()
{
    _lastScanTime = 0.0;
    _targets.clear();
    _buildables.clear();
    _threatSeen = false;
    _healableSeen = false;
	_bot.setTarget(NULL);
}
