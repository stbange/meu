# MEU on GitHub Pages

Live prototype: [play Mechanized Exploration Unit](https://stbange.github.io/meu/).

## Build and publish

The MEU checks and Pages workflow publishes pushes to main after all four foundation jobs pass: specifications, Windows, Linux, and web. Working branches and pull requests run checks without publishing. The workflow can also be started manually for main.

The web job builds the optimized C/WebAssembly/WebGL 2 client, runs compiled core/scenario tests, and checks browser launches at desktop, portrait-phone, and landscape-phone sizes in Chromium, plus both phone sizes in WebKit. The published package contains only index.html, index.js, index.wasm, input.js, style.css, the no-Jekyll marker, and revision.txt. Core-test binaries and test diagnostics are excluded.

The deployment job uses GitHub's Pages artifact service and its github-pages environment. Only that job receives Pages and identity-token write permissions. The website stays at the repository path /meu/; assets use relative URLs.

After deployment, a live verification job checks revision.txt against the source commit and launches the public page in all five browser/layout configurations. Reports and screenshots are retained as the live-pages-checks artifact. A deployment or successful check does not mark gameplay as user-accepted.

## Play and update

Select Begin exploration. M0d adds complete touch contexts, session settings and explicit pause/resume to the connected 240 m home and M0c physical bridge piloting. Approach the central console and use E or the touch prompt. At the helm W/S drives, A/D turns, Space brakes, P parks and E releases control. Mouse/arrow/touch look does not steer. Releasing drive brakes; leaving the helm requests a parked stop. Touch controls adapt to drive/turn with Brake, Park and Release helm buttons. The bridge reports speed, heading, braking distance and any structure/support rejection.

On foot, WASD or touch buttons walk through the bridge, quarters, engineering hall and lift. The lift is behind the start position: walk into its centre, choose 1/2/3 or tap a deck, then walk forward out after arrival. E or a contextual button uses nearby home objects. Settings or F1 opens sensitivity, vertical drag inversion, separate walking bob/machine sway (both initially off) and sound volume. Close settings, then explicitly resume. Focus/background loss and screen changes pause and discard old gestures. Preferences last for this session; durable saves/settings remain M0e.

Future source changes pushed to main rebuild, test, and redeploy automatically. Documentation-only commits do not rebuild the runtime. Use the repository's Pages settings with GitHub Actions as the publishing source.

To run the existing launch checks against the public site, set MEU_RELEASE=1 and MEU_BASE_URL=https://stbange.github.io/meu/, then run:

~~~text
npm run test:web -- --grep "renders the C/OpenGL room"
~~~

The base URL must include its final slash. The tests retain that path and do not start a local server when MEU_BASE_URL is supplied.

## Verified publication

Published on 3 November 2026 from [M0d source commit bd4f5d9](https://github.com/stbange/meu/commit/bd4f5d960007a7da6666d7ee7884cd7e8d722f2b). The [publishing workflow](https://github.com/stbange/meu/actions/runs/41684388321) passed all six jobs: specifications, Windows, Linux, web, deployment and live verification. Runtime identity is 0.0.4, scene m0d-controls-v1, content version 4.

The public site passed all five launch configurations: desktop/portrait/landscape Chromium and portrait/landscape WebKit. Touch settings/start/pause/resume work in WebKit, and test diagnostics are absent. A separate public touch check confirmed settings and mute before Begin, driving and parking, settings releasing helm control, reset defaults, and rotation with explicit resume. It also confirmed the exact published revision and a valid application/wasm response/header. See [M0d evidence, report and captures](evidence/m0d-controls.md). Physical-phone checks, performance and user acceptance remain pending.

The earlier [M0c publication](https://github.com/stbange/meu/actions/runs/41672789694) and [M0b publication](https://github.com/stbange/meu/actions/runs/41655285514) remain recorded in their evidence. Documentation-only commits after the M0d source revision do not rebuild the runtime.
