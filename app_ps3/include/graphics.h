#pragma once
#include <tiny3d.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Resolution standard: 848x512 2D projection space (scales perfectly to 720p/1080p)
#define SCREEN_W 848.0f
#define SCREEN_H 512.0f

// Palette matching Ui.html CSS tokens exactly
#define COLOR_BG_BACKDROP     0xFF0E1018
#define COLOR_PANEL_BG        0xDC18130F // rgba(24, 19, 15, 0.86)
#define COLOR_PANEL_DEEP      0xE6100D0A // rgba(16, 13, 10, 0.90)
#define COLOR_TAB_INACTIVE    0xE64A4138 // rgba(74, 65, 56, 0.90)
#define COLOR_TAB_ACTIVE      0xF02D251E // rgba(45, 37, 30, 0.94)

#define COLOR_BORDER_DARK     0xFF0A0806 // #0a0806
#define COLOR_BORDER_LIGHT    0xFF7C6F63 // #7c6f63
#define COLOR_BORDER_MID      0xFF4E443B // #4e443b

#define COLOR_YELLOW_ACCENT   0xFFF4E436 // #f4e436
#define COLOR_YELLOW_GLOW     0x40F4E436 // rgba(244, 228, 54, 0.22)
#define COLOR_GREEN_ENABLED   0xFF5AE84A // #5ae84a
#define COLOR_RED_DISABLED    0xFFEA4335 // #ea4335

#define COLOR_PS_CROSS        0xFF6AB7FF // #6ab7ff
#define COLOR_PS_CIRCLE       0xFFF28B82 // #f28b82
#define COLOR_PS_SQUARE       0xFFF48FB1 // #f48fb1
#define COLOR_PS_TRIANGLE     0xFF81E699 // #81e699

#define COLOR_TEXT_MAIN       0xFFF4F2EC // #f4f2ec
#define COLOR_TEXT_MUTED      0xFFC9C3B8 // #c9c3b8
#define COLOR_WHITE           0xFFFFFFFF
#define COLOR_BLACK           0xFF000000

// Mod Thumbnail Types (matching pixel art SVG generator in Ui.html)
typedef enum {
    THUMB_TMI = 0,     // Too Many Items (Chest + gems)
    THUMB_XRAY,        // X-Ray Vision (Stone grid + glowing diamond/gold ores)
    THUMB_FLY,         // Fly Mod (Wings / feather in sky)
    THUMB_LIGHTS,      // Dynamic Lights (Torch with radial glow)
    THUMB_FPS,         // Better FPS (RSX processor & circuit traces)
    THUMB_MINIMAP,     // Minimap (Radar HUD & waypoints)
    THUMB_CAPES,       // Custom Capes (Hero cloak)
    THUMB_VOXEL3D,     // Voxel3D First-Person (3D weapon model)
    THUMB_GENERIC      // USB package
} ModThumbType;

void graphics_init(void);
void graphics_begin_frame(void);
void graphics_end_frame(void);

// Primitive rendering
void graphics_draw_rect(float x, float y, float w, float h, uint32_t color);
void graphics_draw_rect_vertical_gradient(float x, float y, float w, float h, uint32_t top_color, uint32_t bot_color);
void graphics_draw_rect_outline(float x, float y, float w, float h, float thickness, uint32_t color);
void graphics_draw_panel_box(float x, float y, float w, float h, uint32_t bg_color, uint32_t border_color);
void graphics_draw_bevel_panel(float x, float y, float w, float h, uint32_t bg_top, uint32_t bg_bot, uint32_t border_light, uint32_t border_dark);
void graphics_draw_gauge_bar(float x, float y, float w, float h, float percentage, uint32_t fill_color, uint32_t bg_color);

// Text rendering with embedded Silkscreen-compatible 8x8 bitmap font
void graphics_draw_text(float x, float y, float scale, uint32_t color, const char *text);
void graphics_draw_text_shadow(float x, float y, float scale, uint32_t color, const char *text);
float graphics_get_text_width(float scale, const char *text);

// High-fidelity Ui.html components in C
void graphics_draw_backdrop(void);
void graphics_draw_ps3_brand(float x, float y);
void graphics_draw_mc_logo(float x, float y, float scale);
void graphics_draw_pixel_thumb(float x, float y, float w, float h, ModThumbType type);

// DualShock 3 vector glyphs
void graphics_draw_ps_cross(float cx, float cy, float radius);
void graphics_draw_ps_circle(float cx, float cy, float radius);
void graphics_draw_ps_square(float cx, float cy, float radius);
void graphics_draw_ps_triangle(float cx, float cy, float radius);

void graphics_draw_textured_rect(float x, float y, float w, float h, uint32_t texture_id);

#ifdef __cplusplus
}
#endif
