# Build and Test MEU

M0a supplies a minimal first-person observation room. It is a platform prototype with basic walking/look controls, room bounds, pause/resume, and gesture-started sound. The four-space interior, steering, lifts, repairs, and saves belong to later steps.

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
| Browser automation | Playwright 1.64.1 with its matching Chromium |
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

Enter or click starts the room and sound; WASD moves, mouse drag or arrow keys looks, and Escape pauses. Enter resumes. M0a has only enclosing room bounds; furniture collision and the connected interior are M0b work.

~~~text
npm run test:native
npm run smoke:native
~~~

The smoke command creates a real OpenGL context, compiles shaders, renders three frames, checks framebuffer variation, writes `test-results/native-room.bmp`, and exits nonzero on failure. It needs a working OpenGL 3.3 driver and windowing session. Windows CI compiles and runs core tests; Linux CI additionally renders with Mesa/Xvfb.

## Web and phone layout

~~~text
npm run bootstrap:web
npm run build:web
npm run serve:web
~~~

Open `http://127.0.0.1:4174` in a WebGL 2 browser. Select Begin exploration. Use the movement buttons and drag the look pad, or use desktop controls. Portrait and landscape work without pointer lock or fullscreen. Focus loss, backgrounding, or resizing pauses and clears held input; resume explicitly.

The server binds to this computer by default. To test a physical phone on the same network, use `npm run serve:web -- --host 0.0.0.0` and open this computer's LAN address on port 4174. Actual phone performance and Safari compatibility remain separate checks; viewport emulation does not establish them.

~~~text
npx playwright install chromium
npm run test:web
~~~

The suite starts and stops its own HTTP server; close a manually running MEU server on port 4174 first. It loads the real WebAssembly build and checks rendering, audio gating, keyboard movement, browser touch events, pause, input cleanup, resize, and responsive controls. Reports include build/scene identity, simulation ticks, input events, screenshots, and failure traces. Read-only diagnostics are enabled by the development build; input tests follow actual keyboard/pointer/touch paths.

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
xvfb-run -a python3 tools/build.py native --test --smoke
python3 tools/build.py core --test --sanitizers
~~~

The smoke job sets `LIBGL_ALWAYS_SOFTWARE=1` and `SDL_AUDIODRIVER=dummy`. A normal Linux desktop can launch without Xvfb. The reference helper requires GCC 14.2.0 instead of silently selecting another compiler. Other C17 compilers can be configured with CMake manually, with their versions/results recorded separately. No local Linux installation is assumed on the Windows development computer.

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

The helper accepts `--build-dir` for an independent generated build directory. Never share a generated tree between native and Emscripten compilers. The `meu_core_tests` target checks simulation, timing, and camera math. The `meu_scenarios` target reads a replay fixture through the production command/update loop. CTest runs both natively and, for web builds, as WebAssembly under Node.

The Windows helper identifies the compiler from object/archive metadata rather than a disposable executable. Building and running the actual tests independently verifies linking/runtime behavior.

CI builds carry the full source commit from GITHUB_SHA, verified by the native scenario and browser checks. Other checkouts use Git's revision/dirty marker. A source archive can explicitly supply CMake's MEU_REVISION value when no Git metadata is available.

## Evidence and limits

See [M0a evidence](evidence/m0a-foundation.md) for results and captures. CI artifacts contain binaries, the web distribution, reports, and images. The optimized runtime is published from main after checks pass; [GitHub Pages publishing](github-pages.md) describes the public package and live verification. The prototype is not a user-accepted release.

M0b–M0f remain open. M0a does not claim a moving interior, mech travel, saves, production graphics-context recovery, real-phone performance, or a complete supported-browser matrix.
