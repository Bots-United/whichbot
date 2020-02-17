//
// $Id: StatusReporter.cpp,v 1.4 2005/02/27 06:40:35 clamatius Exp $

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

#include "StatusReporter.h"
#include "Bot.h"
#include "config/TranslationManager.h"
#include "strategy/HiveMind.h"
#include "strategy/PackInfo.h"
#include <sstream>

StatusReporter::StatusReporter()
{
}


// Scrape rewards into totals based on the description of the reward
vector<Reward> collectRewards(const vector<Reward>& rewards)
{
    float rewardTotal = 0.0;
    hash_map<string, Reward*> collectedRewards;
    for (vector<Reward>::const_iterator ii = rewards.begin(); ii != rewards.end(); ii++) {
        if (ii->getValue() != 0) {
            rewardTotal += fabs(ii->getValue());

            hash_map<string, Reward*>::iterator found = collectedRewards.find(ii->getDescription());
            if (found == collectedRewards.end()) {
                collectedRewards[ii->getDescription()] = (Reward*)&(*ii);

            } else {
                found->second->setValue(found->second->getValue() + ii->getValue());
            }
        }
    }

    vector<Reward> result;
    for (hash_map<string, Reward*>::iterator jj = collectedRewards.begin(); jj != collectedRewards.end(); jj++) {
        Reward newReward(*jj->second);
        float newRewardVal = 100.0 * newReward.getValue() / rewardTotal;
        if (newRewardVal != 0) {
            newReward.setValue(newRewardVal);
            result.push_back(newReward);
        }
    }
    return result;
}


class RewardSorter
{
public:

    bool operator () (const Reward& left, const Reward& right)
    {
        return fabs(left.getValue()) > fabs(right.getValue());
    }
};

string evolutions[] = { "skulk", "lerk", "gorge", "fade", "onos" };

void StatusReporter::generateStatusReport(Bot& bot)
{
    ostringstream status;
    status << TranslationManager::getTranslation("Status");
    status << ": ";
    status << TranslationManager::getTranslation(evolutions[bot.getEvolution()]);
    if (bot.getDesiredEvolution() != bot.getEvolution()) {
        status << string("/") << TranslationManager::getTranslation(evolutions[bot.getDesiredEvolution()]);
    }
    
    if (bot.getNavigationEngine() != NULL) {
        status << " ";
        status << bot.getNavigationEngine()->getCurrentMethodName();
    }
    status << " ";

    int packId = (int)bot.getProperty(Bot::kWolfPackId, -1);
    if (packId >= 0) {
        status << TranslationManager::getTranslation("pack") << string(" ") << packId;
        PackInfo* pPackInfo = HiveMind::getPackManager().getPackInfo(bot);
        if (pPackInfo != NULL && pPackInfo->isLeader(bot)) {
            status << string(" ") << TranslationManager::getTranslation("leader");
        }
        status << " ";
    }

	if (bot.getPathManager().treeValid()) {
		vector<Reward> rewards = bot.getStrategyManager().getRewards(bot.getEvolution());

		vector<Reward> collectedRewards = collectRewards(rewards);

		sort(collectedRewards.begin(), collectedRewards.end(), RewardSorter());

		int numRewardsToDisplay = min(3, collectedRewards.size());
		for (int ii = 0; ii < numRewardsToDisplay; ii++) {
			status << collectedRewards[ii].getDescription();
			status << "(";
			status << (int)collectedRewards[ii].getValue();
			status << ") ";
		}
		 
	} else {
		status << "{pathfinder uninitialized}";
	}
    bot.sayToTeam(status.str());
}
