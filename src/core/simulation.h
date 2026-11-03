#ifndef MEU_SIMULATION_H
#define MEU_SIMULATION_H
#include <stdbool.h>
#include <stdint.h>
#define MEU_TICK_SECONDS (1.0 / 60.0)
#define MEU_MAX_TICKS_PER_FRAME 8
#define MEU_WALK_SPEED 2.5f
typedef enum {
    MEU_FORWARD, MEU_BACKWARD, MEU_LEFT, MEU_RIGHT,
    MEU_LOOK_LEFT, MEU_LOOK_RIGHT, MEU_LOOK_UP, MEU_LOOK_DOWN,
    MEU_ACTION_COUNT
} MeuAction;
typedef struct {
    float x, y, z;
    float yaw, pitch;
    uint32_t held;
    uint64_t ticks;
    double accumulator;
    double dropped_seconds;
    bool started, paused;
} MeuSimulation;
void meu_sim_init(MeuSimulation *state);
void meu_sim_start(MeuSimulation *state);
void meu_sim_pause(MeuSimulation *state);
bool meu_sim_action(MeuSimulation *state, int action, bool pressed);
void meu_sim_look(MeuSimulation *state, float yaw_delta, float pitch_delta);
unsigned meu_sim_advance(MeuSimulation *state, double elapsed_seconds);
#endif
