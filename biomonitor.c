// biomonotor.c - Biomonitor Graph and Text displays.
#include "common.h"
BioMonitorSystem bioMonitor;
static u8 biomPx[BIOM_GRAPH_W*BIOM_GRAPH_H*4];
u32 biomTexId, biomFBO;
static const Color biomBgTransparent = {0.0f, 0.0f, 0.0f, 0.0f};
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
    bioMonitor.beatFinished=World.pauseRelativeTime; bioMonitor.tick0Finished=World.pauseRelativeTime + 0.0211f; bioMonitor.tick1Finished=World.pauseRelativeTime + 0.05f; bioMonitor.tick2Finished=World.pauseRelativeTime + 0.0104f; bioMonitor.tickFinished=World.pauseRelativeTime + 0.02f;
    bioMonitor.currentIndex0=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex1=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex2=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f));
    mset(bioMonitor.ringERG, 0, sizeof(bioMonitor.ringERG)); mset(bioMonitor.ringCHI, 0, sizeof(bioMonitor.ringCHI)); mset(bioMonitor.ringECG, 0, sizeof(bioMonitor.ringECG));
}

void BioMonitorAlignCursors() { BioMonitorClearGraphs(); int idx=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex0=bioMonitor.currentIndex1=bioMonitor.currentIndex2=idx; }
static void IncrementERG() { bioMonitor.currentIndex0++; if (bioMonitor.currentIndex0 >= BIOM_GRAPH_W) {bioMonitor.currentIndex0 = 0;} }
static void IncrementCHI() { bioMonitor.currentIndex1++; if (bioMonitor.currentIndex1 >= BIOM_GRAPH_W) {bioMonitor.currentIndex1 = 0;} }
static void IncrementECG() { bioMonitor.currentIndex2++; if (bioMonitor.currentIndex2 >= BIOM_GRAPH_W) {bioMonitor.currentIndex2 = 0;} }
static void Push(int index, float val) { // Add a data point to the beginning of the graph
    float value = 0.0f; int y0 = 0;
    switch(index) {
        case BIOM_ERG:
            value = inverse_lerp(bioMonitor.min[BIOM_ERG],bioMonitor.max[BIOM_ERG],val);
            y0 = vclamp((int)(value*BIOM_GRAPH_H),0,bioMonitor.ymax);
            bioMonitor.ringERG[bioMonitor.currentIndex0] = (u8)y0;
            break;
        case BIOM_CHI:
            value = inverse_lerp(bioMonitor.min[BIOM_CHI],bioMonitor.max[BIOM_CHI],val);
            y0 = vclamp((int)(value*BIOM_GRAPH_H),0,bioMonitor.ymax);
            bioMonitor.ringCHI[bioMonitor.currentIndex1] = (u8)y0;
            break;
        case BIOM_ECG:
            value = inverse_lerp(bioMonitor.min[BIOM_ECG],bioMonitor.max[BIOM_ECG],val);
            y0 = vclamp((int)(value*BIOM_GRAPH_H),0,bioMonitor.ymax);
            bioMonitor.ringECG[bioMonitor.currentIndex2] = (u8)y0;
            break;
    }
}

void BiomonitorEnergyPulse(float take) { Push(0,take); IncrementERG(); Push(0,take); IncrementERG(); }
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
        if (bioMonitor.tick0Finished < World.pauseRelativeTime) { bioMonitor.tick0Finished = World.pauseRelativeTime + 0.0104f; Push(0,bioMonitor.ergValue); IncrementERG(); }
        if (bioMonitor.tick1Finished < World.pauseRelativeTime) { bioMonitor.tick1Finished = World.pauseRelativeTime + 0.0104f; Push(1,bioMonitor.chiValue); IncrementCHI(); }
        if (bioMonitor.tick2Finished < World.pauseRelativeTime) { bioMonitor.tick2Finished = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementECG(); }
    } else {
        if (bioMonitor.tick0Finished < World.pauseRelativeTime) { bioMonitor.tick0Finished = World.pauseRelativeTime + 0.0211f; Push(0,bioMonitor.ergValue); IncrementERG(); Push(0,bioMonitor.ergValue); IncrementERG(); Push(0,bioMonitor.ergValue); }
        if (bioMonitor.tick1Finished < World.pauseRelativeTime) { bioMonitor.tick1Finished = World.pauseRelativeTime + 0.05f; Push(1,bioMonitor.chiValue); IncrementCHI(); Push(1,bioMonitor.chiValue); IncrementCHI(); Push(1,bioMonitor.chiValue); IncrementCHI(); Push(1,bioMonitor.chiValue); }
        if (bioMonitor.tick2Finished < World.pauseRelativeTime) { bioMonitor.tick2Finished = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementECG(); Push(2,bioMonitor.ecgValue); }
    }
    float fadeDist = 200.0f;
    for (int x=0;x<BIOM_GRAPH_W;x++) {
        int y0erg = (int)bioMonitor.ringERG[x];
        int y0chi = (int)bioMonitor.ringCHI[x];
        int y0ecg = (int)bioMonitor.ringECG[x];
        for (int y=0; y<BIOM_GRAPH_H; y++) {
            Color c = biomBgTransparent;
            float fd = fadeDist; float dp = 1.0f;
            // ERG
            if (y0erg > 0) {
                fd = 200.0f; dp = (float)(bioMonitor.currentIndex0 - x); if ((BIOM_GRAPH_W - x) < fd && bioMonitor.currentIndex0 < fd) dp += BIOM_GRAPH_W; if (dp < 0.0f || dp > fd) dp = fd; dp = vclamp((fd - dp) / fd, 0.0f, 1.0f);
                if (y == y0erg && dp > 0.0f) { Color c0 = bioMonitor.ergColor; c0.a = dp; c = c0; }
            }
            // CHI (if no erg at this pixel)
            if (c.a < 0.01f && y0chi > 0) {
                fd = 180.0f; dp = (float)(bioMonitor.currentIndex1 - x); if ((BIOM_GRAPH_W - x) < fd && bioMonitor.currentIndex1 < fd) dp += BIOM_GRAPH_W; if (dp < 0.0f || dp > fd) dp = fd; dp = vclamp((fd - dp) / fd, 0.0f, 1.0f);
                if (y == y0chi && dp > 0.0f) { Color c0 = bioMonitor.chiColor; c0.a = dp; c = c0; }
            }
            // ECG (if no other)
            if (c.a < 0.01f && y0ecg > 0) {
                fd = 275.0f; dp = (float)(bioMonitor.currentIndex2 - x); if ((BIOM_GRAPH_W - x) < fd && bioMonitor.currentIndex2 < fd) dp += BIOM_GRAPH_W; if (dp < 0.0f || dp > fd) dp = fd; dp = vclamp((fd - dp) / fd, 0.0f, 1.0f);
                if (y == y0ecg && dp > 0.0f) { Color c0 = bioMonitor.ecgColor; c0.a = dp; c = c0; }
            }
            biomPutPx(x, y, c);
        }
    }
    glBindTexture(GL_TEXTURE_2D,biomTexId); glTexSubImage2D(GL_TEXTURE_2D,0,0,0,BIOM_GRAPH_W,BIOM_GRAPH_H,GL_RGBA,GL_UNSIGNED_BYTE,biomPx);
}
