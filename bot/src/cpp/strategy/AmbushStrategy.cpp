//
// $Id: AmbushStrategy.cpp,v 1.7 2004/10/29 20:53:14 clamatius Exp $

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

#include "strategy/AmbushStrategy.h"
#include "BotManager.h"
#include "config/Config.h"
#include "config/TranslationManager.h"
#include "sensory/AuditoryManager.h"
#include "strategy/FleeStrategy.h"
#include "strategy/HiveMind.h"

bool AmbushStrategy::_ambushWaypointsInitialized = false;
vector<tNodeId> AmbushStrategy::_ambushWaypoints;
Log AmbushStrategy::_log(__FILE__);


AmbushStrategy::AmbushStrategy(Bot& bot) :
    _bot(bot)
{
    initializeAmbushWaypoints();
    _ambushReward = Config::getInstance().getTweak("strategy/ambush/ambush_reward", 200.0);
	_maxAmbushWait = Config::getInstance().getTweak("strategy/ambush/ambush_timeout", 45.0);
}


AmbushStrategy::~AmbushStrategy()
{
}


void AmbushStrategy::initializeAmbushWaypoints()
{
    if (!_ambushWaypointsInitialized) {
        tTerrainGraph& terrain = gpBotManager->getWaypointManager().getTerrainGraph(_bot.getEvolution());
        for (int ii = 0; ii < terrain.getNumNodes(); ii++) {
            Node& node = terrain.getNode(ii);
            if ((node.getData().getFlags() & W_FL_AMBUSH) != 0) {
                _ambushWaypoints.push_back(ii);
            }
        }

        _ambushWaypointsInitialized = true;
    }
}


void AmbushStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
    if (isAmbushModeActive()) {
        for (vector<tNodeId>::iterator ii = _ambushWaypoints.begin(); ii != _ambushWaypoints.end(); ii++) {
			// Experiment: reward all the ambush waypoints, we'll just not stop there if there's noone around
	        Reward reward(*ii, _ambushReward, TranslationManager::getTranslation("ambush"));
		    rewards.push_back(reward);
        }
    }
}


void AmbushStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
    if (isAmbushModeActive()) {
        vector<tNodeId>::iterator found = find(_ambushWaypoints.begin(), _ambushWaypoints.end(), wptId);
        if (found != _ambushWaypoints.end()) {
			_log.Debug("Visited ambush waypoint %d", wptId);
            // we're at an ambush waypoint
            if (shouldAmbushHere(wptId)) {
                _log.Debug("Bot ambushing at waypoint %d", wptId);
                _bot.getNavigationEngine()->pause();
				_bot.setProperty(Bot::kAmbushStartTime);
            }
        }
    }
}


bool AmbushStrategy::isAmbushModeActive()
{
	if (_bot.getEvolution() != kSkulk) {
		return false;
	}
	PackInfo* pPackInfo = HiveMind::getPackManager().getPackInfo(_bot);
	// We don't set up ambushes if we're not the pack leader
	// if we're on our own, that's ok
	if (pPackInfo != NULL && !pPackInfo->isLeader(_bot)) {
		return false;
	}
	if (FleeStrategy::botIsScared(_bot) || _bot.getSensor()->threatSeen()) {
		return false;
	}

	if (!gpBotManager->inCombatMode() && HiveMind::getNumEntitiesSeen("phasegate") > 0) {
		return false;
	}
	return true;
}


bool AmbushStrategy::shouldAmbushHere(tNodeId wptId)
{
    bool ambushHere = !FleeStrategy::botIsScared(_bot) && !_bot.getSensor()->threatSeen();
	if (ambushHere) {
		bool heardPlayerNearby = AuditoryManager::getInstance().isHeardPlayerNearby(
			_bot.getEdict()->v.origin, 2.0 * AuditoryManager::getInstance().getHearingRadius());
		if (!heardPlayerNearby) {
			_log.Debug("Didn't hear a player nearby, not ambushing");
		}
		return heardPlayerNearby;
	}
	return false;
}


void AmbushStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	// if it's an ambush waypoint, assume we're ambushing
	if ((gpBotManager->getWaypointManager().getFlags(wptId) & W_FL_AMBUSH) != 0) {
		float ambushTimeElapsed = gpGlobals->time - _bot.getProperty(Bot::kAmbushStartTime);
		if (ambushTimeElapsed > _maxAmbushWait) {
			_log.Debug("Timed out.  Giving up ambush, moving again");
			_bot.getNavigationEngine()->resume();

		} else {
			if (!AuditoryManager::getInstance().isHeardPlayerNearby(
				_bot.getEdict()->v.origin, 2.0 * AuditoryManager::getInstance().getHearingRadius()))
			{
				_log.Debug("Don't hear anyone round here.  Giving up ambush");
				_bot.getNavigationEngine()->resume();
			}
		}
	}
}
