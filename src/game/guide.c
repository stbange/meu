#include "game/guide.h"
#include <math.h>
typedef struct { const char *title,*place,*action,*tip; } Card;
static const Card steps[]={
    {"Let's start on the bridge",
     "Bridge - 196 m",
     "Hi, Commander. I'm Pip, your flight guide. You live inside a 240-metre exploration machine. Our first expedition will recover alien technology and open a new route.",
     "Close help and begin exploration. Walk forward to the central bridge console and use E or its prompt to take the helm. I can explain the controls on the next pages."},
    {"Recover your secure checkpoint",
     "Wrist airlock - engineering, 112 m",
     "The suit has failed. Close help, resume, and choose Recover secure checkpoint at the airlock panel. You must choose recovery yourself.",
     "Recovery preserves your home but loses unsecured progress and salvage. Reading help never moves you or spends resources."},
    {"Return to the cabin now",
     "Follow the railings back to the wrist airlock",
     "Your suit supply is low, exposure is rising, or shelter is failing. Follow the rails through the airlock. Inside cabin X below 14, use Return to cabin / remove suit.",
     "Help pauses the suit clock while you read. Close it and resume when ready to return. Do not remove the suit outside."},
    {"Bring the coupler home",
     "Conduit to wrist airlock - engineering RIGHT wall",
     "Follow the same railings back through the airlock. At cabin X below 14, open the airlock controls and choose Return to cabin / remove suit.",
     "Keep shelter powered. Your recovered coupler is retained when you remove the suit safely."},
    {"Take off the suit safely",
     "Wrist airlock - inside the cabin",
     "You are back inside. Use the airlock console and select Return to cabin / remove suit. Then take the coupler to the workshop on the RIGHT of engineering.",
     "Removing the suit ends exposure and frees you to test and adapt the recovered technology."},
    {"A new route is yours",
     "Powered west service bridge",
     "You completed the authored tower expedition. Park and finish any lift ride, then open Saved journeys and save. You can explore the home or return to the tower.",
     "Keep the stabilizer installed while using its bridge support. Further city expeditions are not available in this build."},
    {"Rebuild your energy margin",
     "Engineering power console - LEFT side, 112 m",
     "Main energy is below the 4000 kJ return margin. Park and deploy collectors at the power console. If already deployed, leave them out while exploring the interior.",
     "Close help and resume to let sunlight charge the machine. Paused time does not charge it. Stow collectors before piloting again."},
    {"Let the machinery cool",
     "Engineering power console - LEFT side",
     "Heat is too high for the next operation. Select cooling priority, then close help and resume while heat drops. Suit preparation needs heat at or below 6000.",
     "Return to fabrication priority before making the adapter. Cooling and charging do not advance while help is open."},
    {"Park at the tower socket",
     "Bridge helm - 196 m",
     "Use the central console. Aim for robot X near 0, Z near -140 and heading 000. Drive north with W or Forward. Brake with Space, then park with P; touch has Brake and Park buttons.",
     "Read the helm coordinates. Docking bounds are X -15 to 15 and Z -175 to -120. Release the helm with E before taking the lift to engineering."},
    {"Isolate the workshop bus",
     "Engineering power console - LEFT side, 112 m",
     "Take the lift to engineering. At the LEFT power console, disconnect the tower grid first if connected, then choose Isolate workshop bus. Deploy collectors while parked.",
     "Doors, lift and life support stay available. Isolation protects the workshop and gives the repair helper safe access."},
    {"Connect the tower grid",
     "Engineering power console - LEFT side",
     "While parked at the tower socket with the workshop bus isolated, choose Connect tower grid. Keep collectors deployed during interior work.",
     "The supply pulses every 12 active seconds. Observe the pattern and inspect the maintenance helper if displays dim or the salvage arm slows."},
    {"Watch the pulsing supply",
     "Engineering power console and maintenance helper",
     "The isolated tower grid is connected. Close help and resume to observe its 12-second cycle. A surge can trip the buffer; inspect the maintenance helper from the power console.",
     "The cycle pauses while you read. Explore the home while collectors charge rather than waiting at a screen."},
    {"Ask the helper to inspect",
     "Power console - Inspect maintenance helper",
     "The distribution buffer has a fault. Open Inspect maintenance helper, then choose Inspect symptoms. It identifies the shared cause of dim displays and the slow salvage arm.",
     "Keep the workshop bus isolated. The known repair reserves 1 alloy and 2 conductor."},
    {"Check your remaining parts",
     "Maintenance helper or workshop stock readout",
     "You lack parts for the next repair or adapter. Check required and available amounts before spending more. Cancelling a repair returns its reserved parts.",
     "Stock is finite. The cradle can be recovered once after repair. If supplies are exhausted, load an earlier journey. Secure checkpoint recovery is offered after suit failure."},
    {"Queue the known repair",
     "Engineering maintenance helper",
     "The buffer fault is diagnosed and the bus isolated. Choose Queue repair. The helper reserves 1 alloy and 2 conductor and needs 10 active seconds.",
     "Close help and resume for work to advance. You can explore inside while it works. Cancellation refunds reserved parts; a saved journey keeps unfinished work."},
    {"Your helper is working",
     "Engineering - maintenance helper readout",
     "A repair is queued. Keep the workshop bus isolated and main energy available. Close help and resume so the 10-second job can finish; its readout shows progress.",
     "You can explore inside while it works. Help pauses the job rather than making it finish for you."},
    {"Recover the stock cradle",
     "Workshop - RIGHT side of engineering, 112 m",
     "The repaired manipulator can recover bulk stock. Approach the RIGHT workshop and choose Manipulator: recover 4 alloy + 4 conductor + 2 sealant.",
     "Recovery costs 150 kJ. This stock supplies the bypass and adapter. The phase coupler needs precise work in the service conduit."},
    {"Scan the service recess",
     "Wrist airlock - engineering RIGHT wall",
     "Approach the wrist airlock beyond the workshop and choose Scan pressure/temp/radiation/debris/field. Stay parked at the aligned tower socket.",
     "The scan checks reach and the environment. Suit work is allowed in this sheltered recess, not anywhere outside the machine."},
    {"Power the shelter field",
     "Wrist airlock console",
     "The landing is scanned. Choose Power shelter field. The buffer repair must be finished; activation costs 200 kJ.",
     "Keep shelter powered throughout the excursion. The coupler supplies it, so fit a bypass before extracting the coupler."},
    {"Prepare your suit",
     "Wrist airlock console - inside engineering",
     "Choose Suit up after scanning and powering shelter. You need a parked, aligned machine, the completed repair, at least 800 kJ and heat at or below 6000.",
     "The suit has 180 active seconds. Follow the rails to the coupler and back. Help and pause stop the clock while you read."},
    {"Follow the railings out",
     "Wrist airlock to the phase coupler at X about 28",
     "Walk RIGHT through the airlock opening and follow the sheltered rails. Approach the phase coupler, face its console and use E or the prompt.",
     "Keep the machine parked. Fit the bypass before extracting the coupler; return along these same railings."},
    {"Protect the return route",
     "Phase coupler and bypass console",
     "Choose Install shelter bypass before extracting the coupler. It needs 1 conductor, 1 sealant and 50 kJ.",
     "The coupler powers shelter. A bypass keeps the field alive when you remove it. Do not disconnect shelter during the excursion."},
    {"Recover the phase coupler",
     "Phase coupler console - service conduit",
     "The bypass is installed. Choose Extract phase coupler, then return along the rails through the wrist airlock.",
     "Inside cabin X below 14, remove the suit at the airlock console before working at the test bench."},
    {"Test the unfamiliar coupler",
     "Workshop - RIGHT side of engineering",
     "At the salvage cradle and test bench, choose Controlled coupler test. The repaired bench spends 100 kJ to discover what it does.",
     "The coupler rejects surge peaks. You can now make its adapter rather than install unfamiliar technology blindly."},
    {"Set fabrication priority",
     "Engineering power console - LEFT side",
     "Switch from cooling to fabrication priority, then return to the workshop to make the adapter.",
     "It needs 3 alloy, 3 conductor, 1 sealant and 300 kJ, with room for 400 heat. Read current stock before committing."},
    {"Make the adapter",
     "Workshop - RIGHT side of engineering",
     "Choose Adapter. It costs 3 alloy, 3 conductor, 1 sealant and 300 kJ. The known repair must be complete and fabrication priority selected.",
     "The adapter prepares the coupler for the machine's one grid-stabilizer socket."},
    {"Disconnect before installation",
     "Engineering power console - LEFT side",
     "Disconnect the tower grid, then return to the workshop to install the stabilizer. Stow collectors when ready to travel.",
     "The buffer must be repaired and the grid disconnected before installation. Keep installed bridge support while using it."},
    {"Install the stabilizer",
     "Workshop installation socket",
     "Choose Install stabilizer. It uses your prepared adapter and costs 50 kJ. The west service bridge becomes real supported terrain.",
     "Recovered technology opens a route the machine could not use before."},
    {"Pack up for departure",
     "Engineering power console, then bridge helm",
     "Stow collectors and disconnect the tower grid at the power console. Take the lift to the bridge and use the central helm.",
     "Collectors and the grid tether prevent travel. Keep the stabilizer installed to retain the powered west bridge."},
    {"Try the newly opened route",
     "Bridge helm - powered west service bridge",
     "Reverse from the tower to Z about -90 before turning so the hull clears the buttress. Turn LEFT to heading 270, drive west onto the turquoise bridge and park beyond X -130.",
     "Use the helm coordinates, Brake and Park. After completing the expedition, save. The stabilizer must remain installed while using its support."}
};
_Static_assert(sizeof(steps)/sizeof(steps[0])==MEU_GUIDE_STEP_COUNT,"guide step count");
static const char *topics[]={"What next?","About MEU","Walking and the lift","Piloting and parking","Power and repairs","Suit and salvage","Journeys and recovery"};
static const Card general[]={
    {"Your home is the exploration machine",
     "One Commander. One enormous home.",
     "You are the sole human Commander inside a 240-metre machine in an abandoned alien city. Walk through its bridge, living quarters and engineering hall, and pilot from the helm. Everything stays first person.",
     "This build offers one tower expedition: park, restore power, repair the arm, recover technology in a suit, adapt it at home and open a west route. Ask What next? whenever you lose your place."},
    {"Find your feet",
     "Bridge 196 m - quarters 156 m - engineering 112 m",
     "Use WASD or touch arrows to walk. Click the 3D view to capture the mouse, then move it to look without holding a button. Arrows or dragging LOOK also turn the view. E or the visible prompt interacts.",
     "The lift is behind the start. Step into its centre, choose 1 Bridge, 2 Quarters or 3 Engineering, wait for arrival, then walk out. Escape pauses and releases the mouse. H or F3 opens Pip; F1 opens Settings."},
    {"Your eyes do not steer the machine",
     "Central bridge helm",
     "Approach the console and use E or Take bridge helm. W/S drive forward/reverse; A/D turn the machine. Mouse, arrows or LOOK turn your view independently. Touch controls change to driving, turning, Brake and Park.",
     "Space brakes; P parks. E releases the helm and requests a stop. Read coordinates, heading, stopping distance and route warnings. Unsupported ground and obstacles stop travel. Stow collectors and disconnect the grid before departure."},
    {"Look for a cause, then repair it",
     "Engineering power LEFT - workshop RIGHT",
     "The power console deploys collectors, isolates the workshop bus, connects the grid and selects cooling/fabrication priority. Keep a 4000 kJ return margin. The tower supply surges every 12 seconds.",
     "Open Inspect maintenance helper, inspect symptoms, then queue the diagnosed repair with the displayed parts. Known repairs take active time and can be cancelled or saved. Close help and resume for charging, cooling and repair to advance."},
    {"Keep your way home protected",
     "Wrist airlock and service conduit",
     "Park aligned at the tower socket. Scan, finish repairs, power shelter and suit up with the required energy/heat margin. The suit lasts 180 active seconds; watch supply/exposure and follow the rails back.",
     "Fit the bypass BEFORE extracting the coupler so shelter stays powered. Return inside cabin X below 14 and remove the suit at the airlock. Then test, adapt and install the technology at the workshop."},
    {"Keep a durable journey",
     "Saved journeys - F2 or the menu button",
     "Save after parking and finishing the lift ride. Native: F2 opens journeys, F5 saves, F9 loads and F8 recovers the previous save. The browser has labelled buttons. Restored journeys stay paused until you resume.",
     "Export a portable .meu backup. Import replaces the current journey; Save to keep it here. Native export/import use F6/F7. Saves stay on this device/browser. Suit checkpoint recovery is explicit and loses unsecured progress."}
};
MeuGuideStep meu_guide_step(const MeuSimulation *s)
{
    const MeuExpedition *e=&s->expedition;
    if(e->catastrophe)return MEU_GUIDE_RECOVER;
    if(e->suit) {
        if(e->suit_ticks<1800 || e->exposure || !e->shelter || (e->coupler && !e->bypass))return MEU_GUIDE_RETURN_NOW;
        if(e->coupler)return s->x<14?MEU_GUIDE_UNSUIT:MEU_GUIDE_RETURN;
        if(s->x<24)return MEU_GUIDE_CONDUIT;
        return e->bypass?MEU_GUIDE_EXTRACT:MEU_GUIDE_BYPASS;
    }
    if(e->completed && e->installed)return MEU_GUIDE_COMPLETE;
    if(!s->started)return MEU_GUIDE_WELCOME;
    if(s->power.energy<MEU_RETURN_MARGIN)return MEU_GUIDE_CHARGE;
    if(s->power.heat>9000 || (!e->coupler && e->bulk && e->shelter && s->power.heat>6000))return MEU_GUIDE_COOL;
    if(!e->installed && (!s->travel.parked || fabs(s->robot.x)>15 || s->robot.z > -120 || s->robot.z < -175 || fabs(remainder(s->robot.yaw,6.283185307179586))>=.05))return MEU_GUIDE_DOCK;
    if(!e->repaired || e->fault || e->job) {
        if(!s->power.isolated)return MEU_GUIDE_ISOLATE;
        if(e->job)return MEU_GUIDE_REPAIR;
        if(e->fault) {
            if(!e->diagnosed)return MEU_GUIDE_DIAGNOSE;
            return e->alloy<1 || e->conductor<2?MEU_GUIDE_PARTS:MEU_GUIDE_QUEUE;
        }
        return s->power.grid?MEU_GUIDE_PULSE:MEU_GUIDE_CONNECT;
    }
    if(!e->bulk)return MEU_GUIDE_BULK;
    if(!e->scanned)return MEU_GUIDE_SCAN;
    if(!e->shelter)return MEU_GUIDE_SHELTER;
    if(!e->coupler)return MEU_GUIDE_SUIT;
    if(!e->tested)return MEU_GUIDE_TEST;
    if(!e->adapter) {
        if(e->alloy<3 || e->conductor<3 || e->sealant<1)return MEU_GUIDE_PARTS;
        return s->power.cooling_priority?MEU_GUIDE_PRIORITY:MEU_GUIDE_ADAPTER;
    }
    if(!e->installed)return s->power.grid?MEU_GUIDE_DISCONNECT:MEU_GUIDE_INSTALL;
    return s->power.grid || s->power.collectors?MEU_GUIDE_PACK:MEU_GUIDE_DEPART;
}
const char *meu_guide_topic(int topic) { return topic>=0 && topic<MEU_GUIDE_TOPICS?topics[topic]:""; }
const char *meu_guide_text(const MeuSimulation *s,int topic,int field)
{
    if(topic<0 || topic>=MEU_GUIDE_TOPICS || field<0 || field>3)return "";
    const Card *c=topic?&general[topic-1]:&steps[meu_guide_step(s)];
    switch(field) {case 0:return c->title;case 1:return c->place;case 2:return c->action;default:return c->tip;}
}
