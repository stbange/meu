# Working-branch Pages publication - 4 November 2026

The user explicitly requested publication and live checks from the working branch. The workflow now allows `codex/m1-authored-expedition` alongside main for push/manual events, and excludes pull requests, tags and other refs. All four existing build/test jobs must pass before Pages can deploy. The optimized runtime whitelist and revision.txt remain the published payload; only the deployment job has Pages/id-token write permissions.

The branch's entire workflow run, including its publication and live verification, is serialized with other runs of the same ref and is not cancelled mid-flight. The live job checks the exact source revision and runs the optimized Chromium/WebKit desktop/phone rendering, settings, journey and Pip cases, plus captured desktop mouse input. This does not mark M0/M1/Pip gameplay accepted.

## Local verification

Node 24.19.0 and the repository's locked yaml/OpenSpec tooling parse and validate the workflow. A local expression check passes 40 gate/event/ref cases, including allowed push/manual runs and rejected PR, PR-target, tag, scheduled and other-branch cases, plus dependency/permission/serialization checks. Nine active OpenSpec changes validate. Actionlint 1.7.7 passes with its optional external shellcheck/pyflakes integrations disabled. Its Linux release archive is verified against SHA256 `023070a287cd8cccd71515fedc843f1985bf96c436b7effaecce67290e7e0757`.

```bash
.tools/actionlint-1.7.7/actionlint -shellcheck= -pyflakes= .github/workflows/ci.yml
OPENSPEC_TELEMETRY=0 npm run spec:validate
```

Publication uses the existing `github-pages` environment. Its repository-side deployment branch policy is separate from the workflow condition. The connected GitHub tools cannot edit environment settings and no browser connection is available. The installed GitHub CLI has no working authenticated session. The existing environment and its protections were retained.

## Remote publication

[Run 41921946687](https://github.com/stbange/meu/actions/runs/41921946687) checked source `f317915d2b2e12eb8b61379935a995154eaa03f2` on `codex/m1-authored-expedition`. Runtime identity remains 0.1.2, scene `m1-tower-v1`, content 6, schema 2; the workflow change does not alter game code. All four required pre-deployment jobs passed on the first attempt:

| Check | Result |
| --- | --- |
| Strict OpenSpec | Nine active changes pass |
| Windows native | 16 groups pass |
| Linux native and sanitizers | Both runs of 16 groups pass; rendered tours, lifecycle, expedition and durable journeys pass |
| Debug and optimized WebAssembly | 15 groups pass in each mode |
| Focused captured mouse / Pip | Six / 11 browser cases pass |
| Complete desktop/phone browser routes | 77 pass, 21 deliberate target-specific skips |
| Optimized browser smoke checks | 16 pass, two deliberate mobile Pointer Lock skips |
| Stage and upload Pages runtime | Pass; [Pages artifact](https://github.com/stbange/meu/actions/runs/41921946687/artifacts/13367904051) created |
| Pages deployment | Rejected before starting the runner by the existing environment branch rule |
| Exact live revision and browser checks | Skipped because deployment was rejected |

The run's public annotation states: `Branch "codex/m1-authored-expedition" is not allowed to deploy to github-pages due to environment protection rules.` It also reports: `The deployment was rejected or didn't satisfy other protection rules.` No working-branch publication or live-check success is claimed; the previous public build remains in place.

## Remaining repository setting

In [Settings → Environments](https://github.com/stbange/meu/settings/environments), open `github-pages` and add the exact branch `codex/m1-authored-expedition` to its selected deployment branches. Preserve main, reviewers and other protections. The user has already authorized this publication; the blocker is the connected tools' inability to edit that setting.

After updating the rule, use **Re-run failed jobs** on the run above to retry deployment and its dependent live checks without rebuilding the passing jobs. The Pages artifact has one-day retention; if it has expired, start a new workflow run on the working branch instead. Record the deployed revision and live results here when they pass. Gameplay acceptance remains pending independently.

Current passing development artifacts: [Windows](https://github.com/stbange/meu/actions/runs/41921946687/artifacts/13368210601), [Linux](https://github.com/stbange/meu/actions/runs/41921946687/artifacts/13366029584), [optimized web and browser reports](https://github.com/stbange/meu/actions/runs/41921946687/artifacts/13368377333).
