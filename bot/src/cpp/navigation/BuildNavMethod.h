//
// $Id: BuildNavMethod.h,v 1.4 2004/05/27 01:38:41 clamatius Exp $

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

#ifndef __NAVIGATION_BUILDNAVMETHOD
#define __NAVIGATION_BUILDNAVMETHOD

#include "BotTypedefs.h"
#include "Bot.h"
#include "navigation/NavigationMethod.h"
#include "navigation/BotMovement.h"
#include "framework/Log.h"
#include "config/EntityInfo.h"

class BuildNavMethod : public NavigationMethod
{
public:

	BuildNavMethod(Bot& bot, eEntityType entityTypeToBuild);

	BuildNavMethod(Bot& bot, eEntityType entityTypeToBuild, const Vector& location);

	virtual ~BuildNavMethod();

	virtual NavigationMethod* navigate();

	// Pause and resume navigation.
	virtual void pause();

	virtual void resume();

	virtual string getName() const;

protected:

	bool findTarget();

	bool findNearbyBuildable();

    edict_t* getTargetEdict(const char* targetClassname);

	void setImpulse();

	eEntityType _entityTypeToBuild;
	Vector _entityLocation;
	Bot& _bot;
    float _lastBuildableCheck;
	float _lastImpulseTime;
	float _lastBuildProgress;
	float _lastBuildProgressTime;
    bool _buildableSeen;
    bool _buildingPlanted;

private:

    static Log _log;
};

#endif
