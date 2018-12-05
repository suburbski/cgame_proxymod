#include "cg_local.h"
#include "cg_cvar.h"
#include "cg_utils.h"

#define ENTITYNUM_WORLD 1022

int should_filter_sound(int entity_num, int is_loop) {
	float local_sounds_only;
	cvar_getFloat("mdd_local_sounds_only", &local_sounds_only);

	// todo: also bail if its sp?
	if (!local_sounds_only)
		return 0;

	// this messes up our own weapon noises
	// can't do anything about this w/o df code
	if(!is_loop && entity_num == ENTITYNUM_WORLD)
		return 1;

	// refers to a entity number we dont have (ie. ourself!)
	// no clue whose sound it could be just let it play
	if(cg_entityStates[entity_num].number == -1)
		return 0;

	return cg_entityStates[entity_num].clientNum != getSnap()->ps.clientNum;
}