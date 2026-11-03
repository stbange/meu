#ifndef MEU_TRAVEL_H
#define MEU_TRAVEL_H
#include "world/travel_world.h"
#define MEU_MAX_SPEED 12.0
#define MEU_ACCELERATION 3.0
#define MEU_BRAKE_DECELERATION 4.0
#define MEU_MAX_TURN .10
#define MEU_TURN_ACCELERATION .25
typedef struct {
    double speed,turn_rate;
    bool helm,park_requested,parked,latched;
    MeuRoute reason;
} MeuTravel;
void meu_travel_init(MeuTravel *travel);
void meu_travel_stop(MeuTravel *travel);
double meu_travel_stop_distance(double speed);
bool meu_travel_envelope(const MeuWorld *world,MeuRobotPose pose,double speed,double turn,MeuRoute *reason);
void meu_travel_tick(MeuTravel *travel,MeuRobotPose *pose,const MeuWorld *world,int drive,int turn,bool brake);
#endif
