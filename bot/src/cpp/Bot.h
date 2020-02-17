//
// $Id: Bot.h,v 1.27 2005/02/25 06:27:08 clamatius Exp $

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

#ifndef __BOT_H
#define __BOT_H

#include "BotTypedefs.h"
#include "extern/halflifesdk/util.h"
#include "extern/halflifesdk/cbase.h"
#include "sensory/BotSensor.h"
#include "sensory/Target.h"
#include "navigation/NavigationEngine.h"
#include "navigation/BotMovement.h"
#include "strategy/StrategyManager.h"
#include "engine/PathManager.h"
#include "worldstate/EntityReference.h"
#include "sensory/BotSensor.h"
#include "StatusReporter.h"

class BotPropertyComparator;

class Bot
{
public:

	typedef enum
	{
		kLastFrameTime,
		kLastBuildTime,
		kLastSpawnTime,
		kLastEvolveCheckTime,
		kLastCheatCheckTime,
		kLastLifeTime,
		kLastWeaponChangeTime,
		kWeaponFireTillTime,
        kWolfPackId,
        kLastRescueTime,
		kAmbushStartTime
	} eBotProperty;

	Bot(int botIdx);

	~Bot();

    int tick();
    
    void create();

    int resetBot();

	void selectItem(const string& msg);

    void userInfoChanged(const char* infoBuffer);

    void sayToTeam(const string& msg);

    void kill();

    bool selectWeapon(int wpnId);

	bool selectWeapon(Target* pTarget, float range, bool shouldUseMeleeAttack);

	void fireWeapon();

	void selectDesiredEvolution();

	bool isCloaked();

    short getResources();

    void giveCheatPoints();

	Vector getGroundLevelOrigin ();

    inline void readyroom() { fakeClientCommand("readyroom"); }

    inline const string* getName() const { return _pName; }

    inline StrategyManager& getStrategyManager() { return _stratMgr; }

	inline PathManager& getPathManager() { return _pathMgr; }

	inline NavigationEngine* getNavigationEngine() { return _pNavEngine; }

    inline tEvolution getEvolution() { return _evolution; }

    inline edict_t* getEdict() { return _entity.getEdict(); }

	inline tBotRole getRole() { return _role; }

    inline BotSensor* getSensor() { return _pSensor; }

	inline tEvolution getDesiredEvolution() { return _desiredEvolution; }

    inline BotMovement* getMovement() { return _pMovement; }

    inline vector<int>& getTraits() { return _traits; }

    inline bool hasTrait(int trait) { return find(_traits.begin(), _traits.end(), trait) != _traits.end(); }

	inline void setProperty(eBotProperty property) { setProperty(property, gpGlobals->time); }

	void setProperty(eBotProperty property, float value);

	float getProperty(eBotProperty property, float defaultVal = 0.0);

	Target* getTarget();

    inline float getEnergy() { return _entity.isNull() ? 0 : _entity.getEdict()->v.fuser3 / 10; }

	void setTarget(Target* newTarget);

    static bool Bot::isEvolving(edict_t* pEdict);

    static tEvolution getEvolution(edict_t* pEdict);

	static Vector getGroundLevelOrigin (const edict_t* pEdict, tEvolution evolution);

    bool needsHealing();

    bool hasFailedToBuildAt(tNodeId waypointId);

    void failedToBuildAt(tNodeId waypointId);

	void generateStatusReport();

	void handleDamage(byte armourDamage, byte healthDamage, const Vector& origin);

	bool isInjured();

protected:

	void checkPackEvolution();

    void startGame();

	void fakeClientCommand(const string& msg);

	void respawnBot();

	void checkCheating();

    void checkEvolution();
    
    byte getCombatEvolutionImpulse();

    void evolve();

	void setWeaponImpulse();

    string* _pName;
    EntityReference _entity;
    BotSensor* _pSensor;
    NavigationEngine* _pNavEngine;
    tEvolution _evolution;
	tEvolution _desiredEvolution;
    StrategyManager _stratMgr;
    PathManager _pathMgr;
    BotMovement* _pMovement;
	Target* _pTarget;
    StatusReporter _statusReporter;
    float _maxArmour;

    vector<int> _traits;
    vector<tNodeId> _buildingBlacklist;

    vector<float> _lastWeaponFireTimes;

	typedef hash_map<int, float> tBotPropertyMap;
	tBotPropertyMap _properties;

    int _lastEvolution;

	int _currentWeaponId;

    bool _dead;

	tBotRole _role;

    static Log _log;
};

extern Bot* gpCurrentThinkingBot;

#endif // __BOT_H
