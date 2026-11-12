#ifndef BEN_BEN_H
#define BEN_BEN_H

/**
 * BEN DROWNED: shared definitions for the hack's systems ("horror kit", see docs/ben/TECH_DESIGN.md).
 * Every edit to vanilla decomp code is tagged with a `// BEN:` comment so upstream merges stay reviewable.
 */

#include "ultra64.h"

/* Feature switches. Each can be overridden per build with `./ben config OPT_<NAME> 0|1` (or the builder app), which
 * passes -DBEN_<NAME>=<value> to the compiler. */
#ifndef BEN_NAME_OVERRIDE
#define BEN_NAME_OVERRIDE 1 // K2: NPC text says "BEN" whatever the file is named
#endif
#ifndef BEN_SPIKE_STATUE
#define BEN_SPIKE_STATUE 1 // Phase 0.3: spawn the stalker statue in South Clock Town
#endif
#ifndef BEN_SKIP_INTRO
#define BEN_SKIP_INTRO 1 // Dev: new files skip the intro and start in South Clock Town (set to 0 to restore it)
#endif
#ifndef BEN_DEMO_TEASER
#define BEN_DEMO_TEASER 1 // Demo: the "day four" session (empty Clock Town, the statue, the Song of Time line)
#endif
#ifndef BEN_BOOT_SAVE
#define BEN_BOOT_SAVE 1 // Session 0: a blank cartridge already has a file called BEN on it, left at Stone Tower
#endif

/* Custom text IDs. Vanilla never reaches 0x4Dxx (credits start at 0x4E20). */
#define BEN_TEXT_TERRIBLE_FATE 0x4D00
#define BEN_TEXT_SHOULDNT_HAVE 0x4D01 // "YOU SHOULDN'T HAVE DONE THAT." after the Song of Time
#define BEN_TEXT_STONE 0x4D02         // the garbled box on BEN's file at Stone Tower
#define BEN_TEXT_SALESMAN 0x4D03      // Session 3: the Happy Mask Salesman, behind Deku Link in South Clock Town

/* En_Ben params */
#define BEN_ACTOR_STATUE 0   // the stalker statue
#define BEN_ACTOR_LAIR 1     // invisible director for the 4th-day visit to Majora's lair
#define BEN_ACTOR_BOOT 2     // BEN's own file at Stone Tower: the statue, and the game giving out
#define BEN_ACTOR_SALESMAN 3 // Session 3: forced Deku form, a strange camera, the Salesman, the cut to Termina Field
#define BEN_ACTOR_THRONE 4   // Session 3's end: the statue in the Deku King's chamber, then BEN / DROWNED

/* Which En_Ben (by params) the story puts in this scene right now, or -1 for none. Spawned by Play_Init. */
s32 Ben_GetDirector(s16 sceneId);

/* Spike audio: Song of Healing slowed and pitched down (the runtime fallback for K7) */
#define BEN_SPIKE_TEMPO_SCALE 60 // percent of normal tempo
#define BEN_SPIKE_FREQ_SCALE 750 // 1000 = normal pitch, 500 = one octave down

/* K1: the story, one 3-day cycle per session (STORY_BIBLE.md §3). Saved with the file, so it survives turning the
 * game off. */
typedef enum BenSession {
    /* 1 */ BEN_SESSION_SOMETHING_WRONG = 1, // Clock Town, then "day four" (Sessions 1 and 2)
    /* 3 */ BEN_SESSION_BEN = 3,             // forced Deku Link, the Salesman, Termina Field, the swamp
    /* 4 */ BEN_SESSION_COLD,                // Mountain Village, Snowhead, Darmani
    /* 5 */ BEN_SESSION_DROWNED,             // Great Bay, Mikau, the open sea
    /* 6 */ BEN_SESSION_STONE,               // Ikana, Stone Tower
    /* 7 */ BEN_SESSION_FREE                 // the Moon, "The counter resets."
} BenSession;

s32 Ben_GetSession(void);
void Ben_SetSession(s32 session); // also sets the step back to 0
/* Progress within the current session. Each session numbers its own steps. */
s32 Ben_GetStep(void);
void Ben_SetStep(s32 step);

/* Session 3 (BEN) steps */
#define BEN_STEP3_ARRIVE 0 // dawn in South Clock Town, as a Deku scrub, and the Salesman
#define BEN_STEP3_FIELD 1  // out in Termina Field and on to the swamp

struct PlayState;
/* Session 2 is over ("YOU SHOULDN'T HAVE DONE THAT."). Turns Link into a Deku scrub and starts Session 3. */
void Ben_StartSessionBen(struct PlayState* play);
/* Session 3 is over: the file is renamed DROWNED, Link is himself again, and the next session is Cold. */
void Ben_BecomeDrowned(void);
/* ...and saved as if the Song of Time were played, with BEN written into the other slot if it's empty (C15).
 * In z_sram_NES.c. Writes synchronously: the game hangs for a moment, which is the point. */
void Sram_BenEndSessionBen(struct PlayState* play);

/* K2: name shown wherever text uses the player-name control code. Same 8-char encoding as playerName. */
const char* Ben_GetPlayerName(void);

/* "day four" (Session 2). Clock Town is normal until you try for a fourth day by playing the Song of Double Time
 * on the Night of the Final Day. That takes you to Majora's lair, and from then on Clock Town is haunted. */
typedef enum BenDayFour {
    /* 0 */ BEN_DAY_FOUR_NOT_YET,
    /* 1 */ BEN_DAY_FOUR_LAIR,    // on the way to, or in, Majora's lair
    /* 2 */ BEN_DAY_FOUR_HAUNTED, // back in an empty Clock Town with the clock stopped
    /* 3 */ BEN_DAY_FOUR_OVER     // Session 3 has begun; Clock Town has its people back
} BenDayFour;

struct PlayState;
s32 Ben_GetDayFour(void);
void Ben_SetDayFour(s32 state);
/* Called when the Song of Double Time is refused on the final night. Returns true if it started the 4th day. */
s32 Ben_TryStartDayFour(struct PlayState* play);

/* Cryptid stalker (En_Ben): which scenes it haunts, and how many times you've noticed it. The count survives scene
 * changes, so it keeps getting closer as you move between districts. */
s32 Ben_IsHauntedScene(s16 sceneId);
s32 Ben_GetSightings(void);
void Ben_AddSighting(void);
/* The one time it stands all along the path you just walked (once per playthrough of the 4th day). */
s32 Ben_HasSeenTrail(void);
void Ben_SetTrailSeen(void);

/* The rest of the 4th day, in canon order (STORY_BIBLE.md C9, C10). You try to drown Link in the Laundry Pool to get
 * away, and the statue is standing right behind you when you come back up. You see it, and Link catches fire and dies.
 * When you're back: "You've met with a terrible fate, haven't you?" Survives scene changes and the cycle reset. */
typedef enum BenFate {
    /* 0 */ BEN_FATE_NONE,
    /* 1 */ BEN_FATE_DROWNED, // pulled under in the Laundry Pool; the next sighting is close behind you
    /* 2 */ BEN_FATE_BURNING, // you saw it after drowning, and Link is on fire
    /* 3 */ BEN_FATE_BURNED,  // died burning; the line is waiting for when you're back
    /* 4 */ BEN_FATE_DONE
} BenFate;

s32 Ben_GetFate(void);
void Ben_SetFate(s32 fate);

/* Demo teaser: the Song of Time was played this session. Survives the cycle reset (code-segment state). */
void Ben_OnSongOfTime(void);
s32 Ben_HasRewound(void);
/* The statue has delivered its "YOU SHOULDN'T HAVE DONE THAT." line since the last rewind. */
void Ben_SetWarned(void);
s32 Ben_HasWarned(void);

/* Session 0 (BOOT): the BEN file found on a blank cartridge (STORY_BIBLE.md C3). */
struct GameState;
struct SramContext;
s32 Ben_IsBootFile(void);
/* Builds BEN's save in gSaveContext: near the end of the game, every mask, Stone Tower on the final morning. */
void Ben_MakeBootSave(void);
/* Called when a normal save is loaded. BEN's file always opens at Stone Tower, an hour before the moon falls. */
void Ben_OpenBootFile(void);
/* Writes BEN's file into slot 1 if both slots are empty. Returns true if it did (z_sram_NES.c). */
s32 Sram_BenPlantBootSave(struct GameState* gameState, struct SramContext* sramCtx);
/* The game gives out: straight back to the title screen, as if the cartridge crashed. */
void Ben_CrashToTitle(struct PlayState* play);

/* Dev: turn a freshly created save into a post-intro save. Called by Sram_InitSave when BEN_SKIP_INTRO is on. */
void Ben_ApplyIntroSkip(void);

#endif
