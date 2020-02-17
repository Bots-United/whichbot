//
// $Id: BotSensor.h,v 1.5 2005/01/04 01:18:06 clamatius Exp $

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

#ifndef __SENSORY_BOTSENSOR_H
#define __SENSORY_BOTSENSOR_H

#include "BotTypedefs.h"
#include "navigation/NavigationEngine.h"
#include "config/EntityInfo.h"
#include "extern/halflifesdk/extdll.h"
#include "framework/Log.h"
#include "worldstate/EntityReference.h"
#include "sensory/Target.h"
class Bot;

class BotSensor  
{
public:

	BotSensor(Bot& bot);

    void scan(bool force = false);

    void scanEntity(edict_t* pEntity);

	void forceScanEntity(edict_t* pEntity);

	bool entityIsVisible(edict_t* pEntity);

	inline bool targetSeen() { return _targets.size() > 0; }

    void clear();

	TargetVector& getTargets() { return _targets; }

	TargetVector& getBuildables() { return _buildables; }

	static bool isPointVisible(const Vector& fromPt, const Vector& toPt, edict_t* observerEntity);

	static bool looksBuildable(edict_t* pEntity);

	inline bool threatSeen() { return _threatSeen; }

    inline bool healableFriendSeen() { return _healableSeen; }

    static bool isElectrified(edict_t* pEntity);

protected:

    void addInfluencingEntity(edict_t* pEntity, int team);

    edict_t* _pEdict;

	TargetVector _targets;
	TargetVector _buildables;

    float _lastScanTime;
    float _scanPeriod;
    float _scanRange;
	static float _lastGlobalScanTime;
    Bot& _bot;

    bool _healableSeen;
	bool _threatSeen;

private:

	static Log _log;
};

#endif // __SENSORY_BOTSENSOR_H
