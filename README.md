# Mechanized Exploration Unit

A single-player, first-person exploration game about living inside a skyscraper-sized robot, navigating an abandoned alien city, and incorporating its technology into your machine.

The implementation target is **C17, SDL3, and OpenGL** on Windows and Linux, plus an **Emscripten/WebAssembly and WebGL 2** build designed for small touch screens.

## Current status

Repository setup and planning are complete. Game implementation has not started. The design and milestone proposals await review; no gameplay milestone or playtest is user-accepted. See the [status record](docs/status.md).

## Start here

- [Game design draft](docs/game-design-draft.md): player experience, scale, city, repairs, salvage, power, and progression.
- [Implementation plan](docs/implementation-plan.md): ordered steps, dependencies, scope limits, and completion gates.
- [Development on the shared computer](docs/development-setup.md): independent checkout, Codex project, tooling, builds, and save data.
- [Verification strategy](docs/verification.md): unit, generation, game, browser, performance, and manual checks.
- [First OpenSpec proposal](openspec/changes/establish-playable-foundation/proposal.md): M0 requirements, design, and unchecked tasks.

The next implementation step is **M0a: reproducible builds and a minimal running client**. There is no playable build or game-launch command yet.

## Specification tooling

Node is development tooling, not a shipped game dependency. Install this repository's pinned OpenSpec tooling and validate its proposals from this folder:

```text
npm ci --ignore-scripts
npm run spec:validate
```

OpenSpec has its own lockfile here. It does not use another game's tooling installation. Compiler, SDL, Emscripten, build, and game-test setup belong to M0a.

## Project boundaries

This is an independent repository with its own Git history, branches, instructions, dependencies, specifications, tests, assets, saves, and releases. It may share a development computer and installed tools with other projects. It has no source, runtime, or build dependency on Wandering Castle.

Commit and push completed implementation steps while recording automated verification and user acceptance separately. See [AGENTS.md](AGENTS.md).
