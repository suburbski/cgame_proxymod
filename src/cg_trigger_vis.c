#include <stdio.h>
#include <stdlib.h>
#include "cg_local.h"
#include "cg_draw.h"
#include "cg_cvar.h"
#include "cg_utils.h"
#include "surfaceflags.h"
#include "q_math.h"

#define MAX_SUBMODELS           256
#define MAX_TOKEN_CHARS         1024

qhandle_t bboxShader;
qhandle_t bboxShader_nocull;

qboolean trigger[MAX_SUBMODELS];

static vmCvar_t trigger_draw;

static cvarTable_t trigger_cvars[] = {
    {&trigger_draw, "mdd_triggers_draw", "0", CVAR_ARCHIVE}
};

static void parse_triggers(void);
static void R_DrawBBox(vec3_t origin, vec3_t mins, vec3_t maxs, vec4_t color);
static void init_trigger_cvars(void);
static void update_trigger_cvars(void);

void trigger_vis_init(void) {
    init_trigger_cvars();
    bboxShader = g_syscall(CG_R_REGISTERSHADER, "bbox");
    bboxShader_nocull = g_syscall(CG_R_REGISTERSHADER, "bbox_nocull");
    memset(trigger, 0, sizeof(trigger));
    parse_triggers();
}

void trigger_vis_render(void) {
    update_trigger_cvars();

    if(!trigger_draw.integer)
        return;

    int num_models = g_syscall(CG_CM_NUMINLINEMODELS);
    for(int i = 0; i < num_models; i++) {
        if (trigger[i]) {
            vec3_t mins;
            vec3_t maxs;
            vec4_t color = { 0, 128, 0, 255 };
            g_syscall(CG_R_MODELBOUNDS, i + 1, mins, maxs);
            R_DrawBBox(vec3_origin, mins, maxs, color);
        }
    }
}

// ripped from breadsticks
static void parse_triggers(void) {
    char token[MAX_TOKEN_CHARS];
    for( ;; ) {
        qboolean is_trigger = qfalse;
        int model = -1;

        if (!g_syscall(CG_GET_ENTITY_TOKEN, token, sizeof(token)))
            break;

        if ( token[0] != '{' )
            g_syscall(CG_ERROR, "mape is borked");

        for( ;; ) {
             g_syscall(CG_GET_ENTITY_TOKEN, token, sizeof(token));

            if ( token[0] == '}' )
                break;

            if ( !strcmp( token, "model" ) ) {
                g_syscall(CG_GET_ENTITY_TOKEN, token, sizeof(token));
                if ( token[0] == '*' )
                    model = atoi( token+1 );
            }

            if ( !strcmp( token, "classname" ) ) {
                g_syscall(CG_GET_ENTITY_TOKEN, token, sizeof(token) );
                is_trigger = !!strstr( token, "trigger" );
            }
        }

        if ( is_trigger && model > 0)
            trigger[model] = qtrue; // why +1? idk
    }
}

// ripped from breadsticks
static void R_DrawBBox(vec3_t origin, vec3_t mins, vec3_t maxs, vec4_t color) {
    int i;
    float extx, exty, extz;
    polyVert_t verts[4];
    vec3_t corners[8];

    // get the extents (size)
    extx = maxs[0] - mins[0];
    exty = maxs[1] - mins[1];
    extz = maxs[2] - mins[2];

    // set the polygon's texture coordinates
    verts[0].st[0] = 0;
    verts[0].st[1] = 0;
    verts[1].st[0] = 0;
    verts[1].st[1] = 1;
    verts[2].st[0] = 1;
    verts[2].st[1] = 1;
    verts[3].st[0] = 1;
    verts[3].st[1] = 0;

    // set the polygon's vertex colors
    for (i = 0; i < 4; i++) {
        //memcpy( verts[i].modulate, color, sizeof(verts[i].modulate) );
        verts[i].modulate[0] = color[0];
        verts[i].modulate[1] = color[1];
        verts[i].modulate[2] = color[2];
        verts[i].modulate[3] = color[3];
    }

    VectorAdd(origin, maxs, corners[3]);

    VectorCopy(corners[3], corners[2]);
    corners[2][0] -= extx;

    VectorCopy(corners[2], corners[1]);
    corners[1][1] -= exty;

    VectorCopy(corners[1], corners[0]);
    corners[0][0] += extx;

    for (i = 0; i < 4; i++) {
        VectorCopy(corners[i], corners[i + 4]);
        corners[i + 4][2] -= extz;
    }

    // top
    VectorCopy(corners[0], verts[0].xyz);
    VectorCopy(corners[1], verts[1].xyz);
    VectorCopy(corners[2], verts[2].xyz);
    VectorCopy(corners[3], verts[3].xyz);
    g_syscall(CG_R_ADDPOLYTOSCENE, bboxShader, 4, verts, 1);

    // bottom
    VectorCopy(corners[7], verts[0].xyz);
    VectorCopy(corners[6], verts[1].xyz);
    VectorCopy(corners[5], verts[2].xyz);
    VectorCopy(corners[4], verts[3].xyz);
    g_syscall(CG_R_ADDPOLYTOSCENE, bboxShader, 4, verts, 1);

    // top side
    VectorCopy(corners[3], verts[0].xyz);
    VectorCopy(corners[2], verts[1].xyz);
    VectorCopy(corners[6], verts[2].xyz);
    VectorCopy(corners[7], verts[3].xyz);
    g_syscall(CG_R_ADDPOLYTOSCENE, bboxShader_nocull, 4, verts, 1);

    // left side
    VectorCopy(corners[2], verts[0].xyz);
    VectorCopy(corners[1], verts[1].xyz);
    VectorCopy(corners[5], verts[2].xyz);
    VectorCopy(corners[6], verts[3].xyz);
    g_syscall(CG_R_ADDPOLYTOSCENE, bboxShader_nocull, 4, verts, 1);

    // right side
    VectorCopy(corners[0], verts[0].xyz);
    VectorCopy(corners[3], verts[1].xyz);
    VectorCopy(corners[7], verts[2].xyz);
    VectorCopy(corners[4], verts[3].xyz);
    g_syscall(CG_R_ADDPOLYTOSCENE, bboxShader_nocull, 4, verts, 1);

    // bottom side
    VectorCopy(corners[1], verts[0].xyz);
    VectorCopy(corners[0], verts[1].xyz);
    VectorCopy(corners[4], verts[2].xyz);
    VectorCopy(corners[5], verts[3].xyz);
    g_syscall(CG_R_ADDPOLYTOSCENE, bboxShader_nocull, 4, verts, 1);
}

static void init_trigger_cvars(void) {
    for(int i = 0; i < ARRAY_LEN(trigger_cvars); i++)
        g_syscall(CG_CVAR_REGISTER, trigger_cvars[i].vmCvar, trigger_cvars[i].cvarName,
            trigger_cvars[i].defaultString, trigger_cvars[i].cvarFlags);
}

static void update_trigger_cvars(void) {
    for(int i = 0; i < ARRAY_LEN(trigger_cvars); i++)
        g_syscall(CG_CVAR_UPDATE, trigger_cvars[i].vmCvar);
}