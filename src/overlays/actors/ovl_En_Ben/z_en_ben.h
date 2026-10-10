#ifndef Z_EN_BEN_H
#define Z_EN_BEN_H

#include "global.h"

struct EnBen;

typedef void (*EnBenActionFunc)(struct EnBen*, PlayState*);

typedef struct EnBen {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderCylinder collider;
    /* 0x190 */ EnBenActionFunc actionFunc;
    /* 0x194 */ s16 musicCooldown; // frames until the music watchdog may restart the sequence again
    /* 0x196 */ u8 hasSpoken;
    /* 0x197 */ u8 isHidden;       // not drawn and not solid while waiting for an unseen spot to appear in
    /* 0x198 */ u8 musicWarped;    // tempo/pitch already applied to the current playback
    /* 0x19A */ s16 appearDelay;   // frames until the next attempt to appear (spaces out the spot search)
} EnBen; // size = 0x19C

typedef enum {
    /* 0 */ EN_BEN_MODE_CREEP // Moves toward the player only while off-screen
} EnBenMode;

#define EN_BEN_GET_MODE(thisx) ((thisx)->params & 0xF)

#endif // Z_EN_BEN_H
