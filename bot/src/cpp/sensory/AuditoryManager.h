//
// $Id: AuditoryManager.h,v 1.5 2004/10/08 01:42:30 clamatius Exp $

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

#ifndef __SENSORY_AUDITORYMANAGER_H
#define __SENSORY_AUDITORYMANAGER_H

#include "BotTypedefs.h"
#include "framework/Log.h"
#include "AuditoryTrace.h"

class AuditoryManager
{
public:

    static AuditoryManager& getInstance();

    void listenForSounds(const Vector& origin, float range);
    
    bool isPlayerNoisy(edict_t* pPlayerEdict);

    bool wasPlayerHeard(edict_t* pPlayerEdict);

	bool isHeardPlayerNearby(const Vector& origin, float range);

    vector<edict_t*> getNearbyHeardPlayers(const Vector& origin, float range);

	inline float getHearingRadius() { return _hearingRadius; }

    ~AuditoryManager();

	bool botIsInRange(edict_t* pTargetEdict, float range);

	void markAllMarinesHeard();

protected:

    AuditoryManager();

    void checkIfPlayerNoisy(edict_t* pPlayerEdict);

    void addTrace(edict_t* pPlayerEdict);

    void removeOldEntries();

    typedef map<edict_t*, AuditoryTrace*> tTraceMap;
    tTraceMap _traces;
	float _hearingRadius;

private:

    static AuditoryManager _singleton;

    static Log _log;
};

#endif // __SENSORY_AUDITORYMANAGER_H
