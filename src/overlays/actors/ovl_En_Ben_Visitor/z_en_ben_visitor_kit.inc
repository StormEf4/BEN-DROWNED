/**
 * The visitors' kit: what every visitor needs (whether you can see it, somewhere to stand, holding Link, hurting him
 * without killing him), shared by the visitors from other stories (En_Ben_Visitor, En_Ben_Smile, En_Ben_Sleep) and the
 * SCPs (En_Ben_Scp173, 096, 106). Each includes it
 * once; each has its own copy, since one overlay can't call into another. Before it: VISITOR_PLACE_GIVE_UP, and an
 * actor on the EnBenVisitor struct (z_en_ben_visitor.h) with its `height` set in Init.
 */

// Where it looks for somewhere to stand out of your sight: behind you first, then round to the sides
static s16 sHiddenYaws[] = { 0x8000, 0x6000, -0x6000, 0x4000, -0x4000, 0x2800, -0x2800 };
static s16 sInViewYaws[] = { 0, 0x0C00, -0x0C00, 0x1800, -0x1800 };
static f32 sDistanceScales[] = { 1.0f, 0.8f, 0.6f };

/**
 * Whether you can see any of it standing at `base`: on screen within `middle` of its centre (1: anywhere), and nothing
 * in the way.
 */
static s32 EnBenVisitor_Seen(EnBenVisitor* this, PlayState* play, Vec3f* base, f32 middle) {
    Camera* camera = GET_ACTIVE_CAM(play);
    f32 height = this->height;
    Vec3f point;
    Vec3f projected;
    Vec3f hitPos;
    CollisionPoly* poly;
    s32 bgId;
    f32 invW;
    s32 i;

    for (i = 0; i < 3; i++) {
        point = *base;
        point.y += 10.0f + (height - 15.0f) * i / 2;
        Actor_GetProjectedPos(play, &point, &projected, &invW);
        if ((projected.z <= 0.0f) || (fabsf(projected.x * invW) > middle) || (fabsf(projected.y * invW) > 1.0f)) {
            continue;
        }
        if (!BgCheck_EntityLineTest1(&play->colCtx, &camera->eye, &point, &hitPos, &poly, true, true, true, true,
                                     &bgId)) {
            return true;
        }
    }
    return false;
}

/**
 * A spot `dist` from `from` along `yaw` with floor near its height and nothing in the way.
 */
static s32 EnBenVisitor_FindSpot(PlayState* play, Vec3f* from, s16 yaw, f32 dist, Vec3f* outPos) {
    Vec3f eye;
    Vec3f target;
    Vec3f hitPos;
    CollisionPoly* poly;
    s32 bgId;
    f32 floorY;

    eye.x = from->x;
    eye.y = from->y + 40.0f;
    eye.z = from->z;
    target.x = eye.x + (Math_SinS(yaw) * dist);
    target.y = eye.y;
    target.z = eye.z + (Math_CosS(yaw) * dist);
    if (BgCheck_EntityLineTest1(&play->colCtx, &eye, &target, &hitPos, &poly, true, false, false, true, &bgId)) {
        return false;
    }
    target.y = from->y + 100.0f;
    floorY = BgCheck_EntityRaycastFloor3(&play->colCtx, &poly, &bgId, &target);
    if ((floorY == BGCHECK_Y_MIN) || (fabsf(floorY - from->y) > 150.0f)) {
        return false;
    }
    outPos->x = target.x;
    outPos->y = floorY;
    outPos->z = target.z;
    return true;
}

static void EnBenVisitor_PlaceAt(EnBenVisitor* this, PlayState* play, Vec3f* pos) {
    Player* player = GET_PLAYER(play);

    this->actor.world.pos = *pos;
    this->actor.prevPos = *pos;
    this->actor.shape.rot.y = this->actor.world.rot.y = Math_Vec3f_Yaw(pos, &player->actor.world.pos);
}

/**
 * Somewhere about `dist` from you that you can (`visible`) or can't see, starting from the yaws given (from where the
 * camera looks).
 */
static s32 EnBenVisitor_TryPlace(EnBenVisitor* this, PlayState* play, f32 dist, s32 visible) {
    Player* player = GET_PLAYER(play);
    s16 camYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play));
    s16* yaws = visible ? sInViewYaws : sHiddenYaws;
    s32 count = visible ? ARRAY_COUNT(sInViewYaws) : ARRAY_COUNT(sHiddenYaws);
    Vec3f pos;
    s32 d;
    s32 i;

    for (d = 0; d < ARRAY_COUNT(sDistanceScales); d++) {
        for (i = 0; i < count; i++) {
            if (EnBenVisitor_FindSpot(play, &player->actor.world.pos, camYaw + yaws[i], dist * sDistanceScales[d],
                                      &pos) &&
                (!EnBenVisitor_Seen(this, play, &pos, 1.0f) == !visible)) {
                EnBenVisitor_PlaceAt(this, play, &pos);
                return true;
            }
        }
    }
    return false;
}

/**
 * Whether something else has Link: a conversation, a cutscene, a door.
 */
static s32 EnBenVisitor_Busy(PlayState* play) {
    return (play->msgCtx.msgMode != MSGMODE_NONE) || (play->csCtx.state != CS_STATE_IDLE) ||
           (play->transitionTrigger != TRANS_TRIGGER_OFF) || (play->transitionMode != TRANS_MODE_OFF);
}

static void EnBenVisitor_Hold(EnBenVisitor* this, PlayState* play) {
    if (!this->holding) {
        Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
        this->holding = true;
    }
}

static void EnBenVisitor_Release(EnBenVisitor* this, PlayState* play) {
    if (this->holding) {
        Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
        this->holding = false;
    }
}

/**
 * It hurts, but it never kills: at least a heart is always left.
 */
static void EnBenVisitor_Hurt(PlayState* play, s16 amount) {
    Player* player = GET_PLAYER(play);

    if (gSaveContext.save.saveInfo.playerData.health > 0x10) {
        gSaveContext.save.saveInfo.playerData.health =
            CLAMP_MIN(gSaveContext.save.saveInfo.playerData.health - amount, 0x10);
    }
    Player_PlaySfx(player, NA_SE_VO_LI_DAMAGE_S + player->ageProperties->voiceSfxIdOffset);
}

/**
 * Its first frames: somewhere to stand, or it doesn't come. Returns true once it's there.
 */
static s32 EnBenVisitor_Arrive(EnBenVisitor* this, PlayState* play, f32 dist, s32 visible) {
    if (this->placed) {
        return true;
    }
    if (EnBenVisitor_TryPlace(this, play, dist, visible) ||
        (visible && EnBenVisitor_TryPlace(this, play, dist * 0.8f, false))) {
        this->placed = true;
        this->isHidden = false;
        this->timer = 0;
        this->unseenFrames = 0;
        this->seenFrames = 0;
        this->stuck = 0;
    } else if (this->timer >= VISITOR_PLACE_GIVE_UP) {
        Actor_Kill(&this->actor);
    }
    return false;
}

// Only the ones that walk on the ground need it (the Smiling Man, SCP-173 and SCP-096)
#if (defined(EN_BEN_VISITOR) && (EN_BEN_VISITOR == BEN_VISITOR_SMILE)) || \
    (defined(EN_BEN_SCP) && (EN_BEN_SCP != BEN_VISITOR_OLD_MAN))
/**
 * One step along `yaw`, if there's ground there and nothing in the way.
 */
static s32 EnBenVisitor_Step(EnBenVisitor* this, PlayState* play, s16 yaw, f32 speed) {
    Vec3f pos;

    if (!EnBenVisitor_FindSpot(play, &this->actor.world.pos, yaw, speed, &pos)) {
        return false;
    }
    this->actor.world.pos = pos;
    return true;
}
#endif
