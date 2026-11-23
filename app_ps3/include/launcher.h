#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Target game title IDs
#define MC_TITLE_ID_EU      "BLES01976"
#define MC_TITLE_ID_US_PSN  "NPUB31419"
#define MC_TITLE_ID_US_DISC "BLUS31426"

// Launches Minecraft via sys_game_process_exitspawn2
bool launcher_boot_minecraft(void);

// Saves active mods config to dev_hdd0/game/BLES01976/USRDIR/voxel3d/config.json
bool launcher_save_active_mods_config(void);

#ifdef __cplusplus
}
#endif
