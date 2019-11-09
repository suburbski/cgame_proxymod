#include "cg_entity.h"

#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"

static vmCvar_t mdd_local_sounds_only;

static cvarTable_t sound_cvars[] = { { &mdd_local_sounds_only, "mdd_local_sounds_only", "0", CVAR_ARCHIVE } };

static entityState_t cg_entityStates[1024];

void init_entityStates(void)
{
  init_cvars(sound_cvars, ARRAY_LEN(sound_cvars));
  memset(cg_entityStates, -1, sizeof(cg_entityStates));
}

void update_entityStates(void)
{
  snapshot_t const* const snap = getSnap();
  for (int i = 0; i < snap->numEntities; i++)
  {
    cg_entityStates[snap->entities[i].number] = snap->entities[i];
  }
}

int8_t should_filter_sound(int entity_num, int8_t is_loop)
{
  update_cvars(sound_cvars, ARRAY_LEN(sound_cvars));

  // todo: also bail if its sp?
  if (!mdd_local_sounds_only.integer) return 0;

  // this messes up our own weapon noises
  // can't do anything about this w/o df code
  if (!is_loop && entity_num == ENTITYNUM_WORLD) return 1;

  // refers to a entity number we dont have (ie. ourself!)
  // no clue whose sound it could be just let it play
  if (cg_entityStates[entity_num].number == -1) return 0;

  return cg_entityStates[entity_num].clientNum != getSnap()->ps.clientNum;
}
