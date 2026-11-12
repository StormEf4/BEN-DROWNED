/**
 * Hide and seek (STORY_BIBLE.md §4 rule 2: BEN plays games). Included by z_en_ben.c.
 *
 * Once a session, from Session 4 to 6, after you've seen it a couple of times, the statue is standing right in front of
 * you instead: "Let's play hide and seek. I hide. You look." Everything goes black, and when it comes back it's gone
 * and the HUD's clock is counting down. It's hiding somewhere round a corner. A child's giggle comes from where it is
 * now and then: louder and more often the closer you are. A heartbeat when you're close.
 *  - Find it (see it from near enough, for a moment) and it laughs and is gone, and leaves you something where it
 *    stood. It leaves you alone for a good while after that.
 *  - Run out of time, and everything goes black, and when it comes back it's standing right in front of you.
 */

#define EN_BEN_SEEK_FROM 2            // hide and seek waits until you've seen it this many times this session
#define EN_BEN_SEEK_SECONDS 75        // how long you have to find it
#define EN_BEN_SEEK_ASK_DIST 1.3f     // it asks from this far in front of you (times EN_BEN_CONFRONT_DIST)
#define EN_BEN_SEEK_BLACK 30          // after it asks, how long the screen stays black while it hides
#define EN_BEN_SEEK_TRIES 24          // ways round a corner it tries each frame of that
#define EN_BEN_SEEK_OUT_MIN 300.0f    // its hiding place: first this far out from you, in some direction
#define EN_BEN_SEEK_OUT_RANGE 500.0f  // plus up to this much
#define EN_BEN_SEEK_SIDE_MIN 200.0f   // then this far off to one side, round the corner
#define EN_BEN_SEEK_SIDE_RANGE 350.0f // plus up to this much
#define EN_BEN_SEEK_FOUND_DIST 600.0f // see it from closer than this
#define EN_BEN_SEEK_FOUND_FRAMES 10   // for this long, and you've found it
#define EN_BEN_SEEK_GIGGLE_SFX NA_SE_EN_STALKIDS_LAUGH_MID // a child's giggle, from where it's hiding
#define EN_BEN_SEEK_GIGGLE_NEAR 25                         // giggles this often when you're right by it
#define EN_BEN_SEEK_GIGGLE_FAR 140                         // and this often when you're far off
#define EN_BEN_SEEK_HEARTBEAT 400.0f                       // closer than this, you hear your heart
#define EN_BEN_SEEK_REST 1600         // found it: it leaves you alone this much longer than usual (80 s)
#define EN_BEN_SEEK_CAUGHT_DIST 70.0f // ran out of time: it's this close when the screen comes back
#define EN_BEN_SEEK_CAUGHT_BLACK 12   // after this long in the black
#define EN_BEN_SEEK_CAUGHT_HOLD 50    // and holds you this long

// The HUD's countdown. The way of using the game's minigame timer is the Majora's Mask: WTF project's timer kit
// (src/code/z_mod_timer.c there): ours only while no minigame of the game's has it, and off with the scene.
#define EN_BEN_TIMER_ID TIMER_ID_MINIGAME_2

s32 EnBen_StartTimer(EnBen* this, s32 seconds) {
    if (gSaveContext.timerStates[EN_BEN_TIMER_ID] != TIMER_STATE_OFF) {
        return false; // one of the game's own minigames has it
    }
    Interface_StartTimer(EN_BEN_TIMER_ID, seconds);
    this->timerOwned = true;
    return true;
}

void EnBen_StopTimer(EnBen* this) {
    if (this->timerOwned) {
        gSaveContext.timerStates[EN_BEN_TIMER_ID] = TIMER_STATE_OFF;
        this->timerOwned = false;
    }
}

// A count-down the game stopped at 0 (it turns it off the frame after)
s32 EnBen_TimerDone(EnBen* this) {
    return this->timerOwned && ((gSaveContext.timerStates[EN_BEN_TIMER_ID] == TIMER_STATE_STOP) ||
                                (gSaveContext.timerStates[EN_BEN_TIMER_ID] == TIMER_STATE_OFF));
}

/**
 * Somewhere to hide: out some way from you, then round to one side, so there's a corner between you and it if the
 * place has one. A spot you can't see from where you stand wins; failing that, one that's at least off the screen.
 */
s32 EnBen_FindHidingSpot(EnBen* this, PlayState* play, Vec3f* outPos) {
    Player* player = GET_PLAYER(play);
    Vec3f mid;
    Vec3f pos;
    Vec3f eye;
    Vec3f head;
    Vec3f hitPos;
    CollisionPoly* poly;
    s32 bgId;
    s32 haveFallback = false;
    s16 yaw;
    s32 i;

    eye = player->actor.world.pos;
    eye.y += 40.0f;
    for (i = 0; i < EN_BEN_SEEK_TRIES; i++) {
        yaw = (s16)Rand_ZeroFloat(65536.0f);
        if (!EnBen_FindSpot(play, player, yaw, EN_BEN_SEEK_OUT_MIN + Rand_ZeroFloat(EN_BEN_SEEK_OUT_RANGE), &mid)) {
            continue;
        }
        yaw += (Rand_ZeroOne() < 0.5f) ? 0x4000 : -0x4000;
        if (!EnBen_FindSpotFrom(play, &mid, yaw, EN_BEN_SEEK_SIDE_MIN + Rand_ZeroFloat(EN_BEN_SEEK_SIDE_RANGE), &pos)) {
            continue;
        }
        head = pos;
        head.y += 60.0f;
        if (BgCheck_EntityLineTest1(&play->colCtx, &eye, &head, &hitPos, &poly, true, true, true, true, &bgId)) {
            *outPos = pos; // round a corner
            return true;
        }
        if (!haveFallback && !EnBen_IsBodyVisible(play, &pos)) {
            *outPos = pos;
            haveFallback = true;
        }
    }
    return haveFallback;
}

/**
 * Whether it's time for hide and seek instead of an ordinary sighting.
 */
s32 EnBen_TrySeek(EnBen* this, PlayState* play) {
    s32 session = Ben_GetSession();

    if ((this->actor.params != BEN_ACTOR_STATUE) || (Ben_GetFate() != BEN_FATE_NONE) || this->inWhiteout ||
        ((session != BEN_SESSION_COLD) && (session != BEN_SESSION_DROWNED) && (session != BEN_SESSION_STONE)) ||
        ((session == BEN_SESSION_DROWNED) && (Ben_GetStep() != BEN_STEP5_SEA)) || Ben_HasPlayedSeek() ||
        (Ben_GetSightings() < EN_BEN_SEEK_FROM) || (Ben_GetSightings() >= EN_BEN_DREAM_SIGHTINGS - 1)) {
        return false;
    }
    this->directorStep = 0;
    this->directorTimer = 0;
    this->seenFrames = 0;
    this->actionFunc = EnBen_Seek;
    return true;
}

void EnBen_Seek(EnBen* this, PlayState* play) {
    static f32 sCaughtScales[] = { 1.0f, 1.5f, 2.5f };
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    f32 dist;
    s32 i;

    this->directorTimer++;

    switch (this->directorStep) {
        case 0: // it's in front of you, and it wants to play
            if ((play->msgCtx.msgMode != MSGMODE_NONE) || (play->csCtx.state != CS_STATE_IDLE) ||
                (play->transitionTrigger != TRANS_TRIGGER_OFF) || !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                break;
            }
            if (!EnBen_FindSpot(play, player, player->actor.shape.rot.y, EN_BEN_CONFRONT_DIST * EN_BEN_SEEK_ASK_DIST,
                                &pos) &&
                !EnBen_FindSpot(play, player, player->actor.shape.rot.y, EN_BEN_CONFRONT_DIST, &pos)) {
                // Nowhere to stand: another time
                this->timer = EN_BEN_RETRY_FRAMES;
                this->actionFunc = EnBen_Lurk;
                break;
            }
            Ben_SetPlayedSeek(); // once a session, however it goes
            EnBen_PlaceAt(this, play, &pos);
            this->isHidden = false;
            this->trailCount = 0;
            EnBen_SetStaring(this, true);
            Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
            Message_StartTextbox(play, BEN_TEXT_SEEK, &this->actor);
            this->directorStep = 1;
            break;

        case 1:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Play_FillScreen(play, true, 0, 0, 0, 255);
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                this->isHidden = true;
                EnBen_SetStaring(this, false);
                this->directorStep = 2;
                this->directorTimer = 0;
                this->hops = false; // found somewhere yet
            }
            break;

        case 2: // in the dark, it hides
            if (!this->hops && EnBen_FindHidingSpot(this, play, &pos)) {
                EnBen_PlaceAt(this, play, &pos);
                this->hops = true;
            }
            if (this->directorTimer < EN_BEN_SEEK_BLACK) {
                break;
            }
            Play_FillScreen(play, false, 0, 0, 0, 0);
            Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
            if (!this->hops) {
                EnBen_Vanish(this); // nowhere to hide here: it just went
                break;
            }
            EnBen_StartTimer(this, EN_BEN_SEEK_SECONDS);
            this->seekFrames = EN_BEN_SEEK_SECONDS * 20;
            this->giggleTimer = EN_BEN_SEEK_GIGGLE_NEAR;
            this->seenFrames = 0;
            this->isHidden = false; // there, if you can find it
            this->directorStep = 3;
            break;

        case 3: // seeking
            dist = this->actor.xzDistToPlayer;
            if (play->msgCtx.msgMode == MSGMODE_NONE) {
                this->seekFrames--;
            }
            if (EnBen_IsBodyVisible(play, &this->actor.world.pos) && (dist < EN_BEN_SEEK_FOUND_DIST)) {
                this->seenFrames++;
            } else {
                this->seenFrames = 0;
            }
            if (this->seenFrames >= EN_BEN_SEEK_FOUND_FRAMES) {
                // Found it. It laughs and it's gone, and it leaves you something where it stood.
                EnBen_StopTimer(this);
                Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
                pos = this->actor.world.pos;
                Item_DropCollectible(play, &pos, ITEM00_RUPEE_RED);
                Play_EnableMotionBlur(EN_BEN_BLUR_ALPHA);
                this->blurFrames = EN_BEN_BLUR_FRAMES;
                EnBen_Leave(this);
                this->timer += EN_BEN_SEEK_REST; // and it leaves you alone for a while
                break;
            }
            if ((this->seekFrames <= 0) || EnBen_TimerDone(this)) {
                // Time's up. It finds you.
                EnBen_StopTimer(this);
                Play_FillScreen(play, true, 0, 0, 0, 255);
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                for (i = 0; i < ARRAY_COUNT(sCaughtScales); i++) {
                    if (EnBen_FindSpot(play, player, player->actor.shape.rot.y,
                                       EN_BEN_SEEK_CAUGHT_DIST * sCaughtScales[i], &pos)) {
                        EnBen_PlaceAt(this, play, &pos);
                        break;
                    }
                }
                this->isHidden = (i == ARRAY_COUNT(sCaughtScales)); // (if there's no room in front of you, it's dark)
                this->directorStep = 4;
                this->directorTimer = 0;
                break;
            }
            // Hot and cold: a giggle from where it is, more often the closer you are
            if (--this->giggleTimer <= 0) {
                Actor_PlaySfx(&this->actor, EN_BEN_SEEK_GIGGLE_SFX);
                this->giggleTimer = CLAMP((s32)(dist / 10.0f), EN_BEN_SEEK_GIGGLE_NEAR, EN_BEN_SEEK_GIGGLE_FAR);
            }
            if (dist < EN_BEN_SEEK_HEARTBEAT) {
                Audio_PlaySfx(NA_SE_EN_LAST2_HEARTBEAT_OLD - SFX_FLAG);
            }
            break;

        case 4: // in the dark
            if (this->directorTimer == EN_BEN_SEEK_CAUGHT_BLACK) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
                EnBen_SetStaring(this, true);
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                Ben_AddSighting(); // it found you: that counts
            } else if (this->directorTimer == (EN_BEN_SEEK_CAUGHT_BLACK + 2)) {
                this->faceFrames = EN_BEN_FACE_FRAMES; // and for a moment, it's his face
            } else if (this->directorTimer >= (EN_BEN_SEEK_CAUGHT_BLACK + EN_BEN_SEEK_CAUGHT_HOLD)) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                EnBen_Leave(this);
            }
            break;

        default:
            break;
    }
}
