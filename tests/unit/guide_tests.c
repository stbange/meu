#include "game/guide.h"
#include "core/snapshot.h"
#include "meu_build_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void ticks(MeuSimulation *s,int n) { for(int i=0;i<n;i++)meu_sim_advance(s,MEU_TICK_SECONDS); }
static void command(MeuSimulation *s,int station,int row) { s->station=station;CHECK(meu_sim_station_command(s,row)); }
static void read_all(const MeuSimulation *s)
{
    MeuSimulation before=*s;
    for(int t=0;t<MEU_GUIDE_TOPICS;t++) {
        CHECK(*meu_guide_topic(t));
        for(int f=0;f<4;f++)CHECK(*meu_guide_text(s,t,f) && strlen(meu_guide_text(s,t,f))<600);
    }
    CHECK(memcmp(&before,s,sizeof(*s))==0);
}
int main(void)
{
    MeuSimulation s;meu_sim_init(&s);
    CHECK(meu_guide_step(&s)==MEU_GUIDE_WELCOME);
    meu_sim_open_help(&s);CHECK(s.help_open && s.paused && !s.started && meu_sim_context(&s)==MEU_CONTEXT_HELP);
    meu_sim_start(&s);CHECK(!s.started && s.paused);read_all(&s);
    meu_sim_close_help(&s);CHECK(s.paused);meu_sim_start(&s);
    CHECK(meu_guide_step(&s)==MEU_GUIDE_DOCK);
    s.robot.z=-140;s.robot.yaw=6.283185307179586;
    s.y=112+MEU_EYE_HEIGHT;s.lift_y=112;s.lift_current=s.lift_target=2;s.visited=5;s.z=-10;
    CHECK(meu_guide_step(&s)==MEU_GUIDE_ISOLATE);
    command(&s,2,1);CHECK(meu_guide_step(&s)==MEU_GUIDE_CONNECT);
    command(&s,2,2);CHECK(meu_guide_step(&s)==MEU_GUIDE_PULSE);
    for(int i=0;i<800 && !s.expedition.fault;i++)ticks(&s,1);
    CHECK(meu_guide_step(&s)==MEU_GUIDE_DIAGNOSE);
    command(&s,3,0);CHECK(meu_guide_step(&s)==MEU_GUIDE_QUEUE);
    command(&s,3,1);CHECK(meu_guide_step(&s)==MEU_GUIDE_REPAIR);
    s.station=0;CHECK(meu_sim_action(&s,MEU_FORWARD,true));
    meu_sim_open_help(&s);MeuSimulation frozen=s;
    ticks(&s,180);read_all(&s);CHECK(memcmp(&s,&frozen,sizeof(s))==0 && s.held==0);
    CHECK(!meu_sim_action(&s,MEU_FORWARD,true));
    meu_sim_pause(&s);CHECK(s.help_open && meu_sim_context(&s)==MEU_CONTEXT_HELP);
    meu_sim_close_help(&s);CHECK(s.paused);meu_sim_start(&s);CHECK(!s.held);
    command(&s,2,2);ticks(&s,610);CHECK(s.expedition.repaired && meu_guide_step(&s)==MEU_GUIDE_BULK);
    command(&s,4,0);CHECK(meu_guide_step(&s)==MEU_GUIDE_SCAN);
    command(&s,5,0);CHECK(meu_guide_step(&s)==MEU_GUIDE_SHELTER);
    command(&s,5,1);CHECK(meu_guide_step(&s)==MEU_GUIDE_SUIT);
    command(&s,5,2);CHECK(meu_guide_step(&s)==MEU_GUIDE_CONDUIT);
    meu_sim_open_help(&s);frozen=s;ticks(&s,180);read_all(&s);
    CHECK(memcmp(&s,&frozen,sizeof(s))==0);
    meu_sim_close_help(&s);meu_sim_start(&s);
    s.x=26;CHECK(meu_guide_step(&s)==MEU_GUIDE_BYPASS);
    command(&s,6,0);CHECK(meu_guide_step(&s)==MEU_GUIDE_EXTRACT);
    command(&s,6,1);CHECK(meu_guide_step(&s)==MEU_GUIDE_RETURN);
    s.expedition.suit_ticks=1700;CHECK(meu_guide_step(&s)==MEU_GUIDE_RETURN_NOW);
    s.expedition.suit_ticks=10800;s.expedition.shelter=false;CHECK(meu_guide_step(&s)==MEU_GUIDE_RETURN_NOW);
    s.expedition.shelter=true;s.expedition.catastrophe=true;CHECK(meu_guide_step(&s)==MEU_GUIDE_RECOVER);
    s.expedition.catastrophe=false;s.x=12;CHECK(meu_guide_step(&s)==MEU_GUIDE_UNSUIT);
    command(&s,5,3);CHECK(meu_guide_step(&s)==MEU_GUIDE_TEST);
    command(&s,4,1);s.power.cooling_priority=true;CHECK(meu_guide_step(&s)==MEU_GUIDE_PRIORITY);
    command(&s,2,3);CHECK(meu_guide_step(&s)==MEU_GUIDE_ADAPTER);
    command(&s,4,2);CHECK(meu_guide_step(&s)==MEU_GUIDE_INSTALL);
    command(&s,2,2);CHECK(meu_guide_step(&s)==MEU_GUIDE_DISCONNECT);
    command(&s,2,2);command(&s,4,3);CHECK(meu_guide_step(&s)==MEU_GUIDE_DEPART);
    command(&s,2,0);CHECK(meu_guide_step(&s)==MEU_GUIDE_PACK);command(&s,2,0);
    s.station=0;s.robot.x=-150;s.robot.z=-90;s.robot.yaw=4.71238898038469;ticks(&s,2);
    CHECK(s.expedition.completed && meu_guide_step(&s)==MEU_GUIDE_COMPLETE);
    meu_sim_pause(&s);uint8_t before[MEU_SAVE_BYTES],after[MEU_SAVE_BYTES];
    CHECK(meu_snapshot_encode(&s,18,before)==MEU_SAVE_OK);
    meu_sim_open_help(&s);read_all(&s);
    CHECK(meu_snapshot_encode(&s,18,after)==MEU_SAVE_OK && memcmp(before,after,sizeof(before))==0);
    MeuSimulation restored;meu_sim_init(&restored);
    CHECK(meu_snapshot_restore(&restored,after,sizeof(after),NULL)==MEU_SAVE_OK);
    CHECK(!restored.help_open && restored.paused && meu_guide_step(&restored)==MEU_GUIDE_COMPLETE);
    meu_sim_open_settings(&s);CHECK(!s.help_open && s.settings_open);
    meu_sim_open_help(&s);CHECK(!s.settings_open && !s.journeys_open);
    meu_sim_open_journeys(&s);CHECK(!s.help_open && s.journeys_open);
    meu_sim_init(&s);meu_sim_start(&s);s.robot.z=-140;
    s.power.energy=3000;CHECK(meu_guide_step(&s)==MEU_GUIDE_CHARGE);
    s.power.energy=18000;s.power.heat=9500;CHECK(meu_guide_step(&s)==MEU_GUIDE_COOL);
    s.power.heat=0;s.power.isolated=true;s.expedition.repaired=true;s.expedition.fault=true;
    CHECK(meu_guide_step(&s)==MEU_GUIDE_DIAGNOSE);
    s.expedition.diagnosed=true;s.expedition.conductor=0;CHECK(meu_guide_step(&s)==MEU_GUIDE_PARTS);
    CHECK(!*meu_guide_topic(-1) && !*meu_guide_topic(MEU_GUIDE_TOPICS));
    CHECK(!*meu_guide_text(&s,-1,0) && !*meu_guide_text(&s,0,4));
    printf("PASS %d guide/progression/lifecycle/save assertions build=%s\n",checks,MEU_BUILD_ID);
    return 0;
}
