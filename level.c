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

ENGINE_TO_MOD void AddInstance(uint16_t entIdx, uint16_t i) {
    if (entIdx >= Sys_Global.entityCount) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d\n",entIdx,Sys_Global.entityCount); OS_Exit(1); }
    
    Entity* e = &Sys_Global.instances[i];
    e->index = entIdx;
    if (ConstIndexIsNPC(entIdx)) InitializeAIAfterLoad(i);
    bool isCardChunk = (Sys_Global.entities[entIdx].entflags & ENTFLAG_CARDCHUNK);
    e->modelIndex = Sys_Global.entities[entIdx].modelIndex;
    e->colliderMeshIndex = Sys_Global.entities[entIdx].colliderMeshIndex;
    e->numclips = Sys_Global.entities[entIdx].numclips;
    e->animationNum = Sys_Global.entities[entIdx].animationNum;
    e->texIndex = Sys_Global.entities[entIdx].texIndex;
    e->glowIndex = Sys_Global.entities[entIdx].glowIndex >= MAX_VALID_TEXTURE ? 0 : Sys_Global.entities[entIdx].glowIndex;
    e->specIndex = Sys_Global.entities[entIdx].specIndex >= MAX_VALID_TEXTURE ? 0 : Sys_Global.entities[entIdx].specIndex;
    e->normIndex = Sys_Global.entities[entIdx].normIndex >= MAX_VALID_TEXTURE ? 0 : Sys_Global.entities[entIdx].normIndex;
    e->lodIndex = Sys_Global.entities[entIdx].lodIndex;
    flag_set(&e->entflags,ENTFLAG_CARDCHUNK,isCardChunk);
    e->gravity = Sys_Global.entities[entIdx].gravity >= 0.0f ? Sys_Global.entities[entIdx].gravity : 0.0f; // No up falling.
    flag_set(&e->entflags,ENTFLAG_KINEMATIC,Sys_Global.entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&e->entflags,ENTFLAG_RIGIDBODY,Sys_Global.entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    flag_set(&e->entflags,ENTFLAG_NO_SHADOWS, Sys_Global.entities[entIdx].entflags & ENTFLAG_NO_SHADOWS);
    e->collider = Sys_Global.entities[entIdx].collider;
    e->colliderCenter = Sys_Global.entities[entIdx].colliderCenter;
    e->colliderSize = Sys_Global.entities[entIdx].colliderSize;
    e->mass = Sys_Global.entities[entIdx].mass > 0.0f ? Sys_Global.entities[entIdx].mass : 1.0f; // Nonzero fallback.
    e->linearDrag = Sys_Global.entities[entIdx].linearDrag > 0.0f ? Sys_Global.entities[entIdx].linearDrag : 0.0f;
    e->angularDrag = Sys_Global.entities[entIdx].angularDrag > 0.0f ? Sys_Global.entities[entIdx].angularDrag : 0.05f;
    for (int c=0;c<MAX_CHILD_COUNT;++c) {
        e->child[c] = Sys_Global.entities[entIdx].child[c];
        e->child_offset[c] = Sys_Global.entities[entIdx].child_offset[c];
        e->child_rotation[c] = Sys_Global.entities[entIdx].child_rotation[c];
        e->child_scale[c] = isCardChunk ? Sys_Global.entities[entIdx].child_scale[c] : (Vector3){ 1.0f, 1.0f, 1.0f };
    }
    
    if (entIdx == 525) { // prop_console01
        // TODO position with forward/right taken into account
        Light blueLight1 = (Light){.pos=(Vector3){e->position.x+0.23f,e->position.y+0.24f,e->position.z},.col=(Color3){0.3531f,0.4837f,0.6509f},.range=1.85f,.intensity=0.7f,.maxIntensity=0.7f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
        Light blueLight2 = (Light){.pos=(Vector3){e->position.x-0.48f,e->position.y-0.64f,e->position.z},.col=(Color3){0.3561f,0.3561f,0.8970f},.range=2.0f,.intensity=1.1165f,.maxIntensity=1.1165f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
        LightAnimation lam={0};
        AddLight(&blueLight1,&lam);
        AddLight(&blueLight2,&lam);
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
    else if (StringsEqual(trimmed_key,"lerpValue"))                    lam->lerpValue = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"intervalSteps.Length"))         lam->numIntervalSteps = parse_numberu8(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"intervalStepisLerping.Length")) lam->numLerpSteps = parse_numberu8(trimmed_value,initialLine,lineNum);
    
    else if (StringsEqual(trimmed_key,"localPosition.x")) lit->pos.x = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"localPosition.y")) lit->pos.y = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"localPosition.z")) lit->pos.z = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.x")) lit->spotDir.x = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.y")) lit->spotDir.y = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.z")) lit->spotDir.z = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.w")) lit->spotDir.w = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"intensity"))    lit->intensity = lit->maxIntensity = parse_float(trimmed_value, initialLine, lineNum) * 0.35f;
    else if (StringsEqual(trimmed_key,"range"))        lit->range = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"spotAngle"))    lit->spotAng = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"type")) {
             if (StringsEqual(trimmed_value,"Spot"))        flag_setu32(&lit->lflags,LSPOT,true);
        else if (StringsEqual(trimmed_value,"Directional")) flag_setu32(&lit->lflags,LDIR,true);
    }
    else if (StringsEqual(trimmed_key,"minIntensity")) lit->minIntensity = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"maxIntensity")) lit->maxIntensity = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.r"))      lit->col.r = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"color.g"))      lit->col.g = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"color.b"))      lit->col.b = parse_float(trimmed_value, initialLine, lineNum);
//     else if (StringsEqual(trimmed_key,"lightOn"))      flag_setu32(&lit->lflags,LIGHTON,parse_bool(trimmed_value,initialLine,lineNum));
    else if (StringsEqual(trimmed_key,"lerpOn"))       flag_setu32(&lit->lflags,LERPON,parse_bool(trimmed_value,initialLine,lineNum));
//  else if (StringsEqual(trimmed_key,"shadows"))      flag_setu32(&lit->lflags,SHADON,trimmed_value[0] != 'N'); // None
}

#define IS_CHANGED(a, b) _Generic((a), float:(vabs((a) - (b)) > 0.0001f), default:((a) != (b)))
#define CHECK_UPDATE(target, value) do { if (IS_CHANGED(target, value)) { (target) = (value); changed = true; }} while(0)
ENGINE_TO_MOD void UpdateLight(uint16_t i, Vector3 pos, Color3 col, float range, float intensity, float maxIntensity, float minIntensity, float spotAng, Quaternion spotDir, bool on, bool shadOn) {
    bool changed = false;
    if ((lights[i].lflags & SHADON) - shadOn) changed = true;
    if ((lights[i].lflags & LIGHTON) - on) changed = true;
    flag_setu32(&lights[i].lflags,SHADON,shadOn);
    flag_setu32(&lights[i].lflags,LIGHTON,on);
    lights[i].intensity=intensity; lights[i].minIntensity=minIntensity; lights[i].maxIntensity=maxIntensity; lights[i].spotAng=spotAng;
    CHECK_UPDATE(lights[i].range,range);
    lights[i].col=col;
    CHECK_UPDATE(lights[i].pos.x,pos.x);
    CHECK_UPDATE(lights[i].pos.y,pos.y);
    CHECK_UPDATE(lights[i].pos.z,pos.z);
    lights[i].spotDir = spotDir;
    if (changed) { lightsNewPosition[i]=pos; flag_setu32(&lights[i].lflags,LDIRTY,true); }
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
    __builtin_memset(lights,0,LIGHT_COUNT * sizeof(Light));
    __builtin_memset(lanims,0,LIGHT_COUNT * sizeof(LightAnimation));
    __builtin_memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    char filename[20]; // Minimum size for 0 through 13.
    StringFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    levelFileHandle = OS_OpenReadonly(filename);
    LoadLevelMod(curlevel);
    OS_Close(levelFileHandle);
    for (int i=0;i<Sys_Global.loadedLights;++i) {/* lights[i].maxIntensity *= 2.0f; */lightsNewPosition[i]=lights[i].pos; }
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
    for (uint16_t i = 0; i < Sys_Global.loadedLights; i++) { lightsNewPosition[i] = lights[i].pos; lightInPVS[i] = false; }
    __builtin_memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, Sys_Global.loadedLights * sizeof(uint32_t)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
}
