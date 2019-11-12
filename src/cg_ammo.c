#include "cg_ammo.h"

#include "bg_public.h"
#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_local.h"
#include "cg_utils.h"

static vmCvar_t ammo;
static vmCvar_t ammo_graph_xywh;
static vmCvar_t ammo_text_xh;
static vmCvar_t ammo_text_rgba;

static cvarTable_t ammo_cvars[] = { { &ammo, "mdd_ammo", "0b0001", CVAR_ARCHIVE },
                                    { &ammo_graph_xywh, "mdd_ammo_graph_xywh", "610 100 24 24", CVAR_ARCHIVE },
                                    { &ammo_text_xh, "mdd_ammo_text_xh", "6 12", CVAR_ARCHIVE },
                                    { &ammo_text_rgba, "mdd_ammo_text_rgba", "1 1 1 1", CVAR_ARCHIVE } };

// mdd_ammo 0b X X X X
//             | | | |
//             | | | + - draw
//             | | + - - no weapon -> no draw
//             | + - - - gun
//             + - - - - 3D
#define AMMO_DRAW 1
#define AMMO_NOWEAPNODRAW 2
#define AMMO_GUN 4
#define AMMO_3D 8

typedef struct
{
  qhandle_t graph_icons[16];
  qhandle_t graph_models[16];
  vec3_t    graph_model_origin;
  vec3_t    graph_model_angles;

  vec4_t graph_xywh;
  vec2_t text_xh;

  vec4_t text_rgba;
} ammo_t;

static ammo_t ammo_;

void init_ammo(void)
{
  init_cvars(ammo_cvars, ARRAY_LEN(ammo_cvars));

  ammo_.graph_icons[0]  = trap_R_RegisterShader("icons/icona_machinegun");
  ammo_.graph_icons[1]  = trap_R_RegisterShader("icons/icona_shotgun");
  ammo_.graph_icons[2]  = trap_R_RegisterShader("icons/icona_grenade");
  ammo_.graph_icons[3]  = trap_R_RegisterShader("icons/icona_rocket");
  ammo_.graph_icons[4]  = trap_R_RegisterShader("icons/icona_lightning");
  ammo_.graph_icons[5]  = trap_R_RegisterShader("icons/icona_railgun");
  ammo_.graph_icons[6]  = trap_R_RegisterShader("icons/icona_plasma");
  ammo_.graph_icons[7]  = trap_R_RegisterShader("icons/icona_bfg");
  ammo_.graph_icons[8]  = trap_R_RegisterShader("icons/iconw_machinegun");
  ammo_.graph_icons[9]  = trap_R_RegisterShader("icons/iconw_shotgun");
  ammo_.graph_icons[10] = trap_R_RegisterShader("icons/iconw_grenade");
  ammo_.graph_icons[11] = trap_R_RegisterShader("icons/iconw_rocket");
  ammo_.graph_icons[12] = trap_R_RegisterShader("icons/iconw_lightning");
  ammo_.graph_icons[13] = trap_R_RegisterShader("icons/iconw_railgun");
  ammo_.graph_icons[14] = trap_R_RegisterShader("icons/iconw_plasma");
  ammo_.graph_icons[15] = trap_R_RegisterShader("icons/iconw_bfg");

  ammo_.graph_models[0]  = trap_R_RegisterModel("models/powerups/ammo/machinegunam.md3");
  ammo_.graph_models[1]  = trap_R_RegisterModel("models/powerups/ammo/shotgunam.md3");
  ammo_.graph_models[2]  = trap_R_RegisterModel("models/powerups/ammo/grenadeam.md3");
  ammo_.graph_models[3]  = trap_R_RegisterModel("models/powerups/ammo/rocketam.md3");
  ammo_.graph_models[4]  = trap_R_RegisterModel("models/powerups/ammo/lightningam.md3");
  ammo_.graph_models[5]  = trap_R_RegisterModel("models/powerups/ammo/railgunam.md3");
  ammo_.graph_models[6]  = trap_R_RegisterModel("models/powerups/ammo/plasmaam.md3");
  ammo_.graph_models[7]  = trap_R_RegisterModel("models/powerups/ammo/bfgam.md3");
  ammo_.graph_models[8]  = trap_R_RegisterModel("models/weapons2/machinegun/machinegun.md3");
  ammo_.graph_models[9]  = trap_R_RegisterModel("models/weapons2/shotgun/shotgun.md3");
  ammo_.graph_models[10] = trap_R_RegisterModel("models/weapons2/grenadel/grenadel.md3");
  ammo_.graph_models[11] = trap_R_RegisterModel("models/weapons2/rocketl/rocketl.md3");
  ammo_.graph_models[12] = trap_R_RegisterModel("models/weapons2/lightning/lightning.md3");
  ammo_.graph_models[13] = trap_R_RegisterModel("models/weapons2/railgun/railgun.md3");
  ammo_.graph_models[14] = trap_R_RegisterModel("models/weapons2/plasma/plasma.md3");
  ammo_.graph_models[15] = trap_R_RegisterModel("models/weapons2/bfg/bfg.md3");

  memset(ammo_.graph_model_origin, 0, 3 * sizeof(vec_t));
  ammo_.graph_model_origin[0] = 70.f;
  memset(ammo_.graph_model_angles, 0, 3 * sizeof(vec_t));
}

void draw_ammo(void)
{
  update_cvars(ammo_cvars, ARRAY_LEN(ammo_cvars));
  ammo.integer = cvar_getInteger("mdd_ammo");

  if (!(ammo.integer & AMMO_DRAW)) return;

  ParseVec(ammo_graph_xywh.string, ammo_.graph_xywh, 4);
  ParseVec(ammo_text_rgba.string, ammo_.text_rgba, 4);

  float                      y  = ammo_.graph_xywh[1];
  playerState_t const* const ps = getPs();
  for (uint8_t i = 0; i < 8; ++i)
  {
    uint16_t const ammoLeft  = ps->ammo[i + 2];
    qboolean const hasWeapon = ps->stats[STAT_WEAPONS] & (1 << (i + 2));

    if (!hasWeapon && (ammo.integer & AMMO_NOWEAPNODRAW || !ammoLeft)) continue;

    if (!(ammo.integer & AMMO_3D))
    {
      CG_DrawPic(
        ammo_.graph_xywh[0],
        y,
        ammo_.graph_xywh[2],
        ammo_.graph_xywh[3],
        ammo_.graph_icons[i + (ammo.integer & AMMO_GUN ? 8 : 0)]);
    }
    else
    {
      ammo_.graph_model_angles[YAW] = 90.f + 20.f * sinf(getSnap()->serverTime / 1000.f);
      CG_Draw3DModel(
        ammo_.graph_xywh[0],
        y,
        ammo_.graph_xywh[2],
        ammo_.graph_xywh[3],
        ammo_.graph_models[i + (ammo.integer & AMMO_GUN ? 8 : 0)],
        0,
        ammo_.graph_model_origin,
        ammo_.graph_model_angles);
    }

    if (!hasWeapon) // Mark weapon as unavailable
    {
      CG_DrawPic(ammo_.graph_xywh[0], y, ammo_.graph_xywh[2], ammo_.graph_xywh[3], cgs.media.deferShader);
    }

    ParseVec(ammo_text_xh.string, ammo_.text_xh, 2);
    qboolean const alignRight = ammo_.graph_xywh[0] + ammo_.graph_xywh[2] / 2.f > SCREEN_WIDTH / 2;
    CG_DrawText(
      alignRight ? ammo_.graph_xywh[0] - ammo_.text_xh[0]
                 : ammo_.graph_xywh[0] + ammo_.graph_xywh[2] + ammo_.text_xh[0],
      y + ammo_.graph_xywh[3] / 2.f - .5f * ammo_.text_xh[1],
      ammo_.text_xh[1],
      vaf("%i", ammoLeft),
      ammo_.text_rgba,
      alignRight,
      qtrue /*shadow*/);
    y += ammo_.graph_xywh[3];
  }
}
