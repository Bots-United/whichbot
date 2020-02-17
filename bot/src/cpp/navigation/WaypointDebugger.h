//
// $Id: WaypointDebugger.h,v 1.8 2003/09/18 01:47:28 clamatius Exp $

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

#ifndef __NAVIGATION_WAYPOINTDEBUGGER
#define __NAVIGATION_WAYPOINTDEBUGGER

#include "BotTypedefs.h"
#include "engine/PathManager.h"
#include "extern/halflifesdk/extdll.h"
using namespace wb_pathematics;

class WaypointDebugger
{
public:

    WaypointDebugger();

    ~WaypointDebugger();

    static void drawDebugBeam(int wptId);

    static void drawDebugBeam(int wptId, int red, int green, int blue);

    static void drawDebugBeam(const Vector& startPosition, const Vector& endPosition, int red, int green, int blue);

	static edict_t* getEditingPlayer();

    void waypointInfo(tEvolution evolution, const Vector& currentPos, edict_t* pObserver);

    void drawNearbyWaypoints(tEvolution evolution, const Vector& currentPos, edict_t* pObserver);

    void nodeAdded(tNodeId nodeId);

    void nodeDeleted(tNodeId nodeId);

protected:

    static Vector getWptOrigin(tNodeId wptId);

    static tNodeFlags getWptFlags(tNodeId wptId);

    static void drawBeam(edict_t* pEntity, const Vector& start, const Vector& end, int width,
                         int noise, int red, int green, int blue, int brightness, int speed);

    float _lastWptDrawTime;

    PathManager* _pathMgr;

    tNodeId _lastWptInfoId;
};

#endif
