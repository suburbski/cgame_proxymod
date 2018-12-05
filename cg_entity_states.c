#include "cg_local.h"
#include "cg_utils.h"

entityState_t cg_entityStates[1024];

void entityStates_init(void) {
    memset(cg_entityStates, -1, sizeof(cg_entityStates));
}

void entityStates_update(void) {
    snapshot_t *snap = getSnap();
    for (int i = 0; i < snap->numEntities; i++) {
        cg_entityStates[snap->entities[i].number] = snap->entities[i];
    }
}