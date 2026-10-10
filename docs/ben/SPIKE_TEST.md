# Phase 0.3 — Shiftability Spike: Test Checklist

The spike checks that a **modified** decomp build still boots and runs before we build any real
content on top of it. It adds a little of everything we'll need: new code in the `code` segment,
a new actor overlay, a new message, a change to a vanilla system, and music control.

## What the spike adds

| Piece | Where | What you should see |
|---|---|---|
| K2 name override | `src/ben/ben_core.c`, `src/code/z_message_nes.c` | **Every** NPC line that uses your name says **BEN**, whatever you typed on the file select |
| K4 stalker statue (`En_Ben`, new actor 0x2B2) | `src/overlays/actors/ovl_En_Ben/` | An Elegy of Emptiness statue in **South Clock Town** that only moves while it's off screen |
| New message `0x4D00` | `assets/text/message_data.h` | When the statue reaches you: *"You've met with a terrible fate, haven't you?"* then *"... **BEN**."* |
| Music swap (K7 runtime fallback) | `En_Ben` update | South Clock Town's music changes to a **slowed, lower** Song of Healing about one second after you arrive |
| Spawn hook | `src/code/z_play.c` (`Play_Init`) | Only South Clock Town is affected |

## Build steps

First time: follow [`BUILDING.md`](BUILDING.md) (on Windows that's one PowerShell command).
After that, every time:

```bash
cd ~/BEN-DROWNED
./ben update      # get the newest version and build it (or ./ben build to rebuild what you have)
./ben run         # open it in ares
```

The ROM is written to `dist/BEN-DROWNED-latest.z64`, and on Windows it's also copied to
`C:\Users\<you>\BEN-DROWNED\BEN-DROWNED-latest.z64`.

## Round 1 results (2026-10-10)

| # | Check | Result | Follow-up |
|---|---|---|---|
| 1 | Boots | ✅ | **The shifted build runs.** New code in the `code` segment, a new overlay, and new text all work |
| 2 | New file | ✅ | |
| 3 | Music changes | ❌ | The one-shot music swap lost the race against the scene's own music start. Fixed: the Song of Healing is now made the scene's music, with a watchdog |
| 4 | Statue starts out of view | ❌ | It appeared at Link's spawn point: at that moment the camera doesn't exist yet and Link is in the doorway, so every spot failed. Fixed: it waits hidden and appears once an unseen spot exists |
| 5–6 | Weeping-angel movement | ✅ | |
| 7 | Forced text with BEN | ✅ | This also confirms the name override (K2), since NPC lines go through the same text code |
| 8 | Reappears behind you | ❌ | It sometimes appeared in front, facing away. "Behind" was measured from Link's back, not from the camera. Fixed: it now picks a spot the camera can't see and turns to face Link |
| 9 | NPC says BEN | — | Can't test as Deku Link. Covered by #7 |
| 10 | Other areas untouched | ✅ | |

## Round 2 test script (about 5 minutes)

Rebuild with `./ben update` and **create a new file** (files made before this update still play the intro).
The intro is skipped for testing (`BEN_SKIP_INTRO`): after "Dawn of the First Day" you step out of the Clock Tower
into South Clock Town as **human Link**, with Tatl, the Ocarina, the Song of Time, the Song of Healing and the
Deku Mask. Play the Song of Time to restart the cycle whenever you want a fresh test.

0. ☐ New file starts outside the Clock Tower door as human Link (no intro)

1. ☐ Within a second or two the music is a **slow, low Song of Healing**
2. ☐ The statue is **not** at the door with you. Look around: it should be somewhere you weren't looking, facing you
3. ☐ It still never moves while any part of it is on screen (even just its head at the edge)
4. ☐ Let it reach you. After the text closes it **vanishes**, and about 2 seconds later is somewhere off-camera, **facing you**
5. ☐ Walk into West/North/East Clock Town: **normal Clock Town music at normal speed**, no statue
6. ☐ Come back to South Clock Town: the slow Song of Healing returns and the statue reappears out of view
7. (Optional, now that you're human) ☐ Any NPC who says your name says **BEN**

## Report back

For each ☐, tell me ✅ or ❌. For any ❌, include:
- what happened (a screenshot or a short clip from ares helps a lot)
- the **ares** version
- if the **build** failed: the log file `./ben` printed (also at `.ben/logs/latest.log`), and the output of `./ben doctor`

## Known limits of the spike (by design)

- Statues behind walls still count as "seen" (only the view frustum is checked; occlusion comes in Phase 1b).
- The music is the Song of Healing **slowed and pitched down**, not truly **reversed**. A real reversed
  sequence needs the extracted song data from your build, so it's Phase 1d (the K7 audio pipeline).
- The statue appears every time you enter South Clock Town. Story flags (K1) come in Phase 1a.
- On the third day the game's Final Hours music and the spike's music watchdog will fight. The spike is only meant
  for Day 1; real per-scene music control comes with K7.
