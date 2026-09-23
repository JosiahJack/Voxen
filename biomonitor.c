// biomonotor.c - Biomonitor Graph and Text displays.
#include "common.h"
BioMonitorSystem bioMonitor;
// CPU-rasterized graph texture (automap pattern): 620x36 matches Unity's BiomonitorGraphSystem graphWidth/graphHeight.
static u8 biomPx[BIOM_GRAPH_W*BIOM_GRAPH_H*4]; static u32 biomTexId=0,biomFBO=0;
/* Empty graph pixels stay fully transparent; only the 3 graphed lines carry visible alpha.
   bioMonitor.backgroundColor (a=0.01) remains the logical "empty pixel" marker for the fade logic and is never written to the raster. */
static const Color biomBgTransparent = {0.0f, 0.0f, 0.0f, 0.0f};
/* Graph placement in UI y-down coords: above the biomonitor text block (text starts at y=83), left-aligned with it at x=4. */
enum { BIOM_UI_X=4, BIOM_UI_Y=41 };
INLINE void biomPutPx(int x,int y,Color c){ u8* p=&biomPx[((u32)y*BIOM_GRAPH_W+(u32)x)*4]; p[0]=(u8)(vclamp(c.r,0.0f,1.0f)*255.0f); p[1]=(u8)(vclamp(c.g,0.0f,1.0f)*255.0f); p[2]=(u8)(vclamp(c.b,0.0f,1.0f)*255.0f); p[3]=(u8)(vclamp(c.a,0.0f,1.0f)*255.0f); }
void BiomonitorInitGL() {GenerateAndBindTexture(&biomTexId,GL_RGBA8,BIOM_GRAPH_W,BIOM_GRAPH_H,GL_RGBA,GL_UNSIGNED_BYTE,GL_LINEAR,NULL); glGenFramebuffers(1,&biomFBO); glBindFramebuffer(GL_FRAMEBUFFER,biomFBO); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,biomTexId,0); glBindFramebuffer(GL_FRAMEBUFFER,0);}
void BiomonitorBlitToUI() {
    if(!(World.invP1.hardwareIsActive & HW_BIO)){return;}
    glBindFramebuffer(GL_READ_FRAMEBUFFER,biomFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
    /*No src flip: texture row 0 = graph minimum = bottom of the displayed graph (Unity SetPixel y=0 is the bottom row).
      UI y-down -> GL y-up (automap pattern): dest rect is (BIOM_UI_X,BIOM_UI_Y)-(BIOM_UI_X+620,BIOM_UI_Y+36) in y-down UI coords.*/
    int gx0=BIOM_UI_X,gy0=UI_H-(BIOM_UI_Y+BIOM_GRAPH_H),gx1=BIOM_UI_X+BIOM_GRAPH_W,gy1=UI_H-BIOM_UI_Y;
    glBlitFramebuffer(0,0,BIOM_GRAPH_W,BIOM_GRAPH_H, gx0,gy0,gx1,gy1, GL_COLOR_BUFFER_BIT,GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
}
/* Debug dump of the CPU-rasterized graph texture (automap AutomapDumpBMP pattern):
   vertical flip so the BMP shows the graph as displayed (row 0 = graph minimum = bottom). */
void BiomonitorDumpBMP() {
    if(!(World.invP1.hardwareIsActive & HW_BIO)){return;} OS_MakeFolder("Screenshots");
    static u8 flip[BIOM_GRAPH_W*BIOM_GRAPH_H*4];
    for(int y=0;y<BIOM_GRAPH_H;++y){u8* dst=flip+(size_t)y*BIOM_GRAPH_W*4,*src=biomPx+(size_t)(BIOM_GRAPH_H-1-y)*BIOM_GRAPH_W*4; for(int x=0;x<BIOM_GRAPH_W;++x){u8* d=dst+(size_t)x*4,*s=src+(size_t)x*4; d[0]=s[0]; d[1]=s[1]; d[2]=s[2]; d[3]=s[3];}}
    char p[96]; sFormat(p,sizeof(p),"Screenshots/biomonitor_%.2f.bmp",get_time()); BmpWrite(p,BIOM_GRAPH_W,BIOM_GRAPH_H,flip);
}
void BioMonitorClearGraphs() {
    for (int x=0;x<BIOM_GRAPH_W;x++) { for (int y=0; y<BIOM_GRAPH_H;y++) { /*tex.SetPixel(x,y,bioMonitor.backgroundColor);*/ /*texture cleared via buffer reset above*/ } }
    for (int y=0;y<BIOM_GRAPH_H;y++) bioMonitor.currentColors[y] = bioMonitor.backgroundColor;
    bioMonitor.ymax = (BIOM_GRAPH_H - 1);
    bioMonitor.beatFinished=World.pauseRelativeTime; bioMonitor.tick0Finished=World.pauseRelativeTime + 0.0211f; bioMonitor.tick1Finished=World.pauseRelativeTime + 0.05f; bioMonitor.tick2Finished=World.pauseRelativeTime + 0.0104f; bioMonitor.tickFinished=World.pauseRelativeTime + 0.02f;
    bioMonitor.currentIndex0=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex1=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex2=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f));
    for (int x=0;x<BIOM_GRAPH_W;x++) { for (int y=0; y<BIOM_GRAPH_H;y++) { bioMonitor.colorsERG[x][y] = bioMonitor.backgroundColor; bioMonitor.colorsCHI[x][y] = bioMonitor.backgroundColor; bioMonitor.colorsECG[x][y] = bioMonitor.backgroundColor; } }
}

static void IncrementERG() { bioMonitor.currentIndex0++; if (bioMonitor.currentIndex0 >= BIOM_GRAPH_W) {bioMonitor.currentIndex0 = 0;} }
static void IncrementCHI() { bioMonitor.currentIndex1++; if (bioMonitor.currentIndex1 >= BIOM_GRAPH_W) {bioMonitor.currentIndex1 = 0;} }
static void IncrementECG() { bioMonitor.currentIndex2++; if (bioMonitor.currentIndex2 >= BIOM_GRAPH_W) {bioMonitor.currentIndex2 = 0;} }
void BioMonitorInit() {
    bioMonitor.beatFinished=get_time() + 0.5; bioMonitor.widthPerc=0.4f; bioMonitor.heightPerc=0.1f; bioMonitor.backgroundColor=(Color){0.2f,0.2f,1.0f,0.01f}; bioMonitor.ergColor=(Color){0,0.5f,1.0f,1.0f}; bioMonitor.ymax=36; bioMonitor.chiColor=(Color){.7f,0,1.f,1.f};
    bioMonitor.ecgColor=(Color){1.f,0,0,1.f}; bioMonitor.min[BIOM_ERG]=0; bioMonitor.min[BIOM_CHI]=-2.f; bioMonitor.min[BIOM_ECG]=-1.f; bioMonitor.max[BIOM_ERG]=1.f; bioMonitor.max[BIOM_CHI]=2.f; bioMonitor.max[BIOM_ECG]=1.f; BioMonitorClearGraphs();
}

static void Push(int index, float val) { // Add a data point to the beginning of the graph
    float value = 0.0f; int dist = 1, y0 = 0; bool down = false;
    for (int y=0;y<BIOM_GRAPH_H;y++) {
        bioMonitor.currentColors[y] = bioMonitor.backgroundColor;
        switch(index){case 0:bioMonitor.colorsERG[bioMonitor.currentIndex0][y]=bioMonitor.backgroundColor; break; case 1:bioMonitor.colorsCHI[bioMonitor.currentIndex1][y]=bioMonitor.backgroundColor; break; case 2:bioMonitor.colorsECG[bioMonitor.currentIndex2][y]=bioMonitor.backgroundColor; break;}
    }
    switch(index) {
        case BIOM_ERG:
            value=inverse_lerp(bioMonitor.min[BIOM_ERG],bioMonitor.max[BIOM_ERG],val); y0=vclamp((int)(value*BIOM_GRAPH_H),0,bioMonitor.ymax); bioMonitor.currentColors[y0]=bioMonitor.ergColor; bioMonitor.colorsERG[bioMonitor.currentIndex0][y0]=bioMonitor.ergColor;
            if (vabs(bioMonitor.lastERG - y0) > 2) {
                dist = bioMonitor.lastERG + ((bioMonitor.lastERG > y0) ? -1 : 1); down = (bioMonitor.lastERG > y0);
                while(dist != y0){if(dist > bioMonitor.ymax || dist < 0){break;} bioMonitor.currentColors[dist]= bioMonitor.ergColor; bioMonitor.colorsERG[bioMonitor.currentIndex0][dist]=bioMonitor.ergColor; if(down){dist--;}else{dist++;}}
            }
            bioMonitor.lastERG=y0; if (y0 > 0 && y0 < bioMonitor.ymax) { bioMonitor.currentColors[y0 + 1] = bioMonitor.ergColor; bioMonitor.colorsERG[bioMonitor.currentIndex0][y0 + 1] = bioMonitor.ergColor; } break; // Increase thickness to 2 pixels
        case BIOM_CHI:
            value=inverse_lerp(bioMonitor.min[BIOM_CHI],bioMonitor.max[BIOM_CHI],val); y0 = vclamp((int)(value * BIOM_GRAPH_H),0,bioMonitor.ymax); bioMonitor.currentColors[y0] = bioMonitor.chiColor; bioMonitor.colorsCHI[bioMonitor.currentIndex1][y0]=bioMonitor.chiColor;
            if (vabs(bioMonitor.lastCHI - y0) > 2) {
                dist = bioMonitor.lastCHI + ((bioMonitor.lastCHI > y0) ? -1 : 1); down = (bioMonitor.lastCHI > y0);
                while(dist != y0){if(dist > bioMonitor.ymax || dist < 0){break;} bioMonitor.currentColors[dist]=bioMonitor.chiColor; bioMonitor.colorsCHI[bioMonitor.currentIndex1][dist]=bioMonitor.chiColor; if(down){dist--;}else{dist++;}}
            }
            bioMonitor.lastCHI=y0; if(y0 > 0 && y0 < bioMonitor.ymax){bioMonitor.currentColors[y0 - 1]=bioMonitor.chiColor; bioMonitor.currentColors[y0 + 1]=bioMonitor.chiColor; bioMonitor.colorsCHI[bioMonitor.currentIndex1][y0 + 1]=bioMonitor.chiColor;} break; // Increase thickness to 3 pixels
        case BIOM_ECG:
            value=inverse_lerp(bioMonitor.min[BIOM_ECG],bioMonitor.max[BIOM_ECG],val); y0=vclamp((int)(value*BIOM_GRAPH_H),0,bioMonitor.ymax); bioMonitor.currentColors[y0]=bioMonitor.ecgColor; bioMonitor.colorsECG[bioMonitor.currentIndex2][y0]=bioMonitor.ecgColor;
            if (vabs(bioMonitor.lastECG - y0) > 2) {
                dist =bioMonitor.lastECG + ((bioMonitor.lastECG > y0) ? -1 : 1); down = (bioMonitor.lastECG > y0);
                while(dist != y0){if (dist > bioMonitor.ymax || dist < 0){break;} bioMonitor.currentColors[dist]=bioMonitor.ecgColor; bioMonitor.colorsECG[bioMonitor.currentIndex2][dist]=bioMonitor.ecgColor; if(down){dist--;}else{dist++;}}
            }
            bioMonitor.lastECG=y0; int half=(int)((bioMonitor.max[BIOM_ECG] - bioMonitor.min[BIOM_ECG]) / 2.0f); if(y0 > 0 && y0 < bioMonitor.ymax && (vabs(y0 - half) > 2)){bioMonitor.currentColors[y0 - 1]=bioMonitor.ecgColor; bioMonitor.currentColors[y0 + 1]=bioMonitor.ecgColor; bioMonitor.colorsECG[bioMonitor.currentIndex2][y0 - 1]=bioMonitor.ecgColor; bioMonitor.colorsECG[bioMonitor.currentIndex2][y0 + 1]=bioMonitor.ecgColor; } break; // Increase thickness to 3 pixels
    }
}

void BiomonitorEnergyPulse(float take) { Push(0,take); IncrementERG(); Push(0,take); IncrementERG(); }
void BioMonitorUpdate() {
    if (!(World.invP1.hasHardware & HW_BIO) || !(World.invP1.hardwareIsActive & HW_BIO)) return;
    bioMonitor.header = 526; bioMonitor.heartRateText = 527; bioMonitor.bpmText = 529; bioMonitor.fatigueDetailText = 531; bioMonitor.fatigue=534; /*Low*/ if(World.invP1.fatigue >= 80.0f){bioMonitor.fatigue=532;/*High!*/}else if(World.invP1.fatigue <  80.0f && World.invP1.fatigue > 30.0f){bioMonitor.fatigue=533;/*Moderate*/}
    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.heartRate = vfloor((70.0f + ((World.invP1.fatigue / 100.0f) * 110.0f)) * random_range(0.95f,1.05f));
    static const float beatThresh=0.1f, beatVariation=0.05f;
    /*Energy Usage*/ bioMonitor.ergValue = vclamp((World.invP1.drainJPM / 255.0f),0.0f,1.0f);
    /*Chi Brain Waves*/ float brainFactor = 0.15f; if (World.invP1.geniusFinished > World.pauseRelativeTime) brainFactor = 0.35f + random_range(-0.3f,0.3f); if(Cheats.showFPS){bioMonitor.chiValue=(((float)World.thisFrameTime/16.0f) * 0.5f) - 2.0f;}else{bioMonitor.chiValue=(float)(vsinf(World.pauseRelativeTime * 10.0 * (double)brainFactor));}
    /*ECG: Create shifted sine wave for heart beat.  Apply percent fatigued to 200bpm max heart rate with baseline 50bpm.*/ float fatigueFactor = ((World.invP1.fatigue / 100.0f) * 120.0f) + 20.0f; fatigueFactor = fatigueFactor / 60.0f;
    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.beatFinished = World.pauseRelativeTime + (1.0 / (double)fatigueFactor);
    bioMonitor.beatShift = (bioMonitor.beatFinished - World.pauseRelativeTime) / (1.0 / (double)fatigueFactor);
    if (bioMonitor.beatShift > 0.94f) bioMonitor.ecgValue = vsinf(bioMonitor.beatShift * 35.0f); else bioMonitor.ecgValue = 0.0f;
    if (bioMonitor.ecgValue > beatThresh || bioMonitor.ecgValue < (beatThresh * -1.0f)) bioMonitor.ecgValue += random_range(-beatVariation,beatVariation);
    if (bioMonitor.tick0Finished < World.pauseRelativeTime) { bioMonitor.tick0Finished = World.pauseRelativeTime + 0.0211f; Push(0,bioMonitor.ergValue); IncrementERG(); Push(0,bioMonitor.ergValue); IncrementERG(); Push(0,bioMonitor.ergValue); }
    if (bioMonitor.tick1Finished < World.pauseRelativeTime) {
        if (Cheats.showFPS) bioMonitor.tick1Finished = World.pauseRelativeTime + 0.0104f; else bioMonitor.tick1Finished = World.pauseRelativeTime + 0.05f;
        Push(1,bioMonitor.chiValue); IncrementCHI(); Push(1,bioMonitor.chiValue); IncrementCHI(); Push(1,bioMonitor.chiValue); IncrementCHI(); Push(1,bioMonitor.chiValue);
    }
    if (bioMonitor.tick2Finished < World.pauseRelativeTime) { bioMonitor.tick2Finished = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementECG(); Push(2,bioMonitor.ecgValue); }
    float distPerc = 1.0f, fadeDist = 100.0f;
    for (int x=0;x<BIOM_GRAPH_W;x++) {
        for (int y=0; y<BIOM_GRAPH_H;y++) {
            bioMonitor.col0 = bioMonitor.colorsERG[x][y]; bioMonitor.col1 = bioMonitor.colorsCHI[x][y]; bioMonitor.col2 = bioMonitor.colorsECG[x][y];
            if (bioMonitor.col0.a > 0.01f) {
                fadeDist = 200.0f; distPerc = (bioMonitor.currentIndex0 - x); if ((BIOM_GRAPH_W - x) < fadeDist && bioMonitor.currentIndex0 < fadeDist) distPerc += BIOM_GRAPH_W; if (distPerc < 0.0f || distPerc > fadeDist) distPerc = fadeDist;
                distPerc = vclamp((fadeDist - distPerc) / fadeDist,0.0f,1.0f); if(distPerc == 0.0f){bioMonitor.colorsERG[x][y]=bioMonitor.backgroundColor;} bioMonitor.col0.a = distPerc; biomPutPx(x,y,bioMonitor.col0);
            } else if (bioMonitor.col1.a > 0.01f) {
                fadeDist = 180.0f; distPerc = (bioMonitor.currentIndex1 - x); if ((BIOM_GRAPH_W - x) < fadeDist && bioMonitor.currentIndex1 < fadeDist) distPerc += BIOM_GRAPH_W; if (distPerc < 0.0f || distPerc > fadeDist) distPerc = fadeDist;
                distPerc = vclamp((fadeDist - distPerc) / fadeDist,0.0f,1.0f); if (distPerc == 0.0f) bioMonitor.colorsCHI[x][y] = bioMonitor.backgroundColor; bioMonitor.col1.a = distPerc; biomPutPx(x,y,bioMonitor.col1);
            } else if (bioMonitor.col2.a > 0.01f) {
                fadeDist = 275.0f; distPerc = (bioMonitor.currentIndex2 - x); if ((BIOM_GRAPH_W - x) < fadeDist && bioMonitor.currentIndex2 < fadeDist) distPerc +=  BIOM_GRAPH_W; if (distPerc < 0.0f || distPerc > fadeDist) distPerc = fadeDist;
                distPerc = vclamp((fadeDist - distPerc) / fadeDist,0.0f,1.0f); if (distPerc == 0.0f) bioMonitor.colorsECG[x][y] = bioMonitor.backgroundColor; bioMonitor.col2.a = distPerc; biomPutPx(x,y,bioMonitor.col2);
            } else { biomPutPx(x,y,biomBgTransparent); }
        }
    }
    glBindTexture(GL_TEXTURE_2D,biomTexId); glTexSubImage2D(GL_TEXTURE_2D,0,0,0,BIOM_GRAPH_W,BIOM_GRAPH_H,GL_RGBA,GL_UNSIGNED_BYTE,biomPx);
    if (bioMonitor.tickFinished < World.pauseRelativeTime) { bioMonitor.tickFinished = World.pauseRelativeTime + 0.02f; IncrementERG(); IncrementCHI(); IncrementECG(); }
}
