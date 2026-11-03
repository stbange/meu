## ADDED Requirements

### Requirement: VIEW-01 Exclusive first-person habitation

The game SHALL present movement, piloting, and interactions from the sole human Commander's first-person viewpoint inside the same connected robot.

#### Scenario: Walk between home spaces
- **WHEN** the Commander travels between the bridge, residence, engineering, and service spine
- **THEN** each space is reachable in first person without a detached camera or a required human crew

#### Scenario: Use the bridge
- **WHEN** the Commander takes or releases helm control
- **THEN** the viewpoint remains at the Commander's physical position and exterior architecture remains visible through the bridge

### Requirement: VIEW-02 Stable interior movement

The game SHALL preserve the Commander's intended position and footing relative to the robot through supported travel and interior movement, without mandatory camera roll or gait shake.

#### Scenario: Robot motion
- **WHEN** the robot translates, turns, accelerates, or brakes within supported limits with the Commander aboard
- **THEN** the Commander is not ejected, carried through floors, or unintentionally moved within the cabin

#### Scenario: Interior traversal during motion
- **WHEN** the motion fixture drives supported robot travel while the Commander walks, uses a ramp, or rides the lift
- **THEN** locomotion and collision remain stable and only the intended local movement changes the Commander's cabin position

### Requirement: VIEW-03 Pause and input cleanup

The game SHALL pause single-player simulation on menu pause or focus/background loss, cancel held gameplay actions, and require explicit resume after lifecycle interruption.

#### Scenario: Background during steering
- **WHEN** the application loses focus or is backgrounded while a travel or look action is held
- **THEN** simulation pauses, the held action is cleared, and resuming does not continue that input without a new action

#### Scenario: Paused time
- **WHEN** a paused application remains inactive and is then resumed
- **THEN** simulation has not advanced through the inactive period and no accumulated movement is applied
