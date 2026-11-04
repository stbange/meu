# M1b maintenance helper — 4 November 2026

Source: commit containing this record on codex/m1-authored-expedition. Host/toolchain matches M1a. A real grid pulse damages the common distribution buffer, causing dim displays and a slow arm. Inspect via engineering power console → maintenance helper; isolate the bus, diagnose and reserve 1 alloy + 2 conductor. The helper works for ten active seconds. Missing parts, lost access and insufficient energy are reported; cancellation refunds stock once. Parts and partial progress survive saved journeys. Essential doors/lift remain independent.

Verification:
- `ASAN_OPTIONS=detect_leaks=0 python3 tools/build.py core --test --sanitizers`: 14/14 passed; ptrace prevents LeakSanitizer on this host.
- `LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy xvfb-run -a python3 tools/build.py native --test --smoke`: 14/14 plus OpenGL smoke passed.
- `OPENSPEC_TELEMETRY=0 npm run spec:validate`: three active changes passed.

The production-loop expedition test checks diagnosis/isolation rejections, insufficient stock, duplicate queue, exact cancellation refunds, blocking without access, partial-job save/reload, paused time and eventual completion. No unfamiliar device can be diagnosed by the helper. Web/Windows verification and user acceptance remain pending as described in M1a. M1a push was rejected by GitHub with Internal Server Error; commits remain local until a push succeeds.
