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
#include "version.h"

vmCvar_t mdd_cgameproxy_version;
vmCvar_t mdd_hud_draw;
vmCvar_t mdd_hud_opacity;

static cvarTable_t cvarTable[] = {
  { &mdd_cgameproxy_version, "mdd_cgameproxy_version", VERSION, CVAR_USERINFO | CVAR_INIT },
  { &mdd_hud_draw, "mdd_hud_draw", "1", CVAR_ARCHIVE },
  { &mdd_hud_opacity, "mdd_hud_opacity", "0.5", CVAR_ARCHIVE }
};

void init_hud(void)
{
  init_cvars(cvarTable, ARRAY_LEN(cvarTable));
}

uint8_t draw_hud(void)
{
  update_cvars(cvarTable, ARRAY_LEN(cvarTable));

  return mdd_hud_draw.integer;
}
