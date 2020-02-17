//
// $Id: NSConstants.h,v 1.40 2005/11/30 23:51:58 clamatius Exp $

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

#ifndef __NSCONSTANTS_H
#define __NSCONSTANTS_H

#define NO_TEAM 0
#define MARINE_TEAM 1
#define ALIEN_TEAM 2

typedef enum
{
    kInvalidEvolution = -1,
    kSkulk = 0,
    kLerk = 1,
    kGorge = 2,
    kFade = 3,
    kOnos = 4
} tEvolution;

extern const char* lifeformNames[];

const int MAX_PACKS = 8;

typedef enum
{
	kGorgeRole,
	kScoutRole,
	kPack1Role,
	kPack2Role,
	kPack3Role,
	kPack4Role,
	kPack5Role,
	kPack6Role,
	kPack7Role,
	kPack8Role
} tBotRole;

const int MARINE_NODE_MASK = 1;
const int ALIEN_NODE_MASK = 2;
const int OCCUPIED_NODE_MASK = MARINE_NODE_MASK | ALIEN_NODE_MASK;

// Distance from origin to bottom of creature
static const float g_CreatureOriginHeights[] = {20, 20, 20, 30,   40};

static const float g_CreatureMaxArmour[] =     {10, 30, 40, 100, 500};
// TODO: these may be out of date in 2.01, gotta check
static const float g_CreatureArmourUpgrade[] = {20, 25, 50,  50, 100};

#define MIN_WALK_EVOLUTION (kLerk)
#define MAX_WALK_EVOLUTION (kOnos)
#define NUM_EVOLUTIONS 5

#define BOT_PITCH_SPEED 40
#define BOT_YAW_SPEED 40

#define IMPULSE_CHUCKLE "chuckle"
#define IMPULSE_CARAPACE "carapace"
#define IMPULSE_REGENERATION "regeneration"
#define IMPULSE_REDEMPTION "redemption"

#define IMPULSE_CELERITY "celerity"
#define IMPULSE_ADRENALINE "adrenaline"
#define IMPULSE_SILENCE "silence"

#define IMPULSE_CLOAK "cloak"
#define IMPULSE_SCENT_OF_FEAR "sof"
#define IMPULSE_FOCUS "focus"

#define IMPULSE_LEAP "leap"
#define IMPULSE_BLINK "blink"
#define IMPULSE_CHARGE "charge"

#define IMPULSE_EVOLVE_TO_SKULK "skulk"
#define IMPULSE_EVOLVE_TO_GORGE "gorge"
#define IMPULSE_EVOLVE_TO_LERK "lerk"
#define IMPULSE_EVOLVE_TO_FADE "fade"
#define IMPULSE_EVOLVE_TO_ONOS "onos"

#define IMPULSE_HIVE2 "hive2"
#define IMPULSE_HIVE3 "hive3"

#define IMPULSE_BUILD_ALIEN_RESOURCE_TOWER "resource"
#define IMPULSE_BUILD_ALIEN_OFFENSE_CHAMBER "offense"
#define IMPULSE_BUILD_ALIEN_DEFENSE_CHAMBER "defense"
#define IMPULSE_BUILD_ALIEN_SENSORY_CHAMBER "sensory"
#define IMPULSE_BUILD_ALIEN_MOVEMENT_CHAMBER "movement"
#define IMPULSE_BUILD_HIVE "hive"

static const char* EVOLUTION_IMPULSES[] = 
{ IMPULSE_EVOLVE_TO_SKULK, 
  IMPULSE_EVOLVE_TO_LERK,
  IMPULSE_EVOLVE_TO_GORGE,
  IMPULSE_EVOLVE_TO_FADE,
  IMPULSE_EVOLVE_TO_ONOS };

#define DEFENSE_TRAIT 1
#define MOVEMENT_TRAIT 3
#define SENSORY_TRAIT 4

#define UPGRADE_CARAPACE (1 << 0)
#define UPGRADE_REGENERATION (1 << 1)
#define UPGRADE_REDEMPTION (1 << 2)
#define UPGRADE_CELERITY (1 << 3)
#define UPGRADE_ADRENALINE (1 << 4)
#define UPGRADE_SILENCE (1 << 5)
#define UPGRADE_CLOAK (1 << 6)
#define UPGRADE_SCENT_OF_FEAR (1 << 7)
#define UPGRADE_FOCUS (1 << 8)
#define UPGRADE_ABILITY_3 (1 << 9)
#define UPGRADE_ABILITY_4 (1 << 10)
#define FORM_CHANGE (1 << 11)

#define DEFENSE_TRAIT_BITMASK (UPGRADE_CARAPACE | UPGRADE_REGENERATION | UPGRADE_REDEMPTION)
#define MOVEMENT_TRAIT_BITMASK (UPGRADE_CELERITY | UPGRADE_ADRENALINE | UPGRADE_SILENCE)
#define SENSORY_TRAIT_BITMASK (UPGRADE_CLOAK | UPGRADE_SCENT_OF_FEAR | UPGRADE_FOCUS)

const float MAX_JUMP_HEIGHT = 60.0; // world units
const float ERROR_MARGIN = 10.0;
const float MAX_HEIGHT_WALKABLE = MAX_JUMP_HEIGHT + ERROR_MARGIN;
const float CHECK_WALKABLE_RESOLUTION = 2.0; // world units

// bitmask weapon types
#define ANTI_BUILDING 1
#define ANTI_PLAYER 2
#define MELEE_ATTACK 4
#define HELPS_FRIENDLY 8

// From the NS mini-SDK

// stored in iuser4
typedef enum
{
	MASK_NONE = 0uL,
	MASK_VIS_SIGHTED = 1uL,					// This means this is an entity that can be seen by at least one member of the opposing team.  Assumes commanders can never be seen.
	MASK_VIS_DETECTED = 2uL,				// This entity has been detected by the other team but isn't currently seen
	MASK_BUILDABLE = 4uL,					// This entity is buildable
	MASK_UPGRADE_1 = 8uL,					// Marine weapons 1, armor, marine basebuildable slot #0
	MASK_UPGRADE_2 = 16uL,					// Marine weapons 2, regen, marine basebuildable slot #1
	MASK_UPGRADE_3 = 32uL,					// Marine weapons 3, redemption, marine basebuildable slot #2
	MASK_UPGRADE_4 = 64uL,					// Marine armor 1, speed, marine basebuildable slot #3
	MASK_UPGRADE_5 = 128uL,					// Marine armor 2, adrenaline, marine basebuildable slot #4
	MASK_UPGRADE_6 = 256uL,					// Marine armor 3, silence, marine basebuildable slot #5
	MASK_UPGRADE_7 = 512uL,					// Marine jetpacks, Cloaking, marine basebuildable slot #6
	MASK_UPGRADE_8 = 1024uL,				// Pheromone, motion-tracking, marine basebuildable slot #7
	MASK_UPGRADE_9 = 2048uL,				// Scent of fear, exoskeleton
	MASK_UPGRADE_10 = 4096uL,				// Defensive level 2, power armor
	MASK_UPGRADE_11 = 8192uL,				// Defensive level 3, electrical defense
	MASK_UPGRADE_12 = 16384uL,				// Movement level 2, 
	MASK_UPGRADE_13 = 32768uL,				// Movement level 3, marine heavy armor
	MASK_UPGRADE_14 = 65536uL,				// Sensory level 2
	MASK_UPGRADE_15 = 131072uL,				// Sensory level 3
	MASK_ALIEN_MOVEMENT = 262144uL,			// Onos is charging
	MASK_WALLSTICKING = 524288uL,  			// Flag for wall-sticking
	MASK_PRIMALSCREAM = 1048576uL,			// Alien is in range of active primal scream
	MASK_UMBRA = 2097152uL,
	MASK_DIGESTING = 4194304uL,				// When set on a visible player, player is digesting.  When set on invisible player, player is being digested
	MASK_RECYCLING = 8388608uL,
	MASK_TOPDOWN = 16777216uL,
	MASK_PLAYER_STUNNED = 33554432uL,		// Player has been stunned by stomp
	MASK_ENSNARED = 67108864uL,
	MASK_ALIEN_EMBRYO = 268435456uL,
	MASK_SELECTABLE = 536870912uL,
	MASK_PARASITED = 1073741824uL,
	MASK_ALIEN_CLOAKED = 2147483648uL
} AvHUpgradeMask;

const float ELECTRIFIED_RANGE = 250.0;

typedef enum
{
	PLAYERCLASS_NONE = 0,
	PLAYERCLASS_ALIVE_MARINE,
	PLAYERCLASS_ALIVE_HEAVY_MARINE,
	PLAYERCLASS_ALIVE_LEVEL1,
	PLAYERCLASS_ALIVE_LEVEL2,
	PLAYERCLASS_ALIVE_LEVEL3,
	PLAYERCLASS_ALIVE_LEVEL4,
	PLAYERCLASS_ALIVE_LEVEL5,
	PLAYERCLASS_ALIVE_GESTATING,
	PLAYERCLASS_ALIVE_DIGESTING,
	PLAYERCLASS_DEAD_MARINE,
	PLAYERCLASS_DEAD_ALIEN,
	PLAYERCLASS_COMMANDER,
	PLAYERCLASS_REINFORCING,
	PLAYERCLASS_SPECTATOR
} AvHPlayerClass;

typedef enum
{
	WEAPON_NONE = 0,

	// Alien weapons
	WEAPON_CLAWS,
	WEAPON_SPIT,
	WEAPON_SPORES,
	WEAPON_SPIKE,
	WEAPON_BITE,		// Level 1 bite
	WEAPON_BITE2,		// Level 3 bite
	WEAPON_SWIPE,
	WEAPON_WEBSPINNER,
	WEAPON_METABOLIZE,
	WEAPON_PARASITE,
	WEAPON_BLINK,
	WEAPON_DIVINEWIND,

	WEAPON_KNIFE,
	WEAPON_PISTOL,
	WEAPON_MG,
	WEAPON_SONIC,
	WEAPON_HMG,
	WEAPON_WELDER,
	WEAPON_MINE,
	WEAPON_GRENADE_GUN,

	// Abilities
	WEAPON_LEAP,
	WEAPON_CHARGE,

	WEAPON_UMBRA,
	WEAPON_PRIMALSCREAM,
	WEAPON_BILEBOMB,
	WEAPON_ACIDROCKET,
	WEAPON_HEALINGSPRAY,
	WEAPON_BABBLER,
	WEAPON_STOMP,
	WEAPON_DEVOUR,

	// Can't go over 32 (client.cpp, GetWeaponData())

	WEAPON_MAX
}
AvHWeaponID;

const tEvolution EVOLUTIONS[] = 
{ kInvalidEvolution, kInvalidEvolution, kInvalidEvolution, 
  kSkulk, kGorge, kLerk, kFade, 
  kOnos, kInvalidEvolution, kInvalidEvolution, kInvalidEvolution, kInvalidEvolution, kInvalidEvolution, kInvalidEvolution, kInvalidEvolution};

#endif // __NSCONSTANTS_H
