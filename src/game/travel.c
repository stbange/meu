#include "game/travel.h"
#include <math.h>
#include <string.h>
static double approach(double value,double target,double step)
{
    return value<target?fmin(value+step,target):fmax(value-step,target);
}
static MeuRobotPose integrate(MeuRobotPose p,double v0,double v1,double w0,double w1,double dt)
{
    double angle=(w0+w1)*.5*dt,mid=p.yaw+angle*.5,distance=(v0+v1)*.5*dt;
    p.x+=sin(mid)*distance; p.z-=cos(mid)*distance; p.yaw+=angle; return p;
}
void meu_travel_init(MeuTravel *t)
{
    memset(t,0,sizeof(*t)); t->park_requested=true; t->parked=true;
}
void meu_travel_stop(MeuTravel *t) { t->park_requested=true; }
double meu_travel_stop_distance(double speed) { return speed*speed/(2*MEU_BRAKE_DECELERATION); }
bool meu_travel_envelope(const MeuWorld *world,MeuRobotPose pose,double speed,double turn,MeuRoute *reason)
{
    if (!isfinite(speed) || !isfinite(turn) || fabs(speed)>MEU_MAX_SPEED+.001 || fabs(turn)>MEU_MAX_TURN+.001) {
        *reason=MEU_ROUTE_INVALID; return false;
    }
    *reason=meu_world_validate(world,pose,MEU_TRAVEL_CLEARANCE);
    if (*reason!=MEU_ROUTE_CLEAR) return false;
    /* Each swept interval encloses every intermediate hull/foot position. */
    for (int i=0;i<65 && (speed!=0 || turn!=0);++i) {
        double dt=.05;
        double v=approach(speed,0,MEU_BRAKE_DECELERATION*dt);
        double w=approach(turn,0,MEU_TURN_ACCELERATION*dt);
        MeuRobotPose next=integrate(pose,speed,v,turn,w,dt);
        *reason=meu_world_sweep(world,pose,next);
        if (*reason!=MEU_ROUTE_CLEAR) return false;
        pose=next; speed=v; turn=w;
    }
    return speed==0 && turn==0;
}
void meu_travel_tick(MeuTravel *t,MeuRobotPose *p,const MeuWorld *world,int drive,int turn,bool brake)
{
    const double dt=1.0/60.0;
    if (!t->helm) { drive=0; turn=0; }
    if (!drive && !turn && t->speed==0 && t->turn_rate==0) t->latched=false;
    bool stop=brake || t->park_requested || t->latched || !t->helm;
    double target=stop?0:(double)drive*MEU_MAX_SPEED,angular=stop?0:(double)turn*MEU_MAX_TURN;
    double accel=target==0 || target*t->speed<0?MEU_BRAKE_DECELERATION:MEU_ACCELERATION;
    double v=approach(t->speed,target,accel*dt),w=approach(t->turn_rate,angular,MEU_TURN_ACCELERATION*dt);
    MeuRobotPose candidate=integrate(*p,t->speed,v,t->turn_rate,w,dt);
    MeuRoute reason=meu_world_sweep(world,*p,candidate);
    if (reason==MEU_ROUTE_CLEAR) meu_travel_envelope(world,candidate,v,w,&reason);
    if (reason!=MEU_ROUTE_CLEAR) {
        t->reason=reason; t->latched=true; t->park_requested=true;
        v=approach(t->speed,0,MEU_BRAKE_DECELERATION*dt);
        w=approach(t->turn_rate,0,MEU_TURN_ACCELERATION*dt);
        candidate=integrate(*p,t->speed,v,t->turn_rate,w,dt);
        /* The previous accepted envelope reserved this braking trajectory.
           This guard also rejects malformed/unready fixture states. */
        if (meu_world_sweep(world,*p,candidate)!=MEU_ROUTE_CLEAR) {
            v=0; w=0; candidate=*p;
        }
    }
    t->speed=v; t->turn_rate=w; *p=candidate;
    t->parked=t->park_requested && v==0 && w==0 && meu_world_validate(world,*p,MEU_TRAVEL_CLEARANCE)==MEU_ROUTE_CLEAR;
}
