## ADDED Requirements
### Requirement: MAINT-01 Diagnosable fault and routine helper work
A common distribution fault SHALL have inspectable symptoms, a specific diagnosis, safe isolation and part replacement. A helper SHALL perform only known routine work with available parts and access, report blocked work and retain jobs through save/reload.
#### Scenario: Unprepared or duplicate repair
- **WHEN** a repair is requested without diagnosis, isolation or parts, or a job is already queued
- **THEN** the request is rejected with a specific explanation and no additional stock is consumed
#### Scenario: Cancel and resume
- **WHEN** a reserved job is cancelled or saved while partly complete
- **THEN** cancellation refunds reserved parts exactly once and reload resumes the recorded job only after explicit game resume
#### Scenario: Essential access
- **WHEN** the workshop bus is isolated, damaged or unpowered
- **THEN** doors, the lift and the route to the bridge remain usable
