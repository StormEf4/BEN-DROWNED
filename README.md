# BEN DROWNED — a Majora's Mask ROM hack

A faithful, 2–3 hour adaptation of Jadusable's (Alex Hall's) 2010 creepypasta / ARG
**"Haunted Majora's Mask Cartridge" (BEN Drowned)**, built as a ROM hack of
*The Legend of Zelda: Majora's Mask* (N64, US) on top of the
[zeldaret/mm](https://github.com/zeldaret/mm) decompilation.

> You bought a cartridge at a yard sale. There is already a save file on it.
> Its name is **BEN**.

## Pillars

1. **You are holding the cartridge.** No narrator and no chapter cards. The ROM boots
   like ordinary Majora's Mask with a `BEN` save already on it, then corrupts as you play.
2. **Faithful first.** Every major beat of the original arc (day four.wmv → BEN.wmv →
   DROWNED.wmv → jadusable.wmv / free.wmv → TheTruth.rtf) happens on screen.
3. **All of Termina.** New horror set pieces in every region extend the canon
   without contradicting it.
4. **Mostly exploration.** Atmosphere, stalking, chase sequences and environmental
   storytelling. Combat is light.
5. **Moon Children, foreshadowed.** A hidden thread across the game leads to a secret
   epilogue that points at the cult, ascension and the "second entity."
6. **Runs on real hardware.** It ships as a `.bps` patch for N64 carts, flashcarts and
   accurate emulators.

## Project docs

| Doc | What it's for |
| --- | --- |
| [`docs/ROADMAP.md`](docs/ROADMAP.md) | Phases, milestones, exit criteria and the order we build in |
| [`docs/STORY_BIBLE.md`](docs/STORY_BIBLE.md) | Canon beat sheet, the chapter outline across Termina, BEN's rules |
| [`docs/TECH_DESIGN.md`](docs/TECH_DESIGN.md) | The "horror kit" systems and the decomp actors and scenes they hook into |
| [`docs/DECISIONS.md`](docs/DECISIONS.md) | Decisions we've made together, plus the open questions |

## Status

**Phase 0: foundation.** Design docs are drafted and the decomp toolchain comes next.
See the roadmap.

## Legal / credits

- **No Nintendo assets are in this repository, and none will ever be.** Building it requires
  your own legally obtained `mm-n64-us` ROM. We release only a patch.
- BEN Drowned and its characters were created by **Alex Hall (Jadusable)**. This is a
  free, non-commercial fan tribute. Full credit goes to the original work, and we'll
  respect any request from its creator.
- The decompilation is by the [ZeldaRET](https://github.com/zeldaret) team.
