#include "cg_ammo.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"

static vmCvar_t mdd_hud_ammo_draw;
static vmCvar_t mdd_hud_ammo_offsetX;
static vmCvar_t mdd_hud_ammo_offsetY;
static vmCvar_t mdd_hud_ammo_size;
static vmCvar_t mdd_hud_ammo_textColor;

static cvarTable_t ammo_cvars[] = { { &mdd_hud_ammo_draw, "mdd_hud_ammo_draw", "0", CVAR_ARCHIVE },
                                    { &mdd_hud_ammo_offsetX, "mdd_hud_ammo_offsetX", "610", CVAR_ARCHIVE },
                                    { &mdd_hud_ammo_offsetY, "mdd_hud_ammo_offsetY", "30", CVAR_ARCHIVE },
                                    { &mdd_hud_ammo_size, "mdd_hud_ammo_size", "32", CVAR_ARCHIVE },
                                    { &mdd_hud_ammo_textColor, "mdd_hud_ammo_textColor", "7", CVAR_ARCHIVE } };

typedef struct
{
  qhandle_t graph_icons[9];

  float   xPos;
  float   yPos;
  float   size;
  uint8_t ammo[16];
  vec4_t  colorText;
  vec4_t  colorBackdrop;
} hud_ammo_t;

static hud_ammo_t ammo_;

void init_ammo(void)
{
  init_cvars(ammo_cvars, ARRAY_LEN(ammo_cvars));

  ammo_.graph_icons[0] = g_syscall(CG_R_REGISTERSHADER, "icons/iconw_gauntlet");
  ammo_.graph_icons[1] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_machinegun");
  ammo_.graph_icons[2] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_shotgun");
  ammo_.graph_icons[3] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_grenade");
  ammo_.graph_icons[4] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_rocket");
  ammo_.graph_icons[5] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_lightning");
  ammo_.graph_icons[6] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_railgun");
  ammo_.graph_icons[7] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_plasma");
  ammo_.graph_icons[8] = g_syscall(CG_R_REGISTERSHADER, "icons/icona_bfg");
}

static int8_t hud_ammoSetup(hud_ammo_t* ammoHud)
{
  float const mdd_hud_opacity = cvar_getValue("mdd_hud_opacity");
  float const xPosAdj         = cvar_getValue("mdd_hud_ammo_offsetX");
  float const yPosAdj         = cvar_getValue("mdd_hud_ammo_offsetY");
  float const size            = cvar_getValue("mdd_hud_ammo_size");
  float const textColor       = cvar_getValue("mdd_hud_ammo_textColor");

  ammoHud->xPos = xPosAdj;
  ammoHud->yPos = yPosAdj;
  ammoHud->size = size;

  getColor(textColor, 1.0, ammoHud->colorText);

  ammoHud->colorBackdrop[0] = 0.0;
  ammoHud->colorBackdrop[1] = 0.0;
  ammoHud->colorBackdrop[2] = 0.0;
  ammoHud->colorBackdrop[3] = mdd_hud_opacity;

  convertAdjustedToNative(&ammoHud->xPos, &ammoHud->yPos, NULL, NULL);
  return qtrue;
}

void draw_ammo(void)
{
  update_cvars(ammo_cvars, ARRAY_LEN(ammo_cvars));
  hud_ammoSetup(&ammo_);

  if (!mdd_hud_ammo_draw.integer) return;

  uint32_t y, i;
  float    size;
  uint16_t hasWeapon;
  uint16_t ammo;

  playerState_t const* const ps = getPs();
  size                          = ammo_.size;

  y = ammo_.yPos;
  for (i = 1; i < 9; i++)
  {
    ammo      = ps->ammo[i + 1];
    hasWeapon = ps->stats[STAT_WEAPONS] & (1 << (i + 1));

    if (!ammo && !hasWeapon)
    {
      continue;
    }

    g_syscall(CG_R_SETCOLOR, ammo_.colorBackdrop);
    CG_DrawPic(ammo_.xPos, y, size, size, cgs.media.gfxWhiteShader);
    g_syscall(CG_R_SETCOLOR, colorWhite);

    CG_DrawPic(ammo_.xPos, y, size, size, ammo_.graph_icons[i]);

    if (!hasWeapon)
    {
      // mark weapon as unavailible
      CG_DrawPic(ammo_.xPos, y, size, size, cgs.media.gfxDeferSymbol);
    }

    CG_DrawText(ammo_.xPos, y + (size / 4), (size / 2), ammo_.colorText, qtrue, vaf("%i", ps->ammo[i + 1]));
    y += size;
  }
}
