## ADDED Requirements

### Requirement: PORT-01 Native and browser foundation

The game SHALL run the same foundation behaviors on Windows, Linux, and the web, including a small-screen touch browser.

#### Scenario: Complete the foundation route
- **WHEN** a player launches a supported build and walks from the residence to the bridge, steers, parks, and visits engineering
- **THEN** all actions complete through the supported input path without a platform-specific gameplay omission

### Requirement: PORT-02 Touch interaction and readable layout

The game SHALL expose every required foundation action without a keyboard, mouse, hover, pointer lock, or fullscreen at 360 by 640 and 640 by 360 CSS pixels.

#### Scenario: Touch-only operation
- **WHEN** the player uses only touch to walk, look, interact, steer, stop, save, and resume
- **THEN** each action is available through readable, reachable controls without dismissing the first-person world view

#### Scenario: Resize or rotate
- **WHEN** the browser viewport changes orientation or size during a held input
- **THEN** the layout reflows without clipping required controls, held inputs cancel, and gameplay pauses until explicit resume

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

### Requirement: PORT-04 Reproducible diagnostics

The game test harness SHALL record the build/content versions, scene identity, input commands, and simulation ticks needed to reproduce a failing foundation scenario.

#### Scenario: Automated scenario failure
- **WHEN** a scripted foundation scenario fails an assertion
- **THEN** its report identifies the failed behavior and includes a replayable input trace and relevant state, with a screenshot when a rendered client is available
