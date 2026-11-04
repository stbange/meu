# M1a power and infrastructure — 4 November 2026

Implementation: fixed-tick integer energy/heat/daylight; parked solar collectors; cooling/fabrication priority; a tower grid with 12-second pulses and isolated overload protection. The bridge expedition chart is at the left rear console; the power console is beside the maintenance helper in engineering. Native station actions use 1–8, E/0 closes; web uses matching labelled buttons. The first-person room remains visible. Life-support allocation cannot be consumed by main-bus work. Return protection safely stops low-energy travel. Rates remain provisional.

Source: M1a commit containing this record, on codex/m1-authored-expedition. Same Debian/GCC/Mesa host as M0f continuation. Commands after sourcing `.tools/activate.sh`:

- `python3 tools/build.py core --test`: 13/13 passed, including charge, overload, pause, transaction rejection, low energy and persisted power.
- `ASAN_OPTIONS=detect_leaks=0 python3 tools/build.py core --test --sanitizers`: 13/13 passed. LeakSanitizer alone is disabled because this host runs under ptrace; address/undefined sanitizers stay enabled.
- `LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy xvfb-run -a python3 tools/build.py native --test --smoke`: 13/13 and rendered smoke passed.
- `node --check web/input.js` and `node --check web/storage.js`: passed.
- `OPENSPEC_TELEMETRY=0 npm run spec:validate`: both changes passed.

Schema 2 uses explicit 256-byte encoding with a second checksum; legacy M0 snapshots migrate without editing source. Tests retain both reference fixtures. Local web SDK/browser downloads remain policy-blocked; browser execution and changed Windows builds are not verified on this host. Actual branch push was server-rejected with `fatal error in commit_refs`; dry-run was insufficient to prove successful publication. Retry at each completed step and report the result.

User reviews are deferred until M1. M0 and M1 acceptance remain pending.
