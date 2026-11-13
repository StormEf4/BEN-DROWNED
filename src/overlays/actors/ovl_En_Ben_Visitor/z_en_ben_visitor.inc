/**
 * BEN DROWNED, the Expanded Horror: visitors from other stories (BEN_EXPANDED; DECISIONS D17).
 *
 * Each has an actor of its own (En_Ben_Visitor for the Tall Man, En_Ben_Smile, En_Ben_Sleep), so only the model of
 * the one that's here is in memory. Each includes this once, after its models and with EN_BEN_VISITOR set to its
 * BEN_VISITOR_*, and defines its profile on the functions here; everything here is static, since all the overlays link
 * into one ELF.
 *
 * The Termina director (En_Ben, BEN_ACTOR_TERMINA, z_en_ben_termina.inc) sends one now and then, from Session 3 on,
 * wherever the story has nothing waiting. Each has its own rule, and none of them is BEN: none of it counts as a
 * sighting or moves the story on. They aren't named in the game.
 *  - The Tall Man (BEN_VISITOR_TALL), in the woods and the fields. Looking at him fills the screen with static, faster
 *    the closer he is. Look away and he's somewhere else, closer. Let the static fill the screen, or stare at him from
 *    up close, and he has you: you wake at the way in, a heart down. Keep your head for a minute and a half and he's
 *    gone.
 *  - The Smiling Man (BEN_VISITOR_SMILE), in Clock Town's streets and the road to the ranch, at night. He dances
 *    towards you down the street, slowly while you watch him, quickly while you don't. Get far enough away, or out of
 *    his sight long enough, and his smile drops and he walks off. If he reaches you: black, and two hearts.
 *  - "Go to sleep." (BEN_VISITOR_SLEEP), indoors at night. The lights go out, and come back, again and again. He only
 *    moves in the dark, and in the dark his face is the only thing you can see. Leave the room, or he reaches you: the
 *    clock jumps an hour on, and you're a heart down.
 * None of them can kill Link. Whatever they do to the screen, the lights or the music is put back when they go.
 */

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.h"
#include "ben/ben.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_LOCK_ON_DISABLED)

#define VISITOR_PLACE_GIVE_UP 60 // if there's nowhere to stand for this long after it's sent, it doesn't come
#define VISITOR_LOOKED_AT 0.45f  // looking straight at it: it's this near the middle of the screen
#define VISITOR_STEP_SFX NA_SE_PL_WALK_GROUND

#define TALL_FIRST_DIST 1300.0f // where the Tall Man stands the first time
#define TALL_CLOSER 0.72f       // each time he moves, this much of the way he was
#define TALL_CLOSEST 280.0f     // and never closer than this
#define TALL_MOVE_UNSEEN 50     // out of your sight this long after you've seen him, and he's somewhere closer
#define TALL_MOVE_UNNOTICED 300 // if you never find him, he comes closer anyway
#define TALL_LONGEST 1800       // after this long (90 s), the next time you look away he's gone
#define TALL_NOISE_SEEN 3       // static per frame you look straight at him
#define TALL_NOISE_NEAR 7       // plus up to this much more the closer he is
#define TALL_NOISE_EDGE 1       // and per frame he's anywhere on screen
#define TALL_NOISE_FADE 2       // and it fades this fast while he isn't
#define TALL_NOISE_SOUND 40     // above this you hear it
#define TALL_GRAB_DIST 360.0f   // stare at him this close
#define TALL_GRAB_FRAMES 25     // for this long, and he has you
#define TALL_TENDRILS 150       // static above this, and you see what's on his back
#define TALL_CAUGHT_SHOW 20     // caught: he's right there this long
#define TALL_CAUGHT_BLACK 40    // then black over this long
#define TALL_CAUGHT_VOID 70     // and then you're back at the way in
#define TALL_CAUGHT_DIST 110.0f // how close he is
#define TALL_DAMAGE 0x10        // a heart

#define SMILE_FIRST_DIST 1100.0f // the Smiling Man starts a long way down the street
#define SMILE_SPEED_SEEN 1.4f    // dancing towards you while you watch
#define SMILE_SPEED_UNSEEN 4.5f  // and while you don't
#define SMILE_HOP_SPEED 0x900    // the dance's beat
#define SMILE_SWAY 0x2400        // each hop goes this far to the side
#define SMILE_HOP_HEIGHT 1400.0f // and this high (model units)
#define SMILE_CATCH 75.0f        // this close and he has you
#define SMILE_ESCAPE 1900.0f     // this far and you got away
#define SMILE_LOST 100           // or out of his sight this long (5 s)
#define SMILE_LOST_NEAR 450.0f   // while he isn't this close
#define SMILE_STUCK 60           // nowhere to dance for this long, and he gives up
#define SMILE_LONGEST 1500       // and after this long anyway (75 s)
#define SMILE_LEAVE_SPEED 2.0f   // walking away
#define SMILE_LEAVE_LONGEST 120  // gone once you look away after this long
#define SMILE_LEAVE_LATEST 400   // or after this long, whatever you do
#define SMILE_CAUGHT_LEAN 25     // caught: his face in yours this long
#define SMILE_CAUGHT_BLACK 40    // then black this long
#define SMILE_DAMAGE 0x20        // two hearts

#define SLEEP_FIRST_DIST 650.0f // he's somewhere in the room you aren't looking
#define SLEEP_LIGHT_FIRST 90    // the lights go out this long after he's there
#define SLEEP_LIGHT 110         // then they're on this long
#define SLEEP_LIGHT_RANGE 60    // plus up to this much
#define SLEEP_LIGHT_SHORTER 15  // a little less every time
#define SLEEP_LIGHT_SHORTEST 40
#define SLEEP_DARK 70       // and out this long
#define SLEEP_DARK_RANGE 40 // plus up to this much
#define SLEEP_DARK_STEP 8   // going out and coming on this fast (0-255 a frame)
#define SLEEP_FLICKER 12    // Screen flashes on: the lights stutter for this long as they go
#define SLEEP_MOVES 200     // he moves once it's this dark
#define SLEEP_STEP_EVERY 22 // a step this often
#define SLEEP_STEP 75.0f    // this long
#define SLEEP_CATCH 85.0f   // this close in the dark and he has you
#define SLEEP_LONGEST 1500  // after this long (75 s), the next time the lights come back he isn't there
#define SLEEP_FOG_NEAR 900  // in the dark the fog starts this near (0-1000)
#define SLEEP_BLACK 60      // after "Go to sleep.", black this long
#define SLEEP_DAMAGE 0x10   // a heart
#define SLEEP_CLICK NA_SE_EV_FOOT_SWITCH

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_kit.inc"

static void EnBenVisitor_Init(Actor* thisx, PlayState* play);
static void EnBenVisitor_Destroy(Actor* thisx, PlayState* play);
static void EnBenVisitor_Update(Actor* thisx, PlayState* play);
static void EnBenVisitor_Draw(Actor* thisx, PlayState* play);

#if EN_BEN_VISITOR == BEN_VISITOR_TALL
static void EnBenVisitor_Tall(EnBenVisitor* this, PlayState* play);
static void EnBenVisitor_TallCaught(EnBenVisitor* this, PlayState* play);
#elif EN_BEN_VISITOR == BEN_VISITOR_SMILE
static void EnBenVisitor_Smile(EnBenVisitor* this, PlayState* play);
static void EnBenVisitor_SmileLeave(EnBenVisitor* this, PlayState* play);
static void EnBenVisitor_SmileCaught(EnBenVisitor* this, PlayState* play);
#else
static void EnBenVisitor_Sleep(EnBenVisitor* this, PlayState* play);
static void EnBenVisitor_SleepCaught(EnBenVisitor* this, PlayState* play);
#endif

// How tall each of them is, for whether you can see them
static f32 sVisitorHeights[] = { 89.0f, 64.0f, 50.0f };

/**
 * The lights, `dark` of the way out (0-255): the fog black and close, and nothing lit.
 */
static void EnBenVisitor_SetDark(PlayState* play, s16 dark) {
    EnvironmentContext* envCtx = &play->envCtx;
    f32 d = dark / 255.0f;
    s32 i;

    envCtx->adjLightSettings.fogNear = (SLEEP_FOG_NEAR - envCtx->lightSettings.fogNear) * d;
    for (i = 0; i < 3; i++) {
        envCtx->adjLightSettings.fogColor[i] = -envCtx->lightSettings.fogColor[i] * d;
        envCtx->adjLightSettings.ambientColor[i] = -envCtx->lightSettings.ambientColor[i] * d;
        envCtx->adjLightSettings.light1Color[i] = -envCtx->lightSettings.light1Color[i] * d;
        envCtx->adjLightSettings.light2Color[i] = -envCtx->lightSettings.light2Color[i] * d;
    }
}

#if EN_BEN_VISITOR == BEN_VISITOR_TALL
/**
 * The Tall Man's static over everything, `noise` of the way (0-255). It flickers only with Screen flashes on.
 */
static void EnBenVisitor_ShowNoise(PlayState* play, s16 noise) {
    s32 grey;
    s32 alpha;

    if (noise <= 0) {
        Play_FillScreen(play, false, 0, 0, 0, 0);
        gPlayVisMonoColor.a = 0;
        return;
    }
    gPlayVisMonoColor.r = 255;
    gPlayVisMonoColor.g = 255;
    gPlayVisMonoColor.b = 255;
    gPlayVisMonoColor.a = noise * 4 / 5; // the colour goes first
    if (BEN_FLASHES) {
        grey = 40 + (s32)Rand_ZeroFloat(180.0f);
        alpha = (noise / 2) + (s32)Rand_ZeroFloat(noise / 4 + 1);
    } else {
        grey = 110;
        alpha = noise * 2 / 5;
    }
    Play_FillScreen(play, true, grey, grey, grey, CLAMP_MAX(alpha, 255));
    if (noise > TALL_NOISE_SOUND) {
        Audio_PlaySfx(NA_SE_EV_SAND_STORM - SFX_FLAG);
    }
}
#endif

/**
 * Everything it did to you, put back.
 */
static void EnBenVisitor_Restore(EnBenVisitor* this, PlayState* play) {
    EnBenVisitor_Release(this, play);
    Play_FillScreen(play, false, 0, 0, 0, 0);
    if (this->noise > 0) {
        gPlayVisMonoColor.a = 0;
        this->noise = 0;
    }
    if ((this->actor.params == BEN_VISITOR_TALL) && this->isNoticed) {
        Audio_SetMainBgmVolume(0x7F, 20); // the music he took
    }
    if (this->dark > 0) {
        EnBenVisitor_SetDark(play, 0);
        this->dark = 0;
    }
}

static void EnBenVisitor_Leave(EnBenVisitor* this, PlayState* play) {
    EnBenVisitor_Restore(this, play);
    Actor_Kill(&this->actor);
}

static void EnBenVisitor_Init(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    if (this->actor.params != EN_BEN_VISITOR) {
        Actor_Kill(&this->actor);
        return;
    }
    this->height = sVisitorHeights[this->actor.params];
    this->isHidden = true;
    this->placed = false;
    this->timer = 0;
    this->life = 0;
    this->noise = 0;
    this->dark = 0;
    this->isDark = false;
    this->holding = false;
    this->frowning = false;
    this->dist = TALL_FIRST_DIST;
#if EN_BEN_VISITOR == BEN_VISITOR_TALL
    this->actionFunc = EnBenVisitor_Tall;
#elif EN_BEN_VISITOR == BEN_VISITOR_SMILE
    this->actionFunc = EnBenVisitor_Smile;
#else
    this->actionFunc = EnBenVisitor_Sleep;
#endif
}

static void EnBenVisitor_Destroy(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    EnBenVisitor_Restore(this, play);
}

#if EN_BEN_VISITOR == BEN_VISITOR_TALL
// ---------------------------------------------------------------------------------------------------------------------
// The Tall Man

static void EnBenVisitor_Tall(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist;
    s32 inView;
    s32 looking;

    if (!EnBenVisitor_Arrive(this, play, this->dist, false)) {
        return;
    }
    if (EnBenVisitor_Busy(play)) {
        EnBenVisitor_Leave(this, play);
        return;
    }

    dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);
    inView = EnBenVisitor_Seen(this, play, &this->actor.world.pos, 1.0f);
    looking = inView && EnBenVisitor_Seen(this, play, &this->actor.world.pos, VISITOR_LOOKED_AT);

    if (inView) {
        if (!this->isNoticed) {
            this->isNoticed = true;
            Audio_SetMainBgmVolume(0, 10); // the music goes the moment you see him
        }
        this->unseenFrames = 0;
        if (looking) {
            this->noise += TALL_NOISE_SEEN + (s32)(TALL_NOISE_NEAR * CLAMP(1.0f - dist / TALL_FIRST_DIST, 0.0f, 1.0f));
        } else {
            this->noise += TALL_NOISE_EDGE;
        }
    } else {
        this->unseenFrames++;
        this->noise -= TALL_NOISE_FADE;
    }
    this->noise = CLAMP(this->noise, 0, 255);
    this->seenFrames = (looking && (dist < TALL_GRAB_DIST)) ? (this->seenFrames + 1) : 0;
    EnBenVisitor_ShowNoise(play, this->noise);

    if ((this->noise >= 255) || (this->seenFrames >= TALL_GRAB_FRAMES)) {
        // He has you
        this->timer = 0;
        EnBenVisitor_Hold(this, play);
        this->actionFunc = EnBenVisitor_TallCaught;
        return;
    }

    if ((this->isNoticed && (this->unseenFrames >= TALL_MOVE_UNSEEN)) ||
        (!this->isNoticed && (this->timer >= TALL_MOVE_UNNOTICED))) {
        if (this->life >= TALL_LONGEST) {
            EnBenVisitor_Leave(this, play); // and the static goes with him
            return;
        }
        // Somewhere else, closer. If there's nowhere, he stays where he was.
        this->dist = CLAMP_MIN(this->dist * TALL_CLOSER, TALL_CLOSEST);
        EnBenVisitor_TryPlace(this, play, this->dist, false);
        this->unseenFrames = 0;
        this->timer = 0;
    }
}

static void EnBenVisitor_TallCaught(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 t = this->timer;
    s32 shade;

    if (t == 1) {
        // Right in front of you
        if (EnBenVisitor_FindSpot(play, &player->actor.world.pos, Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)),
                                  TALL_CAUGHT_DIST, &pos)) {
            EnBenVisitor_PlaceAt(this, play, &pos);
        }
        this->noise = TALL_TENDRILS + 20;
    }
    if (t < TALL_CAUGHT_SHOW) {
        EnBenVisitor_ShowNoise(play, this->noise);
    } else if (t < (TALL_CAUGHT_SHOW + TALL_CAUGHT_BLACK)) {
        shade = 120 * (TALL_CAUGHT_SHOW + TALL_CAUGHT_BLACK - t) / TALL_CAUGHT_BLACK;
        if (BEN_FLASHES) {
            shade = (s32)Rand_ZeroFloat(shade + 1);
        }
        Play_FillScreen(play, true, shade, shade, shade, 255 - shade);
        Audio_PlaySfx(NA_SE_EV_SAND_STORM - SFX_FLAG);
    } else if (t == TALL_CAUGHT_VOID) {
        Play_FillScreen(play, true, 0, 0, 0, 255);
        EnBenVisitor_Hurt(play, TALL_DAMAGE);
        EnBenVisitor_Release(this, play);
        func_80169EFC(play); // back to the way in, as if you'd fallen
        this->isHidden = true;
    }
}

#elif EN_BEN_VISITOR == BEN_VISITOR_SMILE
// ---------------------------------------------------------------------------------------------------------------------
// The Smiling Man

/**
 * The dance: up and down on the beat, and leaning into each hop.
 */
static void EnBenVisitor_Dance(EnBenVisitor* this, f32 amount) {
    s16 beat = this->life * SMILE_HOP_SPEED;
    f32 hop = Math_SinS(beat);

    this->actor.shape.yOffset = fabsf(Math_CosS(beat)) * SMILE_HOP_HEIGHT * amount;
    this->actor.shape.rot.z = (s16)(hop * 0x900 * amount);
    this->headRoll = (s16)(hop * 0xC00 * amount);
    if ((amount > 0.0f) && ((u16)((beat + 0x4000) & 0x7FFF) < SMILE_HOP_SPEED)) {
        Actor_PlaySfx(&this->actor, VISITOR_STEP_SFX); // landing (the bottom of each hop)
    }
}

static void EnBenVisitor_Smile(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist;
    s16 yaw;
    s32 inView;

    if (!EnBenVisitor_Arrive(this, play, SMILE_FIRST_DIST, true)) {
        return;
    }
    if (EnBenVisitor_Busy(play)) {
        EnBenVisitor_Leave(this, play);
        return;
    }

    dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);
    if (dist < SMILE_CATCH) {
        this->timer = 0;
        EnBenVisitor_Hold(this, play);
        this->actionFunc = EnBenVisitor_SmileCaught;
        return;
    }
    inView = EnBenVisitor_Seen(this, play, &this->actor.world.pos, 1.0f);
    this->unseenFrames = inView ? 0 : (this->unseenFrames + 1);

    if ((dist > SMILE_ESCAPE) || ((this->unseenFrames >= SMILE_LOST) && (dist > SMILE_LOST_NEAR)) ||
        (this->stuck >= SMILE_STUCK) || (this->life >= SMILE_LONGEST)) {
        // You got away. The smile drops, and he walks off.
        this->frowning = true;
        this->timer = 0;
        this->actionFunc = EnBenVisitor_SmileLeave;
        return;
    }

    yaw = Actor_WorldYawTowardActor(&this->actor, &player->actor);
    if (EnBenVisitor_Step(this, play, yaw + (s16)(Math_SinS(this->life * SMILE_HOP_SPEED) * SMILE_SWAY),
                          inView ? SMILE_SPEED_SEEN : SMILE_SPEED_UNSEEN) ||
        EnBenVisitor_Step(this, play, yaw, inView ? SMILE_SPEED_SEEN : SMILE_SPEED_UNSEEN)) {
        this->stuck = 0;
    } else {
        this->stuck++;
    }
    this->actor.shape.rot.y = this->actor.world.rot.y = yaw;
    EnBenVisitor_Dance(this, 1.0f);
}

static void EnBenVisitor_SmileLeave(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yaw = Actor_WorldYawTowardActor(&this->actor, &player->actor) + 0x8000;

    if (EnBenVisitor_Busy(play) || (this->timer >= SMILE_LEAVE_LATEST) ||
        ((this->timer >= SMILE_LEAVE_LONGEST) && !EnBenVisitor_Seen(this, play, &this->actor.world.pos, 1.0f))) {
        EnBenVisitor_Leave(this, play);
        return;
    }
    EnBenVisitor_Step(this, play, yaw, SMILE_LEAVE_SPEED);
    Math_ScaledStepToS(&this->actor.shape.rot.y, yaw, 0x400);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnBenVisitor_Dance(this, 0.0f);
}

static void EnBenVisitor_SmileCaught(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->timer < SMILE_CAUGHT_LEAN) {
        // His face in yours, his head going over to one side
        this->actor.shape.rot.y = this->actor.world.rot.y = Actor_WorldYawTowardActor(&this->actor, &player->actor);
        this->actor.shape.yOffset = 0.0f;
        this->actor.shape.rot.z = 0;
        Math_ScaledStepToS(&this->headRoll, 0x2000, 0x200);
    } else if (this->timer == SMILE_CAUGHT_LEAN) {
        Play_FillScreen(play, true, 0, 0, 0, 255);
        EnBenVisitor_Hurt(play, SMILE_DAMAGE);
        this->isHidden = true;
    } else if (this->timer >= (SMILE_CAUGHT_LEAN + SMILE_CAUGHT_BLACK)) {
        EnBenVisitor_Leave(this, play); // and he's nowhere
    }
}

#else
// ---------------------------------------------------------------------------------------------------------------------
// Go to sleep

/**
 * The lights towards on or out. With Screen flashes on they stutter as they go.
 */
static void EnBenVisitor_UpdateLights(EnBenVisitor* this, PlayState* play) {
    s16 target = this->isDark ? 255 : 0;
    s16 shown;

    Math_StepToS(&this->dark, target, SLEEP_DARK_STEP);
    shown = this->dark;
    if (BEN_FLASHES && (this->dark != target) && (this->timer < SLEEP_FLICKER) && ((this->timer / 2) % 2)) {
        shown = 255 - target; // a moment of how it was
    }
    EnBenVisitor_SetDark(play, shown);
}

static void EnBenVisitor_Sleep(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist;
    f32 step;
    Vec3f pos;
    s16 yaw;

    if (!this->placed) {
        EnBenVisitor_Arrive(this, play, SLEEP_FIRST_DIST, false);
        this->phase = SLEEP_LIGHT_FIRST;
        return;
    }
    if (EnBenVisitor_Busy(play)) {
        EnBenVisitor_Leave(this, play);
        return;
    }

    EnBenVisitor_UpdateLights(this, play);
    if (this->frowning) {
        // The lights are back, and he isn't
        if (this->dark == 0) {
            EnBenVisitor_Leave(this, play);
        }
        return;
    }

    this->phase--;
    if (this->phase <= 0) {
        Audio_PlaySfx(SLEEP_CLICK);
        this->timer = 0;
        if (this->isDark) {
            this->isDark = false;
            this->phase =
                CLAMP_MIN(SLEEP_LIGHT + (s32)Rand_ZeroFloat(SLEEP_LIGHT_RANGE) - (this->stuck * SLEEP_LIGHT_SHORTER),
                          SLEEP_LIGHT_SHORTEST);
            if (this->life >= SLEEP_LONGEST) {
                this->isHidden = true;
                this->frowning = true;
            }
        } else {
            this->isDark = true;
            this->phase = SLEEP_DARK + (s32)Rand_ZeroFloat(SLEEP_DARK_RANGE);
            this->stuck++; // how many times they've gone out
        }
    }

    if (!this->isDark || (this->dark < SLEEP_MOVES)) {
        return; // in the light, he never moves
    }
    yaw = Actor_WorldYawTowardActor(&this->actor, &player->actor);
    this->actor.shape.rot.y = this->actor.world.rot.y = yaw;
    dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);
    if (dist < SLEEP_CATCH) {
        this->timer = 0;
        EnBenVisitor_SetDark(play, 255);
        EnBenVisitor_Hold(this, play);
        Message_StartTextbox(play, BEN_TEXT_SLEEP, &this->actor);
        this->actionFunc = EnBenVisitor_SleepCaught;
        return;
    }
    if ((this->timer % SLEEP_STEP_EVERY) == 0) {
        step = CLAMP_MAX(SLEEP_STEP, dist - (SLEEP_CATCH * 0.5f));
        if (EnBenVisitor_FindSpot(play, &this->actor.world.pos, yaw, step, &pos)) {
            this->actor.world.pos = pos;
        }
    }
}

/**
 * The hour he takes: on an hour, unless it would run into dawn (the day changes there) or it's the last night.
 */
static void EnBenVisitor_TakeAnHour(void) {
    u16 time = CURRENT_TIME;

    if ((CURRENT_DAY >= 3) && (time < CLOCK_TIME(6, 0))) {
        return;
    }
    if ((time >= CLOCK_TIME(18, 0)) || (time < CLOCK_TIME(4, 30))) {
        gSaveContext.save.time = (u16)(time + CLOCK_TIME(1, 0));
    }
}

static void EnBenVisitor_SleepCaught(EnBenVisitor* this, PlayState* play) {
    EnBenVisitor_SetDark(play, 255);
    if (this->isHidden) {
        // In the black
        if (this->timer >= SLEEP_BLACK) {
            EnBenVisitor_Leave(this, play); // the lights come back on, and you've lost an hour
        }
        return;
    }
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
        Play_FillScreen(play, true, 0, 0, 0, 255);
        EnBenVisitor_Hurt(play, SLEEP_DAMAGE);
        EnBenVisitor_TakeAnHour();
        this->isHidden = true;
        this->timer = 0;
    }
}

#endif

static void EnBenVisitor_Update(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    this->timer++;
    if (this->placed) {
        this->life++;
    }
    this->actionFunc(this, play);
}

static void EnBenVisitor_Draw(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    if (this->isHidden) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sVisitorSetupDL);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

#if EN_BEN_VISITOR == BEN_VISITOR_TALL
    gSPDisplayList(POLY_OPA_DISP++, sTallManDL);
    if (this->noise >= TALL_TENDRILS) {
        gSPDisplayList(POLY_OPA_DISP++, sTallTendrilsDL);
    }
#elif EN_BEN_VISITOR == BEN_VISITOR_SMILE
    gSPDisplayList(POLY_OPA_DISP++, sSmileBodyDL);
    Matrix_Translate(0.0f, SMILE_MAN_NECK_Y, 0.0f, MTXMODE_APPLY);
    Matrix_RotateZS(this->headRoll, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sSmileHeadDL);
    gSPDisplayList(POLY_OPA_DISP++, this->frowning ? sSmileFrownDL : sSmileGrinDL);
#else
    gSPDisplayList(POLY_OPA_DISP++, sSleepBodyDL);
    Matrix_Translate(0.0f, SLEEP_MAN_NECK_Y, 0.0f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sSleepHairDL);
    // His face isn't lit or fogged: in the dark it's all there is
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sVisitorFaceSetupDL);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sSleepFaceDL);
#endif

    CLOSE_DISPS(play->state.gfxCtx);
}
