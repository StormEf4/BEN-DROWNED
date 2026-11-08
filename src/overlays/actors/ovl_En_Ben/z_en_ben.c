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
 *
 * Making it worse the longer it goes on (our own, beyond the canon):
 *  - Every sighting drains a little more color out of Clock Town, and the fog closes in, darker.
 *  - Some waits end with heavy stone footsteps right behind you. They stay behind you however you turn.
 *  - Now and then it's there for an instant at the edge of the screen, then not. It doesn't count as a sighting.
 *  - The fourth time, there isn't one of it. A line of them stands along the path you just walked, all facing you.
 *  - Later, sometimes it doesn't leave when you look away. Each time you look back it's closer, until it's right
 *    there. While it stares at you the music stops, the color is gone, you hear a heartbeat, your hearts beep as if
 *    you're dying, and the clock runs backwards.
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
// The Happy Mask Salesman's laugh. The Great Fairy's laugh sample is swapped for his
// (assets/audio/samples/SampleBank_0/great_fairy_laughter.wav), so this sound effect plays it.
#define EN_BEN_LAUGH_SFX NA_SE_VO_FR_LAUGH_0

#define EN_BEN_DREAD_SIGHTINGS 7             // by this many sightings, Clock Town is as far gone as it gets
#define EN_BEN_FOG_NEAR 955                  // the fog at its thickest (0-1000: lower starts closer)
#define EN_BEN_AMBIENT_DRAIN 0.5f            // and this much of the ambient light is gone
#define EN_BEN_GREY_MOST 170                 // how grey the world gets from sightings alone (0-255)
#define EN_BEN_GREY_STARING 255              // and while it stares at you
#define EN_BEN_GREY_STEP 6                   // per frame, towards that
#define EN_BEN_STEPS_BEFORE 70               // footsteps start this long before it appears (3.5 s)
#define EN_BEN_STEP_EVERY 14                 // a step this often
#define EN_BEN_STEP_FAR 420.0f               // the first step is this far behind you
#define EN_BEN_STEP_NEAR 120.0f              // and the last this close
#define EN_BEN_STEP_SFX NA_SE_EN_AMOS_WALK   // a stone statue hopping
#define EN_BEN_STEPS_CHANCE 0.5f             // how often a wait ends with footsteps
#define EN_BEN_GLIMPSE_FROM 2                // glimpses start after this many sightings
#define EN_BEN_GLIMPSE_CHANCE 0.4f           // how often a wait has one
#define EN_BEN_GLIMPSE_FRAMES 2              // how long it's there (a tenth of a second)
#define EN_BEN_GLIMPSE_ANGLE 0x1800          // how far off the middle of the screen
#define EN_BEN_GLIMPSE_DIST 750.0f           // and how far away
#define EN_BEN_TRAIL_SIGHTING 3              // the sighting after this many is the line along your path
#define EN_BEN_PATH_EVERY 8                  // your position is remembered this often
#define EN_BEN_PATH_SPACING 60.0f            // if you've moved at least this far since the last one
#define EN_BEN_TRAIL_NEAREST 300.0f          // none of the line stands closer to you than this
#define EN_BEN_TRAIL_GAP 110.0f              // or closer to the next one than this
#define EN_BEN_TRAIL_FEWEST 3                // with fewer than this many places to stand, it's an ordinary sighting
#define EN_BEN_CREEP_FROM 5                  // it starts creeping up on you after this many sightings
#define EN_BEN_CREEP_CHANCE 0.5f             // and does it this often
#define EN_BEN_CREEP_STEP 0.55f              // each time you look away, it's this much of the way to you
#define EN_BEN_CREEP_CLOSEST 100.0f          // this is as close as it comes
#define EN_BEN_CREEP_GIVE_UP 60              // if it has nowhere closer to stand for this long (3 s), it's gone
#define EN_BEN_CREEP_STARE 40                // look at it from that close for this long (2 s), and it's gone
#define EN_BEN_ALARM_DIST 320.0f             // closer than this, your hearts beep as if you're dying
#define EN_BEN_ALARM_EVERY 12                // how often they beep
#define EN_BEN_REWIND_SPEED 40               // while it stares, the clock runs back this much per frame
#define EN_BEN_REWIND_MOST CLOCK_TIME(1, 30) // but never more than this
#define EN_BEN_BLUR_ALPHA 190                // after it's gone up close, the screen smears
#define EN_BEN_BLUR_FRAMES 30                // for this long

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
void EnBen_Creep(EnBen* this, PlayState* play, s32 visible);
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
 * A glimpse: somewhere near the edge of the screen, in plain view, far off. Returns false if there's nowhere.
 */
s32 EnBen_TryPlaceGlimpse(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) +
              ((Rand_ZeroOne() < 0.5f) ? EN_BEN_GLIMPSE_ANGLE : -EN_BEN_GLIMPSE_ANGLE);
    Vec3f pos;
    s32 d;

    for (d = 0; d < ARRAY_COUNT(sDistanceScales); d++) {
        if (EnBen_FindSpot(play, player, yaw, EN_BEN_GLIMPSE_DIST * sDistanceScales[d], &pos) &&
            EnBen_IsBodyVisible(play, &pos)) {
            EnBen_PlaceAt(this, play, &pos);
            return true;
        }
    }
    return false;
}

/**
 * Remember where you've just been, a step at a time, so it can stand there later.
 */
void EnBen_RememberPath(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->pathTimer > 0) {
        this->pathTimer--;
        return;
    }
    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        return;
    }
    if ((this->pathCount > 0) &&
        (Math_Vec3f_DistXZ(&this->path[(this->pathHead + EN_BEN_PATH_POINTS - 1) % EN_BEN_PATH_POINTS],
                           &player->actor.world.pos) < EN_BEN_PATH_SPACING)) {
        return; // you haven't gone anywhere
    }

    this->path[this->pathHead] = player->actor.world.pos;
    this->pathHead = (this->pathHead + 1) % EN_BEN_PATH_POINTS;
    if (this->pathCount < EN_BEN_PATH_POINTS) {
        this->pathCount++;
    }
    this->pathTimer = EN_BEN_PATH_EVERY;
}

/**
 * Stand along the path you just walked: one of it at each place you passed, all out of view and all facing you.
 * Returns false if you haven't walked far enough for that.
 */
s32 EnBen_TryPlaceTrail(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f chosen[EN_BEN_TRAIL_COPIES + 1];
    s32 count = 0;
    s32 i;

    // Oldest first, so the line starts back where you came from.
    for (i = 0; (i < this->pathCount) && (count < ARRAY_COUNT(chosen)); i++) {
        Vec3f* pos = &this->path[(this->pathHead + EN_BEN_PATH_POINTS - this->pathCount + i) % EN_BEN_PATH_POINTS];

        if ((Math_Vec3f_DistXZ(pos, &player->actor.world.pos) < EN_BEN_TRAIL_NEAREST) ||
            ((count > 0) && (Math_Vec3f_DistXZ(pos, &chosen[count - 1]) < EN_BEN_TRAIL_GAP)) ||
            EnBen_IsBodyVisible(play, pos)) {
            continue;
        }
        chosen[count++] = *pos;
    }
    if (count < EN_BEN_TRAIL_FEWEST) {
        return false;
    }

    EnBen_PlaceAt(this, play, &chosen[0]);
    for (i = 1; i < count; i++) {
        this->trail[i - 1] = chosen[i];
        this->trailYaw[i - 1] = Math_Vec3f_Yaw(&chosen[i], &player->actor.world.pos);
    }
    this->trailCount = count - 1;
    return true;
}

/**
 * Appear for a sighting. Usually it just stands somewhere; the fourth time it's a line of them along your path, and
 * later on it sometimes creeps up on you.
 */
s32 EnBen_TryAppear(EnBen* this, PlayState* play) {
    s32 sightings = Ben_GetSightings();

    this->kind = EN_BEN_SIGHTING_STILL;
    this->trailCount = 0;
    if ((Ben_GetFate() == BEN_FATE_NONE) || (Ben_GetFate() == BEN_FATE_DONE)) {
        if ((sightings >= EN_BEN_TRAIL_SIGHTING) && !Ben_HasSeenTrail() && EnBen_TryPlaceTrail(this, play)) {
            this->kind = EN_BEN_SIGHTING_TRAIL;
            return true;
        }
        if ((sightings >= EN_BEN_CREEP_FROM) && (Rand_ZeroOne() < EN_BEN_CREEP_CHANCE)) {
            this->kind = EN_BEN_SIGHTING_CREEP;
        }
    }
    return EnBen_TryPlaceSighting(this, play);
}

/**
 * True if it, or any of it standing along your path, is in plain view.
 */
s32 EnBen_IsAnyVisible(EnBen* this, PlayState* play) {
    s32 i;

    if (EnBen_IsBodyVisible(play, &this->actor.world.pos)) {
        return true;
    }
    for (i = 0; i < this->trailCount; i++) {
        if (EnBen_IsBodyVisible(play, &this->trail[i])) {
            return true;
        }
    }
    return false;
}

/**
 * How close you are to the nearest of it.
 */
f32 EnBen_GetNearestDist(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 nearest = this->actor.xzDistToPlayer;
    s32 i;

    for (i = 0; i < this->trailCount; i++) {
        nearest = CLAMP_MAX(nearest, Math_Vec3f_DistXZ(&this->trail[i], &player->actor.world.pos));
    }
    return nearest;
}

/**
 * Put the clock back where it stopped, if it ran backwards while it stared at you. Nothing else moves it on the
 * 4th day, so anything further off (like the Song of Time's dawn) is left alone.
 */
void EnBen_RestoreClock(EnBen* this) {
    if ((u16)(this->stoppedTime - gSaveContext.save.time) <= (EN_BEN_REWIND_MOST + EN_BEN_REWIND_SPEED)) {
        gSaveContext.save.time = this->stoppedTime;
    }
}

/**
 * It's looking right at you: the music goes, and so does the color. Or it isn't any more.
 */
void EnBen_SetStaring(EnBen* this, s32 staring) {
    if (staring == this->isStaring) {
        return;
    }
    this->isStaring = staring;
    if (staring) {
        this->stoppedTime = gSaveContext.save.time;
        Audio_SetMainBgmVolume(0, 10);
    } else {
        EnBen_RestoreClock(this);
        Audio_SetMainBgmVolume(0x7F, 20);
    }
}

/**
 * How far gone Clock Town is: nothing before the first sighting, all the way by the seventh.
 */
f32 EnBen_GetDread(void) {
    return CLAMP_MAX(Ben_GetSightings(), EN_BEN_DREAD_SIGHTINGS) / (f32)EN_BEN_DREAD_SIGHTINGS;
}

/**
 * The color drains out of the world and the fog closes in, darker, a little more with every sighting. While it
 * stares at you, all the way.
 */
void EnBen_DrainWorld(EnBen* this, PlayState* play) {
    static s16 sFogColor[] = { 12, 6, 14 };
    EnvironmentContext* envCtx = &play->envCtx;
    f32 dread = this->isStaring ? 1.0f : EnBen_GetDread();
    s32 i;

    envCtx->adjLightSettings.fogNear = (envCtx->lightSettings.fogNear > EN_BEN_FOG_NEAR)
                                           ? (EN_BEN_FOG_NEAR - envCtx->lightSettings.fogNear) * dread
                                           : 0;
    for (i = 0; i < 3; i++) {
        envCtx->adjLightSettings.fogColor[i] = (sFogColor[i] - envCtx->lightSettings.fogColor[i]) * dread;
        envCtx->adjLightSettings.ambientColor[i] =
            -envCtx->lightSettings.ambientColor[i] * EN_BEN_AMBIENT_DRAIN * dread;
    }

    Math_StepToS(&this->grey, this->isStaring ? EN_BEN_GREY_STARING : (s16)(EN_BEN_GREY_MOST * dread),
                 EN_BEN_GREY_STEP);
    gPlayVisMonoColor.r = 255;
    gPlayVisMonoColor.g = 255;
    gPlayVisMonoColor.b = 255;
    gPlayVisMonoColor.a = this->grey;
}

/**
 * Heavy stone steps behind you, closer each time, as the wait runs out. Always behind the camera, however you turn.
 */
void EnBen_Footsteps(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 behindYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
    Vec3f pos;
    f32 invW;
    f32 dist;

    if (this->timer > EN_BEN_STEPS_BEFORE) {
        return;
    }
    if (this->stepTimer > 0) {
        this->stepTimer--;
        return;
    }
    this->stepTimer = EN_BEN_STEP_EVERY;

    dist = EN_BEN_STEP_NEAR + ((EN_BEN_STEP_FAR - EN_BEN_STEP_NEAR) * this->timer / EN_BEN_STEPS_BEFORE);
    pos.x = player->actor.world.pos.x + (Math_SinS(behindYaw) * dist);
    pos.y = player->actor.world.pos.y;
    pos.z = player->actor.world.pos.z + (Math_CosS(behindYaw) * dist);
    Actor_GetProjectedPos(play, &pos, &this->stepPos, &invW);
    Audio_PlaySfx_AtPosWithPresetLowFreqAndHighReverb(&this->stepPos, EN_BEN_STEP_SFX);
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
    this->kind = EN_BEN_SIGHTING_STILL;
    this->trailCount = 0;
    EnBen_SetStaring(this, false);
    this->timer = CLAMP_MIN(wait, EN_BEN_LURK_MIN) + (s32)Rand_ZeroFloat(EN_BEN_LURK_RANGE);
    this->withSteps = (Ben_GetSightings() > 0) && (Rand_ZeroOne() < EN_BEN_STEPS_CHANCE);
    this->stepTimer = 0;
    this->glimpseAt = ((Ben_GetSightings() >= EN_BEN_GLIMPSE_FROM) && (Rand_ZeroOne() < EN_BEN_GLIMPSE_CHANCE))
                          ? (this->timer / 2)
                          : -1;
    this->actionFunc = EnBen_Lurk;
}

/**
 * Gone, and the Happy Mask Salesman laughs somewhere. If it was staring at you, the screen smears.
 */
void EnBen_Leave(EnBen* this) {
    Audio_PlaySfx(EN_BEN_LAUGH_SFX);
    if (this->isStaring) {
        Play_EnableMotionBlur(EN_BEN_BLUR_ALPHA);
        this->blurFrames = EN_BEN_BLUR_FRAMES;
    }
    EnBen_Vanish(this);
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
    this->kind = EN_BEN_SIGHTING_STILL;
    this->glimpseAt = -1;
    this->glimpseFrames = 0;
    this->withSteps = false;
    this->stepTimer = 0;
    this->pathTimer = 0;
    this->pathHead = 0;
    this->pathCount = 0;
    this->trailCount = 0;
    this->blurFrames = 0;
    this->isStaring = false;
    this->stoppedTime = gSaveContext.save.time;
    this->grey = EN_BEN_GREY_MOST * EnBen_GetDread(); // already this grey as you walk in
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
    if (this->isStaring) {
        EnBen_RestoreClock(this);
        Audio_SetMainBgmVolume(0x7F, 0);
    }
    if (this->blurFrames > 0) {
        Play_DisableMotionBlur();
    }
    gPlayVisMonoColor.a = 0;

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
    if (this->glimpseFrames > 0) {
        this->glimpseFrames--;
        if (this->glimpseFrames == 0) {
            this->isHidden = true; // was it ever there?
        }
    }

    if (this->timer > 0) {
        if ((this->timer == this->glimpseAt) && EnBen_TryPlaceGlimpse(this, play)) {
            this->isHidden = false;
            this->glimpseFrames = EN_BEN_GLIMPSE_FRAMES;
        }
        if (this->withSteps) {
            EnBen_Footsteps(this, play);
        }
        this->timer--;
        return;
    }
    if (this->glimpseFrames > 0) {
        return;
    }
    if (EnBen_TryAppear(this, play)) {
        this->isHidden = false;
        this->isNoticed = false;
        this->seenFrames = 0;
        this->unseenFrames = 0;
        this->creepFails = 0;
        this->stareFrames = 0;
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
    s32 visible = EnBen_IsAnyVisible(this, play);

    if (!this->isNoticed) {
        if (visible) {
            this->seenFrames++;
            if (this->seenFrames >= EN_BEN_NOTICE_FRAMES) {
                this->isNoticed = true;
                Ben_AddSighting();
                if (Ben_GetSightings() == 1) {
                    play->sceneSequences.seqId = NA_BGM_SONG_OF_HEALING; // the music wakes up
                }
                if (this->kind == EN_BEN_SIGHTING_TRAIL) {
                    Ben_SetTrailSeen();
                }
                if (this->kind != EN_BEN_SIGHTING_STILL) {
                    EnBen_SetStaring(this, true);
                }
                if (Ben_GetFate() == BEN_FATE_DROWNED) {
                    // You couldn't get away. This time it doesn't go anywhere.
                    EnBen_SetStaring(this, true);
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

    if (this->kind == EN_BEN_SIGHTING_CREEP) {
        EnBen_Creep(this, play, visible);
        return;
    }

    if (!visible) {
        this->unseenFrames++;
    } else {
        this->unseenFrames = 0;
    }

    if ((this->unseenFrames >= EN_BEN_GONE_FRAMES) || (EnBen_GetNearestDist(this, play) < EN_BEN_TOO_CLOSE)) {
        EnBen_Leave(this); // the Happy Mask Salesman, somewhere, every time it's gone
    }
}

/**
 * It doesn't leave when you look away. Every time you look back, it's closer, until it's right in front of you.
 */
void EnBen_Creep(EnBen* this, PlayState* play, s32 visible) {
    Player* player = GET_PLAYER(play);
    f32 dist = this->actor.xzDistToPlayer;
    s32 isClose = dist < (EN_BEN_CREEP_CLOSEST * 1.5f);
    Vec3f pos;

    if (visible) {
        this->unseenFrames = 0;
        if (isClose) {
            this->stareFrames++;
            if (this->stareFrames >= EN_BEN_CREEP_STARE) {
                EnBen_Leave(this);
            }
        }
        return;
    }

    this->unseenFrames++;
    if (this->unseenFrames < EN_BEN_GONE_FRAMES) {
        return;
    }
    if (isClose) {
        EnBen_Leave(this); // it was already right there
        return;
    }

    if (EnBen_FindSpot(play, player, Math_Vec3f_Yaw(&player->actor.world.pos, &this->actor.world.pos),
                       CLAMP_MIN(dist * EN_BEN_CREEP_STEP, EN_BEN_CREEP_CLOSEST), &pos) &&
        !EnBen_IsBodyVisible(play, &pos)) {
        EnBen_PlaceAt(this, play, &pos);
        Actor_PlaySfx(&this->actor, EN_BEN_STEP_SFX);
        this->creepFails = 0;
        this->unseenFrames = 0;
    } else {
        this->creepFails++;
        if (this->creepFails >= EN_BEN_CREEP_GIVE_UP) {
            EnBen_Leave(this);
        }
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
    Audio_PlaySfx(EN_BEN_LAUGH_SFX);
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
    EnBen_RememberPath(this, play);

    this->actionFunc(this, play);

    EnBen_DrainWorld(this, play);
    if (this->isStaring) {
        Audio_PlaySfx(NA_SE_EN_LAST2_HEARTBEAT_OLD - SFX_FLAG);
        if ((EnBen_GetNearestDist(this, play) < EN_BEN_ALARM_DIST) &&
            ((play->gameplayFrames % EN_BEN_ALARM_EVERY) == 0)) {
            Audio_PlaySfx(NA_SE_SY_HITPOINT_ALARM); // your hearts say you're dying
        }
        if (!Ben_HasRewound() && ((u16)(this->stoppedTime - gSaveContext.save.time) < EN_BEN_REWIND_MOST)) {
            gSaveContext.save.time -= EN_BEN_REWIND_SPEED; // the clock runs backwards
        }
    }
    if (this->blurFrames > 0) {
        this->blurFrames--;
        if (this->blurFrames == 0) {
            Play_DisableMotionBlur();
        }
    }

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
    s32 i;

    if (this->isHidden) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    // Same opaque path the vanilla Elegy shell uses (En_Torch2).
    Scene_SetRenderModeXlu(play, 0, 0x01);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255);
    Gfx_DrawDListOpa(play, gElegyShellHumanDL);

    // The rest of it, along your path.
    for (i = 0; i < this->trailCount; i++) {
        Matrix_Translate(this->trail[i].x, this->trail[i].y, this->trail[i].z, MTXMODE_NEW);
        Matrix_RotateYS(this->trailYaw[i], MTXMODE_APPLY);
        Matrix_Scale(this->actor.scale.x, this->actor.scale.y, this->actor.scale.z, MTXMODE_APPLY);
        Gfx_DrawDListOpa(play, gElegyShellHumanDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
