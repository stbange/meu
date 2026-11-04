# MEU on GitHub Pages

Live prototype: [play Mechanized Exploration Unit](https://stbange.github.io/meu/).

## Build and publish

The MEU checks and Pages workflow publishes successful pushes and manual runs from `main` and the named working branch, `codex/m1-authored-expedition`, after all four required jobs pass: specifications, Windows, Linux, and web. Pull requests and other refs run checks without publishing. On 4 November 2026 the user explicitly authorized publishing the working branch so the current M1/Pip build can be tested online.

The same branch/event allowlist controls runtime staging, Pages artifact upload, deployment and live verification. Runs of the same workflow/ref are serialized through completion, including publication and live checks. An in-flight run is not cancelled by a new push. Both allowed branches publish to this one site; the latest successful deployment is the public build.

The web job builds the optimized C/WebAssembly/WebGL 2 client, runs compiled core/scenario tests, and checks browser launches and durable saved-journey/export/import operation at desktop, portrait-phone, and landscape-phone sizes in Chromium, plus both phone sizes in WebKit. The published package contains only index.html, index.js, index.wasm, input.js, storage.js, style.css, the no-Jekyll marker, and revision.txt. Core-test binaries and test diagnostics are excluded.

The deployment job uses GitHub's Pages artifact service and its github-pages environment. Only that job receives Pages and identity-token write permissions. The website stays at the repository path /meu/; assets use relative URLs.

After deployment, a live verification job checks revision.txt against the source commit and tests rendering, settings, durable journeys and Pip help in desktop/phone Chromium and phone WebKit, plus actual desktop mouse capture. Reports and screenshots are retained as the live-pages-checks artifact. A deployment or successful check does not mark gameplay as user-accepted.

## Play and update

The working build includes the complete authored M1 expedition, click-captured mouse look, stable floor/wall surfaces and Pip onboard help (runtime 0.1.2). Click/tap Help or press H/F3 for explanations and a next step based on your journey. Reading pauses play; close help and explicitly resume when ready. See [the expedition route](m1-expedition.md) and [current publication evidence](evidence/working-branch-pages.md).

Select Begin exploration and click the 3D view to capture the mouse; Esc releases and pauses. Approach the central console and use E or the touch prompt. At the helm W/S drives, A/D turns, Space brakes, P parks and E releases control. Mouse/arrow/touch look does not steer. Releasing drive brakes; leaving the helm requests a parked stop. Touch controls adapt to drive/turn with Brake, Park and Release helm buttons. The bridge reports speed, heading, braking distance and any structure/support rejection.

On foot, WASD or touch buttons walk through the bridge, quarters, engineering hall and lift. The lift is behind the start position: walk into its centre, choose 1/2/3 or tap a deck, then walk forward out after arrival. E or a contextual button uses nearby home objects. Settings or F1 opens sensitivity, vertical drag inversion, separate walking bob/machine sway (both initially off) and sound volume. Close settings, then explicitly resume. Focus/background loss and screen changes pause and discard old gestures. Pause and select Saved journeys, or press F2 on native desktop. Save after the unit is parked and the lift has finished; settings are included. A valid latest journey restores paused on the next launch. Recover previous retains damaged source data. Export/Import uses portable .meu files; Import changes the running journey and requires a separate Save to keep it locally.

Future source changes pushed to `main` or `codex/m1-authored-expedition` rebuild, test, and redeploy automatically. Documentation-only commits under docs/ or README.md do not rebuild the runtime. Use GitHub Actions as the repository's Pages publishing source. If the `github-pages` environment has selected deployment branches, it must allow the named working branch as well as main; reviewer and other protection settings remain applicable.

To run the existing launch checks against the public site, set MEU_RELEASE=1 and MEU_BASE_URL=https://stbange.github.io/meu/, then run:

~~~text
xvfb-run -a npm run test:web -- --grep "renders the C/OpenGL room|persists settings|persists a saved journey|click captures the desktop mouse|Pip explains the game"
~~~

The base URL must include its final slash. The tests retain that path and do not start a local server when MEU_BASE_URL is supplied. Xvfb supplies a desktop window for Pointer Lock cases on headless Linux; a normal desktop can run the npm command directly.

## Previously verified publication

Published on 3 November 2026 from [M0e source commit b373388](https://github.com/stbange/meu/commit/b3733881d3c7886c4160f51e4629983d6ca22f93). All six [publishing jobs](https://github.com/stbange/meu/actions/runs/41703685599) passed: specifications, Windows, Linux, web, deployment and live verification. Runtime identity is 0.0.5, scene m0e-saves-v1, content version 5; save schema and authored-world compatibility are both version 1.

The public site passed ten cases: launches and durable saved-journey/reload/export/import operation in desktop/portrait/landscape Chromium and portrait/landscape WebKit, with diagnostics absent. An independent HTTP check confirmed the exact source revision, journey menu and storage backend, bounded data validation and a valid application/wasm response/header. See [M0e evidence, report and captures](evidence/m0e-saves.md). Physical-phone/file-picker review, performance and user acceptance remain pending.

Earlier publications remain in [M0d evidence](evidence/m0d-controls.md), [M0c evidence](evidence/m0c-piloting.md) and [M0b evidence](evidence/m0b-interior.md). Documentation-only commits after the M0e source revision do not rebuild the runtime.
