#include "core/core.h"
#include "mod_div/attributes.h"
#include <math.h>

// Max number of teleport positions.
#define MAX_TELEPORTS 3

// Maximum number of entities to consider.
#define MAX_ENTITIES 1024

// Notes:
// Entity types 1 and 7 are players and enemies
// Player x-position in shooting range is -1038 at bottom of stairs.
// Player x-position in shooting range is -1025 at top of stairs.
// Player z-position in shooting range is 4.99 at top of stairs.
// Player z-position in shooting range is 0.68 at step.
// Player z-position in shooting range is -0.14 at bottom.

// Core offsets.
#define OFF_INDEXMODULE 0x3DB6420
#define OFF_RLCLIENT 0x3E3D810
#define OFF_VIEWMATRIX 0x3E3DD00
#define OFF_CONTROLLER 0x3E42DA8

// Camera offsets.
#define OFF_CAMERA_POS 0x40
#define OFF_CAMERA_YAW 0xc0
#define OFF_CAMERA_PITCH 0xc4

// Crosshair offsets.
#define OFF_CROSSHAIR_POS 0x2F5C
#define OFF_CROSSHAIR_YAW 0x30D4
#define OFF_CROSSHAIR_PITCH 0x30B0

// Entity offsets.
#define OFF_ENTITY_CROSSHAIR 0x5C8
#define OFF_ENTITY_POS 0x80
#define OFF_ENTITY_ROT_X 0x48
#define OFF_ENTITY_ROT_Z 0x40

// Warp offset.
#define OFF_WARP_POS 0x70

// Cheat entity.
typedef struct {
	u64 ptr;
	b32 is_gadget;
	b32 is_player;
	u32 level;
	u32 type;
	i32 hp_cur;
	i32 hp_max;
	f32 yaw;
	f32 box_width;
	f32 dist_dot;
	vec3 pos;
	vec3 torso;
	vec3 head;
	vec3 delta;
	i8 name[24];
} Entity;

// Keybindings.
static u32 g_bind_quit;
static u32 g_bind_aim;
static u32 g_bind_clip;
static u32 g_bind_craft_display;
static u32 g_bind_craft_teleport;
static u32 g_bind_teleport;
static u32 g_bind_player_teleport;
static u32 g_bind_save_pos[MAX_TELEPORTS];
static u32 g_bind_load_pos[MAX_TELEPORTS];
static u32 g_bind_speed;
static u32 g_bind_ammo;
static u32 g_bind_rpm;
static u32 g_bind_radar;

// Offsets.
static u64 g_base_ptr;

// Indices.
static u32 g_indices[AtributeIndexMax];

// Cheat state.
static vec3 g_craft_pos;
static vec3 g_player_pos;
static b32 g_is_init;

// Reads a bone position from the game.
static vec3 get_bone_pos(u64 bone_ptr, u32 index) {
	return process_read_vec3(bone_ptr + index * 0x40 + 0x30);
}

// Reads a value index from the game.
static u32 get_index(u64 ii_arr, u32 val) {
	u64 base = process_read_u64(ii_arr + val * 8);
	return process_read_u32(base + 0x50);
}

// Reads an index value.
static f32 get_index_val(u64 ii_arr, u32 index) {
	return process_read_f32(ii_arr + g_indices[index] * 64 + 0x4);
}

// Sets an index value.
static void set_index_val(u64 ii_arr, u32 index, f32 val) {
	process_write_f32(ii_arr + g_indices[index] * 64 + 0x4, val);
}

// Transforms a 3D coordinate into the 2D screen space.
vec2 w2s(vec3 pos, mat4 world, f32 hx, f32 hy) {
	vec3 tv = vec3_set(world._41, world._42, world._43);
	vec3 up = vec3_set(world._21, world._22, world._23);
	vec3 ri = vec3_set(world._11, world._12, world._13);

	f32 w = vec3_dot(tv, pos) + world._44;

	if (w < 0.098f) {
		return vec2_zero();
	}

	f32 x = vec3_dot(ri, pos) + world._14;
	f32 y = vec3_dot(up, pos) + world._24;

	return vec2_set(hx * (1.0f + x / w), hy * (1.0f - y / w));
}

// Aimbot.
static void module_aimbot(u64 obj_ptr, Entity* player, Entity* entities, u32 num_entities, mat4 world, f32 hx, f32 hy) {
	static u64 last_target_ptr;

	if (bind_is_down(g_bind_aim) == false || num_entities < 1) {
		last_target_ptr = 0;
		return;
	}

	u64 ptr0 = process_read_u64(obj_ptr + 0x298);
	u64 ptr1 = process_read_u64(ptr0 + 0x18);
	u64 ptr2 = process_read_u64(ptr1 + 0xB0);
	u64 cam_ptr = process_read_u64(ptr2 + 0x80);

	if (cam_ptr == 0) {
		return;
	}

	// Sort players by distance into a new list.
	static Entity* sorted[MAX_ENTITIES];
	u32 sorted_num = 0;

	for (u32 i = 0; i < num_entities; ++i) {
		Entity* e = entities + (i + 1);
		if (e == player || e->hp_cur == 0 || e->is_gadget) {
			continue;
		}

		vec2 dot = w2s(e->head, world, hx, hy);
		if (dot.x == 0.0f && dot.y == 0.0f) {
			continue;
		}

		e->dist_dot = fabsf(dot.x - hx) + fabsf(dot.y - hy);
		sorted[sorted_num++] = e;
	}

	if (sorted_num < 1) {
		return;
	}

	for (u32 i = 1; i < sorted_num; ++i) {
		Entity *x = sorted[i];
		u32 j = i;

		while (j > 0 && sorted[j - 1]->dist_dot > x->dist_dot) {
			sorted[j] = sorted[j - 1];
			j = j - 1;
		}

		sorted[j] = x;
	}

	// Look for the closest entity to aimbot to and set the crosshair pos to that position.
	u32 target = sorted_num;

	for (u32 i = 0; i < sorted_num; ++i) {
		// Draw the potential head target.
		vec2 screen_head = w2s(sorted[i]->head, world, hx, hy);

		// Check if the target is close enough.
		u32 color;
		if (target == sorted_num && sorted[i]->dist_dot < 500.0 && (last_target_ptr == 0 || sorted[i]->ptr == last_target_ptr)) {
			target = i;
			color = COLOR_CYAN;
		} else {
			color = COLOR_RED;
		}

		renderer_draw_text(screen_head.x - 3, screen_head.y - 4, color, "o");
	}

	// If no valid target found, quit early.
	if (target == sorted_num) {
		return;
	}

	// Store the last player pos.
	g_player_pos = sorted[target]->pos;

	last_target_ptr = sorted[target]->ptr;

	vec3 aim_pos = vec3_set(sorted[target]->head.x, sorted[target]->head.y + 0.095f, sorted[target]->head.z);
	// vec3 aim_pos = sorted[target]->head;
	// vec2 aim_dot = w2s(aim_pos, world, hx, hy);

	vec3 cam_pos = process_read_vec3(cam_ptr + OFF_CAMERA_POS);

	vec3 aim_delta = vec3_sub(aim_pos, cam_pos);

	f32 len = vec3_len(aim_delta);
	if (len == 0.0f) {
		return;
	}

	f32 new_pitch = asinf(aim_delta.y / len);
	f32 cos_pitch = cosf(new_pitch);
	if (cos_pitch == 0.0f) {
		return;
	}

	f32 new_yaw = asin(aim_delta.x / (cos_pitch * len));

	f32 yaw = process_read_f32(cam_ptr + OFF_CAMERA_YAW);
	if (yaw > M_3_PI_2) {
		new_yaw = M_2_PI + new_yaw;
	} else if (yaw > M_PI_2) {
		new_yaw = M_PI - new_yaw;
	}

	////if (new_yaw < 0.0f)
	////{
	////	new_yaw = (2.0 * M_PI) + new_yaw;
	////}
	//	
	////renderer_printf(5, 5, COLOR_WHITE, "%.2f %.2f %.2f", crosshair.x, crosshair.y, crosshair.z);
	//renderer_printf(5, 5, COLOR_PINK, "AIM: %.2f %.2f", new_yaw, new_pitch);
	//// process_write_vec3(cptr + 0x2E5C, aim_pos);

	renderer_printf(5, 15, COLOR_PINK, "%.2f %.2f", new_yaw, new_pitch);

	if (new_yaw > 0.0 && new_pitch < M_2_PI && new_pitch > -M_PI_2 && new_pitch) {
		// Write out the view angles.
		process_write_f32(cam_ptr + OFF_CAMERA_YAW, new_yaw);
		process_write_f32(cam_ptr + OFF_CAMERA_PITCH, new_pitch);

		// Write out the view angles into the crosshair as well.
		u64 cptr = process_read_u64(player->ptr + OFF_ENTITY_CROSSHAIR);
		if (cptr) {
			process_write_f32(cptr + OFF_CROSSHAIR_YAW, new_yaw);
			process_write_f32(cptr + OFF_CROSSHAIR_PITCH, new_pitch);
		}
	}
}

// Allows for infinite ammo.
static void module_ammo(void) {
	// Get ammo pointers.
	u64 aptr0 = process_read_u64(g_base_ptr + OFF_CONTROLLER);
	u64 aptr1 = process_read_u64(aptr0 + 0x8);
	u64 aptr2 = process_read_u64(aptr1 + 0x4C8);
	u64 aptr3 = process_read_u64(aptr2 + 0x18);

	// Old ammo values.
	static i32 ammo_old[128];
	static u32 num_weapons;

	// Handle ammo.
	static b32 is_active;

	if (bind_is_press(g_bind_ammo)) {
		is_active = !is_active;

		if (is_active) {
			num_weapons = process_read_i32(aptr2 + 0x20);

			for (u32 i = 0; i < num_weapons && i < COUNT(ammo_old); ++i) {
				ammo_old[i] = process_read_i32(aptr3 + 0x10 + i * 0x18);
			}
		}
	}

	if (is_active) {
		renderer_draw_text(150, 25, COLOR_WHITE, "AMMO");

		for (u32 i = 0; i < num_weapons && i < COUNT(ammo_old); ++i) {
			process_write_i32(aptr3 + 0x10 + i * 0x18, ammo_old[i]);
		}
	}
}

// Allows for clipping and teleporting.
static void module_clip(Entity* player) {
	// Get the warp position pointer.
	u64 wptr0 = process_read_u64(g_base_ptr + OFF_CONTROLLER);
	u64 wptr1 = process_read_u64(wptr0 + 0x8);
	u64 wptr2 = process_read_u64(wptr1 + 0x1D8);
	u64 wptr3 = process_read_u64(wptr2 + 0x320);
	u64 wptr = process_read_u64(wptr3 + 0x278);

	if (wptr == 0) {
		return;
	}

	// Get the player's warp position.
	vec3 pos = process_read_vec3(wptr + OFF_WARP_POS);

	// Save the position if the keybind is down.
	static vec3 last_pos[MAX_TELEPORTS];

	for (u32 i = 0; i < MAX_TELEPORTS; ++i) {
		if (bind_is_down(g_bind_save_pos[i])) {
			last_pos[i] = pos;
		}
	}

	// Check if it's time to move.
	vec3 new_pos = pos;

	if (bind_is_press(g_bind_clip)) {
		f32 rx = process_read_f32(player->ptr + OFF_ENTITY_ROT_X);
		f32 rz = process_read_f32(player->ptr + OFF_ENTITY_ROT_Z);

		new_pos.x = pos.x + rx * -5.0f;
		new_pos.y = pos.y;
		new_pos.z = pos.z + rz * 5.0f;
	} else if (bind_is_press(g_bind_teleport)) {
		u64 cptr = process_read_u64(player->ptr + OFF_ENTITY_CROSSHAIR);
		if (cptr == 0) {
			return;
		}

		new_pos = process_read_vec3(cptr + OFF_CROSSHAIR_POS);
		new_pos.z += 0.05f;
	} else if (bind_is_press(g_bind_craft_teleport)) {
		new_pos = g_craft_pos;
	} else if (bind_is_down(g_bind_player_teleport)) {
		new_pos = g_player_pos;
	} else {
		for (u32 i = 0; i < MAX_TELEPORTS; ++i) {
			if (bind_is_down(g_bind_load_pos[i])) {
				new_pos = last_pos[i];
			}
		}
	}

	// Check that the position has changed.
	if (new_pos.x == pos.x && new_pos.y == pos.y && new_pos.z == pos.z) {
		return;
	}

	// Check that the new position is valid.
	if (new_pos.x == 0.0f && new_pos.y == 0.0f && new_pos.z == 0.0f) {
		return;
	}

	// Check the distance we're moving to make sure it's sane.
	vec3 delta = vec3_sub(new_pos, player->pos);
	if (vec3_len(delta) > 1000.0f) {
		return;
	}

	// Finish the teleport.
	process_write_vec3(wptr + OFF_WARP_POS, new_pos);
}

// Crafting display.
static void module_craft(u64 obj_ptr, Entity* player, mat4 world, f32 hx, f32 hy) {
	g_craft_pos = vec3_zero();

	static b32 is_active;
	if (bind_is_press(g_bind_craft_display)) {
		is_active = !is_active;
	}

	if (is_active == false) {
		return;
	}

	u64 mel_ptr = process_read_u64(obj_ptr + 0x140);
	u64 mel_arr = process_read_u64(mel_ptr + 0x160);
	u32 mel_num = process_read_u64(mel_ptr + 0x168);

	if (mel_arr == 0 || mel_num > 16384) {
		return;
	}

	for (u32 i = 0; i < mel_num; ++i) {
		u64 ptr = process_read_u64(mel_arr + i * 8);
		if (ptr == 0) {
			continue;
		}

		u64 sptr = process_read_u64(ptr + 0x168);
		if (sptr == 0) {
			continue;
		}

		i8 buf[5];
		process_read_str(sptr + 27, buf, sizeof(buf));

		if (string_compare(buf, "loot") != 0) {
			continue;
		}

		vec3 pos = process_read_vec3(ptr + 0x30);
		vec2 dot = w2s(pos, world, hx, hy);

		if (dot.x == 0 && dot.y == 0) {
			continue;
		}

		u32 color = RGBA(0, 255, 255, 128);
		f32 dx = fabsf(dot.x - hx);
		f32 dy = fabsf(dot.y - hy);

		if (dx < 100 && dy < 100) {
			g_craft_pos = pos;

			color = RGBA(0, 255, 255, 220);
			renderer_draw_esp(dot.x - 11, dot.y - 11, 21, 21, COLOR_WHITE);
		}

		renderer_draw_rect(dot.x - 10, dot.y - 10, 20, 20, color);
	}
}

// RPM module.
static void module_rpm(u64 ii_arr) {
	static b32 is_active;

	if (bind_is_press(g_bind_rpm)) {
		is_active = !is_active;
	}

	if (is_active) {
		renderer_draw_text(150, 15, COLOR_WHITE, "RPM");
		set_index_val(ii_arr, RPM, 40000.0f);
	} else {
		set_index_val(ii_arr, RPM, 1000.0f);
	}
}

// Speedhack module.
static void module_speed(u64 ii_arr) {
	static b32 is_speed_active;

	if (bind_is_press(g_bind_speed)) {
		is_speed_active = !is_speed_active;

		if (is_speed_active == false) {
			set_index_val(ii_arr, MovementSpeedMod, 1.0f);
		}
	}

	if (is_speed_active) {
		renderer_draw_text(150, 5, COLOR_WHITE, "EURO");
		set_index_val(ii_arr, MovementSpeedMod, 2.5f);
	}
}


// Modifies player stats and abilities.
static void module_stats(u64 ent_arr) {
	// Read agent data.
	u64 agent_ptr = process_read_u64(ent_arr);

	// Read and set index data.
	u64 ii_ptr = process_read_u64(agent_ptr + 0x4D8);
	u64 iu_ptr = process_read_u64(ii_ptr + 0x28);
	u64 ii_arr = process_read_u64(iu_ptr + 0x78);

	// Check if there's recoil being added back onto the weapon.
	if (ii_arr) {
		// renderer_printf(5, 5, COLOR_PINK, "%.2f", get_index_val(ii_arr, g_rpm_ind));
		set_index_val(ii_arr, RecoilBase, 0.0f);
		set_index_val(ii_arr, RecoilMax, 0.0f);
		set_index_val(ii_arr, RecoilMin, 0.0f);
		set_index_val(ii_arr, HorizontalRecoilLeft, 0.0f);
		set_index_val(ii_arr, HorizontalRecoilRight, 0.0f);
		set_index_val(ii_arr, SpreadMax, 0.0f);
		set_index_val(ii_arr, SpreadMin, 0.0f);
		set_index_val(ii_arr, AimSpreadMax, 0.0f);
		set_index_val(ii_arr, AimSpreadMin, 0.0f);
		set_index_val(ii_arr, CoverSpreadMax, 0.0f);
		set_index_val(ii_arr, CoverSpreadMin, 0.0f);
		set_index_val(ii_arr, CoverAimSpreadMax, 0.0f);
		set_index_val(ii_arr, CoverAimSpreadMin, 0.0f);
		set_index_val(ii_arr, AimSwayMinVertical, 0.0f);
		set_index_val(ii_arr, AimSwayMaxVertical, 0.0f);
		set_index_val(ii_arr, AimSwayMinHorizontal, 0.0f);
		set_index_val(ii_arr, AimSwayMaxHorizontal, 0.0f);
		set_index_val(ii_arr, AimChangeAccuracyRecoveryTimeMS, 0.0f);
		set_index_val(ii_arr, HorizontalRecoilRecoveryTimeMS, 0.0f);
		set_index_val(ii_arr, ReloadTimeMS, 300.0f);
		set_index_val(ii_arr, EmptyClipFirstRoundReloadTimeMS, 300.0f);

		module_rpm(ii_arr);
		module_speed(ii_arr);

		//renderer_printf(100, 15, COLOR_PINK, "%.2f", get_index_val(ii_arr, OverHeal));
	}
}

// Initializes the cheat's game module.
static b32 game_init(void) {
	// Open the process.

	// Get the base pointer for the module.
	g_base_ptr = process_get_base("TheDivision.exe");
	if (g_base_ptr == 0) {
		return false;
	}

	// Find the index module.
	// RecoilBaseFinal
	// "TheDivision.exe"+03D879B8 -> 348 -> 98 -> 2c0 -> 0
	u64 index_mod = process_read_u64(g_base_ptr + OFF_INDEXMODULE);
	u64 index_arr = process_read_u64(index_mod + 0x98);

	if (index_arr == 0) {
		return false;
	}

	// Read out the indices.
	for (u32 i = 0; i < AtributeIndexMax; ++i) {
		g_indices[i] = get_index(index_arr, i);
	}

	g_is_init = true;

	return true;
}

// Initializes the cheat module.
void cheat_init(void) {
	process_open();

	g_bind_quit = bind_register("Quit", "Ctrl+Alt+F10");

	g_bind_speed = bind_register("Speedhack", "Num7");
	g_bind_rpm = bind_register("RPM", "Num8");
	g_bind_ammo = bind_register("Ammo", "Num9");

	g_bind_clip = bind_register("Clip", "Add");
	g_bind_player_teleport = bind_register("PlayerTeleport", "XBUTTON1");
	g_bind_teleport = bind_register("Teleport", "Num0");

	g_bind_craft_display = bind_register("CraftDisplay", "Divide");
	g_bind_craft_teleport = bind_register("CraftTeleport", "Subtract");

	g_bind_load_pos[0] = bind_register("LoadPos1", "Num1");
	g_bind_load_pos[1] = bind_register("LoadPos2", "Num2");
	g_bind_load_pos[2] = bind_register("LoadPos3", "Num3");

	g_bind_save_pos[0] = bind_register("SavePos1", "Num4");
	g_bind_save_pos[1] = bind_register("SavePos2", "Num5");
	g_bind_save_pos[2] = bind_register("SavePos3", "Num6");

	g_bind_aim = bind_register("Aim", "Capslock");
}

// Updates the cheat.
void cheat_tick(u32 res_x, u32 res_y) {
	// Update keybinds.
	bind_update();

	// Check if we need to initialize the cheat.
	if (g_is_init == false && game_init() == false) {
		renderer_draw_text(5, 5, COLOR_PINK, "Invalid offsets.");
		return;
	}

	// Get the world and camera info.
	mat4 world = mat4_transpose(process_read_mat4(g_base_ptr + OFF_VIEWMATRIX));
	f32 hx = res_x / 2.0f;
	f32 hy = res_y / 2.0f;

	// Get the RLClient data.
	u64 rlc_ptr = process_read_u64(g_base_ptr + OFF_RLCLIENT);
	u64 clt_ptr = process_read_u64(rlc_ptr + 0x120);
	u64 obj_ptr = process_read_u64(clt_ptr + 0x28);
	u64 ent_arr = process_read_u64(obj_ptr + 0x370);
	u32 ent_cur = process_read_i32(obj_ptr + 0x378);

	// Read the list of entities.
	static u64 ent_ptr[4096];
	u32 ent_ptr_num = ent_cur < COUNT(ent_ptr) ? ent_cur : COUNT(ent_ptr);
	process_read(ent_arr, ent_ptr, 8 * ent_ptr_num);

	// Get entity data.
	static Entity entities[MAX_ENTITIES];
	u32 ent_num = 0;
	u32 num_players = 0;

	for (u32 i = 0; i < ent_ptr_num; ++i) {
		u64 ptr = ent_ptr[i];
		if (ptr == 0) {
			continue;
		}

		Entity* e = entities + ent_num;
		e->ptr = ptr;

		// Verify that the entity is valid.
		e->level = process_read_i32(ptr + 0x3c4);
		e->type = process_read_u8(ptr + 0x3ac);
		e->is_player = !process_read_u8(ptr + 0x620);
		e->is_gadget = e->is_player && e->type == 2;


		if (e->level < 1 || e->is_gadget == true) {
			continue;
		}

		// Get entity name.
		//process_read_str(ptr + 0x360, e->name, sizeof(e->name));
		process_read_str(ptr + 0x360, e->name, sizeof(e->name));

		// Verify that the entity isn't a civilian.
		if (string_compare(e->name, "Civilian") == 0) {
			continue;
		}


		// Draw player count.
		num_players += e->is_player;

		// Get entity position.
		e->pos = process_read_vec3(ptr + 0x70);

		// Get the bone matrix.
		u64 bone0 = process_read_u64(ptr + 0x1d8);
		u64 bptr = process_read_u64(bone0 + 0x16D0);

		// Get bone positions.
		e->torso = get_bone_pos(bptr, 0);
		e->head = get_bone_pos(bptr, 5);
		//e->torso = get_bone_pos(bptr, 15);

		if (e->head.x == 0 && e->head.y == 0 && e->head.z == 0) {
			e->head = e->pos;
		}

		if (string_starts_with(e->name, "Drone")) {
			e->head = e->torso;

			//for (u32 k = 0; k < 32; ++k)
			//{
			//	vec2 bp = w2s(get_bone_pos(bptr, k), world, hx, hy);
			//	renderer_printf(bp.x, bp.y, COLOR_CYAN, "%d", i);
			//}
		}


		// Get entity health.
		u64 hptr = process_read_u64(ptr + 0x570);
		e->hp_cur = process_read_i32(hptr + 0xa0);
		e->hp_max = process_read_i32(hptr + 0xa4);

		// Check if the entity is dead.
		if (e->hp_cur == 0) {
			continue;
		}

		// Add the entity.
		++ent_num;

		// Don't draw the player.
		if (i == 0) {
			continue;
		}

		// Draw the bounding box.
		vec2 screen_top = w2s(vec3_set(e->torso.x, e->torso.y + 1.0f, e->torso.z), world, hx, hy);
		vec2 screen_bot = w2s(vec3_set(e->torso.x, e->torso.y - 1.0f, e->torso.z), world, hx, hy);

		f32 h = screen_bot.y - screen_top.y;
		f32 w = h * 0.5f;
		f32 x = screen_top.x - w * 0.5f;

		if (h < 1.0 || h > 600.0f) {
			continue;
		}

		u32 color = e->is_player ? COLOR_CYAN : COLOR_RED;
		renderer_draw_esp(x, screen_top.y, w, h, color);

		// Draw health.
		if (e->hp_max && e->hp_cur) {
			renderer_draw_hp(x, screen_top.y - 7, w, h, color, e->hp_cur / (f32)e->hp_max);
			// renderer_printf(x, screen_top.y - 14, COLOR_RED, "%d", e->hp_cur);
		}

		// Draw name.
		if (w > 7.0f) {
			i8 name[24] = { 0 };
			u32 len = (u32)((w + 14) / 7);
			len = len < sizeof(name) ? len : sizeof(name);

			string_copy(name, len, e->name);

			renderer_draw_text(x, screen_bot.y + 1, color, name);
		}
	}

	// Draw player count.
	if (num_players > 1) {
		renderer_printf(5, 5, COLOR_CYAN, "PLAYERS: %u", num_players - 1);

		u32 n = 0;
		for (u32 i = 1; i < ent_num; ++i) {
			Entity* e = entities + i;
			if (e->is_player) {
				renderer_draw_text(5, 15 + n++ * 10, COLOR_CYAN, e->name);
			}
		}
	}

	// Get the player.
	Entity* player = entities;

	// Reset the last player pos.
	g_player_pos = vec3_zero();

	// Update cheat modules.
	module_stats(ent_arr);
	module_aimbot(obj_ptr, player, entities, ent_num, world, hx, hy);
	module_craft(obj_ptr, player, world, hx, hy);
	module_ammo();
	module_clip(player);
}

void mod_create(void* device, void* context) {
	renderer_create(device, context);
	cheat_init();
}

void mod_destroy(void) {
	renderer_destroy();
}

b32 mod_update(void* rtv, u32 res_x, u32 res_y) {
	renderer_frame_init(rtv, res_x, res_y);
	cheat_tick(res_x, res_y);
	renderer_frame_post();

	return false;
}
