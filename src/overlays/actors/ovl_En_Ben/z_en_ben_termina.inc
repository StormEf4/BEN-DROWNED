/**
 * Termina, a little wrong (STORY_BIBLE.md §4 rule 5: dread, not jump scares). Included by z_en_ben.c.
 *
 * From Session 3 on, wherever the statue isn't stalking you and nothing's scripted (Ben_GetDirector's fallback), the
 * rest of Termina isn't safe either. Every few minutes, one small thing, never the same twice running, and then
 * nothing again for a long while. More of them become possible as the story goes on:
 *  Session 3
 *  - somebody nearby stops what they're doing and stares at you;
 *  - everybody nearby turns to look at you at once;
 *  - everybody nearby turns their back on you;
 *  - a figure a long way off at the edge of the screen, standing still, gone once you look straight at it;
 *  - his laugh, a long way behind you;
 *  - the music sags, slower and lower, and comes back;
 *  - the clock stops for a while, and then goes on as if it hadn't;
 *  - Tatl says something she shouldn't ("Who were you talking to just now?");
 *  - somewhere, faintly, someone plays your song backwards (the Song of Unhealing: a hint at the secret);
 *  Session 4
 *  - stone footsteps behind you, closer, and then they stop;
 *  - the colour goes out of everything for a few seconds;
 *  - the music cuts out completely, and comes back;
 *  - your heartbeat, with nothing there;
 *  - the fog closes in and the light goes, and lifts again;
 *  Session 5
 *  - your hearts beep as if you're dying, though you aren't;
 *  - it's right behind you. Turn round, and it's gone, laughing. If you don't, you never knew;
 *  - the screen goes black for a blink, and it's standing in front of you; another blink and it isn't (Screen
 *    flashes on only);
 *  Session 6
 *  - for a moment Link's legs go the wrong way, the way they do when you look at it at Stone Tower.
 * None of it counts as a sighting, so none of it moves the story on. It keeps out of the way of conversations,
 * cutscenes, the game's minigames and the Moon Children, and it comes a little more often as the story goes on.
 *
 * All of this is the Expanded Horror (BEN_EXPANDED, DECISIONS D17): with it off there's no Termina director at all.
 * It also sends the visitors from other stories (En_Ben_Visitor, En_Ben_Smile, En_Ben_Sleep): once in a while, a little
 * after you arrive somewhere one of them belongs, one of them comes instead (the Tall Man in the woods and the fields,
 * the Smiling Man in town at night, "Go to sleep." indoors at night), and nothing else happens while it's here. Three
 * of the SCP Foundation's come the same way (En_Ben_Scp173, 096 and 106; DECISIONS D18): the sculpture in the temples
 * and dark places, the shy one in the open country, the old man in the swamp and the graveyard at night and under the
 * graveyard.
 */

#define EN_BEN_TERMINA_FIRST 900           // after you arrive, the first thing is at least this long (45 s)
#define EN_BEN_TERMINA_EVERY 2400          // then at least this long between them (2 min)
#define EN_BEN_TERMINA_RANGE 2400          // plus up to this much more
#define EN_BEN_TERMINA_SOONER 300          // each session past the third takes this off the wait (Session 7: 1-3 min)
#define EN_BEN_TERMINA_RETRY 100           // if now's no good (you're talking), try again this soon
#define EN_BEN_TERMINA_STARE_DIST 600.0f   // only people this close, and on screen
#define EN_BEN_TERMINA_STARE_FRAMES 70     // and they stare this long
#define EN_BEN_TERMINA_CROWD_DIST 900.0f   // everybody: everybody this close
#define EN_BEN_TERMINA_CROWD_FEWEST 2      // and there have to be at least this many of them
#define EN_BEN_TERMINA_CROWD_FRAMES 90     // turned this long
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
#define EN_BEN_TERMINA_CLOCK_FRAMES 400    // the clock stops for this long (20 s)
#define EN_BEN_TERMINA_TATL_LINES 4        // what Tatl might say (BEN_TEXT_TATL_1 on)
#define EN_BEN_TERMINA_MELODY_VOLUME 45    // your song backwards, this loud (the statue's is 80)
#define EN_BEN_TERMINA_MELODY_LONGEST 300  // and never longer than this
#define EN_BEN_TERMINA_GREY 150            // the colour goes this far (0-255)
#define EN_BEN_TERMINA_GREY_FRAMES 80      // for this long (4 s)
#define EN_BEN_TERMINA_GREY_STEP 4         // fading this much a frame
#define EN_BEN_TERMINA_SILENCE_FRAMES 120  // the music's gone for this long (6 s)
#define EN_BEN_TERMINA_HEART_FRAMES 100    // your heart, this long (5 s)
#define EN_BEN_TERMINA_FOG_STEP 6          // the fog comes in this much a frame (0-255)
#define EN_BEN_TERMINA_FOG_HOLD 80         // and stays this long before it lifts
#define EN_BEN_TERMINA_ALARM_EVERY 12      // the hearts beep this often
#define EN_BEN_TERMINA_ALARM_BEEPS 6       // this many times
#define EN_BEN_TERMINA_BEHIND_DIST 170.0f  // it's this far behind you
#define EN_BEN_TERMINA_BEHIND_SEEN 4       // in view this long and it's gone
#define EN_BEN_TERMINA_BEHIND_LONGEST 200  // and gone after this long anyway (10 s)
#define EN_BEN_TERMINA_BLINK_DIST 450.0f   // the blink: it's this far in front of you
#define EN_BEN_TERMINA_BLINK_DARK 4        // each blink is this long
#define EN_BEN_TERMINA_BLINK_SHOWN 26      // and it's there this long between them
#define EN_BEN_TERMINA_WRONG_FRAMES 40     // Link's legs go the wrong way this long (2 s)
#define EN_BEN_VISITOR_FIRST 400           // a visitor: at least this long after you arrive (20 s)
#define EN_BEN_VISITOR_RANGE 800           // plus up to this much more (so 20-60 s)

typedef enum EnBenTerminaEvent {
    /*  0 */ EN_BEN_TERMINA_WAIT,
    /*  1 */ EN_BEN_TERMINA_STARE,
    /*  2 */ EN_BEN_TERMINA_FIGURE,
    /*  3 */ EN_BEN_TERMINA_STEPS,
    /*  4 */ EN_BEN_TERMINA_LAUGH,
    /*  5 */ EN_BEN_TERMINA_MUSIC,
    /*  6 */ EN_BEN_TERMINA_GREY_OUT,
    /*  7 */ EN_BEN_TERMINA_CROWD,
    /*  8 */ EN_BEN_TERMINA_BACKS,
    /*  9 */ EN_BEN_TERMINA_CLOCK,
    /* 10 */ EN_BEN_TERMINA_TATL,
    /* 11 */ EN_BEN_TERMINA_MELODY,
    /* 12 */ EN_BEN_TERMINA_SILENCE,
    /* 13 */ EN_BEN_TERMINA_HEART,
    /* 14 */ EN_BEN_TERMINA_FOG,
    /* 15 */ EN_BEN_TERMINA_ALARM,
    /* 16 */ EN_BEN_TERMINA_BEHIND,
    /* 17 */ EN_BEN_TERMINA_BLINK,
    /* 18 */ EN_BEN_TERMINA_WRONG_WAY,
    /* 19 */ EN_BEN_TERMINA_VISITOR // someone from another story is here (this->other)
} EnBenTerminaEvent;

typedef struct EnBenTerminaLine {
    /* 0x0 */ u8 event;
    /* 0x1 */ u8 fromSession; // the first session it can happen in
} EnBenTerminaLine;

void EnBen_Termina(EnBen* this, PlayState* play);

static EnBenTerminaLine sTerminaEvents[] = {
    { EN_BEN_TERMINA_STARE, BEN_SESSION_BEN },     { EN_BEN_TERMINA_CROWD, BEN_SESSION_BEN },
    { EN_BEN_TERMINA_BACKS, BEN_SESSION_BEN },     { EN_BEN_TERMINA_FIGURE, BEN_SESSION_BEN },
    { EN_BEN_TERMINA_LAUGH, BEN_SESSION_BEN },     { EN_BEN_TERMINA_MUSIC, BEN_SESSION_BEN },
    { EN_BEN_TERMINA_CLOCK, BEN_SESSION_BEN },     { EN_BEN_TERMINA_TATL, BEN_SESSION_BEN },
    { EN_BEN_TERMINA_MELODY, BEN_SESSION_BEN },    { EN_BEN_TERMINA_STEPS, BEN_SESSION_COLD },
    { EN_BEN_TERMINA_GREY_OUT, BEN_SESSION_COLD }, { EN_BEN_TERMINA_SILENCE, BEN_SESSION_COLD },
    { EN_BEN_TERMINA_HEART, BEN_SESSION_COLD },    { EN_BEN_TERMINA_FOG, BEN_SESSION_COLD },
    { EN_BEN_TERMINA_ALARM, BEN_SESSION_DROWNED }, { EN_BEN_TERMINA_BEHIND, BEN_SESSION_DROWNED },
    { EN_BEN_TERMINA_BLINK, BEN_SESSION_DROWNED }, { EN_BEN_TERMINA_WRONG_WAY, BEN_SESSION_STONE },
};

typedef struct EnBenVisitorLine {
    /* 0x0 */ s16 sceneId;
    /* 0x2 */ u8 visitor;   // BEN_VISITOR_*
    /* 0x3 */ u8 nightOnly; // only between dusk and dawn
    /* 0x4 */ f32 chance;   // how likely it comes, the one time a scene visit gives it the chance
} EnBenVisitorLine;

// Which actor each of them is (BEN_VISITOR_*): each has its own, so only its model is in memory
static s16 sVisitorActors[BEN_VISITOR_COUNT] = {
    ACTOR_EN_BEN_VISITOR, ACTOR_EN_BEN_SMILE,  ACTOR_EN_BEN_SLEEP,
    ACTOR_EN_BEN_SCP173,  ACTOR_EN_BEN_SCP096, ACTOR_EN_BEN_SCP106,
};

// Where each of them belongs
static EnBenVisitorLine sVisitors[] = {
    { SCENE_26SARUNOMORI, BEN_VISITOR_TALL, false, 0.35f }, // the Woods of Mystery
    { SCENE_00KEIKOKU, BEN_VISITOR_TALL, false, 0.35f },    // Termina Field
    { SCENE_24KEMONOMITI, BEN_VISITOR_TALL, false, 0.35f }, // the road to the swamp
    { SCENE_20SICHITAI, BEN_VISITOR_TALL, false, 0.35f },   // the Southern Swamp
    { SCENE_20SICHITAI2, BEN_VISITOR_TALL, false, 0.35f },  // the Southern Swamp, cleansed
    { SCENE_21MITURINMAE, BEN_VISITOR_TALL, false, 0.35f }, // Woodfall
    { SCENE_IKANA, BEN_VISITOR_TALL, false, 0.35f },        // Ikana Canyon
    { SCENE_IKANAMAE, BEN_VISITOR_TALL, false, 0.35f },     // the road to Ikana
    { SCENE_F01, BEN_VISITOR_TALL, false, 0.35f },          // Romani Ranch
    { SCENE_BOTI, BEN_VISITOR_TALL, false, 0.35f },         // the graveyard
    { SCENE_ROMANYMAE, BEN_VISITOR_TALL, false, 0.2f },     // Milk Road, by day or night
    { SCENE_ROMANYMAE, BEN_VISITOR_SMILE, true, 0.25f },    // and at night, him too
    { SCENE_TOWN, BEN_VISITOR_SMILE, true, 0.4f },          // East Clock Town
    { SCENE_ICHIBA, BEN_VISITOR_SMILE, true, 0.4f },        // West Clock Town
    { SCENE_BACKTOWN, BEN_VISITOR_SMILE, true, 0.4f },      // North Clock Town
    { SCENE_CLOCKTOWER, BEN_VISITOR_SMILE, true, 0.4f },    // South Clock Town
    { SCENE_ALLEY, BEN_VISITOR_SMILE, true, 0.4f },         // the Laundry Pool
    { SCENE_YADOYA, BEN_VISITOR_SLEEP, true, 0.3f },        // the Stock Pot Inn
    { SCENE_MILK_BAR, BEN_VISITOR_SLEEP, true, 0.3f },      // the Milk Bar
    { SCENE_SONCHONOIE, BEN_VISITOR_SLEEP, true, 0.3f },    // the Mayor's Residence
    { SCENE_AYASHIISHOP, BEN_VISITOR_SLEEP, true, 0.3f },   // the Curiosity Shop
    { SCENE_8ITEMSHOP, BEN_VISITOR_SLEEP, true, 0.3f },     // the Trading Post
    { SCENE_OMOYA, BEN_VISITOR_SLEEP, true, 0.3f },         // the ranch house
    { SCENE_MUSICHOUSE, BEN_VISITOR_SLEEP, true, 0.3f },    // the Music Box House
    { SCENE_DOUJOU, BEN_VISITOR_SLEEP, true, 0.3f },        // the Swordsman's School
    { SCENE_POSTHOUSE, BEN_VISITOR_SLEEP, true, 0.3f },     // the Post Office
    { SCENE_BOMYA, BEN_VISITOR_SLEEP, true, 0.3f },         // the Bomb Shop
    { SCENE_TAKARAYA, BEN_VISITOR_SLEEP, true, 0.3f },      // the Treasure Chest Shop
    { SCENE_LABO, BEN_VISITOR_SLEEP, true, 0.3f },          // the Marine Research Lab
    { SCENE_KAJIYA, BEN_VISITOR_SLEEP, true, 0.3f },        // the Mountain Smithy
    // The SCP Foundation's (En_Ben_Scp173, 096 and 106)
    { SCENE_MITURIN, BEN_VISITOR_SCULPTURE, false, 0.3f },     // Woodfall Temple
    { SCENE_HAKUGIN, BEN_VISITOR_SCULPTURE, false, 0.3f },     // Snowhead Temple
    { SCENE_SEA, BEN_VISITOR_SCULPTURE, false, 0.3f },         // Great Bay Temple
    { SCENE_INISIE_N, BEN_VISITOR_SCULPTURE, false, 0.3f },    // Stone Tower Temple
    { SCENE_CASTLE, BEN_VISITOR_SCULPTURE, false, 0.3f },      // Ancient Castle of Ikana
    { SCENE_PIRATE, BEN_VISITOR_SCULPTURE, false, 0.3f },      // the Pirates' Fortress, inside
    { SCENE_KAKUSIANA, BEN_VISITOR_SCULPTURE, false, 0.3f },   // the grottos
    { SCENE_REDEAD, BEN_VISITOR_SCULPTURE, false, 0.3f },      // Beneath the Well
    { SCENE_KINDAN2, BEN_VISITOR_SCULPTURE, false, 0.3f },     // the Oceanside Spider House
    { SCENE_00KEIKOKU, BEN_VISITOR_SHY, false, 0.2f },         // Termina Field
    { SCENE_30GYOSON, BEN_VISITOR_SHY, false, 0.25f },         // the Great Bay Coast
    { SCENE_31MISAKI, BEN_VISITOR_SHY, false, 0.25f },         // Zora Cape
    { SCENE_IKANA, BEN_VISITOR_SHY, false, 0.25f },            // Ikana Canyon
    { SCENE_10YUKIYAMANOMURA, BEN_VISITOR_SHY, false, 0.25f }, // Mountain Village, in winter
    { SCENE_13HUBUKINOMITI, BEN_VISITOR_SHY, false, 0.25f },   // the road to Snowhead
    { SCENE_20SICHITAI, BEN_VISITOR_OLD_MAN, true, 0.3f },     // the Southern Swamp, at night
    { SCENE_20SICHITAI2, BEN_VISITOR_OLD_MAN, true, 0.3f },    // cleansed, at night
    { SCENE_BOTI, BEN_VISITOR_OLD_MAN, true, 0.3f },           // the graveyard, at night
    { SCENE_ALLEY, BEN_VISITOR_OLD_MAN, true, 0.2f },          // the Laundry Pool, at night
    { SCENE_IKANAMAE, BEN_VISITOR_OLD_MAN, true, 0.25f },      // the road to Ikana, at night
    { SCENE_HAKASHITA, BEN_VISITOR_OLD_MAN, false, 0.35f },    // under the graveyard
};

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
    this->fog = 0;
    this->musicWarped = false;
    this->lastEvent = EN_BEN_TERMINA_WAIT;
    this->other = NULL;
    this->directorStep = EN_BEN_TERMINA_WAIT;
    this->directorTimer = 0;
    this->stepTimer = 0;
    this->timer = EN_BEN_TERMINA_FIRST + (s32)Rand_ZeroFloat(EN_BEN_TERMINA_RANGE);
    this->visitorTimer = EN_BEN_VISITOR_FIRST + (s32)Rand_ZeroFloat(EN_BEN_VISITOR_RANGE);
    this->visitorTried = false;
    this->actionFunc = EnBen_Termina;
}

/**
 * Whether the visitor that was sent is still here.
 */
s32 EnBen_VisitorHere(PlayState* play, Actor* visitor) {
    Actor* it;

    for (it = play->actorCtx.actorLists[ACTORCAT_PROP].first; it != NULL; it = it->next) {
        if (it == visitor) {
            return visitor->update != NULL;
        }
    }
    return false;
}

/**
 * Once a scene visit, a little after you arrive: if one of the visitors belongs here at this hour and hasn't been by
 * for a while, maybe it comes. Returns true if one did.
 */
s32 EnBen_TryVisitor(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 start = (s32)Rand_ZeroFloat(ARRAY_COUNT(sVisitors)); // where more than one belongs, any of them first
    s32 n;

    for (n = 0; n < ARRAY_COUNT(sVisitors); n++) {
        EnBenVisitorLine* line = &sVisitors[(start + n) % ARRAY_COUNT(sVisitors)];

        if ((line->sceneId != play->sceneId) || (line->nightOnly && !gSaveContext.save.isNight) ||
            !Ben_VisitorReady(line->visitor) || (Rand_ZeroOne() >= line->chance)) {
            continue;
        }
        this->other = Actor_Spawn(&play->actorCtx, play, sVisitorActors[line->visitor], player->actor.world.pos.x,
                                  player->actor.world.pos.y, player->actor.world.pos.z, 0, 0, 0, line->visitor);
        if (this->other == NULL) {
            return false;
        }
        Ben_VisitorCame(line->visitor);
        this->directorStep = EN_BEN_TERMINA_VISITOR;
        this->directorTimer = 0;
        return true;
    }
    return false;
}

/**
 * The fog and the light, `fog` of the way to how they are when it stares at you (0-255).
 */
void EnBen_TerminaSetFog(PlayState* play, s16 fog) {
    static s16 sFogColor[] = { 12, 6, 14 };
    EnvironmentContext* envCtx = &play->envCtx;
    f32 dread = fog / 255.0f;
    s32 i;

    envCtx->adjLightSettings.fogNear = (envCtx->lightSettings.fogNear > EN_BEN_FOG_NEAR)
                                           ? (EN_BEN_FOG_NEAR - envCtx->lightSettings.fogNear) * dread
                                           : 0;
    for (i = 0; i < 3; i++) {
        envCtx->adjLightSettings.fogColor[i] = (sFogColor[i] - envCtx->lightSettings.fogColor[i]) * dread;
        envCtx->adjLightSettings.ambientColor[i] =
            -envCtx->lightSettings.ambientColor[i] * EN_BEN_AMBIENT_DRAIN * dread;
    }
}

/**
 * What the current thing changed, put back: the clock, the music's volume, the screen, the stick.
 */
void EnBen_TerminaRestore(EnBen* this, PlayState* play) {
    switch (this->directorStep) {
        case EN_BEN_TERMINA_CLOCK:
            gSaveContext.save.timeSpeedOffset = this->savedSpeed;
            break;

        case EN_BEN_TERMINA_SILENCE:
            Audio_SetMainBgmVolume(0x7F, 20);
            break;

        case EN_BEN_TERMINA_BLINK:
            Play_FillScreen(play, false, 0, 0, 0, 0);
            break;

        case EN_BEN_TERMINA_WRONG_WAY:
            Ben_SetInputMode(BEN_INPUT_NORMAL);
            break;

        default:
            break;
    }
}

void EnBen_TerminaDestroy(EnBen* this, PlayState* play) {
    EnBen_TerminaRestore(this, play);
    if (this->musicWarped) {
        SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, 100);
        SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, 1000);
    }
    if (this->grey > 0) {
        gPlayVisMonoColor.a = 0;
    }
    if (this->fog > 0) {
        EnBen_TerminaSetFog(play, 0);
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
 * Whether you're looking straight at it: in plain view, near the middle of the screen.
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
 * About `dist` along `yaw` from you, where it would (`visible`) or wouldn't be in plain view.
 */
s32 EnBen_TryPlaceAlong(EnBen* this, PlayState* play, s16 yaw, f32 dist, s32 visible) {
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    s32 d;

    for (d = 0; d < ARRAY_COUNT(sDistanceScales); d++) {
        if (EnBen_FindSpot(play, player, yaw, dist * sDistanceScales[d], &pos) &&
            (!EnBen_IsBodyVisible(play, &pos) == !visible)) {
            EnBen_PlaceAt(this, play, &pos);
            return true;
        }
    }
    return false;
}

/**
 * Everybody within `range` (the game's people, not its enemies): how many there are. If `turn` isn't 0, also turns
 * them all to look at you (1) or away from you (-1), and holds them there.
 */
s32 EnBen_TerminaPeople(PlayState* play, f32 range, s32 turn) {
    Player* player = GET_PLAYER(play);
    Actor* it;
    s32 count = 0;

    for (it = play->actorCtx.actorLists[ACTORCAT_NPC].first; it != NULL; it = it->next) {
        if ((it->update == NULL) || (Actor_WorldDistXZToActor(&player->actor, it) >= range)) {
            continue;
        }
        count++;
        if (turn != 0) {
            it->shape.rot.y = it->world.rot.y =
                Actor_WorldYawTowardActor(it, &player->actor) + ((turn > 0) ? 0 : 0x8000);
            it->freezeTimer = 2;
        }
    }
    return count;
}

/**
 * Starts the next thing. Returns false if it couldn't (nobody to stare, nowhere to stand), so another can be tried.
 */
s32 EnBen_TerminaStart(EnBen* this, PlayState* play, s32 event) {
    Player* player = GET_PLAYER(play);
    s16 camYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play));
    Vec3f pos;
    Vec3f projected;
    f32 invW;

    this->directorTimer = 0;
    this->seenFrames = 0;
    switch (event) {
        case EN_BEN_TERMINA_STARE:
            this->other = EnBen_FindPerson(play, EN_BEN_TERMINA_STARE_DIST, true);
            if (this->other == NULL) {
                return false;
            }
            break;

        case EN_BEN_TERMINA_CROWD:
            if (EnBen_TerminaPeople(play, EN_BEN_TERMINA_CROWD_DIST, 0) < EN_BEN_TERMINA_CROWD_FEWEST) {
                return false;
            }
            break;

        case EN_BEN_TERMINA_BACKS:
            if (EnBen_FindPerson(play, EN_BEN_TERMINA_STARE_DIST, true) == NULL) {
                return false; // nobody you'd see do it
            }
            break;

        case EN_BEN_TERMINA_FIGURE:
            if (!EnBen_TryPlaceAlong(
                    this, play,
                    camYaw + ((Rand_ZeroOne() < 0.5f) ? EN_BEN_TERMINA_FIGURE_ANGLE : -EN_BEN_TERMINA_FIGURE_ANGLE),
                    EN_BEN_TERMINA_FIGURE_DIST, true)) {
                return false;
            }
            this->isHidden = false;
            break;

        case EN_BEN_TERMINA_STEPS:
            this->timer = EN_BEN_STEPS_BEFORE; // EnBen_Footsteps counts this down
            this->stepTimer = 0;
            break;

        case EN_BEN_TERMINA_LAUGH:
            pos.x = player->actor.world.pos.x - (Math_SinS(camYaw) * EN_BEN_TERMINA_LAUGH_DIST);
            pos.y = player->actor.world.pos.y;
            pos.z = player->actor.world.pos.z - (Math_CosS(camYaw) * EN_BEN_TERMINA_LAUGH_DIST);
            Actor_GetProjectedPos(play, &pos, &projected, &invW);
            this->stepPos = projected;
            Audio_PlaySfx_AtPosWithPresetLowFreqAndHighReverb(&this->stepPos, EN_BEN_LAUGH_SFX);
            break; // nothing more than that

        case EN_BEN_TERMINA_MUSIC:
            if (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_DISABLED) {
                return false; // nothing playing to go wrong
            }
            SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, EN_BEN_TERMINA_MUSIC_TEMPO);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, EN_BEN_TERMINA_MUSIC_FREQ);
            this->musicWarped = true;
            break;

        case EN_BEN_TERMINA_CLOCK:
            this->savedSpeed = gSaveContext.save.timeSpeedOffset;
            gSaveContext.save.timeSpeedOffset = -(s16)R_TIME_SPEED;
            break;

        case EN_BEN_TERMINA_TATL:
            if ((player->tatlActor == NULL) || !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                return false;
            }
            Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
            Message_StartTextbox(
                play, BEN_TEXT_TATL_1 + ((s32)Rand_ZeroFloat(EN_BEN_TERMINA_TATL_LINES) % EN_BEN_TERMINA_TATL_LINES),
                player->tatlActor);
            break;

        case EN_BEN_TERMINA_MELODY:
            Ben_PlayUnhealing(EN_BEN_TERMINA_MELODY_VOLUME);
            break;

        case EN_BEN_TERMINA_SILENCE:
            if (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_DISABLED) {
                return false;
            }
            Audio_SetMainBgmVolume(0, 0);
            break;

        case EN_BEN_TERMINA_ALARM:
            this->timer = EN_BEN_TERMINA_ALARM_BEEPS * EN_BEN_TERMINA_ALARM_EVERY;
            break;

        case EN_BEN_TERMINA_BEHIND:
            if (!EnBen_TryPlaceAlong(this, play, camYaw + 0x8000, EN_BEN_TERMINA_BEHIND_DIST, false)) {
                return false;
            }
            this->isHidden = false;
            break;

        case EN_BEN_TERMINA_BLINK:
            if (!BEN_FLASHES || !EnBen_TryPlaceAlong(this, play, camYaw, EN_BEN_TERMINA_BLINK_DIST, true)) {
                return false;
            }
            Play_FillScreen(play, true, 0, 0, 0, 255);
            break;

        case EN_BEN_TERMINA_WRONG_WAY:
            if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                return false;
            }
            Ben_SetInputMode(BEN_INPUT_INVERTED);
            break;

        case EN_BEN_TERMINA_GREY_OUT:
        case EN_BEN_TERMINA_HEART:
        case EN_BEN_TERMINA_FOG:
            break;

        default:
            return false;
    }
    this->directorStep = event;
    this->lastEvent = event;
    return true;
}

/**
 * Picks one of the session's things at random (never the last one again), trying the rest in turn if it can't happen
 * here and now.
 */
void EnBen_TerminaPick(EnBen* this, PlayState* play) {
    s32 count = ARRAY_COUNT(sTerminaEvents);
    s32 first = (s32)Rand_ZeroFloat(count) % count;
    s32 i;

    for (i = 0; i < count; i++) {
        EnBenTerminaLine* line = &sTerminaEvents[(first + i) % count];

        if ((line->fromSession > Ben_GetSession()) || (line->event == this->lastEvent)) {
            continue;
        }
        if (EnBen_TerminaStart(this, play, line->event)) {
            return;
        }
    }
}

void EnBen_TerminaEnd(EnBen* this, PlayState* play) {
    EnBen_TerminaRestore(this, play);
    this->isHidden = true;
    this->other = NULL;
    this->directorStep = EN_BEN_TERMINA_WAIT;
    this->timer = EnBen_TerminaWait();
}

void EnBen_Termina(EnBen* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 busy;

    this->directorTimer++;

    // The colour and the light come back on their own, whatever happens next
    if (!EnBen_ChildrenHere(this, play)) {
        if ((this->directorStep != EN_BEN_TERMINA_GREY_OUT) && (this->grey > 0)) {
            Math_StepToS(&this->grey, 0, EN_BEN_TERMINA_GREY_STEP);
            gPlayVisMonoColor.a = this->grey;
        }
        if ((this->directorStep != EN_BEN_TERMINA_FOG) && (this->fog > 0)) {
            Math_StepToS(&this->fog, 0, EN_BEN_TERMINA_FOG_STEP);
            EnBen_TerminaSetFog(play, this->fog);
        }
    }

    switch (this->directorStep) {
        case EN_BEN_TERMINA_WAIT:
            if (!this->visitorTried) {
                if (this->visitorTimer > 0) {
                    this->visitorTimer--;
                } else if (!EnBen_TerminaBusy(this, play)) {
                    this->visitorTried = true;
                    if (EnBen_TryVisitor(this, play)) {
                        return;
                    }
                }
            }
            if (this->timer > 0) {
                this->timer--;
                return;
            }
            if (EnBen_TerminaBusy(this, play)) {
                this->timer = EN_BEN_TERMINA_RETRY;
                return;
            }
            this->timer = EnBen_TerminaWait(); // if nothing at all can happen here, wait as long again
            EnBen_TerminaPick(this, play);
            return;

        case EN_BEN_TERMINA_TATL: // Tatl's own conversation
            if (Message_GetState(&play->msgCtx) == TEXT_STATE_CLOSING) {
                Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
                EnBen_TerminaEnd(this, play);
            }
            return;

        case EN_BEN_TERMINA_VISITOR: // nothing else while it's here, and a long quiet after
            if (!EnBen_VisitorHere(play, this->other)) {
                EnBen_TerminaEnd(this, play);
            }
            return;

        case EN_BEN_TERMINA_MELODY: // your song backwards, until it's done
            if (!Ben_IsMelodyPlaying() || (this->directorTimer >= EN_BEN_TERMINA_MELODY_LONGEST)) {
                EnBen_TerminaEnd(this, play);
            }
            return;

        default:
            break;
    }

    // Everything else stops the moment something else wants you
    busy = EnBen_TerminaBusy(this, play);

    switch (this->directorStep) {
        case EN_BEN_TERMINA_STARE: // somebody stops, and looks at you
            if (busy || !EnBen_IsStillHere(play, this->other) || (this->directorTimer >= EN_BEN_TERMINA_STARE_FRAMES)) {
                EnBen_TerminaEnd(this, play);
                break;
            }
            this->other->shape.rot.y = this->other->world.rot.y =
                Actor_WorldYawTowardActor(this->other, &player->actor);
            this->other->freezeTimer = 2;
            break;

        case EN_BEN_TERMINA_CROWD: // everybody, all at once
        case EN_BEN_TERMINA_BACKS:
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_CROWD_FRAMES)) {
                EnBen_TerminaEnd(this, play);
                break;
            }
            EnBen_TerminaPeople(play, EN_BEN_TERMINA_CROWD_DIST, (this->directorStep == EN_BEN_TERMINA_CROWD) ? 1 : -1);
            break;

        case EN_BEN_TERMINA_FIGURE: // standing out there; look at it, and it isn't
            if (EnBen_FigureLookedAt(this, play)) {
                this->seenFrames++;
            }
            if (busy || (this->seenFrames >= EN_BEN_TERMINA_FIGURE_SEEN) ||
                (this->directorTimer >= EN_BEN_TERMINA_FIGURE_LONGEST)) {
                EnBen_TerminaEnd(this, play); // no laugh, no smear: was it ever there?
            }
            break;

        case EN_BEN_TERMINA_STEPS: // closer, closer, and then nothing
            EnBen_Footsteps(this, play);
            if (this->timer > 0) {
                this->timer--;
            } else {
                EnBen_TerminaEnd(this, play);
            }
            break;

        case EN_BEN_TERMINA_LAUGH:
            EnBen_TerminaEnd(this, play);
            break;

        case EN_BEN_TERMINA_MUSIC:
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_MUSIC_FRAMES)) {
                SEQCMD_SCALE_TEMPO(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, 100);
                SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, EN_BEN_TERMINA_MUSIC_EASE, 1000);
                this->musicWarped = false;
                EnBen_TerminaEnd(this, play);
            }
            break;

        case EN_BEN_TERMINA_CLOCK: // and then it goes on as if it never stopped
            gSaveContext.save.timeSpeedOffset = -(s16)R_TIME_SPEED;
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_CLOCK_FRAMES)) {
                EnBen_TerminaEnd(this, play);
            }
            break;

        case EN_BEN_TERMINA_GREY_OUT:
            Math_StepToS(&this->grey, EN_BEN_TERMINA_GREY, EN_BEN_TERMINA_GREY_STEP);
            gPlayVisMonoColor.r = 255;
            gPlayVisMonoColor.g = 255;
            gPlayVisMonoColor.b = 255;
            gPlayVisMonoColor.a = this->grey;
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_GREY_FRAMES)) {
                EnBen_TerminaEnd(this, play); // and it fades back in above
            }
            break;

        case EN_BEN_TERMINA_SILENCE:
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_SILENCE_FRAMES)) {
                EnBen_TerminaEnd(this, play); // the music comes back in
            }
            break;

        case EN_BEN_TERMINA_HEART:
            Audio_PlaySfx(NA_SE_EN_LAST2_HEARTBEAT_OLD - SFX_FLAG);
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_HEART_FRAMES)) {
                EnBen_TerminaEnd(this, play);
            }
            break;

        case EN_BEN_TERMINA_FOG:
            if (this->fog < 255) {
                Math_StepToS(&this->fog, 255, EN_BEN_TERMINA_FOG_STEP);
                this->directorTimer = 0; // the hold starts once it's all the way in
            }
            EnBen_TerminaSetFog(play, this->fog);
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_FOG_HOLD)) {
                EnBen_TerminaEnd(this, play); // and it lifts above
            }
            break;

        case EN_BEN_TERMINA_ALARM: // your hearts say you're dying
            if ((this->timer % EN_BEN_TERMINA_ALARM_EVERY) == 0) {
                Audio_PlaySfx(NA_SE_SY_HITPOINT_ALARM);
            }
            this->timer--;
            if (busy || (this->timer <= 0)) {
                EnBen_TerminaEnd(this, play);
            }
            break;

        case EN_BEN_TERMINA_BEHIND: // right behind you; turn round, and it's gone
            if (EnBen_IsBodyVisible(play, &this->actor.world.pos)) {
                this->seenFrames++;
            }
            if (this->seenFrames >= EN_BEN_TERMINA_BEHIND_SEEN) {
                Audio_PlaySfx(EN_BEN_LAUGH_SFX);
                EnBen_TerminaEnd(this, play);
            } else if (busy || (this->directorTimer >= EN_BEN_TERMINA_BEHIND_LONGEST)) {
                EnBen_TerminaEnd(this, play); // you never knew
            }
            break;

        case EN_BEN_TERMINA_BLINK: // dark; it's there; dark; it isn't
            if (busy) {
                EnBen_TerminaEnd(this, play);
            } else if (this->directorTimer == EN_BEN_TERMINA_BLINK_DARK) {
                Play_FillScreen(play, false, 0, 0, 0, 0);
                this->isHidden = false;
            } else if (this->directorTimer == (EN_BEN_TERMINA_BLINK_DARK + EN_BEN_TERMINA_BLINK_SHOWN)) {
                Play_FillScreen(play, true, 0, 0, 0, 255);
                this->isHidden = true;
            } else if (this->directorTimer >= (2 * EN_BEN_TERMINA_BLINK_DARK + EN_BEN_TERMINA_BLINK_SHOWN)) {
                EnBen_TerminaEnd(this, play);
            }
            break;

        case EN_BEN_TERMINA_WRONG_WAY:
            if (busy || (this->directorTimer >= EN_BEN_TERMINA_WRONG_FRAMES)) {
                EnBen_TerminaEnd(this, play);
            }
            break;

        default:
            EnBen_TerminaEnd(this, play);
            break;
    }
}
