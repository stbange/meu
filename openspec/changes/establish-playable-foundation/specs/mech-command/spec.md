## ADDED Requirements

### Requirement: CMD-01 Physical helm control

The game SHALL permit manual robot travel commands only while the Commander operates the bridge helm, and SHALL request a controlled stop when helm control ends.

#### Scenario: Manual travel
- **WHEN** the Commander operates speed and heading controls at the helm
- **THEN** the robot travels along supported clear ground while the Commander remains in the bridge viewpoint

#### Scenario: Look without steering
- **WHEN** the Commander uses the look action at the helm without issuing a heading command
- **THEN** the viewpoint turns without changing the commanded robot heading

#### Scenario: Leave the helm
- **WHEN** the Commander releases the helm during manual travel
- **THEN** the robot brakes to a stop within its validated stopping envelope and no new manual travel command is accepted away from the helm

#### Scenario: Physical station and input ownership
- **WHEN** the Commander approaches and uses the bridge helm
- **THEN** control begins at that physical first-person position, held on-foot actions are cleared, and drive/turn actions do not walk the Commander

#### Scenario: Release drive or use the brake
- **WHEN** the Commander releases a drive action or uses the brake
- **THEN** the robot decelerates under controlled braking, and releasing the brake does not revive the previous throttle action

#### Scenario: Pause at the helm
- **WHEN** the application pauses or loses focus with travel controls held
- **THEN** pose, velocity and time freeze, helm control and held actions end, and explicit resume continues controlled braking without restoring the throttle

### Requirement: CMD-02 Validated travel and parking

The game SHALL reject travel into unsupported or obstructed space and SHALL enter the parked state only after the robot has stopped on valid support.

#### Scenario: Blocked route
- **WHEN** a requested route lacks sufficient hull clearance or supported ground within the stopping envelope
- **THEN** travel stops before the invalid area and the bridge identifies the reason

#### Scenario: Park while moving
- **WHEN** the Commander requests parking before the robot has stopped
- **THEN** the robot decelerates and does not report itself parked until it is stationary on valid support

#### Scenario: Whole-machine clearance
- **WHEN** a structure intersects a shoulder, arm, torso, roof or head within the proposed movement or braking path
- **THEN** travel is rejected even if the feet could pass the structure

#### Scenario: Turning and reverse stopping
- **WHEN** the Commander turns or reverses near a structure or support boundary
- **THEN** the swept hull, both foot areas and the complete controlled stop are checked before movement is committed

#### Scenario: Gap inside a foot area
- **WHEN** an unsupported gap lies inside either foot area even though its corners are supported
- **THEN** the robot does not enter that gap

#### Scenario: Held command after rejection
- **WHEN** a held travel action encounters an invalid stopping path
- **THEN** the robot brakes, identifies a structure or unsupported ground as the reason, and remains stopped until the old travel input is released and a fresh action is issued

#### Scenario: Invalid parking support
- **WHEN** parking is requested on invalid or unavailable support
- **THEN** the robot is not reported as parked or ready for a later save
