# BEN DROWNED: project handoff

Written 2026-11-23 so the project can continue from a different Claude account. Everything a new Claude needs is
in this repo plus the few files listed under [Files outside the repo](#files-outside-the-repo). Read this first,
then `docs/ben/PLAYTHROUGH.md` (the route as built) and `docs/ben/DECISIONS.md`.

## What the project is

A faithful ROM hack of *The Legend of Zelda: Majora's Mask* (N64, USA) that lets the player live Jadusable's
(Alex Hall's) 2010 creepypasta/ARG **BEN Drowned** as if they were Jadusable holding the haunted cartridge. Built on
the [zeldaret/mm](https://github.com/zeldaret/mm) decomp (forked, pinned to upstream `56fa21dd`), shipped as a `.bps`
patch. Owner and creative director: **Elijah** (GitHub `StormEf4`). Claude writes the code, models, audio pipeline and
docs; Elijah builds, plays and gives the verdict.

## Where things stand (2026-11-23)

- **All work is merged.** PRs #1 to #5 are merged into the default branch; there are no open PRs or issues, and the
  old work branch `claude/project-thread-288g0p` has nothing unmerged. Last merge: PR #5 on 2026-11-17.
- The full Session 0 to 7 route is built (see `docs/ben/PLAYTHROUGH.md`), plus two ways to play, hide and seek,
  3 of 7 Song of Unhealing sigils, creepypasta visitors and three SCPs (Expanded Horror only).
- **Waiting on Elijah's playtest** of PR #5's content: the full route, the rebuilt SCP and visitor models, and the
  bigger, brighter builder checkboxes. Nothing past PR #5 has been played yet.

### Recent history (newest first)

| PR | Merged | What it did |
|---|---|---|
| #5 | 2026-11-17 | The whole story (Sessions 0 to 7) saved with the file; hints that come back; Termina a little wrong; hide and seek; start of the Song of Unhealing; Jadusable Experience vs Expanded Horror toggle; creepypasta visitors and SCP-173/096/106 (one actor each); in-game Salesman laugh; creepier day four; builder builds the branch it was made from; big bright builder checkboxes |
| #4 | 2026-10-14 | Builder horror reskin (BEN icon, torn header, real Salesman laugh on Build); day four: Skull Kid in the lair, the Laundry Pool drowning, the burning death |
| #3 | 2026-10-14 | Day four in canon order; BEN-Builder.exe (drag in the ROM, press Build) |
| #2 | 2026-10-14 | BEN as a cryptid stalker instead of a pest; Song of Time Deku-lock fix |
| #1 | 2026-10-13 | Playable 10-minute teaser; one-click "Make Patch" shortcut |

## Open work

Not built yet (in rough priority order; confirm with Elijah after the playtest):

1. Whatever Elijah's playtest of PR #5 turns up. **Wait for that verdict before stacking more scares.**
2. Song of Unhealing sigils 4 to 7 (`z_en_ben_children.inc`, one per session).
3. The Moon Children / CHILDREN secret and its Bombers' Notebook entry (DECISIONS D11, Q5).
4. Deku Butler chase, Romani Ranch night (Q7), OoT flickers, statue glowing eyes.
5. The README's "Status" section is stale (still says Phase 0); refresh it.

Open questions for Elijah are in `docs/ben/DECISIONS.md` (Q4 canon pass of STORY_BIBLE §1 ⚠️ items, Q5 Moon Children
wording, Q7 Romani Ranch, Q8 outside musician/modeler).

## Story architecture

- Story state is `BenSave` in `SaveInfo.unk_DF4` (unused by vanilla): session, step, dayFour, fate, sightings,
  flags, `seek`, `unhealing`/`sigilAt`. `Sram_BenCommitStory` writes just that block to flash at every session change.
- **`Ben_GetDirector(sceneId)`** in `src/ben/ben_core.c` is the single table of which En_Ben (params STATUE, LAIR,
  BOOT, SALESMAN, FINALE, WARP, MOON, UNHEALING=9, ...) spawns where per session/step. Add new beats there first.
- Sessions 4 to 6 put a WARP director in South Clock Town (`Ben_WarpToSession`) because vanilla item gates (bow,
  Epona, hookshot) would block walking there.
- Every ending has a fallback: after N sightings the ending comes to the player.
- Progression hints: Tatl's C-up line via `Ben_GetHintTextId`, hooked into `QuestHint_GetTatlTextId`. **Every new
  story step needs a hint line** (Elijah: "easy to understand how to progress without being obvious"). Hints re-offer
  after 4 minutes (`BEN_TATL_HINT_AGAIN` in `z_en_elf.c`). The secret Song of Unhealing quest has no Tatl hints by design.
- New files open with Tatl's `BEN_TEXT_OPENING` (`BEN_FLAG_OPENED`); `BEN_SKIP_INTRO` stays on and is the real start (D16).
- Scenes the story doesn't use fall back to the WRONG director (Session 1) or TERMINA (Session 3+, Expanded only,
  `z_en_ben_termina.inc`), except boss rooms and the Moon.
- **Two stories** (`BEN_EXPANDED`, builder radio pair, D17): *the Jadusable Experience* (canon only) or *the Expanded
  Horror* (default). Anything non-canon goes behind `BEN_EXPANDED`. Hide and seek and the Song of Unhealing count as canon.
- Expanded-only actors: En_Ben_Visitor 0x2B3 (tall man), En_Ben_Smile 0x2B7, En_Ben_Sleep 0x2B8, En_Ben_Scp173/096/106
  0x2B4 to 0x2B6, all in `ovl_En_Ben_Visitor` (`z_en_ben_visitor.inc`, `z_en_ben_scp.inc`, `z_en_ben_visitor_kit.inc`),
  dispatched via `sVisitors`/`sVisitorActors` with cooldown `Ben_VisitorReady`. Never named in game, never lethal.
  A big model that only some scenes need gets its own actor so the En_Ben overlay stays small.
- SCP content is CC BY-SA 3.0: credit it in `docs/ben/CREDITS.md`; build models from article text, never wiki images.
- `BEN_FLASHES` (builder "Screen flashes") gates face flashes and fake glitches; keep new flashing effects behind it.
- Models are generated by `tools/ben/gen_ben_*.py` on `tools/ben/modelkit.py` (copied from `StormEf4/Majoras-Mask-WTF`;
  Elijah: only read and copy from WTF, never change it). `tools/ben/preview_model.py` renders previews.
- Design docs: `docs/ben/STORY_BIBLE.md` (canon beats, BEN's rules §4, horror tiers §4a), `TECH_DESIGN.md`, `ROADMAP.md`.

## Build setup

- **Default branch is `claude/gifted-carson-g2jeyn`, not `main`.** PRs target it. The repo is public so the Windows
  setup and `git clone` work without a login.
- **Elijah is on Windows** and builds with **BEN-Builder.exe**: drag in the ROM, pick options, press Build. Source is
  `windows/builder/BenBuilder.cs` (+ `Laugh.cs`, `BuildBranch.cs`, `ben.ico`, `ben.png`, `laugh.wav`). It drives WSL
  and `./ben`. `.github/workflows/builder.yml` compiles it on every change under `windows/builder/` and, **on merge to
  the default branch, publishes it to the `builder` release**:
  https://github.com/StormEf4/BEN-DROWNED/releases/download/builder/BEN-Builder.exe
- `BuildBranch.cs` names the branch the builder builds (CI fills it in; empty = default branch). So a builder compiled
  from a PR branch builds that PR, and the released one builds the default branch.
- Adding a builder option: a `#ifndef`-guarded `BEN_<NAME>` in `include/ben/ben.h` plus one line in `Options` in
  BenBuilder.cs; `./ben config OPT_<NAME> 0|1` carries it to the compiler.
- The PowerShell setup (`windows/BEN-Setup.ps1`) and its desktop shortcuts (Update, Make Patch) still work. On
  Linux/WSL/macOS: `./ben setup <rom>`, then `./ben update`, `./ben run`, `./ben patch`, `./ben doctor`.
- **Why Claude can't build in the cloud:** building needs Elijah's own Majora's Mask (USA) ROM, which is never in the
  repo or the cloud. Every real test is Elijah playing the build, so ask them to retest after each change.
- What Claude can check in the cloud:
  - C: a clang `-fsyntax-only` pass against stubbed generated headers.
  - Formatting: clang-format 14 before pushing (CI's "Check format" uses it, on Ubuntu 24.04).
  - Builder: `apt-get install mono-mcs libmono-system-windows-forms4.0-cil libmono-system-drawing4.0-cil`, then
    `mcs -langversion:5 -target:winexe -optimize+ -out:BEN-Builder.exe -r:System.Windows.Forms.dll -r:System.Drawing.dll -win32icon:ben.ico -resource:ben.ico,BenDrowned.ben.ico -resource:ben.png,BenDrowned.ben.png -resource:laugh.wav,BenDrowned.laugh.wav BenBuilder.cs Laugh.cs BuildBranch.cs`
    from `windows/builder/` (set `BuildBranch.Name` to the PR branch first). `xvfb-run` + `import` can screenshot it.
    The CI artifact can't be downloaded from the cloud container (its blob host is blocked), so compile it locally.
- **CI gotcha:** `tools/format.py` runs `clang-tidy --fix-errors` on each `src/**/*.c` standalone. Code that only
  compiles when `#include`d (needs `z_en_ben.c` statics) must not end in `.c` or tidy rewrites it; use `.inc`.
  Pure data `.inc.c` is fine.

## Elijah's standing preferences

- **Cryptid stalker, not a pest.** BEN appears rarely and at a distance, is gone when you look back, speaks only in
  rare, deliberate moments, and breaks his rules once per session as the scare. Dread comes from rarity, distance,
  silence and doubt.
- **Live Jadusable's story in canon order** (Sessions 0 to 7). Original scares are welcome ("make it more creepy and
  don't be afraid to go offscript") as long as the canon beats still happen.
- **No-Nintendo-assets rule is dropped** (2026-10-14: "I don't care about it"). Game-derived art and audio (the real
  Salesman laugh, the Link fan-art icon) may be bundled; don't flag them.
- **Non-canon creepypasta entities** (Slenderman-style, SCPs, etc., with their own models/mechanics) go behind the
  **Expanded Horror** toggle in the builder. Off = the pure **Jadusable Experience**.
- **Always attach the latest BEN-Builder.exe in the chat/thread after each build or change** ("always send builder
  here for ease"). Compile it from the branch you changed so it builds that branch.
- Progress must be clear without being obvious: the player should know what they can do, not run around aimlessly.
- Elijah tests on Windows in an emulator (ares is the accuracy reference).

## Files outside the repo

Everything needed to build is in the repo, including the builder's icon and laugh (`windows/builder/`) and the
in-game laugh (`assets/audio/samples/SampleBank_0/great_fairy_laughter.wav`). The only other things the new account
needs are:

| File | Needed? | Why |
|---|---|---|
| **BEN-Builder.exe** (fresh, default branch) | Yes, for Elijah | The app Elijah builds with. Also downloadable from the `builder` release link above. |
| Elijah's Majora's Mask (USA) ROM | Yes, on Elijah's PC only | Never share or commit it. |
| `salesman-laugh.mp3` (original upload) | Optional | Elijah's source clip of the Salesman laugh; the repo has the processed WAVs. Keep it in case the laugh needs re-cutting. |
| `link-fan-art.png` (original upload) | Optional | Source image for the builder icon; the repo has `ben.ico`/`ben.png`. |

Skippable (previews and intermediates already represented in the repo): `builder-preview.png`,
`builder-options.png`, `builder-story-choice.png`, `visitors.png`, `visitor-faces.png`, `scps.png`, `scp-faces.png`,
`laugh.wav` (same clip as `windows/builder/laugh.wav`), `salesman-laugh.wav` (an earlier cut), and the old
`BEN-Builder.exe` from 2026-11-13 (it is pinned to the merged PR branch and won't pick up new work).

## Getting the new account set up

1. Connect GitHub in the new Claude account. If it uses a different GitHub login, add it as a collaborator on
   `StormEf4/BEN-DROWNED` (or transfer the repo) so Claude can push branches and open PRs; the Claude GitHub App must
   be installed on the repo.
2. Create a project with this repo, then tell Claude to read `HANDOFF.md`.
3. Upload the optional source files above if you want them kept.
