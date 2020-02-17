//
// $Id: BotManager.h,v 1.10 2005/02/19 05:30:17 clamatius Exp $

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

#ifndef __BOTMANAGER_H
#define __BOTMANAGER_H

#include "BotTypedefs.h"
#include "Bot.h"
#include "BotManager.h"
#include "config/Config.h"
#include "framework/Log.h"
#include "message/ClientCommandDispatcher.h"
#include "message/NetMessageDispatcher.h"
#include "navigation/WaypointManager.h"

typedef vector<Bot*>::iterator tBotIterator;

class BotManager
{
public:

    void addBot(Bot* newBot);

    void removeBot(Bot* oldBot);

    vector<Bot*>::iterator begin();

    vector<Bot*>::iterator end();

    int getBotIndex(Bot& bot);

    void init();

    void newLevel();

    void tick();

    void addBot();

    void clientDisconnected(edict_t* pEdict);

    void triggerStatusReports();

    ClientCommandDispatcher& getClientCommandDispatcher() { return _cmdDispatcher; }

    NetMessageDispatcher& getMessageDispatcher() { return _messageDispatcher; }

	inline bool isBotChatEnabled() const { return Config::getInstance().getTweak("admin/bot_chat_enabled", 1) == 1; }

    inline const char* getCommandPrefix() const { return "wb"; }

    Bot* getBot(edict_t* pEdict);

    void kickLastBot();

    void enableTeamBalance(bool enable);

    inline WaypointManager& getWaypointManager() { return _waypointManager; }

    inline const int getNumBots () const { return _bots.size(); }

    void terrainUpdated(tEvolution forEvolution, int startWptId, int endWptId, float costDiff);

    void startGame();

    bool inCombatMode();

    static bool areCheatsEnabled();

    void toggleCheats(bool enable);
        
    inline bool noAutoDefenses() { return _noAutoDefenses; }

    inline void setNoAutoDefenses(bool newVal) { _noAutoDefenses = newVal; }

protected:

    void checkTeamBalance();

	void checkBotsToAdd();

	void kick(const char* name);

    void checkStatusReport();

	// returns true if a bot was added or removed
    bool updateBotBalance(int numNonWBPlayers, int playerCount);

	void updateDummyPlayerBalance(int alienCount, int marineCount);

	int getTargetNumBots(int numNonWBPlayers);

    float _lastTeamBalanceCheckTime;

	bool _teamBalanceEnabled;

    float _lastBotAddedTime;

    int _numBotsToAdd;

    float _lastFrameTime;

    float _statusReportTime;

    int _lastReportingBot;

    ClientCommandDispatcher _cmdDispatcher;

    NetMessageDispatcher _messageDispatcher;

    WaypointManager _waypointManager;

    bool _levelInitialised;

    bool _noAutoDefenses;

    bool _inCombatMode;

    vector<Bot*> _bots;

private:

    static Log s_log;
};

extern BotManager* gpBotManager;

#endif // __BOTMANAGER_H
