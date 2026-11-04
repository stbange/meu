## ADDED Requirements
### Requirement: EXP-01 Complete authored tower expedition
The Commander SHALL complete an exclusively first-person route through scouting, supported manual travel, diagnosis/repair, bulk recovery, safe suit egress, bypass, precise salvage, return, testing/adaptation/installation and newly accessible travel. Desktop and touch SHALL expose the same station actions, visible costs, rejection reasons and next-objective guidance.
#### Scenario: Complete useful discovery
- **WHEN** the Commander safely recovers, tests, adapts and installs the coupler
- **THEN** the west service bridge becomes visibly supported and parking beyond the previous route boundary records expedition completion
#### Scenario: Failure and continuation
- **WHEN** energy, parts, access, repair, suit conditions or persistence are interrupted
- **THEN** requests reject atomically or stop safely, blocked work is explained, valid saved progress can resume, and catastrophic recovery remains an explicit choice
#### Scenario: Durable completed journey
- **WHEN** a completed expedition is saved and opened in a fresh process or page
- **THEN** installed access, inventory, discoveries and completion are restored paused without claiming user acceptance
