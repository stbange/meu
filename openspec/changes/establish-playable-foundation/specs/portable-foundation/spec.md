## ADDED Requirements

### Requirement: PORT-01 Native and browser foundation

The game SHALL run the same foundation behaviors on Windows, Linux, and the web, including a small-screen touch browser.

#### Scenario: Complete the foundation route
- **WHEN** a player launches a supported build and walks from the residence to the bridge, steers, parks, and visits engineering
- **THEN** all actions complete through the supported input path without a platform-specific gameplay omission

#### Scenario: Initial platform prototype
- **WHEN** the M0a client starts on a supported native target or touch browser
- **THEN** it renders a first-person test room, exposes basic movement/look/start/pause actions, and returns control between frames rather than blocking browser input

### Requirement: PORT-02 Touch interaction and readable layout

The game SHALL expose every required foundation action without a keyboard, mouse, hover, pointer lock, or fullscreen at 360 by 640 and 640 by 360 CSS pixels.

#### Scenario: Touch-only operation
- **WHEN** the player uses only touch to walk, look, interact, steer, stop, save, and resume
- **THEN** each action is available through readable, reachable controls without dismissing the first-person world view

#### Scenario: Resize or rotate
- **WHEN** the browser viewport changes orientation or size during a held input
- **THEN** the layout reflows without clipping required controls, held inputs cancel, and gameplay pauses until explicit resume

#### Scenario: Phone settings and safe areas
- **WHEN** the player opens settings at either required phone size with display safe-area insets
- **THEN** every option is reachable through touch, settings remain within the safe area, and the close/reset actions remain visible with targets at least 48 CSS pixels high

#### Scenario: Contextual touch controls
- **WHEN** the Commander walks from residence to the helm, drives and parks, then visits engineering using only touch
- **THEN** movement labels and station actions match the active context, required gameplay buttons are at least 48 by 48 CSS pixels, and first-person surroundings remain visible

#### Scenario: Audio before beginning
- **WHEN** the player opens settings and adjusts or mutes volume before beginning exploration
- **THEN** no audio device is started until an explicit Begin gesture, and the selected gain applies before playback

#### Scenario: Keyboard access to the same settings
- **WHEN** a desktop player opens settings with the keyboard
- **THEN** controls can be reached and adjusted, keyboard focus remains in the open dialog, reset restores defaults, and closing the dialog requires a separate explicit resume to restart play

### Requirement: PORT-03 Recoverable local saves

The game SHALL preserve basic player and robot state in versioned local saves, retain a previous complete snapshot during publication, and report saved status only after persistent storage confirms completion.

#### Scenario: Reload a confirmed save
- **WHEN** a saved game is closed and loaded in a fresh process or browser page
- **THEN** the Commander location, robot pose, and settings are restored, with travel initially stopped until the world is ready

#### Scenario: Persistence unavailable
- **WHEN** storage is unavailable or a persistent write fails
- **THEN** the game reports the failure, offers export of the current valid state, and does not report a successful save or overwrite the last confirmed snapshot

#### Scenario: Invalid latest snapshot
- **WHEN** the newest snapshot is malformed and an earlier valid snapshot exists
- **THEN** the game offers recovery from the valid snapshot and preserves the malformed data for diagnosis

#### Scenario: Unsupported save version
- **WHEN** an imported or loaded save has an unsupported schema
- **THEN** the game explains the incompatibility without modifying the running state or destroying the source save

#### Scenario: Confirm publication before displaying saved
- **WHEN** a save write has been requested but persistent publication has not completed, or publication fails or is interrupted
- **THEN** the game does not display a successful-save status before completion, keeps a complete previously confirmed snapshot recoverable, and offers export of the current valid journey after failure

#### Scenario: Save a settled journey with settings
- **WHEN** the Commander explicitly saves a parked supported robot with the lift settled
- **THEN** the snapshot preserves robot and player poses, interior/lift location, lights, visited spaces, simulation ticks and all settings, while an unfinished stop or lift ride is rejected without replacing saved generations

#### Scenario: Restore without old input or sound
- **WHEN** a valid latest journey is found on a fresh launch
- **THEN** it restores only after geometry validation, remains paused and parked with no held input or helm ownership, and waits for an explicit resume gesture before simulation or audio starts

#### Scenario: Portable export and staged import
- **WHEN** the player exports a valid journey and imports it in another supported native or web build
- **THEN** the same versioned state can be restored, malformed or unsafe candidates leave live state and local saved generations unchanged, and a successful import stays unsaved locally until an explicit Save action

#### Scenario: Recover missing or damaged latest data
- **WHEN** the latest slot is absent or damaged and the previous slot is valid
- **THEN** previous recovery is offered rather than silently applied, recovering does not rewrite the source, and a later explicit repair save retains the most recent damaged source separately

#### Scenario: Concurrent game windows
- **WHEN** two game windows publish a save to the same local journal
- **THEN** publication is serialized or explicitly rejected, generations do not partially overwrite each other, and the latest and previous complete snapshots remain distinguishable

#### Scenario: Bounded damaged data and responsive journey menu
- **WHEN** malformed stored data exceeds the supported record size or a journey operation is still pending on a phone layout
- **THEN** decoding rejects the data without an unbounded host-to-game copy, required controls remain reachable within safe areas, and gameplay/menu focus does not resume behind the pending operation

### Requirement: PORT-04 Reproducible diagnostics

The game test harness SHALL record the build/content versions, scene identity, input commands, and simulation ticks needed to reproduce a failing foundation scenario.

#### Scenario: Automated scenario failure
- **WHEN** a scripted foundation scenario fails an assertion
- **THEN** its report identifies the failed behavior and includes a replayable input trace and relevant state, with a screenshot when a rendered client is available

### Requirement: PORT-05 Reproducible build and test entry points

The project SHALL provide versioned native, headless C, and web build/test entry points using its own pinned tooling and dependency records.

#### Scenario: Clean reference checkout
- **WHEN** a developer or CI runner follows the documented reference setup from a clean MEU checkout
- **THEN** it can build the native client, compile the web client, and run the production-loop core/scenario checks without files from a sibling game

#### Scenario: Browser input verification
- **WHEN** the browser suite runs against the served diagnostic build
- **THEN** it exercises real input paths at desktop and phone viewport sizes, asserts the resulting C state through read-only diagnostics, and retains failure reproduction data

#### Scenario: Build without test diagnostics
- **WHEN** the optimized web configuration is built and launched
- **THEN** the test-only diagnostic export is absent while normal player input and rendering still operate

#### Scenario: Unavailable or mismatched tool
- **WHEN** the reference setup has no required tool or selects an unsupported native compiler version
- **THEN** the helper exits unsuccessfully with an actionable explanation instead of silently falling back to another project's tools
