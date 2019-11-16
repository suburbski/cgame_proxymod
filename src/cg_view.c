#include "cg_local.h"

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame(int32_t serverTime, stereoFrame_t stereoView, qboolean demoPlayback)
{
  (void)stereoView;
  cg.time         = serverTime;
  cg.demoPlayback = demoPlayback;
}
