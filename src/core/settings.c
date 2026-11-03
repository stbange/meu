#include "core/settings.h"
#include <math.h>
void meu_settings_default(MeuSettings *s)
{
    *s=(MeuSettings){1,.6f,false,false,false};
}
bool meu_settings_set(MeuSettings *s,int id,float value)
{
    if (!isfinite(value) || id<0 || id>=MEU_SETTING_COUNT) return false;
    if (id==MEU_SETTING_SENSITIVITY) s->sensitivity=fmaxf(.25f,fminf(3,value));
    else if (id==MEU_SETTING_VOLUME) s->volume=fmaxf(0,fminf(1,value));
    else {
        if (value!=0 && value!=1) return false;
        if (id==MEU_SETTING_INVERT_Y) s->invert_y=value!=0;
        if (id==MEU_SETTING_WALK_BOB) s->walk_bob=value!=0;
        if (id==MEU_SETTING_MACHINE_SWAY) s->machine_sway=value!=0;
    }
    return true;
}
float meu_settings_value(const MeuSettings *s,int id)
{
    switch (id) {
        case MEU_SETTING_SENSITIVITY:return s->sensitivity;
        case MEU_SETTING_VOLUME:return s->volume;
        case MEU_SETTING_INVERT_Y:return s->invert_y;
        case MEU_SETTING_WALK_BOB:return s->walk_bob;
        case MEU_SETTING_MACHINE_SWAY:return s->machine_sway;
        default:return 0;
    }
}
const char *meu_settings_name(int id)
{
    static const char *names[]={"Look sensitivity","Invert vertical drag","Walking bob","Machine sway","Sound volume"};
    return names[id>=0 && id<MEU_SETTING_COUNT?id:0];
}
