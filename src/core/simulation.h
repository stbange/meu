#ifndef MEU_SIMULATION_H
#define MEU_SIMULATION_H
#include "core/pose.h"
#include "game/interior.h"
#include <stdbool.h>
#include <stdint.h>
#define MEU_TICK_SECONDS (1.0 / 60.0)
#define MEU_MAX_TICKS_PER_FRAME 8
#define MEU_WALK_SPEED 2.5f
typedef enum {
    MEU_FORWARD, MEU_BACKWARD, MEU_LEFT, MEU_RIGHT,
    MEU_LOOK_LEFT, MEU_LOOK_RIGHT, MEU_LOOK_UP, MEU_LOOK_DOWN,
    MEU_INTERACT, MEU_TO_BRIDGE, MEU_TO_QUARTERS, MEU_TO_ENGINEERING, MEU_ACTION_COUNT
} MeuAction;
typedef enum { MEU_USE_NONE, MEU_USE_PLACARD, MEU_USE_LAMP, MEU_USE_INSPECTION, MEU_USE_CALL } MeuUse;
typedef struct {
    /* Authoritative player coordinates always remain in the robot's local frame. */
    float x,y,z,yaw,pitch,previous_x,previous_y,previous_z;
    float lift_y,previous_lift_y,lift_velocity,lift_door;
    int lift_target,lift_current;
    uint32_t held,visited;
    uint64_t ticks;
    double accumulator,dropped_seconds;
    bool started,paused,grounded,quarters_light,inspection_light,info_open,motion_fixture;
    MeuRobotPose robot,previous_robot;
} MeuSimulation;
void meu_sim_init(MeuSimulation *state);
void meu_sim_start(MeuSimulation *state);
void meu_sim_pause(MeuSimulation *state);
bool meu_sim_action(MeuSimulation *state,int action,bool pressed);
void meu_sim_look(MeuSimulation *state,float yaw_delta,float pitch_delta);
unsigned meu_sim_advance(MeuSimulation *state,double elapsed_seconds);
bool meu_sim_on_lift(const MeuSimulation *state);
bool meu_sim_lift_moving(const MeuSimulation *state);
bool meu_sim_lift_ready(const MeuSimulation *state);
bool meu_sim_request_lift(MeuSimulation *state,int deck);
MeuUse meu_sim_use_target(const MeuSimulation *state);
const char *meu_sim_use_name(MeuUse use);
int meu_sim_room(const MeuSimulation *state);
void meu_sim_move(MeuSimulation *state,float dx,float dz);
void meu_sim_render_state(const MeuSimulation *state,MeuSimulation *render);
#endif
