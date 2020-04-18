#include "cg_timer.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "help.h"
#include "nade_tracking.h"

#define MAX_GB_TIME 250
#define MAX_RL_TIME 15000

#define NADE_EXPLODE_TIME 2500

nade_info_t nades[MAX_NADES];

static vmCvar_t timer;
static vmCvar_t timer_xywh;
static vmCvar_t timer_item_w;
static vmCvar_t timer_item_rgba;
static vmCvar_t timer_gb_rgba;
static vmCvar_t timer_outline_w;
static vmCvar_t timer_outline_rgba;

static cvarTable_t timer_cvars[] = {
  { &timer, "mdd_timer", "1", CVAR_ARCHIVE_ND },
  { &timer_xywh, "mdd_timer_xywh", "275 275 100 16", CVAR_ARCHIVE_ND },
  { &timer_item_w, "mdd_timer_item_w", "3", CVAR_ARCHIVE_ND },
  { &timer_item_rgba, "mdd_timer_item_rgba", "1 1 0 1", CVAR_ARCHIVE_ND },
  { &timer_gb_rgba, "mdd_timer_gb_rgba", "1 0 0 1", CVAR_ARCHIVE_ND },
  { &timer_outline_w, "mdd_timer_outline_w", "1", CVAR_ARCHIVE_ND },
  { &timer_outline_rgba, "mdd_timer_outline_rgba", "1 1 1 1", CVAR_ARCHIVE_ND },
};

static help_t timer_help[] = {
  {
    timer_cvars + 1,
    X | Y | W | H,
    {
      "mdd_timer_xywh X X X X",
    },
  },
  {
    timer_cvars + 2,
    W,
    {
      "mdd_timer_item_w X",
    },
  },
  {
    timer_cvars + 3,
    RGBA,
    {
      "mdd_timer_item_rgba X X X X",
    },
  },
  {
    timer_cvars + 4,
    RGBA,
    {
      "mdd_timer_gb_rgba X X X X",
    },
  },
  {
    timer_cvars + 5,
    W,
    {
      "mdd_timer_outline_w X",
    },
  },
  {
    timer_cvars + 6,
    RGBA,
    {
      "mdd_timer_outline_rgba X X X X",
    },
  },
};

void init_timer(void)
{
  init_cvars(timer_cvars, ARRAY_LEN(timer_cvars));
  init_help(timer_help, ARRAY_LEN(timer_help));

  for (uint8_t i = 0; i < MAX_NADES; ++i) nades[i].id = -1;
}

void update_timer(void)
{
  update_cvars(timer_cvars, ARRAY_LEN(timer_cvars));
}

typedef struct
{
  vec4_t graph_xywh;

  vec4_t graph_outline_rgba;
  vec4_t graph_item_rgba;
  vec4_t graph_gb_rgba;
} timer_t;

static timer_t timer_;

// XPC32: Also some of the quadratic loops can be simplified in the nade timer and gl trace code
//        Because of entity state tracking
//        And I should prolly be memsetting ent states array to 0 and setting just number or cn to -1 or something

static int  find_nade(int nade_id);
static int  track_nade(int nade_id, int time);
static void draw_item(float progress, vec4_t const color);

void draw_timer(void)
{
  if (!timer.integer) return;

  ParseVec(timer_xywh.string, timer_.graph_xywh, 4);

  ParseVec(timer_outline_rgba.string, timer_.graph_outline_rgba, 4);
  ParseVec(timer_item_rgba.string, timer_.graph_item_rgba, 4);
  ParseVec(timer_gb_rgba.string, timer_.graph_gb_rgba, 4);

  // draw the outline
  CG_DrawRect(
    timer_.graph_xywh[0],
    timer_.graph_xywh[1],
    timer_.graph_xywh[2],
    timer_.graph_xywh[3],
    timer_outline_w.value,
    timer_.graph_outline_rgba);

  snapshot_t const* const    snap = getSnap();
  playerState_t const* const ps   = getPs();

  // gb stuff
  // TODO: make gb timer off-able and use pps if available and cvar
  if (ps->pm_flags & PMF_TIME_KNOCKBACK && ps->groundEntityNum != ENTITYNUM_NONE && !(ps->pm_flags & PMF_RESPAWNED))
  {
    draw_item(1.f - (float)ps->pm_time / MAX_GB_TIME, timer_.graph_gb_rgba);
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
      float const elapsed_time = (cg.time - entity.pos.trTime) * .001f;

      VectorMA(entity.pos.trBase, elapsed_time, entity.pos.trDelta, origin);
      VectorMA(entity.pos.trBase, MAX_RL_TIME * .001f, entity.pos.trDelta, dest);

      // a rocket dest should never change (ignoring movers)
      // trace doesn't need to be recomputed each time
      trap_CM_BoxTrace(&t, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
      float total_time = Distance(entity.pos.trBase, t.endpos) / VectorLength(entity.pos.trDelta);
      draw_item(elapsed_time / total_time, timer_.graph_item_rgba);
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
      draw_item(progress, timer_.graph_item_rgba);
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
    timer_.graph_xywh[0] + (timer_.graph_xywh[2] - timer_item_w.value) * progress,
    timer_.graph_xywh[1],
    timer_item_w.value,
    timer_.graph_xywh[3],
    color);
}
