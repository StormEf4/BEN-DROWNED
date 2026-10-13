# Demo teaser: "day four" in South Clock Town (about 10 minutes)

A short, playable cut of the Phase 1 slice, built on the Phase 0.3 statue. It's switched on by `BEN_DEMO_TEASER`
in `include/ben/ben.h`.

## What happens

1. Make a **new file**. You step out of the Clock Tower into South Clock Town as human Link (`BEN_SKIP_INTRO`).
2. **Nobody is there.** The music is a slow, low Song of Healing.
3. The Elegy statue stalks you. It only moves while you aren't looking at it. When it reaches you:
   *"You've met with a terrible fate, haven't you? ... BEN."*
4. Play the **Song of Time** and agree to go back.
5. At dawn the statue is **standing right in front of you**: **"YOU SHOULDN'T HAVE DONE THAT."**
6. From then on it is faster and the music is slower and lower. That's the end of the demo.

## Test script

| # | Check |
|---|---|
| 1 | South Clock Town has no townspeople (Tatl is still with you) |
| 2 | Slow Song of Healing plays; the statue appears out of view and only moves while off screen |
| 3 | It speaks the "terrible fate ... BEN" line once, then vanishes and comes back out of view |
| 4 | Song of Time → dawn → the statue is in front of you and the red line appears |
| 5 | After that it moves faster, and the music is noticeably slower and lower |
| 6 | West/North/East Clock Town still have normal music and people |

## Sharing it

Double-click **BEN DROWNED - Make Patch** on your desktop. It saves `BEN-DROWNED-latest.bps` in
`C:\Users\<you>\BEN-DROWNED\`. Players apply it to their own Majora's Mask (USA) ROM with Flips or an online
BPS patcher, then play it in ares.
