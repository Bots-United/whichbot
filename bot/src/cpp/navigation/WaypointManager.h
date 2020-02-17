//
// $Id: WaypointManager.h,v 1.29 2005/02/19 07:40:13 clamatius Exp $

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

#ifndef __NAVIGATION_WAYPOINTMANAGER_H
#define __NAVIGATION_WAYPOINTMANAGER_H

#include "BotTypedefs.h"
#include "engine/PathManager.h"
#include "model/TerrainData.h"
#include "framework/Log.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/vector.h"
#include "navigation/WaypointDebugger.h"

#define W_FL_TEAM        ((1<<0) + (1<<1))  // allow for 4 teams (0-3)
#define W_FL_TEAM_SPECIFIC (1<<2)  // waypoint only for specified team
#define W_FL_CROUCH      (1<<3)  // must crouch to reach this waypoint
#define W_FL_LADDER      (1<<4)  // waypoint on a ladder
#define W_FL_LIFT        (1<<5)  // wait for lift to be down before approaching this waypoint
#define W_FL_DOOR        (1<<6)  // wait for door to open
#define W_FL_HEALTH      (1<<7)  // health kit (or wall mounted) location
#define W_FL_ARMOR       (1<<8)  // armor (or HEV) location
#define W_FL_AMMO        (1<<9)  // ammo location
#define W_FL_AMBUSH      (1<<10) // ambush wait location
#define W_FL_PRONE       (1<<13) // go prone (laying down)
#define W_FL_UNKNOWN15   (1<<19) // TODO - work out what this is
#define W_FL_JUMP        (1<<18) // must jump to reach this waypoint
#define W_FL_DOUBLE_RESOURCE (1<<19) // TODO - check this on maps other than tanith
#define W_FL_WALKABLE  (1<<20) // this waypoint is connected to the main map by walking
#define W_FL_LIFT_SWITCH (1<<21) // press this switch to activate the nearby lift
#define W_FL_LIFT_WAIT (1 << 22) // wait at this waypoint for the lift to move
#define W_FL_FORCED_WALKABLE (1 << 23) // All paths attached to this waypoint are forced to be walkable
#define W_FL_DROP_TOP (1 << 24) // This is the "top" of a drop, or 1-way path
#define W_FL_DROP_BOTTOM (1 << 25) // This is the bottom of a drop, or 1-way path
#define W_FL_DELETED     (1<<31) // used by waypoint allocation code


class WaypointManager
{
public:

    WaypointManager();

    ~WaypointManager();

    void loadWaypoints(const string& mapName);

    void saveWaypoints(const string& mapName);

    Vector getOrigin(int waypointId);

    bool isLadder(int waypointId);

	bool isLift(int waypointId);

	int findFlaggedWaypoint(int fromWaypointId, int mask);

    int getFlags(int waypointId);

    int getNumWaypoints() const { return _terrain[kSkulk]->getNumNodes(); }

    tTerrainGraph& getTerrainGraph(tEvolution evolution);

    void addWaypoint(const Vector& pos);

    void deleteWaypoint(const Vector& pos, edict_t* observer);

    struct WaypointDelta
    {
        tNodeId wptId;
        float distance;

        bool operator < (const WaypointDelta& other) const
        {
            return other.distance < distance;
        }
    };

    typedef priority_queue<WaypointDelta, deque<WaypointDelta>, less<WaypointDelta> > tWaypointQueue;

    tWaypointQueue getNearestWaypoints(tEvolution evolution, const Vector& fromPoint, edict_t* observer, bool noEdgesOk = false);

    tNodeId getNearestWaypoint(tEvolution evolution, const Vector& fromPoint, edict_t* observer, bool noEdgesOk = false);

    void toggleDebugger();

    void tick();

    void startWaypointPathAdd(const Vector& pos, edict_t* observer);

    void endWaypointPathAdd(const Vector& pos, edict_t* observer);

    void startWaypointPathDelete(const Vector& pos, edict_t* observer);

    void endWaypointPathDelete(const Vector& pos, edict_t* observer);

    void toggleWaypointFlag(const Vector& pos, edict_t* observer, int flag);

    float getHeight(Vector& atPoint);

    // warning, this is very expensive - should only be done at level load time
    bool isPathWalkable(const Vector& start, const Vector& end);

    void initLevel();

    float getCostFromDistance(tEvolution evolution, float distance);

    void calculateWalkability();

    void logWaypointFlags(int flags);

	bool isPathValid(int upstreamNodeId, int toNodeId);

protected:

    tNodeId getWalkabilityRootId();

    void calculateUnwalkableNodes(tNodeId nodeId, bit_vector& reachableNodes);

    void addWalkableEdges(tNodeId nodeId);

    void addWalkableEdgesNear(tNodeId nodeId);

    void addEdge(tEvolution evolution, int startId, int endId, float cost);

    void deleteAsymmetricLinks();

    void loadWaypoints(FILE* pFile, int numWaypoints, const string& waypointFilename);

    void loadEdges(FILE* pFile, int numWaypoints, const string& waypointFilename);

    typedef struct {
        char filetype[8];  // should be "HPB_bot\0"
        int  waypoint_file_version;
        int  waypoint_file_flags;  // unused
        int  number_of_waypoints;
        char mapname[32];  // name of map for these waypoints
    } tWaypointHeader;

    typedef struct {
        int flags;
        Vector3D origin;
    } tWaypointEntry;

    vector<tTerrainGraph*> _terrain;

    WaypointDebugger* _debugger;

    tNodeId _pathDeleteStartId;

    tNodeId _pathAddStartId;

private:

    static Log _log;
};

#endif // __NAVIGATION_WAYPOINTMANAGER_H
