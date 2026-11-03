#include "render/mesh.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
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
    size_t n; const MeuBox *boxes=meu_interior_boxes(&n);
    for (size_t i=0;i<n;++i) {
        const MeuBox *b=&boxes[i];
        if (b->flags&MEU_HIDDEN) continue;
        if (b->cylinder) cylinder(b);
        else meu_mesh_box(b->x,b->y,b->z,b->w,b->h,b->d,b->r,b->g,b->b);
    }
    /* These signs face the entering commander; the lift signs face aft. */
    meu_mesh_text("BRIDGE", -2.5f,200,-22.98f,.12f,1,.72f,.87f,.77f);
    meu_mesh_text("MEU 240 M",3.48f,196.8f,-9.63f,.014f,1,.83f,.72f,.42f);
    meu_mesh_text("LIVING QUARTERS", -3.5f,158.7f,-18.98f,.08f,1,.87f,.76f,.52f);
    meu_mesh_text("ENGINEERING", -4.5f,120,-31.97f,.15f,1,.86f,.74f,.42f);
    meu_mesh_text("REACTOR A", -10.2f,127,-20.55f,.09f,1,.16f,.26f,.26f);
    meu_mesh_text("REACTOR B", 5.8f,127,-20.55f,.09f,1,.16f,.26f,.26f);
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
    meu_mesh_box(-24,100,-18,9,83,42,.27f,.34f,.35f);
    meu_mesh_box(15,100,-18,9,83,42,.27f,.34f,.35f);
    meu_mesh_box(-15,100,5,30,83,19,.27f,.34f,.35f);
    meu_mesh_box(-15,100,-18,30,8,23,.27f,.34f,.35f);
    meu_mesh_box(-17,184,5,34,56,20,.31f,.38f,.38f);
    meu_mesh_box(-19,207,-19,38,33,22,.31f,.38f,.38f);
    meu_mesh_box(-26,172,-58,52,11,45,.34f,.40f,.39f);
    /* Roof plates and a human maintenance gallery make the visible hull legible. */
    for (int x=-24;x<=24;x+=6)
        meu_mesh_box((float)x,183.02f,-57,.09f,.035f,43,.21f,.28f,.29f);
    for (int z=-55;z<=-13;z+=6)
        meu_mesh_box(-25,183.02f,(float)z,50,.035f,.09f,.21f,.28f,.29f);
    meu_mesh_box(-1,183.06f,-55,2,.06f,37,.51f,.48f,.34f);
    for (int side=-1;side<=1;side+=2) {
        float x=(float)side*1.05f;
        meu_mesh_box(x,184.2f,-55,.06f,.06f,37,.74f,.65f,.37f);
        for (int z=-55;z<=-18;z+=3)
            meu_mesh_box(x,183.1f,(float)z,.06f,1.1f,.06f,.74f,.65f,.37f);
        for (int z=-52;z<=-22;z+=10) {
            float vx=side<0?-18:12;
            meu_mesh_box(vx,183.05f,(float)z,6,.65f,5,.27f,.34f,.35f);
            for (int j=0;j<6;++j)
                meu_mesh_box(vx+.3f,183.71f,(float)z+.3f+(float)j*.7f,5.4f,.04f,.2f,.13f,.22f,.24f);
        }
    }
    meu_mesh_box(-14,183,-15,28,13,6,.29f,.36f,.36f);
    meu_mesh_box(10.8f,183.1f,-15.02f,1,2.1f,.02f,.56f,.53f,.36f);
    meu_mesh_text("ACCESS",10.84f,185.3f,-15.03f,.023f,1,.82f,.76f,.51f);
    for (int s=-1;s<=1;s+=2) {
        float x=s<0?-56:29;
        meu_mesh_box(x,150,-37,27,23,48,.35f,.40f,.37f);
        meu_mesh_box(x+4,57,-34,19,93,27,.26f,.33f,.35f);
        meu_mesh_box(x+2,45,-47,23,14,39,.38f,.43f,.38f);
        meu_mesh_box(s<0?-22:6,8,-5,16,92,22,.27f,.33f,.35f);
        meu_mesh_box(s<0?-27:4,0,-25,23,9,48,.32f,.38f,.36f);
        for (int k=0;k<5;++k)
            meu_mesh_box(x+3+(float)k*4,173.1f,-34,2.5f,.2f,24,.18f,.31f,.34f);
    }
    /* Cut away the hull along the residence and engineering window routes. */
    /* Opaque hull only begins aft of z=-18; room fronts project past it. */
}
void meu_mesh_exterior(void)
{
    /* M0b scale fixture, deliberately authored rather than the later seeded city. */
    meu_mesh_box(-95,-8,-1000,190,8,1100,.25f,.30f,.28f);
    meu_mesh_box(-220,-8,-750,440,8,300,.29f,.33f,.29f);
    meu_mesh_box(-1000,-8,-1400,830,8,1000,.25f,.30f,.28f);
    meu_mesh_box(170,-8,-1400,830,8,1000,.25f,.30f,.28f);
    /* A canyon cleft runs under the monumental platform. */
    meu_mesh_box(-1100,-200,-1500,750,192,1700,.24f,.27f,.25f);
    meu_mesh_box(350,-260,-1500,750,252,1700,.26f,.28f,.25f);
    meu_mesh_box(-350,-260,-1500,700,5,1700,.10f,.18f,.20f);
    for (int i=0;i<14;++i) {
        float x=(float)(i-7)*105,z=-360-(float)(i%3)*240,h=300+(float)((i*7)%6)*85;
        if (x>-130 && x<230) continue;
        meu_mesh_box(x,0,z,40,h,65,.38f,.46f,.41f);
        meu_mesh_box(x-15,h-45,z-15,70,22,95,.53f,.57f,.46f);
        meu_mesh_box(x+7,80,z+65,3,h*.4f,.6f,.39f,.72f,.63f);
        for (int j=0;j<4;++j)
            meu_mesh_box(x-8,50+(float)j*70,z+65,56,4,10,.46f,.52f,.43f);
    }
    /* 540 m alien doorway; the 240 m unit could walk through it. */
    meu_mesh_box(-155,0,-620,50,540,70,.48f,.54f,.45f);
    meu_mesh_box(105,0,-620,50,540,70,.48f,.54f,.45f);
    meu_mesh_box(-155,490,-620,310,50,70,.53f,.59f,.48f);
    meu_mesh_box(-175,540,-640,350,12,110,.37f,.46f,.41f);
    for (int i=0;i<8;++i) {
        float y=35+(float)i*57;
        meu_mesh_box(-155,y,-548,50,5,1,.35f,.71f,.61f);
        meu_mesh_box(105,y,-548,50,5,1,.35f,.71f,.61f);
    }
    meu_mesh_box(-600,240,-950,1200,15,80,.40f,.48f,.42f);
    meu_mesh_box(-440,0,-1040,30,610,30,.40f,.48f,.42f);
    meu_mesh_box(410,0,-1040,30,610,30,.40f,.48f,.42f);
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
void meu_mesh_hud(const MeuSimulation *s,int width,int height)
{
    char line[120]; float size=width<700?1.5f:2;
    const float left=22,top=(float)height-35;
    snprintf(line,sizeof(line),"%s - %.0f M",meu_room_name(meu_sim_room(s)),s->y-MEU_EYE_HEIGHT);
    meu_mesh_text(line,left,top,0,size,1,.86f,.91f,.79f);
    if (s->paused) {
        meu_mesh_text("ENTER TO EXPLORE",left,top-25,0,size,1,.94f,.76f,.43f);
    } else if (meu_sim_on_lift(s)) {
        meu_mesh_text(meu_sim_lift_moving(s)?"IN TRANSIT - GATES LOCKED":"1 BRIDGE   2 QUARTERS   3 ENGINEERING",left,top-25,0,size,1,.94f,.76f,.43f);
    } else {
        MeuUse use=meu_sim_use_target(s);
        if (use) { snprintf(line,sizeof(line),"E - %s",meu_sim_use_name(use)); meu_mesh_text(line,left,top-25,0,size,1,.94f,.76f,.43f); }
    }
    if (s->info_open) {
        meu_mesh_text("UNIT HEIGHT 240 M - BRIDGE 196 M",left,top-55,0,size,1,.94f,.82f,.57f);
        meu_mesh_text("QUARTERS 156 M - ENGINEERING 112 M",left,top-78,0,size,1,.94f,.82f,.57f);
    }
    meu_mesh_text("WASD WALK - DRAG OR ARROWS LOOK - E USE - ESC PAUSE",left,18,0,size*.75f,1,.68f,.77f,.69f);
    quad((float)width*.5f-3,(float)height*.5f-.6f,0,6,1.2f,.80f,.87f,.73f);
    quad((float)width*.5f-.6f,(float)height*.5f-3,0,1.2f,6,.80f,.87f,.73f);
}
