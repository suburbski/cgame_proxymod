#ifndef BG_PUBLIC_H
#define BG_PUBLIC_H

#include "q_shared.h"

#define DEFAULT_GRAVITY 800

#define MAX_ITEMS 256

#define LIGHTNING_RANGE 768

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define CS_LEVEL_START_TIME 21 // so the timer only shows the current level

#define CS_ITEMS 27 // string of 0's and 1's that tell which items are present

#define CS_MODELS 32

// pmove->pm_flags
#define PMF_DUCKED 1
#define PMF_JUMP_HELD 2
#define PMF_BACKWARDS_JUMP 8   // go into backwards land
#define PMF_BACKWARDS_RUN 16   // coast down to backwards run
#define PMF_TIME_LAND 32       // pm_time is time before rejump
#define PMF_TIME_KNOCKBACK 64  // pm_time is an air-accelerate only time
#define PMF_TIME_WATERJUMP 256 // pm_time is waterjump
#define PMF_RESPAWNED 512      // clear after attack and jump buttons come up
#define PMF_USE_ITEM_HELD 1024
#define PMF_GRAPPLE_PULL 2048 // pull towards grapple location
#define PMF_FOLLOW 4096       // spectate following another player
#define PMF_SCOREBOARD 8192   // spectate as a scoreboard
#define PMF_INVULEXPAND 16384 // invulnerability sphere set to full size

#define PMF_ALL_TIMES (PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_KNOCKBACK)

//===================================================================================

// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum
{
  STAT_HEALTH,
  STAT_HOLDABLE_ITEM,
#ifdef MISSIONPACK
  STAT_PERSISTANT_POWERUP,
#endif
  STAT_WEAPONS, // 16 bit fields
  STAT_ARMOR,
  STAT_DEAD_YAW,      // look this direction when dead (FIXME: get rid of?)
  STAT_CLIENTS_READY, // bit mask of clients wishing to exit the intermission (FIXME: configstring?)
  STAT_MAX_HEALTH     // health / armor limit, changable by handicap
} statIndex_t;

// entityState_t->eFlags
#define EF_DEAD 0x00000001            // don't draw a foe marker over players with EF_DEAD
#define EF_TELEPORT_BIT 0x00000004    // toggled every time the origin abruptly changes
#define EF_AWARD_EXCELLENT 0x00000008 // draw an excellent sprite
#define EF_PLAYER_EVENT 0x00000010
#define EF_BOUNCE 0x00000010         // for missiles
#define EF_BOUNCE_HALF 0x00000020    // for missiles
#define EF_AWARD_GAUNTLET 0x00000040 // draw a gauntlet sprite
#define EF_NODRAW 0x00000080         // may have an event, but no model (unspawned items)
#define EF_FIRING 0x00000100         // for lightning gun
#define EF_KAMIKAZE 0x00000200
#define EF_MOVER_STOP 0x00000400       // will push otherwise
#define EF_AWARD_CAP 0x00000800        // draw the capture sprite
#define EF_TALK 0x00001000             // draw a talk balloon
#define EF_CONNECTION 0x00002000       // draw a connection trouble sprite
#define EF_VOTED 0x00004000            // already cast a vote
#define EF_AWARD_IMPRESSIVE 0x00008000 // draw an impressive sprite
#define EF_AWARD_DEFEND 0x00010000     // draw a defend sprite
#define EF_AWARD_ASSIST 0x00020000     // draw a assist sprite
#define EF_AWARD_DENIED 0x00040000     // denied
#define EF_TEAMVOTED 0x00080000        // already cast a team vote

// NOTE: may not have more than 16
typedef enum
{
  PW_NONE,

  PW_QUAD,
  PW_BATTLESUIT,
  PW_HASTE,
  PW_INVIS,
  PW_REGEN,
  PW_FLIGHT,

  PW_REDFLAG,
  PW_BLUEFLAG,
  PW_NEUTRALFLAG,

  PW_SCOUT,
  PW_GUARD,
  PW_DOUBLER,
  PW_AMMOREGEN,
  PW_INVULNERABILITY,

  PW_NUM_POWERUPS
} powerup_t;

typedef enum
{
  WP_NONE,

  WP_GAUNTLET,
  WP_MACHINEGUN,
  WP_SHOTGUN,
  WP_GRENADE_LAUNCHER,
  WP_ROCKET_LAUNCHER,
  WP_LIGHTNING,
  WP_RAILGUN,
  WP_PLASMAGUN,
  WP_BFG,
  WP_GRAPPLING_HOOK,
#ifdef MISSIONPACK
  WP_NAILGUN,
  WP_PROX_LAUNCHER,
  WP_CHAINGUN,
#endif

  WP_NUM_WEAPONS
} weapon_t;

// content masks
#define MASK_ALL (-1)
#define MASK_SOLID (CONTENTS_SOLID)
#define MASK_PLAYERSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY)
#define MASK_DEADSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP)
#define MASK_WATER (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME)
#define MASK_OPAQUE (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)
#define MASK_SHOT (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE)

//
// entityState_t->eType
//
typedef enum
{
  ET_GENERAL,
  ET_PLAYER,
  ET_ITEM,
  ET_MISSILE,
  ET_MOVER,
  ET_BREAKABLE,
  ET_BEAM,
  ET_PORTAL,
  ET_SPEAKER,
  ET_PUSH_TRIGGER,
  ET_TELEPORT_TRIGGER,
  ET_PHYSICS_TRIGGER,
  ET_INVISIBLE,
  ET_GRAPPLE, // grapple hooked on wall
  ET_SPAWNPOINT,
  ET_SPECTATOR,
  ET_TEAM,

  ET_EVENTS // any of the EV_* events can be added freestanding
            // by setting eType to ET_EVENTS + eventNum
            // this avoids having to set eFlags and eventNum
} entityType_t;

void BG_EvaluateTrajectory(trajectory_t const* tr, int32_t atTime, vec3_t result);
void BG_EvaluateTrajectoryDelta(trajectory_t const* tr, int32_t atTime, vec3_t result);

void BG_PlayerStateToEntityState(playerState_t const* pm_ps, entityState_t* s, qboolean snap);

#endif // BG_PUBLIC_H
