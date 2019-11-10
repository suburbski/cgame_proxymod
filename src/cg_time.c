#include "cg_time.h"

#include "bg_public.h"
#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"
#include "nade_tracking.h"
#include "q_math.h"

#define MAX_GB_TIME 250
#define MAX_RL_TIME 15000

#define NADE_EXPLODE_TIME 2500

nade_info_t nades[MAX_NADES];

static vmCvar_t time;
static vmCvar_t time_xywh;
static vmCvar_t time_item_w;
static vmCvar_t time_item_rgba;
static vmCvar_t time_gb_rgba;
static vmCvar_t time_outline_w;
static vmCvar_t time_outline_rgba;

static cvarTable_t time_cvars[] = { { &time, "mdd_time", "1", CVAR_ARCHIVE },
                                    { &time_xywh, "mdd_time_xywh", "275 275 100 16", CVAR_ARCHIVE },
                                    { &time_item_w, "mdd_time_item_w", "3", CVAR_ARCHIVE },
                                    { &time_item_rgba, "mdd_time_item_rgba", "1 1 0 1", CVAR_ARCHIVE },
                                    { &time_gb_rgba, "mdd_time_gb_rgba", "1 0 0 1", CVAR_ARCHIVE },
                                    { &time_outline_w, "mdd_time_outline_w", "1", CVAR_ARCHIVE },
                                    { &time_outline_rgba, "mdd_time_outline_rgba", "1 1 1 1", CVAR_ARCHIVE } };

void init_time(void)
{
  init_cvars(time_cvars, ARRAY_LEN(time_cvars));

  for (uint8_t i = 0; i < MAX_NADES; ++i) nades[i].id = -1;
}

typedef struct
{
  vec4_t graph_xywh;

  vec4_t graph_outline_rgba;
  vec4_t graph_item_rgba;
  vec4_t graph_gb_rgba;
} time_t;

static time_t time_;

// XPC32: Also some of the quadratic loops can be simplified in the nade timer and gl trace code
//        Because of entity state tracking
//        And I should prolly be memsetting ent states array to 0 and setting just number or cn to -1 or something

static int  find_nade(int nade_id);
static int  track_nade(int nade_id, int time);
static void draw_item(float progress, vec4_t const color);

void draw_time(void)
{
  update_cvars(time_cvars, ARRAY_LEN(time_cvars));

  if (!time.integer) return;

  ParseVec(time_xywh.string, time_.graph_xywh, 4);

  ParseVec(time_outline_rgba.string, time_.graph_outline_rgba, 4);
  ParseVec(time_item_rgba.string, time_.graph_item_rgba, 4);
  ParseVec(time_gb_rgba.string, time_.graph_gb_rgba, 4);

  // draw the outline
  CG_DrawRect(
    time_.graph_xywh[0],
    time_.graph_xywh[1],
    time_.graph_xywh[2],
    time_.graph_xywh[3],
    time_outline_w.value,
    time_.graph_outline_rgba);

  snapshot_t const* const    snap = getSnap();
  playerState_t const* const ps   = getPs();

  // gb stuff
  // todo: make gb timer off-able and use pps if available and cvar
  if (ps->pm_flags & PMF_TIME_KNOCKBACK && ps->groundEntityNum != ENTITYNUM_NONE && !(ps->pm_flags & PMF_RESPAWNED))
  {
    draw_item(1.f - (float)ps->pm_time / MAX_GB_TIME, time_.graph_gb_rgba);
  }

  // cull exploded nades to make space
  // and set valid nades to not seen in snapshot yet
  for (int i = 0; i < MAX_NADES; ++i)
  {
    if (nades[i].id == -1) continue;

    if (nades[i].explode_time - snap->serverTime <= 0)
      nades[i].id = -1;
    else
      nades[i].seen = 0;
  }

  // traverse ent list to update nade infos
  for (int i = 0; i < snap->numEntities; i++)
  {
    entityState_t entity = snap->entities[i];
    if (entity.eType == ET_MISSILE && entity.weapon == WP_GRENADE_LAUNCHER && entity.clientNum == ps->clientNum)
    {
      int const nade_index = find_nade(entity.number);
      if (nade_index == -1) // new nade
        track_nade(entity.number, snap->serverTime);
      else
        nades[nade_index].seen = 1;
    }
    else if (entity.eType == ET_MISSILE && entity.weapon == WP_ROCKET_LAUNCHER && entity.clientNum == ps->clientNum)
    {
      trace_t     t;
      vec3_t      origin;
      vec3_t      dest;
      float const elapsed_time = (cgs.time - entity.pos.trTime) * .001f;

      VectorMA(entity.pos.trBase, elapsed_time, entity.pos.trDelta, origin);
      VectorMA(entity.pos.trBase, MAX_RL_TIME * .001f, entity.pos.trDelta, dest);

      // a rocket dest should never change (ignoring movers)
      // trace doesn't need to be recomputed each time
      g_syscall(CG_CM_BOXTRACE, &t, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
      float total_time = Distance(entity.pos.trBase, t.endpos) / VectorLength(entity.pos.trDelta);
      draw_item(elapsed_time / total_time, time_.graph_item_rgba);
    }
  }

  // cull nades not in snapshot (prolly prematurely detonated) and draw the rest
  for (int i = 0; i < MAX_NADES; i++)
  {
    if (nades[i].id == -1) continue;

    if (!nades[i].seen)
    {
      nades[i].id = -1;
    }
    else
    {
      float progress = 1.f - (float)(nades[i].explode_time - snap->serverTime) / NADE_EXPLODE_TIME;
      draw_item(progress, time_.graph_item_rgba);
    }
  }
}

static int find_nade(int nade_id)
{
  for (uint8_t i = 0; i < MAX_NADES; ++i)
  {
    if (nades[i].id == nade_id) return i;
  }

  return -1;
}

static int track_nade(int nade_id, int time)
{
  for (uint8_t i = 0; i < MAX_NADES; ++i)
  {
    if (nades[i].id == -1)
    {
      nades[i].id           = nade_id;
      nades[i].explode_time = time + NADE_EXPLODE_TIME;
      nades[i].seen         = 1;
      return 0;
    }
  }

  // no free space to track the nade
  return -1;
}

static inline void draw_item(float progress, vec4_t const color)
{
  CG_FillRect(
    time_.graph_xywh[0] + (time_.graph_xywh[2] - time_item_w.value) * progress,
    time_.graph_xywh[1],
    time_item_w.value,
    time_.graph_xywh[3],
    color);
}
