#include <ppu-lv2.h>
#include <sys/spu.h>
#include <lv2/spu.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <io/pad.h>

#include "graphics.h"
#include "pad.h"
#include "menu.h"
#include "launcher.h"

static bool s_running = true;

int main(int argc, char* argv[]) {
    // 1. Initialize graphics & pad
    graphics_init();
    pad_init();
    menu_init();

    // 2. Main application loop
    while (s_running) {
        // Read DualShock 3 inputs
        pad_update();

        // Check for exit condition (Circle only exits if no modal or overlay is active)
        // Pad input is primarily handled inside menu_update()

        // Update UI logic
        menu_update();

        // Render frame
        graphics_begin_frame();
        menu_render();
        graphics_end_frame();
    }

    return 0;
}
