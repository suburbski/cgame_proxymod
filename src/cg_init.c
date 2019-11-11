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
#include "cg_init.h"

#include "bg_public.h"
#include "cg_local.h"
#include "version.h"

#include <stdlib.h>

static void init_gfx(int32_t clientNum);

void cg_init(int32_t cmd, int32_t clientNum)
{
  (void)cmd;

  trap_Print(vaf("^7[^1m^3D^1d^7] cgame-proxy: %s\n", VERSION));
  initVM();

  // g_syscall( CG_MEMSET, ...)
  memset(&cgs, 0, sizeof(cgs));

  init_gfx(clientNum);
}

static void init_gfx(int32_t clientNum)
{
  cgs.clientNum = clientNum;

  trap_GetGlconfig(&cgs.glconfig); // rendering configuration
  cgs.screenXScale = cgs.glconfig.vidWidth / 640.f;
  cgs.screenYScale = cgs.glconfig.vidHeight / 480.f;

  trap_GetGameState(&cgs.gameState);

  cgs.levelStartTime = atoi(CG_ConfigString(CS_LEVEL_START_TIME));

  cgs.media.gfxDeferSymbol     = trap_R_RegisterShader("gfx/2d/defer");
  cgs.media.gfxCharsetShader   = trap_R_RegisterShader("gfx/2d/bigchars");
  cgs.media.gfxWhiteShader     = trap_R_RegisterShader("white");
  cgs.media.gfxCharsetProp     = trap_R_RegisterShader("menu/art/font1_prop.tga");
  cgs.media.gfxCharsetPropGlow = trap_R_RegisterShader("menu/art/font1_prop_glo.tga");
  cgs.media.gfxCharsetPropB    = trap_R_RegisterShader("menu/art/font2_prop.tga");
}
