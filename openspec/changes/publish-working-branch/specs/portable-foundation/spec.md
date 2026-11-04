## ADDED Requirements
### Requirement: PORT-06 Verified working-branch publication
The Pages workflow SHALL publish successful pushes or manual runs from main and the explicitly authorized working branch, `codex/m1-authored-expedition`, after all required build/test jobs pass. It SHALL exclude pull requests and other refs, publish only the optimized runtime package, and retain its source revision.
#### Scenario: Publish a successful working build
- **WHEN** a working-branch run passes specifications, Windows, Linux and web checks
- **THEN** its optimized runtime and exact source revision are deployed to the repository Pages site
#### Scenario: Reject an unapproved or failed source
- **WHEN** a run is for a pull request, an unlisted ref or a failed required build/test job
- **THEN** it does not publish a Pages runtime
### Requirement: PORT-07 Verify the deployed working build
After publication, the workflow SHALL verify the deployed source revision and run optimized desktop/phone browser checks against the public URL, retaining reports. A newer run of the same branch SHALL not interrupt that deployment/verification sequence.
#### Scenario: Verify the live release
- **WHEN** a working build is deployed
- **THEN** revision.txt matches its source commit, and rendering, settings, journeys, captured mouse input and Pip help are tested against the deployed URL
#### Scenario: A new push arrives while verifying
- **WHEN** another run for the same branch is queued during publication or live checks
- **THEN** the in-flight run completes its checks before the next run can publish
