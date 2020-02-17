//
// $Id: AuditoryManager.cpp,v 1.7 2005/12/14 19:24:52 clamatius Exp $

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

#include "AuditoryManager.h"
#include "BotManager.h"
#include "worldstate/WorldStateUtil.h"
#include "config/Config.h"

static const float WALK_SPEED = 100;
static const float TRACE_TIMEOUT = 50.0; // listen traces time out after this

AuditoryManager AuditoryManager::_singleton;

Log AuditoryManager::_log(__FILE__);


AuditoryManager::AuditoryManager() :
	_hearingRadius(0)
{
}


AuditoryManager& AuditoryManager::getInstance()
{
	if (_singleton._hearingRadius <= 0) {
		_singleton._hearingRadius = Config::getInstance().getTweak("game/engine/hearing_radius", 1000.0);
	}
    return _singleton;
}


AuditoryManager::~AuditoryManager()
{
    for (tTraceMap::iterator ii = _traces.begin(); ii != _traces.end(); ii++) {
        delete ii->second;
    }
    _traces.clear();
}


void AuditoryManager::listenForSounds(const Vector& origin, float range)
{
    for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
        CBaseEntity* pPlayerBaseEntity = UTIL_PlayerByIndex(ii);
        
        if (pPlayerBaseEntity != NULL) {
            edict_t* pPlayerEdict = INDEXENT(ii);
            
            if (WorldStateUtil::isPlayerPresent(pPlayerEdict) && pPlayerEdict->v.team == MARINE_TEAM) {
                Vector relativeVector = pPlayerEdict->v.origin - origin;
				//_log.Debug("Player is at range %f", relativeVector.Length());
                if (relativeVector.Length() <= range) {
                    checkIfPlayerNoisy(pPlayerEdict);
                }
            }
        }
    }
    
    removeOldEntries();
}


void AuditoryManager::markAllMarinesHeard()
{
    for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
        CBaseEntity* pPlayerBaseEntity = UTIL_PlayerByIndex(ii);
        
        if (pPlayerBaseEntity != NULL) {
            edict_t* pPlayerEdict = INDEXENT(ii);
            
            if (WorldStateUtil::isPlayerPresent(pPlayerEdict) && pPlayerEdict->v.team == MARINE_TEAM) {
				addTrace(pPlayerEdict);
			}
		}
	}
}
 

void AuditoryManager::checkIfPlayerNoisy(edict_t* pPlayerEdict)
{
    if (isPlayerNoisy(pPlayerEdict)) {
        _log.Debug("Heard player");
        addTrace(pPlayerEdict);
    }
}


bool AuditoryManager::botIsInRange(edict_t* pTargetEdict, float range)
{
	for (vector<Bot*>::iterator ii = gpBotManager->begin(); ii != gpBotManager->end(); ii++) {
		Bot* pBot = *ii;
		if (pBot->getEdict() != NULL) {
			Vector relativeVec(pBot->getEdict()->v.origin - pTargetEdict->v.origin);
			//_log.Debug("Player is at range %f", relativeVec.Length());
			if (relativeVec.Length() <= range) {
				return true;
			}
		}
	}
	return false;
}


bool AuditoryManager::isPlayerNoisy(edict_t* pPlayerEdict)
{
    const int noisyMask = IN_ATTACK | IN_USE | IN_RELOAD | IN_JUMP;
    if ((pPlayerEdict->v.button & noisyMask) != 0) {
        return true;
    }

    if (pPlayerEdict->v.velocity.Length() > WALK_SPEED) {
        return true;
    }

	// parasited players always count as "noisy"
	if ((pPlayerEdict->v.iuser4 & MASK_PARASITED) != 0) {
		return true;
	}

    return false;
}


void AuditoryManager::addTrace(edict_t* pPlayerEdict)
{
    if (pPlayerEdict != NULL) {
        tTraceMap::iterator found = _traces.find(pPlayerEdict);
        if (found == _traces.end()) {
            _traces[pPlayerEdict] = new AuditoryTrace(pPlayerEdict, gpGlobals->time);
            
        } else {
            found->second->updateTimeHeard(gpGlobals->time);
            found->second->updatePosition(pPlayerEdict->v.origin);
        }
    }
}


bool AuditoryManager::wasPlayerHeard(edict_t* pPlayerEdict)
{
    tTraceMap::iterator found = _traces.find(pPlayerEdict);
    return (found != _traces.end());
}


bool AuditoryManager::isHeardPlayerNearby(const Vector& origin, float range)
{
	_log.Debug("Checking for heard players [numTraces=%d, range=%f]", _traces.size(), range);
    for (tTraceMap::iterator ii = _traces.begin(); ii != _traces.end(); ii++) {
        AuditoryTrace* pTrace = ii->second;
        if (pTrace->getEdict() != NULL) {
            Vector relativeVector = origin - pTrace->getPosition();
			_log.Debug("Range to heard player is %f", relativeVector.Length());
            if (range >= relativeVector.Length()) {
				return true;
            }
        }
    }
	return false;
}


vector<edict_t*> AuditoryManager::getNearbyHeardPlayers(const Vector& origin, float range)
{
    vector<edict_t*> nearbyPlayers;

    for (tTraceMap::iterator ii = _traces.begin(); ii != _traces.end(); ii++) {
        AuditoryTrace* pTrace = ii->second;
        if (pTrace->getEdict() != NULL) {
            Vector relativeVector = origin - pTrace->getPosition();
            if (range >= relativeVector.Length()) {
                nearbyPlayers.push_back(pTrace->getEdict());
            }
        }
    }

    return nearbyPlayers;
}


void AuditoryManager::removeOldEntries()
{
    // TODO - this can be done in one pass
    vector<edict_t*> toRemove;
    for (tTraceMap::iterator ii = _traces.begin(); ii != _traces.end(); ii++) {
        AuditoryTrace* pTrace = ii->second;
        if (pTrace->getEdict() == NULL || pTrace->getLastTimeHeard() > gpGlobals->time + TRACE_TIMEOUT) {
            toRemove.push_back(ii->first);
        }
    }

    for (vector<edict_t*>::iterator jj = toRemove.begin(); jj != toRemove.end(); jj++) {
        _traces.erase(*jj);
    }
}
