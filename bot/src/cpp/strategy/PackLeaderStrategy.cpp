//
// $Id: PackLeaderStrategy.cpp,v 1.9 2004/06/07 13:34:29 clamatius Exp $

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
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "navigation/NavigationEngine.h"
#include "navigation/WaypointNavMethod.h"
#include "strategy/PackFollowerStrategy.h"
#include "strategy/PackLeaderStrategy.h"
#include "strategy/FleeStrategy.h"


Log PackLeaderStrategy::_log(__FILE__);
static float CFG_PAUSE_PERIOD = 30.0f;
static float CFG_PAUSE_LENGTH = 5.0f;


PackLeaderStrategy::PackLeaderStrategy(PackInfo& wolfPack, Bot& leader) :
    _wolfPack(wolfPack),
	_leader(leader),
	_lastPauseTime(0)
{
	// NOP
	CFG_PAUSE_PERIOD = Config::getInstance().getTweak("strategy/pack/pause_period", CFG_PAUSE_PERIOD);
	CFG_PAUSE_LENGTH = Config::getInstance().getTweak("strategy/pack/pause_length", CFG_PAUSE_LENGTH);
}


PackLeaderStrategy::~PackLeaderStrategy()
{
	// NOP
}


void PackLeaderStrategy::getRewards (vector<Reward>& rewards, tEvolution evolution)
{
    if (!FleeStrategy::botIsScared(_leader)) {
        switch (_wolfPack.getSlaveMode()) {
        case PackInfo::kNotSlaved:
            break;
        case PackInfo::kSlavedToBot:
            // TODO
            break;
        case PackInfo::kSlavedToPlayer:
            getSlavedToPlayerRewards(rewards);
            break;
        }
    }
}


void PackLeaderStrategy::getSlavedToPlayerRewards(vector<Reward>& rewards)
{
    tNodeId nearestWaypointId = getMasterPlayerWaypointId();
    if (_leader.getPathManager().nodeIdValid(nearestWaypointId)) {
        rewards.push_back(Reward(nearestWaypointId, PackFollowerStrategy::getPackFollowReward(), "FollowLeader"));
    }
}


tNodeId PackLeaderStrategy::getMasterPlayerWaypointId()
{
    edict_t* pPlayerEdict = _wolfPack.getMasterPlayerEdict();
    if (pPlayerEdict != NULL && pPlayerEdict->v.deadflag == DEAD_NO && pPlayerEdict->v.health > 0) {
        tNodeId nearestWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(
            _leader.getEvolution(), pPlayerEdict->v.origin, _leader.getEdict());
        return nearestWaypointId;
    }
    return INVALID_NODE_ID;
}


void PackLeaderStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
    tNodeId masterWaypointId = INVALID_NODE_ID;
    switch (_wolfPack.getSlaveMode()) {
    case PackInfo::kNotSlaved:
	    if ((gpGlobals->time > _lastPauseTime + CFG_PAUSE_PERIOD) && !_wolfPack.areAllFollowersCloseBy()) {
            if ((_leader.getNavigationEngine() != NULL) && !FleeStrategy::botIsScared(_leader)) {
    		    _log.Debug("Hmm, can't see my followers.  Waiting at %d for laggards to catch up.", wptId);
                _leader.getNavigationEngine()->pause();
    		    _lastPauseTime = gpGlobals->time;
            }
	    }
        break;

    case PackInfo::kSlavedToPlayer:
        masterWaypointId = getMasterPlayerWaypointId();
        if (_leader.getPathManager().nodeIdValid(masterWaypointId)) {
            if (_leader.getPathManager().getRootNodeId() == masterWaypointId) {
                _leader.getNavigationEngine()->pause();
            }
        }
        break;

    case PackInfo::kSlavedToBot:
        // TODO
        break;
    }
}


void PackLeaderStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
    bool resume = false;
    tNodeId masterWaypointId = INVALID_NODE_ID;

    switch (_wolfPack.getSlaveMode()) {
    case PackInfo::kNotSlaved:
	    resume = ((gpGlobals->time > _lastPauseTime + CFG_PAUSE_LENGTH) || _wolfPack.areAllFollowersCloseBy());
        break;

    case PackInfo::kSlavedToPlayer:
        masterWaypointId = getMasterPlayerWaypointId();
        resume = !_leader.getPathManager().nodeIdValid(masterWaypointId);
        if (!resume) {
            resume = (_leader.getPathManager().getRootNodeId() != masterWaypointId);
        } 
        break;

    case PackInfo::kSlavedToBot:
        // TODO
        break;
    }

    if (_leader.getNavigationEngine() != NULL && resume) {
		_log.Debug("Resuming normal navigation.");
        _leader.getNavigationEngine()->resume();
    }
}

