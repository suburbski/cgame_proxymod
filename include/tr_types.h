#ifndef TR_TYPES_H
#define TR_TYPES_H

#include "q_shared.h"

typedef struct
{
  vec3_t xyz;
  float  st[2];
  byte   modulate[4];
} polyVert_t;

typedef struct poly_s
{
  qhandle_t   hShader;
  int32_t     numVerts;
  polyVert_t* verts;
} poly_t;

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

typedef enum
{
  STEREO_CENTER,
  STEREO_LEFT,
  STEREO_RIGHT
} stereoFrame_t;

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

#endif // TR_TYPES_H
