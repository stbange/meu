#include "core/simulation.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
static int action_id(const char *name)
{
    const char *names[] = {"forward","backward","left","right","look-left","look-right","look-up","look-down"};
    for (int i = 0; i < MEU_ACTION_COUNT; ++i) if (!strcmp(name, names[i])) return i;
    return -1;
}
int main(int argc, char **argv)
{
    char line[160], command[32], key[32];
    unsigned line_number = 0;
    MeuSimulation state;
    if (argc != 2) { fprintf(stderr, "Usage: meu_scenarios <fixture>\n"); return 2; }
    FILE *fixture = fopen(argv[1], "r");
    if (!fixture) { perror(argv[1]); return 2; }
    meu_sim_init(&state);
    printf("REPLAY build=%s content=%d scene=%s fixture=%s\n",
        MEU_BUILD_ID, MEU_CONTENT_VERSION, MEU_SCENE_ID, argv[1]);
    while (fgets(line, sizeof(line), fixture)) {
        ++line_number;
        if (line[0] == '#' || sscanf(line, "%31s", command) != 1) continue;
        printf("line=%u tick=%" PRIu64 " command=%s", line_number, state.ticks, line);
        if (!strcmp(command, "start")) meu_sim_start(&state);
        else if (!strcmp(command, "pause")) meu_sim_pause(&state);
        else if (!strcmp(command, "press") || !strcmp(command, "release")) {
            if (sscanf(line, "%*s %31s", key) != 1 || action_id(key) < 0) goto fail;
            meu_sim_action(&state, action_id(key), !strcmp(command, "press"));
        } else if (!strcmp(command, "frames")) {
            unsigned count;
            if (sscanf(line, "%*s %u", &count) != 1 || count > 36000) goto fail;
            for (unsigned i = 0; i < count; ++i) meu_sim_advance(&state, MEU_TICK_SECONDS);
        } else if (!strcmp(command, "expect")) {
            double expected, tolerance, actual;
            if (sscanf(line, "%*s %31s %lf %lf", key, &expected, &tolerance) != 3 || tolerance < 0) goto fail;
            if (!strcmp(key, "x")) actual = state.x;
            else if (!strcmp(key, "z")) actual = state.z;
            else if (!strcmp(key, "ticks")) actual = (double)state.ticks;
            else if (!strcmp(key, "held")) actual = state.held;
            else goto fail;
            if (!isfinite(expected) || !isfinite(tolerance) || fabs(actual - expected) > tolerance) goto fail;
        } else goto fail;
    }
    if (ferror(fixture)) goto fail;
    fclose(fixture);
    printf("PASS scene=%s ticks=%" PRIu64 "\n", MEU_SCENE_ID, state.ticks);
    return EXIT_SUCCESS;
fail:
    fprintf(stderr, "FAIL build=%s scene=%s fixture=%s line=%u tick=%" PRIu64
        " x=%.6f z=%.6f held=%u paused=%d\n", MEU_BUILD_ID, MEU_SCENE_ID,
        argv[1], line_number, state.ticks, state.x, state.z, state.held, (int)state.paused);
    fclose(fixture);
    return EXIT_FAILURE;
}
