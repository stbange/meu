# Mechanized Exploration Unit

A single-player, first-person exploration game about living inside a skyscraper-sized robot, navigating an abandoned alien city, and incorporating its technology into your machine.

The implementation target is **C17, SDL3, and OpenGL** on Windows and Linux, plus an **Emscripten/WebAssembly and WebGL 2** build designed for small touch screens.

## Current status

M1a–M1e now connect a complete authored tower expedition: pilot and park, isolate a pulsing grid, diagnose a fault, assign a repair helper, recover stock, enter a sheltered service conduit in a suit, fit a bypass, recover and adapt its coupler, then power a previously unsupported west route. All play remains first person. See [the M1 route](docs/m1-expedition.md), [status](docs/status.md) and [M1e evidence](docs/evidence/m1e-expedition.md).

The implementation is on `codex/m1-authored-expedition`. At the user's request, this branch is now configured to publish to GitHub Pages after all checks, followed by live verification; see [publication status](docs/evidence/working-branch-pages.md). Human reviews were deferred until M1; M0f device/performance work and combined M0/M1 acceptance remain pending. Windows/Linux and debug/optimized web checks pass in the recorded CI run.

Version 0.1.1 added click-captured mouse look and removed overlapping floor/wall surfaces; see the [playtest correction record](docs/evidence/mouse-surfaces.md).

Version 0.1.2 adds **Pip**, a hovering onboard guide. Click/tap Help or press H/F3 for game explanations and a next step based on your journey. Help pauses play; close it and resume when ready. See [Pip evidence](docs/evidence/onboard-guide.md) for verification and current builds.

## Start here

- [Game design draft](docs/game-design-draft.md): player experience, scale, city, repairs, salvage, power, and progression.
- [Implementation plan](docs/implementation-plan.md): ordered steps, dependencies, scope limits, and completion gates.
- [Development on the shared computer](docs/development-setup.md): independent checkout, Codex project, tooling, builds, and save data.
- [Verification strategy](docs/verification.md): unit, generation, game, browser, performance, and manual checks.
- [Build and test](docs/build-and-test.md): pinned tools, launch commands, controls, and checks.
- [First OpenSpec proposal](openspec/changes/establish-playable-foundation/proposal.md): M0 requirements, design, and unchecked tasks.

The next step is remaining platform/device verification and the deferred combined M0/M1 playtest; M2 generation has not started.

## Run the prototype

Play the [MEU web prototype on GitHub Pages](https://stbange.github.io/meu/). See [publishing and live verification](docs/github-pages.md).

On Windows, from this checkout:

~~~text
npm ci --ignore-scripts
npm run bootstrap
npm run dev:native
~~~

For the mobile web build:

~~~text
npm run bootstrap:web
npm run build:web
npm run serve:web
~~~

Open `http://127.0.0.1:4174`. Begin exploration, click the 3D view to capture the mouse (Esc releases and pauses), then use WASD and captured mouse/arrows, or the movement buttons and look pad. Approach the central bridge console and use E or the prompt to take the helm. W/S drives, A/D turns, Space brakes, P parks and E releases the helm. Looking does not steer; releasing drive brakes. Touch controls adapt to piloting, with Brake, Park and Release helm buttons. The lift is behind the starting position: walk backward into its centre, select 1/2/3 (or tap a deck), then walk forward out at your destination. Use E or the contextual button at the dimensions plaque, reading lamp and inspection console. Open Settings or press F1 to adjust controls, camera movement and sound; both camera effects start off. Close settings, then choose Resume exploration (Enter on desktop). Settings are kept when you Save journey: pause, open Saved journeys (F2 on desktop), and save after the unit is parked and the lift has finished. A saved journey loads paused on the next launch. Recover previous keeps a damaged source, and Export/Import moves a .meu backup between native and web builds. Import changes the running journey; Save here to keep it locally. Focus loss, backgrounding, pause, or resizing stops simulation and clears old input until explicit resume. See [build and test](docs/build-and-test.md) for Linux and automated checks.

## Specification tooling

Node is development tooling, not a shipped game dependency. Install this repository's pinned OpenSpec tooling and validate its proposals from this folder:

```text
npm ci --ignore-scripts
npm run spec:validate
```

OpenSpec and Playwright use this repository's own lockfile. Compiler, SDL, Emscripten, build, and test setup are documented in [build and test](docs/build-and-test.md).

## Project boundaries

This is an independent repository with its own Git history, branches, instructions, dependencies, specifications, tests, assets, saves, and releases. It may share a development computer and installed tools with other projects. It has no source, runtime, or build dependency on Wandering Castle.

Commit and push completed implementation steps while recording automated verification and user acceptance separately. See [AGENTS.md](AGENTS.md).
