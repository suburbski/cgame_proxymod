#ifndef Q_SHARED_H
#define Q_SHARED_H

#include <stdint.h>

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

#endif // Q_SHARED_H
