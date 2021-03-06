//
// $Id: PackManager.h,v 1.9 2004/10/19 23:31:32 clamatius Exp $

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

#ifndef __STRATEGY_PACKMANAGER_H
#define __STRATEGY_PACKMANAGER_H

#include "BotTypedefs.h"
#include "Bot.h"
#include "framework/Log.h"
#include "strategy/PackInfo.h"

class PackManager
{
public:

	PackManager();

	virtual ~PackManager();

	void setPackStrategies(Bot& bot);

	void unsetPackStrategies(Bot& bot);

	void clear();

    PackInfo* getPackInfo(Bot& bot, bool addBotToPack = false);

    bool isBotASlave(Bot& bot);

    void enslavePacksToPlayer(edict_t* pPlayerEdict);

	void promoteBot(Bot& bot);

protected:

	void setLeaderStrategies(Bot& bot);
	
    void setFollowerStrategies(Bot& bot);
    
    PackInfo& addBotToPack(Bot& bot);

	vector<PackInfo*> _wolfPacks;

	static Log _log;
};

#endif // __STRATEGY_PACKMANAGER_H
