//
// $Id: CombatNavMethod.h,v 1.8 2004/05/27 01:38:42 clamatius Exp $

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

#ifndef __COMBAT_COMBATNAVMETHOD_H
#define __COMBAT_COMBATNAVMETHOD_H

#include "Bot.h"
#include "navigation/NavigationMethod.h"
#include "navigation/BotMovement.h"
#include "navigation/WaypointNavMethod.h"
#include "sensory/BotSensor.h"
#include "framework/Log.h"
#include "engine/PathManager.h"

typedef struct
{
    int iId;  // the weapon ID value
    char  weapon_name[64];  // name of the weapon when selecting it
    float min_distance;   // 0 = no minimum
    float max_distance;   // 9999 = no maximum
    int   min_hives;
    float min_fire_delay;
    float min_energy;
    float fire_time;
    int   purpose;
    tEvolution evolution;
} tBotWeaponSelect;

extern tBotWeaponSelect gWeaponSelect[];

class CombatNavMethod : public NavigationMethod
{
public:

    CombatNavMethod(Bot& pBot);

    NavigationMethod* navigate();

	void pause();

	void resume();

	virtual string getName() const;

protected:

    Vector calculateInterceptSolution(const Vector& marinePos, const Vector& alienPos, 
                                      const Vector& marineVelocityVec, float alienSpeed);

	void attack();

	bool canAttackNow();

	void moveToAttackPosition();

    void evade(float range);

    void setTarget(Target* pNewTarget);

    bool assessTarget(Target& potentialTarget);

    void checkForTargetDeath();

    bool targetExists();

    Bot& _bot;

    float _lastJumpTime;

    float _lastStrafeTime;

    float _lastTargetSeenTime;

    float _strafeSpeed;

    float _strafeTime;

    float _aimStartTime;

    string _targetName;

    Vector _targetLoc;

	WaypointNavMethod _wptNav;

private:

	static Log _log;
};


#endif // __COMBAT_COMBATNAVMETHOD_H
