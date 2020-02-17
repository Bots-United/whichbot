//
// $Id: EntityInfo.cpp,v 1.2 2005/02/18 00:57:34 clamatius Exp $

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

#include "EntityInfo.h"

EntityInfo::EntityInfo() :
	_type(kInvalidEntity),
    _team(-1),
    _defaultInfluence(0),
    _isBuilding(false),
	_attackValue(0)
{
}



EntityInfo::EntityInfo(eEntityType type, int team, const string& classname, int defaultInfluence, bool isBuilding, int attackValue) :
	_type(type),
    _team(team),
    _classname(classname),
    _defaultInfluence(defaultInfluence),
    _isBuilding(isBuilding),
	_attackValue(attackValue)
{

}


EntityInfo::EntityInfo(const EntityInfo& other) :
	_type(other._type),
    _team(other._team),
    _classname(other._classname),
    _defaultInfluence(other._defaultInfluence),
    _isBuilding(other._isBuilding),
	_attackValue(other._attackValue)
{
}


int EntityInfo::getTeam(edict_t* pEdict) const
{
    if (_type == kPlayer && pEdict != NULL) {
        return pEdict->v.team;

    } else {
        return _team;
    }
}
