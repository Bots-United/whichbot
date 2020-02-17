//
// $Id: PackFollowerStrategy.h,v 1.5 2004/06/06 19:00:43 clamatius Exp $

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

#ifndef __STRATEGY_PACKFOLLOWERSTRATEGY_H
#define __STRATEGY_PACKFOLLOWERSTRATEGY_H

#include "BotTypedefs.h"
#include "sensory/BotSensor.h"
#include "strategy/PackInfo.h"
#include "strategy/Strategy.h"
#include "extern/halflifesdk/extdll.h"
#include "framework/Log.h"
using namespace wb_pathematics;

class PackFollowerStrategy : public Strategy
{
public:

    PackFollowerStrategy(PackInfo& wolfPack, Bot& bot);

    virtual ~PackFollowerStrategy();

    virtual void getRewards(vector<Reward>& rewards, tEvolution evolution);

    virtual void visitedWaypoint(tNodeId wptId, tEvolution evolution);

	virtual void waitedAtWaypoint(tNodeId wptId, tEvolution evolution);

    static tReward getPackFollowReward();

protected:

	// The wolf pack that the bot is assigned to.
    PackInfo& _wolfPack;

	Bot& _bot;

	static Log _log;
};

#endif // __STRATEGY_PACKFOLLOWERSTRATEGY_H
