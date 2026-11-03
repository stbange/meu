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

### Requirement: CMD-02 Validated travel and parking

The game SHALL reject travel into unsupported or obstructed space and SHALL enter the parked state only after the robot has stopped on valid support.

#### Scenario: Blocked route
- **WHEN** a requested route lacks sufficient hull clearance or supported ground within the stopping envelope
- **THEN** travel stops before the invalid area and the bridge identifies the reason

#### Scenario: Park while moving
- **WHEN** the Commander requests parking before the robot has stopped
- **THEN** the robot decelerates and does not report itself parked until it is stationary on valid support
