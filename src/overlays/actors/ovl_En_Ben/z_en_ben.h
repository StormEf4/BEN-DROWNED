#ifndef Z_EN_BEN_H
#define Z_EN_BEN_H

#include "global.h"

struct EnBen;

typedef void (*EnBenActionFunc)(struct EnBen*, PlayState*);

typedef struct EnBen {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderCylinder collider;
    /* 0x190 */ EnBenActionFunc actionFunc;
    /* 0x194 */ s16 musicCooldown;  // frames until the music watchdog may act again
    /* 0x196 */ s16 timer;          // lurk delay while hidden; patience while standing unnoticed
    /* 0x198 */ s16 seenFrames;     // consecutive frames it has been in plain view
    /* 0x19A */ s16 unseenFrames;   // consecutive frames out of view since you noticed it
    /* 0x19C */ u8 isHidden;        // not drawn and not solid
    /* 0x19D */ u8 isNoticed;       // you've looked at it this sighting
    /* 0x19E */ u8 musicWarped;     // tempo/pitch already applied to the current playback
    /* 0x19F */ u8 isConfronting;   // after the Song of Time: appear in plain sight and speak
    /* 0x1A0 */ s16 drownFrames;    // frames in deep water in the Laundry Pool
    /* 0x1A2 */ u8 spawnedSkullKid; // the lair director has put Skull Kid in the air
} EnBen;                            // size = 0x1A4

#endif // Z_EN_BEN_H
