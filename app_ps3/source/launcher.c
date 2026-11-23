#include "launcher.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <lv2/process.h>

static const char *kCandidates[] = {
    "/dev_hdd0/game/" MC_TITLE_ID_EU "/USRDIR/EBOOT.BIN",
    "/dev_hdd0/game/" MC_TITLE_ID_US_PSN "/USRDIR/EBOOT.BIN",
    "/dev_hdd0/game/" MC_TITLE_ID_US_DISC "/USRDIR/EBOOT.BIN",
    "/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN",
    NULL
};

bool launcher_save_active_mods_config(void) {
    const char *cfg_path = "/dev_hdd0/game/" MC_TITLE_ID_EU "/USRDIR/voxel3d/config.json";
    FILE *fp = fopen(cfg_path, "w");
    if (!fp) {
        // Fallback local path
        fp = fopen("/dev_hdd0/game/MCPS3MODS/USRDIR/DATA/config.json", "w");
        if (!fp) return false;
    }

    fprintf(fp, "{\n  \"version\": \"1.4.2\",\n  \"enabled_mods\": [\n");
    int count = menu_get_mod_count();
    bool first = true;
    for (int i = 0; i < count; i++) {
        ModItem *mod = menu_get_mod(i);
        if (mod && mod->enabled) {
            if (!first) fprintf(fp, ",\n");
            fprintf(fp, "    { \"name\": \"%s\", \"version\": \"%s\" }", mod->name, mod->version);
            first = false;
        }
    }
    fprintf(fp, "\n  ]\n}\n");
    fclose(fp);
    return true;
}

bool launcher_boot_minecraft(void) {
    launcher_save_active_mods_config();

    const char *target_eboot = NULL;
    for (int i = 0; kCandidates[i] != NULL; i++) {
        if (access(kCandidates[i], F_OK) == 0) {
            target_eboot = kCandidates[i];
            break;
        }
    }

    if (!target_eboot) {
        // Default to EU title ID
        target_eboot = kCandidates[0];
    }

    // exitspawn2 parameters: (target_path, argv, envp, data, data_size, prio, flags)
    sys_game_process_exitspawn2(target_eboot, NULL, NULL, NULL, 0, 1001, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
    return true;
}
