//
// $Id: CombatStrategy.cpp,v 1.7 2004/05/27 01:38:42 clamatius Exp $

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

#include "combat/CombatStrategy.h"
#include "BotManager.h"
#include "strategy/FleeStrategy.h"

const float TARGET_REWARD = 100.0;

Log CombatStrategy::_log(__FILE__);

CombatStrategy::CombatStrategy(Bot& bot) :
	_bot(bot),
	_targetWptId(-1),
	_lastWptCalcTime(0)
{
}


CombatStrategy::~CombatStrategy()
{
}


void CombatStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
	if (!FleeStrategy::botIsScared(_bot)) {
		Target* pTarget = _bot.getTarget();
		if ((pTarget != NULL) && pTarget->isValid()) {
			// if we haven't calculated the target wpt id, or the target is a player and it's time to recalc
			if ((_targetWptId < 0) || (!pTarget->getInfo()->isBuilding() && (gpGlobals->time > _lastWptCalcTime + 0.1))) {
				calculateTargetWaypoint();
			}
			
			if (_targetWptId >= 0) {
				rewards.push_back(Reward(_targetWptId, TARGET_REWARD, "CombatTarget"));
			}
		}

	}
}


void CombatStrategy::calculateTargetWaypoint()
{
	// TODO - we need to do something a little smarter than the nearest wpt, but maybe we should start with that
	_targetWptId = 
		gpBotManager->getWaypointManager().getNearestWaypoint(
		_bot.getEvolution(), 
		_bot.getTarget()->getOrigin(),
		NULL);
}


void CombatStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
	if (wptId == _targetWptId) {
		_log.Debug("Reached target waypoint");
	}
}


void CombatStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
}
