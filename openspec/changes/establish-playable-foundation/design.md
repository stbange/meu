## Scope

Implement M0 as described in the [game design draft](../../../docs/game-design-draft.md) and [implementation plan](../../../docs/implementation-plan.md). M0a–M0d implement the initial native/web platform, connected interior, supported travel, session settings and lifecycle behavior. Durable saves and integrated real-device acceptance remain pending. See [M0a evidence](../../../docs/evidence/m0a-foundation.md) and [build instructions](../../../docs/build-and-test.md).

## Platform and state boundaries

Use C17 and CMake with pinned SDL3 and Emscripten versions selected during implementation. SDL handles windows, input, audio, and platform lifecycle; OpenGL draws the world. Keep one renderer feature set compatible with WebGL 2, using desktop OpenGL 3.3 and GLSL variants where required. SDL's GPU abstraction is not a substitute for the requested OpenGL renderer.

Separate pure simulation/state, platform/input, rendering, persistence, and a test driver. Commands address the same simulation on every platform. Use a fixed tick and a returning frame callback suitable for browser scheduling. The first browser build uses no worker threads. Budget any work across frames; do not block the browser main loop.

## Movement and scale

Keep a metre-based scale model and a character pose relative to the robot while aboard. Transform views and exterior geometry consistently, applying robot movement exactly once. Start with simple hull, floor, stair-ramp, and doorway collision. The interior floor remains stable while cosmetic machinery suggests walking. An automated motion fixture can drive the robot while a character walks aboard even though normal helm release stops manual travel.

Use supported-foot or simplified footprint travel on the authored test surface. Predict clearance and stopping distance before committing travel. The first build does not simulate a freely balancing giant rigid body. Validate that parking, collision, the visible hull, and interior apertures agree. Record the coordinate convention so later world rebasing does not change logical positions.

### M0c supported travel implementation

The authored exterior uses one set of coloured boxes for rendering, solid structure and flat support rectangles. The robot travels on the y=0 support plane. Both 23 by 48 m foot rectangles must be fully supported, including their interiors; a partition at support boundaries tests the union exactly, rather than sampling corners. Conservative oriented hull bands cover the feet, legs, shoulders/arms, torso, forward deck and 240 m head. They include all visible interior/hull geometry and can reject a narrow route earlier than a detailed hull mesh would.

World pose uses double-precision x/z metres and yaw radians. Robot-local forward is -Z; positive yaw turns right. Interior collision and Commander position remain local. The existing renderer applies the inverse world pose to exterior geometry once, with the same interpolation as the interior/lift. M0c uses continuous footprint travel, with no free balancing or gait-induced camera motion.

Provisional controls use 12 m/s maximum speed, 3 m/s² acceleration, 4 m/s² braking, 0.10 rad/s maximum turn rate and 0.25 rad/s² turn acceleration/braking. Each fixed tick proposes speed and heading changes, then validates both that swept move and the complete braking trajectory. The prediction integrates at bounded 0.05 s intervals. Each interval inflates the hull/feet by translation plus 80 m times angular displacement, enclosing intermediate positions, and retains 2 m horizontal/overhead clearance. A rejected proposal follows the previously reserved braking path. Invalid fixture/state geometry fails closed and cannot report valid parking.

Drive release brakes. Brake clears the old throttle; Park clears travel inputs and requests a stationary supported state. Releasing the helm or pausing relinquishes control, clears held actions and requests parking. Pause first freezes all simulation; braking proceeds after explicit resume. Only a fresh station acquisition and travel input can command further movement. A rejected held command latches the safety stop until released; the bridge displays the reason. Saves themselves remain M0e.

## Input and small screens

Map mouse/keyboard, touch, and later controllers to actions and contexts: on foot, helm, station panel, and pause. Looking around at the helm must not steer. Release or context loss cancels held actions. Use CSS-pixel-aware touch sizing and safe areas; never infer touch layout from framebuffer resolution alone.

Keep a first-person viewport visible behind station interactions. Test 360 by 640 and 640 by 360 CSS-pixel layouts, resize/rotation, focus loss, and explicit resume. Browser audio begins after the initial user gesture. Touch play requires neither pointer lock nor fullscreen.

### M0d settings and context ownership

The pure C simulation owns the foot, helm, station panel, pause and settings contexts, plus an input generation. Context-changing commands clear held actions and advance that generation. The platform synchronously reports changes to the DOM adapter; it discards old key/pointer owners before releasing capture. Look has a single pointer owner, and multiple sources of the same action are reference-counted by ownership rather than releasing each other. Brake removes prior drive owners while retaining the brake gesture; Park, helm changes, lift destination selection and panels quarantine the previous gestures. New gameplay input requires a fresh press.

Settings contain bounded sensitivity (0.25–3), inverted vertical pointer drag, separate walking bob and machine sway, and bounded volume (0–1). Both camera effects default off. Their small cosmetic offsets affect only the rendered eye, never collision, player-local coordinates, robot heading or footing; there is no camera roll. Preferences remain in memory until M0e implements durable snapshots.

The web uses semantic DOM buttons/ranges/checkboxes over the first-person view, a scrollable settings body, fixed dialog actions and CSS safe-area insets. Native F1 opens the same C settings with keyboard selection. Settings pause, release the helm and request parking; closing the dialog stays paused. Only explicit Begin/Resume initializes or resumes audio, after applying volume.

The browser adapter owns all web keyboard input; SDL keyboard binding is disabled there so HTML range/checkbox/button defaults and menu focus remain functional. Capture cancellation, focus/background/freeze events and CSS viewport changes clear gameplay input. Canvas drawing resolution follows CSS size and a capped pixel ratio. Resume resets the frame timer and simulation accumulator, and focus return never resumes automatically.

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
