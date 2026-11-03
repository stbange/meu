# MEU Project Status

Status baseline: 3 November 2026.

## Repository and planning

The independent project is `stbange/meu`, checked out as the `meu` sibling folder on the existing Windows development computer. It has a fresh Git history, its own instructions, design draft, implementation plan, verification strategy, and proposed OpenSpec foundation.

M0a–M0c have been implemented at the user's request. The connected interior now has physical bridge piloting, controlled braking, validated parking and collision/support checks for robot travel. See [M0c evidence](evidence/m0c-piloting.md) for controls, tests and pending acceptance, and the earlier M0a/M0b evidence for the platform and interior foundation.

## Gameplay milestones

| Milestone | Implementation | Automated verification | User acceptance |
| --- | --- | --- | --- |
| M0 Moving home and platform proof | M0a–M0c complete; M0d–M0f pending | Native/WebAssembly travel tests and rendered tours pass; final browser, CI and publication records are in M0c evidence | Pending |
| M1 Complete authored expedition | Not started | Not run | Pending |
| M2 Generated small city | Not started | Not run | Pending |
| M3 Complete exploration arc | Not started | Not run | Pending |
| M4 Release preparation | Not started | Not run | Pending |

## Next action

The [MEU web prototype](https://stbange.github.io/meu/) is published on GitHub Pages from main after checks pass. M0c uses the existing pipeline; exact source and verification are recorded in [publishing evidence](github-pages.md) and the M0c evidence record. Publication does not change pending user acceptance.

The next task is M0d complete touch/settings and lifecycle behavior in the [implementation plan](implementation-plan.md). Preserve the on-foot/helm contexts, predictable stops and connected interior while adding settings and completing device coverage. Review M0c separately from acceptance of the full M0 experience. Commands and selected versions are in [build and test](build-and-test.md).
