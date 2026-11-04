## Why
The M1 playtest reports mouse drag instead of captured first-person look, and flickering surfaces where floors/structures overlap.
## What Changes
- Click the gameplay viewport to capture the mouse on native desktop and supported desktop browsers; look after button release and at window edges.
- Release on Escape, pause, focus loss and menus; keep touch look and a usable browser-denial fallback.
- Render exposed opaque box-union surfaces once, including the additive powered bridge, and draw route paint as depth-biased decals.
## Capabilities
### New Capabilities
None.
### Modified Capabilities
- `commander-experience`: add captured mouse behavior and stable surface ownership to the active foundation proposal.
## Impact
An M0/M1 playtest correction, not M2 generation. Collision geometry, save schema and world identity stay compatible. No release or Pages deployment is implied.
