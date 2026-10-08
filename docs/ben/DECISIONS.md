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

## Open questions

| # | Question | Blocks |
|---|----------|--------|
| Q4 | **Canon pass (yours).** Can you watch the playlist and resolve the ⚠️ items in `STORY_BIBLE.md` §1? (YouTube isn't reachable from the build sandbox.) | Phase 2 |
| Q5 | Moon Children cipher wording, and how cryptic the secret should be (findable blind, or meant for community solving)? | Phase 4 |
| Q6 | Content limits: keep it psychological (current plan), or allow body horror in the dream sequence? | Phase 3d |
| Q7 | Keep Romani Ranch "They" in Session 7, or cut it first if we run long? | Phase 3e |
| Q8 | 3D and audio fall to Claude by default. Do you want to recruit a community musician or 3D modeler for the reversed tracks and the dream geometry, or keep it in-house? | Phase 1d / 3d |
