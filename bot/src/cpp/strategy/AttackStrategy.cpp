//
// $Id: AttackStrategy.cpp,v 1.20 2005/03/08 06:33:55 clamatius Exp $

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
#include "strategy/AttackStrategy.h"
#include "config/Config.h"
#include "config/TranslationManager.h"
#include "strategy/FleeStrategy.h"
#include "strategy/HiveMind.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "BotManager.h"
#include "sensory/AuditoryManager.h"

const double DISTANCE_SCALING_FACTOR = 0.0003;

AttackStrategy::AttackStrategy(Bot& bot) :
	_bot(bot)
{
}


AttackStrategy::~AttackStrategy()
{
}


// Reward the bot for going towards marine buildings
void AttackStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
    if (!FleeStrategy::botIsScared(_bot) && !HiveMind::getPackManager().isBotASlave(_bot)) {
		Config& config = Config::getInstance();
		vector<EntityReference> entities = HiveMind::getEntitiesSeen();
		
		for (vector<EntityReference>::iterator ii = entities.begin(); ii != entities.end(); ii++) {
			EntityReference entity = *ii;
			if (!entity.isNull() && (entity.getHealth() > 0)) {
				EntityInfo* pInfo = config.getEntityInfo(entity.getClassname());
				
				if ((pInfo != NULL) && (pInfo->getTeam() == MARINE_TEAM)) {
					tReward rewardVal = pInfo->getDefaultInfluence();
					
					if (rewardVal != 0.0) {
						tNodeId nearestWaypointId = HiveMind::getSeenWaypoint(entity.getEdict());
		                addReward(rewards, nearestWaypointId, rewardVal, string("Seek ") + TranslationManager::getTranslation(entity.getClassname()));
					}
				}
				
			} else {
				HiveMind::entityDestroyed(entity.getEdict());
			}
		}
		
		addParasitedPlayerRewards(rewards, evolution);
	}
}


void AttackStrategy::addParasitedPlayerRewards(vector<Reward>& rewards, tEvolution evolution)
{
	EntityInfo* pParasitedMarineInfo = Config::getInstance().getEntityInfo("parasited_marine");
	if (pParasitedMarineInfo != NULL) {
		for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
			CBaseEntity* pPlayerBaseEntity = UTIL_PlayerByIndex(ii);
			
			if (pPlayerBaseEntity != NULL) {
				edict_t* pPlayerEdict = INDEXENT(ii);
				
				if (!FNullEnt(pPlayerEdict)) {
					if ((pPlayerEdict->v.team == MARINE_TEAM) && (pPlayerEdict->v.health > 0) &&
						(pPlayerEdict->v.deadflag == DEAD_NO) && (pPlayerEdict->v.skin >= 0))
					{
						if (((pPlayerEdict->v.iuser4 & MASK_PARASITED) != 0) ||
							AuditoryManager::getInstance().isHeardPlayerNearby(
								_bot.getEdict()->v.origin, AuditoryManager::getInstance().getHearingRadius()))
						{
							tNodeId nearestWaypoint = gpBotManager->getWaypointManager().getNearestWaypoint(evolution, pPlayerEdict->v.origin, NULL);
							
							addReward(rewards, nearestWaypoint, pParasitedMarineInfo->getDefaultInfluence(), "ChaseMarine");
						}
					}
				}
			}
		}
	}
}


void AttackStrategy::addReward(vector<Reward>& rewards, tNodeId nearestWaypointId, float rewardVal, const string& description)
{
	if (_bot.getPathManager().nodeIdValid(nearestWaypointId)) {
		float distance = _bot.getPathManager().getDistance(nearestWaypointId);
		// let's not add it if it's not in the optimised tree search space
		
		if (distance < MAX_DISTANCE_ESTIMATE) {
			rewardVal *= exp((-DISTANCE_SCALING_FACTOR) * distance);
			if (rewardVal != 0) {
				rewards.push_back(Reward(nearestWaypointId, rewardVal, description));
			}
		}
	}
}


void AttackStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
	//NOP
}


void AttackStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	//NOP
}
