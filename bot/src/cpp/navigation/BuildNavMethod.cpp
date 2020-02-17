//
// $Id: BuildNavMethod.cpp,v 1.17 2005/11/30 23:07:02 clamatius Exp $

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
#include "navigation/BuildNavMethod.h"
#include "navigation/WaypointNavMethod.h"
#include "worldstate/HiveManager.h"
#include "BotManager.h"
#include "sensory/Target.h"
#include "strategy/HiveMind.h"
#include "worldstate/WorldStateUtil.h"

const float CLOSE_ENOUGH_TO_BUILD = 80.0;
const float CLOSE_ENOUGH_TO_MOVE_TO = 500.0;
const float CLOSE_ENOUGH_TO_HIVE = 1000.0;
const float BUILDABLE_CHECK_UPDATE = 0.2;
const Vector NOT_FOUND(-100000, -100000, -10000);

Log BuildNavMethod::_log(__FILE__);

BuildNavMethod::BuildNavMethod(Bot& bot, eEntityType entityTypeToBuild) :
	_entityTypeToBuild(entityTypeToBuild),
	_bot(bot),
    _entityLocation(NOT_FOUND),
    _lastBuildableCheck(-10),
    _buildableSeen(false),
	_lastImpulseTime(-1),
	_lastBuildProgress(0),
	_lastBuildProgressTime(gpGlobals->time),
    _buildingPlanted(false)
{
	findTarget();
}


BuildNavMethod::BuildNavMethod(Bot& bot, eEntityType entityTypeToBuild, const Vector& location) :
	_entityTypeToBuild(entityTypeToBuild),
	_entityLocation(location),
	_bot(bot)
{
}


BuildNavMethod::~BuildNavMethod()
{
}


NavigationMethod* BuildNavMethod::navigate()
{
	findTarget();
	float range = (_bot.getEdict()->v.origin - _entityLocation).Length();

	Vector targetLocation(_entityLocation);
	targetLocation.z += 100.0;
	_bot.getMovement()->setTarget(targetLocation, CLOSE_ENOUGH_TO_BUILD);

    float closeEnoughRange = CLOSE_ENOUGH_TO_BUILD;
    if (_entityTypeToBuild == kHive) {
        closeEnoughRange = CLOSE_ENOUGH_TO_HIVE;
    }

	if (range < closeEnoughRange) {
        if (!_buildableSeen) {
			if (_lastImpulseTime < 0 || gpGlobals->time > _lastImpulseTime + 0.2) {
	            setImpulse();
				_lastImpulseTime = gpGlobals->time;
                _lastBuildProgressTime = gpGlobals->time;
            }

        } else {
			// assume we've placed it and now it needs building
			_bot.getEdict()->v.button |= IN_USE;
		}
	}
    _bot.getMovement()->move(kGorge);

    if ((_entityLocation == NOT_FOUND) || (_lastImpulseTime > 0 && gpGlobals->time > _lastImpulseTime + 20.0) || 
        (gpGlobals->time > _lastBuildProgressTime + 20.0))
    {
        if (!_buildingPlanted && _entityTypeToBuild != kHive) {
            tNodeId nearestWaypointId = gpBotManager->getWaypointManager().getNearestWaypoint(
                kGorge, _bot.getEdict()->v.origin, _bot.getEdict());
            _log.Debug("Marking waypoint %d as blacklisted for building", nearestWaypointId);
            _bot.failedToBuildAt(nearestWaypointId);
        }
    	return new WaypointNavMethod(_bot, false);

    } else {
        return NULL;
    }
}


void BuildNavMethod::pause()
{
}


void BuildNavMethod::resume()
{
}


bool BuildNavMethod::findTarget()
{
	if (gpGlobals->time > _lastBuildableCheck + BUILDABLE_CHECK_UPDATE) {
        _entityLocation = NOT_FOUND;
        _lastBuildableCheck = gpGlobals->time;
		bool buildableFound = findNearbyBuildable();

		int ii = 0;
		if (!buildableFound && !_buildingPlanted) {
			const char* targetClassname = NULL;
			edict_t* pTargetEdict = NULL;

			switch (_entityTypeToBuild) {
			case kAlienResourceTower:
				targetClassname = "func_resource";
				pTargetEdict = getTargetEdict(targetClassname);
                if (pTargetEdict != NULL) {
                    _entityLocation = pTargetEdict->v.origin;
                }
				break;
			case kHive:
				targetClassname = "team_hive";
				for (ii = 0; ii < 3; ii++) {
					HiveInfo* pHiveInfo = HiveManager::getHive(ii);
					if ((pHiveInfo->getOrigin() - _bot.getEdict()->v.origin).Length() < CLOSE_ENOUGH_TO_HIVE) {
                        if (pHiveInfo->getHealth() == 0) {
    						_entityLocation = pHiveInfo->getOrigin();
                        }
					}
				}
				break;
			// for the other chambers, just plop it down
			case kOffenseChamber:
			case kDefenseChamber:
			case kSensoryChamber:
			case kMovementChamber:
				if (_lastImpulseTime - gpGlobals->time < 0.5) {
					_entityLocation = _bot.getEdict()->v.origin;
				}
				break;
			}

        } else {
            _buildingPlanted = true;
        }
	}

	return (_entityLocation != NOT_FOUND);
}


edict_t* BuildNavMethod::getTargetEdict(const char* targetClassname)
{
    edict_t* pTargetEdict = WorldStateUtil::findClosestEntity(targetClassname, _bot.getEdict()->v.origin);
    if (pTargetEdict != NULL) {
        int wptId = gpBotManager->getWaypointManager().getNearestWaypoint(kGorge, pTargetEdict->v.origin, _bot.getEdict());
        if (wptId >= 0) {
            switch (_entityTypeToBuild) {
            case kAlienResourceTower:
                if (!HiveMind::resourceNodeIsTaken(pTargetEdict->v.origin, OCCUPIED_NODE_MASK)) {
                    return pTargetEdict;

                } else {
                    edict_t* pResTowerEdict = WorldStateUtil::findClosestEntity("alienresourcetower", _bot.getEdict()->v.origin);
                    if (pResTowerEdict != NULL && BotSensor::looksBuildable(pResTowerEdict)) {
                        // we just placed it
                        _buildableSeen = true;
                        return pResTowerEdict;
                    }
                }
                break;
            case kHive:
				if (pTargetEdict->v.health == 0) {
					return pTargetEdict;
				}
				float closest = 500.0;

                break;
            }
        }
    }

    return NULL;
}


bool BuildNavMethod::findNearbyBuildable()
{
	TargetVector& buildables = _bot.getSensor()->getBuildables();
	for (TargetVector::iterator ii = buildables.begin(); ii != buildables.end(); ii++) {
		if (!ii->getEntity().isNull()) {
			if ((_bot.getEdict()->v.origin - ii->getOrigin()).Length() < CLOSE_ENOUGH_TO_MOVE_TO) {
				_entityLocation = ii->getOrigin();

				float currentBuildState = ii->getEntity().getEdict()->v.fuser1;
				if (currentBuildState > _lastBuildProgress) {
					_lastBuildProgress = currentBuildState;
					_lastBuildProgressTime = gpGlobals->time;
				}
			}
            _buildableSeen = true;
			return true;
		}
	}
    _buildableSeen = false;
	return false;
}



void BuildNavMethod::setImpulse()
{
	switch (_entityTypeToBuild) {
	case kAlienResourceTower:
		_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BUILD_ALIEN_RESOURCE_TOWER);
		break;

	case kHive:
		_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BUILD_HIVE);
		break;

	case kDefenseChamber:
		_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BUILD_ALIEN_DEFENSE_CHAMBER);
		break;

	case kOffenseChamber:
		_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BUILD_ALIEN_OFFENSE_CHAMBER);
		break;

	case kMovementChamber:
		_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BUILD_ALIEN_MOVEMENT_CHAMBER);
		break;

	case kSensoryChamber:
		_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BUILD_ALIEN_SENSORY_CHAMBER);
		break;
	}
}


string BuildNavMethod::getName() const
{
	return "BuildNavigation";
}
