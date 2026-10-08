# Building BEN DROWNED

You need your own copy of **The Legend of Zelda: Majora's Mask, Nintendo 64, USA** as a ROM file
(`.z64`, `.n64` or `.v64` all work). It never leaves your computer and is never committed.

Everything else is automatic. One command, `./ben`, sets up, builds, updates, patches and runs the hack, and
the same commands keep working for every future version of the project.

---

## Windows 10 / 11 (recommended path)

Open **PowerShell** (Start menu → type "PowerShell") and paste:

```powershell
irm https://raw.githubusercontent.com/StormEf4/BEN-DROWNED/HEAD/windows/BEN-Setup.ps1 | iex
```

It walks you through everything:

1. **Installs WSL2 + Ubuntu** if you don't have them. Windows asks for admin permission and then a restart.
   After restarting, open **Ubuntu** from the Start menu once, pick a username and password, and paste the
   command above again. It picks up where it left off.
2. **Asks you to pick your ROM** in a normal file window. If you cancel, it searches Downloads, Desktop and
   Documents for you.
3. **Downloads the project** into Ubuntu, installs the build tools (asks for your Ubuntu password once),
   extracts the game's assets and builds the hack. The first time takes about **10–30 minutes**.
4. Opens the folder **`C:\Users\<you>\BEN-DROWNED`** with your ROM in it, and adds two desktop shortcuts:
   - **BEN DROWNED - Update & Build**: gets the newest version and rebuilds it. Use this after every update.
   - **BEN DROWNED - Play**: opens the latest build in ares.

Open `BEN-DROWNED-latest.z64` in [ares](https://ares-emu.net/), our reference emulator.

> Already downloaded the project as a zip? Double-click `windows\BEN-Setup.cmd` instead of pasting the command.

## Linux (Ubuntu/Debian) or an existing WSL terminal

```bash
cd ~
git clone https://github.com/StormEf4/BEN-DROWNED.git
cd BEN-DROWNED
./ben setup "/path/to/Majora's Mask (USA).z64"
```

Leave the path off and `./ben setup` searches your usual folders (including the Windows ones under WSL).

## macOS

Install [Homebrew](https://brew.sh) and MIPS binutils (see "Building mips-linux-binutils" in
[`docs/BUILDING_MACOS.md`](../BUILDING_MACOS.md)), then run the Linux commands above. `./ben setup` installs
the Homebrew packages itself.

---

## Everyday use

| Command | What it does |
|---|---|
| `./ben update` | Get the newest version of the project and build it. **Use this after every update.** |
| `./ben build` | Rebuild what you have (after editing files). `./ben build --clean` forces a full rebuild. |
| `./ben run` | Open the latest build in ares (it finds ares or remembers where you told it). |
| `./ben doctor` | Check your machine and say exactly what to fix. Run this first when something is wrong. |
| `./ben patch` | Make the release `.bps` patch in `dist/` (Flips is fetched and built automatically). |
| `./ben clean` | Delete build output (keeps extracted assets, so the next build is quick). |
| `./ben reset` | Delete everything generated except your ROM. The next build starts from scratch. |
| `./ben config` | Show or change settings: `EMULATOR` (path to ares), `WINDOWS_COPY` (0 or 1). |

Where things end up:

| | |
|---|---|
| Your build | `dist/BEN-DROWNED-latest.z64` (plus a copy named after the version, e.g. `BEN-DROWNED-v0.1-3-gabc123.z64`) |
| Windows copy | `C:\Users\<you>\BEN-DROWNED\` (turn off with `./ben config WINDOWS_COPY 0`) |
| Logs | `.ben/logs/` (`latest.log` is the most recent step) |
| Release patch | `dist/BEN-DROWNED-<version>.bps` |

## Why it keeps working for future builds

- **Every step is fingerprinted.** `./ben build` hashes what each step depends on and redoes only what
  changed: the system packages list, the Python requirements, the asset extractors and their configs, your
  ROM, and the build settings. After an update that changes the extraction tools, assets are re-extracted
  automatically, and after a small change only the changed files recompile.
- **The decomp's own `make` flags are hidden.** The hack is never byte-identical to the original game, so
  `./ben` always builds with `NON_MATCHING=1 COMPARE=0`. You never see a "checksum FAILED" message for a
  correct build.
- **Your ROM is imported once.** It's converted to the right byte order, checked against the decomp's own
  checksum files, and kept in `baseroms/n64-us/` (git-ignored). Updates never ask for it again.
- **Builds are named after the exact version** (`git describe`), so a bug report can always say which build it was.

## Troubleshooting

| Symptom | Fix |
|---|---|
| Anything at all | Run `./ben doctor`. It lists each problem with its fix. |
| "not the N64 USA ROM this hack needs" | That ROM is another region, the GameCube version, or already patched. You need the USA N64 cartridge dump. |
| Very slow builds on Windows | The project is on the Windows drive (`/mnt/c/...`). Clone it into the Ubuntu home folder (`cd ~`) instead. `./ben doctor` warns about this. |
| A step failed | `./ben` prints the first error and a log path. Send the log (or `tail -n 50 .ben/logs/latest.log`) along with the output of `./ben doctor`. |
| "You have local changes" on update | You edited project files. `git stash` saves them aside, then run `./ben update` again (`git stash pop` brings them back). |
| Strange errors after a big update | `./ben build --clean`, and if that fails, `./ben reset` then `./ben build`. |
| Want to see every compiler line | `BEN_VERBOSE=1 ./ben build` |

---

## Maintainer notes (keeping `./ben` future-proof)

- **New system package needed?** Add it to `APT_PACKAGES` (and `BREW_PACKAGES`) in `ben`, and **bump `DEPS_REV`**.
  Everyone's next build installs it.
- **New Python package?** Add it to `requirements.txt`. The venv rebuilds automatically.
- **Changed anything that affects extraction** (`assets/xml`, the extractor tools, `baseroms/n64-us/*`)? Nothing to do;
  it's detected. Our own `tools/ben/` is excluded so editing our helper scripts never triggers re-extraction.
- **New build step** (e.g. the K7 audio pipeline generating sequences)? Add an `ensure_*` function to `ben`
  with its own fingerprint and call it from `do_build`, so the user-facing commands never change.
- Release names come from git tags: tag milestones (`git tag v0.1-slice`) and builds pick the name up.
