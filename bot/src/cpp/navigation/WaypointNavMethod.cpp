//
// $Id: WaypointNavMethod.cpp,v 1.8 2005/12/14 19:25:39 clamatius Exp $

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
#include "navigation/WaypointNavMethod.h"
#include "framework/Log.h"
#include "worldstate/AreaManager.h"
#include "BotManager.h"
#include "config/TranslationManager.h"
#include "worldstate/WorldStateUtil.h"

Log WaypointNavMethod::_log(__FILE__);

const float WAYPOINT_HEIGHT_OFFSET = -10.0;
const float LAST_USED_SWITCH_TIME = 2.5;
const float MAX_TIME_TO_TRAVERSE_WAYPOINTS = 10.0;
const float MAX_TIME_TO_TRAVERSE_GOAL = 60.0;
const float NEAR_WPT_DISTANCE = 40.0;
const float ONOS_NEAR_WPT_DISTANCE = 80.0;
const float REOPTIMISE_FACTOR = 3.0;
const float LONG_DISTANCE = 1000.0;
const float WAYPOINT_LENGTH_ADJUSTMENT_RATE = 0.1;
const float WAYPOINT_NOT_REACHED_TIME_PENALTY = 60.0;
const float IMPASSABLE_THRESHOLD = 40.0;
const float LIFT_NEAR = 50.0;
const float LIFT_TIMEOUT = 15.0;

WaypointNavMethod::WaypointNavMethod(Bot& bot, bool initialSpawn) :
    _mode(LOST),
    _nextWptId(-1),
    _useButtonTime(0),
    _lastWptTime(gpGlobals->time),
    _bot(bot),
    _evolution(bot.getEvolution()),
    _waitAtWp(false)
{
    tTerrainGraph* pExpectedTerrain = &gpBotManager->getWaypointManager().getTerrainGraph(_bot.getEvolution());
    if (_bot.getPathManager().getTerrain() != pExpectedTerrain) {
        _bot.getPathManager().setTerrain(pExpectedTerrain);
    }
}


WaypointNavMethod::~WaypointNavMethod()
{
}


int WaypointNavMethod::getNearestWaypointId()
{
    WaypointManager::tWaypointQueue wptQueue = 
        gpBotManager->getWaypointManager().getNearestWaypoints(_bot.getEvolution(), _bot.getEdict()->v.origin, _bot.getEdict());
    if (!wptQueue.empty()) {
        tNodeId wptId = wptQueue.top().wptId;
        if (wptId == prevWaypointId() && !wptQueue.empty()) {
            wptQueue.pop();
			if (wptQueue.empty()) {
				return INVALID_NODE_ID;

			} else {
	            wptId = wptQueue.top().wptId;
			}
        }
        return wptId;

    } else {
        return INVALID_NODE_ID;
    }
}


NavigationMethod* WaypointNavMethod::navigate()
{
    if (_bot.getMovement() == NULL) {
        return NULL;
    }

    // check to make sure we didn't just change evolution
    if (_bot.getEvolution() != _evolution) {
        changedEvolution();
    }

    switch (_mode) {
    case LOST:
        navigateLost();
        break;

    case SEEKING_NEAREST:
        //DrawDebugBeam(_nextWptId);
        //DrawDebugBeam(_bot.getEdict()->v.origin, gpBotManager->getWaypointManager().getOrigin(_nextWptId), 0, 0, 255);
        if (_nextWptId != prevWaypointId()) {
            addHistoryWaypoint(_nextWptId);
        }

        if (_bot.getPathManager().nodeIdValid(_nextWptId) && (_bot.getPathManager().getRootNodeId() == INVALID_NODE_ID)) {
            _bot.getPathManager().setRootNode(_nextWptId);
        }

        _bot.getPathManager().reoptimiseTree((int)(gpBotManager->getWaypointManager().getNumWaypoints() * REOPTIMISE_FACTOR));
        seekNearestWaypoint();
        break;

    case SEEKING_REWARDS:
        //DrawDebugBeam(_nextWptId);
        //DrawDebugBeam(_bot.getEdict()->v.origin, gpBotManager->getWaypointManager().getOrigin(_nextWptId), 255, 0, 0);
        _bot.getPathManager().reoptimiseTree((int)(gpBotManager->getWaypointManager().getNumWaypoints() * REOPTIMISE_FACTOR));
        seekRewards();
        break;

    case FINDING_SWITCH:
        findSwitch();
        break;

    case RETRACING_STEPS:
        retraceSteps();
        break;

    case WAIT_AT_WAYPOINT:
        // _bot.getPathManager().reoptimiseTree((int)(gpBotManager->getWaypointManager().getNumWaypoints() * REOPTIMISE_FACTOR));
        // Don't move the bot.
        waitAtWaypoint();
        return NULL;

	case WAIT_FOR_LIFT:
		waitForLift();
		break;
    }

	if (_bot.getPathManager().nodeIdValid(_nextWptId) && 
		(gpBotManager->getWaypointManager().getFlags(_nextWptId) & W_FL_JUMP) != 0)
	{
		if (RANDOM_LONG(0,100) > 50) {
			_bot.getEdict()->v.button |= IN_JUMP;
		}
	}

    if (_bot.getMovement() != NULL) {
        if (_bot.getMovement()->shouldUseMeleeAttack()) {
            _bot.selectWeapon(NULL, 0, true);
            _bot.fireWeapon();
        }
        _bot.getMovement()->move(_bot.getEvolution());
    }

    //DrawDebugBeam(_nextWptId);
    return NULL;
}


void WaypointNavMethod::pause()
{
    _waitAtWp = true;
}


void WaypointNavMethod::resume()
{
    _waitAtWp = false;
}


void WaypointNavMethod::changedEvolution()
{
    _evolution = _bot.getEvolution();
    tTerrainGraph* terrain = &gpBotManager->getWaypointManager().getTerrainGraph(_evolution);
    _bot.getPathManager().setTerrain(terrain);
    
    _nextWptId = getNearestWaypointId();
    if (_bot.getPathManager().nodeIdValid(_nextWptId)) {
        _bot.getPathManager().setRootNode(_nextWptId);
        _bot.getPathManager().reoptimiseTree((int)(REOPTIMISE_FACTOR * gpBotManager->getWaypointManager().getNumWaypoints()));
        _bot.getPathManager().trickleRewards(_bot.getStrategyManager().getRewards(_evolution));
        setNextWaypointTarget();
                
    } else {
        _mode = LOST;
    }
}


void WaypointNavMethod::navigateLost()
{
    _nextWptId = getNearestWaypointId();
    if (_bot.getPathManager().nodeIdValid(_nextWptId)) {
        //_log.Debug("Nearest visible waypoint is %d", _nextWptId);
        _lastWptTime = gpGlobals->time;
        _mode = SEEKING_NEAREST;
        setNextWaypointTarget();
        
    } else {
        _log.Debug("No nearby visible waypoint found");
        if (_bot.getMovement() != NULL) {
            _bot.getMovement()->setRandomTarget();
            _bot.getMovement()->move(_bot.getEvolution());
        }
    }
}

Vector WaypointNavMethod::getWptOrigin(tNodeId wptId)
{
    return gpBotManager->getWaypointManager().getOrigin(wptId) + Vector(0, 0, WAYPOINT_HEIGHT_OFFSET);
}


float WaypointNavMethod::getMinTargetDistance()
{
	return (_evolution != kOnos) ? NEAR_WPT_DISTANCE : ONOS_NEAR_WPT_DISTANCE;
}


Vector WaypointNavMethod::vectorToWaypoint(int wptId)
{
    return getWptOrigin(wptId) - _bot.getEdict()->v.origin;
}


void WaypointNavMethod::foundRouteWaypoint()
{
    int flags = gpBotManager->getWaypointManager().getFlags(_nextWptId);
    if (_bot.getPathManager().nodeIdValid(_nextWptId)) {
        _bot.getStrategyManager().visitedWaypoint(_bot.getEvolution(), _nextWptId);
        _bot.getPathManager().setRootNode(_nextWptId);
        _bot.getPathManager().trickleRewards(_bot.getStrategyManager().getRewards(_bot.getEvolution()));

        int prevWptId = prevWaypointId();

        if (_bot.getPathManager().nodeIdValid(prevWptId) &&
            gpBotManager->getWaypointManager().isLadder(_nextWptId) &&
            gpBotManager->getWaypointManager().isLadder(prevWptId))
        {
            Vector vecBetweenWaypoints(getWptOrigin(_nextWptId) - getWptOrigin(prevWaypointId()));

            if (vecBetweenWaypoints.z < 0) {
                // jump off ladders at the bottom
                _bot.getEdict()->v.button |= IN_JUMP;
            }
        }

    } else {
        _log.Debug("Strange - we found the route waypoint, but next wpt id was invalid");
    }

    if ((flags & W_FL_DOOR) && (gpGlobals->time > (_useButtonTime + LAST_USED_SWITCH_TIME))) {
        _mode = FINDING_SWITCH;

    } else if ((_evolution != kSkulk) && (flags & W_FL_LIFT_SWITCH) && (gpGlobals->time > (_useButtonTime + LAST_USED_SWITCH_TIME)) && !liftIsNear()) {
		_mode = FINDING_SWITCH;

	} else if ((_evolution != kSkulk) && (flags & W_FL_LIFT_WAIT) && !liftIsNear()) {
		_mode = WAIT_FOR_LIFT;

	} else if ((_evolution != kSkulk) && (flags & W_FL_LIFT) && liftIsNear() && (gpGlobals->time > (_useButtonTime + LAST_USED_SWITCH_TIME))) {
		// we have to hit the switch to move the damn lift again
		_log.Debug("On lift.  Hitting switch to move lift...");
		_nextWptId = gpBotManager->getWaypointManager().findFlaggedWaypoint(prevWaypointId(), W_FL_LIFT_SWITCH);
	}

}


void WaypointNavMethod::seekNearestWaypoint()
{
    assert(_bot.getPathManager().getTerrain() != NULL);
    assert(_bot.getPathManager().nodeIdValid(_nextWptId));

    if ((_bot.getPathManager().getTerrain() != NULL) && (_bot.getPathManager().nodeIdValid(_nextWptId))) {
        Vector vecToWpt(vectorToWaypoint(_nextWptId));
        bool hitWaypoint = didWeHitWaypoint(vecToWpt);

        if (hitWaypoint) {
            // we hit it
            //_log.Debug("Hit nearest waypoint");
            foundRouteWaypoint();
            _mode = _waitAtWp ? WAIT_AT_WAYPOINT : SEEKING_REWARDS;
        } else if (_bot.getMovement()->getDistanceToTarget(_bot.getEvolution()) > LONG_DISTANCE) {
            _mode = LOST;
            _log.Debug("Nearest waypoint %d (%s) is a suspiciously long way away, resetting", _nextWptId, TranslationManager::getTranslation(AreaManager::getAreaName(gpBotManager->getWaypointManager().getOrigin(_nextWptId))).c_str());
            
        } else {
            if (!checkStuck()) {
                setNextWaypointTarget();
                
            } else {
                _log.Debug("Got stuck finding nearest waypoint, resetting...");
                _mode = LOST;
            }
        }
    }
}


void WaypointNavMethod::setNextWaypointTarget()
{
    int prevWptId = prevWaypointId();

    bool amWalking = ((_bot.getEvolution() >= MIN_WALK_EVOLUTION) && (_bot.getEvolution() <= MAX_WALK_EVOLUTION));

    Vector verticalOffset(0, 0, WAYPOINT_HEIGHT_OFFSET);
    Vector target(getWptOrigin(_nextWptId) + verticalOffset);

    if (amWalking && isLadderPath(prevWptId, _nextWptId)) {
        _bot.getMovement()->setLadderTarget(target, 
                                              getWptOrigin(_nextWptId).z > _bot.getEdict()->v.origin.z,
                                              getMinTargetDistance() * 0.75);
        
    } else {
        _bot.getMovement()->setTarget(target, getMinTargetDistance());
    }
}


void WaypointNavMethod::unableToFindNextWaypoint()
{
    if (_wptHistory.size() > 0) {
        if (_mode != RETRACING_STEPS) {
            _log.Debug("Suspect we're stuck, trying to go to %d (%s).  Going back to earliest waypoint",
                _nextWptId,  TranslationManager::getTranslation(AreaManager::getAreaName(gpBotManager->getWaypointManager().getOrigin(_nextWptId))).c_str());
            updateUnreachedWaypointTravelTime();
            Edge* thisWay = _bot.getPathManager().getTerrain()->getEdge(prevWaypointId(), _nextWptId);
            if (thisWay == NULL || thisWay->getCost() > IMPASSABLE_THRESHOLD) {
                _wptHistory.clear();
                giveUpOnNextWaypoint();
                
            } else {
                _mode = RETRACING_STEPS;
                _lastWptTime = gpGlobals->time;
                _nextWptId = prevWaypointId();
            }
            
        } else {
            _wptHistory.clear();
            _log.Debug("Giving up on next waypoint while retracing steps.");
            giveUpOnNextWaypoint();
        }
        
    } else {
        _log.Debug("Stuck, and no history.  Time to reboot...");
        giveUpOnNextWaypoint();
    }
}



bool WaypointNavMethod::checkStuck()
{
    if ((gpGlobals->time > _lastWptTime + MAX_TIME_TO_TRAVERSE_WAYPOINTS/2.0) && (_evolution == kOnos)) {
        _bot.getEdict()->v.button |= IN_DUCK;
    }

    float timeout = _lastWptTime +
        (WorldStateUtil::isOnLadder(_bot.getEdict()) ? 
        MAX_TIME_TO_TRAVERSE_WAYPOINTS * 5.0 : 
        MAX_TIME_TO_TRAVERSE_WAYPOINTS);

    if (gpGlobals->time > timeout) {
        // stuck?
        if (_bot.getEvolution() != kGorge) {
            // jumping when we're stuck gets us out of a bunch of situations
            _bot.getEdict()->v.button |= IN_JUMP;
            
        } else {
            // Gorges can actually get stuck inside entities thanks to a  bug, so we'll try teleporting them
            // a tad. :)
            _log.Debug("We're stuck.  Trying special gorge unstuck teleport...");
                moveOutsideEntity(_bot.getEdict());
        }

        unableToFindNextWaypoint();
        return true;
    }
    return false;
}


void WaypointNavMethod::seekRewards()
{
    assert(_bot.getPathManager().getTerrain() != NULL);

    if (_bot.getPathManager().getTerrain() != NULL) {
        if (!_bot.getPathManager().nodeIdValid(_nextWptId)) {
            _log.Debug("Uh-oh, can't find next waypoint id");
            _mode = LOST;
            return;
        }

        if (_bot.getMovement()->getDistanceToTarget(_bot.getEvolution()) > LONG_DISTANCE) {
            _log.Debug("Strange, next waypoint %d (%s) is a long way away.  Resetting to LOST", _nextWptId, TranslationManager::getTranslation(AreaManager::getAreaName(gpBotManager->getWaypointManager().getOrigin(_nextWptId))).c_str());
            _mode = LOST;
            return;
        }

        Vector vecToWpt(vectorToWaypoint(_nextWptId));
        bool hitWaypoint = didWeHitWaypoint(vecToWpt);

        if (hitWaypoint) {
            int prevWptId = prevWaypointId();
            if (prevWptId != _nextWptId) {
                updateWaypointTravelTime(prevWptId, _nextWptId, gpGlobals->time - _lastWptTime);
            }

            foundRouteWaypoint();
            
            if (_waitAtWp) {
                _mode = WAIT_AT_WAYPOINT;
            } else {
                calculateNextWaypoint();
                if (_bot.getPathManager().nodeIdValid(_nextWptId)) {
                    setNextWaypointTarget();
                }
            }
        } else {
            int currentNextId = _nextWptId;
            float lastWptTime = _lastWptTime;
            if (!checkStuck()) {
                moveBetweenWaypoints();
            }
        }
    }
}


void WaypointNavMethod::moveBetweenWaypoints()
{
    Vector target(getWptOrigin(_nextWptId));
}


bool WaypointNavMethod::didWeHitWaypoint(Vector& vecToWpt)
{
    bool hitWaypoint = false;
     
    int prevWptId = prevWaypointId();

    if (!WorldStateUtil::isOnLadder(_bot.getEdict())) {
        hitWaypoint = _bot.getMovement()->isAtTargetVector();
    }

    // if we didn't use the short-cut (less than some min distance) waypointhit check, and we are going from
    // one waypoint to another
    if (!hitWaypoint && (_bot.getPathManager().nodeIdValid(prevWptId)))
    {
        Vector vecBetweenWaypoints(getWptOrigin(_nextWptId) - getWptOrigin(prevWptId));
		if (vecBetweenWaypoints.Length() == 0) {
			hitWaypoint = true;

		} else {
			Vector vecFromPrevWaypoint(_bot.getEdict()->v.origin - getWptOrigin(prevWptId));
			
			float projectionDistance = DotProduct(vecFromPrevWaypoint, vecBetweenWaypoints) / vecBetweenWaypoints.Length();
			
			if (projectionDistance + NEAR_WPT_DISTANCE >= vecBetweenWaypoints.Length()) {
                hitWaypoint = true;
			}
		}
    }

    return hitWaypoint;
}


bool WaypointNavMethod::isLadderPath(tNodeId prevWptId, tNodeId nextWptId)
{
    //WaypointDebugger::drawDebugBeam(nextWptId);
    // are both of the waypoints a ladder wpt?
    return ((gpBotManager->getWaypointManager().isLadder(prevWptId) || 
             !_bot.getPathManager().nodeIdValid(prevWptId)) &&
            gpBotManager->getWaypointManager().isLadder(nextWptId));
}


void WaypointNavMethod::calculateNextWaypoint()
{
    if (prevWaypointId() != _nextWptId) {
        addHistoryWaypoint(_nextWptId);
    }

    _nextWptId = _bot.getPathManager().getOptimalNextNodeId();
    if (_bot.getPathManager().nodeIdValid(_nextWptId)) {
        const string& areaName = AreaManager::getAreaName(
            gpBotManager->getWaypointManager().getOrigin(_nextWptId));
        const string& locationName = TranslationManager::getTranslation(areaName);
        //_log.Debug("Next waypoint is %d in %s", _nextWptId, locationName.c_str());
                        
    } else {
        _log.Debug("Can't get a valid next wpt from path manager.  Giving up for now...");
        giveUpOnNextWaypoint();
    }
}


void WaypointNavMethod::giveUpOnNextWaypoint()
{
    _log.Debug("Giving up on next waypoint %d", _nextWptId);
    // TODO - use alternate route instead of just randomly giving up
    _mode = LOST;

    updateUnreachedWaypointTravelTime();
}


int InFieldOfView(float view_angle, Vector dest)
{
    // find angles from source to destination...
    Vector entity_angles = UTIL_VecToAngles(dest);
    
    // make yaw angle 0 to 360 degrees if negative...
    if (entity_angles.y < 0) {
        entity_angles.y += 360;
    }
    
    // make view angle 0 to 360 degrees if negative...
    if (view_angle < 0) {
        view_angle += 360;
    }
    
    // return the absolute value of angle to destination entity
    // zero degrees means straight ahead,  45 degrees to the left or
    // 45 degrees to the right is the limit of the normal view angle
    
    int angle = abs((int)view_angle - (int)entity_angles.y);
    
    if (angle > 180) {
        angle = 360 - angle;
    }
    
    return angle;
}


const int SEARCH_RADIUS = 200;
const char* BUTTON_CLASSNAME = "func_button";
const char* ROTATOR_BUTTON_CLASSNAME = "func_rot_button";
const char* DOOR_CLASSNAME = "func_door";

void WaypointNavMethod::findSwitch()
{
    bool entityFound = tryToUseEntity(BUTTON_CLASSNAME);
	if (!entityFound) {
		entityFound = tryToUseEntity(ROTATOR_BUTTON_CLASSNAME);
	}
    if (!entityFound) {
        entityFound = tryToUseEntity(DOOR_CLASSNAME);
    }

    // if we couldn't see it from where we are now, let's try going back to the waypoint to see if we have better luck next time
    if (!entityFound && _bot.getPathManager().nodeIdValid(prevWaypointId())) {
        setNextWaypointTarget();
    }

    checkStuck();
}



bool WaypointNavMethod::tryToUseEntity(const char* entityClassname)
{
	Vector targetOrigin;
	if (_nextWptId >= 0) {
		targetOrigin = getWptOrigin(_nextWptId);

	} else {
		targetOrigin = _bot.getEdict()->v.origin;
	}
    edict_t* pEntity = WorldStateUtil::findClosestSwitchEntity(entityClassname, targetOrigin);
    if (pEntity == NULL) {
        return false;
    }

    Vector entity_origin = pEntity->v.absmin + (pEntity->v.size * 0.5);
            
    Vector vecStart = _bot.getEdict()->v.origin + _bot.getEdict()->v.view_ofs;
    Vector vecToEntity = entity_origin - vecStart;
    float distance = vecToEntity.Length();
    if (distance > SEARCH_RADIUS) {
        return false;
    }

    TraceResult tr;
	WorldStateUtil::checkVector(vecStart);
	WorldStateUtil::checkVector(entity_origin);
    // trace a line from bot's centre to func_ entity...
    UTIL_TraceLine(vecStart, entity_origin, dont_ignore_monsters,
                   _bot.getEdict()->v.pContainingEntity, &tr);
    //WaypointDebugger::drawDebugBeam(vecStart, entity_origin, 255, 0, 0);            
        
    if ((_useButtonTime + LAST_USED_SWITCH_TIME) < gpGlobals->time) {
        // check if flag not set and facing it...
        int angleToEntity = InFieldOfView(_bot.getEdict()->v.v_angle.y, vecToEntity);
        if (angleToEntity <= 10) {
            
            float minDistance = 75;
            if (distance < minDistance) {
                _log.Debug("Using button");
                _bot.getEdict()->v.button |= IN_USE;
                _useButtonTime = gpGlobals->time;
                _bot.getMovement()->stop(_evolution);
                if ((_evolution != kSkulk) && 
                    (gpBotManager->getWaypointManager().getFlags(_nextWptId) | W_FL_LIFT_SWITCH) != 0)
                {
                    int nearestWaitWaypoint = 
                        gpBotManager->getWaypointManager().findFlaggedWaypoint(_nextWptId, W_FL_LIFT_WAIT);
                    if (nearestWaitWaypoint >= 0) {
                        _nextWptId = nearestWaitWaypoint;
                    }
                }
                _mode = SEEKING_REWARDS;
				calculateNextWaypoint();
            }
        }
    }
    
    _bot.getMovement()->setTarget(entity_origin, getMinTargetDistance());
    return true;
}



const int MAX_HISTORY_LENGTH = 4;
void WaypointNavMethod::addHistoryWaypoint(int wptId)
{
    _lastWptTime = gpGlobals->time;
    _wptHistory.push_front(wptId);
    if (_wptHistory.size() >= MAX_HISTORY_LENGTH) {
        _wptHistory.pop_back();
    }
}


void WaypointNavMethod::resetWaypointHistory()
{
    _wptHistory.clear();
}


void WaypointNavMethod::retraceSteps()
{
    if (_wptHistory.size() > 0) {
        _nextWptId = _wptHistory.front();
    }

    if (_bot.getPathManager().nodeIdValid(_nextWptId)) {
        Vector vecToWpt(vectorToWaypoint(_nextWptId));
        bool hitWaypoint = didWeHitWaypoint(vecToWpt);
                
        if (hitWaypoint) {
            //_log.Debug("Reached next waypoint on route, %d", _nextWptId);
            _lastWptTime = gpGlobals->time;
            _wptHistory.pop_front();
            _bot.getPathManager().setRootNode(_nextWptId);
            if (_wptHistory.size() > 0) {
                _nextWptId = _wptHistory.front();

            } else {
                _mode = SEEKING_REWARDS;
            }
            
        } else {
            if (!checkStuck()) {
                setNextWaypointTarget();
            }
        }

    } else {
        _log.Debug("Invalid waypoint in retrace steps %d, going to lost mode", _nextWptId);
        _mode = LOST;
    }
}


void WaypointNavMethod::updateWaypointTravelTime(int prevWptId, int nextWptId, float travelTime)
{
    if ((travelTime > 0) && _bot.getPathManager().nodeIdValid(prevWptId) && _bot.getPathManager().nodeIdValid(nextWptId)) {
        // find the edge we just travelled

        vector<Edge>& edges = _bot.getPathManager().getTerrain()->getEdges(prevWptId);

        Edge* thisWay = _bot.getPathManager().getTerrain()->getEdge(prevWptId, nextWptId);

		// it's sometimes possible for this to be null (e.g. if we evolved between waypoints where one waypoint isn't valid
		// for one evolution's terrain).
        if (thisWay != NULL) {
            Edge* otherWay = _bot.getPathManager().getTerrain()->getEdge(nextWptId, prevWptId);

            if (otherWay != NULL) {
                float currentCost = thisWay->getCost();
                float newCost = currentCost + (travelTime - currentCost) * WAYPOINT_LENGTH_ADJUSTMENT_RATE;
                float costDiff = newCost - currentCost;
                assert(newCost > 0);

                if (currentCost != newCost) {
                    thisWay->setCost(newCost);
                    otherWay->setCost(newCost);

                    // _log.Debug("Updated waypoint travel cost from %d to %d to %f from %f",
                    //            prevWptId, nextWptId, newCost, currentCost);

                    if (newCost > currentCost) {
                        gpBotManager->terrainUpdated(_bot.getEvolution(), prevWptId, nextWptId, costDiff);
                    }
                }
            }
        }
    }
}


void WaypointNavMethod::updateUnreachedWaypointTravelTime()
{
    int prevWptId = prevWaypointId();
    if (_bot.getPathManager().nodeIdValid(prevWptId)) {
        Edge* edge = _bot.getPathManager().getTerrain()->getEdge(prevWptId, _nextWptId);
        if (edge != NULL) {
            float prevTravelTime = edge->getCost();

            float newTravelTime = prevTravelTime + WAYPOINT_NOT_REACHED_TIME_PENALTY;

            _log.Debug("Updating unreached wpt travel time to %f from %f", newTravelTime, prevTravelTime);
            updateWaypointTravelTime(prevWptId, _nextWptId, newTravelTime);
        }
    }
}

void WaypointNavMethod::waitAtWaypoint()
{
    _bot.getMovement()->stop(_evolution);
    _bot.getMovement()->move(_evolution);
    _bot.getStrategyManager().waitedAtWaypoint(_bot.getEvolution(), _nextWptId);
    _lastWptTime = gpGlobals->time;
    if (!_waitAtWp) {
        // Ok, we're ready to move again.
        calculateNextWaypoint();
        _mode = SEEKING_REWARDS;
    }
}


void WaypointNavMethod::waitForLift()
{
	if (!liftIsNear() && (gpGlobals->time < _lastWptTime + LIFT_TIMEOUT)) {
		_log.Debug("Waiting for lift...");
		_bot.getMovement()->stop(_evolution);

	} else {
		_mode = SEEKING_REWARDS;
	}
}


bool WaypointNavMethod::liftIsNear()
{
	if (_lastLiftEntity.isNull() || (_lastLiftCheckWpt != _nextWptId)) {
		int nearestLiftWaypoint = gpBotManager->getWaypointManager().findFlaggedWaypoint(_nextWptId, W_FL_LIFT);
		if (_bot.getPathManager().nodeIdValid(nearestLiftWaypoint)) {
			_lastLiftEntity.setEdict(WorldStateUtil::findClosestEntity("func_door", gpBotManager->getWaypointManager().getOrigin(nearestLiftWaypoint)));
		}
	}

	if (!_lastLiftEntity.isNull()) {
		_lastLiftCheckWpt = _nextWptId;
		return fabs(_lastLiftEntity.getOrigin()->z - _bot.getEdict()->v.origin.z) < LIFT_NEAR;
	}
	// default to true if we don't find the lift
	_log.Debug("No lift found near lift flagged waypoints!");
	return true;
}


string WaypointNavMethod::getName() const
{
	return "WaypointNavigation";
}
