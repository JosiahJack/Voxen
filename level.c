#include "os.h"
#include "voxen.h"
ENGINE_TO_MOD void InitializeEntity(Entity* entry) { // Blank entity, no index yet, for initial list population or temporary Entity.
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
    entry->tickTime = 0.35f;
    entry->mass = 1.0f;
    entry->angularDrag = 0.05f;
    entry->dynamicFriction = entry->staticFriction = 0.6f;
    entry->frictionCombine = entry->bounceCombine = PHYS_COMBINE_AVG;
    entry->volume = 1.0f;
    for (int i=0;i<MAX_CHILD_COUNT;++i) {
        entry->child[i] = UINT16_MAX;
        entry->child_offset[i].x = entry->child_offset[i].y = entry->child_offset[i].z = 0.0f;
        entry->child_rotation[i].x = entry->child_rotation[i].y = entry->child_rotation[i].z = 0.0f; entry->child_rotation[i].w = 1.0f;
        entry->child_scale[i].x = entry->child_scale[i].y = entry->child_scale[i].z = 1.0f;
    }
    entry->path[0] = '\0';    
}

ENGINE_TO_MOD void AddInstance(uint16_t entIdx, uint16_t i) {
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
    Sys_Global.instances[i].gravity = Sys_Global.entities[entIdx].gravity >= 0.0f ? Sys_Global.entities[entIdx].gravity : 0.0f; // No up falling.
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
        Sys_Global.loadedLights++;
        if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",Sys_Global.loadedLights,Sys_Global.currentLevel); OS_Exit(1); }

        int32_t litIdx = Sys_Global.loadedLights * LIGHT_DATA_SIZE;
        lightOn[Sys_Global.loadedLights] = true;
        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = 0.7f;
        lightMaxIntensity[Sys_Global.loadedLights] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        lightMinIntensity[Sys_Global.loadedLights] = 0.0f;
        lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light (it's a lantern not a flashlight!)
        lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = 1.85f;
        lights[litIdx + LIGHT_DATA_OFFSET_R] = 0.3531f;
        lights[litIdx + LIGHT_DATA_OFFSET_G] = 0.4837f;
        lights[litIdx + LIGHT_DATA_OFFSET_B] = 0.6509f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSX] = Sys_Global.instances[i].position.x + 0.23f; // TODO Multiply against forward/right!  Only good on first one in medical!
        lights[litIdx + LIGHT_DATA_OFFSET_POSY] = Sys_Global.instances[i].position.y + 0.24f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = Sys_Global.instances[i].position.z;
        lightCastsShadows[Sys_Global.loadedLights] = true;
        lightDirty[Sys_Global.loadedLights] = true;
        
        Sys_Global.loadedLights++;
        if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",Sys_Global.loadedLights,Sys_Global.currentLevel); OS_Exit(1); }

        litIdx = Sys_Global.loadedLights * LIGHT_DATA_SIZE;
        lightOn[Sys_Global.loadedLights] = true;
        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = 1.1165f;
        lightMaxIntensity[Sys_Global.loadedLights] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        lightMinIntensity[Sys_Global.loadedLights] = 0.0f;
        lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light (it's a lantern not a flashlight!)
        lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = 2.0f;
        lights[litIdx + LIGHT_DATA_OFFSET_R] = 0.3561f;
        lights[litIdx + LIGHT_DATA_OFFSET_G] = 0.3561f;
        lights[litIdx + LIGHT_DATA_OFFSET_B] = 0.8970f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSX] = Sys_Global.instances[i].position.x - 0.48f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSY] = Sys_Global.instances[i].position.y - 0.64f;
        lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = Sys_Global.instances[i].position.z;
        lightCastsShadows[Sys_Global.loadedLights] = true;
        lightDirty[Sys_Global.loadedLights] = true;
    }
    
    Sys_Global.instances[i].lockedMessageLingdex = Sys_Global.entities[entIdx].lockedMessageLingdex;
    Sys_Global.dirtyInstances[i] = true;
    Sys_Global.loadedInstances++;
}

void RemoveCameraPosition(uint16_t i);
ENGINE_TO_MOD void DeleteInstance(uint16_t i) {
    if (i <= PLAYER2 || i >= Sys_Global.loadedInstances) return; // Don't delete null ent, player 1, nor player 2 or already empty slots.
    
    if (Sys_Global.instances[i].entflags & ENTFLAG_HAS_CAMERA_VIEW) RemoveCameraPosition(i);
    uint16_t endInstance = vmax(vmin(INSTANCE_COUNT - 1, Sys_Global.loadedInstances - 1),START_INDEX_LEVEL_INSTANCES);
//     for (;i<endInstance;++i) Sys_Global.instances[i] = Sys_Global.instances[i + 1]; // Shift the entire list down, overwriting the entity we're deleting at starting i
    for (;i<endInstance;++i) __builtin_memcpy(&Sys_Global.instances[i], &Sys_Global.instances[i+1], sizeof(Entity));
    --Sys_Global.loadedInstances; // Shift final marker.  It's history!
}

ENGINE_TO_MOD void LoadFieldIntoLight(char* trimmed_key, char* trimmed_value, char* initialLine, uint32_t lineNum, uint32_t lightsIdx, bool* lightOnRead, uint8_t* lightType) {
    int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
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
             if (StringsAreEqual(trimmed_value,"Spot"))        *lightType = 1u;
        else if (StringsAreEqual(trimmed_value,"Directional")) *lightType = 2u;
    }
    else if (StringsAreEqual(trimmed_key,"color.r"))         lights[litIdx + LIGHT_DATA_OFFSET_R] = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsAreEqual(trimmed_key,"color.g"))         lights[litIdx + LIGHT_DATA_OFFSET_G] = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsAreEqual(trimmed_key,"color.b"))         lights[litIdx + LIGHT_DATA_OFFSET_B] = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsAreEqual(trimmed_key,"lightOn") && !lightOnRead) { lightOn[lightsIdx] = parse_bool(trimmed_value,initialLine,lineNum); *lightOnRead = true; } // Check lightOnRead in if here since TargetIO also has same value lightOn, whoops!  But guaranteed to be 2nd so get the real one here
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
//     else if (StringsAreEqual(trimmed_key,"shadows"))         lightCastsShadows[lightsIdx] = trimmed_value[0] != 'N';//'N'one
}

ENGINE_TO_MOD void AddLightFromLoad(bool lightOnRead, int32_t* lightsIdx, uint8_t lightType, uint32_t lineNum) { // Fields already read line by line using function above, so just set needed stuff from level load.
    Sys_Global.loadedLights++;
    if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",*lightsIdx,Sys_Global.currentLevel); OS_Exit(1); }
    
    int32_t litIdx = *lightsIdx * LIGHT_DATA_SIZE;
    lightCastsShadows[*lightsIdx] = true;//(lights[litIdx + LIGHT_DATA_OFFSET_RANGE] >= 0.32f);
    if (!lightOnRead) {
        lightOn[*lightsIdx] = true;
        lightMaxIntensity[*lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
    } else {
        // Dynamic Animated light
        if (lightMinIntensity[*lightsIdx] < 0.01f) lightMinIntensity[*lightsIdx] = 0.01f;
        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[*lightsIdx];
        lightLerpUp[*lightsIdx] = true;
    }

    if (lightType == 1) {
        if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Spotlight %d on line %d loaded with spotAngle less than 5deg\n",*lightsIdx,lineNum+1);
    } else if (lightType == 2) {
        lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 180.0f; // Force to be a directional light
    } else {
        lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
    }
//     if (lightMaxIntensity[*lightsIdx] < 0.16f || lights[litIdx + LIGHT_DATA_OFFSET_RANGE] < 0.32f) { *lightsIdx = *lightsIdx - 1; Sys_Global.loadedLights--; }
}

ENGINE_TO_MOD int32_t AddLight(Vector3 pos, Color col, float range, float intensity, float maxIntensity, float minIntensity, float spotAng, Quaternion spotDir, bool on, bool shadOn) {
    int32_t lightsIdx = Sys_Global.loadedLights;
    Sys_Global.loadedLights++;
    if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",lightsIdx,Sys_Global.currentLevel); OS_Exit(1); }

    int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
    lightOn[lightsIdx] = on;
    lightMinIntensity[lightsIdx] = minIntensity; lightMaxIntensity[lightsIdx] = maxIntensity;
    lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = intensity;
    lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = spotAng;
    lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = range;
    lights[litIdx + LIGHT_DATA_OFFSET_R] = col.r; lights[litIdx + LIGHT_DATA_OFFSET_G] = col.g; lights[litIdx + LIGHT_DATA_OFFSET_B] = col.b;
    lights[litIdx + LIGHT_DATA_OFFSET_POSX] = pos.x; lights[litIdx + LIGHT_DATA_OFFSET_POSY] = pos.y; lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = pos.z;
    lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX] = spotDir.x; lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY] = spotDir.y; lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ] = spotDir.z; lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW] = spotDir.w;
    lightsNewPosition[lightsIdx] = pos;
    lightCastsShadows[lightsIdx] = shadOn;
    lightDirty[lightsIdx] = true;
    return lightsIdx;
}

#define IS_CHANGED(a, b) _Generic((a), float:(vabs((a) - (b)) > 0.0001f), default:((a) != (b)))
#define CHECK_UPDATE(target, value) do { if (IS_CHANGED(target, value)) { (target) = (value); changed = true; }} while(0)
ENGINE_TO_MOD void UpdateLight(uint16_t lightsIdx, Vector3 pos, Color col, float range, float intensity, float maxIntensity, float minIntensity, float spotAng, Quaternion spotDir, bool on, bool shadOn) {
    int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
    bool changed = false;
    CHECK_UPDATE(lightOn[lightsIdx],on);
    lightCastsShadows[lightsIdx] = shadOn;
    lightMinIntensity[lightsIdx] = minIntensity;
    lightMaxIntensity[lightsIdx] = maxIntensity;
    lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = intensity;
    lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = spotAng;
    CHECK_UPDATE(lights[litIdx + LIGHT_DATA_OFFSET_RANGE], range);
    lights[litIdx+LIGHT_DATA_OFFSET_R]=col.r; lights[litIdx+LIGHT_DATA_OFFSET_G]=col.g; lights[litIdx+LIGHT_DATA_OFFSET_B]=col.b;
    CHECK_UPDATE(lights[litIdx + LIGHT_DATA_OFFSET_POSX], pos.x);
    CHECK_UPDATE(lights[litIdx + LIGHT_DATA_OFFSET_POSY], pos.y);
    CHECK_UPDATE(lights[litIdx + LIGHT_DATA_OFFSET_POSZ], pos.z);
    lights[litIdx+LIGHT_DATA_OFFSET_SPOTDIRX]=spotDir.x; lights[litIdx+LIGHT_DATA_OFFSET_SPOTDIRY]=spotDir.y; lights[litIdx+LIGHT_DATA_OFFSET_SPOTDIRZ]=spotDir.z; lights[litIdx+LIGHT_DATA_OFFSET_SPOTDIRW]=spotDir.w;
    if (changed) { lightsNewPosition[lightsIdx]=pos; lightDirty[lightsIdx]=true; }
}

ENGINE_TO_MOD int32_t PosGetCellCoords(float pos_x, float pos_z) { return (PosGetCellCoordZ(pos_z) * WORLDX) + PosGetCellCoordX(pos_x); } // Clamped just above.
void LoadTextures(void); void LoadModels(void);
char* GetNextStringUpToNewlineOrEOF(char* buf, int size, OsFileHandle fd);
void CullInit(void);
void RenderLoadingProgress(int32_t offset, const char* text);
OsFileHandle levelFileHandle;
ENGINE_TO_MOD char* GetLevelFileNextStringUpToNewlineOrEOF(char* buf, int size) { return GetNextStringUpToNewlineOrEOF(buf,size,levelFileHandle); }

void LoadLevel(uint8_t curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true;
    queuedLevelToLoad = 255u; // Reset any loading state that got us here.
    RenderLoadingProgress(100,"Loading level...");
    __builtin_memset(lightMinIntensity,0,LIGHT_COUNT * sizeof(float));
    __builtin_memset(lightMaxIntensity,0,LIGHT_COUNT * sizeof(float));
    __builtin_memset(lightOn,1,LIGHT_COUNT * sizeof(bool)); // Default all on, only off if level data specifies
    __builtin_memset(lightCastsShadows,1,LIGHT_COUNT * sizeof(bool)); // Default all on, only off if level data specifies
    __builtin_memset(lightLerpOn,0,LIGHT_COUNT * sizeof(bool));
    __builtin_memset(lightLerpUp,0,LIGHT_COUNT * sizeof(bool));
    __builtin_memset(lightCurrentStep,0,LIGHT_COUNT * sizeof(uint8_t));
    __builtin_memset(lightLerpValue,0,LIGHT_COUNT * sizeof(float));
    __builtin_memset(lightLerpTime,0,LIGHT_COUNT * sizeof(float));
    __builtin_memset(lightLerpStepTime,0,LIGHT_COUNT * sizeof(float));
    __builtin_memset(lightLerpStartTime,0,LIGHT_COUNT * sizeof(float));
    __builtin_memset(lightIntervalStepsLength,0,LIGHT_COUNT * sizeof(uint8_t));
    __builtin_memset(lightIntervalSteps,0,LIGHT_COUNT * 30 * sizeof(float));
    __builtin_memset(lightIntervalStepIsLerpingLength,0,LIGHT_COUNT * sizeof(uint8_t));
    __builtin_memset(intervalStepisLerping,0,LIGHT_COUNT * 30 * sizeof(float));
    __builtin_memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    char filename[20]; // Minimum size for 0 through 13.
    StringFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    levelFileHandle = OS_OpenReadonly(filename);
    LoadLevelMod(curlevel);
    OS_Close(levelFileHandle);
    for (int i=0;i<Sys_Global.loadedLights;++i) lightMaxIntensity[i] *= 2.0f;
    DualLog("Loaded %d entities, %u static lights for Level %d... took %f secs\n",Sys_Global.loadedInstances,Sys_Global.loadedLights,curlevel,get_time() - start_time);
    DebugRAM("end of LoadLevel instances");
    RenderLoadingProgress(110,"Loading models...");
    LoadModels();
    RenderLoadingProgress(110,"Loading textures...");
    LoadTextures();
    RenderLoadingProgress(110,"Initialize entities...");
    for (int i=PLAYER1;i<Sys_Global.loadedInstances;++i) {        
        int32_t cellIdx = PosGetCellCoords(Sys_Global.instances[i].position.x,Sys_Global.instances[i].position.z);
        Sys_Global.instances[i].cellIndex = cellIdx;
    }
    
    ModInitAfterLoad();
    ResetLevelAudio();
    ResetLevelMusic();
    DualLog("Entity instances initialized after load\n");
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; i++) Sys_Global.dirtyInstances[i] = true;
    for (uint16_t i = 0; i < Sys_Global.loadedLights; i++) {
        uint32_t litIdx = i * LIGHT_DATA_SIZE; // lightDirty[i] = true is already done in PortalCulling, leaving commented out here for confirmation.
        lightsNewPosition[i] = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        lightInPVS[i] = false;
    }
    __builtin_memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, Sys_Global.loadedLights * sizeof(uint32_t)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
}
