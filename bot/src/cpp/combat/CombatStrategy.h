//
// $Id: CombatStrategy.h,v 1.5 2003/09/23 01:56:35 clamatius Exp $

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

#ifndef __COMBAT_COMBATSTRATEGY
#define __COMBAT_COMBATSTRATEGY

#include "BotTypedefs.h"
#include "Bot.h"
#include "strategy/Strategy.h"
#include "sensory/BotSensor.h"
#include "framework/Log.h"


/// CombatStrategy is a strategy that attracts the bot to the nearest waypoint to its combat target, if set.
/// The idea is to be able to harness the WaypointNavMethod to move walking lifeforms up ramps, round corners,
/// etc as appropriate to get to the target.  Skulks will typically just be able to run in a straight line towards
/// the target, but that doesn't work out well for Onos, for example.
class CombatStrategy : public Strategy
{
public:

	CombatStrategy(Bot& bot);

    virtual ~CombatStrategy();

    virtual void getRewards(vector<Reward>& rewards, tEvolution evolution);

    virtual void visitedWaypoint(tNodeId wptId, tEvolution evolution);

	virtual void waitedAtWaypoint(tNodeId wptId, tEvolution evolution);

protected:

	void calculateTargetWaypoint ();

	Bot& _bot;
	int _targetWptId;
	float _lastWptCalcTime;

private:

	static Log _log;
};

#endif

