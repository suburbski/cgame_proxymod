/*
  ==============================
  Written by id software, nightmare and hk of mdd
  This file is part of mdd client proxymod.

  mdd client proxymod is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  mdd client proxymod is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mdd client proxymod.  If not, see <http://www.gnu.org/licenses/>.
  ==============================
  Note: mdd client proxymod contains large quantities from the quake III arena source code
*/
#include "cg_hud.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_gl.h"
#include "cg_jump.h"
#include "cg_local.h"
#include "cg_rl.h"
#include "cg_time.h"
#include "cg_utils.h"
#include "version.h"

vmCvar_t mdd_cgameproxy_version;
vmCvar_t mdd_hud_draw;
vmCvar_t mdd_hud_opacity;

vmCvar_t mdd_hud_ammo_draw;
vmCvar_t mdd_hud_ammo_offsetX;
vmCvar_t mdd_hud_ammo_offsetY;
vmCvar_t mdd_hud_ammo_size;
vmCvar_t mdd_hud_ammo_textColor;

vmCvar_t mdd_local_sounds_only;

static cvarTable_t cvarTable[] = {
  { &mdd_cgameproxy_version, "mdd_cgameproxy_version", VERSION, CVAR_USERINFO | CVAR_INIT },
  { &mdd_hud_draw, "mdd_hud_draw", "1", CVAR_ARCHIVE },
  { &mdd_hud_opacity, "mdd_hud_opacity", "0.5", CVAR_ARCHIVE },

  { &mdd_hud_ammo_draw, "mdd_hud_ammo_draw", "0", CVAR_ARCHIVE },
  { &mdd_hud_ammo_offsetX, "mdd_hud_ammo_offsetX", "610", CVAR_ARCHIVE },
  { &mdd_hud_ammo_offsetY, "mdd_hud_ammo_offsetY", "30", CVAR_ARCHIVE },
  { &mdd_hud_ammo_size, "mdd_hud_ammo_size", "32", CVAR_ARCHIVE },
  { &mdd_hud_ammo_textColor, "mdd_hud_ammo_textColor", "7", CVAR_ARCHIVE },

  { &mdd_local_sounds_only, "mdd_local_sounds_only", "0", CVAR_ARCHIVE }
};

static hud_t           hud;
static hud_ammo_t      ammo;

void hud_setup(void)
{
  init_cvars(cvarTable, ARRAY_LEN(cvarTable));
  hud_baseSetup(&hud);
  hud_ammoSetup(&ammo);
  init_time();
  init_rl();
  init_gl();
}

void hud_update(void)
{
  // TODO: instead of just calling setup
  // TODO: we check which structs require an update
  hud_baseSetup(&hud);
  hud_ammoSetup(&ammo);
}

int8_t hud_baseSetup(hud_t* h)
{
  float const hud_opacity = cvar_getValue("mdd_hud_opacity");

  h->color[0] = 1.0;
  h->color[1] = 1.0;
  h->color[2] = 1.0;
  h->color[3] = hud_opacity;

  return qtrue;
}

void hud_draw(void)
{
  float const hud_draw = cvar_getValue("mdd_hud_draw");
  if (!hud_draw) return;

  float const hud_ammo_draw      = cvar_getValue("mdd_hud_ammo_draw");

  if (hud_ammo_draw) hud_ammoDraw(&ammo);

  draw_jump();

  draw_time();

  // make sure the last color doesn't leak into defrag's UI
  g_syscall(CG_R_SETCOLOR, colorWhite);
}

/*
int8_t hud_vBarSetup( hud_bar_t *bar, float xPosAdj, float yPosAdj, float widthAdj, float heightAdj ) {
  // Position graph on adjusted 640x480 grid
  // Switch to native resolution and draw graph
  // Bar slides in from both sides and hits in the center
  float const mdd_hud_opacity = cvar_getValue("mdd_hud_opacity");

  bar->width  = widthAdj;
  bar->height = heightAdj;
  bar->xPos   = xPosAdj;
  bar->yPos   = yPosAdj;

  bar->colorBackdrop[0] = 1.0;
  bar->colorBackdrop[1] = 1.0;
  bar->colorBackdrop[2] = 1.0;
  bar->colorBackdrop[3] = mdd_hud_opacity;
  bar->colorBar[0] = 0.8;
  bar->colorBar[1] = 0.0;
  bar->colorBar[2] = 1.0;
  bar->colorBar[3] = mdd_hud_opacity;

  // convert adjusted coordinates to native ones
  convertAdjustedToNative( &bar->xPos, &bar->yPos, &bar->width, &bar->height );
  return qtrue;
}
*/

/*
 *
 * Ammo Hud
 *
 */

int8_t hud_ammoSetup(hud_ammo_t* ammoHud)
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

int8_t hud_ammoDraw(hud_ammo_t* ammoHud)
{
  uint32_t       y, i;
  playerState_t* ps;
  float          size;
  uint16_t       hasWeapon;
  uint16_t       ammo;

  ps   = getPs();
  size = ammoHud->size;

  // TODO: in case "give all" do not display the hud

  y = ammoHud->yPos;
  for (i = 1; i < 9; i++)
  {
    ammo      = ps->ammo[i + 1];
    hasWeapon = ps->stats[STAT_WEAPONS] & (1 << (i + 1));

    if (!ammo && !hasWeapon)
    {
      continue;
    }

    g_syscall(CG_R_SETCOLOR, ammoHud->colorBackdrop);
    CG_DrawPic(ammoHud->xPos, y, size, size, cgs.media.gfxWhiteShader);
    g_syscall(CG_R_SETCOLOR, colorWhite);

    CG_DrawPic(ammoHud->xPos, y, size, size, cgs.media.gfxAmmo[i]);

    if (!hasWeapon)
    {
      // mark weapon as unavailible
      CG_DrawPic(ammoHud->xPos, y, size, size, cgs.media.gfxDeferSymbol);
    }

    CG_DrawText(ammoHud->xPos, y + (size / 4), (size / 2), ammoHud->colorText, qtrue, vaf("%i", ps->ammo[i + 1]));
    y += size;
  }

  // TODO: color the text of the ammo red in case of low ammo
  // TODO: make textsize cvar dependant
  return qtrue;
}
