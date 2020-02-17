//
// $Id: BuilderStrategy.h,v 1.14 2004/05/27 01:38:40 clamatius Exp $

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

#ifndef __STRATEGY_BUILDERSTRATEGY_H
#define __STRATEGY_BUILDERSTRATEGY_H

#include "BotTypedefs.h"
#include "Strategy.h"
#include "Bot.h"
#include "framework/Log.h"
#include "worldstate/HiveManager.h"

enum eBuildPlan
{
	kNoBuildPlan = 0,
	kBuildResTower = 1,
	kBuildHive = 2,
	kBuildOffenseChamber = 3,
	kBuildDefenseChamber = 4,
	kBuildMovementChamber = 5,
	kBuildSensoryChamber = 6
};

class BuilderStrategy : public Strategy
{
public:

	BuilderStrategy(Bot& bot);

    virtual void getRewards(vector<Reward>& rewards, tEvolution evolution);

    virtual void visitedWaypoint(tNodeId wptId, tEvolution evolution);

	virtual void waitedAtWaypoint(tNodeId wptId, tEvolution evolution);

protected:

    void getResTowerRewards(vector<Reward>& rewards);

	void selectBuildPlan();

	void selectHiveDefensePlan();

	bool rewardResTowerLocations(vector<Reward>& rewards, int nodeMask, bool rewardEmpty);

	void rewardHiveLocations(vector<Reward>& rewards, tHiveFillState desiredHiveState);

	void checkForBuildStart(tNodeId wptId, int targetRes, eEntityType entityType);

	void checkBuildStatus();

	void addReward(vector<Reward>& rewards, tNodeId wptId, float rewardVal);

	bool locationIsBuildable(tNodeId wptId);

    bool otherChamberTooClose(tNodeId wptId);

    void countChambers();

    int getChamberCount(int hiveNum);

	string getBuildPlanName() const;

	Bot& _bot;
	eBuildPlan _plan;
	int _targetRes;
	float _timeLastBuildingDropped;
	float _waitStartTime;
	float _hiveDefenseRange;

    int _numDefenseChambers;
    int _numMovementChambers;
    int _numSensoryChambers;

    enum eBuildPlan _chambers[3];

	vector<tNodeId> _buildableLocations;

private:

	static Log _log;
};


#endif // __STRATEGY_BUILDERSTRATEGY
