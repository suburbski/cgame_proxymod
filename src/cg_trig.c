#include "cg_trig.h"

#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"

#include <stdlib.h>

#define MAX_SUBMODELS 256

static vmCvar_t trig;
static vmCvar_t trig_rgba;

static cvarTable_t trig_cvars[] = { { &trig, "mdd_trig", "0", CVAR_ARCHIVE },
                                    { &trig_rgba, "mdd_trig_rgba", "0 .5 0 1", CVAR_ARCHIVE } };

typedef struct
{
  qhandle_t bboxShader;
  qhandle_t bboxShader_nocull;

  qboolean trigger[MAX_SUBMODELS];

  vec4_t rgba;
} trig_t;

static trig_t trig_;

static void parse_triggers(void);
static void R_DrawBBox(vec3_t const origin, vec3_t const mins, vec3_t const maxs);

void init_trig(void)
{
  init_cvars(trig_cvars, ARRAY_LEN(trig_cvars));
  trig_.bboxShader        = trap_R_RegisterShader("bbox");
  trig_.bboxShader_nocull = trap_R_RegisterShader("bbox_nocull");
  memset(trig_.trigger, 0, sizeof(trig_.trigger));
  parse_triggers();
}

void draw_trig(void)
{
  update_cvars(trig_cvars, ARRAY_LEN(trig_cvars));

  if (!trig.integer) return;

  ParseVec(trig_rgba.string, trig_.rgba, 4);

  int const num_models = trap_CM_NumInlineModels();
  for (int i = 0; i < num_models; ++i)
  {
    if (trig_.trigger[i])
    {
      vec3_t mins;
      vec3_t maxs;
      trap_R_ModelBounds(i + 1, mins, maxs);
      R_DrawBBox(vec3_origin, mins, maxs);
    }
  }
}

// ripped from breadsticks
static void parse_triggers(void)
{
  char token[MAX_TOKEN_CHARS];
  for (;;)
  {
    qboolean is_trigger = qfalse;
    int      model      = -1;

    if (!trap_GetEntityToken(token, sizeof(token))) break;

    if (token[0] != '{') trap_Error("mape is borked");

    for (;;)
    {
      trap_GetEntityToken(token, sizeof(token));

      if (token[0] == '}') break;

      if (!strcmp(token, "model"))
      {
        trap_GetEntityToken(token, sizeof(token));
        if (token[0] == '*') model = atoi(token + 1);
      }

      if (!strcmp(token, "classname"))
      {
        trap_GetEntityToken(token, sizeof(token));
        is_trigger = !!strstr(token, "trigger");
      }
    }

    if (is_trigger && model > 0) trig_.trigger[model] = qtrue; // why +1? idk
  }
}

// ripped from breadsticks
static void R_DrawBBox(vec3_t const origin, vec3_t const mins, vec3_t const maxs)
{
  polyVert_t verts[4];
  vec3_t     corners[8];

  // get the extents (size)
  float const extx = maxs[0] - mins[0];
  float const exty = maxs[1] - mins[1];
  float const extz = maxs[2] - mins[2];

  // set the polygon's texture coordinates
  verts[0].st[0] = 0;
  verts[0].st[1] = 0;
  verts[1].st[0] = 0;
  verts[1].st[1] = 1;
  verts[2].st[0] = 1;
  verts[2].st[1] = 1;
  verts[3].st[0] = 1;
  verts[3].st[1] = 0;

  // set the polygon's vertex colors
  for (uint8_t i = 0; i < 4; ++i)
  {
    // memcpy( verts[i].modulate, color, sizeof(verts[i].modulate) );
    verts[i].modulate[0] = trig_.rgba[0] * 255;
    verts[i].modulate[1] = trig_.rgba[1] * 255;
    verts[i].modulate[2] = trig_.rgba[2] * 255;
    verts[i].modulate[3] = trig_.rgba[3] * 255;
  }

  VectorAdd(origin, maxs, corners[3]);

  VectorCopy(corners[3], corners[2]);
  corners[2][0] -= extx;

  VectorCopy(corners[2], corners[1]);
  corners[1][1] -= exty;

  VectorCopy(corners[1], corners[0]);
  corners[0][0] += extx;

  for (uint8_t i = 0; i < 4; ++i)
  {
    VectorCopy(corners[i], corners[i + 4]);
    corners[i + 4][2] -= extz;
  }

  // top
  VectorCopy(corners[0], verts[0].xyz);
  VectorCopy(corners[1], verts[1].xyz);
  VectorCopy(corners[2], verts[2].xyz);
  VectorCopy(corners[3], verts[3].xyz);
  trap_R_AddPolyToScene(trig_.bboxShader, 4, verts);

  // bottom
  VectorCopy(corners[7], verts[0].xyz);
  VectorCopy(corners[6], verts[1].xyz);
  VectorCopy(corners[5], verts[2].xyz);
  VectorCopy(corners[4], verts[3].xyz);
  trap_R_AddPolyToScene(trig_.bboxShader, 4, verts);

  // top side
  VectorCopy(corners[3], verts[0].xyz);
  VectorCopy(corners[2], verts[1].xyz);
  VectorCopy(corners[6], verts[2].xyz);
  VectorCopy(corners[7], verts[3].xyz);
  trap_R_AddPolyToScene(trig_.bboxShader_nocull, 4, verts);

  // left side
  VectorCopy(corners[2], verts[0].xyz);
  VectorCopy(corners[1], verts[1].xyz);
  VectorCopy(corners[5], verts[2].xyz);
  VectorCopy(corners[6], verts[3].xyz);
  trap_R_AddPolyToScene(trig_.bboxShader_nocull, 4, verts);

  // right side
  VectorCopy(corners[0], verts[0].xyz);
  VectorCopy(corners[3], verts[1].xyz);
  VectorCopy(corners[7], verts[2].xyz);
  VectorCopy(corners[4], verts[3].xyz);
  trap_R_AddPolyToScene(trig_.bboxShader_nocull, 4, verts);

  // bottom side
  VectorCopy(corners[1], verts[0].xyz);
  VectorCopy(corners[0], verts[1].xyz);
  VectorCopy(corners[4], verts[2].xyz);
  VectorCopy(corners[5], verts[3].xyz);
  trap_R_AddPolyToScene(trig_.bboxShader_nocull, 4, verts);
}
