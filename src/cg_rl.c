#include "cg_rl.h"

#include "bg_public.h"
#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "q_math.h"

#define MAX_RL_TIME 15000

#define MAX_MARK_FRAGMENTS 128
#define MAX_MARK_POINTS 384
#define MAX_VERTS_ON_POLY 10

static qhandle_t line_shader;

static vmCvar_t target_draw;
static vmCvar_t target_shader;
static vmCvar_t target_size;
static vmCvar_t path_draw;
static vmCvar_t path_rgba;

static cvarTable_t rl_cvars[] = { { &target_draw, "mdd_rl_target_draw", "0", CVAR_ARCHIVE },
                                  { &target_shader, "mdd_rl_target_shader", "rlTraceMark", CVAR_ARCHIVE },
                                  { &target_size, "mdd_rl_target_size", "24", CVAR_ARCHIVE },
                                  { &path_draw, "mdd_rl_path_draw", "0", CVAR_ARCHIVE },
                                  { &path_rgba, "mdd_rl_path_rgba", "1 0 0 0", CVAR_ARCHIVE } };

void add_mark(
  qhandle_t    markShader,
  vec3_t const origin,
  vec3_t const dir,
  float        orientation,
  float        red,
  float        green,
  float        blue,
  float        alpha,
  qboolean     alphaFade,
  float        radius);

void init_rl(void)
{
  init_cvars(rl_cvars, ARRAY_LEN(rl_cvars));
  line_shader = trap_R_RegisterShader("railCore");
}

void draw_rl(void)
{
  update_cvars(rl_cvars, ARRAY_LEN(rl_cvars));

  if (!target_draw.integer && !path_draw.integer) return;

  refEntity_t beam;
  trace_t     beam_trace;
  vec3_t      origin;
  vec3_t      dest;

  snapshot_t const* const    snap = getSnap();
  playerState_t const* const ps   = getPs();

  // todo: lerp trajectory stuff?
  for (int32_t i = 0; i < snap->numEntities; ++i)
  {
    entityState_t entity = snap->entities[i];
    if (entity.eType == ET_MISSILE && entity.weapon == WP_ROCKET_LAUNCHER && entity.clientNum == ps->clientNum)
    {
      VectorMA(entity.pos.trBase, (cgs.time - entity.pos.trTime) * .001f, entity.pos.trDelta, origin);
      VectorMA(entity.pos.trBase, MAX_RL_TIME * .001f, entity.pos.trDelta, dest);
      trap_CM_BoxTrace(&beam_trace, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
      if (path_draw.integer)
      {
        vec4_t color;
        ParseVec(path_rgba.string, color, 4);

        memset(&beam, 0, sizeof(beam));
        VectorCopy(origin, beam.oldorigin);
        VectorCopy(beam_trace.endpos, beam.origin);
        beam.reType       = RT_RAIL_CORE;
        beam.customShader = line_shader;
        AxisClear(beam.axis);
        beam.shaderRGBA[0] = color[0] * 255;
        beam.shaderRGBA[1] = color[1] * 255;
        beam.shaderRGBA[2] = color[2] * 255;
        beam.shaderRGBA[3] = color[3] * 255;
        trap_R_AddRefEntityToScene(&beam);
      }
      if (target_draw.integer)
      {
        qhandle_t m_shader = trap_R_RegisterShader(target_shader.string);
        add_mark(m_shader, beam_trace.endpos, beam_trace.plane.normal, 0, 1, 1, 1, 1, qfalse, target_size.integer);
      }
    }
  }
}

// ripped CG_ImpactMark
void add_mark(
  qhandle_t    markShader,
  vec3_t const origin,
  vec3_t const dir,
  float        orientation,
  float        red,
  float        green,
  float        blue,
  float        alpha,
  qboolean     alphaFade,
  float        radius)
{
  vec3_t         axis[3];
  float          texCoordScale;
  vec3_t         originalPoints[4];
  byte           colors[4];
  int            i, j;
  int            numFragments;
  markFragment_t markFragments[MAX_MARK_FRAGMENTS], *mf;
  vec3_t         markPoints[MAX_MARK_POINTS];
  vec3_t         projection;

  // create the texture axis
  VectorNormalize2(dir, axis[0]);
  PerpendicularVector(axis[1], axis[0]);
  RotatePointAroundVector(axis[2], axis[0], axis[1], orientation);
  CrossProduct(axis[0], axis[2], axis[1]);

  texCoordScale = .5f * 1.f / radius;

  // create the full polygon
  for (i = 0; i < 3; i++)
  {
    originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
    originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
    originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
    originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
  }

  // get the fragments
  VectorScale(dir, -20, projection);
  numFragments = trap_CM_MarkFragments(
    4, (void*)originalPoints, projection, MAX_MARK_POINTS, markPoints[0], MAX_MARK_FRAGMENTS, markFragments);

  colors[0] = red * 255;
  colors[1] = green * 255;
  colors[2] = blue * 255;
  colors[3] = alpha * 255;

  for (i = 0, mf = markFragments; i < numFragments; i++, mf++)
  {
    polyVert_t* v;
    polyVert_t  verts[MAX_VERTS_ON_POLY];

    // we have an upper limit on the complexity of polygons
    // that we store persistantly
    if (mf->numPoints > MAX_VERTS_ON_POLY)
    {
      mf->numPoints = MAX_VERTS_ON_POLY;
    }
    for (j = 0, v = verts; j < mf->numPoints; j++, v++)
    {
      vec3_t delta;

      VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

      VectorSubtract(v->xyz, origin, delta);
      v->st[0]               = .5f + DotProduct(delta, axis[1]) * texCoordScale;
      v->st[1]               = .5f + DotProduct(delta, axis[2]) * texCoordScale;
      *(int32_t*)v->modulate = *(int32_t*)colors;
    }

    trap_R_AddPolyToScene(markShader, mf->numPoints, verts);
  }
}
