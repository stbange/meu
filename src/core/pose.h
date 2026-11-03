#ifndef MEU_POSE_H
#define MEU_POSE_H
typedef struct { double x, z, yaw; } MeuRobotPose;
void meu_pose_to_world(MeuRobotPose pose, double x, double y, double z, double out[3]);
void meu_pose_to_local(MeuRobotPose pose, const double world[3], double out[3]);
void meu_pose_inverse_matrix(MeuRobotPose pose, float out[16]);
#endif
