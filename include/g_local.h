#ifndef G_LOCAL_H
#define G_LOCAL_H

#include "q_shared.h"

//
// g_missile.c
//
void fire_grenade(entityState_t* ent, vec3_t const start, vec3_t dir);
void fire_rocket(entityState_t* ent, vec3_t const start, vec3_t dir);

//
// g_weapon.c
//
void FireWeapon(playerState_t const* pm_ps, entityState_t* ent);

#endif // G_LOCAL_H
