/**
 * Termina, a little wrong (STORY_BIBLE.md §4 rule 5: dread, not jump scares). Included by z_en_ben.c.
 *
 * From Session 3 on, wherever the statue isn't stalking you and nothing's scripted (Ben_GetDirector's fallback), the
 * rest of Termina isn't safe either. Every few minutes, one small thing, and then nothing again for a long while:
 *  - somebody nearby stops what they're doing and stares at you;
 *  - a figure a long way off at the edge of the screen, standing still, gone once you look straight at it;
 *  - stone footsteps behind you, closer, and then they stop;
 *  - his laugh, a long way behind you;
 *  - the music sags, slower and lower, and comes back;
 *  - the colour goes out of everything for a few seconds.
 * None of it counts as a sighting, so none of it moves the story on. It keeps out of the way of conversations,
 * cutscenes, the game's minigames and the Moon Children, and it comes a little more often as the story goes on.
 */

#define EN_BEN_TERMINA_FIRST 900           // after you arrive, the first thing is at least this long (45 s)
#define EN_BEN_TERMINA_EVERY 2400          // then at least this long between them (2 min)
#define EN_BEN_TERMINA_RANGE 2400          // plus up to this much more
#define EN_BEN_TERMINA_SOONER 300          // each session past the third takes this off the wait (Session 7: 1-3 min)
#define EN_BEN_TERMINA_RETRY 100           // if now's no good (you're talking), try again this soon
#define EN_BEN_TERMINA_STARE_DIST 600.0f   // only people this close, and on screen
#define EN_BEN_TERMINA_STARE_FRAMES 70     // and they stare this long
#define EN_BEN_TERMINA_FIGURE_DIST 1200.0f // the figure: this far off
#define EN_BEN_TERMINA_FIGURE_ANGLE 0x1600 // and this far round from where the camera looks: the edge of the screen
#define EN_BEN_TERMINA_FIGURE_MIDDLE 0.45f // looking straight at it: it's this near the middle of the screen
#define EN_BEN_TERMINA_FIGURE_SEEN 6       // looked at like that this long, and it's gone
#define EN_BEN_TERMINA_FIGURE_LONGEST 120  // and it's gone after this long whatever you do (6 s)
#define EN_BEN_TERMINA_LAUGH_DIST 900.0f   // the laugh: this far behind you
#define EN_BEN_TERMINA_MUSIC_FRAMES 100    // the music sags for this long (5 s)
#define EN_BEN_TERMINA_MUSIC_TEMPO 70      // to this tempo (percent)
#define EN_BEN_TERMINA_MUSIC_FREQ 820      // and this pitch (1000 = normal)
#define EN_BEN_TERMINA_MUSIC_EASE 30       // easing in and out over this long
#define EN_BEN_TERMINA_GREY 150            // the colour goes this far (0-255)
#define EN_BEN_TERMINA_GREY_FRAMES 80      // for this long (4 s)
#define EN_BEN_TERMINA_GREY_STEP 4         // fading this much a frame

typedef enum EnBenTerminaEvent {
    /* 0 */ EN_BEN_TERMINA_WAIT,
    /* 1 */ EN_BEN_TERMINA_STARE,
    /* 2 */ EN_BEN_TERMINA_FIGURE,
    /* 3 */ EN_BEN_TERMINA_STEPS,
    /* 4 */ EN_BEN_TERMINA_LAUGH,
    /* 5 */ EN_BEN_TERMINA_MUSIC,
    /* 6 */ EN_BEN_TERMINA_GREY_OUT
} EnBenTerminaEvent;

void EnBen_Termina(EnBen* this, PlayState* play);

// Session 3 is still early: nothing that walks up behind you, and the colour stays. From Session 4, all of it.
static u8 sTerminaEarly[] = { EN_BEN_TERMINA_STARE, EN_BEN_TERMINA_FIGURE, EN_BEN_TERMINA_LAUGH, EN_BEN_TERMINA_MUSIC };
static u8 sTerminaLate[] = { EN_BEN_TERMINA_STARE, EN_BEN_TERMINA_FIGURE, EN_BEN_TERMINA_STEPS,
                             EN_BEN_TERMINA_LAUGH, EN_BEN_TERMINA_MUSIC,  EN_BEN_TERMINA_GREY_OUT };

s32 EnBen_TerminaWait(void) {
    s32 sooner = (Ben_GetSession() - BEN_SESSION_BEN) * EN_BEN_TERMINA_SOONER;

    return EN_BEN_TERMINA_EVERY - CLAMP_MIN(sooner, 0) + (s32)Rand_ZeroFloat(EN_BEN_TERMINA_RANGE);
}

void EnBen_TerminaInit(EnBen* this, PlayState* play) {
    this->isHidden = true;
    this->isNoticed = false;
    this->trailCount = 0;
    this->faceFrames = 0;
    this->grey = 0;
    this->musicWarped = false;
    this->other = NULL;
    this->directorStep = EN_BEN_TERMINA_WAIT;
    this->directorTimer = 0;
    this->stepTimer = 0;
    this->timer = EN_BEN_TERMINA_FIRST + (s32)Rand_ZeroFloat(EN_BEN_TERMINA_RANGE);
    this->actionFunc = EnBen_Termina;
}

void EnBen_TerminaDestroy(EnBen* this, PlayState* play) {
    if (this->musicWarped) {
        SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, 100);
        SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, 1000);
    }
    if (this->grey > 0) {
        gPlayVisMonoColor.a = 0;
    }
}

/**
 * The Moon Children are here (the Song of Unhealing): the colour and the music are theirs.
 */
s32 EnBen_ChildrenHere(EnBen* this, PlayState* play) {
    Actor* it;

    for (it = play->actorCtx.actorLists[ACTORCAT_PROP].first; it != NULL; it = it->next) {
        if ((it != &this->actor) && (it->id == ACTOR_EN_BEN) && (it->params == BEN_ACTOR_UNHEALING) &&
            (it->update != NULL)) {
            return true;
        }
    }
    return false;
}

/**
 * Whether something else has Link's attention: a conversation, a cutscene, a minigame, the Moon Children.
 */
s32 EnBen_TerminaBusy(EnBen* this, PlayState* play) {
    return (play->msgCtx.msgMode != MSGMODE_NONE) || (play->csCtx.state != CS_STATE_IDLE) ||
           (play->transitionTrigger != TRANS_TRIGGER_OFF) || (play->transitionMode != TRANS_MODE_OFF) ||
           EnBen_MinigameRunning() || Ben_IsMelodyPlaying() || EnBen_ChildrenHere(this, play);
}

/**
 * Whether you're looking straight at the figure: in plain view, near the middle of the screen.
 */
s32 EnBen_FigureLookedAt(EnBen* this, PlayState* play) {
    Vec3f chest;
    Vec3f projected;
    f32 invW;

    if (!EnBen_IsBodyVisible(play, &this->actor.world.pos)) {
        return false;
    }
    chest = this->actor.world.pos;
    chest.y += 40.0f;
    Actor_GetProjectedPos(play, &chest, &projected, &invW);
    return (projected.z > 0.0f) && (fabsf(projected.x * invW) < EN_BEN_TERMINA_FIGURE_MIDDLE);
}

/**
 * Somewhere a long way off, a little to one side of where you're looking, in plain view if you look.
 */
s32 EnBen_TryPlaceFigure(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) +
              ((Rand_ZeroOne() < 0.5f) ? EN_BEN_TERMINA_FIGURE_ANGLE : -EN_BEN_TERMINA_FIGURE_ANGLE);
    Vec3f pos;
    s32 d;

    for (d = 0; d < ARRAY_COUNT(sDistanceScales); d++) {
        if (EnBen_FindSpot(play, player, yaw, EN_BEN_TERMINA_FIGURE_DIST * sDistanceScales[d], &pos) &&
            EnBen_IsBodyVisible(play, &pos)) {
            EnBen_PlaceAt(this, play, &pos);
            return true;
        }
    }
    return false;
}

/**
 * Starts the next thing. Returns false if it couldn't (nobody to stare, nowhere to stand), so another can be tried.
 */
s32 EnBen_TerminaStart(EnBen* this, PlayState* play, s32 event) {
    Player* player = GET_PLAYER(play);
    s16 behindYaw;
    Vec3f pos;
    Vec3f projected;
    f32 invW;

    this->directorTimer = 0;
    switch (event) {
        case EN_BEN_TERMINA_STARE:
            this->other = EnBen_FindPerson(play, EN_BEN_TERMINA_STARE_DIST, true);
            if (this->other == NULL) {
                return false;
            }
            break;

        case EN_BEN_TERMINA_FIGURE:
            if (!EnBen_TryPlaceFigure(this, play)) {
                return false;
            }
            this->isHidden = false;
            this->seenFrames = 0;
            break;

        case EN_BEN_TERMINA_STEPS:
            this->timer = EN_BEN_STEPS_BEFORE; // EnBen_Footsteps counts this down
            this->stepTimer = 0;
            break;

        case EN_BEN_TERMINA_LAUGH:
            behindYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
            pos.x = player->actor.world.pos.x + (Math_SinS(behindYaw) * EN_BEN_TERMINA_LAUGH_DIST);
            pos.y = player->actor.world.pos.y;
            pos.z = player->actor.world.pos.z + (Math_CosS(behindYaw) * EN_BEN_TERMINA_LAUGH_DIST);
            Actor_GetProjectedPos(play, &pos, &projected, &invW);
            this->stepPos = projected;
            Audio_PlaySfx_AtPosWithPresetLowFreqAndHighReverb(&this->stepPos, EN_BEN_LAUGH_SFX);
            return true; // that's all it is

        case EN_BEN_TERMINA_MUSIC:
            if (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_DISABLED) {
                return false; // nothing playing to go wrong
            }
            SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, EN_BEN_TERMINA_MUSIC_TEMPO);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, EN_BEN_TERMINA_MUSIC_FREQ);
            this->musicWarped = true;
            break;

        case EN_BEN_TERMINA_GREY_OUT:
            break;

        default:
            return false;
    }
    this->directorStep = event;
    return true;
}

/**
 * Picks one of the session's things at random, trying the rest in turn if it can't happen here and now.
 */
void EnBen_TerminaPick(EnBen* this, PlayState* play) {
    u8* events = (Ben_GetSession() >= BEN_SESSION_COLD) ? sTerminaLate : sTerminaEarly;
    s32 count = (Ben_GetSession() >= BEN_SESSION_COLD) ? ARRAY_COUNT(sTerminaLate) : ARRAY_COUNT(sTerminaEarly);
    s32 first = (s32)Rand_ZeroFloat(count) % count;
    s32 i;

    for (i = 0; i < count; i++) {
        if (EnBen_TerminaStart(this, play, events[(first + i) % count])) {
            return;
        }
    }
}

void EnBen_TerminaEnd(EnBen* this) {
    this->isHidden = true;
    this->other = NULL;
    this->directorStep = EN_BEN_TERMINA_WAIT;
    this->timer = EnBen_TerminaWait();
}

void EnBen_Termina(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    this->directorTimer++;

    // The colour comes back on its own, whatever happens next
    if ((this->directorStep != EN_BEN_TERMINA_GREY_OUT) && (this->grey > 0) && !EnBen_ChildrenHere(this, play)) {
        Math_StepToS(&this->grey, 0, EN_BEN_TERMINA_GREY_STEP);
        gPlayVisMonoColor.a = this->grey;
    }

    switch (this->directorStep) {
        case EN_BEN_TERMINA_WAIT:
            if (this->timer > 0) {
                this->timer--;
                break;
            }
            if (EnBen_TerminaBusy(this, play)) {
                this->timer = EN_BEN_TERMINA_RETRY;
                break;
            }
            this->timer = EnBen_TerminaWait(); // if nothing at all can happen here, wait as long again
            EnBen_TerminaPick(this, play);
            break;

        case EN_BEN_TERMINA_STARE: // somebody stops, and looks at you
            if (!EnBen_IsStillHere(play, this->other) || (this->directorTimer >= EN_BEN_TERMINA_STARE_FRAMES) ||
                (play->msgCtx.msgMode != MSGMODE_NONE)) {
                EnBen_TerminaEnd(this);
                break;
            }
            this->other->shape.rot.y = this->other->world.rot.y =
                Actor_WorldYawTowardActor(this->other, &player->actor);
            this->other->freezeTimer = 2;
            break;

        case EN_BEN_TERMINA_FIGURE: // standing out there; look at it, and it isn't
            if (EnBen_FigureLookedAt(this, play)) {
                this->seenFrames++;
            }
            if ((this->seenFrames >= EN_BEN_TERMINA_FIGURE_SEEN) ||
                (this->directorTimer >= EN_BEN_TERMINA_FIGURE_LONGEST) || EnBen_TerminaBusy(this, play)) {
                EnBen_TerminaEnd(this); // no laugh, no smear: was it ever there?
            }
            break;

        case EN_BEN_TERMINA_STEPS: // closer, closer, and then nothing
            EnBen_Footsteps(this, play);
            if (this->timer > 0) {
                this->timer--;
            } else {
                EnBen_TerminaEnd(this);
            }
            break;

        case EN_BEN_TERMINA_MUSIC:
            if ((this->directorTimer >= EN_BEN_TERMINA_MUSIC_FRAMES) || EnBen_TerminaBusy(this, play)) {
                SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, 100);
                SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, 1000);
                this->musicWarped = false;
                EnBen_TerminaEnd(this);
            }
            break;

        case EN_BEN_TERMINA_GREY_OUT:
            Math_StepToS(&this->grey, EN_BEN_TERMINA_GREY, EN_BEN_TERMINA_GREY_STEP);
            gPlayVisMonoColor.r = 255;
            gPlayVisMonoColor.g = 255;
            gPlayVisMonoColor.b = 255;
            gPlayVisMonoColor.a = this->grey;
            if ((this->directorTimer >= EN_BEN_TERMINA_GREY_FRAMES) || EnBen_TerminaBusy(this, play)) {
                EnBen_TerminaEnd(this); // and it fades back in above
            }
            break;

        default:
            EnBen_TerminaEnd(this);
            break;
    }
}
