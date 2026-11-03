# Mechanized Exploration Unit Game Design Draft

Draft 1, 3 November 2026. User review is pending; no game implementation or playtest is claimed. Development is organized by the [implementation plan](implementation-plan.md).

## The experience

You are the sole human aboard a walking machine the size of a skyscraper. It is your vehicle, workshop, observatory, and home. Through the bridge windows, a doorway rises several hundred metres above you. Your robot steps through it, and you realize that the skyline you have been approaching is furniture inside a much larger building.

You explore the remains of an absent civilization, learn how its machinery works, and rebuild your own machine with what you recover. A successful expedition changes both your understanding of the city and the places your robot can reach. Between expeditions, you walk its interior, repair its systems, direct small maintenance robots, and make a place for yourself inside an enormous body.

The intended mood is solitude, curiosity, mechanical weight, and the relief of coming home. Environmental danger provides tension. Discoveries and practical engineering provide progress.

## Requirements and proposed direction

The requested foundation is exclusively first-person play from the Mech Commander's perspective; a gigantic, steerable robot with explorable maintenance spaces and living quarters; no other people; robotic repair assistants; sunlight and alien-grid power; salvage, crafting, and technology integration; restricted vac-suit excursions; unusual procedural alien ruins and landscapes; retro graphics and sound; C with SDL and OpenGL; Windows, Linux, and the web, with a mobile-first web interface; OpenSpec planning; and unit plus automated game testing.

The scale measurements, campaign structure, resource model, hazards, and progression below are proposed defaults. OpenSpec is the clarified planning requirement. Proposed defaults remain open for revision through milestone review.

## What makes the game worth playing

- **Live at three scales.** A corridor feels large to you, a city corridor feels large to your robot, and both occupy a landscape that dwarfs them. Move between those scales without leaving your character's eyes.
- **Learn an alien system.** A strange device has a discoverable purpose and a consistent behavior. Understanding it should be as valuable as taking it.
- **Bring discoveries home.** Salvage creates new actions: cross an unsupported span, recover power from a dormant network, see through a mineral storm, or open a previously unusable interior sector.
- **Care for a machine you know.** Recognizing a failing coolant pump by its sound is satisfying because you have walked past it, repaired it, and chosen what it powers.
- **Choose the next expedition.** Routes offer different rewards, power opportunities, hazards, and views. The player has enough information to make a plan and enough uncertainty to be surprised.

## The robot and the feeling of size

Start with one roughly 240-metre-tall bipedal exploration unit. A human is about 1.8 metres tall. Alien public passages are hundreds of metres high, while major structures reach several kilometres. These are initial art and traversal targets, not a demand to simulate every cubic metre.

The accessible interior is a connected set of recognizable places embedded within the much larger machine:

| Space | Player activity | How it expresses scale |
| --- | --- | --- |
| Bridge | Steer, scan, plan routes, operate salvage arms | Human controls sit before windows overlooking immense streets |
| Residential compartment | Sleep, save, furnish, inspect collected objects | A small home occupies a corner of a cavernous chest bay |
| Engineering hall | Diagnose faults and reroute power | A walkway crosses machinery several houses tall |
| Workshop and salvage cradle | Fabricate adapters and install recovered systems | A recovered component can be larger than your living quarters |
| Drone service bay | Assign repair priorities and upgrade helpers | Small repair robots disappear into distant maintenance trunks |
| Observation gallery | Inspect landmarks and the robot's own structure | Human railings frame a view down past the knee assemblies |
| Wrist or lower-body airlock | Board a protected exterior platform | The deployment lift reveals the full distance to the ground |

Begin with four accessible spaces; unlock the others through repairs. Huge unoccupied volumes remain visible beyond windows and service grilles. Familiar signs, stairs, door handles, and furniture keep human scale legible.

Express weight through distant footfalls, flexing mechanisms outside the habitable shell, slow starts and stops, and passing architecture. The character's floor and horizon stay stable. Head bob, camera shake, and gait effects have independent controls, including off. An express lift and a compact service spine keep ordinary trips between useful stations short; the optional long route supplies atmosphere.

All steering, construction, maintenance, and discoveries remain first person. Maps appear on bridge instruments or a handheld display. There is no external piloting camera, detached building camera, or overhead management mode.

## The expedition loop

Aim for a satisfying 20–40 minute expedition, with useful stopping points for shorter sessions.

1. **Observe.** From the bridge, identify a landmark, a possible power source, and an apparent route. Scans reveal clues and measurable hazards rather than every reward.
2. **Prepare.** Choose installed equipment, reserve power, and repair priorities. Fabricate a tool or adapter if the destination calls for one.
3. **Travel.** Steer the robot through streets, halls, and terraces. Negotiate clearance, unstable surfaces, shadow, and energized infrastructure.
4. **Secure the site.** Park on suitable supports, establish a power connection or deploy collectors, and investigate the local systems.
5. **Explore and recover.** Use the robot's manipulators for large salvage. Where conditions permit, enter a sheltered service space in a vac suit for precise work.
6. **Integrate.** Diagnose the discovery, build its interface, install it, and test what it does. Record clues and choose a destination that the new capability makes interesting.

Every major expedition should deliver a practical gain, a meaningful clue, or access to a new kind of place. A failed attempt should still teach a route, hazard, or machine behavior.

## A sample expedition

You spot a tower whose dark rings brighten when sunlight reaches the canyon wall. The bridge identifies a usable grid socket at its base, but the direct route crosses a broken causeway. You take the longer route through an acoustic hall, where walking too quickly excites old structural resonators.

At the tower, your grid adapter detects repeated power surges. Connecting without preparation would overload the workshop bus. You park, walk to engineering, isolate that bus, and route the connection through a sacrificial buffer. Repair drones install the ordinary wiring while you select and test the configuration.

The tower's outer machinery opens a sheltered recess. Its temperature and radiation are now within the suit's limits, and your wrist airlock can reach a service ledge. Outside, you pass an alien fastener larger than your kitchen and enter a maintenance conduit used by small machines.

Inside is a phase coupler. Removing it immediately would shut the recess's sheltering field. Instead, you install a bypass, recover the coupler, and return through the maintained safe route. Your workshop turns it into a grid stabilizer. The socket now supplies steady power, and a deeper district that previously looked impractical becomes a sensible next trip.

## Steering and power management

Bridge controls issue forward/reverse speed, heading, and stop commands. The unit handles individual footsteps and chooses supported foot placements. The challenge is judging a route and operating the machine, rather than manually balancing every joint. Releasing the helm requests a controlled stop; boarding and egress require a fully parked state. A later heading-hold upgrade follows a short validated segment, stops at uncertainty, and lets you move inside during travel.

Large manipulators use simple first-person aiming, alignment guides, and deliberate actions. They recover assemblies, position a connector, or brace against suitable architecture. Bespoke interactions provide more value than unrestricted destruction of every building.

Manage a small set of readable systems:

| System | Main decision |
| --- | --- |
| Stored energy | Travel farther now, or keep capacity for scanning, salvage, and return? |
| Heat | Work quickly and build heat, or reduce load and cool? |
| Component condition | Repair a fault now, isolate it, or finish the task with reduced capability? |
| Fabrication stock | Spend common material on repairs, adapters, drones, or an upgrade? |

Solar collectors provide dependable recovery in exposed locations. Deployable back sails increase collection while parked and create a visible change to the robot. Grid connections provide faster or more sustained power, but require identifying the supply, adapting its output, and understanding what else that circuit controls. Deep enclosed districts make the grid particularly valuable.

Energy planning shows predicted consumption and uncertainty before departure. Navigation protects a return reserve, and the generator must provide a viable route between recovery locations. Solar recovery is paced so that a useful top-up overlaps a short workshop or home activity; repeated empty waiting is a design failure. Electrical energy, heat, and material stock remain distinct: sunlight does not manufacture repair metal.

Life support and essential doors use an isolated reserve. An ordinary subsystem failure cannot instantly kill the player in bed or seal the only route to the bridge. Closed or paused games consume no resources.

## Repairs and robotic assistants

Faults should follow understandable causes: overloading a bus, walking through abrasive material, operating a hot assembly, or integrating an unfamiliar component. Avoid frequent random breakdowns and routine wear that demands the same chore after every trip.

A repair has four useful decisions: identify the symptom, locate the cause, isolate or bypass the affected system, and replace or modify the component. For example, a dim bridge display and a slow salvage arm may share one damaged distribution unit. Replacing the display alone will not solve the fault.

Repair robots handle repetitive work, move stock through the service network, and report blocked jobs. You set priorities such as mobility, cooling, or fabrication. They need parts and access, so they cannot solve an unknown alien device on their own. Show queues, missing parts, and estimates; avoid controlling individual workers constantly.

Keep the robots mechanically expressive through lamps, sounds, movement, and recognizable work habits. They provide companionship without introducing other people or a conversational crew. Decorating the residence, displaying artifacts, and watching a familiar helper return make quiet time worthwhile. Food and sleep are home activities and optional preparation, rather than additional lethal meters.

## Salvage and alien technology

Use two scales of recovery: human-carried diagnostic parts and artifacts, and huge assemblies hauled into the robot's salvage cradle. The machine can recover bulk supplies without requiring a suit trip at every stop.

Technology moves through **observe, test, adapt, install**. A controlled workshop bench exposes an unknown device's inputs and outputs without risking the entire robot. Once understood, familiar recipes become quick fabrication jobs. Initial common stocks are structural alloy, conductor stock, and sealant; distinctive alien cores remain individual components.

Installation uses authored sockets and a simple power/cooling connection model. It shows available versus required stock, capacity, and the resulting tradeoffs before committing. Prototype adapters are removable, and the basic tools needed to understand the first technology are available from the start.

| Discovery | New possibility | Tradeoff |
| --- | --- | --- |
| Grid stabilizer | Use an unstable district supply | Uses an equipment slot and rejects some high-output pulses |
| Structural resonance scanner | Detect hidden supports and unstable floors | A scan needs energy and briefly requires standing still |
| Load-distribution feet | Cross selected damaged or soft surfaces | Higher movement energy cost |
| Field-compatible suit lining | Enter newly viable service recesses | Needs charging and more workshop preparation |
| Heat-to-charge lattice | Recover part of discarded heat | Occupies cooling capacity and cannot create perpetual energy |
| Maintenance relay | Let helpers service distant internal sectors | Needs signal access and recovered control components |

Progression expands the player's verbs and route options. Avoid a long ladder of numerically stronger versions of the same item. Recovered technology also changes the interior visibly, opening a compartment or adding machinery the player can walk around.

## The alien city and its landscape

Place the city across a vast tectonic rift. Exposed upper terraces receive sunlight. A mineral canopy spans parts of the canyon, with districts suspended beneath it. Lower structures descend into an ancient seabed and thermal machinery. Routes run above, inside, and below the city; landscape and architecture explain one another.

| District family | Architecture and landscape | Distinct play |
| --- | --- | --- |
| Heliostat Courts | Rotating stone mirrors and hollow towers on sunlit cliffs | Redirect light to charge the robot or wake a mechanism |
| Suspended Galleries | Buildings hang from the canopy above a deep chasm | Read support structures and find safe crossings |
| Acoustic Halls | Ribbed vaults amplify motion and weather | Choose stride speed, damp mechanisms, follow sound clues |
| Inverted Gardens | Mineral growth hangs above dry channels and thermal vents | Redirect flow and heat to reveal service routes |
| Buried Transit | Monumental tunnels pass through sediment and collapsed foundations | Restore local grid segments and unlock shortcuts |
| Meridian Archive | An observatory built into the rift's deepest structural spine | Combine earlier discoveries to reconstruct the city's history |

Use recurring alien construction principles, such as suspended loads, three-part sockets, resonant materials, and fluid circuits. Players learn to interpret the strangeness. Small service spaces exist for ancient maintenance machines, which explains human-scale access inside colossal architecture.

Environmental threats include electrical surges, dust fronts, moving infrastructure, weak floors, heat, and automated systems still doing obsolete work. The initial design has no mandatory combat system. Encounters revolve around observation, avoidance, isolation, or repurposing machinery. The aliens remain absent.

## Leaving the robot

A vac suit permits survival only within supported combinations of pressure, temperature, radiation, debris, and electrical conditions. Exits require a parked robot, a reachable landing, and a scanned route that stays within the suit's limits. These are physical conditions, not arbitrary mission markers.

Eligible places include sheltered grid-service chambers, shielded terraces, and recesses activated by restored machinery. Most of the city's exposed volume is lethal or inaccessible on foot. The bridge explains the blocking condition and what technology or local repair could change it.

Suit excursions are short, deliberate explorations with precise interactions, route finding, and discoveries the robot's hands cannot manipulate. A return indicator and safety margin make the available time readable. The robot stays parked; helpers preserve the access route. Predictable changes give enough warning to retreat, and removing a component that powers shelter is clearly signaled.

Standard mode provides an explicit recovery option to the last secure checkpoint after a catastrophic mistake, losing progress and unsecured salvage since that checkpoint. It never silently teleports the player or deletes the established home. Manual saves remain available outside an active transaction. More punitive expedition rules can wait until the core experience is proven.

## Procedural generation and replayability

Generate a bounded city for each campaign, initially targeting five or six substantial districts. A smaller dense world with meaningful changes is preferable to an indefinitely repeated city. Each seed changes geography, routes, landmark assemblies, local problems, salvage placement, and optional discoveries.

Generation starts with a progression graph: starting shelter, energy recovery sites, district connections, technology opportunities, and the final archive. It then builds the rift, large structures, traversable routes, expedition sites, and decoration in that order. Giant architecture and service-scale spaces share one spatial model.

Use authored architectural grammars and puzzle components inside generated layouts. A heliostat court can have different orientations, damaged equipment, access paths, and rewards while retaining understandable rules. Fixed physical rules make experiments useful across locations. A small selection of coherent technology combinations changes each campaign's preferred solutions.

Validators must check mech clearance, supported parking, progression reachability, obtainable repair stock, power recovery, and a safe suit return for mandatory foot sites. No required tool may sit behind its own prerequisite. A bounded retry process falls back to a known viable layout when generation fails. Checksums, seed, and generator version make failures reproducible.

New districts introduce new architectural logic and interactions, not only new colors. Budget discoveries so that several nearby landmarks do not repeat the same encounter. Players can share seeds, but knowledge of a shared seed is explicitly part of that experience.

## Campaign progress and home life

The campaign goal is to reach the Meridian Archive and learn why the city persists after its builders have gone. Key discoveries are physical evidence: routing patterns, structures converted from one purpose to another, and recorded machine states. A consistent authored history survives procedural rearrangement.

Progress moves from restoring the unit's basic systems, through learning to use local infrastructure, to reaching the deep city. The final act uses several learned technologies to make a major restoration decision. That decision should have visible, comprehensible effects on power and accessible routes. Details of the disappearance and ending remain writing work for a later milestone.

After the ending, the player can continue exploring the generated city. A new campaign offers different geography and technology combinations. Player knowledge carries over naturally; permanent account bonuses should not bypass the early discovery loop.

Home improvements are useful and expressive: a better workbench, quieter cooling, a repaired panoramic gallery, artifact shelves, more capable helpers, and restored living space. Preserve familiar rooms as the machinery evolves. Calm intervals are a reward for good preparation.

## Retro presentation and mobile play

Use restrained low-polygon 3D, small readable textures, simple materials, atmospheric fog, and strong silhouettes. Interiors use functional industrial shapes and warm practical lighting. The alien city uses unfamiliar structure, weathering, and material combinations without requiring ornate surface detail. Retro presentation does not mean tiny unreadable text, compulsory distortion, or heavy screen noise.

Sound sells size: slow actuators, distant impacts through the frame, contactors, cooling circuits, and silence outside. Sparse synthesized music makes major discoveries stand out. Important alarms also have clear visual cues and captions.

Design for a phone from the first prototype. Landscape is the primary play layout; portrait retains the full first-person viewport and rearranges its controls and station panels. On foot, use a left movement area, right look area, and large context-sensitive interaction button. At the helm, replace movement controls with heading and speed controls. Tap selects a valid nearby tool target; precision aiming is optional. Offer left-handed placement, adjustable sensitivity, toggle actions, and readable text scaling.

Consoles present one task at a time. A device-sized panel may enlarge for reading while retaining the visible room around it. Touch targets start around 48 CSS pixels, and the layout is checked at 360 by 640 CSS pixels as well as landscape equivalents. No action depends on hover, pointer lock, or a physical keyboard. Desktop mouse/keyboard and gamepad inputs map to the same actions.

Pause on backgrounding or focus loss, cancel held inputs, and offer an explicit resume. Start browser audio after a player gesture. Keep the game playable without fullscreen. The initial download should include the first district and shared assets; later content can arrive in small packs.

## Technical direction

Use C17, SDL3 for platform/input/audio/windowing, and a direct OpenGL renderer. Windows and Linux use native executables; Emscripten builds the C game to WebAssembly for browsers. SDL documents this source-build path in its [Emscripten introduction](https://wiki.libsdl.org/SDL3/INTRO-emscripten).

Target the WebGL 2 compatible OpenGL ES 3.0 subset. Desktop can use OpenGL 3.3 core with equivalent shader variants; keep features inside the common supported subset. Avoid legacy immediate-mode emulation and desktop-only rendering dependencies. This follows the efficient path described in [Emscripten's OpenGL guidance](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html).

Separate simulation, generation, persistence, input commands, and presentation. A fixed simulation tick owns gameplay state. The renderer reads it; it cannot grant resources or repair components. Begin with a single-threaded web build and incrementally budget generation work, so web deployment does not require shared-memory threading headers. CMake manages the native, headless-test, and Emscripten targets. Pin tool and library versions when the foundation starts.

Keep the interior in robot-local coordinates and transform the exterior view consistently. Walking, turning, lifts, and docking must agree on the same transforms. Use simple supported-foot navigation and capsule collision rather than full-body rigid-body simulation. Stream distant city chunks with level of detail, instancing, and occlusion; keep only nearby interior sectors active. Rebase rendering coordinates to preserve precision in kilometre-scale scenery. The playable interior must match the visible hull and apertures.

Save seed, generator/content versions, visited-world changes, discoveries, installed technology, inventories, faults, robot pose, and player-local pose. Preserve previous saves during publication and version migrations. Browser saves require persistent storage and asynchronous completion handling; writing only to the in-memory filesystem is insufficient, as explained in [SDL's browser storage documentation](https://wiki.libsdl.org/SDL3/README-emscripten#automount-persistent-storage). Provide export/import and an honest saving/saved/failed indicator.

Initial measurement targets are 30 fps on a selected midrange Android phone and iPhone, 60 fps on a selected ordinary desktop, and less than 256 MiB of WebAssembly linear memory during the slice. Graphics allocations are measured separately. These are budgets to verify, not established performance claims. Select and record actual reference devices during the foundation; browser emulation cannot establish hardware performance.

## Testing and OpenSpec planning

OpenSpec change folders contain a proposal, behavioral scenarios, technical design, and unchecked tasks. The [first proposed foundation change](../openspec/changes/establish-playable-foundation/proposal.md) is included with this draft. Main specifications describe implemented, verified behavior only. A commit or push does not mark a milestone or playtest as user-accepted. This organization follows the [OpenSpec change workflow](https://github.com/Fission-AI/OpenSpec#see-it-in-action).

Test through several layers with different purposes:

| Layer | Main evidence |
| --- | --- |
| C unit tests | Resource conservation, heat and fault transitions, recipe transactions, suit eligibility, transforms, serialization, invalid commands |
| Generator tests | Fixed seeds plus thousands of generated layouts; clearance, power, prerequisites, return routes, deterministic identity |
| Headless game scenarios | Real command/update loop for steer, park, charge, diagnose, salvage, install, and reload; asserted state and failure reasons |
| Rendered game scenarios | Actual player input, collision, bridge use, moving interiors, arm/airlock alignment, and persistence through the running client |
| Browser automation | Load the real WebAssembly build; complete a touch-only route; rotate, resize, background, resume, and reload saves; test storage failures and graphics-context recovery |
| Performance and human playtests | Streaming and long-session memory budgets; real phone interaction, motion comfort, perceived size, discovery quality, and repair enjoyment |

Use Playwright for the browser suite, with browser projects and mobile viewport configurations as described in its [official documentation](https://playwright.dev/docs/test-projects). Emulated mobile tests are supplemented with real Android Chrome and iPhone Safari checks. A canvas needs game-state assertions through a test-only bridge, plus genuine input tests; successful page loading is not a successful game test.

Replay logs include seed, versions, commands, and simulation ticks. Use integer quantities for inventory and power accounting where useful, and stable generation algorithms with fixed-width arithmetic. Verify selected generated identities across native and web builds. Do not assume arbitrary floating-point simulations will produce identical hashes across platforms. Render checks use tolerances and targeted scenes rather than universal pixel-perfect screenshots. Run sanitizers on suitable native builds, fuzz save decoders and content inputs, and retain failure replays and screenshots.

Every implementation change adds tests for its own observable behavior. Build and run core tests on Windows and Linux and the browser smoke suite on each change. Run larger seed sweeps, long travel sessions, and performance measurements on scheduled or release checks. Report automated verification and user acceptance separately.

## Proposed delivery milestones

| Milestone | Playable outcome | Completion evidence |
| --- | --- | --- |
| M0 Moving home and platform proof | Four connected spaces, first-person bridge steering, stable interior, safe stop, basic saves, desktop and phone browser controls | Native/web build checks, motion/input/save scenarios, user scale and comfort review |
| M1 One complete expedition | An authored route with sunlight, one grid puzzle, one fault, a repair helper, one suit recess, and one installed discovery that opens an alternate route | Automated expedition and failure recovery; user review of the complete loop |
| M2 A generated small city | Three district families, seeded routes and sites, generation validation, persistent mutations | Seed sweeps, progression and reload tests, user review of variation |
| M3 A complete exploration arc | Five or six districts, several distinct technologies, home restoration, archive objective and ending | End-to-end campaign scenarios and user review of progression and novelty |
| M4 Release preparation | Supported Windows/Linux packages and a polished mobile web build | Device matrix, performance/soak evidence, accessibility and save recovery checks, release acceptance |

M0 should prove the risky experience before a large content investment. M1 should prove that a discovery changes how the player plays. M2 should prove that generation produces meaningfully different expeditions. All milestones are currently proposed and unimplemented.

## Scope discipline

Keep one robot body, modular installation sockets, bounded campaigns, environmental encounters, and a compact set of meaningful systems. Defer multiplayer, human crew, unrestricted block construction, arbitrary building destruction, full physical leg balancing, spaceflight, and a large combat tree. The main risks are repetitive maintenance, empty travel, opaque alien puzzles, weak scale cues, and phone usability. Each milestone's playtest must examine those risks before adding more content.
