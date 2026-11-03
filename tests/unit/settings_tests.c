#include "core/simulation.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static int checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#c); return EXIT_FAILURE; } } while (0)
#define NEAR(a,b) (fabs((double)(a)-(double)(b))<.00005)
static void frames(MeuSimulation *s,int n) { for (int i=0;i<n;++i) meu_sim_advance(s,MEU_TICK_SECONDS); }
int main(void)
{
    MeuSimulation s; meu_sim_init(&s);
    CHECK(meu_sim_context(&s)==MEU_CONTEXT_PAUSED && s.input_epoch==1);
    CHECK(NEAR(s.settings.sensitivity,1) && NEAR(s.settings.volume,.6));
    CHECK(!s.settings.invert_y && !s.settings.walk_bob && !s.settings.machine_sway);
    CHECK(!meu_sim_setting(&s,0,2));
    meu_sim_start(&s); meu_sim_action(&s,MEU_FORWARD,true); frames(&s,10);
    uint32_t epoch=s.input_epoch; float x=s.x,y=s.y,z=s.z; uint64_t ticks=s.ticks;
    meu_sim_open_settings(&s);
    CHECK(meu_sim_context(&s)==MEU_CONTEXT_SETTINGS && s.paused && s.held==0 && s.input_epoch!=epoch);
    CHECK(!meu_sim_action(&s,MEU_FORWARD,true)); meu_sim_start(&s); frames(&s,600);
    CHECK(s.paused && s.ticks==ticks && s.x==x && s.y==y && s.z==z);
    CHECK(meu_sim_setting(&s,0,100) && NEAR(s.settings.sensitivity,3));
    CHECK(meu_sim_setting(&s,0,-1) && NEAR(s.settings.sensitivity,.25));
    CHECK(!meu_sim_setting(&s,0,NAN) && !meu_sim_setting(&s,4,INFINITY));
    CHECK(!meu_sim_setting(&s,-1,1) && !meu_sim_setting(&s,MEU_SETTING_COUNT,1));
    CHECK(!meu_sim_setting(&s,1,.5f) && !s.settings.invert_y);
    CHECK(meu_sim_setting(&s,0,2) && meu_sim_setting(&s,1,1));
    CHECK(meu_sim_setting(&s,4,2) && NEAR(s.settings.volume,1));
    CHECK(meu_sim_setting(&s,4,-2) && s.settings.volume==0);
    meu_sim_pointer_look(&s,50,25); CHECK(s.yaw==0);
    meu_sim_close_settings(&s); CHECK(s.paused && meu_sim_context(&s)==MEU_CONTEXT_PAUSED);
    frames(&s,60); CHECK(s.ticks==ticks);
    meu_sim_start(&s); CHECK(s.held==0);
    float pitch=s.pitch;
    meu_sim_pointer_look(&s,50,25); CHECK(NEAR(s.yaw,.4) && NEAR(s.pitch,pitch+.2));
    meu_sim_pointer_look(&s,NAN,3); CHECK(NEAR(s.yaw,.4));
    meu_sim_action(&s,MEU_LOOK_RIGHT,true); frames(&s,30); meu_sim_action(&s,MEU_LOOK_RIGHT,false);
    CHECK(NEAR(s.yaw,1.9) && s.robot.yaw==0);
    /* A station panel locks gameplay and clears the prior on-foot gesture. */
    meu_sim_init(&s); meu_sim_start(&s); s.x=3.85f; s.z=-8;
    CHECK(meu_sim_use_target(&s)==MEU_USE_PLACARD);
    meu_sim_action(&s,MEU_FORWARD,true); epoch=s.input_epoch;
    CHECK(meu_sim_action(&s,MEU_INTERACT,true) && meu_sim_context(&s)==MEU_CONTEXT_PANEL);
    CHECK(s.held==0 && s.input_epoch!=epoch);
    CHECK(!meu_sim_action(&s,MEU_FORWARD,true));
    x=s.x; y=s.y; z=s.z; float yaw=s.yaw;
    meu_sim_pointer_look(&s,100,100); frames(&s,60);
    CHECK(s.x==x && s.y==y && s.z==z && s.yaw==yaw);
    CHECK(meu_sim_action(&s,MEU_INTERACT,true) && meu_sim_context(&s)==MEU_CONTEXT_FOOT && s.held==0);
    /* All camera options alter only the rendered eye, never collision/footing. */
    meu_sim_init(&s); meu_sim_start(&s); meu_sim_action(&s,MEU_FORWARD,true); frames(&s,30);
    MeuCamera eye=meu_sim_camera(&s); CHECK(eye.x==s.x && eye.y==s.y && eye.z==s.z);
    y=s.y; s.settings.walk_bob=true; eye=meu_sim_camera(&s);
    CHECK(fabsf(eye.y-y)>.001f && fabsf(eye.y-y)<=.0251f && s.y==y && s.grounded);
    s.settings.walk_bob=false; s.settings.machine_sway=true; s.travel.speed=8; s.ticks=60;
    eye=meu_sim_camera(&s);
    CHECK(hypotf(eye.x-s.x,eye.y-s.y)>.001f && hypotf(eye.x-s.x,eye.y-s.y)<.02f && s.y==y);
    s.settings.machine_sway=false; eye=meu_sim_camera(&s); CHECK(eye.x==s.x && eye.y==s.y);
    /* Opening settings at a live helm freezes first, then resumes into braking. */
    meu_sim_init(&s); meu_sim_start(&s); meu_sim_action(&s,MEU_FORWARD,true); frames(&s,100);
    meu_sim_action(&s,MEU_FORWARD,false); CHECK(meu_sim_action(&s,MEU_INTERACT,true));
    meu_sim_action(&s,MEU_FORWARD,true); frames(&s,90); CHECK(s.travel.speed>4);
    meu_sim_open_settings(&s); double speed=s.travel.speed; MeuRobotPose pose=s.robot;
    CHECK(!s.travel.helm && s.travel.park_requested && !s.travel.parked && s.held==0);
    ticks=s.ticks; frames(&s,1000);
    CHECK(s.ticks==ticks && s.travel.speed==speed && s.robot.x==pose.x && s.robot.z==pose.z);
    meu_sim_close_settings(&s); frames(&s,60); CHECK(s.ticks==ticks);
    meu_sim_start(&s); frames(&s,100);
    CHECK(s.travel.parked && s.travel.speed==0 && s.held==0 && !s.travel.helm);
    meu_settings_default(&s.settings);
    CHECK(NEAR(s.settings.sensitivity,1) && NEAR(s.settings.volume,.6) && !s.settings.invert_y && !s.settings.walk_bob && !s.settings.machine_sway);
    printf("PASS %d settings/context assertions build=%s scene=%s\n",checks,MEU_BUILD_ID,MEU_SCENE_ID);
    return EXIT_SUCCESS;
}
