# MEU Project Status

Status baseline: 3 November 2026.

## Repository and planning

The independent project is `stbange/meu`, checked out as the `meu` sibling folder on the existing Windows development computer. It has its own Git history, instructions, design, implementation plan, tests and proposed OpenSpec foundation.

M0a–M0e have been implemented at the user's request. The connected 240-metre home has supported bridge travel, complete touch contexts/settings, explicit pause/resume and durable local journeys with recovery and portable export/import. See [M0e evidence](evidence/m0e-saves.md); earlier records cover the platform, interior, travel and lifecycle foundation.

## Gameplay milestones

| Milestone | Implementation | Automated verification | User acceptance |
| --- | --- | --- | --- |
| M0 Moving home and platform proof | M0a–M0e complete; M0f pending | 12 native/headless and 11 debug/optimized WebAssembly groups, all native replays, 49 browser cases, final parser/focus checks and 10 shipping cases passed; clean remote checks pending; see evidence | Pending; physical-phone interaction and performance review pending |
| M1 Complete authored expedition | Not started | Not run | Pending |
| M2 Generated small city | Not started | Not run | Pending |
| M3 Complete exploration arc | Not started | Not run | Pending |
| M4 Release preparation | Not started | Not run | Pending |

## Next action

The [MEU web prototype](https://stbange.github.io/meu/) publishes from main after checks pass. M0e publication is being verified; the current published-source record is in [GitHub Pages evidence](github-pages.md). Publication does not change pending user acceptance.

Next is M0f integrated verification and focused playtest in the [implementation plan](implementation-plan.md): select actual Windows/Linux and Android/iPhone reference devices, measure the defined performance budgets, review scale/comfort/discoverability and record explicit user feedback. Preserve validated save publication, source recovery, context cleanup and supported travel. Settings are kept when included in an explicit saved journey. Commands are in [build and test](build-and-test.md); [save format and recovery](save-format.md) records storage behavior and compatibility.
