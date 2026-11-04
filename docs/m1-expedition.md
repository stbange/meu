# M1 tower expedition

Implementation branch: `codex/m1-authored-expedition`. The public Pages prototype remains on main until a separate merge/publication decision. M0/M1 human review is pending; this guide does not request interim acceptance.

## Launch on this Linux machine

```bash
cd /workspace/meu
source .tools/activate.sh
npm run dev:native
```

For a clean supported Windows setup use `npm ci --ignore-scripts`, `npm run bootstrap`, then `npm run dev:native`. Native station panels use 1–8 for their numbered actions and 0/E to close. Browser builds use the same labelled buttons; keyboard digits also work. Walk with WASD, drag/arrows to look, E to use; the service lift uses 1/2/3. Station panels retain the first-person surroundings and active simulation time. Esc pauses; Enter resumes. F2 opens durable journeys.

## Complete the expedition

1. At the left bridge chart, read the objective, projected 3000 kJ expenditure, 2000 kJ uncertainty and 4000 kJ return margin. Take the central helm. Drive north to around Z-140, brake and park, then release the helm. Tower socket bounds are X+/-15, Z-120..-175, heading000.
2. Walk back into the lift, choose 3 for engineering. The power console/helper is on the left. Deploy collectors so useful charging overlaps interior work. Isolate the workshop bus, connect the tower grid and observe its repeating 12-second surge. Essential doors, lift and life support remain independent.
3. Open the maintenance helper, inspect the dim-display/slow-arm symptoms and diagnose the damaged distribution buffer. Queue the known repair: 1 alloy and 2 conductor. It takes 10 active seconds with isolation and power. Missing parts/access are reported; cancellation refunds reserved parts once. Save/reload can preserve partial work.
4. At the workshop on the right, use the repaired manipulator to recover 4 alloy, 4 conductor and 2 sealant. The phase coupler itself needs precise suit work.
5. Continue through the opening in the right structural rib to the wrist airlock. Scan the landing: 80 kPa, -10 C, 0.2 mSv/h, low debris. Power its shelter field, then suit up with at least 800 kJ and heat<=6000. Walk right through the airlock along the railings. The unit stays parked. The HUD shows 180-second suit supply, exposure and the return direction.
6. At the conduit console, fit the parallel bypass before removing the coupler (1 conductor, 1 sealant, 50 kJ). Extract the coupler; return along the rails to cabin X<14, then remove the suit at the airlock console. Removing the source without bypass or disconnecting shelter raises exposure. Catastrophe never relocates you automatically: choose checkpoint recovery explicitly, losing unsecured progress/salvage while preserving the home.
7. In engineering power, select fabrication priority and stow collectors. Disconnect the grid if still connected. At the workshop, test the coupler (100 kJ), fabricate its adapter (3 alloy, 3 conductor, 1 sealant, 300 kJ), then install the stabilizer (50 kJ). Every panel displays available/required resources and heat costs. Repeated requests do not duplicate stock or charge twice.
8. Return to bridge and take the helm. Reverse to around Z-90 to clear the tower buttress. Turn left to heading 270 and drive west onto the turquoise powered bridge. Park beyond X-130 to complete the expedition. That position was unsupported before installation. Do not retract the bridge while over it; removal requires returning to the main tower socket.
9. Release the helm, pause and use F2→F5 to save the completed journey. Loading restores installed access, materials and completion paused. Schema 1 M0 journeys remain importable; new schema 2 journeys include expedition state and previous-generation recovery/export/import.

## Verification and limitations

```bash
source .tools/activate.sh
ASAN_OPTIONS=detect_leaks=0 python3 tools/build.py core --test --sanitizers
LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy xvfb-run -a python3 tools/build.py native --test --smoke --tour --pilot --lifecycle --persistence --expedition
```

`npm run test:expedition` is the shorter native test/replay entry point (run under Xvfb when headless). The replay starts clean, uses actual SDL input without teleports or resource grants, completes the expedition, then publishes and reloads a real completed journal. Its saves go under `build/native/expedition-replay`, never the player’s save directory. Native-to-browser fixtures and captures are generated in `test-results/m1-*`.

Emscripten and matching browsers cannot download on this host under the current network policy. Web/Windows behavior must be verified in CI or on another capable host; the added browser scenarios are not claimed as locally executed. Human phone usability, graphics/hardware performance,20–40-minute pacing and combined M0/M1 acceptance remain unmeasured/pending. All resource rates remain provisional.
