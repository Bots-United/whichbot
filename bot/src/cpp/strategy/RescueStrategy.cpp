//
// $Id: RescueStrategy.cpp,v 1.11 2004/09/10 05:57:47 clamatius Exp $

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
#include "config/Config.h"
#include "config/TranslationManager.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "engine/PathManager.h"
#include "strategy/HiveMind.h"
#include "strategy/RescueStrategy.h"
#include "worldstate/AreaManager.h"


Log RescueStrategy::_log(__FILE__);

static tReward getConfiguredReward(const char* entityClassName);


RescueStrategy::RescueStrategy(Bot& bot) :
    _bot(bot)
{
	// NOP
}


RescueStrategy::~RescueStrategy()
{
	// NOP
}

// Reward the bot for rescuing entities.
void RescueStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
    if (!_currentRescueTarget.isNull()) {
		if (HiveMind::isEntityUnderAttack(_currentRescueTarget.getEdict())) {
			// We already have a target.  Let's reward the bot again.
			rewardBot(rewards);
			return;

		} else {
			const char* className = _currentRescueTarget.getClassname();
			_log.Debug("Rescue target [%s] hasn't been attacked in a while.  Selecting new rescue target if any", className);
		}
	}

	// Check to see if something else is under attack
    EntityReference* pNewRescueEntity = HiveMind::getEntityToRescue();
    if (pNewRescueEntity != NULL) {
        _currentRescueTarget = *pNewRescueEntity;
        float lastRescueTime = _bot.getProperty(Bot::kLastRescueTime);
        // don't keep resetting the last wpt id over and over, it'll confuse the nav system too much
        if (gpGlobals->time > lastRescueTime + 20.0) {
            // this should let the bot turn round
            _bot.getPathManager().resetLastNodeId();
            _bot.setProperty(Bot::kLastRescueTime);
        }

    } else {
        _currentRescueTarget.clear();
    }

	if (!_currentRescueTarget.isNull()) {
		rewardBot(rewards);
	}
}

void RescueStrategy::rewardBot(vector<Reward>& rewards)
{
	// We want to return the current known waypoint ID for the entity rather than have the strategy
	// calculate it.  This way, the strategy is only as effective as the bots' scouting effectiveness.
	tNodeId rescueWaypointId = HiveMind::getSeenWaypoint(_currentRescueTarget.getEdict());

	if ((rescueWaypointId != INVALID_NODE_ID)
		&& (rescueWaypointId < gpBotManager->getWaypointManager().getNumWaypoints())
		&& _bot.getPathManager().treeValid()
		&& _bot.getPathManager().nodeIdValid(rescueWaypointId)) {

		const char* className = _currentRescueTarget.getClassname();
		_log.Debug("Attempting to rescue %s at waypoint %d (%s)", className, rescueWaypointId,
			   TranslationManager::getTranslation(
			       AreaManager::getAreaName(gpBotManager->getWaypointManager().getOrigin(rescueWaypointId))).c_str());

		tReward rewardVal = getConfiguredReward(className);
        rewards.push_back(Reward(rescueWaypointId, rewardVal, string("Rescue") + TranslationManager::getTranslation(className)));
	}
}


void RescueStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
	// NOP
}

void RescueStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	// NOP
}

// TODO:  Read this from a config file.
// Make the rescue target really really (did I mention really?) shiney in the reward tree.
static const tReward DEFAULT_REWARD = 1000.0f;
static tReward getConfiguredReward(const char* entityClassName)
{
	return DEFAULT_REWARD;
}
