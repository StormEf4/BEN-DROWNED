#ifndef BEN_BEN_H
#define BEN_BEN_H

/**
 * BEN DROWNED: shared definitions for the hack's systems ("horror kit", see docs/ben/TECH_DESIGN.md).
 * Every edit to vanilla decomp code is tagged with a `// BEN:` comment so upstream merges stay reviewable.
 */

#include "ultra64.h"

/* Feature switches. Spike features are on so the Phase 0.3 build can be checked. */
#define BEN_NAME_OVERRIDE 1   // K2: NPC text says "BEN" whatever the file is named
#define BEN_SPIKE_STATUE 1    // Phase 0.3: spawn the stalker statue in South Clock Town

/* Custom text IDs. Vanilla never reaches 0x4Dxx (credits start at 0x4E20). */
#define BEN_TEXT_TERRIBLE_FATE 0x4D00

/* Spike audio: Song of Healing slowed and pitched down (the runtime fallback for K7) */
#define BEN_SPIKE_TEMPO_SCALE 60 // percent of normal tempo
#define BEN_SPIKE_FREQ_SCALE 750 // 1000 = normal pitch, 500 = one octave down

/* K2: name shown wherever text uses the player-name control code. Same 8-char encoding as playerName. */
const char* Ben_GetPlayerName(void);

#endif
