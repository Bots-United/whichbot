//
// $Id: PackFollowerStrategy.cpp,v 1.8 2004/06/06 19:00:42 clamatius Exp $

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
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "strategy/PackFollowerStrategy.h"
#include "strategy/FleeStrategy.h"
#include "BotManager.h"


Log PackFollowerStrategy::_log(__FILE__);


PackFollowerStrategy::PackFollowerStrategy(PackInfo& wolfPack, Bot& bot) :
    _wolfPack(wolfPack), _bot(bot)
{
}


PackFollowerStrategy::~PackFollowerStrategy()
{
	//empty
}

// Reward the bot for trying to go to the pack leader's current closest waypoint.
void PackFollowerStrategy::getRewards (vector<Reward>& rewards, tEvolution evolution)
{
	tNodeId leaderNearestWptId = _wolfPack.getLeaderNearestWaypointId();

	if ((leaderNearestWptId >= 0) && (leaderNearestWptId < gpBotManager->getWaypointManager().getNumWaypoints())) {
		tReward rewardVal = getPackFollowReward();

		// Add the reward as-is.  We don't want to adjust the reward to distance
		// since if the bot is assigned to wolfpack duty that we always want the bot
		// to seek out the pack leader.
		rewards.push_back(Reward(leaderNearestWptId, rewardVal, "FollowLeader"));
	}
}

void PackFollowerStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
	if (_wolfPack.isLeaderCloseBy(_bot)) {
		_log.Debug("We're right next to the leader.");
        if ((_bot.getNavigationEngine() != NULL) && !FleeStrategy::botIsScared(_bot)) {
            _bot.getNavigationEngine()->pause();
        }
	}
}

void PackFollowerStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	if (!_wolfPack.isLeaderCloseBy(_bot)) {
		_log.Debug("Leader's moving again.  Resuming navigation.");
        if (_bot.getNavigationEngine() != NULL) {
            _bot.getNavigationEngine()->resume();
        }
	}
}

const char* PACK_LEADER_WP_ENTITY_NAME = "pack_leader_waypoint";

static const tReward DEFAULT_REWARD = 100.0f;
tReward PackFollowerStrategy::getPackFollowReward()
{
	EntityInfo* pInfo = Config::getInstance().getEntityInfo(PACK_LEADER_WP_ENTITY_NAME);
	if (pInfo == NULL) {
		return DEFAULT_REWARD;
	} else {
		return pInfo->getDefaultInfluence();
	}
}
