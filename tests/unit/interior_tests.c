#include "core/simulation.h"
#include "core/matrix.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static int checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#c); return EXIT_FAILURE; } } while (0)
#define NEAR(a,b,e) (fabs((double)(a)-(double)(b))<(e))
static void frames(MeuSimulation *s,int n) { for (int i=0;i<n;++i) meu_sim_advance(s,MEU_TICK_SECONDS); }
static void walk(MeuSimulation *s,int action,int n) { meu_sim_action(s,action,true); frames(s,n); meu_sim_action(s,action,false); }
int main(void)
{
    MeuSimulation s,render; meu_sim_init(&s); meu_sim_start(&s);
    CHECK(MEU_ROBOT_HEIGHT==240 && meu_deck_height(0)-meu_deck_height(2)==84);
    CHECK(!meu_sim_request_lift(&s,1));
    walk(&s,MEU_BACKWARD,180);
    CHECK(meu_sim_on_lift(&s) && NEAR(s.z,-.5,.002));
    CHECK(meu_sim_request_lift(&s,1));
    frames(&s,45); CHECK(meu_sim_lift_moving(&s) && s.lift_door==0);
    CHECK(!meu_sim_request_lift(&s,2));
    walk(&s,MEU_FORWARD,120);
    CHECK(s.z>-1.82 && s.grounded);
    meu_sim_pause(&s); float y=s.y,car=s.lift_y; frames(&s,600);
    CHECK(s.y==y && s.lift_y==car && s.held==0);
    meu_sim_start(&s); frames(&s,420);
    CHECK(!meu_sim_lift_moving(&s) && NEAR(s.y,157.7,.002));
    walk(&s,MEU_FORWARD,160); CHECK(meu_sim_room(&s)==MEU_QUARTERS);
    /* A swept capsule cannot tunnel through the 2 m bed or the wall. */
    s.x=-5.7f; s.z=-12; s.y=157.7f;
    meu_sim_move(&s,0,-8); CHECK(s.z>-13.4f && s.z<-13.25f && s.grounded);
    s.x=0; s.z=-10; meu_sim_move(&s,100,0);
    CHECK(s.x<7.73f && s.x>7.6f && s.grounded);
    CHECK(meu_sim_use_target(&s)==MEU_USE_NONE);
    s.x=5; s.z=-7.25f; s.y=157.7f; s.yaw=1.5707963f; s.pitch=0;
    CHECK(meu_sim_use_target(&s)==MEU_USE_LAMP);
    CHECK(meu_sim_action(&s,MEU_INTERACT,true) && !s.quarters_light);
    s.yaw=-1.5707963f; CHECK(meu_sim_use_target(&s)==MEU_USE_NONE);
    /* Unattended lift can be summoned; the empty shaft stays gated. */
    s.x=0; s.z=-3.6f; s.y=157.7f; s.yaw=-.9f;
    s.lift_y=196; s.lift_target=0; s.lift_current=0; s.lift_door=1;
    meu_sim_move(&s,0,4); CHECK(s.z<-2.5f);
    s.x=0; s.z=-3.4f;
    CHECK(meu_sim_use_target(&s)==MEU_USE_CALL);
    CHECK(meu_sim_action(&s,MEU_INTERACT,true)); frames(&s,420);
    CHECK(NEAR(s.lift_y,156,.001) && !meu_sim_lift_moving(&s));
    s.x=0; s.z=-1.8f; CHECK(!meu_sim_lift_ready(&s) && !meu_sim_request_lift(&s,2));
    s.z=0; s.x=1.75f; CHECK(meu_sim_on_lift(&s) && !meu_sim_lift_ready(&s));
    s.x=0; CHECK(meu_sim_lift_ready(&s));
    /* Ramp ascent and descent stay smooth, without floor penetration. */
    s.x=0; s.z=-10; s.y=113.7f;
    for (int i=0;i<300;++i) {
        float before=s.y; meu_sim_move(&s,0,-.04f);
        CHECK(s.grounded && s.y>=before-.001f && s.y-before<.02f);
    }
    CHECK(NEAR(s.y,114.9,.002)); meu_sim_move(&s,0,12); CHECK(NEAR(s.y,113.7,.002));
    /* Translation, turning, acceleration and braking never integrate local pose twice. */
    meu_sim_init(&s); meu_sim_start(&s); s.motion_fixture=true;
    double max_drift=0;
    for (int i=0;i<3600;++i) {
        meu_sim_advance(&s,MEU_TICK_SECONDS);
        double world[3],local[3]; meu_pose_to_world(s.robot,s.x,s.y,s.z,world); meu_pose_to_local(s.robot,world,local);
        double drift=hypot(local[0],local[2]+8); if (drift>max_drift) max_drift=drift;
        CHECK(s.grounded && NEAR(local[1],197.7,.001));
        float inv[16]; meu_pose_inverse_matrix(s.robot,inv);
        CHECK(NEAR(inv[0]*world[0]+inv[8]*world[2]+inv[12],s.x,.00002));
        CHECK(NEAR(inv[2]*world[0]+inv[10]*world[2]+inv[14],s.z,.00002));
    }
    CHECK(max_drift<.02);
    s.x=0; s.z=0; s.y=s.lift_y+MEU_EYE_HEIGHT; CHECK(meu_sim_request_lift(&s,2));
    frames(&s,160); meu_sim_advance(&s,MEU_TICK_SECONDS*.5); meu_sim_render_state(&s,&render);
    CHECK(NEAR(render.y-render.lift_y,MEU_EYE_HEIGHT,.00005));
    printf("PASS %d interior assertions drift=%.9f m build=%s scene=%s\n",checks,max_drift,MEU_BUILD_ID,MEU_SCENE_ID);
    return EXIT_SUCCESS;
}
