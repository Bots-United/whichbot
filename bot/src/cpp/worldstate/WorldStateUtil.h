//
// $Id: WorldStateUtil.h,v 1.14 2004/09/24 05:38:05 clamatius Exp $

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

#ifndef __WORLDSTATE_WORLDSTATEUTIL_H
#define __WORLDSTATE_WORLDSTATEUTIL_H

#include "BotTypedefs.h"
#include "extern/halflifesdk/extdll.h"
#include "BotManager.h"

class WorldStateUtil
{
public:

	static edict_t* findClosestEntity(const char* classname, const Vector& fromPos);

    static edict_t* findClosestSwitchEntity(const char* classname, const Vector& fromPos);

	static int countPlayersWithEvolution(tEvolution evolution);

    static int countEntities(const char* classname);

	static edict_t* createPlayer(const string& name);

	static void joinTeam(edict_t* pEdict, int teamToJoin);

    static float getMaxArmour(edict_t* pEdict);

	static void checkAngle(float val);

	static void checkAnglesForVector(const Vector& vec);

	static void checkAnglesForEdict(edict_t* pEdict);

	static void checkVector(const Vector& vec);

	static bool isFinite(float val);	

    static bool isOnLadder(edict_t* pEdict);

    static bool isPlayerPresent(edict_t* pPlayerEdict);
};

#endif // __WORLDSTATE_WORLDSTATEUTIL_H
