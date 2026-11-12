#ifndef Z_EN_BEN_H
#define Z_EN_BEN_H

#include "global.h"

struct EnBen;

typedef void (*EnBenActionFunc)(struct EnBen*, PlayState*);

#define EN_BEN_PATH_POINTS 12 // how many of the places you've just walked through it remembers
#define EN_BEN_TRAIL_COPIES 5 // when it stands along your path, how many more of it there are

typedef enum EnBenSightingKind {
    /* 0 */ EN_BEN_SIGHTING_STILL, // it stands there, and is gone when you look away
    /* 1 */ EN_BEN_SIGHTING_TRAIL, // a line of them, standing where you just walked
    /* 2 */ EN_BEN_SIGHTING_CREEP  // it stays, and every time you look away it's closer
} EnBenSightingKind;

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
    /* 0x1A3 */ u8 kind;            // EnBenSightingKind of the current sighting
    /* 0x1A4 */ s16 glimpseAt;      // while lurking: the timer value at which it shows itself for an instant, or -1
    /* 0x1A6 */ s16 glimpseFrames;  // frames left of that glimpse
    /* 0x1A8 */ s16 stepTimer;      // frames until the next footstep behind you
    /* 0x1AA */ s16 pathTimer;      // frames until your position is next remembered
    /* 0x1AC */ s16 creepFails;     // frames it has had nowhere closer to go
    /* 0x1AE */ s16 stareFrames;    // frames you've looked at it from up close
    /* 0x1B0 */ s16 blurFrames;     // frames left of the smear after it's gone
    /* 0x1B2 */ s16 grey;           // how much color has drained out of the world (0-255)
    /* 0x1B4 */ u16 stoppedTime;    // the time the clock stopped at, put back when it's gone
    /* 0x1B6 */ u8 withSteps;       // this wait ends with footsteps behind you
    /* 0x1B7 */ u8 isStaring;       // it's looking right at you: no music, the heartbeat, no color
    /* 0x1B8 */ u8 trailCount;      // copies standing along your path
    /* 0x1B9 */ u8 pathHead;        // next slot in path to write
    /* 0x1BA */ u8 pathCount;       // remembered places so far
    /* 0x1BC */ Vec3f stepPos;      // where the footsteps are heard, relative to the camera
    /* 0x1C8 */ Vec3f path[EN_BEN_PATH_POINTS];
    /* 0x258 */ Vec3f trail[EN_BEN_TRAIL_COPIES];
    /* 0x294 */ s16 trailYaw[EN_BEN_TRAIL_COPIES];
    /* 0x29E */ s16 directorTimer; // scripted scenes: frames since the current step began
    /* 0x2A0 */ u8 directorStep;   // scripted scenes: which step of the scene we're on
    /* 0x2A2 */ s16 subCamId;      // scripted scenes: the camera they look through
    /* 0x2A4 */ Actor* other;      // scripted scenes: someone else in them (the Salesman)
    /* 0x2A8 */ s16 whiteFrames;   // Session 4: frames left of a whiteout
    /* 0x2AA */ s16 watchFrames;   // Session 7: frames you've watched it without it moving
    /* 0x2AC */ u8 inWhiteout;     // Session 4: it's coming in this whiteout
} EnBen;                           // size = 0x2B0

#endif // Z_EN_BEN_H
