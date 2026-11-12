#ifndef Z_EN_BEN_H
#define Z_EN_BEN_H

#include "global.h"

struct EnBen;

typedef void (*EnBenActionFunc)(struct EnBen*, PlayState*);

#define EN_BEN_PATH_POINTS 12 // how many of the places you've just walked through it remembers
#define EN_BEN_TRAIL_COPIES 5 // when it stands along your path, how many more of it there are
#define EN_BEN_FEET 16        // the Song of Unhealing's wet footprints, the newest of them

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
    /* 0x2AC */ u8 inWhiteout;     // Session 4: it's coming in this whiteout (or, after the Elegy, in plain view)
    /* 0x2AD */ u8 songLatch;      // the ocarina song just played has been noticed
    /* 0x2AE */ s16 faceFrames;    // frames left of BEN's face on screen
    /* 0x2B0 */ s16 glitchFrames;  // frames left of the game seeming to break
    /* 0x2B2 */ s16 glitchAt;      // while lurking: the timer value at which it breaks, or -1
    /* 0x2B4 */ u8 songReply;      // a song it's going to answer (EN_BEN_REPLY_*)
    /* 0x2B5 */ u8 timerOwned;     // the HUD's countdown is ours (hide and seek, standing still)
    /* 0x2B6 */ s16 seekFrames;    // hide and seek: frames left to find it
    /* 0x2B8 */ s16 giggleTimer;   // hide and seek: frames until it giggles again
    /* 0x2BA */ u8 task;           // the Song of Unhealing: which sigil's task (EN_BEN_TASK_*)
    /* 0x2BB */ u8 hops;           // how far the child has led you
    /* 0x2BC */ Vec3f childPos;    // the Moon Child
    /* 0x2C8 */ Vec3f sigilPos;    // the sigil on the ground
    /* 0x2D4 */ Vec3f stillPos;    // where you were told to stand still
    /* 0x2E0 */ s16 childYaw;      // which way its body faces
    /* 0x2E2 */ s16 headYaw;       // and its head, on its own
    /* 0x2E4 */ s16 headRoll;      // its head's tilt
    /* 0x2E6 */ s16 sigilAlpha;    // 0 when there's no sigil
    /* 0x2E8 */ u8 showChild;      // the Moon Child is drawn
    /* 0x2E9 */ u8 feetHead;       // next footprint slot
    /* 0x2EA */ u8 feetCount;      // footprints left so far
    /* 0x2EB */ u8 showStatue;     // the task's own statue, drawn at trail[0]
    /* 0x2EC */ Vec3f feet[EN_BEN_FEET];
    /* 0x3AC */ s16 feetYaw[EN_BEN_FEET];
    /* 0x3CC */ s16 walkYaw;      // where the wet feet are heading
    /* 0x3CE */ u8 lastEvent;     // Termina: the last small thing that happened, so the next is something else
    /* 0x3D0 */ s16 savedSpeed;   // Termina: the clock's speed before it stopped
    /* 0x3D2 */ s16 fog;          // Termina: how far the fog has closed in (0-255)
    /* 0x3D4 */ s16 visitorTimer; // Termina (Expanded Horror): frames until someone from another story might come
    /* 0x3D6 */ u8 visitorTried;  // Termina: they've had their chance in this scene
} EnBen;                          // size = 0x3D8

#endif // Z_EN_BEN_H
