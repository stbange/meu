#ifndef MEU_RENDERER_H
#define MEU_RENDERER_H
#include "core/simulation.h"
#include <stdbool.h>
bool meu_renderer_init(void);
bool meu_renderer_draw(const MeuSimulation *state, int width, int height);
int meu_renderer_variation(int width, int height);
bool meu_renderer_capture(const char *path, int width, int height);
void meu_renderer_destroy(void);
#endif
