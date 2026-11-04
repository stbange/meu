# Working-branch Pages publication

Use the same explicit main/working-branch condition for runtime staging, Pages artifact upload, deployment and live verification. A push or manual run for either named branch can publish only after specifications, Windows, Linux and web succeed. Pull requests and other refs cannot publish. Keep Pages/id-token write permissions on the deployment job and retain the existing github-pages environment.

Serialize each workflow/ref through completion, including deployment and its live checks, without cancelling an in-flight publication. The existing Pages deployment lock remains. Stage only shipping runtime files and revision.txt, verify that revision over HTTP, then run the existing optimized Chromium/WebKit desktop/phone launch, settings, durable-save, captured-mouse and Pip checks against the deployment URL. No game runtime, save schema or main-branch merge is required.

Repository environment branch restrictions are independent of workflow conditions. If the github-pages environment rejects the working ref, add only the explicitly authorized working branch to its allowed deployment refs, preserving reviewer and other protection settings. Record actual deployment and live-check outcomes.
