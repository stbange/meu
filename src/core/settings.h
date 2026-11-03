#ifndef MEU_SETTINGS_H
#define MEU_SETTINGS_H
#include <stdbool.h>
typedef enum {
    MEU_SETTING_SENSITIVITY, MEU_SETTING_INVERT_Y, MEU_SETTING_WALK_BOB,
    MEU_SETTING_MACHINE_SWAY, MEU_SETTING_VOLUME, MEU_SETTING_COUNT
} MeuSetting;
typedef struct { float sensitivity,volume; bool invert_y,walk_bob,machine_sway; } MeuSettings;
void meu_settings_default(MeuSettings *settings);
bool meu_settings_set(MeuSettings *settings,int id,float value);
float meu_settings_value(const MeuSettings *settings,int id);
const char *meu_settings_name(int id);
#endif
