# The full route: Sessions 0 to 7

Every session is one 3-day cycle (STORY_BIBLE.md §2, §3). Between the scripted moments you're playing real Majora's
Mask: walking, swimming, fighting what's there. The statue stalks you through each session's region the way it
stalks Clock Town on the 4th day. The story is saved with the file (`SaveInfo.unk_DF4`), and each time a session ends
it's written to the cartridge right away, so turning the game off doesn't undo it.

If you can't reach a session's ending (it needs an item you don't have, say), **the ending comes to you**: after
enough sightings, the next one is the ending.

**The start.** A new file skips the vanilla intro: you step out of the Clock Tower into South Clock Town on the
first morning, human, with the ocarina. A few seconds later Tatl speaks up on her own: three days and then the moon,
"...Right?", and *"If you don't know where to go, listen for me, and press [C-Up]. I'll know."* That's the whole tutorial.

**Finding the way.** Tatl knows where the story wants you, and she doesn't like it. When she starts flashing, press
C-up: every step of the story has a line from her that points the way without spelling it out (the Song of Double
Time's notes, the Laundry Pool, the swamp, Darmani's grave, the open sea, the tower at midnight). From Session 3 on,
every one of them ends in the Salesman's laugh. The game's Tatl goes quiet for twenty minutes after a
hint; for the story's she calls again after four, so it's never far off when you're lost. In Session 1 a townsperson whispers the 4th-day rumor too.

| Session | Where | What happens | How it ends |
|---|---|---|---|
| 0 **BOOT** | File select, Stone Tower | On a blank cartridge, slot 1 already holds a file called **BEN**. Open it: Stone Tower an hour before the moon falls, a garbled "St o n e" box, the statue, then the game gives out. | Back to the title screen. Erase BEN or use slot 2. |
| 1 **Something's wrong** | Clock Town (and anywhere with people) | Tatl's opening lines, then the town is almost normal. Now and then somebody stops dead and stares at you a beat too long, in Clock Town or wherever you've wandered. The first night you walk up to somebody, they turn and whisper: *"They say there's a fourth day... They say a boy found it. He never came home."* | Playing the Song of Double Time on the final night, and once more. |
| 2 **day four** | Clock Town | The demo (DEMO.md): the 4th day, the empty town, the Laundry Pool, Link burning. | The Song of Time, and "YOU SHOULDN'T HAVE DONE THAT." |
| 3 **BEN** | South Clock Town → Termina Field → Southern Swamp → Deku Palace | A white flash and you're a **Deku scrub** at dawn. The camera hangs low and crooked; when it rights itself the **Happy Mask Salesman** is behind you and knows. A cut to Termina Field, where his theme plays slowed and low. From now on **every Tatl line ends in his laugh**. Go back into the **Clock Tower** and he says it again, the line from the start of the game: *"You've met with a terrible fate, haven't you?"* The swamp and the palace are silent, and the palace is empty. | It's waiting in the **Deku King's chamber**. Walk up to it or stare and the game gives out. The file select now reads **DROWNED** (your file) and **BEN** (the other slot, if it was empty). |
| 4 **Cold** | Mountain Village, the paths, Snowhead, Goron Village | South Clock Town is closed: a laugh and a white flash put you back in the snow. **Blizzard whiteouts**: the screen goes white, and when it clears the statue is standing in front of you. | **Darmani's grave** in the Goron Graveyard, or after 7 sightings. There are two statues. The other one is Darmani. |
| 5 **DROWNED** | Great Bay Coast, Zora Cape, Zora Hall | Mikau is still washed up on the beach. **The game starts to break**: now and then the screen floods with a wrong color, the music lurches, and sometimes **BEN's face** is there for a frame. | **Swim out into the open sea.** After 3 seconds out there you go under, the statue is standing on the water in front of you, a red flash and the laugh. After 7 sightings, anywhere you swim. |
| 5½ **Moon** | The Moon | Silence. *"Your friend is waiting at the stone tower."* | Stone Tower. |
| 6 **St o n e** | Stone Tower (both ways up), Ikana Canyon, Ikana Graveyard | **Every time you look at it, the stick is backwards.** Mind the cliffs. Sometimes there's **a second statue** a little way off, facing away. It isn't him. Play the **Elegy of Emptiness** and when you turn round, one of the shells isn't yours. | **The dream** (after 6 sightings): it stands in front of you and comes closer *while you watch*. Link gets harder to move, then moves backwards, then won't move. *"Now you look like me."* |
| 7 **free** | Clock Town on the Night of the Final Day | You wake an hour before the end, every time, however you got here. The Final Hours, slowed. **The rules are off: it moves while you watch.** | The **top of the Clock Tower** at midnight (or 7 sightings): *"Let's play."* Then Majora's lair, empty, and it comes for you whatever you do. Black: **"The counter resets. I'm glad you did that."** Your file is renamed **BEN**, and from now on it opens at Stone Tower like his did. |

### Anywhere he stalks you

- **Play the Song of Healing at it.** Once a session, it answers: *"That song doesn't work on me."* If it isn't there
  when you play it, it heard, and you hear it coming.
- **Erase the BEN file** at the file select and he laughs.
- Flashing can be turned off with the builder's **Screen flashes** option (`BEN_FLASHES`).

### Two ways to play

The builder asks which you want (`BEN_EXPANDED`, DECISIONS D17):

- **The Jadusable Experience**: BEN's story as it was first told. Every session, the statue, the scripted scenes, hide
  and seek and the Song of Unhealing are all there; Termina outside the story stays quiet.
- **The Expanded Horror** (the default): the same story, plus the two sections below, *Termina, a little wrong* and
  *Visitors from other stories*.

### Termina, a little wrong (Session 3 on, Expanded Horror)

Everywhere the story isn't (the rest of Termina, inside and out, but never the boss rooms or the Moon), every two to
four minutes one small thing happens, never the same one twice running, and then nothing again for a long while. It's
a little more often each session, and more kinds of thing become possible as the story goes on:

| From | What can happen |
|---|---|
| Session 3 | Somebody nearby stares at you. Everybody nearby turns to look at you at once, or all turn their backs on you. A figure far off at the edge of the screen, gone when you look straight at it. His laugh, far behind you. The music sags, slower and lower. The clock stops for 20 seconds. Tatl says something she shouldn't ("Who were you talking to just now?", "Have we been here before?", "Don't look behind us.", "Why does everyone keep calling you that?"). Somewhere, faintly, someone plays the Song of Unhealing: a hint at the secret. |
| Session 4 | Stone footsteps behind you, closer, then nothing. The colour drains for a few seconds. The music cuts out completely, then comes back. Your heartbeat, with nothing there. The fog closes in and the light goes, then lifts. |
| Session 5 | Your hearts beep as if you're dying. It's right behind you: turn round and it's gone, laughing (if you don't turn, you never know). A blink of black, it's standing in front of you, another blink and it's gone (only with Screen flashes on). |
| Session 6 | For two seconds Link's legs go the wrong way, the way they do when you look at it at Stone Tower. |

None of it counts as a sighting, so it never moves the story on, and it stays out of conversations, cutscenes, the
game's minigames and the Moon Children. Anything it changes (the clock, the music, the colour, the stick) is put back
when it's over or when you leave.

### Visitors from other stories (Session 3 on, Expanded Horror)

Sometimes, 20 to 60 seconds after you arrive somewhere one of them belongs, someone who isn't BEN comes instead of the
small scares above. They're never named. Each comes at most once every three hours of the game's clock, none of them
can kill Link, and leaving the area always ends it (the screen, the lights and the music go back to normal).

| Who | Where | What to do |
|---|---|---|
| **The Tall Man**: twice Link's height, a black suit, no face | The woods and the fields, any time: Termina Field, the Woods of Mystery, the road to the swamp, the swamp, Woodfall, Milk Road, Romani Ranch, the road to Ikana, Ikana Canyon, the graveyard | Looking at him fills the screen with static (it flickers only with Screen flashes on), faster the closer he is, and the music stops. Look away and he's somewhere else, closer. If the static fills the screen, or you stare at him up close, he has you: you see what's on his back, then black, and you're back at the way in, a heart down. Glance, look away, keep moving; after 90 seconds he's gone the next time you look away. |
| **The Smiling Man**: thin, a dark suit, a grin far too wide | Clock Town's streets, the Laundry Pool and Milk Road, at night | He dances towards you, slowly while you watch, fast while you don't. Get far enough away, or out of his sight for 5 seconds, and his smile drops and he walks off. If he reaches you: his face in yours, black, two hearts. |
| **"Go to sleep."**: a white hoodie, a knife, a face that's always there | Inside at night: the inn, the Milk Bar, the Mayor's, the shops, the ranch house, the Music Box House, the dojo, the Post Office, the lab, the smithy | The lights go out and come back, again and again, the light shorter each time. He only moves in the dark, and in the dark his face is the only thing you can see. Leave the room. If he reaches you: *"Go to sleep."*, black, an hour gone (never across dawn or on the final night), a heart down. After 75 seconds, the lights come back and he isn't there. |

### Three from the SCP Foundation (Session 3 on, Expanded Horror)

Three of the SCP wiki's best known come the same way, with the same rules: never named, at most once every three game
hours each, never lethal, and leaving the area (or, in a temple, the room) ends it. Each keeps its article's rule.
Where more than one visitor belongs (Termina Field, Ikana Canyon, the swamp), which one tries first is random. The
articles are credited in [CREDITS.md](CREDITS.md).

| Who | Where | What to do |
|---|---|---|
| **The sculpture** (SCP-173): concrete and rebar, flecks of green spray paint, black eyes | The temples, the Ancient Castle, Beneath the Well, the Pirates' Fortress, the grottos, the Oceanside Spider House, any time | It can't move while you can see it. Z-target it to keep your eyes on it. But Link blinks every 4 to 6 seconds (a few black frames with Screen flashes on, a short fade with them off), and in each blink it's closer with the scrape of stone. Look away and it slides after you. If it's on you when you can't see it: it's right there, then black and a snap, three hearts, back at the way in. Its stain stays where it first stood. Gone after 90 seconds, the next time you look away. |
| **The shy one** (SCP-096): very tall, thin, pale, long arms | Termina Field, the Great Bay Coast, Zora Cape, Ikana Canyon, Mountain Village and the road to Snowhead, any time | Far off, pacing, its back to you; every so often it turns round. Don't look at its face. If you see it (a glimpse is enough): the music stops, it covers its face and screams for 8 seconds, then comes for you through anything, faster the further away you are. Leave the area. If it reaches you: black, a scream, four hearts, back at the way in. After 45 seconds it gives up and sits with its face in its hands. |
| **The old man** (SCP-106): rotten black, hunched, glinting eyes | At night: the swamp, the graveyard, the Laundry Pool, the road to Ikana. Any time: under the graveyard | Bubbling and a groan where you aren't looking: he rises out of a pool of corrosion and walks slowly to you through walls, leaving pools behind. Every 20 seconds or so he sinks and comes up again closer, sometimes where you're looking. A light arrow near him sends him down for good. If he reaches you: everything goes his colour, two hearts, back at the way in. After 90 seconds he sinks and doesn't come up. |

### Hide and seek (Sessions 4 to 6)

Once a session, after you've seen it a couple of times, the statue is standing right in front of you instead:
*"Let's play hide and seek. I hide. You look."* Black, and when the screen comes back it's gone and the HUD's clock is
counting down from 1:15. It's hiding round a corner somewhere. A child's giggle comes from where it is: louder and more
often the closer you get, and your heart starts when you're close.

- **Find it** (see it from near enough, for half a second) and it laughs and is gone, leaving a red rupee where it
  stood. It leaves you alone for a good while after that.
- **Run out of time** and the screen goes black, and when it comes back it's standing right in front of you. That
  counts as a sighting.

### The Song of Unhealing (secret)

**Learning it.** Play the Song of Healing at the statue. After *"That song doesn't work on me."* it plays your song
back to you, backwards: **C-Down, C-Right, C-Left, C-Down, C-Right, C-Left**. From Session 4 on you can play it even if
it never taught you.

**Playing it.** Anywhere, with nothing else going on. The music goes and the colour drains, and the Moon Children answer
with the next sigil, one a session ("another night"):

| Sigil | What happens |
|---|---|
| 1 **The child no one sees** | A Moon Child in a torn white robe and a blank mask stands out there, its head tilted, watching you. Walk up to it and it's gone, then it giggles from further on. It leads you four times, and the last time it's standing on a glowing sigil. |
| 2 **Don't move** | *"Stand still with us. If you move, he'll know where you are."* A minute on the HUD's clock. The child walks round and round you, its head always turned to you. Every few seconds the screen blinks (white in the snow) and the statue is somewhere new, closer; footsteps all round; his face, once. Move, and it's right in front of you (a sighting); play the song again to retry. |
| 3 **The wet feet** | Small wet footprints appear in front of you, one step at a time, walking away. If you fall behind they wait. At the end the child stands with its back to you. Get close and its head turns all the way round. |

Each ends with the sigil glowing, the moon's cry, and the Moon Children's phrase a word longer
(**THE MOON SEES THE CHILD . . .**), with a line saying what to do next time. The sigil is written to the cartridge
straight away. Played again the same session: *"...not tonight."* Sigils 4 to 7 and the CHILDREN epilogue
(STORY_BIBLE.md §5) are still to come.

## Test script

| # | Check |
|---|---|
| 1 | After "YOU SHOULDN'T HAVE DONE THAT.": laugh, white flash, Deku Link in South Clock Town, no music |
| 2 | A low tilted camera for about 3 s, then a level shot of the Salesman behind you, his four boxes, his laugh, then Termina Field |
| 3 | Termina Field and the swamp road: the Salesman's theme, slow and low. The swamp and Deku Palace: silence, and nobody in the palace |
| 4 | Talk to Tatl (C-up) anywhere: the laugh plays as her box closes |
| 5 | Deku King's chamber: the statue in front of you. Walk to it: smear, laugh, a short hang, title screen. File select shows DROWNED and BEN |
| 6 | Load DROWNED: South Clock Town at dawn, then a laugh and a white flash into Mountain Village |
| 7 | Mountain area: whiteouts with a howling storm sound, the statue in front of you as the white clears |
| 8 | Goron Graveyard: the statue, its two boxes, a white flash into Great Bay Coast. Turn the game off and back on: loading the file still takes you to Great Bay |
| 9 | Great Bay: swim well out to sea. Pulled under, the statue on the water, a red flash, then the Moon |
| 10 | The Moon: silence, the three boxes, then Stone Tower |
| 11 | Stone Tower: when you see the statue, up is down until you look away. After 6 sightings, the dream, then black, then Clock Town at 11 PM on the final night |
| 12 | Final night: the statue steps closer even while you watch it. On the Clock Tower (or the 7th sighting): "Let's play.", Majora's lair, it walks to you, black, the last line, title screen. The file is now BEN and opens at Stone Tower |
| 13 | Session 1 (after the first cycle): somebody freezes and turns to stare at you every minute or two. First night, walk up to somebody: they turn and whisper the rumor (or it's whispered from nowhere after about 45 s with nobody around) |
| 14 | Tatl flashes and C-up gives the hint for the current step, in every session; from Session 3 her lines end in the laugh |
| 15 | Song of Healing at a statue you can see: its line once, a smear, the laugh. Played with no statue around: footsteps, then it's there |
| 16 | Session 3: walk into the Clock Tower: the Salesman's line, the laugh |
| 17 | Session 5 on: occasional colored flicker and a lurch in the music, sometimes the face. Off with the Screen flashes option |
| 18 | Session 4: after two sightings, the statue in front of you: the hide and seek box, black, then the HUD clock at 1:15 and the giggle |
| 19 | Find it before the clock runs out: chime, laugh, smear, a red rupee where it stood, and a long quiet after |
| 20 | Let the clock run out (another session, or a fresh file): black, then it's right in front of you, his face for a frame, the laugh |
| 21 | Song of Healing at the statue: its line, then the reversed melody plays on the ocarina, then "...Did you hear that?" |
| 22 | Play C-Down, C-Right, C-Left twice: the correct chime, the music fades, grey. The Moon Child appears ahead; walk to it four times; the sigil, the moon's cry, "THE MOON . . ." |
| 23 | Same session, play it again: "...not tonight." Next session, play it and stand still for the minute: the blinks, the statue closer each time, the sigil under your feet, "THE MOON SEES". Move once to check it catches you |
| 24 | Next session: footprints walk away from you and wait when you stop; the child at the end turns its head round; the sigil, "THE MOON SEES THE CHILD" |
| 25 | New file: step out into South Clock Town; about 3 s later Tatl's opening lines play once, with Link held still. Save, reload: they don't play again |
| 26 | Session 1, outside Clock Town (Termina Field at night, a shop): somebody stares now and then; on the first night the rumor still comes |
| 27 | Session 3 on, somewhere the story isn't (Milk Road, Romani Ranch, a shop): stand around for 5 minutes and expect one or two small things (a stare, a far figure, a laugh, the music sagging). None during a minigame or a conversation |
| 28 | Session 4 on: the footsteps and the colour draining show up among them. Leave the scene mid-way: the music and colour are back to normal in the next one |
| 29 | Tatl's C-up in Session 4 ends "...And if we don't go to him, it'll bring us."; in Session 6, "Let it find you." |
| 30 | After reading a hint, keep wandering: about 4 minutes later Tatl calls again with the same hint |
| 31 | Session 3: over a long stretch somewhere quiet, expect the crowd turning, the clock stopping (the HUD clock freezes), a Tatl line ending in the laugh, the faint backwards song |
| 32 | Session 5: turn round now and then; sometimes it's right behind you and vanishes with a laugh. With Screen flashes off, the blink never happens |
| 33 | Session 6: once in a while the stick is backwards for two seconds, then normal again; leaving the scene mid-way, it's normal in the next |
| 34 | Build with **The Jadusable Experience**: from Session 3, nowhere outside the story has small scares or visitors; the story itself plays the same |
| 35 | Expanded Horror, Session 3, Termina Field: within a minute or so of arriving the Tall Man is sometimes behind you. Looking at him brings static; looking away, he's closer. Let the static fill up: black, back at the entrance, a heart down |
| 36 | The Tall Man again: glance and look away for 90 s; he goes, the static and music come back. Leave the field mid-way: the next area is normal |
| 37 | Expanded Horror, Session 3, East Clock Town at night: the Smiling Man dances towards you; run until he's far, or break his line of sight: he frowns and walks off. Let him reach you: black, two hearts, never lethal on low health |
| 38 | Expanded Horror, Session 3, inside the Milk Bar or a shop at night: the lights go out and come back; he only moves in the dark, his face bright. Let him reach you: "Go to sleep.", black, an hour later on the clock, a heart down. Walk out mid-way: the next area's lights are normal |
| 39 | A visitor doesn't come back to the same kind of place for three game hours, and none appears during a conversation or a minigame |
| 40 | Expanded Horror, Session 3, a temple or Beneath the Well: the sculpture is somewhere behind you. Watch it (Z-target works): it doesn't move between blinks, and is closer after each. Turn away: the scrape, and it's on you: black, a snap, three hearts, back at the way in. Never lethal on low health |
| 41 | The sculpture with Screen flashes off: the blinks are short fades, never a hard cut. Walk into the next room mid-way: the screen is normal |
| 42 | Expanded Horror, Session 3, Termina Field or Ikana Canyon: the shy one far off, pacing. Keep it at the edge of the screen and don't look at its face when it turns: nothing happens and it goes after 2 minutes |
| 43 | Look at its face: the music stops, it screams into its hands for 8 s, then runs at you through walls. Leave the area before it reaches you: the next area's music is normal. Let it reach you: four hearts, never below one |
| 44 | Expanded Horror, Session 3, the swamp or the graveyard at night (or under the graveyard): bubbling and a groan; the old man rises from a pool behind you, walks through walls, leaves pools, sinks and comes up closer. Fire a light arrow near him: he sinks and is gone. Let him reach you: brown-black, two hearts, back at the way in |

## Tuning

Termina's small scares are `EN_BEN_TERMINA_*` in `z_en_ben_termina.inc` (how often: `_FIRST`, `_EVERY`, `_RANGE`,
`_SOONER`); where the visitors come and how likely, its `sVisitors` table and `EN_BEN_VISITOR_*`, and how often each
can come, `BEN_VISITOR_EVERY` in `src/ben/ben_core.c`. What the visitors do is `TALL_*`, `SMILE_*` and `SLEEP_*` in
`src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.c`, and the SCPs' is `STATUE_*`, `SHY_*` and `OLD_*` in
`src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_scp.inc`. The rest are all in `src/overlays/actors/ovl_En_Ben/z_en_ben.c`: `EN_BEN_END_SIGHTINGS` (when the ending comes to you),
`EN_BEN_DREAM_SIGHTINGS`, `EN_BEN_SEA_DEPTH` (if the open sea never takes you, lower it), `EN_BEN_WHITEOUT_*`, and the
Salesman scene's `EN_BEN_SALESMAN_*`. Where each session puts you is `Ben_WarpToSession` in `src/ben/ben_core.c`.
Hide and seek's numbers are `EN_BEN_SEEK_*` in `z_en_ben_seek.inc`; the sigil tasks' are `EN_BEN_CHILD_*`,
`EN_BEN_STILL_*` and `EN_BEN_FEET_*` in `z_en_ben_children.inc`.

## Models

The Moon Child, the sigil and the footprints are built in Python with the model kit copied from the Majora's Mask: WTF
project (`tools/ben/modelkit.py`): edit `tools/ben/gen_ben_models.py` and run it, never the generated
`z_en_ben_models.inc.c`. `tools/ben/preview_model.py` renders one to a PNG without the game, for example
`python3 tools/ben/preview_model.py src/overlays/actors/ovl_En_Ben/z_en_ben_models.inc.c child.png sMoonChildBodyDL "sMoonChildHeadDL@0,3200,0,0,0,0,1"`.
The visitors are `tools/ben/gen_ben_visitors.py`, written to
`src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c` the same way. They're an actor of their own
(`ACTOR_EN_BEN_VISITOR`), so their models are in memory only while one of them is here. The same script writes the
SCPs' to `ovl_En_Ben_Scp173/z_en_ben_scp173_models.inc.c` (and 096's and 106's), each its own actor
(`ACTOR_EN_BEN_SCP173` and so on, so only the one that's here is in memory);
the two share their helpers through `ovl_En_Ben_Visitor/z_en_ben_visitor_kit.inc`. The SCP models are built from the
articles' descriptions only; no image from the wiki was used.
