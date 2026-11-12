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
#include "overlays/actors/ovl_En_Osn/z_en_osn.h"

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

#define EN_BEN_BOOT_TEXT_DELAY 60  // BEN's file: how long you stand at Stone Tower before the garbled box
#define EN_BEN_BOOT_FIRST_LURK 160 // after the box, how long before the statue is somewhere
#define EN_BEN_BOOT_ROAM 700       // then how long you have before the game gives out (35 s)
#define EN_BEN_BOOT_GIVE_OUT 50    // and how long it takes to give out
#define EN_BEN_BOOT_BLUR_ALPHA 235

#define EN_BEN_SALESMAN_START 20         // Session 3: frames after the white flash before it starts
#define EN_BEN_SALESMAN_ANGLE_FRAMES 60  // how long the strange angle holds before he's there
#define EN_BEN_SALESMAN_REVEAL_FRAMES 30 // how long you look at him before he speaks
#define EN_BEN_SALESMAN_CUT_FRAMES 15    // after his laugh, the cut to Termina Field
#define EN_BEN_SALESMAN_BEHIND 70.0f     // how far behind you he stands
#define EN_BEN_SALESMAN_ROLL 0x1C00      // the camera's tilt
#define EN_BEN_SALESMAN_TEMPO_SCALE 55   // Session 3: his theme, slowed (percent)
#define EN_BEN_SALESMAN_FREQ_SCALE 700   // and lowered (1000 = normal)

#define EN_BEN_FINALE_DIST 350.0f // Session 3's end: how far in front of you it stands in the Deku King's chamber
#define EN_BEN_FINALE_NEAR 180.0f // walk this close to it
#define EN_BEN_FINALE_STARE 100   // or look at it this long (5 s)
#define EN_BEN_FINALE_GIVE_OUT 40 // then the screen goes, and the game saves and gives out

#define EN_BEN_END_SIGHTINGS 7    // Sessions 4 and 7: if you haven't found the end by this many sightings, it finds you
#define EN_BEN_WHITEOUT_FRAMES 40 // Session 4: a whiteout, start to finish
#define EN_BEN_WHITEOUT_PEAK 20   // the frame it's all white, and it moves
#define EN_BEN_SEA_DEPTH 400.0f   // Session 5: water deeper than this is the open sea
#define EN_BEN_SEA_FRAMES 60      // swim out there this long (3 s) and it takes you
#define EN_BEN_SEA_STATUE 140.0f  // and it's standing on the water this close in front of you
#define EN_BEN_SEA_FLASH 40       // then a red flash, his laugh
#define EN_BEN_SEA_LEAVE 75       // then the Moon
#define EN_BEN_MOON_DELAY 80      // Session 5's end: how long you're on the Moon before they ask
#define EN_BEN_MOON_LEAVE 60      // after the question, how long before Stone Tower
#define EN_BEN_WARP_DELAY 40      // Sessions 4-6: how long South Clock Town lasts
#define EN_BEN_DREAM_SIGHTINGS 6  // Session 6: the dream comes after this many sightings
#define EN_BEN_DREAM_DIST 320.0f  // where it stands to begin with
#define EN_BEN_DREAM_STEP 55      // it comes closer this often, while you watch
#define EN_BEN_DREAM_INVERT 120   // then the stick is backwards
#define EN_BEN_DREAM_REFUSE 240   // then nothing works
#define EN_BEN_DREAM_SPEAK 300    // then it speaks
#define EN_BEN_WATCH_STEP 45      // Session 7: watch it this long and it moves anyway
#define EN_BEN_LAST_DIST 600.0f   // in Majora's lair: where it starts
#define EN_BEN_LAST_STEP 30       // how often it comes closer, whatever you do
#define EN_BEN_LAST_NEAR 140.0f   // and how close it gets before the end

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
void EnBen_SalesmanScene(EnBen* this, PlayState* play);
void EnBen_Finale(EnBen* this, PlayState* play);
u16 EnBen_GetSceneSeq(EnBen* this, PlayState* play);
void EnBen_SetSceneSeq(EnBen* this, PlayState* play);
void EnBen_WarpAway(EnBen* this, PlayState* play);
void EnBen_MoonScene(EnBen* this, PlayState* play);
void EnBen_Drowning(EnBen* this, PlayState* play);
void EnBen_Dream(EnBen* this, PlayState* play);
void EnBen_LastGame(EnBen* this, PlayState* play);
s32 EnBen_EndComes(EnBen* this, PlayState* play);
s32 EnBen_TryPlaceInWhiteout(EnBen* this, PlayState* play);

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
    if ((this->actor.params == BEN_ACTOR_BOOT) || (Ben_GetSession() == BEN_SESSION_FREE)) {
        this->kind = EN_BEN_SIGHTING_CREEP; // on its own file, and at the end, it doesn't bother leaving
    } else if ((Ben_GetFate() == BEN_FATE_NONE) || (Ben_GetFate() == BEN_FATE_DONE)) {
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
    if ((this->actor.params == BEN_ACTOR_SALESMAN) || (this->actor.params == BEN_ACTOR_WARP) ||
        (this->actor.params == BEN_ACTOR_MOON)) {
        // Directors with no body. Dawn, or the Moon, and no music at all.
        play->sceneSequences.seqId = NA_BGM_NO_MUSIC;
        this->isHidden = true;
        this->directorStep = 0;
        this->directorTimer = 0;
        this->other = NULL;
        this->actionFunc = (this->actor.params == BEN_ACTOR_SALESMAN) ? EnBen_SalesmanScene
                           : (this->actor.params == BEN_ACTOR_WARP)   ? EnBen_WarpAway
                                                                      : EnBen_MoonScene;
        return;
    }

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.textId = BEN_TEXT_SHOULDNT_HAVE;
    this->actor.gravity = -1.0f;
    this->musicCooldown = 0;
    this->musicWarped = false;
    this->isConfronting =
        BEN_DEMO_TEASER && (Ben_GetDayFour() == BEN_DAY_FOUR_HAUNTED) && Ben_HasRewound() && !Ben_HasWarned();
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
    this->whiteFrames = 0;
    this->watchFrames = 0;
    this->inWhiteout = false;
    this->isStaring = false;
    if ((Ben_GetSession() == BEN_SESSION_FREE) && (gSaveContext.save.day < 3)) {
        // Session 7 is the Night of the Final Day, however you got here. The counter is wrong.
        gSaveContext.save.day = 3;
        gSaveContext.save.time = CLOCK_TIME(23, 0);
        gSaveContext.save.isNight = true;
    }
    this->stoppedTime = gSaveContext.save.time;
    this->grey = EN_BEN_GREY_MOST * EnBen_GetDread(); // already this grey as you walk in
    if (Ben_GetFate() == BEN_FATE_BURNING) {
        Ben_SetFate(BEN_FATE_BURNED); // Link burned to death, and this is where you came back
    }

    // Spawned during Play_Init, before the scene's music starts. On the 4th day, until you've seen it, there is no
    // music at all.
    EnBen_SetSceneSeq(this, play);

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
    } else if (this->actor.params == BEN_ACTOR_FINALE) {
        this->directorStep = 0;
        this->directorTimer = 0;
        this->actionFunc = (play->sceneId == SCENE_LAST_BS) ? EnBen_LastGame : EnBen_Finale;
    } else if (this->actor.params == BEN_ACTOR_BOOT) {
        this->directorStep = 0;
        this->directorTimer = 0;
        this->timer = EN_BEN_BOOT_TEXT_DELAY + EN_BEN_BOOT_FIRST_LURK;
        this->actionFunc = EnBen_Lurk;
    } else {
        this->timer = EN_BEN_FIRST_LURK_MIN + (s32)Rand_ZeroFloat(EN_BEN_FIRST_LURK_RANGE);
        this->actionFunc = EnBen_Lurk;
    }
}

void EnBen_Destroy(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    // Whatever it was doing to you stops when you leave.
    Ben_SetInputMode(BEN_INPUT_NORMAL);
    Play_FillScreen(play, false, 0, 0, 0, 0);

    if ((this->actor.params == BEN_ACTOR_LAIR) || (this->actor.params == BEN_ACTOR_WARP) ||
        (this->actor.params == BEN_ACTOR_MOON)) {
        return;
    }
    if (this->actor.params == BEN_ACTOR_SALESMAN) {
        if ((this->directorStep != 0) && (this->directorStep < 5)) {
            Cutscene_StopManual(play, &play->csCtx);
        }
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
 * What should be playing here, or NA_BGM_DISABLED for silence.
 *  - The 4th day: nothing until the first sighting, then a slowed, pitched-down Song of Healing.
 *  - Session 3: the Happy Mask Salesman's theme, slowed and lowered, in Termina Field and on the road to the swamp
 *    (STORY_BIBLE.md C14). The swamp and Deku Palace are silent.
 *  - Session 7: the Final Hours, slowed and lowered.
 *  - Otherwise, as on the 4th day.
 *  - BEN's own file, and wherever a session ends: silence.
 */
u16 EnBen_GetSceneSeq(EnBen* this, PlayState* play) {
    if ((this->actor.params == BEN_ACTOR_BOOT) || (this->actor.params == BEN_ACTOR_FINALE)) {
        return NA_BGM_DISABLED;
    }
    if (Ben_GetSession() == BEN_SESSION_BEN) {
        return ((play->sceneId == SCENE_00KEIKOKU) || (play->sceneId == SCENE_24KEMONOMITI))
                   ? NA_BGM_HAPPY_MASK_SALESMAN
                   : NA_BGM_DISABLED;
    }
    if (Ben_GetSession() == BEN_SESSION_FREE) {
        return NA_BGM_FINAL_HOURS;
    }
    return (Ben_GetSightings() > 0) ? NA_BGM_SONG_OF_HEALING : NA_BGM_DISABLED;
}

/**
 * Tell the scene what its music is (it only listens while it's loading, or when it next restarts its music).
 */
void EnBen_SetSceneSeq(EnBen* this, PlayState* play) {
    u16 seqId = EnBen_GetSceneSeq(this, play);

    play->sceneSequences.seqId = (seqId == NA_BGM_DISABLED) ? NA_BGM_NO_MUSIC : seqId;
}

/**
 * Keeps the scene's music what the story wants, slowed and lowered, whatever else tries to start music.
 */
void EnBen_UpdateMusic(EnBen* this, PlayState* play) {
    u16 activeSeqId = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
    u16 seqId = EnBen_GetSceneSeq(this, play);

    if (this->musicCooldown > 0) {
        this->musicCooldown--;
    }

    if (seqId == NA_BGM_DISABLED) {
        if ((activeSeqId != NA_BGM_DISABLED) && (this->musicCooldown == 0)) {
            SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 20);
            this->musicCooldown = EN_BEN_MUSIC_RETRY_FRAMES;
        }
        return;
    }

    if (activeSeqId != seqId) {
        this->musicWarped = false;
        if (this->musicCooldown == 0) {
            SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, seqId);
            this->musicCooldown = EN_BEN_MUSIC_RETRY_FRAMES;
        }
    } else if (!this->musicWarped) {
        if (seqId == NA_BGM_HAPPY_MASK_SALESMAN) {
            SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, EN_BEN_SALESMAN_TEMPO_SCALE);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, EN_BEN_SALESMAN_FREQ_SCALE);
        } else if (BEN_DEMO_TEASER && Ben_HasRewound()) {
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
    if (EnBen_EndComes(this, play)) {
        return;
    }
    if ((Ben_GetSession() == BEN_SESSION_COLD) && !this->inWhiteout) {
        // Session 4: the snow comes in first, and when it clears, it's there.
        this->inWhiteout = true;
        this->whiteFrames = EN_BEN_WHITEOUT_FRAMES;
        this->timer = EN_BEN_WHITEOUT_FRAMES - EN_BEN_WHITEOUT_PEAK;
        return;
    }
    if (this->inWhiteout ? EnBen_TryPlaceInWhiteout(this, play) : EnBen_TryAppear(this, play)) {
        this->inWhiteout = false;
        this->isHidden = false;
        this->isNoticed = false;
        this->seenFrames = 0;
        this->unseenFrames = 0;
        this->creepFails = 0;
        this->stareFrames = 0;
        this->timer = EN_BEN_PATIENCE;
        this->actionFunc = EnBen_Stand;
    } else if (this->inWhiteout && (this->whiteFrames == 0)) {
        this->inWhiteout = false; // nowhere to stand this time; the snow clears on nothing
        this->timer = EN_BEN_RETRY_FRAMES;
    } else {
        this->timer = this->inWhiteout ? 0 : EN_BEN_RETRY_FRAMES;
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
                    EnBen_SetSceneSeq(this, play); // the music wakes up
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
        } else if (Ben_GetSession() == BEN_SESSION_FREE) {
            // Session 7: the rules are off. It moves while you watch.
            this->watchFrames++;
            if ((this->watchFrames >= EN_BEN_WATCH_STEP) &&
                EnBen_FindSpot(play, player, Math_Vec3f_Yaw(&player->actor.world.pos, &this->actor.world.pos),
                               CLAMP_MIN(dist * EN_BEN_CREEP_STEP, EN_BEN_CREEP_CLOSEST), &pos)) {
                EnBen_PlaceAt(this, play, &pos);
                Actor_PlaySfx(&this->actor, EN_BEN_STEP_SFX);
                this->watchFrames = 0;
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
        Audio_PlaySfx(EN_BEN_LAUGH_SFX);
        Ben_StartSessionBen(play); // and it turns you into him
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

    if ((play->sceneId != SCENE_ALLEY) || (Ben_GetDayFour() != BEN_DAY_FOUR_HAUNTED) ||
        (Ben_GetFate() != BEN_FATE_NONE) || (play->transitionTrigger != TRANS_TRIGGER_OFF)) {
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

/**
 * Session 3 (STORY_BIBLE.md C12). Dawn in South Clock Town, and you're a Deku scrub. The camera hangs low and
 * crooked. When it rights itself, the Happy Mask Salesman is standing there, and he knows. Then he laughs, and you're
 * in Termina Field.
 */
void EnBen_SalesmanScene(EnBen* this, PlayState* play) {
    static s16 sYaws[] = { 0x8000, 0x6000, -0x6000, 0x4000, -0x4000, 0 }; // behind you first
    Player* player = GET_PLAYER(play);
    Vec3f eye;
    Vec3f at;
    Vec3f pos;
    s32 i;

    this->directorTimer++;

    switch (this->directorStep) {
        case 0: // let the flash clear
            if ((this->directorTimer >= EN_BEN_SALESMAN_START) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
                (play->transitionMode == TRANS_MODE_OFF) && (play->msgCtx.msgMode == MSGMODE_NONE)) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Cutscene_StartManual(play, &play->csCtx);
                this->subCamId = Play_CreateSubCamera(play);
                Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STATUS_WAIT);
                Play_ChangeCameraStatus(play, this->subCamId, CAM_STATUS_ACTIVE);
                this->directorStep = 1;
                this->directorTimer = 0;
            }
            break;

        case 1: // down on the ground, tilted, looking up past him
            eye.x = player->actor.world.pos.x + (Math_SinS(player->actor.shape.rot.y + 0x2800) * 75.0f);
            eye.y = player->actor.world.pos.y + 4.0f;
            eye.z = player->actor.world.pos.z + (Math_CosS(player->actor.shape.rot.y + 0x2800) * 75.0f);
            at = player->actor.world.pos;
            at.y += 35.0f;
            Play_SetCameraAtEye(play, this->subCamId, &at, &eye);
            Play_SetCameraRoll(play, this->subCamId, EN_BEN_SALESMAN_ROLL);

            if (this->directorTimer >= EN_BEN_SALESMAN_ANGLE_FRAMES) {
                // While you couldn't see behind you, he got there.
                pos = player->actor.world.pos;
                pos.x -= Math_SinS(player->actor.shape.rot.y) * EN_BEN_SALESMAN_BEHIND;
                pos.z -= Math_CosS(player->actor.shape.rot.y) * EN_BEN_SALESMAN_BEHIND;
                for (i = 0; i < ARRAY_COUNT(sYaws); i++) {
                    if (EnBen_FindSpot(play, player, player->actor.shape.rot.y + sYaws[i], EN_BEN_SALESMAN_BEHIND,
                                       &pos)) {
                        break;
                    }
                }
                this->other = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_OSN, pos.x, pos.y, pos.z, 0,
                                          Math_Vec3f_Yaw(&pos, &player->actor.world.pos), 0, OSN_TYPE_CUTSCENE);
                if (this->other == NULL) {
                    this->directorStep = 4; // nobody came; just go
                } else {
                    this->directorStep = 2;
                }
                this->directorTimer = 0;
            }
            break;

        case 2: // there he is
        case 3:
            eye.x = player->actor.world.pos.x +
                    (Math_SinS(Math_Vec3f_Yaw(&this->other->world.pos, &player->actor.world.pos)) * 90.0f);
            eye.y = player->actor.world.pos.y + 15.0f;
            eye.z = player->actor.world.pos.z +
                    (Math_CosS(Math_Vec3f_Yaw(&this->other->world.pos, &player->actor.world.pos)) * 90.0f);
            at = this->other->world.pos;
            at.y += 60.0f;
            Play_SetCameraAtEye(play, this->subCamId, &at, &eye);
            Play_SetCameraRoll(play, this->subCamId, 0);

            if ((this->directorStep == 2) && (this->directorTimer >= EN_BEN_SALESMAN_REVEAL_FRAMES)) {
                Message_StartTextbox(play, BEN_TEXT_SALESMAN, this->other);
                this->directorStep = 3;
            } else if ((this->directorStep == 3) && (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING)) {
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                this->directorStep = 4;
                this->directorTimer = 0;
            }
            break;

        case 4: // and you're in Termina Field
            if (this->directorTimer >= EN_BEN_SALESMAN_CUT_FRAMES) {
                func_80169AFC(play, this->subCamId, 0);
                Cutscene_StopManual(play, &play->csCtx);
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                Ben_SetStep(BEN_STEP3_FIELD);
                play->nextEntrance = ENTRANCE(TERMINA_FIELD, 0);
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_INSTANT;
                this->directorStep = 5;
            }
            break;

        default:
            break;
    }
}

/**
 * Session 3's end. The Deku King's chamber is empty, and it's on the throne... no, in front of it, waiting. Walk up to
 * it or look at it too long, and the game gives out. When it comes back, your file is called DROWNED, and BEN is in
 * the other slot (STORY_BIBLE.md C15).
 */
void EnBen_Finale(EnBen* this, PlayState* play) {
    static f32 sScales[] = { 1.0f, 0.75f, 0.5f };
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 i;

    this->directorTimer++;

    switch (this->directorStep) {
        case 0: // somewhere in front of you, once you're in
            if ((this->directorTimer < EN_BEN_SALESMAN_START) || (play->csCtx.state != CS_STATE_IDLE) ||
                (play->msgCtx.msgMode != MSGMODE_NONE)) {
                break;
            }
            for (i = 0; i < ARRAY_COUNT(sScales); i++) {
                if (EnBen_FindSpot(play, player, player->actor.shape.rot.y, EN_BEN_FINALE_DIST * sScales[i], &pos)) {
                    EnBen_PlaceAt(this, play, &pos);
                    this->isHidden = false;
                    this->kind = EN_BEN_SIGHTING_STILL;
                    this->seenFrames = 0;
                    EnBen_SetStaring(this, true);
                    this->directorStep = 1;
                    this->directorTimer = 0;
                    break;
                }
            }
            break;

        case 1: // it just waits for you
            if (EnBen_IsAnyVisible(this, play)) {
                this->seenFrames++;
            }
            if ((this->seenFrames >= EN_BEN_FINALE_STARE) || (this->actor.xzDistToPlayer < EN_BEN_FINALE_NEAR)) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Play_EnableMotionBlur(EN_BEN_BOOT_BLUR_ALPHA);
                this->blurFrames = 0x7FFF; // Destroy turns it off
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                this->directorStep = 2;
                this->directorTimer = 0;
            }
            break;

        case 2:
            if (this->directorTimer < EN_BEN_FINALE_GIVE_OUT) {
                break;
            }
            if (Ben_GetSession() == BEN_SESSION_BEN) {
                // The game gives out. When it comes back: BEN, and DROWNED.
                this->directorStep = 5;
                Sram_BenEndSessionBen(play);
                Ben_CrashToTitle(play);
                break;
            }
            Message_StartTextbox(play, (Ben_GetSession() == BEN_SESSION_COLD) ? BEN_TEXT_COLD : BEN_TEXT_LETS_PLAY,
                                 NULL);
            this->directorStep = 3;
            break;

        case 3:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                this->directorStep = 4;
                this->directorTimer = 0;
            }
            break;

        case 4:
            if (this->directorTimer < EN_BEN_SALESMAN_CUT_FRAMES) {
                break;
            }
            this->directorStep = 5;
            if (Ben_GetSession() == BEN_SESSION_COLD) {
                Ben_AdvanceSession(play, BEN_SESSION_DROWNED); // Session 5: Great Bay
                Ben_WarpToSession(play);
            } else {
                // Session 7: "Let's play." In Majora's lair.
                Ben_SetStep(BEN_STEP7_LAIR);
                Sram_BenCommitStory(play);
                play->nextEntrance = ENTRANCE(MAJORAS_LAIR, 0);
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_WHITE;
            }
            break;

        default:
            break;
    }
}

/**
 * Sessions 4 and 7: if you haven't come to the end, it comes to you. Session 6: the dream. Returns true if the next
 * appearance is that.
 */
s32 EnBen_EndComes(EnBen* this, PlayState* play) {
    if ((this->actor.params != BEN_ACTOR_STATUE) || (Ben_GetFate() != BEN_FATE_NONE)) {
        return false;
    }
    switch (Ben_GetSession()) {
        case BEN_SESSION_COLD:
        case BEN_SESSION_FREE:
            if (Ben_GetSightings() < EN_BEN_END_SIGHTINGS) {
                return false;
            }
            this->actionFunc = EnBen_Finale;
            break;

        case BEN_SESSION_STONE:
            if (Ben_GetSightings() < EN_BEN_DREAM_SIGHTINGS) {
                return false;
            }
            this->actionFunc = EnBen_Dream;
            break;

        default:
            return false;
    }
    this->directorStep = 0;
    this->directorTimer = 0;
    this->inWhiteout = false;
    return true;
}

/**
 * Session 4: while the snow is all you can see, it stands in front of you, about as far off as this sighting calls for.
 */
s32 EnBen_TryPlaceInWhiteout(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play));
    f32 dist = sSightingDistances[CLAMP_MAX(Ben_GetSightings(), ARRAY_COUNT(sSightingDistances) - 1)] * 0.5f;
    Vec3f pos;
    s32 d;

    for (d = 0; d < ARRAY_COUNT(sDistanceScales); d++) {
        if (EnBen_FindSpot(play, player, yaw, dist * sDistanceScales[d], &pos)) {
            EnBen_PlaceAt(this, play, &pos);
            this->kind = EN_BEN_SIGHTING_STILL;
            this->trailCount = 0;
            return true;
        }
    }
    return false;
}

/**
 * Session 4: the blizzard. White, all the way, for a moment.
 */
void EnBen_UpdateWhiteout(EnBen* this, PlayState* play) {
    s32 alpha;

    if (this->whiteFrames <= 0) {
        return;
    }
    this->whiteFrames--;
    Audio_PlaySfx(NA_SE_EV_SNOWSTORM_HARD - SFX_FLAG);
    if (this->whiteFrames == 0) {
        Play_FillScreen(play, false, 0, 0, 0, 0);
        return;
    }
    // In over 16 frames, all white for 8, out over 16.
    alpha = (this->whiteFrames >= 24)   ? ((EN_BEN_WHITEOUT_FRAMES - this->whiteFrames) * 16)
            : (this->whiteFrames >= 16) ? 255
                                        : (this->whiteFrames * 16);
    Play_FillScreen(play, true, 235, 240, 255, CLAMP_MAX(alpha, 255));
}

/**
 * Who else is here. Nobody, in Clock Town on the 4th day and at the end, or in Deku Palace, or by Darmani's grave.
 * Elsewhere people are where they always are (Mikau is still washed up on the beach).
 */
void EnBen_EmptyScene(EnBen* this, PlayState* play) {
    if ((this->actor.params == BEN_ACTOR_BOOT) || Ben_IsHauntedScene(play->sceneId)) {
        EnBen_EmptyTown(play);
        return;
    }
    switch (Ben_GetSession()) {
        case BEN_SESSION_BEN:
            if ((play->sceneId == SCENE_22DEKUCITY) || (play->sceneId == SCENE_DEKU_KING)) {
                EnBen_EmptyTown(play);
            }
            break;

        case BEN_SESSION_COLD:
            if (play->sceneId == SCENE_GORON_HAKA) {
                EnBen_EmptyTown(play);
            }
            break;

        case BEN_SESSION_FREE:
            if ((play->sceneId == SCENE_OKUJOU) || (play->sceneId == SCENE_LAST_BS)) {
                EnBen_EmptyLair(play); // not even Skull Kid, not even the Mask
            } else {
                EnBen_EmptyTown(play);
            }
            break;

        default:
            break;
    }
}

/**
 * Session 5 (STORY_BIBLE.md C16): swim out into the open sea and it takes you.
 */
void EnBen_CheckSea(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if ((this->actor.params != BEN_ACTOR_STATUE) || (Ben_GetSession() != BEN_SESSION_DROWNED) ||
        (Ben_GetStep() != BEN_STEP5_SEA) || ((play->sceneId != SCENE_30GYOSON) && (play->sceneId != SCENE_31MISAKI)) ||
        (this->actionFunc == EnBen_Drowning) || (play->transitionTrigger != TRANS_TRIGGER_OFF)) {
        return;
    }

    // Out where it's deep. After enough sightings, anywhere you swim.
    if ((player->actor.bgCheckFlags & BGCHECKFLAG_WATER) && (player->actor.depthInWater > EN_BEN_DROWN_DEPTH) &&
        (((player->actor.world.pos.y - player->actor.floorHeight) > EN_BEN_SEA_DEPTH) ||
         (Ben_GetSightings() >= EN_BEN_END_SIGHTINGS))) {
        this->drownFrames++;
    } else {
        this->drownFrames = 0;
    }

    if (this->drownFrames >= EN_BEN_SEA_FRAMES) {
        this->directorStep = 0;
        this->directorTimer = 0;
        this->actionFunc = EnBen_Drowning;
    }
}

/**
 * Session 5's end. It's standing on the water in front of you, and you go under. A red flash, his laugh, and you're
 * on the Moon (STORY_BIBLE.md C16, C18).
 */
void EnBen_Drowning(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f pos;

    this->directorTimer++;
    player->actor.freezeTimer = 2; // you can't swim away

    switch (this->directorStep) {
        case 0:
            Player_PlaySfx(player, NA_SE_VO_LI_TAKEN_AWAY + player->ageProperties->voiceSfxIdOffset);
            Audio_PlaySfx(NA_SE_OC_ABYSS);
            pos = player->actor.world.pos;
            pos.x += Math_SinS(player->actor.shape.rot.y) * EN_BEN_SEA_STATUE;
            pos.z += Math_CosS(player->actor.shape.rot.y) * EN_BEN_SEA_STATUE;
            pos.y += player->actor.depthInWater - 20.0f; // up to its knees in the sea
            EnBen_PlaceAt(this, play, &pos);
            this->actor.gravity = 0.0f; // it doesn't sink
            this->isHidden = false;
            this->trailCount = 0;
            EnBen_SetStaring(this, true);
            Play_EnableMotionBlur(EN_BEN_BOOT_BLUR_ALPHA);
            this->blurFrames = 0x7FFF; // Destroy turns it off
            this->directorStep = 1;
            break;

        case 1:
            if (this->directorTimer == EN_BEN_SEA_FLASH) {
                Play_FillScreen(play, true, 140, 0, 0, 255);
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
            } else if (this->directorTimer == (EN_BEN_SEA_FLASH + 3)) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
            } else if (this->directorTimer >= EN_BEN_SEA_LEAVE) {
                this->directorStep = 2;
                Ben_SetStep(BEN_STEP5_MOON);
                Sram_BenCommitStory(play);
                Ben_WarpToSession(play);
            }
            break;

        default:
            break;
    }
}

/**
 * Sessions 4-6: you don't get to stay in Clock Town. Whenever you're back (the Song of Time, a game over, loading the
 * file), his laugh and a white flash take you back to where the session is.
 */
void EnBen_WarpAway(EnBen* this, PlayState* play) {
    this->directorTimer++;
    if ((this->directorStep == 0) && (this->directorTimer >= EN_BEN_WARP_DELAY) &&
        (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF) &&
        (play->csCtx.state == CS_STATE_IDLE) && (play->msgCtx.msgMode == MSGMODE_NONE)) {
        Audio_PlaySfx(EN_BEN_LAUGH_SFX);
        Ben_WarpToSession(play);
        this->directorStep = 1;
    }
}

/**
 * Session 5's end, on the Moon (STORY_BIBLE.md C18, 5 and a half). Silence, then the children's question, rewritten.
 * Then Stone Tower.
 */
void EnBen_MoonScene(EnBen* this, PlayState* play) {
    this->directorTimer++;

    switch (this->directorStep) {
        case 0:
            if ((this->directorTimer >= EN_BEN_MOON_DELAY) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
                (play->transitionMode == TRANS_MODE_OFF) && (play->msgCtx.msgMode == MSGMODE_NONE)) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Message_StartTextbox(play, BEN_TEXT_MOON, NULL);
                this->directorStep = 1;
            }
            break;

        case 1:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                this->directorStep = 2;
                this->directorTimer = 0;
            }
            break;

        case 2:
            if (this->directorTimer >= EN_BEN_MOON_LEAVE) {
                this->directorStep = 3;
                Ben_AdvanceSession(play, BEN_SESSION_STONE); // Session 6
                Ben_WarpToSession(play);
            }
            break;

        default:
            break;
    }
}

/**
 * Session 6's end (STORY_BIBLE.md C20): the dream where it makes you into a statue. It stands in front of you and
 * comes closer while you watch. Link gets harder to move, then moves backwards, then won't move at all. Then it
 * speaks, and the next thing you know it's the Night of the Final Day.
 */
void EnBen_Dream(EnBen* this, PlayState* play) {
    static f32 sScales[] = { 1.0f, 0.75f, 0.5f };
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 i;

    this->directorTimer++;

    switch (this->directorStep) {
        case 0:
            if ((play->csCtx.state != CS_STATE_IDLE) || (play->msgCtx.msgMode != MSGMODE_NONE)) {
                break;
            }
            for (i = 0; i < ARRAY_COUNT(sScales); i++) {
                if (EnBen_FindSpot(play, player, player->actor.shape.rot.y, EN_BEN_DREAM_DIST * sScales[i], &pos)) {
                    EnBen_PlaceAt(this, play, &pos);
                    this->isHidden = false;
                    this->trailCount = 0;
                    EnBen_SetStaring(this, true);
                    Ben_SetInputMode(BEN_INPUT_SLOW);
                    this->directorStep = 1;
                    this->directorTimer = 0;
                    break;
                }
            }
            break;

        case 1:
            if (((this->directorTimer % EN_BEN_DREAM_STEP) == 0) &&
                (this->actor.xzDistToPlayer > (EN_BEN_CREEP_CLOSEST * 1.2f)) &&
                EnBen_FindSpot(play, player, Math_Vec3f_Yaw(&player->actor.world.pos, &this->actor.world.pos),
                               CLAMP_MIN(this->actor.xzDistToPlayer * 0.7f, EN_BEN_CREEP_CLOSEST), &pos)) {
                EnBen_PlaceAt(this, play, &pos); // in plain sight, this once
                Actor_PlaySfx(&this->actor, EN_BEN_STEP_SFX);
            }
            if (this->directorTimer == EN_BEN_DREAM_INVERT) {
                Ben_SetInputMode(BEN_INPUT_INVERTED);
            } else if (this->directorTimer == EN_BEN_DREAM_REFUSE) {
                Ben_SetInputMode(BEN_INPUT_REFUSED);
                Play_EnableMotionBlur(EN_BEN_BLUR_ALPHA);
                this->blurFrames = 0x7FFF; // Destroy turns it off
            } else if ((this->directorTimer >= EN_BEN_DREAM_SPEAK) && (play->msgCtx.msgMode == MSGMODE_NONE)) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Message_StartTextbox(play, BEN_TEXT_DREAM, NULL);
                this->directorStep = 2;
            }
            break;

        case 2:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Play_FillScreen(play, true, 0, 0, 0, 255);
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                this->directorStep = 3;
                this->directorTimer = 0;
            }
            break;

        case 3:
            if (this->directorTimer >= EN_BEN_FINALE_GIVE_OUT) {
                // Session 7 (free): you wake up in Clock Town, and it's an hour before the end.
                this->directorStep = 4;
                Ben_AdvanceSession(play, BEN_SESSION_FREE);
                gSaveContext.save.day = 3;
                gSaveContext.save.time = CLOCK_TIME(23, 0);
                gSaveContext.save.isNight = true;
                play->nextEntrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
                play->transitionTrigger = TRANS_TRIGGER_START;
                play->transitionType = TRANS_TYPE_FADE_BLACK_SLOW;
            }
            break;

        default:
            break;
    }
}

/**
 * The end (STORY_BIBLE.md C21). Majora's lair, empty. It comes towards you whatever you do, and when it reaches you
 * the screen goes black: "The counter resets. I'm glad you did that." Your file is his now.
 */
void EnBen_LastGame(EnBen* this, PlayState* play) {
    static f32 sScales[] = { 1.0f, 0.75f, 0.5f };
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 i;

    this->directorTimer++;

    switch (this->directorStep) {
        case 0:
            if ((this->directorTimer < EN_BEN_SALESMAN_START) || (play->csCtx.state != CS_STATE_IDLE) ||
                (play->msgCtx.msgMode != MSGMODE_NONE)) {
                break;
            }
            for (i = 0; i < ARRAY_COUNT(sScales); i++) {
                if (EnBen_FindSpot(play, player, player->actor.shape.rot.y, EN_BEN_LAST_DIST * sScales[i], &pos)) {
                    EnBen_PlaceAt(this, play, &pos);
                    this->isHidden = false;
                    this->trailCount = 0;
                    EnBen_SetStaring(this, true);
                    this->directorStep = 1;
                    this->directorTimer = 0;
                    break;
                }
            }
            break;

        case 1: // it doesn't care whether you're looking
            if (this->actor.xzDistToPlayer < EN_BEN_LAST_NEAR) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Play_FillScreen(play, true, 0, 0, 0, 255);
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                this->directorStep = 2;
                this->directorTimer = 0;
            } else if (((this->directorTimer % EN_BEN_LAST_STEP) == 0) &&
                       EnBen_FindSpot(play, player, Math_Vec3f_Yaw(&player->actor.world.pos, &this->actor.world.pos),
                                      this->actor.xzDistToPlayer * 0.75f, &pos)) {
                EnBen_PlaceAt(this, play, &pos);
                Actor_PlaySfx(&this->actor, EN_BEN_STEP_SFX);
            }
            break;

        case 2:
            if (this->directorTimer >= EN_BEN_FINALE_GIVE_OUT) {
                Message_StartTextbox(play, BEN_TEXT_COUNTER, NULL);
                this->directorStep = 3;
            }
            break;

        case 3:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                this->directorStep = 4;
                this->directorTimer = 0;
            }
            break;

        case 4:
            if (this->directorTimer >= EN_BEN_SALESMAN_CUT_FRAMES) {
                this->directorStep = 5;
                Sram_BenEndGame(play);
                Ben_CrashToTitle(play);
            }
            break;

        default:
            break;
    }
}

/**
 * BEN's own file. You're at Stone Tower an hour before the moon falls, with every mask, and something is wrong with
 * the text. The statue is somewhere, and gets closer whenever you look away. Then the game gives out and you're back
 * at the title screen.
 */
void EnBen_BootDirector(EnBen* this, PlayState* play) {
    this->directorTimer++;

    switch (this->directorStep) {
        case 0: // stand there a moment, then the box
            if ((this->directorTimer >= EN_BEN_BOOT_TEXT_DELAY) && (play->msgCtx.msgMode == MSGMODE_NONE)) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                Message_StartTextbox(play, BEN_TEXT_STONE, NULL);
                this->directorStep = 1;
            }
            break;

        case 1:
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                this->directorStep = 2;
                this->directorTimer = 0;
            }
            break;

        case 2: // walk around, if you want
            if (this->directorTimer >= EN_BEN_BOOT_ROAM) {
                EnBen_SetStaring(this, true);
                Play_EnableMotionBlur(EN_BEN_BOOT_BLUR_ALPHA);
                this->blurFrames = 0x7FFF; // Destroy turns it off
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
                this->directorStep = 3;
                this->directorTimer = 0;
            }
            break;

        case 3: // and it's gone
            if (this->directorTimer >= EN_BEN_BOOT_GIVE_OUT) {
                this->directorStep = 4;
                Ben_CrashToTitle(play);
            }
            break;

        default:
            break;
    }
}

void EnBen_Update(Actor* thisx, PlayState* play) {
    EnBen* this = (EnBen*)thisx;

    if (this->actor.params == BEN_ACTOR_LAIR) {
        EnBen_EmptyLair(play);
        this->actionFunc(this, play);
        return;
    }
    if ((this->actor.params == BEN_ACTOR_SALESMAN) || (this->actor.params == BEN_ACTOR_WARP) ||
        (this->actor.params == BEN_ACTOR_MOON)) {
        if (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) != NA_BGM_DISABLED) {
            SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0);
        }
        this->actionFunc(this, play);
        return;
    }

    if (this->actor.params == BEN_ACTOR_BOOT) {
        EnBen_BootDirector(this, play);
    }

    // The clock doesn't move on the 4th day.
    if (Ben_IsHauntedScene(play->sceneId)) {
        gSaveContext.save.timeSpeedOffset = -(s16)R_TIME_SPEED;
    }

    EnBen_UpdateMusic(this, play);
    EnBen_EmptyScene(this, play);
    EnBen_CheckDrowning(this, play);
    EnBen_CheckSea(this, play);
    EnBen_RememberPath(this, play);

    this->actionFunc(this, play);

    EnBen_UpdateWhiteout(this, play);
    if ((Ben_GetSession() == BEN_SESSION_STONE) && (this->actionFunc == EnBen_Stand)) {
        // Session 6: every time you look at it, Link's legs go the wrong way.
        Ben_SetInputMode((this->isNoticed && !this->isHidden) ? BEN_INPUT_INVERTED : BEN_INPUT_NORMAL);
    } else if ((Ben_GetSession() == BEN_SESSION_STONE) && (this->actionFunc != EnBen_Dream)) {
        Ben_SetInputMode(BEN_INPUT_NORMAL);
    }
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
