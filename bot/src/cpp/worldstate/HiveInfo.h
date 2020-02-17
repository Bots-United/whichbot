//
// $Id: HiveInfo.h,v 1.1 2003/09/18 01:47:28 clamatius Exp $

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

#ifndef __WORLDSTATE_HIVEINFO_H
#define __WORLDSTATE_HIVEINFO_H

#include "BotManager.h"

extern const int FULL_HIVE_HEALTH;

extern edict_t* getHiveEntity(Vector& position);

typedef enum
{
    kAnyHive,
    kFilledHive,
    kEmptyHive
}  tHiveFillState;


class HiveInfo  
{
public:

	HiveInfo (const int hiveId, edict_t* pEntity) :
	    _hiveId(hiveId),
	    _health(0),
	    _entity(pEntity),
		_wptId(INVALID_NODE_ID),
		_origin(pEntity->v.origin)
	{
		_wptId = gpBotManager->getWaypointManager().getNearestWaypoint(kGorge, pEntity->v.origin, NULL);
	}

	// Copy ctor
	HiveInfo (const HiveInfo& other) :
		_hiveId(other._hiveId),
		_health(other._health),
		_entity(other._entity),
		_wptId(other._wptId),
		_origin(other._origin)
	{
		//empty
	}

	void updateHealth (byte health)
	{
		_health = health;
	}

	int getHiveId() const { return _hiveId; }

	int getWaypointId() const { return _wptId; }

	int getHealth() const { return _health; }

    tHiveFillState getHiveFillState() const { return (_health == 0) ? kEmptyHive : kFilledHive; }

    bool isActive () { return getHiveFillState() == kFilledHive; }

	EntityReference& getEntity()
	{
		if (_entity.isNull()) {
			_entity.setEdict(getHiveEntity(_origin));
		}
		return _entity;
	}

	Vector& getOrigin() { return _origin; }

protected:

	const int	_hiveId;
	int         _wptId;
	int		    _health;
	EntityReference	_entity;
	Vector _origin;
};

#endif // __WORLDSTATE_HIVEINFO_H
