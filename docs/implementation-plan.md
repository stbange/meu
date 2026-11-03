# Mechanized Exploration Unit Implementation Plan

Planning baseline: 3 November 2026. M0a is implemented; later steps remain pending. The [design draft](game-design-draft.md) describes the experience, [status](status.md) records progress/acceptance, and [M0a evidence](evidence/m0a-foundation.md) records verification and limits.

## Delivery approach

Build the risky foundations first, then one complete authored expedition, then procedural variation. Establish Windows, Linux, and phone-browser builds in M0 instead of treating the web as a later port. Each implementation step ends with relevant checks, evidence, a commit, and a push. User acceptance is recorded separately and is never inferred from a successful test or push.

The critical path is **M0 moving home → M1 complete expedition → M2 generated city → M3 campaign → M4 release preparation**. Content work can use placeholders until its interactions work. Set delivery estimates after M0 measures the platform and movement risks; the milestone names are scope boundaries, not calendar promises.

## Independent project foundation

The repository root contains the whole MEU project. Its remote is `https://github.com/stbange/meu.git`. Source, specifications, dependencies, test fixtures, saves, build outputs, asset licenses, and release automation stay independent of other games on the computer.

The planned source layout is:

```text
src/core/          simulation clock, quantities, transforms, commands, transactions
src/game/          player, mech, power, faults, salvage, crafting, suit, progression
src/world/         generation, route/support data, chunk lifecycle, mutations
src/platform/      SDL, lifecycle, input, filesystem and browser adapters
src/render/        OpenGL renderer, materials, visibility, shader variants
data/              validated content definitions and authored layouts
assets/            MEU-owned visual/audio assets and license records
web/               browser shell and touch/accessibility integration
tests/unit/        pure C tests
tests/scenarios/   deterministic command/update-loop tests
tests/client/      rendered collision, input and interaction tests
tests/web/         real WebAssembly browser tests
tests/fixtures/    reference seeds, saves, replay logs and content fixtures
tools/             build, serve, verify, capture and content tools
docs/evidence/     verification reports and focused playtest instructions
openspec/changes/  proposed behavior and implementation work
openspec/specs/    verified behavior promoted after change completion
```

M0a establishes the core/platform/render source, CMake targets, fixtures, browser shell, and tools. Other directories/systems remain planned. Keep a small number of explicit state owners instead of building a general engine framework first. Use a 60 Hz fixed tick with bounded catch-up, then add presentation interpolation as motion develops; backgrounded time does not become accumulated simulation work.

## M0 Moving home and platform proof

Owner: [establish-playable-foundation](../openspec/changes/establish-playable-foundation/proposal.md). No gameplay dependency. Use an authored test environment, one robot hull, and four connected spaces. Power consumption, active helpers, crafting, egress, generation, and campaign content are excluded.

### M0a Reproducible builds and a minimal running client

Establish C17 and CMake targets for a native client, a pure C test executable, and an Emscripten web client. Select and pin compatible SDL3, CMake, compiler, and Emscripten versions; record the setup and resolved versions. Keep dependencies inside this repository's build/download locations or explicit toolchain paths. No build command may find files through a sibling project.

Open a window, draw a simple first-person room with direct OpenGL, handle the initial audio gesture, and return control to the browser each frame. Map touch and desktop inputs to the same small action vocabulary from the outset. Create a documented web server command that serves the build over HTTP.

Add CI for Windows native, Linux native, browser compilation, core tests, and specification validation. Add the browser launch check when the minimal client exists. Prove that a clean checkout plus documented setup builds without untracked local files. This step implements the initial part of PORT-01 and the test entry points for PORT-04. It does not complete the full foundation route.

**Evidence:** reproducible configure/build commands, exact versions, CI results, one native and browser capture, and a working core-test report. Document missing reference hardware without claiming performance acceptance.

### M0b Habitable interior and convincing scale

Build the bridge, residence, engineering hall, and lift/service spine. Place them inside a roughly 240-metre hull next to an alien doorway and a small set of monumental structures. Human furniture, rails, doors, and readable machinery establish scale. Use simple retro materials and sound placeholders.

Implement first-person walking, looking, ramps, lift use, collision, and interaction. Store the aboard player's pose in robot-local coordinates and transform it exactly once. Supply both touch and keyboard/mouse access to the new actions. A motion fixture exercises aboard movement while the robot moves, even though normal manual travel will stop on helm release.

**Evidence:** VIEW-01/02 tests, lift and collision scenarios, transform tests for translation/turn/acceleration/braking, and fixed-view scale captures. Unintended standing drift must remain under 2 cm over 60 seconds in the supported motion fixture. Ejection or floor penetration fails the step regardless of the drift result.

### M0c Bridge piloting and supported travel

Implement speed, heading, brake, helm entry/release, and valid parking. Keep view direction separate from travel commands. Use simplified support/foot placement and hull clearance rather than free physical balancing. Validate the stopping envelope before advancing toward an obstruction or unsupported ground. Expose understandable rejection reasons at the bridge.

**Evidence:** CMD-01/02 scenarios through actual input paths, blocked and unsupported routes, stopping-distance boundaries, helm release, and save-ready parked state. Test that an on-foot input cannot accidentally move the robot.

### M0d Complete touch and lifecycle behavior

Finish the full on-foot and helm contexts in landscape and portrait, including 360 by 640 and 640 by 360 CSS-pixel layouts. Keep readable text and large touch targets; respect display safe areas. Station interaction retains the first-person surroundings. Pointer lock, hover, fullscreen, and a hardware keyboard are optional.

Pause on focus/background loss, viewport rotation during held input, and menu pause. Clear held actions and require explicit resume after interruption. Avoid audio playback before a gesture. Add adjustable sensitivity and independent camera-motion options.

**Evidence:** VIEW-03 and PORT-02 running-client/browser scenarios, a touch-only route from residence to helm and engineering, resize/rotation tests, and real-phone interaction checks. Tests must include cancellation and context changes rather than only direct invocation of game commands.

### M0e Durable local saves

Persist schema/version, snapshot identity, robot pose, player-local pose, interior location, parked state, and settings. Validate staged data before live replacement. Retain a previous complete generation and resume travel stopped after geometry is ready.

Implement desktop publication and browser persistence with completion/error reporting. Add export/import, storage-unavailable handling, corrupted-latest recovery, and incompatible-schema rejection. A write into browser memory is not a completed save.

**Evidence:** PORT-03 unit, integration, and fresh-page reload tests; injected interrupted and failed writes; source-save preservation on rejection; consistent settings and location after restoration. Show saved status only after the platform persistence layer confirms completion.

### M0f Foundation verification and focused playtest

Integrate the scenarios, replay logs, and failure captures described in [verification](verification.md). Select and record reference Windows/Linux systems and real Android/iPhone devices. Measure a defined scene for 60 fps desktop, 30 fps phone, and under 256 MiB WebAssembly linear memory; track graphics allocations separately. These remain provisional budgets until measured.

**User acceptance:** the robot feels enormous from the player's eyes; steering and walking aboard feel comfortable; useful rooms are easy to reach; phone controls are readable and practical. Record acceptance or specific revisions without checking off an unreviewed playtest. Close M0 only with its required automated evidence and recorded user review.

## M1 One complete authored expedition

Depends on the verified M0 foundation and its scale/comfort review. Introduce each system against a fixed route before generating it. Create focused OpenSpec changes with the proposed capability names below before implementing their behavior.

### M1a Power and infrastructure

Proposed capabilities: `mech-power` and `alien-grid`. Add stored energy, fixed-tick consumption, solar exposure, deployable parked collectors, power priorities, heat generation/cooling, and one grid socket with an interpretable surge pattern. Keep an isolated life-support reserve. Show projected energy use, uncertainty, and a return margin. Closed and paused games do not advance these systems.

**Evidence:** power conservation/accounting tests, saturation and invalid-state checks, deterministic daylight sampling, safe connection/disconnection, overload isolation, and a scenario that obtains useful solar charge while the player performs an interior task. Reject pacing that forces repeated empty waiting.

### M1b Fault diagnosis and one repair assistant

Proposed capability: `maintenance`. Add a small bus/component graph with one deliberately caused fault, inspectable symptoms, isolation/bypass, part replacement, and a helper job queue. The helper can perform routine known work when parts and access exist, and reports blocked jobs. It cannot understand unfamiliar technology for the player.

**Evidence:** diagnostic state transitions, blocked jobs, cancellation, stock reservation, save/reload during a job, and recovery without losing essential doors or the only route to the bridge. The playtest must distinguish an interesting diagnosis from repetitive upkeep.

### M1c Salvage and technology integration

Proposed capabilities: `salvage` and `technology-integration`. Add one manipulator recovery point, a salvage cradle, a controlled test bench, fabrication stock, adapters, and installation sockets. Recover a grid stabilizer that changes the usable route. Display available/required materials and power/cooling tradeoffs before committing a recipe or installation.

**Evidence:** atomic costs and outputs, capacity limits, canceled/duplicate requests, install/remove/save round trips, and a route that is observably different with the new component. Basic diagnostic tools and common repair material must be obtainable without the recovered component.

### M1d Restricted suit excursion

Proposed capability: `suit-egress`. Add one reachable deployment airlock, protected service recess, measured environmental limits, suit status, and a clearly marked return route. Require parked state, landing clearance, and a viable environmental envelope. The robot remains parked throughout the excursion.

**Evidence:** every blocked exit reason, exposure thresholds and warnings, removing shelter power, bypass preservation, save/reload on both sides of the airlock, and a safe return. Include the explicit checkpoint recovery path for catastrophic mistakes and test its treatment of unsecured salvage.

### M1e Complete expedition and evaluate the loop

Build the sample tower expedition: scout, select a route, approach, isolate the grid fault, establish shelter, enter the service conduit, bypass the field supply, recover the coupler, return, install the stabilizer, and demonstrate new access. Add only the art and sound required to communicate that journey.

**Evidence:** one complete real-client scenario plus variants for low energy, missing parts, unsafe egress, interrupted repair, reload, and recovery. A focused user playtest checks whether the 20–40 minute loop is understandable, repairs feel purposeful, and bringing home technology changes play. Tune the authored loop before M2 expands it.

## M2 A generated small city

Depends on the useful authored interactions in M1. Proposed capabilities: `world-generation`, `world-streaming`, and `world-persistence`. Begin with three district families and a bounded city.

### M2a Stable seeded identity and world records

Define explicit seed, generator/content versions, stable object IDs, and deterministic generation arithmetic. Store generated baselines separately from visited-world mutations. Define compatibility and migration policy before changing generation algorithms. Cross-platform reference fixtures compare stable generated records, not arbitrary floating-point frame hashes.

### M2b Traversable landscape and streaming

Construct the rift, major elevation bands, district graph, recovery locations, supported mech routes, and parking before adding buildings or decoration. Use bounded residency, advance requests, and per-frame attachment budgets. The robot must stop safely before entering unloaded or collision-unready space. Keep the player's occupied sector and access route resident. Save dirty changes before eviction.

Use logical world coordinates with local render/physics rebasing. Shift relevant projections together, retain IDs and save locations, and test interior/exterior alignment through long travel. The initial web implementation budgets work across frames instead of requiring worker threads.

### M2c Architecture and expedition composition

Implement Heliostat Courts, Suspended Galleries, and one enclosed district family using authored grammars. Compose landmark silhouettes, supported passages, service-scale spaces, power circuits, faults, recoverable technologies, and clues. Variations must change route or problem solving rather than merely decoration.

### M2d Validation and replayability review

Construct at least one known viable progression route and check required clearances, stock, power recovery, technology prerequisites, and suit returns. Bound retries and fall back to a known valid layout when generation fails. Keep failing seeds as fixtures. Give nearby sites a repetition budget and maintain coherent physical rules across a campaign.

**Evidence:** an initial 1,000-seed nightly sweep, fixed regression seeds, native/web identity fixtures, multi-district save round trips, interrupted publication, and streaming soaks. Scale seed coverage as runtime cost is measured. User review compares several unseen seeds for novelty, legibility, and viable expedition choices. Seed count alone does not establish replayability or guarantee arbitrary player choices remain recoverable.

## M3 A complete exploration arc

Depends on viable, persistent procedural districts. Proposed capabilities: `exploration-progression`, `mech-home`, and `campaign-discovery`.

### M3a Distinct technologies and route alternatives

Add a small set of new verbs: structural scanning, load-distribution traversal, a new suit envelope, improved maintenance reach, and an energy/heat conversion option. Give each a visible tradeoff and a distinct use. Validate the progression graph against circular prerequisites. Preserve more than one sensible expedition choice where the district structure allows it.

### M3b Restore and inhabit the robot

Open the workshop, observation gallery, helper bay, and other selected interior sectors through meaningful repairs. Add artifact displays, quieter or improved machinery, helper upgrades, and living-space customization using authored sockets. Preserve recognizable rooms and inventory. Avoid block construction and compulsory hunger/sleep maintenance.

### M3c City history and final archive

Expand to five or six district families and compose a consistent authored history through physical evidence and machine records. Assemble the Meridian Archive route from previously learned interactions. Implement a comprehensible restoration choice with visible consequences and continued exploration afterward. No present human crew or surviving alien population is introduced.

**Evidence:** full progression/campaign scenarios, at least two equipment/route solutions for selected sites, ending and post-ending saves, novelty reviews, and a user playtest of discovery pacing and attachment to the home. Introduce no additional major simulation system solely to fill content volume.

## M4 Release preparation

Depends on an accepted complete exploration arc. Define the actual supported OS, browser, graphics, and reference-device matrix using measured evidence.

- Produce repeatable Windows/Linux packages and a versioned web distribution with a small initial download, asset checksums, and explicit cache/version handling.
- Test supported Chromium/Firefox/WebKit desktop configurations, actual Android Chrome and iPhone Safari, low-memory behavior, background/resume, audio gestures, context loss/restoration, storage quota failure, and save export/import.
- Complete touch remapping, readable text scaling, captions/visual alarms, motion settings, color-independent warnings, and accessible menu operation.
- Run long travel and repair/crafting soaks, generator regression suites, save migration/recovery fixtures, relevant sanitizers, and fuzz tests of untrusted save/content decoders.
- Record asset origins/licenses, distribution notices, controls, save locations, recovery instructions, and known limitations. Game code and asset licensing remain an explicit project decision before release.
- Prepare release notes and deployment instructions. Publishing a playable web release or enabling hosting is a separate release action, not implied by pushing development documentation.

**User acceptance:** release candidates are reviewed on the supported input/device types. Tag a release only after its required checks and release decision are recorded.

## Definition of a completed implementation step

The intended behavior and failure cases are in the owning OpenSpec change. The implementation and relevant tests pass on the affected targets. Evidence names the tested commit, versions, scenes/seeds, commands, and material limitations. Documentation and status reflect the actual result. Commit and push the step to its working branch, even if product acceptance is pending.

Only mark a user acceptance item complete after explicit user feedback. If a playtest changes the design, update the owning requirements and plan before implementing the revision. Keep implementation completion, automated verification, and user acceptance as separate fields in the [status record](status.md).

## First implementation handoff

M0a is implemented on `codex/m0a-foundation`. Next, read its evidence and the foundation proposal/design/scenarios, then implement M0b's four connected spaces and scale scene. Preserve the working native/web paths and add the relevant collision, lift, and coordinate tests. Keep user acceptance pending until explicitly reviewed.
