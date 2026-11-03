# Mechanized Exploration Unit

## Project boundaries

- Work in this repository and use its own origin, dependencies, documentation, tests, assets, and output folders. Do not rely on a sibling game checkout.
- Keep C17, SDL, direct OpenGL, Windows/Linux, and mobile-first browser support as the implementation direction. Gameplay remains exclusively first person from the sole human Commander's perspective.
- Read `docs/implementation-plan.md`, `docs/status.md`, and the relevant OpenSpec change before implementation. Follow dependency order; begin with M0a when runtime work is requested.
- The game design is a draft. Do not convert proposed tuning, features, or acceptance criteria into claims of implemented or user-accepted behavior.

## Repository workflow

- After each completed implementation step, commit the changes and push the commit to the working branch. Do this even when user acceptance is still pending.
- Record pending acceptance accurately. A commit or push does not mark a playtest or milestone as user-accepted.
- Use a `codex/` prefix for new working branches. Do not force-push or rewrite published history without a specific request.
- Stage only the files belonging to the completed step; preserve unrelated work.

## Verification and records

- Keep behavioral requirements and observable failure cases in OpenSpec changes; keep algorithm and platform choices in design artifacts.
- Add appropriate unit and game checks with the behavior they verify. Test against the real input/update paths, including touch, lifecycle interruption, and durable saves where relevant.
- Run `npm run spec:validate` after specification changes. Add native/web build and test commands in M0a, then run the checks applicable to each implementation step.
- Record reproducible commands, versions, device/scene identity, results, limitations, and pending user acceptance in `docs/evidence/` as implementation begins.
- Keep `docs/status.md` current. Update the plan and owning scenarios when scope changes. Archive a change only after its required checks and recorded acceptance conditions are satisfied.
