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

#endif // TR_TYPES_H
