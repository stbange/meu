#ifndef MEU_MESH_H
#define MEU_MESH_H
#include "core/simulation.h"
typedef struct { float position[3],color[3]; } MeuVertex;
extern MeuVertex meu_vertices[180000];
extern unsigned meu_vertex_count;
void meu_mesh_box(float x,float y,float z,float w,float h,float d,float r,float g,float b);
void meu_mesh_text(const char *text,float x,float y,float z,float pixel,int direction,float r,float g,float b);
void meu_mesh_interior(void);
/* Opaque box union: emit exposed faces, assigning coplanar overlap to the
   earliest box. first permits an additive mesh without redrawing base faces. */
void meu_mesh_solids(const MeuBox *boxes,size_t count,size_t first);
void meu_mesh_world(const MeuWorld *world,size_t first);
void meu_mesh_exterior(void);
void meu_mesh_route_markings(void);
void meu_mesh_lift(const MeuSimulation *state);
void meu_mesh_hud(const MeuSimulation *state,int width,int height,int selection,const char *storage_status);
/* Native pointer coordinates are pixels from the top-left; -1 means no control. */
int meu_mesh_help_hit(int width,int height,float x,float y,bool open);
#endif
