/*
 * File: z_en_ben.c
 * Overlay: ovl_En_Ben
 * Description: BEN DROWNED - the Elegy of Emptiness statue that follows you (K4 stalker).
 *
 * Phase 0.3 spike behavior (CREEP mode):
 *  - Never moves while it is on screen. While off screen it closes the distance.
 *  - When it reaches you it forces a conversation (BEN_TEXT_TERRIBLE_FATE), then reappears behind you.
 *  - Swaps the scene music for a slowed, pitched-down Song of Healing.
 */

#include "z_en_ben.h"
#include "ben/ben.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_LOCK_ON_DISABLED)

#define EN_BEN_CREEP_SPEED 5.0f      // units per frame while unseen
#define EN_BEN_SPEAK_RANGE 90.0f     // xz distance that triggers the forced conversation
#define EN_BEN_REAPPEAR_DIST 400.0f  // how far from the player it reappears
#define EN_BEN_MUSIC_START_FRAME 20  // wait for the scene's own music to start before replacing it
#define EN_BEN_MUSIC_WARP_FRAME 30   // tempo and pitch changes only stick once the sequence is playing

void EnBen_Init(Actor* thisx, PlayState* play);
void EnBen_Destroy(Actor* thisx, PlayState* play);
void EnBen_Update(Actor* thisx, PlayState* play);
void EnBen_Draw(Actor* thisx, PlayState* play);

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

// Yaw offsets tried, in order, when looking for somewhere to reappear: straight behind first, then fanning out.
static s16 sReappearYawOffsets[] = { 0x8000, 0x6000, -0x6000, 0x4000, -0x4000 };

/**
 * True if the statue's focus point is inside the view frustum and in front of the camera.
 * (Actor_OnScreen alone can be fooled by points behind the camera.)
 */
s32 EnBen_IsSeen(EnBen* this, PlayState* play) {
    Vec3f projectedPos;
    f32 invW;

    Actor_GetProjectedPos(play, &this->actor.focus.pos, &projectedPos, &invW);

    if (projectedPos.z <= 0.0f) {
        return false;
    }
    return (projectedPos.x * invW >= -1.0f) && (projectedPos.x * invW <= 1.0f) && (projectedPos.y * invW >= -1.0f) &&
           (projectedPos.y * invW <= 1.0f);
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
 * Place the statue behind the player, facing them. Stays where it is if there's no safe spot.
 */
void EnBen_MoveBehindPlayer(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sReappearYawOffsets); i++) {
        s16 yaw = player->actor.shape.rot.y + sReappearYawOffsets[i];

        if (EnBen_FindSpot(play, player, yaw, EN_BEN_REAPPEAR_DIST, &pos)) {
            this->actor.world.pos = pos;
            this->actor.velocity.y = 0.0f;
            this->actor.shape.rot.y = yaw + 0x8000;
            this->actor.world.rot.y = this->actor.shape.rot.y;
            return;
        }
    }
}

void EnBen_Init(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.textId = BEN_TEXT_TERRIBLE_FATE;
    this->actor.gravity = -1.0f;
    this->timer = 0;
    this->hasSpoken = false;

    EnBen_MoveBehindPlayer(this, play);
    this->actionFunc = EnBen_Creep;
}

void EnBen_Destroy(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

void EnBen_UpdateMusic(EnBen* this, PlayState* play) {
    if (this->timer == EN_BEN_MUSIC_START_FRAME) {
        SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, NA_BGM_SONG_OF_HEALING);
    } else if (this->timer == EN_BEN_MUSIC_WARP_FRAME) {
        SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, BEN_SPIKE_TEMPO_SCALE);
        SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, BEN_SPIKE_FREQ_SCALE);
    }
}

void EnBen_Creep(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (!this->hasSpoken && (this->actor.xzDistToPlayer < EN_BEN_SPEAK_RANGE)) {
        this->actionFunc = EnBen_Speak;
        return;
    }

    // The rule: it never moves while you can see it.
    if (!EnBen_IsSeen(this, play) && (this->actor.xzDistToPlayer > (EN_BEN_SPEAK_RANGE * 0.5f))) {
        Math_Vec3f_StepToXZ(&this->actor.world.pos, &player->actor.world.pos, EN_BEN_CREEP_SPEED);
        this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
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
        EnBen_MoveBehindPlayer(this, play);
        this->actionFunc = EnBen_Creep;
    }
}

void EnBen_Update(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    if (this->timer < 0x7FFF) {
        this->timer++;
    }
    EnBen_UpdateMusic(this, play);

    this->actionFunc(this, play);

    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 30.0f, 20.0f, 70.0f, UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4);
    Actor_SetFocus(&this->actor, 40.0f);

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

void EnBen_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    // Same opaque path the vanilla Elegy shell uses (En_Torch2).
    Scene_SetRenderModeXlu(play, 0, 0x01);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255);
    Gfx_DrawDListOpa(play, gElegyShellHumanDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
