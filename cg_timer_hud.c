#include <stdio.h>
#include "cg_local.h"
#include "cg_draw.h"
#include "cg_cvar.h"
#include "cg_utils.h"
#include "q_math.h"
#include "surfaceflags.h"

#define	PMF_TIME_KNOCKBACK	64
#define	PMF_RESPAWNED		512

#define ET_MISSILE 3
#define MAX_GB_TIME 250
#define MAX_RL_TIME 15000

#define MAX_NADES 10
#define NADE_EXPLODE_TIME 2500

typedef struct {
	/** the entity number of the nade being tracked, -1 if not tracking */
	int id;
	/** predicted time of when the nade will explode */
	int explode_time;
	/** flag for whether this nade was seen in current snapshot */
	int seen;
} nade_info_t;

static nade_info_t nades[MAX_NADES];

static vmCvar_t timer_draw;
static vmCvar_t timer_x;
static vmCvar_t timer_y;
static vmCvar_t timer_w;
static vmCvar_t timer_h;
static vmCvar_t timer_item_w;
static vmCvar_t timer_item_rgba;
static vmCvar_t timer_gb_rgba;
static vmCvar_t timer_outline_rgba;

static cvarTable_t timer_cvars[] = {
	{&timer_draw, "mdd_hud_timer_draw", "0", CVAR_ARCHIVE},
	{&timer_x, "mdd_hud_timer_x", "275", CVAR_ARCHIVE},
	{&timer_y, "mdd_hud_timer_y", "275", CVAR_ARCHIVE},
	{&timer_w, "mdd_hud_timer_w", "100", CVAR_ARCHIVE},
	{&timer_h, "mdd_hud_timer_h", "16", CVAR_ARCHIVE},
	{&timer_item_w, "mdd_hud_timer_item_w", "5", CVAR_ARCHIVE},
	{&timer_item_rgba, "mdd_hud_timer_item_rgba", "1 1 0 1", CVAR_ARCHIVE},
	{&timer_gb_rgba, "mdd_hud_timer_gb_rgba", "1 0 0 1", CVAR_ARCHIVE},
	{&timer_outline_rgba, "mdd_hud_timer_outline_rgba", "1 1 1 1", CVAR_ARCHIVE}
};

// forward declarations for helpers
static int find_nade(int nade_id);
static int track_nade(int nade_id, int time);
static void draw_outline(vec4_t color);
static void draw_item(float progress, vec4_t color);
static void init_timer_cvars(void);
static void update_timer_cvars(void);

void timer_hud_init(void) {
	init_timer_cvars();

	for (int i = 0; i < MAX_NADES; i++)
		nades[i].id = -1;
}

void timer_hud_draw(void) {
	vec4_t outline_color;
	vec4_t item_color;
	vec4_t gb_color;
	snapshot_t *snap;
	playerState_t *ps;

	update_timer_cvars();

	if (!timer_draw.integer)
		return;

	sscanf(timer_outline_rgba.string, "%f %f %f %f",
			&outline_color[0], &outline_color[1],
			&outline_color[2], &outline_color[3]);

	sscanf(timer_item_rgba.string, "%f %f %f %f",
			&item_color[0], &item_color[1],
			&item_color[2], &item_color[3]);

	sscanf(timer_gb_rgba.string, "%f %f %f %f",
			&gb_color[0], &gb_color[1],
			&gb_color[2], &gb_color[3]);

	// draw the outline
	draw_outline(outline_color);

	snap = getSnap();
	ps = getPs();

	// gb stuff
	// todo: make gb timer off-able and use pps if available and cvar
	if (ps->pm_flags & PMF_TIME_KNOCKBACK
		&& ps->groundEntityNum != ENTITYNUM_NONE
		&& !(ps->pm_flags & PMF_RESPAWNED))
	{
		float gb_progress = 1.0 - (float)ps->pm_time / MAX_GB_TIME;
		draw_item(gb_progress, gb_color);
	}

	// cull exploded nades to make space
	// and set valid nades to not seen in snapshot yet
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == -1)
			continue;

		if (nades[i].explode_time - snap->serverTime <= 0)
			nades[i].id = -1;
		else
			nades[i].seen = 0;
	}

	// traverse ent list to update nade infos
	for (int i = 0; i < snap->numEntities; i++) {
		entityState_t entity = snap->entities[i];
		if (entity.eType == ET_MISSILE
			&& entity.weapon == WP_GRENADE_LAUNCHER
			&& entity.clientNum == ps->clientNum)
		{
			int nade_index = find_nade(entity.number);
			if (nade_index == -1) // new nade
				track_nade(entity.number, snap->serverTime);
			else
				nades[nade_index].seen = 1;
		}
		else if (entity.eType == ET_MISSILE
					&& entity.weapon == WP_ROCKET_LAUNCHER
					&& entity.clientNum == ps->clientNum)
		{
			trace_t t;
			vec3_t origin;
			vec3_t dest;
			float elapsed_time = (cgs.time - entity.pos.trTime)*0.001;

			VectorMA(entity.pos.trBase, elapsed_time,
				entity.pos.trDelta, origin);
			VectorMA(entity.pos.trBase, MAX_RL_TIME*0.001, entity.pos.trDelta,
				dest);

			// a rocket dest should never change (ignoring movers)
			// trace doesn't need to be recomputed each time
			g_syscall(CG_CM_BOXTRACE, &t, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
			float total_time = Distance(entity.pos.trBase, t.endpos) / VectorLength(entity.pos.trDelta);
			draw_item(elapsed_time / total_time, item_color);
		}
	}

	// cull nades not in snapshot (prolly prematurely detonated) and draw the rest
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == -1)
			continue;

		if (!nades[i].seen) {
			nades[i].id = -1;
		}
		else {
			float progress = 1.0 -
				(float)(nades[i].explode_time - snap->serverTime) / NADE_EXPLODE_TIME;
			draw_item(progress, item_color);
		}
	}
}

static int find_nade(int nade_id) {
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == nade_id)
			return i;
	}

	return -1;
}

static int track_nade(int nade_id, int time) {
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == -1) {
			nades[i].id = nade_id;
			nades[i].explode_time = time + NADE_EXPLODE_TIME;
			nades[i].seen = 1;
			return 0;
		}
	}

	// no free space to track the nade
	return -1;
}

static void draw_outline(vec4_t color) {
	int x = timer_x.integer;
	int y = timer_y.integer;
	int w = timer_w.integer;
	int h = timer_h.integer;

	g_syscall( CG_R_SETCOLOR, color );
	CG_DrawAdjPic(x, y - 1, w, 1, cgs.media.gfxWhiteShader);
	CG_DrawAdjPic(x, y + h, w, 1, cgs.media.gfxWhiteShader);
	CG_DrawAdjPic(x + w, y - 1, 1, h + 2, cgs.media.gfxWhiteShader);
	CG_DrawAdjPic(x - 1, y - 1, 1, h + 2, cgs.media.gfxWhiteShader);
}

static void draw_item(float progress, vec4_t color) {
	int x = timer_x.integer;
	int y = timer_y.integer;
	int w = timer_w.integer;
	int h = timer_h.integer;
	int i_w = timer_item_w.integer;

	g_syscall( CG_R_SETCOLOR, color );
	CG_DrawAdjPic(x + (w - i_w) * progress, y, i_w, h, cgs.media.gfxWhiteShader);
}

static void init_timer_cvars(void) {
	for(int i = 0; i < ARRAY_LEN(timer_cvars); i++)
		g_syscall(CG_CVAR_REGISTER, timer_cvars[i].vmCvar, timer_cvars[i].cvarName,
			timer_cvars[i].defaultString, timer_cvars[i].cvarFlags);
}

static void update_timer_cvars(void) {
	for(int i = 0; i < ARRAY_LEN(timer_cvars); i++)
		g_syscall(CG_CVAR_UPDATE, timer_cvars[i].vmCvar);
}