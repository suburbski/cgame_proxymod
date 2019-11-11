#ifndef Q_SHARED_H
#define Q_SHARED_H

#ifdef WIN32
#  include <Windows.h>
#  pragma warning(disable : 4996)
#  define QDECL __cdecl
#  ifdef linux
#    undef linux
#  endif
#  define __DLLEXPORT__ __declspec(dllexport)
#else
#  include <string.h>
#  define QDECL
#  define __DLLEXPORT__
#endif

#include <stdint.h>

/* Mod stuff */
#define DEFAULT_MODDIR "baseq3"
#define DEFAULT_VMPATH "vm/cgame.qvm"
#define GAME "Q3A"

typedef int32_t(QDECL* syscall_t)(uint32_t, ...);
typedef uint32_t (
  *pfn_t)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
typedef void (*Function)(void);
extern syscall_t g_syscall;

//=================================================

//=============================================================

typedef uint8_t byte;

typedef enum
{
  qfalse,
  qtrue
} qboolean;

typedef uint32_t qhandle_t;
typedef int32_t  sfxHandle_t;
typedef int32_t  fileHandle_t;
typedef int32_t  clipHandle_t;

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x) (ARRAY_LEN(x) - 1)

// angle indexes
#define PITCH 0 // up / down
#define YAW 1   // left / right
#define ROLL 2  // fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define MAX_STRING_CHARS 1024  // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS 1024 // max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS 1024   // max length of an individual token

//
// these aren't needed by any of the VMs.  put in another header?
//
#define MAX_MAP_AREA_BYTES 32 // bit vector of area visibility

/*
==============================================================

MATHLIB

==============================================================
*/

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef int32_t fixed4_t;
typedef int32_t fixed8_t;
typedef int32_t fixed16_t;

#ifndef M_PI
#  define M_PI 3.14159265358979323846f // matches value in gcc v2 math.h
#endif

//=============================================

// portable case insensitive compare
int Q_stricmp(char const* s1, char const* s2);
int Q_strncmp(char const* s1, char const* s2, int n);
int Q_stricmpn(char const* s1, char const* s2, int n);

//=============================================

/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define CVAR_ARCHIVE                                                                                                   \
  0x0001                       // set to cause it to be saved to vars.rc
                               // used for system variables, not for player
                               // specific configurations
#define CVAR_USERINFO 0x0002   // sent to server on connect or change
#define CVAR_SERVERINFO 0x0004 // sent in response to front end requests
#define CVAR_SYSTEMINFO 0x0008 // these cvars will be duplicated on all clients
#define CVAR_INIT                                                                                                      \
  0x0010 // don't allow change from console at all,
         // but can be set from the command line
#define CVAR_LATCH                                                                                                     \
  0x0020                         // will only change when C code next does
                                 // a Cvar_Get(), so it can't be changed
                                 // without proper initialization.  modified
                                 // will be set, even though the value hasn't
                                 // changed yet
#define CVAR_ROM 0x0040          // display only, cannot be set by user at all
#define CVAR_USER_CREATED 0x0080 // created by a set command
#define CVAR_TEMP 0x0100         // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT 0x0200        // can not be changed if cheats are disabled
#define CVAR_NORESTART 0x0400    // do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED 0x0800 // cvar was created by a server the client connected to.
#define CVAR_VM_CREATED 0x1000     // cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED 0x2000      // prevent modifying this var from VMs or the server
// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED 0x40000000    // Cvar was modified
#define CVAR_NONEXISTENT 0x80000000 // Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s cvar_t;

struct cvar_s
{
  char*    name;
  char*    string;
  char*    resetString;   // cvar_restart will reset to this value
  char*    latchedString; // for CVAR_LATCH vars
  int      flags;
  qboolean modified;          // set each time the cvar is changed
  int      modificationCount; // incremented each time the cvar is changed
  float    value;             // atof( string )
  int      integer;           // atoi( string )
  qboolean validate;
  qboolean integral;
  float    min;
  float    max;
  char*    description;

  cvar_t* next;
  cvar_t* prev;
  cvar_t* hashNext;
  cvar_t* hashPrev;
  int     hashIndex;
};

#define MAX_CVAR_VALUE_STRING 256

typedef uint32_t cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct
{
  cvarHandle_t handle;
  int32_t      modificationCount;
  float        value;
  int32_t      integer;
  char         string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define ANGLE2SHORT(x) ((int32_t)((x)*65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

#define GENTITYNUM_BITS 10 // don't need to send any more
#define MAX_GENTITIES (1 << GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define ENTITYNUM_NONE (MAX_GENTITIES - 1)
#define ENTITYNUM_WORLD (MAX_GENTITIES - 2)
#define ENTITYNUM_MAX_NORMAL (MAX_GENTITIES - 2)

//=========================================================

// bit field limits
#define MAX_STATS 16
#define MAX_PERSISTANT 16
#define MAX_POWERUPS 16
#define MAX_WEAPONS 16

#define MAX_PS_EVENTS 2

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s
{
  int32_t commandTime; // cmd->serverTime of last executed command
  int32_t pm_type;
  int32_t bobCycle; // for view bobbing and footstep generation
  int32_t pm_flags; // ducked, jump_held, etc
  int32_t pm_time;

  vec3_t  origin;
  vec3_t  velocity;
  int32_t weaponTime;
  int32_t gravity;
  int32_t speed;
  int32_t delta_angles[3]; // add to command angles to get view direction
                           // changed by spawns, rotating objects, and teleporters

  int32_t groundEntityNum; // ENTITYNUM_NONE = in air

  int32_t legsTimer; // don't change low priority animations until this runs out
  int32_t legsAnim;  // mask off ANIM_TOGGLEBIT

  int32_t torsoTimer; // don't change low priority animations until this runs out
  int32_t torsoAnim;  // mask off ANIM_TOGGLEBIT

  int32_t movementDir; // a number 0 to 7 that represents the reletive angle
                       // of movement to the view angle (axial and diagonals)
                       // when at rest, the value will remain unchanged
                       // used to twist the legs during strafing

  vec3_t grapplePoint; // location of grapple to pull towards if PMF_GRAPPLE_PULL

  int32_t eFlags; // copied to entityState_t->eFlags

  int32_t eventSequence; // pmove generated events
  int32_t events[MAX_PS_EVENTS];
  int32_t eventParms[MAX_PS_EVENTS];

  int32_t externalEvent; // events set on player from another source
  int32_t externalEventParm;
  int32_t externalEventTime;

  int32_t clientNum; // ranges from 0 to MAX_CLIENTS-1
  int32_t weapon;    // copied to entityState_t->weapon
  int32_t weaponstate;

  vec3_t  viewangles; // for fixed views
  int32_t viewheight;

  // damage feedback
  int32_t damageEvent; // when it changes, latch the other parms
  int32_t damageYaw;
  int32_t damagePitch;
  int32_t damageCount;

  int32_t stats[MAX_STATS];
  int32_t persistant[MAX_PERSISTANT]; // stats that aren't cleared on death
  int32_t powerups[MAX_POWERUPS];     // level.time that the powerup runs out
  int32_t ammo[MAX_WEAPONS];

  int32_t generic1;
  int32_t loopSound;
  int32_t jumppad_ent; // jumppad entity hit this frame

  // not communicated over the net at all
  int32_t ping;             // server to game info for scoreboard
  int32_t pmove_framecount; // FIXME: don't transmit over the network
  int32_t jumppad_frame;
  int32_t entityEventSequence;
} playerState_t;

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define SOLID_BMODEL 0xffffff

typedef enum
{
  TR_STATIONARY,
  TR_INTERPOLATE, // non-parametric, but interpolate between snapshots
  TR_LINEAR,
  TR_LINEAR_STOP,
  TR_SINE, // value = base + sin( time / duration ) * delta
  TR_GRAVITY
} trType_t;

typedef struct
{
  trType_t trType;
  int32_t  trTime;
  int32_t  trDuration; // if non 0, trTime + trDuration = stop time
  vec3_t   trBase;
  vec3_t   trDelta; // velocity, etc
} trajectory_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s
{
  int32_t number; // entity index
  int32_t eType;  // entityType_t
  int32_t eFlags;

  trajectory_t pos;  // for calculating position
  trajectory_t apos; // for calculating angles

  int32_t time;
  int32_t time2;

  vec3_t origin;
  vec3_t origin2;

  vec3_t angles;
  vec3_t angles2;

  int32_t otherEntityNum; // shotgun sources, etc
  int32_t otherEntityNum2;

  int32_t groundEntityNum; // -1 = in air

  int32_t constantLight; // r + (g<<8) + (b<<16) + (intensity<<24)
  int32_t loopSound;     // constantly loop this sound

  int32_t modelindex;
  int32_t modelindex2;
  int32_t clientNum; // 0 to (MAX_CLIENTS - 1), for players and corpses
  int32_t frame;

  int32_t solid; // for client side prediction, trap_linkentity sets this properly

  int32_t event; // impulse events -- muzzle flashes, footsteps, etc
  int32_t eventParm;

  // for players
  int32_t powerups;  // bit flags
  int32_t weapon;    // determines weapon and flash model, etc
  int32_t legsAnim;  // mask off ANIM_TOGGLEBIT
  int32_t torsoAnim; // mask off ANIM_TOGGLEBIT

  int32_t generic1;
} entityState_t;

#endif // Q_SHARED_H
