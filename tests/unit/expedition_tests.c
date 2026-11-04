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
    meu_sim_init(&s);s.expedition.repaired=true;meu_sim_start(&s);s.station=5;
    CHECK(!meu_sim_station_command(&s,2));s.robot.z=-140;
    CHECK(!meu_sim_station_command(&s,2));CHECK(meu_sim_station_command(&s,0));
    CHECK(!meu_sim_station_command(&s,2));CHECK(meu_sim_station_command(&s,1));
    s.power.heat=6500;CHECK(!meu_sim_station_command(&s,2));s.power.heat=0;
    s.power.energy=100;CHECK(!meu_sim_station_command(&s,2));s.power.energy=18000;
    CHECK(meu_sim_station_command(&s,2));CHECK(s.expedition.suit && s.expedition.checkpoint);
    /* Fixture places the Commander at the engineering airlock; the crossing
       itself uses the normal movement/collision implementation. */
    s.x=12;s.z=-10;s.y=112+MEU_EYE_HEIGHT;s.lift_y=112;s.lift_current=s.lift_target=2;s.visited=15;
    meu_sim_move(&s,13,0);CHECK(s.x>24 && s.grounded);
    s.station=6;CHECK(meu_sim_station_command(&s,0));CHECK(meu_sim_station_command(&s,1));ticks(&s,120);
    CHECK(s.expedition.coupler && s.expedition.bypass && s.expedition.exposure==0);
    meu_sim_pause(&s);CHECK(meu_snapshot_encode(&s,3,data)==MEU_SAVE_OK);
    CHECK(meu_snapshot_decode(data,sizeof(data),&loaded,NULL)==MEU_SAVE_OK && loaded.expedition.suit && loaded.expedition.coupler);
    meu_sim_start(&loaded);meu_sim_move(&loaded,-13,0);CHECK(loaded.x<14);loaded.station=5;
    CHECK(meu_sim_station_command(&loaded,3));CHECK(!loaded.expedition.suit && loaded.expedition.coupler);
    /* Unsafe extraction is permitted with a warning, never a silent teleport. */
    meu_sim_start(&s);s.station=6;s.expedition.bypass=false;ticks(&s,600);
    CHECK(s.expedition.catastrophe && s.x>24 && s.station==5);
    CHECK(!meu_sim_station_command(&s,-1));CHECK(meu_sim_station_command(&s,7));
    CHECK(s.x==12 && !s.expedition.coupler && !s.expedition.suit && !s.expedition.catastrophe);
    CHECK(s.expedition.alloy==8 && s.expedition.conductor==8 && s.expedition.sealant==4);
    CHECK(meu_sim_valid_position(&s));
    /* Every egress gate rejects before checkpoint or inventory mutation. */
    for(int invalid=0;invalid<8;++invalid) {
        meu_sim_init(&s);meu_sim_start(&s);s.station=5;s.robot.z=-140;
        s.expedition.repaired=true;s.expedition.scanned=true;s.expedition.shelter=true;
        if(invalid==0)s.travel.parked=false;
        if(invalid==1)s.robot.yaw=.2;
        if(invalid==2)s.expedition.scanned=false;
        if(invalid==3)s.expedition.fault=true;
        if(invalid==4)s.expedition.shelter=false;
        if(invalid==5)s.power.energy=799;
        if(invalid==6)s.power.heat=6001;
        if(invalid==7)s.expedition.repaired=false;
        unsigned stock=s.expedition.alloy,charge=s.power.energy;
        CHECK(!meu_sim_station_command(&s,2));CHECK(!s.expedition.suit && !s.expedition.checkpoint);
        CHECK(s.expedition.alloy==stock && s.power.energy==charge);
    }
    meu_sim_init(&s);meu_sim_start(&s);s.robot.z=-140;s.station=4;s.expedition.repaired=true;
    s.expedition.alloy=100;unsigned charge=s.power.energy;
    CHECK(!meu_sim_station_command(&s,0));CHECK(s.expedition.alloy==100 && s.power.energy==charge && !s.expedition.bulk);
    /* A full turn is physically the same docking heading, including on reload. */
    meu_sim_init(&s);meu_sim_start(&s);s.station=5;s.robot.z=-140;s.robot.yaw=6.283185307179586;
    s.expedition.repaired=true;s.expedition.scanned=true;s.expedition.shelter=true;
    CHECK(meu_sim_station_command(&s,2));s.x=25;s.z=-10;s.y=112+MEU_EYE_HEIGHT;
    s.lift_current=s.lift_target=2;s.lift_y=112;s.visited=15;
    CHECK(meu_snapshot_encode(&s,5,data)==MEU_SAVE_OK);
    CHECK(meu_snapshot_decode(data,sizeof(data),&loaded,NULL)==MEU_SAVE_OK && loaded.expedition.suit);
    charge=s.power.energy;
    /* An invalid power state cannot exploit unsigned heat subtraction. */
    s.power.heat=10001;CHECK(!meu_power_spend(&s.power,1,1));CHECK(s.power.energy==charge);
    puts("PASS diagnosis, isolation, helper stock reservation, cancellation, blocking and saved-job resume");return EXIT_SUCCESS;
}
