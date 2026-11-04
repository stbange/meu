#include "render/mesh.h"
#include "world/travel_world.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
MeuVertex meu_vertices[180000];
unsigned meu_vertex_count;
static void vertex(float x,float y,float z,float r,float g,float b)
{
    assert(meu_vertex_count<sizeof(meu_vertices)/sizeof(meu_vertices[0]));
    meu_vertices[meu_vertex_count++]=(MeuVertex){{x,y,z},{r,g,b}};
}
void meu_mesh_box(float x,float y,float z,float w,float h,float d,float r,float g,float b)
{
    static const unsigned corners[36]={0,1,2,0,2,3,5,4,7,5,7,6,4,0,3,4,3,7,1,5,6,1,6,2,3,2,6,3,6,7,4,5,1,4,1,0};
    static const float shade[6]={.82f,.72f,.62f,.88f,1,.5f};
    for (unsigned i=0;i<36;++i) {
        unsigned c=corners[i]; float s=shade[i/6];
        vertex(x+((c==1||c==2||c==5||c==6)?w:0),y+((c==2||c==3||c==6||c==7)?h:0),z+(c>=4?d:0),r*s,g*s,b*s);
    }
}
/* Box interiors and coplanar faces must not be submitted twice: even tiny
   floating-point differences otherwise make depth ownership change per frame. */
typedef struct { float u0,v0,u1,v1; } FaceRect;
#define MEU_FACE_PARTS 1024
static FaceRect fragments[2][MEU_FACE_PARTS];
static void append_rect(FaceRect *out,size_t *count,float u0,float v0,float u1,float v1)
{
    if(u1-u0<=.0001f || v1-v0<=.0001f)return;
    if(*count>=MEU_FACE_PARTS)abort();
    out[(*count)++]=(FaceRect){u0,v0,u1,v1};
}
static size_t subtract_rect(const FaceRect *in,size_t count,FaceRect *out,FaceRect mask)
{
    size_t n=0;
    for(size_t i=0;i<count;++i) {
        FaceRect r=in[i];
        float u0=fmaxf(r.u0,mask.u0),v0=fmaxf(r.v0,mask.v0);
        float u1=fminf(r.u1,mask.u1),v1=fminf(r.v1,mask.v1);
        if(u1-u0<=.0001f || v1-v0<=.0001f) { append_rect(out,&n,r.u0,r.v0,r.u1,r.v1);continue; }
        append_rect(out,&n,r.u0,r.v0,u0,r.v1);append_rect(out,&n,u1,r.v0,r.u1,r.v1);
        append_rect(out,&n,u0,r.v0,u1,v0);append_rect(out,&n,u0,v1,u1,r.v1);
    }
    return n;
}
static void solid_face(int axis,int side,float plane,FaceRect r,const MeuBox *b)
{
    static const unsigned forward[6]={0,1,2,0,2,3},reverse[6]={0,3,2,0,2,1};
    static const float shades[3][2]={{.62f,.88f},{.5f,1},{.82f,.72f}};
    const unsigned *corners=side?forward:reverse;
    for(unsigned i=0;i<6;++i) {
        unsigned c=corners[i];float point[3];point[axis]=plane;
        point[(axis+1)%3]=(c==1 || c==2)?r.u1:r.u0;
        point[(axis+2)%3]=c>=2?r.v1:r.v0;
        float shade=shades[axis][side];vertex(point[0],point[1],point[2],b->r*shade,b->g*shade,b->b*shade);
    }
}
void meu_mesh_solids(const MeuBox *boxes,size_t count,size_t first)
{
    const float epsilon=.0001f;
    for(size_t i=first;i<count;++i) {
        const MeuBox *b=&boxes[i];if(b->cylinder || (b->flags&MEU_HIDDEN))continue;
        float lo[3]={b->x,b->y,b->z},hi[3]={b->x+b->w,b->y+b->h,b->z+b->d};
        for(int axis=0;axis<3;++axis)for(int side=0;side<2;++side) {
            int u=(axis+1)%3,v=(axis+2)%3,active=0;size_t n=1;
            float plane=side?hi[axis]:lo[axis];
            fragments[0][0]=(FaceRect){lo[u],lo[v],hi[u],hi[v]};
            for(size_t j=0;j<count && n;++j)if(i!=j && !boxes[j].cylinder && !(boxes[j].flags&MEU_HIDDEN)) {
                const MeuBox *o=&boxes[j];
                float a[3]={o->x,o->y,o->z},z[3]={o->x+o->w,o->y+o->h,o->z+o->d};
                bool buried=side?(a[axis]<=plane+epsilon && z[axis]>plane+epsilon):
                    (a[axis]<plane-epsilon && z[axis]>=plane-epsilon);
                bool coplanar=j<i && fabsf((side?z[axis]:a[axis])-plane)<=epsilon;
                if(!buried && !coplanar)continue;
                n=subtract_rect(fragments[active],n,fragments[1-active],(FaceRect){a[u],a[v],z[u],z[v]});active=1-active;
            }
            for(size_t k=0;k<n;++k)solid_face(axis,side,plane,fragments[active][k],b);
        }
    }
}
void meu_mesh_world(const MeuWorld *world,size_t first)
{
    MeuBox boxes[512];if(world->count>512)abort();
    for(size_t i=0;i<world->count;++i) {
        const MeuWorldBox *b=&world->boxes[i];
        boxes[i]=(MeuBox){(float)b->x,(float)b->y,(float)b->z,(float)b->w,(float)b->h,(float)b->d,b->r,b->g,b->b,0,false};
    }
    meu_mesh_solids(boxes,world->count,first);
}
static MeuBox interior_solids[1500];
static size_t interior_solid_count;
static void solid_box(float x,float y,float z,float w,float h,float d,float r,float g,float b)
{
    if(interior_solid_count>=sizeof(interior_solids)/sizeof(interior_solids[0]))abort();
    interior_solids[interior_solid_count++]=(MeuBox){x,y,z,w,h,d,r,g,b,0,false};
}
static void quad(float x,float y,float z,float w,float h,float r,float g,float b)
{
    vertex(x,y,z,r,g,b); vertex(x+w,y,z,r,g,b); vertex(x+w,y+h,z,r,g,b);
    vertex(x,y,z,r,g,b); vertex(x+w,y+h,z,r,g,b); vertex(x,y+h,z,r,g,b);
}
/* Original 5 x 7 bitmap lettering; no texture, font dependency, or smoothing. */
static const unsigned char glyphs[39][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
 {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{31,4,4,4,4,4,31},
 {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
 {14,17,17,15,1,1,14},{0,0,0,31,0,0,0},{0,4,4,0,4,4,0},{0,0,0,0,0,4,4}
};
void meu_mesh_text(const char *text,float x,float y,float z,float pixel,int direction,float r,float g,float b)
{
    for (;*text;++text,x+=6*pixel*(float)direction) {
        int c=toupper((unsigned char)*text), n=-1;
        if (c>='A' && c<='Z') n=c-'A';
        else if (c>='0' && c<='9') n=26+c-'0';
        else if (c=='-') n=36; else if (c==':') n=37; else if (c=='.') n=38;
        if (n<0) continue;
        for (int row=0;row<7;++row) for (int col=0;col<5;++col)
            if (glyphs[n][row]&(1<<(4-col)))
                quad(x+(float)(col*direction)*pixel,y+(float)(6-row)*pixel,z,pixel*(float)direction,pixel,r,g,b);
    }
}
static void cylinder(const MeuBox *b)
{
    float radius=b->w*.5f,cx=b->x+radius,cz=b->z+radius;
    for (int i=0;i<12;++i) {
        float a=(float)i*.5235988f, next=a+.5235988f;
        float x1=cx+cosf(a)*radius,z1=cz+sinf(a)*radius,x2=cx+cosf(next)*radius,z2=cz+sinf(next)*radius;
        float shade=.65f+.3f*(cosf(a)*.5f+.5f),r=b->r*shade,g=b->g*shade,blue=b->b*shade;
        vertex(x1,b->y,z1,r,g,blue); vertex(x2,b->y,z2,r,g,blue); vertex(x2,b->y+b->h,z2,r,g,blue);
        vertex(x1,b->y,z1,r,g,blue); vertex(x2,b->y+b->h,z2,r,g,blue); vertex(x1,b->y+b->h,z1,r,g,blue);
        vertex(cx,b->y+b->h,cz,b->r,b->g,b->b);
        vertex(x1,b->y+b->h,z1,b->r,b->g,b->b); vertex(x2,b->y+b->h,z2,b->r,b->g,b->b);
    }
}
void meu_mesh_interior(void)
{
    interior_solid_count=0;
    size_t n; const MeuBox *boxes=meu_interior_boxes(&n);
    for (size_t i=0;i<n;++i) {
        const MeuBox *b=&boxes[i];
        if (b->flags&MEU_HIDDEN) continue;
        if (b->cylinder) cylinder(b);
        else solid_box(b->x,b->y,b->z,b->w,b->h,b->d,b->r,b->g,b->b);
    }
    /* These signs face the entering commander; the lift signs face aft. */
    meu_mesh_text("HELM",-.6f,197,-14.49f,.04f,1,.83f,.87f,.63f);
    meu_mesh_text("BRIDGE", -2.5f,200,-22.98f,.12f,1,.72f,.87f,.77f);
    meu_mesh_text("MEU 240 M",3.48f,196.8f,-9.63f,.014f,1,.83f,.72f,.42f);
    meu_mesh_text("LIVING QUARTERS", -3.5f,158.7f,-18.98f,.08f,1,.87f,.76f,.52f);
    meu_mesh_text("ENGINEERING", -4.5f,120,-31.97f,.15f,1,.86f,.74f,.42f);
    meu_mesh_text("REACTOR A", -10.2f,127,-20.55f,.09f,1,.16f,.26f,.26f);
    meu_mesh_text("REACTOR B", 5.8f,127,-20.55f,.09f,1,.16f,.26f,.26f);
    meu_mesh_text("EXPEDITION",-6.9f,197.0f,-10.98f,.025f,1,.83f,.80f,.58f);
    meu_mesh_text("AIRLOCK",11.4f,114.3f,-11.8f,.035f,1,.83f,.80f,.58f);
    meu_mesh_text("PHASE COUPLER",25.8f,113.8f,-11.7f,.025f,1,.83f,.80f,.58f);
    meu_mesh_text("WORKSHOP",6.1f,112.9f,-10.49f,.024f,1,.83f,.80f,.58f);
    meu_mesh_text("POWER",-9.8f,113.0f,-9.0f,.025f,1,.83f,.80f,.58f);
    meu_mesh_text("UNIT DIMENSIONS",3.48f,197.05f,-9.63f,.009f,1,.83f,.80f,.58f);
    for (int i=0;i<3;++i) {
        float y=meu_deck_height(i);
        meu_mesh_text("SERVICE LIFT",1.65f,y+2.82f,-4.08f,.045f,-1,.81f,.76f,.49f);
        meu_mesh_text("CALL",-1.78f,y+1.2f,-3.12f,.012f,1,.10f,.27f,.25f);
        char label[40]; snprintf(label,sizeof(label),"%d - %s - %.0f M",i+1,meu_room_name(i),y);
        meu_mesh_text(label,2.05f,y+1.9f,2.08f,.026f,-1,.78f,.81f,.60f);
    }
    /* The visible machine has feet at 0 m and a crown at 240 m.
       Its forward hull is stepped below every occupied window. */
    solid_box(-24,100,-18,9,83,42,.27f,.34f,.35f);
    solid_box(15,100,-18,9,83,6,.27f,.34f,.35f);
    solid_box(15,100,-8,9,83,32,.27f,.34f,.35f);
    solid_box(15,100,-12,9,12,4,.27f,.34f,.35f);
    solid_box(15,114.8f,-12,9,68.2f,4,.27f,.34f,.35f);
    solid_box(-15,100,5,30,83,19,.27f,.34f,.35f);
    solid_box(-15,100,-18,30,8,23,.27f,.34f,.35f);
    solid_box(-17,184,5,34,56,20,.31f,.38f,.38f);
    solid_box(-19,207,-19,38,33,22,.31f,.38f,.38f);
    solid_box(-26,172,-58,52,11,45,.34f,.40f,.39f);
    /* Roof plates and a human maintenance gallery make the visible hull legible. */
    for (int x=-24;x<=24;x+=6)
        solid_box((float)x,183.02f,-57,.09f,.035f,43,.21f,.28f,.29f);
    for (int z=-55;z<=-13;z+=6)
        solid_box(-25,183.02f,(float)z,50,.035f,.09f,.21f,.28f,.29f);
    solid_box(-1,183.06f,-55,2,.06f,37,.51f,.48f,.34f);
    for (int side=-1;side<=1;side+=2) {
        float x=(float)side*1.05f;
        solid_box(x,184.2f,-55,.06f,.06f,37,.74f,.65f,.37f);
        for (int z=-55;z<=-18;z+=3)
            solid_box(x,183.1f,(float)z,.06f,1.1f,.06f,.74f,.65f,.37f);
        for (int z=-52;z<=-22;z+=10) {
            float vx=side<0?-18:12;
            solid_box(vx,183.05f,(float)z,6,.65f,5,.27f,.34f,.35f);
            for (int j=0;j<6;++j)
                solid_box(vx+.3f,183.71f,(float)z+.3f+(float)j*.7f,5.4f,.04f,.2f,.13f,.22f,.24f);
        }
    }
    solid_box(-14,183,-15,28,13,6,.29f,.36f,.36f);
    solid_box(10.8f,183.1f,-15.02f,1,2.1f,.02f,.56f,.53f,.36f);
    meu_mesh_text("ACCESS",10.84f,185.3f,-15.03f,.023f,1,.82f,.76f,.51f);
    for (int s=-1;s<=1;s+=2) {
        float x=s<0?-56:29;
        solid_box(x,150,-37,27,23,48,.35f,.40f,.37f);
        solid_box(x+4,57,-34,19,93,27,.26f,.33f,.35f);
        solid_box(x+2,45,-47,23,14,39,.38f,.43f,.38f);
        solid_box(s<0?-22:6,8,-5,16,92,22,.27f,.33f,.35f);
        solid_box(s<0?-27:4,0,-25,23,9,48,.32f,.38f,.36f);
        for (int k=0;k<5;++k)
            solid_box(x+3+(float)k*4,173.1f,-34,2.5f,.2f,24,.18f,.31f,.34f);
    }
    /* Cut away the hull along the residence and engineering window routes. */
    /* Opaque hull only begins aft of z=-18; room fronts project past it. */
    meu_mesh_solids(interior_solids,interior_solid_count,0);
}
void meu_mesh_exterior(void)
{
    meu_mesh_world(meu_world_default(),0);
}
static void ground_mark(float x,float z,float w,float d,float r,float g,float b)
{
    vertex(x,0,z,r,g,b);vertex(x+w,0,z,r,g,b);vertex(x+w,0,z+d,r,g,b);
    vertex(x,0,z,r,g,b);vertex(x+w,0,z+d,r,g,b);vertex(x,0,z+d,r,g,b);
}
void meu_mesh_route_markings(void)
{
    /* Paint, not paper-thin boxes. The renderer offsets these decals in depth. */
    for(int z=-25;z>=-975;z-=25) {
        ground_mark(-1,(float)z,2,8,.56f,.63f,.46f);
        ground_mark(-93,(float)z,3,8,.68f,.55f,.32f);
        ground_mark(90,(float)z,3,8,.68f,.55f,.32f);
    }
}
void meu_mesh_lift(const MeuSimulation *s)
{
    float y=s->lift_y;
    meu_mesh_box(-2.2f,y-.3f,-2.2f,4.4f,.3f,4.4f,.33f,.39f,.37f);
    meu_mesh_box(-2.2f,y+2.8f,-2.2f,4.4f,.2f,4.4f,.28f,.35f,.36f);
    for (int side=-1;side<=1;side+=2) {
        float x=(float)side*2.12f;
        meu_mesh_box(x,y,-2.2f,.08f,2.8f,.1f,.65f,.61f,.42f);
        meu_mesh_box(x,y,2.05f,.08f,2.8f,.1f,.65f,.61f,.42f);
        meu_mesh_box(x,y+1.1f,-2.1f,.08f,.08f,4.2f,.83f,.74f,.44f);
        for (int i=0;i<5;++i)
            meu_mesh_box(x,y,-1.7f+(float)i*.8f,.04f,1.1f,.04f,.48f,.57f,.54f);
    }
    meu_mesh_box(-2.1f,y+1.1f,2.1f,4.2f,.08f,.08f,.83f,.74f,.44f);
    if (s->lift_door<.98f) {
        meu_mesh_box(-2.1f,y+1.1f,-2.1f,4.2f,.08f,.08f,.83f,.74f,.44f);
        for (int i=0;i<13;++i)
            meu_mesh_box(-2.1f+(float)i*.35f,y,-2.1f,.035f,2.6f,.06f,.53f,.60f,.53f);
    }
    meu_mesh_box(-.65f,y+2.7f,-1.4f,1.3f,.03f,2.8f,.86f,.83f,.57f);
    for (int deck=0;deck<3;++deck) {
        float floor=meu_deck_height(deck);
        float open=fabsf(y-floor)<.02f?s->lift_door:0;
        /* Sliding mesh gates: clear open doorway, solid closed interlock. */
        for (int side=-1;side<=1;side+=2) {
            float x=side<0?-2.2f-open*2.2f:open*2.2f;
            meu_mesh_box(x,floor,-2.28f,2.2f,.18f,.08f,.72f,.61f,.31f);
            meu_mesh_box(x,floor+2.45f,-2.28f,2.2f,.18f,.08f,.72f,.61f,.31f);
            for (int i=0;i<7;++i)
                meu_mesh_box(x+(float)i*.33f,floor,-2.28f,.04f,2.5f,.08f,.50f,.57f,.51f);
        }
    }
}
void meu_mesh_hud(const MeuSimulation *s,int width,int height,int selection,const char *storage_status)
{
    char line[120]; float size=width<700?1.5f:2;
    const float left=22,top=(float)height-35;
    snprintf(line,sizeof(line),"%s - %.0f M",meu_room_name(meu_sim_room(s)),s->y-MEU_EYE_HEIGHT);
    meu_mesh_text(line,left,top,0,size,1,.86f,.91f,.79f);
    if (s->journeys_open) {
        float x=fmaxf(20,((float)width-760)*.5f),y=fmaxf(50,((float)height-440)*.5f);
        quad(x,y,0,fminf((float)width-40,760),440,.08f,.14f,.15f);
        meu_mesh_text("SAVED JOURNEYS",x+20,y+400,0,2,1,.91f,.81f,.54f);
        const char *items[]={"F5 SAVE JOURNEY - INCLUDES SETTINGS","F9 LOAD LATEST - F8 RECOVER PREVIOUS","F6 EXPORT JOURNEY - F7 IMPORT JOURNEY","PARK AND FINISH THE LIFT RIDE BEFORE SAVING","F2 OR ESC CLOSE - THEN ENTER TO RESUME"};
        for (int i=0;i<5;++i) meu_mesh_text(items[i],x+20,y+350-(float)i*45,0,1.5f,1,.81f,.85f,.74f);
        if (storage_status) {
            char first[61];snprintf(first,sizeof(first),"%.60s",storage_status);
            meu_mesh_text(first,x+20,y+85,0,1.3f,1,.94f,.76f,.43f);
            if (strlen(storage_status)>60) meu_mesh_text(storage_status+60,x+20,y+60,0,1.3f,1,.94f,.76f,.43f);
        }
    } else if (s->settings_open) {
        float w=fminf((float)width-40,680),h=fminf((float)height-40,430);
        float x=((float)width-w)*.5f,y=((float)height-h)*.5f;
        float font=fminf(2,(w-40)/330),row=(h-140)/5;
        quad(x,y,0,w,h,.08f,.14f,.15f);
        meu_mesh_text("SETTINGS",x+20,y+h-32,0,font,1,.91f,.81f,.54f);
        for (int i=0;i<MEU_SETTING_COUNT;++i) {
            float value=meu_settings_value(&s->settings,i);
            char option[100],setting[25];
            if (i==MEU_SETTING_SENSITIVITY) snprintf(setting,sizeof(setting),"%.2f X",value);
            else if (i==MEU_SETTING_VOLUME) snprintf(setting,sizeof(setting),"%.0f PCT",value*100);
            else snprintf(setting,sizeof(setting),"%s",value?"ON":"OFF");
            snprintf(option,sizeof(option),"%s  %s",meu_settings_name(i),setting);
            meu_mesh_text(option,x+20,y+h-75-(float)i*row,0,font,1,
                i==selection?.98f:.76f,i==selection?.79f:.84f,i==selection?.43f:.73f);
        }
        meu_mesh_text("UP DOWN CHOOSE - LEFT RIGHT ADJUST",x+20,y+57,0,font*.7f,1,.73f,.81f,.72f);
        meu_mesh_text("SPACE TOGGLE - R RESET - F1 CLOSE",x+20,y+34,0,font*.7f,1,.73f,.81f,.72f);
        meu_mesh_text("SESSION SETTINGS - CLOSE THEN ENTER TO RESUME",x+20,y+14,0,font*.55f,1,.73f,.81f,.72f);
    } else if (s->station && !s->paused) {
        float w=fminf((float)width-40,960),h=fminf((float)height-90,470);
        float x=((float)width-w)*.5f,y=((float)height-h)*.5f;
        float font=fminf(1.8f,(w-32)/620),row=(h-140)/8;
        quad(x,y,0,w,h,.08f,.14f,.15f);
        meu_mesh_text(meu_sim_station_title(s),x+16,y+h-28,0,font*1.15f,1,.91f,.81f,.54f);
        char text[256];meu_sim_station_status(s,text,sizeof(text));
        meu_mesh_text(text,x+16,y+h-55,0,font*.85f,1,.81f,.87f,.76f);
        for (int i=0;i<8;++i) {
            char option[220];meu_sim_station_line(s,i,text,sizeof(text));
            snprintf(option,sizeof(option),"%d %.*s",i+1,200,text);
            if (*text) meu_mesh_text(option,x+16,y+h-85-(float)i*row,0,font,1,.82f,.87f,.75f);
        }
        meu_mesh_text(s->message,x+16,y+35,0,font*.8f,1,.94f,.76f,.43f);
        meu_mesh_text("0 OR E CLOSE - GAME TIME CONTINUES",x+16,y+13,0,font*.8f,1,.73f,.81f,.72f);
    } else if (s->paused) {
        meu_mesh_text("ENTER EXPLORE - F1 SETTINGS - F2 JOURNEYS",left,top-25,0,size,1,.94f,.76f,.43f);
    } else if (s->travel.helm) {
        snprintf(line,sizeof(line),"HELM %.1f M/S - HEADING %.0f - X %.0f Z %.0f",s->travel.speed,s->robot.yaw*57.295779513,s->robot.x,s->robot.z);
        meu_mesh_text(line,left,top-25,0,size,1,.94f,.76f,.43f);
        const char *status=s->travel.reason?meu_route_name(s->travel.reason):
            s->travel.parked?"PARKED - SUPPORT VERIFIED":s->travel.park_requested?"BRAKING TO PARK":"MANUAL TRAVEL";
        meu_mesh_text(status,left,top-50,0,size,1,.94f,.76f,.43f);
    } else if (meu_sim_on_lift(s)) {
        const char *lift=meu_sim_lift_moving(s)?"IN TRANSIT - GATES LOCKED":
            meu_sim_lift_ready(s)?"1 BRIDGE   2 QUARTERS   3 ENGINEERING":"STEP INTO CENTRE TO CHOOSE A DECK";
        meu_mesh_text(lift,left,top-25,0,size,1,.94f,.76f,.43f);
    } else {
        MeuUse use=meu_sim_use_target(s);
        if (use) { snprintf(line,sizeof(line),"E - %s",meu_sim_use_name(use)); meu_mesh_text(line,left,top-25,0,size,1,.94f,.76f,.43f); }
    }
    if(!s->station && !s->paused) {
        meu_mesh_text(meu_sim_objective(s),left,top-105,0,size*.65f,1,.86f,.88f,.70f);
    }
    if(s->expedition.suit && !s->station) {
        char suit[160];meu_sim_station_status(s,suit,sizeof(suit));
        meu_mesh_text(suit,left,top-78,0,size*.8f,1,.94f,.76f,.43f);
    }
    if (s->info_open) {
        meu_mesh_text("UNIT HEIGHT 240 M - BRIDGE 196 M",left,top-55,0,size,1,.94f,.82f,.57f);
        meu_mesh_text("QUARTERS 156 M - ENGINEERING 112 M",left,top-78,0,size,1,.94f,.82f,.57f);
    }
    meu_mesh_text(s->travel.helm?"WS DRIVE - AD TURN - SPACE BRAKE - P PARK - E RELEASE":"WASD WALK - CLICK CAPTURES MOUSE - E USE - ESC RELEASES - F2 JOURNEYS",left,18,0,size*.75f,1,.68f,.77f,.69f);
    quad((float)width*.5f-3,(float)height*.5f-.6f,0,6,1.2f,.80f,.87f,.73f);
    quad((float)width*.5f-.6f,(float)height*.5f-3,0,1.2f,6,.80f,.87f,.73f);
}
