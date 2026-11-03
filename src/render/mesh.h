#ifndef MEU_MESH_H
#define MEU_MESH_H
#include "core/simulation.h"
typedef struct { float position[3],color[3]; } MeuVertex;
extern MeuVertex meu_vertices[180000];
extern unsigned meu_vertex_count;
void meu_mesh_box(float x,float y,float z,float w,float h,float d,float r,float g,float b);
void meu_mesh_text(const char *text,float x,float y,float z,float pixel,int direction,float r,float g,float b);
void meu_mesh_interior(void);
void meu_mesh_exterior(void);
void meu_mesh_lift(const MeuSimulation *state);
void meu_mesh_hud(const MeuSimulation *state,int width,int height,int selection,const char *storage_status);
#endif
