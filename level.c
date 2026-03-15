#include "os.h"
#include "voxen.h"
#define FLT_MAX 3.402823466e+38F
extern uint16_t headmountedLanternLight;
extern Vector3 lanternPos;
extern uint16_t editModeSelection;
bool EntNotVisible(uint16_t i, bool otherCondition);

void InitializeEntity(Entity* entry) { // Blank entity, no index yet, for initial list population or temporary Entity.
    entry->index = UINT16_MAX; // memset here would be harmful as only a handful of fields are the same.
    entry->entflags = ENTFLAG_KINEMATIC; // Zeroes the rest out.
    entry->modelIndex = MODEL_IDX_MAX;
    entry->layer = PhysicsLayer_Default;
    flag_set(&entry->entflags, ENTFLAG_ANIMATED, false);
    entry->texIndex = entry->glowIndex = entry->specIndex = entry->normIndex = MAX_VALID_TEXTURE;
    entry->lodIndex  = MODEL_IDX_MAX;
    entry->rotation.x = entry->rotation.y = entry->rotation.z = 0.0f; entry->rotation.w = 1.0f; // Quaternion identity
    entry->scale.x = entry->scale.y = entry->scale.z = 1.0f;
    entry->collider = COLLIDER_TYPE_NONE;
    entry->colliderMeshIndex = MODEL_IDX_MAX;
    entry->mass = 1.0f;
    entry->angularDrag = 0.05f;
    entry->dynamicFriction = entry->staticFriction = 0.6f;
    entry->frictionCombine = entry->bounceCombine = PHYS_COMBINE_AVG;
    entry->volume = 1.0f;
    flag_set(&entry->entflags, ENTFLAG_TEST_PERSISTENT, false);
    for (int i=0;i<MAX_CHILD_COUNT;++i) {
        entry->child[i] = UINT16_MAX;
        entry->child_offset[i].x = entry->child_offset[i].y = entry->child_offset[i].z = 0.0f;
        entry->child_rotation[i].x = entry->child_rotation[i].y = entry->child_rotation[i].z = 0.0f; entry->child_rotation[i].w = 1.0f;
        entry->child_scale[i].x = entry->child_scale[i].y = entry->child_scale[i].z = 1.0f;
    }
    entry->path[0] = '\0';    
}

void ResetLevelAudio(void);
void InitAfterLoad(void) { // Init entities after level load and after already having generic entity type fields set.
    for (int i=PLAYER1;i<Sys_Global.loadedInstances;++i) {        
        int32_t cellIdx = PosGetCellCoords(Sys_Global.instances[i].position.x,Sys_Global.instances[i].position.z);
        Sys_Global.instances[i].cellIndex = cellIdx;
    }
    
    ModInitAfterLoad();
    ResetLevelAudio();
    ResetLevelMusic();
    DualLog("Entity instances initialized after load\n");
}

void AddInstance(uint16_t entIdx, uint16_t i) {
    if (entIdx >= Sys_Global.entityCount) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d\n",entIdx,Sys_Global.entityCount); OS_Exit(1); }
        
    Sys_Global.instances[i].index = entIdx;
    if (ConstIndexIsNPC(entIdx)) InitializeAIAfterLoad(i);
    bool isCardChunk = (Sys_Global.entities[entIdx].entflags & ENTFLAG_CARDCHUNK);
    Sys_Global.instances[i].modelIndex = Sys_Global.entities[entIdx].modelIndex;
    Sys_Global.instances[i].colliderMeshIndex = Sys_Global.entities[entIdx].colliderMeshIndex;
    Sys_Global.instances[i].numclips = Sys_Global.entities[entIdx].numclips;
    Sys_Global.instances[i].animationNum = Sys_Global.entities[entIdx].animationNum;
    Sys_Global.instances[i].texIndex = Sys_Global.entities[entIdx].texIndex;
    Sys_Global.instances[i].glowIndex = Sys_Global.entities[entIdx].glowIndex;
    if (Sys_Global.instances[i].glowIndex >= MAX_VALID_TEXTURE) Sys_Global.instances[i].glowIndex = 0;
    Sys_Global.instances[i].specIndex = Sys_Global.entities[entIdx].specIndex;
    if (Sys_Global.instances[i].specIndex >= MAX_VALID_TEXTURE) Sys_Global.instances[i].specIndex = 0;
    Sys_Global.instances[i].normIndex = Sys_Global.entities[entIdx].normIndex;
    if (Sys_Global.instances[i].normIndex >= MAX_VALID_TEXTURE) Sys_Global.instances[i].normIndex = 0;
    Sys_Global.instances[i].lodIndex = Sys_Global.entities[entIdx].lodIndex;
    flag_set(&Sys_Global.instances[i].entflags, ENTFLAG_CARDCHUNK,  isCardChunk);
    flag_set(&Sys_Global.instances[i].entflags, ENTFLAG_USEGRAVITY,  Sys_Global.entities[entIdx].entflags & ENTFLAG_USEGRAVITY);
    flag_set(&Sys_Global.instances[i].entflags, ENTFLAG_KINEMATIC,  Sys_Global.entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&Sys_Global.instances[i].entflags, ENTFLAG_RIGIDBODY,  Sys_Global.entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    flag_set(&Sys_Global.instances[i].entflags, ENTFLAG_NO_SHADOWS,  Sys_Global.entities[entIdx].entflags & ENTFLAG_NO_SHADOWS);
    Sys_Global.instances[i].collider = Sys_Global.entities[entIdx].collider;
    Sys_Global.instances[i].colliderCenter = Sys_Global.entities[entIdx].colliderCenter;
    Sys_Global.instances[i].colliderSize = Sys_Global.entities[entIdx].colliderSize;
    Sys_Global.instances[i].mass = Sys_Global.entities[entIdx].mass > 0.0f ? Sys_Global.entities[entIdx].mass : 1.0f; // Nonzero fallback.
    Sys_Global.instances[i].linearDrag = Sys_Global.entities[entIdx].linearDrag > 0.0f ? Sys_Global.entities[entIdx].linearDrag : 0.0f;
    Sys_Global.instances[i].angularDrag = Sys_Global.entities[entIdx].angularDrag > 0.0f ? Sys_Global.entities[entIdx].angularDrag : 0.05f;
    for (int c=0;c<MAX_CHILD_COUNT;++c) {
        Sys_Global.instances[i].child[c] = Sys_Global.entities[entIdx].child[c];
        Sys_Global.instances[i].child_offset[c] = Sys_Global.entities[entIdx].child_offset[c];
        Sys_Global.instances[i].child_rotation[c] = Sys_Global.entities[entIdx].child_rotation[c];
        Sys_Global.instances[i].child_scale[c] = isCardChunk ? Sys_Global.entities[entIdx].child_scale[c] : (Vector3){ 1.0f, 1.0f, 1.0f };
    }
    
    if (entIdx == 525) { // prop_console01
        loadedLights++;
        if (loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",loadedLights,Sys_Global.currentLevel); OS_Exit(1); }

        int32_t litIdx = loadedLights * LIGHT_DATA_SIZE;
        lightOn[loadedLights] = true;
        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = 0.7f;
        lightMaxIntensity[loadedLights] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        lightMinIntensity[loadedLights] = 0.0f;
        lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light (it's a lantern not a flashlight!)
        lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = 1.85f;
        lights[litIdx + LIGHT_DATA_OFFSET_R] = 0.3531f;
        lights[litIdx + LIGHT_DATA_OFFSET_G] = 0.4837f;
        lights[litIdx + LIGHT_DATA_OFFSET_B] = 0.6509f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSX] = Sys_Global.instances[i].position.x + 0.23f; // TODO Multiply against forward/right!  Only good on first one in medical!
        lights[litIdx + LIGHT_DATA_OFFSET_POSY] = Sys_Global.instances[i].position.y + 0.24f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = Sys_Global.instances[i].position.z;
        lightCastsShadows[loadedLights] = true;
        lightDirty[loadedLights] = true;
        
        loadedLights++;
        if (loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",loadedLights,Sys_Global.currentLevel); OS_Exit(1); }

        litIdx = loadedLights * LIGHT_DATA_SIZE;
        lightOn[loadedLights] = true;
        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = 1.1165f;
        lightMaxIntensity[loadedLights] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        lightMinIntensity[loadedLights] = 0.0f;
        lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light (it's a lantern not a flashlight!)
        lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = 2.0f;
        lights[litIdx + LIGHT_DATA_OFFSET_R] = 0.3561f;
        lights[litIdx + LIGHT_DATA_OFFSET_G] = 0.3561f;
        lights[litIdx + LIGHT_DATA_OFFSET_B] = 0.8970f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSX] = Sys_Global.instances[i].position.x - 0.48f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSY] = Sys_Global.instances[i].position.y - 0.64f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = Sys_Global.instances[i].position.z;
        lightCastsShadows[loadedLights] = true;
        lightDirty[loadedLights] = true;
    }
    
    Sys_Global.instances[i].lockedMessageLingdex = Sys_Global.entities[entIdx].lockedMessageLingdex;
    dirtyInstances[i] = true;
    Sys_Global.loadedInstances++;
}

void DeleteInstance(uint16_t i) {
    if (i <= PLAYER2 || i >= Sys_Global.loadedInstances) return; // Don't delete null ent, player 1, nor player 2 or already empty slots.
    
    if (Sys_Global.instances[i].entflags & ENTFLAG_HAS_CAMERA_VIEW) RemoveCameraPosition(i);
    uint16_t endInstance = vmax(vmin(INSTANCE_COUNT - 1, Sys_Global.loadedInstances - 1),START_INDEX_LEVEL_INSTANCES);
    for (;i<endInstance;++i) Sys_Global.instances[i] = Sys_Global.instances[i + 1]; // Shift the entire list down, overwriting the entity we're deleting at starting i
    --Sys_Global.loadedInstances; // Shift final marker.  It's history!
}

void CopyInstanceRegion(uint16_t head, uint16_t* instanceTypeArray, Entity* tempInstances, uint16_t* targetIndex, uint16_t nextRegionStart) {
    for (uint16_t modelIdx = 0; modelIdx < MODEL_IDX_MAX; modelIdx++) {
        for (uint16_t j = 0; j < head; j++) {
            uint16_t i = instanceTypeArray[j];
            if (tempInstances[i].modelIndex == modelIdx) {
                if (*targetIndex >= nextRegionStart) { DualLogError("Instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, *targetIndex); OS_Exit(1); }
                
                Sys_Global.instances[*targetIndex] = tempInstances[i];
                (*targetIndex) += 1;
            }
        }
    }
}

#define LINE_LEN_MAX 81920
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
void LoadTextures(void); void LoadModels(void);
char* GetNextStringUpToNewlineOrEOF(char* buf, int size, OsFileHandle fd);
void LoadLevel(uint8_t curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true;
    queuedLevelToLoad = 255u; // Reset any loading state that got us here.
    RenderLoadingProgress(100,"Loading level...");
    if (!Sys_Global.levelCurrentlyLoading) SetMemoryToValueForNBytes(Sys_Global.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    Sys_Global.levelCurrentlyLoading = true;
    Sys_Global.currentLevel = curlevel;
    Sys_Global.loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    loadedLights = 0;
    switch(curlevel) { // Setting these as early as possible. TODO: These are Citadel specific offsets.  Ideally we just determine these from modelBounds of each instance we load later on...
        case 0: worldMin_x = -38.40f + ( 0.00000f +    3.6000f); worldMin_z = -51.20f + (0.0f + 1.0f); break;
        case 1: worldMin_x = -76.80f + ( 0.00000f +   25.5600f); worldMin_z = -56.32f + (0.0f + -5.2f); break;
        case 2: worldMin_x = -40.96f + ( 0.00000f +   -2.6000f); worldMin_z = -46.08f + (0.0f + -7.7f); break;
        case 3: worldMin_x = -53.76f + (50.17400f +  -45.1200f); worldMin_z = -46.08f + (13.714f + -16.32f); break;
        case 4: worldMin_x =  -7.68f + ( 1.17800f +  -20.4000f); worldMin_z = -64.00f + (1.292799f + 11.48f); break;
        case 5: worldMin_x = -35.84f + ( 1.17780f +  -10.1400f); worldMin_z = -51.20f + (-1.2417f + -0.0383f); break;
        case 6: worldMin_x = -64.00f + ( 1.29280f +   -0.6728f); worldMin_z = -71.68f + (-1.2033f + 3.76f); break;
        case 7: worldMin_x = -58.88f + ( 1.24110f +   -6.7000f); worldMin_z = -79.36f + (-1.2544f + 1.16f); break;
        case 8: worldMin_x = -40.96f + (-1.30560f +    1.0800f); worldMin_z = -43.52f + (1.2928f + 0.8f); break;
        case 9: worldMin_x = -51.20f + (-1.34390f +    3.6000f); worldMin_z = -64.0f + (-1.1906f + -1.28f); break;
        case 10:worldMin_x =-128.00f + (-0.90945f +  107.3700f); worldMin_z = -71.68f + (-1.0372f + 35.48f); break;
        case 11:worldMin_x = -38.40f + (-1.26720f +   15.0500f); worldMin_z =  51.2f + (0.96056f + -77.94f); break;
        case 12:worldMin_x = -34.53f + ( 0.00000f +   19.0400f); worldMin_z = -123.74f + (0.0f + 95.8f); break;
    }
    
    // worldMin_x and worldMin_z are the center points of the cells at furthest extents, thus correspond to minimum x or z positions in open cells the player can access.
    worldMin_x -= CELL_SIZE; // Add one cell gap around edges, now they are floating in guaranteed closed cells instead of empty space
    worldMin_z -= CELL_SIZE;
    voxelMinCenterX = worldMin_x + VOXEL_HALF;
    voxelMinCenterZ = worldMin_z + VOXEL_HALF;
    SetMemoryToValueForNBytes(lightMinIntensity,0,LIGHT_COUNT * sizeof(float));
    SetMemoryToValueForNBytes(lightMaxIntensity,0,LIGHT_COUNT * sizeof(float));
    SetMemoryToValueForNBytes(lightOn,1,LIGHT_COUNT * sizeof(bool)); // Default all on, only off if level data specifies
    SetMemoryToValueForNBytes(lightCastsShadows,1,LIGHT_COUNT * sizeof(bool)); // Default all on, only off if level data specifies
    SetMemoryToValueForNBytes(lightLerpOn,0,LIGHT_COUNT * sizeof(bool));
    SetMemoryToValueForNBytes(lightLerpUp,0,LIGHT_COUNT * sizeof(bool));
    SetMemoryToValueForNBytes(lightCurrentStep,0,LIGHT_COUNT * sizeof(uint8_t));
    SetMemoryToValueForNBytes(lightLerpValue,0,LIGHT_COUNT * sizeof(float));
    SetMemoryToValueForNBytes(lightLerpTime,0,LIGHT_COUNT * sizeof(float));
    SetMemoryToValueForNBytes(lightLerpStepTime,0,LIGHT_COUNT * sizeof(float));
    SetMemoryToValueForNBytes(lightLerpStartTime,0,LIGHT_COUNT * sizeof(float));
    SetMemoryToValueForNBytes(lightIntervalStepsLength,0,LIGHT_COUNT * sizeof(uint8_t));
    SetMemoryToValueForNBytes(lightIntervalSteps,0,LIGHT_COUNT * 30 * sizeof(float));
    SetMemoryToValueForNBytes(lightIntervalStepIsLerpingLength,0,LIGHT_COUNT * sizeof(uint8_t));
    SetMemoryToValueForNBytes(intervalStepisLerping,0,LIGHT_COUNT * 30 * sizeof(float));
    if (curlevel >= Sys_Global.numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n", curlevel, Sys_Global.numLevels - 1); OS_Exit(1); }
    
    for (uint16_t idx = START_INDEX_LEVEL_INSTANCES;idx<INSTANCE_COUNT;idx++) { InitializeEntity(&Sys_Global.instances[idx]); dirtyInstances[idx] = true; } // Start AFTER player indices and NULLENT
    SetMemoryToValueForNBytes(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    char filename[20]; // Minimum size for 0 through 13.
    StringFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    OsFileHandle fd = OS_OpenReadonly(filename);
    uint32_t lineNum = 0;
    int32_t instanceIdx = PLAYER2;
    int32_t lightsIdx = -1; // Start at 0 on first loop iteration, -1 here due to ++ positioning, as it needs to iterate before each blank or commented line skip
    char lineSpace[LINE_LEN_MAX];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[LINE_LEN_MAX];
    while (GetNextStringUpToNewlineOrEOF(lineSpace,LINE_LEN_MAX,fd)) {
        size_t len = GetStringLength(lineSpace);
        while (len && (lineSpace[len - 1] == '\n' || lineSpace[len - 1] == '\r'))
        lineSpace[--len] = '\0';
        line = lineSpace;
        StringFormat(initialLine, sizeof(initialLine), "%s", line);
        CopyMemoryFromBtoAForNBytes(firstKeyCheck,line,10); firstKeyCheck[10] = '\0';
        lineNum++;
        bool isLight = true;
        if (StringsAreEqual(firstKeyCheck, "constIndex")) isLight = false;  // constIndex specified indicating this is a real entity?
        if (isLight) {
            lightsIdx++;
            if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",lightsIdx,curlevel); OS_Exit(1); }
        } else {
            instanceIdx++;
            if (instanceIdx >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n",instanceIdx,curlevel); OS_Exit(1); }
        }
        
        int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
        uint8_t lightType = 0u; // Point
        bool lightOnRead = false;
        bool activeStateRead = false;
        while(line[0] != '\0') { // Guaranteed no leading whitespaces,k comments, or blank lines, so don't bother
            char* pipe = StringFindFirstCharWithin(line,'|');
            char* kvString = line; // key:value pair before the pipe as a string
            if (pipe) {
                *pipe = '\0';          // Split string at the pipe
                line = pipe + 1;       // Point to rest of the line after the pipe
            } else { // Else this is the last string after the last pipe with last kv pair
                line += GetStringLength(line);
            }
           
            if (kvString[0] == '\0' || StringFindFirstCharWithin(kvString, ':') == NULL) continue;
            
            char *colon = StringFindFirstCharWithin(kvString, ':');
            if (colon[1] == '\0') continue; // Don't care about the name.  Need to skip this in the middle, but this also handles the very end
            
            *colon = '\0';           // Split string at the colon
            char *key = kvString;    // Assign key to before colon
            char *value = colon + 1; // Assing value to after colon
            if (!key) { DualLogError("Invalid key-value pair at line %u (as viewed by text editor): %s\n", lineNum, initialLine); OS_Exit(1); }

            char trimmed_key[64];
            char trimmed_value[256];
            StringFormat(trimmed_key, sizeof(trimmed_key), "%s", key);
            StringFormat(trimmed_value, sizeof(trimmed_value), "%s", value);
            trimmed_key[sizeof(trimmed_key) - 1] = '\0';
            trimmed_value[sizeof(trimmed_value) - 1] = '\0';
            if (isLight) {
                     if (StringsAreEqual(trimmed_key,"localPosition.x")) lights[litIdx + LIGHT_DATA_OFFSET_POSX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localPosition.y")) lights[litIdx + LIGHT_DATA_OFFSET_POSY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localPosition.z")) lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.x")) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.y")) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.z")) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.w")) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intensity"))       lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = parse_float(trimmed_value, initialLine, lineNum) * 0.35f;
                else if (StringsAreEqual(trimmed_key,"range"))           lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"spotAngle"))       lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"type")) {
                         if (StringsAreEqual(trimmed_value,"Spot"))        lightType = 1u;
                    else if (StringsAreEqual(trimmed_value,"Directional")) lightType = 2u;
                }
                else if (StringsAreEqual(trimmed_key,"color.r"))         lights[litIdx + LIGHT_DATA_OFFSET_R] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"color.g"))         lights[litIdx + LIGHT_DATA_OFFSET_G] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"color.b"))         lights[litIdx + LIGHT_DATA_OFFSET_B] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"lightOn") && !lightOnRead) { lightOn[lightsIdx] = parse_bool(trimmed_value,initialLine,lineNum); lightOnRead = true; } // Check lightOnRead in if here since TargetIO also has same value lightOn, whoops!  But guaranteed to be 2nd so get the real one here
                else if (StringsAreEqual(trimmed_key,"lerpOn"))          lightLerpOn[lightsIdx] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"currentStep"))     lightCurrentStep[lightsIdx] = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"lerpValue"))       lightLerpValue[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps.Length")) lightIntervalStepsLength[lightsIdx] = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[0]"))     lightIntervalSteps[lightsIdx][0] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[1]"))     lightIntervalSteps[lightsIdx][1] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[2]"))     lightIntervalSteps[lightsIdx][2] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[3]"))     lightIntervalSteps[lightsIdx][3] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[4]"))     lightIntervalSteps[lightsIdx][4] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[5]"))     lightIntervalSteps[lightsIdx][5] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[6]"))     lightIntervalSteps[lightsIdx][6] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[7]"))     lightIntervalSteps[lightsIdx][7] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[8]"))     lightIntervalSteps[lightsIdx][8] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[9]"))     lightIntervalSteps[lightsIdx][9] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[10]"))    lightIntervalSteps[lightsIdx][10] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[11]"))    lightIntervalSteps[lightsIdx][11] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[12]"))    lightIntervalSteps[lightsIdx][12] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[13]"))    lightIntervalSteps[lightsIdx][13] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[14]"))    lightIntervalSteps[lightsIdx][14] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[15]"))    lightIntervalSteps[lightsIdx][15] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[16]"))    lightIntervalSteps[lightsIdx][16] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[17]"))    lightIntervalSteps[lightsIdx][17] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[18]"))    lightIntervalSteps[lightsIdx][18] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[19]"))    lightIntervalSteps[lightsIdx][19] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[20]"))    lightIntervalSteps[lightsIdx][20]= parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[21]"))    lightIntervalSteps[lightsIdx][21] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[22]"))    lightIntervalSteps[lightsIdx][22] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[23]"))    lightIntervalSteps[lightsIdx][23] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[24]"))    lightIntervalSteps[lightsIdx][24] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[25]"))    lightIntervalSteps[lightsIdx][25] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[26]"))    lightIntervalSteps[lightsIdx][26] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[27]"))    lightIntervalSteps[lightsIdx][27] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[28]"))    lightIntervalSteps[lightsIdx][28] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalSteps[29]"))    lightIntervalSteps[lightsIdx][29] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping.Length")) lightIntervalStepIsLerpingLength[lightsIdx] = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[0]"))     intervalStepisLerping[lightsIdx][0] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[1]"))     intervalStepisLerping[lightsIdx][1] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[2]"))     intervalStepisLerping[lightsIdx][2] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[3]"))     intervalStepisLerping[lightsIdx][3] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[4]"))     intervalStepisLerping[lightsIdx][4] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[5]"))     intervalStepisLerping[lightsIdx][5] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[6]"))     intervalStepisLerping[lightsIdx][6] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[7]"))     intervalStepisLerping[lightsIdx][7] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[8]"))     intervalStepisLerping[lightsIdx][8] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[9]"))     intervalStepisLerping[lightsIdx][9] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[10]"))    intervalStepisLerping[lightsIdx][10] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[11]"))    intervalStepisLerping[lightsIdx][11] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[12]"))    intervalStepisLerping[lightsIdx][12] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[13]"))    intervalStepisLerping[lightsIdx][13] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[14]"))    intervalStepisLerping[lightsIdx][14] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[15]"))    intervalStepisLerping[lightsIdx][15] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[16]"))    intervalStepisLerping[lightsIdx][16] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[17]"))    intervalStepisLerping[lightsIdx][17] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[18]"))    intervalStepisLerping[lightsIdx][18] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[19]"))    intervalStepisLerping[lightsIdx][19] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[20]"))    intervalStepisLerping[lightsIdx][20] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[21]"))    intervalStepisLerping[lightsIdx][21] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[22]"))    intervalStepisLerping[lightsIdx][22] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[23]"))    intervalStepisLerping[lightsIdx][23] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[24]"))    intervalStepisLerping[lightsIdx][24] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[25]"))    intervalStepisLerping[lightsIdx][25] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[26]"))    intervalStepisLerping[lightsIdx][26] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[27]"))    intervalStepisLerping[lightsIdx][27] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[28]"))    intervalStepisLerping[lightsIdx][28] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"intervalStepisLerping[29]"))    intervalStepisLerping[lightsIdx][29] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"minIntensity"))    lightMinIntensity[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"maxIntensity"))    lightMaxIntensity[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
            } else {
                     if (StringsAreEqual(trimmed_key,"constIndex"))      Sys_Global.instances[instanceIdx].index = parse_numberu16(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localPosition.x")) Sys_Global.instances[instanceIdx].position.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localPosition.y")) Sys_Global.instances[instanceIdx].position.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localPosition.z")) Sys_Global.instances[instanceIdx].position.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.x")) Sys_Global.instances[instanceIdx].rotation.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.y")) Sys_Global.instances[instanceIdx].rotation.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.z")) Sys_Global.instances[instanceIdx].rotation.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localRotation.w")) Sys_Global.instances[instanceIdx].rotation.w = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localScale.x"))    Sys_Global.instances[instanceIdx].scale.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localScale.y"))    Sys_Global.instances[instanceIdx].scale.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"localScale.z"))    Sys_Global.instances[instanceIdx].scale.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"go.activeSelf")) { activeStateRead = true ; flag_set(&Sys_Global.instances[instanceIdx].entflags, ENTFLAG_ACTIVE, parse_bool(trimmed_value, initialLine, lineNum)); }
                else if (StringsAreEqual(trimmed_key,"requireReset"))    flag_set(&Sys_Global.instances[instanceIdx].entflags, ENTFLAG_REQUIRE_RESET, parse_bool(trimmed_value, initialLine, lineNum));
                else if (StringsAreEqual(trimmed_key,"amount"))          Sys_Global.instances[instanceIdx].amount = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"resetTime"))       Sys_Global.instances[instanceIdx].resetTime = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"minSecurityLevel"))Sys_Global.instances[instanceIdx].minSecurityLevel = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsAreEqual(trimmed_key,"damageOnUse"))     flag_set(&Sys_Global.instances[instanceIdx].entflags, ENTFLAG_DAMAGE_ON_USE, parse_bool(trimmed_value, initialLine, lineNum));
                else if (StringsAreEqual(trimmed_key,"target"))          StringCopyInto_A_From_B(Sys_Global.instances[instanceIdx].target,trimmed_value,TARGET_STRING_LENGTH);
                else if (StringsAreEqual(trimmed_key,"targetname"))      StringCopyInto_A_From_B(Sys_Global.instances[instanceIdx].targetname,trimmed_value,TARGET_STRING_LENGTH);
            }
        }
        
        if (isLight) {
            loadedLights++;
            if (!lightOnRead) {
                lightOn[lightsIdx] = true;
                lightMaxIntensity[lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            } else {
                // Dynamic Animated light
                if (lightMinIntensity[lightsIdx] < 0.01f) lightMinIntensity[lightsIdx] = 0.01f;
                lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[lightsIdx];
                lightLerpUp[lightsIdx] = true;
            }

            if (lightMaxIntensity[lightsIdx] < 0.16f || lights[litIdx + LIGHT_DATA_OFFSET_RANGE] < 0.32f) { lightsIdx--; loadedLights--; }
            lightCastsShadows[lightsIdx] = (lights[litIdx + LIGHT_DATA_OFFSET_RANGE] >= 0.32f) && (lightType == 0);
            if (lightType == 1) {
                if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Spotlight %d on line %d loaded with spotAngle less than 5deg\n",lightsIdx,lineNum+1);
            } else if (lightType == 2) {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 180.0f; // Force to be a directional light
            } else {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            }
        } else {
            uint16_t parent = instanceIdx; // Needed as adding children moves the instanceIdx.
            uint16_t entIdx = Sys_Global.instances[parent].index;
//             if (Sys_Global.instances[parent].index == 766) {
//                 editModeSelection = parent;
//                 flag_set(&Sys_Global.instances[editModeSelection].entflags,ENTFLAG_ACTIVE,true);
//                 //Sys_Global.instances[editModeSelection].scale = (Vector3){1.0f,1.0f,1.0f};
//             }
            AddInstance(entIdx, parent);
            if (!activeStateRead) flag_set(&Sys_Global.instances[parent].entflags, ENTFLAG_ACTIVE, true);
            if (EntityIndexIsPortalBlockingDoor(entIdx)) {
                float nudgeAmount = entIdx == 499 || entIdx == 509 ? 3.84f : 0.32f; // Bulkhead and giant elevator door need to nudge further to be sure.
                Sys_Global.instances[parent].portalIndex = numActivePortals;
                bool isOpen = (Sys_Global.instances[parent].doorState != DoorState_Closed); // Allows for any of DoorState_Open, DoorState_Opening, or DoorState_Closing to be considered open as far as portals are concerned so we can draw objects between the door panels.
                float obj_x = Sys_Global.instances[parent].position.x;
                float obj_z = Sys_Global.instances[parent].position.z;
                uint16_t cellIndexCurrentX = PosGetCellCoordX(obj_x);
                uint16_t cellIndexCurrentZ = PosGetCellCoordZ(obj_z);
                uint16_t cellCurrent = (cellIndexCurrentZ * WORLDX) + cellIndexCurrentX;
                uint16_t cellIndexUp = PosGetCellCoordZ(obj_z + nudgeAmount);
                uint16_t cellIndexDn = PosGetCellCoordZ(obj_z - nudgeAmount);
                uint16_t cellIndexRight = PosGetCellCoordX(obj_x + nudgeAmount);
                uint16_t cellIndexLeft = PosGetCellCoordX(obj_x - nudgeAmount);
                uint16_t cellN_idx = PosGetCellCoords(obj_x, obj_z + nudgeAmount);
                uint16_t cellE_idx = PosGetCellCoords(obj_x + nudgeAmount, obj_z);
                uint16_t cellS_idx = PosGetCellCoords(obj_x, obj_z - nudgeAmount);
                uint16_t cellW_idx = PosGetCellCoords(obj_x - nudgeAmount, obj_z); // Don't actually need to check it.
                bool isNS = (cellN_idx != cellCurrent || cellS_idx != cellCurrent);
                if (isNS) { // Portal is a North
                            //             South pair
                    PortalCell cellN, cellS;
                    cellN.x = cellS.x = PosGetCellCoordX(obj_x);
                    cellN.z = (cellN_idx != cellCurrent) ? cellIndexUp : cellIndexCurrentZ; // Ensure that cellA is always the north cell of the pair
                    cellS.z = (cellS_idx != cellCurrent) ? cellIndexDn : cellIndexCurrentZ;
                    activePortals[numActivePortals] = (Portal){ .cellA = cellN, .cellB = cellS, .portalNS = true, .open = isOpen, .dirty = true };
                } else { // Portal is an East-West pair
                    PortalCell cellE, cellW;
                    cellE.z = cellW.z = PosGetCellCoordZ(obj_z);
                    cellE.x = (cellE_idx != cellCurrent) ? cellIndexRight : cellIndexCurrentX; // Ensure that cellA is always the east cell of the pair
                    cellW.x = (cellW_idx != cellCurrent) ? cellIndexLeft : cellIndexCurrentX;
                    activePortals[numActivePortals] = (Portal){ .cellA = cellE, .cellB = cellW, .portalNS = false, .open = isOpen, .dirty = true };
                }
                
                numActivePortals++;
            }
            
            for (int i=0;i<MAX_CHILD_COUNT;++i) {
                if (Sys_Global.instances[parent].child[i] < Sys_Global.entityCount) {
                    if (Sys_Global.entities[entIdx].child[i] != UINT16_MAX) { // Add child
                        instanceIdx++; // Increment head of the list an extra time for the child entity.
                        AddInstance(Sys_Global.entities[entIdx].child[i], instanceIdx);
                        Sys_Global.instances[instanceIdx].index = Sys_Global.entities[entIdx].child[i];
                        Sys_Global.instances[instanceIdx].position.x = Sys_Global.instances[parent].position.x + Sys_Global.entities[entIdx].child_offset[i].x;
                        Sys_Global.instances[instanceIdx].position.y = Sys_Global.instances[parent].position.y + Sys_Global.entities[entIdx].child_offset[i].y;
                        Sys_Global.instances[instanceIdx].position.z = Sys_Global.instances[parent].position.z + Sys_Global.entities[entIdx].child_offset[i].z;
                        Sys_Global.instances[instanceIdx].scale.x = Sys_Global.instances[parent].scale.x * Sys_Global.entities[entIdx].child_scale[i].x;
                        Sys_Global.instances[instanceIdx].scale.y = Sys_Global.instances[parent].scale.y * Sys_Global.entities[entIdx].child_scale[i].y;
                        Sys_Global.instances[instanceIdx].scale.z = Sys_Global.instances[parent].scale.z * Sys_Global.entities[entIdx].child_scale[i].z;
                    }
                }
            }
        }
    }
    
    OS_Close(fd);

    // Add instances for shield generators
    if (curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7) {
        instanceIdx++;
        AddInstance(754, instanceIdx);
        Sys_Global.instances[instanceIdx].position = (Vector3){ -51.30664f, -47.42f, 56.42651f };
        Sys_Global.instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 0 45
        instanceIdx++;
        AddInstance(754, instanceIdx);
        Sys_Global.instances[instanceIdx].position = (Vector3){ 71.5f, -47.42f, -66.6f };
        Sys_Global.instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 180 45
        instanceIdx++;
        AddInstance(754, instanceIdx);
        Sys_Global.instances[instanceIdx].position = (Vector3){ -51.306650f, -47.42f, -66.66652f };
        Sys_Global.instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 0 -45
        instanceIdx++;
        AddInstance(754, instanceIdx);
        Sys_Global.instances[instanceIdx].position = (Vector3){ 71.78664f, -47.42f, 56.42651f };
        Sys_Global.instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 180 -45
        instanceIdx++;
    }
        
    // Add player headmounted lantern light
    loadedLights++;
    lightsIdx++;
    DualLog("lightsIdx %u vs loadedLights %u\n",lightsIdx,loadedLights);
    if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",lightsIdx,curlevel); OS_Exit(1); }

    int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
    headmountedLanternLight = lightsIdx;
    lightOn[lightsIdx] = false;
    lightMaxIntensity[lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
    lightMinIntensity[lightsIdx] = 0.0f;
    lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light (it's a lantern not a flashlight!)
    lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = 11.52f;
    lights[litIdx + LIGHT_DATA_OFFSET_R] = 1.0f;
    lights[litIdx + LIGHT_DATA_OFFSET_G] = 1.0f;
    lights[litIdx + LIGHT_DATA_OFFSET_B] = 1.0f;
    lanternPos = Sys_Global.instances[PLAYER1].position;
    lights[litIdx + LIGHT_DATA_OFFSET_POSX] = lanternPos.x;
    lights[litIdx + LIGHT_DATA_OFFSET_POSY] = lanternPos.y;
    lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = lanternPos.z;
    lightCastsShadows[lightsIdx] = true;
    lightDirty[lightsIdx] = true;
    
    // Set Fog
    switch(curlevel) {
        case  0: fogColorR = 0.3207547f;  fogColorG = 0.29200783f;  fogColorB = 0.29200783f;  fogBaseDensityForLevel = 0.07f;  break;
        case  1: fogColorR = 0.34509805f; fogColorG = 0.38431373f;  fogColorB = 0.49019608f;  fogBaseDensityForLevel = 0.055f; break;
        case  2: fogColorR = 0.47058824f; fogColorG = 0.3882353f;   fogColorB = 0.3928334f;   fogBaseDensityForLevel = 0.05f;  break;
        case  3: fogColorR = 0.32941177f; fogColorG = 0.29411766f;  fogColorB = 0.2509804f;   fogBaseDensityForLevel = 0.065f; break;
        case  4: fogColorR = 0.3882353f;  fogColorG = 0.452415f;    fogColorB = 0.47058824f;  fogBaseDensityForLevel = 0.075f; break;
        case  5: fogColorR = 0.3882353f;  fogColorG = 0.4117647f;   fogColorB = 0.47058824f;  fogBaseDensityForLevel = 0.03f;  break;
        case  6: fogColorR = 0.3f;        fogColorG = 0.24f;        fogColorB = 0.33f;        fogBaseDensityForLevel = 0.07f;  break;
        case  7: fogColorR = 0.38679248f; fogColorG = 0.3471719f;   fogColorB = 0.3302332f;   fogBaseDensityForLevel = 0.07f;  break;
        case  8: fogColorR = 0.44708973f; fogColorG = 0.45681614f;  fogColorB = 0.4811321f;   fogBaseDensityForLevel = 0.04f;  break;
        case  9: fogColorR = 0.4056604f;  fogColorG = 0.3992963f;   fogColorB = 0.36930403f;  fogBaseDensityForLevel = 0.05f;  break;
        case 10: fogColorR = 0.48235294f; fogColorG = 0.58431375f;  fogColorB = 0.5176471f;   fogBaseDensityForLevel = 0.04f;  break;
        case 11: fogColorR = 0.52872473f; fogColorG = 0.58431375f;  fogColorB = 0.48235294f;  fogBaseDensityForLevel = 0.04f;  break;
        case 12: fogColorR = 0.48235294f; fogColorG = 0.58431375f;  fogColorB = 0.5176471f;   fogBaseDensityForLevel = 0.05f;  break;
        case 13: fogColorR = 0.0f;        fogColorG = 0.0f;         fogColorB = 0.0f;         fogBaseDensityForLevel = 0.005f; break;
    }

    fogBaseDensityForLevel *= 3.8f; // Global modifier to tweak it.
    DualLog("Loaded %d entities, %u static lights, %u doors for Level %d... took %f secs\n", Sys_Global.loadedInstances, loadedLights, numActivePortals, curlevel, get_time() - start_time);
    DebugRAM("end of LoadLevel instances");
    RenderLoadingProgress(110,"Loading models...");
    LoadModels();
    RenderLoadingProgress(110,"Loading textures...");
    LoadTextures();
    RenderLoadingProgress(110,"Initialize entities...");
    InitAfterLoad();
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; i++) dirtyInstances[i] = true;
    for (uint16_t i = 0; i < loadedLights; i++) {
        uint32_t litIdx = i * LIGHT_DATA_SIZE; // lightDirty[i] = true is already done in PortalCulling, leaving commented out here for confirmation.
        lightsNewPosition[i] = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        lightInPVS[i] = false;
    }
    SetMemoryToValueForNBytes(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
}
#pragma GCC diagnostic pop
