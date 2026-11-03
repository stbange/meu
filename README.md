# Mechanized Exploration Unit

A single-player, first-person exploration game about living inside a skyscraper-sized robot, navigating an abandoned alien city, and incorporating its technology into your machine.

The implementation target is **C17, SDL3, and OpenGL** on Windows and Linux, plus an **Emscripten/WebAssembly and WebGL 2** build designed for small touch screens.

## Current status

M0b adds the bridge, living quarters, engineering hall and an 84-metre service lift inside a 240-metre robot. Furniture, reactors, visible hull and a monumental alien doorway establish scale. Walking, ramps, collision, contextual interactions and lift travel work with desktop and touch controls. No gameplay milestone or playtest is user-accepted. See [status](docs/status.md) and [M0b evidence](docs/evidence/m0b-interior.md).

## Start here

- [Game design draft](docs/game-design-draft.md): player experience, scale, city, repairs, salvage, power, and progression.
- [Implementation plan](docs/implementation-plan.md): ordered steps, dependencies, scope limits, and completion gates.
- [Development on the shared computer](docs/development-setup.md): independent checkout, Codex project, tooling, builds, and save data.
- [Verification strategy](docs/verification.md): unit, generation, game, browser, performance, and manual checks.
- [Build and test](docs/build-and-test.md): pinned tools, launch commands, controls, and checks.
- [First OpenSpec proposal](openspec/changes/establish-playable-foundation/proposal.md): M0 requirements, design, and unchecked tasks.

The next step is **M0c: bridge piloting and supported travel**. The complete M0 experience remains in development.

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

Open `http://127.0.0.1:4174`. Begin exploration, then use WASD and mouse drag/arrows, or the movement buttons and look pad. The lift is behind the starting position: walk backward into its centre, select 1/2/3 (or tap a deck), then walk forward out at your destination. Use E or the contextual button at the dimensions plaque, reading lamp and inspection console. Focus loss, pause, or resizing stops simulation until explicit resume. See [build and test](docs/build-and-test.md) for Linux and automated checks.

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
