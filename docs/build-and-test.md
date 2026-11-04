# Build and Test MEU

M1a–M1e are implemented on `codex/m1-authored-expedition`. See [the full expedition route](m1-expedition.md) and [M1e evidence](evidence/m1e-expedition.md). Run `python tools/build.py native --test --expedition` for the full SDL-input route. On the current Linux host first source `.tools/activate.sh` for project-local development libraries.

M0e adds durable local journeys, saved settings, snapshot recovery and portable export/import to the connected 240 m home and supported bridge travel. The bridge is at 196 m, living quarters at 156 m, engineering at 112 m, with a service lift connecting them. Approach the central bridge console and use E or the touch prompt to take the helm. W/S drives, A/D turns, Space brakes, P parks and E releases control. Looking is independent of steering. Releasing drive brakes; releasing the helm requests a parked stop. The exterior is an authored scale fixture; seeded city generation and repair gameplay come later.

## Reference versions

| Component | Pinned version |
| --- | --- |
| MEU source language | C17 without GNU extensions |
| CMake | 3.31.6 |
| Ninja | 1.12.1 |
| Windows/Linux reference compiler | GCC 14.2.0 |
| Windows compiler distribution | w64devkit 2.1.0 |
| Windows archive reader | 7zr 25.01 |
| SDL | 3.4.18, built from checksum-pinned source |
| Emscripten | 6.0.10 |
| Emscripten SDK manifest | d7b4130688bfa348392b921ab04015aa15c4f154 |
| Browser automation | Playwright 1.64.1 with its matching Chromium and WebKit |
| CI Node | 24.18.0 |
| OpenSpec | 1.14.3 |

Python 3.11 or later and Git are prerequisites for the helpers. Node 22.12 or later runs the development scripts. Bootstrap downloads stay in this checkout's ignored `.tools/`. Portable tools and source archives are checked against the SHA-256 values in `tools/toolchains.json`; CMake separately checks SDL's source archive. Emsdk installs its compiler bundle through its pinned manifest. Bootstrap does not change system PATH, registry, or global SDK activation. The build helper constructs a per-process environment.

SDL's browser backend uses GNU interop macros, so that dependency compiles with extensions enabled. MEU's application, simulation, and tests remain strict C17. Desktop rendering uses direct OpenGL 3.3 core; the web uses the equivalent OpenGL ES 3/WebGL 2 path. The game does not use SDL's renderer or GPU abstraction.

## Windows quick start

From the MEU checkout:

~~~text
npm ci --ignore-scripts
npm run bootstrap
npm run dev:native
~~~

Enter or click begins exploration and sound. WASD walks; clicking the view captures the mouse; moving it or arrows look; E uses a nearby object; Escape pauses; Enter resumes. F1 opens/closes Settings: Up/Down selects, Left/Right adjusts, Space toggles, R resets. Closing settings leaves the game paused; Enter explicitly resumes. Sensitivity, vertical drag inversion, walking bob, machine sway and volume apply immediately. Both camera effects default off. Save a journey to keep settings; closing settings alone does not save them. The lift is behind the starting position. Walk backward into its centre, press 1 (bridge), 2 (quarters), or 3 (engineering), wait for the gates, then walk forward out. Furniture, windows and walls block movement. Engineering's central ramp leads to the reactor catwalk.

~~~text
npm run test:native
npm run smoke:native
python tools/build.py native --test --tour --pilot --lifecycle --persistence
~~~

The smoke command creates a real OpenGL context, compiles shaders, renders three frames, checks framebuffer variation, writes `test-results/native-room.bmp`, and exits nonzero on failure. It needs a working OpenGL 3.3 driver and windowing session. The tour injects SDL keyboard events through the production input handler, stands for 60 simulated seconds, walks all decks, rides the lift and climbs the ramp while a motion fixture translates, turns, accelerates and brakes the robot. It captures four first-person views and rejects standing drift beyond 2 cm, loss of support or an incomplete route. The separate piloting tour uses the actual helm: it reaches full speed, looks without steering, turns toward a visible buttress and checks a controlled safety stop while throttle remains held. Its captures are named native-pilot-helm/travel/blocked.bmp. The lifecycle replay edits settings before audio starts, pauses held input through settings/focus loss/resize, checks frozen time, explicit resume and fresh look input, and captures native-settings/resumed.bmp. The fixture drives the normal robot pose input; player coordinates remain robot-local. Windows CI compiles and runs core tests; Linux CI additionally renders the tour with Mesa/Xvfb.

## Web and phone layout

~~~text
npm run bootstrap:web
npm run build:web
npm run serve:web
~~~

Open `http://127.0.0.1:4174` in a WebGL 2 browser. Select Begin exploration. Use the movement buttons and drag the look pad, or use desktop controls. Walk backward into the lift and tap a destination. Contextual buttons provide the same actions as E. Desktop canvas clicks request pointer lock; Escape releases and pauses. Touch portrait/landscape and browser-denial fallback remain usable without pointer lock or fullscreen. Focus loss, backgrounding, or resizing pauses and clears held input; resume explicitly.

The server binds to this computer by default. To test a physical phone on the same network, use `npm run serve:web -- --host 0.0.0.0` and open this computer's LAN address on port 4174. Physical Android/iPhone interaction, performance and Safari compatibility remain pending; Chromium/WebKit viewport tests do not establish them.

~~~text
npx playwright install chromium webkit
npm run test:web
~~~

The suite starts and stops its own HTTP server; close a manually running MEU server on port 4174 first. It loads the real WebAssembly build and checks rendering, audio gating, keyboard movement, browser touch events, pause, input cleanup, resize, and responsive controls. Keyboard/touch piloting scenarios check look/steer independence, parking while moving, helm release, pause/resume and automatic stops at structures and the causeway edge. A continuous keyboard/touch route visits quarters, returns to the bridge helm to drive and park, visits engineering, confirms a durable save, reloads a fresh WebAssembly instance at the catwalk with settings restored, and returns to the bridge. It also checks lift gates, mid-ride pause, room heights and the engineering ramp. Settings checks cover every option, audio gain, no pre-gesture audio, reset, keyboard access and safe-area bounds. Multi-finger tests cover simultaneous walk/look, cancellation, capture loss, duplicate owners, context changes and resume while old fingers stay down. Browser-driven minimization and freezing verify focus interruption and no catch-up after explicit resume. WebKit phone-layout tests render the real WebGL client and operate settings/start/pause/resume through touch; they are engine tests rather than physical iPhone evidence. Reports include build/scene identity, simulation ticks, input events, screenshots, and failure traces. Read-only diagnostics are enabled by the development build; input tests follow actual keyboard/pointer/touch paths.

For an optimized build without the test bridge:

~~~text
python tools/build.py web --release --test
~~~

Serve it with `python tools/serve.py --directory build/web-release/bin`. CI also runs launch and durable-save/export/import checks against this build and the public page at all five browser/layout configurations, requiring the diagnostic export to be absent. Exported input functions are the production browser adapter.

## Saved journeys

Pause and select Saved journeys on the web, or press F2 on native desktop. Save journey includes the Commander/robot pose, lift location, lights, visited spaces, simulation ticks and all settings. Park the unit and let the lift finish before saving. If you opened the menu while moving, close it and resume to let the unit brake or the lift arrive, then save. Save completion is shown only after the platform confirms publication.

Native keys inside the menu: F5 saves, F9 loads the latest, F8 recovers the previous snapshot, F6 exports through the file picker, F7 imports, and F2/Escape closes. The `--persistence` replay uses separate processes and an isolated `build/native/persistence-replay` directory; it does not use personal saved journeys. It writes native-save-menu/restored.bmp captures.

Normal native saves use SDL's independent `MEU/MechanizedExplorationUnit` preference directory (typically under AppData/Roaming on Windows or XDG data on Linux). The native log prints the exact path. `--save-dir <directory>` selects an explicit independent directory. Browser saves use MEU's own `meu-local-saves` IndexedDB database for the current origin/browser profile; the browser does not share them with a native installation or another browser.

A valid latest journey is restored automatically, paused and parked with inputs/helm cleared. A damaged or incompatible latest file is kept and previous recovery is offered rather than applied automatically. Export produces the same versioned .meu snapshot on native and web. Import validates the whole candidate before replacing live state and does not overwrite local saved generations; Save here explicitly to keep it. Export remains available when local storage fails. The most recent damaged source from each slot is retained when a later explicit save repairs the journal.

Settings are saved as part of a journey, not automatically when changed. Closing/backgrounding the game does not silently save. See [snapshot format and publication protocol](save-format.md) for schema, validation, paths, recovery and durability boundaries. Browser export reports that a download started; it does not claim a local save was confirmed.

## Linux reference build

The automated reference is the official `gcc:14.2.0-bookworm` container. Install Python 3, certificate support, and the SDL/OpenGL development packages listed in `.github/workflows/ci.yml`, then run:

~~~text
python3 tools/bootstrap.py
python3 tools/build.py native --test
xvfb-run -a python3 tools/build.py native --test --tour --pilot --lifecycle --persistence
python3 tools/build.py core --test --sanitizers
~~~

The rendered tour job sets `LIBGL_ALWAYS_SOFTWARE=1` and `SDL_AUDIODRIVER=dummy`. A normal Linux desktop can launch without Xvfb. The reference helper requires GCC 14.2.0 instead of silently selecting another compiler. Other C17 compilers can be configured with CMake manually, with their versions/results recorded separately. No local Linux installation is assumed on the Windows development computer.

## Headless and manual CMake use

The core build requires no SDL, display, audio, OpenGL, or network-fetched runtime dependency:

~~~text
python tools/build.py core --test
~~~

With an already installed C compiler and CMake:

~~~text
cmake -S . -B build/manual-core -DMEU_BUILD_CLIENT=OFF
cmake --build build/manual-core
ctest --test-dir build/manual-core --output-on-failure
~~~

The helper accepts `--build-dir` for an independent generated build directory. Never share a generated tree between native and Emscripten compilers. The `meu_core_tests` target checks simulation, timing, and camera math. The `meu_interior_tests` target checks collision, lift interlocks, remote calls, interactions, ramp continuity, transform inversion and a 60-second standing-motion fixture. The `meu_scenarios` target reads a replay fixture through the production command/update loop. The new meu_travel_tests target checks stopping boundaries, reverse travel, complete foot support, swept turning clearance, overhead/arm clearance and context ownership. The meu_settings_tests target checks validation, settings/panel contexts, camera isolation and pause/resume through moving-helm settings. A deterministic settings fixture exercises the production commands. The snapshot tests verify 600 assertions and 5,000 bounded hostile payloads against the portable reference file. The native file-journal tests inject write/flush/replace/interruption failures and verify source retention and previous-snapshot recovery. The new snapshot command fixture uses the production loop. CTest runs 12 groups natively/headless and 11 WebAssembly groups under Node; native file I/O is exercised through IndexedDB browser tests on the web.

The Windows helper identifies the compiler from object/archive metadata rather than a disposable executable. Building and running the actual tests independently verifies linking/runtime behavior.

CI builds carry the full source commit from GITHUB_SHA, verified by the native scenario and browser checks. Other checkouts use Git's revision/dirty marker. A source archive can explicitly supply CMake's MEU_REVISION value when no Git metadata is available.

## Evidence and limits

See [M0e evidence](evidence/m0e-saves.md) for results and captures. CI artifacts contain binaries, the web distribution, reports, and images. The optimized runtime is published from main after checks pass; [GitHub Pages publishing](github-pages.md) describes the public package and live verification. The prototype is not a user-accepted release.

M0a–M0e are implemented; M0f remains open. Automated checks remain separate from pending physical-phone checks and user acceptance. M0c uses authored flat support and conservative hull bands. It supplies continuous supported footprint travel; free physical balancing and generated terrain belong to later work. Production graphics-context recovery, physical-phone performance and the final supported-browser matrix remain pending.
