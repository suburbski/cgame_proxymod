#include "cg_local.h"
#include "cg_cvar.h"
#include "cg_utils.h"

#define ENTITYNUM_WORLD 1022

int should_filter_sound(int entity_num, int is_loop) {
	float local_sounds_only;
	cvar_getFloat("mdd_local_sounds_only", &local_sounds_only);

	// todo: also bail, if its sp?
	if (!local_sounds_only)
		return 0;

	snapshot_t *snap = getSnap();
	if(!is_loop && entity_num == ENTITYNUM_WORLD)
		return 1;

	for(int i = 0; i < snap->numEntities; i++) {
		if(entity_num == snap->entities[i].number &&
			snap->entities[i].clientNum != snap->ps.clientNum)
		{
			return 1;
		}
	}

	return 0;
}