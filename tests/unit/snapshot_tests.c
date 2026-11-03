#include "core/snapshot.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#c); return EXIT_FAILURE; } } while (0)
static void put32(uint8_t *p,uint32_t v) { for (int i=0;i<4;++i) p[i]=(uint8_t)(v>>(i*8)); }
static void seal(uint8_t *data)
{
    uint32_t crc=UINT32_MAX;
    for (int i=0;i<124;++i) { crc^=data[i]; for (int b=0;b<8;++b) crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0); }
    put32(data+124,~crc);
}
int main(int argc,char **argv)
{
    CHECK(argc==2);
    MeuSimulation s,restored; meu_sim_init(&s);
    uint8_t data[MEU_SAVE_BYTES],golden[MEU_SAVE_BYTES],bad[MEU_SAVE_BYTES+1]; uint32_t generation=99;
    s.ticks=12345;
    CHECK(meu_snapshot_encode(&s,7,data)==MEU_SAVE_OK);
    FILE *fixture=fopen(argv[1],"rb"); CHECK(fixture!=NULL);
    CHECK(fread(golden,1,sizeof(golden),fixture)==sizeof(golden) && fgetc(fixture)==EOF); fclose(fixture);
    CHECK(memcmp(data,golden,sizeof(data))==0);
    CHECK(meu_snapshot_decode(data,sizeof(data),&restored,&generation)==MEU_SAVE_OK && generation==7);
    CHECK(restored.paused && restored.started && restored.held==0 && restored.travel.parked && !restored.travel.helm);
    CHECK(restored.x==s.x && restored.y==s.y && restored.z==s.z && restored.ticks==s.ticks);
    CHECK(restored.robot.x==s.robot.x && restored.robot.z==s.robot.z && restored.robot.yaw==s.robot.yaw);
    CHECK(restored.settings.sensitivity==s.settings.sensitivity && restored.settings.volume==s.settings.volume);
    for (int room=0;room<3;++room) {
        meu_sim_init(&s); s.lift_current=s.lift_target=room; s.lift_y=meu_deck_height(room);
        s.y=s.lift_y+MEU_EYE_HEIGHT; s.visited=15; s.robot.z=-10; s.robot.yaw=.02;
        s.settings=(MeuSettings){2.25f,.2f,true,true,true}; s.quarters_light=false; s.inspection_light=false;
        CHECK(meu_snapshot_encode(&s,12,data)==MEU_SAVE_OK);
        CHECK(meu_snapshot_decode(data,sizeof(data),&restored,NULL)==MEU_SAVE_OK);
        CHECK(meu_sim_room(&restored)==room && restored.y==s.y && restored.lift_current==room);
        CHECK(restored.settings.invert_y && restored.settings.walk_bob && restored.settings.machine_sway &&
            restored.settings.sensitivity==2.25f && restored.settings.volume==.2f && !restored.quarters_light && !restored.inspection_light);
        s.x=0; s.z=0; CHECK(meu_sim_on_lift(&s));
        CHECK(meu_snapshot_encode(&s,13,data)==MEU_SAVE_OK);
        CHECK(meu_snapshot_decode(data,sizeof(data),&restored,NULL)==MEU_SAVE_OK && meu_sim_on_lift(&restored));
    }
    meu_sim_init(&s); s.y=114.9f; s.z=-24; s.lift_current=s.lift_target=2;s.lift_y=112;s.visited=15;
    CHECK(meu_snapshot_encode(&s,15,data)==MEU_SAVE_OK);
    CHECK(meu_snapshot_decode(data,sizeof(data),&restored,NULL)==MEU_SAVE_OK && restored.y==s.y);
    memcpy(data,golden,sizeof(data));
    MeuSimulation unchanged=restored;
    for (size_t size=0;size<MEU_SAVE_BYTES;++size) {
        generation=99;
        CHECK(meu_snapshot_decode(data,size,&restored,&generation)!=MEU_SAVE_OK);
        CHECK(memcmp(&restored,&unchanged,sizeof(restored))==0 && generation==99);
    }
    memcpy(bad,data,sizeof(data));bad[MEU_SAVE_BYTES]=0;
    CHECK(meu_snapshot_decode(bad,sizeof(bad),&restored,NULL)==MEU_SAVE_MALFORMED);
    for (int i=0;i<MEU_SAVE_BYTES;++i) {
        memcpy(bad,data,sizeof(data)); bad[i]^=1;
        CHECK(meu_snapshot_decode(bad,MEU_SAVE_BYTES,&restored,NULL)!=MEU_SAVE_OK);
        CHECK(memcmp(&restored,&unchanged,sizeof(restored))==0);
    }
    /* Resealed hostile values prove that checksum verification is not the
       validation boundary for imported data. */
    const struct { int offset; uint32_t value; } invalid[]={
        {48,0x7fc00000},{52,0x7f800000},{56,0x7f800000},{60,0x41200000},{64,0x40000000},
        {68,0x7fc00000},{72,3},{76,2},{88,0},{88,16},{92,0},{92,0xffff},{96,0},{100,0x40000000},{104,1},
        {48,0x42c80000},{52,0x43480000},{28,0x7ff00000}
    };
    for (size_t i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) {
        memcpy(bad,data,sizeof(data)); put32(bad+invalid[i].offset,invalid[i].value); seal(bad);
        CHECK(meu_snapshot_decode(bad,MEU_SAVE_BYTES,&restored,NULL)!=MEU_SAVE_OK);
        CHECK(memcmp(&restored,&unchanged,sizeof(restored))==0);
    }
    memcpy(bad,data,sizeof(data));put32(bad+8,2);seal(bad);
    CHECK(meu_snapshot_decode(bad,MEU_SAVE_BYTES,&restored,NULL)==MEU_SAVE_SCHEMA_UNSUPPORTED);
    memcpy(bad,data,sizeof(data));put32(bad+12,99);seal(bad);
    CHECK(meu_snapshot_decode(bad,MEU_SAVE_BYTES,&restored,NULL)==MEU_SAVE_WORLD_UNSUPPORTED);
    char hex[MEU_SAVE_HEX_BYTES];meu_snapshot_hex(data,hex);
    CHECK(meu_snapshot_unhex(hex,bad) && memcmp(bad,data,MEU_SAVE_BYTES)==0);
    CHECK(!meu_snapshot_unhex(NULL,bad)); hex[5]='x'; CHECK(!meu_snapshot_unhex(hex,bad));
    meu_snapshot_hex(data,hex);hex[10]=0; CHECK(!meu_snapshot_unhex(hex,bad));
    meu_sim_init(&s);meu_sim_start(&s);meu_sim_action(&s,MEU_FORWARD,true);
    unchanged=s; generation=99;
    CHECK(meu_snapshot_restore(&s,data,sizeof(data),&generation)==MEU_SAVE_BUSY && generation==99 && memcmp(&s,&unchanged,sizeof(s))==0);
    meu_sim_open_journeys(&s);uint32_t epoch=s.input_epoch;
    CHECK(meu_snapshot_restore(&s,data,sizeof(data),&generation)==MEU_SAVE_OK && generation==7);
    CHECK(s.journeys_open && s.input_epoch!=epoch && s.held==0 && s.paused && s.travel.parked && s.travel.speed==0);
    CHECK(s.previous_x==s.x && s.previous_y==s.y && s.previous_z==s.z && s.accumulator==0);
    uint64_t ticks=s.ticks;meu_sim_advance(&s,600);CHECK(s.ticks==ticks);
    meu_sim_start(&s);CHECK(s.paused);meu_sim_close_journeys(&s);meu_sim_start(&s);CHECK(!s.paused && s.held==0);
    uint8_t canary[MEU_SAVE_BYTES];memset(canary,0x55,sizeof(canary));memcpy(bad,canary,sizeof(canary));
    CHECK(meu_snapshot_encode(&s,0,bad)==MEU_SAVE_MALFORMED && memcmp(bad,canary,sizeof(canary))==0);
    s.settings.volume=NAN;CHECK(meu_snapshot_encode(&s,1,bad)==MEU_SAVE_UNSAFE && memcmp(bad,canary,sizeof(canary))==0);
    meu_sim_init(&s);s.travel.parked=false;s.travel.speed=1;CHECK(meu_snapshot_encode(&s,1,bad)==MEU_SAVE_BUSY);
    meu_sim_init(&s);s.lift_target=1;CHECK(meu_snapshot_encode(&s,1,bad)==MEU_SAVE_BUSY);
    meu_sim_init(&s);s.x=9.9f;CHECK(meu_snapshot_encode(&s,1,bad)==MEU_SAVE_UNSAFE);
    /* Bounded malformed-input sweep under native ASan/UBSan and WebAssembly. */
    uint32_t random=1234567;
    for (int i=0;i<5000;++i) {
        memcpy(bad,data,sizeof(data));
        random=random*1664525u+1013904223u;unsigned offset=24+(random%20)*4;
        random=random*1664525u+1013904223u;put32(bad+offset,random);seal(bad);
        (void)meu_snapshot_decode(bad,MEU_SAVE_BYTES,NULL,NULL);
    }
    printf("PASS %d snapshot assertions and 5000 hostile payloads build=%s scene=%s\n",checks,MEU_BUILD_ID,MEU_SCENE_ID);
    return EXIT_SUCCESS;
}
