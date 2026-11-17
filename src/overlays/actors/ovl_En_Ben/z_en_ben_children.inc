/**
 * The Song of Unhealing, and the Moon Children who answer it (STORY_BIBLE.md §5). Included by z_en_ben.c.
 *
 * The statue teaches it: play it the Song of Healing and it plays your song back to you, backwards (C-Down, C-Right,
 * C-Left, twice). Play that anywhere, and this En_Ben (BEN_ACTOR_UNHEALING, spawned by Ben_UpdateSongs) answers with
 * the next sigil's task, one a session ("another night"). The music goes, the colour drains, and:
 *  1. The child no one sees. A Moon Child (a model of our own, tools/ben/gen_ben_models.py) is standing out there,
 *     looking at you, its head tilted. Walk up to it and it's gone, and then it's further off, giggling. It leads you
 *     somewhere, and the last time, it's standing on a sigil.
 *  2. Don't move. "Stand still with us. If you move, he'll know where you are." A minute on the HUD's clock. The child
 *     walks round and round you, its head always turned to you. Every few seconds the screen blinks (white, in the
 *     snow) and the statue is somewhere new, closer. Footsteps all round. Move, and it's right in front of you.
 *  3. The wet feet. Small wet footprints start to appear in front of you, one step at a time, walking away. They wait
 *     for you if you fall behind. At the end of them the child stands with its back to you. When you're close, its head
 *     turns all the way round.
 * Each one ends with a sigil glowing on the ground, the moon's cry, and the Moon Children's phrase a word longer.
 * Played when there's nothing for it this session: "...not tonight."
 */

#include "z_en_ben_models.inc.c"

#define EN_BEN_TASK_NONE 0  // nothing for it this session
#define EN_BEN_TASK_CHILD 1 // the first sigil
#define EN_BEN_TASK_STILL 2 // the second
#define EN_BEN_TASK_FEET 3  // the third

// Steps every task ends with
#define EN_BEN_UH_STAMP 20 // the sigil glows, and the moon cries
#define EN_BEN_UH_TEXT 21  // the phrase
#define EN_BEN_UH_FADE 22  // everything fades, and it's over

#define EN_BEN_UH_SETTLE 40            // after the song, the quiet before anything happens
#define EN_BEN_UH_GREY 150             // how grey the world goes while they're here
#define EN_BEN_UH_STAMP_TEXT 35        // after the sigil appears, how long before the phrase
#define EN_BEN_UH_FADE_STEP 6          // the sigil and the grey fade this much a frame at the end
#define EN_BEN_UH_GIVE_UP_DIST 2000.0f // walk this far away from them
#define EN_BEN_UH_GIVE_UP 400          // for this long, and they're gone
#define EN_BEN_CHILD_SCALE 0.01f
#define EN_BEN_SIGIL_SCALE 0.015f
#define EN_BEN_FOOT_SCALE 0.012f
#define EN_BEN_CHILD_SFX_GONE NA_SE_EN_STALKIDS_FADEOUT
#define EN_BEN_CHILD_SFX_GIGGLE NA_SE_EN_STALKIDS_LAUGH_MID

#define EN_BEN_CHILD_HOPS 4       // the child: how many times it moves on before the sigil
#define EN_BEN_CHILD_NEAR 240.0f  // walk this close and it's gone
#define EN_BEN_CHILD_REACH 130.0f // on the sigil: this close and it's yours
#define EN_BEN_CHILD_AWAY 25      // gone this long before it's somewhere else
#define EN_BEN_CHILD_TILT 0x900   // its head, tilted
#define EN_BEN_CHILD_NECK 0x5000  // its head turns this far from its body before the body follows (out of sight)

#define EN_BEN_STILL_SECONDS 60   // don't move: for this long
#define EN_BEN_STILL_SLACK 15.0f  // this far from where you stood counts as moving
#define EN_BEN_STILL_ORBIT 170.0f // the child walks round you this far out
#define EN_BEN_STILL_ORBIT_SPEED 0x60
#define EN_BEN_STILL_BLINK_EVERY 170 // the screen blinks this often
#define EN_BEN_STILL_BLINK_FRAMES 4  // for this long, and the statue's somewhere new
#define EN_BEN_STILL_FACE_BLINK 5    // the blink with BEN's face in it
#define EN_BEN_STILL_STEP_MIN 50     // footsteps all round you, this often
#define EN_BEN_STILL_STEP_RANGE 50   // plus up to this
#define EN_BEN_STILL_CAUGHT_DIST 70.0f
#define EN_BEN_STILL_CAUGHT_BLACK 10
#define EN_BEN_STILL_CAUGHT_HOLD 55

#define EN_BEN_FEET_PRINTS 28    // the wet feet: how many steps before the child
#define EN_BEN_FEET_EVERY 12     // a step this often
#define EN_BEN_FEET_STRIDE 32.0f // and this long
#define EN_BEN_FEET_WAIT 320.0f  // if you're further behind than this, they wait for you
#define EN_BEN_FEET_TURN 160.0f  // walk this close to the child at the end, and its head turns round
#define EN_BEN_FEET_LOOK 25      // it looks at you this long
#define EN_BEN_FEET_STEP_SFX NA_SE_PL_WALK_WATER1

// Where the statue stands at each blink while you stand still: how far, and how far round from in front of you
static f32 sStillDists[] = { 700.0f, 540.0f, 400.0f, 280.0f, 190.0f, 120.0f };
static s16 sStillTurns[] = { 0x7000, 0x4000, -0x4000, 0x0000, -0x7800, 0x0000 };

// Ways round when the child or the feet can't go straight on (the feet's first try is a little random)
static s16 sTurns[] = { 0, 0x1800, -0x1800, 0x3000, -0x3000, 0x5000, -0x5000, 0x7000, -0x7000 };

void EnBen_UnhealingInit(EnBen* this, PlayState* play) {
    this->isHidden = true; // the statue itself is never this one's to draw (only showStatue's)
    this->directorStep = 0;
    this->directorTimer = 0;
    this->timer = 0;
    this->showChild = false;
    this->showStatue = false;
    this->sigilAlpha = 0;
    this->feetCount = 0;
    this->feetHead = 0;
    this->hops = 0;
    this->trailCount = 0;
    this->faceFrames = 0;
    this->blurFrames = 0;
    this->grey = 0;
    this->timerOwned = false;
    this->isStaring = false;
    this->task = Ben_CanStampSigil() ? (EN_BEN_TASK_CHILD + Ben_GetSigils()) : EN_BEN_TASK_NONE;
    this->actionFunc = EnBen_Unhealing;
}

void EnBen_UnhealingDestroy(EnBen* this, PlayState* play) {
    if (this->grey > 0) {
        gPlayVisMonoColor.a = 0;
    }
    if (this->directorStep <= EN_BEN_UH_FADE) {
        Audio_SetMainBgmVolume(0x7F, 0); // gone before it was over (a scene change)
    }
    if (this->blurFrames > 0) {
        Play_DisableMotionBlur();
    }
}

s32 EnBen_IsFree(PlayState* play) {
    return (play->msgCtx.msgMode == MSGMODE_NONE) && (play->csCtx.state == CS_STATE_IDLE) &&
           (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF);
}

/**
 * While they're here, the statue keeps its distance: it isn't its turn.
 */
void EnBen_HushStatues(EnBen* this, PlayState* play) {
    Actor* it;

    for (it = play->actorCtx.actorLists[ACTORCAT_PROP].first; it != NULL; it = it->next) {
        EnBen* other = (EnBen*)it;

        if ((it == &this->actor) || (it->id != ACTOR_EN_BEN) || (it->params != BEN_ACTOR_STATUE) ||
            (it->update == NULL)) {
            continue;
        }
        if (other->actionFunc == EnBen_Lurk) {
            other->timer = CLAMP_MIN(other->timer, EN_BEN_STEPS_BEFORE + 20);
        } else if (other->actionFunc == EnBen_Stand) {
            EnBen_Vanish(other);
        }
    }
}

/**
 * A sound from somewhere in the world (not from this actor, which stays where the song was played).
 */
void EnBen_SfxAt(EnBen* this, PlayState* play, Vec3f* pos, u16 sfxId) {
    f32 invW;

    Actor_GetProjectedPos(play, pos, &this->stepPos, &invW);
    Audio_PlaySfx_AtPosWithPresetLowFreqAndHighReverb(&this->stepPos, sfxId);
}

/**
 * The child's head follows you. Its body only turns when you can't see it.
 */
void EnBen_ChildLooks(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 toYou = Math_Vec3f_Yaw(&this->childPos, &player->actor.world.pos);
    s16 neck;

    this->headYaw = toYou;
    neck = (s16)(this->headYaw - this->childYaw);
    if (((neck > EN_BEN_CHILD_NECK) || (neck < -EN_BEN_CHILD_NECK)) && !EnBen_IsBodyVisible(play, &this->childPos)) {
        this->childYaw = toYou;
    }
    this->headRoll = EN_BEN_CHILD_TILT + (s16)(Math_SinS(this->directorTimer * 0x280) * 0x300);
}

/**
 * The child: somewhere out in front of you, in plain view if it can be.
 */
s32 EnBen_PlaceChildInView(EnBen* this, PlayState* play) {
    static f32 sDists[] = { 520.0f, 400.0f, 300.0f, 220.0f };
    Player* player = GET_PLAYER(play);
    s16 camYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play));
    Vec3f pos;
    s32 d;
    s32 t;

    for (d = 0; d < ARRAY_COUNT(sDists); d++) {
        for (t = 0; t < 5; t++) {
            if (EnBen_FindSpot(play, player, camYaw + sTurns[t], sDists[d], &pos) && EnBen_IsBodyVisible(play, &pos)) {
                this->childPos = pos;
                this->childYaw = this->headYaw = Math_Vec3f_Yaw(&pos, &player->actor.world.pos);
                return true;
            }
        }
    }
    return false;
}

/**
 * The child moves on: further away from you, round whatever's in the way. In view if it can be; if not, you hear it.
 */
s32 EnBen_MoveChildOn(EnBen* this, PlayState* play) {
    static f32 sDists[] = { 550.0f, 420.0f, 300.0f };
    Player* player = GET_PLAYER(play);
    s16 away = Math_Vec3f_Yaw(&player->actor.world.pos, &this->childPos);
    Vec3f from = this->childPos;
    Vec3f pos;
    Vec3f fallback;
    s32 haveFallback = false;
    s32 d;
    s32 t;

    for (d = 0; d < ARRAY_COUNT(sDists); d++) {
        for (t = 0; t < ARRAY_COUNT(sTurns); t++) {
            if (!EnBen_FindSpotFrom(play, &from, away + sTurns[t], sDists[d], &pos)) {
                continue;
            }
            if (EnBen_IsBodyVisible(play, &pos)) {
                this->childPos = pos;
                return true;
            }
            if (!haveFallback) {
                fallback = pos;
                haveFallback = true;
            }
        }
    }
    if (haveFallback) {
        this->childPos = fallback;
    }
    return haveFallback;
}

void EnBen_StartStamp(EnBen* this, PlayState* play, Vec3f* at) {
    this->sigilPos = *at;
    if (this->sigilAlpha == 0) {
        this->sigilAlpha = 1;
    }
    Ben_StampSigil(play);
    Audio_PlaySfx(NA_SE_EV_MOON_CRY);
    Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
    this->directorStep = EN_BEN_UH_STAMP;
    this->directorTimer = 0;
}

/**
 * Task 1: the child no one sees. It leads you somewhere.
 */
void EnBen_TaskChild(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist = Math_Vec3f_DistXZ(&this->childPos, &player->actor.world.pos);

    switch (this->directorStep) {
        case 1: // there it is
            if (!EnBen_PlaceChildInView(this, play)) {
                if (this->directorTimer > EN_BEN_UH_SETTLE + 60) {
                    // Nowhere for it to stand here. It was here, though.
                    EnBen_StartStamp(this, play, &player->actor.world.pos);
                }
                break;
            }
            this->showChild = true;
            this->hops = 0;
            this->directorStep = 2;
            this->directorTimer = 0;
            break;

        case 2: // it waits for you
            EnBen_ChildLooks(this, play);
            if (this->hops >= EN_BEN_CHILD_HOPS) {
                if (dist < EN_BEN_CHILD_REACH) {
                    this->showChild = false;
                    EnBen_SfxAt(this, play, &this->childPos, EN_BEN_CHILD_SFX_GONE);
                    EnBen_StartStamp(this, play, &this->sigilPos);
                }
                break;
            }
            if (dist < EN_BEN_CHILD_NEAR) {
                this->showChild = false;
                EnBen_SfxAt(this, play, &this->childPos, EN_BEN_CHILD_SFX_GONE);
                this->directorStep = 3;
                this->directorTimer = 0;
            } else if (dist > EN_BEN_UH_GIVE_UP_DIST) {
                if (++this->timer >= EN_BEN_UH_GIVE_UP) {
                    this->showChild = false;
                    this->directorStep = EN_BEN_UH_FADE; // you didn't follow
                }
            } else {
                this->timer = 0;
            }
            break;

        case 3: // gone; and then it's further on
            if (this->directorTimer < EN_BEN_CHILD_AWAY) {
                break;
            }
            this->hops++;
            if (!EnBen_MoveChildOn(this, play)) {
                this->hops = EN_BEN_CHILD_HOPS; // it can't go on: this is the place
            }
            this->childYaw = this->headYaw = Math_Vec3f_Yaw(&this->childPos, &player->actor.world.pos);
            if (this->hops >= EN_BEN_CHILD_HOPS) {
                // The last time, it's standing on it
                this->sigilPos = this->childPos;
                this->sigilAlpha = 1;
            }
            this->showChild = true;
            EnBen_SfxAt(this, play, &this->childPos, EN_BEN_CHILD_SFX_GIGGLE);
            this->timer = 0;
            this->directorStep = 2;
            this->directorTimer = 0;
            break;

        default:
            break;
    }
}

/**
 * Task 2: don't move. A minute, with the child walking round you and the statue coming closer at every blink.
 */
void EnBen_TaskStill(EnBen* this, PlayState* play) {
    static f32 sCaughtScales[] = { 1.0f, 1.5f, 2.5f };
    Player* player = GET_PLAYER(play);
    s32 blink;
    Vec3f pos;
    f32 floorY;
    CollisionPoly* poly;
    s32 bgId;
    s32 i;

    switch (this->directorStep) {
        case 1: // a child, close in front of you, tells you
            this->showChild = EnBen_FindSpot(play, player, player->actor.shape.rot.y, 150.0f, &pos) ||
                              EnBen_FindSpot(play, player, player->actor.shape.rot.y, 100.0f, &pos);
            if (this->showChild) {
                this->childPos = pos;
                this->childYaw = this->headYaw = Math_Vec3f_Yaw(&pos, &player->actor.world.pos);
            }
            Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
            Message_StartTextbox(play, BEN_TEXT_DONT_MOVE, NULL);
            this->directorStep = 2;
            break;

        case 2:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                this->stillPos = player->actor.world.pos;
                this->walkYaw = Math_Vec3f_Yaw(&this->stillPos, &this->childPos);
                EnBen_StartTimer(this, EN_BEN_STILL_SECONDS);
                this->seekFrames = EN_BEN_STILL_SECONDS * 20;
                this->giggleTimer = EN_BEN_STILL_STEP_MIN;
                this->hops = 0; // blinks so far
                this->directorStep = 3;
                this->directorTimer = 0;
            }
            break;

        case 3: // standing still
            if ((Math_Vec3f_DistXZ(&player->actor.world.pos, &this->stillPos) > EN_BEN_STILL_SLACK) ||
                !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                // You moved. It knows where you are.
                EnBen_StopTimer(this);
                Play_FillScreen(play, true, 0, 0, 0, 255);
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                this->showChild = false;
                this->showStatue = false;
                for (i = 0; i < ARRAY_COUNT(sCaughtScales); i++) {
                    if (EnBen_FindSpot(play, player, player->actor.shape.rot.y,
                                       EN_BEN_STILL_CAUGHT_DIST * sCaughtScales[i], &pos)) {
                        this->trail[0] = pos;
                        this->trailYaw[0] = Math_Vec3f_Yaw(&pos, &player->actor.world.pos);
                        this->showStatue = true;
                        break;
                    }
                }
                this->directorStep = 5;
                this->directorTimer = 0;
                break;
            }
            if (play->msgCtx.msgMode == MSGMODE_NONE) {
                this->seekFrames--;
            }
            if ((this->seekFrames <= 0) || EnBen_TimerDone(this)) {
                // You stood still. It's all gone, and there's a sigil under your feet.
                EnBen_StopTimer(this);
                Play_FillScreen(play, true, 0, 0, 0, 255);
                this->showChild = false;
                this->showStatue = false;
                this->directorStep = 4;
                this->directorTimer = 0;
                break;
            }

            // The child walks round you, its head always turned to you
            this->walkYaw += EN_BEN_STILL_ORBIT_SPEED;
            pos.x = this->stillPos.x + Math_SinS(this->walkYaw) * EN_BEN_STILL_ORBIT;
            pos.y = this->stillPos.y + 60.0f;
            pos.z = this->stillPos.z + Math_CosS(this->walkYaw) * EN_BEN_STILL_ORBIT;
            floorY = BgCheck_EntityRaycastFloor3(&play->colCtx, &poly, &bgId, &pos);
            if ((floorY != BGCHECK_Y_MIN) && (fabsf(floorY - this->stillPos.y) < 80.0f)) {
                pos.y = floorY;
                this->childPos = pos;
                this->showChild = true;
            } else {
                this->showChild = false; // round the back of something
            }
            this->childYaw = this->walkYaw + 0x4000;
            this->headYaw = Math_Vec3f_Yaw(&this->childPos, &player->actor.world.pos);
            this->headRoll = EN_BEN_CHILD_TILT;

            // Every so often the screen blinks, and the statue's somewhere new, closer
            blink = this->directorTimer % EN_BEN_STILL_BLINK_EVERY;
            if ((blink == 0) && (this->directorTimer > 0)) {
                if (Ben_GetSession() == BEN_SESSION_COLD) {
                    Play_FillScreen(play, true, 235, 240, 255, 255); // in the snow, it's white
                } else {
                    Play_FillScreen(play, true, 0, 0, 0, 255);
                }
                i = CLAMP_MAX(this->hops, ARRAY_COUNT(sStillDists) - 1);
                if (EnBen_FindSpot(play, player, player->actor.shape.rot.y + sStillTurns[i], sStillDists[i], &pos) ||
                    EnBen_FindSpot(play, player, player->actor.shape.rot.y + sStillTurns[i], sStillDists[i] * 0.7f,
                                   &pos)) {
                    this->trail[0] = pos;
                    this->trailYaw[0] = Math_Vec3f_Yaw(&pos, &player->actor.world.pos);
                    this->showStatue = true;
                    EnBen_SfxAt(this, play, &this->trail[0], EN_BEN_STEP_SFX);
                }
                this->hops++;
                if (this->hops == EN_BEN_STILL_FACE_BLINK) {
                    this->faceFrames = EN_BEN_FACE_FRAMES;
                }
            } else if (blink == EN_BEN_STILL_BLINK_FRAMES) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
            }

            // Footsteps all round you
            if (--this->giggleTimer <= 0) {
                s16 yaw = (s16)Rand_ZeroFloat(65536.0f);

                pos.x = this->stillPos.x + Math_SinS(yaw) * (150.0f + Rand_ZeroFloat(250.0f));
                pos.y = this->stillPos.y;
                pos.z = this->stillPos.z + Math_CosS(yaw) * (150.0f + Rand_ZeroFloat(250.0f));
                EnBen_SfxAt(this, play, &pos, EN_BEN_STEP_SFX);
                this->giggleTimer = EN_BEN_STILL_STEP_MIN + (s32)Rand_ZeroFloat(EN_BEN_STILL_STEP_RANGE);
            }
            if (this->showStatue && (Math_Vec3f_DistXZ(&this->trail[0], &player->actor.world.pos) < 300.0f)) {
                Audio_PlaySfx(NA_SE_EN_LAST2_HEARTBEAT_OLD - SFX_FLAG);
            }
            break;

        case 4: // it's over
            if (this->directorTimer >= 8) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
                EnBen_StartStamp(this, play, &this->stillPos);
            }
            break;

        case 5: // you moved
            if (this->directorTimer == EN_BEN_STILL_CAUGHT_BLACK) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                Ben_AddSighting();
                this->grey = 255;
            } else if (this->directorTimer == (EN_BEN_STILL_CAUGHT_BLACK + 2)) {
                this->faceFrames = EN_BEN_FACE_FRAMES;
            } else if ((this->directorTimer > EN_BEN_STILL_CAUGHT_BLACK) &&
                       (this->directorTimer < (EN_BEN_STILL_CAUGHT_BLACK + EN_BEN_STILL_CAUGHT_HOLD))) {
                Audio_PlaySfx(NA_SE_EN_LAST2_HEARTBEAT_OLD - SFX_FLAG);
            } else if (this->directorTimer >= (EN_BEN_STILL_CAUGHT_BLACK + EN_BEN_STILL_CAUGHT_HOLD)) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                Play_EnableMotionBlur(EN_BEN_BLUR_ALPHA);
                this->blurFrames = EN_BEN_BLUR_FRAMES;
                this->showStatue = false;
                this->directorStep = EN_BEN_UH_FADE; // try again another time
            }
            break;

        default:
            break;
    }
}

/**
 * The next wet step: a stride on, turning round whatever's in the way. False if there's nowhere to step.
 */
s32 EnBen_NextWetStep(EnBen* this, PlayState* play) {
    Vec3f from = this->childPos;
    Vec3f to;
    Vec3f hitPos;
    CollisionPoly* poly;
    s32 bgId;
    f32 floorY;
    s16 yaw;
    s32 t;

    from.y += 20.0f;
    for (t = 0; t < ARRAY_COUNT(sTurns); t++) {
        yaw = this->walkYaw + ((t == 0) ? (s16)Rand_CenteredFloat(0x1800) : sTurns[t]);
        to.x = from.x + Math_SinS(yaw) * EN_BEN_FEET_STRIDE;
        to.y = from.y;
        to.z = from.z + Math_CosS(yaw) * EN_BEN_FEET_STRIDE;
        if (BgCheck_EntityLineTest1(&play->colCtx, &from, &to, &hitPos, &poly, true, false, false, true, &bgId)) {
            continue;
        }
        to.y += 40.0f;
        floorY = BgCheck_EntityRaycastFloor3(&play->colCtx, &poly, &bgId, &to);
        if ((floorY == BGCHECK_Y_MIN) || (fabsf(floorY - this->childPos.y) > 40.0f)) {
            continue;
        }
        this->childPos.x = to.x;
        this->childPos.y = floorY;
        this->childPos.z = to.z;
        this->walkYaw = yaw;
        return true;
    }
    return false;
}

/**
 * Task 3: the wet feet. Something small and invisible walks away from you, leaving wet footprints, and waits if you
 * fall behind. At the end of them, the child, with its back to you.
 */
void EnBen_TaskFeet(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist = Math_Vec3f_DistXZ(&this->childPos, &player->actor.world.pos);
    s16 side;
    u8 slot;

    switch (this->directorStep) {
        case 1: // it starts where you're standing
            this->childPos = player->actor.world.pos;
            this->walkYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play));
            this->hops = 0;
            EnBen_SfxAt(this, play, &this->childPos, NA_SE_EV_WATERDROP_GRD);
            this->directorStep = 2;
            this->directorTimer = 0;
            break;

        case 2: // step by step, away from you
            if (dist > EN_BEN_UH_GIVE_UP_DIST) {
                if (++this->timer >= EN_BEN_UH_GIVE_UP) {
                    this->directorStep = EN_BEN_UH_FADE; // they dried up
                }
                break;
            }
            this->timer = 0;
            if (((this->directorTimer % EN_BEN_FEET_EVERY) != 0) || (dist > EN_BEN_FEET_WAIT)) {
                break; // (it waits for you)
            }
            if ((this->hops >= EN_BEN_FEET_PRINTS) || !EnBen_NextWetStep(this, play)) {
                // The end of them. The child, facing away.
                this->childYaw = this->headYaw = this->walkYaw;
                this->headRoll = 0;
                this->showChild = true;
                this->directorStep = 3;
                this->directorTimer = 0;
                break;
            }
            // Left, right, left: each a little to its side of the way it's going
            side = (this->hops % 2) ? 0x4000 : -0x4000;
            slot = this->feetHead;
            this->feet[slot].x = this->childPos.x + Math_SinS(this->walkYaw + side) * 5.0f;
            this->feet[slot].y = this->childPos.y;
            this->feet[slot].z = this->childPos.z + Math_CosS(this->walkYaw + side) * 5.0f;
            this->feetYaw[slot] = this->walkYaw;
            this->feetHead = (this->feetHead + 1) % EN_BEN_FEET;
            if (this->feetCount < EN_BEN_FEET) {
                this->feetCount++;
            }
            EnBen_SfxAt(this, play, &this->feet[slot], EN_BEN_FEET_STEP_SFX);
            this->hops++;
            break;

        case 3: // its back to you
            if (dist < EN_BEN_FEET_TURN) {
                this->directorStep = 4;
                this->directorTimer = 0;
            }
            break;

        case 4: // and its head turns all the way round
            Math_ScaledStepToS(&this->headYaw, Math_Vec3f_Yaw(&this->childPos, &player->actor.world.pos), 0x600);
            this->headRoll = (s16)(this->directorTimer * 0x40);
            if (this->directorTimer == 1) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                EnBen_SfxAt(this, play, &this->childPos, NA_SE_EN_STALKIDS_TURN);
            } else if (this->directorTimer >= EN_BEN_FEET_LOOK) {
                Play_FillScreen(play, true, 0, 0, 0, 255);
                this->showChild = false;
                this->directorStep = 5;
                this->directorTimer = 0;
            }
            break;

        case 5:
            if (this->directorTimer >= 4) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                EnBen_StartStamp(this, play, &this->childPos);
            }
            break;

        default:
            break;
    }
}

void EnBen_Unhealing(EnBen* this, PlayState* play) {
    this->directorTimer++;

    // While they're here: the music's gone, the colour drains, and the statue keeps away
    if (this->directorStep < EN_BEN_UH_FADE) {
        EnBen_HushStatues(this, play);
        Math_StepToS(&this->grey, EN_BEN_UH_GREY, EN_BEN_GREY_STEP);
    } else {
        Math_StepToS(&this->grey, 0, EN_BEN_GREY_STEP);
    }
    if (this->grey > 0) {
        gPlayVisMonoColor.r = 255;
        gPlayVisMonoColor.g = 255;
        gPlayVisMonoColor.b = 255;
        gPlayVisMonoColor.a = CLAMP_MIN(gPlayVisMonoColor.a, this->grey);
    }
    if ((this->sigilAlpha > 0) && (this->directorStep < EN_BEN_UH_FADE)) {
        this->sigilAlpha = CLAMP_MAX(this->sigilAlpha + 4, 255);
    }

    switch (this->directorStep) {
        case 0: // the quiet after the song
            if (this->directorTimer == 1) {
                Audio_SetMainBgmVolume(0, 20);
            }
            if ((this->directorTimer < EN_BEN_UH_SETTLE) || !EnBen_IsFree(play) ||
                !(GET_PLAYER(play)->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                return;
            }
            this->directorTimer = 0;
            if (this->task == EN_BEN_TASK_NONE) {
                // Nothing for it tonight, or nothing more yet: the phrase as far as it goes
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Message_StartTextbox(play,
                                     (Ben_GetSigils() >= BEN_SIGILS_BUILT) ? (BEN_TEXT_SIGIL_1 + BEN_SIGILS_BUILT - 1)
                                                                           : BEN_TEXT_NOT_YET,
                                     NULL);
                this->directorStep = EN_BEN_UH_TEXT;
            } else {
                this->directorStep = 1;
            }
            return;

        case EN_BEN_UH_STAMP: // the sigil glows, and the moon cries
            if (this->directorTimer >= EN_BEN_UH_STAMP_TEXT) {
                Message_StartTextbox(play, BEN_TEXT_SIGIL_1 + Ben_GetSigils() - 1, NULL);
                this->directorStep = EN_BEN_UH_TEXT;
            }
            return;

        case EN_BEN_UH_TEXT:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                this->showChild = false;
                this->showStatue = false;
                this->directorStep = EN_BEN_UH_FADE;
                this->directorTimer = 0;
            }
            return;

        case EN_BEN_UH_FADE: // and it's over
            if (this->directorTimer == 1) {
                Audio_SetMainBgmVolume(0x7F, 40);
            }
            this->sigilAlpha = CLAMP_MIN(this->sigilAlpha - EN_BEN_UH_FADE_STEP, 0);
            if (this->feetCount > 0) {
                this->feetCount--; // the oldest dry up first
            }
            if ((this->grey == 0) && (this->sigilAlpha == 0) && (this->feetCount == 0)) {
                gPlayVisMonoColor.a = 0;
                this->directorStep = EN_BEN_UH_FADE + 1;
                Actor_Kill(&this->actor);
            }
            return;

        default:
            break;
    }

    switch (this->task) {
        case EN_BEN_TASK_CHILD:
            EnBen_TaskChild(this, play);
            break;

        case EN_BEN_TASK_STILL:
            EnBen_TaskStill(this, play);
            break;

        case EN_BEN_TASK_FEET:
            EnBen_TaskFeet(this, play);
            break;

        default:
            break;
    }
}

void EnBen_DrawUnhealing(EnBen* this, PlayState* play) {
    s32 i;
    s32 slot;
    u8 alpha;

    OPEN_DISPS(play->state.gfxCtx);

    if ((this->feetCount > 0) || (this->sigilAlpha > 0)) {
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, sBenGroundSetupDL);
        // The newest footprints are the wettest
        for (i = 0; i < this->feetCount; i++) {
            slot = (this->feetHead + EN_BEN_FEET - 1 - i) % EN_BEN_FEET;
            alpha = 210 - (i * 10);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 18, 26, 34, alpha);
            Matrix_Translate(this->feet[slot].x, this->feet[slot].y, this->feet[slot].z, MTXMODE_NEW);
            Matrix_RotateYS(this->feetYaw[slot], MTXMODE_APPLY);
            // Every other one the other foot
            Matrix_Scale((slot % 2) ? -EN_BEN_FOOT_SCALE : EN_BEN_FOOT_SCALE, EN_BEN_FOOT_SCALE, EN_BEN_FOOT_SCALE,
                         MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, sWetFootDL);
        }
        if (this->sigilAlpha > 0) {
            // It breathes
            alpha = (u8)(this->sigilAlpha * (0.75f + 0.25f * Math_SinS(this->directorTimer * 0x500)));
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 190, 210, 255, alpha);
            Matrix_Translate(this->sigilPos.x, this->sigilPos.y, this->sigilPos.z, MTXMODE_NEW);
            Matrix_RotateYS((s16)(this->directorTimer * 0x40), MTXMODE_APPLY);
            Matrix_Scale(EN_BEN_SIGIL_SCALE, EN_BEN_SIGIL_SCALE, EN_BEN_SIGIL_SCALE, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, sMoonSigilDL);
        }
    }

    if (this->showChild) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, sMoonChildSetupDL);
        Matrix_Translate(this->childPos.x, this->childPos.y, this->childPos.z, MTXMODE_NEW);
        Matrix_RotateYS(this->childYaw, MTXMODE_APPLY);
        Matrix_Scale(EN_BEN_CHILD_SCALE, EN_BEN_CHILD_SCALE, EN_BEN_CHILD_SCALE, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, sMoonChildBodyDL);
        // Its head, on its own
        Matrix_Translate(0.0f, MOON_CHILD_NECK_Y, 0.0f, MTXMODE_APPLY);
        Matrix_RotateYS((s16)(this->headYaw - this->childYaw), MTXMODE_APPLY);
        Matrix_RotateZS(this->headRoll, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, sMoonChildHeadDL);
    }

    if (this->showStatue) {
        // The statue, where this task put it (the same way the statue draws itself)
        Scene_SetRenderModeXlu(play, 0, 0x01);
        gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255);
        Matrix_Translate(this->trail[0].x, this->trail[0].y, this->trail[0].z, MTXMODE_NEW);
        Matrix_RotateYS(this->trailYaw[0], MTXMODE_APPLY);
        Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
        Gfx_DrawDListOpa(play, gElegyShellHumanDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
