#include "core/simulation.h"
#include "core/matrix.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int checks = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
    fprintf(stderr, "FAIL %s:%d: %s build=%s scene=%s\n", __FILE__, __LINE__, #condition, \
        MEU_BUILD_ID, MEU_SCENE_ID); return EXIT_FAILURE; } } while (0)
#define NEAR(a,b) (fabs((double)(a) - (double)(b)) < 0.0001)
int main(void)
{
    MeuSimulation a, b;
    float view[16], projection[16], product[16];
    meu_sim_init(&a);
    CHECK(a.paused && !a.started && a.ticks == 0);
    CHECK(meu_sim_advance(&a, 1000) == 0);
    CHECK(!meu_sim_action(&a, MEU_FORWARD, true));
    CHECK(!meu_sim_action(&a, -1, true));
    CHECK(!meu_sim_action(&a, MEU_ACTION_COUNT, true));
    meu_sim_start(&a);
    CHECK(meu_sim_action(&a, MEU_FORWARD, true));
    for (int i = 0; i < 60; ++i) meu_sim_advance(&a, 1.0 / 60.0);
    CHECK(a.ticks == 60 && NEAR(a.z, 3.0f));
    meu_sim_init(&b); meu_sim_start(&b); meu_sim_action(&b, MEU_FORWARD, true);
    for (int i = 0; i < 30; ++i) meu_sim_advance(&b, 1.0 / 30.0);
    CHECK(b.ticks == a.ticks && NEAR(b.z, a.z));
    meu_sim_init(&b); meu_sim_start(&b);
    meu_sim_action(&b, MEU_FORWARD, true); meu_sim_action(&b, MEU_RIGHT, true);
    for (int i = 0; i < 60; ++i) meu_sim_advance(&b, MEU_TICK_SECONDS);
    CHECK(NEAR(hypotf(b.x, b.z - 5.5f), MEU_WALK_SPEED));
    meu_sim_pause(&b);
    CHECK(b.held == 0 && b.accumulator == 0);
    CHECK(meu_sim_advance(&b, 600) == 0 && b.ticks == 60);
    CHECK(!meu_sim_action(&b, MEU_FORWARD, true));
    meu_sim_start(&b); meu_sim_advance(&b, MEU_TICK_SECONDS);
    CHECK(b.held == 0 && NEAR(b.x, a.x + MEU_WALK_SPEED / sqrtf(2.0f)));
    CHECK(meu_sim_advance(&b, NAN) == 0);
    CHECK(meu_sim_advance(&b, INFINITY) == 0);
    CHECK(meu_sim_advance(&b, -1) == 0);
    CHECK(meu_sim_advance(&b, 600) == MEU_MAX_TICKS_PER_FRAME);
    CHECK(b.dropped_seconds > 599 && b.accumulator < MEU_TICK_SECONDS);
    meu_sim_init(&a); meu_sim_start(&a);
    meu_sim_look(&a, 1.57079632679f, 0);
    meu_sim_action(&a, MEU_FORWARD, true);
    for (int i = 0; i < 60; ++i) meu_sim_advance(&a, MEU_TICK_SECONDS);
    CHECK(NEAR(a.x, 2.5f) && NEAR(a.z, 5.5f));
    meu_sim_look(&a, NAN, 0); CHECK(isfinite(a.yaw));
    meu_sim_look(&a, 0, 100); CHECK(NEAR(a.pitch, 1.4f));
    meu_sim_action(&a, MEU_BACKWARD, true);
    float before = a.x;
    for (int i = 0; i < 60; ++i) meu_sim_advance(&a, MEU_TICK_SECONDS);
    CHECK(NEAR(a.x, before));
    meu_sim_action(&a, MEU_BACKWARD, false);
    for (int i = 0; i < 600; ++i) meu_sim_advance(&a, MEU_TICK_SECONDS);
    CHECK(NEAR(a.x, 5.8f));
    meu_matrix_view(view, 2, 1.7f, 5, 0, 0);
    CHECK(NEAR(view[0], 1) && NEAR(view[5], 1) && NEAR(view[10], 1));
    CHECK(NEAR(view[12], -2) && NEAR(view[13], -1.7f) && NEAR(view[14], -5));
    meu_matrix_view(view, 2, 1.7f, 5, 0.7f, -0.4f);
    for (int row = 0; row < 3; ++row) {
        CHECK(NEAR(view[row] * 2 + view[4 + row] * 1.7f + view[8 + row] * 5 + view[12 + row], 0));
    }
    meu_matrix_perspective(projection, 0);
    CHECK(isfinite(projection[0]) && projection[11] == -1);
    float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    meu_matrix_multiply(product, projection, identity);
    CHECK(memcmp(product, projection, sizeof(product)) == 0);
    meu_matrix_multiply(projection, projection, identity);
    CHECK(memcmp(product, projection, sizeof(product)) == 0);
    printf("PASS %d core assertions build=%s scene=%s\n", checks, MEU_BUILD_ID, MEU_SCENE_ID);
    return EXIT_SUCCESS;
}
