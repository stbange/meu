# Pip onboard help - 4 November 2026

User-requested post-M1 onboarding: a hovering guide projection explains the game and the next actionable step. Runtime 0.1.2; scene/content and save schema remain unchanged. M2 and human acceptance remain pending. Work is on `codex/m1-authored-expedition`; main/Pages are unchanged.

## Shared content and progression

`src/game/guide.c` supplies seven topics and 30 next-step states from the real simulation: welcome, docking, isolation/grid/repair, stock, suit/shelter/bypass, safe return, testing/adaptation/installation and west-bridge departure/completion. Safety, low energy, heat, missing parts and renewed faults take precedence where applicable. Help reads state; it never performs a command or grants progress. Native/browser use the same authored content without an external help service.

Help is a transient paused context. It clears held input and helm ownership, blocks start while open, retains frozen resources/job/suit clocks, closes other menus and leaves explicit resume after closing. It is excluded from snapshot bytes.

## Checks so far

Debian 13, GCC 14.2.0, pinned CMake 3.31.6/Ninja 1.12.1. Source `.tools/activate.sh` first.

```bash
python3 tools/build.py core --test
OPENSPEC_TELEMETRY=0 npm run spec:validate
```

All 16 native/core groups and eight active OpenSpec changes pass. The guide test follows production station commands through diagnosis/repair, sheltered bypass recovery, testing, fabrication, installation and completion. It also verifies paused reading is read-only, blocks old actions, preserves snapshot bytes and derives completed guidance after restore. Unit fixture poses do not claim physical navigation; rendered SDL/browser paths remain to be checked.

Native/DOM presentation, actual captured-input interruption, sanitizer and cross-platform debug/optimized browser verification are pending. Matching Emscripten/browser downloads remain restricted locally; remote CI supplies those checks. No physical-device usability, performance or user acceptance is claimed.
