/**
 * BEN DROWNED, the Expanded Horror: three of the SCP Foundation's (BEN_EXPANDED; DECISIONS D18).
 *
 * Each has an actor of its own (En_Ben_Scp173, En_Ben_Scp096, En_Ben_Scp106), so only the model of the one that's
 * here is in memory. Each includes this once, after its models and with EN_BEN_SCP set to its BEN_VISITOR_*, and
 * defines its profile on the functions here; everything here is static, since all the overlays link into one ELF.
 *
 * Sent by the Termina director (En_Ben, z_en_ben_termina.inc) the way the visitors from other stories are, once in a
 * while, where each belongs; nothing else happens while one is here, and none of it is a sighting. Each keeps its
 * article's rule (the SCP wiki, CC BY-SA 3.0: docs/ben/CREDITS.md). They aren't named in the game.
 *  - SCP-173 (BEN_VISITOR_SCULPTURE), in dungeons and dark places. Concrete and rebar, flecks of spray paint. It can't
 *    move while you can see it, and you can Z-target it to keep your eyes on it. But Link blinks: every few seconds
 *    the screen goes black for a moment, and in that moment it's closer, with the scrape of stone. Look away and it
 *    slides after you. If it's on you when you can't see it: the snap, three hearts, and you wake at the way in.
 *  - SCP-096 (BEN_VISITOR_SHY), in the field, on the coast, in the canyon and the mountains. Very tall and thin and
 *    pale, pacing a long way off with its back to you; now and then it turns round. See its face and it covers it and
 *    screams, and you have a few seconds before it comes for you, through anything, faster the further it has to go.
 *    Get out of the place, or it has you: four hearts, and the way in. After a while it gives up and sits.
 *  - SCP-106 (BEN_VISITOR_OLD_MAN), in the swamp and the graveyard at night, and under Ikana. You hear it first. An old
 *    man, rotten black, rising out of a pool of corrosion where you weren't looking; he walks to you slowly, through
 *    walls, leaving pools behind, and sinks and comes up again closer. A light arrow near him sends him back down. If
 *    he reaches you, he takes you down with him: two hearts, and the way in.
 * None of them can kill Link. Whatever they do to the screen or the music is put back when they go.
 */

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "ben/ben.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_LOCK_ON_DISABLED)

#define VISITOR_PLACE_GIVE_UP 60 // if there's nowhere to stand for this long after it's sent, it doesn't come
#define SCP_FAR_GONE 1800.0f     // this far away and out of sight, and it's gone

#define STATUE_FIRST_DIST 700.0f // SCP-173 comes where you aren't looking, this far off
#define STATUE_BLINK_EVERY 80    // Link blinks this often
#define STATUE_BLINK_RANGE 40    // plus up to this much
#define STATUE_BLINK_SHUT 4      // Screen flashes on: the eyes shut this long, all at once
#define STATUE_BLINK_FADE 6      // off: closing and opening over this long each
#define STATUE_BLINK_HELD 4      // and shut this long between
#define STATUE_LEAP 220.0f       // in a blink, it comes this far
#define STATUE_SLIDE 10.0f       // out of your sight, this fast
#define STATUE_SLIDE_AFTER 8     // once you've looked away this long
#define STATUE_SCRAPE_EVERY 8    // the scrape, this often as it slides
#define STATUE_CATCH 55.0f       // this close while you can't see it, and it has you
#define STATUE_LONGEST 1800      // after this long (90 s), the next time you look away it's gone
#define STATUE_CAUGHT_SHOW 15    // caught: it's there in front of you this long
#define STATUE_CAUGHT_VOID 45    // and then black until you're back at the way in
#define STATUE_DAMAGE 0x30       // three hearts
#define STATUE_SCRAPE NA_SE_EV_ROCK_SLIDE
#define STATUE_SLIDE_SFX NA_SE_EV_OBJECT_SLIDE
#define STATUE_SNAP NA_SE_EV_WOODBOX_BREAK

#define SHY_FIRST_DIST 1100.0f // SCP-096 is a long way off, where you can see it
#define SHY_PACE 150.0f        // pacing this far either side of where it came
#define SHY_PACE_SPEED 0.8f
#define SHY_TURN_CHANCE 0.004f // now and then (about every 12 s) it turns round
#define SHY_TURN 70            // for this long
#define SHY_SEE_DIST 1500.0f   // you can make out its face from this far
#define SHY_FACE_ARC 0x2000    // when it's turned this near to you
#define SHY_FACE_MIDDLE 0.6f   // and it's this near the middle of the screen
#define SHY_FACE_FRAMES 4      // for this long: a glimpse is enough
#define SHY_UPSET 160          // then it screams into its hands this long (8 s) before it comes
#define SHY_SCREAM_EVERY 40
#define SHY_SPEED_DIV 60.0f // coming for you at its distance over this a frame
#define SHY_SPEED_MIN 6.5f  // at least this (Link running is about 6)
#define SHY_SPEED_MAX 16.0f // and at most this
#define SHY_CATCH 60.0f
#define SHY_CHASE_LONGEST 900 // after this long (45 s) it gives up, and sits
#define SHY_SIT_GONE 40       // and once you've looked away from it this long, it's gone
#define SHY_CALM_LONGEST 2400 // if you never see its face, it's gone after this long (2 min) once you look away
#define SHY_CAUGHT_SHOW 20
#define SHY_CAUGHT_VOID 50
#define SHY_DAMAGE 0x40 // four hearts
#define SHY_SCREAM NA_SE_EN_HALF_REDEAD_SCREAME
#define SHY_MOAN NA_SE_EN_REDEAD_WEAKENED1

#define OLD_FIRST_DIST 600.0f // SCP-106 comes up where you aren't looking
#define OLD_RISE 4            // coming up and going down this fast (0-255 a frame)
#define OLD_SPEED 2.2f        // walking to you, through anything
#define OLD_MARK_EVERY 30     // leaving a pool this often
#define OLD_SINK_EVERY 400    // going down and coming up again closer this often
#define OLD_SINK_RANGE 120    // plus up to this much
#define OLD_CLOSER 0.7f       // this much of the way he was
#define OLD_CLOSEST 250.0f
#define OLD_CATCH 60.0f
#define OLD_LONGEST 1800      // after this long (90 s), he goes down and doesn't come up
#define OLD_LIGHT_DIST 150.0f // a light arrow this close and he goes down
#define OLD_CAUGHT_DOWN 40    // caught: everything goes the colour of him over this long
#define OLD_CAUGHT_VOID 70    // and then you're back at the way in
#define OLD_DAMAGE 0x20       // two hearts
#define OLD_POOL_SCALE 0.012f // the pool he stands in
#define OLD_MARK_SCALE 0.007f // the pools he leaves
#define OLD_BUBBLE NA_SE_EV_WATER_BUBBLE
#define OLD_GROAN NA_SE_EN_REDEAD_AIM

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_kit.inc"

#if EN_BEN_SCP == BEN_VISITOR_SCULPTURE
static void EnBenScp_Statue(EnBenVisitor* this, PlayState* play);
static void EnBenScp_StatueCaught(EnBenVisitor* this, PlayState* play);
#elif EN_BEN_SCP == BEN_VISITOR_SHY
static void EnBenScp_Shy(EnBenVisitor* this, PlayState* play);
static void EnBenScp_ShyCaught(EnBenVisitor* this, PlayState* play);
#else
static void EnBenScp_OldRise(EnBenVisitor* this, PlayState* play);
static void EnBenScp_OldWalk(EnBenVisitor* this, PlayState* play);
static void EnBenScp_OldSink(EnBenVisitor* this, PlayState* play);
static void EnBenScp_OldCaught(EnBenVisitor* this, PlayState* play);
#endif

// SCP-096's moods
#define EN_BEN_SCP_SHY_CALM 0
#define EN_BEN_SCP_SHY_UPSET 1
#define EN_BEN_SCP_SHY_CHASE 2
#define EN_BEN_SCP_SHY_SIT 3

// How tall each of them is, for whether you can see them
static f32 sScpHeights[] = { 66.0f, 81.0f, 62.0f };

#if EN_BEN_SCP != BEN_VISITOR_SHY
#define EN_BEN_SCP_MARKS // it leaves something on the ground
#endif

/**
 * Everything it did to you, put back.
 */
static void EnBenScp_Restore(EnBenVisitor* this, PlayState* play) {
    EnBenVisitor_Release(this, play);
    Play_FillScreen(play, false, 0, 0, 0, 0);
    if ((this->actor.params == BEN_VISITOR_SHY) && (this->mood != EN_BEN_SCP_SHY_CALM)) {
        Audio_SetMainBgmVolume(0x7F, 20); // the music it took
    }
}

static void EnBenScp_Leave(EnBenVisitor* this, PlayState* play) {
    EnBenScp_Restore(this, play);
    Actor_Kill(&this->actor);
}

#if EN_BEN_SCP != BEN_VISITOR_SCULPTURE
/**
 * The floor under (x, z), looking down from `fromY`, or BGCHECK_Y_MIN if there isn't one.
 */
static f32 EnBenScp_FloorAt(PlayState* play, f32 x, f32 z, f32 fromY) {
    CollisionPoly* poly;
    s32 bgId;
    Vec3f pos;

    pos.x = x;
    pos.y = fromY;
    pos.z = z;
    return BgCheck_EntityRaycastFloor3(&play->colCtx, &poly, &bgId, &pos);
}

/**
 * Straight towards `target` at `speed`, through anything in the way, keeping to the floor where there is one.
 */
static void EnBenScp_Through(EnBenVisitor* this, PlayState* play, Vec3f* target, f32 speed) {
    s16 yaw = Math_Vec3f_Yaw(&this->actor.world.pos, target);
    f32 floorY;

    this->actor.world.pos.x += Math_SinS(yaw) * speed;
    this->actor.world.pos.z += Math_CosS(yaw) * speed;
    floorY = EnBenScp_FloorAt(play, this->actor.world.pos.x, this->actor.world.pos.z,
                              MAX(this->actor.world.pos.y, target->y) + 100.0f);
    if (floorY != BGCHECK_Y_MIN) {
        this->actor.world.pos.y = floorY;
    }
    this->actor.shape.rot.y = this->actor.world.rot.y = yaw;
}
#endif

#ifdef EN_BEN_SCP_MARKS
/**
 * A mark on the floor where it is now: SCP-106's corrosion, SCP-173's stain.
 */
static void EnBenScp_Mark(EnBenVisitor* this) {
    this->marks[this->markHead] = this->actor.world.pos;
    this->markHead = (this->markHead + 1) % EN_BEN_VISITOR_MARKS;
    if (this->markCount < EN_BEN_VISITOR_MARKS) {
        this->markCount++;
    }
}
#endif

static void EnBenScp_Init(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    if (this->actor.params != EN_BEN_SCP) {
        Actor_Kill(&this->actor);
        return;
    }
    this->height = sScpHeights[this->actor.params - BEN_VISITOR_SCP_FIRST];
    this->isHidden = true;
    this->placed = false;
    this->holding = false;
    this->frowning = false;
    this->timer = 0;
    this->life = 0;
    this->phase = 0;
    this->rise = 0;
    this->mood = EN_BEN_SCP_SHY_CALM;
    this->markHead = 0;
    this->markCount = 0;
    this->blink = STATUE_BLINK_EVERY + (s32)Rand_ZeroFloat(STATUE_BLINK_RANGE);
    this->dist = OLD_FIRST_DIST;
#if EN_BEN_SCP == BEN_VISITOR_SCULPTURE
    // You can keep your eyes on it with Z
    this->actor.flags &= ~ACTOR_FLAG_LOCK_ON_DISABLED;
    this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    this->actor.attentionRangeType = ATTENTION_RANGE_5;
    this->actionFunc = EnBenScp_Statue;
#elif EN_BEN_SCP == BEN_VISITOR_SHY
    this->actionFunc = EnBenScp_Shy;
#else
    this->actionFunc = EnBenScp_OldRise;
#endif
}

static void EnBenScp_Destroy(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    EnBenScp_Restore(this, play);
}

#if EN_BEN_SCP == BEN_VISITOR_SCULPTURE
// ---------------------------------------------------------------------------------------------------------------------
// SCP-173

/**
 * Link's eyes: shut, opening or closing (`phase` frames into a blink). Returns true while they're all the way shut.
 */
static s32 EnBenScp_Blink(EnBenVisitor* this, PlayState* play) {
    s32 t = this->phase;
    s32 shade;

    if (BEN_FLASHES) {
        if (t >= STATUE_BLINK_SHUT) {
            this->phase = 0;
            Play_FillScreen(play, false, 0, 0, 0, 0);
            return false;
        }
        this->phase++;
        Play_FillScreen(play, true, 0, 0, 0, 255);
        return true;
    }
    // Without Screen flashes, the eyes close and open
    if (t >= (STATUE_BLINK_FADE * 2 + STATUE_BLINK_HELD)) {
        this->phase = 0;
        Play_FillScreen(play, false, 0, 0, 0, 0);
        return false;
    }
    this->phase++;
    if (t < STATUE_BLINK_FADE) {
        shade = 255 * (t + 1) / (STATUE_BLINK_FADE + 1);
    } else if (t < (STATUE_BLINK_FADE + STATUE_BLINK_HELD)) {
        shade = 255;
    } else {
        shade = 255 * (STATUE_BLINK_FADE * 2 + STATUE_BLINK_HELD - t) / (STATUE_BLINK_FADE + 1);
    }
    Play_FillScreen(play, true, 0, 0, 0, shade);
    return shade >= 255;
}

/**
 * In a blink: closer, all at once, facing you. Returns true if that's close enough to have you.
 */
static s32 EnBenScp_Leap(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);
    s16 yaw = Actor_WorldYawTowardActor(&this->actor, &player->actor);
    f32 leap = CLAMP_MAX(STATUE_LEAP, dist - (STATUE_CATCH * 0.6f));
    Vec3f pos;

    if ((leap > 0.0f) && (EnBenVisitor_FindSpot(play, &this->actor.world.pos, yaw, leap, &pos) ||
                          EnBenVisitor_FindSpot(play, &this->actor.world.pos, yaw, leap * 0.5f, &pos))) {
        EnBenVisitor_PlaceAt(this, play, &pos);
        Actor_PlaySfx(&this->actor, STATUE_SCRAPE);
    }
    return Actor_WorldDistXZToActor(&this->actor, &player->actor) < STATUE_CATCH;
}

static void EnBenScp_Statue(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 shut = false;
    s32 seen;
    f32 dist;
    s16 yaw;

    if (!EnBenVisitor_Arrive(this, play, STATUE_FIRST_DIST, false)) {
        return;
    }
    if (this->markCount == 0) {
        EnBenScp_Mark(this); // the stain where it was standing
    }
    if (EnBenVisitor_Busy(play)) {
        EnBenScp_Leave(this, play);
        return;
    }
    this->actor.focus.pos = this->actor.world.pos;
    this->actor.focus.pos.y += this->height * 0.8f;

    // Link blinks
    if (this->phase > 0) {
        shut = EnBenScp_Blink(this, play);
    } else if (--this->blink <= 0) {
        this->blink = STATUE_BLINK_EVERY + (s32)Rand_ZeroFloat(STATUE_BLINK_RANGE);
        shut = EnBenScp_Blink(this, play);
    }
    if (shut && (this->phase == (BEN_FLASHES ? 1 : (STATUE_BLINK_FADE + 1)))) {
        // The first moment the eyes are all the way shut
        if (EnBenScp_Leap(this, play)) {
            this->timer = 0;
            EnBenVisitor_Hold(this, play);
            this->actionFunc = EnBenScp_StatueCaught;
            return;
        }
    }

    seen = !shut && EnBenVisitor_Seen(this, play, &this->actor.world.pos, 1.0f);
    this->unseenFrames = seen ? 0 : (this->unseenFrames + 1);
    if (seen) {
        this->isNoticed = true;
        return; // it can't move
    }

    dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);
    if ((dist > SCP_FAR_GONE) || ((this->life >= STATUE_LONGEST) && (this->unseenFrames > STATUE_SLIDE_AFTER))) {
        EnBenScp_Leave(this, play);
        return;
    }
    if (dist < STATUE_CATCH) {
        this->timer = 0;
        EnBenVisitor_Hold(this, play);
        this->actionFunc = EnBenScp_StatueCaught;
        return;
    }
    if (this->unseenFrames < STATUE_SLIDE_AFTER) {
        return;
    }
    yaw = Actor_WorldYawTowardActor(&this->actor, &player->actor);
    if (EnBenVisitor_Step(this, play, yaw, CLAMP_MAX(STATUE_SLIDE, dist - (STATUE_CATCH * 0.6f))) ||
        EnBenVisitor_Step(this, play, yaw + 0x2000, STATUE_SLIDE) ||
        EnBenVisitor_Step(this, play, yaw - 0x2000, STATUE_SLIDE)) {
        if ((this->unseenFrames % STATUE_SCRAPE_EVERY) == 0) {
            Actor_PlaySfx(&this->actor, STATUE_SLIDE_SFX);
        }
    }
    this->actor.shape.rot.y = this->actor.world.rot.y = yaw;
}

static void EnBenScp_StatueCaught(EnBenVisitor* this, PlayState* play) {
    if (this->timer < STATUE_CAUGHT_SHOW) {
        Play_FillScreen(play, false, 0, 0, 0, 0); // eyes open, and it's right there
    } else if (this->timer == STATUE_CAUGHT_SHOW) {
        Play_FillScreen(play, true, 0, 0, 0, 255);
        Audio_PlaySfx(STATUE_SNAP);
        EnBenVisitor_Hurt(play, STATUE_DAMAGE);
        this->isHidden = true;
    } else if (this->timer == STATUE_CAUGHT_VOID) {
        EnBenVisitor_Release(this, play);
        func_80169EFC(play); // back to the way in, as if you'd fallen
    }
}

#elif EN_BEN_SCP == BEN_VISITOR_SHY
// ---------------------------------------------------------------------------------------------------------------------
// SCP-096

/**
 * Whether you've just seen its face: it's turned to you, you're near enough to make it out, and it's there in front of
 * you.
 */
static s32 EnBenScp_FaceSeen(EnBenVisitor* this, PlayState* play) {
    Camera* camera = GET_ACTIVE_CAM(play);
    s16 toEye = Math_Vec3f_Yaw(&this->actor.world.pos, &camera->eye);
    s16 off = toEye - this->actor.shape.rot.y;

    return (ABS(off) < SHY_FACE_ARC) && (Math_Vec3f_DistXZ(&this->actor.world.pos, &camera->eye) < SHY_SEE_DIST) &&
           EnBenVisitor_Seen(this, play, &this->actor.world.pos, SHY_FACE_MIDDLE);
}

static void EnBenScp_ShyCalm(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yawToYou = Actor_WorldYawTowardActor(&this->actor, &player->actor);
    s32 inView = EnBenVisitor_Seen(this, play, &this->actor.world.pos, 1.0f);

    this->unseenFrames = inView ? 0 : (this->unseenFrames + 1);
    if ((this->life >= SHY_CALM_LONGEST) && (this->unseenFrames > SHY_SIT_GONE)) {
        EnBenScp_Leave(this, play);
        return;
    }

    if (this->phase > 0) {
        // Turned round, looking for something
        this->phase--;
        Math_ScaledStepToS(&this->actor.shape.rot.y, yawToYou, 0x300);
        if (this->phase == 0) {
            this->actor.world.rot.y = yawToYou + 0x4000; // and back to pacing
        }
    } else {
        if (Math_Vec3f_DistXZ(&this->actor.world.pos, &this->home) > SHY_PACE) {
            this->actor.world.rot.y = Math_Vec3f_Yaw(&this->actor.world.pos, &this->home); // back the other way
        }
        if (!EnBenVisitor_Step(this, play, this->actor.world.rot.y, SHY_PACE_SPEED)) {
            this->actor.world.rot.y += 0x8000; // or nowhere to go
        }
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.world.rot.y, 0x200);
        if (Rand_ZeroOne() < SHY_TURN_CHANCE) {
            this->phase = SHY_TURN;
            Actor_PlaySfx(&this->actor, SHY_MOAN);
        }
    }

    this->seenFrames = EnBenScp_FaceSeen(this, play) ? (this->seenFrames + 1) : 0;
    if (this->seenFrames >= SHY_FACE_FRAMES) {
        // You saw it
        this->mood = EN_BEN_SCP_SHY_UPSET;
        this->timer = 0;
        Audio_SetMainBgmVolume(0, 5);
        Actor_PlaySfx(&this->actor, SHY_SCREAM);
    }
}

static void EnBenScp_Shy(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist;

    if (!this->placed) {
        EnBenVisitor_Arrive(this, play, SHY_FIRST_DIST, true);
        if (this->placed) {
            // Pacing across, its back to you
            this->home = this->actor.world.pos;
            this->actor.world.rot.y += 0x4000;
            this->actor.shape.rot.y = this->actor.world.rot.y + 0x2000;
        }
        return;
    }
    if (EnBenVisitor_Busy(play)) {
        EnBenScp_Leave(this, play);
        return;
    }
    dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);

    switch (this->mood) {
        case EN_BEN_SCP_SHY_CALM:
            EnBenScp_ShyCalm(this, play);
            break;

        case EN_BEN_SCP_SHY_UPSET:
            // Its hands over its face, screaming, shaking, turning to you
            Math_ScaledStepToS(&this->actor.shape.rot.y, Actor_WorldYawTowardActor(&this->actor, &player->actor),
                               0x400);
            this->actor.shape.rot.z = (s16)Rand_CenteredFloat(0x500);
            this->headRoll = (s16)Rand_CenteredFloat(0x900);
            if ((this->timer % SHY_SCREAM_EVERY) == 0) {
                Actor_PlaySfx(&this->actor, SHY_SCREAM);
            }
            if (this->timer >= SHY_UPSET) {
                this->mood = EN_BEN_SCP_SHY_CHASE;
                this->timer = 0;
                this->actor.shape.rot.z = 0;
            }
            break;

        case EN_BEN_SCP_SHY_CHASE:
            // Coming for you, through anything
            if (dist < SHY_CATCH) {
                this->timer = 0;
                EnBenVisitor_Hold(this, play);
                this->actionFunc = EnBenScp_ShyCaught;
                return;
            }
            EnBenScp_Through(this, play, &player->actor.world.pos,
                             CLAMP(dist / SHY_SPEED_DIV, SHY_SPEED_MIN, SHY_SPEED_MAX));
            this->headRoll = (s16)(Math_SinS(this->timer * 0x1800) * 0x600);
            if ((this->timer % SHY_SCREAM_EVERY) == 0) {
                Actor_PlaySfx(&this->actor, SHY_SCREAM);
            }
            if ((this->timer % 6) == 0) {
                Actor_PlaySfx(&this->actor, NA_SE_PL_WALK_GROUND);
            }
            if (this->timer >= SHY_CHASE_LONGEST) {
                this->mood = EN_BEN_SCP_SHY_SIT; // it gives up
                this->timer = 0;
                this->headRoll = 0;
                Actor_PlaySfx(&this->actor, SHY_MOAN);
            }
            break;

        default:
            // Sitting with its face in its hands, until you look away
            this->unseenFrames =
                EnBenVisitor_Seen(this, play, &this->actor.world.pos, 1.0f) ? 0 : (this->unseenFrames + 1);
            if ((this->unseenFrames > SHY_SIT_GONE) || (dist > SCP_FAR_GONE)) {
                EnBenScp_Leave(this, play);
            }
            break;
    }
}

static void EnBenScp_ShyCaught(EnBenVisitor* this, PlayState* play) {
    if (this->timer < SHY_CAUGHT_SHOW) {
        this->headRoll = (s16)Rand_CenteredFloat(0xC00); // its mouth, right in front of you
    } else if (this->timer == SHY_CAUGHT_SHOW) {
        Play_FillScreen(play, true, 0, 0, 0, 255);
        Actor_PlaySfx(&this->actor, SHY_SCREAM);
        EnBenVisitor_Hurt(play, SHY_DAMAGE);
        this->isHidden = true;
    } else if (this->timer == SHY_CAUGHT_VOID) {
        EnBenVisitor_Release(this, play);
        func_80169EFC(play);
    }
}

#else
// ---------------------------------------------------------------------------------------------------------------------
// SCP-106

/**
 * How far out of the ground he is, in the model: none of him at 0, all of him at 255.
 */
static void EnBenScp_SetRise(EnBenVisitor* this) {
    this->actor.shape.yOffset = -this->height * 100.0f * (1.0f - (this->rise / 255.0f));
}

/**
 * Whether a light arrow is near him.
 */
static s32 EnBenScp_LightNear(EnBenVisitor* this, PlayState* play) {
    Actor* it;
    Vec3f middle = this->actor.world.pos;

    middle.y += this->height * 0.5f;
    for (it = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first; it != NULL; it = it->next) {
        if ((it->id == ACTOR_EN_ARROW) && (it->params == ARROW_TYPE_LIGHT) &&
            (Math_Vec3f_DistXYZ(&it->world.pos, &middle) < OLD_LIGHT_DIST)) {
            return true;
        }
    }
    return false;
}

static void EnBenScp_OldSinkNow(EnBenVisitor* this, s32 forGood) {
    this->frowning = forGood;
    this->timer = 0;
    Actor_PlaySfx(&this->actor, OLD_GROAN);
    this->actionFunc = EnBenScp_OldSink;
}

static void EnBenScp_OldRise(EnBenVisitor* this, PlayState* play) {
    if (!this->placed) {
        EnBenVisitor_Arrive(this, play, OLD_FIRST_DIST, false);
        if (this->placed) {
            this->rise = 0;
            EnBenScp_SetRise(this);
            Actor_PlaySfx(&this->actor, OLD_GROAN);
        }
        return;
    }
    if (EnBenVisitor_Busy(play)) {
        EnBenScp_Leave(this, play);
        return;
    }
    if (EnBenScp_LightNear(this, play)) {
        EnBenScp_OldSinkNow(this, true);
        return;
    }
    Math_StepToS(&this->rise, 255, OLD_RISE);
    EnBenScp_SetRise(this);
    if ((this->timer % 10) == 0) {
        Actor_PlaySfx(&this->actor, OLD_BUBBLE);
    }
    if (this->rise >= 255) {
        this->timer = 0;
        this->phase = OLD_SINK_EVERY + (s32)Rand_ZeroFloat(OLD_SINK_RANGE);
        this->actionFunc = EnBenScp_OldWalk;
    }
}

static void EnBenScp_OldWalk(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 dist = Actor_WorldDistXZToActor(&this->actor, &player->actor);

    if (EnBenVisitor_Busy(play)) {
        EnBenScp_Leave(this, play);
        return;
    }
    if (dist < OLD_CATCH) {
        this->timer = 0;
        EnBenVisitor_Hold(this, play);
        Actor_PlaySfx(&this->actor, OLD_GROAN);
        this->actionFunc = EnBenScp_OldCaught;
        return;
    }
    if (EnBenScp_LightNear(this, play) || (dist > SCP_FAR_GONE) || (this->life >= OLD_LONGEST)) {
        EnBenScp_OldSinkNow(this, true);
        return;
    }
    if (--this->phase <= 0) {
        EnBenScp_OldSinkNow(this, false); // and up again, closer
        return;
    }
    EnBenScp_Through(this, play, &player->actor.world.pos, OLD_SPEED);
    if ((this->timer % OLD_MARK_EVERY) == 0) {
        EnBenScp_Mark(this);
    }
    if ((this->timer % 20) == 0) {
        Actor_PlaySfx(&this->actor, NA_SE_PL_WALK_GROUND);
    }
}

static void EnBenScp_OldSink(EnBenVisitor* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_StepToS(&this->rise, 0, this->frowning ? (OLD_RISE * 2) : OLD_RISE);
    EnBenScp_SetRise(this);
    if ((this->timer % 10) == 0) {
        Actor_PlaySfx(&this->actor, OLD_BUBBLE);
    }
    if (this->rise > 0) {
        return;
    }
    if (this->frowning || EnBenVisitor_Busy(play)) {
        EnBenScp_Leave(this, play);
        return;
    }
    // Somewhere closer: where you're looking if there's room, otherwise where you aren't. If there's nowhere, the same
    // pool.
    EnBenScp_Mark(this);
    this->dist = CLAMP_MIN(Actor_WorldDistXZToActor(&this->actor, &player->actor) * OLD_CLOSER, OLD_CLOSEST);
    if (!EnBenVisitor_TryPlace(this, play, this->dist, true)) {
        EnBenVisitor_TryPlace(this, play, this->dist, false);
    }
    this->timer = 0;
    Actor_PlaySfx(&this->actor, OLD_GROAN);
    this->actionFunc = EnBenScp_OldRise;
}

static void EnBenScp_OldCaught(EnBenVisitor* this, PlayState* play) {
    s32 t = this->timer;

    if (t < OLD_CAUGHT_DOWN) {
        // Down into the dark with him
        Play_FillScreen(play, true, 20, 12, 6, 255 * t / OLD_CAUGHT_DOWN);
        if ((t % 10) == 0) {
            Actor_PlaySfx(&this->actor, OLD_BUBBLE);
        }
    } else if (t == OLD_CAUGHT_DOWN) {
        Play_FillScreen(play, true, 20, 12, 6, 255);
        EnBenVisitor_Hurt(play, OLD_DAMAGE);
        this->isHidden = true;
    } else if (t == OLD_CAUGHT_VOID) {
        EnBenVisitor_Release(this, play);
        func_80169EFC(play);
    }
}
#endif

static void EnBenScp_Update(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;

    this->timer++;
    if (this->placed) {
        this->life++;
    }
    this->actionFunc(this, play);
}

#ifdef EN_BEN_SCP_MARKS
/**
 * A pool on the ground at `pos`, `scale` the size of the model's, `alpha` showing.
 */
static void EnBenScp_DrawPool(PlayState* play, Vec3f* pos, f32 scale, u8 r, u8 g, u8 b, u8 alpha) {
    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Translate(pos->x, pos->y + 0.5f, pos->z, MTXMODE_NEW);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, r, g, b, alpha);
    gSPDisplayList(POLY_XLU_DISP++, sScpPoolDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
#endif

static void EnBenScp_Draw(Actor* thisx, PlayState* play) {
    EnBenVisitor* this = (EnBenVisitor*)thisx;
#if EN_BEN_SCP == BEN_VISITOR_SHY
    s32 screaming = (this->mood == EN_BEN_SCP_SHY_UPSET) || (this->mood == EN_BEN_SCP_SHY_CHASE);
#elif EN_BEN_SCP == BEN_VISITOR_OLD_MAN
    s32 i;
    s32 age;
#endif

    if (!this->placed) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    if (!this->isHidden) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, sScpSetupDL);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

#if EN_BEN_SCP == BEN_VISITOR_SCULPTURE
        gSPDisplayList(POLY_OPA_DISP++, sScp173DL);
#elif EN_BEN_SCP == BEN_VISITOR_SHY
        gSPDisplayList(POLY_OPA_DISP++, sScp096BodyDL);
        // Its hands over its face while it screams and while it sits
        gSPDisplayList(POLY_OPA_DISP++, ((this->mood == EN_BEN_SCP_SHY_UPSET) || (this->mood == EN_BEN_SCP_SHY_SIT))
                                            ? sScp096ArmsUpDL
                                            : sScp096ArmsDownDL);
        Matrix_Translate(0.0f, SCP_096_NECK_Y, 0.0f, MTXMODE_APPLY);
        Matrix_RotateZS(this->headRoll, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, screaming ? sScp096ScreamDL : sScp096HeadDL);
#else
        gSPDisplayList(POLY_OPA_DISP++, sScp106DL);
#endif
    }

    CLOSE_DISPS(play->state.gfxCtx);

#ifdef EN_BEN_SCP_MARKS
    // What it leaves on the ground
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    OPEN_DISPS(play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sScpGroundSetupDL);
    CLOSE_DISPS(play->state.gfxCtx);
#if EN_BEN_SCP == BEN_VISITOR_SCULPTURE
    if (this->markCount > 0) {
        // The reddish-brown stain where it was standing
        EnBenScp_DrawPool(play, &this->marks[0], OLD_MARK_SCALE, 92, 40, 22, 200);
    }
#else
    for (i = 0; i < this->markCount; i++) {
        age = (this->markHead - 1 - i + EN_BEN_VISITOR_MARKS) % EN_BEN_VISITOR_MARKS;
        EnBenScp_DrawPool(play, &this->marks[age], OLD_MARK_SCALE, 24, 16, 8,
                          (u8)(220 - (i * 160 / EN_BEN_VISITOR_MARKS)));
    }
    if (!this->isHidden && (this->actionFunc != EnBenScp_OldWalk)) {
        // The pool he's coming up out of, or going down into
        EnBenScp_DrawPool(play, &this->actor.world.pos, OLD_POOL_SCALE, 20, 12, 6, 235);
    }
#endif
#endif
}
