#include "core/simulation.h"
#include <math.h>
#include <string.h>
static float clampf(float value, float low, float high)
{
    return value < low ? low : (value > high ? high : value);
}
void meu_sim_init(MeuSimulation *state)
{
    memset(state, 0, sizeof(*state));
    state->y = 1.7f;
    state->z = 5.5f;
    state->paused = true;
}
void meu_sim_start(MeuSimulation *state)
{
    state->started = true;
    state->paused = false;
    state->held = 0;
    state->accumulator = 0;
}
void meu_sim_pause(MeuSimulation *state)
{
    state->paused = true;
    state->held = 0;
    state->accumulator = 0;
}
bool meu_sim_action(MeuSimulation *state, int action, bool pressed)
{
    if (action < 0 || action >= MEU_ACTION_COUNT) return false;
    if (pressed && (!state->started || state->paused)) return false;
    if (pressed) state->held |= UINT32_C(1) << action;
    else state->held &= ~(UINT32_C(1) << action);
    return true;
}
void meu_sim_look(MeuSimulation *state, float yaw_delta, float pitch_delta)
{
    if (state->paused || !isfinite(yaw_delta) || !isfinite(pitch_delta)) return;
    state->yaw = remainderf(state->yaw + clampf(yaw_delta, -6.2831853f, 6.2831853f), 6.2831853f);
    state->pitch = clampf(state->pitch + pitch_delta, -1.4f, 1.4f);
}
static int held(const MeuSimulation *state, MeuAction action)
{
    return (state->held & (UINT32_C(1) << action)) != 0;
}
static void tick(MeuSimulation *state)
{
    const float dt = (float)MEU_TICK_SECONDS;
    float forward = (float)(held(state, MEU_FORWARD) - held(state, MEU_BACKWARD));
    float right = (float)(held(state, MEU_RIGHT) - held(state, MEU_LEFT));
    float length = sqrtf(forward * forward + right * right);
    if (length > 1.0f) { forward /= length; right /= length; }
    meu_sim_look(state, (float)(held(state, MEU_LOOK_RIGHT) - held(state, MEU_LOOK_LEFT)) * 1.5f * dt,
        (float)(held(state, MEU_LOOK_UP) - held(state, MEU_LOOK_DOWN)) * 1.5f * dt);
    state->x += (sinf(state->yaw) * forward + cosf(state->yaw) * right) * MEU_WALK_SPEED * dt;
    state->z += (-cosf(state->yaw) * forward + sinf(state->yaw) * right) * MEU_WALK_SPEED * dt;
    /* Only room bounds in M0a. Interior obstacle/lift collision belongs to M0b. */
    state->x = clampf(state->x, -5.8f, 5.8f);
    state->z = clampf(state->z, -6.4f, 7.8f);
    ++state->ticks;
}
unsigned meu_sim_advance(MeuSimulation *state, double elapsed_seconds)
{
    unsigned count = 0;
    if (state->paused || !isfinite(elapsed_seconds) || elapsed_seconds <= 0) return 0;
    if (elapsed_seconds > 0.25) {
        state->dropped_seconds += elapsed_seconds - 0.25;
        elapsed_seconds = 0.25;
    }
    state->accumulator += elapsed_seconds;
    while (state->accumulator + 1e-12 >= MEU_TICK_SECONDS && count < MEU_MAX_TICKS_PER_FRAME) {
        tick(state);
        state->accumulator -= MEU_TICK_SECONDS;
        ++count;
    }
    if (state->accumulator >= MEU_TICK_SECONDS) {
        double remainder = fmod(state->accumulator, MEU_TICK_SECONDS);
        state->dropped_seconds += state->accumulator - remainder;
        state->accumulator = remainder;
    }
    if (state->accumulator < 0) state->accumulator = 0;
    return count;
}
