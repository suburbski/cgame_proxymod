#include <stdio.h>
#include "cg_local.h"
#include "cg_utils.h"
#include "cg_cvar.h"
#include "q_math.h"
#include "nade_tracking.h"

#define SnapVector(v) {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}
#define MASK_SHOT 0x6000001

void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
    float       deltaTime;
    deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
    VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
    result[2] -= 0.5 * 800 * deltaTime * deltaTime;
}

void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
    float   deltaTime;
    deltaTime = ( atTime - tr->trTime ) * 0.001;    // milliseconds to seconds
    VectorCopy( tr->trDelta, result );
    result[2] -= 800 * deltaTime;
}

qhandle_t beam_shader;

static vmCvar_t gl_path_draw;
static vmCvar_t gl_path_rgba;
static vmCvar_t gl_path_preview_draw;
static vmCvar_t gl_path_preview_rgba;

static cvarTable_t gl_trace_cvars[] = {
    {&gl_path_draw, "mdd_gl_path_draw", "0", CVAR_ARCHIVE},
    {&gl_path_rgba, "mdd_gl_path_rgba", "0.25 0.25 0.25 1", CVAR_ARCHIVE},
    {&gl_path_preview_draw, "mdd_gl_path_preview_draw", "0", CVAR_ARCHIVE},
    {&gl_path_preview_rgba, "mdd_gl_path_preview_rgba", "0 0.5 0 1", CVAR_ARCHIVE},
};

void draw_nade_path(trajectory_t *pos, int end_time, const unsigned char *color) {
    refEntity_t beam;
    trace_t trace;
    int sample_timer = 0;
    vec3_t currentOrigin, origin;

    memset(&beam, 0, sizeof(beam));

    beam.reType = RT_RAIL_CORE;
    beam.customShader = beam_shader;

    AxisClear(beam.axis);
    memcpy(beam.shaderRGBA, color, sizeof(beam.shaderRGBA));

    VectorCopy(pos->trBase, currentOrigin);
    if(cgs.time > pos->trTime)
        BG_EvaluateTrajectory(pos, cgs.time, beam.oldorigin);
    else
        VectorCopy(pos->trBase, beam.oldorigin);

    for(int leveltime = pos->trTime + 8; leveltime < end_time; leveltime += 8) {
        BG_EvaluateTrajectory(pos, leveltime, origin);
        g_syscall(CG_CM_BOXTRACE, &trace, currentOrigin, origin, NULL, NULL, NULL, MASK_SHOT);
        VectorCopy(trace.endpos, currentOrigin);

        sample_timer -= 8;
        if(sample_timer <= 0) {
            sample_timer = 32;
            VectorCopy(origin, beam.origin);
            if(leveltime >= cgs.time) {
                vec3_t d, saved_origin;
                VectorCopy(beam.origin, saved_origin);
                VectorSubtract(beam.origin, beam.oldorigin, d);
                VectorMA(beam.oldorigin, 0.5, d, beam.origin);
                g_syscall(CG_R_ADDREFENTITYTOSCENE, &beam);
                VectorCopy(saved_origin, beam.oldorigin);
            }
        }

        if(trace.fraction != 1) {
            vec3_t velocity;
            float dot;
            int hitTime;

            hitTime = leveltime - 8 + 8 * trace.fraction;
            BG_EvaluateTrajectoryDelta(pos, hitTime, velocity);
            dot = DotProduct(velocity, trace.plane.normal);
            VectorMA(velocity, -2*dot, trace.plane.normal, pos->trDelta);

            VectorScale(pos->trDelta, 0.65, pos->trDelta);

            VectorAdd(currentOrigin, trace.plane.normal, currentOrigin);
            VectorCopy(currentOrigin, pos->trBase);
            pos->trTime = leveltime;
            sample_timer = 0;
            if(cgs.time > pos->trTime)
                BG_EvaluateTrajectory(pos, cgs.time, beam.oldorigin);
            else
                VectorCopy(pos->trBase, beam.oldorigin);
        }
    }
}


void init_gl_trace_cvars(void) {
    for(int i = 0; i < ARRAY_LEN(gl_trace_cvars); i++) {
        g_syscall(CG_CVAR_REGISTER, gl_trace_cvars[i].vmCvar, gl_trace_cvars[i].cvarName,
                gl_trace_cvars[i].defaultString, gl_trace_cvars[i].cvarFlags);
    }
}

void update_gl_trace_cvars(void) {
    for(int i = 0; i < ARRAY_LEN(gl_trace_cvars); i++)
        g_syscall(CG_CVAR_UPDATE, gl_trace_cvars[i].vmCvar);
}

void gl_trace_init(void) {
    init_gl_trace_cvars();
    beam_shader = g_syscall(CG_R_REGISTERSHADER, "railCore");
}

void gl_trace_render(void) {
    vec3_t forward, muzzle;
    entityState_t entity;
    unsigned char path_color[4];
    unsigned char preview_color[4];
    vec4_t color;

    update_gl_trace_cvars();

    sscanf(gl_path_rgba.string, "%f %f %f %f", &color[0], &color[1], &color[2], &color[3]);
    for(int i = 0; i < 4; i++)
        path_color[i] = color[i] * 255;

    sscanf(gl_path_preview_rgba.string, "%f %f %f %f", &color[0], &color[1], &color[2], &color[3]);
    for(int i = 0; i < 4; i++)
        preview_color[i] = color[i] * 255;

    playerState_t *ps = getPs();
    snapshot_t *snap = getSnap();

    if(ps->weapon == WP_GRENADE_LAUNCHER && gl_path_preview_draw.integer) {
        AngleVectors(ps->viewangles, forward, NULL, NULL);
        VectorCopy(ps->origin, muzzle);
        muzzle[2] += ps->viewheight;
        VectorMA( muzzle, 14, forward, muzzle );
        SnapVector(muzzle);

        forward[2] += 0.2;
        VectorNormalize(forward);

        entity.pos.trType = TR_GRAVITY;
        entity.pos.trTime = snap->serverTime - 50;
        VectorCopy(muzzle, entity.pos.trBase);
        VectorScale(forward, 700, entity.pos.trDelta);
        SnapVector(entity.pos.trDelta);

        draw_nade_path(&entity.pos, cgs.time + 2500, preview_color);
    }

    if(!gl_path_draw.integer)
        return;

    for(int i = 0; i < MAX_NADES; i++) {
        if(nades[i].id >= 0 && nades[i].seen) {
            for(int j = 0; j < snap->numEntities; j++) {
                entityState_t *entity = &snap->entities[j];
                if(entity->number != nades[i].id)
                    continue;
                draw_nade_path(&entity->pos, nades[i].explode_time, path_color);
            }
        }
    }
}
