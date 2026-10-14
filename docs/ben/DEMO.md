# Demo teaser: "day four" in South Clock Town (about 10 minutes)

A short, playable cut of the Phase 1 slice, built on the Phase 0.3 statue. BEN is a cryptid here, not a follower
(STORY_BIBLE.md §4): rare, distant, gone when you look back, and silent except for one line. It's switched on by `BEN_DEMO_TEASER`
in `include/ben/ben.h`.

## What happens

1. Make a **new file**. You step out of the Clock Tower into South Clock Town as human Link (`BEN_SKIP_INTRO`).
2. **Nobody is there, and there's no music.** That's true in all four districts and the Laundry Pool.
3. Sooner or later you turn around and the Elegy statue is standing far off: at the end of a street, around a
   corner, behind you. It never moves while you watch it. Look away and it's gone.
4. That first sighting wakes the music: a slow, low Song of Healing. Each sighting after it is a little closer, even
   across districts. Every third time it vanishes, the Happy Mask Salesman laughs somewhere.
5. It never speaks, except once. Play the **Song of Time** and agree to go back. At dawn it is **standing right in
   front of you**: **"YOU SHOULDN'T HAVE DONE THAT."** After that the music is slower and lower.

## Test script

| # | Check |
|---|---|
| 1 | No townspeople and no music in any Clock Town district or the Laundry Pool (Tatl is still with you) |
| 2 | Within about 15 s the statue is somewhere you weren't looking, far away, facing you |
| 3 | It never moves on screen. Look away after seeing it and it's gone. Walk up to it and it's gone |
| 4 | The first sighting starts the slow Song of Healing; later sightings get closer |
| 5 | Every third vanish, the Salesman's laugh plays (check it's actually his laugh; tell Claude if not) |
| 6 | Song of Time → dawn as **human** Link in South Clock Town → the statue in front of you and the red line |
| 7 | After that the music is noticeably slower and lower |

## Sharing it

Double-click **BEN DROWNED - Make Patch** on your desktop. It saves `BEN-DROWNED-latest.bps` in
`C:\Users\<you>\BEN-DROWNED\`. Players apply it to their own Majora's Mask (USA) ROM with Flips or an online
BPS patcher, then play it in ares.
