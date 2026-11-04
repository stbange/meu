## ADDED Requirements
### Requirement: GRID-01 Readable isolated grid connection
The Commander SHALL connect to the authored tower socket only while parked in reach, see its repeating surge pattern, isolate the workshop bus, and disconnect safely.
#### Scenario: Unsafe connection
- **WHEN** connection is requested away from the socket, while moving, or without bus isolation
- **THEN** the request is rejected with a specific reason and the essential bus remains available
#### Scenario: Grid surge
- **WHEN** an unstabilized connection encounters its periodic surge
- **THEN** the input buffer trips without consuming life support or blocking essential doors
