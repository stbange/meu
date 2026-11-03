# MEU on GitHub Pages

Live prototype: [play Mechanized Exploration Unit](https://stbange.github.io/meu/).

## Build and publish

The MEU checks and Pages workflow publishes pushes to main after all four foundation jobs pass: specifications, Windows, Linux, and web. Working branches and pull requests run checks without publishing. The workflow can also be started manually for main.

The web job builds the optimized C/WebAssembly/WebGL 2 client, runs compiled core/scenario tests, and checks browser launches at desktop, portrait-phone, and landscape-phone sizes. The published package contains only index.html, index.js, index.wasm, input.js, style.css, the no-Jekyll marker, and revision.txt. Core-test binaries and test diagnostics are excluded.

The deployment job uses GitHub's Pages artifact service and its github-pages environment. Only that job receives Pages and identity-token write permissions. The website stays at the repository path /meu/; assets use relative URLs.

After deployment, a live verification job checks revision.txt against the source commit and launches the public page in all three browser layouts. Reports and screenshots are retained as the live-pages-checks artifact. A deployment or successful check does not mark gameplay as user-accepted.

## Play and update

Select Begin exploration. M0b includes the bridge, living quarters, engineering hall and connecting lift inside a 240 m robot. Use WASD and mouse drag/arrows, or touch movement buttons and the look pad. Walk backward from the starting position into the lift's centre, select 1/2/3 or tap a deck, then walk forward out when it stops. E or a contextual touch button uses nearby home objects. The bridge frames the robot's hull and a monumental alien city. Steering and saves remain later steps.

Future source changes pushed to main rebuild, test, and redeploy automatically. Documentation-only commits do not rebuild the runtime. Use the repository's Pages settings with GitHub Actions as the publishing source.

To run the existing launch checks against the public site, set MEU_RELEASE=1 and MEU_BASE_URL=https://stbange.github.io/meu/, then run:

~~~text
npm run test:web -- --grep "renders the C/OpenGL room"
~~~

The base URL must include its final slash. The tests retain that path and do not start a local server when MEU_BASE_URL is supplied.

## Verified publication

Published on 3 November 2026 from [M0b source commit 1b32a36](https://github.com/stbange/meu/commit/1b32a367f075ddfd25be7020edd759eb84d8fb29). The [publishing workflow](https://github.com/stbange/meu/actions/runs/41655285514) passed all six jobs: specifications, Windows, Linux, web, deployment and live verification. Runtime identity is 0.0.2, scene m0b-unit-v1, content version 2.

The public site passed all three launch checks: desktop, portrait phone and landscape phone. An independent HTTP check confirmed the M0b page, the WebAssembly response with application/wasm content type and the exact published source revision. Physical-phone performance and gameplay acceptance remain pending. See [M0b evidence and captures](evidence/m0b-interior.md).

The [initial M0b publication](https://github.com/stbange/meu/actions/runs/41653332287) also passed all six jobs. The follow-up clarifies safe lift boarding and checks the cab-edge message before the full route. Documentation-only commits after the source revision do not rebuild the runtime.
