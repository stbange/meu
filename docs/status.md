# MEU Project Status

Status baseline: 3 November 2026.

## Repository and planning

The independent project is `stbange/meu`, checked out as the `meu` sibling folder on the existing Windows development computer. It has its own Git history, instructions, design, implementation plan, tests and proposed OpenSpec foundation.

M0a–M0d have been implemented at the user's request. The connected 240-metre home has bridge piloting, controlled stops, validated travel, complete touch contexts, session settings and explicit pause/resume. See [M0d evidence](evidence/m0d-controls.md) for verification and pending acceptance; earlier records cover the platform, interior and travel foundation.

## Gameplay milestones

| Milestone | Implementation | Automated verification | User acceptance |
| --- | --- | --- | --- |
| M0 Moving home and platform proof | M0a–M0d complete; M0e–M0f pending | 9 groups per native/headless/debug/optimized WebAssembly target, all SDL replays, Linux sanitizers, 34 browser cases, 5 release/live launches and clean Windows/Linux CI passed; see evidence | Pending; physical-phone interaction and performance review pending |
| M1 Complete authored expedition | Not started | Not run | Pending |
| M2 Generated small city | Not started | Not run | Pending |
| M3 Complete exploration arc | Not started | Not run | Pending |
| M4 Release preparation | Not started | Not run | Pending |

## Next action

The [MEU web prototype](https://stbange.github.io/meu/) publishes from main after checks pass. All six [M0d publishing jobs](https://github.com/stbange/meu/actions/runs/41684388321) passed. The exact source, live touch check and publication record are in [M0d evidence](evidence/m0d-controls.md) and [GitHub Pages evidence](github-pages.md). Publication does not change pending user acceptance.

The next implementation task is M0e durable local saves in the [implementation plan](implementation-plan.md). Persist the basic parked journey and settings, confirm durable browser completion, preserve a previous complete snapshot and test fresh-page restoration and failure recovery. Settings currently last only for the session. Preserve context cleanup, explicit resume, predictable stops and the connected interior. Physical Android/iPhone interaction checks and the M0f scale/comfort review remain pending. Commands and selected versions are in [build and test](build-and-test.md).
