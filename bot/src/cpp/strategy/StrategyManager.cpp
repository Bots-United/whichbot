//
// $Id: StrategyManager.cpp,v 1.9 2004/08/11 22:52:56 clamatius Exp $

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
#include "strategy/StrategyManager.h"
#include "strategy/Strategy.h"
#include "strategy/HiveMind.h"
#include "Bot.h"

StrategyManager::~StrategyManager()
{
    clear();
}


void StrategyManager::addStrategy(Strategy* strategy, float weight)
{
    StrategyEntry* pEntry = new StrategyEntry();
    pEntry->strategy = strategy;
    pEntry->weight = weight;
    _strategies.push_back(pEntry);
}


void StrategyManager::clear()
{
    for (vector<StrategyEntry*>::iterator ii = _strategies.begin(); ii != _strategies.end(); ii++) {
        delete (*ii)->strategy;
        delete (*ii);
    }
    _strategies.clear();
}


vector<Reward> StrategyManager::getRewards(tEvolution evolution)
{
    vector<Reward> rewards;
    for (vector<StrategyEntry*>::iterator ii = _strategies.begin(); ii != _strategies.end(); ii++) {
        Strategy* strat = (*ii)->strategy;
        int positionBefore = rewards.size();
        strat->getRewards(rewards, evolution);
        for (int jj = positionBefore; jj < rewards.size(); jj++) {
            rewards[jj].applyWeight((*ii)->weight);
        }
    }
    return rewards;
}


void StrategyManager::visitedWaypoint(tEvolution evolution, tNodeId nodeId)
{
    HiveMind::visitedWaypoint(nodeId);
    for (vector<StrategyEntry*>::iterator ii = _strategies.begin(); ii != _strategies.end(); ii++) {
        Strategy* strat = (*ii)->strategy;
        strat->visitedWaypoint(nodeId, evolution);
    }
}

void StrategyManager::waitedAtWaypoint(tEvolution evolution, tNodeId nodeId)
{
    HiveMind::waitedAtWaypoint(nodeId, evolution);

    for (vector<StrategyEntry*>::iterator ii = _strategies.begin(); ii != _strategies.end(); ii++) {
        Strategy* strat = (*ii)->strategy;
        strat->waitedAtWaypoint(nodeId, evolution);
    }
}


float StrategyManager::getStrategyWeight(Strategy& strat)
{
	for (vector<StrategyEntry*>::iterator ii = _strategies.begin(); ii != _strategies.end(); ii++) {
		if (&strat == (*ii)->strategy) {
			return (*ii)->weight;
		}
	}

	return 0.0;
}
