//
// $Id: PackInfo.h,v 1.8 2004/06/12 01:59:02 clamatius Exp $

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

#ifndef __STRATEGY_PACKINFO_H
#define __STRATEGY_PACKINFO_H

#include "BotTypedefs.h"
#include "extern/halflifesdk/extdll.h"
#include "Bot.h"
#include "framework/Log.h"

class PackInfo  
{
public:

    typedef enum
    {
        kNotSlaved,
        kSlavedToPlayer,
        kSlavedToBot
    } eSlaveMode;

	PackInfo(int packId);

	virtual ~PackInfo();

	void addBot(Bot& bot);

	void removeBot(Bot& bot);

	bool isMember(Bot& bot);

	Bot* getLeader();

	bool areAllFollowersCloseBy();

	bool isLeaderCloseBy(Bot& bot);

	tNodeId getLeaderNearestWaypointId();

	bool closeEnoughToLeader(tNodeId followerWptId);

	tDistanceEstimate getDistanceToLaggard();

	tNodeId getLaggardNearestWaypointId();

	bool isLeader(Bot& bot);

	void clear();

	inline const byte getPackId() const { return _packId; }

	inline const int getNumBots() const { return _bots.size(); }

    bool isPackFull() const;

    void engagePlayerSlaveMode(edict_t* pMasterPlayerEdict);

    void engageBotSlaveMode(Bot& newBotMaster);

    void disengageBotSlaveMode();

    inline Bot* getBotMaster() const { return _pBotMaster; }

    inline edict_t* getMasterPlayerEdict() { return _playerMaster.getEdict(); }

    inline eSlaveMode getSlaveMode() { return _slaveMode; }

	void setLeader(Bot& bot);

protected:

	void assignNewLeader();

	byte _packId;
	vector<Bot*> _bots;
    eSlaveMode _slaveMode;
    Bot* _pBotMaster;
    EntityReference _playerMaster;
	Bot* _pLeader;

	static Log _log;
};

#endif // __STRATEGY_PACKINFO_H
