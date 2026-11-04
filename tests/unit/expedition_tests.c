#include "core/simulation.h"
#include "core/snapshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#c); return EXIT_FAILURE; } } while(0)
static void ticks(MeuSimulation *s,unsigned n) { for(unsigned i=0;i<n;++i) meu_sim_advance(s,MEU_TICK_SECONDS); }
int main(void)
{
    MeuSimulation s;meu_sim_init(&s);meu_sim_start(&s);s.station=3;
    CHECK(!meu_sim_station_command(&s,1));s.power.overload=true;ticks(&s,1);CHECK(s.expedition.fault);
    CHECK(meu_sim_station_command(&s,0));CHECK(!meu_sim_station_command(&s,1));s.power.isolated=true;
    unsigned a=s.expedition.alloy,c=s.expedition.conductor;
    CHECK(meu_sim_station_command(&s,1));CHECK(s.expedition.alloy==a-1 && s.expedition.conductor==c-2);
    CHECK(!meu_sim_station_command(&s,1));CHECK(s.expedition.alloy==a-1);
    CHECK(meu_sim_station_command(&s,2));CHECK(s.expedition.alloy==a && s.expedition.conductor==c);
    CHECK(!meu_sim_station_command(&s,2));CHECK(s.expedition.alloy==a);
    s.expedition.conductor=0;CHECK(!meu_sim_station_command(&s,1));CHECK(s.expedition.alloy==a);
    s.expedition.conductor=c;CHECK(meu_sim_station_command(&s,1));ticks(&s,240);CHECK(s.expedition.job_ticks==240);
    s.power.isolated=false;ticks(&s,60);CHECK(s.expedition.job_ticks==240);
    s.power.isolated=true;meu_sim_pause(&s);uint8_t data[MEU_SAVE_BYTES];
    CHECK(meu_snapshot_encode(&s,1,data)==MEU_SAVE_OK);MeuSimulation loaded;
    CHECK(meu_snapshot_decode(data,sizeof(data),&loaded,NULL)==MEU_SAVE_OK);
    CHECK(loaded.expedition.job_ticks==240 && loaded.expedition.alloy==a-1 && loaded.expedition.diagnosed);
    ticks(&loaded,600);CHECK(loaded.expedition.job_ticks==240);meu_sim_start(&loaded);ticks(&loaded,360);
    CHECK(!loaded.expedition.fault && !loaded.expedition.job && !loaded.power.overload);
    CHECK(loaded.expedition.alloy==a-1 && loaded.expedition.conductor==c-2 && meu_expedition_valid(&loaded.expedition));
    s=loaded;s.station=4;s.robot.z=-140;
    a=s.expedition.alloy;c=s.expedition.conductor;unsigned energy=s.power.energy;
    CHECK(meu_sim_station_command(&s,0));CHECK(s.expedition.alloy==a+4 && s.expedition.conductor==c+4 && s.power.energy==energy-150);
    CHECK(!meu_sim_station_command(&s,0));CHECK(!meu_sim_station_command(&s,1));
    s.expedition.coupler=true;CHECK(meu_sim_station_command(&s,1));CHECK(!meu_sim_station_command(&s,1));
    CHECK(!meu_sim_station_command(&s,2));s.power.cooling_priority=false;
    a=s.expedition.alloy;c=s.expedition.conductor;energy=s.power.energy;
    s.power.energy=0;CHECK(!meu_sim_station_command(&s,2));CHECK(s.expedition.alloy==a && s.expedition.conductor==c);
    s.power.energy=energy;CHECK(meu_sim_station_command(&s,2));CHECK(!meu_sim_station_command(&s,2));
    CHECK(s.expedition.alloy==a-3 && s.expedition.conductor==c-3);
    CHECK(meu_sim_station_command(&s,3));CHECK(s.expedition.installed);
    ticks(&s,1);CHECK(s.world==meu_world_with_bridge());
    CHECK(meu_world_validate(meu_world_default(),(MeuRobotPose){-170,-100,0},2)==MEU_ROUTE_SUPPORT);
    CHECK(meu_world_validate(s.world,(MeuRobotPose){-170,-100,0},2)==MEU_ROUTE_CLEAR);
    meu_sim_pause(&s);CHECK(meu_snapshot_encode(&s,2,data)==MEU_SAVE_OK);
    CHECK(meu_snapshot_decode(data,sizeof(data),&loaded,NULL)==MEU_SAVE_OK && loaded.expedition.installed && loaded.expedition.adapter);
    meu_sim_start(&s);s.station=4;s.power.grid=true;CHECK(!meu_sim_station_command(&s,4));s.power.grid=false;
    s.robot.x=-170;CHECK(!meu_sim_station_command(&s,4));s.robot.x=0;
    CHECK(meu_sim_station_command(&s,4));CHECK(s.expedition.adapter && !s.expedition.installed);
    CHECK(meu_sim_station_command(&s,3));
    puts("PASS diagnosis, isolation, helper stock reservation, cancellation, blocking and saved-job resume");return EXIT_SUCCESS;
}
