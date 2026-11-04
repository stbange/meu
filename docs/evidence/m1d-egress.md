# M1d restricted suit excursion — 4 November 2026

Source: commit containing this record. Host matches M1a. Engineering's right-side wrist airlock opens onto a railed service conduit at112 m; the existing capsule/floor/ray and first-person renderer handle the whole crossing. No egress teleport. The parked aligned tower scan reports pressure, temperature, radiation, debris and field conditions. The airlock rejects missing scan/repair/shelter, low return energy, excessive heat or unavailable landing. During egress the unit stays parked and HUD reports suit time, exposure and the cabin return direction.

A fitted bypass preserves shelter during coupler recovery. Unsafe removal/disconnection increases exposure with a retreat window. Exceeded limits freeze movement without automatic relocation; the explicit recovery button restores a secure cabin checkpoint, losing unsecured coupler/progress and preserving the home. Native/browser codecs retain outside pose, timer, exposure, bypass, field and checkpoint; schema1 still migrates.

Verification after `.tools/activate.sh`:
- `ASAN_OPTIONS=detect_leaks=0 python3 tools/build.py core --test --sanitizers`:14/14 passed (LeakSanitizer unavailable under ptrace).
- `LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy xvfb-run -a python3 tools/build.py native --test --smoke --tour --pilot --lifecycle --persistence`:14/14 plus all rendered/regression/persistence checks passed.
- `OPENSPEC_TELEMETRY=0 npm run spec:validate`:five active changes passed.

The production-loop expedition test checks blocked egress conditions, normal movement across the actual aperture, bypass/recovery, save/reload outside and return, unsafe extraction catastrophe, explicit recovery and exact stock restoration. The integrated rendered expedition is M1e. Web/Windows execution remains separate from these local results; reviews remain deferred. M1c push hit the same intermittent GitHub server error and is retried with this step.
