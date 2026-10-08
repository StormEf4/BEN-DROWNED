# Technical Design — BEN DROWNED

**Base:** [zeldaret/mm](https://github.com/zeldaret/mm), N64 US. The pinned upstream commit
at the time of writing is `56fa21dd` (2026-09-26).
**Output:** a `.bps` patch against the retail US ROM
(`mm-n64-us`, compressed, md5 `2a0a8acb61538235bc1094d297fb6556`).

The guiding idea is to **build a small "horror kit" once and make the chapters mostly
data.** Every session in `STORY_BIBLE.md` should be producible by placing actors and
writing event tables, not by writing new C each time.

---

## Build & release pipeline

```
your own ROM ──► baseroms/n64-us/baserom.z64
                 make init            (extract assets, one time)
                 make NON_MATCHING=1 COMPARE=0   (our modified build)
                 ──► build/n64-us/mm-n64-us-compressed.z64
                 tools/ben/make_patch.sh  (flips --create, vanilla vs ours)
                 ──► dist/ben-drowned-vX.Y.bps
```

- **The ROM and extracted assets are never committed.** `.gitignore` enforces this.
- Builds go through the decomp's `Dockerfile` (Ubuntu 24.04), so every machine is
  identical.
- Test targets: **ares** is the accuracy reference and must pass. Project64 and RMG are for quick
  iteration. Real N64 + flashcart testing comes through recruited testers before beta (D10).
- **Dev boot:** the decomp ships a map-select gamestate (`src/overlays/gamestates/ovl_select`).
  We extend it with a **session select** that jumps to any session with the right
  story flags already set.

---

## The horror kit (K1–K10)

| ID | System | Hooks into (decomp) | Notes |
|----|--------|---------------------|-------|
| **K1** | **Story state** | `include/z64save.h` (`SaveInfo`, `weekEventReg[100]`), `src/code/z_sram_NES.c` | A `BenState` block holding the current session, sigils found, reset-detection flag, boot counter and "freed" flag. It goes in unused save space, or we deliberately extend the save (save size is `0x4000` per buffer). It must survive the cycle reset, so it's stored like the "permanent" flags. |
| **K2** | **Name override** | `src/code/z_message.c` (player-name control code around line 2330), `ovl_file_choose` | Every NPC says "BEN" no matter what you typed. Exceptions are tied to the session: some lines use your real name, which is scarier. |
| **K3** | **Pre-seeded BEN save** | `z_sram_NES.c`, `src/overlays/gamestates/ovl_file_choose` | On first boot with blank flash, write a canned BEN file at Stone Tower on the Final Day. Erasing it shows custom erase text. Selecting it runs the BOOT vignette. Later sessions rename files to BEN / DROWNED. |
| **K4** | **Stalker statue** (`ovl_En_Ben`, new) | Model and object borrowed from `ovl_En_Torch2` (Elegy shell) | Modes: `WATCH` (stands at a distance), `CREEP` (moves only when off-camera or occluded), `BEHIND` (teleports behind the player), `WATER` (appears at water edges), `SCRIPTED` (driven by K5). Takes parameters per placement. |
| **K5** | **Event director** (`ovl_Ben_Director`, new) | Actor spawned per scene setup | **Data-driven** trigger→action tables. Triggers: volume, timer, flag, camera-facing, song played, time of day. Actions: play/stop sequence, silence, fade, spawn/move `En_Ben`, force mask, camera override, show message, custom death, warp, set flag, glitch FX. **Most chapter work happens here.** |
| **K6** | **Glitch FX** | `src/code/PreRender.c`, framebuffer and display-list hooks, `z_message.c`, the fault/crash screen | Screen tear and jitter, palette corruption, corrupted text (random glyphs from `assets/text/charmap.txt`), a fake crash screen, "St o n e"-style spaced region names. **Each effect gets a performance check on real hardware early.** |
| **K7** | **Audio** | `assets/audio` + `docs/audio/*.md` (soundfont/samplebank XML) | New sequences: reversed Song of Healing, reversed Stone Tower, slowed and reversed Salesman theme. Pipeline: vanilla seq → MIDI → reverse/stretch → seq → insert. A reversed Salesman laugh as SFX. **Fallback:** runtime tempo and pitch changes to existing sequences. |
| **K8** | **Custom deaths** | `src/overlays/actors/ovl_player_actor/z_player.c`, the Game Over flow | Burn death (C10) and drowning death (C9, C16), each with custom Game Over text. Deaths can be scripted (triggered by K5) as well as "real" ones. |
| **K9** | **Scene variants** | Scene XML in `assets/xml/scenes/*`, alternate scene setups, Fast64 (Blender) for any new geometry | "Empty" versions of scenes are alternate setups with the actor lists stripped and K5 added. **We reuse vanilla geometry wherever we can**, and new rooms only for the dream sequence and the secret epilogue. |
| **K10** | **Meta / cartridge tricks** | K1, `ovl_title`, `ovl_file_choose`, `ovl_daytelop` | **Reset detection:** set a flag when a session starts and clear it at a clean save point. If it's set at boot, you reset mid-session and BEN reacts. A boot counter. The post-"free" boot changes the title screen and file select. A custom "Dawn of the ___ Day" card (`ovl_daytelop`) for "day four". |

### Vanilla actors and scenes we lean on

| Purpose | Actor / scene |
|---|---|
| Elegy statue model | `ovl_En_Torch2` |
| Happy Mask Salesman | `ovl_En_Osn` |
| Skull Kid levitating | `ovl_Dm_Stk` |
| Moon Children | `ovl_En_Js` |
| Majora | `ovl_Boss_07` |
| Darmani's ghost | `ovl_En_Gg` |
| Mikau | `ovl_En_Zog` |
| Pamela's father | `ovl_En_Hgo` |
| Moon-watching man (the 4th-day rumor) | `ovl_En_Sth` |
| Clock Town: South / North / East / West / Laundry Pool | `Z2_CLOCKTOWER` / `Z2_BACKTOWN` / `Z2_TOWN` / `Z2_ICHIBA` / `Z2_ALLEY` |
| Clock Tower interior / rooftop | `Z2_INSIDETOWER` / `Z2_OKUJOU` |
| Majora's lair | `Z2_LAST_BS` |
| Termina Field | `Z2_00KEIKOKU` |
| Southern Swamp / Deku Palace / Woodfall | `Z2_20SICHITAI` / `Z2_22DEKUCITY` / `Z2_21MITURINMAE` |
| Mountain Village / Snowhead / Goron Graveyard | `Z2_10YUKIYAMANOMURA` / `Z2_12HAKUGINMAE` / `Z2_GORON_HAKA` |
| Great Bay Coast / Zora Cape / Zora Hall | `Z2_30GYOSON` / `Z2_31MISAKI` / `Z2_33ZORACITY` |
| Ikana Graveyard / Canyon / Stone Tower / Inverted | `Z2_BOTI` / `Z2_IKANA` / `Z2_F40` / `Z2_F41` |
| Romani Ranch | `Z2_F01` |
| Moon field | `Z2_SOUGEN` |

*(The mapping of scene IDs to places should be double-checked in Phase 0 against the scene
table. A few, like `Z2_ALLEY` for the Laundry Pool and `Z2_BOTI` for the graveyard, are from memory.)*

---

## Code layout (proposed)

```
src/overlays/actors/ovl_En_Ben/        K4 stalker statue
src/overlays/actors/ovl_Ben_Director/  K5 event director
src/ben/                               K1 state, K2 name, K6 fx, K10 meta (linked into code)
src/ben/events/session_*.c             K5 tables, one file per session (data, not logic)
assets/ben/                            our new text, sequences and textures (no vanilla data)
tools/ben/                             make_patch.sh, seq reverse script, helpers
```

Every vanilla edit is wrapped as `#ifdef BEN_DROWNED … #endif`, or at least tagged
`// BEN:`, so upstream decomp merges stay easy to review.

---

## Technical risks

| Risk | Impact | Mitigation |
|---|---|---|
| Parts of the decomp aren't "shiftable" yet (upstream warns about this) | Adding code or data could crash the ROM | **A Phase 0 spike**: add a new actor, new text and new audio, build with `NON_MATCHING=1 COMPARE=0`, and boot on ares and on hardware before writing any real content. |
| Upstream decomp churn | Merge pain | Pin a commit and merge upstream only between phases. |
| Inserting audio | Signature sounds get delayed | Do the K7 spike in Phase 1. The fallback is runtime tempo and pitch. |
| Framebuffer FX are slow on real N64 | Frame drops, broken effects | Test every K6 effect on hardware as it's written, and keep cheap versions ready. |
| Save layout changes | Broken saves between dev builds | Version `BenState`, and wipe on version mismatch in dev builds. |
