// biomonotor.c - Biomonitor Graph and Text displays.
#include "common.h"
#include "lib.h"
#define BIOM_ERG 0
#define BIOM_CHI 1
#define BIOM_ECG 2
#define BIOM_GRAPH_W 620
#define BIOM_GRAPH_H  36
typedef struct {
	float heartRate;
	Text patchEffects;
	Text heartRateText;
	Text header;
	Text bpmText;
	Text fatigueDetailText;
	Text fatigue;
	double beatFinished; // Visual only, Time.time controlled
	float widthPerc;
    float heightPerc;
    float max[3]; // Value at the top of the graph
    float min[3]; // Value at the bottom of the graph
    Color currentColors[BIOM_GRAPH_H];
    Color colorsERG[BIOM_GRAPH_W][BIOM_GRAPH_H];
    Color colorsCHI[BIOM_GRAPH_W][BIOM_GRAPH_H];
    Color colorsECG[BIOM_GRAPH_W][BIOM_GRAPH_H];
    int lastERG;
    int lastCHI;
    int lastECG;
    Color backgroundColor;
    Color ergColor;
    Color chiColor;
    Color ecgColor;
    Color col;
    int ymax;
	float ecgValue;
	float ergValue;
	float chiValue;
	float beatShift;
    double tick0Finished;
    double tick1Finished;
    double tick2Finished;
    double tickFinished; // Overall marching.
    double tick0;
    double tick1;
    double tick2;
    double tick;
    int currentIndex0;
    int currentIndex1;
    int currentIndex2;
    Color col0;
    Color col1;
    Color col2;
} BioMonitorSystem;
BioMonitorSystem bioMonitor;
void BioMonitorClearGraphs(void) {
    for (int x=0;x<BIOM_GRAPH_W;x++) {
        for (int y=0; y<BIOM_GRAPH_H;y++) {
//             tex.SetPixel(x,y,bioMonitor.backgroundColor); // TODO, clear texture
        }
    }
    for (int y=0;y<BIOM_GRAPH_H;y++) bioMonitor.currentColors[y] = bioMonitor.backgroundColor;
    bioMonitor.ymax = (BIOM_GRAPH_H - 1);
    bioMonitor.beatFinished  = World.pauseRelativeTime;
    bioMonitor.tick0Finished = World.pauseRelativeTime + bioMonitor.tick0;
    bioMonitor.tick1Finished = World.pauseRelativeTime + bioMonitor.tick1;
    bioMonitor.tick2Finished = World.pauseRelativeTime + bioMonitor.tick2;
    bioMonitor.tickFinished  = World.pauseRelativeTime + bioMonitor.tick;
    bioMonitor.currentIndex0 = (int)(BIOM_GRAPH_W * random_range(0.0f,1.0f));
    bioMonitor.currentIndex1 = (int)(BIOM_GRAPH_W * random_range(0.0f,1.0f));
    bioMonitor.currentIndex2 = (int)(BIOM_GRAPH_W * random_range(0.0f,1.0f));
    for (int x=0;x<BIOM_GRAPH_W;x++) {
        for (int y=0; y<BIOM_GRAPH_H;y++) {
            bioMonitor.colorsERG[x][y] = bioMonitor.backgroundColor;
            bioMonitor.colorsCHI[x][y] = bioMonitor.backgroundColor;
            bioMonitor.colorsECG[x][y] = bioMonitor.backgroundColor;
        }
    }
}

static void IncrementERG(void) { bioMonitor.currentIndex0++; if (bioMonitor.currentIndex0 >= BIOM_GRAPH_W) {bioMonitor.currentIndex0 = 0;} }
static void IncrementCHI(void) { bioMonitor.currentIndex1++; if (bioMonitor.currentIndex1 >= BIOM_GRAPH_W) {bioMonitor.currentIndex1 = 0;} }
static void IncrementECG(void) { bioMonitor.currentIndex2++; if (bioMonitor.currentIndex2 >= BIOM_GRAPH_W) {bioMonitor.currentIndex2 = 0;} }
void BioMonitorInit(void) {
    bioMonitor.beatFinished = get_time() + 0.5; // Half second beat tick
    bioMonitor.widthPerc = 0.4f; bioMonitor.heightPerc = 0.1f;
    bioMonitor.backgroundColor = (Color){0.2f,0.2f,1.0f,0.01f};
    bioMonitor.ergColor = (Color){0.0f,0.5f,1.0f,1.0f}; bioMonitor.chiColor = (Color){0.7f,0.0f,1.0f,1.0f}; bioMonitor.ecgColor = (Color){1.0f,0.0f,0.0f,1.0f};
    bioMonitor.ymax = 36;
    bioMonitor.tick = 0.02; bioMonitor.tick0 = 0.0211; bioMonitor.tick1 = 0.050; bioMonitor.tick2 = 0.0104;
    bioMonitor.min[BIOM_ERG] =  0.0f; bioMonitor.min[BIOM_CHI] = -2.0f; bioMonitor.min[BIOM_ECG] = -1.0f;
    bioMonitor.max[BIOM_ERG] =  1.0f; bioMonitor.max[BIOM_CHI] =  2.0f; bioMonitor.max[BIOM_ECG] =  1.0f;
    BioMonitorClearGraphs();
}

// Add a data point to the beginning of the graph
static void Push(int index, float val) {
    float value = 0.0f;
    int dist = 1, y0 = 0;
    bool down = false;
    for (int y=0;y<BIOM_GRAPH_H;y++) {
        bioMonitor.currentColors[y] = bioMonitor.backgroundColor;
        switch(index) {
            case 0: bioMonitor.colorsERG[bioMonitor.currentIndex0][y] = bioMonitor.backgroundColor; break;
            case 1: bioMonitor.colorsCHI[bioMonitor.currentIndex1][y] = bioMonitor.backgroundColor; break;
            case 2: bioMonitor.colorsECG[bioMonitor.currentIndex2][y] = bioMonitor.backgroundColor; break;
        }
    }

    switch(index) {
        case BIOM_ERG:
            value = inverse_lerp(bioMonitor.min[BIOM_ERG],bioMonitor.max[BIOM_ERG],val);
            y0 = (int)(value * BIOM_GRAPH_H);
            if (y0 > bioMonitor.ymax) y0 = bioMonitor.ymax;
            if (y0 < 0) y0 = 0;
            bioMonitor.currentColors[y0] = bioMonitor.ergColor;
            bioMonitor.colorsERG[bioMonitor.currentIndex0][y0] = bioMonitor.ergColor;
            if (vabs(bioMonitor.lastERG - y0) > 2) {
                dist = bioMonitor.lastERG + ((bioMonitor.lastERG > y0) ? -1 : 1);
                down = (bioMonitor.lastERG > y0);
                while (dist != y0) {
                    if (dist > bioMonitor.ymax || dist < 0) break;

                    bioMonitor.currentColors[dist]                       = bioMonitor.ergColor;
                    bioMonitor.colorsERG[bioMonitor.currentIndex0][dist] = bioMonitor.ergColor;
                    if (down) dist--;
                    else dist++;
                }
            }
            bioMonitor.lastERG = y0;
            if (y0 > 0 && y0 < bioMonitor.ymax) { // Increase thickness to 2 pixels
                bioMonitor.currentColors[y0 + 1] = bioMonitor.ergColor;
                bioMonitor.colorsERG[bioMonitor.currentIndex0][y0 + 1] = bioMonitor.ergColor;
            }
            break;
        case BIOM_CHI:
            value = inverse_lerp(bioMonitor.min[BIOM_CHI],bioMonitor.max[BIOM_CHI],val);
            y0 = (int)(value * BIOM_GRAPH_H);
            if (y0 > bioMonitor.ymax) y0 = bioMonitor.ymax;
            if (y0 < 0) y0 = 0;
            bioMonitor.currentColors[y0] = bioMonitor.chiColor;
            bioMonitor.colorsCHI[bioMonitor.currentIndex1][y0] = bioMonitor.chiColor;
            if (vabs(bioMonitor.lastCHI - y0) > 2) {
                dist = bioMonitor.lastCHI + ((bioMonitor.lastCHI > y0) ? -1 : 1);
                down = (bioMonitor.lastCHI > y0);
                while (dist != y0) {
                    if (dist > bioMonitor.ymax || dist < 0) break;

                    bioMonitor.currentColors[dist]                       = bioMonitor.chiColor;
                    bioMonitor.colorsCHI[bioMonitor.currentIndex1][dist] = bioMonitor.chiColor;
                    if (down) dist--;
                    else dist++;
                }
            }
            bioMonitor.lastCHI = y0;
            if (y0 > 0 && y0 < bioMonitor.ymax) { // Increase thickness to 3 pixels
                bioMonitor.currentColors[y0 - 1] = bioMonitor.chiColor;
                bioMonitor.currentColors[y0 + 1] = bioMonitor.chiColor;
                bioMonitor.colorsCHI[bioMonitor.currentIndex1][y0 + 1] = bioMonitor.chiColor;
            }
            break;
        case BIOM_ECG:
            value = inverse_lerp(bioMonitor.min[BIOM_ECG],bioMonitor.max[BIOM_ECG],val);
            y0 = (int)(value * BIOM_GRAPH_H);
            if (y0 > bioMonitor.ymax) y0 = bioMonitor.ymax;
            if (y0 < 0) y0 = 0;
            bioMonitor.currentColors[y0] = bioMonitor.ecgColor;
            bioMonitor.colorsECG[bioMonitor.currentIndex2][y0] = bioMonitor.ecgColor;
            if (vabs(bioMonitor.lastECG - y0) > 2) {
                dist =bioMonitor.lastECG + ((bioMonitor.lastECG > y0) ? -1 : 1);
                down = (bioMonitor.lastECG > y0);
                while (dist != y0) {
                    if (dist > bioMonitor.ymax || dist < 0) break;

                    bioMonitor.currentColors[dist]                       = bioMonitor.ecgColor;
                    bioMonitor.colorsECG[bioMonitor.currentIndex2][dist] = bioMonitor.ecgColor;
                    if (down) dist--;
                    else dist++;
                }
            }
            bioMonitor.lastECG = y0;
            int half = (int)((bioMonitor.max[BIOM_ECG] - bioMonitor.min[BIOM_ECG]) / 2.0f);

            // Increase thickness to 3 pixels
            if (y0 > 0 && y0 < bioMonitor.ymax && (vabs(y0 - half) > 2)) {
                bioMonitor.currentColors[y0 - 1] = bioMonitor.ecgColor;
                bioMonitor.currentColors[y0 + 1] = bioMonitor.ecgColor;
                bioMonitor.colorsECG[bioMonitor.currentIndex2][y0 - 1] = bioMonitor.ecgColor;
                bioMonitor.colorsECG[bioMonitor.currentIndex2][y0 + 1] = bioMonitor.ecgColor;
            }
            break;
    }
}

void BiomonitorEnergyPulse(float take) { Push(0,take); IncrementERG(); Push(0,take); IncrementERG(); }
void BioMonitorUpdate() {
    if (!(World.invP1.hasHardware & HW_BIO) || !(World.invP1.hardwareIsActive & HW_BIO)) return;
    bioMonitor.header = 526; bioMonitor.heartRateText = 527; bioMonitor.bpmText = 529; bioMonitor.fatigueDetailText = 531;
                                                                          bioMonitor.fatigue = 534; // Low
         if (World.invP1.fatigue >= 80.0f)                                bioMonitor.fatigue = 532; // High!
    else if (World.invP1.fatigue <  80.0f && World.invP1.fatigue > 30.0f) bioMonitor.fatigue = 533; // Moderate

    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.heartRate = vfloor((70.0f + ((World.invP1.fatigue / 100.0f) * 110.0f)) * random_range(0.95f,1.05f));
    if (World.invP1.hardwareVersion[HW_BIO_IDX] > 1 && (World.invP1.patchActive & 127)) {
//         bioMonitor.patchesActiveText = Text->stringTable[528]; // TODO actually render text
//         if (World.invP1.patchActive & PATCH_MEDI))     { tempStr.Append(Text->stringTable[520]); tempStr.Append(" "); }
//         if (World.invP1.patchActive & PATCH_STAMINUP)) { tempStr.Append(Text->stringTable[521]); tempStr.Append(" "); }
//         if (World.invP1.patchActive & PATCH_SIGHT))    { tempStr.Append(Text->stringTable[522]); tempStr.Append(" "); }
//         if (World.invP1.patchActive & PATCH_GENIUS))   { tempStr.Append(Text->stringTable[523]); tempStr.Append(" "); }
//         if (World.invP1.patchActive & PATCH_BERSERK))  { tempStr.Append(Text->stringTable[524]); tempStr.Append(" "); }
//         if (World.invP1.patchActive & PATCH_REFLEX))   { tempStr.Append(Text->stringTable[525]); tempStr.Append(" "); }
//         if (World.invP1.patchActive & PATCH_DETOX))    { tempStr.Append(Text->stringTable[530]); }
//         patchEffects.text = tempStr.ToString();
    }

    static const float beatThresh = 0.1f;
    static const float beatVariation = 0.05f;

    // Energy Usage
    bioMonitor.ergValue = (World.invP1.drainJPM / 255.0f);
    if (bioMonitor.ergValue < 0.0f) bioMonitor.ergValue = 0.0f;
    if (bioMonitor.ergValue > 1.0f) bioMonitor.ergValue = 1.0f;

    // Chi Brain Waves
    float brainFactor = 0.15f;
    if (World.invP1.geniusFinished > World.pauseRelativeTime) brainFactor = 0.35f + random_range(-0.3f,0.3f);
    if (Cheats.showFPS) bioMonitor.chiValue = (((float)World.thisFrameTime/16.0f) * 0.5f) - 2.0f;
    else                     bioMonitor.chiValue = (float)(vsinf(World.pauseRelativeTime * 10.0 * (double)brainFactor));

    // ECG: Create shifted sine wave for heart beat.
    // Apply percent fatigued to 200bpm max heart rate with baseline 50bpm.
    float fatigueFactor = ((World.invP1.fatigue / 100.0f) * 120.0f) + 20.0f;
    fatigueFactor = fatigueFactor / 60.0f;
    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.beatFinished = World.pauseRelativeTime + (1.0 / (double)fatigueFactor);
    bioMonitor.beatShift = (bioMonitor.beatFinished - World.pauseRelativeTime) / (1.0 / (double)fatigueFactor);
    if (bioMonitor.beatShift > 0.94f) bioMonitor.ecgValue = vsinf(bioMonitor.beatShift * 35.0f);
    else bioMonitor.ecgValue = 0.0f;

        // Inject variation when beating
    if (bioMonitor.ecgValue > beatThresh || bioMonitor.ecgValue < (beatThresh * -1.0f)) bioMonitor.ecgValue += random_range_i32(-beatVariation,beatVariation);
    if (bioMonitor.tick0Finished < World.pauseRelativeTime) {
        bioMonitor.tick0Finished = World.pauseRelativeTime + bioMonitor.tick0;
        Push(0,bioMonitor.ergValue); IncrementERG();
        Push(0,bioMonitor.ergValue); IncrementERG();
        Push(0,bioMonitor.ergValue);
    }

    if (bioMonitor.tick1Finished < World.pauseRelativeTime) {
        if (Cheats.showFPS) bioMonitor.tick1Finished = World.pauseRelativeTime + bioMonitor.tick2;
        else                     bioMonitor.tick1Finished = World.pauseRelativeTime + bioMonitor.tick1;

        Push(1,bioMonitor.chiValue); IncrementCHI();
        Push(1,bioMonitor.chiValue); IncrementCHI();
        Push(1,bioMonitor.chiValue); IncrementCHI();
        Push(1,bioMonitor.chiValue);
    }

    if (bioMonitor.tick2Finished < World.pauseRelativeTime) {
        bioMonitor.tick2Finished = World.pauseRelativeTime + bioMonitor.tick2;
        Push(2,bioMonitor.ecgValue); IncrementECG();
        Push(2,bioMonitor.ecgValue);
    }

    float distPerc = 1.0f, fadeDist = 100.0f;
    for (int x=0;x<BIOM_GRAPH_W;x++) {
        for (int y=0; y<BIOM_GRAPH_H;y++) {
            bioMonitor.col0 = bioMonitor.colorsERG[x][y]; bioMonitor.col1 = bioMonitor.colorsCHI[x][y]; bioMonitor.col2 = bioMonitor.colorsECG[x][y];
            if (bioMonitor.col0.a > 0.01f) {
                fadeDist = 200.0f;
                distPerc = (bioMonitor.currentIndex0 - x);
                if ((BIOM_GRAPH_W - x) < fadeDist && bioMonitor.currentIndex0 < fadeDist) distPerc += BIOM_GRAPH_W;
                if (distPerc < 0.0f || distPerc > fadeDist) distPerc = fadeDist;
                distPerc = vclamp((fadeDist - distPerc) / fadeDist,0.0f,1.0f);
                if (distPerc == 0.0f) bioMonitor.colorsERG[x][y] = bioMonitor.backgroundColor;
                bioMonitor.col0.a = distPerc;
//                 tex.SetPixel(x,y,bioMonitor.col0); TODO
            } else if (bioMonitor.col1.a > 0.01f) {
                fadeDist = 180.0f;
                distPerc = (bioMonitor.currentIndex1 - x);
                if ((BIOM_GRAPH_W - x) < fadeDist && bioMonitor.currentIndex1 < fadeDist) distPerc += BIOM_GRAPH_W;
                if (distPerc < 0.0f || distPerc > fadeDist) distPerc = fadeDist;
                distPerc = vclamp((fadeDist - distPerc) / fadeDist,0.0f,1.0f);
                if (distPerc == 0.0f) bioMonitor.colorsCHI[x][y] = bioMonitor.backgroundColor;
                bioMonitor.col1.a = distPerc;
//                 tex.SetPixel(x,y,bioMonitor.col1); TODO
            } else if (bioMonitor.col2.a > 0.01f) {
                fadeDist = 275.0f;
                distPerc = (bioMonitor.currentIndex2 - x);
                if ((BIOM_GRAPH_W - x) < fadeDist && bioMonitor.currentIndex2 < fadeDist) distPerc +=  BIOM_GRAPH_W;
                if (distPerc < 0.0f || distPerc > fadeDist) distPerc = fadeDist;
                distPerc = vclamp((fadeDist - distPerc) / fadeDist,0.0f,1.0f);
                if (distPerc == 0.0f) bioMonitor.colorsECG[x][y] = bioMonitor.backgroundColor;
                bioMonitor.col2.a = distPerc;
//                 tex.SetPixel(x,y,bioMonitor.col2); TODO
            } else {
//                 tex.SetPixel(x,y,bioMonitor.backgroundColor); TODO
            }
        }
    }

    if (bioMonitor.tickFinished < World.pauseRelativeTime) {
        bioMonitor.tickFinished = World.pauseRelativeTime + bioMonitor.tick;
        IncrementERG(); IncrementCHI(); IncrementECG();
    }
    
    // TODO actually render texture
//     tex.Apply();
//     OutputTexture.texture = (Texture)tex;
}
