#include "world/travel_world.h"
#include <math.h>
#include <stdlib.h>
static MeuWorldBox boxes[160];
static MeuWorld authored;
static void add(double x,double y,double z,double w,double h,double d,float r,float g,float b)
{
    if (authored.count>=sizeof(boxes)/sizeof(boxes[0])) abort();
    boxes[authored.count++]=(MeuWorldBox){x,y,z,w,h,d,r,g,b,false};
}
const MeuWorld *meu_world_default(void)
{
    if (authored.count) return &authored;
    authored.boxes=boxes;
    add(-95,-8,-1000,190,8,1100,.25f,.30f,.28f);
    add(-220,-8,-750,440,8,300,.29f,.33f,.29f);
    add(-1000,-8,-1400,830,8,1000,.25f,.30f,.28f);
    add(170,-8,-1400,830,8,1000,.25f,.30f,.28f);
    for (size_t i=0;i<4;++i) boxes[i].support=true;
    add(-1100,-200,-1500,750,192,1700,.24f,.27f,.25f);
    add(350,-260,-1500,750,252,1700,.26f,.28f,.25f);
    add(-350,-260,-1500,700,5,1700,.10f,.18f,.20f);
    for (int i=0;i<14;++i) {
        double x=(i-7)*105,z=-360-(i%3)*240,h=300+((i*7)%6)*85;
        if (x>-130 && x<230) continue;
        add(x,0,z,40,h,65,.38f,.46f,.41f);
        add(x-15,h-45,z-15,70,22,95,.53f,.57f,.46f);
        add(x+7,80,z+65,3,h*.4,.6,.39f,.72f,.63f);
        for (int j=0;j<4;++j) add(x-8,50+j*70,z+65,56,4,10,.46f,.52f,.43f);
    }
    add(-155,0,-620,50,540,70,.48f,.54f,.45f);
    add(105,0,-620,50,540,70,.48f,.54f,.45f);
    add(-155,490,-620,310,50,70,.53f,.59f,.48f);
    add(-175,540,-640,350,12,110,.37f,.46f,.41f);
    for (int i=0;i<8;++i) {
        double y=35+i*57;
        add(-155,y,-548,50,5,1,.35f,.71f,.61f);
        add(105,y,-548,50,5,1,.35f,.71f,.61f);
    }
    add(-600,240,-950,1200,15,80,.40f,.48f,.42f);
    add(-440,0,-1040,30,610,30,.40f,.48f,.42f);
    add(410,0,-1040,30,610,30,.40f,.48f,.42f);
    /* Near gate buttresses make shoulder clearance visible during the approach. */
    add(78,0,-215,30,320,30,.42f,.49f,.41f);
    add(-128,0,-215,30,320,30,.42f,.49f,.41f);
    add(-142,320,-225,264,12,50,.49f,.55f,.44f);
    return &authored;
}
typedef struct { double x,z,hx,hz,c,s; } Obb;
static Obb placed(MeuRobotPose p,double x,double z,double hx,double hz,double margin)
{
    double c=cos(p.yaw),s=sin(p.yaw);
    return (Obb){p.x+c*x-s*z,p.z+s*x+c*z,hx+margin,hz+margin,c,s};
}
static bool overlaps(Obb a,double x,double z,double hx,double hz)
{
    double dx=x-a.x,dz=z-a.z,c=fabs(a.c),s=fabs(a.s),epsilon=1e-8;
    return fabs(dx)<a.hx*c+a.hz*s+hx-epsilon &&
        fabs(dz)<a.hx*s+a.hz*c+hz-epsilon &&
        fabs(dx*a.c+dz*a.s)<a.hx+hx*c+hz*s-epsilon &&
        fabs(-dx*a.s+dz*a.c)<a.hz+hx*s+hz*c-epsilon;
}
static void edge(double *values,size_t *n,double value,double low,double high)
{
    if (value>low && value<high) values[(*n)++]=value;
}
static void sort(double *values,size_t n)
{
    for (size_t i=1;i<n;++i) {
        double v=values[i]; size_t j=i;
        while (j && values[j-1]>v) { values[j]=values[j-1]; --j; }
        values[j]=v;
    }
}
/* Partition at every support boundary. A cell is wholly covered or wholly
   uncovered, so even a narrow hole inside a foot cannot hide between samples. */
static bool supported(const MeuWorld *world,Obb foot)
{
    double hx=foot.hx*fabs(foot.c)+foot.hz*fabs(foot.s);
    double hz=foot.hx*fabs(foot.s)+foot.hz*fabs(foot.c);
    double left=foot.x-hx,right=foot.x+hx,front=foot.z-hz,back=foot.z+hz;
    double xs[66]={left,right},zs[66]={front,back}; size_t nx=2,nz=2,supports=0;
    for (size_t i=0;i<world->count;++i) if (world->boxes[i].support) {
        const MeuWorldBox *b=&world->boxes[i];
        if (++supports>32 || fabs(b->y+b->h)>1e-6) return false;
        edge(xs,&nx,b->x,left,right); edge(xs,&nx,b->x+b->w,left,right);
        edge(zs,&nz,b->z,front,back); edge(zs,&nz,b->z+b->d,front,back);
    }
    sort(xs,nx); sort(zs,nz);
    for (size_t ix=1;ix<nx;++ix) for (size_t iz=1;iz<nz;++iz) {
        double w=xs[ix]-xs[ix-1],d=zs[iz]-zs[iz-1];
        if (w<1e-8 || d<1e-8) continue;
        double x=(xs[ix]+xs[ix-1])*.5,z=(zs[iz]+zs[iz-1])*.5;
        if (!overlaps(foot,x,z,w*.5,d*.5)) continue;
        bool covered=false;
        for (size_t i=0;i<world->count;++i) {
            const MeuWorldBox *b=&world->boxes[i];
            if (b->support && x>=b->x && x<=b->x+b->w && z>=b->z && z<=b->z+b->d) { covered=true; break; }
        }
        if (!covered) return false;
    }
    return supports>0;
}
MeuRoute meu_world_validate(const MeuWorld *world,MeuRobotPose p,double margin)
{
    if (!world || !world->boxes || world->count>512 || !isfinite(p.x) || !isfinite(p.z) ||
        !isfinite(p.yaw) || !isfinite(margin) || margin<0 || margin>1000) return MEU_ROUTE_INVALID;
    if (!supported(world,placed(p,-15.5,-1,11.5,24,margin)) ||
        !supported(world,placed(p,15.5,-1,11.5,24,margin))) return MEU_ROUTE_SUPPORT;
    /* Conservative bands cover the visible feet, legs, arms, torso, roof and head. */
    static const double hull[][6]={
        {-27,0,-25,54,9,48},{-22,8,-5,44,92,22},
        {-56,45,-47,112,129,58},{-24,100,-32,48,84,56},
        {-26,172,-58,52,14,45},{-19,184,-23.4,38,56,48.4}
    };
    for (size_t band=0;band<sizeof(hull)/sizeof(hull[0]);++band) {
        const double *h=hull[band];
        Obb shape=placed(p,h[0]+h[3]*.5,h[2]+h[5]*.5,h[3]*.5,h[5]*.5,margin);
        for (size_t i=0;i<world->count;++i) {
            const MeuWorldBox *b=&world->boxes[i];
            if (b->y+b->h<=h[1]+1e-7 || b->y>=h[1]+h[4]+margin) continue;
            if (overlaps(shape,b->x+b->w*.5,b->z+b->d*.5,b->w*.5,b->d*.5)) return MEU_ROUTE_STRUCTURE;
        }
    }
    return MEU_ROUTE_CLEAR;
}
MeuRoute meu_world_sweep(const MeuWorld *world,MeuRobotPose a,MeuRobotPose b)
{
    double motion=hypot(b.x-a.x,b.z-a.z)+MEU_HULL_RADIUS*fabs(b.yaw-a.yaw);
    return meu_world_validate(world,a,MEU_TRAVEL_CLEARANCE+motion);
}
const char *meu_route_name(MeuRoute reason)
{
    static const char *names[]={"Route clear","Structure in stopping path","Unsupported ground ahead","Travel unavailable"};
    return names[reason>=0 && reason<=MEU_ROUTE_INVALID?reason:MEU_ROUTE_INVALID];
}

const MeuWorld *meu_world_with_bridge(void)
{
    static MeuWorldBox extended[161];static MeuWorld world;
    if (!world.count) {
        const MeuWorld *base=meu_world_default();
        for(size_t i=0;i<base->count;++i)extended[i]=base->boxes[i];
        extended[base->count]=(MeuWorldBox){-260,-8,-180,180,8,150,.24f,.61f,.57f,true};
        world=(MeuWorld){extended,base->count+1};
    }
    return &world;
}
