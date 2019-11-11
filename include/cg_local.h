/*
  ==============================
  Written by id software, nightmare and hk of mdd
  This file is part of mdd client proxymod.

  mdd client proxymod is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  mdd client proxymod is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mdd client proxymod.  If not, see <http://www.gnu.org/licenses/>.
  ==============================
  Note: mdd client proxymod contains large quantities from the quake III arena source code
*/
#ifndef CG_LOCAL_H
#define CG_LOCAL_H

#include "cg_public.h"
#include "q_shared.h"
#include "tr_types.h"

/* cg_vm.c */
int32_t callVM(
  int32_t cmd,
  int32_t arg0,
  int32_t arg1,
  int32_t arg2,
  int32_t arg3,
  int32_t arg4,
  int32_t arg5,
  int32_t arg6,
  int32_t arg7,
  int32_t arg8,
  int32_t arg9,
  int32_t arg10,
  int32_t arg11);
int32_t callVM_Destroy(void);
int32_t setVMPtr(int32_t arg0);
int32_t initVM(void);

//======================================================================

#define MAX_CONFIGSTRINGS 1024
#define MAX_GAMESTATE_CHARS 16000

typedef struct
{
  int32_t stringOffsets[MAX_CONFIGSTRINGS];
  char    stringData[MAX_GAMESTATE_CHARS];
  int32_t dataCount;
} gameState_t;

/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum
{
  TC_NONE,
  TC_S3TC
} textureCompression_t;

typedef enum
{
  GLDRV_ICD,        // driver is integrated with window system
                    // WARNING: there are tests that check for
                    // > GLDRV_ICD for minidriverness, so this
                    // should always be the lowest value in this
                    // enum set
  GLDRV_STANDALONE, // driver is a non-3Dfx standalone driver
  GLDRV_VOODOO      // driver is a 3Dfx standalone driver
} glDriverType_t;

typedef enum
{
  GLHW_GENERIC,   // where everthing works the way it should
  GLHW_3DFX_2D3D, // Voodoo Banshee or Voodoo3, relevant since if this is
                  // the hardware type then there can NOT exist a secondary
                  // display adapter
  GLHW_RIVA128,   // where you can't interpolate alpha
  GLHW_RAGEPRO,   // where you can't modulate alpha on alpha textures
  GLHW_PERMEDIA2  // where you don't have src*dst
} glHardwareType_t;

typedef struct
{
  char renderer_string[MAX_STRING_CHARS];
  char vendor_string[MAX_STRING_CHARS];
  char version_string[MAX_STRING_CHARS];
  char extensions_string[BIG_INFO_STRING];

  int32_t maxTextureSize;    // queried from GL
  int32_t maxActiveTextures; // multitexture ability

  int32_t colorBits, depthBits, stencilBits;

  glDriverType_t   driverType;
  glHardwareType_t hardwareType;

  qboolean             deviceSupportsGamma;
  textureCompression_t textureCompression;
  qboolean             textureEnvAddAvailable;

  int32_t vidWidth, vidHeight;
  // aspect is the screen's physical width / height, which may be different
  // than scrWidth / scrHeight if the pixels are non-square
  // normal screens should be 4/3, but wide aspect monitors may be 16/9
  float windowAspect;

  int32_t displayFrequency;

  // synonymous with "does rendering consume the entire screen?", therefore
  // a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
  // used CDS.
  qboolean isFullscreen;
  qboolean stereoEnabled;
  qboolean smpActive; // dual processor
} glconfig_t;

typedef struct
{
  /* GFX Handles */
  qhandle_t gfxDeferSymbol;
  qhandle_t gfxWhiteShader;
  qhandle_t gfxCharsetShader;
  qhandle_t gfxCharsetProp;
  qhandle_t gfxCharsetPropGlow;
  qhandle_t gfxCharsetPropB;
} cgMedia_t;

typedef struct
{
  /* from cgs_t */
  gameState_t gameState; // gamestate from server
  glconfig_t  glconfig;  // rendering configuration
  int32_t     clientNum;
  float       screenXScale; // derived from glconfig
  float       screenYScale;
  int32_t     levelStartTime;
  float       screenXBias;

  /* from cg */
  int32_t time;

  cgMedia_t media;
} cgs_t;

extern cgs_t cgs;

//
// cg_main.c
//
char const* CG_ConfigString(int32_t index);

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand(void);
void     CG_InitConsoleCommands(void);

//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void trap_Print(char const* fmt);

// abort the game
void trap_Error(char const* fmt);

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int32_t trap_Milliseconds(void);

// console variable interaction
void trap_Cvar_Register(vmCvar_t* vmCvar, char const* varName, char const* defaultValue, int32_t flags);
void trap_Cvar_Update(vmCvar_t* vmCvar);
void trap_Cvar_Set(char const* var_name, char const* value);
void trap_Cvar_VariableStringBuffer(char const* var_name, char* buffer, int32_t bufsize);

// ServerCommand and ConsoleCommand parameter access
int32_t trap_Argc(void);
void    trap_Argv(int32_t n, char* buffer, int32_t bufferLength);
void    trap_Args(char* buffer, int32_t bufferLength);

// filesystem access
// returns length of file
int32_t trap_FS_FOpenFile(char const* qpath, fileHandle_t* f, fsMode_t mode);
void    trap_FS_Read(void* buffer, int32_t len, fileHandle_t f);
void    trap_FS_Write(void const* buffer, int32_t len, fileHandle_t f);
void    trap_FS_FCloseFile(fileHandle_t f);
int32_t trap_FS_Seek(fileHandle_t f, long offset, int32_t origin); // fsOrigin_t

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void trap_AddCommand(char const* cmdName);
void trap_RemoveCommand(char const* cmdName);

// model collision
int32_t trap_CM_NumInlineModels(void);
int32_t trap_CM_PointContents(vec3_t const p, clipHandle_t model);
void    trap_CM_BoxTrace(
     trace_t*     results,
     vec3_t const start,
     vec3_t const end,
     vec3_t const mins,
     vec3_t const maxs,
     clipHandle_t model,
     int32_t      brushmask);

// Returns the projection of a polygon onto the solid brushes in the world
int32_t trap_CM_MarkFragments(
  int32_t         numPoints,
  vec3_t const*   points,
  vec3_t const    projection,
  int32_t         maxPoints,
  vec3_t          pointBuffer,
  int32_t         maxFragments,
  markFragment_t* fragmentBuffer);

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t trap_R_RegisterModel(char const* name);       // returns rgb axis if not found
qhandle_t trap_R_RegisterSkin(char const* name);        // returns all white if not found
qhandle_t trap_R_RegisterShader(char const* name);      // returns all white if not found
qhandle_t trap_R_RegisterShaderNoMip(char const* name); // returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void trap_R_ClearScene(void);
void trap_R_AddRefEntityToScene(refEntity_t const* re);

// polys are intended for simple wall marks, not really for doing
// significant construction
void    trap_R_AddPolyToScene(qhandle_t hShader, int32_t numVerts, polyVert_t const* verts);
void    trap_R_AddPolysToScene(qhandle_t hShader, int32_t numVerts, polyVert_t const* verts, int32_t numPolys);
void    trap_R_AddLightToScene(vec3_t const org, float intensity, float r, float g, float b);
void    trap_R_AddAdditiveLightToScene(vec3_t const org, float intensity, float r, float g, float b);
int32_t trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir);
void    trap_R_RenderScene(refdef_t const* fd);
void    trap_R_SetColor(float const* rgba); // NULL = 1,1,1,1
void    trap_R_DrawStretchPic(
     float     x,
     float     y,
     float     w,
     float     h,
     float     s1,
     float     t1,
     float     s2,
     float     t2,
     qhandle_t hShader);
void trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void trap_GetGlconfig(glconfig_t* glconfig);

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void trap_GetGameState(gameState_t* gamestate);

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void trap_GetCurrentSnapshotNumber(int32_t* snapshotNumber, int32_t* serverTime);

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean trap_GetSnapshot(int32_t snapshotNumber, snapshot_t* snapshot);

qboolean trap_GetEntityToken(char* buffer, int32_t bufferSize);

#define PROP_GAP_WIDTH 3
#define PROP_SPACE_WIDTH 8
#define PROP_HEIGHT 27
#define PROP_SMALL_SIZE_SCALE 0.75

#define BLINK_DIVISOR 200
#define PULSE_DIVISOR 75

#define UI_LEFT 0x00000000 // default
#define UI_CENTER 0x00000001
#define UI_RIGHT 0x00000002
#define UI_FORMATMASK 0x00000007
#define UI_SMALLFONT 0x00000010
#define UI_BIGFONT 0x00000020 // default
#define UI_GIANTFONT 0x00000040
#define UI_DROPSHADOW 0x00000800
#define UI_BLINK 0x00001000
#define UI_INVERSE 0x00002000
#define UI_PULSE 0x00004000

typedef struct qtime_s
{
  int32_t tm_sec;   /* seconds after the minute - [0,59] */
  int32_t tm_min;   /* minutes after the hour - [0,59] */
  int32_t tm_hour;  /* hours since midnight - [0,23] */
  int32_t tm_mday;  /* day of the month - [1,31] */
  int32_t tm_mon;   /* months since January - [0,11] */
  int32_t tm_year;  /* years since 1900 */
  int32_t tm_wday;  /* days since Sunday - [0,6] */
  int32_t tm_yday;  /* days since January 1 - [0,365] */
  int32_t tm_isdst; /* daylight savings time flag */
} qtime_t;

#endif // CG_LOCAL_H
