## Why

The game's central promise depends on feeling tiny inside a huge moving home while steering it through even larger architecture. Prove that experience and the phone controls before building salvage, procedural districts, or a campaign.

Status: M0 in progress. M0a supplies the native/web test room, pinned build setup, and initial checks. M0b–M0f and user acceptance remain pending. See [M0a evidence](../../../docs/evidence/m0a-foundation.md).

## What Changes

- Establish a separate C game with SDL and OpenGL, native Windows/Linux builds, and an Emscripten web build.
- Add four connected interior spaces: bridge, residence, engineering, and a lift/service spine.
- Place a roughly 240-metre robot in a small authored alien scale scene with a colossal doorway and validated walking surface.
- Add exclusive first-person movement, interaction, physical bridge steering, controlled stopping, and parking.
- Add touch controls, small-screen layouts, pause/resume, and a basic versioned save.
- Establish C unit tests, repeatable running-game scenarios, and browser smoke/input/persistence checks.

The proposed defaults are C17, SDL3, OpenGL 3.3 core on desktop with a WebGL 2 compatible renderer, a single-threaded browser build, fixed simulation ticks, and robot-local interior coordinates. The design draft contains the rationale and later milestones.

## Capabilities

### New Capabilities

- `commander-experience`: First-person habitation, readable interactions, stable movement, and pause behavior.
- `mech-command`: Bridge-only manual travel, controlled stops, and validated parking.
- `portable-foundation`: Native/web operation, touch input, basic durable saves, and reproducible diagnostics.

### Modified Capabilities

None. This foundation introduces MEU's initial capabilities.

## Impact

Creates the M0 foundation at this repository's root. No milestone dependency. Excludes procedural generation, salvage/crafting, power economics, faults/repair jobs, active helper AI, exterior egress, unrestricted construction, combat, and campaign content. M0's robot moves through a fixed test scene; its energy budget is not yet gameplay. The [implementation plan](../../../docs/implementation-plan.md) divides M0 into steps M0a–M0f and defines the dependent milestones.

Automated checks establish correctness. A separate user review decides whether scale, comfort, and phone usability support proceeding to M1. Commits and pushes do not imply acceptance.
