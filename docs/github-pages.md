# MEU on GitHub Pages

Live prototype: [play Mechanized Exploration Unit](https://stbange.github.io/meu/).

## Build and publish

The MEU checks and Pages workflow publishes pushes to main after all four foundation jobs pass: specifications, Windows, Linux, and web. Working branches and pull requests run checks without publishing. The workflow can also be started manually for main.

The web job builds the optimized C/WebAssembly/WebGL 2 client, runs compiled core/scenario tests, and checks browser launches and durable saved-journey/export/import operation at desktop, portrait-phone, and landscape-phone sizes in Chromium, plus both phone sizes in WebKit. The published package contains only index.html, index.js, index.wasm, input.js, storage.js, style.css, the no-Jekyll marker, and revision.txt. Core-test binaries and test diagnostics are excluded.

The deployment job uses GitHub's Pages artifact service and its github-pages environment. Only that job receives Pages and identity-token write permissions. The website stays at the repository path /meu/; assets use relative URLs.

After deployment, a live verification job checks revision.txt against the source commit and launches the public page and verifies saved journeys in all five browser/layout configurations. Reports and screenshots are retained as the live-pages-checks artifact. A deployment or successful check does not mark gameplay as user-accepted.

## Play and update

Select Begin exploration. M0d adds complete touch contexts, session settings and explicit pause/resume to the connected 240 m home and M0c physical bridge piloting. Approach the central console and use E or the touch prompt. At the helm W/S drives, A/D turns, Space brakes, P parks and E releases control. Mouse/arrow/touch look does not steer. Releasing drive brakes; leaving the helm requests a parked stop. Touch controls adapt to drive/turn with Brake, Park and Release helm buttons. The bridge reports speed, heading, braking distance and any structure/support rejection.

On foot, WASD or touch buttons walk through the bridge, quarters, engineering hall and lift. The lift is behind the start position: walk into its centre, choose 1/2/3 or tap a deck, then walk forward out after arrival. E or a contextual button uses nearby home objects. Settings or F1 opens sensitivity, vertical drag inversion, separate walking bob/machine sway (both initially off) and sound volume. Close settings, then explicitly resume. Focus/background loss and screen changes pause and discard old gestures. Pause and select Saved journeys, or press F2 on native desktop. Save after the unit is parked and the lift has finished; settings are included. A valid latest journey restores paused on the next launch. Recover previous retains damaged source data. Export/Import uses portable .meu files; Import changes the running journey and requires a separate Save to keep it locally.

Future source changes pushed to main rebuild, test, and redeploy automatically. Documentation-only commits do not rebuild the runtime. Use the repository's Pages settings with GitHub Actions as the publishing source.

To run the existing launch checks against the public site, set MEU_RELEASE=1 and MEU_BASE_URL=https://stbange.github.io/meu/, then run:

~~~text
npm run test:web -- --grep "renders the C/OpenGL room|persists settings|persists a saved journey"
~~~

The base URL must include its final slash. The tests retain that path and do not start a local server when MEU_BASE_URL is supplied.

## Verified publication

Published on 3 November 2026 from [M0e source commit b373388](https://github.com/stbange/meu/commit/b3733881d3c7886c4160f51e4629983d6ca22f93). All six [publishing jobs](https://github.com/stbange/meu/actions/runs/41703685599) passed: specifications, Windows, Linux, web, deployment and live verification. Runtime identity is 0.0.5, scene m0e-saves-v1, content version 5; save schema and authored-world compatibility are both version 1.

The public site passed ten cases: launches and durable saved-journey/reload/export/import operation in desktop/portrait/landscape Chromium and portrait/landscape WebKit, with diagnostics absent. An independent HTTP check confirmed the exact source revision, journey menu and storage backend, bounded data validation and a valid application/wasm response/header. See [M0e evidence, report and captures](evidence/m0e-saves.md). Physical-phone/file-picker review, performance and user acceptance remain pending.

Earlier publications remain in [M0d evidence](evidence/m0d-controls.md), [M0c evidence](evidence/m0c-piloting.md) and [M0b evidence](evidence/m0b-interior.md). Documentation-only commits after the M0e source revision do not rebuild the runtime.
