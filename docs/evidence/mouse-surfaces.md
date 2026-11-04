# Captured mouse and stable surfaces — 4 November 2026

This M0/M1 playtest correction addresses the user's request to capture the mouse on gameplay clicks and remove flickering overlapping floors/structures. Runtime version 0.1.1; existing scene/world/save identities stay compatible. M2 has not started. User confirmation remains pending.

## Changes

- Native: clicking the gameplay view enables real SDL window-relative mouse mode, hides/confines the pointer and uses relative motion after button release. Escape pauses/releases; focus loss and settings/journey/station panels release. Keyboard resume alone does not recapture. Touch-synthesized mouse events are excluded.
- Browser: clicking the 3D canvas requests actual Pointer Lock in the trusted mouse gesture. Relative mousemove supplies look once, including beyond the viewport. Intentional panel release preserves station simulation; unsolicited loss pauses and clears old inputs. Browser denial retains drag/look-pad/keyboard fallback. Touch does not request pointer lock.
- Rendering: the causeway slabs overlap at Y0, and the installed bridge overlaps that same plane. Opaque room/hull boxes also share faces. A bounded box-union mesher subtracts buried/shared rectangles and gives each coplanar region one owner. Cylinders are excluded from rectangular masks. The bridge emits only faces not already owned by base geometry.
- Route paint is now a flat, depth-biased decal pass instead of thin overlapping boxes. Collision/support records and saved positions are unchanged.

The prior M1 documentation CI exposed another interruption race: production state showed the suited Commander still inside the airlock, no held movement, and no pause. The station panel remained in the DOM until the next six-frame HUD refresh after its C input context had closed. A fresh movement touch could therefore hit the old overlay. Context changes now hide it synchronously; station commands also refresh content immediately. The real-touch route records panel visibility during the close event itself, before a later frame can hide the bug.

## Local checks

Debian 13/GCC 14.2.0, CMake 3.31.6, Ninja 1.12.1, SDL 3.4.18, Mesa 25.0.7 llvmpipe, same host as M1e. Source `.tools/activate.sh` first.

```bash
ASAN_OPTIONS=detect_leaks=0 python3 tools/build.py core --test --sanitizers
LIBGL_ALWAYS_SOFTWARE=1 SDL_AUDIODRIVER=dummy xvfb-run -a python3 tools/build.py native --test --smoke --tour --pilot --lifecycle --persistence --expedition
OPENSPEC_TELEMETRY=0 npm run spec:validate
node --check web/input.js
npx playwright test --list
```

Results: 15/15 native/core and address/undefined sanitizer groups pass. LeakSanitizer remains unavailable under this host's ptrace. Geometry regressions assert unique coplanar faces, exact union floor area, deterministic duplicate ownership, contained/shared-face removal and the actual base-plus-bridge join. Interior mesh remains within the existing vertex budget. Native smoke, complete interior/pilot/lifecycle/persistence tours and the full SDL-input expedition pass. Lifecycle explicitly checks real SDL relative mode after mouse-button release, continued look without a button, menu release, focus loss, no automatic recapture and fresh-click recapture. SDL mode changes flush queued motion; the expedition replay therefore acquires capture before injecting its look delta. The complete route still saves/reloads successfully (8531 ticks, 20114 kJ, 8 alloy, 6 conductor, 4 sealant).

All seven active OpenSpec changes validate. Playwright configures 84 cases; new desktop tests cover genuine lock, outside-window relative look, Escape, settings/journeys, active station release and denied-lock fallback. Existing touch and browser-save checks remain. Optimized/live-check filters include captured mouse behavior. Matching SDK/browser downloads remain policy-blocked locally; the full browser checks use remote CI.

## Remote checks and builds

Verified runtime/test source: `ff694a85b1ce01cd4e804dd7e02912ac911f505e`, [CI run 41825493264](https://github.com/stbange/meu/actions/runs/41825493264). All required development jobs pass. Runtime 0.1.1, scene `m1-tower-v1`, content 6, schema 2. The final evidence update changes documentation and verification checkboxes only.

| Check | Result |
| --- | --- |
| Strict OpenSpec validation | 7 active changes pass |
| Windows native | 15/15 test groups pass |
| Linux native | 15/15 groups, rendered tours, captured SDL lifecycle, full expedition and separate-process durable save/reload pass |
| Linux address/undefined sanitizers | 15/15 groups pass |
| Debug WebAssembly | 14/14 compiled test groups pass |
| Focused desktop Pointer Lock | 5/5 real-browser cases pass under Xvfb |
| Full Chromium/WebKit desktop and phone layouts | 65 pass, 19 deliberate target-specific skips, no retries |
| Optimized WebAssembly | 14/14 compiled test groups pass |
| Optimized browser rendering/settings/journeys/capture | 11 pass, 2 deliberate mobile Pointer Lock skips, no retries |

Development builds from this source:

- [Windows archive](https://github.com/stbange/meu/actions/runs/41825493264/artifacts/13318324584): extract and run `bin/meu.exe`.
- [Linux archive](https://github.com/stbange/meu/actions/runs/41825493264/artifacts/13319865455).
- [Optimized web build and browser reports](https://github.com/stbange/meu/actions/runs/41825493264/artifacts/13319089058): serve `build/web-release/bin` over HTTP.

Remote browser commands now use `xvfb-run -a npm run test:web`, with a focused `tests/web/mouse.spec.cjs --project=desktop` run first. Only Pointer Lock cases request a desktop window; existing background/minimize emulation retains headless mode because bare Xvfb cannot minimize a real window without a window manager. The optimized run sets `MEU_RELEASE=1` and selects the rendering, persisted-settings, saved-journey and captured-mouse cases. No public deployment was requested.

## Browser diagnostics

An isolated diagnostic using the installed Chromium 154.0.8037.57 reproduces an `InvalidStateError` when fallback `setPointerCapture` runs during a pending Pointer Lock request. Fallback capture now waits for denial; the diagnostic confirms genuine locked relative mouse events, including movement beyond the viewport, without a JavaScript error. This diagnostic mocks C calls and does not substitute for the full Wasm/browser tests. New mouse failures retain production state and event diagnostics; CI runs the desktop capture cases before the longer route suite.

The focused 8ed4d82 browser run passes both menus, station release and refused-lock fallback. Its headless event stream records an equal reverse delta after each requested outside-viewport movement, cancelling the observed final yaw. The headed/Xvfb b858dab run passes all five mouse cases. Relative look also waits for the lock-change confirmation so acquisition movement cannot rotate the Commander before capture is established. Earlier failed runs remain diagnostic evidence, rather than being recorded as passing verification.

Forcing the entire b858dab suite into headed mode passed 62 cases and deliberately skipped 19, but failed three backgrounding/minimization cases: the virtual desktop has no window manager to minimize real windows. Launch mode now belongs to the Pointer Lock test file, keeping the existing headless interruption emulation for the other suites. Source ff694a8 passes the focused five capture cases, the complete 84-case debug configuration and the optimized checks with this configuration.

## Limits

This correction covers the authored world and interior opaque geometry. The interior union partitions more faces while avoiding hidden surfaces (20504 interior triangles in the current fixture); no real-device performance claim is made. The browser API must be supported/allowed to acquire a lock; touch and denied-lock fallback remain usable. Main and public Pages are unchanged until a separate merge/deployment decision. Neither successful checks nor the requested fixes imply acceptance of all M0/M1 gameplay.
