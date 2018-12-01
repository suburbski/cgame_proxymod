#include <stdio.h>
#include "cg_local.h"
#include "cg_draw.h"
#include "cg_cvar.h"
#include "cg_utils.h"
#include "surfaceflags.h"
#include "q_math.h"

#define ET_MISSILE 3
#define MAX_RL_TIME 15000

#define	MAX_MARK_FRAGMENTS	128
#define	MAX_MARK_POINTS		384
#define	MAX_VERTS_ON_POLY	10

static qhandle_t line_shader;

static vmCvar_t mark_draw;
static vmCvar_t mark_shader;
static vmCvar_t mark_size;
static vmCvar_t line_draw;
static vmCvar_t line_rgba;

static cvarTable_t rl_trace_cvars[] = {
	{&mark_draw, "mdd_rl_trace_mark_draw", "0", CVAR_ARCHIVE},
	{&mark_shader, "mdd_rl_trace_mark_shader", "rlTraceMark", CVAR_ARCHIVE},
	{&mark_size, "mdd_rl_trace_mark_size", "24", CVAR_ARCHIVE},
	{&line_draw, "mdd_rl_trace_line_draw", "0", CVAR_ARCHIVE},
	{&line_rgba, "mdd_rl_trace_line_rgba", "1 0 0 0", CVAR_ARCHIVE}
};

void add_mark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
				   float orientation, float red, float green, float blue, float alpha,
				   qboolean alphaFade, float radius);
static void init_rl_trace_cvars(void);
static void update_rl_trace_cvars(void);

void rl_trace_init(void) {
	init_rl_trace_cvars();
	line_shader = g_syscall(CG_R_REGISTERSHADER, "railCore");
}

void rl_trace_render(void) {
	snapshot_t *snap;
	playerState_t *ps;
	refEntity_t beam;
	trace_t beam_trace;
	vec3_t origin;
	vec3_t dest;

	snap = getSnap();
	ps = getPs();

	update_rl_trace_cvars();

	if(!mark_draw.integer && !line_draw.integer)
		return;

	// todo: lerp trajectory stuff?
	for (int i = 0; i < snap->numEntities; i++) {
		entityState_t entity = snap->entities[i];
		if (entity.eType == ET_MISSILE
			&& entity.weapon == WP_ROCKET_LAUNCHER
			&& entity.clientNum == ps->clientNum)
		{
			VectorMA(entity.pos.trBase, (cgs.time - entity.pos.trTime)*0.001,
				entity.pos.trDelta, origin);
			VectorMA(entity.pos.trBase, MAX_RL_TIME*0.001, entity.pos.trDelta,
				dest);
			g_syscall(CG_CM_BOXTRACE, &beam_trace, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
			if(line_draw.integer) {
				vec4_t color;
				sscanf(line_rgba.string, "%f %f %f %f", &color[0],
					&color[1], &color[2], &color[3]);

				memset( &beam, 0, sizeof( beam ) );
				VectorCopy(origin, beam.oldorigin);
				VectorCopy(beam_trace.endpos, beam.origin);
				beam.reType = RT_RAIL_CORE;
				beam.customShader = line_shader;
				AxisClear( beam.axis );
				beam.shaderRGBA[0] = color[0] * 255;
				beam.shaderRGBA[1] = color[1] * 255;
				beam.shaderRGBA[2] = color[2] * 255;
				beam.shaderRGBA[3] = color[3] * 255;
				g_syscall(CG_R_ADDREFENTITYTOSCENE, &beam);
			}
			if(mark_draw.integer) {
				qhandle_t m_shader = g_syscall(CG_R_REGISTERSHADER, mark_shader.string);
				add_mark(m_shader, beam_trace.endpos, beam_trace.plane.normal, 0, 1, 1, 1, 1,
					qfalse, mark_size.integer);
			}
		}
	}
}

// ripped CG_ImpactMark
void add_mark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
				   float orientation, float red, float green, float blue, float alpha,
				   qboolean alphaFade, float radius) {
	vec3_t			axis[3];
	float			texCoordScale;
	vec3_t			originalPoints[4];
	byte			colors[4];
	int				i, j;
	int				numFragments;
	markFragment_t	markFragments[MAX_MARK_FRAGMENTS], *mf;
	vec3_t			markPoints[MAX_MARK_POINTS];
	vec3_t			projection;

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	// get the fragments
	VectorScale( dir, -20, projection );
	numFragments = g_syscall(CG_CM_MARKFRAGMENTS, 4, (void *)originalPoints,
					projection, MAX_MARK_POINTS, markPoints[0],
					MAX_MARK_FRAGMENTS, markFragments );

	colors[0] = red * 255;
	colors[1] = green * 255;
	colors[2] = blue * 255;
	colors[3] = alpha * 255;

	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
		polyVert_t	*v;
		polyVert_t	verts[MAX_VERTS_ON_POLY];

		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
			mf->numPoints = MAX_VERTS_ON_POLY;
		}
		for ( j = 0, v = verts ; j < mf->numPoints ; j++, v++ ) {
			vec3_t		delta;

			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );

			VectorSubtract( v->xyz, origin, delta );
			v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
			v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
			*(int *)v->modulate = *(int *)colors;
		}

		g_syscall(CG_R_ADDPOLYTOSCENE, markShader, mf->numPoints, verts );
	}
}

static void init_rl_trace_cvars(void) {
	for(int i = 0; i < ARRAY_LEN(rl_trace_cvars); i++)
		g_syscall(CG_CVAR_REGISTER, rl_trace_cvars[i].vmCvar, rl_trace_cvars[i].cvarName,
			rl_trace_cvars[i].defaultString, rl_trace_cvars[i].cvarFlags);
}

static void update_rl_trace_cvars(void) {
	for(int i = 0; i < ARRAY_LEN(rl_trace_cvars); i++)
		g_syscall(CG_CVAR_UPDATE, rl_trace_cvars[i].vmCvar);
}