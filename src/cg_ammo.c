#include "cg_ammo.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"

static vmCvar_t ammo;
static vmCvar_t ammo_graph_xywh;
static vmCvar_t ammo_text_xh;
static vmCvar_t ammo_text_rgba;

static cvarTable_t ammo_cvars[] = { { &ammo, "mdd_ammo", "1", CVAR_ARCHIVE },
                                    { &ammo_graph_xywh, "mdd_ammo_graph_xywh", "610 100 24 24", CVAR_ARCHIVE },
                                    { &ammo_text_xh, "mdd_ammo_text_xh", "6 12", CVAR_ARCHIVE },
                                    { &ammo_text_rgba, "mdd_ammo_text_rgba", "1 1 1 1", CVAR_ARCHIVE } };

typedef struct
{
  qhandle_t graph_icons[8];

  vec4_t graph_xywh;
  vec2_t text_xh;

  vec4_t text_rgba;
} ammo_t;

static ammo_t ammo_;

void init_ammo(void)
{
  init_cvars(ammo_cvars, ARRAY_LEN(ammo_cvars));

  ammo_.graph_icons[0] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_machinegun");
  ammo_.graph_icons[1] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_shotgun");
  ammo_.graph_icons[2] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_grenade");
  ammo_.graph_icons[3] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_rocket");
  ammo_.graph_icons[4] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_lightning");
  ammo_.graph_icons[5] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_railgun");
  ammo_.graph_icons[6] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_plasma");
  ammo_.graph_icons[7] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_bfg");
}

void draw_ammo(void)
{
  update_cvars(ammo_cvars, ARRAY_LEN(ammo_cvars));

  if (!ammo.integer) return;

  ParseVec(ammo_graph_xywh.string, ammo_.graph_xywh, 4);
  ParseVec(ammo_text_rgba.string, ammo_.text_rgba, 4);

  float                      y  = ammo_.graph_xywh[1];
  playerState_t const* const ps = getPs();
  for (uint8_t i = 0; i < 8; ++i)
  {
    uint16_t const ammo      = ps->ammo[i + 2];
    qboolean const hasWeapon = ps->stats[STAT_WEAPONS] & (1 << (i + 2));

    if (!ammo && !hasWeapon) continue;

    CG_DrawPic(ammo_.graph_xywh[0], y, ammo_.graph_xywh[2], ammo_.graph_xywh[3], ammo_.graph_icons[i]);

    if (!hasWeapon) // Mark weapon as unavailable
    {
      CG_DrawPic(ammo_.graph_xywh[0], y, ammo_.graph_xywh[2], ammo_.graph_xywh[3], cgs.media.gfxDeferSymbol);
    }

    ParseVec(ammo_text_xh.string, ammo_.text_xh, 2);
    qboolean const alignRight = ammo_.graph_xywh[0] + ammo_.graph_xywh[2] / 2.f > SCREEN_WIDTH / 2;
    CG_DrawText(
      alignRight ? ammo_.graph_xywh[0] - ammo_.text_xh[0]
                 : ammo_.graph_xywh[0] + ammo_.graph_xywh[2] + ammo_.text_xh[0],
      y + ammo_.graph_xywh[3] / 2.f - .5f * ammo_.text_xh[1],
      ammo_.text_xh[1],
      vaf("%i", ammo),
      ammo_.text_rgba,
      alignRight,
      qtrue /*shadow*/);
    y += ammo_.graph_xywh[3];
  }
}
