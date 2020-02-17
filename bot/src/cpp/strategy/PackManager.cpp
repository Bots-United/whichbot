//
// $Id: PackManager.cpp,v 1.29 2005/02/28 06:24:48 clamatius Exp $

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
#include "combat/CombatStrategy.h"
#include "config/TranslationManager.h"
#include "strategy/AmbushStrategy.h"
#include "strategy/AttackStrategy.h"
#include "strategy/PackFollowerStrategy.h"
#include "strategy/FleeStrategy.h"
#include "strategy/PackLeaderStrategy.h"
#include "strategy/RescueStrategy.h"
#include "strategy/ScoutStrategy.h"
#include "strategy/PackManager.h"
#include "strategy/PackInfo.h"

Log PackManager::_log(__FILE__);

PackManager::PackManager ()
{
	for (int ii = 0; ii < MAX_PACKS; ii++) {
		_wolfPacks.push_back(new PackInfo(ii+1));
	}
}


PackManager::~PackManager ()
{
    clear();
}


void PackManager::setPackStrategies(Bot& bot)
{
    PackInfo* pWolfPack = getPackInfo(bot, true);
	if (pWolfPack->isLeader(bot)) {
		_log.Debug("[%s] is the wolf pack leader", bot.getName()->c_str());
		setLeaderStrategies(bot);

	} else {
		_log.Debug("[%s] is a wolf pack follower", bot.getName()->c_str());
		setFollowerStrategies(bot);
	}
	bot.getStrategyManager().addStrategy(new CombatStrategy(bot), 100.0);
	bot.getStrategyManager().addStrategy(new FleeStrategy(bot), 100.0);
}


PackInfo* PackManager::getPackInfo(Bot& bot, bool shouldAddBotToPack)
{
    PackInfo* pInfo = NULL;
    int packId = (int)bot.getProperty(Bot::kWolfPackId, -1);
    if (packId >= 0) {
        pInfo = _wolfPacks[packId - 1];

    } else {
        for (vector<PackInfo*>::iterator ii = _wolfPacks.begin(); ii != _wolfPacks.end(); ii++) {
            if ((*ii)->isMember(bot)) {
                pInfo = *ii;
            }
        }
    }
    
    if (pInfo == NULL && shouldAddBotToPack) {
        pInfo = &addBotToPack(bot);
    }
    return pInfo;
}


PackInfo& PackManager::addBotToPack(Bot& bot)
{
	int packIdx = bot.getRole() - kPack1Role;

    PackInfo* pInfo = _wolfPacks[packIdx];
	pInfo->addBot(bot);

    return *pInfo;
}


void PackManager::promoteBot(Bot& bot)
{
	PackInfo* pWolfPack = getPackInfo(bot);
	if (pWolfPack != NULL) {
		Bot* pPreviousLeader = pWolfPack->getLeader();
		if (&bot != pPreviousLeader) {
			setLeaderStrategies(bot);
		    //if (gpBotManager->isBotChatEnabled()) {
			   // bot.sayToTeam(TranslationManager::getTranslation("leading_pack"));
		    //}
			if (pPreviousLeader != NULL) {
				setFollowerStrategies(*pPreviousLeader);
			}
			pWolfPack->setLeader(bot);
		}
	}
}


void PackManager::unsetPackStrategies(Bot& bot)
{
    PackInfo* pWolfPack = getPackInfo(bot);
    if (pWolfPack != NULL) {
	    if (pWolfPack->isLeader(bot)) {
		    // Promote the next bot in line to the leadership position.
		    if (pWolfPack->getNumBots() > 1) {
			    _log.Debug("Wolf pack leader [%s] is now leaving the wolf pack", bot.getName()->c_str());
			    pWolfPack->removeBot(bot);

			    Bot* pNewPackLeader = pWolfPack->getLeader();
                if (pNewPackLeader != NULL) {
                    setLeaderStrategies(*pNewPackLeader);
                    _log.Debug("[%s] is now the new wolf pack leader", pNewPackLeader->getName()->c_str());
				    //if (gpBotManager->isBotChatEnabled()) {
					   // pNewPackLeader->sayToTeam(TranslationManager::getTranslation("leading_pack"));
				    //}
                }

		    } else {
			    pWolfPack->removeBot(bot);
		    }

	    } else {
		    pWolfPack->removeBot(bot);
	    }
    }
}


void PackManager::clear()
{
    for (vector<PackInfo*>::iterator ii = _wolfPacks.begin(); ii != _wolfPacks.end(); ii++) {
    	(*ii)->clear();
        delete (*ii);
    }
    _wolfPacks.clear();    
}


void PackManager::setLeaderStrategies(Bot& bot)
{
	StrategyManager& stratMgr = bot.getStrategyManager();
	PathManager& pathMgr = bot.getPathManager();

	// Let's clear out any existing strategies that the bot may have first.
	stratMgr.clear();

	stratMgr.addStrategy(new AttackStrategy(bot), 0.10);
	stratMgr.addStrategy(new RescueStrategy(bot), 0.60);
	stratMgr.addStrategy(new ScoutStrategy(bot), 0.10);
    PackInfo* pInfo = getPackInfo(bot, true);
	stratMgr.addStrategy(new PackLeaderStrategy(*pInfo, bot), 0.20);
}


void PackManager::setFollowerStrategies(Bot& bot)
{
	StrategyManager& stratMgr = bot.getStrategyManager();
	PathManager& pathMgr = bot.getPathManager();

	// Let's clear out any existing strategies that the bot may have first.
	stratMgr.clear();

    PackInfo* pInfo = getPackInfo(bot, true);
	stratMgr.addStrategy(new PackFollowerStrategy(*pInfo, bot), 1.0);
}


void PackManager::enslavePacksToPlayer(edict_t* pPlayerEdict)
{
    if (pPlayerEdict != NULL) {
        for (vector<PackInfo*>::iterator ii = _wolfPacks.begin(); ii != _wolfPacks.end(); ii++) {
            (*ii)->engagePlayerSlaveMode(pPlayerEdict);
            Bot* pLeader = (*ii)->getLeader();
            if (pLeader != NULL) {
                pLeader->sayToTeam(TranslationManager::getTranslation("following"));
            }
        }

    } else {
        for (vector<PackInfo*>::iterator ii = _wolfPacks.begin(); ii != _wolfPacks.end(); ii++) {
            bool wasSlavedToPlayer = (*ii)->getSlaveMode() == PackInfo::kSlavedToPlayer;
            (*ii)->disengageBotSlaveMode();
			Bot* pLeader = (*ii)->getLeader();
            if (pLeader != NULL && wasSlavedToPlayer) {
                pLeader->sayToTeam(TranslationManager::getTranslation("resuming"));
            }
        }
    }
}


bool PackManager::isBotASlave(Bot& bot)
{
    PackInfo* pInfo = getPackInfo(bot);
    return pInfo != NULL ? (pInfo->getSlaveMode() != PackInfo::kNotSlaved) : false;
}
