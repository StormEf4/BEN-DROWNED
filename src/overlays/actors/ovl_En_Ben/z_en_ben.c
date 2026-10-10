/*
 * File: z_en_ben.c
 * Overlay: ovl_En_Ben
 * Description: BEN DROWNED - the Elegy of Emptiness statue that follows you (K4 stalker).
 *
 * Phase 0.3 spike behavior (CREEP mode):
 *  - Starts hidden and appears somewhere the camera can't see, facing the player.
 *  - Never moves while any part of it is on screen. While off screen it closes the distance.
 *  - When it reaches you it forces a conversation (BEN_TEXT_TERRIBLE_FATE), then vanishes and reappears out of view.
 *  - Makes a slowed, pitched-down Song of Healing the scene's music.
 */

#include "z_en_ben.h"
#include "ben/ben.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_LOCK_ON_DISABLED)

#define EN_BEN_CREEP_SPEED 5.0f     // units per frame while unseen
#define EN_BEN_SPEAK_RANGE 90.0f    // xz distance that triggers the forced conversation
#define EN_BEN_MUSIC_RETRY_FRAMES 60 // how long to let a requested sequence load before requesting it again
#define EN_BEN_APPEAR_RETRY_FRAMES 8 // a failed spot search is retried this often, not every frame
#define EN_BEN_VANISH_FRAMES 40      // how long it stays gone after speaking

void EnBen_Init(Actor* thisx, PlayState* play);
void EnBen_Destroy(Actor* thisx, PlayState* play);
void EnBen_Update(Actor* thisx, PlayState* play);
void EnBen_Draw(Actor* thisx, PlayState* play);

void EnBen_WaitToAppear(EnBen* this, PlayState* play);
void EnBen_Creep(EnBen* this, PlayState* play);
void EnBen_Speak(EnBen* this, PlayState* play);
void EnBen_Speaking(EnBen* this, PlayState* play);

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

// Heights (feet, chest, head) checked against the camera: if any is on screen, the statue counts as seen.
static f32 sBodyCheckHeights[] = { 10.0f, 40.0f, 75.0f };

// Distances tried when looking for somewhere to appear, nearest-feeling first.
static f32 sAppearDistances[] = { 400.0f, 550.0f, 300.0f };

#define EN_BEN_APPEAR_DIRECTIONS 16

/**
 * True if a world point is inside the view frustum and in front of the camera.
 * (Checking x/y alone can be fooled by points behind the camera.)
 */
s32 EnBen_IsPointSeen(PlayState* play, Vec3f* point) {
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
 * True if any part of a statue standing at `base` would be on screen.
 */
s32 EnBen_IsBodySeen(PlayState* play, Vec3f* base) {
    Vec3f point;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sBodyCheckHeights); i++) {
        point.x = base->x;
        point.y = base->y + sBodyCheckHeights[i];
        point.z = base->z;
        if (EnBen_IsPointSeen(play, &point)) {
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
    if ((floorY == BGCHECK_Y_MIN) || (fabsf(floorY - player->actor.world.pos.y) > 100.0f)) {
        return false; // no floor, or a big drop or climb
    }

    outPos->x = target.x;
    outPos->y = floorY;
    outPos->z = target.z;
    return true;
}

/**
 * Try to put the statue somewhere safe that the camera can't see, facing the player. The search starts on the
 * camera's side of the player (behind the camera) and fans out around the player.
 * Returns false (and leaves the statue where it is) if no such spot exists right now.
 */
s32 EnBen_TryAppearOutOfView(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 towardCameraYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
    Vec3f pos;
    s32 d;
    s32 i;

    for (d = 0; d < ARRAY_COUNT(sAppearDistances); d++) {
        for (i = 0; i < EN_BEN_APPEAR_DIRECTIONS; i++) {
            // 0, +1, -1, +2, -2, ... steps of 1/16 turn away from "toward the camera"
            s16 step = (i + 1) / 2;
            s16 yaw = towardCameraYaw + (((i % 2) != 0) ? step : -step) * (0x10000 / EN_BEN_APPEAR_DIRECTIONS);

            if (EnBen_FindSpot(play, player, yaw, sAppearDistances[d], &pos) && !EnBen_IsBodySeen(play, &pos)) {
                this->actor.world.pos = pos;
                this->actor.prevPos = pos;
                this->actor.velocity.y = 0.0f;
                this->actor.shape.rot.y = Math_Vec3f_Yaw(&pos, &player->actor.world.pos);
                this->actor.world.rot.y = this->actor.shape.rot.y;
                return true;
            }
        }
    }
    return false;
}

void EnBen_Hide(EnBen* this, s16 delay) {
    this->isHidden = true;
    this->appearDelay = delay;
    this->actionFunc = EnBen_WaitToAppear;
}

void EnBen_Init(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.textId = BEN_TEXT_TERRIBLE_FATE;
    this->actor.gravity = -1.0f;
    this->hasSpoken = false;
    this->musicCooldown = 0;
    this->musicWarped = false;

    // Spawned during Play_Init, before the scene's music starts: make the Song of Healing the scene's music so the
    // game's own music system starts it. The watchdog in EnBen_UpdateMusic covers the cases where it doesn't.
    play->sceneSequences.seqId = NA_BGM_SONG_OF_HEALING;

    // The camera isn't set up yet during Play_Init, so pick the spot once updates start.
    EnBen_Hide(this, 0);
}

void EnBen_Destroy(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    Collider_DestroyCylinder(play, &this->collider);

    // Leaving the scene: undo the slowdown and make the next scene start its own music, even if it uses the same
    // sequence the game last requested before we took over (e.g. the Clock Town theme in the next district).
    SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, 100);
    SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, 1000);
    Audio_ResetRequestedSceneSeqId();
}

/**
 * Keep the Song of Healing playing, slowed and pitched down, whatever else tries to start music.
 */
void EnBen_UpdateMusic(EnBen* this, PlayState* play) {
    if (this->musicCooldown > 0) {
        this->musicCooldown--;
    }

    if (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) != NA_BGM_SONG_OF_HEALING) {
        this->musicWarped = false;
        if (this->musicCooldown == 0) {
            SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, NA_BGM_SONG_OF_HEALING);
            this->musicCooldown = EN_BEN_MUSIC_RETRY_FRAMES;
        }
    } else if (!this->musicWarped) {
        SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, BEN_SPIKE_TEMPO_SCALE);
        SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, BEN_SPIKE_FREQ_SCALE);
        this->musicWarped = true;
    }
}

void EnBen_WaitToAppear(EnBen* this, PlayState* play) {
    if (this->appearDelay > 0) {
        this->appearDelay--;
        return;
    }
    if (EnBen_TryAppearOutOfView(this, play)) {
        this->isHidden = false;
        this->actionFunc = EnBen_Creep;
    } else {
        this->appearDelay = EN_BEN_APPEAR_RETRY_FRAMES;
    }
}

void EnBen_Creep(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (!this->hasSpoken && (this->actor.xzDistToPlayer < EN_BEN_SPEAK_RANGE)) {
        this->actionFunc = EnBen_Speak;
        return;
    }

    // The rule: it never moves while you can see it.
    if (!EnBen_IsBodySeen(play, &this->actor.world.pos) && (this->actor.xzDistToPlayer > (EN_BEN_SPEAK_RANGE * 0.5f))) {
        Math_Vec3f_StepToXZ(&this->actor.world.pos, &player->actor.world.pos, EN_BEN_CREEP_SPEED);
        this->actor.shape.rot.y = Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos);
        this->actor.world.rot.y = this->actor.shape.rot.y;
    }
}

void EnBen_Speak(EnBen* this, PlayState* play) {
    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        this->actionFunc = EnBen_Speaking;
    } else {
        this->actor.flags |= ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        Actor_OfferTalk(&this->actor, play, EN_BEN_SPEAK_RANGE * 2.0f);
    }
}

void EnBen_Speaking(EnBen* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actor.flags &= ~ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED;
        this->hasSpoken = true;
        // Vanish, then come back somewhere you aren't looking.
        EnBen_Hide(this, EN_BEN_VANISH_FRAMES);
    }
}

void EnBen_Update(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    EnBen_UpdateMusic(this, play);

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
