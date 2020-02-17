//
// $Id: EntityReference.cpp,v 1.3 2003/09/18 01:47:28 clamatius Exp $

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

#include "worldstate/EntityReference.h"
#include "BotManager.h"

extern enginefuncs_t g_engfuncs;


EntityReference::EntityReference(edict_t* pEdict) :
    _pEdict(NULL),
    _id(-1),
    _lastTimeChecked(0)
{
    if (pEdict != NULL) {
        setEdict(pEdict);
    }
}


EntityReference::EntityReference(const EntityReference& other) :
    _pEdict(other._pEdict),
    _id(other._id),
    _lastTimeChecked(other._lastTimeChecked)
{
}


edict_t* EntityReference::getEdict()
{
    check();
    return _pEdict;
}


void EntityReference::setEdict(edict_t* pEdict)
{
    _pEdict = pEdict;
    _id = g_engfuncs.pfnIndexOfEdict(pEdict);
    _lastTimeChecked = gpGlobals->time;
}


void EntityReference::check()
{
    if (gpGlobals->time == _lastTimeChecked) {
        return;
    }

    _lastTimeChecked = gpGlobals->time;

    if ((_pEdict == NULL) || (_id < 0)) {
        clear();
    }

    edict_t* pNewEdict = g_engfuncs.pfnPEntityOfEntIndex(_id);

    if (pNewEdict != _pEdict) {
        clear();
    }
}


void EntityReference::clear()
{
    _id = -1;
    _pEdict = NULL;
}


bool EntityReference::isNull()
{
    check();
    return (_pEdict == NULL);
}


Vector* EntityReference::getOrigin()
{
    return isNull() ? NULL : &_pEdict->v.origin;
}


const char* EntityReference::getClassname()
{
    return isNull() ? NULL : STRING(_pEdict->v.classname);
}


float EntityReference::getHealth()
{
    return isNull() ? 0.0f : _pEdict->v.health;
}


int EntityReference::getTeam()
{
    return isNull() ? -1 : _pEdict->v.team;
}


bool EntityReference::operator == (const EntityReference& other)
{
    check();
    return _pEdict == other._pEdict;
}


bool EntityReference::operator == (edict_t* pOther)
{
    check();
    return _pEdict == pOther;
}
