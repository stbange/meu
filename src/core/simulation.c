#include "core/simulation.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
static float clampf(float v,float a,float b) { return fmaxf(a,fminf(b,v)); }
static void previous(MeuSimulation *s)
{
    s->previous_x=s->x; s->previous_y=s->y; s->previous_z=s->z;
    s->previous_lift_y=s->lift_y; s->previous_robot=s->robot;
}
void meu_sim_init(MeuSimulation *s)
{
    memset(s,0,sizeof(*s)); s->y=196+MEU_EYE_HEIGHT; s->z=-8; s->pitch=-.08f;
    s->lift_y=196; s->lift_door=1; s->paused=true; s->grounded=true;
    s->world=meu_world_default(); meu_travel_init(&s->travel); meu_power_init(&s->power); meu_expedition_init(&s->expedition);
    meu_settings_default(&s->settings); s->input_epoch=1;
    s->quarters_light=true; s->inspection_light=true; s->visited=1; previous(s);
}
MeuContext meu_sim_context(const MeuSimulation *s)
{
    if (s->journeys_open) return MEU_CONTEXT_JOURNEYS;
    if (s->settings_open) return MEU_CONTEXT_SETTINGS;
    if (s->paused) return MEU_CONTEXT_PAUSED;
    if (s->station) return MEU_CONTEXT_STATION;
    if (s->info_open) return MEU_CONTEXT_PANEL;
    return s->travel.helm?MEU_CONTEXT_HELM:MEU_CONTEXT_FOOT;
}
void meu_sim_clear_input(MeuSimulation *s) { s->held=0; ++s->input_epoch; }
void meu_sim_start(MeuSimulation *s)
{
    if (s->settings_open || s->journeys_open) return;
    meu_sim_clear_input(s);
    s->started=true; s->paused=false; if(s->expedition.catastrophe)s->station=5; s->held=0; s->accumulator=0; previous(s);
}
void meu_sim_release_helm(MeuSimulation *s)
{
    s->travel.helm=false; meu_sim_clear_input(s); meu_travel_stop(&s->travel);
}
void meu_sim_pause(MeuSimulation *s)
{
    meu_sim_release_helm(s);
    s->paused=true; s->info_open=false; s->station=0; s->held=0; s->accumulator=0; previous(s);
}
void meu_sim_open_settings(MeuSimulation *s)
{
    meu_sim_pause(s); s->journeys_open=false; s->settings_open=true; meu_sim_clear_input(s);
}
void meu_sim_close_settings(MeuSimulation *s)
{
    s->settings_open=false; meu_sim_clear_input(s);
}
void meu_sim_open_journeys(MeuSimulation *s)
{
    meu_sim_pause(s); s->settings_open=false; s->journeys_open=true; meu_sim_clear_input(s);
}
void meu_sim_close_journeys(MeuSimulation *s)
{
    s->journeys_open=false; meu_sim_clear_input(s);
}
bool meu_sim_setting(MeuSimulation *s,int id,float value)
{
    return s->settings_open && meu_settings_set(&s->settings,id,value);
}
void meu_sim_pointer_look(MeuSimulation *s,float dx,float dy)
{
    if (!isfinite(dx) || !isfinite(dy)) return;
    float scale=.004f*s->settings.sensitivity;
    meu_sim_look(s,dx*scale,dy*scale*(s->settings.invert_y?1:-1));
}
MeuCamera meu_sim_camera(const MeuSimulation *s)
{
    MeuCamera eye={s->x,s->y,s->z,s->yaw,s->pitch};
    if (s->settings.walk_bob) eye.y+=.025f*sinf(s->walk_phase)*s->walk_amount;
    if (s->settings.machine_sway) {
        float amount=fminf(1,(float)(fabs(s->travel.speed)/MEU_MAX_SPEED+fabs(s->travel.turn_rate)/MEU_MAX_TURN*.3));
        float phase=(float)((double)s->ticks*MEU_TICK_SECONDS);
        eye.x+=sinf(phase*1.7f)*.006f*amount;
        eye.y+=sinf(phase*2.3f)*.012f*amount;
    }
    return eye;
}
bool meu_sim_on_lift(const MeuSimulation *s)
{
    return fabsf(s->x)<=1.9f && fabsf(s->z)<=1.9f && fabsf(s->y-MEU_EYE_HEIGHT-s->lift_y)<.04f;
}
bool meu_sim_lift_moving(const MeuSimulation *s)
{
    return fabsf(s->lift_y-meu_deck_height(s->lift_target))>.01f || fabsf(s->lift_velocity)>.01f || s->lift_door<.98f;
}
int meu_sim_room(const MeuSimulation *s)
{
    if (meu_sim_on_lift(s) || (s->z>-4 && fabsf(s->x)<2.5f)) return MEU_LIFT;
    return s->y>180 ? MEU_BRIDGE : s->y>145 ? MEU_QUARTERS : MEU_ENGINEERING;
}
bool meu_sim_lift_ready(const MeuSimulation *s)
{
    return s->started && !s->paused && meu_sim_on_lift(s) && fabsf(s->x)<=1.6f &&
        fabsf(s->z)<=1.6f && !meu_sim_lift_moving(s);
}
bool meu_sim_request_lift(MeuSimulation *s,int deck)
{
    if (s->travel.helm || deck<0 || deck>2 || !meu_sim_lift_ready(s)) return false;
    s->lift_target=deck; meu_sim_clear_input(s); return true;
}
MeuUse meu_sim_use_target(const MeuSimulation *s)
{
    if (s->station) return MEU_USE_NONE;
    if (s->travel.helm) return MEU_USE_HELM;
    if (s->info_open) return MEU_USE_PLACARD;
    float positions[12][3]={{3.85f,197.3f,-9.7f},{6.55f,157.4f,-7.25f},{0,114.5f,-26.5f},
        {-1.65f,197.25f,-3.25f},{-1.65f,157.25f,-3.25f},{-1.65f,113.25f,-3.25f},{0,197.2f,-14.5f},{-6,197.3f,-11.4f},{-9,113.3f,-9.3f},{7,113.3f,-10.5f},{13.5f,113.3f,-10},{27.8f,113.3f,-10}};
    for (int i=0;i<12;++i) {
        if (i==6 && (!s->travel.parked || s->motion_fixture)) continue;
        float dx=positions[i][0]-s->x,dy=positions[i][1]-s->y,dz=positions[i][2]-s->z;
        float distance=sqrtf(dx*dx+dy*dy+dz*dz);
        float facing=(dx*sinf(s->yaw)*cosf(s->pitch)+dy*sinf(s->pitch)-dz*cosf(s->yaw)*cosf(s->pitch))/fmaxf(distance,.001f);
        if (distance<2.8f && facing>.4f && meu_interior_clear_ray(s->x,s->y,s->z,positions[i][0],positions[i][1],positions[i][2]))
            return i>=7 ? (MeuUse)(MEU_USE_NAVIGATION+i-7) : i<3 ? (MeuUse)(i+1) : i==6 ? MEU_USE_HELM : MEU_USE_CALL;
    }
    return MEU_USE_NONE;
}
const char *meu_sim_use_name(MeuUse use)
{
    static const char *names[]={"", "Read unit dimensions", "Toggle reading lamp", "Toggle inspection lights", "Call service lift", "Take bridge helm", "Expedition chart", "Power and infrastructure", "Salvage cradle and test bench", "Wrist airlock", "Phase coupler and bypass"};
    return names[use>=0 && use<=MEU_USE_COUPLER ? use : 0];
}
bool meu_sim_action(MeuSimulation *s,int action,bool pressed)
{
    if (action<0 || action>=MEU_ACTION_COUNT) return false;
    if (pressed && (!s->started || s->paused || s->expedition.catastrophe)) return false;
    if ((s->info_open || s->station) && action!=MEU_INTERACT) {
        if (!pressed) s->held&=~(UINT32_C(1)<<action);
        return !pressed;
    }
    if (action==MEU_PARK) {
        if (!pressed) return true;
        if (!s->travel.helm) return false;
        meu_sim_clear_input(s); meu_travel_stop(&s->travel); return true;
    }
    if (action==MEU_BRAKE) {
        if (pressed && !s->travel.helm) return false;
        if (pressed) s->held=(s->held & ~UINT32_C(15)) | (UINT32_C(1)<<MEU_BRAKE);
        else s->held&=~(UINT32_C(1)<<MEU_BRAKE);
        return true;
    }
    if (action>=MEU_TO_BRIDGE && action<=MEU_TO_ENGINEERING)
        return !pressed || meu_sim_request_lift(s,action-MEU_TO_BRIDGE);
    if (action==MEU_INTERACT) {
        if (!pressed) return true;
        if (s->station) return meu_sim_station_command(s,-1);
        switch (meu_sim_use_target(s)) {
            case MEU_USE_AIRLOCK:case MEU_USE_COUPLER:
                s->station=meu_sim_use_target(s)==MEU_USE_AIRLOCK?5:6;meu_sim_clear_input(s);break;
            case MEU_USE_WORKSHOP:
                s->station=4;meu_sim_clear_input(s);break;
            case MEU_USE_NAVIGATION: case MEU_USE_POWER:
                s->station=meu_sim_use_target(s)-MEU_USE_NAVIGATION+1;
                meu_sim_clear_input(s); break;
            case MEU_USE_HELM:
                if (s->travel.helm) meu_sim_release_helm(s);
                else { s->travel.helm=true; meu_sim_clear_input(s); s->info_open=false; }
                break;
            case MEU_USE_PLACARD: s->info_open=!s->info_open; meu_sim_clear_input(s); break;
            case MEU_USE_LAMP: s->quarters_light=!s->quarters_light; break;
            case MEU_USE_INSPECTION: s->inspection_light=!s->inspection_light; break;
            case MEU_USE_CALL:
                if (meu_sim_lift_moving(s)) return false;
                s->lift_target=s->y>180 ? 0 : s->y>145 ? 1 : 2; break;
            default: return false;
        }
    } else if (pressed) {
        if (s->travel.helm && action<=MEU_RIGHT) {
            if ((s->held&(UINT32_C(1)<<MEU_BRAKE)) ||
                (s->travel.park_requested && !s->travel.parked)) return false;
            if (s->travel.parked && !(s->held&15)) {
                s->travel.latched=false; s->travel.reason=MEU_ROUTE_CLEAR;
            }
            if (!s->travel.latched) s->travel.park_requested=false;
        }
        s->held|=UINT32_C(1)<<action;
    } else s->held&=~(UINT32_C(1)<<action);
    return true;
}
void meu_sim_look(MeuSimulation *s,float yaw,float pitch)
{
    if (s->paused || s->info_open || s->station || !isfinite(yaw) || !isfinite(pitch)) return;
    s->yaw=remainderf(s->yaw+clampf(yaw,-6.2831853f,6.2831853f),6.2831853f);
    s->pitch=clampf(s->pitch+pitch,-1.4f,1.4f);
}
static bool support(const MeuSimulation *s,float x,float z,float *floor)
{
    if (x>13.7f && s->y<120 && !s->expedition.suit) return false;
    float feet=s->y-MEU_EYE_HEIGHT;
    if (!meu_interior_floor(x,z,feet,s->lift_y,floor)) return false;
    static const float offsets[4][2]={{.27f,0},{-.27f,0},{0,.27f},{0,-.27f}};
    for (int i=0;i<4;++i) {
        float edge;
        if (!meu_interior_floor(x+offsets[i][0],z+offsets[i][1],*floor,s->lift_y,&edge) || fabsf(edge-*floor)>.16f) return false;
    }
    return !meu_interior_blocked(x,*floor,z,s->lift_y,s->lift_door);
}
bool meu_sim_valid_position(const MeuSimulation *s)
{
    float floor;
    return isfinite(s->x) && isfinite(s->y) && isfinite(s->z) && support(s,s->x,s->z,&floor) &&
        fabsf(floor-(s->y-MEU_EYE_HEIGHT))<.02f;
}
void meu_sim_move(MeuSimulation *s,float dx,float dz)
{
    if (s->expedition.catastrophe) return;
    if (!isfinite(dx) || !isfinite(dz) || fabsf(dx)>100 || fabsf(dz)>100) return;
    int steps=(int)ceilf(fmaxf(fabsf(dx),fabsf(dz))/.06f); if (steps<1) steps=1;
    dx/=(float)steps; dz/=(float)steps;
    for (int i=0;i<steps;++i) {
        float floor;
        if (support(s,s->x+dx,s->z,&floor)) { s->x+=dx; s->y=floor+MEU_EYE_HEIGHT; }
        if (support(s,s->x,s->z+dz,&floor)) { s->z+=dz; s->y=floor+MEU_EYE_HEIGHT; }
    }
    float floor;
    s->grounded=meu_interior_floor(s->x,s->z,s->y-MEU_EYE_HEIGHT,s->lift_y,&floor) && fabsf(s->y-MEU_EYE_HEIGHT-floor)<.02f;
}
static void lift_tick(MeuSimulation *s)
{
    const float dt=(float)MEU_TICK_SECONDS;
    float distance=meu_deck_height(s->lift_target)-s->lift_y;
    bool passenger=meu_sim_on_lift(s); float before=s->lift_y;
    if (fabsf(distance)>.001f) {
        s->lift_door=fmaxf(0,s->lift_door-dt*3);
        if (s->lift_door<=0) {
            float desired=copysignf(fminf(12,sqrtf(16*fabsf(distance))),distance);
            s->lift_velocity+=clampf(desired-s->lift_velocity,-8*dt,8*dt);
            float step=s->lift_velocity*dt;
            if (fabsf(step)>=fabsf(distance)) { s->lift_y+=distance; s->lift_velocity=0; }
            else s->lift_y+=step;
        }
    } else {
        s->lift_current=s->lift_target; s->lift_velocity=0;
        s->lift_door=fminf(1,s->lift_door+dt*3);
    }
    if (passenger) s->y+=s->lift_y-before;
}
static int held(const MeuSimulation *s,MeuAction action) { return (s->held&(UINT32_C(1)<<action))!=0; }
static void tick(MeuSimulation *s)
{
    const float dt=(float)MEU_TICK_SECONDS;
    previous(s); lift_tick(s);
    if (s->world==meu_world_default() || s->world==meu_world_with_bridge())
        s->world=s->expedition.installed?meu_world_with_bridge():meu_world_default();
    float forward=(float)(held(s,MEU_FORWARD)-held(s,MEU_BACKWARD));
    float right=(float)(held(s,MEU_RIGHT)-held(s,MEU_LEFT));
    float length=sqrtf(forward*forward+right*right);
    if (length>1 && !s->travel.helm) { forward/=length; right/=length; }
    meu_sim_look(s,(float)(held(s,MEU_LOOK_RIGHT)-held(s,MEU_LOOK_LEFT))*1.5f*dt*s->settings.sensitivity,
        (float)(held(s,MEU_LOOK_UP)-held(s,MEU_LOOK_DOWN))*1.5f*dt*s->settings.sensitivity);
    float old_x=s->x,old_z=s->z;
    if (!s->travel.helm && !s->info_open && !s->station) meu_sim_move(s,(sinf(s->yaw)*forward+cosf(s->yaw)*right)*MEU_WALK_SPEED*dt,
        (-cosf(s->yaw)*forward+sinf(s->yaw)*right)*MEU_WALK_SPEED*dt);
    float distance=hypotf(s->x-old_x,s->z-old_z);
    s->walk_phase=remainderf(s->walk_phase+distance*5.5f,6.2831853f);
    s->walk_amount=clampf(s->walk_amount+(distance>.0001f?1:-1)*dt*8,0,1);
    if (s->power.energy<MEU_RETURN_MARGIN || s->power.grid || s->power.collectors || s->expedition.suit) meu_travel_stop(&s->travel);
    if (!s->motion_fixture)
        meu_travel_tick(&s->travel,&s->robot,s->world,(int)forward,(int)right,held(s,MEU_BRAKE)!=0);
    meu_power_tick(&s->power,!s->travel.parked,s->expedition.installed);
    meu_expedition_tick(&s->expedition,&s->power);
    if (s->expedition.suit && s->x>14 && !s->expedition.catastrophe) {
        MeuExpedition *e=&s->expedition;
        if(e->suit_ticks) --e->suit_ticks;
        if (!e->shelter || (!e->bypass && e->coupler) || s->power.energy<2 || s->power.heat>6000) {
            if(e->exposure<600) ++e->exposure;
        } else if(e->exposure) --e->exposure;
        if(s->power.phase==0 && s->power.energy>=2) s->power.energy-=2;
        if(!e->suit_ticks || e->exposure==600) {
            e->catastrophe=true;s->station=5;meu_sim_clear_input(s);
            snprintf(s->message,sizeof(s->message),"Suit limit exceeded. Choose explicit checkpoint recovery; unsecured progress will be lost.");
        }
    }
    ++s->ticks; s->visited|=UINT32_C(1)<<meu_sim_room(s);
    /* Test-only motion driver: the future steering system supplies this same pose. */
    if (s->motion_fixture) {
        double t=fmod((double)s->ticks*MEU_TICK_SECONDS,60);
        if (t>30) t=60-t;
        s->robot=(MeuRobotPose){30*(1-cos(t*3.141592653589793/30)),-2*t,.3*sin(t*3.141592653589793/30)};
    }
}
unsigned meu_sim_advance(MeuSimulation *s,double elapsed)
{
    unsigned count=0;
    if (s->paused || !isfinite(elapsed) || elapsed<=0) return 0;
    if (elapsed>.25) { s->dropped_seconds+=elapsed-.25; elapsed=.25; }
    s->accumulator+=elapsed;
    while (s->accumulator+1e-12>=MEU_TICK_SECONDS && count<MEU_MAX_TICKS_PER_FRAME) {
        tick(s); s->accumulator-=MEU_TICK_SECONDS; ++count;
    }
    if (s->accumulator>=MEU_TICK_SECONDS) {
        double remainder=fmod(s->accumulator,MEU_TICK_SECONDS);
        s->dropped_seconds+=s->accumulator-remainder; s->accumulator=remainder;
    }
    if (s->accumulator<0) s->accumulator=0;
    return count;
}
void meu_sim_render_state(const MeuSimulation *s,MeuSimulation *r)
{
    *r=*s; float a=(float)(s->accumulator/MEU_TICK_SECONDS);
    r->x=s->previous_x+(s->x-s->previous_x)*a; r->y=s->previous_y+(s->y-s->previous_y)*a;
    r->z=s->previous_z+(s->z-s->previous_z)*a; r->lift_y=s->previous_lift_y+(s->lift_y-s->previous_lift_y)*a;
    r->robot.x=s->previous_robot.x+(s->robot.x-s->previous_robot.x)*a;
    r->robot.z=s->previous_robot.z+(s->robot.z-s->previous_robot.z)*a;
    r->robot.yaw=s->previous_robot.yaw+(s->robot.yaw-s->previous_robot.yaw)*a;
}

const char *meu_sim_station_title(const MeuSimulation *s)
{
    const char *titles[]={"", "Tower expedition chart", "Engineering power console", "Maintenance helper", "Salvage cradle and test bench", "Wrist airlock", "Phase coupler and bypass"};
    return titles[s->station>=0 && s->station<=6?s->station:0];
}
void meu_sim_station_status(const MeuSimulation *s,char *line,unsigned size)
{
    if(s->expedition.suit) {
        snprintf(line,size,"Suit %us | exposure %u/600 | %s | return: cabin X<14",
            s->expedition.suit_ticks/60,s->expedition.exposure,
            s->expedition.shelter && (s->expedition.bypass || !s->expedition.coupler)?"field protected":"FIELD FAILING");
    } else snprintf(line,size,"Energy %u/%u kJ | heat %u/10000 | sun %u%% | life reserve %u kJ",
        s->power.energy,MEU_ENERGY_CAPACITY,s->power.heat,meu_power_sun(&s->power)/10,MEU_LIFE_RESERVE);
}
void meu_sim_station_line(const MeuSimulation *s,int row,char *line,unsigned size)
{
    if (s->station==5 || s->station==6) {
        const MeuExpedition *e=&s->expedition;
        if(s->station==5) {
            switch(row) {
                case 0:snprintf(line,size,"Scan pressure/temp/radiation/debris/field (30 kJ)");break;
                case 1:snprintf(line,size,"%s shelter field (200 kJ to activate)",e->shelter?"Disconnect":"Power");break;
                case 2:snprintf(line,size,"Suit up: parked, scanned landing, repair complete, 800 kJ margin");break;
                case 3:snprintf(line,size,"Return to cabin / remove suit (inside airlock only)");break;
                case 4:snprintf(line,size,"Suit %us | exposure %u/600 | return: follow rails to cabin X<14",e->suit_ticks/60,e->exposure);break;
                case 7:snprintf(line,size,"Recover secure checkpoint (catastrophic failure only; lose unsecured salvage)");break;
                default:if(size)line[0]=0;break;
            }
        } else {
            const char *lines[]={"Install shelter bypass: 1 conductor, 1 sealant, 50 kJ", "Extract phase coupler (warning: without bypass shelter fails)", "Inspect: coupler supplies field; bypass preserves safe return"};
            snprintf(line,size,"%s",row>=0 && row<3?lines[row]:"");
        }
    } else if (s->station==1) {
        const char *lines[]={"Tower socket: pilot north to Z -120..-175; park", "Expedition estimate: 3000 kJ + 2000 uncertainty", "Keep 4000 kJ return margin; solar recovery available", "Engineering console: beside the helper on deck 3"};
        snprintf(line,size,"%s",row>=0 && row<4?lines[row]:"");
    } else if (s->station==4) {
        switch(row) {
            case 0:snprintf(line,size,"Manipulator: recover 4 alloy + 4 conductor + 2 sealant (150 kJ)");break;
            case 1:snprintf(line,size,"Controlled coupler test (100 kJ, +120 heat)");break;
            case 2:snprintf(line,size,"Adapter: alloy %u/3 conductor %u/3 sealant %u/1 (300 kJ)",s->expedition.alloy,s->expedition.conductor,s->expedition.sealant);break;
            case 3:snprintf(line,size,"Install stabilizer: 1 slot, 220 kJ/s, +12 heat/s (50 kJ)");break;
            case 4:snprintf(line,size,"Remove stabilizer intact (disconnect grid first)");break;
            case 5:snprintf(line,size,"Socket: %s | coupler: %s",s->expedition.installed?"installed":"empty",s->expedition.tested?"tested":s->expedition.coupler?"unfamiliar":"not recovered");break;
            default:if(size)line[0]=0;break;
        }
    } else if (s->station==3) {
        switch(row) {
            case 0:snprintf(line,size,"Inspect symptoms: %s",s->expedition.fault?"dim displays, slow arm":"systems nominal");break;
            case 1:snprintf(line,size,"Queue repair: alloy %u/1, conductor %u/2",s->expedition.alloy,s->expedition.conductor);break;
            case 2:snprintf(line,size,"Cancel repair and refund reserved parts");break;
            case 3:snprintf(line,size,"%s",meu_helper_status(&s->expedition,&s->power));break;
            case 4:snprintf(line,size,"Back to power console");break;
            default:if(size)line[0]=0;break;
        }
    } else {
        switch (row) {
            case 0:snprintf(line,size,"%s collectors (parked only)",s->power.collectors?"Stow":"Deploy");break;
            case 1:snprintf(line,size,"%s workshop bus",s->power.isolated?"Reconnect":"Isolate");break;
            case 2:snprintf(line,size,"%s tower grid (pulses every 12 s)",s->power.grid?"Disconnect":"Connect");break;
            case 3:snprintf(line,size,"Priority: %s",s->power.cooling_priority?"cooling":"fabrication");break;
            case 7:snprintf(line,size,"Inspect maintenance helper");break;
            default:if (size) line[0]=0;break;
        }
    }
}
bool meu_sim_station_command(MeuSimulation *s,int row)
{
    if (!s->station || s->paused) return false;
    if (row==-1 && s->expedition.catastrophe) return false;
    if (row==-1) { s->station=0; meu_sim_clear_input(s); return true; }
    const char *message="Read the chart, then approach the tower at the helm."; bool accepted=false;
    if(s->station==5 || s->station==6) {
        MeuExpedition *e=&s->expedition;
        bool site=s->travel.parked && s->robot.z<=-120 && s->robot.z>=-175 && fabs(s->robot.x)<=15 && fabs(s->robot.yaw)<.05;
        if(e->catastrophe) {
            if(row==7 && e->checkpoint) {
                meu_expedition_recover(e,&s->power);s->x=12;s->y=112+MEU_EYE_HEIGHT;s->z=-10;s->yaw=0;s->pitch=0;
                s->previous_x=s->x;s->previous_y=s->y;s->previous_z=s->z;
                message="Secure checkpoint restored. Unsecured salvage/progress lost; home preserved.";accepted=true;
            } else message="Choose checkpoint recovery explicitly to continue.";
        } else if(s->station==5) {
            if(row==0) {
                if(!site)message="Scan blocked: park aligned at tower socket, heading 000, X +/-15, Z -120..-175.";
                else if(!meu_power_spend(&s->power,30,0)) message="Scan needs 30 kJ.";
                else { e->scanned=true;accepted=true;message="Landing in reach. Pressure 80 kPa; -10 C; radiation 0.2 mSv/h; debris low. Field must stay powered."; }
            } else if(row==1) {
                if(e->shelter) { e->shelter=false;accepted=true;message="Shelter disconnected. Outside exposure rises; retreat immediately."; }
                else if(!site || !e->scanned)message="Scan a reachable parked landing before powering shelter.";
                else if(e->fault || e->job)message="Repair the distribution buffer before powering the recess.";
                else if(e->coupler && !e->bypass)message="Shelter source removed. Fit bypass or recover the checkpoint.";
                else if(!meu_power_spend(&s->power,200,100))message="Shelter activation needs 200 kJ and 100 heat capacity.";
                else { e->shelter=true;accepted=true;message="Service recess sheltered. Suit limit 180 active seconds; return along the rails."; }
            } else if(row==2) {
                if(e->suit)message="Suit already active; no checkpoint overwritten.";
                else if(!site)message="Egress blocked: park aligned with the reachable landing.";
                else if(!e->scanned)message="Egress blocked: scan pressure, temperature, radiation and landing clearance.";
                else if(e->fault || e->job)message="Egress blocked: finish the known repair and clear helper access.";
                else if(!e->shelter || (e->coupler && !e->bypass))message="Egress blocked: shelter field unavailable.";
                else if(s->power.energy<800 || s->power.heat>6000)message="Egress blocked: need 800 kJ return margin and heat <=6000.";
                else { meu_expedition_checkpoint(e,&s->power);e->suit=true;e->suit_ticks=10800;e->exposure=0;accepted=true;
                    message="Suit ready. Walk through the wrist airlock to the conduit. Unit remains parked."; }
            } else if(row==3) {
                if(s->x>=14)message="Return along the railings through the airlock before removing the suit.";
                else if(!e->suit)message="Already in the cabin.";
                else { e->suit=false;e->checkpoint=false;e->suit_ticks=0;e->exposure=0;accepted=true;message="Cabin secure. Coupler retained; take it to the workshop."; }
            }
        } else if(!e->suit || s->x<24)message="Precise work requires the suit and reaching the service conduit.";
        else if(row==0) {
            if(e->bypass)message="Bypass already fitted. No duplicate costs.";
            else if(e->conductor<1 || e->sealant<1)message="Bypass needs 1 conductor and 1 sealant.";
            else if(!meu_power_spend(&s->power,50,20))message="Bypass needs 50 kJ and 20 heat capacity.";
            else { --e->conductor;--e->sealant;e->bypass=true;accepted=true;message="Bypass fitted. Shelter stays powered when the coupler is removed."; }
        } else if(row==1) {
            if(e->coupler)message="Coupler already recovered. Return to the cabin.";
            else { e->coupler=true;accepted=true;message=e->bypass?"Coupler recovered; bypass maintains shelter. Return along rails.":"Coupler recovered without bypass: field failing! Return now or choose checkpoint after suit failure."; }
        } else message="The coupler powers shelter. A parallel bypass preserves the return route.";
    } else if (s->station==4) {
        if (row==4 && (fabs(s->robot.x)>15 || s->robot.z > -120 || s->robot.z < -175)) {
            snprintf(s->message,sizeof(s->message),"Return to the main tower socket before retracting the powered bridge.");return false;
        }
        message=meu_salvage_command(&s->expedition,&s->power,row,
            s->travel.parked && s->robot.z<=-120 && s->robot.z>=-175 && fabs(s->robot.x)<=15,&accepted);
    } else if (s->station==3) {
        if(row==4) { s->station=2;accepted=true;message="Power console."; }
        else message=meu_maintenance_command(&s->expedition,&s->power,row,&accepted);
    } else if (s->station==2) {
        if(row==7) { s->station=3;accepted=true;message=meu_helper_status(&s->expedition,&s->power); }
        else
        if (row==0) {
            if (!s->travel.parked) message="Park before deploying collectors.";
            else { s->power.collectors=!s->power.collectors; accepted=true; message="Collector configuration updated. Charging continues during interior tasks."; }
        } else if (row==1) {
            if (s->power.grid) message="Disconnect the grid before reconnecting the workshop bus.";
            else { s->power.isolated=!s->power.isolated; accepted=true; message="Essential doors and life support remain isolated and available."; }
        } else if (row==2) {
            if (s->power.grid) { s->power.grid=false; accepted=true; message="Grid disconnected safely."; }
            else if (!s->travel.parked || s->robot.z > -120 || s->robot.z < -175 || fabs(s->robot.x)>15) message="Park at tower socket: X +/-15, Z -120..-175.";
            else if (!s->power.isolated) message="Surge pattern detected: isolate the workshop bus first.";
            else { s->power.grid=true; accepted=true; message="Grid connected through isolation. Pulses may trip the buffer."; }
        } else if (row==3) { s->power.cooling_priority=!s->power.cooling_priority;accepted=true;message="Power priority updated."; }
    }
    snprintf(s->message,sizeof(s->message),"%s",message); return accepted;
}
