# The full route: Sessions 0 to 7

Every session is one 3-day cycle (STORY_BIBLE.md §2, §3). Between the scripted moments you're playing real Majora's
Mask: walking, swimming, fighting what's there. The statue stalks you through each session's region the way it
stalks Clock Town on the 4th day. The story is saved with the file (`SaveInfo.unk_DF4`), and each time a session ends
it's written to the cartridge right away, so turning the game off doesn't undo it.

If you can't reach a session's ending (it needs an item you don't have, say), **the ending comes to you**: after
enough sightings, the next one is the ending.

**Finding the way.** Tatl knows where the story wants you, and she doesn't like it. When she starts flashing, press
C-up: every step of the story has a line from her that points the way without spelling it out (the Song of Double
Time's notes, the Laundry Pool, the swamp, Darmani's grave, the open sea, the tower at midnight). From Session 3 on,
every one of them ends in the Salesman's laugh. In Session 1 a townsperson whispers the 4th-day rumor too.

| Session | Where | What happens | How it ends |
|---|---|---|---|
| 0 **BOOT** | File select, Stone Tower | On a blank cartridge, slot 1 already holds a file called **BEN**. Open it: Stone Tower an hour before the moon falls, a garbled "St o n e" box, the statue, then the game gives out. | Back to the title screen. Erase BEN or use slot 2. |
| 1 **Something's wrong** | Clock Town | Once the first cycle's done, the town is almost normal. Now and then somebody stops dead and stares at you a beat too long. The first night you walk up to somebody, they turn and whisper: *"They say there's a fourth day... They say a boy found it. He never came home."* | Playing the Song of Double Time on the final night, and once more. |
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

## Tuning

All in `src/overlays/actors/ovl_En_Ben/z_en_ben.c`: `EN_BEN_END_SIGHTINGS` (when the ending comes to you),
`EN_BEN_DREAM_SIGHTINGS`, `EN_BEN_SEA_DEPTH` (if the open sea never takes you, lower it), `EN_BEN_WHITEOUT_*`, and the
Salesman scene's `EN_BEN_SALESMAN_*`. Where each session puts you is `Ben_WarpToSession` in `src/ben/ben_core.c`.
