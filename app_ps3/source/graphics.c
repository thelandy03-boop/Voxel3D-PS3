#include "graphics.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Embedded 8x8 Silkscreen/Pixel-Art Font Matrix (ASCII 32 to 126)
// ---------------------------------------------------------------------------
static const uint8_t s_font8x8[95][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 32 ' '
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // 33 '!'
    {0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, // 34 '"'
    {0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00}, // 35 '#'
    {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00}, // 36 '$'
    {0x00, 0x66, 0xA6, 0xD8, 0x1B, 0x65, 0x66, 0x00}, // 37 '%'
    {0x30, 0x48, 0x30, 0x6E, 0xD9, 0x89, 0x76, 0x00}, // 38 '&'
    {0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00}, // 39 '''
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00}, // 40 '('
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00}, // 41 ')'
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // 42 '*'
    {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00}, // 43 '+'
    {0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 0x00}, // 44 ','
    {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, // 45 '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // 46 '.'
    {0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00}, // 47 '/'
    {0x3C, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x3C, 0x00}, // 48 '0'
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 49 '1'
    {0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00}, // 50 '2'
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, // 51 '3'
    {0x0C, 0x1C, 0x3C, 0x6C, 0xFE, 0x0C, 0x0C, 0x00}, // 52 '4'
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 53 '5'
    {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, // 54 '6'
    {0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00}, // 55 '7'
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, // 56 '8'
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00}, // 57 '9'
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // 58 ':'
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00}, // 59 ';'
    {0x0E, 0x1C, 0x38, 0x70, 0x38, 0x1C, 0x0E, 0x00}, // 60 '<'
    {0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, // 61 '='
    {0x70, 0x38, 0x1C, 0x0E, 0x1C, 0x38, 0x70, 0x00}, // 62 '>'
    {0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00}, // 63 '?'
    {0x3C, 0x42, 0x99, 0xA5, 0x99, 0x42, 0x3C, 0x00}, // 64 '@'
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00}, // 65 'A'
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // 66 'B'
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // 67 'C'
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // 68 'D'
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, // 69 'E'
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, // 70 'F'
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3E, 0x00}, // 71 'G'
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // 72 'H'
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 73 'I'
    {0x0E, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 74 'J'
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // 75 'K'
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // 76 'L'
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // 77 'M'
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, // 78 'N'
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 79 'O'
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, // 80 'P'
    {0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00}, // 81 'Q'
    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00}, // 82 'R'
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, // 83 'S'
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 84 'T'
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 85 'U'
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // 86 'V'
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // 87 'W'
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, // 88 'X'
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, // 89 'Y'
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, // 90 'Z'
    {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00}, // 91 '['
    {0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00}, // 92 '\'
    {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00}, // 93 ']'
    {0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00}, // 94 '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00}, // 95 '_'
    {0x30, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00}, // 96 '`'
    {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00}, // 97 'a'
    {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00}, // 98 'b'
    {0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00}, // 99 'c'
    {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00}, // 100 'd'
    {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, // 101 'e'
    {0x1C, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x30, 0x00}, // 102 'f'
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C}, // 103 'g'
    {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, // 104 'h'
    {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 105 'i'
    {0x06, 0x00, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 106 'j'
    {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00}, // 107 'k'
    {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 108 'l'
    {0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // 109 'm'
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, // 110 'n'
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 111 'o'
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60}, // 112 'p'
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06}, // 113 'q'
    {0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00}, // 114 'r'
    {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00}, // 115 's'
    {0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0C, 0x00}, // 116 't'
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00}, // 117 'u'
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // 118 'v'
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00}, // 119 'w'
    {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00}, // 120 'x'
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C}, // 121 'y'
    {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}, // 122 'z'
    {0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00}, // 123 '{'
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 124 '|'
    {0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00}, // 125 '}'
    {0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 126 '~'
};

void graphics_init(void) {
    tiny3d_Init(1024 * 1024);
}

void graphics_begin_frame(void) {
    tiny3d_Clear(COLOR_BG_BACKDROP, TINY3D_CLEAR_ALL);
    tiny3d_AlphaTest(1, 0, TINY3D_ALPHA_FUNC_GEQUAL);
    tiny3d_BlendFunc(1, 
                     TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
                     TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ONE_MINUS_SRC_ALPHA,
                     TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
    tiny3d_Project2D();
    
    // Draw high-fidelity blurred Minecraft sunset backdrop
    graphics_draw_backdrop();
}

void graphics_end_frame(void) {
    tiny3d_Flip();
}

void graphics_draw_rect(float x, float y, float w, float h, uint32_t color) {
    tiny3d_SetPolygon(TINY3D_QUADS);
    tiny3d_VertexPos(x, y, 65535);
    tiny3d_VertexColor(color);
    tiny3d_VertexPos(x + w, y, 65535);
    tiny3d_VertexColor(color);
    tiny3d_VertexPos(x + w, y + h, 65535);
    tiny3d_VertexColor(color);
    tiny3d_VertexPos(x, y + h, 65535);
    tiny3d_VertexColor(color);
    tiny3d_End();
}

void graphics_draw_rect_vertical_gradient(float x, float y, float w, float h, uint32_t top_color, uint32_t bot_color) {
    tiny3d_SetPolygon(TINY3D_QUADS);
    tiny3d_VertexPos(x, y, 65535);
    tiny3d_VertexColor(top_color);
    tiny3d_VertexPos(x + w, y, 65535);
    tiny3d_VertexColor(top_color);
    tiny3d_VertexPos(x + w, y + h, 65535);
    tiny3d_VertexColor(bot_color);
    tiny3d_VertexPos(x, y + h, 65535);
    tiny3d_VertexColor(bot_color);
    tiny3d_End();
}

void graphics_draw_rect_outline(float x, float y, float w, float h, float thickness, uint32_t color) {
    graphics_draw_rect(x, y, w, thickness, color);
    graphics_draw_rect(x, y + h - thickness, w, thickness, color);
    graphics_draw_rect(x, y + thickness, thickness, h - (thickness * 2.0f), color);
    graphics_draw_rect(x + w - thickness, y + thickness, thickness, h - (thickness * 2.0f), color);
}

void graphics_draw_panel_box(float x, float y, float w, float h, uint32_t bg_color, uint32_t border_color) {
    graphics_draw_rect(x, y, w, h, bg_color);
    graphics_draw_rect_outline(x, y, w, h, 2.0f, border_color);
    graphics_draw_rect_outline(x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, 1.0f, 0x33FFFFFF);
}

void graphics_draw_bevel_panel(float x, float y, float w, float h, uint32_t bg_top, uint32_t bg_bot, uint32_t border_light, uint32_t border_dark) {
    // Fill interior with linear vertical gradient
    graphics_draw_rect_vertical_gradient(x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, bg_top, bg_bot);
    // Outer dark frame
    graphics_draw_rect_outline(x, y, w, h, 2.0f, border_dark);
    // Inset top and left highlight
    graphics_draw_rect(x + 2.0f, y + 2.0f, w - 4.0f, 2.0f, border_light);
    graphics_draw_rect(x + 2.0f, y + 2.0f, 2.0f, h - 4.0f, border_light);
    // Inset bottom and right shadow
    graphics_draw_rect(x + 2.0f, y + h - 4.0f, w - 4.0f, 2.0f, border_dark);
    graphics_draw_rect(x + w - 4.0f, y + 2.0f, 2.0f, h - 4.0f, border_dark);
}

void graphics_draw_gauge_bar(float x, float y, float w, float h, float percentage, uint32_t fill_color, uint32_t bg_color) {
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;

    // Background track (dark inset box)
    graphics_draw_rect(x, y, w, h, bg_color);
    graphics_draw_rect_outline(x, y, w, h, 1.5f, COLOR_BORDER_DARK);

    // Active fill with subtle 3D highlight
    float fill_w = (w - 4.0f) * percentage;
    if (fill_w > 0.0f) {
        graphics_draw_rect_vertical_gradient(x + 2.0f, y + 2.0f, fill_w, h - 4.0f, fill_color, fill_color - 0x00222222);
        graphics_draw_rect(x + 2.0f, y + 2.0f, fill_w, 1.5f, 0x55FFFFFF);
    }
}

// ---------------------------------------------------------------------------
// Text Rendering System (Crisp 8x8 Silkscreen/Minecraft Pixel Font)
// ---------------------------------------------------------------------------
void graphics_draw_text(float x, float y, float scale, uint32_t color, const char *text) {
    if (!text || !*text) return;
    if (scale <= 0.1f) scale = 1.0f;

    float cur_x = x;
    float cur_y = y;
    float pw = 1.25f * scale;
    float ph = 1.25f * scale;

    while (*text) {
        char c = *text++;
        if (c == '\n') {
            cur_y += 10.0f * scale;
            cur_x = x;
            continue;
        }

        if (c < 32 || c > 126) c = '?';
        int idx = c - 32;

        for (int r = 0; r < 8; r++) {
            uint8_t row = s_font8x8[idx][r];
            if (!row) continue;
            for (int col = 0; col < 8; col++) {
                if (row & (0x80 >> col)) {
                    graphics_draw_rect(cur_x + (col * pw), cur_y + (r * ph), pw, ph, color);
                }
            }
        }
        cur_x += 8.0f * pw + (0.5f * scale);
    }
}

void graphics_draw_text_shadow(float x, float y, float scale, uint32_t color, const char *text) {
    if (!text || !*text) return;
    // Minecraft authentic 2px drop shadow
    float shadow_off = 1.5f * scale;
    graphics_draw_text(x + shadow_off, y + shadow_off, scale, 0xDD000000, text);
    graphics_draw_text(x, y, scale, color, text);
}

float graphics_get_text_width(float scale, const char *text) {
    if (!text) return 0.0f;
    float pw = 1.25f * scale;
    return (float)strlen(text) * (8.0f * pw + (0.5f * scale));
}

// ---------------------------------------------------------------------------
// Minecraft Sunset Backdrop & Atmosphere (Matching Ui.html)
// ---------------------------------------------------------------------------
void graphics_draw_backdrop(void) {
    // 1. Sky gradients: twilight blue -> sunset orange -> dusk purple
    graphics_draw_rect_vertical_gradient(0, 0, SCREEN_W, 200, 0xFF14192B, 0xFF3D2533);
    graphics_draw_rect_vertical_gradient(0, 200, SCREEN_W, 160, 0xFF3D2533, 0xFF7D3C25);
    graphics_draw_rect_vertical_gradient(0, 360, SCREEN_W, 152, 0xFF7D3C25, 0xFF181014);

    // 2. Silhouetted distant Minecraft voxel landscape at horizon
    float hor_y = 330.0f;
    graphics_draw_rect(0, hor_y + 20, 160, 162, 0xFF1C1318);
    graphics_draw_rect(160, hor_y + 35, 240, 147, 0xFF160E12);
    graphics_draw_rect(400, hor_y + 10, 280, 172, 0xFF1A1116);
    graphics_draw_rect(680, hor_y + 28, 168, 154, 0xFF140D10);

    // 3. Dark cinematic vignette gradient overlays (top and bottom)
    graphics_draw_rect_vertical_gradient(0, 0, SCREEN_W, 80, 0x99000000, 0x22000000);
    graphics_draw_rect_vertical_gradient(0, SCREEN_H - 100, SCREEN_W, 100, 0x33000000, 0xCC000000);
}

// ---------------------------------------------------------------------------
// PlayStation Brand Wordmark & Curved Vector Logo
// ---------------------------------------------------------------------------
void graphics_draw_ps3_brand(float x, float y) {
    // Curved PS logo emblem
    graphics_draw_rect(x + 2, y + 2, 4, 18, 0xFFF5F5F5);
    graphics_draw_rect(x + 6, y + 2, 10, 4, 0xFFF5F5F5);
    graphics_draw_rect(x + 12, y + 6, 4, 6, 0xFFF5F5F5);
    graphics_draw_rect(x + 6, y + 10, 8, 4, 0xFFF5F5F5);
    graphics_draw_rect(x + 2, y + 14, 16, 4, 0xFFF5F5F5);
    graphics_draw_rect(x + 14, y + 18, 4, 4, 0xFFF5F5F5);

    // Wordmark
    graphics_draw_text_shadow(x + 24, y + 3, 1.15f, 0xFFF5F5F5, "PlayStation(R)3");
}

// ---------------------------------------------------------------------------
// 3D Stone Minecraft Logo with Cracks & Creeper Face (Matching Ui.html SVG)
// ---------------------------------------------------------------------------
void graphics_draw_mc_logo(float x, float y, float scale) {
    if (scale <= 0.1f) scale = 1.0f;
    float lw = 230.0f * scale;
    float lh = 44.0f * scale;

    // 1. 3D Extruded Stone Depth (dark stone shadow shifted +6px down)
    graphics_draw_rect_vertical_gradient(x - 4, y + 6, lw + 8, lh, 0xFF333333, 0xFF141414);
    graphics_draw_rect_outline(x - 4, y + 6, lw + 8, lh, 2.0f, COLOR_BORDER_DARK);

    // 2. Front Stone Face (light stone gradient)
    graphics_draw_rect_vertical_gradient(x - 4, y, lw + 8, lh, 0xFFD8D8D8, 0xFF888888);
    graphics_draw_rect_outline(x - 4, y, lw + 8, lh, 2.5f, 0xFF222222);

    // 3. Crisp Stone Cracks & Texture Shading
    graphics_draw_rect(x + 18 * scale, y + 8 * scale, 6 * scale, 3 * scale, 0xFF4A4A4A);
    graphics_draw_rect(x + 65 * scale, y + 22 * scale, 8 * scale, 3 * scale, 0xFF4A4A4A);
    graphics_draw_rect(x + 110 * scale, y + 14 * scale, 6 * scale, 4 * scale, 0xFF4A4A4A);
    graphics_draw_rect(x + 175 * scale, y + 24 * scale, 7 * scale, 3 * scale, 0xFF4A4A4A);

    // 4. Creeper Face in the Center 'A'
    float c_x = x + (lw * 0.5f) - (8.0f * scale);
    float c_y = y + (lh * 0.5f) - (8.0f * scale);
    graphics_draw_rect(c_x - (4 * scale), c_y - (3 * scale), 4 * scale, 4 * scale, 0xFF121212); // Left eye
    graphics_draw_rect(c_x + (4 * scale), c_y - (3 * scale), 4 * scale, 4 * scale, 0xFF121212); // Right eye
    graphics_draw_rect(c_x - (2 * scale), c_y + (1 * scale), 4 * scale, 3 * scale, 0xFF121212); // Nose
    graphics_draw_rect(c_x - (4 * scale), c_y + (4 * scale), 8 * scale, 4 * scale, 0xFF121212); // Mouth
    graphics_draw_rect(c_x - (4 * scale), c_y + (7 * scale), 2 * scale, 3 * scale, 0xFF121212);
    graphics_draw_rect(c_x + (2 * scale), c_y + (7 * scale), 2 * scale, 3 * scale, 0xFF121212);

    // 5. Lettering overlay: "MINECRAFT" in bold stone glyphs
    graphics_draw_text_shadow(x + (14 * scale), y + (10 * scale), 1.9f * scale, 0xFFF0F0F0, "MINECRAFT");
}

// ---------------------------------------------------------------------------
// DualShock 3 Prompt Button Glyphs
// ---------------------------------------------------------------------------
void graphics_draw_ps_cross(float cx, float cy, float radius) {
    float t = 2.0f;
    // Draw diagonal cross arms
    for (float i = -radius; i <= radius; i += 1.0f) {
        graphics_draw_rect(cx + i - (t * 0.5f), cy + i - (t * 0.5f), t, t, COLOR_PS_CROSS);
        graphics_draw_rect(cx + i - (t * 0.5f), cy - i - (t * 0.5f), t, t, COLOR_PS_CROSS);
    }
}

void graphics_draw_ps_circle(float cx, float cy, float radius) {
    float d = radius * 2.0f;
    graphics_draw_rect_outline(cx - radius, cy - radius, d, d, 2.0f, COLOR_PS_CIRCLE);
}

void graphics_draw_ps_square(float cx, float cy, float radius) {
    float d = radius * 2.0f;
    graphics_draw_rect_outline(cx - radius, cy - radius, d, d, 2.0f, COLOR_PS_SQUARE);
}

void graphics_draw_ps_triangle(float cx, float cy, float radius) {
    // Hollow isosceles triangle
    float h = radius * 1.8f;
    float w = radius * 1.8f;
    // Base
    graphics_draw_rect(cx - (w * 0.5f), cy + (h * 0.5f), w, 2.0f, COLOR_PS_TRIANGLE);
    // Left & right diagonals
    for (float i = 0; i <= h; i += 1.0f) {
        float x_span = (i / h) * (w * 0.5f);
        graphics_draw_rect(cx - x_span, cy + (h * 0.5f) - i, 2.0f, 2.0f, COLOR_PS_TRIANGLE);
        graphics_draw_rect(cx + x_span, cy + (h * 0.5f) - i, 2.0f, 2.0f, COLOR_PS_TRIANGLE);
    }
}

// ---------------------------------------------------------------------------
// Mod Pixel-Art Thumbnails (Matching makeSvgThumb in Ui.html)
// ---------------------------------------------------------------------------
void graphics_draw_pixel_thumb(float x, float y, float w, float h, ModThumbType type) {
    // Dark beveled container
    graphics_draw_panel_box(x, y, w, h, 0xFF171310, COLOR_BORDER_DARK);

    float cx = x + (w * 0.5f);
    float cy = y + (h * 0.5f);

    switch (type) {
        case THUMB_TMI: { // Too Many Items: Wood chest + floating gems
            // Chest body (oak wood)
            graphics_draw_bevel_panel(cx - 24, cy - 14, 48, 34, 0xFF9E703E, 0xFF694825, 0xFFBE8A52, 0xFF3B2712);
            // Iron lock / latch
            graphics_draw_rect(cx - 5, cy - 3, 10, 10, 0xFFE0E0E0);
            graphics_draw_rect(cx - 2, cy - 1, 4, 6, 0xFF111111);
            // Floating diamond, gold, redstone gems above
            graphics_draw_rect(cx - 18, cy - 28, 8, 8, 0xFF4DE0E8); // Diamond
            graphics_draw_rect(cx - 4, cy - 32, 8, 8, 0xFFF7D53B);  // Gold
            graphics_draw_rect(cx + 10, cy - 28, 8, 8, 0xFFE84B3A); // Redstone
            break;
        }

        case THUMB_XRAY: { // X-Ray Vision: Dark stone grid + glowing ores
            graphics_draw_rect(x + 4, y + 4, w - 8, h - 8, 0xFF141B26);
            // Grid lines
            for (int i = 1; i <= 3; i++) {
                graphics_draw_rect(x + 4 + (i * 18), y + 4, 1.5f, h - 8, 0xFF35485E);
                graphics_draw_rect(x + 4, y + 4 + (i * 18), w - 8, 1.5f, 0xFF35485E);
            }
            // Glowing ore deposits
            graphics_draw_rect(cx - 6, cy - 6, 12, 12, 0xFF4EF2FF); // Diamond ore
            graphics_draw_rect(x + 12, y + 14, 9, 9, 0xFFFFD93B);   // Gold ore
            graphics_draw_rect(x + w - 24, y + h - 24, 10, 10, 0xFF52E85B); // Emerald ore
            break;
        }

        case THUMB_FLY: { // Fly Mod: Wings and feather in sky
            graphics_draw_rect_vertical_gradient(x + 4, y + 4, w - 8, h - 8, 0xFF2A5E8C, 0xFF1A3854);
            // Wing feathers
            for (int i = 0; i < 4; i++) {
                graphics_draw_rect(cx - 22 + (i * 6), cy + 8 - (i * 8), 16, 6, 0xFFFFFFFF);
                graphics_draw_rect(cx + 6 + (i * 6), cy - 16 + (i * 8), 16, 6, 0xFFFFFFFF);
            }
            // Gold quill center
            graphics_draw_rect(cx - 3, cy - 16, 6, 32, 0xFFF7D53B);
            break;
        }

        case THUMB_LIGHTS: { // Dynamic Lights: Glowing torch + flame
            graphics_draw_rect(x + 4, y + 4, w - 8, h - 8, 0xFF1A130C);
            // Ambient warm halo
            graphics_draw_rect(cx - 24, cy - 24, 48, 48, 0x44FF8C1A);
            graphics_draw_rect(cx - 16, cy - 16, 32, 32, 0x66FFB338);
            // Torch stick
            graphics_draw_rect(cx - 4, cy - 2, 8, 26, 0xFF694825);
            // Burning head
            graphics_draw_rect(cx - 6, cy - 14, 12, 12, 0xFFFFCC33);
            graphics_draw_rect(cx - 3, cy - 18, 6, 6, 0xFFFF4400);
            break;
        }

        case THUMB_FPS: { // Better FPS: High-tech RSX chip & circuits
            graphics_draw_rect(x + 4, y + 4, w - 8, h - 8, 0xFF0D2418);
            // Gold circuit traces
            graphics_draw_rect(cx - 26, cy - 1, 52, 2, 0xFFC29B38);
            graphics_draw_rect(cx - 1, cy - 26, 2, 52, 0xFFC29B38);
            // Silicon CPU package
            graphics_draw_bevel_panel(cx - 18, cy - 18, 36, 36, 0xFF2B3A33, 0xFF161F1A, 0xFF52E85B, 0xFF0A0F0D);
            graphics_draw_text_shadow(cx - 10, cy - 6, 0.75f, 0xFF5AE84A, "RSX");
            break;
        }

        case THUMB_MINIMAP: { // Minimap: Radar display
            graphics_draw_rect(x + 4, y + 4, w - 8, h - 8, 0xFF0F1A12);
            graphics_draw_rect_outline(cx - 28, cy - 28, 56, 56, 2.0f, 0xFF356942);
            // Terrain chunks
            graphics_draw_rect(cx - 20, cy - 16, 18, 22, 0xFF448A3A); // Grass
            graphics_draw_rect(cx + 2, cy - 8, 22, 14, 0xFF2A69A4);   // River
            // Player arrow at center
            graphics_draw_rect(cx - 3, cy - 3, 6, 6, 0xFFEA4335);
            break;
        }

        case THUMB_CAPES: { // Custom Capes: Royal red cape with gold trim
            graphics_draw_rect(x + 4, y + 4, w - 8, h - 8, 0xFF18131B);
            // Cape mantle
            graphics_draw_bevel_panel(cx - 16, cy - 22, 32, 44, 0xFF9E1F24, 0xFF5C0F12, 0xFFD4353B, 0xFF38080A);
            // Gold embroidery border
            graphics_draw_rect_outline(cx - 14, cy - 20, 28, 40, 2.0f, 0xFFF4E436);
            graphics_draw_rect(cx - 4, cy - 14, 8, 8, 0xFFF4E436); // Emblem
            break;
        }

        case THUMB_VOXEL3D: { // Voxel3D: 3D Voxel weapon
            graphics_draw_rect(x + 4, y + 4, w - 8, h - 8, 0xFF141924);
            // Rifle receiver
            graphics_draw_rect(cx - 22, cy - 6, 36, 10, 0xFF3A4454);
            // Barrel & Muzzle
            graphics_draw_rect(cx + 14, cy - 4, 16, 6, 0xFF262C38);
            graphics_draw_rect(cx + 30, cy - 5, 4, 8, 0xFFE08226); // Muzzle flash
            // Magazine & Pistol Grip
            graphics_draw_rect(cx - 6, cy + 4, 8, 14, 0xFF1E242E);
            graphics_draw_rect(cx - 18, cy + 4, 7, 16, 0xFF4E3D2A);
            break;
        }

        default: { // Generic USB package
            graphics_draw_bevel_panel(cx - 18, cy - 18, 36, 36, 0xFF3A3632, 0xFF221F1C, 0xFF6E6862, 0xFF12100E);
            graphics_draw_text_shadow(cx - 12, cy - 6, 0.75f, COLOR_YELLOW_ACCENT, "PKG");
            break;
        }
    }
}

void graphics_draw_textured_rect(float x, float y, float w, float h, uint32_t texture_id) {
    tiny3d_SetPolygon(TINY3D_QUADS);
    tiny3d_VertexPos(x, y, 65535);
    tiny3d_VertexTexture(0.0f, 0.0f);
    tiny3d_VertexPos(x + w, y, 65535);
    tiny3d_VertexTexture(1.0f, 0.0f);
    tiny3d_VertexPos(x + w, y + h, 65535);
    tiny3d_VertexTexture(1.0f, 1.0f);
    tiny3d_VertexPos(x, y + h, 65535);
    tiny3d_VertexTexture(0.0f, 1.0f);
    tiny3d_End();
}
