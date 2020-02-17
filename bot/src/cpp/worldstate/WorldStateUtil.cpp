//
// $Id: WorldStateUtil.cpp,v 1.29 2005/12/14 19:21:42 clamatius Exp $

#include "worldstate/WorldStateUtil.h"
#include "extern/halflifesdk/extdll.h"
#include "extern/halflifesdk/util.h"
#include "extern/metamod/meta_api.h"
#include <float.h>
extern "C" void* player(entvars_t* pev);


int WorldStateUtil::countEntities(const char* classname)
{
    CBaseEntity* pEntity = NULL;
    int count = 0;
    bool noMoreEntities = false;

	do {
        noMoreEntities = true;

		pEntity = UTIL_FindEntityByClassname(pEntity, classname);
        if (pEntity != NULL) {
            edict_t* pEdict = pEntity->edict();

            if (!FNullEnt(pEdict)) {
                count++;
                noMoreEntities = false;
            }
        }

    } while (!noMoreEntities);

    return count;
}


edict_t* WorldStateUtil::findClosestEntity(const char* classname, const Vector& fromPos)
{
	float minRange = 100000.0;
	edict_t* pClosestEdict = NULL;

	CBaseEntity* pEntity = NULL;
	do {
		pEntity = UTIL_FindEntityByClassname(pEntity, classname);
		if (pEntity != NULL) {
            edict_t* pEdict = pEntity->edict();

            if (!FNullEnt(pEdict)) {
				float range = (pEdict->v.origin - fromPos).Length();
  				if (range < minRange) {
					minRange = range;
					pClosestEdict = pEdict;
				}
			}
        }

    } while (pEntity != NULL);

	return pClosestEdict;
}


edict_t* WorldStateUtil::findClosestSwitchEntity(const char* classname, const Vector& fromPos)
{
	float minRange = 100000.0;
	edict_t* pClosestEdict = NULL;

	CBaseEntity* pEntity = NULL;
	do {
		pEntity = UTIL_FindEntityByClassname(pEntity, classname);
		if (pEntity != NULL) {
            edict_t* pEdict = pEntity->edict();

			if (!FNullEnt(pEdict)) {
                Vector entityOrigin = pEdict->v.absmin + (pEdict->v.size * 0.5);

				float range = (entityOrigin - fromPos).Length();
              
				if (range < minRange) {
					minRange = range;
					pClosestEdict = pEdict;
				}
			}
        }

    } while (pEntity != NULL);

	return pClosestEdict;
}


int WorldStateUtil::countPlayersWithEvolution(tEvolution evolution)
{
	int count = 0;
	for (int ii = 1; ii <= gpGlobals->maxClients; ii++) {
		edict_t* pPlayerEdict = INDEXENT(ii);
		if (!FNullEnt(pPlayerEdict) &&
			(pPlayerEdict->v.team == ALIEN_TEAM) &&
			(Bot::getEvolution(pPlayerEdict) == evolution))
		{
			count++;
		}
	}
	return count;
}


edict_t* WorldStateUtil::createPlayer(const string& name)
{
    char ptr[128];
    char namebuf[255];
    strncpy(namebuf, name.c_str(), 255);
    
    edict_t* pEdict = (*g_engfuncs.pfnCreateFakeClient)(namebuf);
    if (pEdict != NULL) {
        pEdict->v.yaw_speed = 15;
        pEdict->v.pitch_speed = 15;
        
        player(&pEdict->v);
        
        MDLL_ClientConnect(pEdict, namebuf, "127.0.0.1", ptr);
        
        MDLL_ClientPutInServer(pEdict);
        
        pEdict->v.v_angle.x = 0;
        pEdict->v.v_angle.y = 0;
        pEdict->v.v_angle.z = 0;
        
        pEdict->v.flags |= FL_FAKECLIENT;
        
    } else {
        LOG_CONSOLE(PLID, "Unable to create bot, createFakeClient failed");
    }
	return pEdict;
}


void WorldStateUtil::joinTeam(edict_t* pEdict, int teamToJoin)
{
    CBaseEntity* pEntity = NULL;
    // Join the team by touching an info_join_team entity
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "info_join_team")) != NULL) {
        if (pEntity->pev->team == teamToJoin) {
            MDLL_Touch(pEntity->edict(), pEdict);
            break;
        }
    }
}


bool WorldStateUtil::isFinite(float val)
{
#ifdef WIN32
	if (!_finite(val)) {
		return false;
		val = 0.0;
	}
#else
    if (isnan(val)) {
		return false;
    }
#endif

	return true;
}

void WorldStateUtil::checkAngle(float val)
{
//	assert(isFinite(val));
//	assert(val >= -180.0);
//	assert(val <= 180.0);

	if (!isFinite(val)) {
		val = 0.0f;
	}

	if (val >= 180.0) {
		val = 179.0;
		
	} else if (val <= -180.0) {
		val = -179.0;
	}
}


void WorldStateUtil::checkAnglesForVector(const Vector& vec)
{
	checkAngle(vec.x);
	checkAngle(vec.y);
	checkAngle(vec.z);
}


void WorldStateUtil::checkAnglesForEdict(edict_t* pEdict)
{
	if (!FNullEnt(pEdict)) {
		checkAnglesForVector(pEdict->v.angles);
		checkAnglesForVector(pEdict->v.v_angle);
		checkAngle(pEdict->v.ideal_yaw);
		checkAngle(pEdict->v.idealpitch);
	}
}


void WorldStateUtil::checkVector(const Vector& vec)
{
	assert(isFinite(vec.x));
	assert(isFinite(vec.y));
	assert(isFinite(vec.z));
	if (!isFinite(vec.x)) {
		((Vector&)vec).x = 0.0;
	}
	if (!isFinite(vec.y)) {
		((Vector&)vec).y = 0.0;
	}
	if (!isFinite(vec.z)) {
		((Vector&)vec).z = 0.0;
	}
}



float WorldStateUtil::getMaxArmour(edict_t* pEdict)
{
    if ((pEdict != NULL) && (pEdict->v.team == ALIEN_TEAM)) {
        tEvolution evolution = Bot::getEvolution(pEdict);
        if (evolution != kInvalidEvolution) {
            // ignore carapace for now
            return g_CreatureMaxArmour[(int)evolution];
        }
    }
    return 0;
}


bool WorldStateUtil::isOnLadder(edict_t* pEdict)
{
    return pEdict != NULL ? (pEdict->v.movetype == MOVETYPE_FLY) : false;
}


bool WorldStateUtil::isPlayerPresent(edict_t* pPlayerEdict)
{
    // This is kind of a hack (checking the player has a skin) but I'm not sure how else
    // to determine whether there's currently a player there or not.  If a player was
    // playing and isn't any more, most of the old values get left behind.
    return (!FNullEnt(pPlayerEdict) && (pPlayerEdict->v.skin >= 0));
}
