//
// $Id: WaypointManager.cpp,v 1.9 2005/02/19 07:40:13 clamatius Exp $

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
#include "navigation/WaypointManager.h"
#include "model/TerrainData.h"
#include "sensory/BotSensor.h"
#include <stdio.h>
#include "extern/metamod/meta_api.h"
#include "Bot.h"
#include "worldstate/WorldStateUtil.h"

const float DEFAULT_REWARD_FACTOR = 1.0;
const int WAYPOINT_VERSION = 4;

Log WaypointManager::_log(__FILE__);
const Vector NOWHERE(-16000, -16000, -16000);
const Vector3D NOWHERE3D(-16000, -16000, -16000);

// skulk, lerk, gorge, fade, onos
const float evolutionSpeeds[] = { 250.0, 250.0, 150.0, 200.0, 150.0 };

WaypointManager::WaypointManager() :
    _terrain(),
    _debugger(NULL),
    _pathDeleteStartId(INVALID_NODE_ID),
    _pathAddStartId(INVALID_NODE_ID)
{
    _terrain.resize(NUM_EVOLUTIONS);
    for (int ii = 0; ii < NUM_EVOLUTIONS; ii++) {
        _terrain[ii] = new tTerrainGraph();
    }
}

WaypointManager::~WaypointManager()
{
    delete _debugger;
    _debugger = NULL;
}


string buildFileName(const string& dir, const string& filename)
{
    // Get the game directory from Metamod
    string gameDir(GET_GAME_INFO(PLID, GINFO_GAMEDIR));
    // and append our relative path to the config file

    string returnVal = gameDir + "/" + dir + "/" + filename;

#ifndef __linux
	// TODO - work out the real way to do this
	string tmp;
	for (int ii = 0; ii < (int)returnVal.length(); ii++) {
		if (returnVal[ii] == '/') {
			tmp += '\\';

		} else {
			tmp += returnVal[ii];
		}
	}
	returnVal = tmp;
	//returnVal.replace(returnVal.begin(), returnVal.end(), string("/"), string("\\"));
#endif

	return returnVal;
}


Vector convertToHalfLife(const Vector3D& pathematicsVector)
{
    return Vector(pathematicsVector.getX(), pathematicsVector.getY(), pathematicsVector.getZ());
}


void WaypointManager::addEdge(tEvolution evolution, int startId, int endId, float cost)
{
    assert(evolution >= 0);
    assert(evolution < NUM_EVOLUTIONS);
    assert(_terrain[evolution]->nodeIdValid(startId));
    assert(_terrain[evolution]->nodeIdValid(endId));

    if ((evolution >= 0) && (evolution < NUM_EVOLUTIONS) &&
        _terrain[evolution]->nodeIdValid(startId) && _terrain[evolution]->nodeIdValid(endId))
    {
        tTerrainEdgeVector& edges = _terrain[evolution]->getEdges(startId);
        for (tTerrainEdgeVector::iterator ii = edges.begin(); ii != edges.end(); ii++) {
            if (ii->getEndId() == endId) {
                return;
            }
        }
        
        _terrain[evolution]->addEdge(startId, endId, cost);
    }
}


void WaypointManager::saveWaypoints(const string& mapName)
{
    string waypointFilename(buildFileName("addons/whichbot/data", mapName + ".wpt"));

    LOG_CONSOLE(PLID, "Saving waypoint file %s", waypointFilename.c_str());

    FILE* pFile = fopen(waypointFilename.c_str(), "wb");

    if (pFile != NULL) {
        tWaypointHeader header;    
        strcpy(header.filetype, "HPB_bot");
        header.waypoint_file_version = WAYPOINT_VERSION;
        header.waypoint_file_flags = 0;
        memset(header.mapname, 0, sizeof(header.mapname));
        strncpy(header.mapname, STRING(gpGlobals->mapname), sizeof(header.mapname)-1);
        header.number_of_waypoints = _terrain[kSkulk]->getNumNodes();

        // write the waypoint header to the file...
        fwrite(&header, sizeof(header), 1, pFile);

        // now write the waypoints
        for (int ii = 0; ii < header.number_of_waypoints; ii++) {
            tWaypointEntry waypoint;
            waypoint.flags = _terrain[kSkulk]->getNode(ii).getData().getFlags();
            waypoint.origin = _terrain[kSkulk]->getNode(ii).getData().getPos();
            fwrite(&waypoint, sizeof(waypoint), 1, pFile);
        }

        // now write the edges
        for (int jj = 0; jj < header.number_of_waypoints; jj++) {
            vector<Edge>& edges = _terrain[kSkulk]->getEdges(jj);
            short numEdges = edges.size();

            fwrite(&numEdges, sizeof(numEdges), 1, pFile);
            for (vector<Edge>::iterator kk = edges.begin(); kk != edges.end(); kk++) {
                short endpointId = kk->getEndId();
                
                fwrite(&endpointId, sizeof(endpointId), 1, pFile);
            }
        }

        fclose(pFile);

    } else {
        LOG_ERROR(PLID, "WaypointManager: Couldn't save waypoint file");
    }
}

void WaypointManager::loadWaypoints(const string& mapName)
{
    int ii;
    for (ii = 0; ii < NUM_EVOLUTIONS; ii++) {
        _terrain[ii]->clear();
    }

    string waypointFilename(buildFileName("addons/whichbot/data", mapName + ".wpt"));

    LOG_CONSOLE(PLID, "Loading waypoint file %s", waypointFilename.c_str());

    FILE* pFile = fopen(waypointFilename.c_str(), "rb");

    if (pFile != NULL) {
        tWaypointHeader header;
        int headerRead = fread(&header, sizeof(header), 1, pFile);
        if (headerRead == 1) {
            for (ii = 0; ii < NUM_EVOLUTIONS; ii++) {
                _terrain[ii]->resize(header.number_of_waypoints);
            }

            loadWaypoints(pFile, header.number_of_waypoints, waypointFilename);
            loadEdges(pFile, header.number_of_waypoints, waypointFilename);
 
            deleteAsymmetricLinks();
        }
        fclose(pFile);

    } else {
        LOG_ERROR(PLID, "WaypointManager: Couldn't load waypoint file %s", waypointFilename.c_str());
    }
}


void WaypointManager::loadWaypoints(FILE* pFile, int numWaypoints, const string& waypointFilename)
{
    for (int ii = 0; ii < numWaypoints; ii++) {
        tWaypointEntry waypoint;
        int amountRead = fread(&waypoint, sizeof(waypoint), 1, pFile);
        
        if (amountRead != 1) {
            _log.Debug("Couldn't read waypoint %d from file %s", ii, waypointFilename.c_str());
            break;
        }
        
        for (int jj = 0; jj < NUM_EVOLUTIONS; jj++) {
            _terrain[jj]->getNode(ii).getData().setPos(waypoint.origin);
            _terrain[jj]->getNode(ii).getData().setFlags(waypoint.flags);
        }
    }
}


void WaypointManager::loadEdges(FILE* pFile, int numWaypoints, const string& waypointFilename)
{
    for (int ii = 0; ii < numWaypoints; ii++) {
        short numEdges = 0;
        int amountRead = fread(&numEdges, sizeof(numEdges), 1, pFile);
        if (amountRead != 1) {
            _log.Debug("Couldn't read numEdges for waypoint %d from file %s", ii, waypointFilename.c_str());
            break;
        }
        
        assert(numEdges >= 0);
        assert(numEdges <= 50);
        for (int jj = 0; jj < numEdges; jj++) {
            short endpointId;
            int amountRead = fread(&endpointId, sizeof(endpointId), 1, pFile);
            // TODO - get rid of this assert
            assert(amountRead == 1);
            
            if ((endpointId >= 0) && (endpointId < numWaypoints) && (endpointId != ii)) {
                Vector startPos = convertToHalfLife(_terrain[kSkulk]->getNode(ii).getData().getPos());
                Vector endPos = convertToHalfLife(_terrain[kSkulk]->getNode(endpointId).getData().getPos());
                Vector delta = endPos - startPos;
                if (delta.Length() < 1000) {
                    addEdge(kSkulk, ii, endpointId, getCostFromDistance(kSkulk, delta.Length()));
                }
            }
        }
    }
}


void WaypointManager::addWalkableEdges(tNodeId nodeId)
{
    assert(_terrain[kGorge]->nodeIdValid(nodeId));

    if (_terrain[kGorge]->nodeIdValid(nodeId)) {
        vector<Edge>& edges = _terrain[kSkulk]->getEdges(nodeId);
        for (int ii = MIN_WALK_EVOLUTION; ii <= MAX_WALK_EVOLUTION; ii++) {
            _terrain[ii]->getEdges(nodeId).clear();
        }

        for (vector<Edge>::iterator edge = edges.begin(); edge != edges.end(); edge++) {
            tNodeId endpointId = edge->getEndId();
            
            int startFlags = getFlags(nodeId);
            int endFlags = getFlags(endpointId);
            Vector startPos = convertToHalfLife(_terrain[kSkulk]->getNode(nodeId).getData().getPos());
            Vector endPos = convertToHalfLife(_terrain[kSkulk]->getNode(endpointId).getData().getPos());
            
            bool walkable = (((startFlags | endFlags) & (W_FL_LADDER | W_FL_FORCED_WALKABLE)) != 0) ||
							(isLift(nodeId) && isLift(endpointId)) ||
                            isPathWalkable(startPos, endPos);
            
            if (walkable) {
                for (int ii = MIN_WALK_EVOLUTION; ii <= MAX_WALK_EVOLUTION; ii++) {
                    float cost = getCostFromDistance((tEvolution)ii, (endPos - startPos).Length());
					if ((startFlags & endFlags & W_FL_LADDER) != 0) {
						// Make ladder route cost more expensive
						cost *= 5.0;
					}
                    addEdge((tEvolution)ii, nodeId, endpointId, cost);
                }
            }
        }
    }
}


void WaypointManager::deleteAsymmetricLinks()
{
    int numDeletedEdges = 0;
    int numNormalEdges = 0;
    // do reverse-lookup on all edges to make sure they're symmetric. delete edges that aren't.
    for (int evolution = 0; evolution < NUM_EVOLUTIONS; evolution++) {
        vector<Edge*> edgesToDelete;

        for (int startNodeId = 0; startNodeId < _terrain[evolution]->getNumNodes(); startNodeId++) {
            vector<Edge>& edges = _terrain[evolution]->getEdges(startNodeId);
            
            for (vector<Edge>::iterator edge = edges.begin(); edge != edges.end(); edge++) {
                vector<Edge>& endpointEdges = _terrain[evolution]->getEdges(edge->getEndId());
                
                bool reverseFound = false;
                for (vector<Edge>::iterator reverseEdge = endpointEdges.begin(); reverseEdge != endpointEdges.end(); reverseEdge++) {
                    if (edge->getStartId() == reverseEdge->getEndId()) {
                        numNormalEdges++;
                        reverseFound = true;
                        break;
                    }
                }
                
                if (!reverseFound) {
                    //_terrain[evolution]->addEdge(edge->getEndId(), edge->getStartId(), edge->getCost());
                    edgesToDelete.push_back(new Edge(edge->getStartId(), edge->getEndId(), edge->getCost()));
                }
            }
        }
        
        for (vector<Edge*>::iterator edge = edgesToDelete.begin(); edge != edgesToDelete.end(); edge++) {
            tNodeId startId = (*edge)->getStartId();
            tNodeId endId = (*edge)->getEndId();
            _log.Debug("Edge from %d to %d doesn't have a reverse link.  Deleting it.", startId, endId);
            _terrain[evolution]->deleteEdge((*edge)->getStartId(), (*edge)->getEndId());
            numDeletedEdges++;
            delete (*edge);
        }
    }
}


Vector WaypointManager::getOrigin(int waypointId)
{
    assert(waypointId >= 0);
    assert(waypointId < _terrain[kSkulk]->getNumNodes());

    if (_terrain[kSkulk]->nodeIdValid(waypointId)) {
        return convertToHalfLife(_terrain[kSkulk]->getNode(waypointId).getData().getPos());

    } else {
        return NOWHERE;
    }
}


int WaypointManager::getFlags(int waypointId)
{
    assert(waypointId >= 0);
    assert(waypointId < _terrain[kSkulk]->getNumNodes());

    if (_terrain[kSkulk]->nodeIdValid(waypointId)) {
        return _terrain[kSkulk]->getNode(waypointId).getData().getFlags();

    } else {
        return 0;
    }
}


WaypointManager::tWaypointQueue WaypointManager::getNearestWaypoints(tEvolution evolution, const Vector& fromPoint, edict_t* observer, bool noEdgesOk)
{
    assert(evolution >= 0);
    assert(evolution < NUM_EVOLUTIONS);

    tWaypointQueue waypoints;
    if ((evolution >= 0) && (evolution < NUM_EVOLUTIONS)) {
        // FIXME - need octree here, linear scanning sucks
        // FIXME - need to check reachability of waypoints
        float closestSquare = 1000000.0;
        tNodeId closestNodeId = INVALID_NODE_ID;
        
        for (tNodeId ii = 0; ii < _terrain[evolution]->getNumNodes(); ii++) {
            if (noEdgesOk || _terrain[evolution]->getEdges(ii).size() > 0) {
                TerrainData& data = _terrain[evolution]->getNode(ii).getData();
                
                Vector toPoint(data.getPos().getX(), data.getPos().getY(), data.getPos().getZ());
                
                float xdiff = fromPoint.x - toPoint.x;
                float ydiff = fromPoint.y - toPoint.y;
                float zdiff = fromPoint.z - toPoint.z;
                
                float distanceSquare = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

                bool reachable = true;
				switch (evolution) {
				case kSkulk:
					reachable = BotSensor::isPointVisible(fromPoint, toPoint, observer);
					break;
				case kGorge:
                case kLerk:
                case kFade:
                case kOnos:
					reachable = (distanceSquare < 400.0 * 400.0) && (((getFlags(ii) & W_FL_LADDER | W_FL_FORCED_WALKABLE) != 0) || 
								isPathWalkable(fromPoint, toPoint));
					break;
				}
                if (reachable) {
                    WaypointDelta delta;
                    delta.wptId = ii;
                    delta.distance = sqrt(distanceSquare);
                    waypoints.push(delta);
                }
            }
        }
    }

    return waypoints;
}


tNodeId WaypointManager::getNearestWaypoint(tEvolution evolution, const Vector& fromPoint, edict_t* observer, bool noEdgesOk)
{
    assert(evolution >= 0);
    assert(evolution < NUM_EVOLUTIONS);

    // FIXME - need octree here, linear scanning sucks
    // FIXME - need to check reachability of waypoints
    float closestSquare = 1000000.0;
    tNodeId closestNodeId = INVALID_NODE_ID;

    if ((evolution >= 0) && (evolution < NUM_EVOLUTIONS)) {
        
        for (tNodeId ii = 0; ii < _terrain[evolution]->getNumNodes(); ii++) {
            if (noEdgesOk || _terrain[evolution]->getEdges(ii).size() > 0) {
                TerrainData& data = _terrain[evolution]->getNode(ii).getData();
                
                Vector toPoint(data.getPos().getX(), data.getPos().getY(), data.getPos().getZ());
                
                float xdiff = fromPoint.x - toPoint.x;
                float ydiff = fromPoint.y - toPoint.y;
                float zdiff = fromPoint.z - toPoint.z;
                
                float distanceSquare = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;
                
                if (distanceSquare < closestSquare && BotSensor::isPointVisible(fromPoint, toPoint, observer)) {
                    closestSquare = distanceSquare;
                    closestNodeId = ii;
                }
            }
        }
    }

    return closestNodeId;
}


void WaypointManager::toggleDebugger()
{
    if (_debugger == NULL) {
        _debugger = new WaypointDebugger();
        LOG_CONSOLE(PLID, "Waypoint debugging enabled.");

    } else {
        delete _debugger;
        _debugger = NULL;
        LOG_CONSOLE(PLID, "Waypoint debugging disabled.");
    }
}


void WaypointManager::tick()
{
    if (_debugger != NULL) {
        // draw waypoint beams around all the players
		edict_t* pEditor = _debugger->getEditingPlayer();
        if (pEditor != NULL) {
    		tEvolution evolution = Bot::getEvolution(pEditor);
            _debugger->drawNearbyWaypoints(evolution, pEditor->v.origin, pEditor);
        }
    }
}


void WaypointManager::addWaypoint(const Vector& pos)
{
    // first try to find an unused node id
    tNodeId nodeId = INVALID_NODE_ID;
    int numNodes = _terrain[kSkulk]->getNumNodes();
    if (numNodes > 1) {
        for (int ii = 0; ii < numNodes; ii++) {
            bool unusedNode = (ii != _pathAddStartId) && (getFlags(ii) == 0);
			if (unusedNode) {
				for (int moveType = 0; moveType < NUM_EVOLUTIONS; moveType++) {
					int numEdges = _terrain[moveType]->getEdges(ii).size();
					if (numEdges != 0) {
						unusedNode = false;
						break;
					}
				}
			}
            if (unusedNode) {
                nodeId = ii;
                break;
            }
        }
    }
    if (nodeId == INVALID_NODE_ID) {
        nodeId = numNodes;
    }

    Vector3D pathematicsPos(pos.x, pos.y, pos.z);
    TerrainData data(pathematicsPos, 0);
    Node node(nodeId, data);
    for (int moveType = 0; moveType < NUM_EVOLUTIONS; moveType++) {
        _terrain[moveType]->addNode(node);
    }

    /*
    // now add the edges
    tWaypointQueue nearestWaypoints = getNearestWaypoints(kSkulk, pos, NULL, true);
    while (!nearestWaypoints.empty()) {
        tNodeId endpointId = nearestWaypoints.top().wptId;

        // we only auto-add nearby edges that are walkable
        if (endpointId != nodeId && nearestWaypoints.top().distance < 250) {

            if (isPathWalkable(pos, getOrigin(endpointId))) {
                _terrain[kSkulk]->addEdge(nodeId, endpointId, nearestWaypoints.top().distance);
                _terrain[kSkulk]->addEdge(nearestWaypoints.top().wptId, nodeId, nearestWaypoints.top().distance);
                _terrain[kLerk]->addEdge(nodeId, endpointId, nearestWaypoints.top().distance);
                _terrain[kLerk]->addEdge(nearestWaypoints.top().wptId, nodeId, nearestWaypoints.top().distance);
            }
        }

        nearestWaypoints.pop();
    }

    addWalkableEdgesNear(nodeId);
    */
    if (_debugger != NULL) {
        _debugger->nodeAdded(nodeId);
    }

    LOG_CONSOLE(PLID, "Added waypoint %d", nodeId);
}


void WaypointManager::startWaypointPathAdd(const Vector& pos, edict_t* observer)
{
    _pathAddStartId = getNearestWaypoint(kSkulk, pos, observer, true);
    LOG_CONSOLE(PLID, "Start waypoint add id is %d", _pathAddStartId);
}


void WaypointManager::startWaypointPathDelete(const Vector& pos, edict_t* observer)
{
    _pathDeleteStartId = getNearestWaypoint(kSkulk, pos, observer);
    LOG_CONSOLE(PLID, "Start waypoint delete id is %d", _pathDeleteStartId);
}


void WaypointManager::endWaypointPathAdd(const Vector& pos, edict_t* observer)
{
    tNodeId pathAddEndId = getNearestWaypoint(kSkulk, pos, observer, true);
    if ((pathAddEndId >= 0) && (_pathAddStartId >= 0)) {
        if (pathAddEndId != _pathAddStartId) {
            float distance = (pos - getOrigin(_pathAddStartId)).Length();

            _terrain[kSkulk]->addEdge(_pathAddStartId, pathAddEndId, distance);
            _terrain[kSkulk]->addEdge(pathAddEndId, _pathAddStartId, distance);
            _terrain[kLerk]->addEdge(_pathAddStartId, pathAddEndId, distance);
            _terrain[kLerk]->addEdge(pathAddEndId, _pathAddStartId, distance);

            addWalkableEdges(_pathAddStartId);
            addWalkableEdges(pathAddEndId);

            LOG_CONSOLE(PLID, "Added path from %d to %d", _pathAddStartId, pathAddEndId);
            _pathAddStartId = INVALID_NODE_ID;
        }
    }
}

void WaypointManager::endWaypointPathDelete(const Vector& pos, edict_t* observer)
{
    tNodeId pathDeleteEndId = getNearestWaypoint(kSkulk, pos, observer);
    if ((pathDeleteEndId >= 0) && (_pathDeleteStartId >= 0)) {
        if (pathDeleteEndId != _pathDeleteStartId) {
            _terrain[kSkulk]->deleteEdge(_pathDeleteStartId, pathDeleteEndId);
            _terrain[kSkulk]->deleteEdge(pathDeleteEndId, _pathDeleteStartId);
            _terrain[kLerk]->deleteEdge(_pathDeleteStartId, pathDeleteEndId);
            _terrain[kLerk]->deleteEdge(pathDeleteEndId, _pathDeleteStartId);
        
            addWalkableEdges(_pathDeleteStartId);
            addWalkableEdges(pathDeleteEndId);

            LOG_CONSOLE(PLID, "Deleted path from %d to %d", _pathDeleteStartId, pathDeleteEndId);
            _pathDeleteStartId = INVALID_NODE_ID;
        }
    }
}


void WaypointManager::deleteWaypoint(const Vector& pos, edict_t* observer)
{
    tNodeId nodeId = getNearestWaypoint(kSkulk, pos, observer, true);
    if (nodeId >= 0 && nodeId < getNumWaypoints()) {
        for (int jj = 0; jj < NUM_EVOLUTIONS; jj++) {
            vector<Edge> edges = _terrain[jj]->getEdges(nodeId);
            for (int edgeIdx = 0; edgeIdx < edges.size(); edgeIdx++) {
                Edge& edge = edges[edgeIdx];
                tNodeId endpointId = edge.getEndId();
                if (endpointId >= 0 && endpointId < getNumWaypoints()) {
                    _terrain[jj]->deleteEdge(endpointId, nodeId);
                }
                _terrain[jj]->deleteEdge(nodeId, endpointId);
            }
            _terrain[jj]->getNode(nodeId).getData().setPos(NOWHERE3D);
        }
        
        if (_debugger != NULL) {
            _debugger->nodeDeleted(nodeId);
        }
    }
}

tTerrainGraph& WaypointManager::getTerrainGraph(tEvolution evolution)
{
    assert(evolution >= 0);
    assert(evolution < NUM_EVOLUTIONS);

    if ((evolution >= 0) && (evolution < NUM_EVOLUTIONS)) {
        return *_terrain[evolution];

    } else {
        return *_terrain[kSkulk];
    }
}


void WaypointManager::toggleWaypointFlag(const Vector& pos, edict_t* observer, int flag)
{
    tNodeId nodeId = getNearestWaypoint(kSkulk, pos, observer, true);
    if (_terrain[kSkulk]->nodeIdValid(nodeId)) {
        int flags = getFlags(nodeId);
        flags ^= flag;
        for (int ii = 0; ii < NUM_EVOLUTIONS; ii++) {
            _terrain[ii]->getNode(nodeId).getData().setFlags(flags);
        }

        logWaypointFlags(flags);

        addWalkableEdgesNear(nodeId);
    }
}


void WaypointManager::logWaypointFlags(int flags)
{
    if ((flags & W_FL_DOOR) != 0) {
        LOG_CONSOLE(PLID, "It's a door waypoint");
    }
    if ((flags & W_FL_LADDER) != 0) {
        LOG_CONSOLE(PLID, "It's a ladder waypoint");
    }
    if ((flags & W_FL_LIFT) != 0) {
        LOG_CONSOLE(PLID, "It's a lift waypoint");
    }
    if ((flags & W_FL_LIFT_SWITCH) != 0) {
        LOG_CONSOLE(PLID, "It's a lift switch waypoint");
    }
    if ((flags & W_FL_LIFT_WAIT) != 0) {
        LOG_CONSOLE(PLID, "It's a lift wait waypoint");
    }
    if ((flags & W_FL_WALKABLE) == 0) {
        LOG_CONSOLE(PLID, "You can't walk to it from the CC");
    }
    if ((flags & W_FL_FORCED_WALKABLE) != 0) {
        LOG_CONSOLE(PLID, "Paths attached to this waypoint are automatically walkable");
    }    
    if ((flags & W_FL_JUMP) != 0) {
        LOG_CONSOLE(PLID, "Bots approaching this waypoint will jump");
    }
    if ((flags & W_FL_AMBUSH) != 0) {
        LOG_CONSOLE(PLID, "It's an ambush waypoint");
    }
	if ((flags & W_FL_DROP_TOP) != 0) {
        LOG_CONSOLE(PLID, "It's the start of a 1-way path");
	}

	if ((flags & W_FL_DROP_BOTTOM) != 0) {
        LOG_CONSOLE(PLID, "It's the end of a 1-way path");
	}
}


void WaypointManager::addWalkableEdgesNear(tNodeId nodeId)
{
    assert(_terrain[kGorge]->nodeIdValid(nodeId));

    if (_terrain[kGorge]->nodeIdValid(nodeId)) {
        addWalkableEdges(nodeId);
        tWaypointQueue nearestWaypoints = getNearestWaypoints(kSkulk, getOrigin(nodeId), NULL, true);
        while (!nearestWaypoints.empty()) {
            addWalkableEdges(nearestWaypoints.top().wptId);
            nearestWaypoints.pop();
        }
    }
}

float WaypointManager::getHeight(Vector& atPoint)
{
    Vector below(atPoint);
    below.z -= 1000.0;

    TraceResult tr;
	WorldStateUtil::checkVector(atPoint);
	WorldStateUtil::checkVector(below);
    UTIL_TraceLine(atPoint, below, ignore_monsters, NULL, &tr);

    if (tr.fStartSolid) {
        return -1;

    } else {
        return tr.flFraction * 1000.0;
    }
}


bool WaypointManager::isPathWalkable(const Vector& start, const Vector& end)
{
    // This is going to be pretty damn inefficient, but hopefully we'll only need it at level load time
    bool pointVisible = BotSensor::isPointVisible(start, end, NULL);

    if (!pointVisible) {
        // dirty little hack to let us "look through" doors
        TraceResult tr;
        UTIL_TraceLine(start, end, ignore_monsters, NULL, &tr);
        if ((tr.pHit != NULL) && 
			strncmp(STRING(tr.pHit->v.classname), "func_door", strlen("func_door")) == 0)
		{
            return true;
        }
    }

    if (pointVisible) {
        if ((POINT_CONTENTS(start.toConstFloatArray()) == CONTENTS_WATER) && 
            (POINT_CONTENTS(end.toConstFloatArray()) == CONTENTS_WATER))
        {
            // we'll assume we can swim there
            return true;
        }

        Vector direction(end - start);
        float distance = direction.Length();
        direction = direction.Normalize();

        Vector checkStart(start);

        float currentHeight = 0.0;

        do {
            checkStart = checkStart + (direction * CHECK_WALKABLE_RESOLUTION);

            currentHeight = getHeight(checkStart);

            distance -= CHECK_WALKABLE_RESOLUTION;

        } while ((distance >= CHECK_WALKABLE_RESOLUTION) && (currentHeight <= MAX_HEIGHT_WALKABLE) && (currentHeight >= 0));

        if ((distance < CHECK_WALKABLE_RESOLUTION) && (currentHeight <= MAX_HEIGHT_WALKABLE) && (currentHeight >= 0)) {
            return true;
        }
    }

    return false;
}


void WaypointManager::calculateUnwalkableNodes(tNodeId nodeId, bit_vector& reachableNodes)
{
    assert(_terrain[kSkulk]->nodeIdValid(nodeId));

    if (_terrain[kSkulk]->nodeIdValid(nodeId)) {
        reachableNodes[nodeId] = true;
        for (int moveType = 0; moveType < NUM_EVOLUTIONS; moveType++) {
            _terrain[moveType]->getNode(nodeId).getData().setFlags(getFlags(nodeId) | W_FL_WALKABLE);
        }
        
        vector<Edge>& edges = _terrain[MIN_WALK_EVOLUTION]->getEdges(nodeId);
        for (vector<Edge>::iterator ii = edges.begin(); ii != edges.end(); ii++) {
            tNodeId endpointId = ii->getEndId();
            if (!reachableNodes[endpointId]) {            
                calculateUnwalkableNodes(endpointId, reachableNodes);
            }
        }
    }
}


tNodeId WaypointManager::getWalkabilityRootId()
{
    // you must be able to walk to the cc to be considered a walkable position
    CBaseEntity* pEntity = NULL;
    pEntity = UTIL_FindEntityByClassname(pEntity, "team_command");

    if ((pEntity != NULL) && (pEntity->edict() != NULL)) {
        return getNearestWaypoint(MIN_WALK_EVOLUTION, pEntity->edict()->v.origin, NULL);

    } else {
        return INVALID_NODE_ID;
    }
}


void WaypointManager::initLevel()
{
    calculateWalkability();
}


void WaypointManager::calculateWalkability()
{
    int numWaypoints = _terrain[MIN_WALK_EVOLUTION]->getNumNodes();

    int ii = 0;
    for (ii = 0; ii < numWaypoints; ii++) {
        for (int moveType = 0; moveType < NUM_EVOLUTIONS; moveType++) {
            _terrain[moveType]->getNode(ii).getData().setFlags(getFlags(ii) & ~W_FL_WALKABLE);
        }
    }

    for (ii = 0; ii < numWaypoints; ii++) {
        addWalkableEdges(ii);
    }

    if (numWaypoints > 0) {
        tNodeId rootId = getWalkabilityRootId();
		if (_terrain[MIN_WALK_EVOLUTION]->nodeIdValid(rootId)) {
	        bit_vector reachableNodes(numWaypoints);
		    calculateUnwalkableNodes(rootId, reachableNodes);
		}
    }
}


bool WaypointManager::isLadder(int waypointId)
{
    return (_terrain[kSkulk]->nodeIdValid(waypointId) && ((getFlags(waypointId) & W_FL_LADDER) != 0));
}


bool WaypointManager::isLift(int waypointId)
{
	return (_terrain[kSkulk]->nodeIdValid(waypointId) && ((getFlags(waypointId) & W_FL_LIFT) != 0));
}


float WaypointManager::getCostFromDistance(tEvolution evolution, float distance)
{
    assert(evolution >= 0);
    assert(evolution < NUM_EVOLUTIONS);
    assert(distance > 0);

    if ((evolution >= 0) && (evolution < NUM_EVOLUTIONS) && (distance > 0)) {
        float expectedSpeed = evolutionSpeeds[evolution];
        if (expectedSpeed > 0) {
            return distance / expectedSpeed;

        } else {
            return 10000000.0;
        }

    } else {
        return MAX_DISTANCE_ESTIMATE;
    }
}


int WaypointManager::findFlaggedWaypoint(int fromWaypointId, int mask)
{
	assert(_terrain[kGorge]->nodeIdValid(fromWaypointId));

	if (_terrain[kGorge]->nodeIdValid(fromWaypointId)) {
		tWaypointQueue nearestWpts = getNearestWaypoints(kGorge, getOrigin(fromWaypointId), NULL);
		while (!nearestWpts.empty()) {
	        tNodeId wptId = nearestWpts.top().wptId;
			if ((getFlags(wptId) & mask) == mask) {
				return wptId;

			}
			nearestWpts.pop();
		}
	}
	return INVALID_NODE_ID;
}


bool WaypointManager::isPathValid(int upstreamNodeId, int toNodeId)
{
	// Path is valid unless it goes up a "drop" path, e.g. a drop from a ceiling
	// Don't forget - when we're propagating rewards it goes from the reward source to the bot
	// so here "upstream" means "towards the reward"
	bool upstreamIsTop = (getFlags(upstreamNodeId) & W_FL_DROP_TOP) != 0;
	if (upstreamIsTop) {
		bool thisNodeIsBottom = (getFlags(toNodeId) & W_FL_DROP_BOTTOM) != 0;
		if (thisNodeIsBottom) {
			return false;
		}
	}
	return true;
}
