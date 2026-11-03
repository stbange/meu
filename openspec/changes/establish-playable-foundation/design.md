## Scope

Implement the proposed M0 described in the [game design draft](../../../docs/game-design-draft.md) and steps M0a–M0f of the [implementation plan](../../../docs/implementation-plan.md). This artifact is a technical proposal; no selected approach has been validated by a running prototype.

## Platform and state boundaries

Use C17 and CMake with pinned SDL3 and Emscripten versions selected during implementation. SDL handles windows, input, audio, and platform lifecycle; OpenGL draws the world. Keep one renderer feature set compatible with WebGL 2, using desktop OpenGL 3.3 and GLSL variants where required. SDL's GPU abstraction is not a substitute for the requested OpenGL renderer.

Separate pure simulation/state, platform/input, rendering, persistence, and a test driver. Commands address the same simulation on every platform. Use a fixed tick and a returning frame callback suitable for browser scheduling. The first browser build uses no worker threads. Budget any work across frames; do not block the browser main loop.

## Movement and scale

Keep a metre-based scale model and a character pose relative to the robot while aboard. Transform views and exterior geometry consistently, applying robot movement exactly once. Start with simple hull, floor, stair-ramp, and doorway collision. The interior floor remains stable while cosmetic machinery suggests walking. An automated motion fixture can drive the robot while a character walks aboard even though normal helm release stops manual travel.

Use supported-foot or simplified footprint travel on the authored test surface. Predict clearance and stopping distance before committing travel. The first build does not simulate a freely balancing giant rigid body. Validate that parking, collision, the visible hull, and interior apertures agree. Record the coordinate convention so later world rebasing does not change logical positions.

## Input and small screens

Map mouse/keyboard, touch, and later controllers to actions and contexts: on foot, helm, station panel, and pause. Looking around at the helm must not steer. Release or context loss cancels held actions. Use CSS-pixel-aware touch sizing and safe areas; never infer touch layout from framebuffer resolution alone.

Keep a first-person viewport visible behind station interactions. Test 360 by 640 and 640 by 360 CSS-pixel layouts, resize/rotation, focus loss, and explicit resume. Browser audio begins after the initial user gesture. Touch play requires neither pointer lock nor fullscreen.

## Basic persistence

Store a schema version, snapshot generation, robot world pose, player-local pose, current interior location, parked state, and settings. Load into a staging state and validate before replacing the live state. Preserve a prior complete snapshot during publication. Reject unsupported or malformed data without deleting it or changing the running state. No legacy migration is needed until a later schema exists.

Desktop storage uses platform-appropriate user data locations and recoverable publication. Browser storage uses IndexedDB-backed persistence with an explicit completion/error signal, not a successful in-memory write alone. Coordinate asynchronous synchronization with the game lifecycle, retain a previous confirmed generation, and expose export/import. Resume a loaded journey parked after collision and required geometry are ready.

## Verification and budgets

Use a C test executable for transforms, travel validation, input state transitions, and save decoding. Add a headless command runner and rendered tests that exercise genuine movement and interaction paths. A test-only state bridge supports browser assertions; shipping builds omit mutation/debug endpoints. Use Playwright to load the WebAssembly build, perform touch and desktop interactions, and verify a save across a fresh page load.

Motion scenarios cover translation, turning, acceleration, braking, and lift transitions. Bound unintended standing drift and test collision/footing separately from image comparisons. Save tests include failed persistence, corrupt newest generation, recovery from the previous valid generation, and resume without unintended travel.

Record selected Windows/Linux machines and real Android/iPhone reference devices. Initial targets are 60 fps desktop, 30 fps phones, and under 256 MiB WebAssembly linear memory during M0; graphics allocations are accounted separately. Report measured scenes and devices. Browser automation or software rendering alone cannot establish phone performance or perceived comfort.

## Risks and fallback decisions

- If the moving interior drifts, retain the robot-local simulation and correct transform ownership before adding content.
- If an interface obscures the world on phones, simplify station tasks and controls before reducing text size.
- If the robot feels small, revise foreground references, doorway proportions, motion, and sound; an external camera does not satisfy the requirement.
- If full exterior detail exceeds the phone budget, reduce visible geometry and materials while preserving collision, scale, and player actions.
