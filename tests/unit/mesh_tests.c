#include "render/mesh.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#c);return EXIT_FAILURE; } } while(0)
typedef struct { int axis;float plane,lo[3],hi[3]; } Quad;
static Quad quads[30000];
static size_t read_quads(void)
{
    size_t count=0;
    for(unsigned v=0;v+5<meu_vertex_count;v+=6) {
        Quad q={0};for(int a=0;a<3;++a){q.lo[a]=FLT_MAX;q.hi[a]=-FLT_MAX;}
        for(unsigned i=0;i<6;++i)for(int a=0;a<3;++a) {
            float p=meu_vertices[v+i].position[a];q.lo[a]=fminf(q.lo[a],p);q.hi[a]=fmaxf(q.hi[a],p);
        }
        for(int a=0;a<3;++a)if(q.hi[a]-q.lo[a]<.0001f){q.axis=a;q.plane=q.lo[a];break;}
        quads[count++]=q;
    }
    return count;
}
static bool unique_faces(void)
{
    size_t count=read_quads();
    for(size_t i=0;i<count;++i)for(size_t j=0;j<i;++j) {
        Quad a=quads[i],b=quads[j];if(a.axis!=b.axis || fabsf(a.plane-b.plane)>.0001f)continue;
        int u=(a.axis+1)%3,v=(a.axis+2)%3;
        if(fminf(a.hi[u],b.hi[u])-fmaxf(a.lo[u],b.lo[u])>.0001f &&
           fminf(a.hi[v],b.hi[v])-fmaxf(a.lo[v],b.lo[v])>.0001f)return false;
    }
    return true;
}
static float top_area(float plane)
{
    size_t count=read_quads();float area=0;
    for(size_t i=0;i<count;++i)if(quads[i].axis==1 && fabsf(quads[i].plane-plane)<.0001f)
        area+=(quads[i].hi[0]-quads[i].lo[0])*(quads[i].hi[2]-quads[i].lo[2]);
    return area;
}
int main(void)
{
    MeuBox boxes[3]={{0,-1,0,4,1,4,1,0,0,0,false},{0,-1,0,4,1,4,0,1,0,0,false}};
    meu_vertex_count=0;meu_mesh_solids(boxes,2,0);CHECK(meu_vertex_count==36 && unique_faces());
    CHECK(top_area(0)==16);
    for(unsigned i=0;i<meu_vertex_count;++i)CHECK(meu_vertices[i].color[1]==0);
    boxes[1].x=2;boxes[1].z=2;
    meu_vertex_count=0;meu_mesh_solids(boxes,2,0);CHECK(unique_faces() && top_area(0)==28);
    boxes[1].x=4;boxes[1].z=0;
    meu_vertex_count=0;meu_mesh_solids(boxes,2,0);CHECK(unique_faces() && top_area(0)==32);
    for(unsigned i=0;i<meu_vertex_count;i+=6) {
        bool internal=true;for(unsigned j=0;j<6;++j)internal&=meu_vertices[i+j].position[0]==4;
        CHECK(!internal);
    }
    boxes[1]=(MeuBox){1,-.8f,1,2,.5f,2,0,1,0,0,false};
    meu_vertex_count=0;meu_mesh_solids(boxes,2,0);CHECK(meu_vertex_count==36 && unique_faces());
    meu_vertex_count=0;meu_mesh_world(meu_world_default(),0);CHECK(unique_faces());
    unsigned base=meu_vertex_count;
    meu_mesh_world(meu_world_with_bridge(),meu_world_default()->count);CHECK(unique_faces());
    CHECK(meu_vertex_count>base && meu_vertex_count<180000);
    meu_vertex_count=0;meu_mesh_interior();CHECK(meu_vertex_count>0 && meu_vertex_count<180000);
    meu_vertex_count=0;meu_mesh_route_markings();
    for(unsigned i=0;i<meu_vertex_count;++i)CHECK(meu_vertices[i].position[1]==0);
    puts("PASS unique coplanar surfaces, union area, hidden/shared faces, additive bridge and flat decals");return EXIT_SUCCESS;
}
