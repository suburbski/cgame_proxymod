#include "cg_cgaz.h"

#include "assert.h"
#include "bg_pmove.h"
#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_local.h"
#include "cg_utils.h"

static vmCvar_t cgaz;
static vmCvar_t cgaz_trueness;
static vmCvar_t cgaz_speed;
static vmCvar_t cgaz_yh;
static vmCvar_t cgaz_rgbaNoAccel;
static vmCvar_t cgaz_rgbaPartialAccel;
static vmCvar_t cgaz_rgbaFullAccel;
static vmCvar_t cgaz_rgbaTurnZone;

static cvarTable_t cgaz_cvars[] = {
  { &cgaz, "mdd_cgaz", "0b1", CVAR_ARCHIVE_ND },
  { &cgaz_trueness, "mdd_cgaz_trueness", "0b110", CVAR_ARCHIVE_ND },
  { &cgaz_speed, "mdd_cgaz_speed", "1", CVAR_ARCHIVE_ND },
  { &cgaz_yh, "mdd_cgaz_yh", "180 12", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaNoAccel, "mdd_cgaz_rgbaNoAccel", ".25 .25 .25 .5", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaPartialAccel, "mdd_cgaz_rgbaPartialAccel", "0 1 0 .5", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaFullAccel, "mdd_cgaz_rgbaFullAccel", "0 .25 .25 .5", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaTurnZone, "mdd_cgaz_rgbaTurnZone", "1 1 0 .5", CVAR_ARCHIVE_ND },
};

// mdd_cgaz 0b X
//             |
//             + - draw
#define CGAZ_DRAW 1

// mdd_cgaz_trueness 0b X X X
//                      | | |
//                      | | + - jump/crouch influence
//                      | + - - CPM air control zones
//                      + - - - ground
#define CGAZ_JUMPCROUCH 1
#define CGAZ_CPM 2
#define CGAZ_GROUND 4

void init_cgaz(void)
{
  init_cvars(cgaz_cvars, ARRAY_LEN(cgaz_cvars));
}

typedef struct
{
  float v_squared;
  float vf_squared;
  float a_squared;

  float v;
  float vf;
  float a;

  float wishspeed;
} state_t;

typedef struct
{
  vec2_t graph_yh;

  vec4_t graph_rgbaNoAccel;
  vec4_t graph_rgbaPartialAccel;
  vec4_t graph_rgbaFullAccel;
  vec4_t graph_rgbaTurnZone;

  float d_min;
  float d_opt;
  float d_max_cos;
  float d_max;

  vec2_t wishvel;

  // float p;
  // float prev_d;
  // float prev_vf_squared;
  // float prev_wishspeed;
  // float prev_a;

  // float p_min;
  // float p_opt;
  // float p_max_cos;
  // float p_max;

  pmove_t       pm;
  playerState_t pm_ps;
  pml_t         pml;

  state_t t;
} cgaz_t;

static cgaz_t s;

static void PmoveSingle(void);
static void PM_AirMove(void);
static void PM_WalkMove(void);

void draw_cgaz(void)
{
  update_cvars(cgaz_cvars, ARRAY_LEN(cgaz_cvars));

  cgaz.integer = cvar_getInteger("mdd_cgaz");
  if (!(cgaz.integer & CGAZ_DRAW)) return;

  cgaz_trueness.integer = cvar_getInteger("mdd_cgaz_trueness");

  s.pm_ps = *getPs();

  s.pm.tracemask = s.pm_ps.pm_type == PM_DEAD ? MASK_PLAYERSOLID & ~CONTENTS_BODY : MASK_PLAYERSOLID;

  if (VectorLengthSquared2(s.pm_ps.velocity) >= cgaz_speed.value * cgaz_speed.value) PmoveSingle();
}

static void PmoveSingle(void)
{
  int8_t const scale = s.pm_ps.stats[13] & PSF_USERINPUT_WALK ? 64 : 127;
  if (!cg.demoPlayback && !(s.pm_ps.pm_flags & PMF_FOLLOW))
  {
    int32_t const cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &s.pm.cmd);
  }
  else
  {
    s.pm.cmd.forwardmove = scale * ((s.pm_ps.stats[13] & PSF_USERINPUT_FORWARD) / PSF_USERINPUT_FORWARD -
                                    (s.pm_ps.stats[13] & PSF_USERINPUT_BACKWARD) / PSF_USERINPUT_BACKWARD);
    s.pm.cmd.rightmove   = scale * ((s.pm_ps.stats[13] & PSF_USERINPUT_RIGHT) / PSF_USERINPUT_RIGHT -
                                  (s.pm_ps.stats[13] & PSF_USERINPUT_LEFT) / PSF_USERINPUT_LEFT);
    s.pm.cmd.upmove      = scale * ((s.pm_ps.stats[13] & PSF_USERINPUT_JUMP) / PSF_USERINPUT_JUMP -
                               (s.pm_ps.stats[13] & PSF_USERINPUT_CROUCH) / PSF_USERINPUT_CROUCH);
  }

  // clear all pmove local vars
  memset(&s.pml, 0, sizeof(s.pml));

  // save old velocity for crashlanding
  VectorCopy(s.pm_ps.velocity, s.pml.previous_velocity);

  AngleVectors(s.pm_ps.viewangles, s.pml.forward, s.pml.right, s.pml.up);

  if (s.pm.cmd.upmove < 10)
  {
    // not holding jump
    s.pm_ps.pm_flags &= ~PMF_JUMP_HELD;
  }

  if (s.pm_ps.pm_type >= PM_DEAD)
  {
    s.pm.cmd.forwardmove = 0;
    s.pm.cmd.rightmove   = 0;
    s.pm.cmd.upmove      = 0;
  }

  // Use default key combination when no user input
  if (!s.pm.cmd.forwardmove && !s.pm.cmd.rightmove)
  {
    s.pm.cmd.forwardmove = scale;
  }

  // set mins, maxs, and viewheight
  PM_CheckDuck(&s.pm, &s.pm_ps);

  // set watertype, and waterlevel
  PM_SetWaterLevel(&s.pm, &s.pm_ps);

  // set groundentity
  PM_GroundTrace(&s.pm, &s.pm_ps, &s.pml);

  // if ( s.pm_ps.pm_type == PM_DEAD ) {
  //   PM_DeadMove ();
  // }

  if (s.pm_ps.powerups[PW_FLIGHT])
  {
    // // flight powerup doesn't allow jump and has different friction
    // PM_FlyMove();
    return;
  }
  else if (s.pm_ps.pm_flags & PMF_GRAPPLE_PULL)
  {
    // PM_GrappleMove();
    // // We can wiggle a bit
    // PM_AirMove();
    return;
  }
  else if (s.pm_ps.pm_flags & PMF_TIME_WATERJUMP)
  {
    // PM_WaterJumpMove();
    return;
  }
  else if (s.pm.waterlevel > 1)
  {
    // // swimming
    // PM_WaterMove();
    return;
  }
  else if (s.pml.walking)
  {
    // walking on ground
    PM_WalkMove();
  }
  else
  {
    // airborne
    PM_AirMove();
  }
}

static void CG_DrawCGaz(void)
{
  float const yaw = atan2f(s.wishvel[1], s.wishvel[0]) - atan2f(s.pml.previous_velocity[1], s.pml.previous_velocity[0]);

  ParseVec(cgaz_yh.string, s.graph_yh, 2);
  ParseVec(cgaz_rgbaNoAccel.string, s.graph_rgbaNoAccel, 4);
  ParseVec(cgaz_rgbaPartialAccel.string, s.graph_rgbaPartialAccel, 4);
  ParseVec(cgaz_rgbaFullAccel.string, s.graph_rgbaFullAccel, 4);
  ParseVec(cgaz_rgbaTurnZone.string, s.graph_rgbaTurnZone, 4);

  CG_FillAngleYaw(-s.d_min, +s.d_min, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaNoAccel);

  CG_FillAngleYaw(+s.d_min, +s.d_opt, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(-s.d_opt, -s.d_min, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);

  CG_FillAngleYaw(+s.d_opt, +s.d_max_cos, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);
  CG_FillAngleYaw(-s.d_max_cos, -s.d_opt, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);

  CG_FillAngleYaw(+s.d_max_cos, +s.d_max, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
  CG_FillAngleYaw(-s.d_max, -s.d_max_cos, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);

  // #define SCREEN_CENTER_X (SCREEN_WIDTH / 2)
  //   float const scale = 1;
  //   s.p *= scale;
  //   s.p_min *= scale;
  //   s.p_opt *= scale;
  //   s.p_max_cos *= scale;
  //   s.p_max *= scale;
  //   CG_FillRect(SCREEN_CENTER_X - s.p, s.graph_yh[0] - 30, s.p_min, s.graph_yh[1], s.graph_rgbaNoAccel);
  //   CG_FillRect(
  //     SCREEN_CENTER_X + s.p_min - s.p, s.graph_yh[0] - 30, s.p_opt - s.p_min, s.graph_yh[1],
  //     s.graph_rgbaPartialAccel);
  //   CG_FillRect(
  //     SCREEN_CENTER_X + s.p_opt - s.p, s.graph_yh[0] - 30, s.p_max_cos - s.p_opt, s.graph_yh[1],
  //     s.graph_rgbaFullAccel);
  //   CG_FillRect(SCREEN_CENTER_X + s.p_max_cos - s.p,
  //               s.graph_yh[0] - 30,
  //               s.p_max - s.p_max_cos,
  //               s.graph_yh[1],
  //               s.graph_rgbaTurnZone);

  //   /// et CGaz
  //   // absoluate velocity angle
  //   float vel_angle = AngleNormalize180(RAD2DEG(atan2f(s.pml.previous_velocity[1], s.pml.previous_velocity[0])));
  //   // relative velocity angle to viewangles[1]
  //   float vel_relang = AngleNormalize180(s.pm_ps.viewangles[YAW] - vel_angle);
  //   vel_relang       = DEG2RAD(vel_relang);
  //   float vel_size = VectorLength2(s.pm_ps.velocity);

  // #define SCREEN_CENTER_X (cgs.glconfig.vidWidth / 2)
  // #define SCREEN_CENTER_Y (cgs.glconfig.vidHeight / 2)

  //   DrawLine(SCREEN_CENTER_X,
  //            SCREEN_CENTER_Y,
  //            SCREEN_CENTER_X + s.pm.cmd.rightmove,
  //            SCREEN_CENTER_Y - s.pm.cmd.forwardmove,
  //            colorCyan);

  //   vel_size /= 5;
  //   DrawLine(SCREEN_CENTER_X,
  //            SCREEN_CENTER_Y,
  //            SCREEN_CENTER_X + vel_size * sin(vel_relang),
  //            SCREEN_CENTER_Y - vel_size * cos(vel_relang),
  //            colorRed);
  //   if (vel_size > SCREEN_HEIGHT / 2)
  //   {
  //     vel_size = SCREEN_HEIGHT / 2;
  //   }
  //   vel_size /= 2;
  //   DrawLine(SCREEN_CENTER_X,
  //            SCREEN_CENTER_Y,
  //            SCREEN_CENTER_X + vel_size * sin(vel_relang + s.d_opt),
  //            SCREEN_CENTER_Y - vel_size * cos(vel_relang + s.d_opt),
  //            s.graph_rgbaFullAccel);
  //   DrawLine(SCREEN_CENTER_X,
  //            SCREEN_CENTER_Y,
  //            SCREEN_CENTER_X + vel_size * sin(vel_relang - s.d_opt),
  //            SCREEN_CENTER_Y - vel_size * cos(vel_relang - s.d_opt),
  //            s.graph_rgbaFullAccel);

  //   DrawLine(SCREEN_CENTER_X,
  //            SCREEN_CENTER_Y,
  //            SCREEN_CENTER_X + vel_size * sin(vel_relang + s.d_min),
  //            SCREEN_CENTER_Y - vel_size * cos(vel_relang + s.d_min),
  //            s.graph_rgbaPartialAccel);
  //   DrawLine(SCREEN_CENTER_X,
  //            SCREEN_CENTER_Y,
  //            SCREEN_CENTER_X + vel_size * sin(vel_relang - s.d_min),
  //            SCREEN_CENTER_Y - vel_size * cos(vel_relang - s.d_min),
  //            s.graph_rgbaPartialAccel);
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
// TODO: Write assert to assume 0 <= cT <= 1
static void PM_Friction(void)
{
  // ignore slope movement
  float const speed = s.pml.walking ? VectorLength2(s.pm_ps.velocity) : VectorLength(s.pm_ps.velocity);
  if (speed < 1)
  {
    s.pm_ps.velocity[0] = 0;
    s.pm_ps.velocity[1] = 0; // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  // apply ground friction
  float drop = 0;
  if (
    s.pm.waterlevel <= 1 && s.pml.walking && !(s.pml.groundTrace.surfaceFlags & SURF_SLICK) &&
    !(s.pm_ps.pm_flags & PMF_TIME_KNOCKBACK))
  {
    float const control = speed < pm_stopspeed ? pm_stopspeed : speed;
    drop += control * pm_friction * pm_frametime;
  }

  // apply water friction even if just wading
  if (s.pm.waterlevel)
  {
    drop += speed * pm_waterfriction * s.pm.waterlevel * pm_frametime;
  }

  // apply flying friction
  if (s.pm_ps.powerups[PW_FLIGHT])
  {
    drop += speed * pm_flightfriction * pm_frametime;
  }

  if (s.pm_ps.pm_type == PM_SPECTATOR)
  {
    drop += speed * pm_spectatorfriction * pm_frametime;
  }

  // scale the velocity
  float newspeed = speed - drop;
  if (newspeed < 0)
  {
    newspeed = 0;
  }
  newspeed /= speed;

  for (uint8_t i = 0; i < 3; ++i) s.pm_ps.velocity[i] *= newspeed;
}

static void update_d_min(void)
{
#ifndef NDEBUG
  if (s.t.a == 0)
  {
    ASSERT_EQ(s.t.v_squared - s.t.vf_squared, 2 * s.t.a * s.t.wishspeed - s.t.a_squared);
  }
  else
  {
    ASSERT_LT(s.t.v_squared - s.t.vf_squared, 2 * s.t.a * s.t.wishspeed - s.t.a_squared);
  }
#endif
  float const num_squared = s.t.wishspeed * s.t.wishspeed - s.t.v_squared + s.t.vf_squared;
  ASSERT_GE(num_squared, 0);
  float const num = sqrtf(num_squared);
  s.d_min         = num >= s.t.vf ? 0 : acosf(num / s.t.vf);
}

static void update_d_opt(void)
{
  float const num = s.t.wishspeed - s.t.a;
  s.d_opt         = num >= s.t.vf ? 0 : acosf(num / s.t.vf);
}

static void update_d_max_cos(void)
{
  float const num = s.t.v - s.t.vf;
  s.d_max_cos     = num >= s.t.a ? 0 : acosf(num / s.t.a);
  if (s.d_max_cos < s.d_opt)
  {
    ASSERT_FLOAT_GE(s.t.v * s.t.vf - s.t.vf_squared, s.t.a * s.t.wishspeed - s.t.a_squared);
    s.d_max_cos = s.d_opt;
  }
}

static void update_d_max(void)
{
#ifndef NDEBUG
  if (s.t.a == 0)
  {
    ASSERT_EQ(s.t.v_squared - s.t.vf_squared, 2 * s.t.a * s.t.wishspeed - s.t.a_squared);
  }
  else
  {
    ASSERT_LT(s.t.v_squared - s.t.vf_squared, 2 * s.t.a * s.t.wishspeed - s.t.a_squared);
  }
#endif
  float const num = s.t.v_squared - s.t.vf_squared - s.t.a_squared;
  float const den = 2 * s.t.a * s.t.vf;
  if (num >= den)
  {
    s.d_max = 0;
  }
  else if (-num >= den)
  {
    s.d_max = (float)M_PI;
  }
  else
  {
    s.d_max = acosf(num / den);
  }
}

/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate(float const wishspeed, float const accel)
{
  s.t.v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
  s.t.vf_squared = VectorLengthSquared2(s.pm_ps.velocity);
  s.t.wishspeed  = wishspeed;
  s.t.a          = accel * s.t.wishspeed * pm_frametime;
  s.t.a_squared  = s.t.a * s.t.a;
  if (
    !(cgaz_trueness.integer & CGAZ_GROUND) ||
    s.t.v_squared - s.t.vf_squared >= 2 * s.t.a * s.t.wishspeed - s.t.a_squared)
  {
    s.t.v_squared = s.t.vf_squared;
  }
  s.t.v  = sqrtf(s.t.v_squared);
  s.t.vf = sqrtf(s.t.vf_squared);

  ASSERT_LE(s.t.a * pm_frametime, 1);

  update_d_min();
  update_d_opt();
  update_d_max_cos();
  update_d_max();

  ASSERT_LE(s.d_min, s.d_opt);
  ASSERT_LE(s.d_opt, s.d_max_cos);
  ASSERT_LE(s.d_max_cos, s.d_max);

  // {
  //   float const d        = DEG2RAD(s.pm_ps.viewangles[YAW]) + s.prev_d;
  //   float       cosine   = cosf(d);
  //   float const prev_vf  = sqrtf(s.prev_vf_squared);
  //   float       addspeed = s.prev_wishspeed - prev_vf * cosine;
  //   if (addspeed < 0) addspeed = 0;
  //   float const accelspeed = s.prev_a > addspeed ? addspeed : s.prev_a;
  //   float const r          = sqrtf(s.prev_vf_squared + accelspeed * accelspeed + 2 * prev_vf * accelspeed * cosine);
  //   s.p                    = fabsf(RAD2DEG(asinf(accelspeed * sinf(d) / r)) / pm_frametime);

  //   s.prev_d = -atan2f(s.pml.previous_velocity[1], s.pml.previous_velocity[0]) +
  //              atan2f(-s.pm.cmd.rightmove, s.pm.cmd.forwardmove);
  //   s.prev_vf_squared = vf_squared;
  //   s.prev_wishspeed  = wishspeed;
  //   s.prev_a          = a;
  // }
  // {
  //   s.p_min = RAD2DEG(asinf(a * sqrtf(v_squared - wishspeed * wishspeed) / (v * vf))) / pm_frametime;
  // }
  // {
  //   float const tmp = vf_squared - a * a + 2 * a * wishspeed;
  //   s.p_opt         = RAD2DEG(asinf(a * sqrtf(tmp - wishspeed * wishspeed) / (vf * sqrtf(tmp)))) / pm_frametime;
  // }
  // {
  //   float const tmp = a * a - vf_squared + 2 * v * vf;
  //   s.p_max_cos     = RAD2DEG(asinf(sqrtf(tmp - v_squared) / sqrtf(tmp))) / pm_frametime;
  //   // if (s.p_max_cos < s.p_opt)
  // }
  // {
  //   float const tmp = v_squared - vf_squared - a * a;
  //   s.p_max         = RAD2DEG(asinf(sqrtf(4 * a * a * vf_squared - tmp * tmp) / (2 * v * vf))) / pm_frametime;
  // }
  // g_syscall(CG_PRINT, vaf("a: %1.3f - %1.3f %1.3f %1.3f %1.3f\n", s.p, s.p_min, s.p_opt, s.p_max_cos, s.p_max));

  CG_DrawCGaz();
}

static void PM_SlickAccelerate(float const wishspeed, float const accel)
{
  float const g          = s.pm_ps.gravity * pm_frametime;
  float const g_squared  = g * g;
  float const a          = accel * wishspeed * pm_frametime;
  float       v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
  float const vf_squared = VectorLengthSquared2(s.pm_ps.velocity);
  if (!(cgaz_trueness.integer & CGAZ_GROUND) || v_squared - vf_squared >= 2 * a * wishspeed - a * a)
  {
    v_squared = vf_squared;
  }
  // float const v  = sqrtf(v_squared);
  float const vf = sqrtf(vf_squared);
  {
    float const num = sqrtf(wishspeed * wishspeed - v_squared + vf_squared + g_squared);
    s.d_min         = num >= vf ? 0 : acosf(num / vf);
  }
  {
    float const num = wishspeed - a;
    s.d_opt         = num >= vf ? 0 : acosf(num / vf);
  }
  {
    float const num = sqrtf(v_squared - g_squared) - vf;
    s.d_max_cos     = num >= a ? 0 : acosf(num / a);
    if (s.d_max_cos < s.d_opt) s.d_max_cos = s.d_opt; // v * vf - vf_squared >= a * wishspeed - a * a
  }
  {
    float const num = v_squared - vf_squared - a * a - g_squared;
    float const den = 2 * a * vf;
    if (num >= den)
    {
      s.d_max = 0;
    }
    else if (-num >= den)
    {
      s.d_max = (float)M_PI;
    }
    else
    {
      s.d_max = acosf(num / den);
    }
    if (s.d_max < s.d_max_cos)
    {
      ASSERT_FALSE(a);
      s.d_max = s.d_max_cos;
    }
  }

  ASSERT_LE(s.d_min, s.d_opt);
  ASSERT_LE(s.d_opt, s.d_max_cos);
  ASSERT_LE(s.d_max_cos, s.d_max);

  CG_DrawCGaz();
}

/*
===================
PM_AirMove

===================
*/
static void PM_AirMove(void)
{
  PM_Friction();

  float const scale = cgaz_trueness.integer & CGAZ_JUMPCROUCH ? PM_CmdScale(&s.pm_ps, &s.pm.cmd)
                                                              : PM_AltCmdScale(&s.pm_ps, &s.pm.cmd);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2]   = 0;
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  for (uint8_t i = 0; i < 2; ++i)
  {
    s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
  }

  float const wishspeed = scale * VectorLength2(s.wishvel);

  if (cgaz_trueness.integer & CGAZ_CPM && s.pm_ps.pm_flags & PMF_PROMODE)
  {
    if (s.pm.cmd.forwardmove == 0 && s.pm.cmd.rightmove != 0)
    {
      PM_Accelerate(wishspeed > cpm_airwishspeed ? cpm_airwishspeed : wishspeed, cpm_airstrafeaccelerate);
    }
    else
    {
      // Air control when s.pm.cmd.forwardmove != 0 && s.pm.cmd.rightmove == 0 only changes direction
      PM_Accelerate(wishspeed, pm_airaccelerate);
      if (s.d_max > (float)M_PI / 2)
      {
        float       v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
        float const vf_squared = VectorLengthSquared2(s.pm_ps.velocity);
        float const a          = cpm_airstopaccelerate * wishspeed * pm_frametime;
        if (v_squared - vf_squared >= 2 * a * wishspeed - a * a) v_squared = vf_squared;
        float const vf = sqrtf(vf_squared);
        {
          float const num = v_squared - vf_squared - a * a;
          float const den = 2 * a * vf;
          if (num >= den)
          {
            s.d_max = 0;
          }
          else if (-num >= den)
          {
            s.d_max = (float)M_PI;
          }
          else
          {
            s.d_max = acosf(num / den);
          }
        }
        ASSERT_LE(s.d_max_cos, s.d_max);
      }
    }
  }

  else
  {
    PM_Accelerate(wishspeed, pm_airaccelerate);
  }

  // // we may have a ground plane that is very steep, even
  // // though we don't have a groundentity
  // // slide along the steep plane
  // if (s.pml->groundPlane)
  // {
  //   PM_ClipVelocity(vf, s.pml->groundTrace.plane.normal, vf, OVERCLIP);
  // }

#if 0
  // ZOID:  If we are on the grapple, try stair-stepping
  // this allows a player to use the grapple to pull himself
  // over a ledge
  if (s.pm_ps.pm_flags & PMF_GRAPPLE_PULL)
    PM_StepSlideMove(qtrue);
  else
    PM_SlideMove(qtrue);
#endif

  // PM_StepSlideMove(qtrue);
}

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove(void)
{
  if (s.pm.waterlevel > 2 && DotProduct(s.pml.forward, s.pml.groundTrace.plane.normal) > 0)
  {
    // // begin swimming
    // PM_WaterMove();
    return;
  }

  if (PM_CheckJump(&s.pm, &s.pm_ps, &s.pml))
  {
    // jumped away
    if (s.pm.waterlevel > 1)
    {
      // PM_WaterMove();
    }
    else
    {
      PM_AirMove();
    }
    return;
  }

  PM_Friction();

  float const scale = cgaz_trueness.integer & CGAZ_JUMPCROUCH ? PM_CmdScale(&s.pm_ps, &s.pm.cmd)
                                                              : PM_AltCmdScale(&s.pm_ps, &s.pm.cmd);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2]   = 0;

  // TODO: only flat ground correct now
  // // project the forward and right directions onto the ground plane
  // PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
  // PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
  //
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  for (uint8_t i = 0; i < 2; ++i)
  {
    s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
  }

  float wishspeed = scale * VectorLength2(s.wishvel);

  // clamp the speed lower if ducking
  if (s.pm_ps.pm_flags & PMF_DUCKED && wishspeed > s.pm_ps.speed * pm_duckScale)
  {
    wishspeed = s.pm_ps.speed * pm_duckScale;
  }

  // clamp the speed lower if wading or walking on the bottom
  if (s.pm.waterlevel)
  {
    float const waterScale = 1.f - (1.f - pm_swimScale) * s.pm.waterlevel / 3.f;
    if (wishspeed > s.pm_ps.speed * waterScale)
    {
      wishspeed = s.pm_ps.speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if (s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.pm_ps.pm_flags & PMF_TIME_KNOCKBACK)
  {
    // PM_Accelerate(wishspeed, s.pm_ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.d_min, s.d_opt, s.d_max_cos, s.d_max));
    PM_SlickAccelerate(wishspeed, s.pm_ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.d_min, s.d_opt, s.d_max_cos, s.d_max));
  }
  else
  {
    // don't reset the z velocity for slopes
    // s.pm_ps.velocity[2] = 0;
    PM_Accelerate(wishspeed, s.pm_ps.pm_flags & PMF_PROMODE ? cpm_accelerate : pm_accelerate);
  }

  // // don't do anything if standing still
  // if (!s.pm_ps.velocity[0] && !s.pm_ps.velocity[1])
  // {
  //   return;
  // }

  // PM_StepSlideMove(qfalse);
}
