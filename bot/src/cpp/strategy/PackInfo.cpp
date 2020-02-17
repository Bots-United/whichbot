//
// $Id: PackInfo.cpp,v 1.15 2004/10/08 01:42:30 clamatius Exp $

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
#include "strategy/PackInfo.h"

Log PackInfo::_log(__FILE__);

PackInfo::PackInfo(int packId) :
    _packId(packId),
    _slaveMode(kNotSlaved),
    _pBotMaster(NULL),
	_pLeader(NULL)
{
}


PackInfo::~PackInfo ()
{
}


void PackInfo::addBot (Bot& bot)
{
	_bots.push_back(&bot);
    bot.setProperty(Bot::kWolfPackId, _packId);
	if (_bots.size() == 1) {
		_pLeader = &bot;
	}
}


void PackInfo::removeBot(Bot& bot)
{
    for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ii++) {
		Bot* packBot = *ii;
		if (packBot == &bot) {
			_bots.erase(ii);
            bot.setProperty(Bot::kWolfPackId, -1);
			break;
		}
	}
	if (&bot == _pLeader) {
		_pLeader = NULL;
		assignNewLeader();
	}
}


void PackInfo::assignNewLeader()
{
	tEvolution maxEvolution = kSkulk;
    for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ii++) {
		Bot* pBot = (*ii);
		if (pBot->getEvolution() >= maxEvolution) {
			maxEvolution = pBot->getEvolution();
			_pLeader = pBot;
		}
	}
}


void PackInfo::setLeader(Bot& bot)
{
	if (isMember(bot)) {
		_pLeader = &bot;
	}
}


bool PackInfo::isMember (Bot& bot)
{
	for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ++ii)
	{
		Bot* packBot = *ii;
		if (packBot == &bot) {
			return true;
		}
	}
	return false;
}


Bot* PackInfo::getLeader()
{
	return _pLeader;
}

// TODO:  Read this from a config file.
// Leader should be at least this far from the followers.
static const float MAX_ALLOWABLE_FOLLOWER_DISTANCE = 300.0f;
bool PackInfo::areAllFollowersCloseBy ()
{
	Bot* leader = getLeader();
	assert(leader != NULL);

	for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ++ii)
	{
		Bot* bot = *ii;
		if (bot != leader) {
			Vector v = leader->getEdict()->v.origin - bot->getEdict()->v.origin;
			if ((!leader->getSensor()->entityIsVisible(bot->getEdict())) & (v.Length() > MAX_ALLOWABLE_FOLLOWER_DISTANCE)) {
				return false;
			}
		}
	}

	// All of followers are reasonably close
	return true;
}

// TODO:  Read this from a config file.
// Follower should be at least this far from the leader.
static const float MIN_LEADER_DISTANCE = 200.0f;
bool PackInfo::isLeaderCloseBy (Bot& bot)
{
	Bot* leader = getLeader();
	assert(leader != NULL);
	
	Vector v = leader->getEdict()->v.origin - bot.getEdict()->v.origin;
	return (v.Length() <= MIN_LEADER_DISTANCE);
}

tNodeId PackInfo::getLeaderNearestWaypointId ()
{
	Bot* packLeader = getLeader();
	assert(packLeader != NULL);

	tNodeId nearestLeaderWaypointId = -1;

	edict_t* pPackLeaderEntity = packLeader->getEdict();
	nearestLeaderWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(packLeader->getEvolution(), pPackLeaderEntity->v.origin, pPackLeaderEntity);

	return nearestLeaderWaypointId;
}

tDistanceEstimate MAX_DISTANCE = 4.0f;  // In seconds
tDistanceEstimate PackInfo::getDistanceToLaggard ()
{
	Bot* leader = getLeader();
	assert(leader != NULL);

	edict_t* pLeaderEntity = leader->getEdict();
	tNodeId nearestLeaderWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(leader->getEvolution(), pLeaderEntity->v.origin, pLeaderEntity);
	if ((nearestLeaderWaypointId >= 0) && (nearestLeaderWaypointId < gpBotManager->getWaypointManager().getNumWaypoints())) {
		for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ++ii)
		{
			Bot* laggard = *ii;
			if (laggard != leader) {
				// Potential laggard
				PathManager& laggardPathMgr = laggard->getPathManager();
				// Make sure the laggard bot hasn't just spawned.
				if (laggardPathMgr.treeValid() && laggardPathMgr.nodeIdValid(nearestLeaderWaypointId)) {
					tDistanceEstimate distance = laggardPathMgr.getDistance(nearestLeaderWaypointId);
					if (distance > MAX_DISTANCE) {
						return distance;
					}
				}
			}
		}
	}

	return 0.0;
}


tDistanceEstimate LEADER_MAX_DISTANCE = 1.0f;  // In seconds
bool PackInfo::closeEnoughToLeader (tNodeId followerWptId)
{
	Bot* leader = getLeader();
	assert(leader != NULL);

	PathManager& leaderPathMgr = leader->getPathManager();

	// Make sure the leader bot hasn't just spawned.
	if (leaderPathMgr.treeValid() && leaderPathMgr.nodeIdValid(followerWptId)) {
		return leaderPathMgr.getDistance(followerWptId) < LEADER_MAX_DISTANCE;
	} else {
		return false;
	}
}


tNodeId PackInfo::getLaggardNearestWaypointId ()
{
	Bot* leader = getLeader();

	edict_t* pLeaderEntity = leader->getEdict();
	tNodeId nearestLeaderWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(leader->getEvolution(), pLeaderEntity->v.origin, pLeaderEntity);
    if ((nearestLeaderWaypointId >= 0) && (nearestLeaderWaypointId < gpBotManager->getWaypointManager().getNumWaypoints())) {
		for (vector<Bot*>::iterator ii = _bots.begin(); ii != _bots.end(); ++ii)
		{
			Bot* laggard = *ii;
			if (laggard != getLeader()) {
				// Potential laggard
				PathManager& laggardPathMgr = laggard->getPathManager();
				// Make sure the laggard bot hasn't just spawned.
				if (laggard->getPathManager().treeValid() && laggardPathMgr.nodeIdValid(nearestLeaderWaypointId)) {
					tDistanceEstimate distance = laggardPathMgr.getDistance(nearestLeaderWaypointId);
					if (distance > MAX_DISTANCE) {
						edict_t* pLaggardEntity = laggard->getEdict();
						return gpBotManager->getWaypointManager().getNearestWaypoint(laggard->getEvolution(), pLaggardEntity->v.origin, pLaggardEntity);
					}
				}
			}
		}
	}

	return -1;
}


bool PackInfo::isLeader(Bot& bot)
{
    return getLeader() == &bot;
}


void PackInfo::clear ()
{
	_bots.clear();
}


bool PackInfo::isPackFull() const
{
    return _bots.size() >= 1;
}


void PackInfo::engagePlayerSlaveMode(edict_t* pMasterPlayerEdict)
{
    disengageBotSlaveMode();
    if (pMasterPlayerEdict != NULL) {
        _slaveMode = kSlavedToPlayer;
        _playerMaster.setEdict(pMasterPlayerEdict);
    }
}


void PackInfo::engageBotSlaveMode(Bot& newBotMaster)
{
    disengageBotSlaveMode();
    _slaveMode = kSlavedToBot;
    _pBotMaster = &newBotMaster;
}


void PackInfo::disengageBotSlaveMode()
{
    _pBotMaster = NULL;
    _playerMaster.setEdict(NULL);
    _slaveMode = kNotSlaved;
}

