#ifndef VOXEL3D_CONFIG_H
#define VOXEL3D_CONFIG_H

// Valores tomados de config.json (title_id BLES01976).
#define VOX3_ITEM_ID              290u
#define VOX3_ITEM_METADATA        (-1)

#define VOX3_SCALE                0.0625f

// transform.rest (posicion de cadera)
#define VOX3_HIP_X                0.26f
#define VOX3_HIP_Y                (-0.21f)
#define VOX3_HIP_Z                0.48f
#define VOX3_HIP_ROT_X            (-4.0f)
#define VOX3_HIP_ROT_Y            (-16.0f)
#define VOX3_HIP_ROT_Z            6.0f

// transform.ads (miras alineadas)
#define VOX3_ADS_X                0.0f
#define VOX3_ADS_Y                (-0.135f)
#define VOX3_ADS_Z                0.29f
#define VOX3_ADS_ROT_X            0.0f
#define VOX3_ADS_ROT_Y            0.0f
#define VOX3_ADS_ROT_Z            0.0f
#define VOX3_ADS_FOV              0.70f

// Velocidad de la interpolacion LERP de ADS (L2).
#define VOX3_ADS_SPEED            8.0f

// recoil.kick_* y velocidades.
#define VOX3_RECOIL_POS_X         0.0f
#define VOX3_RECOIL_POS_Y         0.018f
#define VOX3_RECOIL_POS_Z         (-0.07f)
#define VOX3_RECOIL_ROT_X         11.0f
#define VOX3_RECOIL_ROT_Y         1.2f
#define VOX3_RECOIL_ROT_Z         (-2.0f)
#define VOX3_RECOIL_KICK          22.0f
#define VOX3_RECOIL_RECOVER       15.0f
#define VOX3_RECOIL_FRAMES        6

// weapon.fire_rate_rpm = 700 -> 60000/700 = 85.7 ms entre disparos.
#define VOX3_FIRE_INTERVAL        0.0857f
#define VOX3_MAGAZINE_SIZE        30u
#define VOX3_RESERVE_AMMO         120u

#define VOX3_DEG2RAD(d)           ((d) * 0.017453292519943295f)

#endif /* VOXEL3D_CONFIG_H */
