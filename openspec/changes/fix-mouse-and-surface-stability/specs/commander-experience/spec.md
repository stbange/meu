## ADDED Requirements
### Requirement: INPUT-01 Click-captured first-person mouse look
A mouse click in the active gameplay view SHALL capture the pointer on native desktop and browsers supporting pointer lock, providing continuous relative look after button release and at viewport edges. Pointer capture SHALL not steer the robot. Touch SHALL retain its drag controls without requiring a lock.
#### Scenario: Capture and free relative look
- **WHEN** the Commander clicks the gameplay view with a mouse and moves it without holding a button
- **THEN** the pointer is hidden/captured and relative movement changes the first-person view using current sensitivity/inversion settings
#### Scenario: Release and fresh capture
- **WHEN** Escape, pause, focus/background loss or a settings/journey/station panel interrupts captured play
- **THEN** the pointer is released and old held input is cleared; returning does not automatically recapture without a fresh gameplay click
#### Scenario: Unsupported, denied or cancelled lock
- **WHEN** a browser refuses capture or a request finishes after a menu interrupts it
- **THEN** the game stays operable with drag/touch/keyboard controls and no late capture takes ownership over the menu
### Requirement: VIEW-04 Stable shared visible surfaces
Overlapping opaque structures SHALL render one visible owner for a shared surface rather than depth-fighting copies, and decorative ground markings SHALL remain visually stable as the viewpoint moves. Rendering fixes SHALL preserve supported travel, interior collision and valid saves.
#### Scenario: Causeway and powered bridge
- **WHEN** the view crosses overlapping floor slabs or the installed bridge joins the causeway
- **THEN** coincident floor regions have one rendered surface and the same supported walking/travel footprint
#### Scenario: Walls and markings
- **WHEN** room/hull boxes overlap or distant route paint lies on the floor
- **THEN** buried/shared solid faces are omitted and the markings have stable depth ownership
