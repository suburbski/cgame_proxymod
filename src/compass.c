#include "compass.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"
#include "help.h"

static vmCvar_t compass;
static vmCvar_t compass_yh;
static vmCvar_t compass_quadrant_rgbas;
static vmCvar_t compass_ticks_rgba;
static vmCvar_t compass_arrow_rgbas;

static cvarTable_t compass_cvars[] = {
  { &compass, "mdd_compass", "0b111", CVAR_ARCHIVE_ND },
  { &compass_yh, "mdd_compass_yh", "192 12", CVAR_ARCHIVE_ND },
  { &compass_quadrant_rgbas,
    "mdd_compass_quadrant_rgbas",
    "1 1 0 .25 / 0 1 0 .25 / 0 0 1 .25 / 1 0 1 .25",
    CVAR_ARCHIVE_ND },
  { &compass_ticks_rgba, "mdd_compass_ticks_rgba", "1 1 1 1", CVAR_ARCHIVE_ND },
  { &compass_arrow_rgbas, "mdd_compass_arrow_rgbas", "1 1 1 1 / 1 .5 0 1", CVAR_ARCHIVE_ND },
};

static help_t compass_help[] = {
  { compass_cvars + 0,
    BINARY_LITERAL,
    {
      "mdd_compass 0bXXX",
      "              |||",
      "              ||+- draw quadrants",
      "              |+-- draw ticks",
      "              +--- draw arrow",
    } },
#define QUADRANTS 1
#define TICKS     2
#define ARROW     4
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
      "mdd_compass_quadrant_rgbas X X X X / X X X X / X X X X / X X X X",
    },
  },
  {
    compass_cvars + 3,
    RGBA,
    {
      "mdd_compass_ticks_rgba X X X X",
    },
  },
  {
    compass_cvars + 4,
    RGBAS,
    {
      "mdd_compass_arrow_rgbas X X X X / X X X X",
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
  compass.integer = cvar_getInteger("mdd_compass");
}

typedef struct
{
  vec2_t graph_yh;
  vec4_t graph_quadrant_rgbas[4];
  vec4_t graph_arrow_rgba[2];
  vec4_t graph_ticks_rgba;

  playerState_t pm_ps;
} compass_t;

static compass_t s;

void draw_compass(void)
{
  if (!compass.integer) return;

  ParseVec(compass_yh.string, s.graph_yh, 2);

  s.pm_ps = *getPs();

  float const yaw = DEG2RAD(s.pm_ps.viewangles[YAW]);

  if (compass.integer & QUADRANTS)
  {
    ParseVec4(compass_quadrant_rgbas.string, s.graph_quadrant_rgbas, 4);
    CG_FillAngleYaw(0, (float)M_PI / 2, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[0]);
    CG_FillAngleYaw((float)M_PI / 2, (float)M_PI, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[1]);
    CG_FillAngleYaw(-(float)M_PI / 2, -(float)M_PI, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[2]);
    CG_FillAngleYaw(0, -(float)M_PI / 2, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[3]);
  }

  if (compass.integer & TICKS)
  {
    ParseVec(compass_ticks_rgba.string, s.graph_ticks_rgba, 4);
    {
      float const y = s.graph_yh[0] + s.graph_yh[1] / 2;
      float const w = 1;
      float const h = s.graph_yh[1] / 2;
      CG_DrawLineYaw(0, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw((float)M_PI / 2, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw((float)M_PI, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(-(float)M_PI / 2, yaw, y, w, h, s.graph_ticks_rgba);
    }
    {
      float const y = s.graph_yh[0] + 3 * s.graph_yh[1] / 4;
      float const w = 1;
      float const h = s.graph_yh[1] / 4;
      CG_DrawLineYaw((float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(3 * (float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(-(float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(-3 * (float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
    }
  }

  if (compass.integer & ARROW && (s.pm_ps.velocity[0] != 0 || s.pm_ps.velocity[1] != 0))
  {
    ParseVec4(compass_arrow_rgbas.string, s.graph_arrow_rgba, 2);
    vec4_t* color = &s.graph_arrow_rgba[0];
    if (s.pm_ps.velocity[0] == 0 || s.pm_ps.velocity[1] == 0)
    {
      color = &s.graph_arrow_rgba[1];
    }

    float const v_dir = atan2f(s.pm_ps.velocity[1], s.pm_ps.velocity[0]);

    float const y = s.graph_yh[0] + s.graph_yh[1];
    float const w = s.graph_yh[1];
    float const h = s.graph_yh[1] / 2;

    uint8_t const ch_up   = 135; // Arrow pointing up.
    uint8_t const ch_down = 134; // Arrow pointing down.

    CG_DrawCharYaw(v_dir, yaw, y, w, h, ch_up, *color);
    CG_DrawCharYaw(v_dir, yaw - (float)M_PI, y, w, h, ch_down, *color);
  }
}
