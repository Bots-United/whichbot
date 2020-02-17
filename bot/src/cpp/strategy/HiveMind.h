//
// $Id: HiveMind.h,v 1.31 2005/02/25 23:38:51 clamatius Exp $

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

#ifndef __STRATEGY_HIVEMIND_H
#define __STRATEGY_HIVEMIND_H

#include "BotTypedefs.h"
#include "strategy/Strategy.h"
#include "extern/halflifesdk/extdll.h"
#include <map>
#include "Bot.h"
#include "strategy/PackManager.h"
#include "framework/Log.h"
#include "worldstate/EntityRegistry.h"

// HiveMind keeps track of where all the bots are and what they should be doing.
// It also keeps track of what the bots see and where they saw it.
class HiveMind
{
public:

    static void tick();

    static void reset();

    static void visitedWaypoint(tNodeId wptId);

	static void waitedAtWaypoint(tNodeId wptId, tEvolution evolution);

    static float getVisitTime(tNodeId wptId);

    static void entitySeen(tEvolution byEvolution, edict_t* pEntity);

	static int getNumEntitiesSeen(const string& className);

    static tNodeId getSeenWaypoint(edict_t* seenEntity);

    static tNodeId getNearestWaypoint(tEvolution evolution, edict_t* pNearEdict);

    static void entityDestroyed(edict_t* pEntity);

    static vector<EntityReference> getEntitiesSeen();

    static tEvolution getDesiredEvolution(Bot& thisBot);

    static int getDesiredUpgrade(Bot& thisBot);

	static void setStrategies(Bot& bot);

	// Called for cleanup purposes.
	static void unsetStrategies(Bot& bot);

	static PackManager& getPackManager();

    static void entityUnderAttack(const Vector& atLocation);

	static void entityUnderAttack(edict_t* pEntity);

	static bool isEntityUnderAttack(edict_t* pEntity);

	static EntityReference* getEntityToRescue();

    static bool resourceNodeIsTaken(Vector& nodeLoc, int nodeMask);

    static void dumpStatusToLog();

    static void setLifeformOverride(tEvolution lifeform);

protected:

	typedef hash_map<tNodeId, vector<EntityReference> > tWaypointMap;

    static void addInfluencingEntities(const char* entityClassname);

	static void setStandardStrategies (Bot& bot);

	static void setGorgeStrategies (Bot& bot);

    static void cleanupEntities();

    static bool entityEntryIsInvalid(edict_t* pEdict);

    static tEvolution getDesiredClassicEvolution(Bot& thisBot);

    static tEvolution getDesiredCombatEvolution(Bot& thisBot);

    static int getDesiredClassicUpgrade(Bot& thisBot);

    static int getDesiredCombatUpgrade(Bot& thisBot);

    static vector<float> _visitTimes;
	static EntityRegistry _entities;
	static PackManager _wolfPackMgr;
    static Log _log;
    static tEvolution _lifeformOverride;
};

#endif // __STRATEGY_HIVEMIND_H
