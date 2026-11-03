#include "core/matrix.h"
#include <math.h>
#include <string.h>
void meu_matrix_perspective(float out[16], float aspect)
{
    const float near_plane = 0.05f, far_plane = 1800.0f;
    const float f = 1.428148f; /* vertical field of view: 70 degrees */
    memset(out, 0, sizeof(float) * 16);
    if (!isfinite(aspect) || aspect <= 0) aspect = 1;
    out[0] = f / aspect; out[5] = f;
    out[10] = (far_plane + near_plane) / (near_plane - far_plane);
    out[11] = -1;
    out[14] = (2 * far_plane * near_plane) / (near_plane - far_plane);
}
void meu_matrix_view(float out[16], float x, float y, float z, float yaw, float pitch)
{
    float cy = cosf(yaw), sy = sinf(yaw), cp = cosf(pitch), sp = sinf(pitch);
    out[0] = cy; out[4] = 0; out[8] = sy;
    out[1] = -sy * sp; out[5] = cp; out[9] = cy * sp;
    out[2] = -sy * cp; out[6] = -sp; out[10] = cy * cp;
    out[3] = out[7] = out[11] = 0;
    out[12] = -(out[0] * x + out[4] * y + out[8] * z);
    out[13] = -(out[1] * x + out[5] * y + out[9] * z);
    out[14] = -(out[2] * x + out[6] * y + out[10] * z);
    out[15] = 1;
}
void meu_matrix_multiply(float out[16], const float a[16], const float b[16])
{
    float result[16];
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float value = 0;
            for (int k = 0; k < 4; ++k) value += a[k * 4 + row] * b[column * 4 + k];
            result[column * 4 + row] = value;
        }
    }
    memcpy(out, result, sizeof(result));
}
