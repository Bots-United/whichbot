//
// $Id: BotMovement.h,v 1.8 2005/12/14 23:09:44 clamatius Exp $

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

#ifndef __NAVIGATION_BOTMOVEMENT_H
#define __NAVIGATION_BOTMOVEMENT_H

#include "BotTypedefs.h"
class Bot;
#include "extern/halflifesdk/util.h"
#include "framework/Log.h"

// Adapted from hullu's Unstuck Pro metamod.
extern bool moveOutsideEntity(edict_t* pEntity);


/**
 * This class is intended to provide low-level movement routines associated with movement to a point.
 */
class BotMovement  
{
public:

	BotMovement(Bot& bot);

	virtual ~BotMovement();

    /**
     * Tell the movement that we want to go to this point (should be very close, but not necessarily visible).
     */
	void setTarget(Vector& targetPoint, float cushionDistance, bool noJump = false);

    /**
     * We want to go to this point, but it's via a ladder (up or down).
     */
    void setLadderTarget(Vector& targetPoint, bool goingUp, float minDistance);

    void setRandomTarget();

    /**
     * Don't move for now.
     */
	void stop(tEvolution evolution);

    /**
     * Execute movement instructions for this frame.
     */
	void move(tEvolution evolution);

    /**
     * Execute a null move (useful if we're dead or something).
     */
    void nullMove();

	void strafe(float speed) { _speed.x += speed; _speed.z -= speed; }

    inline edict_t* getHit() const { return _pLastHit; }

    inline float getHitTime() const { return _lastHitTime; }

	float getDistanceToTarget (tEvolution evolution) const;

	bool isAtTargetVector () const;

	inline bool shouldUseMeleeAttack() const { return _shouldUseMeleeAttack; }

	/**
	 * Calculate the surface angle relative to the bot's horizontal plane
	 */
	float getRelativeHitAngleDeg(const Vector& vec);

	void doBlink();

protected:

    /**
     * Guess how long the next frame will take to render.
     */
	byte getMillisecondDelay();

    /**
     * Move the bot directly towards the desired target irregardless of obstacles.
     */
    void moveDirectlyTowardsTarget(const Vector& targetVector, tEvolution evolution);

	bool canWeFeelTheTarget(const Vector& targetVector, tEvolution evolution, TraceResult& collisionTr);

	float getHitHeight (const TraceResult& upperFrontWhiskerTr, const TraceResult& lowerFrontWhiskerTr, float whiskerPitchAngleRad, float whiskerLength);

	/**
	 * Apply movement corrections based on nearby obstacles.
	 */
	void moveToTarget(const Vector& targetVector, tEvolution evolution);

	/**
	 * Util method to determine if the specified entity is a ladder.
	 */
	bool isLadder(const edict_t* pEdict);

	/**
	 * Util method
	 */
	Vector calculateVector (const Vector& origin, float pitchRad, float yawRad, float length) const;

	/**
	 * Trace from the bot's origin to the whiskerOrigin
	 */
	TraceResult traceSkinnyWhisker (Vector& whiskerOrigin, float pitchRad, float yawRad,
		float whiskerLength, int red, int green, int blue) const;

	/**
	 * Use a larger hull to trace from the bot's origin to the whiskerOrigin
	 */
	TraceResult traceFatWhisker (Vector& whiskerOrigin, float pitchRad, float yawRad,
		float whiskerLength, int red, int green, int blue) const;

	/**
	 * Set the bot's speed for the next tick
	 */
	void setSpeed (const Vector& targetVector, float fabsDeltaAngleDeg, float maxDeltaAngleDeg, tEvolution evolution);

	/**
	 * Determine if the bot's stuck
	 */
	bool isStuck () const;

	/**
	 * Determine if the bot is close to the assigned target vector.
	 */
	bool isCloseToTarget (const Vector& targetVector, tEvolution evolution) const;

	/**
	 * Determine if the target vector is directly over the bot's head
	 */
	bool isTargetOverhead (float relativeTargetPitchDeg) const;

	/**
	 * Return the set of angles to the target relative to the bot's origin
	 */
	void getTargetAnglesDeg (const Vector& targetVector, float& pitchDeg, float& yawDeg) const;

	/**
	 * React to center whisker collision.  Generally we'd want to eat whatever's in our way.
	 */
	void reactToCollision (TraceResult& tr);

	bool backPedal ();
	float getDistanceToTarget (const Vector& targetVector, tEvolution evolution) const;
	float get2DDistanceToTarget (const Vector& targetVector, tEvolution evolution) const;

    // If we're a lerk, we should see if we need to flap to continue flying
    void checkLerkFlap();

    // if we're executing normal movement, check to see if we should do passive stuff (self-heal, blink)
    void checkForPassives(tEvolution evolution);

	// bool backPedal ();

	Bot& _bot;

	Vector _speed;
	float _maxSpeed;
    float _lastRandomPickTime;
	bool _shouldUseMeleeAttack;
	bool _noJump;

	float _lastFrameTime;
	byte _msecval;

	float _lastHitTime;
	edict_t* _pLastHit;

	// The bot's current target vector;
	Vector _targetVector;

	// If the bot can get within this radius of the target, we'll call it good.
	float _minTargetRadius;

	Vector _lastTargetVector;
	Vector _lastBotOrigin;

	float _stuckTimeStart;
	float _backPedalStartTime;

    float _lastFlapStart;

	bool _arrivedAtTargetVector;

private:

	void drawViewDebugBeam () const;
    static Log _log;
};

#endif // __NAVIGATION_BOTMOVEMENT_H
