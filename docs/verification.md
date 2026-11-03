# MEU Verification Strategy

This strategy is planned for the implementation milestones. At the planning baseline, only documentation/specification validation is available. No C, client, generation, or browser game test has run because the game is not implemented.

## Test boundaries

| Layer | Scope | First milestone |
| --- | --- | --- |
| Pure C unit tests | Transforms, clock, command validation, resource accounting, transactions, state transitions, save validation | M0 |
| Command-loop scenarios | Real commands and simulation ticks with observable results and rejection reasons | M0, expanded in M1 |
| Rendered client scenarios | Actual input, collision, interaction targeting, bridge control, lifts, apertures, and frame coordination | M0 |
| Browser scenarios | Actual served WebAssembly build, desktop/touch input, layout, lifecycle, storage, and graphics failures | M0 |
| Generator/property tests | Stable identities, route/support constraints, prerequisites, recovery, bounded retries, and regression seeds | M2 |
| Soak/performance tests | Bounded memory/residency, repeated transactions, long travel, thermal load, and frame-time behavior | M0 budget checks; expanded M1–M4 |
| Focused human playtests | Scale, comfort, discoverability, novelty, maintenance enjoyment, and real-phone usability | Every milestone |

Separate the state used by the simulation from rendering and platform APIs so core tests do not require a window. The headless scenario driver must call the production command/update loop. Rendered tests must also exercise real input and collision; a privileged test hook that directly edits state cannot replace them.

## Determinism and reproduction

Record build ID, content/generator/save versions, test scene or seed, starting fixture, commands, and tick numbers. Assert exact integer quantities and stable generated records. Compare floating-point positions with documented tolerances and image output with targeted visual tolerances. Do not require arbitrary cross-platform frame hashes to match.

On failure, retain a compact state report, replay log, relevant seed/save, and screenshot or browser trace when available. Reproduce from the fixture instead of using blind retries to turn flaky results green. A deliberate fixture teleport may establish the start of a test; its actual gameplay assertions must follow normal paths.

## Required foundation scenarios

Map checks to VIEW-01/02/03, CMD-01/02, and PORT-01/02/03/04 in the [foundation specifications](../openspec/changes/establish-playable-foundation/proposal.md).

- Traverse all four spaces, use ramps/lift, operate the helm, park, and return to engineering in first person.
- Exercise standing and walking aboard during translation, turning, acceleration, and braking. Measure unintended standing drift against the 2 cm over 60 second target; reject ejection and floor penetration independently.
- Look at the helm without steering, reject travel away from the helm, stop on helm release, and reject an obstructed/unsupported route before reaching it.
- Complete the route using touch only at 360 by 640 and 640 by 360 CSS pixels. Check controls and text at browser safe areas and with device pixel ratios distinct from CSS sizing.
- Interrupt held movement/look/travel with pause, focus loss, backgrounding, resize, and rotation. Resume explicitly without stale input or accumulated simulation ticks.
- Save and reload in a fresh process/page, confirm settings and poses, and resume stopped. Inject unavailable storage, quota failure, interrupted publication, malformed latest snapshot, and unsupported schema. Preserve the last confirmed valid data.

Browser automation loads the built game through an HTTP server. Use Playwright browser projects for Chromium, Firefox, and WebKit where the execution host supports them; include a compatible macOS/Linux CI job for the WebKit project. Supplement with actual Android Chrome and iPhone Safari. Test a shipping-like build without mutation/debug endpoints as well as a test build with read-only assertions and controlled fault injection.

## Expedition and generation scenarios

M1 adds a complete authored route through charging, grid adaptation, fault diagnosis, assistant work, salvage, suit egress, bypass, return, installation, and newly accessible travel. Assert resource conservation, atomic costs/rewards, safe cancellation, blocked actions, reload at each major state, and checkpoint recovery. Test removing the component that powers shelter with and without a bypass.

M2 adds constructive progression validation, energy recovery locations, mech clearance/support, valid parking, accessible prerequisites, material availability, and safe returns from mandatory foot sites. Begin with fixed seeds on each change and a 1,000-seed nightly batch, then adjust coverage to measured cost. Keep every discovered regression seed. Exercise bounded failure/fallback when the generator cannot satisfy constraints.

Streaming tests cover collision readiness, dirty-state publication before eviction, occupied-sector pinning, rebasing, reload consistency, and stable state after repeatedly entering/leaving a region. A long route must not increase active chunk or renderer resources without bound.

## CI and performance gates

M0a creates the commands and workflows; they are not available at this baseline. Each code change must build affected targets and run relevant unit/scenario tests. Cross-platform compile/core checks and a browser launch smoke check run from the foundation. Add the full input/save browser suite as those behaviors land. Specification validation remains a separate required check.

Larger seed sweeps, sanitizers, fuzzing, and long soaks run nightly or for release candidates as their systems exist. Compiler/sanitizer configurations must match supported native environments; software-rendered CI does not establish real GPU or phone performance.

Measure a named scene, camera route, settings, device, browser/driver, and duration. Report frame-time distribution and stalls, not only average fps. Initial targets are 60 fps on selected ordinary desktop hardware and 30 fps on selected midrange Android/iPhone devices, with under 256 MiB WebAssembly linear memory in the slice and graphics memory tracked separately. Measure browser startup/download and touch responsiveness as well. Record actual devices in milestone evidence before claiming these budgets pass.

## Evidence and acceptance

For each completed implementation step, add a concise evidence record under `docs/evidence/` with behavior, tested commit, commands/results, scene/seed/version identity, screenshots/replays where useful, and limitations that affect interpretation. List a focused manual playtest and the pending user decision separately.

Update `docs/status.md` with three independent fields: implementation, automated verification, and user acceptance. Only explicit user feedback completes the last field. A successful commit, push, CI run, or screenshot cannot substitute for a playtest decision.
