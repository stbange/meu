#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "core/simulation.h"
#include "render/renderer.h"
#include "meu_build_info.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/console.h>
#define WEB_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define WEB_EXPORT
#endif

typedef struct {
    SDL_Window *window;
    SDL_GLContext context;
    SDL_AudioStream *audio;
    MeuSimulation simulation;
    double previous_time;
    unsigned frames, audio_attempts;
    int variation, exit_code;
    bool running, smoke, dragging, audio_ready;
    const char *capture;
} Application;
static Application app;

#ifdef __EMSCRIPTEN__
static void browser_log(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    (void)userdata; (void)category;
    if (priority >= SDL_LOG_PRIORITY_ERROR) emscripten_console_error(message);
    else if (priority >= SDL_LOG_PRIORITY_WARN) emscripten_console_warn(message);
    else emscripten_console_log(message);
}
#endif

static void pause_game(void)
{
    meu_sim_pause(&app.simulation);
    app.dragging = false;
    if (app.audio) SDL_PauseAudioStreamDevice(app.audio);
    SDL_SetWindowTitle(app.window, "MEU - Paused | Enter: resume | WASD: move | drag / arrows: look | Esc: pause");
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.paused();");
#endif
}

static void start_game(void)
{
    meu_sim_start(&app.simulation);
    app.previous_time = (double)SDL_GetTicksNS() / 1e9;
    if (app.audio_attempts == 0) {
        ++app.audio_attempts;
        SDL_AudioSpec spec = {.format = SDL_AUDIO_F32, .channels = 1, .freq = 48000};
        if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            app.audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        }
        if (app.audio) {
            float samples[4800];
            for (int i = 0; i < 4800; ++i) {
                float envelope = sinf(3.14159265f * (float)i / 4800.0f);
                samples[i] = 0.04f * envelope * sinf(6.2831853f * 440.0f * (float)i / 48000.0f);
            }
            SDL_PutAudioStreamData(app.audio, samples, sizeof(samples));
        } else {
            SDL_Log("Audio unavailable; exploration remains available: %s", SDL_GetError());
        }
    }
    if (app.audio) app.audio_ready = SDL_ResumeAudioStreamDevice(app.audio);
    SDL_SetWindowTitle(app.window, "MEU - M0a test room | WASD: move | drag / arrows: look | Esc: pause");
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.started();");
#endif
}

WEB_EXPORT void meu_web_action(int action, int pressed)
{
    meu_sim_action(&app.simulation, action, pressed != 0);
}
WEB_EXPORT void meu_web_look(float x, float y)
{
    meu_sim_look(&app.simulation, x, y);
}
WEB_EXPORT void meu_web_start(void) { start_game(); }
WEB_EXPORT void meu_web_pause(void) { pause_game(); }

#if defined(__EMSCRIPTEN__) && defined(MEU_DIAGNOSTICS)
WEB_EXPORT const char *meu_state_json(void)
{
    static char result[768];
    snprintf(result, sizeof(result),
        "{\"build\":\"%s\",\"scene\":\"%s\",\"content\":%d,\"ticks\":%" PRIu64
        ",\"x\":%.6f,\"y\":%.6f,\"z\":%.6f,\"yaw\":%.6f,\"pitch\":%.6f,"
        "\"held\":%u,\"paused\":%s,\"started\":%s,\"frames\":%u,\"variation\":%d,"
        "\"audioAttempts\":%u,\"audioReady\":%s}",
        MEU_BUILD_ID, MEU_SCENE_ID, MEU_CONTENT_VERSION, app.simulation.ticks,
        app.simulation.x, app.simulation.y, app.simulation.z, app.simulation.yaw, app.simulation.pitch,
        app.simulation.held, app.simulation.paused ? "true" : "false",
        app.simulation.started ? "true" : "false", app.frames, app.variation, app.audio_attempts,
        app.audio_ready ? "true" : "false");
    return result;
}
#endif

#ifndef __EMSCRIPTEN__
static int key_action(SDL_Scancode key)
{
    switch (key) {
        case SDL_SCANCODE_W: return MEU_FORWARD;
        case SDL_SCANCODE_S: return MEU_BACKWARD;
        case SDL_SCANCODE_A: return MEU_LEFT;
        case SDL_SCANCODE_D: return MEU_RIGHT;
        case SDL_SCANCODE_LEFT: return MEU_LOOK_LEFT;
        case SDL_SCANCODE_RIGHT: return MEU_LOOK_RIGHT;
        case SDL_SCANCODE_UP: return MEU_LOOK_UP;
        case SDL_SCANCODE_DOWN: return MEU_LOOK_DOWN;
        default: return -1;
    }
}
#endif

static void event(const SDL_Event *input)
{
    if (input->type == SDL_EVENT_QUIT) app.running = false;
    if (input->type == SDL_EVENT_WINDOW_FOCUS_LOST || input->type == SDL_EVENT_WINDOW_MINIMIZED)
        pause_game();
#ifndef __EMSCRIPTEN__
    if (input->type == SDL_EVENT_KEY_DOWN || input->type == SDL_EVENT_KEY_UP) {
        bool down = input->type == SDL_EVENT_KEY_DOWN;
        if (down && !input->key.repeat && input->key.scancode == SDL_SCANCODE_RETURN) start_game();
        else if (down && !input->key.repeat && input->key.scancode == SDL_SCANCODE_ESCAPE) pause_game();
        else meu_sim_action(&app.simulation, key_action(input->key.scancode), down);
    }
    if (input->type == SDL_EVENT_MOUSE_BUTTON_DOWN && input->button.button == SDL_BUTTON_LEFT) {
        if (app.simulation.paused) start_game();
        else app.dragging = true;
    }
    if (input->type == SDL_EVENT_MOUSE_BUTTON_UP) app.dragging = false;
    if (input->type == SDL_EVENT_MOUSE_MOTION && app.dragging)
        meu_sim_look(&app.simulation, input->motion.xrel * 0.004f, -input->motion.yrel * 0.004f);
    if (input->type == SDL_EVENT_FINGER_DOWN && input->tfinger.x < 0.5f)
        meu_sim_action(&app.simulation, MEU_FORWARD, true);
    if (input->type == SDL_EVENT_FINGER_UP)
        meu_sim_action(&app.simulation, MEU_FORWARD, false);
    if (input->type == SDL_EVENT_FINGER_MOTION && input->tfinger.x >= 0.5f)
        meu_sim_look(&app.simulation, input->tfinger.dx * 3, -input->tfinger.dy * 3);
#endif
}

static void cleanup(void)
{
    SDL_DestroyAudioStream(app.audio);
    if (app.context) meu_renderer_destroy();
    SDL_GL_DestroyContext(app.context);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}

static void frame(void)
{
    SDL_Event input;
    while (SDL_PollEvent(&input)) event(&input);
    double now = (double)SDL_GetTicksNS() / 1e9;
    meu_sim_advance(&app.simulation, now - app.previous_time);
    app.previous_time = now;
    int width = 0, height = 0;
#ifdef __EMSCRIPTEN__
    emscripten_get_canvas_element_size("#canvas", &width, &height);
#else
    SDL_GetWindowSizeInPixels(app.window, &width, &height);
#endif
    if (!meu_renderer_draw(&app.simulation, width, height)) {
        SDL_Log("OpenGL frame failed"); app.exit_code = EXIT_FAILURE; app.running = false;
    }
    ++app.frames;
    if (app.frames == 3 && width > 0 && height > 0) {
        app.variation = meu_renderer_variation(width, height);
        if (app.variation < 8) {
            SDL_Log("Rendered scene is blank"); app.exit_code = EXIT_FAILURE; app.running = false;
        }
#ifdef __EMSCRIPTEN__
        emscripten_run_script("document.body.dataset.rendered = 'true';");
#endif
        if (app.capture && !meu_renderer_capture(app.capture, width, height)) {
            SDL_Log("Capture failed: %s", SDL_GetError()); app.exit_code = EXIT_FAILURE;
        }
        if (app.smoke) {
            printf("SMOKE build=%s scene=%s frames=%u variation=%d result=%s\n",
                MEU_BUILD_ID, MEU_SCENE_ID, app.frames, app.variation, app.exit_code ? "FAIL" : "PASS");
            app.running = false;
        }
    }
    SDL_GL_SwapWindow(app.window);
#ifdef __EMSCRIPTEN__
    if (!app.running) {
        emscripten_cancel_main_loop(); cleanup(); exit(app.exit_code);
    }
#endif
}

int main(int argc, char **argv)
{
    memset(&app, 0, sizeof(app));
#ifdef __EMSCRIPTEN__
    SDL_SetLogOutputFunction(browser_log, NULL);
#endif
    app.running = true;
    meu_sim_init(&app.simulation);
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--smoke-test")) app.smoke = true;
        else if (!strcmp(argv[i], "--capture") && i + 1 < argc) app.capture = argv[++i];
        else { fprintf(stderr, "Unknown or incomplete argument: %s\n", argv[i]); return EXIT_FAILURE; }
    }
    if (!SDL_Init(SDL_INIT_VIDEO)) { SDL_Log("SDL: %s", SDL_GetError()); return EXIT_FAILURE; }
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    app.window = SDL_CreateWindow("MEU - Enter or click to begin", 1100, 700,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (app.window) app.context = SDL_GL_CreateContext(app.window);
    if (!app.context || !meu_renderer_init()) {
        SDL_Log("Client initialization failed: %s", SDL_GetError()); cleanup(); return EXIT_FAILURE;
    }
    SDL_GL_SetSwapInterval(1);
    SDL_Log("MEU build=%s scene=%s content=%d", MEU_BUILD_ID, MEU_SCENE_ID, MEU_CONTENT_VERSION);
    app.previous_time = (double)SDL_GetTicksNS() / 1e9;
    if (app.smoke) meu_sim_start(&app.simulation);
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.ready();");
    emscripten_set_main_loop(frame, 0, 1);
#else
    while (app.running) { frame(); SDL_Delay(1); }
    cleanup();
#endif
    return app.exit_code;
}
