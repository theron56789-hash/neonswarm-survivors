// SAVE AS: source/main.c   (Neon Swarm: Survivors - 3DS, libctru + citro2d)
#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define NE 110
#define NB 48
#define NG 150
#define NF 6
#define NR 8
#define RGB(r,g,b) C2D_Color32(r,g,b,255)
enum{BLASTER,BLADES,PULSE,BOLT,SPD,HPUP,MAG,DMG,NK};
static const char*NAME[NK]={"Blaster","Orbit Blades","Shockwave","Lightning","Speed Boots","Heart Up","Magnet","Power Core"};
static const char*DESC[NK]={"Shoots nearest monster","Blades spin around you","Blast ring hurts all close","Zaps random monsters","+12% move speed","+25 max HP and heal","Grab gems from farther","+15% damage"};
typedef struct{float x,y,r,hp,s,dm,hc,f;int v,on;u32 c;}En;
typedef struct{float x,y,vx,vy,t,d;}Bu;
typedef struct{float x,y;int v,m,on;}Ge;
typedef struct{float x,y;int on;}Fo;
typedef struct{float x,y,x2,y2,mr,t;int k;}Ri;
static En E[NE];static Bu B[NB];static Ge G[NG];static Fo F[NF];static Ri R[NR];
static float px,py,php,pmh,psp,pmag,pdm,ptime,sp,cd[3],ang,bl[8][2],best;
static int L[NK],xp,nd,lv,kills,mode,opt[3],nopt,nbl;
// mode: 0 pick first weapon, 1 playing, 2 level-up pick, 3 game over
static C2D_TextBuf tb;
static float dist(float a,float b,float c,float d){return hypotf(c-a,d-b);}
static void txt(const char*s,float x,float y,float sz,u32 c){C2D_Text t;C2D_TextParse(&t,tb,s);C2D_TextOptimize(&t);C2D_DrawText(&t,C2D_WithColor,x,y,0.5f,sz,sz,c);}

static void reset(void){
 memset(E,0,sizeof E);memset(B,0,sizeof B);memset(G,0,sizeof G);memset(F,0,sizeof F);memset(R,0,sizeof R);memset(L,0,sizeof L);
 px=py=0;php=pmh=100;psp=90;pmag=55;pdm=1;ptime=0;sp=0;cd[0]=cd[1]=cd[2]=0;ang=0;nbl=0;xp=0;nd=5;lv=1;kills=0;}
static void offer(int first){
 int a[NK],n=0;for(int k=0;k<(first?4:NK);k++)if(L[k]<5)a[n++]=k;
 for(int i=n-1;i>0;i--){int j=rand()%(i+1),t=a[i];a[i]=a[j];a[j]=t;}
 nopt=n<3?n:3;for(int i=0;i<nopt;i++)opt[i]=a[i];
 if(!nopt){php=pmh;mode=1;return;}
 mode=first?0:2;}
static void pick(int k){
 L[k]++;if(k==SPD)psp*=1.12f;if(k==HPUP){pmh+=25;php=fminf(pmh,php+25);}if(k==MAG)pmag+=35;if(k==DMG)pdm+=.15f;mode=1;}
static void spawn(void){
 for(int i=0;i<NE;i++)if(!E[i].on){
  En*e=&E[i];float a=(rand()%628)/100.f,r=(rand()%100)/100.f;
  *e=(En){0};e->on=1;e->r=12;e->hp=10+ptime*.2f;e->s=50+fminf(40,ptime/3);e->dm=10;e->v=1;e->c=RGB(255,77,141);
  if(ptime>25&&r<.2f){e->r=9;e->hp=6+ptime*.1f;e->s=105;e->dm=8;e->c=RGB(255,210,63);}
  else if(ptime>50&&r<.3f){e->r=20;e->hp=40+ptime*.8f;e->s=42;e->dm=18;e->v=4;e->c=RGB(155,92,255);}
  e->x=px+cosf(a)*250;e->y=py+sinf(a)*250;return;}}
static void hit(En*e,float d){
 if(!e->on)return;e->hp-=d*pdm;e->f=.08f;
 if(e->hp<=0){e->on=0;kills++;
  for(int i=0;i<NG;i++)if(!G[i].on){G[i]=(Ge){e->x,e->y,e->v,0,1};break;}
  if(rand()%100<5)for(int i=0;i<NF;i++)if(!F[i].on){F[i]=(Fo){e->x,e->y,1};break;}}}
static En*nearest(void){En*b=0;float bd=1e9f;for(int i=0;i<NE;i++)if(E[i].on){float d=dist(px,py,E[i].x,E[i].y);if(d<bd){bd=d;b=&E[i];}}return b;}
static void addR(int k,float x,float y,float x2,float y2,float mr,float t){
 for(int i=0;i<NR;i++)if(R[i].t<=0){R[i]=(Ri){x,y,x2,y2,mr,t,k};return;}}

static void update(float dt,circlePosition cp,u32 kh){
 ptime+=dt;
 float mx=cp.dx/156.f,my=-cp.dy/156.f;if(fabsf(mx)<.1f)mx=0;if(fabsf(my)<.1f)my=0;
 if(kh&KEY_DLEFT)mx-=1;if(kh&KEY_DRIGHT)mx+=1;if(kh&KEY_DUP)my-=1;if(kh&KEY_DDOWN)my+=1;
 float l=hypotf(mx,my);if(l>1){mx/=l;my/=l;}
 px+=mx*psp*dt;py+=my*psp*dt;
 sp+=dt*(1.3f+ptime/22);while(sp>=1){spawn();sp--;}
 for(int k=0;k<3;k++)cd[k]-=dt;
 if(L[BLASTER]&&cd[0]<=0){int lv_=L[BLASTER];cd[0]=fmaxf(.2f,.8f-.1f*lv_);En*t=nearest();
  if(t){float a=atan2f(t->y-py,t->x-px);int n=1+(lv_>>1);
   for(int i=0;i<n;i++){float o=a+(i-(n-1)/2.f)*.2f;
    for(int j=0;j<NB;j++)if(B[j].t<=0){B[j]=(Bu){px,py,cosf(o)*260,sinf(o)*260,1.2f,8.f+lv_*2};break;}}}}
 if(L[PULSE]&&cd[1]<=0){int lv_=L[PULSE];float r=60+lv_*18;cd[1]=3.2f-.3f*lv_;addR(0,px,py,0,0,r,.35f);
  for(int i=0;i<NE;i++)if(E[i].on&&dist(E[i].x,E[i].y,px,py)<r+E[i].r)hit(&E[i],10+lv_*4);}
 if(L[BOLT]&&cd[2]<=0){int lv_=L[BOLT];cd[2]=1.8f-.2f*lv_;int n=0;
  for(int tries=0;tries<40&&n<lv_+1;tries++){En*e=&E[rand()%NE];
   if(e->on&&dist(e->x,e->y,px,py)<260){hit(e,14+lv_*3);addR(1,e->x,e->y-300,e->x,e->y,0,.15f);n++;}}}
 nbl=0;
 if(L[BLADES]){int lv_=L[BLADES],n=lv_+1;float r=50+lv_*4;ang+=3.2f*dt;
  for(int i=0;i<n;i++){float a=ang+i*6.2832f/n,bx=px+cosf(a)*r,by=py+sinf(a)*r;bl[nbl][0]=bx;bl[nbl][1]=by;nbl++;
   for(int j=0;j<NE;j++)if(E[j].on&&E[j].hc<=0&&dist(E[j].x,E[j].y,bx,by)<E[j].r+9){hit(&E[j],7+lv_*2);E[j].hc=.3f;}}}
 for(int i=0;i<NB;i++)if(B[i].t>0){Bu*b=&B[i];b->x+=b->vx*dt;b->y+=b->vy*dt;b->t-=dt;
  for(int j=0;j<NE;j++)if(E[j].on&&dist(E[j].x,E[j].y,b->x,b->y)<E[j].r+4){hit(&E[j],b->d);b->t=0;break;}}
 for(int i=0;i<NE;i++)if(E[i].on){En*e=&E[i];e->hc-=dt;e->f-=dt;float a=atan2f(py-e->y,px-e->x);
  e->x+=cosf(a)*e->s*dt;e->y+=sinf(a)*e->s*dt;if(dist(px,py,e->x,e->y)<e->r+12)php-=e->dm*dt;}
 for(int i=0;i<NG;i++)if(G[i].on){Ge*g=&G[i];float d=dist(px,py,g->x,g->y);if(d<pmag)g->m=1;
  if(g->m){float a=atan2f(py-g->y,px-g->x);g->x+=cosf(a)*260*dt;g->y+=sinf(a)*260*dt;}
  if(d<14){xp+=g->v;g->on=0;}}
 for(int i=0;i<NF;i++)if(F[i].on&&dist(px,py,F[i].x,F[i].y)<18){php=fminf(pmh,php+25);F[i].on=0;}
 for(int i=0;i<NR;i++)R[i].t-=dt;
 if(xp>=nd){xp-=nd;lv++;nd=(int)(nd*1.3f+3);offer(0);}
 if(php<=0){mode=3;if(ptime>best)best=ptime;}}

#define SX(x) ((x)-px+200)
#define SY(y) ((y)-py+120)
static void drawTop(void){
 char s[64];u32 gr=RGB(28,28,58);
 float ox=-fmodf(px,40.f),oy=-fmodf(py,40.f);if(ox<0)ox+=40;if(oy<0)oy+=40;
 for(float x=ox;x<400;x+=40)C2D_DrawLine(x,0,gr,x,240,gr,1,0);
 for(float y=oy;y<240;y+=40)C2D_DrawLine(0,y,gr,400,y,gr,1,0);
 for(int i=0;i<NG;i++)if(G[i].on)C2D_DrawCircleSolid(SX(G[i].x),SY(G[i].y),0,G[i].v>1?6:4,G[i].v>1?RGB(255,102,255):RGB(51,255,255));
 for(int i=0;i<NF;i++)if(F[i].on){float x=SX(F[i].x),y=SY(F[i].y);C2D_DrawCircleSolid(x,y,0,8,RGB(51,255,102));C2D_DrawRectSolid(x-5,y-1.5f,0,10,3,RGB(255,255,255));C2D_DrawRectSolid(x-1.5f,y-5,0,3,10,RGB(255,255,255));}
 for(int i=0;i<NE;i++)if(E[i].on){En*e=&E[i];float x=SX(e->x),y=SY(e->y);
  C2D_DrawCircleSolid(x,y,0,e->r,e->f>0?RGB(255,255,255):e->c);
  C2D_DrawRectSolid(x-e->r*.45f,y-e->r*.3f,0,3,3,RGB(0,0,0));C2D_DrawRectSolid(x+e->r*.2f,y-e->r*.3f,0,3,3,RGB(0,0,0));}
 for(int i=0;i<nbl;i++)C2D_DrawCircleSolid(SX(bl[i][0]),SY(bl[i][1]),0,9,RGB(136,204,255));
 for(int i=0;i<NB;i++)if(B[i].t>0)C2D_DrawCircleSolid(SX(B[i].x),SY(B[i].y),0,4,RGB(255,255,0));
 for(int i=0;i<NR;i++)if(R[i].t>0){Ri*r=&R[i];
  if(r->k==0)C2D_DrawCircleSolid(SX(r->x),SY(r->y),0,r->mr*(1-r->t/.35f),C2D_Color32(80,255,255,70));
  else C2D_DrawLine(SX(r->x),SY(r->y),RGB(255,255,136),SX(r->x2),SY(r->y2),RGB(255,255,136),3,0);}
 C2D_DrawCircleSolid(200,120,0,12,RGB(51,255,255));C2D_DrawCircleSolid(200,120,0,6,RGB(255,255,255));
 C2D_DrawRectSolid(180,142,0,40,5,RGB(68,0,0));C2D_DrawRectSolid(180,142,0,40*fmaxf(0,php)/pmh,5,RGB(51,255,102));
 C2D_DrawRectSolid(0,0,0,400,8,RGB(34,34,51));C2D_DrawRectSolid(0,0,0,400.f*xp/nd,8,RGB(51,255,255));
 snprintf(s,64,"LV %d",lv);txt(s,6,10,.5f,RGB(255,255,255));
 snprintf(s,64,"%d:%02d",(int)ptime/60,(int)ptime%60);txt(s,185,10,.5f,RGB(255,255,255));
 snprintf(s,64,"Kills %d",kills);txt(s,335,10,.5f,RGB(255,255,255));
 if(mode==0||mode==2)txt(mode?"LEVEL UP! Pick on bottom screen":"Pick a weapon on bottom screen",70,100,.55f,RGB(51,255,255));}

static void drawBot(void){
 char s[96];u32 w=RGB(255,255,255);
 if(mode==0||mode==2){
  txt(mode?"LEVEL UP!":"CHOOSE WEAPON",100,4,.6f,RGB(51,255,255));
  for(int i=0;i<nopt;i++){float y=28+i*70;int k=opt[i];
   C2D_DrawRectSolid(10,y,0,300,62,RGB(51,255,255));C2D_DrawRectSolid(12,y+2,0,296,58,RGB(22,22,58));
   snprintf(s,96,"%d) %s  %s",i+1,NAME[k],L[k]?"(upgrade)":"(NEW)");txt(s,20,y+8,.55f,w);txt(DESC[k],20,y+32,.45f,RGB(190,200,220));}
  txt("Tap a card or press A / B / X",50,225,.4f,RGB(150,160,180));}
 else if(mode==3){
  txt("YOU GOT SWARMED!",70,40,.7f,RGB(255,77,141));
  snprintf(s,96,"Survived %d:%02d   Kills %d   Level %d",(int)ptime/60,(int)ptime%60,kills,lv);txt(s,30,100,.5f,w);
  snprintf(s,96,"Best %d:%02d",(int)best/60,(int)best%60);txt(s,110,130,.5f,w);
  txt("Press A or tap to play again",55,180,.5f,RGB(51,255,255));}
 else{
  txt("Move: Circle Pad / D-Pad   START: quit",20,6,.45f,RGB(150,160,180));
  snprintf(s,96,"HP %d / %d",(int)php,(int)pmh);txt(s,20,36,.5f,w);
  int y=66;for(int k=0;k<NK;k++)if(L[k]){snprintf(s,96,"%s  Lv %d",NAME[k],L[k]);txt(s,20,y,.45f,k<4?RGB(255,255,0):RGB(51,255,255));y+=22;}}}

int main(void){
 gfxInitDefault();C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);C2D_Init(C2D_DEFAULT_MAX_OBJECTS);C2D_Prepare();
 C3D_RenderTarget*top=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT),*bot=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);
 tb=C2D_TextBufNew(4096);srand(time(NULL));reset();offer(1);
 u64 last=osGetTime();
 while(aptMainLoop()){
  hidScanInput();u32 kd=hidKeysDown(),kh=hidKeysHeld();if(kd&KEY_START)break;
  u64 now=osGetTime();float dt=(now-last)/1000.f;last=now;if(dt>.05f)dt=.05f;
  touchPosition tp;hidTouchRead(&tp);circlePosition cp;hidCircleRead(&cp);
  if(mode==0||mode==2){int c=-1;
   if(kd&KEY_A)c=0;else if(kd&KEY_B)c=1;else if(kd&KEY_X)c=2;
   if(kd&KEY_TOUCH)for(int i=0;i<nopt;i++){float y=28+i*70;if(tp.px>=10&&tp.px<=310&&tp.py>=y&&tp.py<=y+62)c=i;}
   if(c>=0&&c<nopt)pick(opt[c]);}
  else if(mode==3){if(kd&(KEY_A|KEY_TOUCH)){reset();offer(1);}}
  else update(dt,cp,kh);
  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(tb);
  C2D_TargetClear(top,RGB(11,11,26));C2D_SceneBegin(top);drawTop();
  C2D_TargetClear(bot,RGB(15,15,35));C2D_SceneBegin(bot);drawBot();
  C3D_FrameEnd(0);}
 C2D_TextBufDelete(tb);C2D_Fini();C3D_Fini();gfxExit();return 0;}
