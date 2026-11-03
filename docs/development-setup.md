# MEU Development on the Shared Computer

Use the same Windows computer with two sibling folders and two independent projects:

```text
C:\Users\steph\OneDrive\Dokumente\ChatGPT\
  WanderingCastle\  -> github.com/stbange/WanderingCastle
  meu\              -> github.com/stbange/meu
```

MEU is a fresh clone of its own repository, not a subfolder, submodule, or worktree of Wandering Castle. The computer and installed programs can be shared. Project state stays separate.

## Open the independent project

In the desktop app, add a separate local project named **Mechanized Exploration Unit** and select `C:\Users\steph\OneDrive\Dokumente\ChatGPT\meu` as its only folder. Start future MEU chats from that project. Keep Wandering Castle attached only to its own project. There is no need to add MEU as an extra folder in the Wandering Castle project.

The current migration chat began in Wandering Castle; a new MEU chat should read this repository's README, AGENTS.md, implementation plan, status, and foundation proposal. Keeping those decisions in the repository makes the new chat independent of this conversation's history.

For a terminal-based workflow, start in the MEU folder:

```powershell
Set-Location -LiteralPath 'C:\Users\steph\OneDrive\Dokumente\ChatGPT\meu'
codex
```

The [official project documentation](https://learn.chatgpt.com/docs/projects) explains primary folders and separate project contexts. If accessing the computer remotely, keep using the same connected computer and select the MEU project; the host connection and the project are different choices. See the [official remote access documentation](https://learn.chatgpt.com/docs/remote-connections).

## Git boundaries

MEU has a fresh Git history and one origin: `https://github.com/stbange/meu.git`. Its initial planning baseline is on `main`. Begin implementation on an independent branch, for example:

```powershell
git switch -c codex/m0-build-foundation
```

Use the MEU repository's status and remote before committing. Commit and push each completed implementation step to its working branch. Keep its issues, pull requests, release tags, and verification artifacts in `stbange/meu`. Do not add the other game's remote, copy its Git database, or reuse its worktrees. GitHub credentials may be shared through the computer's normal credential manager.

The old concept files are removed from Wandering Castle after the standalone copy is published. Historical commits remain intact; no history rewriting is needed. The MEU repository is the current source of truth.

## Tooling and build boundaries

Git, Node, editors, and compiler installations can be shared. Versions and configuration are selected by MEU and recorded here. OpenSpec is pinned independently in this repository's package.json and package-lock.json; run its commands from this folder. Node/OpenSpec are authoring tools, not shipped runtime dependencies.

```text
npm ci --ignore-scripts
npm run spec:validate
```

Use MEU-local `node_modules/`, ignored `.tools/` or an explicitly configured SDK path, and `build/` subdirectories for Windows, Linux, web, and tests. Do not resolve SDL, assets, scripts, or Node packages through a sibling game folder. Do not change global tool settings merely to make one game build.

At migration, the machine exposes Git 2.55.0.windows.5, Node 24.18.0, and npm 11.6.1 on PATH. CMake, Ninja, a C compiler, and Emscripten were not found on PATH during the initial check. That does not establish that they are absent from the computer. M0a must inspect existing installations, choose compatible versions, and document setup before installing anything new. The native/browser build commands do not exist yet.

Windows is the primary local development environment. Use independent Linux CI for native portability from M0a. A later local WSL workflow, if useful, uses its own MEU checkout/build tree and the same GitHub repository; installing WSL is not required by this planning setup.

Reserve development server port 4174 for MEU by default when the web serve command is added. Allow a per-process override and document it. Do not reuse a running sibling game's server or alter its ports. Ignore build output, downloaded toolchains, test captures, and machine-specific settings in Git.

## Saves and browser data

Use `SDL_GetPrefPath("stbange", "meu")` for MEU user data and report the resolved location in diagnostics. Place test saves in temporary fixture directories rather than the player's data folder. MEU must never read or migrate another game's saves.

For the web, use MEU-specific storage and cache names, such as `meu-saves-v1` and `meu-assets-<content-version>`, and a dedicated persistent filesystem mount. Paths on a shared website do not by themselves isolate browser storage, so choose explicit names and test them. Keep save export/import self-contained. A browser development URL or port change may change the storage origin; do not mistake that for a lost save.

## Start implementation

The next requested runtime task should identify **M0a** in the [implementation plan](implementation-plan.md). A suitable first prompt is:

> Implement M0a from docs/implementation-plan.md. Read AGENTS.md, docs/status.md, and the foundation OpenSpec change. Establish reproducible C17/SDL/OpenGL native and web builds, a minimal first-person room, and the initial automated checks. Record evidence, commit, and push the completed step; leave user acceptance pending.

This setup creates the independent workspace and planning baseline. It does not install the game toolchain, implement gameplay, accept a milestone, or publish a playable release.
