//
// $Id: EntityInfo.h,v 1.2 2005/02/18 00:57:34 clamatius Exp $

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

#ifndef __CONFIG_ENTITYINFO_H
#define __CONFIG_ENTITYINFO_H

#include "BotTypedefs.h"

typedef enum {
	kInvalidEntity = 0,

    kPlayer = 1,

    kCommandChair = 2,
    kTurretFactory = 3,
    kTurret = 4,
    kArmsLab = 5,
    kPrototypeLab =6,
    kInfantryPortal = 7,
    kObservatory = 8,
    kArmory = 9,
    kResourceTower = 10,
    kHiveLoc = 11,
    kResourceLoc = 12,

    kOffenseChamber = 13,
    kDefenseChamber = 14,
    kMovementChamber = 15,
    kSensoryChamber = 16,
    kAlienResourceTower = 17,
    kPhaseGate = 18,
    kLandMine = 19,
    kTripMine = 20,
    kSiegeTurret = 21,
	kHive = 22

} eEntityType;

class EntityInfo  
{
public:

    EntityInfo();

    EntityInfo(eEntityType type, int team, const string& classname, int defaultInfluence, 
				 bool isBuilding, int attackValue = 0);

    EntityInfo(const EntityInfo& other);

    inline int getTeam() const { return _team; }

    int getTeam(edict_t* pEdict) const;

    inline int getDefaultInfluence() const { return _defaultInfluence; }

	inline int getAttackValue() const { return _attackValue; }

	inline eEntityType getType() const { return _type; }

	inline bool isBuilding() const { return _isBuilding; }

	const string& getClassname() const { return _classname; }

protected:

	eEntityType _type;
    int _team;
    string _classname;
    int _defaultInfluence;
    bool _isBuilding;
	int _attackValue;
};

#endif // __CONFIG_ENTITYINFO_H
