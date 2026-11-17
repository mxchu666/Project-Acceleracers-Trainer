#pragma once

#include "imgui.h"

namespace remake
{
    enum class Tab
    {
        Legit,
        Rage,
        Players,
        Visuals,
        Misc,
        Config
    };

    struct UiState
    {
        ImFont* brand_font = nullptr;
        ImFont* esp_font = nullptr;
        ImFont* weapon_icon_font = nullptr;
        Tab tab = Tab::Legit;
        bool menu_open = true;
        bool rainbow = false;
        bool restrictions = true;
        bool legit_enable = false;
        bool legit_aimbot = false;
        bool legit_rcs = false;
        bool legit_backtrack = false;
        bool legit_trigger = false;
        bool rage_enable = false;
        bool player_enemy_box = false;
        float player_enemy_box_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        bool player_friendly_box = false;
        bool player_distance = false;
        float player_distance_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        bool player_ammo = false;
        bool player_skeleton = false;
        bool player_dormant = false;
        bool player_flags = false;
        unsigned int player_flags_mask = 1u;
        int player_box_type = 0;
        bool player_reveal_radar = false;
        bool player_enemy_chams = false;
        bool player_enemy_xqz = false;
        bool player_enemy_glow = false;
        bool player_friendly_chams = false;
        bool player_friendly_xqz = false;
        bool player_friendly_glow = false;
        int glow_reflectivity = 0;
        int glow_translucency = 0;
        int glow_shine = 0;
        bool visuals_enable = false;
        bool auto_bhop = false;
        bool auto_strafe = false;
        bool aim_key_capture = false;
        bool trigger_key_capture = false;
        int fade_speed = 5;
        int selected_restriction = 1;
        int aim_key = 0;
        int trigger_key = 0;
        unsigned int resolver_mask = 1u;
        int fov = 1;
        int threshold = 0;
        int retarget = 0;
        int speed = 1;
        int backtrack_range = 1;
        int trigger_delay = 0;
        int minimum_damage = 1;
        int max_misses = 0;
        int hitchance = 0;
        int wall_alpha = 0;
        int view_fov = 0;
        int keep_killfeed = 0;
        int weapon_volume = 100;
        int footstep_volume = 100;
        int other_volume = 100;
        float accent[3] = { 181.0f / 255.0f, 0.0f, 41.0f / 255.0f };
        ImVec2 window_pos = ImVec2(100.0f, 100.0f);
        float content_scroll = 0.0f;
        ImVec2 preview_pos = ImVec2(650.0f, 100.0f);
        ImVec2 preview_drag_offset = ImVec2(0.0f, 0.0f);
        ImVec2 drag_offset = ImVec2(0.0f, 0.0f);
        bool dragging = false;
        bool preview_dragging = false;
        bool preview_rotating = false;
        bool preview_rotation_paused = false;
        int preview_anchor_drag = -1;
        ImVec2 preview_anchor_drag_start = ImVec2(0.0f, 0.0f);
        ImVec2 preview_anchor_offset_start = ImVec2(0.0f, 0.0f);
        ImVec2 preview_name_offset = ImVec2(0.0f, 0.0f);
        ImVec2 preview_health_offset = ImVec2(0.0f, 0.0f);
        ImVec2 preview_flags_offset = ImVec2(0.0f, 0.0f);
        ImVec2 preview_weapon_offset = ImVec2(0.0f, 0.0f);
        ImVec2 preview_distance_offset = ImVec2(0.0f, 0.0f);
        int preview_name_slot = 1;
        int preview_health_slot = 9;
        int preview_flags_slot = 3;
        int preview_weapon_slot = 7;
        int preview_distance_slot = 6;
        float preview_yaw = 0.0f;
        float preview_pitch = 0.0f;
        float preview_zoom = 1.0f;
        ImVec2 preview_rotate_start = ImVec2(0.0f, 0.0f);
        float preview_yaw_start = 0.0f;
        float preview_pitch_start = 0.0f;
        double preview_last_rotate_time = 0.0;
        bool hitboxes[5] = {};
        bool legit_friendlies = false;
        bool auto_fire = false;
        bool trigger_friendlies = false;
        bool trigger_scope = false;
        bool trigger_smoke = false;
        bool small_visualize = false;
        bool no_recoil = false;
        bool rage_silent = false;
        bool rage_auto_fire = false;
        bool rage_baim_fakewalk = false;
        bool visuals_dropped_weapons = false;
        bool visuals_projectiles = false;
        bool visuals_bomb = false;
        bool visuals_other_entities = false;
        bool visuals_no_smoke = false;
        bool visuals_no_flash = false;
        bool visuals_no_fog = false;
        bool visuals_no_scope = false;
        bool visuals_no_post_processing = false;
        bool visuals_custom_world_color = false;
        bool autowall = false;
        bool save_fps = false;
        bool hitchance_enabled = false;
        bool fake_lag = false;
        bool anti_aim = false;
        bool player_name = false;
        float player_name_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        bool player_health = false;
        bool player_weapon = false;
        unsigned int player_weapon_display_mask = 3u;
        bool spectator_list = false;
        bool grenade_prediction = false;
        bool recoil_crosshair = false;
        bool shot_tracers = false;
        bool auto_accept = false;
        bool rank_reveal = false;
        bool unlock_inventory = false;
        bool hitmarker = false;
        bool auto_defuse = false;
        bool save_clicked = false;
        bool load_clicked = false;
        bool unload_clicked = false;
    };
}
