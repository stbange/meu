#ifndef MEU_POWER_H
#define MEU_POWER_H
#include <stdbool.h>
#include <stdint.h>
#define MEU_ENERGY_CAPACITY 30000u
#define MEU_RETURN_MARGIN 4000u
#define MEU_LIFE_RESERVE 2000u
typedef struct {
    uint32_t energy,heat,clock,phase;
    bool collectors,grid,isolated,cooling_priority,overload;
} MeuPower;
void meu_power_init(MeuPower *p);
bool meu_power_valid(const MeuPower *p);
unsigned meu_power_sun(const MeuPower *p);
void meu_power_tick(MeuPower *p,bool moving,bool stabilized);
bool meu_power_spend(MeuPower *p,unsigned energy,unsigned heat);
#endif
