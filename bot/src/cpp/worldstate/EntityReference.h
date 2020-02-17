//
// $Id: EntityReference.h,v 1.4 2004/10/20 17:21:17 clamatius Exp $

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

#ifndef __WORLDSTATE_ENTITYREFERENCE_H
#define __WORLDSTATE_ENTITYREFERENCE_H

#include "BotTypedefs.h"
#include "extern/halflifesdk/extdll.h"

/// EntityReference represents a reference to a HL entity (edict_t) that must be kept between frames.
/// You MUST keep references using this class, since otherwise the edict_t* could be deleted between frames
/// and the program may seg fault.

class EntityReference
{
public:

    EntityReference(edict_t* pEdict = NULL);

    EntityReference(const EntityReference& other);

    edict_t* getEdict();

    int getId();

    void clear();

    void setEdict(edict_t* pEdict);

    bool isNull();

    Vector* getOrigin();

    bool operator == (const EntityReference& other);

    bool operator == (edict_t* pOther);

    const char* getClassname();

    float getHealth();

    int getTeam();

protected:

    void check();


    edict_t* _pEdict;
    int _id;
    float _lastTimeChecked;
};

#endif // __WORLDSTATE_ENTITYREFERENCE_H
