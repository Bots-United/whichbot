//
// $Id: HiveManager.cpp,v 1.5 2005/02/19 07:59:05 clamatius Exp $

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

#include "HiveManager.h"


//////////////////////////////////////////////////////////////////////
// Impl
//////////////////////////////////////////////////////////////////////

vector<HiveInfo*> HiveManager::_hives;
map<int, int> HiveManager::_traits;

// NOTE:  If the order of the hives in the initial hive list sent by the server
// when the game first starts ever differs from implicit order derived from the
// hive status update messages, this code will not function as expected.
void HiveManager::addHive (HiveInfo& info)
{
	HiveInfo* newInfo = new HiveInfo(info);
	_hives.push_back(newInfo);
}

HiveInfo* HiveManager::getHive (const int hiveId)
{
	if ((hiveId >= 0) && (hiveId < (int)_hives.size())) {
		return _hives[hiveId];
	} else {
		return NULL;
	}
}


int HiveManager::getActiveHiveCount ()
{
    int numHives = 0;
    for (vector<HiveInfo*>::iterator ii = _hives.begin(); ii != _hives.end(); ii++) {
        float hiveHealth = (*ii)->getHealth();
        if (hiveHealth == FULL_HIVE_HEALTH) {
            numHives++;
        }
    }
	return numHives;
}

void HiveManager::reset ()
{
    for (vector<HiveInfo*>::iterator ii = _hives.begin(); ii != _hives.end(); ++ii) {
		delete *ii;
    }

	_hives.clear();

    resetTraits();

	if (gpBotManager->inCombatMode()) {
		// This is a hack to manually add the hive entry till we understand the hive messages
		// in Combat.
		CBaseEntity* pEntity = UTIL_FindEntityByClassname(NULL, "team_hive");
		if (pEntity != NULL && !FNullEnt(pEntity->edict())) {
			HiveInfo info(0, pEntity->edict());
			addHive(info);
		}
	}
}


void HiveManager::resetTraits()
{
    _traits.clear();
}


void HiveManager::addTraitLevel(int traitId)
{
    map<int, int>::iterator found = _traits.find(traitId);
    if (found != _traits.end()) {
        _traits[traitId] = found->second + 1;

    } else {
        _traits[traitId] = 1;
    }
}


int HiveManager::getTraitLevel(int traitId)
{
    map<int, int>::iterator found = _traits.find(traitId);
    if (found != _traits.end()) {
        return found->second;

    } else {
        return 0;
    }
}


float HiveManager::distanceToNearestHive(Vector& fromPoint)
{
	float closest = MAX_DISTANCE_ESTIMATE;
    for (vector<HiveInfo*>::iterator ii = _hives.begin(); ii != _hives.end(); ++ii) {
		float range = ((*ii)->getOrigin() - fromPoint).Length();
		if (range < closest) {
			closest = range;
		}
	}
	return closest;
}


HiveInfo* HiveManager::getNearestActiveHive(Vector& fromPoint)
{
	HiveInfo* closest = NULL;
    float closestRange = MAX_DISTANCE_ESTIMATE;
    for (vector<HiveInfo*>::iterator ii = _hives.begin(); ii != _hives.end(); ++ii) {
		float range = ((*ii)->getOrigin() - fromPoint).Length();
		if ((range < closestRange) && ((*ii)->isActive() || gpBotManager->inCombatMode())) {
			closest = *ii;
            closestRange = range;
		}
	}
	return closest;
}


HiveInfo* HiveManager::getNearestHive(Vector& fromPoint)
{
	HiveInfo* closest = NULL;
    float closestRange = MAX_DISTANCE_ESTIMATE;
    for (vector<HiveInfo*>::iterator ii = _hives.begin(); ii != _hives.end(); ++ii) {
        if ((*ii)->isActive()) {
            float range = ((*ii)->getOrigin() - fromPoint).Length();
            if (range < closestRange) {
                closest = *ii;
                closestRange = range;
            }
        }
	}
	return closest;
}
