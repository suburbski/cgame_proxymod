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
#include "cg_draw.h"

#include "cg_utils.h"
#include "q_math.h"

#define RF_NOSHADOW 0x0040      // don't add stencil shadows
#define RDF_NOWORLDMODEL 0x0001 // used for player configuration screen

static inline int PASSFLOAT(float x)
{
  union
  {
    float f;
    int   i;
  } floatInt = { .f = x };
  return floatInt.i;
}

static inline void
  trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader)
{
  g_syscall(
    CG_R_DRAWSTRETCHPIC,
    PASSFLOAT(x),
    PASSFLOAT(y),
    PASSFLOAT(w),
    PASSFLOAT(h),
    PASSFLOAT(s1),
    PASSFLOAT(t1),
    PASSFLOAT(s2),
    PASSFLOAT(t2),
    hShader);
}

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640(float* x, float* y, float* w, float* h)
{
#if 0
  // adjust for wide screens
  if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
    *x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
  }
#endif
  // scale for screen sizes
  *x *= cgs.screenXScale;
  *y *= cgs.screenXScale; // Note that screenXScale is used to avoid widescreen stretching.
  *w *= cgs.screenXScale;
  *h *= cgs.screenXScale; // Note that screenXScale is used to avoid widescreen stretching.
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect(float x, float y, float w, float h, vec4_t const color)
{
  g_syscall(CG_R_SETCOLOR, color);
  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.gfxWhiteShader);
  g_syscall(CG_R_SETCOLOR, NULL);
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size)
{
  CG_AdjustFrom640(&x, &y, &w, &h);
  size *= cgs.screenXScale;
  trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.gfxWhiteShader);
  trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.gfxWhiteShader);
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size)
{
  CG_AdjustFrom640(&x, &y, &w, &h);
  size *= cgs.screenXScale;
  trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.gfxWhiteShader);
  trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.gfxWhiteShader);
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect(float x, float y, float w, float h, float size, vec4_t const color)
{
  g_syscall(CG_R_SETCOLOR, color);
  CG_DrawTopBottom(x, y, w, h, size);
  CG_DrawSides(x, y + size, w, h - size * 2, size);
  g_syscall(CG_R_SETCOLOR, NULL);
}

/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic(float x, float y, float w, float h, qhandle_t hShader)
{
  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, hShader);
}

void CG_DrawAdjPic(float x, float y, float width, float height, qhandle_t hShader)
{
  CG_AdjustFrom640(&x, &y, &width, &height);
  g_syscall(
    CG_R_DRAWSTRETCHPIC,
    PASSFLOAT(x),
    PASSFLOAT(y),
    PASSFLOAT(width),
    PASSFLOAT(height),
    PASSFLOAT(0),
    PASSFLOAT(0),
    PASSFLOAT(1),
    PASSFLOAT(1),
    hShader);
}

void convertAdjustedToNative(float* xAdj, float* yAdj, float* wAdj, float* hAdj)
{
  if (xAdj != NULL) *xAdj = ((cgs.glconfig.vidWidth) / 640.0) * (*xAdj);

  if (yAdj != NULL) *yAdj = ((cgs.glconfig.vidHeight) / 480.0) * (*yAdj);

  if (wAdj != NULL) *wAdj = ((cgs.glconfig.vidWidth) / 640.0) * (*wAdj);

  if (hAdj != NULL) *hAdj = ((cgs.glconfig.vidHeight) / 480.0) * (*hAdj);

  return;
}

/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar(float x, float y, float w, float h, uint8_t ch)
{
  if (ch == ' ') return;

  float const frow = .0625 * (ch >> 4);
  float const fcol = .0625 * (ch & 15);
  float const size = .0625;

  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, fcol, frow, fcol + size, frow + size, cgs.media.gfxCharsetShader);
}

void CG_DrawText(
  float        x,
  float        y,
  float        sizePx,
  char const*  string,
  vec4_t const color,
  qboolean     alignRight,
  qboolean     shadow)
{
  if (string == NULL) return;

  float         tmpX = x;
  int32_t const len  = strlen(string);

  // draw the drop shadow
  if (shadow)
  {
    g_syscall(CG_R_SETCOLOR, colorBlack);
    if (alignRight)
    {
      for (int32_t i = len - 1; i >= 0; --i)
      {
        tmpX -= sizePx;
        CG_DrawChar(tmpX + 2, y + 2, sizePx, sizePx, string[i]);
      }
    }
    else
    {
      for (int32_t i = 0; i < len; ++i)
      {
        CG_DrawChar(tmpX + 2, y + 2, sizePx, sizePx, string[i]);
        tmpX += sizePx;
      }
    }
  }

  tmpX = x;
  g_syscall(CG_R_SETCOLOR, color);
  if (alignRight)
  {
    for (int32_t i = len - 1; i >= 0; --i)
    {
      tmpX -= sizePx;
      CG_DrawChar(tmpX, y, sizePx, sizePx, string[i]);
    }
  }
  else
  {
    for (int32_t i = 0; i < len; ++i)
    {
      CG_DrawChar(tmpX, y, sizePx, sizePx, string[i]);
      tmpX += sizePx;
    }
  }
  g_syscall(CG_R_SETCOLOR, NULL);
}

void CG_Draw3DModel(
  float        x,
  float        y,
  float        w,
  float        h,
  qhandle_t    model,
  qhandle_t    skin,
  vec3_t const origin,
  vec3_t const angles)
{
  refdef_t    refdef;
  refEntity_t ent;

  // if ( !cg_draw3dIcons.integer || !cg_drawIcons.integer ) {
  //  return;
  // }

  CG_AdjustFrom640(&x, &y, &w, &h);

  memset(&refdef, 0, sizeof(refdef));

  memset(&ent, 0, sizeof(ent));
  AnglesToAxis(angles, ent.axis);
  VectorCopy(origin, ent.origin);
  ent.hModel     = model;
  ent.customSkin = skin;
  ent.renderfx   = RF_NOSHADOW; // no stencil shadows

  refdef.rdflags = RDF_NOWORLDMODEL;

  AxisClear(refdef.viewaxis);

  refdef.fov_x = 30;
  refdef.fov_y = 30;

  refdef.x      = x;
  refdef.y      = y;
  refdef.width  = w;
  refdef.height = h;

  refdef.time = 0; // getSnap()->serverTime;
  // refdef.time = cg.time;

  g_syscall(CG_R_CLEARSCENE);
  g_syscall(CG_R_ADDREFENTITYTOSCENE, &ent);
  g_syscall(CG_R_RENDERSCENE, &refdef);
}

int8_t getColor(uint8_t color, float opacity, vec4_t c)
{
  float tmp;

  tmp = opacity;
  if (opacity > 1.0 || opacity < 0) tmp = 1.0;

  switch (color)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 7:
    c[0] = color & 1;
    c[1] = color & 2;
    c[2] = color & 4;
    c[3] = tmp;
    break;

  // these two colors don't fit the pattern, probably a mistake.
  case 5:
    c[0] = 0.0;
    c[1] = 1.0;
    c[2] = 1.0;
    c[3] = tmp;
    break;

  case 6:
    c[0] = 1.0;
    c[1] = 0.0;
    c[2] = 1.0;
    c[3] = tmp;
    break;

  default:
    c[0] = 1.0;
    c[1] = 1.0;
    c[2] = 1.0;
    c[3] = tmp;
    return qfalse;
    break;
  }

  return qtrue;
}
