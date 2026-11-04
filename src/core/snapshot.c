#include "core/snapshot.h"
#include <float.h>
#include <math.h>
#include <string.h>
_Static_assert(sizeof(float)==4 && FLT_MANT_DIG==24 && sizeof(double)==8 && DBL_MANT_DIG==53 && FLT_RADIX==2,"Snapshots require IEEE 754 binary32/binary64");
static const uint8_t magic[8]={'M','E','U','S','A','V','E',0};
static void put32(uint8_t *p,uint32_t v) { for (int i=0;i<4;++i) p[i]=(uint8_t)(v>>(i*8)); }
static uint32_t get32(const uint8_t *p) { uint32_t v=0; for (int i=0;i<4;++i) v|=(uint32_t)p[i]<<(i*8); return v; }
static void put64(uint8_t *p,uint64_t v) { for (int i=0;i<8;++i) p[i]=(uint8_t)(v>>(i*8)); }
static uint64_t get64(const uint8_t *p) { uint64_t v=0; for (int i=0;i<8;++i) v|=(uint64_t)p[i]<<(i*8); return v; }
static void putf(uint8_t *p,float v) { uint32_t u; memcpy(&u,&v,4); put32(p,u); }
static float getf(const uint8_t *p) { uint32_t u=get32(p); float v; memcpy(&v,&u,4); return v; }
static void putd(uint8_t *p,double v) { uint64_t u; memcpy(&u,&v,8); put64(p,u); }
static double getd(const uint8_t *p) { uint64_t u=get64(p); double v; memcpy(&v,&u,8); return v; }
static uint32_t checksum(const uint8_t *data,size_t size)
{
    uint32_t crc=UINT32_MAX;
    for (size_t i=0;i<size;++i) {
        crc^=data[i];
        for (int bit=0;bit<8;++bit) crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0);
    }
    return ~crc;
}
static bool bounded(double value,double minimum,double maximum) { return isfinite(value) && value>=minimum && value<=maximum; }
static MeuSaveResult valid(const MeuSimulation *s)
{
    if (!meu_power_valid(&s->power) || !meu_expedition_valid(&s->expedition)) return MEU_SAVE_UNSAFE;
    if (s->world!=meu_world_default() && s->world!=meu_world_with_bridge()) return MEU_SAVE_WORLD_UNSUPPORTED;
    if (!bounded(s->x,-100,100) || !bounded(s->y,100,250) || !bounded(s->z,-100,100) ||
        !bounded(s->yaw,-3.141594,3.141594) || !bounded(s->pitch,-1.400001,1.400001) ||
        !bounded(s->robot.x,-1000000,1000000) || !bounded(s->robot.z,-1000000,1000000) ||
        !bounded(s->robot.yaw,-1000000,1000000) || !bounded(s->settings.sensitivity,.25,3) ||
        !bounded(s->settings.volume,0,1) || s->ticks>UINT64_C(9007199254740991) ||
        (s->visited&~UINT32_C(15)) || !(s->visited&(UINT32_C(1)<<meu_sim_room(s)))) return MEU_SAVE_UNSAFE;
    if (s->x>14 && (!s->expedition.suit || !s->expedition.scanned || !s->expedition.checkpoint ||
        s->robot.z > -120 || s->robot.z < -175 || fabs(s->robot.x)>15 || fabs(s->robot.yaw)>=.05)) return MEU_SAVE_UNSAFE;
    if (!s->travel.parked || s->travel.speed!=0 || s->travel.turn_rate!=0 || s->travel.helm ||
        s->motion_fixture || s->lift_velocity!=0 || meu_sim_lift_moving(s)) return MEU_SAVE_BUSY;
    if (s->lift_current<0 || s->lift_current>2 || s->lift_target!=s->lift_current ||
        !bounded(s->lift_y,112,196) || !bounded(s->lift_door,.98,1) || fabsf(s->lift_y-meu_deck_height(s->lift_current))>.001f ||
        !meu_sim_valid_position(s) || meu_world_validate(s->world,s->robot,MEU_TRAVEL_CLEARANCE)!=MEU_ROUTE_CLEAR)
        return MEU_SAVE_UNSAFE;
    return MEU_SAVE_OK;
}
MeuSaveResult meu_snapshot_encode(const MeuSimulation *s,uint32_t generation,uint8_t out[MEU_SAVE_BYTES])
{
    MeuSaveResult result=valid(s);
    if (result!=MEU_SAVE_OK) return result;
    if (!generation) return MEU_SAVE_MALFORMED;
    uint8_t data[MEU_SAVE_BYTES]={0};
    memcpy(data,magic,8); put32(data+8,MEU_SAVE_SCHEMA); put32(data+12,MEU_SAVE_WORLD);
    put32(data+16,generation); put32(data+20,MEU_SAVE_BYTES);
    putd(data+24,s->robot.x); putd(data+32,s->robot.z); putd(data+40,s->robot.yaw);
    putf(data+48,s->x); putf(data+52,s->y); putf(data+56,s->z); putf(data+60,s->yaw); putf(data+64,s->pitch);
    putf(data+68,s->lift_y); put32(data+72,(uint32_t)s->lift_current); put32(data+76,(uint32_t)meu_sim_room(s));
    put64(data+80,s->ticks); put32(data+88,s->visited);
    uint32_t flags=32u | (s->quarters_light?1u:0) | (s->inspection_light?2u:0) |
        (s->settings.invert_y?4u:0) | (s->settings.walk_bob?8u:0) | (s->settings.machine_sway?16u:0);
    put32(data+92,flags); putf(data+96,s->settings.sensitivity); putf(data+100,s->settings.volume);
    put32(data+124,checksum(data,124));
    put32(data+128,s->power.energy); put32(data+132,s->power.heat);
    put32(data+136,s->power.clock); put32(data+140,s->power.phase);
    put32(data+144,(s->power.collectors?1u:0)|(s->power.grid?2u:0)|(s->power.isolated?4u:0)|
        (s->power.cooling_priority?8u:0)|(s->power.overload?16u:0));
    put32(data+148,s->expedition.alloy);put32(data+152,s->expedition.conductor);put32(data+156,s->expedition.sealant);
    put32(data+160,(s->expedition.fault?1u:0)|(s->expedition.diagnosed?2u:0));
    put32(data+164,s->expedition.job);put32(data+168,s->expedition.job_ticks);
    put32(data+172,meu_expedition_flags(&s->expedition));
    put32(data+176,s->expedition.suit_ticks);put32(data+180,s->expedition.exposure);
    if(s->expedition.checkpoint) {
        put32(data+184,s->expedition.checkpoint_alloy);put32(data+188,s->expedition.checkpoint_conductor);
        put32(data+192,s->expedition.checkpoint_sealant);put32(data+196,s->expedition.checkpoint_flags);
        const MeuPower *cp=&s->expedition.checkpoint_power;
        put32(data+200,cp->energy);put32(data+204,cp->heat);put32(data+208,cp->clock);put32(data+212,cp->phase);
        put32(data+216,(cp->collectors?1u:0)|(cp->grid?2u:0)|(cp->isolated?4u:0)|(cp->cooling_priority?8u:0)|(cp->overload?16u:0));
    }
    put32(data+252,checksum(data,252)); memcpy(out,data,sizeof(data)); return MEU_SAVE_OK;
}
MeuSaveResult meu_snapshot_decode(const uint8_t *data,size_t size,MeuSimulation *out,uint32_t *generation)
{
    if (!data || size<24 || memcmp(data,magic,8)) return MEU_SAVE_MALFORMED;
    uint32_t schema=get32(data+8),bytes=schema==1?128:MEU_SAVE_BYTES;
    if (schema!=1 && schema!=MEU_SAVE_SCHEMA) return MEU_SAVE_SCHEMA_UNSUPPORTED;
    if ((size!=bytes && !(schema==1 && size==MEU_SAVE_BYTES)) || get32(data+20)!=bytes || !get32(data+16) ||
        get32(data+124)!=checksum(data,124)) return MEU_SAVE_MALFORMED;
    if (schema==1 && size>128) for (size_t i=128;i<size;++i) if(data[i]) return MEU_SAVE_MALFORMED;
    if (schema==2 && get32(data+252)!=checksum(data,252)) return MEU_SAVE_MALFORMED;
    if (get32(data+12)!=MEU_SAVE_WORLD) return MEU_SAVE_WORLD_UNSUPPORTED;
    for (int i=104;i<124;++i) if (data[i]) return MEU_SAVE_MALFORMED;
    uint32_t flags=get32(data+92);
    if ((flags&~UINT32_C(63)) || !(flags&32) || get32(data+72)>2 || get32(data+76)>3) return MEU_SAVE_MALFORMED;
    MeuSimulation candidate; meu_sim_init(&candidate);
    candidate.robot=(MeuRobotPose){getd(data+24),getd(data+32),getd(data+40)};
    candidate.x=getf(data+48); candidate.y=getf(data+52); candidate.z=getf(data+56);
    candidate.yaw=getf(data+60); candidate.pitch=getf(data+64); candidate.lift_y=getf(data+68);
    candidate.lift_current=candidate.lift_target=(int)get32(data+72);
    candidate.ticks=get64(data+80); candidate.visited=get32(data+88);
    candidate.quarters_light=(flags&1)!=0; candidate.inspection_light=(flags&2)!=0;
    candidate.settings=(MeuSettings){getf(data+96),getf(data+100),(flags&4)!=0,(flags&8)!=0,(flags&16)!=0};
    if (schema==2) {
        uint32_t bits=get32(data+144);
        if (bits&~31u) return MEU_SAVE_MALFORMED;
        for (int i=220;i<252;++i) if (data[i]) return MEU_SAVE_MALFORMED;
        uint32_t progress=get32(data+160);if(progress&~3u) return MEU_SAVE_MALFORMED;
        candidate.expedition=(MeuExpedition){.alloy=get32(data+148),.conductor=get32(data+152),.sealant=get32(data+156),
            .fault=(progress&1)!=0,.diagnosed=(progress&2)!=0,.job=get32(data+164),.job_ticks=get32(data+168)};
        uint32_t discovery=get32(data+172);if(discovery&~8191u)return MEU_SAVE_MALFORMED;
        meu_expedition_set_flags(&candidate.expedition,discovery);
        candidate.expedition.suit_ticks=get32(data+176);candidate.expedition.exposure=get32(data+180);
        candidate.expedition.checkpoint_alloy=get32(data+184);candidate.expedition.checkpoint_conductor=get32(data+188);
        candidate.expedition.checkpoint_sealant=get32(data+192);candidate.expedition.checkpoint_flags=get32(data+196);
        uint32_t cpbits=get32(data+216);if(cpbits&~31u)return MEU_SAVE_MALFORMED;
        if(!candidate.expedition.checkpoint) for(int i=184;i<220;++i)if(data[i])return MEU_SAVE_MALFORMED;
        candidate.expedition.checkpoint_power=(MeuPower){get32(data+200),get32(data+204),get32(data+208),get32(data+212),
            (cpbits&1)!=0,(cpbits&2)!=0,(cpbits&4)!=0,(cpbits&8)!=0,(cpbits&16)!=0};
        candidate.power=(MeuPower){get32(data+128),get32(data+132),get32(data+136),get32(data+140),
            (bits&1)!=0,(bits&2)!=0,(bits&4)!=0,(bits&8)!=0,(bits&16)!=0};
    }
    candidate.world=candidate.expedition.installed?meu_world_with_bridge():meu_world_default();
    candidate.started=true;
    MeuSaveResult result=valid(&candidate);
    if (result!=MEU_SAVE_OK) return result;
    if ((uint32_t)meu_sim_room(&candidate)!=get32(data+76)) return MEU_SAVE_UNSAFE;
    /* Also resets interpolation, held actions and accumulated/background time. */
    meu_sim_pause(&candidate);
    if (out) *out=candidate;
    if (generation) *generation=get32(data+16);
    return MEU_SAVE_OK;
}
MeuSaveResult meu_snapshot_restore(MeuSimulation *s,const uint8_t *data,size_t size,uint32_t *generation)
{
    if (!s->paused) return MEU_SAVE_BUSY;
    MeuSimulation candidate;
    MeuSaveResult result=meu_snapshot_decode(data,size,&candidate,generation);
    if (result!=MEU_SAVE_OK) return result;
    candidate.input_epoch=s->input_epoch+1; candidate.journeys_open=s->journeys_open;
    *s=candidate; return MEU_SAVE_OK;
}
void meu_snapshot_hex(const uint8_t data[MEU_SAVE_BYTES],char out[MEU_SAVE_HEX_BYTES])
{
    static const char hex[]="0123456789abcdef";
    for (int i=0;i<MEU_SAVE_BYTES;++i) { out[i*2]=hex[data[i]>>4]; out[i*2+1]=hex[data[i]&15]; }
    out[MEU_SAVE_BYTES*2]=0;
}
static int nibble(char c) { return c>='0' && c<='9'?c-'0':c>='a' && c<='f'?c-'a'+10:-1; }
bool meu_snapshot_unhex(const char *text,uint8_t out[MEU_SAVE_BYTES])
{
    if (!text) return false;
    size_t length=0; while (length<MEU_SAVE_HEX_BYTES && text[length]) ++length;
    if (length!=MEU_SAVE_BYTES*2 && length!=256) return false;
    uint8_t data[MEU_SAVE_BYTES]={0};
    for (size_t i=0;i<length/2;++i) {
        int a=nibble(text[i*2]),b=nibble(text[i*2+1]);
        if (a<0 || b<0) return false;
        data[i]=(uint8_t)(a*16+b);
    }
    memcpy(out,data,sizeof(data)); return true;
}
const char *meu_save_message(MeuSaveResult result)
{
    switch (result) {
        case MEU_SAVE_OK:return "Journey validated.";
        case MEU_SAVE_SCHEMA_UNSUPPORTED:return "Unsupported save version. The source was kept.";
        case MEU_SAVE_WORLD_UNSUPPORTED:return "This save uses incompatible world content. The source was kept.";
        case MEU_SAVE_UNSAFE:return "The saved position or settings are invalid. The source was kept.";
        case MEU_SAVE_BUSY:return "Park the unit and finish the lift ride before saving. Resume to let it stop.";
        case MEU_SAVE_IO:return "Storage is unavailable. Export a backup or try again.";
        case MEU_SAVE_MISSING:return "No saved journey in that slot.";
        default:return "The save is damaged or incomplete. The source was kept.";
    }
}
