#include "core/simulation.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static int checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#c); return EXIT_FAILURE; } } while (0)
#define NEAR(a,b,e) (fabs((double)(a)-(double)(b))<(e))
static void frames(MeuSimulation *s,int n) { for (int i=0;i<n;++i) meu_sim_advance(s,MEU_TICK_SECONDS); }
static void helm(MeuSimulation *s)
{
    meu_sim_init(s); meu_sim_start(s); meu_sim_action(s,MEU_FORWARD,true); frames(s,100);
    meu_sim_action(s,MEU_FORWARD,false); meu_sim_action(s,MEU_INTERACT,true);
}
static const MeuWorldBox ground={-500,-8,-500,1000,8,1000,0,0,0,true};
int main(void)
{
    MeuSimulation s; meu_sim_init(&s); meu_sim_start(&s);
    CHECK(!meu_sim_action(&s,MEU_PARK,true) && !meu_sim_action(&s,MEU_BRAKE,true));
    meu_sim_action(&s,MEU_FORWARD,true); frames(&s,60);
    CHECK(s.robot.x==0 && s.robot.z==0 && s.travel.parked && !s.travel.helm);
    helm(&s); CHECK(s.travel.helm && s.held==0 && meu_sim_room(&s)==MEU_BRIDGE);
    float local_x=s.x,local_y=s.y,local_z=s.z;
    meu_sim_action(&s,MEU_LOOK_RIGHT,true); frames(&s,60); meu_sim_action(&s,MEU_LOOK_RIGHT,false);
    CHECK(s.yaw>1.4 && s.robot.yaw==0);
    meu_sim_action(&s,MEU_FORWARD,true); frames(&s,60);
    CHECK(NEAR(s.travel.speed,3,.00001) && NEAR(s.robot.z,-1.5,.00001) && s.robot.x==0);
    CHECK(s.x==local_x && s.y==local_y && s.z==local_z);
    /* The view turned, but travel remains aligned with the robot, not the gaze. */
    meu_sim_action(&s,MEU_FORWARD,false); double before=s.robot.z;
    frames(&s,60); CHECK(s.travel.speed==0 && NEAR(before-s.robot.z,1.125,.001));
    CHECK(!s.travel.parked); meu_sim_action(&s,MEU_PARK,true); frames(&s,1); CHECK(s.travel.parked);
    meu_sim_action(&s,MEU_FORWARD,true); frames(&s,240);
    CHECK(NEAR(s.travel.speed,MEU_MAX_SPEED,.00001));
    before=s.robot.z; meu_sim_action(&s,MEU_PARK,true);
    CHECK(s.travel.park_requested && !s.travel.parked && s.held==0);
    CHECK(!meu_sim_action(&s,MEU_FORWARD,true));
    frames(&s,182);
    CHECK(s.travel.parked && s.travel.speed==0 && NEAR(before-s.robot.z,18,.001));
    CHECK(meu_world_validate(s.world,s.robot,MEU_TRAVEL_CLEARANCE)==MEU_ROUTE_CLEAR);
    /* Releasing the helm during motion brakes while the Commander walks aboard. */
    meu_sim_action(&s,MEU_FORWARD,true); frames(&s,120);
    double speed=s.travel.speed; before=s.robot.z; local_z=s.z;
    CHECK(meu_sim_action(&s,MEU_INTERACT,true) && !s.travel.helm && s.held==0);
    meu_sim_action(&s,MEU_BACKWARD,true); frames(&s,120); meu_sim_action(&s,MEU_BACKWARD,false);
    CHECK(s.travel.parked && s.travel.speed==0 && hypotf(s.x-local_x,s.z-local_z)>2 && s.grounded);
    CHECK(NEAR(before-s.robot.z,meu_travel_stop_distance(speed),.001));
    /* Brake input clears the old throttle; releasing it cannot restart travel. */
    helm(&s); meu_sim_action(&s,MEU_BACKWARD,true); frames(&s,90);
    CHECK(s.travel.speed<0);
    meu_sim_action(&s,MEU_BRAKE,true); frames(&s,90); meu_sim_action(&s,MEU_BRAKE,false);
    CHECK(s.travel.speed==0 && !(s.held&15)); before=s.robot.z; frames(&s,120); CHECK(s.robot.z==before);
    /* Focus/menu loss freezes first, then resumes into a controlled parked stop. */
    helm(&s); meu_sim_action(&s,MEU_FORWARD,true); meu_sim_action(&s,MEU_RIGHT,true); frames(&s,80);
    meu_sim_pause(&s); MeuRobotPose frozen=s.robot; uint64_t tick=s.ticks; speed=s.travel.speed;
    frames(&s,1000);
    CHECK(s.robot.x==frozen.x && s.robot.z==frozen.z && s.ticks==tick && s.travel.speed==speed && !s.travel.helm);
    meu_sim_start(&s); frames(&s,120); CHECK(s.travel.parked && s.held==0 && s.travel.turn_rate==0);
    /* Analytic stopping boundaries, tested independently of the input driver. */
    MeuWorldBox wall_boxes[]={ground,{ -500,0,-80,1000,300,1,0,0,0,false }};
    MeuWorld wall={wall_boxes,2}; MeuRoute reason;
    CHECK(meu_travel_envelope(&wall,(MeuRobotPose){0,0,0},12,0,&reason));
    CHECK(!meu_travel_envelope(&wall,(MeuRobotPose){0,-3,0},12,0,&reason) && reason==MEU_ROUTE_STRUCTURE);
    MeuWorldBox cliff_box={-500,-8,-80,1000,8,580,0,0,0,true}; MeuWorld cliff={&cliff_box,1};
    CHECK(meu_travel_envelope(&cliff,(MeuRobotPose){0,-34,0},12,0,&reason));
    CHECK(!meu_travel_envelope(&cliff,(MeuRobotPose){0,-37,0},12,0,&reason) && reason==MEU_ROUTE_SUPPORT);
    /* A 1 cm unsupported gap lies inside the feet; all four corners alone miss it. */
    MeuWorldBox gap_boxes[]={{-500,-8,-500,1000,8,490,0,0,0,true},{-500,-8,-9.99,1000,8,509.99,0,0,0,true}};
    MeuWorld gap={gap_boxes,2};
    CHECK(meu_world_validate(&gap,(MeuRobotPose){0,0,0},0)==MEU_ROUTE_SUPPORT);
    MeuWorldBox body_boxes[]={ground,{50,70,0,10,5,10,0,0,0,false}}; MeuWorld body={body_boxes,2};
    CHECK(meu_world_validate(&body,(MeuRobotPose){0,0,0},0)==MEU_ROUTE_STRUCTURE); /* arm, not feet */
    body_boxes[1]=(MeuWorldBox){-200,235,-120,400,10,240,0,0,0,false};
    CHECK(meu_world_validate(&body,(MeuRobotPose){0,0,0},2)==MEU_ROUTE_STRUCTURE);
    body_boxes[1].y=246; CHECK(meu_world_validate(&body,(MeuRobotPose){0,0,0},2)==MEU_ROUTE_CLEAR);
    body_boxes[1]=(MeuWorldBox){58.5,155,-50,3,10,8,0,0,0,false};
    CHECK(meu_world_validate(&body,(MeuRobotPose){0,0,0},2)==MEU_ROUTE_CLEAR);
    CHECK(!meu_travel_envelope(&body,(MeuRobotPose){0,0,0},0,.1,&reason) && reason==MEU_ROUTE_STRUCTURE);
    CHECK(meu_world_validate(&wall,(MeuRobotPose){NAN,0,0},2)==MEU_ROUTE_INVALID);
    CHECK(!meu_travel_envelope(&wall,(MeuRobotPose){0,0,0},INFINITY,0,&reason));
    /* Drive into wall and cliff fixtures: every actual tick remains supported,
       clear, and decelerates at the controlled rate, including reverse travel. */
    double maximum_deceleration=0;
    for (int fixture=0;fixture<3;++fixture) {
        MeuTravel t; meu_travel_init(&t); t.helm=true; t.park_requested=false;
        MeuRobotPose p={0,0,0}; const MeuWorld *world=fixture==1?&cliff:&wall;
        int drive=1;
        if (fixture==2) { p.z=-160; drive=-1; }
        for (int i=0;i<900;++i) {
            double old=t.speed;
            meu_travel_tick(&t,&p,world,drive,0,false);
            double decel=(fabs(old)-fabs(t.speed))*60;
            if (decel>maximum_deceleration) maximum_deceleration=decel;
            CHECK(decel<=MEU_BRAKE_DECELERATION+.00001);
            CHECK(meu_world_validate(world,p,0)==MEU_ROUTE_CLEAR);
            if (t.parked) CHECK(t.speed==0 && t.turn_rate==0);
        }
        CHECK(t.latched && t.parked && t.reason==(fixture==1?MEU_ROUTE_SUPPORT:MEU_ROUTE_STRUCTURE));
        CHECK(fixture==1?p.z>-54:fixture==0?p.z>-21:p.z<-106);
    }
    MeuTravel invalid; meu_travel_init(&invalid); MeuRobotPose bad={0,0,0};
    meu_travel_tick(&invalid,&bad,&gap,0,0,false); CHECK(!invalid.parked && invalid.reason==MEU_ROUTE_SUPPORT);
    printf("PASS %d travel assertions max-deceleration=%.6f m/s2 build=%s scene=%s\n",
        checks,maximum_deceleration,MEU_BUILD_ID,MEU_SCENE_ID);
    return EXIT_SUCCESS;
}
