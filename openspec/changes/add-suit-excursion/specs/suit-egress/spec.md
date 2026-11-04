## ADDED Requirements
### Requirement: SUIT-01 Conditional first-person egress and return
The Commander SHALL leave through a reachable airlock only with parked alignment, scanned landing/environment, powered shelter and viable suit/energy/thermal limits. The unit SHALL stay parked during the excursion and the Commander SHALL see suit status, warnings and a marked return route.
#### Scenario: Blocked exit
- **WHEN** parking, alignment, scan, repair, field power or resource/thermal margin is missing
- **THEN** egress is rejected with its specific condition and the unsuited Commander cannot cross the airlock boundary
#### Scenario: Saved exterior journey
- **WHEN** the Commander saves outside and reloads
- **THEN** location, suit time, exposure, field and return route are restored paused without background consumption
### Requirement: SUIT-02 Preserve shelter or recover explicitly
The Commander SHALL be warned that removing the coupler powers down shelter without a bypass. A fitted bypass SHALL preserve the return envelope. A catastrophic failure SHALL offer explicit recovery to the last secure checkpoint, losing progress and unsecured salvage since that point without deleting the established home.
#### Scenario: Unsafe source removal
- **WHEN** the source is removed without bypass or shelter power is disconnected
- **THEN** exposure warnings rise toward the suit limit and the Commander can retreat before failure
#### Scenario: Deliberate recovery
- **WHEN** suit limits are exceeded
- **THEN** no automatic relocation occurs; explicit recovery restores the secure cabin checkpoint and discards unsecured expedition gains
