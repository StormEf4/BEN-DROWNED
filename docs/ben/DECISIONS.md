# Decisions & Open Questions

A running log of choices we make together. Newest at the bottom of each section.

## Decided

| # | Date | Decision | Why |
|---|------|----------|-----|
| D1 | 2026-10-08 | **Platform: the zeldaret/mm N64 decomp**, released as a `.bps` patch | Runs on real carts (fits "haunted cartridge"), full control over scenes, text and music |
| D2 | 2026-10-08 | **Framing: "you hold the cartridge"**: diegetic, with a pre-existing BEN save and no narrator | The most faithful to the original's feel |
| D3 | 2026-10-08 | **Canon scope: the Haunted Cartridge arc plus Moon Children hints** (secret epilogue) | Fits 2–3 hours and rewards the ARG fans |
| D4 | 2026-10-08 | **Gameplay: mostly exploration**, light combat | Effort goes into atmosphere, the stalker and set pieces |
| D5 | 2026-10-08 | Each 3-day cycle is one "session" matching one of Jadusable's videos | Uses MM's own reset mechanic as "the counter resets" |
| D6 | 2026-10-08 | Session 1 starts **after the first cycle** (human Link, ocarina) | The vanilla intro is 45+ minutes and would blow the time budget |
| D7 | 2026-10-08 | **This repo is a fork of zeldaret/mm** (history merged, pinned to `56fa21dd`, `upstream` remote for later merges). Our docs live in `docs/ben/`, and the decomp README moved to `docs/DECOMP_README.md` | The standard decomp-hack layout, with edits going directly into the source |
| D8 | 2026-10-08 | **Builds run on your Windows PC via WSL2** (Docker as an alternative). Claude writes the code; you build, play and report | The ROM can't go in the repo or the cloud sandbox |
| D9 | 2026-10-08 | **Roles:** you own **creative direction** (story calls, canon verification, beat-sheet approval, playtesting). Claude owns code, 3D, audio pipeline and docs | Your choice |
| D10 | 2026-10-08 | **Testing: emulator only.** ares is the accuracy reference. We recruit 1–2 testers with real N64 + flashcart before beta | No hardware on hand |
| D11 | 2026-10-08 | **The Moon Children secret is findable solo with extra effort**: a Bombers' Notebook entry and one small optional task per session, not a community cipher | Rewarding for a single player, and keeps the ARG flavor |
| D12 | 2026-10-08 | **Horror arc: start slow and psychological, descend into full madness.** Five tiers (STORY_BIBLE §4a). Body horror is allowed from Session 6 (implied rather than graphic) | Your call; matches how the original escalated |
| D13 | 2026-10-08 | **Start the Phase 0.3 spike now**, ahead of your first build | Ready to test the moment the toolchain works |
| D14 | 2026-10-08 | **`./ben` is the only build interface.** One-command setup (`windows/BEN-Setup.ps1` on Windows), fingerprinted steps so updates redo only what's stale, and the decomp's make flags hidden. Its commands stay the same for every future build | Your request: a friendlier setup that keeps working as the project grows |
| D15 | 2026-10-10 | **Skip the intro during development** (`BEN_SKIP_INTRO` in `include/ben/ben.h`). New files start at the South Clock Town door after the dawn card, as human Link with the D6 kit. It gets turned off when the BOOT and Session 1 content replaces the vanilla intro | Faster testing; your request |
| D16 | 2026-11-12 | **The skipped intro is the real start.** `BEN_SKIP_INTRO` stays on: a new file steps out into South Clock Town with the ocarina, and Tatl's opening lines (`BEN_TEXT_OPENING`) say what there is to do, hint at a fourth day, and teach that C-up always knows where to go. From Session 3 every scene the story doesn't use gets the Termina director (small, rare, uncounted scares) | You asked for a proper start, a clear way forward, and more horror across Termina without overdoing it |
| D17 | 2026-11-12 | **Two ways to play, chosen in the builder** (`BEN_EXPANDED`): **the Jadusable Experience** (BEN's story only) or **the Expanded Horror** (the default). Everything that isn't the Ben Drowned canon goes behind it: the Termina director's small scares, and the visitors from other creepypastas (a tall faceless man, a smiling man who dances, "Go to sleep."), never named in the game. The visitors are an actor of their own, `En_Ben_Visitor`, so their models load only while one is here. Hide and seek and the Song of Unhealing belong to BEN's own story and stay in both | Your request: new entities from other creepypastas, kept apart from the canon as an installer option |
| D18 | 2026-11-12 | **Three SCPs in the Expanded Horror**: SCP-173 (the sculpture: moves only when unseen, Link blinks), SCP-096 (the shy one: see its face and it comes for you through anything) and SCP-106 (the old man: rises from corrosion, walks through walls, light sends him down). An actor of their own, `En_Ben_Scp`, sent by the Termina director like the visitors, never named in game, never lethal. The SCP articles are CC BY-SA 3.0, so they're credited in `docs/ben/CREDITS.md`, and the models are original, built from the articles' text (the well-known SCP-173 photo isn't under that licence and wasn't used) | Your request: famous SCPs with their behaviours and models |

## Open questions

| # | Question | Blocks |
|---|----------|--------|
| Q4 | **Canon pass (yours).** Can you watch the playlist and resolve the ⚠️ items in `STORY_BIBLE.md` §1? (YouTube isn't reachable from the build sandbox.) | Phase 2 |
| Q5 | Moon Children phrase wording (the seven fragments) | Phase 4 |
| Q7 | Keep Romani Ranch "They" in Session 7, or cut it first if we run long? | Phase 3e |
| Q8 | 3D and audio fall to Claude by default. Do you want to recruit a community musician or 3D modeler for the reversed tracks and the dream geometry, or keep it in-house? | Phase 1d / 3d |
