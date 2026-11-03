## ADDED Requirements

### Requirement: VIEW-01 Exclusive first-person habitation

The game SHALL present movement, piloting, and interactions from the sole human Commander's first-person viewpoint inside the same connected robot.

#### Scenario: Walk between home spaces
- **WHEN** the Commander travels between the bridge, residence, engineering, and service spine
- **THEN** each space is reachable in first person without a detached camera or a required human crew

#### Scenario: Select a lift destination
- **WHEN** the Commander is fully inside the stopped service lift and selects bridge, residence, or engineering using keyboard or touch
- **THEN** the lift carries the Commander to that deck without changing viewpoint, and the destination is displayed

#### Scenario: Use a home object
- **WHEN** the Commander faces an unobstructed home object within reach and uses it
- **THEN** a dimensions plaque opens readable unit dimensions, a reading lamp changes the residence light, or an inspection console changes the engineering light

#### Scenario: Boarding at the cab edge
- **WHEN** the Commander stands too close to the lift's edge to depart safely
- **THEN** destination controls are unavailable and the display asks the Commander to step into the centre

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

#### Scenario: Lift safety and pause
- **WHEN** the car is moving, the Commander tries to leave or retarget it, or the application pauses mid-ride
- **THEN** gates prevent leaving into the shaft, a new destination is rejected until arrival, and pause freezes both rider and car until explicit resume

#### Scenario: Empty lift landing
- **WHEN** the car is absent from a landing
- **THEN** a closed landing gate prevents entering the shaft and the Commander can summon the car from the landing's call panel

#### Scenario: Human-scale collision and ramps
- **WHEN** the Commander walks against furniture, structure, windows or a catwalk edge, or ascends and descends the engineering ramp
- **THEN** solid objects and unsupported edges block passage and supported ramp movement preserves footing without a height jump

#### Scenario: Standing motion fixture
- **WHEN** the Commander stands still for 60 seconds while the supported fixture translates, turns, accelerates and brakes the robot
- **THEN** unintended robot-local standing drift is less than 2 cm, with no ejection or floor penetration

### Requirement: VIEW-03 Pause and input cleanup

The game SHALL pause single-player simulation on menu pause or focus/background loss, cancel held gameplay actions, and require explicit resume after lifecycle interruption.

#### Scenario: Background during steering
- **WHEN** the application loses focus or is backgrounded while a travel or look action is held
- **THEN** simulation pauses, the held action is cleared, and resuming does not continue that input without a new action

#### Scenario: Paused time
- **WHEN** a paused application remains inactive and is then resumed
- **THEN** simulation has not advanced through the inactive period and no accumulated movement is applied

#### Scenario: Open and close settings during travel
- **WHEN** the Commander opens settings while steering, walking, or riding the lift
- **THEN** the simulation freezes, held input clears, helm control is released with a parking request, and closing settings leaves the game paused until explicit resume

#### Scenario: Old fingers after a context transition
- **WHEN** a held gesture spans a helm, lift, station-panel, pause, or settings transition
- **THEN** it cannot command movement in the new context, and fresh input remains usable even while the old fingers are still touching the screen

#### Scenario: Cancellation and overlapping sources
- **WHEN** a held pointer is cancelled or loses capture while another pointer or key holds the same action
- **THEN** only the cancelled source is removed, and releasing the final owner stops that action without sticking input

#### Scenario: Independent camera comfort options
- **WHEN** the Commander changes sensitivity, vertical drag inversion, walking bob, or machine sway in settings
- **THEN** the bounded setting applies immediately for the session, both motion effects can be independently disabled and default off, and cosmetic camera changes do not alter footing, player position, or robot heading
