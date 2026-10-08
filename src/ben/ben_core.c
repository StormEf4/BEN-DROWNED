/*
 * File: ben_core.c
 * Description: BEN DROWNED core state and helpers, linked into the code segment.
 */

#include "ben/ben.h"
#include "z64save.h"

// "BEN" in the file-select character encoding: 0x0A + n is the nth capital letter, 0x3E is a space.
static char sBenName[8] = { 0x0B, 0x0E, 0x17, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E };

const char* Ben_GetPlayerName(void) {
    if (BEN_NAME_OVERRIDE) {
        return sBenName;
    }
    return gSaveContext.save.saveInfo.playerData.playerName;
}
