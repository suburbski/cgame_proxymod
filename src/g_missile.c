#include "cg_local.h"
#include "g_local.h"

#define MISSILE_PRESTEP_TIME 50

/*
=================
fire_grenade
=================
*/
void fire_grenade(entityState_t* ent, vec3_t const start, vec3_t dir)
{
  VectorNormalize(dir);

  ent->pos.trType = TR_GRAVITY;
  ent->pos.trTime = cg.time - MISSILE_PRESTEP_TIME; // move a bit on the very first frame
  VectorCopy(start, ent->pos.trBase);
  VectorScale(dir, 700, ent->pos.trDelta);
  SnapVector(ent->pos.trDelta); // save net bandwidth
}

/*
=================
fire_rocket
=================
*/
void fire_rocket(entityState_t* ent, vec3_t const start, vec3_t dir)
{
  VectorNormalize(dir);

  ent->pos.trType = TR_LINEAR;
  ent->pos.trTime = cg.time - MISSILE_PRESTEP_TIME; // move a bit on the very first frame
  VectorCopy(start, ent->pos.trBase);
  VectorScale(dir, 900, ent->pos.trDelta);
  SnapVector(ent->pos.trDelta); // save net bandwidth
}
