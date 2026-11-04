# Pip, the onboard flight guide

Keep the first-person world visible behind an optional reader. A compact hovering robot badge opens help through a mouse/touch button or H/F3. The browser uses an accessible dialog, a topic selector and previous/next controls; native uses the same C content with wrapped text and keyboard/mouse controls. Pip is a guide projection in the interface, not a physical companion with collision or autonomous repair powers. Respect reduced-motion preferences in the browser.

A pure C guide module reads production state and returns authored advice. Prioritize suit emergencies and explicit recovery, then docking, power/repair prerequisites, salvage, shelter, bypass, testing, installation and departure. Include low-energy, heat and missing-parts advice. General topics explain the game, walking/lifts, piloting, infrastructure, suit work and durable journeys. Browser adapters read the same content exports; help needs no external service or network request.

Opening help pauses, releases helm/mouse capture and clears input through the existing context protocol. Append a transient help context after existing IDs. Closing leaves play paused until explicit resume and a fresh capture click. Focus changes keep the reader paused. Other menus close help; saved journey bytes/schema stay unchanged. Reading or changing topics cannot execute actions or advance suit/repair/power clocks.

Verify progression and read-only behavior in core tests, frozen resources and fresh input in the real SDL lifecycle replay, and mouse/touch/keyboard help interaction in debug and optimized Chromium/WebKit, including portrait/landscape layout and reload.
