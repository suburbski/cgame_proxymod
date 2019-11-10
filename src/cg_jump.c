#include "cg_jump.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"

static vmCvar_t mdd_hud_jumpDelay_draw;
static vmCvar_t mdd_hud_jumpDelay_offsetX;
static vmCvar_t mdd_hud_jumpDelay_offsetY;
static vmCvar_t mdd_hud_jumpDelay_width;
static vmCvar_t mdd_hud_jumpDelay_height;

static vmCvar_t mdd_hud_jumpDelay_textOffsetX;
static vmCvar_t mdd_hud_jumpDelay_textOffsetY;
static vmCvar_t mdd_hud_jumpDelay_textSize;
static vmCvar_t mdd_hud_jumpDelay_textColor;

static cvarTable_t jump_cvars[] = {
  { &mdd_hud_jumpDelay_draw, "mdd_hud_jumpDelay_draw", "0", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_offsetX, "mdd_hud_jumpDelay_graphOffsetX", "330", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_offsetY, "mdd_hud_jumpDelay_graphOffsetY", "140", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_width, "mdd_hud_jumpDelay_graphWidth", "16", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_height, "mdd_hud_jumpDelay_graphHeight", "300", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_textOffsetX, "mdd_hud_jumpDelay_textOffsetX", "320", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_textOffsetY, "mdd_hud_jumpDelay_textOffsetY", "220", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_textSize, "mdd_hud_jumpDelay_textSize", "16", CVAR_ARCHIVE },
  { &mdd_hud_jumpDelay_textColor, "mdd_hud_jumpDelay_textColor", "7", CVAR_ARCHIVE }
};

void init_jump(void)
{
  init_cvars(jump_cvars, ARRAY_LEN(jump_cvars));
}

typedef struct
{
  float xPos;
  float yPos;
  float textPosX;
  float textPosY;
  float textSize;
  float width;
  float height;

  uint32_t mode;

  // timestamps for computation
  uint32_t t_jumpPreGround;
  uint32_t t_nojumpPostGround;
  uint32_t t_groundTouch;

  // state machine
  uint8_t lastState;

  // draw data
  int32_t postDelay;
  int32_t preDelay;
  int32_t fullDelay;

  vec4_t postJumpColor;
  vec4_t preJumpColor;
  vec4_t textColor;
} hud_jumpDelay_t;

static hud_jumpDelay_t jump_;

enum
{
  AIR_NOJUMP,
  AIR_JUMP,
  GROUND_JUMP,
  GROUND_NOJUMP,
  AIR_JUMPNORELEASE
};

static int8_t hud_jumpDelayControl(hud_jumpDelay_t* jumpHud)
{
  int8_t   inAir = 0, jump = 0, state = 0, lastState = 0;
  uint32_t now;
  /*
   * To draw this hud we have to make a little state machine
   *
   * AIR_NOJUMP:        The player is midair, not holding the jump button
   * AIR_JUMP:          The player is midair, holding jump button
   * GROUND_JUMP:       The player is on the ground, holding jump button
   * GROUND_NOJUMP:     The player is on the ground, not holding jump button
   * AIR_JUMPNORELEASE: The player is midair, without releasing the jump button
   */

  now                           = getSnap()->serverTime;
  playerState_t const* const ps = getPs();
  inAir                         = isInAir(ps);
  jump                          = isJumping(ps);
  lastState                     = jumpHud->lastState;

  // determine current state
  switch (lastState)
  {
  case AIR_JUMP:
  case AIR_NOJUMP:
    if (inAir)
    {
      if (jump)
        state = AIR_JUMP;
      else
        state = AIR_NOJUMP;
    }
    else
    {
      if (jump)
        state = GROUND_JUMP;
      else
        state = GROUND_NOJUMP;
    }
    break;

  // edge case at end of cycle
  case GROUND_NOJUMP:
  case GROUND_JUMP:
  case AIR_JUMPNORELEASE:
    if (inAir)
    {
      if (jump)
        state = AIR_JUMPNORELEASE;
      else
        state = AIR_NOJUMP;
    }
    else
    {
      if (jump)
        state = GROUND_JUMP;
      else
        state = GROUND_NOJUMP;
    }
    break;

  default:
    state = GROUND_NOJUMP;
    break;
  }

  // act on current state
  switch (state)
  {
  case AIR_NOJUMP:
    jumpHud->fullDelay = jumpHud->postDelay + jumpHud->preDelay;
    if (lastState == AIR_JUMP)
    {
      jumpHud->preDelay  = 0;
      jumpHud->postDelay = 0;
    }
    // we spend the most time in this state
    // that is why here we show the last jump stats
    break;

  case AIR_JUMP:
    if (lastState == AIR_NOJUMP)
    {
      jumpHud->t_jumpPreGround = now;
    }
    jumpHud->preDelay = now - jumpHud->t_jumpPreGround; // ms
    break;

  case GROUND_JUMP:
    if (lastState == AIR_JUMP)
    {
      jumpHud->t_groundTouch = now;
    }
    jumpHud->postDelay = 0;
    break;

  case GROUND_NOJUMP:
    jumpHud->t_jumpPreGround = now; // display 0 on 2nd jump CJ
    jumpHud->t_groundTouch   = now;
    jumpHud->preDelay        = 0;
    jumpHud->postDelay       = 0;
    break;

  case AIR_JUMPNORELEASE:
    jumpHud->postDelay = now - jumpHud->t_groundTouch; // ms
    break;

  default:
    break;
  }

  //  g_syscall( CG_PRINT, vaf("%u\n", state));
  jumpHud->lastState = state;

  return qtrue;
}

static int8_t hud_jumpDelaySetup(hud_jumpDelay_t* jumpHud)
{
  float const mdd_hud_opacity = cvar_getValue("mdd_hud_opacity");
  float const draw            = cvar_getValue("mdd_hud_jumpDelay_draw");
  float const widthPx         = cvar_getValue("mdd_hud_jumpDelay_graphWidth");
  float const heightPx        = cvar_getValue("mdd_hud_jumpDelay_graphHeight");
  float       xPos            = cvar_getValue("mdd_hud_jumpDelay_graphOffsetX");
  float       yPos            = cvar_getValue("mdd_hud_jumpDelay_graphOffsetY");
  float       textPosX        = cvar_getValue("mdd_hud_jumpDelay_textOffsetX");
  float       textPosY        = cvar_getValue("mdd_hud_jumpDelay_textOffsetY");
  float const textSize        = cvar_getValue("mdd_hud_jumpDelay_textSize");
  float const textColor       = cvar_getValue("mdd_hud_jumpDelay_textColor");

  convertAdjustedToNative(&xPos, &yPos, &textPosX, &textPosY);

  jumpHud->mode = draw; // 0=off, 1=text, 2=graph, 3=text and graph

  jumpHud->xPos   = xPos;
  jumpHud->yPos   = yPos;
  jumpHud->width  = widthPx;
  jumpHud->height = heightPx;

  jumpHud->preJumpColor[0] = 0.0;
  jumpHud->preJumpColor[1] = 0.0;
  jumpHud->preJumpColor[2] = 1.0;
  jumpHud->preJumpColor[3] = mdd_hud_opacity + 0.5;
  if (jumpHud->preJumpColor[3] > 1.0) jumpHud->preJumpColor[3] = 1.0;

  jumpHud->postJumpColor[0] = 1.0;
  jumpHud->postJumpColor[1] = 0.0;
  jumpHud->postJumpColor[2] = 0.0;
  jumpHud->postJumpColor[3] = mdd_hud_opacity + 0.5;
  if (jumpHud->postJumpColor[3] > 1.0) jumpHud->postJumpColor[3] = 1.0;

  getColor(textColor, 1.0, jumpHud->textColor);

  jumpHud->textPosX = textPosX;
  jumpHud->textPosY = textPosY;
  jumpHud->textSize = textSize;

  return qtrue;
}

static int8_t hud_boxDraw(float x, float y, float w, float h)
{
  // Draw a simple transparent box to put graphs into
  vec4_t backdrop;

  float const hud_opacity = cvar_getValue("mdd_hud_opacity");
  backdrop[0]             = 1.f;
  backdrop[1]             = 1.f;
  backdrop[2]             = 1.f;
  backdrop[3]             = hud_opacity;

  g_syscall(CG_R_SETCOLOR, backdrop);
  CG_DrawPic(x, y, w, h, cgs.media.gfxWhiteShader); // backdrop

  // make border stand out
  backdrop[3] += 0.5;
  if (backdrop[3] > 1.0)
  {
    backdrop[3] = 1.0;
  }

  g_syscall(CG_R_SETCOLOR, backdrop);
  CG_DrawPic(x, y - 1, w, 1, cgs.media.gfxWhiteShader);         // North
  CG_DrawPic(x, y + h, w, 1, cgs.media.gfxWhiteShader);         // South
  CG_DrawPic(x + w, y - 1, 1, h + 2, cgs.media.gfxWhiteShader); // East
  CG_DrawPic(x - 1, y - 1, 1, h + 2, cgs.media.gfxWhiteShader); // West

  return qtrue;
}

void draw_jump(void)
{
  update_cvars(jump_cvars, ARRAY_LEN(jump_cvars));
  hud_jumpDelaySetup(&jump_);

  if (!mdd_hud_jumpDelay_draw.integer) return;

  hud_jumpDelayControl(&jump_);

  const float rangeMs = 300;
  float       middle, upHeight, downHeight, barUp, barDown;

  middle  = jump_.yPos + (jump_.height / 2.0);
  barUp   = jump_.postDelay;
  barDown = jump_.preDelay;

  // clamp values
  if (jump_.postDelay > rangeMs) barUp = rangeMs;

  if (jump_.preDelay > rangeMs) barDown = rangeMs;

  upHeight   = (jump_.height / 2) * (barUp / rangeMs);
  downHeight = (jump_.height / 2) * (barDown / rangeMs);

  // draw graph
  if (jump_.mode & 2)
  {
    hud_boxDraw(jump_.xPos, jump_.yPos, jump_.width, jump_.height);

    g_syscall(CG_R_SETCOLOR, jump_.preJumpColor);
    CG_DrawPic(jump_.xPos, middle, jump_.width, downHeight, cgs.media.gfxWhiteShader);

    g_syscall(CG_R_SETCOLOR, jump_.postJumpColor);
    CG_DrawPic(jump_.xPos, (middle - upHeight), jump_.width, upHeight, cgs.media.gfxWhiteShader);
  }
  // draw text next to it
  if (jump_.mode & 1)
  {
    CG_DrawText(jump_.textPosX, jump_.textPosY, jump_.textSize, jump_.textColor, qfalse, vaf("%i ms", jump_.fullDelay));
  }
}
