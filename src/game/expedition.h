#ifndef MEU_EXPEDITION_H
#define MEU_EXPEDITION_H
#include <stdbool.h>
#include <stdint.h>
#include "game/power.h"
typedef struct {
    uint32_t alloy,conductor,sealant;
    bool fault,diagnosed;
    uint32_t job,job_ticks;
    bool bulk,coupler,tested,adapter,installed;
    bool scanned,shelter,bypass,suit,catastrophe,checkpoint;
    uint32_t suit_ticks,exposure;
    uint32_t checkpoint_alloy,checkpoint_conductor,checkpoint_sealant,checkpoint_flags;
    MeuPower checkpoint_power;
} MeuExpedition;
void meu_expedition_init(MeuExpedition *e);
bool meu_expedition_valid(const MeuExpedition *e);
const char *meu_maintenance_command(MeuExpedition *e,MeuPower *p,int action,bool *accepted);
const char *meu_salvage_command(MeuExpedition *e,MeuPower *p,int action,bool at_site,bool *accepted);
void meu_expedition_tick(MeuExpedition *e,MeuPower *p);
uint32_t meu_expedition_flags(const MeuExpedition *e);
void meu_expedition_set_flags(MeuExpedition *e,uint32_t flags);
void meu_expedition_checkpoint(MeuExpedition *e,const MeuPower *p);
void meu_expedition_recover(MeuExpedition *e,MeuPower *p);
const char *meu_helper_status(const MeuExpedition *e,const MeuPower *p);
#endif
