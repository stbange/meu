#ifndef MEU_GUIDE_H
#define MEU_GUIDE_H
#include "core/simulation.h"
#define MEU_GUIDE_TOPICS 7
typedef enum {
    MEU_GUIDE_WELCOME,
    MEU_GUIDE_RECOVER,
    MEU_GUIDE_RETURN_NOW,
    MEU_GUIDE_RETURN,
    MEU_GUIDE_UNSUIT,
    MEU_GUIDE_COMPLETE,
    MEU_GUIDE_CHARGE,
    MEU_GUIDE_COOL,
    MEU_GUIDE_DOCK,
    MEU_GUIDE_ISOLATE,
    MEU_GUIDE_CONNECT,
    MEU_GUIDE_PULSE,
    MEU_GUIDE_DIAGNOSE,
    MEU_GUIDE_PARTS,
    MEU_GUIDE_QUEUE,
    MEU_GUIDE_REPAIR,
    MEU_GUIDE_BULK,
    MEU_GUIDE_SCAN,
    MEU_GUIDE_SHELTER,
    MEU_GUIDE_SUIT,
    MEU_GUIDE_CONDUIT,
    MEU_GUIDE_BYPASS,
    MEU_GUIDE_EXTRACT,
    MEU_GUIDE_TEST,
    MEU_GUIDE_PRIORITY,
    MEU_GUIDE_ADAPTER,
    MEU_GUIDE_DISCONNECT,
    MEU_GUIDE_INSTALL,
    MEU_GUIDE_PACK,
    MEU_GUIDE_DEPART,
    MEU_GUIDE_STEP_COUNT
} MeuGuideStep;
MeuGuideStep meu_guide_step(const MeuSimulation *state);
const char *meu_guide_topic(int topic);
/* field: 0 heading, 1 place, 2 action/explanation, 3 reason/tip. */
const char *meu_guide_text(const MeuSimulation *state,int topic,int field);
#endif
