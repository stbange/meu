#include "game/interior.h"
#include <assert.h>
#include <math.h>
static MeuBox boxes[1200];
static size_t count;
static void box(float x,float y,float z,float w,float h,float d,float r,float g,float b,unsigned flags)
{
    assert(count < sizeof(boxes)/sizeof(boxes[0]));
    boxes[count++] = (MeuBox){x,y,z,w,h,d,r,g,b,flags,false};
}
static void cylinder(float x,float y,float z,float radius,float h,float r,float g,float b)
{
    box(x-radius,y,z-radius,radius*2,h,radius*2,r,g,b,MEU_SOLID);
    boxes[count-1].cylinder = true;
}
float meu_deck_height(int deck)
{
    static const float heights[3] = {196,156,112};
    return heights[deck >= 0 && deck < 3 ? deck : 0];
}
const char *meu_room_name(int room)
{
    static const char *names[] = {"Bridge", "Living quarters", "Engineering hall", "Service lift"};
    return names[room >= 0 && room < 4 ? room : 0];
}
static void shell(int deck,float half,float front,float ceiling,float r,float g,float b)
{
    float y = meu_deck_height(deck), depth = -4-front;
    box(-half,y-.35f,front,half*2,.35f,depth,r,g,b,MEU_FLOOR|MEU_SOLID);
    box(-half-.3f,y,front,.3f,ceiling,depth,.24f,.30f,.32f,MEU_SOLID);
    box(half,y,front,.3f,ceiling,depth,.24f,.30f,.32f,MEU_SOLID);
    box(-half,y+ceiling,front,half*2,.3f,depth,.15f,.20f,.23f,MEU_SOLID);
    box(-half,y,-4,half-2,ceiling,.35f,.25f,.31f,.32f,MEU_SOLID);
    box(2,y,-4,half-2,ceiling,.35f,.25f,.31f,.32f,MEU_SOLID);
    box(-2,y+2.65f,-4,4,ceiling-2.65f,.35f,.25f,.31f,.32f,MEU_SOLID);
    box(-2.2f,y-.3f,-4,4.4f,.3f,2.2f,r,g,b,MEU_FLOOR|MEU_SOLID);
    box(-2.5f,y,-4,.3f,3.2f,1.8f,.22f,.28f,.30f,MEU_SOLID);
    box(2.2f,y,-4,.3f,3.2f,1.8f,.22f,.28f,.30f,MEU_SOLID);
    box(-2.2f,y+3.2f,-4,4.4f,.2f,1.8f,.22f,.28f,.30f,MEU_SOLID);
    /* Window collision uses the same boundary as its visible frame. */
    box(-half,y,front-.3f,half*2,.75f,.3f,.30f,.37f,.37f,MEU_SOLID);
    box(-half,y+.75f,front-.25f,half*2,ceiling-.75f,.2f,0,0,0,MEU_SOLID|MEU_HIDDEN);
    box(-half,y+ceiling-1,front-.3f,half*2,1,.3f,.30f,.37f,.37f,MEU_SOLID);
    for (int i = -1; i <= 1; ++i)
        box((float)i*(half-.25f)-.1f,y+.75f,front-.3f,.2f,ceiling-1.75f,.4f,.38f,.47f,.45f,MEU_SOLID);
    for (float z = front+1; z < -4; z += 2) {
        box(-half+.1f,y+.005f,z,half*2-.2f,.01f,.025f,.32f,.39f,.39f,0);
        box(-half+.05f,y+2.7f,z,.1f,.12f,.9f,.87f,.78f,.53f,0);
        box(half-.15f,y+2.7f,z,.1f,.12f,.9f,.87f,.78f,.53f,0);
    }
    for (float x = -half+2; x < half; x += 2)
        box(x,y+.005f,front,.025f,.01f,depth,.30f,.37f,.37f,0);
    box(-1.8f,y+1,-3.4f,.3f,.45f,.25f,.15f,.69f,.62f,MEU_SOLID);
    box(-2,y+2.35f,-4.02f,4,.2f,.15f,.88f,.67f,.30f,0);
}
static void build(void)
{
    if (count) return;
    shell(0,10,-23,7,.23f,.30f,.32f);
    shell(1,8,-19,3.8f,.29f,.28f,.24f);
    shell(2,14,-32,24,.20f,.25f,.27f);
    /* Human-sized bridge stations leave a clear central route to the window. */
    for (int side = -1; side <= 1; side += 2) {
        float x = side < 0 ? -8 : 3.5f;
        box(x,196,-19,4.5f,1.05f,1.2f,.25f,.33f,.34f,MEU_SOLID);
        box(x+.15f,197.06f,-18.9f,4.2f,.05f,.9f,.18f,.72f,.65f,0);
        box(x+1.4f,196,-16.8f,.8f,.5f,.8f,.42f,.31f,.20f,MEU_SOLID);
        box(x+1.4f,196.5f,-16.2f,.8f,.8f,.15f,.42f,.31f,.20f,MEU_SOLID);
        box(side < 0 ? -7 : 5,196,-12,2,1.05f,1,.28f,.34f,.33f,MEU_SOLID);
    }
    box(-1.3f,196,-15.5f,2.6f,1.1f,1,.23f,.34f,.35f,MEU_SOLID);
    box(-1.15f,197.11f,-15.4f,2.3f,.05f,.8f,.22f,.77f,.67f,0);
    box(3.4f,196,-10,.9f,1.4f,.35f,.34f,.41f,.38f,MEU_SOLID);
    box(3.5f,197.42f,-10,.7f,.03f,.35f,.90f,.76f,.43f,0);
    /* Residence: 2 m bed, table, storage, reading light, and a plant. */
    box(-6.7f,156,-15.8f,1.7f,.55f,2.2f,.36f,.28f,.22f,MEU_SOLID);
    box(-6.65f,156.55f,-15.75f,1.6f,.22f,2.1f,.66f,.57f,.39f,0);
    box(-6.55f,156.78f,-15.65f,1.4f,.15f,.45f,.81f,.77f,.61f,0);
    box(-6.8f,156,-17.9f,1.9f,2.1f,.65f,.40f,.34f,.27f,MEU_SOLID);
    box(4.8f,156,-14,2.4f,.8f,1,.46f,.37f,.25f,MEU_SOLID);
    box(5.5f,156,-12.3f,.8f,.5f,.8f,.49f,.36f,.22f,MEU_SOLID);
    box(5.5f,156.5f,-11.65f,.8f,.7f,.15f,.49f,.36f,.22f,MEU_SOLID);
    box(-3,156.02f,-14,6,.01f,6,.35f,.39f,.32f,0);
    box(5.9f,156,-7.7f,1.3f,.6f,1,.39f,.33f,.25f,MEU_SOLID);
    cylinder(6.55f,156.6f,-7.25f,.12f,.65f,.62f,.53f,.36f);
    cylinder(6.55f,157.25f,-7.25f,.35f,.3f,.95f,.77f,.42f);
    cylinder(-6.5f,156,-7.5f,.45f,.7f,.48f,.34f,.24f);
    cylinder(-6.5f,156.7f,-7.5f,.6f,1.1f,.30f,.46f,.32f);
    /* Engineering reactors dwarf the 1.1 m railings and the commander. */
    for (int s = -1; s <= 1; s += 2) {
        cylinder((float)s*8,112,-24,3.2f,18,.34f,.40f,.39f);
        for (int n = 0; n < 6; ++n)
            cylinder((float)s*8,113+(float)n*3,-24,3.35f,.25f,.58f,.64f,.50f);
        box((float)s*11-.3f,112,-30,.6f,22,25,.35f,.42f,.42f,MEU_SOLID);
        box((float)s*4.5f-.3f,112,-29,.6f,20,.7f,.45f,.52f,.48f,MEU_SOLID);
    }
    box(-3.1f,112,-27,6.2f,1.2f,9,.26f,.32f,.33f,MEU_FLOOR|MEU_SOLID);
    for (int i = 0; i < 30; ++i) {
        float z = -12-(float)(i+1)*.2f;
        box(-3.1f,112,z,6.2f,(float)(i+1)*.04f,.2f,.31f,.38f,.37f,0);
    }
    for (int s = -1; s <= 1; s += 2) {
        for (int i = 0; i < 6; ++i)
            box((float)s*3.05f-.04f,113.2f,-27+(float)i*1.8f,.08f,1.1f,.08f,.79f,.65f,.32f,MEU_SOLID);
        box((float)s*3.05f-.04f,114.2f,-27,.08f,.08f,9,.79f,.65f,.32f,MEU_SOLID);
    }
    box(-1,113.2f,-26.8f,2,1.05f,.6f,.28f,.36f,.37f,MEU_SOLID);
    box(-.9f,114.26f,-26.75f,1.8f,.04f,.5f,.83f,.65f,.28f,0);
    box(-13,132,-22,26,1.2f,1.4f,.56f,.47f,.29f,0);
    box(-.4f,121,-22,.8f,11,.8f,.44f,.47f,.42f,0);
    /* Human-scale salvage/test bench, clear of the reactor access lane. */
    box(6,112,-11.5f,2,1.1f,1,.30f,.39f,.38f,MEU_SOLID);
    box(6.1f,113.12f,-11.4f,1.8f,.04f,.8f,.23f,.72f,.65f,0);
    /* A docked maintenance robot, visibly mechanical, with no human crew. */
    box(-10,112,-10,1.5f,.6f,2,.18f,.24f,.27f,MEU_SOLID);
    box(-9.8f,112.6f,-9.7f,1.1f,.65f,1.3f,.70f,.52f,.23f,MEU_SOLID);
    box(-9.5f,113.25f,-9.8f,.5f,.25f,.4f,.22f,.74f,.68f,0);
    /* 100 m open service shaft: ribs and illuminated landings pass the cab. */
    box(-2.8f,108,-2.5f,.4f,103,5.2f,.16f,.22f,.25f,MEU_SOLID);
    box(2.4f,108,-2.5f,.4f,103,5.2f,.16f,.22f,.25f,MEU_SOLID);
    box(-2.4f,108,2.3f,4.8f,103,.4f,.14f,.20f,.24f,MEU_SOLID);
    box(-2.4f,136,-4.4f,4.8f,20,.3f,.18f,.25f,.28f,MEU_SOLID);
    box(-2.4f,160,-4.4f,4.8f,36,.3f,.18f,.25f,.28f,MEU_SOLID);
    for (int y = 110; y <= 208; y += 4) {
        box(-2.35f,(float)y,2.1f,4.7f,.18f,.2f,.44f,.51f,.49f,0);
        box(-2.4f,(float)y,-2.3f,.15f,.15f,4.5f,.37f,.46f,.44f,0);
        box(2.25f,(float)y,-2.3f,.15f,.15f,4.5f,.37f,.46f,.44f,0);
        box(-2.3f,(float)y,-4.05f,4.6f,.12f,.12f,.45f,.53f,.49f,0);
    }
}
const MeuBox *meu_interior_boxes(size_t *n) { build(); *n = count; return boxes; }
static float clamp(float v,float a,float b) { return fmaxf(a,fminf(b,v)); }
static bool overlap(const MeuBox *b,float x,float feet,float z)
{
    if (feet >= b->y+b->h-.015f || feet+MEU_PLAYER_HEIGHT <= b->y+.01f) return false;
    /* The capsule toe overlaps the platform before its centre leaves the ramp. */
    if ((b->flags&MEU_FLOOR) && b->y+b->h-feet<=.08f) return false;
    if (b->cylinder) {
        float dx=x-(b->x+b->w*.5f), dz=z-(b->z+b->d*.5f), r=b->w*.5f+MEU_PLAYER_RADIUS;
        return dx*dx+dz*dz < r*r;
    }
    float dx=x-clamp(x,b->x,b->x+b->w), dz=z-clamp(z,b->z,b->z+b->d);
    return dx*dx+dz*dz < MEU_PLAYER_RADIUS*MEU_PLAYER_RADIUS;
}
bool meu_interior_floor(float x,float z,float feet,float lift_y,float *height)
{
    build(); bool found=false; float best=-10000;
    for (size_t i=0; i<count; ++i) {
        const MeuBox *b=&boxes[i]; float top=b->y+b->h;
        if ((b->flags&MEU_FLOOR) && x>=b->x && x<=b->x+b->w && z>=b->z && z<=b->z+b->d &&
            top<=feet+.29f && top>=feet-.4f && top>best) { best=top; found=true; }
    }
    if (fabsf(x)<=3.1f && z>=-18 && z<=-12) {
        float top=112+(-z-12)*.2f;
        if (fabsf(top-feet)<=.4f && top>best) { best=top; found=true; }
    }
    if (fabsf(x)<=2.2f && fabsf(z)<=2.2f && fabsf(lift_y-feet)<=.3f && lift_y>best) {
        best=lift_y; found=true;
    }
    if (found) *height=best;
    return found;
}
bool meu_interior_blocked(float x,float feet,float z,float lift_y,float door)
{
    build();
    for (size_t i=0; i<count; ++i) if ((boxes[i].flags&MEU_SOLID) && overlap(&boxes[i],x,feet,z)) return true;
    for (int i=0;i<3;++i) {
        float y=meu_deck_height(i);
        if (fabsf(lift_y-y)>.02f || door<.98f) {
            MeuBox gate={-2.2f,y,-2.3f,4.4f,2.7f,.12f,0,0,0,MEU_SOLID,false};
            if (overlap(&gate,x,feet,z)) return true;
        }
    }
    if (fabsf(feet-lift_y)<.1f && z>-2.2f && z<2.2f) {
        if (fabsf(x)>1.8f || z>1.8f || (door<.98f && z<-1.8f)) return true;
    }
    return false;
}
bool meu_interior_clear_ray(float x,float y,float z,float tx,float ty,float tz)
{
    build(); float origin[3]={x,y,z}, dir[3]={tx-x,ty-y,tz-z};
    for (size_t i=0;i<count;++i) if (boxes[i].flags&MEU_SOLID) {
        const MeuBox *b=&boxes[i]; float lo[3]={b->x,b->y,b->z}, hi[3]={b->x+b->w,b->y+b->h,b->z+b->d};
        if (tx>=lo[0] && tx<=hi[0] && ty>=lo[1] && ty<=hi[1] && tz>=lo[2] && tz<=hi[2]) continue;
        float near=0,far=1;
        for (int k=0;k<3;++k) {
            if (fabsf(dir[k])<1e-6f) { if (origin[k]<lo[k] || origin[k]>hi[k]) far=-1; }
            else {
                float a=(lo[k]-origin[k])/dir[k], c=(hi[k]-origin[k])/dir[k];
                if (a>c) { float swap=a; a=c; c=swap; }
                near=fmaxf(near,a); far=fminf(far,c);
            }
        }
        if (near<=far && far>0 && near<.999f) return false;
    }
    return true;
}
