#ifndef MEU_TRAVEL_WORLD_H
#define MEU_TRAVEL_WORLD_H
#include "core/pose.h"
#include <stdbool.h>
#include <stddef.h>
typedef struct {
    double x,y,z,w,h,d;
    float r,g,b;
    bool support;
} MeuWorldBox;
typedef struct { const MeuWorldBox *boxes; size_t count; } MeuWorld;
typedef enum { MEU_ROUTE_CLEAR, MEU_ROUTE_STRUCTURE, MEU_ROUTE_SUPPORT, MEU_ROUTE_INVALID } MeuRoute;
const MeuWorld *meu_world_default(void);
/* Entire feet and all hull bands, with extra clearance in metres. */
MeuRoute meu_world_validate(const MeuWorld *world,MeuRobotPose pose,double clearance);
MeuRoute meu_world_sweep(const MeuWorld *world,MeuRobotPose from,MeuRobotPose to);
const char *meu_route_name(MeuRoute reason);
#define MEU_TRAVEL_CLEARANCE 2.0
#define MEU_HULL_RADIUS 80.0
#endif
