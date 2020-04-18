#include "compass.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"
#include "help.h"

static vmCvar_t compass;
static vmCvar_t compass_yh;
static vmCvar_t compass_rgbas;

static cvarTable_t compass_cvars[] = {
  { &compass, "mdd_compass", "1", CVAR_ARCHIVE_ND },
  { &compass_yh, "mdd_compass_yh", "192 12", CVAR_ARCHIVE_ND },
  { &compass_rgbas, "mdd_compass_rgbas", "1 1 0 .25 / 0 1 0 .25 / 0 0 1 .25 / 1 0 1 .25", CVAR_ARCHIVE_ND },
};

static help_t compass_help[] = {
  {
    compass_cvars + 1,
    Y | H,
    {
      "mdd_compass_yh X X",
    },
  },
  {
    compass_cvars + 2,
    RGBAS,
    {
      "mdd_compass_rgbas X X X X / X X X X / X X X X / X X X X",
    },
  },
};

void init_compass(void)
{
  init_cvars(compass_cvars, ARRAY_LEN(compass_cvars));
  init_help(compass_help, ARRAY_LEN(compass_help));
}

void update_compass(void)
{
  update_cvars(compass_cvars, ARRAY_LEN(compass_cvars));
}

typedef struct
{
  vec2_t graph_yh;
  vec4_t graph_rgba[4];

  playerState_t pm_ps;
} compass_t;

static compass_t s;

void draw_compass(void)
{
  if (!compass.integer) return;

  ParseVec(compass_yh.string, s.graph_yh, 2);
  ParseVec4(compass_rgbas.string, s.graph_rgba, 4);
  s.pm_ps = *getPs();

  float const yaw = DEG2RAD(s.pm_ps.viewangles[YAW]);
  CG_FillAngleYaw(0, (float)M_PI / 2, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgba[0]);
  CG_FillAngleYaw((float)M_PI / 2, (float)M_PI, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgba[1]);
  CG_FillAngleYaw(-(float)M_PI / 2, -(float)M_PI, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgba[2]);
  CG_FillAngleYaw(0, -(float)M_PI / 2, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgba[3]);
}
