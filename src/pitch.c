#include "pitch.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"
#include "help.h"

#include <stdlib.h>

static vmCvar_t pitch;
static vmCvar_t pitch_xwh;
static vmCvar_t pitch_rgba;

static cvarTable_t pitch_cvars[] = {
  { &pitch, "mdd_pitch", "", CVAR_ARCHIVE_ND },
  { &pitch_xwh, "mdd_pitch_xwh", "316 8 1", CVAR_ARCHIVE_ND },
  { &pitch_rgba, "mdd_pitch_rgba", ".8 .8 .8 .8", CVAR_ARCHIVE_ND },
};

static help_t pitch_help[] = {
  {
    pitch_cvars + 1,
    X | W | H,
    {
      "mdd_pitch_xwh X X X",
    },
  },
  {
    pitch_cvars + 2,
    RGBA,
    {
      "mdd_pitch_rgba X X X X",
    },
  },
};

void init_pitch(void)
{
  init_cvars(pitch_cvars, ARRAY_LEN(pitch_cvars));
  init_help(pitch_help, ARRAY_LEN(pitch_help));
}

void update_pitch(void)
{
  update_cvars(pitch_cvars, ARRAY_LEN(pitch_cvars));
}

typedef struct
{
  vec3_t graph_xwh;
  vec4_t graph_rgba;

  playerState_t pm_ps;
} pitch_t;

static pitch_t s;

void draw_pitch(void)
{
  if (pitch.string[0] == '\0') return;

  ParseVec(pitch_xwh.string, s.graph_xwh, 3);
  ParseVec(pitch_rgba.string, s.graph_rgba, 4);

  s.pm_ps = *getPs();

  float const p = DEG2RAD(s.pm_ps.viewangles[PITCH]);

  char const* str = pitch.string;
  char*       end;
  for (float angle = strtof(str, &end); str != end; angle = strtof(str, &end))
  {
    float const x = s.graph_xwh[0];
    float const w = s.graph_xwh[1];
    float const h = s.graph_xwh[2];
    CG_DrawLinePitch(DEG2RAD(angle), p, x, w, h, s.graph_rgba);
    str = end;
  }
}
