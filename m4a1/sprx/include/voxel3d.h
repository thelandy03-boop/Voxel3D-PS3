#ifndef VOXEL3D_H
#define VOXEL3D_H

#include <stdint.h>
#include "vx3_format.h"

// Configuracion resuelta del item (data-driven desde config.json).
#pragma pack(push, 4)

struct VoxelItemConfig {
    uint32_t item_id;
    int32_t  metadata;
    char     model_path[128];
    float    scale[3];
    float    rest_pos[3];
    float    rest_rot[3];
    float    ads_pos[3];
    float    ads_rot[3];
    float    ads_fov;
    float    recoil_kick_pos[3];
    float    recoil_kick_rot[3];
    float    recoil_recovery;
    float    muzzle_pos[3];
    uint32_t model_slot_index;
    uint8_t  active;
};

struct VoxelWeaponConfig {
    float  damage;
    float  fire_interval;
    float  projectile_speed;
    uint32_t magazine_size;
    uint32_t reserve_ammo;
    float  reload_seconds;
    uint8_t auto_fire;
};

#pragma pack(pop)

#endif /* VOXEL3D_H */
