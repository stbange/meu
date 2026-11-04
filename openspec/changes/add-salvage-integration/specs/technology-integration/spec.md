## ADDED Requirements
### Requirement: TECH-01 Understand and install meaningful technology
The Commander SHALL test unfamiliar salvage, see material and power/cooling tradeoffs before adapting it, and install/remove the prepared stabilizer in one authored socket. Installation SHALL make a previously unsupported route traversable, with persisted discovery identity.
#### Scenario: Atomic adaptation
- **WHEN** the Commander lacks tested salvage, stock, energy, heat capacity or workshop access
- **THEN** adaptation rejects without any costs; a successful adaptation pays once
#### Scenario: Route and safe removal
- **WHEN** the stabilizer is installed or removed at a safe primary socket
- **THEN** the west bridge's visible and traversable support changes together, while removal over the bridge is rejected
#### Scenario: Reload installed technology
- **WHEN** a journey with installed technology is restored
- **THEN** the installation and newly supported route exist before saved-position validation
