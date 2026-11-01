// biomonotor.c - Biomonitor Graph and Text displays.
#include "common.h"
BioMonitorSystem bioMonitor;
static u8 biomPx[BIOM_GRAPH_W*BIOM_GRAPH_H*4];
u32 biomTexId, biomFBO;
enum { BIOM_UI_X=4, BIOM_UI_Y=41 };
INLINE void biomPutPx(int x,int y,Color c){ u8* p=&biomPx[((u32)y*BIOM_GRAPH_W+(u32)x)*4]; p[0]=(u8)(vclamp(c.r,0.0f,1.0f)*255.0f); p[1]=(u8)(vclamp(c.g,0.0f,1.0f)*255.0f); p[2]=(u8)(vclamp(c.b,0.0f,1.0f)*255.0f); p[3]=(u8)(vclamp(c.a,0.0f,1.0f)*255.0f); }
extern FGL_C glClear;
void BiomonitorClearRenderTarget() { if(!biomFBO){return;} glBindFramebuffer(GL_FRAMEBUFFER,biomFBO); glClear(GL_COLOR_BUFFER_BIT); }
void BiomonitorBlitToUI() {
    if(!(World.invP1.hardwareIsActive & HW_BIO)){return;}
    glBindFramebuffer(GL_READ_FRAMEBUFFER,biomFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
    int gx0=BIOM_UI_X,gy0=UI_H-(BIOM_UI_Y+BIOM_GRAPH_H),gx1=BIOM_UI_X+BIOM_GRAPH_W,gy1=UI_H-BIOM_UI_Y;
    glBlitFramebuffer(0,0,BIOM_GRAPH_W,BIOM_GRAPH_H, gx0,gy0,gx1,gy1, GL_COLOR_BUFFER_BIT,GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
}

void BioMonitorClearGraphs() {
    bioMonitor.ymax = (BIOM_GRAPH_H - 1);
    bioMonitor.beatFinished=World.pauseRelativeTime; bioMonitor.ticksFinished[0]=World.pauseRelativeTime + 0.0211f; bioMonitor.ticksFinished[1]=World.pauseRelativeTime + 0.05f; bioMonitor.ticksFinished[2]=World.pauseRelativeTime + 0.0104f; bioMonitor.tickFinished=World.pauseRelativeTime + 0.02f;
    bioMonitor.currentIndex[0]=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex[1]=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex[2]=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f));
    mset(bioMonitor.rings,0,sizeof(bioMonitor.rings));
}

void BioMonitorAlignCursors() { BioMonitorClearGraphs(); int idx=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex[0]=bioMonitor.currentIndex[1]=bioMonitor.currentIndex[2]=idx; }
static void IncrementGraph(int graph) { if (++bioMonitor.currentIndex[graph]>=BIOM_GRAPH_W) bioMonitor.currentIndex[graph]=0; }


static void Push(int index,float val) { if (index<BIOM_ERG || index>BIOM_ECG) return; bioMonitor.rings[index][bioMonitor.currentIndex[index]]=(u8)vclamp((int)(inverse_lerp(bioMonitor.min[index],bioMonitor.max[index],val)*BIOM_GRAPH_H),0,bioMonitor.ymax); }

void BiomonitorEnergyPulse(float take) { Push(0,take); IncrementGraph(0); Push(0,take); IncrementGraph(0); }
static const float biomFsGpu=33.0f, biomFsCpu=17.0f, biomFsFrame=16.6666f;/*ms at the top of each graph: 30fps for gpu, 60fps for cpu and frame*/
INLINE float biomMsToGraph(int graph, double ms, float fullScale) { return bioMonitor.min[graph] + vclamp((float)ms/fullScale,0.0f,1.0f) * (bioMonitor.max[graph] - bioMonitor.min[graph]); }
void BioMonitorUpdate() {
    if (!Cheats.showFPS && !(World.invP1.hasHardware & HW_BIO)) return;
    if (!(World.invP1.hardwareIsActive & HW_BIO)) return;
    bioMonitor.header = 526; bioMonitor.fatigue=534; /*Low*/ if(World.invP1.fatigue >= 80.0f){bioMonitor.fatigue=532;/*High!*/}else if(World.invP1.fatigue <  80.0f && World.invP1.fatigue > 30.0f){bioMonitor.fatigue=533;/*Moderate*/}
    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.heartRate = vfloor((70.0f + ((World.invP1.fatigue / 100.0f) * 110.0f)) * random_range(0.95f,1.05f));
    static const float beatThresh=0.1f, beatVariation=0.05f;
    /*Energy Usage / showFPS: GPU ms*/ bioMonitor.ergValue = Cheats.showFPS ? biomMsToGraph(BIOM_ERG,World.gpuFrameMs,biomFsGpu) : vclamp((World.invP1.drainJPM / 255.0f),0.0f,1.0f);
    /*Chi Brain Waves / showFPS: CPU ms*/ float brainFactor = 0.15f; if (World.invP1.geniusFinished > World.pauseRelativeTime) brainFactor = 0.35f + random_range(-0.3f,0.3f); if(Cheats.showFPS){bioMonitor.chiValue=biomMsToGraph(BIOM_CHI,World.cpuFrameTime,biomFsCpu);}else{bioMonitor.chiValue=(float)(vsinf(World.pauseRelativeTime * 10.0 * (double)brainFactor));}
    /*ECG: Create shifted sine wave for heart beat.  Apply percent fatigued to 200bpm max heart rate with baseline 50bpm.  showFPS overrides with total frame ms.*/ if (Cheats.showFPS) { bioMonitor.ecgValue = biomMsToGraph(BIOM_ECG,World.thisFrameTime,biomFsFrame); }
    else { float fatigueFactor = ((World.invP1.fatigue / 100.0f) * 120.0f) + 20.0f; fatigueFactor = fatigueFactor / 60.0f;
    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.beatFinished = World.pauseRelativeTime + (1.0 / (double)fatigueFactor);
    bioMonitor.beatShift = (bioMonitor.beatFinished - World.pauseRelativeTime) / (1.0 / (double)fatigueFactor);
    if (bioMonitor.beatShift > 0.94f) bioMonitor.ecgValue = vsinf(bioMonitor.beatShift * 35.0f); else bioMonitor.ecgValue = 0.0f;
    if (bioMonitor.ecgValue > beatThresh || bioMonitor.ecgValue < (beatThresh * -1.0f)) bioMonitor.ecgValue += random_range(-beatVariation,beatVariation); }
    if (Cheats.showFPS) {
        if (bioMonitor.ticksFinished[0] < World.pauseRelativeTime) { bioMonitor.ticksFinished[0] = World.pauseRelativeTime + 0.0104f; Push(0,bioMonitor.ergValue); IncrementGraph(0); }
        if (bioMonitor.ticksFinished[1] < World.pauseRelativeTime) { bioMonitor.ticksFinished[1] = World.pauseRelativeTime + 0.0104f; Push(1,bioMonitor.chiValue); IncrementGraph(1); }
        if (bioMonitor.ticksFinished[2] < World.pauseRelativeTime) { bioMonitor.ticksFinished[2] = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementGraph(2); }
    } else {
        if (bioMonitor.ticksFinished[0] < World.pauseRelativeTime) { bioMonitor.ticksFinished[0] = World.pauseRelativeTime + 0.0211f; Push(0,bioMonitor.ergValue); IncrementGraph(0); Push(0,bioMonitor.ergValue); IncrementGraph(0); Push(0,bioMonitor.ergValue); }
        if (bioMonitor.ticksFinished[1] < World.pauseRelativeTime) { bioMonitor.ticksFinished[1] = World.pauseRelativeTime + 0.05f; Push(1,bioMonitor.chiValue); IncrementGraph(1); Push(1,bioMonitor.chiValue); IncrementGraph(1); Push(1,bioMonitor.chiValue); IncrementGraph(1); Push(1,bioMonitor.chiValue); }
        if (bioMonitor.ticksFinished[2] < World.pauseRelativeTime) { bioMonitor.ticksFinished[2] = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementGraph(2); Push(2,bioMonitor.ecgValue); }
    }
    // Clear once and draw at most three samples per column; retain ERG/CHI/ECG overlap priority.
    mset(biomPx,0,sizeof(biomPx)); const float fade[3]={200,180,275},invFade[3]={1.0f/200,1.0f/180,1.0f/275}; Color colors[3]={bioMonitor.ergColor,bioMonitor.chiColor,bioMonitor.ecgColor};
    for (int x=0;x<BIOM_GRAPH_W;++x) {
        float alpha[3]={0};
        for (int g=0;g<3;++g) {
            int y=bioMonitor.rings[g][x]; if (y<=0 || y>=BIOM_GRAPH_H) continue;
            float dp=bioMonitor.currentIndex[g]-x; if (BIOM_GRAPH_W-x<fade[g] && bioMonitor.currentIndex[g]<fade[g]) dp+=BIOM_GRAPH_W;
            if (dp<0 || dp>fade[g]) dp=fade[g]; alpha[g]=vclamp((fade[g]-dp)*invFade[g],0.0f,1.0f); if (alpha[g]<=0) continue;
            bool blocked=false; for (int k=0;k<g;++k) if (bioMonitor.rings[k][x]==y && alpha[k]>=0.01f) { blocked=true; break; }
            if (!blocked) { Color c=colors[g]; c.a=alpha[g]; biomPutPx(x,y,c); }
        }
    }
    glBindTexture(GL_TEXTURE_2D,biomTexId); glTexSubImage2D(GL_TEXTURE_2D,0,0,0,BIOM_GRAPH_W,BIOM_GRAPH_H,GL_RGBA,GL_UNSIGNED_BYTE,biomPx);
}
