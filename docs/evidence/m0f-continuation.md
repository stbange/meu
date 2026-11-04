# M0f automated continuation — 4 November 2026

Source baseline: c1dc6eb. Host: Debian 13 x86-64; GCC 14.2.0, Python 3.12.14, CMake 3.31.6, Ninja 1.12.1, SDL 3.4.18, Mesa llvmpipe 25.0.7. Scene: m0e-saves-v1. This is software rendering, not a reference-device performance result.

From `/workspace/meu`, source `.tools/activate.sh`, then run:

- `python3 tools/build.py core --test`: 12/12 passed.
- `npm run test:native`: native client builds; 12/12 passed.
- `LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy xvfb-run -a python3 tools/build.py native --smoke --tour --pilot --lifecycle --persistence`: all checks passed, including separate-process read/export/import.
- `OPENSPEC_TELEMETRY=0 npm run spec:validate`: foundation proposal passes.

Earlier Windows/Linux/web/browser results remain in the M0e evidence; they were not rerun on this host. Emscripten and matching browser downloads are blocked by the managed network policy (storage.googleapis.com and cdn.playwright.dev). Native and core verification can continue locally; changed web builds must run in remote CI until this is resolved.

The user explicitly requested no reviews until M1 is done. M0f real hardware/phone measurements and user scale, comfort and usability decisions stay pending; M0 is not accepted or archived. Proceed through M1 under this authorization without requesting interim sign-off.
