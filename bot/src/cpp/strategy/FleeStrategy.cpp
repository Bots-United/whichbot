//
// $Id: FleeStrategy.cpp,v 1.7 2004/06/05 00:42:50 clamatius Exp $

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
#include "BotManager.h"
#include "strategy/FleeStrategy.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "strategy/HiveMind.h"
#include "config/Config.h"
#include "worldstate/HiveManager.h"

const double DISTANCE_SCALING_FACTOR = 0.0003;

bool fleeCfgInitialised = false;
float CFG_DEFENSE_CHAMBER_FLEE_REWARD = 100.0;
float CFG_HIVE_FLEE_REWARD = 300.0;

// skulk, lerk, gorge, fade, onos
float CFG_COWARDICE[] = {0.0, 0.8, 0.5, 0.4, 0.4};

FleeStrategy::FleeStrategy(Bot& bot) :
    _bot(bot)
{
    if (!fleeCfgInitialised) {
        CFG_DEFENSE_CHAMBER_FLEE_REWARD = 
            Config::getInstance().getTweak("strategy/flee/defensechamber_reward", CFG_DEFENSE_CHAMBER_FLEE_REWARD);
        CFG_HIVE_FLEE_REWARD = 
            Config::getInstance().getTweak("strategy/flee/hive_reward", CFG_HIVE_FLEE_REWARD);
        for (int ii = 0; ii < NUM_EVOLUTIONS; ii++) {
            CFG_COWARDICE[ii] = 
                Config::getInstance().getArrayTweak("strategy/flee/cowardice", ii, CFG_COWARDICE[ii]);
        }
        fleeCfgInitialised = true;
    }
}


FleeStrategy::~FleeStrategy()
{
}


void FleeStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
    if (_bot.getEdict() != NULL) {
        if (botIsScared(_bot)) {
            CBaseEntity* pEntity = NULL;
            do {
                pEntity = UTIL_FindEntityByClassname(pEntity, "defensechamber");
                if ((pEntity != NULL) && !FNullEnt(pEntity->edict())) {
                    tNodeId nearestWaypointId = HiveMind::getSeenWaypoint(pEntity->edict());
                    if ((nearestWaypointId >= 0) &&
                        (nearestWaypointId < gpBotManager->getWaypointManager().getNumWaypoints()))
                    {
                        addReward(rewards, nearestWaypointId, CFG_DEFENSE_CHAMBER_FLEE_REWARD);
                    }
                }
			
            } while (pEntity != NULL);
		
            HiveInfo* pHiveInfo = HiveManager::getNearestActiveHive(_bot.getEdict()->v.origin);
            if (pHiveInfo != NULL) {
                addReward(rewards, pHiveInfo->getWaypointId(), CFG_HIVE_FLEE_REWARD);
            }
        }
    }
}


void FleeStrategy::addReward(vector<Reward>& rewards, tNodeId wptId, float rewardVal)
{
	if (_bot.getPathManager().nodeIdValid(wptId)) {
		float distance = _bot.getPathManager().getDistance(wptId);
		if (distance < MAX_DISTANCE_ESTIMATE) {
			rewardVal *= exp((-DISTANCE_SCALING_FACTOR) * distance);
			rewards.push_back(Reward(wptId, rewardVal, "FleeEnemies"));
		}
	}
}

void FleeStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
}


void FleeStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
}


bool FleeStrategy::botIsScared(Bot& bot)
{
	if (bot.getEdict() == NULL) {
		return false;
	}

    if (bot.getEvolution() >= NUM_EVOLUTIONS) {
        return false;
    }

	float maxHealth = bot.getEdict()->v.max_health;
	float healthRatio = (maxHealth > 0) ? (bot.getEdict()->v.health / maxHealth) : 1.0;

	float cowardiceRatio = CFG_COWARDICE[bot.getEvolution()];
	bool isScared = (healthRatio < cowardiceRatio);
	return isScared;
}
