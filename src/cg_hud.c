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
#include "cg_local.h"
#include "version.h"

static vmCvar_t hud;
static vmCvar_t version;

static cvarTable_t hud_cvars[] = {
  { &hud, "mdd_hud", "1", CVAR_ARCHIVE_ND },
  { &version, "mdd_version", VERSION, CVAR_USERINFO | CVAR_INIT },
};

void init_hud(void)
{
  init_cvars(hud_cvars, ARRAY_LEN(hud_cvars));
}

uint8_t draw_hud(void)
{
  // First check if we have models, otherwise CM_ClipHandleToModel will fail
  if (!trap_CM_NumInlineModels()) return 0;

  update_cvars(hud_cvars, ARRAY_LEN(hud_cvars));

  return !!hud.integer;
}
