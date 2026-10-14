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
 *    Majora's lair instead (BEN_ACTOR_LAIR), where only Skull Kid is waiting, floating in the air, and then back to
 *    Clock Town with the clock stopped.
 *  - Try to get away by drowning Link in the Laundry Pool and it pulls you under. When you come back up, the statue is
 *    standing right behind you. Once you've seen it, it stays, and Link catches fire and dies. When you're back:
 *    "You've met with a terrible fate, haven't you?" (STORY_BIBLE.md C9, C10).
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

#define EN_BEN_SKULL_KID_DIST 260.0f   // in the lair: how far in front of you Skull Kid floats
#define EN_BEN_SKULL_KID_HEIGHT 160.0f // and how high

#define EN_BEN_DROWN_DEPTH 10.0f   // Laundry Pool water deeper than this counts as trying to drown
#define EN_BEN_DROWN_FRAMES 40     // stay in it this long (2 s) and it pulls you under
#define EN_BEN_DROWNED_DELAY 30    // after you come back up, how soon it's standing behind you
#define EN_BEN_DROWNED_DIST 260.0f // and how close
#define EN_BEN_IGNITE_DELAY 30     // after you see it then, how long before Link catches fire (1.5 s)
#define EN_BEN_BURN_DRAIN_EVERY 2  // while burning, 1/16 of a heart goes every this many frames, on top of the flames
#define EN_BEN_FATE_DELAY 50       // after the game over, how long before the line appears

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
void EnBen_Ignite(EnBen* this, PlayState* play);
void EnBen_Burning(EnBen* this, PlayState* play);
void EnBen_FateLine(EnBen* this, PlayState* play);
void EnBen_FateLineSpeaking(EnBen* this, PlayState* play);

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
        // Right after the Laundry Pool, it's close.
        f32 dist = ((Ben_GetFate() == BEN_FATE_DROWNED) ? EN_BEN_DROWNED_DIST : sSightingDistances[sighting]) *
                   sDistanceScales[d];

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
        // Nothing to hear. Just wait.
        play->sceneSequences.seqId = NA_BGM_NO_MUSIC;
        this->isHidden = true;
        this->spawnedSkullKid = false;
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
    this->drownFrames = 0;
    if (Ben_GetFate() == BEN_FATE_BURNING) {
        Ben_SetFate(BEN_FATE_BURNED); // Link burned to death, and this is where you came back
    }

    // Spawned during Play_Init, before the scene's music starts. Until you've seen it, there is no music at all.
    play->sceneSequences.seqId = (Ben_GetSightings() > 0) ? NA_BGM_SONG_OF_HEALING : NA_BGM_NO_MUSIC;

    // The camera isn't set up yet during Play_Init, so nothing is placed until updates start.
    this->isHidden = true;
    if (Ben_GetFate() == BEN_FATE_BURNED) {
        this->timer = EN_BEN_FATE_DELAY;
        this->actionFunc = EnBen_FateLine;
    } else if (this->isConfronting) {
        this->timer = EN_BEN_CONFRONT_DELAY;
        this->actionFunc = EnBen_Confront;
    } else if (Ben_GetFate() == BEN_FATE_DROWNED) {
        this->timer = EN_BEN_DROWNED_DELAY;
        this->actionFunc = EnBen_Lurk;
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
                if (Ben_GetFate() == BEN_FATE_DROWNED) {
                    // You couldn't get away. This time it doesn't go anywhere.
                    this->timer = EN_BEN_IGNITE_DELAY;
                    this->actionFunc = EnBen_Ignite;
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
 * After the Laundry Pool: you've seen it, and it stays where it is. Then Link catches fire.
 */
void EnBen_Ignite(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 i;

    if (this->timer > 0) {
        this->timer--;
        return;
    }

    for (i = 0; i < ARRAY_COUNT(player->bodyFlameTimers); i++) {
        player->bodyFlameTimers[i] = 200 + (s32)Rand_ZeroFloat(55.0f);
    }
    player->bodyIsBurning = true;
    Actor_PlaySfx(&player->actor, NA_SE_EV_FLAME_IGNITION);
    Player_PlaySfx(player, NA_SE_VO_LI_FALL_L + player->ageProperties->voiceSfxIdOffset);
    Ben_SetFate(BEN_FATE_BURNING);
    this->actionFunc = EnBen_Burning;
}

/**
 * Nothing puts it out. It watches until Link is dead; the game over takes it from there.
 */
void EnBen_Burning(EnBen* this, PlayState* play) {
    if ((gSaveContext.save.saveInfo.playerData.health > 0) && ((play->gameplayFrames % EN_BEN_BURN_DRAIN_EVERY) == 0)) {
        gSaveContext.save.saveInfo.playerData.health--;
    }
}

/**
 * Back after burning to death. No one is there, but someone has something to say.
 */
void EnBen_FateLine(EnBen* this, PlayState* play) {
    if (this->timer > 0) {
        this->timer--;
        return;
    }
    if (play->msgCtx.msgMode != MSGMODE_NONE) {
        return;
    }
    Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
    Message_StartTextbox(play, BEN_TEXT_TERRIBLE_FATE, NULL);
    this->actionFunc = EnBen_FateLineSpeaking;
}

void EnBen_FateLineSpeaking(EnBen* this, PlayState* play) {
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
        Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
        Ben_SetFate(BEN_FATE_DONE);
        EnBen_Vanish(this);
    }
}

/**
 * Trying to drown Link in the Laundry Pool to get away from it. It lets you, for a moment, then pulls you under.
 */
void EnBen_CheckDrowning(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if ((play->sceneId != SCENE_ALLEY) || (Ben_GetFate() != BEN_FATE_NONE) ||
        (play->transitionTrigger != TRANS_TRIGGER_OFF)) {
        return;
    }

    if ((player->actor.bgCheckFlags & BGCHECKFLAG_WATER) && (player->actor.depthInWater > EN_BEN_DROWN_DEPTH)) {
        if (player->transformation == PLAYER_FORM_DEKU) {
            Ben_SetFate(BEN_FATE_DROWNED); // a Deku scrub sinks by itself, and the game voids you out
            return;
        }
        this->drownFrames++;
    } else {
        this->drownFrames = 0;
    }

    if (this->drownFrames >= EN_BEN_DROWN_FRAMES) {
        Ben_SetFate(BEN_FATE_DROWNED);
        player->actor.freezeTimer = 40;
        Player_PlaySfx(player, NA_SE_VO_LI_TAKEN_AWAY + player->ageProperties->voiceSfxIdOffset);
        Audio_PlaySfx(NA_SE_OC_ABYSS);
        func_80169EFC(play); // void out: back to where you went in
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

    if (!this->spawnedSkullKid) {
        // Skull Kid, floating in the air in front of you and facing you. He does nothing. (Its object is loaded with
        // the scene; see Scene_CommandSpawnList.)
        Player* player = GET_PLAYER(play);
        s16 yaw = player->actor.shape.rot.y;

        this->spawnedSkullKid = true;
        if (Object_GetSlot(&play->objectCtx, OBJECT_STK) > OBJECT_SLOT_NONE) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_DM_STK,
                        player->actor.world.pos.x + (Math_SinS(yaw) * EN_BEN_SKULL_KID_DIST),
                        player->actor.world.pos.y + EN_BEN_SKULL_KID_HEIGHT,
                        player->actor.world.pos.z + (Math_CosS(yaw) * EN_BEN_SKULL_KID_DIST), 0, yaw + 0x8000, 0,
                        0); // DM_STK_TYPE_SKULL_KID
        }
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
    EnBen_CheckDrowning(this, play);

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
