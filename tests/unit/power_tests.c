#include "core/simulation.h"
#include "core/snapshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#c); return EXIT_FAILURE; } } while(0)
static void seconds(MeuSimulation *s,unsigned n) { for (unsigned i=0;i<n*60;++i) meu_sim_advance(s,MEU_TICK_SECONDS); }
int main(void)
{
    MeuSimulation s;meu_sim_init(&s);meu_sim_start(&s);
    unsigned energy=s.power.energy;meu_sim_pause(&s);seconds(&s,600);CHECK(s.power.energy==energy && s.power.clock==120);
    meu_sim_start(&s);s.station=2;CHECK(meu_sim_station_command(&s,0));seconds(&s,30);CHECK(s.power.energy>energy+1500);
    CHECK(!meu_sim_station_command(&s,2));CHECK(!s.power.grid);
    s.robot.z=-140;CHECK(!meu_sim_station_command(&s,2));CHECK(meu_sim_station_command(&s,1));CHECK(meu_sim_station_command(&s,2));
    seconds(&s,12);CHECK(!s.power.grid && s.power.overload && s.power.isolated);
    MeuPower before=s.power;CHECK(!meu_power_spend(&s.power,MEU_ENERGY_CAPACITY+1,0));CHECK(memcmp(&before,&s.power,sizeof(before))==0);
    CHECK(!meu_power_spend(&s.power,0,10001));
    s.power.energy=MEU_ENERGY_CAPACITY;seconds(&s,600);CHECK(s.power.energy<=MEU_ENERGY_CAPACITY && meu_power_valid(&s.power));
    s.power.energy=0;s.power.collectors=false;s.power.clock=0;seconds(&s,1);CHECK(s.power.energy==0);
    s.power.energy=18000;s.power.heat=9990;s.power.grid=true;seconds(&s,1);CHECK(!s.power.grid && s.power.heat<=10000);
    meu_sim_pause(&s);uint8_t data[MEU_SAVE_BYTES];CHECK(meu_snapshot_encode(&s,1,data)==MEU_SAVE_OK);
    MeuSimulation restored;CHECK(meu_snapshot_decode(data,sizeof(data),&restored,NULL)==MEU_SAVE_OK);
    CHECK(restored.power.energy==s.power.energy && restored.power.heat==s.power.heat && restored.power.clock==s.power.clock);
    CHECK(restored.power.isolated==s.power.isolated && restored.power.collectors==s.power.collectors);
    meu_sim_init(&s);s.power.energy=0;meu_sim_start(&s);s.travel.helm=true;meu_sim_action(&s,MEU_FORWARD,true);seconds(&s,2);
    CHECK(s.travel.parked && s.travel.speed==0);CHECK(meu_sim_request_lift(&s,1)==false);
    puts("PASS power conservation, parked collectors, isolation, overload, pause, bounded heat, save and travel reserve");return EXIT_SUCCESS;
}
