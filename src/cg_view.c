#include "cg_cvar.h"
#include "cg_local.h"

static vmCvar_t fov;

static cvarTable_t view_cvars[] = { { &fov, "mdd_fov", "0", CVAR_ARCHIVE } };

/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect(void)
{
  cg.refdef.width = cgs.glconfig.vidWidth;

  cg.refdef.height = cgs.glconfig.vidHeight;

  cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width) / 2;
  cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height) / 2;
}

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
static void CG_CalcFov(void)
{
  float fov_x = fov.value ? fov.value : cvar_getValue("cg_fov");
  if (fov_x < 1)
  {
    fov_x = 1;
  }
  else if (fov_x > 160)
  {
    fov_x = 160;
  }

  // set it
  cg.refdef.fov_x = DEG2RAD(fov_x);
  cg.refdef.fov_y = atan2f(cg.refdef.height, cg.refdef.width / tanf(cg.refdef.fov_x / 2)) * 2;
}

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static void CG_CalcViewValues(void)
{
  memset(&cg.refdef, 0, sizeof(cg.refdef));

  // calculate size of 3D view
  CG_CalcVrect();

  // field of view
  CG_CalcFov();
}

void init_view(void)
{
  init_cvars(view_cvars, ARRAY_LEN(view_cvars));
}

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame(int32_t serverTime, stereoFrame_t stereoView, qboolean demoPlayback)
{
  (void)stereoView;

  update_cvars(view_cvars, ARRAY_LEN(view_cvars));

  cg.time         = serverTime;
  cg.demoPlayback = demoPlayback;

  // build cg.refdef
  CG_CalcViewValues();

  cg.refdef.time = cg.time;
}
