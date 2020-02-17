//
// $Id: EntityRegistry.cpp,v 1.2 2005/02/17 06:30:45 clamatius Exp $


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

#include "BotTypedefs.h"
#include "EntityRegistry.h"
#include "extern/halflifesdk/util.h"

EntityRegistry::~EntityRegistry()
{
	clear();
}


void EntityRegistry::clear()
{
	for (map<edict_t*, Entry*>::iterator ii = _entriesByEdict.begin(); ii != _entriesByEdict.end(); ii++) {
		Entry* pEntry = ii->second;
		delete pEntry->pExtraData;
		delete pEntry;
	}
	_entriesByEdict.clear();
	for (map<string, tEntrySet*>::iterator jj = _entriesByName.begin(); jj != _entriesByName.end(); jj++) {
		tEntrySet* pSet = jj->second;
		delete pSet;
	}
	_entriesByName.clear();
}



void EntityRegistry::addEntry(edict_t* pEdict, HiveMindEntityInfo* pExtraData)
{
	if (!FNullEnt(pEdict) && getEntry(pEdict) == NULL) {
		Entry* pEntry = new Entry();
		pEntry->entity.setEdict(pEdict);
		pEntry->classname = STRING(pEdict->v.classname);
		pEntry->pExtraData = pExtraData;

		_entriesByEdict[pEdict] = pEntry;

		map<string, tEntrySet*>::iterator found = _entriesByName.find(pEntry->classname);
		tEntrySet* pSet = getEntries(pEntry->classname);
		if (pSet == NULL) {
			pSet = new tEntrySet;
			_entriesByName[pEntry->classname] = pSet;
		}

		pSet->insert(pEntry);
	}
}



EntityRegistry::Entry* EntityRegistry::getEntry(edict_t* pEdict)
{
	map<edict_t*, Entry*>::iterator found = _entriesByEdict.find(pEdict);
	return found != _entriesByEdict.end() ? found->second : NULL;
}



EntityRegistry::tEntrySet* EntityRegistry::getEntries(const string& classname)
{
	map<string, tEntrySet*>::iterator found = _entriesByName.find(classname);
	return found != _entriesByName.end() ? found->second : NULL;
}



void EntityRegistry::removeEntry(edict_t* pEdict)
{
	map<edict_t*, Entry*>::iterator found = _entriesByEdict.find(pEdict);
	if (found != _entriesByEdict.end()) {
		Entry* pEntry = found->second;
		tEntrySet* pSet = getEntries(pEntry->classname);
		if (pSet != NULL) {
			tEntrySet::iterator setEntryFound = pSet->find(pEntry);
			if (setEntryFound != pSet->end()) {
				pSet->erase(setEntryFound);
				_entriesByEdict.erase(found);
				delete pEntry->pExtraData;
				delete pEntry;
			}
		}
	}
}


int EntityRegistry::size()
{
	return _entriesByEdict.size();
}


vector<EntityReference> EntityRegistry::getAllEntities()
{
	vector<EntityReference> allEntities;
	for (map<edict_t*, Entry*>::iterator ii = _entriesByEdict.begin(); ii != _entriesByEdict.end(); ii++) {
		Entry* pEntry = ii->second;
		if (!pEntry->entity.isNull()) {
			allEntities.push_back(pEntry->entity);
		}
	}
	return allEntities;
}


