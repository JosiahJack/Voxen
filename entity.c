#include "os.h"
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "entity.h"
#include "voxen.h"
#include "vmath.h"
#include "todo.h"

//#define DEBUG_ENTITIES
Entity entities[MAX_ENTITIES]; // Global array of entity definitions
char entityNames[MAX_ENTITIES][34];
int32_t entityCount = 0;            // Number of entities loaded
DataParser entity_parser;
uint16_t invalidModelIndexCount;
uint16_t* modelTypeCountsOpaque = NULL;
uint16_t* modelTypeCountsDoubleSided = NULL;
uint16_t* modelTypeCountsTransparent = NULL;
uint16_t* modelTypeOffsetsOpaque = NULL;
uint16_t* modelTypeOffsetsDoubleSided = NULL;
uint16_t* modelTypeOffsetsTransparent = NULL;
uint16_t opaqueInstancesHead = 0;
uint16_t renderableCount = 0;
uint16_t loadedInstances = 0;
uint16_t startOfDoubleSidedInstances = INSTANCE_COUNT - 1;
uint16_t startOfTransparentInstances = INSTANCE_COUNT - 1;
uint16_t doubleSidedInstancesHead = 0;
uint16_t transparentInstancesHead = 0;
float correctionX, correctionY, correctionZ;
float correctionNPCX, correctionNPCY, correctionNPCZ;
float correctionDoorsX, correctionDoorsY, correctionDoorsZ;
float correctionDynamicsX, correctionDynamicsY, correctionDynamicsZ;
float correctionLightsSaveableX, correctionLightsSaveableY, correctionLightsSaveableZ;
float correctionStaticImmutableX, correctionStaticImmutableY, correctionStaticImmutableZ;
float correctionStaticSaveableX, correctionStaticSaveableY, correctionStaticSaveableZ;
float correctionLightX, correctionLightY, correctionLightZ;
uint16_t loadedLights = 0;
float lightMinIntensity[LIGHT_COUNT] = {0};
float lightMaxIntensity[LIGHT_COUNT] = {0};
bool lightOn[LIGHT_COUNT] = {1};
bool lightLerpOn[LIGHT_COUNT] = {0};
bool lightLerpUp[LIGHT_COUNT] = {0};
uint8_t lightCurrentStep[LIGHT_COUNT] = {0};
float lightLerpValue[LIGHT_COUNT] = {0};
float lightLerpTime[LIGHT_COUNT] = {0};
float lightLerpStepTime[LIGHT_COUNT] = {0};
float lightLerpStartTime[LIGHT_COUNT] = {0};
uint8_t lightIntervalStepsLength[LIGHT_COUNT] = {0};
float lightIntervalSteps[LIGHT_COUNT][30] = {0};
uint8_t lightIntervalStepIsLerpingLength[LIGHT_COUNT] = {0};
float intervalStepisLerping[LIGHT_COUNT][30] = {0};

#ifdef DEBUG_ENTITIES
    void DualLogEntity(Entity ent) {
        DualLog("Entity::\n"
                "    index: %u\n"
                "    entflags: %u [\n      ACTIVE:     %u\n      CARDCHUNK:  %u\n      GROUNDED:   %u\n      USEGRAVITY: %u\n      KINEMATIC:  %u\n      RIGIDBODY:  %u\n            ]\n"
                "    modelIndex: %u\n"
                "    animated:   %u\n"
                "    texIndex:   %u\n"
                "    glowIndex:  %u\n"
                "    specIndex:  %u\n"
                "    normIndex:  %u\n"
                "    lodIndex:  %u\n"
                "    position.x: %f, .y: %f, .z: %f\n"
                "    rotation.x: %f, .y: %f, .z: %f, .w: %f\n"
                "    scale.x: %f, .y: %f, .z: %f\n"
                "    velocity.x: %f, .y: %f, .z: %f\n"
                "    angularVelocity.x: %f, .y: %f, .z: %f\n"
                "    bodyState: %u\n"
                "    collider: %u\n"
                "    colliderCenter.x: %f, .y: %f, .z: %f\n"
                "    colliderSize.x: %f, .y: %f, .z: %f\n"
                "    colliderMeshIndex: %u,\n"
                "    mass: %f\n"
                "    linearDrag: %f\n"
                "    angularDrag: %f\n"
                "    inertia: %f\n"
                "    accumulatedForce.x: %f, .y: %f, .z: %f\n"
                "    accumulatedTorque.x: %f, .y: %f, .z: %f\n"
                "    bounciness: %f\n"
                "    dynamicFriction: %f\n"
                "    staticFriction: %f\n"
                "    frictionCombine: %u\n"
                "    bounceCombine: %u\n"
                "    volume: %f\n"
                ,
                ent.index,
                ent.entflags,
                    (ent.entflags & ENTFLAG_ACTIVE) > 0,
                    (ent.entflags & ENTFLAG_CARDCHUNK) > 0,
                    (ent.entflags & ENTFLAG_GROUNDED) > 0,
                    (ent.entflags & ENTFLAG_USEGRAVITY) > 0,
                    (ent.entflags & ENTFLAG_KINEMATIC) > 0,
                    (ent.entflags & ENTFLAG_RIGIDBODY) > 0,
                ent.modelIndex,
                ent.animated,
                ent.texIndex,
                ent.glowIndex,
                ent.specIndex,
                ent.normIndex,
                ent.lodIndex,
                ent.position.x, ent.position.y, ent.position.z,
                ent.rotation.x, ent.rotation.y, ent.rotation.z, ent.rotation.w,
                ent.scale.x, ent.scale.y, ent.scale.z,
                ent.velocity.x, ent.velocity.y, ent.velocity.z,
                ent.angularVelocity.x, ent.angularVelocity.y, ent.angularVelocity.z,
                ent.bodyState,
                ent.collider,
                ent.colliderCenter.x, ent.colliderCenter.y, ent.colliderCenter.z,
                ent.colliderSize.x, ent.colliderSize.y, ent.colliderSize.z,
                ent.colliderMeshIndex,
                ent.mass,
                ent.linearDrag,
                ent.angularDrag,
                ent.inertia,
                ent.accumulatedForce.x, ent.accumulatedForce.y, ent.accumulatedForce.z,
                ent.accumulatedTorque.x, ent.accumulatedTorque.y, ent.accumulatedTorque.z,
                ent.dynamicFriction,
                ent.staticFriction,
                ent.bounciness,
                ent.frictionCombine,
                ent.bounceCombine,
                ent.volume);
    }

    void DualLogEntityInstance(uint16_t idx) {
        DualLog("Logging instance[%u] ",idx);
        DualLogEntity(instances[idx]);
    }
#endif

void InitializeEntity(Entity* entry) {
    entry->index = UINT16_MAX; // memset here would be harmful as only a handful of fields are the same.
    entry->entflags = ENTFLAG_KINEMATIC; // Zeroes the rest out.
    entry->modelIndex = MODEL_IDX_MAX;
    entry->animated = 0u;
    entry->texIndex = entry->glowIndex = entry->specIndex = entry->normIndex = MATERIAL_IDX_MAX;
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
    for (int i=0;i<MAX_CHILD_COUNT;++i) {
        entry->child[i] = UINT16_MAX;
        entry->child_offset[i].x = entry->child_offset[i].y = entry->child_offset[i].z = 0.0f;
        entry->child_rotation[i].x = entry->child_rotation[i].y = entry->child_rotation[i].z = 0.0f; entry->child_rotation[i].w = 1.0f;
        entry->child_scale[i].x = entry->child_scale[i].y = entry->child_scale[i].z = 1.0f;
    }
    entry->path[0] = '\0';    
}

void LoadEntities(void) {
    double start_time = get_time();
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); OS_Exit(1); }
    
    entityCount = entity_parser.count;
    DualLog("Loading  %d entities...", entityCount);
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); OS_Exit(1); }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); OS_Exit(1); }

    memset(entities,0,MAX_ENTITIES * sizeof(Entity));
    for (int32_t i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        entities[i] = entity_parser.entries[i];
        flag_enable(&entities[i].entflags, ENTFLAG_ACTIVE);
        flag_set(&entities[i].entflags,    ENTFLAG_GROUNDED, false);
        flag_set(&entities[i].entflags,    ENTFLAG_RIGIDBODY, ConstIndexIsDynamicObject(entities[i].index));
        if (entity_parser.entries[i].entflags & ENTFLAG_CARDCHUNK) {
            entities[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX; // Generic LOD card
            entities[i].collider = COLLIDER_TYPE_BOX;
            entities[i].colliderCenter.x = 0.0f;
            entities[i].colliderCenter.y = 1.44f;
            entities[i].colliderCenter.z = 0.0f;
            entities[i].colliderSize.x = 2.56f;
            entities[i].colliderSize.y = 0.32f;
            entities[i].colliderSize.z = 2.56f;
        }

        entities[i].inertia = 0.0f;
        entities[i].accumulatedForce.x = 0.0f;
        entities[i].accumulatedForce.y = 0.0f;
        entities[i].accumulatedForce.z = 0.0f;
        entities[i].accumulatedTorque.x = 0.0f;
        entities[i].accumulatedTorque.y = 0.0f;
        entities[i].accumulatedTorque.z = 0.0f;
        #ifdef DEBUG_ENTITIES
            DualLogEntity(entities[i]);
        #endif
    }

    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

void CopyInstanceRegion(uint16_t head, uint16_t* instanceTypeArray, Entity* tempInstances, uint16_t* targetIndex, uint16_t nextRegionStart) {
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        for (uint16_t j = 0; j < head; j++) {
            uint16_t i = instanceTypeArray[j];
            if (tempInstances[i].modelIndex == modelIdx) {
                if (*targetIndex >= nextRegionStart) { DualLogError("Instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, *targetIndex); OS_Exit(1); }
                
                instances[*targetIndex] = tempInstances[i];
                (*targetIndex) += 1;
            }
        }
    }
}

uint16_t opaqueInstances[INSTANCE_COUNT] = {0};
uint16_t doubleSidedInstances[INSTANCE_COUNT] = {0};
uint16_t transparentInstances[INSTANCE_COUNT] = {0};
Entity tempInstances[INSTANCE_COUNT] = {0};
    
void SortInstances(void) { // Reorder instances such that each type is grouped opaque->doublesided->transparent in that order in instances[].
    double start_time = get_time();
    DualLog("Sorting entity instances... ");
    if (modelTypeCountsOpaque      ) { free(modelTypeCountsOpaque      ); }   modelTypeCountsOpaque = calloc(loadedModels,sizeof(uint16_t)); // Zero out all arrays and counters
    if (modelTypeCountsDoubleSided ) { free(modelTypeCountsDoubleSided ); }   modelTypeCountsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeCountsTransparent ) { free(modelTypeCountsTransparent ); }   modelTypeCountsTransparent = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeOffsetsOpaque     ) { free(modelTypeOffsetsOpaque     ); }   modelTypeOffsetsOpaque = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeOffsetsDoubleSided) { free(modelTypeOffsetsDoubleSided); }   modelTypeOffsetsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeOffsetsTransparent) { free(modelTypeOffsetsTransparent); }   modelTypeOffsetsTransparent = calloc(loadedModels,sizeof(uint16_t));
    memset(opaqueInstances,0,INSTANCE_COUNT * sizeof(uint16_t));
    memset(doubleSidedInstances,0,INSTANCE_COUNT * sizeof(uint16_t));
    memset(transparentInstances,0,INSTANCE_COUNT * sizeof(uint16_t));
    opaqueInstancesHead = doubleSidedInstancesHead = transparentInstancesHead = invalidModelIndexCount = 0;
    for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { // Skip player instances and NULLENT by starting at 3.
        if (instances[i].texIndex >= loadedTextures && instances[i].texIndex != MATERIAL_IDX_MAX) { DualLogError("Invalid texIndex %u for instance %u\n", instances[i].texIndex, i); invalidModelIndexCount++; continue; }
        if (instances[i].modelIndex >= loadedModels || instances[i].modelIndex == UINT16_MAX) { invalidModelIndexCount++; continue; }
        if (instances[i].index >= MAX_ENTITIES) { DualLogError("Invalid entity index %u for instance %u\n", instances[i].index, i); invalidModelIndexCount++; continue; }

        bool is_double_sided = isDoubleSided(instances[i].texIndex) || instances[i].scale.x < 0.0f || instances[i].scale.y < 0.0f || instances[i].scale.z < 0.0f;
        if (isTransparent(instances[i].texIndex)) {
            if (transparentInstancesHead >= INSTANCE_COUNT) { DualLogError("Transparent instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            transparentInstances[transparentInstancesHead++] = i;
            modelTypeCountsTransparent[instances[i].modelIndex]++;
        } else if (is_double_sided) {
            if (doubleSidedInstancesHead >= INSTANCE_COUNT) { DualLogError("Double-sided instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            doubleSidedInstances[doubleSidedInstancesHead++] = i;
            modelTypeCountsDoubleSided[instances[i].modelIndex]++;
        } else {
            if (opaqueInstancesHead >= INSTANCE_COUNT) { DualLogError("Opaque instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            opaqueInstances[opaqueInstancesHead++] = i;
            modelTypeCountsOpaque[instances[i].modelIndex]++;
        }
    }

    // Compute offsets
    uint16_t currentOffset = START_INDEX_LEVEL_INSTANCES;
    uint16_t i = 0;
    for (; i < loadedModels; i++) { modelTypeOffsetsOpaque[i] = currentOffset; currentOffset += modelTypeCountsOpaque[i]; }
    startOfDoubleSidedInstances = currentOffset;
    for (i = 0; i < loadedModels; i++) { modelTypeOffsetsDoubleSided[i] = currentOffset; currentOffset += modelTypeCountsDoubleSided[i]; }
    startOfTransparentInstances = currentOffset;
    for (i = 0; i < loadedModels; i++) { modelTypeOffsetsTransparent[i] = currentOffset; currentOffset += modelTypeCountsTransparent[i]; }
    if ((startOfTransparentInstances + transparentInstancesHead) > (loadedInstances - invalidModelIndexCount)) { DualLogError("Transparent range overflow: start %u, head %u, limit %u\n", startOfTransparentInstances, transparentInstancesHead, loadedInstances - invalidModelIndexCount); OS_Exit(1); }

    memset(tempInstances,0,INSTANCE_COUNT * sizeof(Entity));
    memcpy(tempInstances, instances, loadedInstances * sizeof(Entity));
    uint16_t targetIdx = START_INDEX_LEVEL_INSTANCES;
    CopyInstanceRegion(opaqueInstancesHead,           opaqueInstances, tempInstances, &targetIdx, startOfDoubleSidedInstances); // Copy opaque instances
    CopyInstanceRegion(doubleSidedInstancesHead, doubleSidedInstances, tempInstances, &targetIdx, startOfTransparentInstances); // Copy doublesided instances
    CopyInstanceRegion(transparentInstancesHead, transparentInstances, tempInstances, &targetIdx,             loadedInstances); // Copy transparent instances
    for (i = 0; i < loadedInstances; ++i) { // Put all the invisible entities at the end of the list now
        if (tempInstances[i].modelIndex > loadedModels) { instances[targetIdx] = tempInstances[i]; targetIdx++; }
    }

    // Update cellIndexForInstance
    for (i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) { // Skip player index and start at 3?
        float x = instances[i].position.x;
        float z = instances[i].position.z;
        int32_t cellX = (int32_t)vfloor((x - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t cellZ = (int32_t)vfloor((z - worldMin_z) / WORLDCELL_WIDTH_F);
        cellX = clamp(cellX, 0, 63);
        cellZ = clamp(cellZ, 0, 63);
        cellIndexForInstance[i] = cellZ * 64 + cellX;
    }
    
    DualLog("opaque: %u, double-sided: %u, transparent: %u, invisible: %u...", opaqueInstancesHead, doubleSidedInstancesHead, transparentInstancesHead, invalidModelIndexCount);
    DualLog(" took %f secs\n", get_time() - start_time);
    loadedAmbients = 0;
    for (i = opaqueInstancesHead + doubleSidedInstancesHead + transparentInstancesHead; i<loadedInstances;++i) {
        uint16_t entIdx = instances[i].index;
        if (ConstIndexIsAmbient(entIdx)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); OS_Exit(1); }
            
            instances[i].volume = entities[entIdx].volume * 0.5f;
        }
    }
}

void AddInstance(uint16_t entIdx, uint16_t instanceIdx, uint32_t lineNum) {
    if (entIdx >= entityCount) { DualLogError("\nEntity index when loading non-light entity %d was %d, exceeds max defined entity count of %d\n",lineNum,entIdx,entityCount); OS_Exit(1); }
        
    instances[instanceIdx].index = entIdx;
    instances[instanceIdx].modelIndex = entities[entIdx].modelIndex;
    instances[instanceIdx].animated = modelAnimationType[instances[instanceIdx].modelIndex];
    if (instances[instanceIdx].modelIndex < loadedModels) renderableCount++;
    instances[instanceIdx].texIndex = entities[entIdx].texIndex;
    instances[instanceIdx].glowIndex = entities[entIdx].glowIndex;
    if (instances[instanceIdx].glowIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].glowIndex = 0;
    instances[instanceIdx].specIndex = entities[entIdx].specIndex;
    if (instances[instanceIdx].specIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].specIndex = 0;
    instances[instanceIdx].normIndex = entities[entIdx].normIndex;
    if (instances[instanceIdx].normIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].normIndex = 0;
    instances[instanceIdx].lodIndex = entities[entIdx].lodIndex;
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_CARDCHUNK,  entities[entIdx].entflags & ENTFLAG_CARDCHUNK); // Decided `instances[instanceIdx].entflags = entities[entIdx].entflags;` was dangerous/error-prone, commented out in lieu of these explicit sets to better preserve the loaded data:
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_USEGRAVITY,  entities[entIdx].entflags & ENTFLAG_USEGRAVITY);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_KINEMATIC,  entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_RIGIDBODY,  entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    instances[instanceIdx].collider = entities[entIdx].collider;
    instances[instanceIdx].colliderCenter.x = entities[entIdx].colliderCenter.x;
    instances[instanceIdx].colliderCenter.y = entities[entIdx].colliderCenter.y;
    instances[instanceIdx].colliderCenter.z = entities[entIdx].colliderCenter.z;
    instances[instanceIdx].colliderSize.x = entities[entIdx].colliderSize.x;
    instances[instanceIdx].colliderSize.y = entities[entIdx].colliderSize.y;
    instances[instanceIdx].colliderSize.z = entities[entIdx].colliderSize.z;
    instances[instanceIdx].colliderMeshIndex = entities[entIdx].colliderMeshIndex;
    instances[instanceIdx].mass = entities[entIdx].mass > 0.0f ? entities[entIdx].mass : 1.0f; // Nonzero fallback.
    instances[instanceIdx].linearDrag = entities[entIdx].linearDrag > 0.0f ? entities[entIdx].linearDrag : 0.0f;
    instances[instanceIdx].angularDrag = entities[entIdx].angularDrag > 0.0f ? entities[entIdx].angularDrag : 0.05f;
    for (int i=0;i<MAX_CHILD_COUNT;++i) {
        instances[instanceIdx].child[i] = entities[entIdx].child[i];
        instances[instanceIdx].child_offset[i].x = entities[entIdx].child_offset[i].x;
        instances[instanceIdx].child_offset[i].y = entities[entIdx].child_offset[i].y;
        instances[instanceIdx].child_offset[i].z = entities[entIdx].child_offset[i].z;
        instances[instanceIdx].child_rotation[i].x = entities[entIdx].child_rotation[i].x;
        instances[instanceIdx].child_rotation[i].y = entities[entIdx].child_rotation[i].y;
        instances[instanceIdx].child_rotation[i].z = entities[entIdx].child_rotation[i].z;
        instances[instanceIdx].child_rotation[i].w = entities[entIdx].child_rotation[i].w;
        instances[instanceIdx].child_scale[i].x = entities[entIdx].child_scale[i].x;
        instances[instanceIdx].child_scale[i].y = entities[entIdx].child_scale[i].y;
        instances[instanceIdx].child_scale[i].z = entities[entIdx].child_scale[i].z;
    }
    
    ApplyUnityHierarchyCorrectionAtLevelLoad(instanceIdx, entIdx);
    dirtyInstances[instanceIdx] = true;
    loadedInstances++;
}

void AddChild(uint16_t child, uint16_t childIndex, uint16_t parent, uint16_t entIdx, int32_t* instanceIdx, uint32_t lineNum) {
    if (child == UINT16_MAX) return;
    
    (*instanceIdx)++; // Increment head of the list an extra time for the child entity
    AddInstance(child, *instanceIdx, lineNum);
    instances[*instanceIdx].index = child;
    instances[*instanceIdx].position.x = instances[parent].position.x + entities[entIdx].child_offset[childIndex].x;
    instances[*instanceIdx].position.y = instances[parent].position.y + entities[entIdx].child_offset[childIndex].y;
    instances[*instanceIdx].position.z = instances[parent].position.z + entities[entIdx].child_offset[childIndex].z;
    instances[*instanceIdx].scale.x = instances[parent].scale.x * entities[entIdx].child_scale[childIndex].x;
    instances[*instanceIdx].scale.y = instances[parent].scale.y * entities[entIdx].child_scale[childIndex].y;
    instances[*instanceIdx].scale.z = instances[parent].scale.z * entities[entIdx].child_scale[childIndex].z;
}

#define LINE_LEN_MAX 81920

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
void LoadLevel(uint8_t curlevel) {
    double start_time = get_time();
    if (!levelCurrentlyLoading) memset(instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    levelCurrentlyLoading = true;
    DebugRAM("start of LoadLevel");
    currentLevel = curlevel;
    renderableCount = 0;
    loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    loadedLights = 0;
    loadedAmbients = 0;
    if (curlevel >= numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); OS_Exit(1); }
    
    for (uint16_t idx = START_INDEX_LEVEL_INSTANCES;idx<INSTANCE_COUNT;idx++) { InitializeEntity(&instances[idx]); dirtyInstances[idx] = true; } // Start AFTER player indices and NULLENT
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    char filename[20]; // Minimum size for 0 through 13.
    snprintf(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); OS_Exit(1); }

    int32_t lineNum = -1; // Start at 0 on first loop iteration, as it needs to iterate before each blank or commented line skip
    int32_t instanceIdx = PLAYER2;
    int32_t lightsIdx = -1;
    char lineSpace[LINE_LEN_MAX];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[LINE_LEN_MAX];
    SetUnityHierarchyOffsets(curlevel);
    while (fgets(lineSpace, LINE_LEN_MAX, file)) {
        size_t len = strlen(lineSpace);
        while (len && (lineSpace[len - 1] == '\n' || lineSpace[len - 1] == '\r'))
        lineSpace[--len] = '\0';
        line = lineSpace;
        snprintf(initialLine, sizeof(initialLine), "%s", line);
        memcpy(firstKeyCheck,line,10); firstKeyCheck[10] = '\0';
        lineNum++;
        bool isLight = true;
        if (strcmp(firstKeyCheck, "constIndex") == 0) isLight = false;  // constIndex specified indicating this is a real entity?
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
        while(line[0] != '\0') {
            // Guaranteed no leading whitespaces,k comments, or blank lines, so don't bother
            char* pipe = strchr(line,'|');
            char* kvString = line; // key:value pair before the pipe as a string
            if (pipe) {
                *pipe = '\0';          // Split string at the pipe
                line = pipe + 1;       // Point to rest of the line after the pipe
            } else { // Else this is the last string after the last pipe with last kv pair
                line += strlen(line);
            }
           
            if (kvString[0] == '\0' || strchr(kvString, ':') == NULL) continue;
            
            char *colon = strchr(kvString, ':');
            if (colon[1] == '\0') continue; // Don't care about the name.  Need to skip this in the middle, but this also handles the very end
            
            *colon = '\0';           // Split string at the colon
            char *key = kvString;    // Assign key to before colon
            char *value = colon + 1; // Assing value to after colon
            if (!key || !value) { DualLogError("Invalid key-value pair at line %u (as viewed by text editor): %s\n", lineNum+1, initialLine); OS_Exit(1); }

            char trimmed_key[64];
            char trimmed_value[256];
            snprintf(trimmed_key, sizeof(trimmed_key), "%s", key);
            snprintf(trimmed_value, sizeof(trimmed_value), "%s", value);
            trimmed_key[sizeof(trimmed_key) - 1] = '\0';
            trimmed_value[sizeof(trimmed_value) - 1] = '\0';
//             sanitize_utf8_ascii(trimmed_key);
//             sanitize_utf8_ascii(trimmed_value);
            if (isLight) {
                     if (strcmp(trimmed_key, "localPosition.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSX] = parse_float(trimmed_value, initialLine, lineNum) + correctionX;
                else if (strcmp(trimmed_key, "localPosition.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSY] = parse_float(trimmed_value, initialLine, lineNum) + correctionY;
                else if (strcmp(trimmed_key, "localPosition.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = parse_float(trimmed_value, initialLine, lineNum) + correctionZ;
                else if (strcmp(trimmed_key, "localRotation.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intensity") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = parse_float(trimmed_value, initialLine, lineNum) * 0.451f; // ;)
                else if (strcmp(trimmed_key, "range") == 0)           lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "spotAngle") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "type") == 0) {
                    if ((strcmp(trimmed_value, "Spot") == 0)) lightType = 1u;
                    else if ((strcmp(trimmed_value, "Directional") == 0)) lightType = 2u;
                }
                else if (strcmp(trimmed_key, "color.r") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_R] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "color.g") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_G] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "color.b") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_B] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "lightOn") == 0 && !lightOnRead) {       lightOn[lightsIdx] = parse_bool(trimmed_value, initialLine, lineNum); lightOnRead = true; } // Check lightOnRead in if here since TargetIO also has same value lightOn, whoops!  But guaranteed to be 2nd so get the real one here
                else if (strcmp(trimmed_key, "lerpOn") == 0)          lightLerpOn[lightsIdx] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "currentStep") == 0)     lightCurrentStep[lightsIdx] = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "lerpValue") == 0)       lightLerpValue[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "lerpTime") == 0) {      float lt = LoadRelativeTimeDifferential(trimmed_value, initialLine, lineNum); lightLerpTime[lightsIdx] = lt < 0.1f ? 0.1f : lt; }
                else if (strcmp(trimmed_key, "stepTime") == 0)        lightLerpStepTime[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "lerpStartTime") == 0)   lightLerpStartTime[lightsIdx] = LoadRelativeTimeDifferential(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps.Length") == 0) lightIntervalStepsLength[lightsIdx] = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[0]") == 0)     lightIntervalSteps[lightsIdx][0] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[1]") == 0)     lightIntervalSteps[lightsIdx][1] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[2]") == 0)     lightIntervalSteps[lightsIdx][2] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[3]") == 0)     lightIntervalSteps[lightsIdx][3] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[4]") == 0)     lightIntervalSteps[lightsIdx][4] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[5]") == 0)     lightIntervalSteps[lightsIdx][5] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[6]") == 0)     lightIntervalSteps[lightsIdx][6] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[7]") == 0)     lightIntervalSteps[lightsIdx][7] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[8]") == 0)     lightIntervalSteps[lightsIdx][8] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[9]") == 0)     lightIntervalSteps[lightsIdx][9] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[10]") == 0)    lightIntervalSteps[lightsIdx][10] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[11]") == 0)    lightIntervalSteps[lightsIdx][11] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[12]") == 0)    lightIntervalSteps[lightsIdx][12] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[13]") == 0)    lightIntervalSteps[lightsIdx][13] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[14]") == 0)    lightIntervalSteps[lightsIdx][14] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[15]") == 0)    lightIntervalSteps[lightsIdx][15] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[16]") == 0)    lightIntervalSteps[lightsIdx][16] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[17]") == 0)    lightIntervalSteps[lightsIdx][17] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[18]") == 0)    lightIntervalSteps[lightsIdx][18] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[19]") == 0)    lightIntervalSteps[lightsIdx][19] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[20]") == 0)    lightIntervalSteps[lightsIdx][20]= parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[21]") == 0)    lightIntervalSteps[lightsIdx][21] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[22]") == 0)    lightIntervalSteps[lightsIdx][22] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[23]") == 0)    lightIntervalSteps[lightsIdx][23] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[24]") == 0)    lightIntervalSteps[lightsIdx][24] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[25]") == 0)    lightIntervalSteps[lightsIdx][25] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[26]") == 0)    lightIntervalSteps[lightsIdx][26] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[27]") == 0)    lightIntervalSteps[lightsIdx][27] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[28]") == 0)    lightIntervalSteps[lightsIdx][28] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalSteps[29]") == 0)    lightIntervalSteps[lightsIdx][29] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping.Length") == 0) lightIntervalStepIsLerpingLength[lightsIdx] = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[0]") == 0)     intervalStepisLerping[lightsIdx][0] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[1]") == 0)     intervalStepisLerping[lightsIdx][1] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[2]") == 0)     intervalStepisLerping[lightsIdx][2] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[3]") == 0)     intervalStepisLerping[lightsIdx][3] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[4]") == 0)     intervalStepisLerping[lightsIdx][4] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[5]") == 0)     intervalStepisLerping[lightsIdx][5] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[6]") == 0)     intervalStepisLerping[lightsIdx][6] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[7]") == 0)     intervalStepisLerping[lightsIdx][7] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[8]") == 0)     intervalStepisLerping[lightsIdx][8] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[9]") == 0)     intervalStepisLerping[lightsIdx][9] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[10]") == 0)    intervalStepisLerping[lightsIdx][10] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[11]") == 0)    intervalStepisLerping[lightsIdx][11] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[12]") == 0)    intervalStepisLerping[lightsIdx][12] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[13]") == 0)    intervalStepisLerping[lightsIdx][13] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[14]") == 0)    intervalStepisLerping[lightsIdx][14] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[15]") == 0)    intervalStepisLerping[lightsIdx][15] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[16]") == 0)    intervalStepisLerping[lightsIdx][16] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[17]") == 0)    intervalStepisLerping[lightsIdx][17] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[18]") == 0)    intervalStepisLerping[lightsIdx][18] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[19]") == 0)    intervalStepisLerping[lightsIdx][19] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[20]") == 0)    intervalStepisLerping[lightsIdx][20] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[21]") == 0)    intervalStepisLerping[lightsIdx][21] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[22]") == 0)    intervalStepisLerping[lightsIdx][22] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[23]") == 0)    intervalStepisLerping[lightsIdx][23] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[24]") == 0)    intervalStepisLerping[lightsIdx][24] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[25]") == 0)    intervalStepisLerping[lightsIdx][25] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[26]") == 0)    intervalStepisLerping[lightsIdx][26] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[27]") == 0)    intervalStepisLerping[lightsIdx][27] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[28]") == 0)    intervalStepisLerping[lightsIdx][28] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intervalStepisLerping[29]") == 0)    intervalStepisLerping[lightsIdx][29] = parse_bool(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "minIntensity") == 0)    lightMinIntensity[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "maxIntensity") == 0)    lightMaxIntensity[lightsIdx] = parse_float(trimmed_value, initialLine, lineNum);
            } else {
                     if (strcmp(trimmed_key, "constIndex") == 0)      instances[instanceIdx].index = parse_numberu16(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.x") == 0) instances[instanceIdx].position.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.y") == 0) instances[instanceIdx].position.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.z") == 0) instances[instanceIdx].position.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.x") == 0) instances[instanceIdx].rotation.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) instances[instanceIdx].rotation.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) instances[instanceIdx].rotation.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) instances[instanceIdx].rotation.w = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.x") == 0)    instances[instanceIdx].scale.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.y") == 0)    instances[instanceIdx].scale.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.z") == 0)    instances[instanceIdx].scale.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "go.activeSelf") == 0)   flag_set(&instances[instanceIdx].entflags, ENTFLAG_ACTIVE, parse_bool(trimmed_value, initialLine, lineNum));
            }
        }
        
        if (isLight) {
            loadedLights++;
            if (!lightOnRead) {
                lightOn[lightsIdx] = true;
            } else {
                // Dynamic Animated light
                if (lightMinIntensity[lightsIdx] < 0.01f) lightMinIntensity[lightsIdx] = 0.01f;
                lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[lightsIdx];
                lightLerpUp[lightsIdx] = true;
                lights[litIdx + LIGHT_DATA_OFFSET_POSX] += correctionLightX;
                lights[litIdx + LIGHT_DATA_OFFSET_POSY] += correctionLightY;
                lights[litIdx + LIGHT_DATA_OFFSET_POSZ] += correctionLightZ;
            }
            
            lightsRangeSquared[lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_RANGE] * lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            if (lightType == 1) {
                if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Spotlight %d on line %d loaded with spotAngle less than 5deg\n",lightsIdx,lineNum+1);
            } else if (lightType == 2) {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 180.0f; // Force to be a directional light
            } else {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            }
        } else {
            uint16_t parent = instanceIdx; // Needed as adding children moves the instanceIdx.
            uint16_t entIdx = instances[parent].index;
            AddInstance(entIdx, parent, lineNum);
            for (int i=0;i<MAX_CHILD_COUNT;++i) {
                if (instances[parent].child[i] < entityCount) AddChild(entities[entIdx].child[i], i, parent, entIdx, &instanceIdx, lineNum);
            }
        }
    }
    
    fclose(file);
    
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
    SetFog();
    malloc_trim(0);
    DualLog("Loaded %d geometry chunks and %u static lights for Level %d... took %f secs\n", loadedInstances, loadedLights, curlevel, get_time() - start_time);
    DebugRAM("end of LoadLevel instances");
    SortInstances(); // All instances loaded, sort them for render order: opaques, doublesideds, transparents.  REORDERS instances[] INDICES!!  CAREFUL!!
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (uint16_t i = 3; i < INSTANCE_COUNT; i++) UpdateInstanceMatrix(i); // Skip player indices and start at 3
    glNamedBufferData(matricesBuffer, loadedInstances * 16 * sizeof(uint32_t), modelMatrices, GL_DYNAMIC_DRAW);
    UpdateVoxelLightLists();
    voxen_GL_Comms.shadowMapSSBO = SetupSSBO(voxen_GL_Comms.shadowMapSSBO, 5, TOTAL_SHADOWMAP_PIXELS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    glUseProgram(voxen_GL_Comms.shadowmapsClearShaderProgram);
    GLuint groupX_shadClear = (TOTAL_SHADOWMAP_PIXELS + 31) / 32;
    glDispatchCompute(groupX_shadClear,1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //play_mp3("./Audio/music/THM1-19_medicalstart.mp3",((float)voxen_Settings.VolumeMusic/100.0f) * 0.4f,100);
    RenderShadowmaps();
    Input_MouselookApply();
    levelCurrentlyLoading = false;
}
#pragma GCC diagnostic pop
