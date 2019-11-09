#include "cg_entity.h"

#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"

entityState_t cg_entityStates[1024];

void init_entityStates(void)
{
  memset(cg_entityStates, -1, sizeof(cg_entityStates));
}

void update_entityStates(void)
{
  snapshot_t* snap = getSnap();
  for (int i = 0; i < snap->numEntities; i++)
  {
    cg_entityStates[snap->entities[i].number] = snap->entities[i];
  }
}

int8_t should_filter_sound(int entity_num, int8_t is_loop)
{
  float const local_sounds_only = cvar_getValue("mdd_local_sounds_only");

  // todo: also bail if its sp?
  if (!local_sounds_only) return 0;

  // this messes up our own weapon noises
  // can't do anything about this w/o df code
  if (!is_loop && entity_num == ENTITYNUM_WORLD) return 1;

  // refers to a entity number we dont have (ie. ourself!)
  // no clue whose sound it could be just let it play
  if (cg_entityStates[entity_num].number == -1) return 0;

  return cg_entityStates[entity_num].clientNum != getSnap()->ps.clientNum;
}
