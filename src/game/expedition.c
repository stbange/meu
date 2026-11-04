#include "game/expedition.h"
#include <string.h>
void meu_expedition_init(MeuExpedition *e)
{
    memset(e,0,sizeof(*e));e->alloy=8;e->conductor=8;e->sealant=4;
}
bool meu_expedition_valid(const MeuExpedition *e)
{
    if(e->checkpoint) {
        MeuExpedition checkpoint={0};
        checkpoint.alloy=e->checkpoint_alloy;checkpoint.conductor=e->checkpoint_conductor;checkpoint.sealant=e->checkpoint_sealant;
        meu_expedition_set_flags(&checkpoint,e->checkpoint_flags);
        if(!meu_expedition_valid(&checkpoint))return false;
    }
    return (!e->completed || (e->adapter && e->repaired)) && e->suit_ticks<=10800 && e->exposure<=600 &&
        (!e->checkpoint || e->suit) &&
        (e->suit || (!e->suit_ticks && !e->exposure)) &&
        (!e->catastrophe || (e->suit && e->checkpoint)) && (!e->suit || (e->checkpoint && e->scanned)) &&
        (!e->checkpoint || (e->checkpoint_alloy<=100 && e->checkpoint_conductor<=100 && e->checkpoint_sealant<=100 &&
            !(e->checkpoint_flags&~6399u) && meu_power_valid(&e->checkpoint_power))) &&
        e->alloy<=100 && e->conductor<=100 && e->sealant<=100 && e->job<=1 && e->job_ticks<=600 &&
        (e->fault || !e->diagnosed) && (e->job || !e->job_ticks) &&
        (!e->installed || (e->coupler && e->tested && e->adapter)) &&
        (!e->adapter || (e->coupler && e->tested)) && (!e->tested || e->coupler) &&
        (!e->job || (e->fault && e->diagnosed && e->alloy<=99 && e->conductor<=98));
}
const char *meu_helper_status(const MeuExpedition *e,const MeuPower *p)
{
    if (!e->job) return "Helper idle. Known repairs only; unfamiliar technology needs your test bench.";
    if (!p->isolated) return "Helper blocked: isolate the workshop bus.";
    if (p->energy<2) return "Helper blocked: insufficient main energy; deploy collectors.";
    return "Helper replacing the diagnosed distribution buffer (10 active seconds).";
}
const char *meu_maintenance_command(MeuExpedition *e,MeuPower *p,int action,bool *accepted)
{
    *accepted=false;
    if (action==0) {
        if (!e->fault) return "Displays and salvage bus normal. No parts needed.";
        e->diagnosed=true;*accepted=true;return "Diagnosis: surge-damaged distribution buffer; isolate, reserve 1 alloy + 2 conductor.";
    }
    if (action==1) {
        if (!e->fault || !e->diagnosed) return "Inspect symptoms and diagnose the cause before assigning a repair.";
        if (e->job) return "A repair is already queued. No duplicate reservation.";
        if (!p->isolated) return "Isolate the workshop bus; essential doors stay powered.";
        if (e->alloy<1 || e->conductor<2) return "Missing repair parts: need 1 alloy and 2 conductor.";
        --e->alloy;e->conductor-=2;e->job=1;e->job_ticks=0;*accepted=true;
        return "Parts reserved. Helper repair queued; work survives a saved journey.";
    }
    if (action==2) {
        if (!e->job) return "No job to cancel.";
        ++e->alloy;e->conductor+=2;e->job=0;e->job_ticks=0;*accepted=true;
        return "Job cancelled. Reserved parts returned exactly once.";
    }
    return "Unknown maintenance action.";
}
void meu_expedition_tick(MeuExpedition *e,MeuPower *p)
{
    if (p->overload && !e->fault) { e->fault=true;e->diagnosed=false; }
    if (e->job && p->isolated && p->energy>=2) {
        if (e->job_ticks%60==0) --p->energy;
        if (++e->job_ticks==600) {
            e->job=0;e->job_ticks=0;e->repaired=true;e->fault=false;e->diagnosed=false;p->overload=false;
        }
    }
}

const char *meu_salvage_command(MeuExpedition *e,MeuPower *p,int action,bool at_site,bool *accepted)
{
    *accepted=false;
    if (action==0) {
        if (!at_site) return "Manipulator out of reach: park at the tower socket.";
        if (e->fault || !e->repaired) return "Salvage arm slow: diagnose and repair its shared distribution buffer.";
        if (e->bulk) return "Cradle already recovered. No duplicate salvage.";
        if (e->alloy>96 || e->conductor>96 || e->sealant>98) return "Cradle capacity exceeded. No stock or energy changed.";
        if (!meu_power_spend(p,150,100)) return "Manipulator needs 150 kJ and 100 heat capacity.";
        e->alloy+=4;e->conductor+=4;e->sealant+=2;e->bulk=true;*accepted=true;
        return "Bulk stock recovered: 4 alloy, 4 conductor, 2 sealant. Phase coupler needs precise suit work.";
    }
    if (action==1) {
        if (!e->coupler) return "No phase coupler in the cradle. Recover it from the sheltered service conduit.";
        if (e->tested) return "Bench record already complete. No energy spent.";
        if (e->fault) return "Repair the distribution buffer before controlled testing.";
        if (!meu_power_spend(p,100,120)) return "Test bench needs 100 kJ and 120 heat capacity.";
        e->tested=true;*accepted=true;return "Test: rejects surge peaks. Adapter costs 3 alloy, 3 conductor, 1 sealant, 300 kJ.";
    }
    if (action==2) {
        if (!e->tested) return "Test the unfamiliar coupler before adapting it.";
        if (e->adapter) return "Adapter already fabricated. No duplicate costs.";
        if (e->alloy<3 || e->conductor<3 || e->sealant<1) return "Need 3 alloy, 3 conductor, 1 sealant; common stock is available via manipulator.";
        if (e->fault || e->job) return "Workshop unavailable until the known repair finishes.";
        if (p->cooling_priority) return "Select fabrication priority at the power console before fabrication.";
        if (!meu_power_spend(p,300,400)) return "Fabrication needs 300 kJ and 400 heat capacity.";
        e->alloy-=3;e->conductor-=3;--e->sealant;e->adapter=true;*accepted=true;
        return "Adapter fabricated. Install in the one grid socket; steady supply trades output for stability.";
    }
    if (action==3) {
        if (!e->adapter || e->installed) return "Need one prepared adapter and an empty installation socket.";
        if (e->fault || p->grid) return "Repair the buffer and disconnect the grid before installation.";
        if (!meu_power_spend(p,50,50)) return "Installation needs 50 kJ and 50 heat capacity.";
        e->installed=true;*accepted=true;return "Grid stabilizer installed: 220 kJ/s, 12 heat/s; high pulses rejected. West service bridge now powered.";
    }
    if (action==4) {
        if (!e->installed) return "Socket is already empty.";
        if (p->grid) return "Disconnect the grid before removing the stabilizer.";
        e->installed=false;*accepted=true;return "Stabilizer removed intact. West bridge access requires reinstalling it.";
    }
    return "Unknown workshop action.";
}

uint32_t meu_expedition_flags(const MeuExpedition *e)
{
    return (e->bulk?1u:0)|(e->coupler?2u:0)|(e->tested?4u:0)|(e->adapter?8u:0)|(e->installed?16u:0)|
        (e->scanned?32u:0)|(e->shelter?64u:0)|(e->bypass?128u:0)|(e->suit?256u:0)|
        (e->catastrophe?512u:0)|(e->checkpoint?1024u:0)|(e->completed?2048u:0)|(e->repaired?4096u:0);
}
void meu_expedition_set_flags(MeuExpedition *e,uint32_t f)
{
    e->bulk=(f&1)!=0;e->coupler=(f&2)!=0;e->tested=(f&4)!=0;e->adapter=(f&8)!=0;e->installed=(f&16)!=0;
    e->scanned=(f&32)!=0;e->shelter=(f&64)!=0;e->bypass=(f&128)!=0;e->suit=(f&256)!=0;
    e->catastrophe=(f&512)!=0;e->checkpoint=(f&1024)!=0;e->completed=(f&2048)!=0;e->repaired=(f&4096)!=0;
}
void meu_expedition_checkpoint(MeuExpedition *e,const MeuPower *p)
{
    e->checkpoint_alloy=e->alloy;e->checkpoint_conductor=e->conductor;e->checkpoint_sealant=e->sealant;
    e->checkpoint_flags=meu_expedition_flags(e)&6399u;e->checkpoint_power=*p;e->checkpoint=true;
}
void meu_expedition_recover(MeuExpedition *e,MeuPower *p)
{
    e->alloy=e->checkpoint_alloy;e->conductor=e->checkpoint_conductor;e->sealant=e->checkpoint_sealant;
    meu_expedition_set_flags(e,e->checkpoint_flags);*p=e->checkpoint_power;
    e->fault=false;e->diagnosed=false;
    e->suit_ticks=0;e->exposure=0;e->job=0;e->job_ticks=0;
}
