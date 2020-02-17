//
// $Id: WaypointDebugger.cpp,v 1.27 2004/10/07 17:08:27 clamatius Exp $

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

#include "BotTypedefs.h"
#include "navigation/WaypointDebugger.h"
#include "BotManager.h"
#include "extern/metamod/meta_api.h"

WaypointDebugger::WaypointDebugger() :
    _lastWptDrawTime(gpGlobals->time),
    _lastWptInfoId(INVALID_NODE_ID),
    _pathMgr(NULL)
{
}


WaypointDebugger::~WaypointDebugger()
{
    delete _pathMgr;
}

Vector WaypointDebugger::getWptOrigin(tNodeId wptId)
{
    return gpBotManager->getWaypointManager().getOrigin(wptId);
}


tNodeFlags WaypointDebugger::getWptFlags(tNodeId wptId)
{
    return gpBotManager->getWaypointManager().getFlags(wptId);
}


extern int gLightningSpriteTexture;
void WaypointDebugger::drawBeam(edict_t* pEntity, const Vector& start, const Vector& end, int width,
                                int noise, int red, int green, int blue, int brightness, int speed)
{
    MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity);
    WRITE_BYTE( TE_BEAMPOINTS);
    WRITE_COORD(start.x);
    WRITE_COORD(start.y);
    WRITE_COORD(start.z);
    WRITE_COORD(end.x);
    WRITE_COORD(end.y);
    WRITE_COORD(end.z);
    WRITE_SHORT( gLightningSpriteTexture );
    WRITE_BYTE( 1 ); // framestart
    WRITE_BYTE( 10 ); // framerate
    WRITE_BYTE( 10 ); // life in 0.1's
    WRITE_BYTE( width ); // width
    WRITE_BYTE( noise );  // noise
    
    WRITE_BYTE( red );   // r, g, b
    WRITE_BYTE( green );   // r, g, b
    WRITE_BYTE( blue );   // r, g, b
    
    WRITE_BYTE( brightness );   // brightness
    WRITE_BYTE( speed );    // speed
    MESSAGE_END();
}


void WaypointDebugger::drawDebugBeam(const Vector& startPosition, const Vector& endPosition, int red, int green, int blue)
{
    for (int player_index = 1; player_index <= gpGlobals->maxClients; player_index++)
    {
        edict_t* pPlayer = INDEXENT(player_index);
        
        if (pPlayer && !pPlayer->free) {
            if (FBitSet(pPlayer->v.flags, FL_CLIENT)) {
                drawBeam(pPlayer, startPosition, endPosition, 30, 0, red, green, blue, 250, 5);
            }
        }
    }
}


void WaypointDebugger::drawDebugBeam(int wptId)
{
    drawDebugBeam(wptId, 0, 250, 0);
}


void WaypointDebugger::drawDebugBeam(int wptId, int red, int green, int blue)
{
	if (wptId >= 0) {
        for (int player_index = 1; player_index <= gpGlobals->maxClients; player_index++)
        {
            edict_t* pPlayer = INDEXENT(player_index);
            
            if (pPlayer && !pPlayer->free) {
                if (FBitSet(pPlayer->v.flags, FL_CLIENT)) {
					Vector start;
					Vector end;
					if (getWptFlags(wptId) & W_FL_CROUCH) {
                        start = getWptOrigin(wptId) - Vector(0, 0, 17);
                        end = start + Vector(0, 0, 34);
                    } else {
                        start = getWptOrigin(wptId) - Vector(0, 0, 34);
                        end = start + Vector(0, 0, 68);
                    }
					//_log.Debug("Drawing waypoint beam for %d", wptId);
					drawBeam(pPlayer, start, end, 30, 0, red, green, blue, 250, 5);
                }
            }
        }
	}
}


void WaypointDebugger::drawNearbyWaypoints(tEvolution evolution, const Vector& currentPos, edict_t* pObserver)
{
    if (_pathMgr == NULL) {
        _pathMgr = new PathManager(1.0);
        _pathMgr->setTerrain(&gpBotManager->getWaypointManager().getTerrainGraph(evolution));

        tNodeId rootNodeId = gpBotManager->getWaypointManager().getNearestWaypoint(evolution, currentPos, pObserver);
        if (rootNodeId >= 0) {
            _pathMgr->setRootNode(rootNodeId);
        }
    }

    if (_pathMgr != NULL) {
        _pathMgr->reoptimiseTree(5000);
    }

    if (gpGlobals->time > _lastWptDrawTime + 1.0) {
        WaypointManager::tWaypointQueue wptQueue = 
            gpBotManager->getWaypointManager().getNearestWaypoints(evolution, currentPos, pObserver, true);
        
        int count = 0;

        if (!wptQueue.empty() && wptQueue.top().wptId != _lastWptInfoId) {
            waypointInfo(evolution, currentPos, pObserver);
            _lastWptInfoId = wptQueue.top().wptId;

            int numEdges = gpBotManager->getWaypointManager().getTerrainGraph(evolution).getEdges(_lastWptInfoId).size();
            if ((_pathMgr != NULL) && (numEdges > 0)) {
                _pathMgr->optimiseNode(_lastWptInfoId);
            }
        }

        while (!wptQueue.empty()) {
            WaypointManager::WaypointDelta delta = wptQueue.top();
            wptQueue.pop();
            
            if (count++ > 10) {
                break;
            }
			int flags = gpBotManager->getWaypointManager().getFlags(delta.wptId);
            int highlightMask = (W_FL_LADDER | W_FL_DOOR | W_FL_AMBUSH | W_FL_LIFT_SWITCH | W_FL_LIFT_WAIT | W_FL_JUMP);
            if (flags == 0) {
                drawDebugBeam(delta.wptId, 120, 5, 0);

            } else if ((flags & highlightMask) != 0) {
                drawDebugBeam(delta.wptId, 0, 255, 0);

            } else {
				drawDebugBeam(delta.wptId, 255, 0, 0);
            }

            vector<Edge>& edges = gpBotManager->getWaypointManager().getTerrainGraph(evolution).getEdges(delta.wptId);
            for (vector<Edge>::iterator ii = edges.begin(); ii != edges.end(); ii++) {
                Vector start(getWptOrigin(ii->getStartId()));
                Vector end(getWptOrigin(ii->getEndId()));
                drawDebugBeam(start, end, 5, 250, 255);
            }
        }
        _lastWptDrawTime = gpGlobals->time;
    }
}


void WaypointDebugger::waypointInfo(tEvolution evolution, const Vector& currentPos, edict_t* pObserver)
{
    tNodeId nodeId = gpBotManager->getWaypointManager().getNearestWaypoint(evolution, currentPos, pObserver, true);
    if (nodeId >= 0) {
        LOG_CONSOLE(PLID, "Nearest waypoint is %d", nodeId);
        int flags = gpBotManager->getWaypointManager().getFlags(nodeId);
        gpBotManager->getWaypointManager().logWaypointFlags(flags);


        drawDebugBeam(nodeId, 255, 0, 0);

        if ((_pathMgr != NULL) && _pathMgr->treeValid()) {
            LOG_CONSOLE(PLID, "Distance estimate: %f", _pathMgr->getDistance(nodeId));
        }

        vector<Edge>& edges = gpBotManager->getWaypointManager().getTerrainGraph(evolution).getEdges(nodeId);
        for (vector<Edge>::iterator ii = edges.begin(); ii != edges.end(); ii++) {
            LOG_CONSOLE(PLID, "Edge to %d, length %f", ii->getEndId(), ii->getCost());
        }


    } else {
        LOG_CONSOLE(PLID, "Couldn't find a nearby waypoint");
        LOG_ERROR(PLID, "Couldn't find a nearby waypoint");
    }
}


void WaypointDebugger::nodeAdded(tNodeId nodeId)
{
    if (_pathMgr != NULL) {
        if (nodeId >= _pathMgr->getTerrain()->getNumNodes() - 1) {
            _pathMgr->setRootNode(nodeId);
        }
    }
}


void WaypointDebugger::nodeDeleted(tNodeId nodeId)
{
    if (_pathMgr != NULL) {
        _pathMgr->rebuildTree();
    }
}


edict_t* WaypointDebugger::getEditingPlayer()
{
    edict_t* returnEdict = NULL;
    int numPlayers = 0;
  	for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
    	edict_t* pPlayerEdict = INDEXENT(ii);

		if (pPlayerEdict && !pPlayerEdict->free && FBitSet(pPlayerEdict->v.flags, FL_CLIENT) &&
            (gpBotManager->getBot(pPlayerEdict) == NULL) && (pPlayerEdict->v.health > 0) && (pPlayerEdict->v.deadflag == DEAD_NO) &&
            (pPlayerEdict->v.skin >= 0))
        {
            returnEdict = pPlayerEdict;
            numPlayers++;
        }
    }
    return (numPlayers == 1) ? returnEdict : NULL;
}
