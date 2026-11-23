# Notes for Claude

Start with [HANDOFF.md](HANDOFF.md): project goal, where things stand, story architecture, build setup, open work
and Elijah's standing preferences. The route as built is in `docs/ben/PLAYTHROUGH.md`; decisions and open questions
are in `docs/ben/DECISIONS.md`.

- The default branch is `claude/gifted-carson-g2jeyn`; PRs target it.
- You can't build the ROM here (it needs Elijah's own ROM). Check C with clang `-fsyntax-only`, run clang-format 14,
  and ask Elijah to playtest.
- After every change, compile BEN-Builder.exe from your branch and attach it in the chat.
