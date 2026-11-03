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
    uint32_t input_epoch_seen;
    int settings_selection;
    SDL_FingerID move_finger,look_finger;
    bool move_touch,look_touch;
    int variation, exit_code;
    bool running, smoke, tour, pilot, lifecycle, dragging, audio_ready;
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

static void sync_input_context(void)
{
    MeuSimulation *s=&app.simulation;
    if (app.input_epoch_seen==s->input_epoch) return;
    app.input_epoch_seen=s->input_epoch; app.dragging=false;
    app.move_touch=app.look_touch=false;
#ifdef __EMSCRIPTEN__
    char script[160];
    snprintf(script,sizeof(script),"if(window.MEU)window.MEU.context(%d,%u);",meu_sim_context(s),s->input_epoch);
    emscripten_run_script(script);
#endif
}
static void sync_preferences(void)
{
    const MeuSettings *s=&app.simulation.settings;
    if (app.audio) SDL_SetAudioStreamGain(app.audio,s->volume);
#ifdef __EMSCRIPTEN__
    char script[240];
    snprintf(script,sizeof(script),"if(window.MEU)window.MEU.preferences(%d,%.2f,%d,%d,%d,%.2f);",
        app.simulation.settings_open,s->sensitivity,s->invert_y,s->walk_bob,s->machine_sway,s->volume);
    emscripten_run_script(script);
#endif
}

static void pause_game(void)
{
    meu_sim_pause(&app.simulation);
    app.dragging = false;
    if (app.audio) SDL_PauseAudioStreamDevice(app.audio);
    SDL_SetWindowTitle(app.window, "MEU - Paused | Enter: resume | F1: settings");
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.paused();");
#endif
    sync_input_context();
}

static void start_game(void)
{
    if (app.simulation.settings_open) return;
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
    sync_preferences();
    if (app.audio) app.audio_ready = SDL_ResumeAudioStreamDevice(app.audio);
    SDL_SetWindowTitle(app.window, "MEU - Exploration | F1: settings | WASD: move | drag / arrows: look | Esc: pause");
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.started();");
#endif
    sync_input_context();
}

WEB_EXPORT int meu_web_action(int action, int pressed)
{
    bool accepted=meu_sim_action(&app.simulation,action,pressed!=0);
    sync_input_context(); return accepted;
}
WEB_EXPORT void meu_web_look(float x, float y)
{
    meu_sim_pointer_look(&app.simulation, x, y);
}
WEB_EXPORT void meu_web_start(void) { start_game(); }
WEB_EXPORT void meu_web_pause(void) { pause_game(); }

WEB_EXPORT void meu_web_settings(int operation)
{
    if (operation==0) { pause_game(); meu_sim_open_settings(&app.simulation); app.settings_selection=0; }
    else if (operation==1) meu_sim_close_settings(&app.simulation);
    else if (operation==2 && app.simulation.settings_open) meu_settings_default(&app.simulation.settings);
    sync_input_context(); sync_preferences();
}
WEB_EXPORT int meu_web_setting(int id,float value)
{
    bool accepted=meu_sim_setting(&app.simulation,id,value);
    sync_preferences(); return accepted;
}

#if defined(__EMSCRIPTEN__) && defined(MEU_DIAGNOSTICS)
WEB_EXPORT const char *meu_state_json(void)
{
    static char result[2200];
    snprintf(result, sizeof(result),
        "{\"build\":\"%s\",\"scene\":\"%s\",\"content\":%d,\"ticks\":%" PRIu64
        ",\"x\":%.6f,\"y\":%.6f,\"z\":%.6f,\"yaw\":%.6f,\"pitch\":%.6f,"
        "\"held\":%u,\"paused\":%s,\"started\":%s,\"frames\":%u,\"variation\":%d,"
        "\"audioAttempts\":%u,\"audioReady\":%s,\"room\":%d,\"liftY\":%.5f,"
        "\"onLift\":%s,\"liftMoving\":%s,\"liftDoor\":%.3f,\"visited\":%u,\"grounded\":%s,\"use\":%d,"
        "\"robotX\":%.6f,\"robotZ\":%.6f,\"robotYaw\":%.6f,\"speed\":%.6f,\"turnRate\":%.6f,"
        "\"helm\":%s,\"parked\":%s,\"parking\":%s,\"travelReason\":%d,\"travelSafe\":%s,"
        "\"context\":%d,\"inputEpoch\":%u,\"settingsOpen\":%s,\"sensitivity\":%.3f,\"invertY\":%s,"
        "\"walkBob\":%s,\"machineSway\":%s,\"volume\":%.3f,\"audioGain\":%.3f}",
        MEU_BUILD_ID, MEU_SCENE_ID, MEU_CONTENT_VERSION, app.simulation.ticks,
        app.simulation.x, app.simulation.y, app.simulation.z, app.simulation.yaw, app.simulation.pitch,
        app.simulation.held, app.simulation.paused ? "true" : "false",
        app.simulation.started ? "true" : "false", app.frames, app.variation, app.audio_attempts,
        app.audio_ready ? "true" : "false", meu_sim_room(&app.simulation),app.simulation.lift_y,
        meu_sim_on_lift(&app.simulation)?"true":"false",meu_sim_lift_moving(&app.simulation)?"true":"false",
        app.simulation.lift_door,app.simulation.visited,app.simulation.grounded?"true":"false",meu_sim_use_target(&app.simulation),
        app.simulation.robot.x,app.simulation.robot.z,app.simulation.robot.yaw,
        app.simulation.travel.speed,app.simulation.travel.turn_rate,app.simulation.travel.helm?"true":"false",
        app.simulation.travel.parked?"true":"false",app.simulation.travel.park_requested?"true":"false",app.simulation.travel.reason,
        meu_world_validate(app.simulation.world,app.simulation.robot,0)==MEU_ROUTE_CLEAR?"true":"false",
        meu_sim_context(&app.simulation),app.simulation.input_epoch,app.simulation.settings_open?"true":"false",
        app.simulation.settings.sensitivity,app.simulation.settings.invert_y?"true":"false",
        app.simulation.settings.walk_bob?"true":"false",app.simulation.settings.machine_sway?"true":"false",app.simulation.settings.volume,app.audio?SDL_GetAudioStreamGain(app.audio):-1);
    return result;
}
#endif

#ifndef __EMSCRIPTEN__
static void settings_key(SDL_Scancode key)
{
    int *row=&app.settings_selection;
    if (key==SDL_SCANCODE_UP) *row=(*row+MEU_SETTING_COUNT-1)%MEU_SETTING_COUNT;
    if (key==SDL_SCANCODE_DOWN) *row=(*row+1)%MEU_SETTING_COUNT;
    if (key==SDL_SCANCODE_R) meu_web_settings(2);
    if (key==SDL_SCANCODE_LEFT || key==SDL_SCANCODE_RIGHT || key==SDL_SCANCODE_SPACE) {
        float old=meu_settings_value(&app.simulation.settings,*row),value;
        if (*row==MEU_SETTING_SENSITIVITY || *row==MEU_SETTING_VOLUME)
            value=old+(key==SDL_SCANCODE_LEFT?-.1f:.1f);
        else value=key==SDL_SCANCODE_SPACE?!old:key==SDL_SCANCODE_RIGHT;
        meu_web_setting(*row,value);
    }
}
static int key_action(SDL_Scancode key)
{
    switch (key) {
        case SDL_SCANCODE_SPACE: return MEU_BRAKE;
        case SDL_SCANCODE_P: return MEU_PARK;
        case SDL_SCANCODE_E: return MEU_INTERACT;
        case SDL_SCANCODE_1: return MEU_TO_BRIDGE;
        case SDL_SCANCODE_2: return MEU_TO_QUARTERS;
        case SDL_SCANCODE_3: return MEU_TO_ENGINEERING;
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
    if ((input->type==SDL_EVENT_WINDOW_RESIZED || input->type==SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) && app.simulation.started && !app.simulation.paused) pause_game();
    if (input->type==SDL_EVENT_WILL_ENTER_BACKGROUND || input->type==SDL_EVENT_DID_ENTER_BACKGROUND) pause_game();
#ifndef __EMSCRIPTEN__
    if (input->type == SDL_EVENT_KEY_DOWN || input->type == SDL_EVENT_KEY_UP) {
        bool down = input->type == SDL_EVENT_KEY_DOWN;
        if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_F1)
            meu_web_settings(app.simulation.settings_open?1:0);
        else if (app.simulation.settings_open) {
            if (down && !input->key.repeat) {
                if (input->key.scancode==SDL_SCANCODE_ESCAPE) meu_web_settings(1);
                else settings_key(input->key.scancode);
            }
        } else if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_RETURN) start_game();
        else if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_ESCAPE) pause_game();
        else if (!input->key.repeat) meu_sim_action(&app.simulation,key_action(input->key.scancode),down);
    }
    if (input->type == SDL_EVENT_MOUSE_BUTTON_DOWN && input->button.button == SDL_BUTTON_LEFT) {
        if (app.simulation.paused) start_game();
        else app.dragging = true;
    }
    if (input->type == SDL_EVENT_MOUSE_BUTTON_UP) app.dragging = false;
    if (input->type == SDL_EVENT_MOUSE_MOTION && app.dragging)
        meu_sim_pointer_look(&app.simulation,input->motion.xrel,input->motion.yrel);
    if (input->type==SDL_EVENT_FINGER_DOWN) {
        if (input->tfinger.x<.5f && !app.move_touch) {
            app.move_touch=meu_sim_action(&app.simulation,MEU_FORWARD,true);
            app.move_finger=input->tfinger.fingerID;
        } else if (input->tfinger.x>=.5f && !app.look_touch && !app.simulation.paused) {
            app.look_touch=true; app.look_finger=input->tfinger.fingerID;
        }
    }
    if (input->type==SDL_EVENT_FINGER_UP || input->type==SDL_EVENT_FINGER_CANCELED) {
        if (app.move_touch && app.move_finger==input->tfinger.fingerID) {
            meu_sim_action(&app.simulation,MEU_FORWARD,false); app.move_touch=false;
        }
        if (app.look_touch && app.look_finger==input->tfinger.fingerID) app.look_touch=false;
    }
    if (input->type==SDL_EVENT_FINGER_MOTION && app.look_touch && app.look_finger==input->tfinger.fingerID) {
        int w,h; SDL_GetWindowSize(app.window,&w,&h);
        meu_sim_pointer_look(&app.simulation,input->tfinger.dx*(float)w,input->tfinger.dy*(float)h);
    }
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

#ifndef __EMSCRIPTEN__
static void tour_key(SDL_Scancode code,bool down)
{
    SDL_Event e={0}; e.type=down?SDL_EVENT_KEY_DOWN:SDL_EVENT_KEY_UP;
    e.key.scancode=code; e.key.down=down; SDL_PushEvent(&e);
}
static void tour_input(void)
{
    unsigned f=app.frames;
    if (f==0) { tour_key(SDL_SCANCODE_RETURN,true); tour_key(SDL_SCANCODE_RETURN,false); }
    if (f<3600) return;
    f-=3600;
    if (f==0 || f==790 || f==1971) tour_key(SDL_SCANCODE_S,true);
    if (f==180 || f==970 || f==2511) {
        tour_key(SDL_SCANCODE_S,false);
        SDL_Scancode floor=f==180?SDL_SCANCODE_2:f==970?SDL_SCANCODE_3:SDL_SCANCODE_1;
        tour_key(floor,true); tour_key(floor,false);
    }
    if (f==601 || f==1430 || f==3171) tour_key(SDL_SCANCODE_W,true);
    if (f==781 || f==1970 || f==3351) tour_key(SDL_SCANCODE_W,false);
}
static void tour_check(int width,int height)
{
    unsigned f=app.frames;
    if (!app.simulation.grounded) { SDL_Log("TOUR floor penetration frame=%u",f); app.exit_code=1; app.running=false; }
    if (f==3600 && (fabsf(app.simulation.x)>.02f || fabsf(app.simulation.z+8)>.02f)) {
        SDL_Log("TOUR standing drift exceeded 2 cm"); app.exit_code=1; app.running=false;
    }
    const char *name=NULL; int room=-1;
    if (f==3) { name="bridge"; room=MEU_BRIDGE; }
    if (f==3900) { name="lift"; room=MEU_LIFT; }
    if (f==4382) { name="quarters"; room=MEU_QUARTERS; }
    if (f==5201) { name="engineering"; room=MEU_ENGINEERING; }
    if (name) {
        char path[240]; snprintf(path,sizeof(path),"test-results/native-%s.bmp",name);
        if (meu_sim_room(&app.simulation)!=room || !meu_renderer_capture(path,width,height)) {
            SDL_Log("TOUR room/capture failed at %s",name); app.exit_code=1; app.running=false;
        }
    }
    if (f>=6960) {
        if (app.simulation.visited!=15 || meu_sim_room(&app.simulation)!=MEU_BRIDGE) app.exit_code=1;
        printf("TOUR build=%s scene=%s visited=%u ticks=%" PRIu64 " result=%s\n",
            MEU_BUILD_ID,MEU_SCENE_ID,app.simulation.visited,app.simulation.ticks,app.exit_code?"FAIL":"PASS");
        app.running=false;
    }
}
static void pilot_input(void)
{
    unsigned f=app.frames;
    if (f==0) { tour_key(SDL_SCANCODE_RETURN,true); tour_key(SDL_SCANCODE_RETURN,false); }
    if (f==1 || f==102) tour_key(SDL_SCANCODE_W,true);
    if (f==101) { tour_key(SDL_SCANCODE_W,false); tour_key(SDL_SCANCODE_E,true); tour_key(SDL_SCANCODE_E,false); }
    if (f==140) tour_key(SDL_SCANCODE_RIGHT,true);
    if (f==170) tour_key(SDL_SCANCODE_RIGHT,false);
    if (f==400) tour_key(SDL_SCANCODE_D,true);
    if (f==520) tour_key(SDL_SCANCODE_D,false);
    if (f==1610) {
        tour_key(SDL_SCANCODE_W,false); tour_key(SDL_SCANCODE_E,true); tour_key(SDL_SCANCODE_E,false);
        tour_key(SDL_SCANCODE_S,true);
    }
    if (f==1730) tour_key(SDL_SCANCODE_S,false);
}
static void pilot_check(int width,int height)
{
    MeuSimulation *s=&app.simulation;
    static double maximum_speed,previous_speed;
    if ((fabs(previous_speed)-fabs(s->travel.speed))*60>MEU_BRAKE_DECELERATION+.0001) {
        SDL_Log("PILOT uncontrolled stop"); app.exit_code=1; app.running=false;
    }
    previous_speed=s->travel.speed;
    if (fabs(s->travel.speed)>maximum_speed) maximum_speed=fabs(s->travel.speed);
    if (!s->grounded || meu_world_validate(s->world,s->robot,0)!=MEU_ROUTE_CLEAR ||
        (s->travel.helm && (fabsf(s->x)>.001f || fabsf(s->z+12.1667f)>.01f))) {
        SDL_Log("PILOT invalid footing/pose frame=%u",app.frames); app.exit_code=1; app.running=false;
    }
    if (app.frames==171 && fabs(s->robot.yaw)>.001) { SDL_Log("PILOT look changed heading"); app.exit_code=1; app.running=false; }
    const char *name=app.frames==102?"helm":app.frames==600?"travel":app.frames==1600?"blocked":NULL;
    if (name) {
        char path[240]; snprintf(path,sizeof(path),"test-results/native-pilot-%s.bmp",name);
        if (!meu_renderer_capture(path,width,height)) { app.exit_code=1; app.running=false; }
    }
    if (app.frames>=1800) {
        if (maximum_speed<8 || s->robot.z>-70 || !s->travel.parked || s->travel.helm ||
            s->travel.reason!=MEU_ROUTE_STRUCTURE) app.exit_code=1;
        printf("PILOT build=%s scene=%s distance=%.3f peak=%.3f reason=%d parked=%d result=%s\n",
            MEU_BUILD_ID,MEU_SCENE_ID,hypot(s->robot.x,s->robot.z),maximum_speed,s->travel.reason,
            s->travel.parked,app.exit_code?"FAIL":"PASS");
        app.running=false;
    }
}
static void lifecycle_input(void)
{
    unsigned f=app.frames; SDL_Scancode key=SDL_SCANCODE_UNKNOWN;
    switch (f) {
        case 4: case 50: case 53: case 130: case 132:key=SDL_SCANCODE_F1;break;
        case 5: case 131:key=SDL_SCANCODE_RIGHT;break;
        case 6: case 8: case 10: case 12:key=SDL_SCANCODE_DOWN;break;
        case 7: case 9: case 11:key=SDL_SCANCODE_SPACE;break;
        case 13:key=SDL_SCANCODE_LEFT;break;
        case 14:key=SDL_SCANCODE_W;break;
        case 20:key=SDL_SCANCODE_ESCAPE;break;
        case 25: case 55: case 80: case 100: case 133:key=SDL_SCANCODE_RETURN;break;
        case 52:key=SDL_SCANCODE_R;break;
        default:break;
    }
    if (key!=SDL_SCANCODE_UNKNOWN) { tour_key(key,true); tour_key(key,false); }
    if (f==26) tour_key(SDL_SCANCODE_W,true);
    if (f==60) tour_key(SDL_SCANCODE_RIGHT,true);
    if (f==51) {
        SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.scancode=SDL_SCANCODE_W;e.key.repeat=true;SDL_PushEvent(&e);
    }
    if (f==70 || f==71 || f==90 || f==91) {
        SDL_Event e={0}; e.type=(f==70)?SDL_EVENT_WINDOW_FOCUS_LOST:
            (f==90)?SDL_EVENT_WINDOW_RESIZED:SDL_EVENT_WINDOW_FOCUS_GAINED;
        SDL_PushEvent(&e);
    }
    if (f==110 || f==112) {
        SDL_Event e={0};e.type=f==110?SDL_EVENT_MOUSE_BUTTON_DOWN:SDL_EVENT_MOUSE_BUTTON_UP;
        e.button.button=SDL_BUTTON_LEFT;SDL_PushEvent(&e);
    }
    if (f==111) {
        SDL_Event e={0};e.type=SDL_EVENT_MOUSE_MOTION;e.motion.xrel=30;e.motion.yrel=20;SDL_PushEvent(&e);
    }
}
static void lifecycle_check(int width,int height)
{
    MeuSimulation *s=&app.simulation; unsigned f=app.frames;
    bool bad=false;
    if (f==16) {
        bad=!s->settings_open || app.audio_attempts!=0 || fabsf(s->settings.sensitivity-1.1f)>.001f ||
            !s->settings.invert_y || !s->settings.walk_bob || !s->settings.machine_sway || fabsf(s->settings.volume-.5f)>.001f;
        bad|=!meu_renderer_capture("test-results/native-settings.bmp",width,height);
    }
    if (f==22 || f==54 || f==73 || f==93) bad|=!s->paused || s->held!=0;
    if (f>=51) bad|=fabsf(s->z+9)>.01f || fabsf(s->x)>.001f || !s->grounded;
    if (f==121) bad|=!meu_renderer_capture("test-results/native-resumed.bmp",width,height);
    if (f>=150) {
        bad|=s->paused || s->settings_open || s->held!=0 || fabsf(s->yaw-.37f)>.005f ||
            fabsf(s->pitch+.16f)>.005f || fabsf(s->settings.sensitivity-1.1f)>.001f || app.audio_attempts!=1;
        printf("LIFECYCLE build=%s scene=%s settings=%.2f context=%d result=%s\n",MEU_BUILD_ID,MEU_SCENE_ID,
            s->settings.sensitivity,meu_sim_context(s),bad?"FAIL":"PASS");
        app.running=false;
    }
    if (bad) { SDL_Log("LIFECYCLE failed frame=%u z=%.4f context=%d held=%u",f,s->z,meu_sim_context(s),s->held); app.exit_code=1;app.running=false; }
}
#endif

static void frame(void)
{
#ifndef __EMSCRIPTEN__
    if (app.tour) tour_input();
    if (app.pilot) pilot_input();
    if (app.lifecycle) lifecycle_input();
#endif
    SDL_Event input;
    while (SDL_PollEvent(&input)) { event(&input); sync_input_context(); }
    double now = (double)SDL_GetTicksNS() / 1e9;
    meu_sim_advance(&app.simulation, (app.tour || app.pilot || app.lifecycle) ? MEU_TICK_SECONDS : now - app.previous_time);
    app.previous_time = now;
    int width = 0, height = 0;
#ifdef __EMSCRIPTEN__
    emscripten_get_canvas_element_size("#canvas", &width, &height);
#else
    SDL_GetWindowSizeInPixels(app.window, &width, &height);
#endif
    MeuSimulation render; meu_sim_render_state(&app.simulation,&render);
    bool draw=(!app.tour && !app.pilot && !app.lifecycle) || app.frames%30==0 || app.frames==2 || app.frames==4381 || app.frames==5200 || app.frames==3899 || (app.pilot && (app.frames==101 || app.frames==599 || app.frames==1599)) || (app.lifecycle && (app.frames==15 || app.frames==120));
    if (draw && !meu_renderer_draw(&render, width, height,app.settings_selection)) {
        SDL_Log("OpenGL frame failed"); app.exit_code = EXIT_FAILURE; app.running = false;
    }
#ifdef __EMSCRIPTEN__
    if (app.frames%6==0) {
        char hud[500];
        snprintf(hud,sizeof(hud),"if(window.MEU)window.MEU.hud(%d,%.1f,%d,%d,%d,%d,%d,%d,%d);",
            meu_sim_room(&app.simulation),app.simulation.y-MEU_EYE_HEIGHT,
            meu_sim_on_lift(&app.simulation),meu_sim_lift_moving(&app.simulation),
            app.simulation.lift_current,app.simulation.lift_target,meu_sim_use_target(&app.simulation),app.simulation.info_open,meu_sim_lift_ready(&app.simulation));
        emscripten_run_script(hud);
        snprintf(hud,sizeof(hud),"if(window.MEU)window.MEU.navigation(%d,%.2f,%.1f,%d,%d,%d,%.1f);",
            app.simulation.travel.helm,app.simulation.travel.speed,app.simulation.robot.yaw*57.295779513,
            app.simulation.travel.parked,app.simulation.travel.park_requested,app.simulation.travel.reason,
            meu_travel_stop_distance(app.simulation.travel.speed));
        emscripten_run_script(hud);
    }
#endif
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
#ifndef __EMSCRIPTEN__
    if (app.tour) tour_check(width,height);
    if (app.pilot) pilot_check(width,height);
    if (app.lifecycle) lifecycle_check(width,height);
#endif
    if (draw) SDL_GL_SwapWindow(app.window);
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
    // The DOM input adapter owns web keys, including native form controls.
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#none");
#endif
    app.running = true;
    meu_sim_init(&app.simulation);
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--smoke-test")) app.smoke = true;
        else if (!strcmp(argv[i], "--lifecycle-test")) app.lifecycle=true;
        else if (!strcmp(argv[i], "--pilot-test")) app.pilot=true;
        else if (!strcmp(argv[i], "--tour-test")) { app.tour=true; app.simulation.motion_fixture=true; }
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
    SDL_GL_SetSwapInterval((app.tour || app.pilot || app.lifecycle)?0:1);
    SDL_Log("MEU build=%s scene=%s content=%d", MEU_BUILD_ID, MEU_SCENE_ID, MEU_CONTENT_VERSION);
    app.previous_time = (double)SDL_GetTicksNS() / 1e9;
    if (app.smoke) meu_sim_start(&app.simulation);
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.ready();");
    sync_input_context(); sync_preferences();
    emscripten_set_main_loop(frame, 0, 1);
#else
    while (app.running) { frame(); if (!app.tour && !app.pilot && !app.lifecycle) SDL_Delay(1); }
    cleanup();
#endif
    return app.exit_code;
}
