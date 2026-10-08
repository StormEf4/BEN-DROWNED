#ifndef Z_EN_BEN_H
#define Z_EN_BEN_H

#include "global.h"

struct EnBen;

typedef void (*EnBenActionFunc)(struct EnBen*, PlayState*);

typedef struct EnBen {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderCylinder collider;
    /* 0x190 */ EnBenActionFunc actionFunc;
    /* 0x194 */ s16 timer;
    /* 0x196 */ u8 hasSpoken;
} EnBen; // size = 0x198

typedef enum {
    /* 0 */ EN_BEN_MODE_CREEP // Moves toward the player only while off-screen
} EnBenMode;

#define EN_BEN_GET_MODE(thisx) ((thisx)->params & 0xF)

#endif // Z_EN_BEN_H
