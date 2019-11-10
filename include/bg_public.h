#ifndef BG_PUBLIC_H
#define BG_PUBLIC_H

#include "cg_local.h"
#include "surfaceflags.h"

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

// content masks
#define MASK_ALL (-1)
#define MASK_SOLID (CONTENTS_SOLID)
#define MASK_PLAYERSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY)
#define MASK_DEADSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP)
#define MASK_WATER (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME)
#define MASK_OPAQUE (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)
#define MASK_SHOT (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE)

void BG_EvaluateTrajectory(trajectory_t const* tr, int32_t atTime, vec3_t result);
void BG_EvaluateTrajectoryDelta(trajectory_t const* tr, int32_t atTime, vec3_t result);

#endif // BG_PUBLIC_H
