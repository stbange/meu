#include "game/expedition.h"
#include <string.h>
void meu_expedition_init(MeuExpedition *e)
{
    memset(e,0,sizeof(*e));e->alloy=8;e->conductor=8;e->sealant=4;
}
bool meu_expedition_valid(const MeuExpedition *e)
{
    return e->alloy<=100 && e->conductor<=100 && e->sealant<=100 && e->job<=1 && e->job_ticks<=600 &&
        (e->fault || !e->diagnosed) && (e->job || !e->job_ticks) &&
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
            e->job=0;e->job_ticks=0;e->fault=false;e->diagnosed=false;p->overload=false;
        }
    }
}
