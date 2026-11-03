# MEU Project Status

Status baseline: 3 November 2026.

## Repository and planning

The independent project is `stbange/meu`, checked out as the `meu` sibling folder on the existing Windows development computer. It has a fresh Git history, its own instructions, design draft, implementation plan, verification strategy, and proposed OpenSpec foundation.

M0a and M0b have been implemented at the user's request. The bridge, living quarters, engineering hall and lift run on Windows and in the browser. See [M0b evidence](evidence/m0b-interior.md) for scope, controls, automated results and pending acceptance, and [M0a evidence](evidence/m0a-foundation.md) for the original platform foundation.

## Gameplay milestones

| Milestone | Implementation | Automated verification | User acceptance |
| --- | --- | --- | --- |
| M0 Moving home and platform proof | M0a–M0b complete; M0c–M0f pending | M0b native/WebAssembly tests, rendered SDL motion tour, desktop/phone routes, release launch and OpenSpec checks; final CI/publication recorded in evidence | Pending |
| M1 Complete authored expedition | Not started | Not run | Pending |
| M2 Generated small city | Not started | Not run | Pending |
| M3 Complete exploration arc | Not started | Not run | Pending |
| M4 Release preparation | Not started | Not run | Pending |

## Next action

The [MEU web prototype](https://stbange.github.io/meu/) is published on GitHub Pages from main after checks pass. M0b publication uses the existing pipeline; exact source and workflow verification are recorded in [publishing evidence](github-pages.md) and the M0b evidence record. Publication does not change pending user acceptance.

The next task is M0c bridge piloting and supported travel in the [implementation plan](implementation-plan.md). The motion fixture supplies the pose interface for steering to drive. Review the M0b interior separately from acceptance of the full M0 experience. Commands and selected versions are in [build and test](build-and-test.md).
