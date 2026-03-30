#include "os.h"
#include "voxen.h"
ENGINE_TO_MOD void InitializeEntity(Entity* entry) { // Blank entity, no index yet, for initial list population or temporary Entity.
    entry->index = UINT16_MAX; // memset here would be harmful as only a handful of fields are the same.
    entry->entflags = ENTFLAG_KINEMATIC; // Zeroes the rest out.
    entry->modelIndex = MODEL_IDX_MAX;
    entry->layer = PhysicsLayer_Default;
    entry->texIndex = entry->glowIndex = entry->specIndex = entry->normIndex = MAX_VALID_TEXTURE;
    entry->lodIndex  = MODEL_IDX_MAX;
    entry->camView = 255;
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

ENGINE_TO_MOD int32_t AddLight(Light* lit, LightAnimation* lanim) {
    int32_t i = Sys_Global.loadedLights;
    Sys_Global.loadedLights++;
    if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,Sys_Global.currentLevel); OS_Exit(1); }

    __builtin_memcpy(&lights[i],lit,sizeof(Light));
    __builtin_memcpy(&lanims[i],lanim,sizeof(LightAnimation));
    lightsNewPosition[i] = lit->pos;
    flag_setu32(&lights[i].lflags,LDIRTY,true);
    return i;
}

ENGINE_TO_MOD void TurnLightOff(uint16_t litIdx) {
    if (litIdx >= Sys_Global.loadedLights) return;
    
    flag_setu32(&lights[litIdx].lflags,LIGHTON,false);
}

ENGINE_TO_MOD void LoadFieldIntoLight(char* trimmed_key, char* trimmed_value, char* initialLine, uint32_t lineNum, Light* lit, LightAnimation* lam) {
    char buffer[32];
    for (int i=0;i <32;++i) {
        StringFormat(buffer,sizeof(buffer),"intervalSteps[%d]",i);
        if (StringsEqual(trimmed_key,buffer)) { lam->intervalSteps[i] = parse_float(trimmed_value,initialLine,lineNum); return; }
    }
    
    for (int i=0;i <32;++i) {
        StringFormat(buffer,sizeof(buffer),"intervalStepisLerping[%d]",i);
        if (StringsEqual(trimmed_key,buffer)) { lam->stepIsLerping[i] = parse_float(trimmed_value,initialLine,lineNum); return; }
    }
    
         if (StringsEqual(trimmed_key,"currentStep"))                  lam->currentStep = parse_numberu8(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"lerpValue"))                    lam->lerpValue = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"intervalSteps.Length"))         lam->numIntervalSteps = parse_numberu8(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"intervalStepisLerping.Length")) lam->numLerpSteps = parse_numberu8(trimmed_value,initialLine,lineNum);
    
    else if (StringsEqual(trimmed_key,"localPosition.x")) lit->pos.x = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localPosition.y")) lit->pos.y = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localPosition.z")) lit->pos.z = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.x")) lit->spotDir.x = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.y")) lit->spotDir.y = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.z")) lit->spotDir.z = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.w")) lit->spotDir.w = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"intensity"))    lit->intensity = lit->maxIntensity = parse_float(trimmed_value,initialLine,lineNum) * 0.35f;
    else if (StringsEqual(trimmed_key,"range"))        lit->range = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"spotAngle"))    lit->spotAng = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"type")) {
             if (StringsEqual(trimmed_value,"Spot"))        flag_setu32(&lit->lflags,LSPOT,true);
        else if (StringsEqual(trimmed_value,"Directional")) flag_setu32(&lit->lflags,LDIR,true);
    }
    else if (StringsEqual(trimmed_key,"minIntensity")) lit->minIntensity = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"maxIntensity")) lit->maxIntensity = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.r"))      lit->col.r = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.g"))      lit->col.g = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.b"))      lit->col.b = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"lightOn"))      flag_setu32(&lit->lflags,LIGHTON,parse_bool(trimmed_value,initialLine,lineNum));
    else if (StringsEqual(trimmed_key,"lerpOn"))       flag_setu32(&lit->lflags,LERPON,parse_bool(trimmed_value,initialLine,lineNum));
//     else if (StringsEqual(trimmed_key,"shadows"))      flag_setu32(&lit->lflags,SHADON,trimmed_value[0] != 'N'); // None
}
