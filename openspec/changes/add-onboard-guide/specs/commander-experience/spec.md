## ADDED Requirements
### Requirement: HELP-01 Onboard game help
The Commander SHALL be able to open optional in-game help through a visible hovering-robot guide and keyboard on native desktop and browser mouse/touch clients. Help SHALL explain the current playable goal, controls, rooms/lift, piloting, infrastructure/repair, suit work and durable journeys using shared authored content.
#### Scenario: Explain the game before starting
- **WHEN** the Commander opens Pip before beginning exploration
- **THEN** the game and first action are explained without starting simulation or sound
#### Scenario: Browse topics on a phone
- **WHEN** help is opened in portrait or landscape on a touch client
- **THEN** topics are readable and scrollable, controls have at least 48 CSS-pixel touch targets, and help closes without pointer lock or a hardware keyboard
### Requirement: HELP-02 Guidance follows the actual journey
The guide SHALL explain the next actionable step from production state, including prerequisites, locations and safety/recovery conditions. Reading help SHALL not perform the action, grant resources or alter progression.
#### Scenario: Progress and reload
- **WHEN** repair, suit, salvage or installation progress changes or a journey is loaded
- **THEN** the next-step page reflects that state rather than a tutorial counter or stale instruction
#### Scenario: Blocked work or unsafe suit state
- **WHEN** required isolation, energy, cooling, parts or suit protection is missing
- **THEN** guidance explains the blocking condition or safe return/recovery before suggesting later work
### Requirement: HELP-03 Safe first-person help lifecycle
Help SHALL retain first-person surroundings, pause time, clear held input and release mouse capture. Closing SHALL leave play paused until explicit resume and SHALL not automatically recapture the mouse. Help SHALL preserve existing save compatibility.
#### Scenario: Read during movement or suit work
- **WHEN** help opens with movement held, at the helm or while suited
- **THEN** old movement is released and position, power, repair and exposure time stay frozen while reading
#### Scenario: Return after reading
- **WHEN** help closes or the window loses focus during reading
- **THEN** play remains paused and returning requires fresh input with no replay of an old gesture
