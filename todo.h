#pragma once
#define GEOMETRY_LOD_CARD_MODEL_IDX 178 // Need to specify in gamedata.txt
extern float correctionX, correctionY, correctionZ;
extern float correctionNPCX, correctionNPCY, correctionNPCZ;
extern float correctionDoorsX, correctionDoorsY, correctionDoorsZ;
extern float correctionDynamicsX, correctionDynamicsY, correctionDynamicsZ;
extern float correctionLightsSaveableX, correctionLightsSaveableY, correctionLightsSaveableZ;
extern float correctionStaticImmutableX, correctionStaticImmutableY, correctionStaticImmutableZ;
extern float correctionStaticSaveableX, correctionStaticSaveableY, correctionStaticSaveableZ;
extern float correctionLightX, correctionLightY, correctionLightZ;
void SetUnityHierarchyOffsets(uint8_t curlevel);
void ApplyUnityHierarchyCorrectionAtLevelLoad(uint16_t instanceIdx, uint16_t entIdx);
void EnableCheatArsenal(uint8_t level);
uint16_t SpawnDynamicObject(int val, bool cheat);
void cmd_kill(void);
void cmd_undo(void);
void cmd_shake(void);
float GetPainStatic(void);
Color GetPainStaticColor(void);
void CycleToNextMonitor(GLFWwindow* window);
