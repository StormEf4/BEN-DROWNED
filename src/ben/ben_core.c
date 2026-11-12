/*
 * File: ben_core.c
 * Description: BEN DROWNED core state and helpers, linked into the code segment.
 */

#include "ben/ben.h"
#include "global.h"
#include "overlays/gamestates/ovl_opening/z_opening.h"

// "BEN" in the file-select character encoding: 0x0A + n is the nth capital letter, 0x3E is a space.
static char sBenName[8] = { 0x0B, 0x0E, 0x17, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E };
// and "DROWNED"
static char sDrownedName[8] = { 0x0D, 0x1B, 0x18, 0x20, 0x17, 0x0E, 0x0D, 0x3E };

/**
 * K1 story state. It lives in a block of the save file that vanilla never touches (SaveInfo.unk_DF4, 0x54 bytes):
 * zeroed on a new file, kept by the Song of Time's cycle reset, and written and checksummed with every owl save and
 * cycle save. So the story survives turning the game off, the same way your items do.
 */
typedef struct BenSave {
    /* 0x00 */ u8 session;   // BenSession, 0 on a new file (the same as Session 1)
    /* 0x01 */ u8 dayFour;   // BenDayFour
    /* 0x02 */ u8 fate;      // BenFate
    /* 0x03 */ u8 sightings; // times you've noticed the statue
    /* 0x04 */ u8 flags;     // BEN_FLAG_*
    /* 0x05 */ u8 step;      // where you are within the current session (each session numbers its own steps)
} BenSave;

#define BEN_FLAG_REWOUND (1 << 0)    // the Song of Time was played on the 4th day
#define BEN_FLAG_WARNED (1 << 1)     // "YOU SHOULDN'T HAVE DONE THAT." has been said since
#define BEN_FLAG_TRAIL_SEEN (1 << 2) // the line of statues along your path has happened
#define BEN_FLAG_BOOT_FILE (1 << 3)  // this is BEN's file, the one that was already on the cartridge
// BEN_FLAG_RUMOR and BEN_FLAG_HEALING are in ben.h, for En_Ben

#define BEN_SAVE ((BenSave*)gSaveContext.save.saveInfo.unk_DF4)

s32 Ben_GetFlag(u8 flag) {
    return (BEN_SAVE->flags & flag) != 0;
}

void Ben_SetFlag(u8 flag, s32 on) {
    if (on) {
        BEN_SAVE->flags |= flag;
    } else {
        BEN_SAVE->flags &= ~flag;
    }
}

s32 Ben_GetSession(void) {
    return (BEN_SAVE->session < BEN_SESSION_SOMETHING_WRONG) ? BEN_SESSION_SOMETHING_WRONG : BEN_SAVE->session;
}

void Ben_SetSession(s32 session) {
    BEN_SAVE->session = session;
    BEN_SAVE->step = 0;
    // Each session starts its stalking over: far away again, and one more line of statues to come.
    BEN_SAVE->sightings = 0;
    Ben_SetFlag(BEN_FLAG_TRAIL_SEEN, false);
    Ben_SetFlag(BEN_FLAG_HEALING, false); // it can say that once a session
}

s32 Ben_GetStep(void) {
    return BEN_SAVE->step;
}

void Ben_SetStep(s32 step) {
    BEN_SAVE->step = step;
}

s32 Ben_GetDayFour(void) {
    return BEN_SAVE->dayFour;
}

void Ben_SetDayFour(s32 state) {
    BEN_SAVE->dayFour = state;
}

s32 Ben_GetFate(void) {
    return BEN_SAVE->fate;
}

void Ben_SetFate(s32 fate) {
    BEN_SAVE->fate = fate;
}

s32 Ben_TryStartDayFour(PlayState* play) {
    if (!BEN_DEMO_TEASER || (Ben_GetSession() != BEN_SESSION_SOMETHING_WRONG) ||
        (Ben_GetDayFour() != BEN_DAY_FOUR_NOT_YET)) {
        return false;
    }
    Ben_SetDayFour(BEN_DAY_FOUR_LAIR);

    // No "you can't" message. The ocarina just closes and everything slowly goes black.
    Message_CloseTextbox(play);
    play->msgCtx.ocarinaMode = OCARINA_MODE_END;
    gSaveContext.timerStates[TIMER_ID_MOON_CRASH] = TIMER_STATE_OFF;
    play->nextEntrance = ENTRANCE(MAJORAS_LAIR, 0);
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_BLACK_SLOW;
    return true;
}

s32 Ben_GetDirector(s16 sceneId) {
    if (BEN_BOOT_SAVE && Ben_IsBootFile()) {
        return (sceneId == SCENE_F40) ? BEN_ACTOR_BOOT : -1;
    }
    if (Ben_IsHauntedScene(sceneId)) {
        return BEN_ACTOR_STATUE;
    }
    if ((sceneId == SCENE_LAST_BS) && (Ben_GetDayFour() == BEN_DAY_FOUR_LAIR)) {
        return BEN_ACTOR_LAIR;
    }

    switch (Ben_GetSession()) {
        case BEN_SESSION_SOMETHING_WRONG:
            // Session 1: once you have the ocarina back, Clock Town is a little wrong.
            if ((Ben_GetDayFour() != BEN_DAY_FOUR_NOT_YET) ||
                (INV_CONTENT(ITEM_OCARINA_OF_TIME) != ITEM_OCARINA_OF_TIME)) {
                return -1;
            }
            switch (sceneId) {
                case SCENE_CLOCKTOWER:
                case SCENE_TOWN:
                case SCENE_ICHIBA:
                case SCENE_BACKTOWN:
                    return BEN_ACTOR_WRONG;

                default:
                    return -1;
            }

        case BEN_SESSION_BEN:
            switch (sceneId) {
                case SCENE_CLOCKTOWER:
                    return (Ben_GetStep() == BEN_STEP3_ARRIVE) ? BEN_ACTOR_SALESMAN : -1;

                case SCENE_INSIDETOWER: // Clock Tower: he says it again (STORY_BIBLE.md C13)
                    return BEN_ACTOR_TOWER;

                case SCENE_00KEIKOKU:    // Termina Field
                case SCENE_24KEMONOMITI: // Road to Southern Swamp
                case SCENE_20SICHITAI:   // Southern Swamp
                case SCENE_22DEKUCITY:   // Deku Palace
                    return BEN_ACTOR_STATUE;

                case SCENE_DEKU_KING:
                    return BEN_ACTOR_FINALE;

                default:
                    return -1;
            }

        case BEN_SESSION_COLD:
            switch (sceneId) {
                case SCENE_CLOCKTOWER:
                    return BEN_ACTOR_WARP;

                case SCENE_10YUKIYAMANOMURA: // Mountain Village, in winter
                case SCENE_13HUBUKINOMITI:   // Path to Goron Village
                case SCENE_14YUKIDAMANOMITI: // Path to Snowhead
                case SCENE_12HAKUGINMAE:     // Snowhead
                case SCENE_11GORONNOSATO:    // Goron Village
                    return BEN_ACTOR_STATUE;

                case SCENE_GORON_HAKA: // Goron Graveyard, where Darmani is buried
                    return BEN_ACTOR_FINALE;

                default:
                    return -1;
            }

        case BEN_SESSION_DROWNED:
            if (Ben_GetStep() == BEN_STEP5_MOON) {
                return (sceneId == SCENE_SOUGEN) ? BEN_ACTOR_MOON
                                                 : ((sceneId == SCENE_CLOCKTOWER) ? BEN_ACTOR_WARP : -1);
            }
            switch (sceneId) {
                case SCENE_CLOCKTOWER:
                    return BEN_ACTOR_WARP;

                case SCENE_30GYOSON:   // Great Bay Coast
                case SCENE_31MISAKI:   // Zora Cape
                case SCENE_33ZORACITY: // Zora Hall
                    return BEN_ACTOR_STATUE;

                default:
                    return -1;
            }

        case BEN_SESSION_STONE:
            switch (sceneId) {
                case SCENE_CLOCKTOWER:
                    return BEN_ACTOR_WARP;

                case SCENE_F40:   // Stone Tower
                case SCENE_F41:   // Stone Tower, upside down
                case SCENE_IKANA: // Ikana Canyon
                case SCENE_BOTI:  // Ikana Graveyard
                    return BEN_ACTOR_STATUE;

                default:
                    return -1;
            }

        case BEN_SESSION_FREE:
            switch (Ben_GetStep()) {
                case BEN_STEP7_NIGHT:
                    switch (sceneId) {
                        case SCENE_CLOCKTOWER:
                        case SCENE_TOWN:
                        case SCENE_ICHIBA:
                        case SCENE_BACKTOWN:
                        case SCENE_ALLEY:
                            return BEN_ACTOR_STATUE;

                        case SCENE_OKUJOU: // the top of the Clock Tower
                            return BEN_ACTOR_FINALE;

                        default:
                            return -1;
                    }

                case BEN_STEP7_LAIR:
                    return (sceneId == SCENE_LAST_BS) ? BEN_ACTOR_FINALE : -1;

                default:
                    return -1;
            }

        default:
            return -1;
    }
}

void Ben_AdvanceSession(PlayState* play, s32 session) {
    Ben_SetSession(session);
    Sram_BenCommitStory(play);
}

void Ben_WarpToSession(PlayState* play) {
    switch (Ben_GetSession()) {
        case BEN_SESSION_COLD:
            play->nextEntrance = ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 8); // by the owl statue
            break;

        case BEN_SESSION_DROWNED:
            play->nextEntrance =
                (Ben_GetStep() == BEN_STEP5_MOON) ? ENTRANCE(THE_MOON, 0) : ENTRANCE(GREAT_BAY_COAST, 11);
            break;

        case BEN_SESSION_STONE:
            play->nextEntrance = ENTRANCE(STONE_TOWER, 3);
            break;

        default:
            return;
    }
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE;
}

void Ben_BecomeBen(void) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = sBenName[i];
    }
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    Ben_SetSession(BEN_SESSION_FREE);
    Ben_SetStep(BEN_STEP7_DONE);
    Ben_SetFlag(BEN_FLAG_BOOT_FILE, true); // from now on it opens at Stone Tower, like his did
}

s32 Ben_IsBenName(const char* name) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sBenName); i++) {
        if (name[i] != sBenName[i]) {
            return false;
        }
    }
    return true;
}

/**
 * Tatl's hint for where the story is (her C-up, the way she nags in the first cycle). 0 for none, which leaves her
 * vanilla hints alone. Every line points the way without saying it outright.
 */
u16 Ben_GetHintTextId(PlayState* play) {
    if (BEN_BOOT_SAVE && Ben_IsBootFile()) {
        return BEN_TEXT_HINT_BOOT;
    }
    switch (Ben_GetSession()) {
        case BEN_SESSION_SOMETHING_WRONG:
            switch (Ben_GetDayFour()) {
                case BEN_DAY_FOUR_NOT_YET:
                    // Only once the first cycle's done; until then she has her own job to do.
                    return (INV_CONTENT(ITEM_OCARINA_OF_TIME) == ITEM_OCARINA_OF_TIME) ? BEN_TEXT_HINT_DOUBLE_TIME : 0;

                case BEN_DAY_FOUR_HAUNTED:
                    switch (Ben_GetFate()) {
                        case BEN_FATE_NONE:
                            return BEN_TEXT_HINT_WATER;

                        case BEN_FATE_DROWNED:
                            return BEN_TEXT_HINT_DONT_LOOK;

                        default:
                            return Ben_HasRewound() ? 0 : BEN_TEXT_HINT_GO_BACK;
                    }

                default:
                    return 0;
            }

        case BEN_SESSION_BEN:
            return (Ben_GetStep() == BEN_STEP3_FIELD) ? BEN_TEXT_HINT_SWAMP : 0;

        case BEN_SESSION_COLD:
            return BEN_TEXT_HINT_GRAVE;

        case BEN_SESSION_DROWNED:
            return (Ben_GetStep() == BEN_STEP5_SEA) ? BEN_TEXT_HINT_SEA : 0;

        case BEN_SESSION_STONE:
            return BEN_TEXT_HINT_STONE;

        case BEN_SESSION_FREE:
            return (Ben_GetStep() == BEN_STEP7_NIGHT) ? BEN_TEXT_HINT_TOWER : 0;

        default:
            return 0;
    }
}

static s32 sBenInputMode = BEN_INPUT_NORMAL;

void Ben_SetInputMode(s32 mode) {
    sBenInputMode = mode;
}

void Ben_FilterInput(Input* input) {
    switch (sBenInputMode) {
        case BEN_INPUT_SLOW:
            input->rel.stick_x /= 4;
            input->rel.stick_y /= 4;
            break;

        case BEN_INPUT_INVERTED:
            input->rel.stick_x = -input->rel.stick_x;
            input->rel.stick_y = -input->rel.stick_y;
            break;

        case BEN_INPUT_REFUSED:
            input->rel.stick_x = 0;
            input->rel.stick_y = 0;
            input->press.button &= BTN_START;
            break;

        default:
            break;
    }
}

void Ben_StartSessionBen(PlayState* play) {
    Ben_SetSession(BEN_SESSION_BEN);
    Ben_SetDayFour(BEN_DAY_FOUR_OVER);
    Ben_SetFate(BEN_FATE_NONE);

    // It turns you into him (STORY_BIBLE.md C12). A flash, and you're a Deku scrub at dawn.
    gSaveContext.save.playerForm = PLAYER_FORM_DEKU;
    play->nextEntrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE_FAST;
}

void Ben_BecomeDrowned(void) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = sDrownedName[i];
    }
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    Ben_SetSession(BEN_SESSION_COLD);
}

s32 Ben_IsHauntedScene(s16 sceneId) {
    if (Ben_GetDayFour() != BEN_DAY_FOUR_HAUNTED) {
        return false; // until the 4th day, Clock Town is just Clock Town
    }
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
    return BEN_SAVE->sightings;
}

void Ben_AddSighting(void) {
    if (BEN_SAVE->sightings < 255) {
        BEN_SAVE->sightings++;
    }
}

s32 Ben_HasSeenTrail(void) {
    return Ben_GetFlag(BEN_FLAG_TRAIL_SEEN);
}

void Ben_SetTrailSeen(void) {
    Ben_SetFlag(BEN_FLAG_TRAIL_SEEN, true);
}

void Ben_OnSongOfTime(void) {
    if (Ben_GetDayFour() != BEN_DAY_FOUR_HAUNTED) {
        return; // an ordinary reset
    }
    Ben_SetFlag(BEN_FLAG_REWOUND, true);
    Ben_SetFlag(BEN_FLAG_WARNED, false);
}

s32 Ben_HasRewound(void) {
    return Ben_GetFlag(BEN_FLAG_REWOUND);
}

void Ben_SetWarned(void) {
    Ben_SetFlag(BEN_FLAG_WARNED, true);
}

s32 Ben_HasWarned(void) {
    return Ben_GetFlag(BEN_FLAG_WARNED);
}

#define BEN_BOOT_DAY 3
#define BEN_BOOT_TIME CLOCK_TIME(5, 0) // an hour before the moon falls

s32 Ben_IsBootFile(void) {
    return Ben_GetFlag(BEN_FLAG_BOOT_FILE);
}

void Ben_MakeBootSave(void) {
    s32 i;

    // Someone got almost to the end: every mask, a full set of hearts.
    Sram_InitDebugSave();
    for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = sBenName[i];
    }
    gSaveContext.save.saveInfo.playerData.newf[0] = 'Z';
    gSaveContext.save.saveInfo.playerData.newf[1] = 'E';
    gSaveContext.save.saveInfo.playerData.newf[2] = 'L';
    gSaveContext.save.saveInfo.playerData.newf[3] = 'D';
    gSaveContext.save.saveInfo.playerData.newf[4] = 'A';
    gSaveContext.save.saveInfo.playerData.newf[5] = '3';
    gSaveContext.save.saveInfo.playerData.threeDayResetCount = 23;
    gSaveContext.save.isFirstCycle = false;
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    gSaveContext.save.day = BEN_BOOT_DAY;
    gSaveContext.save.time = BEN_BOOT_TIME;
    gSaveContext.save.isNight = true;
    Ben_SetFlag(BEN_FLAG_BOOT_FILE, true);
}

void Ben_OpenBootFile(void) {
    if (!BEN_BOOT_SAVE || !Ben_IsBootFile()) {
        return;
    }
    // Wherever you left it, it's back where he left it.
    gSaveContext.save.entrance = ENTRANCE(STONE_TOWER, 3);
    gSaveContext.nextCutsceneIndex = 0xFFEF;
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    gSaveContext.save.day = BEN_BOOT_DAY;
    gSaveContext.save.time = BEN_BOOT_TIME;
    gSaveContext.save.isNight = true;
}

void Ben_CrashToTitle(PlayState* play) {
    // En_Ben's Destroy puts the screen and the controller back when the scene is torn down.
    STOP_GAMESTATE(&play->state);
    SET_NEXT_GAMESTATE(&play->state, TitleSetup_Init, sizeof(TitleSetupState));
    Audio_MuteAllSeqExceptSystemAndOcarina(0);
    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
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
    // A brand new file starts the story from the beginning (Sram_ResetSave has already zeroed the block).
    bzero(BEN_SAVE, sizeof(BenSave));

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
