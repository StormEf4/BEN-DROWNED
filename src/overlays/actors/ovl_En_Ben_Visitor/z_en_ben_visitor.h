#ifndef Z_EN_BEN_VISITOR_H
#define Z_EN_BEN_VISITOR_H

#include "global.h"

struct EnBenVisitor;

typedef void (*EnBenVisitorActionFunc)(struct EnBenVisitor*, PlayState*);

typedef struct EnBenVisitor {
    /* 0x000 */ Actor actor;
    /* 0x144 */ EnBenVisitorActionFunc actionFunc;
    /* 0x148 */ s16 timer;        // frames in the current step
    /* 0x14A */ s16 life;         // frames since it came
    /* 0x14C */ s16 seenFrames;   // frames you've looked at it, this time
    /* 0x14E */ s16 unseenFrames; // frames it's been out of your sight
    /* 0x150 */ s16 noise;        // the Tall Man: how much static (0-255)
    /* 0x152 */ s16 dark;         // Go to sleep: how far the lights are out (0-255)
    /* 0x154 */ s16 phase;        // Go to sleep: frames left of this dark or light
    /* 0x156 */ s16 stuck;    // the Smiling Man: frames with nowhere to step; Go to sleep: times the lights went out
    /* 0x158 */ s16 headRoll; // its head's tilt
    /* 0x15A */ u8 isHidden;  // not drawn
    /* 0x15B */ u8 isNoticed; // you've seen it at least once
    /* 0x15C */ u8 holding;   // Link is held still
    /* 0x15D */ u8 frowning;  // the Smiling Man has given up on you; Go to sleep: he's gone, the lights coming back
    /* 0x15E */ u8 isDark;    // Go to sleep: the lights are out (or going out)
    /* 0x15F */ u8 placed;    // it's somewhere (placed once the camera's set up)
    /* 0x160 */ f32 dist;     // the Tall Man: how far away he stands next
} EnBenVisitor;               // size = 0x164

#endif // Z_EN_BEN_VISITOR_H
