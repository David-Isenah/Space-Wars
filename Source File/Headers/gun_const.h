#ifndef GUN_CONST_H_INCLUDED
#define GUN_CONST_H_INCLUDED

#include "gun.h"

namespace Gun
{
    static const unsigned int GUN_MAGAZINE_SIZE[NumOfGuns] = {10, 32, 60, 36};
    static const unsigned int GUN_LEVEL_START_AMMO[NumOfGuns] = {0, 120, 0, 36};
    static const unsigned int GUN_MAX_AMMO[NumOfGuns] = {0, 300, 160, 180};
    static const unsigned int GUN_MIN_AMMO_RESTOCK[NumOfGuns] = {0, 20, 8, 10};
    static const unsigned int GUN_MAX_AMMO_RESTOCK[NumOfGuns] = {0, 60, 32, 24};
    static const unsigned int GUN_RESTOCK_PER_POWER_FACTOR[NumOfGuns] = {0, 20, 6, 8};
    static const float GUN_RELOAD_TIME[NumOfGuns] = {1.f, 1.f, 1.4f};
    static const float GUN_AI_AMMO_USAGE[NumOfGuns] = {0.8f, 0.8f, 0.9f, 0.9f};
}

#endif // GUN_CONST_H_INCLUDED
