# MEU on GitHub Pages

Publication target: [the web prototype](https://stbange.github.io/meu/). The initial deployment is being configured; verification is recorded here after it completes.

## Build and publish

The MEU checks and Pages workflow publishes pushes to main after all four foundation jobs pass: specifications, Windows, Linux, and web. Working branches and pull requests run checks without publishing. The workflow can also be started manually for main.

The web job builds the optimized C/WebAssembly/WebGL 2 client, runs compiled core/scenario tests, and checks browser launches at desktop, portrait-phone, and landscape-phone sizes. The published package contains only index.html, index.js, index.wasm, input.js, style.css, the no-Jekyll marker, and revision.txt. Core-test binaries and test diagnostics are excluded.

The deployment job uses GitHub's Pages artifact service and its github-pages environment. Only that job receives Pages and identity-token write permissions. The website stays at the repository path /meu/; assets use relative URLs.

After deployment, a live verification job checks revision.txt against the source commit and launches the public page in all three browser layouts. Reports and screenshots are retained as the live-pages-checks artifact. A deployment or successful check does not mark gameplay as user-accepted.

## Play and update

Select Begin exploration to start the room and sound. Use WASD and mouse drag/arrows, or touch movement buttons and the look pad. The current public build is the M0a platform prototype; the connected interior, mech steering, and saves are later work.

Future source changes pushed to main rebuild, test, and redeploy automatically. Documentation-only commits do not rebuild the runtime. Use the repository's Pages settings with GitHub Actions as the publishing source.

To run the existing launch checks against the public site, set MEU_RELEASE=1 and MEU_BASE_URL=https://stbange.github.io/meu/, then run:

~~~text
npm run test:web -- --grep "renders the C/OpenGL room"
~~~

The base URL must include its final slash. The tests retain that path and do not start a local server when MEU_BASE_URL is supplied.
