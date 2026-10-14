#ifndef BEN_BEN_H
#define BEN_BEN_H

/**
 * BEN DROWNED: shared definitions for the hack's systems ("horror kit", see docs/ben/TECH_DESIGN.md).
 * Every edit to vanilla decomp code is tagged with a `// BEN:` comment so upstream merges stay reviewable.
 */

#include "ultra64.h"

/* Feature switches. Spike features are on so the Phase 0.3 build can be checked. */
#define BEN_NAME_OVERRIDE 1 // K2: NPC text says "BEN" whatever the file is named
#define BEN_SPIKE_STATUE 1  // Phase 0.3: spawn the stalker statue in South Clock Town
#define BEN_SKIP_INTRO 1    // Dev: new files skip the intro and start in South Clock Town (set to 0 to restore it)
#define BEN_DEMO_TEASER 1   // Demo: the "day four" session (empty Clock Town, the statue, the Song of Time line)

/* Custom text IDs. Vanilla never reaches 0x4Dxx (credits start at 0x4E20). */
#define BEN_TEXT_TERRIBLE_FATE 0x4D00
#define BEN_TEXT_SHOULDNT_HAVE 0x4D01 // "YOU SHOULDN'T HAVE DONE THAT." after the Song of Time

/* En_Ben params */
#define BEN_ACTOR_STATUE 0 // the stalker statue
#define BEN_ACTOR_LAIR 1   // invisible director for the 4th-day visit to Majora's lair

/* Spike audio: Song of Healing slowed and pitched down (the runtime fallback for K7) */
#define BEN_SPIKE_TEMPO_SCALE 60 // percent of normal tempo
#define BEN_SPIKE_FREQ_SCALE 750 // 1000 = normal pitch, 500 = one octave down

/* K2: name shown wherever text uses the player-name control code. Same 8-char encoding as playerName. */
const char* Ben_GetPlayerName(void);

/* "day four" (Session 2). Clock Town is normal until you try for a fourth day by playing the Song of Double Time
 * on the Night of the Final Day. That takes you to Majora's lair, and from then on Clock Town is haunted. */
typedef enum BenDayFour {
    /* 0 */ BEN_DAY_FOUR_NOT_YET,
    /* 1 */ BEN_DAY_FOUR_LAIR,   // on the way to, or in, Majora's lair
    /* 2 */ BEN_DAY_FOUR_HAUNTED // back in an empty Clock Town with the clock stopped
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

/* Demo teaser: the Song of Time was played this session. Survives the cycle reset (code-segment state). */
void Ben_OnSongOfTime(void);
s32 Ben_HasRewound(void);
/* The statue has delivered its "YOU SHOULDN'T HAVE DONE THAT." line since the last rewind. */
void Ben_SetWarned(void);
s32 Ben_HasWarned(void);

/* Dev: turn a freshly created save into a post-intro save. Called by Sram_InitSave when BEN_SKIP_INTRO is on. */
void Ben_ApplyIntroSkip(void);

#endif
