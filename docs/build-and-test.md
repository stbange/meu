# Build and Test MEU

M0d adds complete touch contexts, session settings and interruption handling to the connected 240 m home and its supported bridge travel. The bridge is at 196 m, living quarters at 156 m, engineering at 112 m, with a service lift connecting them. Approach the central bridge console and use E or the touch prompt to take the helm. W/S drives, A/D turns, Space brakes, P parks and E releases control. Looking is independent of steering. Releasing drive brakes; releasing the helm requests a parked stop. The exterior is an authored scale fixture; seeded city generation, repair gameplay and saves come later.

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

Enter or click begins exploration and sound. WASD walks; mouse drag or arrows look; E uses a nearby object; Escape pauses; Enter resumes. F1 opens/closes Settings: Up/Down selects, Left/Right adjusts, Space toggles, R resets. Closing settings leaves the game paused; Enter explicitly resumes. Sensitivity, vertical drag inversion, walking bob, machine sway and volume apply immediately for this session. Both camera effects default off; saved settings arrive in M0e. The lift is behind the starting position. Walk backward into its centre, press 1 (bridge), 2 (quarters), or 3 (engineering), wait for the gates, then walk forward out. Furniture, windows and walls block movement. Engineering's central ramp leads to the reactor catwalk.

~~~text
npm run test:native
npm run smoke:native
python tools/build.py native --test --tour --pilot --lifecycle
~~~

The smoke command creates a real OpenGL context, compiles shaders, renders three frames, checks framebuffer variation, writes `test-results/native-room.bmp`, and exits nonzero on failure. It needs a working OpenGL 3.3 driver and windowing session. The tour injects SDL keyboard events through the production input handler, stands for 60 simulated seconds, walks all decks, rides the lift and climbs the ramp while a motion fixture translates, turns, accelerates and brakes the robot. It captures four first-person views and rejects standing drift beyond 2 cm, loss of support or an incomplete route. The separate piloting tour uses the actual helm: it reaches full speed, looks without steering, turns toward a visible buttress and checks a controlled safety stop while throttle remains held. Its captures are named native-pilot-helm/travel/blocked.bmp. The lifecycle replay edits settings before audio starts, pauses held input through settings/focus loss/resize, checks frozen time, explicit resume and fresh look input, and captures native-settings/resumed.bmp. The fixture drives the normal robot pose input; player coordinates remain robot-local. Windows CI compiles and runs core tests; Linux CI additionally renders the tour with Mesa/Xvfb.

## Web and phone layout

~~~text
npm run bootstrap:web
npm run build:web
npm run serve:web
~~~

Open `http://127.0.0.1:4174` in a WebGL 2 browser. Select Begin exploration. Use the movement buttons and drag the look pad, or use desktop controls. Walk backward into the lift and tap a destination. Contextual buttons provide the same actions as E. Portrait and landscape work without pointer lock or fullscreen. Focus loss, backgrounding, or resizing pauses and clears held input; resume explicitly.

The server binds to this computer by default. To test a physical phone on the same network, use `npm run serve:web -- --host 0.0.0.0` and open this computer's LAN address on port 4174. Physical Android/iPhone interaction, performance and Safari compatibility remain pending; Chromium/WebKit viewport tests do not establish them.

~~~text
npx playwright install chromium webkit
npm run test:web
~~~

The suite starts and stops its own HTTP server; close a manually running MEU server on port 4174 first. It loads the real WebAssembly build and checks rendering, audio gating, keyboard movement, browser touch events, pause, input cleanup, resize, and responsive controls. Keyboard/touch piloting scenarios check look/steer independence, parking while moving, helm release, pause/resume and automatic stops at structures and the causeway edge. A continuous keyboard/touch route visits quarters, returns to the bridge helm to drive and park, visits engineering, and returns to the bridge. It also checks lift gates, mid-ride pause, room heights and the engineering ramp. Settings checks cover every option, audio gain, no pre-gesture audio, reset, keyboard access and safe-area bounds. Multi-finger tests cover simultaneous walk/look, cancellation, capture loss, duplicate owners, context changes and resume while old fingers stay down. Browser-driven minimization and freezing verify focus interruption and no catch-up after explicit resume. WebKit phone-layout tests render the real WebGL client and operate settings/start/pause/resume through touch; they are engine tests rather than physical iPhone evidence. Reports include build/scene identity, simulation ticks, input events, screenshots, and failure traces. Read-only diagnostics are enabled by the development build; input tests follow actual keyboard/pointer/touch paths.

For an optimized build without the test bridge:

~~~text
python tools/build.py web --release --test
~~~

Serve it with `python tools/serve.py --directory build/web-release/bin`. CI also runs launch checks against this build and requires the diagnostic export to be absent. Exported input functions are the production browser adapter.

## Linux reference build

The automated reference is the official `gcc:14.2.0-bookworm` container. Install Python 3, certificate support, and the SDL/OpenGL development packages listed in `.github/workflows/ci.yml`, then run:

~~~text
python3 tools/bootstrap.py
python3 tools/build.py native --test
xvfb-run -a python3 tools/build.py native --test --tour --pilot --lifecycle
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

The helper accepts `--build-dir` for an independent generated build directory. Never share a generated tree between native and Emscripten compilers. The `meu_core_tests` target checks simulation, timing, and camera math. The `meu_interior_tests` target checks collision, lift interlocks, remote calls, interactions, ramp continuity, transform inversion and a 60-second standing-motion fixture. The `meu_scenarios` target reads a replay fixture through the production command/update loop. The new meu_travel_tests target checks stopping boundaries, reverse travel, complete foot support, swept turning clearance, overhead/arm clearance and context ownership. The meu_settings_tests target checks validation, settings/panel contexts, camera isolation and pause/resume through moving-helm settings. A deterministic settings fixture exercises the production commands. CTest runs all nine groups natively and, for web builds, as WebAssembly under Node.

The Windows helper identifies the compiler from object/archive metadata rather than a disposable executable. Building and running the actual tests independently verifies linking/runtime behavior.

CI builds carry the full source commit from GITHUB_SHA, verified by the native scenario and browser checks. Other checkouts use Git's revision/dirty marker. A source archive can explicitly supply CMake's MEU_REVISION value when no Git metadata is available.

## Evidence and limits

See [M0d evidence](evidence/m0d-controls.md) for results and captures. CI artifacts contain binaries, the web distribution, reports, and images. The optimized runtime is published from main after checks pass; [GitHub Pages publishing](github-pages.md) describes the public package and live verification. The prototype is not a user-accepted release.

M0e–M0f remain open. M0d implementation and automated checks are recorded separately from pending physical-phone checks and user acceptance. M0c uses authored flat support and conservative hull bands. It supplies continuous supported footprint travel; free physical balancing and generated terrain belong to later work. Saves and persistent settings, production graphics-context recovery, real-phone performance and the final supported-browser matrix remain pending.
