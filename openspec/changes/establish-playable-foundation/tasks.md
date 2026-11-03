## Implementation

- [x] 1. M0a: Establish isolated native, web, and headless CMake targets; pin dependencies, add initial CI, and record reproducible build commands. Maps to PORT-01 and PORT-05. Local verification is in docs/evidence/m0a-foundation.md; remote CI verification is recorded separately.
- [ ] 2. M0b: Build the authored scale scene, robot hull, four connected spaces, and lift/service spine. Maps to VIEW-01 and VIEW-02.
- [ ] 3. M0b: Implement stable first-person locomotion, collision, interaction, touch/desktop actions, and coordinate ownership. Maps to VIEW-01 and VIEW-02.
- [ ] 4. M0c: Implement helm input, look/steer separation, controlled stop, clearance rejection, and valid parking. Maps to CMD-01 and CMD-02.
- [ ] 5. M0d: Complete touch layouts, input-context cleanup, lifecycle pause, and explicit resume. Maps to VIEW-03 and PORT-02.
- [ ] 6. M0e: Implement versioned basic saves, durable browser completion, previous-snapshot recovery, and export/import. Maps to PORT-03.

## Automated verification

- [ ] 7. Run C tests for transforms, travel rejection/stopping, lifecycle input, malformed saves, and snapshot recovery. Retain reports and failing fixtures.
- [ ] 8. Run rendered movement, collision, lift, and helm scenarios. Verify translation/turn/start/stop motion with no unintended standing drift beyond 2 cm over 60 seconds, and no ejection or floor penetration.
- [ ] 9. Build and run Windows/Linux foundation checks and the actual WebAssembly browser scenario. Retain versions, logs, screenshots, and input replays. Maps to PORT-01 and PORT-04.
- [ ] 10. Complete a touch-only bridge-to-engineering route at 360 by 640 and 640 by 360 CSS pixels; rotate, background, resume, and reload a confirmed save. Maps to VIEW-03 and PORT-02/03.
- [ ] 11. Test unavailable storage, corrupt latest snapshot, unsupported schema, and save interruption. No successful-save status before durable completion. Maps to PORT-03.
- [ ] 12. Select real reference devices, measure the documented performance budgets, and investigate any regression before closing verification.

## User acceptance

- [ ] 13. User reviews whether the robot feels enormous while all play remains first person.
- [ ] 14. User reviews steering, interior motion comfort, and readable/useful home spaces.
- [ ] 15. User reviews real-phone touch usability and the proposed direction for M1.

Task 1 is implemented. The remaining full-foundation checks and acceptance tasks are pending; initial M0a tests do not complete travel, interior, or save verification. Add checks as each step lands; M0f integrates the full evidence/playtest. Record automated results separately from user acceptance. Commit and push each completed step. The [implementation plan](../../../docs/implementation-plan.md) gives each step's scope and evidence.
