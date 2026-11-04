## ADDED Requirements
### Requirement: PWR-01 Active-time power and safe recovery
The unit SHALL account for stored energy, solar input and heat only during active simulation, keep an isolated life-support reserve, and show projected expenditure, uncertainty and return margin before travel. Collectors SHALL require parking; essential interior access SHALL remain usable at zero main energy.
#### Scenario: Useful charging during interior work
- **WHEN** the Commander deploys collectors while parked in sunlight and performs an interior task
- **THEN** stored energy increases within capacity without requiring an empty waiting interaction
#### Scenario: Paused and invalid operations
- **WHEN** the game is paused or an unaffordable or overheating operation is requested
- **THEN** paused time consumes nothing and rejected transactions preserve stock and energy
#### Scenario: Low energy travel
- **WHEN** available main energy falls below the protected return margin
- **THEN** manual travel stops safely and interior access and solar recovery remain available
