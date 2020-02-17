//
// $Id: EntityRegistry.h,v 1.3 2005/12/14 19:17:27 clamatius Exp $

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

#ifndef __WORLDSTATE_ENTITYREGISTRY_H
#define __WORLDSTATE_ENTITYREGISTRY_H

#include "BotTypedefs.h"
#include "EntityReference.h"
#include <set>

struct HiveMindEntityInfo
{
	HiveMindEntityInfo (tNodeId nearestWpId) : 
        currentNearestWpId(nearestWpId), lastAttacked(0), numRescuers(0)
    {
    }

	tNodeId currentNearestWpId;
	float lastAttacked;
	int numRescuers;
};


class EntityRegistry
{
public:

	~EntityRegistry();

	struct Entry
	{
		EntityReference entity;
		string classname;
		HiveMindEntityInfo* pExtraData;
	};
	typedef set<Entry*> tEntrySet;

	typedef map<edict_t*, Entry*>::iterator iterator; 
	inline iterator begin() { return _entriesByEdict.begin(); }

	inline iterator end() { return _entriesByEdict.end(); }

	void addEntry(edict_t* pEdict, HiveMindEntityInfo* pExtraData);

	Entry* getEntry(edict_t* pEdict);

	tEntrySet* getEntries(const string& classname);

	void removeEntry(edict_t* pEdict);

	int size();

	void clear();

	vector<EntityReference> getAllEntities();

protected:

	map<string, tEntrySet*> _entriesByName;
	map<edict_t*, Entry*> _entriesByEdict;
};


#endif // __WORLDSTATE_ENTITYREGISTRY_H
