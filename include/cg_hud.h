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
#ifndef CG_HUD_H
#define CG_HUD_H

#include "cg_local.h"

typedef struct
{
  vec4_t color;
} hud_t;

typedef struct
{
  float  width;
  float  height;
  float  xPos;
  float  yPos;
  vec4_t colorBar;
  vec4_t colorBackdrop;
  float  value;
} hud_bar_t;

void   hud_setup(void);
void   hud_update(void);
int8_t hud_baseSetup(hud_t* h);
void   hud_draw(void);

#endif // CG_HUD_H
