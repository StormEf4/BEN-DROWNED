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

## Build steps (WSL2)

```bash
make init -j$(nproc)          # first time only: must end with "mm-n64-us.z64: OK" before our changes count
# (if init fails the md5 check now, that is expected: our changes are already in the tree.
#  Run `git stash`, `make init`, `git stash pop` if you want to see the vanilla OK first.)
python3 tools/ben/check_text_ids.py   # must print "OK: no text ID collisions"
make -j$(nproc) NON_MATCHING=1 COMPARE=0
```

Output: `build/n64-us/mm-n64-us-compressed.z64`. Open it in **ares**.

## Test script (about 10 minutes)

1. **Boot.** Title screen, then file select. ☐ Boots with no crash
2. Make a new file with any name **except** BEN (e.g. `LINK`). ☐
3. Play the intro until you're in **South Clock Town** as Deku Link (the first time you leave the
   Clock Tower). ☐ The music becomes a slow, low Song of Healing
4. **Without moving the camera**, look around: ☐ the statue isn't in view at first (it starts
   400 units behind you)
5. Turn the camera to find it. ☐ It never moves while on screen
6. Turn away for a few seconds, then look back. ☐ It's closer
7. Let it reach you. ☐ The forced textbox appears with your name shown as **BEN** in red
8. Close the text. ☐ The statue reappears behind you
9. Talk to any NPC whose dialogue uses your name. ☐ It says BEN
10. Leave South Clock Town and come back. ☐ No crash on scene change. ☐ Other areas are untouched

## Report back

For each ☐, tell me ✅ or ❌. For any ❌, include:
- what happened (a screenshot or a short clip from ares helps a lot)
- the **ares** version
- the last ~30 lines of the build output if the **build** failed

## Known limits of the spike (by design)

- Statues behind walls still count as "seen" (only the view frustum is checked; occlusion comes in Phase 1b).
- The music is the Song of Healing **slowed and pitched down**, not truly **reversed**. A real reversed
  sequence needs the extracted song data from your build, so it's Phase 1d (the K7 audio pipeline).
- The statue appears every time you enter South Clock Town. Story flags (K1) come in Phase 1a.
