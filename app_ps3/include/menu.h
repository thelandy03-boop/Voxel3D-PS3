#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

// Four main navigation tabs
typedef enum {
    TAB_INSTALLED_MODS = 0,
    TAB_USB_INSTALL,
    TAB_SETTINGS,
    TAB_SYSTEM,
    TAB_COUNT
} MenuTab;

// TAB 1: Installed Mod Item
typedef struct {
    char name[64];
    char version[16];
    char author[32];
    char description[256];
    char file_size[16];
    bool enabled;
    bool disables_trophies;
    ModThumbType thumb_type;
    char sprx_path[128];
} ModItem;

// TAB 2: USB Mod Item
typedef struct {
    char name[64];
    char version[16];
    char file_size[16];
    char usb_path[128];
    bool installed;
    float install_progress; // 0.0f to 1.0f
} UsbModItem;

// TAB 3: Setting Item
typedef enum {
    SETTING_TOGGLE = 0,
    SETTING_SELECT
} SettingType;

typedef struct {
    char label[64];
    char description[128];
    SettingType type;
    bool bool_val;
    int int_val;
    int int_max;
    const char *options[4];
} SettingItem;

// TAB 4: System State
typedef struct {
    int cpu_temp; // e.g. 56 C
    int rsx_temp; // e.g. 51 C
    int ram_used_mb;
    int ram_total_mb;
    int vram_used_mb;
    int vram_total_mb;
    float hdd_free_gb;
    float hdd_total_gb;
    int fan_preset; // 0: SYSCON, 1: 35%, 2: 45%, 3: 60%
    char cfw_version[32];
    char title_id[16];
    bool xmb_rebooting;
} SystemHudInfo;

// Mod Options Modal (e.g. Fly Mod options)
typedef struct {
    bool is_open;
    float flight_speed; // 1.0f - 5.0f
    bool double_tap_jump;
    bool survival_flight;
    int selected_field;
} ModOptionsModal;

// Floating PS3 Toast Notification
typedef struct {
    bool active;
    char message[64];
    int timer_frames;
} ToastNotification;

// Core Menu API
void menu_init(void);
void menu_update(void);
void menu_render(void);

// Tab specific renders
void menu_render_tab_installed_mods(void);
void menu_render_tab_usb_install(void);
void menu_render_tab_settings(void);
void menu_render_tab_system(void);
void menu_render_modal(void);
void menu_render_toast(void);
void menu_render_xmb_reboot(void);

// Interactive actions
void menu_show_toast(const char *msg);
void menu_trigger_xmb_reboot(void);

// Data accessors
int menu_get_mod_count(void);
ModItem* menu_get_mod(int index);
int menu_get_selected_index(void);
MenuTab menu_get_current_tab(void);
bool menu_is_modal_open(void);

#ifdef __cplusplus
}
#endif
