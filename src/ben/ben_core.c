/*
 * File: ben_core.c
 * Description: BEN DROWNED core state and helpers, linked into the code segment.
 */

#include "ben/ben.h"
#include "global.h"

// "BEN" in the file-select character encoding: 0x0A + n is the nth capital letter, 0x3E is a space.
static char sBenName[8] = { 0x0B, 0x0E, 0x17, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E };

// Demo teaser state. Lives in the code segment, so it survives the Song of Time's scene reload but not a reboot.
static u8 sHasRewound = false;
static u8 sHasWarned = false;
static u8 sSightings = 0;

s32 Ben_IsHauntedScene(s16 sceneId) {
    switch (sceneId) {
        case SCENE_CLOCKTOWER: // South Clock Town
        case SCENE_TOWN:       // East Clock Town
        case SCENE_ICHIBA:     // West Clock Town
        case SCENE_BACKTOWN:   // North Clock Town
        case SCENE_ALLEY:      // Laundry Pool
            return true;

        default:
            return false;
    }
}

s32 Ben_GetSightings(void) {
    return sSightings;
}

void Ben_AddSighting(void) {
    if (sSightings < 255) {
        sSightings++;
    }
}

void Ben_OnSongOfTime(void) {
    sHasRewound = true;
    sHasWarned = false;
}

s32 Ben_HasRewound(void) {
    return sHasRewound;
}

void Ben_SetWarned(void) {
    sHasWarned = true;
}

s32 Ben_HasWarned(void) {
    return sHasWarned;
}

const char* Ben_GetPlayerName(void) {
    if (BEN_NAME_OVERRIDE) {
        return sBenName;
    }
    return gSaveContext.save.saveInfo.playerData.playerName;
}

/**
 * Dev shortcut while the intro is being rebuilt: make a new file behave like a vanilla save made after the intro.
 * Loading a first-cycle save already sends Link out of the Clock Tower door into South Clock Town on the dawn of
 * Day 1 (see Sram_OpenSave), so this only marks the file as such, sets what the intro would have set, and hands out
 * the post-intro kit planned for Session 1 (DECISIONS D6): human Link, Ocarina, Song of Time, Song of Healing,
 * Deku Mask.
 */
void Ben_ApplyIntroSkip(void) {
    // A brand new file starts the teaser from the beginning.
    sHasRewound = false;
    sHasWarned = false;
    sSightings = 0;

    gSaveContext.save.isFirstCycle = true;
    gSaveContext.save.cutsceneIndex = 0; // no intro cutscene
    // The intro's scripted return to Dawn of the First Day counts as the first reset. Without it, the player's
    // first Song of Time replays that scripted return (the Deku Link cutscene into the Clock Tower).
    gSaveContext.save.saveInfo.playerData.threeDayResetCount = 1;
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    gSaveContext.save.hasTatl = true;

    // The same "intro is done" flags the decomp's debug save sets: Clock Tower door opened, entered South Clock
    // Town, Tatl's first-cycle hint given.
    SET_WEEKEVENTREG(WEEKEVENTREG_15_20);
    SET_WEEKEVENTREG(WEEKEVENTREG_59_04);
    SET_WEEKEVENTREG(WEEKEVENTREG_31_04);
    gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_INSIDETOWER].switch0 = 1;

    gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] = ITEM_OCARINA_OF_TIME;
    gSaveContext.save.saveInfo.inventory.items[SLOT_MASK_DEKU] = ITEM_MASK_DEKU;
    SET_QUEST_ITEM(QUEST_SONG_TIME);
    SET_QUEST_ITEM(QUEST_SONG_HEALING);
    BUTTON_ITEM_EQUIP(PLAYER_FORM_HUMAN, EQUIP_SLOT_C_LEFT) = ITEM_OCARINA_OF_TIME;
    C_SLOT_EQUIP(PLAYER_FORM_HUMAN, EQUIP_SLOT_C_LEFT) = SLOT_OCARINA;
}
