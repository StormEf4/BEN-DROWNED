# Roadmap — BEN DROWNED v1.0

**Goal:** a 2–3 hour, mostly-exploration horror adaptation of the Haunted Cartridge
arc, covering every region of Termina, with a secret Moon Children epilogue. It ships as a
`.bps` patch that runs on real N64 hardware.

## How we stay efficient

1. **Retire the scary tech risk first.** Before any content, prove that a modified,
   shifted decomp build with a new actor, new text and new music boots on emulator and
   hardware (Phase 0).
2. **Build the vertical slice first.** "day four" is the most iconic chapter and uses
   almost every system. Once it's fun and scary, the rest is repetition.
3. **Chapters are data.** The event director (K5) and stalker (K4) mean most chapter work
   is placing actors and writing trigger tables, not writing new C.
4. **Reuse vanilla Termina.** New geometry only for the dream sequence and the secret
   epilogue.
5. **Lock the story before building each chapter.** Canon verification and beat sheets
   run *ahead* of production, in parallel.
6. **Always playable.** Every phase ends with a ROM someone can play start to finish
   through the content that exists. A session-select debug boot makes any chapter
   reachable in seconds.
7. **Hard cap on scope.** The cut list lives in `STORY_BIBLE.md` §3. When a phase runs
   over, we cut rather than slip.

Durations assume part-time hobby pace, with Claude writing code, docs, 3D and the audio
pipeline, and you directing, building, testing and verifying canon. They're for sequencing, not promises.

---

## Phase 0 — Foundation (~1–2 weeks)

| # | Task | Owner | Done when |
|---|------|-------|-----------|
| 0.1 | ✅ Repo layout: zeldaret/mm decomp merged into this repo, pinned to `56fa21dd` (DECISIONS D7) | Claude | `make` targets exist in this repo |
| 0.2 | 🟡 One-command build env: `./ben` (setup, build, update, run, patch, doctor) plus the Windows installer `windows/BEN-Setup.ps1`. See [`BUILDING.md`](BUILDING.md). You run it with your own US ROM | Claude wrote it, you run it | `./ben setup` finishes on your PC and prints the ROM path |
| 0.3 | 🟡 **Shiftability spike** written: `En_Ben` statue actor, BEN name override, message `0x4D00`, slowed Song of Healing. Waiting on your build and run of [`SPIKE_TEST.md`](SPIKE_TEST.md) | Claude wrote it, you run it | Every check in SPIKE_TEST.md passes on ares |
| 0.4 | 🟡 `tools/ben/make_patch.sh` written (flips, round-trip verified); `tools/ben/check_text_ids.py` guards message IDs | Claude wrote it, you run it | The patch applies cleanly to a retail ROM |
| 0.5 | Session-select debug boot (extend `ovl_select`) | Claude | You can jump to any scene with flags set |
| 0.6 | GitHub milestones (one per phase) and issues for every task here | Claude | Board exists |
| 0.7 | **Canon verification pass**: watch the playlist and resolve every ⚠️ in `STORY_BIBLE.md` §1 | **You** | No ⚠️ left |

**Exit:** a patched ROM in which a Clock Town NPC calls you "BEN" and a test sequence plays.

## Phase 1 — Horror kit + vertical slice "day four" (~3–5 weeks)

Build K1–K5, K7 (reversed Song of Healing), K8 (burn), K9 (empty Clock Town) and the
start of K10, then use them to make **Sessions 0–2** playable:

> Boot → BEN file on the file select → "St o n e" vignette → erase BEN → new file
> → Clock Town where everyone calls you BEN → the 4th-day rumor → Majora's lair with
> Skull Kid → empty Clock Town → the statue stalks you → the Laundry Pool → you burn →
> Song of Time → **"YOU SHOULDN'T HAVE DONE THAT."**

- 1a: K1 state, K2 name override, K3 pre-seeded save (file-select beats)
- 1b: K4 stalker statue with its `CREEP` and `BEHIND` modes, tuned in an empty test room
- 1c: K5 director and the `session_02` table
- 1d: K7 audio pipeline, with the reversed Song of Healing as the first track out of it
- 1e: K8 burn and drown deaths and the custom Game Over text
- 1f: **Playtest with 2–3 people who haven't seen the docs.** Tune it.

**Exit (Milestone "Slice"):** Sessions 0–2, about 45 minutes, playable on ares and Project64/RMG,
and at least one tester says it scared them. *Show this off; it's the project's proof of concept.*

## Phase 2 — Story lock (runs parallel to Phase 1, ~1–2 weeks)

- A final beat sheet per session (shot list, trigger list, every line of text)
- The final Moon Children cipher wording and sigil locations
- The script for all altered dialogue (NPCs, Tatl, the Salesman, Moon Children)

**Exit:** every chapter of Phase 3 has a locked beat sheet before production starts on it.

## Phase 3 — Content production (~10–14 weeks)

One chapter at a time, **in story order**. Each one follows the same loop:
*beat sheet → greybox (scene setups + director table) → audio → FX → playtest → polish.*

| Step | Session | Main new work | Est. |
|---|---|---|---|
| 3a | 3 — **BEN** (Swamp) | Forced Deku form, camera overrides, the Butler-race chase, the BEN/DROWNED file rename, reversed and slowed Salesman theme | 2–3 wk |
| 3b | 4 — **Cold** (Snowhead) | Blizzard visibility mode for the stalker, the Darmani event, ice fall | 2 wk |
| 3c | 5 — **DROWNED** (Great Bay) | `WATER` stalker mode, the open-sea drowning, OoT flicker, the Skull Kid face flash, the Moon teleport | 2–3 wk |
| 3d | 5½ + 6 — **Moon (false)** + **St o n e** (Ikana) | Elegy shell climbing, the reversed Stone Tower theme, the **dream sequence** (new geometry), the second-statue hint | 3 wk |
| 3e | 7 — **jadusable / free** | Romani "They" night, the finale fight with BEN in control, the free.wmv black screen, the post-game boot state | 2–3 wk |

**Exit (Milestone "Alpha"):** the whole game is playable start to finish in 2–3 hours, with rough edges allowed.

## Phase 4 — Secret thread + meta (~2 weeks)

- The Moon Children sub-mission: the Bombers' Notebook entry plus seven small optional tasks, one per session (STORY_BIBLE §5)
- The **CHILDREN** epilogue (secret boot): second entity, ascension
- K10 reactions: reset mid-session, power-off detection, erased-file reactions, boot count

**Exit:** the secret ending is reachable, and every K10 trick works on hardware.

## Phase 5 — Polish & beta (~3–4 weeks)

- A pacing pass to hit 2–3 hours (time three or more fresh players)
- Audio mix, silence pass, madness-curve audit (each session sits in its tier, STORY_BIBLE §4a)
- Hardware matrix: ares (reference), Project64, RMG, plus **1–2 recruited testers with real N64 + flashcart** (SummerCart64 / EverDrive)
- Soak tests: deaths everywhere, save/reset at every session boundary, 100% sigils
- A release README with a content note and credits to Jadusable/Alex Hall and ZeldaRET

**Exit (Milestone "Beta → RC"):** no known crashes, and the pacing is on target.

## Phase 6 — Release v1.0

- `ben-drowned-v1.0.bps`, checksums, install guide
- Optional: a trailer recorded in the lo-fi style of the original `.wmv` videos

---

## Critical path (at a glance)

```
P0 spike ─► P1 kit+slice ─► P3a ─► P3b ─► P3c ─► P3d ─► P3e ─► P4 ─► P5 ─► v1.0
   └─ P0.7 canon pass ─► P2 story lock (stays one chapter ahead of P3) ─┘
```

**The single biggest efficiency lever:** if Phase 1 builds the director (K5) properly,
Phase 3 chapters become mostly data entry, and the estimate falls toward the low end.

## Project risks (non-technical)

| Risk | Mitigation |
|---|---|
| Scope creep ("what if every temple…") | The 2–3 hour cap and the cut list; any new idea goes to a `v1.1` label |
| Canon errors annoy fans | The Phase 0.7 verification pass; ⚠️ items are blocked until resolved |
| IP/takedown | No Nintendo assets ever, patch only, non-commercial, full credit to Alex Hall |
| Burnout | The slice milestone gives a real, shareable win early |
