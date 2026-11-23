#include "menu.h"
#include "graphics.h"
#include "pad.h"
#include "launcher.h"
#include <stdio.h>
#include <string.h>

#define MAX_INSTALLED_MODS 16
#define MAX_USB_MODS 8
#define MAX_SETTINGS 6

// State variables
static ModItem s_installed_mods[MAX_INSTALLED_MODS];
static int s_installed_count = 0;
static int s_selected_mod_idx = 2; // Default to FLY MOD v2.1 as in Ui.html mockup

static UsbModItem s_usb_mods[MAX_USB_MODS];
static int s_usb_count = 0;
static int s_selected_usb_idx = 0;

static SettingItem s_settings[MAX_SETTINGS];
static int s_selected_setting_idx = 0;

static SystemHudInfo s_sys_info;
static ModOptionsModal s_modal;
static ToastNotification s_toast;
static MenuTab s_current_tab = TAB_INSTALLED_MODS;

void menu_show_toast(const char *msg) {
    if (!msg) return;
    strncpy(s_toast.message, msg, sizeof(s_toast.message) - 1);
    s_toast.timer_frames = 120; // 2 seconds at 60 FPS
    s_toast.active = true;
}

void menu_trigger_xmb_reboot(void) {
    s_sys_info.xmb_rebooting = true;
    menu_show_toast("REBOOTING CONSOLE TO XMB...");
}

void menu_init(void) {
    // ----------------------------------------------------
    // 1. Initialize Installed Mods (matching Ui.html)
    // ----------------------------------------------------
    s_installed_count = 8;

    // Mod 0: Too Many Items
    strncpy(s_installed_mods[0].name, "TOO MANY ITEMS", 63);
    strncpy(s_installed_mods[0].version, "v1.0", 15);
    strncpy(s_installed_mods[0].author, "Marglyph", 31);
    strncpy(s_installed_mods[0].description, "In-game inventory editor, recipe book and item spawner.", 255);
    strncpy(s_installed_mods[0].file_size, "180 KB", 15);
    s_installed_mods[0].enabled = true;
    s_installed_mods[0].disables_trophies = true;
    s_installed_mods[0].thumb_type = THUMB_TMI;

    // Mod 1: X-Ray Vision
    strncpy(s_installed_mods[1].name, "X-RAY VISION", 63);
    strncpy(s_installed_mods[1].version, "v1.2", 15);
    strncpy(s_installed_mods[1].author, "MinecraftX", 31);
    strncpy(s_installed_mods[1].description, "Highlight diamond, iron and coal ores through stone blocks.", 255);
    strncpy(s_installed_mods[1].file_size, "95 KB", 15);
    s_installed_mods[1].enabled = true;
    s_installed_mods[1].disables_trophies = true;
    s_installed_mods[1].thumb_type = THUMB_XRAY;

    // Mod 2: Fly Mod v2.1
    strncpy(s_installed_mods[2].name, "FLY MOD v2.1", 63);
    strncpy(s_installed_mods[2].version, "v2.1", 15);
    strncpy(s_installed_mods[2].author, "PS3Modder99", 31);
    strncpy(s_installed_mods[2].description, "Toggle flight in Survival mode. Double-tap jump to fly. Adjust speeds in mod options.", 255);
    strncpy(s_installed_mods[2].file_size, "240 KB", 15);
    s_installed_mods[2].enabled = true;
    s_installed_mods[2].disables_trophies = true;
    s_installed_mods[2].thumb_type = THUMB_FLY;

    // Mod 3: Dynamic Lights
    strncpy(s_installed_mods[3].name, "DYNAMIC LIGHTS", 63);
    strncpy(s_installed_mods[3].version, "v1.4", 15);
    strncpy(s_installed_mods[3].author, "AtomicStryker", 31);
    strncpy(s_installed_mods[3].description, "Held torches and glowstone emit real-time dynamic light around player.", 255);
    strncpy(s_installed_mods[3].file_size, "310 KB", 15);
    s_installed_mods[3].enabled = true;
    s_installed_mods[3].disables_trophies = false;
    s_installed_mods[3].thumb_type = THUMB_LIGHTS;

    // Mod 4: Better FPS (PS3)
    strncpy(s_installed_mods[4].name, "BETTER FPS (PS3)", 63);
    strncpy(s_installed_mods[4].version, "v1.0", 15);
    strncpy(s_installed_mods[4].author, "Guichaguri", 31);
    strncpy(s_installed_mods[4].description, "RSX optimization patch, memory cleanup and improved chunk rendering.", 255);
    strncpy(s_installed_mods[4].file_size, "150 KB", 15);
    s_installed_mods[4].enabled = true;
    s_installed_mods[4].disables_trophies = false;
    s_installed_mods[4].thumb_type = THUMB_FPS;

    // Mod 5: Minimap Mod v3.0
    strncpy(s_installed_mods[5].name, "MINIMAP MOD v3.0", 63);
    strncpy(s_installed_mods[5].version, "v3.0", 15);
    strncpy(s_installed_mods[5].author, "ReiF", 31);
    strncpy(s_installed_mods[5].description, "Top-right radar HUD showing terrain, hostile mobs and custom waypoints.", 255);
    strncpy(s_installed_mods[5].file_size, "210 KB", 15);
    s_installed_mods[5].enabled = true;
    s_installed_mods[5].disables_trophies = false;
    s_installed_mods[5].thumb_type = THUMB_MINIMAP;

    // Mod 6: Custom Capes
    strncpy(s_installed_mods[6].name, "CUSTOM CAPES", 63);
    strncpy(s_installed_mods[6].version, "v1.0", 15);
    strncpy(s_installed_mods[6].author, "CapeDev", 31);
    strncpy(s_installed_mods[6].description, "Render custom cloak textures from USB drive /dev_usb000/capes/.", 255);
    strncpy(s_installed_mods[6].file_size, "80 KB", 15);
    s_installed_mods[6].enabled = false;
    s_installed_mods[6].disables_trophies = false;
    s_installed_mods[6].thumb_type = THUMB_CAPES;

    // Mod 7: Voxel3D First-Person
    strncpy(s_installed_mods[7].name, "VOXEL3D FIRST-PERSON", 63);
    strncpy(s_installed_mods[7].version, "v1.0", 15);
    strncpy(s_installed_mods[7].author, "Landy", 31);
    strncpy(s_installed_mods[7].description, "Custom 3D weapon and item models rendered via RSX vertex pipeline.", 255);
    strncpy(s_installed_mods[7].file_size, "450 KB", 15);
    s_installed_mods[7].enabled = true;
    s_installed_mods[7].disables_trophies = false;
    s_installed_mods[7].thumb_type = THUMB_VOXEL3D;

    // ----------------------------------------------------
    // 2. Initialize USB Mods (Tab 2)
    // ----------------------------------------------------
    s_usb_count = 3;
    strncpy(s_usb_mods[0].name, "Damage Indicators v1.7.10", 63);
    strncpy(s_usb_mods[0].version, "v1.7.10", 15);
    strncpy(s_usb_mods[0].file_size, "320 KB", 15);
    strncpy(s_usb_mods[0].usb_path, "/dev_usb000/mc_mods/damage_indicators.sprx", 127);
    s_usb_mods[0].installed = false;
    s_usb_mods[0].install_progress = 0.0f;

    strncpy(s_usb_mods[1].name, "Mo' Creatures (PS3 Edition)", 63);
    strncpy(s_usb_mods[1].version, "v6.3.1", 15);
    strncpy(s_usb_mods[1].file_size, "1.8 MB", 15);
    strncpy(s_usb_mods[1].usb_path, "/dev_usb000/mc_mods/mocreatures_ps3.sprx", 127);
    s_usb_mods[1].installed = false;
    s_usb_mods[1].install_progress = 0.0f;

    strncpy(s_usb_mods[2].name, "OptiFine HD (Cell BE Tuning)", 63);
    strncpy(s_usb_mods[2].version, "v1.4.2", 15);
    strncpy(s_usb_mods[2].file_size, "512 KB", 15);
    strncpy(s_usb_mods[2].usb_path, "/dev_usb000/mc_mods/optifine_cell.sprx", 127);
    s_usb_mods[2].installed = true;
    s_usb_mods[2].install_progress = 1.0f;

    // ----------------------------------------------------
    // 3. Initialize Settings (Tab 3)
    // ----------------------------------------------------
    strncpy(s_settings[0].label, "Autoload Mods on Boot", 63);
    strncpy(s_settings[0].description, "Automatically inject enabled mods when Minecraft boots", 127);
    s_settings[0].type = SETTING_TOGGLE;
    s_settings[0].bool_val = true;

    strncpy(s_settings[1].label, "Skip Game Intros", 63);
    strncpy(s_settings[1].description, "Bypass Mojang and 4J Studios intro logos for fast launch", 127);
    s_settings[1].type = SETTING_TOGGLE;
    s_settings[1].bool_val = true;

    strncpy(s_settings[2].label, "RSX Performance Mode", 63);
    strncpy(s_settings[2].description, "Clock speed and pipeline shader optimizations", 127);
    s_settings[2].type = SETTING_SELECT;
    s_settings[2].int_val = 1; // 0: Normal, 1: Boost, 2: Ultra
    s_settings[2].int_max = 3;
    s_settings[2].options[0] = "NORMAL";
    s_settings[2].options[1] = "BOOST";
    s_settings[2].options[2] = "ULTRA";

    strncpy(s_settings[3].label, "Show Telemetry HUD", 63);
    strncpy(s_settings[3].description, "Display CPU/RSX thermal monitors and RAM usage overlay", 127);
    s_settings[3].type = SETTING_TOGGLE;
    s_settings[3].bool_val = true;

    strncpy(s_settings[4].label, "Auto-Update Mods via LAN", 63);
    strncpy(s_settings[4].description, "Check PC build directory for updated SPRX binaries", 127);
    s_settings[4].type = SETTING_TOGGLE;
    s_settings[4].bool_val = false;

    strncpy(s_settings[5].label, "Debug NetLogger", 63);
    strncpy(s_settings[5].description, "Broadcast console debug messages to UDP port 18194", 127);
    s_settings[5].type = SETTING_TOGGLE;
    s_settings[5].bool_val = true;

    // ----------------------------------------------------
    // 4. Initialize Hardware Telemetry (Tab 4)
    // ----------------------------------------------------
    s_sys_info.cpu_temp = 56;
    s_sys_info.rsx_temp = 51;
    s_sys_info.ram_used_mb = 174;
    s_sys_info.ram_total_mb = 256;
    s_sys_info.vram_used_mb = 188;
    s_sys_info.vram_total_mb = 256;
    s_sys_info.hdd_free_gb = 78.4f;
    s_sys_info.hdd_total_gb = 120.0f;
    s_sys_info.fan_preset = 0; // SYSCON
    s_sys_info.xmb_rebooting = false;
    strncpy(s_sys_info.cfw_version, "CFW 4.84.2 D-REX (DEX)", 31);
    strncpy(s_sys_info.title_id, "BLUS31426 (US)", 15);

    // ----------------------------------------------------
    // 5. Initialize Modal State
    // ----------------------------------------------------
    s_modal.is_open = false;
    s_modal.flight_speed = 2.5f;
    s_modal.double_tap_jump = true;
    s_modal.survival_flight = true;
    s_modal.selected_field = 0;

    // ----------------------------------------------------
    // 6. Toast initial welcome
    // ----------------------------------------------------
    menu_show_toast("MINECRAFT PS3 MOD LOADER READY");
}

void menu_update(void) {
    // If rebooting to XMB, allow dismissing with Circle
    if (s_sys_info.xmb_rebooting) {
        if (pad_just_pressed(1 << 13)) { // Circle
            s_sys_info.xmb_rebooting = false;
            menu_show_toast("RETURNED TO MOD LOADER");
        }
        return;
    }

    // Update toast timer
    if (s_toast.active) {
        if (--s_toast.timer_frames <= 0) {
            s_toast.active = false;
        }
    }

    // If modal dialog is open, route inputs exclusively to modal
    if (s_modal.is_open) {
        if (pad_just_pressed(1 << 13)) { // Circle: Close/Cancel
            s_modal.is_open = false;
            menu_show_toast("MOD OPTIONS CANCELLED");
            return;
        }
        if (pad_just_pressed(1 << 14)) { // Cross: Save & Close / Toggle
            if (s_modal.selected_field == 0) {
                s_modal.flight_speed += 0.5f;
                if (s_modal.flight_speed > 5.0f) s_modal.flight_speed = 1.0f;
            } else if (s_modal.selected_field == 1) {
                s_modal.double_tap_jump = !s_modal.double_tap_jump;
            } else if (s_modal.selected_field == 2) {
                s_modal.survival_flight = !s_modal.survival_flight;
            }
            menu_show_toast("OPTIONS UPDATED");
        }
        if (pad_just_pressed(1 << 4)) { // UP
            if (s_modal.selected_field > 0) s_modal.selected_field--;
        }
        if (pad_just_pressed(1 << 6)) { // DOWN
            if (s_modal.selected_field < 2) s_modal.selected_field++;
        }
        if (pad_just_pressed(1 << 7)) { // LEFT (adjust slider)
            if (s_modal.selected_field == 0 && s_modal.flight_speed > 1.0f) {
                s_modal.flight_speed -= 0.5f;
            }
        }
        if (pad_just_pressed(1 << 5)) { // RIGHT
            if (s_modal.selected_field == 0 && s_modal.flight_speed < 5.0f) {
                s_modal.flight_speed += 0.5f;
            }
        }
        return;
    }

    // Tab Navigation: L1 (Previous) and R1 (Next)
    if (pad_just_pressed(1 << 10)) { // L1 (Bumper Left)
        if (s_current_tab > 0) s_current_tab--;
        else s_current_tab = TAB_SYSTEM;
    }
    if (pad_just_pressed(1 << 11)) { // R1 (Bumper Right)
        if (s_current_tab < TAB_COUNT - 1) s_current_tab++;
        else s_current_tab = TAB_INSTALLED_MODS;
    }

    // Tab-specific controls
    switch (s_current_tab) {
        case TAB_INSTALLED_MODS:
            if (pad_just_pressed(1 << 4)) { // UP
                if (s_selected_mod_idx > 0) s_selected_mod_idx--;
            }
            if (pad_just_pressed(1 << 6)) { // DOWN
                if (s_selected_mod_idx < s_installed_count - 1) s_selected_mod_idx++;
            }
            if (pad_just_pressed(1 << 14)) { // CROSS: Toggle enabled
                s_installed_mods[s_selected_mod_idx].enabled = !s_installed_mods[s_selected_mod_idx].enabled;
                if (s_installed_mods[s_selected_mod_idx].enabled) {
                    menu_show_toast("MOD ENABLED");
                } else {
                    menu_show_toast("MOD DISABLED");
                }
            }
            if (pad_just_pressed(1 << 15)) { // SQUARE: Open Mod Options
                s_modal.is_open = true;
            }
            if (pad_just_pressed(1 << 12)) { // TRIANGLE: Refresh mods list
                menu_show_toast("MOD LIST REFRESHED");
            }
            if (pad_just_pressed(1 << 3)) { // START: Launch Game
                menu_show_toast("LAUNCHING MINECRAFT PS3...");
                launcher_boot_minecraft();
            }
            break;

        case TAB_USB_INSTALL:
            if (pad_just_pressed(1 << 4)) { // UP
                if (s_selected_usb_idx > 0) s_selected_usb_idx--;
            }
            if (pad_just_pressed(1 << 6)) { // DOWN
                if (s_selected_usb_idx < s_usb_count - 1) s_selected_usb_idx++;
            }
            if (pad_just_pressed(1 << 14)) { // CROSS: Install selected mod
                s_usb_mods[s_selected_usb_idx].installed = true;
                s_usb_mods[s_selected_usb_idx].install_progress = 1.0f;
                menu_show_toast("MOD INSTALLED TO HDD0");
            }
            if (pad_just_pressed(1 << 12)) { // TRIANGLE: Rescan USB
                menu_show_toast("SCANNING /dev_usb000/MCPS3_MODS/...");
            }
            break;

        case TAB_SETTINGS:
            if (pad_just_pressed(1 << 4)) { // UP
                if (s_selected_setting_idx > 0) s_selected_setting_idx--;
            }
            if (pad_just_pressed(1 << 6)) { // DOWN
                if (s_selected_setting_idx < MAX_SETTINGS - 1) s_selected_setting_idx++;
            }
            if (pad_just_pressed(1 << 14) || pad_just_pressed(1 << 5)) { // CROSS or RIGHT
                SettingItem *set = &s_settings[s_selected_setting_idx];
                if (set->type == SETTING_TOGGLE) {
                    set->bool_val = !set->bool_val;
                    menu_show_toast(set->bool_val ? "SETTING ENABLED" : "SETTING DISABLED");
                } else if (set->type == SETTING_SELECT) {
                    set->int_val = (set->int_val + 1) % set->int_max;
                    menu_show_toast("PROFILE CHANGED");
                }
            }
            if (pad_just_pressed(1 << 7)) { // LEFT
                SettingItem *set = &s_settings[s_selected_setting_idx];
                if (set->type == SETTING_TOGGLE) {
                    set->bool_val = !set->bool_val;
                } else if (set->type == SETTING_SELECT) {
                    set->int_val = (set->int_val - 1 + set->int_max) % set->int_max;
                }
            }
            break;

        case TAB_SYSTEM:
            if (pad_just_pressed(1 << 7)) { // LEFT: fan preset
                if (s_sys_info.fan_preset > 0) s_sys_info.fan_preset--;
                menu_show_toast("FAN PRESET UPDATED");
            }
            if (pad_just_pressed(1 << 5)) { // RIGHT: fan preset
                if (s_sys_info.fan_preset < 3) s_sys_info.fan_preset++;
                menu_show_toast("FAN PRESET UPDATED");
            }
            if (pad_just_pressed(1 << 15)) { // SQUARE: Clear Cache
                menu_show_toast("MOD CACHE PURGED");
            }
            if (pad_just_pressed(1 << 12)) { // TRIANGLE: Reboot to XMB
                menu_trigger_xmb_reboot();
            }
            break;

        default:
            break;
    }
}

// ----------------------------------------------------
// Header Rendering (PlayStation 3 + Minecraft 3D Stone Logo + Telemetry)
// ----------------------------------------------------
static void menu_render_header(void) {
    // Top gradient overlay strip
    graphics_draw_rect_vertical_gradient(0, 0, SCREEN_W, 58, 0xAA000000, 0x11000000);

    // 1. Left: PS3 Brand Wordmark & Emblem
    graphics_draw_ps3_brand(20, 10);

    // 2. Center: Minecraft 3D Extruded Stone Logo & Banner Stack
    graphics_draw_mc_logo(300, 4, 1.0f);

    // Edition Banner Box with Minecraft bevel styling
    float ban_w = 210.0f;
    float ban_x = 300.0f + (230.0f - ban_w) * 0.5f;
    graphics_draw_bevel_panel(ban_x, 50, ban_w, 18, 0xFF3C3631, 0xFF1C1916, 0xFF6E645B, 0xFF090807);
    graphics_draw_text_shadow(ban_x + 12, 53, 0.72f, 0xFFF2F0EA, "PLAYSTATION(R)3 EDITION");

    // Yellow Mod Loader Subtitle
    graphics_draw_text_shadow(304, 72, 0.95f, COLOR_YELLOW_ACCENT, "MOD LOADER & MANAGER v1.4.2");

    // 3. Right: System Telemetry HUD
    char hud_buf[64];
    snprintf(hud_buf, sizeof(hud_buf), "CPU: %d C | RSX: %d C", s_sys_info.cpu_temp, s_sys_info.rsx_temp);
    graphics_draw_text_shadow(640, 10, 0.78f, COLOR_TEXT_MAIN, hud_buf);

    snprintf(hud_buf, sizeof(hud_buf), "RAM: %d / %d MB", s_sys_info.ram_used_mb, s_sys_info.ram_total_mb);
    graphics_draw_text_shadow(665, 24, 0.78f, COLOR_TEXT_MAIN, hud_buf);

    graphics_draw_text_shadow(692, 38, 0.78f, COLOR_TEXT_MUTED, "CFW 4.84 DEX");
}

// ----------------------------------------------------
// Tab Bar Rendering ([L1] TABS [R1])
// ----------------------------------------------------
static void menu_render_tab_bar(void) {
    float tab_y = 96.0f;

    // L1 Bumper Button
    graphics_draw_bevel_panel(40, tab_y + 2, 44, 28, 0xFF4B433B, 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(48, tab_y + 9, 0.82f, COLOR_WHITE, "[L1]");

    // Tabs Group
    const char *kTabNames[TAB_COUNT] = { "INSTALLED MODS", "USB INSTALL", "SETTINGS", "SYSTEM" };
    float tab_w = 170.0f;
    float tab_x = 90.0f;

    for (int i = 0; i < TAB_COUNT; i++) {
        bool is_active = (s_current_tab == i);
        uint32_t bg_top = is_active ? 0xFF3D3228 : 0xFF5A4F44;
        uint32_t bg_bot = is_active ? 0xFF221A14 : 0xFF362E27;
        float h = is_active ? 32.0f : 28.0f;
        float y = is_active ? tab_y : tab_y + 4.0f;

        graphics_draw_bevel_panel(tab_x, y, tab_w, h, bg_top, bg_bot, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);

        if (is_active) {
            // Active yellow underline bar
            graphics_draw_rect(tab_x + 4, y + h - 3.0f, tab_w - 8, 2.5f, COLOR_YELLOW_ACCENT);
        }

        uint32_t text_color = is_active ? COLOR_YELLOW_ACCENT : COLOR_TEXT_MUTED;
        graphics_draw_text_shadow(tab_x + 18, y + 8, 0.85f, text_color, kTabNames[i]);

        tab_x += tab_w + 6.0f;
    }

    // R1 Bumper Button
    graphics_draw_bevel_panel(tab_x, tab_y + 2, 44, 28, 0xFF4B433B, 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(tab_x + 8, tab_y + 9, 0.82f, COLOR_WHITE, "[R1]");
}

// ----------------------------------------------------
// TAB 1: INSTALLED MODS VIEW (Matching Ui.html)
// ----------------------------------------------------
void menu_render_tab_installed_mods(void) {
    float panel_x = 40.0f;
    float panel_y = 130.0f;

    // Left Mod List Box
    float list_w = 365.0f;
    float list_h = 325.0f;
    graphics_draw_bevel_panel(panel_x, panel_y, list_w, list_h, 0xEE16120E, 0xFA0F0D0A, COLOR_BORDER_MID, COLOR_BORDER_DARK);

    float item_y = panel_y + 8.0f;
    for (int i = 0; i < s_installed_count; i++) {
        bool is_sel = (i == s_selected_mod_idx);

        // Highlight box for selected item (yellow border & glow)
        if (is_sel) {
            graphics_draw_rect_vertical_gradient(panel_x + 6, item_y - 2, list_w - 32, 36, 0x66F4E436, 0x22F4E436);
            graphics_draw_rect_outline(panel_x + 6, item_y - 2, list_w - 32, 36, 2.0f, COLOR_YELLOW_ACCENT);
        } else {
            graphics_draw_rect(panel_x + 6, item_y - 2, list_w - 32, 36, 0x1AFFFFFF);
        }

        // Custom PS3 Checkbox
        float chk_x = panel_x + 14.0f;
        float chk_y = item_y + 5.0f;
        graphics_draw_panel_box(chk_x, chk_y, 20, 20, 0xFF221A14, COLOR_BORDER_DARK);

        if (s_installed_mods[i].enabled) {
            graphics_draw_rect(chk_x + 3, chk_y + 3, 14, 14, 0xFF286E24);
            graphics_draw_text_shadow(chk_x + 6, chk_y + 4, 0.75f, COLOR_GREEN_ENABLED, "V");
        } else {
            graphics_draw_rect(chk_x + 3, chk_y + 3, 14, 14, 0xFF5E1E1E);
            graphics_draw_text_shadow(chk_x + 6, chk_y + 4, 0.75f, COLOR_RED_DISABLED, "X");
        }

        // Mod Title
        uint32_t name_color = is_sel ? COLOR_YELLOW_ACCENT : COLOR_TEXT_MAIN;
        graphics_draw_text_shadow(panel_x + 44, item_y + 8, 0.88f, name_color, s_installed_mods[i].name);

        item_y += 39.0f;
    }

    // Scrollbar Track & Thumb with Up/Down Arrows
    float sb_x = panel_x + list_w - 20.0f;
    graphics_draw_rect(sb_x, panel_y + 4, 16, list_h - 8, 0xFF14100E);
    graphics_draw_rect_outline(sb_x, panel_y + 4, 16, list_h - 8, 1.0f, COLOR_BORDER_DARK);

    // Up Arrow ▲
    graphics_draw_panel_box(sb_x, panel_y + 4, 16, 16, 0xFF3D3730, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(sb_x + 4, panel_y + 6, 0.65f, COLOR_WHITE, "^");

    // Thumb
    float thumb_y = panel_y + 24.0f + (s_selected_mod_idx * 30.0f);
    graphics_draw_bevel_panel(sb_x + 2, thumb_y, 12, 38, 0xFF7D766E, 0xFF4E4740, 0xFF9E968D, 0xFF28231E);

    // Down Arrow ▼
    graphics_draw_panel_box(sb_x, panel_y + list_h - 20, 16, 16, 0xFF3D3730, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(sb_x + 4, panel_y + list_h - 18, 0.65f, COLOR_WHITE, "v");

    // Right Mod Details Box
    float card_x = 415.0f;
    float card_w = 393.0f;
    float card_h = 325.0f;
    graphics_draw_bevel_panel(card_x, panel_y, card_w, card_h, 0xEE16120E, 0xFA0F0D0A, COLOR_BORDER_MID, COLOR_BORDER_DARK);

    if (s_selected_mod_idx >= 0 && s_selected_mod_idx < s_installed_count) {
        ModItem *sel = &s_installed_mods[s_selected_mod_idx];

        // 1. High-fidelity Pixel-Art Thumbnail
        float thumb_x = card_x + 16.0f;
        float thumb_y = panel_y + 16.0f;
        graphics_draw_pixel_thumb(thumb_x, thumb_y, 74, 74, sel->thumb_type);

        // 2. Heading: Title & Author
        graphics_draw_text_shadow(card_x + 102, thumb_y + 8, 1.08f, COLOR_WHITE, sel->name);
        char author_buf[64];
        snprintf(author_buf, sizeof(author_buf), "Author: %s", sel->author);
        graphics_draw_text_shadow(card_x + 102, thumb_y + 32, 0.82f, COLOR_TEXT_MUTED, author_buf);

        // 3. Description Box
        float desc_y = panel_y + 104.0f;
        graphics_draw_text_shadow(card_x + 16, desc_y, 0.85f, COLOR_TEXT_MAIN, "Description:");
        graphics_draw_text_shadow(card_x + 16, desc_y + 20, 0.80f, COLOR_TEXT_MUTED, sel->description);

        // 4. Metadata: Status, File Size, Trophy Warning
        float meta_y = desc_y + 92.0f;
        graphics_draw_text_shadow(card_x + 16, meta_y, 0.85f, COLOR_WHITE, "Status:");
        if (sel->enabled) {
            graphics_draw_bevel_panel(card_x + 85, meta_y - 4, 82, 22, 0xFF2A6E24, 0xFF174213, 0xFF4AE640, 0xFF0B2109);
            graphics_draw_text_shadow(card_x + 95, meta_y, 0.80f, COLOR_GREEN_ENABLED, "ENABLED");
        } else {
            graphics_draw_bevel_panel(card_x + 85, meta_y - 4, 88, 22, 0xFF6E2424, 0xFF421313, 0xFFE64040, 0xFF210909);
            graphics_draw_text_shadow(card_x + 95, meta_y, 0.80f, COLOR_RED_DISABLED, "DISABLED");
        }

        char size_buf[64];
        snprintf(size_buf, sizeof(size_buf), "File Size: %s", sel->file_size);
        graphics_draw_text_shadow(card_x + 16, meta_y + 26, 0.82f, COLOR_TEXT_MAIN, size_buf);

        if (sel->disables_trophies) {
            graphics_draw_text_shadow(card_x + 16, meta_y + 50, 0.82f, COLOR_RED_DISABLED,
                                      "Warning: Disables Trophy acquisition.");
        }
    }
}

// ----------------------------------------------------
// TAB 2: USB INSTALL VIEW
// ----------------------------------------------------
void menu_render_tab_usb_install(void) {
    float panel_x = 40.0f;
    float panel_y = 130.0f;
    float panel_w = 768.0f;
    float panel_h = 325.0f;

    graphics_draw_bevel_panel(panel_x, panel_y, panel_w, panel_h, 0xEE16120E, 0xFA0F0D0A, COLOR_BORDER_MID, COLOR_BORDER_DARK);

    // USB Header Strip: Path & Action Buttons
    char count_buf[64];
    snprintf(count_buf, sizeof(count_buf), "DEVICE: /dev_usb000/MCPS3_MODS/ (%d PACKAGES FOUND)", s_usb_count);
    graphics_draw_text_shadow(panel_x + 16, panel_y + 14, 0.85f, COLOR_YELLOW_ACCENT, count_buf);

    graphics_draw_bevel_panel(panel_x + 480, panel_y + 10, 126, 26, 0xFF4B433B, 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(panel_x + 490, panel_y + 16, 0.78f, COLOR_WHITE, "RESCAN USB000");

    graphics_draw_bevel_panel(panel_x + 616, panel_y + 10, 134, 26, 0xFF2A6E24, 0xFF174213, 0xFF4AE640, 0xFF0B2109);
    graphics_draw_text_shadow(panel_x + 630, panel_y + 16, 0.78f, COLOR_GREEN_ENABLED, "INSTALL ALL");

    graphics_draw_rect(panel_x + 12, panel_y + 44, panel_w - 24, 2, 0x33FFFFFF);

    // USB Items List
    float item_y = panel_y + 54.0f;
    for (int i = 0; i < s_usb_count; i++) {
        bool is_sel = (i == s_selected_usb_idx);
        uint32_t bg = is_sel ? 0x44F4E436 : 0x1AFFFFFF;
        graphics_draw_rect(panel_x + 14, item_y, panel_w - 28, 56, bg);
        if (is_sel) {
            graphics_draw_rect_outline(panel_x + 14, item_y, panel_w - 28, 56, 2.0f, COLOR_YELLOW_ACCENT);
        }

        // Mod title and path
        graphics_draw_text_shadow(panel_x + 26, item_y + 10, 0.92f, is_sel ? COLOR_YELLOW_ACCENT : COLOR_WHITE, s_usb_mods[i].name);

        char meta_buf[128];
        snprintf(meta_buf, sizeof(meta_buf), "Size: %s | Source: %s", s_usb_mods[i].file_size, s_usb_mods[i].usb_path);
        graphics_draw_text_shadow(panel_x + 26, item_y + 32, 0.76f, COLOR_TEXT_MUTED, meta_buf);

        // Install button state
        float btn_x = panel_x + panel_w - 150.0f;
        if (s_usb_mods[i].installed) {
            graphics_draw_bevel_panel(btn_x, item_y + 12, 115, 30, 0xFF2A6E24, 0xFF174213, 0xFF4AE640, 0xFF0B2109);
            graphics_draw_text_shadow(btn_x + 16, item_y + 20, 0.82f, COLOR_GREEN_ENABLED, "INSTALLED");
        } else {
            graphics_draw_bevel_panel(btn_x, item_y + 12, 115, 30, 0xFF4B433B, 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
            graphics_draw_text_shadow(btn_x + 24, item_y + 20, 0.82f, COLOR_WHITE, "INSTALL");
        }

        item_y += 66.0f;
    }
}

// ----------------------------------------------------
// TAB 3: SETTINGS VIEW (Matching Ui.html)
// ----------------------------------------------------
void menu_render_tab_settings(void) {
    float panel_x = 40.0f;
    float panel_y = 130.0f;
    float panel_w = 768.0f;
    float panel_h = 325.0f;

    graphics_draw_bevel_panel(panel_x, panel_y, panel_w, panel_h, 0xEE16120E, 0xFA0F0D0A, COLOR_BORDER_MID, COLOR_BORDER_DARK);

    float item_y = panel_y + 14.0f;
    for (int i = 0; i < MAX_SETTINGS; i++) {
        bool is_sel = (i == s_selected_setting_idx);
        uint32_t bg = is_sel ? 0x44F4E436 : 0x1AFFFFFF;

        graphics_draw_rect(panel_x + 16, item_y, panel_w - 32, 45, bg);
        if (is_sel) {
            graphics_draw_rect_outline(panel_x + 16, item_y, panel_w - 32, 45, 2.0f, COLOR_YELLOW_ACCENT);
        }

        graphics_draw_text_shadow(panel_x + 28, item_y + 8, 0.90f, is_sel ? COLOR_YELLOW_ACCENT : COLOR_WHITE, s_settings[i].label);
        graphics_draw_text_shadow(panel_x + 28, item_y + 26, 0.74f, COLOR_TEXT_MUTED, s_settings[i].description);

        // Control Pill (ON/OFF or SELECT)
        float ctrl_x = panel_x + panel_w - 140.0f;
        if (s_settings[i].type == SETTING_TOGGLE) {
            bool val = s_settings[i].bool_val;
            uint32_t top = val ? 0xFF2A6E24 : 0xFF4B433B;
            uint32_t bot = val ? 0xFF174213 : 0xFF2B2520;
            graphics_draw_bevel_panel(ctrl_x, item_y + 8, 85, 26, top, bot, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
            graphics_draw_text_shadow(ctrl_x + 28, item_y + 14, 0.82f, val ? COLOR_GREEN_ENABLED : COLOR_TEXT_MUTED, val ? "ON" : "OFF");
        } else if (s_settings[i].type == SETTING_SELECT) {
            graphics_draw_bevel_panel(ctrl_x - 20, item_y + 8, 105, 26, 0xFF3D3228, 0xFF221A14, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
            const char *opt = s_settings[i].options[s_settings[i].int_val];
            graphics_draw_text_shadow(ctrl_x - 5, item_y + 14, 0.82f, COLOR_YELLOW_ACCENT, opt);
        }

        item_y += 50.0f;
    }
}

// ----------------------------------------------------
// TAB 4: SYSTEM METRICS & HARDWARE (Matching Ui.html)
// ----------------------------------------------------
void menu_render_tab_system(void) {
    float panel_x = 40.0f;
    float panel_y = 130.0f;

    // Card 1: Thermal & Fan Control
    float c1_w = 370.0f;
    float c1_h = 240.0f;
    graphics_draw_bevel_panel(panel_x, panel_y, c1_w, c1_h, 0xEE16120E, 0xFA0F0D0A, COLOR_BORDER_MID, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(panel_x + 14, panel_y + 14, 0.92f, COLOR_YELLOW_ACCENT, "THERMAL & FAN CONTROL");

    // CPU Temp Gauge
    char cpu_str[32];
    snprintf(cpu_str, sizeof(cpu_str), "CELL CPU TEMP: %d C", s_sys_info.cpu_temp);
    graphics_draw_text_shadow(panel_x + 14, panel_y + 40, 0.80f, COLOR_WHITE, cpu_str);
    graphics_draw_gauge_bar(panel_x + 14, panel_y + 56, c1_w - 28, 14, s_sys_info.cpu_temp / 100.0f, COLOR_GREEN_ENABLED, 0xFF14100E);

    // RSX Temp Gauge
    char rsx_str[32];
    snprintf(rsx_str, sizeof(rsx_str), "RSX GPU TEMP: %d C", s_sys_info.rsx_temp);
    graphics_draw_text_shadow(panel_x + 14, panel_y + 80, 0.80f, COLOR_WHITE, rsx_str);
    graphics_draw_gauge_bar(panel_x + 14, panel_y + 96, c1_w - 28, 14, s_sys_info.rsx_temp / 100.0f, COLOR_GREEN_ENABLED, 0xFF14100E);

    // RAM Usage Gauge
    char ram_str[32];
    snprintf(ram_str, sizeof(ram_str), "XDR SYSTEM RAM: %d / %d MB", s_sys_info.ram_used_mb, s_sys_info.ram_total_mb);
    graphics_draw_text_shadow(panel_x + 14, panel_y + 120, 0.80f, COLOR_WHITE, ram_str);
    graphics_draw_gauge_bar(panel_x + 14, panel_y + 136, c1_w - 28, 14, (float)s_sys_info.ram_used_mb / (float)s_sys_info.ram_total_mb, COLOR_YELLOW_ACCENT, 0xFF14100E);

    // Fan Presets
    graphics_draw_text_shadow(panel_x + 14, panel_y + 162, 0.80f, COLOR_TEXT_MUTED, "FAN POLICY:");
    const char *kFanPresets[4] = { "SYSCON", "35%", "45%", "60%" };
    float f_x = panel_x + 14.0f;
    for (int i = 0; i < 4; i++) {
        bool f_sel = (s_sys_info.fan_preset == i);
        uint32_t top = f_sel ? 0xFF5A4F44 : 0xFF3D3228;
        uint32_t bot = f_sel ? 0xFF362E27 : 0xFF221A14;
        graphics_draw_bevel_panel(f_x, panel_y + 182, 75, 26, top, bot, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
        graphics_draw_text_shadow(f_x + 10, panel_y + 188, 0.76f, f_sel ? COLOR_YELLOW_ACCENT : COLOR_TEXT_MUTED, kFanPresets[i]);
        f_x += 85.0f;
    }

    // Card 2: Storage & Firmware
    float c2_x = 425.0f;
    float c2_w = 383.0f;
    float c2_h = 240.0f;
    graphics_draw_bevel_panel(c2_x, panel_y, c2_w, c2_h, 0xEE16120E, 0xFA0F0D0A, COLOR_BORDER_MID, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(c2_x + 14, panel_y + 14, 0.92f, COLOR_YELLOW_ACCENT, "CFW & STORAGE INFO");

    graphics_draw_text_shadow(c2_x + 14, panel_y + 40, 0.80f, COLOR_TEXT_MAIN, "FIRMWARE: CFW 4.84.2 D-REX (DEX)");
    graphics_draw_text_shadow(c2_x + 14, panel_y + 62, 0.80f, COLOR_TEXT_MAIN, "COBRA PAYLOAD: v8.3 ENABLED");
    graphics_draw_text_shadow(c2_x + 14, panel_y + 84, 0.80f, COLOR_TEXT_MAIN, "ACTIVE TITLE ID: BLUS31426 (US)");
    graphics_draw_text_shadow(c2_x + 14, panel_y + 106, 0.80f, COLOR_GREEN_ENABLED, "ACTIVE MODS: 7 / 8 ENABLED");
    graphics_draw_text_shadow(c2_x + 14, panel_y + 128, 0.80f, COLOR_TEXT_MUTED, "HDD0 MOD DIR: /dev_hdd0/game/USRDIR/");

    // Actions in Card 2
    graphics_draw_bevel_panel(c2_x + 14, panel_y + 175, 160, 32, 0xFF4B433B, 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(c2_x + 24, panel_y + 184, 0.78f, COLOR_WHITE, "CLEAR MOD CACHE");

    graphics_draw_bevel_panel(c2_x + 190, panel_y + 175, 160, 32, 0xFF8A241E, 0xFF4E130F, 0xFFD4352D, 0xFF2E0A07);
    graphics_draw_text_shadow(c2_x + 208, panel_y + 184, 0.78f, COLOR_WHITE, "REBOOT TO XMB");
}

// ----------------------------------------------------
// Mod Options Modal Dialog (Matching Ui.html)
// ----------------------------------------------------
void menu_render_modal(void) {
    if (!s_modal.is_open) return;

    // Dim background overlay
    graphics_draw_rect(0, 0, SCREEN_W, SCREEN_H, 0xB8000000);

    float mw = 480.0f;
    float mh = 260.0f;
    float mx = (SCREEN_W - mw) * 0.5f;
    float my = (SCREEN_H - mh) * 0.5f;

    graphics_draw_bevel_panel(mx, my, mw, mh, 0xFF2D251E, 0xFF171310, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);

    // Modal Header
    graphics_draw_text_shadow(mx + 20, my + 16, 1.0f, COLOR_YELLOW_ACCENT, "MOD OPTIONS -- FLY MOD v2.1");
    graphics_draw_panel_box(mx + mw - 34, my + 12, 22, 22, 0xFF3D3730, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(mx + mw - 27, my + 15, 0.75f, COLOR_WHITE, "X");

    graphics_draw_rect(mx + 16, my + 44, mw - 32, 2, 0x33FFFFFF);

    // Option 1: Flight Speed Slider
    graphics_draw_text_shadow(mx + 20, my + 56, 0.85f, (s_modal.selected_field == 0) ? COLOR_YELLOW_ACCENT : COLOR_WHITE, "Flight Speed:");
    char spd_buf[32];
    snprintf(spd_buf, sizeof(spd_buf), "%.1fx", s_modal.flight_speed);
    graphics_draw_text_shadow(mx + mw - 70, my + 56, 0.85f, COLOR_YELLOW_ACCENT, spd_buf);
    graphics_draw_gauge_bar(mx + 20, my + 74, mw - 40, 14, s_modal.flight_speed / 5.0f, COLOR_YELLOW_ACCENT, 0xFF14100E);

    // Option 2: Double-tap Jump Toggle
    graphics_draw_text_shadow(mx + 20, my + 104, 0.85f, (s_modal.selected_field == 1) ? COLOR_YELLOW_ACCENT : COLOR_WHITE, "Double-tap Jump to Fly:");
    graphics_draw_bevel_panel(mx + mw - 90, my + 100, 70, 24, s_modal.double_tap_jump ? 0xFF2A6E24 : 0xFF4B433B, s_modal.double_tap_jump ? 0xFF174213 : 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(mx + mw - 70, my + 105, 0.78f, s_modal.double_tap_jump ? COLOR_GREEN_ENABLED : COLOR_TEXT_MUTED, s_modal.double_tap_jump ? "ON" : "OFF");

    // Option 3: Survival Mode Flight Toggle
    graphics_draw_text_shadow(mx + 20, my + 138, 0.85f, (s_modal.selected_field == 2) ? COLOR_YELLOW_ACCENT : COLOR_WHITE, "Fly in Survival Mode:");
    graphics_draw_bevel_panel(mx + mw - 90, my + 134, 70, 24, s_modal.survival_flight ? 0xFF2A6E24 : 0xFF4B433B, s_modal.survival_flight ? 0xFF174213 : 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(mx + mw - 70, my + 139, 0.78f, s_modal.survival_flight ? COLOR_GREEN_ENABLED : COLOR_TEXT_MUTED, s_modal.survival_flight ? "ON" : "OFF");

    // Modal Footer: Save / Cancel
    graphics_draw_rect(mx + 16, my + 180, mw - 32, 2, 0x33FFFFFF);

    graphics_draw_bevel_panel(mx + 40, my + 198, 140, 32, 0xFF4B433B, 0xFF2B2520, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(mx + 75, my + 207, 0.82f, COLOR_WHITE, "DEFAULTS");

    graphics_draw_bevel_panel(mx + mw - 190, my + 198, 150, 32, 0xFF2A6E24, 0xFF174213, 0xFF4AE640, 0xFF0B2109);
    graphics_draw_text_shadow(mx + mw - 170, my + 207, 0.82f, COLOR_GREEN_ENABLED, "SAVE & APPLY");
}

// ----------------------------------------------------
// Toast Notification Overlay (Matching Ui.html)
// ----------------------------------------------------
void menu_render_toast(void) {
    if (!s_toast.active) return;

    float tw = 320.0f;
    float th = 34.0f;
    float tx = SCREEN_W - tw - 20.0f;
    float ty = 74.0f;

    // Toast panel with green accent border
    graphics_draw_bevel_panel(tx, ty, tw, th, 0xEE1E1812, 0xFA120E0A, 0xFF4AE640, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(tx + 16, ty + 10, 0.82f, COLOR_YELLOW_ACCENT, s_toast.message);
}

// ----------------------------------------------------
// Simulated PS3 XMB Reboot Overlay (Matching Ui.html)
// ----------------------------------------------------
void menu_render_xmb_reboot(void) {
    if (!s_sys_info.xmb_rebooting) return;

    graphics_draw_rect(0, 0, SCREEN_W, SCREEN_H, 0xFA000000);

    graphics_draw_text_shadow(330, 210, 1.6f, COLOR_WHITE, "PlayStation(R)3");
    graphics_draw_text_shadow(270, 255, 0.95f, 0xFF9EC5FF, "RETURNING TO XMB(TM) -- CFW 4.84 DEX...");

    graphics_draw_bevel_panel(320, 310, 220, 36, 0xFF3D3228, 0xFF221A14, COLOR_BORDER_LIGHT, COLOR_BORDER_DARK);
    graphics_draw_text_shadow(335, 321, 0.85f, COLOR_YELLOW_ACCENT, "RETURN TO MOD LOADER (O)");
}

// ----------------------------------------------------
// Footer: DualShock 3 Prompt Bar (With Vector PS Buttons)
// ----------------------------------------------------
static void menu_render_footer(void) {
    float foot_y = 478.0f;
    graphics_draw_rect_vertical_gradient(0, foot_y - 8, SCREEN_W, 42, 0xCC0A0806, 0xFF000000);

    // Cross: Toggle/Select
    graphics_draw_ps_cross(46, foot_y + 11, 4.5f);
    graphics_draw_text_shadow(56, foot_y + 4, 0.82f, COLOR_PS_CROSS, "TOGGLE/SELECT");

    // Circle: Back
    graphics_draw_ps_circle(196, foot_y + 11, 4.5f);
    graphics_draw_text_shadow(206, foot_y + 4, 0.82f, COLOR_PS_CIRCLE, "BACK");

    // Square: Options
    graphics_draw_ps_square(276, foot_y + 11, 4.5f);
    graphics_draw_text_shadow(286, foot_y + 4, 0.82f, COLOR_PS_SQUARE, "OPTIONS");

    // Triangle: Refresh
    graphics_draw_ps_triangle(376, foot_y + 11, 4.5f);
    graphics_draw_text_shadow(386, foot_y + 4, 0.82f, COLOR_PS_TRIANGLE, "REFRESH");

    // L3/R3: Scroll
    graphics_draw_text_shadow(486, foot_y + 4, 0.82f, COLOR_TEXT_MUTED, "L3/R3 SCROLL");

    // START: Boot
    graphics_draw_text_shadow(680, foot_y + 4, 0.88f, COLOR_YELLOW_ACCENT, "START BOOT");
}

// ----------------------------------------------------
// Main Master Render
// ----------------------------------------------------
void menu_render(void) {
    menu_render_header();
    menu_render_tab_bar();

    switch (s_current_tab) {
        case TAB_INSTALLED_MODS:
            menu_render_tab_installed_mods();
            break;
        case TAB_USB_INSTALL:
            menu_render_tab_usb_install();
            break;
        case TAB_SETTINGS:
            menu_render_tab_settings();
            break;
        case TAB_SYSTEM:
            menu_render_tab_system();
            break;
        default:
            break;
    }

    menu_render_footer();
    menu_render_modal();
    menu_render_toast();
    menu_render_xmb_reboot();
}

int menu_get_mod_count(void) { return s_installed_count; }
ModItem* menu_get_mod(int index) {
    if (index >= 0 && index < s_installed_count) return &s_installed_mods[index];
    return NULL;
}
int menu_get_selected_index(void) { return s_selected_mod_idx; }
MenuTab menu_get_current_tab(void) { return s_current_tab; }
bool menu_is_modal_open(void) { return s_modal.is_open; }
