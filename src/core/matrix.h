#ifndef MEU_MATRIX_H
#define MEU_MATRIX_H
void meu_matrix_perspective(float out[16], float aspect);
void meu_matrix_view(float out[16], float x, float y, float z, float yaw, float pitch);
void meu_matrix_multiply(float out[16], const float a[16], const float b[16]);
#endif
