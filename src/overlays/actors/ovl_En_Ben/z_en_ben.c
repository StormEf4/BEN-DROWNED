/*
 * File: z_en_ben.c
 * Overlay: ovl_En_Ben
 * Description: BEN DROWNED - the Elegy of Emptiness statue (K4 stalker), as a cryptid rather than a follower.
 *
 * Behavior (STORY_BIBLE.md §4):
 *  - Clock Town and the Laundry Pool are empty and silent. The statue is hidden most of the time.
 *  - Now and then it stands somewhere you aren't looking: far down a street, around a corner, behind you. It never
 *    moves while you can see it. Once you've looked at it and look away, it's gone.
 *  - Every sighting puts the next one a little closer, across districts (Ben_GetSightings).
 *  - The first sighting wakes the music: a slowed, pitched-down Song of Healing. Until then there is none.
 *  - None of this starts until the 4th day: playing the Song of Double Time on the Night of the Final Day takes you to
 *    Majora's lair instead (BEN_ACTOR_LAIR), where nothing is waiting, and then back to Clock Town with the clock
 *    stopped.
 *  - It almost never speaks. The one line it has is after the Song of Time, when it breaks its rule and is standing
 *    right in front of you at dawn: "YOU SHOULDN'T HAVE DONE THAT."
 */

#include "z_en_ben.h"
#include "ben/ben.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_LOCK_ON_DISABLED)

// Game logic runs at 20 frames per second.
#define EN_BEN_FIRST_LURK_MIN 140    // after entering an area, the earliest it can first appear (7 s)
#define EN_BEN_FIRST_LURK_RANGE 160  // plus up to this much more
#define EN_BEN_LURK_MIN 260          // between sightings: silence and nothing (13 s at the closest)
#define EN_BEN_LURK_RANGE 240        // plus up to this much more
#define EN_BEN_LURK_SHRINK 40        // each sighting so far takes this off the wait
#define EN_BEN_LURK_LONGEST 700      // early waits are this long at most (35 s), before the random part
#define EN_BEN_PATIENCE 400          // if you never look at it for this long (20 s), it goes somewhere else
#define EN_BEN_NOTICE_FRAMES 8       // in plain view this long counts as you having seen it
#define EN_BEN_GONE_FRAMES 3         // out of view this long after you saw it, and it's gone
#define EN_BEN_TOO_CLOSE 220.0f      // walk up to it and it isn't there any more
#define EN_BEN_RETRY_FRAMES 8        // a failed spot search is retried this often, not every frame
#define EN_BEN_MUSIC_RETRY_FRAMES 60 // how long to let a requested sequence load before requesting it again
#define EN_BEN_LAUGH_EVERY 3         // the Salesman's laugh answers every third time it vanishes

#define EN_BEN_CONFRONT_DIST 120.0f // after the Song of Time: how far in front of you it stands
#define EN_BEN_CONFRONT_DELAY 90    // frames to wait after arriving, so the day title has gone
#define EN_BEN_CONFRONT_RANGE 220.0f

#define EN_BEN_LAIR_FRAMES 200                 // how long you stand alone in Majora's lair before it all fades (10 s)
#define EN_BEN_DAY_FOUR_TIME CLOCK_TIME(23, 0) // the clock stops here, an hour before the Clock Tower would open

#define EN_BEN_REWOUND_TEMPO_SCALE 45
#define EN_BEN_REWOUND_FREQ_SCALE 650

void EnBen_Init(Actor* thisx, PlayState* play);
void EnBen_Destroy(Actor* thisx, PlayState* play);
void EnBen_Update(Actor* thisx, PlayState* play);
void EnBen_Draw(Actor* thisx, PlayState* play);

void EnBen_Lurk(EnBen* this, PlayState* play);
void EnBen_Stand(EnBen* this, PlayState* play);
void EnBen_Confront(EnBen* this, PlayState* play);
void EnBen_Speaking(EnBen* this, PlayState* play);
void EnBen_LairWait(EnBen* this, PlayState* play);

ActorProfile En_Ben_Profile = {
    /**/ ACTOR_EN_BEN,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBen),
    /**/ EnBen_Init,
    /**/ EnBen_Destroy,
    /**/ EnBen_Update,
    /**/ EnBen_Draw,
};

// Solid to the player, but cannot be hit or hookshotted.
static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_METAL,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK2,
        { 0x00000000, 0, 0 },
        { 0x00000000, 0, 0 },
        ATELEM_NONE,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 20, 60, 0, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_S8(colChkInfo.mass, MASS_IMMOVABLE, ICHAIN_STOP),
};

// Heights (feet, chest, head) checked against the camera: if any is in plain view, the statue counts as seen.
static f32 sBodyCheckHeights[] = { 10.0f, 40.0f, 75.0f };

// How far away each sighting is, first to last. After the last one it stays that close.
static f32 sSightingDistances[] = { 1100.0f, 950.0f, 800.0f, 650.0f, 520.0f, 400.0f, 300.0f };

// Fallbacks when nothing fits at the planned distance (narrow alleys, the Laundry Pool).
static f32 sDistanceScales[] = { 1.0f, 0.8f, 0.6f, 0.45f };

#define EN_BEN_SPOT_DIRECTIONS 16

/**
 * True if a world point is inside the view frustum and in front of the camera.
 * (Checking x/y alone can be fooled by points behind the camera.)
 */
s32 EnBen_IsPointInFrustum(PlayState* play, Vec3f* point) {
    Vec3f projectedPos;
    f32 invW;

    Actor_GetProjectedPos(play, point, &projectedPos, &invW);

    if (projectedPos.z <= 0.0f) {
        return false;
    }
    return (projectedPos.x * invW >= -1.0f) && (projectedPos.x * invW <= 1.0f) && (projectedPos.y * invW >= -1.0f) &&
           (projectedPos.y * invW <= 1.0f);
}

/**
 * True if the camera has a clear line to a world point (no wall, floor or ceiling in between).
 */
s32 EnBen_IsPointInSight(PlayState* play, Vec3f* point) {
    Camera* camera = GET_ACTIVE_CAM(play);
    Vec3f hitPos;
    CollisionPoly* poly;
    s32 bgId;

    return !BgCheck_EntityLineTest1(&play->colCtx, &camera->eye, point, &hitPos, &poly, true, true, true, true, &bgId);
}

/**
 * True if any part of a statue standing at `base` would be in plain view: on screen and not behind a building.
 */
s32 EnBen_IsBodyVisible(PlayState* play, Vec3f* base) {
    Vec3f point;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sBodyCheckHeights); i++) {
        point.x = base->x;
        point.y = base->y + sBodyCheckHeights[i];
        point.z = base->z;
        if (EnBen_IsPointInFrustum(play, &point) && EnBen_IsPointInSight(play, &point)) {
            return true;
        }
    }
    return false;
}

/**
 * Find a spot `dist` units from the player along `yaw` that has floor near the player's height and a clear
 * line back to the player (so the statue never ends up inside a wall or over a pit).
 */
s32 EnBen_FindSpot(PlayState* play, Player* player, s16 yaw, f32 dist, Vec3f* outPos) {
    Vec3f eye;
    Vec3f target;
    Vec3f hitPos;
    CollisionPoly* poly;
    s32 bgId;
    f32 floorY;

    eye.x = player->actor.world.pos.x;
    eye.y = player->actor.world.pos.y + 40.0f;
    eye.z = player->actor.world.pos.z;
    target.x = eye.x + (Math_SinS(yaw) * dist);
    target.y = eye.y;
    target.z = eye.z + (Math_CosS(yaw) * dist);

    if (BgCheck_EntityLineTest1(&play->colCtx, &eye, &target, &hitPos, &poly, true, false, false, true, &bgId)) {
        return false; // a wall is in the way
    }

    target.y = player->actor.world.pos.y + 100.0f;
    floorY = BgCheck_EntityRaycastFloor3(&play->colCtx, &poly, &bgId, &target);
    if ((floorY == BGCHECK_Y_MIN) || (fabsf(floorY - player->actor.world.pos.y) > 150.0f)) {
        return false; // no floor, or a big drop or climb
    }

    outPos->x = target.x;
    outPos->y = floorY;
    outPos->z = target.z;
    return true;
}

void EnBen_PlaceAt(EnBen* this, PlayState* play, Vec3f* pos) {
    Player* player = GET_PLAYER(play);

    this->actor.world.pos = *pos;
    this->actor.prevPos = *pos;
    this->actor.velocity.y = 0.0f;
    // Facing you, decided once. It never turns while you can see it.
    this->actor.shape.rot.y = Math_Vec3f_Yaw(pos, &player->actor.world.pos);
    this->actor.world.rot.y = this->actor.shape.rot.y;
}

/**
 * Put the statue somewhere out of plain view at about the distance this sighting calls for. The search starts on
 * the far side of the camera (behind you) and fans out, so the usual way to find it is to turn around.
 * Returns false (and leaves the statue hidden) if no such spot exists right now.
 */
s32 EnBen_TryPlaceSighting(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 behindYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
    s32 sighting = CLAMP_MAX(Ben_GetSightings(), ARRAY_COUNT(sSightingDistances) - 1);
    Vec3f pos;
    s32 d;
    s32 i;

    for (d = 0; d < ARRAY_COUNT(sDistanceScales); d++) {
        f32 dist = sSightingDistances[sighting] * sDistanceScales[d];

        for (i = 0; i < EN_BEN_SPOT_DIRECTIONS; i++) {
            // 0, +1, -1, +2, -2, ... steps of 1/16 turn away from "behind you"
            s16 step = (i + 1) / 2;
            s16 yaw = behindYaw + (((i % 2) != 0) ? step : -step) * (0x10000 / EN_BEN_SPOT_DIRECTIONS);

            if (EnBen_FindSpot(play, player, yaw, dist, &pos) && !EnBen_IsBodyVisible(play, &pos)) {
                EnBen_PlaceAt(this, play, &pos);
                return true;
            }
        }
    }
    return false;
}

/**
 * After the Song of Time: stand in plain view in front of the player. Tries a little nearer or further if the
 * exact spot has no floor. Returns false if nothing in front of the player works right now.
 */
s32 EnBen_TryPlaceInFront(EnBen* this, PlayState* play) {
    static f32 sScales[] = { 1.0f, 0.75f, 1.4f };
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sScales); i++) {
        if (EnBen_FindSpot(play, player, player->actor.shape.rot.y, EN_BEN_CONFRONT_DIST * sScales[i], &pos)) {
            EnBen_PlaceAt(this, play, &pos);
            return true;
        }
    }
    return false;
}

/**
 * Nobody else is here. Runs every frame, since people can spawn after the statue does.
 */
void EnBen_EmptyTown(PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_NPC].first;

    while (actor != NULL) {
        Actor* next = actor->next;

        Actor_Kill(actor);
        actor = next;
    }
}

/**
 * Go away for a while. The wait gets shorter the more often you've seen it.
 */
void EnBen_Vanish(EnBen* this) {
    s32 wait = EN_BEN_LURK_LONGEST - (Ben_GetSightings() * EN_BEN_LURK_SHRINK);

    this->isHidden = true;
    this->timer = CLAMP_MIN(wait, EN_BEN_LURK_MIN) + (s32)Rand_ZeroFloat(EN_BEN_LURK_RANGE);
    this->actionFunc = EnBen_Lurk;
}

void EnBen_Init(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    if (this->actor.params == BEN_ACTOR_LAIR) {
        // Nothing to see and nothing to hear. Just wait.
        play->sceneSequences.seqId = NA_BGM_NO_MUSIC;
        this->isHidden = true;
        this->timer = EN_BEN_LAIR_FRAMES;
        this->actionFunc = EnBen_LairWait;
        return;
    }

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.textId = BEN_TEXT_SHOULDNT_HAVE;
    this->actor.gravity = -1.0f;
    this->musicCooldown = 0;
    this->musicWarped = false;
    this->isConfronting = BEN_DEMO_TEASER && Ben_HasRewound() && !Ben_HasWarned();

    // Spawned during Play_Init, before the scene's music starts. Until you've seen it, there is no music at all.
    play->sceneSequences.seqId = (Ben_GetSightings() > 0) ? NA_BGM_SONG_OF_HEALING : NA_BGM_NO_MUSIC;

    // The camera isn't set up yet during Play_Init, so nothing is placed until updates start.
    this->isHidden = true;
    if (this->isConfronting) {
        this->timer = EN_BEN_CONFRONT_DELAY;
        this->actionFunc = EnBen_Confront;
    } else {
        this->timer = EN_BEN_FIRST_LURK_MIN + (s32)Rand_ZeroFloat(EN_BEN_FIRST_LURK_RANGE);
        this->actionFunc = EnBen_Lurk;
    }
}

void EnBen_Destroy(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    if (this->actor.params == BEN_ACTOR_LAIR) {
        return;
    }

    Collider_DestroyCylinder(play, &this->collider);
    gSaveContext.save.timeSpeedOffset = 0; // outside Clock Town the clock runs again

    // Leaving the scene: undo the slowdown and make the next scene start its own music, even if it uses the same
    // sequence the game last requested before we took over (e.g. the Clock Town theme in the next district).
    SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, 100);
    SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, 1000);
    Audio_ResetRequestedSceneSeqId();
}

/**
 * Silence until the first sighting. After it, the Song of Healing, slowed and pitched down (more so after the Song
 * of Time), whatever else tries to start music.
 */
void EnBen_UpdateMusic(EnBen* this, PlayState* play) {
    u16 activeSeqId = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);

    if (this->musicCooldown > 0) {
        this->musicCooldown--;
    }

    if (Ben_GetSightings() == 0) {
        if ((activeSeqId != NA_BGM_DISABLED) && (this->musicCooldown == 0)) {
            SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 20);
            this->musicCooldown = EN_BEN_MUSIC_RETRY_FRAMES;
        }
        return;
    }

    if (activeSeqId != NA_BGM_SONG_OF_HEALING) {
        this->musicWarped = false;
        if (this->musicCooldown == 0) {
            SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, NA_BGM_SONG_OF_HEALING);
            this->musicCooldown = EN_BEN_MUSIC_RETRY_FRAMES;
        }
    } else if (!this->musicWarped) {
        if (BEN_DEMO_TEASER && Ben_HasRewound()) {
            SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, EN_BEN_REWOUND_TEMPO_SCALE);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, EN_BEN_REWOUND_FREQ_SCALE);
        } else {
            SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, BEN_SPIKE_TEMPO_SCALE);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, BEN_SPIKE_FREQ_SCALE);
        }
        this->musicWarped = true;
    }
}

/**
 * Hidden. Wait, then appear somewhere you aren't looking.
 */
void EnBen_Lurk(EnBen* this, PlayState* play) {
    if (this->timer > 0) {
        this->timer--;
        return;
    }
    if (EnBen_TryPlaceSighting(this, play)) {
        this->isHidden = false;
        this->isNoticed = false;
        this->seenFrames = 0;
        this->unseenFrames = 0;
        this->timer = EN_BEN_PATIENCE;
        this->actionFunc = EnBen_Stand;
    } else {
        this->timer = EN_BEN_RETRY_FRAMES;
    }
}

/**
 * Standing still somewhere, facing you. Once you've seen it, it's gone the moment you look away.
 */
void EnBen_Stand(EnBen* this, PlayState* play) {
    s32 visible = EnBen_IsBodyVisible(play, &this->actor.world.pos);

    if (!this->isNoticed) {
        if (visible) {
            this->seenFrames++;
            if (this->seenFrames >= EN_BEN_NOTICE_FRAMES) {
                this->isNoticed = true;
                Ben_AddSighting();
                if (Ben_GetSightings() == 1) {
                    play->sceneSequences.seqId = NA_BGM_SONG_OF_HEALING; // the music wakes up
                }
            }
        } else {
            this->seenFrames = 0;
            this->timer--;
            if (this->timer <= 0) {
                // You never turned around. Try somewhere else later; this one doesn't count.
                EnBen_Vanish(this);
            }
        }
        return;
    }

    if (!visible) {
        this->unseenFrames++;
    } else {
        this->unseenFrames = 0;
    }

    if ((this->unseenFrames >= EN_BEN_GONE_FRAMES) || (this->actor.xzDistToPlayer < EN_BEN_TOO_CLOSE)) {
        if ((Ben_GetSightings() % EN_BEN_LAUGH_EVERY) == 0) {
            Audio_PlaySfx(NA_SE_VO_OMVO00); // the Happy Mask Salesman, somewhere
        }
        EnBen_Vanish(this);
    }
}

/**
 * After the Song of Time, once per rewind: it breaks its rule and stands right in front of you, and speaks.
 */
void EnBen_Confront(EnBen* this, PlayState* play) {
    if (this->isHidden) {
        if (this->timer > 0) {
            this->timer--;
            return;
        }
        if (EnBen_TryPlaceInFront(this, play)) {
            this->isHidden = false;
        } else {
            this->timer = EN_BEN_RETRY_FRAMES;
        }
        return;
    }

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = EnBen_Speaking;
    } else {
        this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        Actor_OfferTalk(&this->actor, play, EN_BEN_CONFRONT_RANGE);
    }
}

void EnBen_Speaking(EnBen* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        this->isConfronting = false;
        Ben_SetWarned();
        EnBen_Vanish(this);
    }
}

/**
 * Kill everything in Majora's lair that would fight or talk, so the room is empty.
 */
void EnBen_EmptyLair(PlayState* play) {
    static u8 sCategories[] = { ACTORCAT_BOSS, ACTORCAT_ENEMY, ACTORCAT_NPC };
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sCategories); i++) {
        Actor* actor = play->actorCtx.actorLists[sCategories[i]].first;

        while (actor != NULL) {
            Actor* next = actor->next;

            Actor_Kill(actor);
            actor = next;
        }
    }
}

/**
 * The 4th day starts here: Majora's lair, empty and silent. Then black, then Clock Town with the clock stopped.
 */
void EnBen_LairWait(EnBen* this, PlayState* play) {
    if (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) != NA_BGM_DISABLED) {
        SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0);
    }

    if (this->timer > 0) {
        this->timer--;
        return;
    }
    if (play->transitionTrigger != TRANS_TRIGGER_OFF) {
        return;
    }

    Ben_SetDayFour(BEN_DAY_FOUR_HAUNTED);
    gSaveContext.save.time = EN_BEN_DAY_FOUR_TIME;
    play->nextEntrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_BLACK_SLOW;
}

void EnBen_Update(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    if (this->actor.params == BEN_ACTOR_LAIR) {
        EnBen_EmptyLair(play);
        this->actionFunc(this, play);
        return;
    }

    // The clock doesn't move on the 4th day.
    gSaveContext.save.timeSpeedOffset = -(s16)R_TIME_SPEED;

    EnBen_UpdateMusic(this, play);
    EnBen_EmptyTown(play);

    this->actionFunc(this, play);

    if (this->isHidden) {
        return;
    }

    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 30.0f, 20.0f, 70.0f, UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4);
    Actor_SetFocus(&this->actor, 40.0f);

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

void EnBen_Draw(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    if (this->isHidden) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    // Same opaque path the vanilla Elegy shell uses (En_Torch2).
    Scene_SetRenderModeXlu(play, 0, 0x01);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255);
    Gfx_DrawDListOpa(play, gElegyShellHumanDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
