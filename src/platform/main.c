#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "core/simulation.h"
#include "core/snapshot.h"
#include "game/guide.h"
#ifndef __EMSCRIPTEN__
#include "platform/file_storage.h"
#endif
#include "render/renderer.h"
#include "render/mesh.h"
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
    bool running, smoke, tour, pilot, lifecycle, expedition_replay, mouse_captured, audio_ready;
    const char *capture;
    char storage_status[180];
#ifndef __EMSCRIPTEN__
    char storage_directory[MEU_PATH_BYTES];
    const char *save_directory_argument;
    Uint32 file_event;
    bool dialog_pending;
#endif
    int persistence_test;
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

#ifndef __EMSCRIPTEN__
static bool mouse_playing(void)
{
    MeuContext c=meu_sim_context(&app.simulation);return c==MEU_CONTEXT_FOOT || c==MEU_CONTEXT_HELM;
}
static void release_mouse(void)
{
    if(app.mouse_captured)SDL_SetWindowRelativeMouseMode(app.window,false);
    app.mouse_captured=false;
}
static void capture_mouse(void)
{
    if(!app.mouse_captured && mouse_playing()) {
        app.mouse_captured=SDL_SetWindowRelativeMouseMode(app.window,true);
        if(!app.mouse_captured)SDL_Log("Mouse capture unavailable: %s",SDL_GetError());
    }
}
#endif

static void sync_input_context(void)
{
    MeuSimulation *s=&app.simulation;
    if (app.input_epoch_seen==s->input_epoch) return;
    app.input_epoch_seen=s->input_epoch;
#ifndef __EMSCRIPTEN__
    if(!mouse_playing())release_mouse();
#endif
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
#ifndef __EMSCRIPTEN__
    release_mouse();
#endif
    if (app.audio) SDL_PauseAudioStreamDevice(app.audio);
    SDL_SetWindowTitle(app.window, "MEU - Paused | Enter: resume | F1: settings");
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.paused();");
#endif
    sync_input_context();
}

static void start_game(void)
{
    if (app.simulation.settings_open || app.simulation.journeys_open || app.simulation.help_open) return;
#ifndef __EMSCRIPTEN__
    if (app.dialog_pending) return;
#endif
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
    SDL_SetWindowTitle(app.window, "MEU - Exploration | F1: settings | WASD: move | click: capture mouse | Esc: release and pause");
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
WEB_EXPORT int meu_web_station(int row)
{
    bool accepted=meu_sim_station_command(&app.simulation,row);
    sync_input_context();
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if(window.MEU)window.MEU.station();");
#endif
    return accepted;
}
WEB_EXPORT const char *meu_web_station_text(int row)
{
    static char text[256];
    if (row==-4) return meu_sim_objective(&app.simulation);
    if (row==-3) return meu_sim_station_title(&app.simulation);
    if (row==-2) return app.simulation.message;
    if (row==-1) meu_sim_station_status(&app.simulation,text,sizeof(text));
    else meu_sim_station_line(&app.simulation,row,text,sizeof(text));
    return text;
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

WEB_EXPORT int meu_web_help(int operation)
{
    if(operation<0)return app.settings_selection;
#ifndef __EMSCRIPTEN__
    if(app.dialog_pending)return app.settings_selection;
#endif
    if(operation==0) { pause_game();meu_sim_open_help(&app.simulation);app.settings_selection=0; }
    else if(operation==1)meu_sim_close_help(&app.simulation);
    else if(app.simulation.help_open) {
        if(operation==2)app.settings_selection=(app.settings_selection+MEU_GUIDE_TOPICS-1)%MEU_GUIDE_TOPICS;
        else if(operation==3)app.settings_selection=(app.settings_selection+1)%MEU_GUIDE_TOPICS;
        else if(operation>=4 && operation<4+MEU_GUIDE_TOPICS)app.settings_selection=operation-4;
    }
    sync_input_context();sync_preferences();
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if(window.MEU)window.MEU.help();");
#endif
    return app.settings_selection;
}
WEB_EXPORT const char *meu_web_help_text(int field)
{
    if(field<=-10)return meu_guide_topic(-10-field);
    if(field==-1)return meu_guide_topic(app.settings_selection);
    return meu_guide_text(&app.simulation,app.settings_selection,field);
}

static void restored_game(void)
{
    app.previous_time=(double)SDL_GetTicksNS()/1e9;
    if (app.audio) SDL_PauseAudioStreamDevice(app.audio);
    sync_input_context(); sync_preferences();
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if(window.MEU)window.MEU.restored();");
#endif
}
WEB_EXPORT void meu_web_journeys(int operation)
{
    if (operation==0) { pause_game(); meu_sim_open_journeys(&app.simulation); }
    else if (operation==1) meu_sim_close_journeys(&app.simulation);
    sync_input_context(); sync_preferences();
}
#ifdef __EMSCRIPTEN__
WEB_EXPORT const char *meu_web_snapshot(uint32_t generation)
{
    static char response[MEU_SAVE_HEX_BYTES+80]; char hex[MEU_SAVE_HEX_BYTES]={0}; uint8_t data[MEU_SAVE_BYTES];
    MeuSaveResult result=meu_snapshot_encode(&app.simulation,generation,data);
    if (result==MEU_SAVE_OK) meu_snapshot_hex(data,hex);
    snprintf(response,sizeof(response),"{\"result\":%d,\"data\":\"%s\"}",result,hex);
    return response;
}
WEB_EXPORT const char *meu_web_probe(const char *hex)
{
    static char response[120]; uint8_t data[MEU_SAVE_BYTES];uint32_t generation=0;MeuSimulation candidate;
    MeuSaveResult result=MEU_SAVE_MALFORMED;
    if (meu_snapshot_unhex(hex,data)) result=meu_snapshot_decode(data,sizeof(data),&candidate,&generation);
    snprintf(response,sizeof(response),"{\"result\":%d,\"generation\":%u,\"room\":%d}",
        result,generation,result==MEU_SAVE_OK?meu_sim_room(&candidate):-1);
    return response;
}
WEB_EXPORT int meu_web_restore(const char *hex)
{
    uint8_t data[MEU_SAVE_BYTES];MeuSaveResult result=MEU_SAVE_MALFORMED;
    if (meu_snapshot_unhex(hex,data)) result=meu_snapshot_restore(&app.simulation,data,sizeof(data),NULL);
    if (result==MEU_SAVE_OK) restored_game();
    return result;
}
WEB_EXPORT const char *meu_web_save_message(int result) { return meu_save_message((MeuSaveResult)result); }
#else
static void storage_message(const char *message)
{
    snprintf(app.storage_status,sizeof(app.storage_status),"%s",message);
    SDL_Log("%s",message);
}
static void storage_action(int operation,const char *path)
{
    MeuSaveResult result=MEU_SAVE_IO;uint32_t generation=0;
    if (operation==0) result=meu_file_save(app.storage_directory,&app.simulation,&generation);
    if (operation==1 || operation==2) result=meu_file_load(app.storage_directory,operation==2,&app.simulation,&generation);
    if (operation==3) result=meu_file_export(path,&app.simulation);
    if (operation==4) result=meu_file_import(path,&app.simulation,&generation);
    if (result!=MEU_SAVE_OK) { storage_message(meu_save_message(result)); return; }
    if (operation==1 || operation==2 || operation==4) restored_game();
    char message[180];
    if (operation==0) snprintf(message,sizeof(message),"Saved journey %u. Settings included.",generation);
    else if (operation==1) snprintf(message,sizeof(message),"Loaded journey %u. Close, then resume.",generation);
    else if (operation==2) snprintf(message,sizeof(message),"Recovered journey %u. Damaged source kept.",generation);
    else snprintf(message,sizeof(message),"%s",operation==3?"Journey exported. Local save unchanged.":"Journey imported. Save here to keep it.");
    storage_message(message);
}
static void SDLCALL chosen_file(void *userdata,const char * const *files,int filter)
{
    (void)filter;SDL_Event e={0};e.type=app.file_event;e.user.code=(Sint32)(intptr_t)userdata;
    e.user.data1=files && files[0]?SDL_strdup(files[0]):NULL;
    e.user.data2=(void *)(intptr_t)(files?0:1);
    if (!SDL_PushEvent(&e)) SDL_free(e.user.data1);
}
static void choose_file(int operation)
{
    static const SDL_DialogFileFilter filter={"MEU journey","meu"};
    if (app.dialog_pending) return;
    if (app.persistence_test) {
        char path[MEU_PATH_BYTES];
        if (meu_file_path(app.storage_directory,"roundtrip.meu",path)) storage_action(operation,path);
        return;
    }
    if (operation==3) {
        uint8_t data[MEU_SAVE_BYTES];MeuSaveResult result=meu_snapshot_encode(&app.simulation,1,data);
        if (result!=MEU_SAVE_OK) { storage_message(meu_save_message(result));return; }
    }
    app.dialog_pending=true;storage_message("Choose a journey file. The game remains paused.");
    if (operation==3) SDL_ShowSaveFileDialog(chosen_file,(void *)(intptr_t)operation,app.window,&filter,1,NULL);
    else SDL_ShowOpenFileDialog(chosen_file,(void *)(intptr_t)operation,app.window,&filter,1,NULL,false);
}
static void initialize_storage(void)
{
    if (app.smoke || app.tour || app.pilot || app.lifecycle) return;
    if (app.save_directory_argument) {
        if (strlen(app.save_directory_argument)>=sizeof(app.storage_directory)) { storage_message(meu_save_message(MEU_SAVE_IO));return; }
        snprintf(app.storage_directory,sizeof(app.storage_directory),"%s",app.save_directory_argument);
    }
    else {
        char *directory=SDL_GetPrefPath("MEU","MechanizedExplorationUnit");
        if (directory) {
            if (strlen(directory)>=sizeof(app.storage_directory)) { SDL_free(directory);storage_message(meu_save_message(MEU_SAVE_IO));return; }
            snprintf(app.storage_directory,sizeof(app.storage_directory),"%s",directory);SDL_free(directory);
        }
    }
    if (!meu_file_mkdir(app.storage_directory)) { storage_message(meu_save_message(MEU_SAVE_IO));return; }
    SDL_Log("MEU save directory: %s",app.storage_directory);
    if (app.persistence_test==1 || app.expedition_replay) { storage_message("Persistence replay ready.");return; }
    MeuJournal journal;
    if (!meu_file_inspect(app.storage_directory,&journal)) storage_message(meu_save_message(MEU_SAVE_IO));
    else if (journal.latest.result==MEU_SAVE_OK) storage_action(1,NULL);
    else if (journal.latest.present) storage_message(journal.previous.result==MEU_SAVE_OK?
        "Latest save unavailable. F2 then F8 recovers the previous journey. Source kept.":meu_save_message(journal.latest.result));
    else if (journal.previous.result==MEU_SAVE_OK) storage_message("Latest save missing. F2 then F8 recovers the previous journey.");
    else if (journal.previous.present) storage_message(meu_save_message(journal.previous.result));
    else storage_message("No saved journey yet. F2 opens saved journeys.");
}
#endif

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
    size_t used=strlen(result);
    if(used && used<sizeof(result)-600) {
        MeuSimulation *s=&app.simulation;MeuExpedition *e=&s->expedition;
        snprintf(result+used-1,sizeof(result)-used+1,",\"energy\":%u,\"heat\":%u,\"station\":%d,\"fault\":%s,\"job\":%u,\"jobTicks\":%u,\"bulk\":%s,\"coupler\":%s,\"bypass\":%s,\"suit\":%s,\"suitTicks\":%u,\"exposure\":%u,\"installed\":%s,\"completed\":%s}",
            s->power.energy,s->power.heat,s->station,e->fault?"true":"false",e->job,e->job_ticks,e->bulk?"true":"false",
            e->coupler?"true":"false",e->bypass?"true":"false",e->suit?"true":"false",e->suit_ticks,e->exposure,
            e->installed?"true":"false",e->completed?"true":"false");
    }
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
    if (input->type==app.file_event) {
        app.dialog_pending=false;
        if (input->user.data1) storage_action(input->user.code,input->user.data1);
        else storage_message(input->user.data2?"The file picker is unavailable.":"File selection cancelled. Journey unchanged.");
        SDL_free(input->user.data1);
    }
    if (input->type == SDL_EVENT_KEY_DOWN || input->type == SDL_EVENT_KEY_UP) {
        bool down = input->type == SDL_EVENT_KEY_DOWN;
        if (app.dialog_pending) return;
        if(down && !input->key.repeat && (input->key.scancode==SDL_SCANCODE_H || input->key.scancode==SDL_SCANCODE_F3))
            meu_web_help(app.simulation.help_open?1:0);
        else if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_F2)
            meu_web_journeys(app.simulation.journeys_open?1:0);
        else if (app.simulation.journeys_open) {
            if (down && !input->key.repeat) {
                SDL_Scancode key=input->key.scancode;
                if (key==SDL_SCANCODE_ESCAPE) meu_web_journeys(1);
                if (key==SDL_SCANCODE_F5) storage_action(0,NULL);
                if (key==SDL_SCANCODE_F9) storage_action(1,NULL);
                if (key==SDL_SCANCODE_F8) storage_action(2,NULL);
                if (key==SDL_SCANCODE_F6) choose_file(3);
                if (key==SDL_SCANCODE_F7) choose_file(4);
            }
        } else if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_F1)
            meu_web_settings(app.simulation.settings_open?1:0);
        else if (app.simulation.settings_open) {
            if (down && !input->key.repeat) {
                if (input->key.scancode==SDL_SCANCODE_ESCAPE) meu_web_settings(1);
                else settings_key(input->key.scancode);
            }
        } else if(app.simulation.help_open) {
            if(down && !input->key.repeat) {
                SDL_Scancode key=input->key.scancode;
                if(key==SDL_SCANCODE_ESCAPE)meu_web_help(1);
                else if(key==SDL_SCANCODE_LEFT)meu_web_help(2);
                else if(key==SDL_SCANCODE_RIGHT)meu_web_help(3);
                else if(key>=SDL_SCANCODE_1 && key<=SDL_SCANCODE_7)meu_web_help(4+key-SDL_SCANCODE_1);
            }
        } else if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_RETURN) start_game();
        else if (down && !input->key.repeat && input->key.scancode==SDL_SCANCODE_ESCAPE) pause_game();
        else if (app.simulation.station) {
            if (down && !input->key.repeat) {
                SDL_Scancode key=input->key.scancode;
                if (key==SDL_SCANCODE_E || key==SDL_SCANCODE_0) meu_web_station(-1);
                else if (key>=SDL_SCANCODE_1 && key<=SDL_SCANCODE_8) meu_web_station(key-SDL_SCANCODE_1);
            }
        }
        else if (!input->key.repeat) meu_sim_action(&app.simulation,key_action(input->key.scancode),down);
    }
    if (input->type == SDL_EVENT_MOUSE_BUTTON_DOWN && input->button.button == SDL_BUTTON_LEFT && input->button.which!=SDL_TOUCH_MOUSEID) {
        if(!app.mouse_captured) {
            int w,h,pw,ph;SDL_GetWindowSize(app.window,&w,&h);SDL_GetWindowSizeInPixels(app.window,&pw,&ph);
            int command=w>0 && h>0?meu_mesh_help_hit(pw,ph,input->button.x*(float)pw/(float)w,input->button.y*(float)ph/(float)h,app.simulation.help_open):-1;
            if(command>=0) { meu_web_help(command);return; }
            if(app.simulation.help_open)return;
        }
        if (app.simulation.paused) start_game();
        capture_mouse();
    }
    if (input->type == SDL_EVENT_MOUSE_MOTION && app.mouse_captured && input->motion.which!=SDL_TOUCH_MOUSEID)
        meu_sim_pointer_look(&app.simulation,input->motion.xrel,input->motion.yrel);
    if (input->type==SDL_EVENT_FINGER_DOWN) {
        int w,h;SDL_GetWindowSizeInPixels(app.window,&w,&h);
        int command=meu_mesh_help_hit(w,h,input->tfinger.x*(float)w,input->tfinger.y*(float)h,app.simulation.help_open);
        if(command>=0) { meu_web_help(command);return; }
        if(app.simulation.help_open)return;
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
#ifndef __EMSCRIPTEN__
    release_mouse();
#endif
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
        case 1: case 2: case 154: case 170:key=SDL_SCANCODE_H;break;
        case 156:key=SDL_SCANCODE_RIGHT;break;
        case 162:key=SDL_SCANCODE_ESCAPE;break;
        case 163: case 172:key=SDL_SCANCODE_RETURN;break;
        case 4: case 50: case 53: case 130: case 132:key=SDL_SCANCODE_F1;break;
        case 5: case 131:key=SDL_SCANCODE_RIGHT;break;
        case 6: case 8: case 10: case 12:key=SDL_SCANCODE_DOWN;break;
        case 7: case 9: case 11:key=SDL_SCANCODE_SPACE;break;
        case 13:key=SDL_SCANCODE_LEFT;break;
        case 14:key=SDL_SCANCODE_W;break;
        case 142: case 144:key=SDL_SCANCODE_F2;break;
        case 20:key=SDL_SCANCODE_ESCAPE;break;
        case 25: case 55: case 80: case 100: case 133: case 137: case 145:key=SDL_SCANCODE_RETURN;break;
        case 52:key=SDL_SCANCODE_R;break;
        default:break;
    }
    if(f==154)tour_key(SDL_SCANCODE_W,true);
    if (key!=SDL_SCANCODE_UNKNOWN) { tour_key(key,true); tour_key(key,false); }
    if(f==165)tour_key(SDL_SCANCODE_W,false);
    if(f==166) {
        int w,h;SDL_GetWindowSize(app.window,&w,&h);(void)h;
        SDL_Event e={0};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.button=SDL_BUTTON_LEFT;
        e.button.x=(float)w-60;e.button.y=60;SDL_PushEvent(&e);
    }
    if (f==26) tour_key(SDL_SCANCODE_W,true);
    if (f==60) tour_key(SDL_SCANCODE_RIGHT,true);
    if (f==51) {
        SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.scancode=SDL_SCANCODE_W;e.key.repeat=true;SDL_PushEvent(&e);
    }
    if (f==70 || f==71 || f==90 || f==91 || f==135 || f==136 || f==159 || f==160) {
        SDL_Event e={0}; e.type=(f==70 || f==135 || f==159)?SDL_EVENT_WINDOW_FOCUS_LOST:
            (f==90)?SDL_EVENT_WINDOW_RESIZED:SDL_EVENT_WINDOW_FOCUS_GAINED;
        SDL_PushEvent(&e);
    }
    if (f==110 || f==112 || f==134 || f==140 || f==152 || f==153) {
        SDL_Event e={0};e.type=(f==112 || f==153)?SDL_EVENT_MOUSE_BUTTON_UP:SDL_EVENT_MOUSE_BUTTON_DOWN;
        e.button.button=SDL_BUTTON_LEFT;SDL_PushEvent(&e);
    }
    if (f==111 || f==114) {
        SDL_Event e={0};e.type=SDL_EVENT_MOUSE_MOTION;e.motion.xrel=f==111?30:10;e.motion.yrel=f==111?20:-5;SDL_PushEvent(&e);
    }
}
static void lifecycle_check(int width,int height)
{
    MeuSimulation *s=&app.simulation; unsigned f=app.frames;
    static MeuSimulation help_frozen;
    bool bad=false;
    if(f==2)bad|=!s->help_open || s->started || app.audio_attempts!=0;
    if(f==155) {
        bad|=!s->help_open || !s->paused || s->held || app.mouse_captured || SDL_GetWindowRelativeMouseMode(app.window);
        help_frozen=*s;
    }
    if(f>=156 && f<=159)bad|=memcmp(&help_frozen,s,sizeof(*s))!=0;
    if(f==158)bad|=app.settings_selection!=1 || !meu_renderer_capture("test-results/native-guide.bmp",width,height);
    if(f==160 || f==161 || f==167)bad|=!s->help_open || !s->paused || s->held;
    if(f==163 || f==164 || f==171 || f==173)bad|=s->help_open || s->held || app.mouse_captured || SDL_GetWindowRelativeMouseMode(app.window);
    if(f==163 || f==171)bad|=!s->paused;
    if(f==167)bad|=app.settings_selection!=0;
    if (f==16) {
        bad=!s->settings_open || app.audio_attempts!=0 || fabsf(s->settings.sensitivity-1.1f)>.001f ||
            !s->settings.invert_y || !s->settings.walk_bob || !s->settings.machine_sway || fabsf(s->settings.volume-.5f)>.001f;
        bad|=!meu_renderer_capture("test-results/native-settings.bmp",width,height);
    }
    if (f==22 || f==54 || f==73 || f==93) bad|=!s->paused || s->held!=0;
    if (f>=51) bad|=fabsf(s->z+9)>.01f || fabsf(s->x)>.001f || !s->grounded;
    if (f==113 || f==115 || f==135 || f==141)bad|=!app.mouse_captured || !SDL_GetWindowRelativeMouseMode(app.window);
    if (f==131 || f==136 || f==137 || f==138 || f==143 || f==146)
        bad|=app.mouse_captured || SDL_GetWindowRelativeMouseMode(app.window);
    if (f==121) bad|=!meu_renderer_capture("test-results/native-resumed.bmp",width,height);
    if (f>=180) {
        bad|=s->paused || s->settings_open || s->held!=0 || fabsf(s->yaw-.41f)>.005f ||
            fabsf(s->pitch+.14f)>.005f || fabsf(s->settings.sensitivity-1.1f)>.001f || app.audio_attempts!=1;
        printf("LIFECYCLE build=%s scene=%s settings=%.2f context=%d result=%s\n",MEU_BUILD_ID,MEU_SCENE_ID,
            s->settings.sensitivity,meu_sim_context(s),bad?"FAIL":"PASS");
        app.running=false;
    }
    if (bad) { SDL_Log("LIFECYCLE failed frame=%u z=%.4f context=%d held=%u",f,s->z,meu_sim_context(s),s->held); app.exit_code=1;app.running=false; }
}
static void persistence_input(void)
{
    unsigned f=app.frames;SDL_Scancode key=SDL_SCANCODE_UNKNOWN;
    if (app.persistence_test==1 || app.expedition_replay) {
        switch(f) {
            case 0:key=SDL_SCANCODE_RETURN;break;
            case 62:case 72:key=SDL_SCANCODE_F1;break;
            case 63:key=SDL_SCANCODE_RIGHT;break;
            case 64:case 66:case 68:case 70:key=SDL_SCANCODE_DOWN;break;
            case 65:case 67:case 69:key=SDL_SCANCODE_SPACE;break;
            case 71:key=SDL_SCANCODE_LEFT;break;
            case 73:key=SDL_SCANCODE_F2;break;
            case 74:key=SDL_SCANCODE_F5;break;
            default:break;
        }
        if (f==1) tour_key(SDL_SCANCODE_W,true);
        if (f==61) tour_key(SDL_SCANCODE_W,false);
    } else {
        switch(f) {
            case 11:case 13:case 40:case 45:case 51:key=SDL_SCANCODE_F2;break;
            case 12:key=SDL_SCANCODE_F6;break;
            case 14:case 46:key=SDL_SCANCODE_RETURN;break;
            case 41:key=SDL_SCANCODE_F7;break;
            case 50:key=SDL_SCANCODE_ESCAPE;break;
            case 52:key=SDL_SCANCODE_F9;break;
            default:break;
        }
        if (f==15) tour_key(SDL_SCANCODE_W,true);
        if (f==39) tour_key(SDL_SCANCODE_W,false);
    }
    if (key!=SDL_SCANCODE_UNKNOWN) {tour_key(key,true);tour_key(key,false);}
}
static void persistence_check(int width,int height)
{
    MeuSimulation *s=&app.simulation;unsigned f=app.frames;bool bad=false;
    if (app.persistence_test==2 && f==9) bad=!s->started || !s->paused || s->held!=0 || app.audio_attempts!=0 || fabsf(s->z+10.5f)>.01f;
    if ((app.persistence_test==1 && f==80) || (app.persistence_test==2 && f==60)) {
        MeuJournal journal={0};bad|=!meu_file_inspect(app.storage_directory,&journal) || journal.latest.result!=MEU_SAVE_OK;
        bad|=!s->paused || !s->journeys_open || s->held!=0 || !s->travel.parked || s->travel.helm ||
            fabsf(s->x)>.001f || fabsf(s->z+10.5f)>.01f || fabsf(s->settings.sensitivity-1.1f)>.001f ||
            !s->settings.invert_y || !s->settings.walk_bob || !s->settings.machine_sway || fabsf(s->settings.volume-.5f)>.001f;
        const char *capture=app.persistence_test==1?"test-results/native-save-menu.bmp":"test-results/native-save-restored.bmp";
        bad|=!meu_renderer_capture(capture,width,height);
        printf("PERSISTENCE mode=%s build=%s scene=%s generation=%u result=%s\n",app.persistence_test==1?"write":"fresh-read-export-import",
            MEU_BUILD_ID,MEU_SCENE_ID,journal.latest.generation,bad?"FAIL":"PASS");
        app.running=false;
    }
    if (bad) {SDL_Log("PERSISTENCE failed frame=%u z=%.4f status=%s",f,s->z,app.storage_status);app.exit_code=1;app.running=false;}
}
#include "../../tests/client/expedition_replay.h"
#endif

static void frame(void)
{
#ifndef __EMSCRIPTEN__
    if (app.expedition_replay) expedition_input();
    if (app.tour) tour_input();
    if (app.pilot) pilot_input();
    if (app.lifecycle) lifecycle_input();
    if (app.persistence_test) persistence_input();
#endif
    SDL_Event input;
    while (SDL_PollEvent(&input)) { event(&input); sync_input_context(); }
    double now = (double)SDL_GetTicksNS() / 1e9;
    meu_sim_advance(&app.simulation, (app.tour || app.pilot || app.lifecycle || app.persistence_test || app.expedition_replay) ? MEU_TICK_SECONDS : now - app.previous_time);
    app.previous_time = now;
    int width = 0, height = 0;
#ifdef __EMSCRIPTEN__
    emscripten_get_canvas_element_size("#canvas", &width, &height);
#else
    SDL_GetWindowSizeInPixels(app.window, &width, &height);
#endif
    MeuSimulation render; meu_sim_render_state(&app.simulation,&render);
    bool draw=(app.expedition_replay && (app.simulation.station || app.simulation.expedition.completed)) || (!app.tour && !app.pilot && !app.lifecycle && !app.expedition_replay) || app.frames%30==0 || app.frames==2 || app.frames==4381 || app.frames==5200 || app.frames==3899 || (app.pilot && (app.frames==101 || app.frames==599 || app.frames==1599)) || (app.lifecycle && (app.frames==15 || app.frames==120 || app.frames==157));
    if (draw && !meu_renderer_draw(&render, width, height,app.settings_selection,app.storage_status)) {
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
        emscripten_run_script("if(window.MEU)window.MEU.station();");
        snprintf(hud,sizeof(hud),"if(window.MEU)window.MEU.navigation(%d,%.2f,%.1f,%d,%d,%d,%.1f,%.1f,%.1f);",
            app.simulation.travel.helm,app.simulation.travel.speed,app.simulation.robot.yaw*57.295779513,
            app.simulation.travel.parked,app.simulation.travel.park_requested,app.simulation.travel.reason,
            meu_travel_stop_distance(app.simulation.travel.speed),app.simulation.robot.x,app.simulation.robot.z);
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
    if (draw && app.expedition_replay) expedition_artifacts(width,height);
    if (app.tour) tour_check(width,height);
    if (app.pilot) pilot_check(width,height);
    if (app.lifecycle) lifecycle_check(width,height);
    if (app.persistence_test) persistence_check(width,height);
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
#ifndef __EMSCRIPTEN__
        else if (!strcmp(argv[i], "--expedition-test")) app.expedition_replay=true;
        else if (!strcmp(argv[i], "--persistence-test") && i+1<argc) {
            const char *mode=argv[++i];app.persistence_test=!strcmp(mode,"write")?1:!strcmp(mode,"read")?2:0;
            if (!app.persistence_test) return EXIT_FAILURE;
        }
        else if (!strcmp(argv[i], "--save-dir") && i+1<argc) app.save_directory_argument=argv[++i];
#endif
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
    SDL_GL_SetSwapInterval((app.tour || app.pilot || app.lifecycle || app.persistence_test || app.expedition_replay)?0:1);
    SDL_Log("MEU build=%s scene=%s content=%d", MEU_BUILD_ID, MEU_SCENE_ID, MEU_CONTENT_VERSION);
    app.previous_time = (double)SDL_GetTicksNS() / 1e9;
    if (app.smoke) meu_sim_start(&app.simulation);
#ifdef __EMSCRIPTEN__
    emscripten_run_script("if (window.MEU) window.MEU.ready();");
    sync_input_context(); sync_preferences();
    emscripten_set_main_loop(frame, 0, 1);
#else
    app.file_event=SDL_RegisterEvents(1);
    initialize_storage();
    while (app.running) { frame(); if (!app.tour && !app.pilot && !app.lifecycle && !app.persistence_test && !app.expedition_replay) SDL_Delay(1); }
    cleanup();
#endif
    return app.exit_code;
}
