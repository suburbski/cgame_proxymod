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

#include "q_shared.h"

// mode parm for FS_FOpenFile
typedef enum
{
  FS_READ,
  FS_WRITE,
  FS_APPEND,
  FS_APPEND_SYNC
} fsMode_t;

typedef enum
{
  FS_SEEK_CUR,
  FS_SEEK_END,
  FS_SEEK_SET
} fsOrigin_t;

typedef enum
{
  ET_GENERAL,
  ET_PLAYER,
  ET_ITEM,
  ET_MISSILE,
  ET_MOVER,
  ET_BEAM,
  ET_PORTAL,
  ET_SPEAKER,
  ET_PUSH_TRIGGER,
  ET_TELEPORT_TRIGGER,
  ET_INVISIBLE,
  ET_GRAPPLE, // grapple hooked on wall
  ET_TEAM,

  ET_EVENTS // any of the EV_* events can be added freestanding
            // by setting eType to ET_EVENTS + eventNum
            // this avoids having to set eFlags and eventNum
} entityType_t;

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

/* cg_modules */
int32_t loadModules(void);

#define MAX_STRING_CHARS 1024 // max length of a string passed to Cmd_TokenizeString
#define MAX_CONFIGSTRINGS 1024
#define MAX_GAMESTATE_CHARS 16000
#define BIG_INFO_STRING 8192 // used for system info key only

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

// markfragments are returned by CM_MarkFragments()
typedef struct
{
  int32_t firstPoint;
  int32_t numPoints;
} markFragment_t;

#define MAX_QPATH 64 // max length of a quake game pathname
#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1
typedef struct
{
  int32_t   height;      // number of scan lines
  int32_t   top;         // top of glyph in buffer
  int32_t   bottom;      // bottom of glyph in buffer
  int32_t   pitch;       // width for copying
  int32_t   xSkip;       // x adjustment
  int32_t   imageWidth;  // width of actual image
  int32_t   imageHeight; // height of actual image
  float     s;           // x offset in image where glyph starts
  float     t;           // y offset in image where glyph starts
  float     s2;
  float     t2;
  qhandle_t glyph; // handle to the shader with the glyph
  char      shaderName[32];
} glyphInfo_t;

typedef struct
{
  glyphInfo_t glyphs[GLYPHS_PER_FONT];
  float       glyphScale;
  char        name[MAX_QPATH];
} fontInfo_t;

typedef enum
{
  RT_MODEL,
  RT_POLY,
  RT_SPRITE,
  RT_BEAM,
  RT_RAIL_CORE,
  RT_RAIL_RINGS,
  RT_LIGHTNING,
  RT_PORTALSURFACE, // doesn't draw anything, just info for portals

  RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

typedef struct
{
  refEntityType_t reType;
  int32_t         renderfx;

  qhandle_t hModel; // opaque type outside refresh

  // most recent data
  vec3_t lightingOrigin; // so multi-part models can be lit identically (RF_LIGHTING_ORIGIN)
  float  shadowPlane;    // projection shadows go here, stencils go slightly lower

  vec3_t   axis[3];           // rotation vectors
  qboolean nonNormalizedAxes; // axis are not normalized, i.e. they have scale
  float    origin[3];         // also used as MODEL_BEAM's "from"
  int32_t  frame;             // also used as MODEL_BEAM's diameter

  // previous data for frame interpolation
  float   oldorigin[3]; // also used as MODEL_BEAM's "to"
  int32_t oldframe;
  float   backlerp; // 0.0 = current, 1.0 = old

  // texturing
  int32_t   skinNum;      // inline skin index
  qhandle_t customSkin;   // NULL for default skin
  qhandle_t customShader; // use one image for the entire thing

  // misc
  byte  shaderRGBA[4];     // colors used by rgbgen entity shaders
  float shaderTexCoord[2]; // texture coordinates used by tcMod entity modifiers
  float shaderTime;        // subtracted from refdef time to control effect start times

  // extra sprite information
  float radius;
  float rotation;
} refEntity_t;

#define MAX_TOKENLENGTH 1024

typedef struct pc_token_s
{
  int32_t type;
  int32_t subtype;
  int32_t intvalue;
  float   floatvalue;
  char    string[MAX_TOKENLENGTH];
} pc_token_t;

#define MAX_RENDER_STRINGS 8
#define MAX_RENDER_STRING_LENGTH 32
typedef struct
{
  int32_t x, y, width, height;
  float   fov_x, fov_y;
  vec3_t  vieworg;
  vec3_t  viewaxis[3]; // transformation matrix

  // time in milliseconds for shader effects and other time dependent rendering issues
  int32_t time;

  int32_t rdflags; // RDF_NOWORLDMODEL, etc

  // 1 bits will prevent the associated area from rendering at all
  byte areamask[MAX_MAP_AREA_BYTES];

  // text messages for deform text shaders
  char text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];
} refdef_t;

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

typedef struct
{
  vec3_t xyz;
  float  st[2];
  byte   modulate[4];
} polyVert_t;

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s
{
  int32_t     serverTime;
  int32_t     angles[3];
  int32_t     buttons;
  byte        weapon; // weapon
  signed char forwardmove, rightmove, upmove;
} usercmd_t;

// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum
{
  STAT_HEALTH,
  STAT_HOLDABLE_ITEM,
  STAT_WEAPONS, // 16 bit fields
  STAT_ARMOR,
  STAT_DEAD_YAW,      // look this direction when dead (FIXME: get rid of?)
  STAT_CLIENTS_READY, // bit mask of clients wishing to exit the intermission (FIXME: configstring?)
  STAT_MAX_HEALTH     // health / armor limit, changable by handicap
} statIndex_t;

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

  WP_NUM_WEAPONS
} weapon_t;

// trace stuff
typedef struct cplane_s
{
  vec3_t normal;
  float  dist;
  byte   type;     // for fast side tests: 0,1,2 = axial, 3 = nonaxial
  byte   signbits; // signx + (signy<<1) + (signz<<2), used as lookup during collision
  byte   pad[2];
} cplane_t;

// a trace is returned when a box is swept through the world
typedef struct
{
  qboolean allsolid;     // if true, plane is not valid
  qboolean startsolid;   // if true, the initial point was in a solid area
  float    fraction;     // time completed, 1.0 = didn't hit anything
  vec3_t   endpos;       // final position
  cplane_t plane;        // surface normal at impact, transformed to world space
  int      surfaceFlags; // surface hit
  int      contents;     // contents on other side of surface hit
  int      entityNum;    // entity the contacted surface is a part of
} trace_t;

#endif // CG_LOCAL_H
