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
    puts("PASS diagnosis, isolation, helper stock reservation, cancellation, blocking and saved-job resume");return EXIT_SUCCESS;
}
