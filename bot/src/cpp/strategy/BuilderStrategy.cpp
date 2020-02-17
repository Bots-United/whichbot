//
// $Id: BuilderStrategy.cpp,v 1.63 2005/02/28 05:20:14 clamatius Exp $

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

#include "BotManager.h"
#include "config/Config.h"
#include "config/TranslationManager.h"
#include "engine/Reward.h"
#include "strategy/BuilderStrategy.h"
#include "strategy/HiveMind.h"
#include "navigation/BuildNavMethod.h"
#include "worldstate/HiveManager.h"
#include "worldstate/WorldStateUtil.h"
#include "extern/metamod/meta_api.h"


static double DISTANCE_SCALING_FACTOR = 0.03;
static double TIME_SCALING_FACTOR = 0.008;

// TODO - config file?
static int MIN_RES_FOR_DEFENSE_CHAMBER = 10;
static int MIN_RES_FOR_MOVEMENT_CHAMBER = 10;
static int MIN_RES_FOR_SENSORY_CHAMBER = 10;
static int MIN_RES_FOR_OFFENSE_CHAMBER = 10;
static int MIN_RES_FOR_RESOURCE_TOWER = 15;
static int MIN_RES_FOR_HIVE = 40;
const float REWARD_TIMEOUT = 300.0f;
static float RESNODE_REWARD = 100.0f;
static float UNBUILT_HIVE_REWARD = 100.0f;
static float HIVE_REWARD = 100.0f;
static float HIVE_DEFENSE_RANGE = 1000.0f;
static float HIVE_DEFENSE_TIMEOUT = 10.0f;
const float RES_WAIT_TIMEOUT = 120.0f;

static float MIN_OC_DISTANCE = 200.0f;
static float MIN_SC_DISTANCE = 500.0f;

static float HIVE_DESIRE = 30.0f;
static float HIVE_DEFENSE_DESIRE = 30.0f;
static float SCALING_FACTOR = 10.0f;
static float MIN_BUILD_WAIT = 10.0f;

const float ARTIFICIAL_DISTANCE_BOOST_THRESHOLD = 20.0;
const float ARTIFICIAL_DISTANCE_BOOST = 10.0;


BuilderStrategy::BuilderStrategy(Bot& bot) :
	_bot(bot),
	_plan(kNoBuildPlan),
	_targetRes(0),
	_timeLastBuildingDropped(0),
	_waitStartTime(-1),
    _hiveDefenseRange(HIVE_DEFENSE_RANGE)
{
	MIN_RES_FOR_DEFENSE_CHAMBER =
		Config::getInstance().getTweak("game/resources/defensechamber", MIN_RES_FOR_DEFENSE_CHAMBER);
	MIN_RES_FOR_SENSORY_CHAMBER =
		Config::getInstance().getTweak("game/resources/sensorychamber", MIN_RES_FOR_SENSORY_CHAMBER);
	MIN_RES_FOR_MOVEMENT_CHAMBER =
		Config::getInstance().getTweak("game/resources/movementchamber", MIN_RES_FOR_MOVEMENT_CHAMBER);
	MIN_RES_FOR_OFFENSE_CHAMBER =
		Config::getInstance().getTweak("game/resources/offensechamber", MIN_RES_FOR_OFFENSE_CHAMBER);
	MIN_RES_FOR_RESOURCE_TOWER =
		Config::getInstance().getTweak("game/resources/alienresourcetower", MIN_RES_FOR_RESOURCE_TOWER);
	MIN_RES_FOR_HIVE =
		Config::getInstance().getTweak("game/resources/team_hive", MIN_RES_FOR_HIVE);

    MIN_OC_DISTANCE =
        Config::getInstance().getTweak("strategy/builder/min_oc_distance", MIN_OC_DISTANCE);
    MIN_SC_DISTANCE =
        Config::getInstance().getTweak("strategy/builder/min_sc_distance", MIN_OC_DISTANCE);

	SCALING_FACTOR =
		Config::getInstance().getTweak("strategy/builder/scale_factor", SCALING_FACTOR);
	DISTANCE_SCALING_FACTOR =
		Config::getInstance().getTweak("strategy/builder/distance_scale", DISTANCE_SCALING_FACTOR);

	RESNODE_REWARD =
		Config::getInstance().getTweak("strategy/builder/resnode_reward", RESNODE_REWARD);
	UNBUILT_HIVE_REWARD =
		Config::getInstance().getTweak("strategy/builder/unbuilt_hive_reward", UNBUILT_HIVE_REWARD);
	HIVE_REWARD =
		Config::getInstance().getTweak("strategy/builder/hive_reward", HIVE_REWARD);

	HIVE_DEFENSE_RANGE =
		Config::getInstance().getTweak("strategy/builder/hive_defense_range", HIVE_DEFENSE_RANGE);
	HIVE_DEFENSE_TIMEOUT =
		Config::getInstance().getTweak("strategy/builder/hive_defense_timeout", HIVE_DEFENSE_TIMEOUT);

	HIVE_DESIRE =
		Config::getInstance().getTweak("strategy/builder/hive_desire", HIVE_DESIRE);
	HIVE_DEFENSE_DESIRE =
		Config::getInstance().getTweak("strategy/builder/hive_defense_desire", HIVE_DEFENSE_DESIRE);

    for (int ii = 0; ii < 3; ii++) {
        _chambers[ii] = (eBuildPlan)(int)Config::getInstance().getArrayTweak("strategy/roles/builder/chamber_order", ii, 0.0);
    }
	if (_chambers[0] == kNoBuildPlan) {
		LOG_CONSOLE(PLID, "Warning: chamber order config incorrect.  Gorge bots will not function correctly.");
	}
}


void BuilderStrategy::countChambers()
{
    _numDefenseChambers = WorldStateUtil::countEntities("defensechamber");
    _numMovementChambers = WorldStateUtil::countEntities("movementchamber");
    _numSensoryChambers = WorldStateUtil::countEntities("sensorychamber");    
}


void BuilderStrategy::getRewards(vector<Reward>& rewards, tEvolution evolution)
{
	_buildableLocations.clear();
    countChambers();

	switch (_plan) {
		case kNoBuildPlan:
		selectBuildPlan();
		if (_plan != kNoBuildPlan) {
			getRewards(rewards, evolution);
		}
		break;

	case kBuildResTower:
        getResTowerRewards(rewards);
		break;

	case kBuildHive:
		rewardHiveLocations(rewards, kEmptyHive);
		break;

	case kBuildOffenseChamber:
        rewardHiveLocations(rewards, kAnyHive);
        rewardResTowerLocations(rewards, ALIEN_NODE_MASK, false);
        break;

	case kBuildDefenseChamber:
	case kBuildMovementChamber:
	case kBuildSensoryChamber:
        tHiveFillState desiredHiveState = kAnyHive;
        if (_numDefenseChambers < 3) {
            desiredHiveState = kFilledHive;
        }
		rewardHiveLocations(rewards, desiredHiveState);
		break;
	}

    
}


void BuilderStrategy::getResTowerRewards(vector<Reward>& rewards)
{
    bool emptyNodeFound = rewardResTowerLocations(rewards, OCCUPIED_NODE_MASK, true);
    if (!emptyNodeFound) {
        if (HiveManager::getActiveHiveCount() > 1) {
            // if we have more than 1 hive, we have bilebomb so let's go kill the marine nodes
            bool marineNodeFound = rewardResTowerLocations(rewards, MARINE_NODE_MASK, false);
            if (!marineNodeFound) {
                // if there aren't any marine nodes
                selectHiveDefensePlan();
                rewardHiveLocations(rewards, kAnyHive);
            }

        } else {
            _plan = kBuildHive;
            rewardHiveLocations(rewards, kEmptyHive);
        }
    }
}


void BuilderStrategy::visitedWaypoint(tNodeId wptId, tEvolution evolution)
{
	if (evolution == kGorge) {
		switch (_plan) {
		case kBuildResTower:
			checkForBuildStart(wptId, MIN_RES_FOR_RESOURCE_TOWER, kAlienResourceTower);
			break;

		case kBuildHive:
			checkForBuildStart(wptId, MIN_RES_FOR_HIVE, kHive);
			break;

		case kBuildDefenseChamber:
			checkForBuildStart(wptId, MIN_RES_FOR_DEFENSE_CHAMBER, kDefenseChamber);
			break;

		case kBuildOffenseChamber:
			checkForBuildStart(wptId, MIN_RES_FOR_OFFENSE_CHAMBER, kOffenseChamber);
			break;

		case kBuildMovementChamber:
			checkForBuildStart(wptId, MIN_RES_FOR_MOVEMENT_CHAMBER, kMovementChamber);
			break;

		case kBuildSensoryChamber:
			checkForBuildStart(wptId, MIN_RES_FOR_SENSORY_CHAMBER, kSensoryChamber);
			break;
		}

	}
}


void BuilderStrategy::waitedAtWaypoint(tNodeId wptId, tEvolution evolution)
{
	if (evolution == kGorge && locationIsBuildable(wptId)) {
		checkBuildStatus();

	} else {
        if (_bot.getNavigationEngine() != NULL) {
            _bot.getNavigationEngine()->resume();
        }
		_waitStartTime = -1;
	}
}


int getResTowerCount()
{
	int resTowerCount = 0;
	CBaseEntity* pEntity = NULL;
	do {
		pEntity = UTIL_FindEntityByClassname(pEntity, "alienresourcetower");
		if ((pEntity != NULL) && !FNullEnt(pEntity->edict())) {
			resTowerCount++;
		}
	} while (pEntity != NULL);

	return resTowerCount;
}


int BuilderStrategy::getChamberCount(int hiveNum)
{
    switch (_chambers[hiveNum]) {
    case kBuildDefenseChamber:
        return _numDefenseChambers;
    case kBuildMovementChamber:
        return _numMovementChambers;
    case kBuildSensoryChamber:
        return _numSensoryChambers;
    default:
        return 0;
    }
}


void BuilderStrategy::selectHiveDefensePlan()
{
	int die = RANDOM_LONG(0, 100);
	int chance = 10;

	_plan = kBuildOffenseChamber;
	int hiveCount = HiveManager::getActiveHiveCount();
	switch (hiveCount) {
	case 0:
		// shouldn't be building hive defense at 0 hives, we can't build anything anyway
		break;
	case 1:
		if (getChamberCount(0) < 3) {
			_plan = _chambers[0];
		}
		break;
	case 2:        
		if (getChamberCount(1) < 3) {
			_plan = _chambers[1];
		}
		break;
	case 3:
        if (getChamberCount(0) < 3) {
            _plan = _chambers[0];

        } else if (getChamberCount(1) < 3) {
            _plan = _chambers[1];

        } else if (getChamberCount(2) < 3) {
            _plan = _chambers[2];
        }
		break;
	}

	if (_plan == kBuildOffenseChamber) {
		_hiveDefenseRange = HIVE_DEFENSE_RANGE * RANDOM_FLOAT(0.3, 1.0);

	} else {
		_hiveDefenseRange = HIVE_DEFENSE_RANGE * RANDOM_FLOAT(0.3, 0.6);
	}
}


void BuilderStrategy::selectBuildPlan()
{
	// TODO - better algorithm than this chimpy one
	int hiveCount = HiveManager::getActiveHiveCount();
	int resTowerCount = getResTowerCount();
	int die = RANDOM_LONG(0, 100);
				
    int totalTraitCount = _numMovementChambers + _numDefenseChambers + _numSensoryChambers;

	switch (hiveCount) {
	case 0:
        _log.Debug("No hives! Selecting hive build plan.");
		_plan = kBuildHive;
		break;
		
	case 1:
		if (_bot.getResources() > 80 && resTowerCount > 1 && totalTraitCount == 3 && !gpBotManager->areCheatsEnabled())
        {
			_plan = kBuildHive;

		} else {
			if (resTowerCount < 3) {
				_plan = kBuildResTower;
				
			} else if ((getChamberCount(0) < 3) &&
                       (getChamberCount(1) == 0) &&
                       (getChamberCount(2) == 0))
			{
				selectHiveDefensePlan();
				
			} else {
				if (resTowerCount < 4) {
					_plan = kBuildResTower;
					
				} else {
					_plan = kBuildHive;
				}
			}
		}
		break;
	
	case 2:
		if (_bot.getResources() > 80 && resTowerCount > 1 && totalTraitCount == 6 && !gpBotManager->areCheatsEnabled()) {
			_plan = kBuildHive;

		} else {
			if ((getChamberCount(2) == 0) &&
				((getChamberCount(0) < 3) ||
 				 (getChamberCount(1) < 3)))
			{
				selectHiveDefensePlan();
				
			} else {
				if (resTowerCount < 2) {
					_plan = kBuildResTower;
					
				} else if (die < HIVE_DESIRE) {
					_plan = kBuildHive;
					
				} else {
					selectHiveDefensePlan();
				}
			}
		}
		break;
		
	case 3:
		if (die > 20 * resTowerCount || die > 95) {
			_plan = kBuildResTower;
			
		} else {
			selectHiveDefensePlan();
		}
		break;
	}

	switch (_plan) {
	case kBuildResTower:
		_log.Debug("Selected res tower build plan");
		break;
	case kBuildHive:
		_log.Debug("Selected hive build plan");
		break;
	case kBuildDefenseChamber:
		_log.Debug("Selected defense chamber build plan");
		break;
	case kBuildMovementChamber:
		_log.Debug("Selected movement chamber build build plan");
		break;
	case kBuildSensoryChamber:
		_log.Debug("Selected sensory chamber build build plan");
		break;
	case kBuildOffenseChamber:
		_log.Debug("Selected offense chamber build plan");
		break;
	}
}


bool BuilderStrategy::rewardResTowerLocations(vector<Reward>& rewards, int nodeMask, bool rewardEmpty)
{
	const char* resLocClassname = "func_resource";

    bool locationFound = false;

	CBaseEntity* pEntity = NULL;
	do {
		pEntity = UTIL_FindEntityByClassname(pEntity, resLocClassname);
		if (pEntity != NULL) {
            edict_t* pEdict = pEntity->edict();
            bool resNodeTaken = HiveMind::resourceNodeIsTaken(pEdict->v.origin, nodeMask);
            
            if ((rewardEmpty && !resNodeTaken) || (!rewardEmpty && resNodeTaken)) {
                 tNodeId wptId = HiveMind::getNearestWaypoint(kGorge, pEdict);
 
                 if (_bot.getPathManager().nodeIdValid(wptId)) {
                     addReward(rewards, wptId, RESNODE_REWARD);
                     locationFound = true;
                 }
            }
        }

    } while (pEntity != NULL);
    return locationFound;
}


void BuilderStrategy::rewardHiveLocations(vector<Reward>& rewards, tHiveFillState desiredHiveState)
{
    int numRewardsGiven = 0;
	for (int ii = 0; ii < HiveManager::getHiveCount(); ii++) {
		HiveInfo* pHiveInfo = HiveManager::getHive(ii);
		if (pHiveInfo->getWaypointId() >= 0) {

            if ((desiredHiveState == kAnyHive) ||
                (desiredHiveState == pHiveInfo->getHiveFillState()))
            {
    			addReward(rewards, pHiveInfo->getWaypointId(), HIVE_REWARD);
                numRewardsGiven++;
            }
		}
	}

    if (numRewardsGiven == 0) {
        _log.Debug("Wanted to reward hive locations, but no empty hives found!");
        _plan = kNoBuildPlan;
    }
}


void BuilderStrategy::checkForBuildStart(tNodeId wptId, int targetRes, eEntityType entityType)
{
	if (locationIsBuildable(wptId)) {
		_targetRes = targetRes;
		if (_bot.getResources() < _targetRes) {
			_log.Debug("Reached buildable wpt %d, waiting for more res", wptId);
            if (_bot.getNavigationEngine() != NULL) {
                _bot.getNavigationEngine()->pause();
            }
			_waitStartTime = gpGlobals->time;

		} else {
			// Don't switch to build mode too often, it can cause some pathological sticking issues where the
			// bot can't actually build but thinks it can and continuously switches in and out of build mode
			if (gpGlobals->time > _bot.getProperty(Bot::kLastBuildTime) + MIN_BUILD_WAIT) {
                
                _log.Debug("Reached buildable wpt %d, switching to build mode", wptId);
                _bot.setProperty(Bot::kLastBuildTime);
                if (_bot.getNavigationEngine() != NULL) {
                    _bot.getNavigationEngine()->setNextMethod(new BuildNavMethod(_bot, entityType));
                }
                _timeLastBuildingDropped = gpGlobals->time;
				_plan = kNoBuildPlan;
			}
		}
	}
}


void BuilderStrategy::checkBuildStatus()
{
    if (_bot.getNavigationEngine() != NULL) {
        if (_bot.getResources() >= _targetRes) {
            _log.Debug("Resources have reached target, switching to build mode");
            _bot.getNavigationEngine()->resume();
            _waitStartTime = -1;
            
        } else if (gpGlobals->time > _waitStartTime + RES_WAIT_TIMEOUT) {
            _log.Debug("Moving around to keep our feet warm");
            _bot.getNavigationEngine()->resume();
            _waitStartTime = -1;
        }
    }
}


void BuilderStrategy::addReward(vector<Reward>& rewards, tNodeId wptId, float rewardVal)
{
	if (_bot.getPathManager().nodeIdValid(wptId)) {
		float distance = _bot.getPathManager().getDistance(wptId);
		// let's not add it if it's not in the optimised tree search space
		
		if (!_bot.hasFailedToBuildAt(wptId) && distance < MAX_DISTANCE_ESTIMATE) {
			float timeDiff = gpGlobals->time - HiveMind::getVisitTime(wptId);
			float scale = (REWARD_TIMEOUT + timeDiff) / REWARD_TIMEOUT;
			
			// let's add an exponentially increasing desire to build so that we always keep building and don't
			// camp in a hive (e.g. if we're overly scared of marine buildings)
			float lastBuildTimeDiff = gpGlobals->time - _timeLastBuildingDropped;
			if (lastBuildTimeDiff > 0) {
				scale *= (1 + exp(TIME_SCALING_FACTOR * (lastBuildTimeDiff * - 60)));
			}
			
			float scaledRewardVal = SCALING_FACTOR * rewardVal * exp((-DISTANCE_SCALING_FACTOR) * distance) * scale * scale;
			
			// Add a strong imperative to go to things that are very close to us
			if (distance < ARTIFICIAL_DISTANCE_BOOST_THRESHOLD) {
				scaledRewardVal *= ARTIFICIAL_DISTANCE_BOOST;
			}
			
			Reward reward(wptId, scaledRewardVal, TranslationManager::getTranslation(getBuildPlanName()));
			rewards.push_back(reward);
			//_log.Debug("Rewarding waypoint %d with reward %f", wptId, scaledRewardVal);
			
			_buildableLocations.push_back(wptId);
			//WaypointDebugger::drawDebugBeam(_bot.getEdict()->v.origin, gpBotManager->getWaypointManager().getOrigin(wptId), 255, 0, 0);
		}
	}
}


string BuilderStrategy::getBuildPlanName() const
{
	switch (_plan) {
	case kNoBuildPlan: return "BuildNothing";
	case kBuildResTower: return "BuildResTower";
	case kBuildHive: return "BuildHive";
	case kBuildOffenseChamber: return "BuildOffenseChamber";
	case kBuildDefenseChamber: return "BuildDefenseChamber";
	case kBuildMovementChamber: return "BuildMovementChamber";
	case kBuildSensoryChamber: return "BuildSensoryChamber";
	default: return "BuildUnknown";
	}
}


bool BuilderStrategy::locationIsBuildable(tNodeId wptId)
{
    if (_bot.hasFailedToBuildAt(wptId)) {
        return false;
    }
	switch (_plan) {
	case kBuildHive:
	case kBuildResTower:
		return find(_buildableLocations.begin(), _buildableLocations.end(), wptId) != _buildableLocations.end();
	case kBuildDefenseChamber:
	case kBuildSensoryChamber:
	case kBuildMovementChamber:
	case kBuildOffenseChamber:
		bool locationOk = false;
		if ((gpGlobals->time > _timeLastBuildingDropped + HIVE_DEFENSE_TIMEOUT) && 
			((find(_buildableLocations.begin(), _buildableLocations.end(), wptId) != _buildableLocations.end()) || 
			(HiveManager::distanceToNearestHive(_bot.getEdict()->v.origin) < _hiveDefenseRange)))
		{
			// Let's not build underwater
			if (UTIL_PointContents(_bot.getEdict()->v.origin) == CONTENTS_WATER) {
				return false;
			}

			// Let's not block ladders
			edict_t* pLadder = WorldStateUtil::findClosestEntity("func_ladder", _bot.getEdict()->v.origin);
			if (!FNullEnt(pLadder)) {
				float range = (_bot.getEdict()->v.origin.Make2D() - pLadder->v.origin.Make2D()).Length();
				return (range > 200);

			} else {
				return true;
			}
		}
	}
	return false;
}


bool BuilderStrategy::otherChamberTooClose(tNodeId wptId)
{
    edict_t* pClosestChamber = NULL;
    float allowedRange = 10000.0;

    switch (_plan) {
    case kBuildOffenseChamber:
        pClosestChamber = WorldStateUtil::findClosestEntity("offensechamber", _bot.getEdict()->v.origin);
        allowedRange = MIN_OC_DISTANCE;
        break;
    case kBuildSensoryChamber:
        pClosestChamber = WorldStateUtil::findClosestEntity("sensorychamber", _bot.getEdict()->v.origin);
        allowedRange = MIN_SC_DISTANCE;
        break;
    }
    if (pClosestChamber != NULL) {
        float range = (_bot.getEdict()->v.origin - pClosestChamber->v.origin).Length();
        if (range < allowedRange) {
            return true;
        }
    }
    return false;
}


Log BuilderStrategy::_log("strategy/BuilderStrategy");
