//
// $Id: BotMovement.cpp,v 1.31 2005/12/14 23:09:44 clamatius Exp $

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

#include "navigation/BotMovement.h"
#include "sensory/BotSensor.h"
#include "NSConstants.h"
#include "Bot.h"
#include "config/Config.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/cbase.h"
#include <math.h>
#include "navigation/WaypointDebugger.h"
#include "worldstate/WorldStateUtil.h"
#include "strategy/FleeStrategy.h"

Log BotMovement::_log(__FILE__);

const float JUMP_DISTANCE = 50.0;
const float LATERAL_LOOKAHEAD_DISTANCE = 70.0;
const float STRAFE_SPEED = 350.0;
const float WHISKER_LENGTH = 75.0;
const float MAX_WHISKER_DEVIATION = 90;
const float LADDER_SEARCH_RANGE = 50;
const float MIN_FLAP_SPEED = 350.0;
const float FLAP_TIME = 0.05;
const float MIN_FLYING_ENERGY = 60;
const float MIN_BLINK_ENERGY = 60;
const float BLINK_ENERGY_COST = 50;

// HIT_BOUNDARY * WHISKER_LENGTH is the cutoff for deciding if we "hit" something
const float HIT_BOUNDARY = 0.25; 

// Distance from origin to one side of creature.
// static const float creatureOriginWidths[] = {10, 15, 15, 10, 40};

// Distance from origin to head
// static const float creatureOriginLengths[] = {40, 40, 40, 10, 100};

const float DEFAULT_MIN_TARGET_RADIUS = 40.0;


BotMovement::BotMovement(Bot& bot) :
	_bot(bot),
	_speed(0.0, 0.0, 0.0),
	_maxSpeed(1500.0),
	_lastFrameTime(0),
    _lastRandomPickTime(0),
	_pLastHit(NULL),
	_lastHitTime(0),
	_stuckTimeStart(0.0),
	_backPedalStartTime(0.0),
	_arrivedAtTargetVector(false),
	_shouldUseMeleeAttack(false),
	_minTargetRadius(DEFAULT_MIN_TARGET_RADIUS),
	_noJump(false),
    _lastFlapStart(0.0)
{
	// Hit the ground running
	_bot.getEdict()->v.yaw_speed = BOT_YAW_SPEED;
	_bot.getEdict()->v.pitch_speed = BOT_PITCH_SPEED;
	_speed.z = _maxSpeed;
}


BotMovement::~BotMovement()
{
}


/**
 * What this function is doing is guessing at how long the next frame will take to render.
 * The _msecval parameter to pfnRunPlayerMove controls how long that action applies.
 * Since we want to update every frame, we want the msecval to be the time before we get
 * called again.
 * In other words, if we are getting 10fps, _msecval should be 100.
 */
byte BotMovement::getMillisecondDelay()
{
	byte msecVal = 1;

	if (_lastFrameTime != 0) {
		float timeDiffMs = 1000 * (gpGlobals->time - _lastFrameTime);
		if (timeDiffMs <= 255) {
			msecVal = (byte)timeDiffMs;

		} else {
            // not good, means the machine is running too slowly
			msecVal = 255;
		}
	}

	_lastFrameTime = gpGlobals->time;
	return msecVal;
}


float toForwardArc(float angle)
{
	float fixedAngle = angle;
    // check for wrap around of angle...
    if (angle > 180) {
        fixedAngle -= 360;
    }
    
    if (angle < -180) {
        fixedAngle += 360;
    }
	return fixedAngle;
}


// Returns the optimal angular rate of change.
float changeAngle(float ideal, float speed, float& angle)
{
    float current_180;	// current +/- 180 degrees
    
    // turn from the current v_angle pitch to the idealpitch by selecting
    // the quickest way to turn to face that direction
    
    float current = angle;
        
    // find the difference in the current and ideal angle
    float diff = fabs(current - ideal);
    
    // check if the bot is already facing the ideal direction...
    if (diff <= 1) {
		angle = ideal;
        return diff;
	}
    
    // check if difference is less than the max degrees per turn
    if (diff < speed) {
        speed = diff;  // just need to turn a little bit (less than max)
    }

    // here we have four cases, both angle positive, one positive and
    // the other negative, one negative and the other positive, or
    // both negative.  handle each case separately...
    
    if ((current >= 0) && (ideal >= 0)) { // both positive
        if (current > ideal) {
            current -= speed;
        } else {
            current += speed;
        }
    }
    else if ((current >= 0) && (ideal < 0))
    {
        current_180 = current - 180;
        
        if (current_180 > ideal) {
            current += speed;
        } else {
            current -= speed;
        }
    }
    else if ((current < 0) && (ideal >= 0))
    {
        current_180 = current + 180;
        if (current_180 > ideal) {
            current += speed;
        } else {
            current -= speed;
        }
    }
    else  // (current < 0) && (ideal < 0)  both negative
    {
        if (current > ideal) {
            current -= speed;
        } else {
            current += speed;
        }
    }
    
    angle = toForwardArc(current);
    
    return speed;
}


void BotMovement::setLadderTarget(Vector& targetPoint, bool goingUp, float minDistance)
{
	setTarget(targetPoint, minDistance, goingUp);
}


void BotMovement::moveDirectlyTowardsTarget(const Vector& targetVector, tEvolution evolution)
{
	if (_bot.getEdict()->v.origin != targetVector) {
		// note that ideal pitch and yaw are relative to current orientation
		float newPitchDeg = 0.0f;
		float newYawDeg = 0.0f;

		getTargetAnglesDeg(targetVector, newPitchDeg, newYawDeg);

		if (fabs(newYawDeg) > fabs(newPitchDeg)) {
			// 180.0 is the max possible deflection
			setSpeed(targetVector, fabs(_bot.getEdict()->v.ideal_yaw - newYawDeg), 180.0, evolution);
		} else {
			// 180.0 is the max possible deflection
			setSpeed(targetVector, fabs(_bot.getEdict()->v.idealpitch - newPitchDeg), 180.0, evolution);
		}

        switch (evolution) {
        case kSkulk:
			_bot.getEdict()->v.idealpitch = toForwardArc(newPitchDeg);
            break;
        case kFade:
			_bot.getEdict()->v.idealpitch = toForwardArc(newPitchDeg);
            break;
        case kLerk:
            // tweak upwards slightly
            newPitchDeg += 15;
   			_bot.getEdict()->v.idealpitch = toForwardArc(newPitchDeg);
            break;
        default:
			_bot.getEdict()->v.idealpitch = 0;
			_bot.getEdict()->v.v_angle.x = toForwardArc(newPitchDeg);
            break;
        }

		_bot.getEdict()->v.ideal_yaw = newYawDeg;
	}
}


void BotMovement::setRandomTarget()
{	
    if (gpGlobals->time > _lastRandomPickTime + 2.0) {
		_targetVector = _bot.getEdict()->v.origin + Vector(RANDOM_LONG(-50, 50), RANDOM_LONG(-50, 50), _bot.getEdict()->v.origin.z);
        _lastRandomPickTime = gpGlobals->time;
		_arrivedAtTargetVector = false;
    }
}


void BotMovement::setTarget(Vector& targetVector, float minTargetRadius, bool noJump)
{
    // no strafing unless we really want to do it
    _noJump = noJump;
    _speed.x = 0.0;
	_targetVector = targetVector;
	_minTargetRadius = minTargetRadius;
	_arrivedAtTargetVector = false;
}


float toRad(float degrees)
{
	return M_PI * degrees / 180.0;
}


float toDegrees(float rad)
{
	return rad / M_PI * 180.0;
}


// If we had a really long whisker, can we feel our target?
bool BotMovement::canWeFeelTheTarget(const Vector& targetVector, tEvolution evolution, TraceResult& collisionTr)
{
	const float whiskerLength = getDistanceToTarget(targetVector, evolution);
	
	float targetPitchDeg, targetYawDeg;
	getTargetAnglesDeg(targetVector, targetPitchDeg, targetYawDeg);

	float targetPitchRad = toRad(targetPitchDeg + 90);
	float targetYawRad = toRad(targetYawDeg);

	// We don't want to use the ground level origin here for obvious reasons.
	collisionTr = traceFatWhisker(_bot.getEdict()->v.origin, targetPitchRad, targetYawRad, whiskerLength, 0, 0, 0);

	float length = collisionTr.flFraction * whiskerLength;
	Vector vecToHitOrigin = calculateVector(_bot.getEdict()->v.origin, targetPitchRad, targetYawRad, length);
	Vector vecToTarget = vecToHitOrigin - targetVector;

	//_log.Debug("hit origin to target length = %f", vecToTarget.Length());

	if (vecToTarget.Length() < _minTargetRadius) {
		//_log.Debug("We can head towards target again.");
		return true;
	} else {
		//_log.Debug("Still can't feel our target");
		return false;
	}
}


float BotMovement::getHitHeight (const TraceResult& upperFrontWhiskerTr, const TraceResult& lowerFrontWhiskerTr, float whiskerPitchAngleRad, float whiskerLength)
{
	float upperHitLength = upperFrontWhiskerTr.flFraction * whiskerLength;
	float lowerHitLength = lowerFrontWhiskerTr.flFraction * whiskerLength;
	float s = sin(whiskerPitchAngleRad);
	return (s * upperHitLength) + (s * lowerHitLength);
}


float BotMovement::getRelativeHitAngleDeg (const Vector& vec)
{
	float pitchRad = toRad(_bot.getEdict()->v.idealpitch + 90);
	float yawRad = toRad(_bot.getEdict()->v.ideal_yaw);
	Vector botVec(sin(pitchRad)*cos(yawRad), sin(pitchRad)*sin(yawRad), cos(pitchRad));
	botVec = botVec.Normalize();
	Vector normalizedVec = vec.Normalize();
	//Vector botVec = calculateVector(_bot.getEdict()->v.origin, 0, 0, WHISKER_LENGTH);
	float dotProduct = DotProduct(normalizedVec, botVec);
    float hitAngleRadians = acos(dotProduct);

    // Apparently sometimes acos(-1) == NaN (presumably for small values of -1).  Isn't floating point
    // math amazing?
    if (!WorldStateUtil::isFinite(hitAngleRadians)) {
        hitAngleRadians = -179.0;
    }

	return toDegrees(hitAngleRadians);
}


void BotMovement::moveToTarget (const Vector& targetVector, tEvolution evolution)
{
	float piOver2 = M_PI/2;
	float piOver4 = M_PI/4;
	float piOver6 = M_PI/6;

	// Our approximate creature height;
	float creatureHeight = g_CreatureOriginHeights[evolution] * 2;

	// z is normally straight up.  HL's z for pitch is offset by 90 degrees (horizontal)
	// Negative idealpitch is upwards.
	float pitchRad = toRad(_bot.getEdict()->v.idealpitch + 90);
	float yawRad = toRad(_bot.getEdict()->v.ideal_yaw);

	float targetPitchDeg, targetYawDeg;
	getTargetAnglesDeg(targetVector, targetPitchDeg, targetYawDeg);

	// Note that pitch=0 is horizontal.
	float relativeTargetPitchDeg = targetPitchDeg - _bot.getEdict()->v.idealpitch;
	float relativeTargetYawDeg = targetYawDeg - _bot.getEdict()->v.ideal_yaw;

	//_log.Debug("relativeTargetPitchDeg=%f, relativeTargetYawDeg=%f", relativeTargetPitchDeg, relativeTargetYawDeg);

	// Whisker lengths should be a function of the creature's dimensions (origin height may do)
	float lrWhiskerLength = creatureHeight * 2;
	if (evolution == kOnos) {
		// the approximation doesn't seem to hold for Onos because they're really long
		lrWhiskerLength *= 4;
	}
	TraceResult leftWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad, yawRad+piOver4, lrWhiskerLength, 0, 0, 0);
	TraceResult rightWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad, yawRad-piOver4, lrWhiskerLength, 0, 0, 0);

	float upperLowerFrontWhiskerLength = WHISKER_LENGTH;
	float upperLowerFrontWhiskerPitchAngleRad = piOver6;
	TraceResult upperFrontWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad-upperLowerFrontWhiskerPitchAngleRad, yawRad, upperLowerFrontWhiskerLength, 0, 0, 0);
	TraceResult lowerFrontWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad+upperLowerFrontWhiskerPitchAngleRad, yawRad, upperLowerFrontWhiskerLength, 0, 0, 0);
	TraceResult centerFrontWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad, yawRad, WHISKER_LENGTH, 0, 0, 0);

	float topBottomWhiskerLength = creatureHeight;
	// TraceResult bottomWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad+piOver2, yawRad, topBottomWhiskerLength, 0, 0, 0);
	TraceResult topWhiskerTr = traceSkinnyWhisker(_bot.getEdict()->v.origin, pitchRad-piOver2, yawRad, topBottomWhiskerLength, 0, 0, 0);

	float correctionYawDeg = 0.0f;
	float correctionPitchDeg = 0.0f;
    TraceResult frontCollision;
	bool canHeadToTarget = canWeFeelTheTarget(targetVector, evolution, frontCollision);

	// Calculate yaw correction
	if (!canHeadToTarget) {
		// If the left and right whiskers are in contact with something and we can't "feel" our target yet, angle the
		// bot so that it will hug the object and move around it.  1/2 of each whisker is for left yaw correction,
		// the other half is for right yaw correction.  We want the max angle the bot should turn to hug the wall to
		// be less than the possible deflection so not to head directly into the object it's feeling its way around.
		if ((leftWhiskerTr.flFraction == rightWhiskerTr.flFraction) && (leftWhiskerTr.flFraction != 1.0f) && (centerFrontWhiskerTr.flFraction != 1.0f)) {
			// Arbitrarily go left if we're headed for a corner.
			correctionYawDeg = 90.0f;
		} else {
			if (!isTargetOverhead(relativeTargetPitchDeg)) {
				//_log.Debug("left=%f, right=%f", leftWhiskerTr.flFraction, rightWhiskerTr.flFraction);
				if (leftWhiskerTr.flFraction != 1.0) {
					correctionYawDeg = ((leftWhiskerTr.flFraction - 0.5)/0.5);
				}
				if (rightWhiskerTr.flFraction != 1.0) {
					correctionYawDeg += ((0.5 - rightWhiskerTr.flFraction)/0.5);
				}
			}
		}
	}

	// Calculate pitch correction
	if (!isCloseToTarget(targetVector, evolution)) {
		float frontClearanceHeight = getHitHeight(upperFrontWhiskerTr, lowerFrontWhiskerTr, upperLowerFrontWhiskerPitchAngleRad, upperLowerFrontWhiskerLength);

		if (evolution == kOnos && upperFrontWhiskerTr.flFraction < 0.5) {
			_bot.getEdict()->v.button |= IN_DUCK;
		}
		if (frontClearanceHeight < g_CreatureOriginHeights[evolution]) {
			if (topWhiskerTr.flFraction == 1.0) {
				// We can't fit through the opening or we're about to run into a wall, let's pitch up
				correctionPitchDeg = (upperFrontWhiskerTr.flFraction - 1) * MAX_WHISKER_DEVIATION;
			} else {
				// No room on top to pitch up, let's arbitrarily pitch down and hope for the best.
				correctionPitchDeg = (1 - upperFrontWhiskerTr.flFraction) * MAX_WHISKER_DEVIATION;
			}
		} else if (lowerFrontWhiskerTr.flFraction != 1.0) {
			// Determine our ideal pitch while on a relatively flat surface.
			float relativePitch = getRelativeHitAngleDeg(lowerFrontWhiskerTr.vecPlaneNormal) - 90;
			if (relativeTargetPitchDeg < 10) {
				// Target is slightly below or above us.  Let's pitch up.
				correctionPitchDeg = -relativePitch;
			} else {
				// Target is below us.  Let's pitch down.
				correctionPitchDeg = relativePitch;
			}
		} else {
			// We're probably suspended in midair due to an overshoot, let's aim for the target
			// vector.
			correctionPitchDeg = relativeTargetPitchDeg;
		}

		// So if we can't walk on the walls, try jumping over the obstacle
		if ((correctionPitchDeg < -45) && (relativeTargetPitchDeg > -45) && (relativeTargetPitchDeg < 30) && !_noJump) {
			if (evolution != kSkulk) {
				// Something is in our way, try jumping over it.
                // Check to make sure that we're not climbing a ladder
                if (!WorldStateUtil::isOnLadder(_bot.getEdict()) || relativeTargetPitchDeg <= 0) {
    				_bot.getEdict()->v.button |= IN_JUMP;
                }
			}
		}
	}

	//_log.Debug("whisker:  correctionYawDeg=%f, correctionPitchDeg=%f", correctionYawDeg, correctionPitchDeg);
	// _log.Debug("before:  idealpitch=%f, ideal_yaw=%f", _bot.getEdict()->v.idealpitch, _bot.getEdict()->v.ideal_yaw);

	float fabsCorrectionYawDeg = fabs(correctionYawDeg);
	float fabsCorrectionPitchDeg = fabs(correctionPitchDeg);

	// Set the yaw for this tick
	if (fabsCorrectionYawDeg < 0.1) {
		_bot.getEdict()->v.ideal_yaw = toForwardArc(targetYawDeg);
	} else {
		_bot.getEdict()->v.ideal_yaw = toForwardArc(_bot.getEdict()->v.ideal_yaw + correctionYawDeg);
	}

	// Set the pitch for this tick
	if (evolution == kSkulk || evolution == kLerk || evolution == kFade) {
		_bot.getEdict()->v.idealpitch = toForwardArc(_bot.getEdict()->v.idealpitch + correctionPitchDeg);

    } else {
		// Make the creature's view angle's x-component match where the target is since the
		// bot can't wall walk
		_bot.getEdict()->v.idealpitch = 0;
		if (relativeTargetPitchDeg < -60) {
			_bot.getEdict()->v.v_angle.x = -60;	
		} else {
			_bot.getEdict()->v.v_angle.x = toForwardArc(relativeTargetPitchDeg);
		}
	}

	// Make the speed dependent on how fast the yaw/pitch angle is changing.
	if (fabsCorrectionYawDeg > fabsCorrectionPitchDeg) {
		setSpeed(targetVector, fabsCorrectionYawDeg, 180.0, evolution);
	} else {
		setSpeed(targetVector, fabsCorrectionPitchDeg, 180.0, evolution);
	}

	if (frontCollision.flFraction != 1.0) {
        float range = frontCollision.flFraction * (targetVector - _bot.getEdict()->v.origin).Length();
        if (range < 120.0) {
    		reactToCollision(frontCollision);
        }
	}

	// _log.Debug("after:  idealpitch=%f, ideal_yaw=%f", _bot.getEdict()->v.idealpitch, _bot.getEdict()->v.ideal_yaw);
}


Vector BotMovement::calculateVector (const Vector& origin, float pitchRad, float yawRad, float length) const
{
	Vector vec(sin(pitchRad)*cos(yawRad), sin(pitchRad)*sin(yawRad), cos(pitchRad));
	vec = vec.Normalize();
	vec *= length;
	return origin + vec;
}


TraceResult BotMovement::traceSkinnyWhisker (Vector& whiskerOrigin, float pitchRad, float yawRad, float whiskerLength,
											   int red, int green, int blue) const
{
	IGNORE_MONSTERS ignore = dont_ignore_monsters;
	Vector whiskerTarget = calculateVector(whiskerOrigin, pitchRad, yawRad, whiskerLength);
	TraceResult tr;
	WorldStateUtil::checkVector(whiskerOrigin);
	WorldStateUtil::checkVector(whiskerTarget);
 	UTIL_TraceLine(whiskerOrigin, whiskerTarget, ignore, _bot.getEdict()->v.pContainingEntity, &tr);

	// Note that the beams are drawn before the bot's move is processed so you'll see a slight delay before the
	// orientation of the bot catches up with the direction of the beams.
	if ((red + green + blue) != 0) {
		WaypointDebugger::drawDebugBeam(whiskerOrigin, whiskerTarget, red, green, blue);
	}
	
	return tr;
}


TraceResult BotMovement::traceFatWhisker (Vector& whiskerOrigin, float pitchRad, float yawRad, float whiskerLength,
											int red, int green, int blue) const
{
	IGNORE_MONSTERS ignore = dont_ignore_monsters;
	Vector whiskerTarget = calculateVector(whiskerOrigin, pitchRad, yawRad, whiskerLength);
	TraceResult tr;
	WorldStateUtil::checkVector(whiskerOrigin);
	WorldStateUtil::checkVector(whiskerTarget);
 	UTIL_TraceHull(whiskerOrigin, whiskerTarget, ignore, head_hull, _bot.getEdict()->v.pContainingEntity, &tr);

	// Note that the beams are drawn before the bot's move is processed so you'll see a slight delay before the
	// orientation of the bot catches up with the direction of the beams.
	if ((red + green + blue) != 0) {
		WaypointDebugger::drawDebugBeam(whiskerOrigin, whiskerTarget, red, green, blue);
	}

	return tr;
}


void BotMovement::setSpeed (const Vector& targetVector, float fabsDeltaAngleDeg, float maxDeltaAngleDeg,
							  tEvolution evolution)
{
	// More we turn, the slower we go.
	assert(maxDeltaAngleDeg != 0);
    if (maxDeltaAngleDeg == 0) {
        maxDeltaAngleDeg = 1.0;
    }
    WorldStateUtil::checkVector(targetVector);
    assert(WorldStateUtil::isFinite(fabsDeltaAngleDeg));
    assert(WorldStateUtil::isFinite(maxDeltaAngleDeg));


	float turnFactor = (1.0 - (fabsDeltaAngleDeg/maxDeltaAngleDeg));
    assert(WorldStateUtil::isFinite(turnFactor));


	// Slow down once we start getting close to the target vector.
	assert(_minTargetRadius != 0);
	float distanceFactor = getDistanceToTarget(targetVector, evolution) / (_minTargetRadius * 5.0);

    assert(WorldStateUtil::isFinite(distanceFactor));
	if (distanceFactor > 1.0) {
		distanceFactor = 1.0f;

    } else if (distanceFactor < 0.0) {
		distanceFactor = 0.0f;
	}

    assert(WorldStateUtil::isFinite(_maxSpeed));

	_speed.z = _maxSpeed * turnFactor * distanceFactor;
    assert(WorldStateUtil::isFinite(_speed.z));

    assert(_speed.z <= _maxSpeed);
    if (_speed.z > _maxSpeed) {
        _speed.z = _maxSpeed;
    }
    WorldStateUtil::checkVector(_speed);
}


bool BotMovement::isStuck () const
{
	return (_lastBotOrigin == _bot.getEdict()->v.origin) ? true : false;
}


bool BotMovement::isCloseToTarget (const Vector& targetVector, tEvolution evolution) const
{
	return (getDistanceToTarget(targetVector, evolution) < _minTargetRadius);
}


bool BotMovement::isTargetOverhead (float relativeTargetPitchDeg) const
{
	// Note that the angle is negative if we're pitching upwards.
	return ((relativeTargetPitchDeg < -40.0) && (relativeTargetPitchDeg > -135.0));
}


// Note that angles returned are in degrees.
void BotMovement::getTargetAnglesDeg (const Vector& targetVector, float& pitchDeg, float& yawDeg) const
{
	Vector relativeVector = targetVector - _bot.getEdict()->v.origin;
    Vector anglesToPoint(1, 0, 0);
    if (relativeVector.Length() > 1) {
    	anglesToPoint = UTIL_VecToAngles(relativeVector);
    }

	pitchDeg = toForwardArc(-anglesToPoint.x);
    yawDeg = toForwardArc(anglesToPoint.y);
	WorldStateUtil::checkAngle(pitchDeg);
	WorldStateUtil::checkAngle(yawDeg);
}


float BotMovement::getDistanceToTarget (tEvolution evolution) const
{
	return getDistanceToTarget(_targetVector, evolution);
}


void BotMovement::reactToCollision (TraceResult& tr)
{
    if (tr.pHit != NULL) {
        _pLastHit = tr.pHit;
        _lastHitTime = gpGlobals->time;

        // hmm, something there.  if it's an alien building, let's try to hop over it.
        const char* classname = STRING(tr.pHit->v.classname);
        
		// _log.Debug("We've run into a %s with the %s whisker.", classname, whiskerName);
        
		EntityInfo* info = Config::getInstance().getEntityInfo(classname);
        if (info != NULL) {
            switch (info->getTeam(tr.pHit)) {
            case MARINE_TEAM:
				_shouldUseMeleeAttack = true;
                break;

            default:
				break;
            }
        } else {
			// if it's breakable, let's chomp/slash through it			
			if (strcmp(classname, "func_breakable") == 0) {
				_shouldUseMeleeAttack = true;
			}

		}
    }
}


void BotMovement::checkForPassives(tEvolution evolution)
{
    // FIXME: this doesn't really belong here now we're doing self-heal as well as blink
    if (evolution == kFade) {
        if ((_bot.getEnergy() >= MIN_BLINK_ENERGY || FleeStrategy::botIsScared(_bot)) && ((_bot.getEdict()->v.button & IN_ATTACK) == 0)) {
			Vector relativeVector(_targetVector - _bot.getEdict()->v.origin);
			// don't blink if we're in swipe range
			if (relativeVector.Length() > 100) {
                _bot.getEdict()->v.button |= IN_DUCK;
	            if (_bot.selectWeapon(WEAPON_BLINK)) {
		            _bot.fireWeapon();
					_bot.getEdict()->v.impulse = Config::getInstance().getImpulse(IMPULSE_BLINK);
					_bot.getEdict()->v.fuser3 -= BLINK_ENERGY_COST;

				} else if (_bot.isInjured() && _bot.selectWeapon(WEAPON_METABOLIZE)) {
					_bot.fireWeapon();
				}
			}
        }

    } else if (evolution == kGorge) {
        if (_bot.isInjured() && _bot.selectWeapon(WEAPON_HEALINGSPRAY)) {
            _bot.fireWeapon();
        }
    }
}


// A lerk flap actually consists of releasing JUMP for a short period - the rest of the time, we hold JUMP
// down so we glide.
void BotMovement::checkLerkFlap()
{
    if (_bot.getEvolution() == kLerk) {
        bool haveEnoughEnergyToFlap = (_bot.getEnergy() > MIN_FLYING_ENERGY) ||
            FleeStrategy::botIsScared(_bot) ||
            ((_bot.getEnergy() > MIN_FLYING_ENERGY / 2) && 
            (_bot.getSensor()->threatSeen() || (_targetVector.z / _targetVector.Length() >= 0.5)));

        if (haveEnoughEnergyToFlap && _speed.z != 0) {
            // firstly, if the normal engine thinks we should flap, let's flap
            // secondly, if we're going too slowly, we're probably not flying
            bool amSupposedToJump = (_bot.getEdict()->v.button & IN_JUMP) != 0;
            bool doFlap = amSupposedToJump || (_bot.getEdict()->v.velocity.Length() < MIN_FLAP_SPEED);
            
            float timeSinceLastFlap = gpGlobals->time - _lastFlapStart;
            
            // Ok, this is a new flap if it's the first this game or we're past our 
            bool newFlap = (timeSinceLastFlap < 0) || (timeSinceLastFlap > FLAP_TIME * 2);
            if (doFlap && (newFlap || timeSinceLastFlap < FLAP_TIME))
            {
                // we're not jumping, mask off jump
                _bot.getEdict()->v.button &= ~IN_JUMP;
                if (newFlap) {
                    _lastFlapStart = gpGlobals->time;
                }
                
            } else {
                // Ok, we're gliding
                _bot.getEdict()->v.button |= IN_JUMP;
            }

        } else {
            // if we don't have enough energy for flapping, just glide
            _bot.getEdict()->v.button |= IN_JUMP;
        }
    }
}


void BotMovement::move(tEvolution evolution)
{
    //WaypointDebugger::drawDebugBeam(_bot.getEdict()->v.origin, _targetVector, 0, 255, 0);
    if (isCloseToTarget(_targetVector, evolution) && !WorldStateUtil::isOnLadder(_bot.getEdict())) {
		// We're close enough.  Let's stop moving.
		moveDirectlyTowardsTarget(_targetVector, evolution);
		_speed.z = 0;
		// _log.Debug("Arrived at target vector, minTargetRadius=%f, distance to target=%f", _minTargetRadius, getDistanceToTarget(_targetVector, evolution));
		_arrivedAtTargetVector = true;

	} else {
		if (!backPedal()) {
			moveToTarget(_targetVector, evolution);
            checkForPassives(evolution);
		}
	}

    checkLerkFlap();

	changeAngle(_bot.getEdict()->v.ideal_yaw, _bot.getEdict()->v.yaw_speed, _bot.getEdict()->v.v_angle.y);
	changeAngle(_bot.getEdict()->v.idealpitch, _bot.getEdict()->v.pitch_speed, _bot.getEdict()->v.v_angle.x);

	if (_speed.z > 0) {
		_bot.getEdict()->v.button |= IN_FORWARD;
	}

	if (_speed.z < 0) {
		_bot.getEdict()->v.button |= IN_BACK;
	}

	// Make the body face the same way we're looking
	// x is pitch
	_bot.getEdict()->v.angles.x = -_bot.getEdict()->v.v_angle.x;
	_bot.getEdict()->v.angles.y = _bot.getEdict()->v.v_angle.y;
	_bot.getEdict()->v.angles.z = 0;
	_bot.getEdict()->v.v_angle.z = 0;

	drawViewDebugBeam();

	_lastTargetVector = _targetVector;
 	_lastBotOrigin = _bot.getEdict()->v.origin;

	// Finally, carry out our move instructions this frame.
	// Apparently vertical movement doesn't do anything, we just look up and go forwards to move vertically.
	// _log.Debug("speed=%f", _speed.z);
	if (_shouldUseMeleeAttack) {
		// FIXME - this is still less than ideal because we should select the right weapon first
		_bot.getEdict()->v.button |= IN_ATTACK;
	}
    WorldStateUtil::checkVector(_bot.getEdict()->v.origin);
    WorldStateUtil::checkVector(_speed);
	WorldStateUtil::checkAnglesForEdict(_bot.getEdict());

	g_engfuncs.pfnRunPlayerMove(_bot.getEdict(),
                                _bot.getEdict()->v.v_angle.toConstFloatArray(),
                                _speed.z,
                                _speed.x,
                                0.0,
                                _bot.getEdict()->v.button,
                                _bot.getEdict()->v.impulse,
                                getMillisecondDelay());

	_bot.getEdict()->v.impulse = 0;
	_bot.getEdict()->v.button = 0;
	_shouldUseMeleeAttack = false;
}


// Make the bot back pedal to try to free himself.
bool BotMovement::backPedal ()
{
    // don't backpedal on ladders, it doesn't help
    if (WorldStateUtil::isOnLadder(_bot.getEdict())) {
        return false;
    }

	if (_backPedalStartTime > 0.0) {
		if ((gpGlobals->time - _backPedalStartTime) >= 1.0) {
			_backPedalStartTime = 0;
			return false;
		} else {
			_speed.z = -_maxSpeed;
			_bot.getEdict()->v.button |= IN_BACK;
			return true;
		}
	} else if (isStuck()) {
		if ((_stuckTimeStart > 0.0) && (gpGlobals->time - _stuckTimeStart) >= 1.0) {
			// Hmm, we haven't moved in a given span of time.  Let's try backing up some.
			_speed.z = -_maxSpeed;
			_bot.getEdict()->v.button |= IN_BACK;
			_backPedalStartTime = gpGlobals->time;
			_stuckTimeStart = 0.0f;
		} else {
			if (_stuckTimeStart == 0.0) {
				_stuckTimeStart = gpGlobals->time;
			}
		}
		return true;

	} else {
		return false;
	}
}


float BotMovement::getDistanceToTarget (const Vector& targetVector, tEvolution evolution) const
{
	Vector creatureGroundOrigin = Bot::getGroundLevelOrigin(_bot.getEdict(), evolution);
	return (targetVector - creatureGroundOrigin).Length();
}


float BotMovement::get2DDistanceToTarget (const Vector& targetVector, tEvolution evolution) const
{
	return (targetVector - _bot.getEdict()->v.origin).Length2D();
}


bool BotMovement::isAtTargetVector() const
{
	return _arrivedAtTargetVector;
}


void BotMovement::drawViewDebugBeam () const
{
	float viewPitchRad = toRad(_bot.getEdict()->v.v_angle.x + 90.0f);
	float viewYawRad = toRad(_bot.getEdict()->v.v_angle.y);
	Vector viewVector(sin(viewPitchRad)*cos(viewYawRad), sin(viewPitchRad)*sin(viewYawRad), cos(viewPitchRad));
	viewVector.Normalize();
	viewVector *= 200.0;
	Vector eyeLevelVector = _bot.getEdict()->v.origin + _bot.getEdict()->v.view_ofs;
	Vector viewVectorTarget =  eyeLevelVector + viewVector;
	//WaypointDebugger::drawDebugBeam(eyeLevelVector, viewVectorTarget, 100, 100, 100);
	//WaypointDebugger::drawDebugBeam(_bot.getEdict()->v.origin, _targetVector, 100, 100, 0);
}


void BotMovement::stop(tEvolution evolution)
{
	_speed = Vector(0.0, 0.0, 0.0);
	_targetVector = _bot.getEdict()->v.origin;
}


void BotMovement::nullMove()
{
	_bot.getEdict()->v.ideal_yaw = _bot.getEdict()->v.idealpitch = 0;
	_bot.getEdict()->v.angles.x = _bot.getEdict()->v.angles.y = _bot.getEdict()->v.angles.z = 0;
	_bot.getEdict()->v.v_angle.x = _bot.getEdict()->v.v_angle.y = _bot.getEdict()->v.v_angle.z = 0;
	_bot.getEdict()->v.button = 0;
	WorldStateUtil::checkAnglesForEdict(_bot.getEdict());
	g_engfuncs.pfnRunPlayerMove(_bot.getEdict(), _bot.getEdict()->v.v_angle.toConstFloatArray(), 
                                0, 0, 0.0, 0, 0, getMillisecondDelay());
}



/// Adapted from hullu's Unstuck Pro metamod.
const char* getModel(edict_t* pEntity) 
{
    const char *model;
    
    if(!FNullEnt(pEntity)) {
        model = STRING(pEntity->v.model);
        
        if (!strcmp( model, "models/player/alien1/alien1.mdl" )) {
            // Skulk
            return "alien1";
        } else if (!strcmp( model, "models/player/alien2/alien2.mdl" )) {
            // Gorge
            return "alien2";
        } else if (!strcmp( model, "models/player/alien3/alien3.mdl" )) {
            // Lerk
            return "alien3";
        } else if (!strcmp( model, "models/player/alien4/alien4.mdl" )) {
            // Fade
            return "alien4";
        } else if (!strcmp( model, "models/player/alien5/alien5.mdl" )) {
            // Onos
            return "alien5";
        } else if (!strcmp( model, "models/player/soldier/soldier.mdl" )) {
            // Marine (normal/jetpack)
            return "soldier";
        } else if (!strcmp( model, "models/player/heavy/heavy.mdl" )) {
            // Heavy armour marine
            return "heavy";
        }
    }
	
    static char retval[1];
    retval[0]=0;
    return retval;
}

/// Adapted from hullu's Unstuck Pro metamod.
int getHullIndex(edict_t* pEntity)
{
	int hullIndex = -1;
	//Check which hull should be used
	const char* model = getModel(pEntity);
	int model_5 = model[5];
		
	if((model_5=='1' || model_5=='2' || model_5=='3') && model[6]==0) {
		//Small aliens (always duck hull)
		hullIndex = (int)head_hull;

	} else if (model_5=='4' && model[6]==0) {
		//Check if ducking
		hullIndex = ( pEntity->v.flags & FL_DUCKING ) ? (int)head_hull : (int)human_hull;

	} else if (model_5=='5' && model[6]==0) {
		//Check if onos ducking
		hullIndex = ( pEntity->v.flags & FL_DUCKING ) ? (int)human_hull : (int)large_hull;

	}
	return hullIndex;
}


/// Adapted from hullu's Unstuck Pro metamod.
bool moveOutsideEntity(edict_t* pEntity)
{
	int hullIndex = getHullIndex(pEntity);
	
	if (hullIndex < 0) {
		return false;
	}
		
	TraceResult tr;
	Vector &origin = pEntity->v.origin;
	
	float height;
	float width;
	
	switch(hullIndex) {
	case (int)human_hull:
		height = 72.0f;
		width = 32.0f;
		break;
		
	case (int)large_hull:
		height = 108.0f;
		width = 64.0f;
		break;
		
	case (int)head_hull:
	default:
		height = 36.0f;
		width = 32.0f;
		break;
	}
	
	height *= 1.5f;
	width *= 1.5f;
	
	float half_height = height * 0.5f;
	float half_width = width * 0.5f;
	
	Vector new_origin;
	
	int type = 0;
	
	for (int i = 0; i < 6*128; ++i) {
		float x,y,z;

		switch (type++) {
		case 0: 
			x = half_width + RANDOM_FLOAT(0,half_width);
			y = RANDOM_FLOAT(-width,width);
			z = RANDOM_FLOAT(-height,height);
			break;
			
		case 1: 
			x = -half_width - RANDOM_FLOAT(0,half_width);
			y = RANDOM_FLOAT(-width,width);
			z = RANDOM_FLOAT(-height,height);
			break;
			
		case 2: 
			x = RANDOM_FLOAT(-width,width);
			y = half_width + RANDOM_FLOAT(0,half_width);
			z = RANDOM_FLOAT(-height,height);
			break;
			
		case 3: 
			x = RANDOM_FLOAT(-width,width);
			y = -half_width - RANDOM_FLOAT(0,half_width);
			z = RANDOM_FLOAT(-height,height);
			break;
			
		case 4: 
			x = RANDOM_FLOAT(-width,width);
			y = RANDOM_FLOAT(-width,width);
			z = half_height + RANDOM_FLOAT(0,half_height);
			break;
			
		default:
			x = RANDOM_FLOAT(-width,width);
			y = RANDOM_FLOAT(-width,width);
			z = -half_height - RANDOM_FLOAT(0,half_height);
			
			type = 0;
			break;
		}
		
		new_origin = origin + Vector(x, y, z);
		
		WorldStateUtil::checkVector(origin);
		WorldStateUtil::checkVector(new_origin);
		TRACE_LINE(origin.toConstFloatArray(), new_origin.toConstFloatArray(), FALSE, NULL, &tr);
		
		if (tr.flFraction >= 0.998f) {
			TRACE_HULL(new_origin.toConstFloatArray(), new_origin.toConstFloatArray(), FALSE, hullIndex, NULL, &tr);
			
			if(!tr.fStartSolid) {
				pEntity->v.origin = new_origin;
				break;
			}
		}
	}

	return true;
}


const float BLINK_VELOCITY = 750.0;
const float BLINK_PITCH_ANGLE = -10.0;

void BotMovement::doBlink()
{
	if (_targetVector.Length() > 0) {
		Vector targetVector(_targetVector);
		// pitch the target upwards a bit - this is a big hack right now just to avoid blinking into the floor
		targetVector.z += 50.0;
		Vector relativeTargetVector = targetVector - _bot.getEdict()->v.origin;
		//WaypointDebugger::drawDebugBeam(_bot.getEdict()->v.origin, targetVector, 255, 0, 0);
		_bot.getEdict()->v.velocity = relativeTargetVector.Normalize() * BLINK_VELOCITY;
		_bot.getEdict()->v.button |= IN_JUMP;
	}
}
