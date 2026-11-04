# MEU Project Status

Status updated: 4 November 2026.

## Repository and planning

The independent project is `stbange/meu`, checked out as the `meu` sibling folder on the existing Windows development computer. It has its own Git history, instructions, design, implementation plan, tests and proposed OpenSpec foundation.

M0a–M0e have been implemented at the user's request. The connected 240-metre home has supported bridge travel, complete touch contexts/settings, explicit pause/resume and durable local journeys with recovery and portable export/import. See [M0e evidence](evidence/m0e-saves.md); earlier records cover the platform, interior, travel and lifecycle foundation.

## Gameplay milestones

| Milestone | Implementation | Automated verification | User acceptance |
| --- | --- | --- | --- |
| M0 Moving home and platform proof | M0a–M0e complete; M0f device/performance work pending | Original platform evidence retained; current native regressions pass | Deferred to combined post-M1 playtest; real phones/performance pending |
| M1 Complete authored expedition | M1a–M1e implemented, plus input/surface fixes and Pip onboarding | 16 native/core groups; ASan/UBSan; rendered regressions; complete SDL-input expedition and durable completed journey pass. CI 0bbf740 passes Windows/Linux, 15 Wasm groups in both modes, 77 full browser cases and 16 optimized cases; see Pip evidence | Pending; user requested testing after M1 |
| M2 Generated small city | Not started | Not run | Pending |
| M3 Complete exploration arc | Not started | Not run | Pending |
| M4 Release preparation | Not started | Not run | Pending |

## Review schedule

On 4 November 2026 the user requested implementation through M1 without interim reviews and will test after M1. Human acceptance and physical-device performance remain pending; this explicitly waives the M0 review dependency for M1 implementation, not its eventual acceptance.

## M1 playtest corrections

The user's mouse-capture and floor-flicker corrections are implemented in 0.1.1 ([evidence and current builds](evidence/mouse-surfaces.md)). Native capture/geometry/sanitizer and complete expedition checks pass. Windows/Linux, debug/optimized WebAssembly and the full desktop/phone browser configuration pass. The browser suite also verifies synchronous station-overlay removal before a fresh movement touch. User confirmation remains pending; M2 is untouched.

## Onboard help

The user requested in-game explanations and next steps, preferably through a flying robot. Pip is implemented for 0.1.2 with seven shared topics and journey-based next steps. The native reader, 16 core/sanitizer groups and actual SDL help/capture/focus/save/expedition checks pass. Windows/Linux, 15 Wasm groups in each mode, six focused mouse cases, 11 focused Pip cases, the complete 98-case browser configuration (77 pass/21 deliberate skips) and optimized checks (16 pass/two deliberate skips) pass without retries. See [Pip evidence and current builds](evidence/onboard-guide.md). User acceptance remains pending.

## Next action

The user has now explicitly authorized publishing `codex/m1-authored-expedition` to GitHub Pages. Workflow changes allow this branch alongside main after all existing checks, followed by exact-revision and live browser verification. Publication is being verified; see [working-branch Pages evidence](evidence/working-branch-pages.md). No merge into main is needed, and gameplay acceptance remains pending.

M1 implementation is ready for the user’s deferred test. See [the complete route and commands](m1-expedition.md) and [M1e evidence/limits](evidence/m1e-expedition.md). The implementation is on `codex/m1-authored-expedition` and is not merged into main. Previous M0e publication remains documented separately; the public [web prototype](https://stbange.github.io/meu/) will update after working-branch deployment succeeds.

The [verified 0.1.2 source CI run](https://github.com/stbange/meu/actions/runs/41849350807) passes all required development jobs; deployment was skipped. This machine has a working native build but matching Emscripten/Playwright browser downloads are policy-blocked. Finish M0f reference-system and physical Android/iPhone measurements and the combined M0/M1 scale, comfort, controls and expedition review when the user chooses to test. Provisional targets remain 60 fps desktop, 30 fps phone, under 256 MiB Wasm linear memory; none is a real-device result yet.

The automated route completes in 8531 active ticks (about 142 seconds) with optimized input and no reading/exploration time. That is correctness evidence, not confirmation of the proposed 20–40-minute human loop or satisfying repair pacing. Do not start M2 generation or archive any active change as accepted until the remaining verification and required user decisions are recorded.
