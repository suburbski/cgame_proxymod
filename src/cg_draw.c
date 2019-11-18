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

#include "assert.h"
#include "cg_local.h"

#define RF_NOSHADOW 0x0040      // don't add stencil shadows
#define RDF_NOWORLDMODEL 0x0001 // used for player configuration screen

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
  if (!w || !h) return;
  trap_R_SetColor(color);
  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
  trap_R_SetColor(NULL);
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
  trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
  trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size)
{
  CG_AdjustFrom640(&x, &y, &w, &h);
  size *= cgs.screenXScale;
  trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
  trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect(float x, float y, float w, float h, float size, vec4_t const color)
{
  trap_R_SetColor(color);
  CG_DrawTopBottom(x, y, w, h, size);
  CG_DrawSides(x, y + size, w, h - size * 2, size);
  trap_R_SetColor(NULL);
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
  trap_R_DrawStretchPic(x, y, w, h, fcol, frow, fcol + size, frow + size, cgs.media.charsetShader);
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
  int32_t const len  = (int32_t)strlen(string);

  // draw the drop shadow
  if (shadow)
  {
    trap_R_SetColor(colorBlack);
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
  trap_R_SetColor(color);
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
  trap_R_SetColor(NULL);
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

  trap_R_ClearScene();
  trap_R_AddRefEntityToScene(&ent);
  trap_R_RenderScene(&refdef);
}

/*
=================
AngleToX
=================
*/
static inline float AngleToX(float const angle)
{
  ASSERT_FLOAT_EQ(angle, AngleNormalizePI(angle));
  float const half_fov_x = cg.refdef.fov_x / 2;
  if (angle >= half_fov_x) return 0;
  if (angle <= -half_fov_x) return SCREEN_WIDTH;
  return SCREEN_WIDTH / 2 * (1 - tanf(angle) / tanf(half_fov_x));
}

typedef struct
{
  float    x1;
  float    x2;
  qboolean split;
} range_t;

static inline range_t AnglesToRange(float start, float end, float const yaw)
{
  if (fabsf(end - start) > 2 * (float)M_PI)
  {
    range_t const ret = { 0, SCREEN_WIDTH, 0 };
    return ret;
  }

  qboolean const split = end > start;
  start                = AngleNormalizePI(start - yaw);
  end                  = AngleNormalizePI(end - yaw);

  if (end > start)
  {
    range_t const ret = { AngleToX(end), AngleToX(start), !split };
    return ret;
  }
  else
  {
    range_t const ret = { AngleToX(start), AngleToX(end), split };
    return ret;
  }
}

void CG_FillAngleYaw(float start, float end, float yaw, float y, float h, vec4_t const color)
{
  range_t const lc_Range = AnglesToRange(start, end, yaw);
  if (!lc_Range.split)
  {
    CG_FillRect(lc_Range.x1, y, lc_Range.x2 - lc_Range.x1, h, color);
  }
  else
  {
    CG_FillRect(0, y, lc_Range.x1, h, color);
    CG_FillRect(lc_Range.x2, y, SCREEN_WIDTH - lc_Range.x2, h, color);
  }
}
