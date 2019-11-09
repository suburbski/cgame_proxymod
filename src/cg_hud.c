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

#include "cg_ammo.h"
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

vmCvar_t mdd_local_sounds_only;

static cvarTable_t cvarTable[] = {
  { &mdd_cgameproxy_version, "mdd_cgameproxy_version", VERSION, CVAR_USERINFO | CVAR_INIT },
  { &mdd_hud_draw, "mdd_hud_draw", "1", CVAR_ARCHIVE },
  { &mdd_hud_opacity, "mdd_hud_opacity", "0.5", CVAR_ARCHIVE },

  { &mdd_local_sounds_only, "mdd_local_sounds_only", "0", CVAR_ARCHIVE }
};

static hud_t hud;

void hud_setup(void)
{
  init_cvars(cvarTable, ARRAY_LEN(cvarTable));
  hud_baseSetup(&hud);
  init_time();
  init_rl();
  init_gl();
}

void hud_update(void)
{
  // TODO: instead of just calling setup
  // TODO: we check which structs require an update
  hud_baseSetup(&hud);
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

  draw_ammo();
  draw_jump();
  draw_time();

  // make sure the last color doesn't leak into defrag's UI
  g_syscall(CG_R_SETCOLOR, colorWhite);
}
