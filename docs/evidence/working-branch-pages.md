# Working-branch Pages publication - 4 November 2026

The user explicitly requested publication and live checks from the working branch. The workflow now allows `codex/m1-authored-expedition` alongside main for push/manual events, and excludes pull requests, tags and other refs. All four existing build/test jobs must pass before Pages can deploy. The optimized runtime whitelist and revision.txt remain the published payload; only the deployment job has Pages/id-token write permissions.

The branch's entire workflow run, including its publication and live verification, is serialized with other runs of the same ref and is not cancelled mid-flight. The live job checks the exact source revision and runs the optimized Chromium/WebKit desktop/phone rendering, settings, journey and Pip cases, plus captured desktop mouse input. This does not mark M0/M1/Pip gameplay accepted.

## Local verification

Node 24.19.0 and the repository's locked yaml/OpenSpec tooling parse and validate the workflow. A local expression check passes 40 gate/event/ref cases, including allowed push/manual runs and rejected PR, PR-target, tag, scheduled and other-branch cases, plus dependency/permission/serialization checks. Nine active OpenSpec changes validate. Actionlint 1.7.7 passes with its optional external shellcheck/pyflakes integrations disabled. Its Linux release archive is verified against SHA256 `023070a287cd8cccd71515fedc843f1985bf96c436b7effaecce67290e7e0757`. The real deployment/live checks are pending.

```bash
.tools/actionlint-1.7.7/actionlint -shellcheck= -pyflakes= .github/workflows/ci.yml
OPENSPEC_TELEMETRY=0 npm run spec:validate
```

Publication uses the existing `github-pages` environment. Its repository-side deployment branch policy is separate from the workflow condition; the named working branch must be allowed if the environment restricts branches. The connected GitHub fetch tool does not expose environment settings. Actual environment/deployment results will be recorded here.
