#include "game/power.h"
#include <string.h>
void meu_power_init(MeuPower *p)
{
    memset(p,0,sizeof(*p)); p->energy=18000; p->clock=120; p->cooling_priority=true;
}
bool meu_power_valid(const MeuPower *p)
{
    return p->energy<=MEU_ENERGY_CAPACITY && p->heat<=10000 && p->clock<600 && p->phase<60 && (!p->grid || p->isolated);
}
unsigned meu_power_sun(const MeuPower *p)
{
    /* Ten active minutes; integer daylight identity is the same on every target. */
    unsigned t=p->clock; return t<300?t*1000/300:(600-t)*1000/300;
}
bool meu_power_spend(MeuPower *p,unsigned energy,unsigned heat)
{
    if (energy>p->energy || heat>10000-p->heat) return false;
    p->energy-=energy; p->heat+=heat; return true;
}
void meu_power_tick(MeuPower *p,bool moving,bool stabilized)
{
    if (++p->phase<60) return;
    p->phase=0; p->clock=(p->clock+1)%600;
    if (moving) { p->collectors=false; p->grid=false; }
    unsigned supply=meu_power_sun(p)*(p->collectors?150u:18u)/1000u;
    if (p->grid) {
        /* Predictable 12-second pulses. Isolation protects the essential bus. */
        if (!stabilized && p->clock%12<3) { p->overload=true; p->grid=false; }
        else { supply+=stabilized?220:100; p->heat+=stabilized?12:25; }
    }
    unsigned cost=1+(moving?24u:0u);
    p->energy=p->energy+supply>MEU_ENERGY_CAPACITY?MEU_ENERGY_CAPACITY:p->energy+supply;
    p->energy=p->energy>cost?p->energy-cost:0;
    unsigned cooling=p->cooling_priority?35:10;
    p->heat=p->heat>cooling?p->heat-cooling:0;
    if (p->heat>=9000) { p->grid=false; p->overload=true; }
    if (p->heat>10000) p->heat=10000;
}
