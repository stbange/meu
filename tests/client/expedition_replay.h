/* Deterministic integration replay. No fixture teleport or resource mutation:
   every movement, station and transaction enters through SDL input events. */
static unsigned expedition_step,expedition_step_frame;
static void expedition_press(SDL_Scancode code) { tour_key(code,true);tour_key(code,false); }
static void expedition_next(void)
{
    tour_key(SDL_SCANCODE_W,false);tour_key(SDL_SCANCODE_S,false);
    ++expedition_step;expedition_step_frame=app.frames;
    SDL_Log("EXPEDITION step=%u tick=%" PRIu64 " pos=%.2f,%.2f,%.2f robot=%.2f,%.2f station=%d message=%s",
        expedition_step,app.simulation.ticks,app.simulation.x,app.simulation.y,app.simulation.z,
        app.simulation.robot.x,app.simulation.robot.z,app.simulation.station,app.simulation.message);
}
static void expedition_face(float x,float y,float z)
{
    MeuSimulation *s=&app.simulation;
    float yaw=atan2f(x-s->x,-(z-s->z));
    float pitch=atan2f(y-s->y,hypotf(x-s->x,z-s->z));
    SDL_Event e={0};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.button=SDL_BUTTON_LEFT;SDL_PushEvent(&e);
    e=(SDL_Event){0};e.type=SDL_EVENT_MOUSE_MOTION;
    e.motion.xrel=remainderf(yaw-s->yaw,6.2831853f)/(.004f*s->settings.sensitivity);
    e.motion.yrel=-(pitch-s->pitch)/(.004f*s->settings.sensitivity);SDL_PushEvent(&e);
}
static void expedition_walk(float x,float z)
{
    if(hypotf(app.simulation.x-x,app.simulation.z-z)<.065f) { expedition_next();return; }
    expedition_face(x,app.simulation.y,z);tour_key(SDL_SCANCODE_W,true);
}
static void expedition_use(float x,float y,float z)
{
    expedition_face(x,y,z);expedition_press(SDL_SCANCODE_E);expedition_next();
}
static void expedition_digit(SDL_Scancode code) { expedition_press(code);expedition_next(); }
static void expedition_input(void)
{
    MeuSimulation *s=&app.simulation;
    if(app.frames-expedition_step_frame>2400 || app.frames>25000 || !s->grounded || s->expedition.catastrophe) {
        SDL_Log("EXPEDITION FAIL step=%u frame=%u pos=%.3f,%.3f,%.3f robot=%.2f,%.2f yaw=%.3f speed=%.2f reason=%d installed=%d power=%u message=%s",expedition_step,app.frames,s->x,s->y,s->z,s->robot.x,s->robot.z,s->robot.yaw,s->travel.speed,s->travel.reason,s->expedition.installed,s->power.energy,s->message);
        app.exit_code=EXIT_FAILURE;app.running=false;return;
    }
    switch(expedition_step) {
        case 0:expedition_digit(SDL_SCANCODE_RETURN);break;
        case 1:expedition_walk(0,-13);break;
        case 2:expedition_use(0,197.2f,-14.5f);break;
        case 3:
            if(!s->travel.helm) {app.exit_code=EXIT_FAILURE;app.running=false;break;}
            if(s->robot.z<=-122) {expedition_press(SDL_SCANCODE_P);expedition_next();}
            else { tour_key(SDL_SCANCODE_W,true); }
            break;
        case 4:if(s->travel.parked)expedition_digit(SDL_SCANCODE_E);break;
        case 5:expedition_walk(0,-6);break;
        case 6:expedition_walk(0,0);break;
        case 7:expedition_digit(SDL_SCANCODE_3);break;
        case 8:if(!meu_sim_lift_moving(s) && s->lift_current==2)expedition_next();break;
        case 9:expedition_walk(0,-6);break;
        case 10:expedition_walk(-4,-6.8f);break;
        case 11:expedition_walk(-8,-6.8f);break;
        case 12:expedition_use(-9,113.3f,-9.3f);break;
        case 13:expedition_digit(SDL_SCANCODE_1);break;
        case 14:expedition_digit(SDL_SCANCODE_2);break;
        case 15:expedition_digit(SDL_SCANCODE_3);break;
        case 16:if(s->expedition.fault)expedition_next();break;
        case 17:expedition_digit(SDL_SCANCODE_8);break;
        case 18:expedition_digit(SDL_SCANCODE_1);break;
        case 19:expedition_digit(SDL_SCANCODE_2);break;
        case 20:if(s->expedition.repaired && !s->expedition.job)expedition_next();break;
        case 21:expedition_digit(SDL_SCANCODE_E);break;
        case 22:expedition_walk(-4,-6.8f);break;
        case 23:expedition_walk(0,-7);break;
        case 24:expedition_walk(6,-8.4f);break;
        case 25:expedition_use(7,113.3f,-10.5f);break;
        case 26:expedition_digit(SDL_SCANCODE_1);break;
        case 27:expedition_digit(SDL_SCANCODE_E);break;
        case 28:expedition_walk(10,-8.5f);break;
        case 29:expedition_walk(12,-10);break;
        case 30:expedition_use(13.5f,113.3f,-10);break;
        case 31:expedition_digit(SDL_SCANCODE_1);break;
        case 32:expedition_digit(SDL_SCANCODE_2);break;
        case 33:expedition_digit(SDL_SCANCODE_3);break;
        case 34:expedition_digit(SDL_SCANCODE_E);break;
        case 35:expedition_walk(25.5f,-10);break;
        case 36:expedition_use(27.8f,113.3f,-10);break;
        case 37:expedition_digit(SDL_SCANCODE_1);break;
        case 38:expedition_digit(SDL_SCANCODE_2);break;
        case 39:expedition_digit(SDL_SCANCODE_E);break;
        case 40:expedition_walk(12,-10);break;
        case 41:expedition_use(13.5f,113.3f,-10);break;
        case 42:expedition_digit(SDL_SCANCODE_4);break;
        case 43:expedition_digit(SDL_SCANCODE_E);break;
        case 44:expedition_walk(10,-8.5f);break;
        case 45:expedition_walk(0,-7);break;
        case 46:expedition_walk(-8,-6.8f);break;
        case 47:expedition_use(-9,113.3f,-9.3f);break;
        case 48:expedition_digit(SDL_SCANCODE_4);break;
        case 49:expedition_digit(SDL_SCANCODE_1);break;
        case 50:expedition_digit(SDL_SCANCODE_E);break;
        case 51:expedition_walk(0,-7);break;
        case 52:expedition_walk(6,-8.4f);break;
        case 53:expedition_use(7,113.3f,-10.5f);break;
        case 54:expedition_digit(SDL_SCANCODE_2);break;
        case 55:expedition_digit(SDL_SCANCODE_3);break;
        case 56:expedition_digit(SDL_SCANCODE_4);break;
        case 57:expedition_digit(SDL_SCANCODE_E);break;
        case 58:expedition_walk(0,-7);break;
        case 59:expedition_walk(0,0);break;
        case 60:expedition_digit(SDL_SCANCODE_1);break;
        case 61:if(!meu_sim_lift_moving(s) && s->lift_current==0)expedition_next();break;
        case 62:expedition_walk(0,-6);break;
        case 63:expedition_walk(0,-13);break;
        case 64:expedition_use(0,197.2f,-14.5f);break;
        case 65:
            if(s->robot.z>=-108) {expedition_press(SDL_SCANCODE_P);expedition_next();}
            else {tour_key(SDL_SCANCODE_S,true);}
            break;
        case 66:if(s->travel.parked)expedition_next();break;
        case 67:
            if(s->robot.yaw<=-1.5508) {tour_key(SDL_SCANCODE_A,false);expedition_next();}
            else { tour_key(SDL_SCANCODE_A,true); }
            break;
        case 68:if(s->travel.turn_rate==0)expedition_next();break;
        case 69:
            if(s->robot.x<=-145) {expedition_press(SDL_SCANCODE_P);expedition_next();}
            else { tour_key(SDL_SCANCODE_W,true); }
            break;
        case 70:if(s->travel.parked && s->expedition.completed)expedition_next();break;
        case 71:expedition_digit(SDL_SCANCODE_E);break;
        case 72:expedition_digit(SDL_SCANCODE_ESCAPE);break;
        case 73:expedition_digit(SDL_SCANCODE_F2);break;
        case 74:expedition_digit(SDL_SCANCODE_F5);break;
        case 75: {
            MeuSimulation restored;meu_sim_init(&restored);uint32_t generation=0;
            bool valid=meu_file_load(app.storage_directory,false,&restored,&generation)==MEU_SAVE_OK &&
                restored.expedition.completed && restored.expedition.installed && restored.expedition.repaired &&
                restored.expedition.bypass && restored.expedition.coupler && !restored.expedition.suit &&
                restored.robot.x < -130 && restored.paused && restored.travel.parked;
            printf("EXPEDITION build=%s scene=%s ticks=%" PRIu64 " steps=%u energy=%u alloy=%u conductor=%u sealant=%u save=%u result=%s\n",
                MEU_BUILD_ID,MEU_SCENE_ID,s->ticks,expedition_step,s->power.energy,s->expedition.alloy,
                s->expedition.conductor,s->expedition.sealant,generation,valid?"PASS":"FAIL");
            if(!valid) { app.exit_code=EXIT_FAILURE; }
            app.running=false;break;
        }
    }
}
static void expedition_artifacts(int width,int height)
{
    static unsigned captured;
    const unsigned steps[]={31,37,54,72};
    const char *names[]={"airlock","coupler","workshop","bridge"};
    for(unsigned i=0;i<4;++i)if(expedition_step==steps[i] && !(captured&(1u<<i))) {
        char path[160];snprintf(path,sizeof(path),"test-results/m1-%s.bmp",names[i]);
        bool ok=meu_renderer_capture(path,width,height);
        snprintf(path,sizeof(path),"test-results/m1-%s.meu",names[i]);
        ok=ok && meu_file_export(path,&app.simulation)==MEU_SAVE_OK;
        if(!ok){app.exit_code=EXIT_FAILURE;app.running=false;}
        captured|=1u<<i;
    }
}
