#include "core/pose.h"
#include <math.h>
#include <string.h>
void meu_pose_to_world(MeuRobotPose p, double x, double y, double z, double out[3])
{
    double c = cos(p.yaw), s = sin(p.yaw);
    out[0] = p.x + c*x - s*z; out[1] = y; out[2] = p.z + s*x + c*z;
}
void meu_pose_to_local(MeuRobotPose p, const double world[3], double out[3])
{
    double c = cos(p.yaw), s = sin(p.yaw), x = world[0]-p.x, z = world[2]-p.z;
    out[0] = c*x+s*z; out[1] = world[1]; out[2] = -s*x+c*z;
}
void meu_pose_inverse_matrix(MeuRobotPose p, float out[16])
{
    double c = cos(p.yaw), s = sin(p.yaw);
    memset(out, 0, 16*sizeof(float));
    out[0] = (float)c; out[8] = (float)s; out[5] = 1;
    out[2] = (float)-s; out[10] = (float)c; out[15] = 1;
    out[12] = (float)(-c*p.x-s*p.z); out[14] = (float)(s*p.x-c*p.z);
}
