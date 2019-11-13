#include "cg_gl.h"

#include "bg_public.h"
#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "g_local.h"
#include "nade_tracking.h"

static vmCvar_t gl_path_draw;
static vmCvar_t gl_path_rgba;
static vmCvar_t gl_path_preview_draw;
static vmCvar_t gl_path_preview_rgba;

static cvarTable_t gl_cvars[] = {
  { &gl_path_draw, "mdd_gl_path_draw", "1", CVAR_ARCHIVE },
  { &gl_path_rgba, "mdd_gl_path_rgba", "0 1 0 1", CVAR_ARCHIVE },
  { &gl_path_preview_draw, "mdd_gl_path_preview_draw", "1", CVAR_ARCHIVE },
  { &gl_path_preview_rgba, "mdd_gl_path_preview_rgba", "0 .5 0 1", CVAR_ARCHIVE },
};

static qhandle_t beam_shader;

void init_gl(void)
{
  init_cvars(gl_cvars, ARRAY_LEN(gl_cvars));
  beam_shader = trap_R_RegisterShader("railCore");
}

static void draw_nade_path(trajectory_t const* pos, int end_time, uint8_t const* color);

void draw_gl(void)
{
  entityState_t entity;
  uint8_t       path_color[4];
  uint8_t       preview_color[4];
  vec4_t        color;

  playerState_t const* const ps = getPs();
  if (ps->weapon != WP_GRENADE_LAUNCHER) return;

  update_cvars(gl_cvars, ARRAY_LEN(gl_cvars));

  snapshot_t const* const snap = getSnap();

  if (gl_path_preview_draw.integer)
  {
    ParseVec(gl_path_preview_rgba.string, color, 4);
    for (uint8_t i = 0; i < 4; ++i) preview_color[i] = color[i] * 255;

    FireWeapon(ps, &entity);
    draw_nade_path(&entity.pos, cg.time + 2500, preview_color);
  }

  if (!gl_path_draw.integer) return;

  ParseVec(gl_path_rgba.string, color, 4);
  for (uint8_t i = 0; i < 4; ++i) path_color[i] = color[i] * 255;

  for (uint8_t i = 0; i < MAX_NADES; ++i)
  {
    if (nades[i].id >= 0 && nades[i].seen)
    {
      for (int j = 0; j < snap->numEntities; ++j)
      {
        entityState_t const* const entity = &snap->entities[j];
        if (entity->number != nades[i].id) continue;
        draw_nade_path(&entity->pos, nades[i].explode_time, path_color);
      }
    }
  }
}

static void draw_nade_path(trajectory_t const* pos, int end_time, uint8_t const* color)
{
  refEntity_t beam;
  trace_t     trace;
  int         sample_timer = 0;
  vec3_t      currentOrigin, origin;

  if (pos->trType != TR_GRAVITY) return;

  memset(&beam, 0, sizeof(beam));

  beam.reType       = RT_RAIL_CORE;
  beam.customShader = beam_shader;

  AxisClear(beam.axis);
  memcpy(beam.shaderRGBA, color, sizeof(beam.shaderRGBA));

  VectorCopy(pos->trBase, currentOrigin);
  if (cg.time > pos->trTime)
    BG_EvaluateTrajectory(pos, cg.time, beam.oldorigin);
  else
    VectorCopy(pos->trBase, beam.oldorigin);

  trajectory_t local_pos = *pos;
  for (int leveltime = local_pos.trTime + 8; leveltime < end_time; leveltime += 8)
  {
    BG_EvaluateTrajectory(&local_pos, leveltime, origin);
    trap_CM_BoxTrace(&trace, currentOrigin, origin, NULL, NULL, 0, MASK_SHOT);
    VectorCopy(trace.endpos, currentOrigin);

    sample_timer -= 8;
    if (sample_timer <= 0)
    {
      sample_timer = 32;
      VectorCopy(origin, beam.origin);
      if (leveltime >= cg.time)
      {
        vec3_t d, saved_origin;
        VectorCopy(beam.origin, saved_origin);
        VectorSubtract(beam.origin, beam.oldorigin, d);
        VectorMA(beam.oldorigin, .5f, d, beam.origin);
        trap_R_AddRefEntityToScene(&beam);
        VectorCopy(saved_origin, beam.oldorigin);
      }
    }

    if (trace.fraction != 1)
    {
      // CG_ReflectVelocity: reflect the velocity on the trace plane
      vec3_t velocity;
      float  dot;
      int    hitTime;

      hitTime = leveltime - 8 + 8 * trace.fraction;
      BG_EvaluateTrajectoryDelta(&local_pos, hitTime, velocity);
      dot = DotProduct(velocity, trace.plane.normal);
      VectorMA(velocity, -2 * dot, trace.plane.normal, local_pos.trDelta);

      VectorScale(local_pos.trDelta, .65f, local_pos.trDelta);

      VectorAdd(currentOrigin, trace.plane.normal, currentOrigin);
      VectorCopy(currentOrigin, local_pos.trBase);
      local_pos.trTime = leveltime;

      sample_timer = 0;
      if (cg.time > local_pos.trTime)
        BG_EvaluateTrajectory(&local_pos, cg.time, beam.oldorigin);
      else
        VectorCopy(local_pos.trBase, beam.oldorigin);
    }
  }
}
