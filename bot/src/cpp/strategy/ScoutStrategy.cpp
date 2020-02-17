//
// $Id: ScoutStrategy.cpp,v 1.22 2004/06/06 21:28:23 clamatius Exp $

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

#include "strategy/ScoutStrategy.h"
#include "strategy/FleeStrategy.h"
#include "strategy/HiveMind.h"
#include "engine/Reward.h"
#include "BotManager.h"
#include "config/Config.h"
#include "config/TranslationManager.h"
#include "extern/metamod/meta_api.h"

const float REWARD_TIMEOUT = 360.0f;
const double DISTANCE_SCALING_FACTOR = 0.0003;


ScoutStrategy::ScoutStrategy(Bot& bot) :
    _bot(bot)
{
}

ScoutStrategy::~ScoutStrategy()
{
}


void ScoutStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
    if (!FleeStrategy::botIsScared(_bot) && !HiveMind::getPackManager().isBotASlave(_bot)) {
        addRewards(rewards, "func_resource", evolution);
        addRewards(rewards, "team_hive", evolution);
    }
}


void ScoutStrategy::addRewards(vector<Reward>& rewards, const char* classname, tEvolution evolution)
{
    EntityInfo* pInfo = Config::getInstance().getEntityInfo(classname);

	if (pInfo == NULL) {
		LOG_ERROR(PLID, "Unable to get configuration for info classname %s.  Check config files are present and readable", classname);
		return;
	}

	CBaseEntity* pEntity = NULL;
	do {
		pEntity = UTIL_FindEntityByClassname(pEntity, classname);
		if (pEntity != NULL) {
            edict_t* pEdict = pEntity->edict();

            tNodeId nearestWaypointId = HiveMind::getNearestWaypoint(evolution, pEdict);
            if (_bot.getPathManager().nodeIdValid(nearestWaypointId)) {
                tReward rewardVal = pInfo->getDefaultInfluence();

                float timeDiff = gpGlobals->time - HiveMind::getVisitTime(nearestWaypointId);
                float scale = timeDiff / REWARD_TIMEOUT;

                float distance = _bot.getPathManager().getDistance(nearestWaypointId);
                // let's not add it if it's not in the optimised tree search space

                if (distance < MAX_DISTANCE_ESTIMATE) {
                    rewardVal *= scale * scale;

                    rewardVal *= exp((-DISTANCE_SCALING_FACTOR) * distance);

                    if (evolution == kOnos || evolution == kFade) {
                        rewardVal *= 0.01;
                    }
                    if (rewardVal > 0) {
                        rewards.push_back(Reward(nearestWaypointId, rewardVal, TranslationManager::getTranslation("Scouting") + string(" ") + TranslationManager::getTranslation(classname)));
                    }
                }
            }
		}

	} while (pEntity != NULL);
}


void ScoutStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
	//NOP
}

void ScoutStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	//NOP
}
