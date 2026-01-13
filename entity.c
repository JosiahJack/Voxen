#include "os.h"
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <float.h>
#include "voxen.h"
#include "voxen.h"
#include "todo.h"

#define DEBUG_ENTITIES
// #define DEBUG_ENTITY_DEFINITIONS
#ifdef DEBUG_ENTITIES
    void DualLogEntity(Entity ent) {
        DualLog("Entity::%s\n"
                "    index: %u\n"
                "    entflags: %u [\n      ACTIVE:     %u\n      CARDCHUNK:  %u\n      GROUNDED:   %u\n      USEGRAVITY: %u\n      KINEMATIC:  %u\n      RIGIDBODY:  %u\n      DOUBLESIDED:  %u\n      TRANSPARENT:  %u\n      CHANGE_TEX_ON_ACTIVE:  %u\n      BLINK_TEX_ON_ACTIVE:  %u\n            ]\n"
                "    modelIndex: %u\n"
                "    animated:   %u\n"
                "    texIndex:   %u\n"
                "    altTexIndex:   %u\n"
                "    glowIndex:  %u\n"
                "    altGlowIndex:  %u\n"
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
                "    persistent: %u\n"
                "    overrideTest: %u\n"
                "    path: %s\n"
                , GetPrefabNameFromIndex(ent.index),
                ent.index,
                ent.entflags,
                    (ent.entflags & ENTFLAG_ACTIVE) > 0,
                    (ent.entflags & ENTFLAG_CARDCHUNK) > 0,
                    (ent.entflags & ENTFLAG_GROUNDED) > 0,
                    (ent.entflags & ENTFLAG_USEGRAVITY) > 0,
                    (ent.entflags & ENTFLAG_KINEMATIC) > 0,
                    (ent.entflags & ENTFLAG_RIGIDBODY) > 0,
                    (ent.entflags & ENTFLAG_DOUBLESIDED) > 0,
                    (ent.entflags & ENTFLAG_TRANSPARENT) > 0,
                    (ent.entflags & ENTFLAG_CHANGE_TEX_ON_ACTIVE) > 0,
                    (ent.entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) > 0,
                ent.modelIndex,
                ent.animated,
                ent.texIndex,
                ent.altTexIndex,
                ent.glowIndex,
                ent.altGlowIndex,
                ent.specIndex,
                ent.normIndex,
                ent.lodIndex,
                (double)ent.position.x, (double)ent.position.y, (double)ent.position.z,
                (double)ent.rotation.x, (double)ent.rotation.y, (double)ent.rotation.z, (double)ent.rotation.w,
                (double)ent.scale.x, (double)ent.scale.y, (double)ent.scale.z,
                (double)ent.velocity.x, (double)ent.velocity.y, (double)ent.velocity.z,
                (double)ent.angularVelocity.x, (double)ent.angularVelocity.y, (double)ent.angularVelocity.z,
                ent.bodyState,
                ent.collider,
                (double)ent.colliderCenter.x, (double)ent.colliderCenter.y, (double)ent.colliderCenter.z,
                (double)ent.colliderSize.x, (double)ent.colliderSize.y, (double)ent.colliderSize.z,
                ent.colliderMeshIndex,
                (double)ent.mass,
                (double)ent.linearDrag,
                (double)ent.angularDrag,
                (double)ent.inertia,
                (double)ent.accumulatedForce.x, (double)ent.accumulatedForce.y, (double)ent.accumulatedForce.z,
                (double)ent.accumulatedTorque.x, (double)ent.accumulatedTorque.y, (double)ent.accumulatedTorque.z,
                (double)ent.dynamicFriction,
                (double)ent.staticFriction,
                (double)ent.bounciness,
                (double)ent.frictionCombine,
                ent.bounceCombine,
                (double)ent.volume,
                ent.persistent,
                ent.overrideTest,
                ent.path);
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
    entry->layer = 0u; // PhysicsLayer_Default
    entry->animated = 0u;
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
    entry->persistent = false;
    for (int i=0;i<MAX_CHILD_COUNT;++i) {
        entry->child[i] = UINT16_MAX;
        entry->child_offset[i].x = entry->child_offset[i].y = entry->child_offset[i].z = 0.0f;
        entry->child_rotation[i].x = entry->child_rotation[i].y = entry->child_rotation[i].z = 0.0f; entry->child_rotation[i].w = 1.0f;
        entry->child_scale[i].x = entry->child_scale[i].y = entry->child_scale[i].z = 1.0f;
    }
    entry->path[0] = '\0';    
}

Entity entities[MAX_ENTITIES]; // Global array of entity definitions
uint16_t entityCount; // Number of entities loaded
DataParser entity_parser;
void LoadEntities(void) {
    double start_time = get_time();
    entityCount = 0;
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); OS_Exit(1); }
    
    entityCount = (uint16_t)entity_parser.count;
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
            #ifdef DEBUG_ENTITY_DEFINITIONS
                DualLogEntity(entities[i]);
            #endif
        #endif
    }

    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

void CopyInstanceRegion(uint16_t head, uint16_t* instanceTypeArray, Entity* tempInstances, uint16_t* targetIndex, uint16_t nextRegionStart) {
    for (uint16_t modelIdx = 0; modelIdx < MODEL_IDX_MAX; modelIdx++) {
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

__attribute__((pure)) bool isDoubleSided(uint32_t texIndexToCheck) {
    if (texIndexToCheck >= MAX_VALID_TEXTURE) return false;
    return doubleSidedTexture[texIndexToCheck] > 0 ? 1 : 0;
}
__attribute__((pure)) bool isTransparent(uint32_t texIndexToCheck) {
    if (texIndexToCheck >= MAX_VALID_TEXTURE) return false;
    return transparentTexture[texIndexToCheck] > 0 ? 1 : 0;    
}

uint16_t modelTypeCountsOpaque[MODEL_IDX_MAX];
uint16_t modelTypeCountsDoubleSided[MODEL_IDX_MAX];
uint16_t modelTypeCountsTransparent[MODEL_IDX_MAX];
uint16_t invalidModelIndexCount;
uint16_t startOfDoubleSidedInstances, startOfTransparentInstances;
uint16_t loadedInstances;
void SortInstances(void) { // Reorder instances such that each type is grouped opaque->doublesided->transparent in that order in instances[].
    double start_time = get_time();
    DualLog("Sorting entity instances... ");
    memset(modelTypeCountsOpaque, 0, MODEL_IDX_MAX * sizeof(uint16_t)); // Zero out all arrays and counters
    memset(modelTypeCountsDoubleSided, 0, MODEL_IDX_MAX * sizeof(uint16_t));
    memset(modelTypeCountsTransparent, 0, MODEL_IDX_MAX * sizeof(uint16_t));
    uint16_t* opaqueInstances      = calloc(INSTANCE_COUNT,sizeof(uint16_t));
    uint16_t* doubleSidedInstances = calloc(INSTANCE_COUNT,sizeof(uint16_t));
    uint16_t* transparentInstances = calloc(INSTANCE_COUNT,sizeof(uint16_t));
    uint16_t opaqueInstancesHead = 0, doubleSidedInstancesHead = 0, transparentInstancesHead = 0;
    invalidModelIndexCount = 0;
    startOfDoubleSidedInstances = startOfTransparentInstances = INSTANCE_COUNT - 1;
    for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { // Skip player instances and NULLENT by starting at 3.
        if (instances[i].texIndex >= MAX_VALID_TEXTURE && instances[i].texIndex != MAX_VALID_TEXTURE) { DualLogError("Invalid texIndex %u for instance %u\n", instances[i].texIndex, i); invalidModelIndexCount++; continue; }
        if (instances[i].modelIndex >= MODEL_IDX_MAX || instances[i].modelIndex == UINT16_MAX) { invalidModelIndexCount++; continue; }
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
    for (; i < MODEL_IDX_MAX; i++) { currentOffset += modelTypeCountsOpaque[i]; }
    startOfDoubleSidedInstances = currentOffset;
    for (i = 0; i < MODEL_IDX_MAX; i++) { currentOffset += modelTypeCountsDoubleSided[i]; }
    startOfTransparentInstances = currentOffset;
    for (i = 0; i < MODEL_IDX_MAX; i++) { currentOffset += modelTypeCountsTransparent[i]; }
    if ((startOfTransparentInstances + transparentInstancesHead) > (loadedInstances - invalidModelIndexCount)) { DualLogError("Transparent range overflow: start %u, head %u, limit %u\n", startOfTransparentInstances, transparentInstancesHead, loadedInstances - invalidModelIndexCount); OS_Exit(1); }

    Entity* tempInstances = calloc(INSTANCE_COUNT,sizeof(Entity));
    memcpy(tempInstances, instances, loadedInstances * sizeof(Entity));
    uint16_t targetIdx = START_INDEX_LEVEL_INSTANCES;
    CopyInstanceRegion(opaqueInstancesHead,           opaqueInstances, tempInstances, &targetIdx, startOfDoubleSidedInstances); // Copy opaque instances
    CopyInstanceRegion(doubleSidedInstancesHead, doubleSidedInstances, tempInstances, &targetIdx, startOfTransparentInstances); // Copy doublesided instances
    CopyInstanceRegion(transparentInstancesHead, transparentInstances, tempInstances, &targetIdx,             loadedInstances); // Copy transparent instances
    for (i = 0; i < loadedInstances; ++i) { // Put all the invisible entities at the end of the list now
        if (tempInstances[i].modelIndex > MODEL_IDX_MAX) { instances[targetIdx] = tempInstances[i]; targetIdx++; }
    }

    free(transparentInstances); free(doubleSidedInstances); free(opaqueInstances); free(tempInstances);
    malloc_trim(0);
    DualLog("opaque: %u, double-sided: %u, transparent: %u, invisible: %u...", opaqueInstancesHead, doubleSidedInstancesHead, transparentInstancesHead, invalidModelIndexCount);
    DualLog(" took %f secs\n", get_time() - start_time);
    ResetLevelAudio();
}

#ifdef ONLY_LOAD_LEVEL_NEEDS
bool modelIndexUsedForCurrentLevel[MODEL_IDX_MAX];
bool textureIndexUsedForCurrentLevel[MAX_VALID_TEXTURE];
#endif
void AddInstance(uint16_t entIdx, uint16_t instanceIdx) {
    if (entIdx >= entityCount) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d\n",entIdx,entityCount); OS_Exit(1); }
        
    instances[instanceIdx].index = entIdx;
    bool isCardChunk = (entities[entIdx].entflags & ENTFLAG_CARDCHUNK);
    instances[instanceIdx].modelIndex = entities[entIdx].modelIndex;
    instances[instanceIdx].colliderMeshIndex = entities[entIdx].colliderMeshIndex;
    instances[instanceIdx].animated = modelAnimationType[instances[instanceIdx].modelIndex];
    instances[instanceIdx].numclips = entities[entIdx].numclips;
    instances[instanceIdx].animationNum = entities[entIdx].animationNum;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[instanceIdx].modelIndex < MODEL_IDX_MAX) modelIndexUsedForCurrentLevel[instances[instanceIdx].modelIndex] = true;
        if (instances[instanceIdx].colliderMeshIndex < MODEL_IDX_MAX) modelIndexUsedForCurrentLevel[instances[instanceIdx].colliderMeshIndex] = true;
        if (EntityIsAnimated(entIdx)) {
            uint16_t numClips = entities[entIdx].numclips;
            uint16_t animNum = entities[entIdx].animationNum;
            for (int c=0;c<numClips;++c) {
                uint16_t startMindex = modelAnimationClips[animNum][c].frameStartModelIndex;
                uint16_t endMindex = modelAnimationClips[animNum][c].frameEndModelIndex;
                for (int mindex=startMindex;mindex<=endMindex;++mindex) modelIndexUsedForCurrentLevel[mindex] = true;
            }
        }
    #endif
    
    instances[instanceIdx].texIndex = entities[entIdx].texIndex;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[instanceIdx].texIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[instanceIdx].texIndex] = true;
    #endif
    
    instances[instanceIdx].glowIndex = entities[entIdx].glowIndex;
    if (instances[instanceIdx].glowIndex >= MAX_VALID_TEXTURE) instances[instanceIdx].glowIndex = 0;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[instanceIdx].glowIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[instanceIdx].glowIndex] = true;
    #endif
    
    instances[instanceIdx].specIndex = entities[entIdx].specIndex;
    if (instances[instanceIdx].specIndex >= MAX_VALID_TEXTURE) instances[instanceIdx].specIndex = 0;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[instanceIdx].specIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[instanceIdx].specIndex] = true;
    #endif

    instances[instanceIdx].normIndex = entities[entIdx].normIndex;
    if (instances[instanceIdx].normIndex >= MAX_VALID_TEXTURE) instances[instanceIdx].normIndex = 0;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[instanceIdx].normIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[instanceIdx].normIndex] = true;
    #endif

    instances[instanceIdx].lodIndex = entities[entIdx].lodIndex;
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_CARDCHUNK,  isCardChunk); // Decided `instances[instanceIdx].entflags = entities[entIdx].entflags;` was dangerous/error-prone, commented out in lieu of these explicit sets to better preserve the loaded data:
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_USEGRAVITY,  entities[entIdx].entflags & ENTFLAG_USEGRAVITY);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_KINEMATIC,  entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_RIGIDBODY,  entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_NO_SHADOWS,  entities[entIdx].entflags & ENTFLAG_NO_SHADOWS);
    instances[instanceIdx].collider = entities[entIdx].collider;
    instances[instanceIdx].colliderCenter.x = entities[entIdx].colliderCenter.x;
    instances[instanceIdx].colliderCenter.y = entities[entIdx].colliderCenter.y;
    instances[instanceIdx].colliderCenter.z = entities[entIdx].colliderCenter.z;
    instances[instanceIdx].colliderSize.x = entities[entIdx].colliderSize.x;
    instances[instanceIdx].colliderSize.y = entities[entIdx].colliderSize.y;
    instances[instanceIdx].colliderSize.z = entities[entIdx].colliderSize.z;
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
        instances[instanceIdx].child_scale[i].x = isCardChunk ? entities[entIdx].child_scale[i].x : 1.0f;
        instances[instanceIdx].child_scale[i].y = isCardChunk ? entities[entIdx].child_scale[i].y : 1.0f;
        instances[instanceIdx].child_scale[i].z = isCardChunk ? entities[entIdx].child_scale[i].z : 1.0f;
    }
    
    ApplyUnityHierarchyCorrectionAtLevelLoad(instanceIdx, entIdx); // TODO: Manually fix these all up to not be needed.
    dirtyInstances[instanceIdx] = true;
    loadedInstances++;
}

void SetAnimationTables(void) {
    // doorB (door2)
    modelAnimationClips[0][ANIM_LOOP_ALL]    = (AnimationClip){ .frameStart = 2, .frameEnd = 21, .frameStartModelIndex = 699, .frameEndModelIndex = 718, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[0][ANIM_IDLE_CLOSED] = (AnimationClip){ .frameStart = 2, .frameEnd = 2, .frameStartModelIndex = 699, .frameEndModelIndex = 698, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[0][ANIM_OPENING]     = (AnimationClip){ .frameStart = 2, .frameEnd = 11, .frameStartModelIndex = 699, .frameEndModelIndex = 708, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[0][ANIM_IDLE_OPEN]   = (AnimationClip){ .frameStart = 11, .frameEnd = 11, .frameStartModelIndex = 708, .frameEndModelIndex = 708, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[0][ANIM_CLOSING]     = (AnimationClip){ .frameStart = 12, .frameEnd = 21, .frameStartModelIndex = 709, .frameEndModelIndex = 718, .speed = 1.0f, .framerate = 24u };
    
    // doorA (door1)
    modelAnimationClips[1][ANIM_LOOP_ALL]    = (AnimationClip){ .frameStart = 2, .frameEnd = 24, .frameStartModelIndex = 719, .frameEndModelIndex = 741, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[1][ANIM_IDLE_CLOSED] = (AnimationClip){ .frameStart = 2, .frameEnd = 2, .frameStartModelIndex = 719, .frameEndModelIndex = 719, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[1][ANIM_OPENING]     = (AnimationClip){ .frameStart = 2, .frameEnd = 12, .frameStartModelIndex = 719, .frameEndModelIndex = 729, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[1][ANIM_IDLE_OPEN]   = (AnimationClip){ .frameStart = 12, .frameEnd = 12, .frameStartModelIndex = 729, .frameEndModelIndex = 729, .speed = 1.0f, .framerate = 24u };
    modelAnimationClips[1][ANIM_CLOSING]     = (AnimationClip){ .frameStart = 14, .frameEnd = 24, .frameStartModelIndex = 731, .frameEndModelIndex = 741, .speed = 1.0f, .framerate = 24u };

    // npc_humanoid_mutant
    modelAnimationClips[2][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 182, .frameStartModelIndex = 742, .frameEndModelIndex = 923, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_IDLE]     = (AnimationClip){ .frameStart = 0, .frameEnd = 38, .frameStartModelIndex = 742, .frameEndModelIndex = 780, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_WALK] = (AnimationClip){ .frameStart = 49, .frameEnd = 99, .frameStartModelIndex = 791, .frameEndModelIndex = 841, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_RUN] = (AnimationClip){ .frameStart = 49, .frameEnd = 99, .frameStartModelIndex = 791, .frameEndModelIndex = 841, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_ATTACK1] = (AnimationClip){ .frameStart = 111, .frameEnd = 137, .frameStartModelIndex = 853, .frameEndModelIndex = 879, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_ATTACK2] = (AnimationClip){ .frameStart = 111, .frameEnd = 137, .frameStartModelIndex = 853, .frameEndModelIndex = 879, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_ATTACK3] = (AnimationClip){ .frameStart = 111, .frameEnd = 137, .frameStartModelIndex = 853, .frameEndModelIndex = 879, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_PAIN] = (AnimationClip){ .frameStart = 138, .frameEnd = 151, .frameStartModelIndex = 880, .frameEndModelIndex = 893, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_DYING] = (AnimationClip){ .frameStart = 152, .frameEnd = 181, .frameStartModelIndex = 894, .frameEndModelIndex = 923, .speed = 1.0f, .framerate = 30u };
    modelAnimationClips[2][ANIM_DEAD] = (AnimationClip){ .frameStart = 181, .frameEnd = 181, .frameStartModelIndex = 923, .frameEndModelIndex = 923, .speed = 1.0f, .framerate = 30u };

    // npc_cyborg_drone
    modelAnimationClips[3][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 376, .frameStartModelIndex = 924, .frameEndModelIndex = 1300, .speed = 1.0f, .framerate = 24u };

    // doorD (door4, bulkhead 1)
    modelAnimationClips[4][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 97, .frameStartModelIndex = 1301, .frameEndModelIndex = 1397, .speed = 1.0f, .framerate = 24u };

    // doorC (door3)
    modelAnimationClips[5][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 46, .frameStartModelIndex = 1398, .frameEndModelIndex = 1443, .speed = 1.0f, .framerate = 24u };

    // doorK (xdoor1)
    modelAnimationClips[6][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 66, .frameStartModelIndex = 1444, .frameEndModelIndex = 1509, .speed = 1.0f, .framerate = 24u };

    // doorJ (xdoor2)
    modelAnimationClips[7][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 50, .frameStartModelIndex = 1510, .frameEndModelIndex = 1559, .speed = 1.0f, .framerate = 24u };

    // doorL (door10)
    modelAnimationClips[8][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 52, .frameStartModelIndex = 1560, .frameEndModelIndex = 1611, .speed = 1.0f, .framerate = 24u };

    // doorE (door5)
    modelAnimationClips[9][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 40, .frameStartModelIndex = 1612, .frameEndModelIndex = 1651, .speed = 1.0f, .framerate = 24u };

    // doorF (door6)
    modelAnimationClips[10][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 47, .frameStartModelIndex = 1652, .frameEndModelIndex = 1698, .speed = 1.0f, .framerate = 24u };

    // doorG (door7)
    modelAnimationClips[11][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 43, .frameStartModelIndex = 1699, .frameEndModelIndex = 1741, .speed = 1.0f, .framerate = 24u };

    // doorH (door8)
    modelAnimationClips[12][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 50, .frameStartModelIndex = 1742, .frameEndModelIndex = 1791, .speed = 1.0f, .framerate = 24u };

    // doorI (door9)
    modelAnimationClips[13][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 53, .frameStartModelIndex = 1792, .frameEndModelIndex = 1844, .speed = 1.0f, .framerate = 24u };

    // door_elevator1
    modelAnimationClips[14][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 42, .frameStartModelIndex = 1845, .frameEndModelIndex = 1886, .speed = 1.0f, .framerate = 24u };

    // door_elevator2
    modelAnimationClips[15][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 42, .frameStartModelIndex = 1887, .frameEndModelIndex = 1928, .speed = 1.0f, .framerate = 24u };

    // door_elevator3
    modelAnimationClips[16][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 44, .frameStartModelIndex = 1929, .frameEndModelIndex = 1972, .speed = 1.0f, .framerate = 24u };

    // door_elevator4
    modelAnimationClips[17][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 63, .frameStartModelIndex = 1973, .frameEndModelIndex = 2035, .speed = 1.0f, .framerate = 24u };

    // door_secret2 (door_wall1)
    modelAnimationClips[18][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 42, .frameStartModelIndex = 2036, .frameEndModelIndex = 2077, .speed = 1.0f, .framerate = 24u };

    // door_secret1 (door_wall2)
    modelAnimationClips[19][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 42, .frameStartModelIndex = 2078, .frameEndModelIndex = 2119, .speed = 1.0f, .framerate = 24u };

    // door_secret3 (door_wall3)
    modelAnimationClips[20][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 34, .frameStartModelIndex = 2120, .frameEndModelIndex = 2153, .speed = 1.0f, .framerate = 24u };

    // chunk_eng2_6 (eng_wallpump)
    modelAnimationClips[21][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 47, .frameStartModelIndex = 2154, .frameEndModelIndex = 2200, .speed = 1.0f, .framerate = 24u };

    // flight_fanwall
    modelAnimationClips[22][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 50, .frameStartModelIndex = 2201, .frameEndModelIndex = 2250, .speed = 1.0f, .framerate = 24u };

    // npc_bot_cortex_reaver
    modelAnimationClips[23][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 105, .frameStartModelIndex = 2251, .frameEndModelIndex = 2356, .speed = 1.0f, .framerate = 24u };

    // npc_cyborgassassin
    modelAnimationClips[24][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 278, .frameStartModelIndex = 2357, .frameEndModelIndex = 2635, .speed = 1.0f, .framerate = 24u };

    // npc_cyborg_diego
    modelAnimationClips[25][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 433, .frameStartModelIndex = 2636, .frameEndModelIndex = 3069, .speed = 1.0f, .framerate = 30u };

    // npc_cyborg_elite
    modelAnimationClips[26][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 449, .frameStartModelIndex = 3070, .frameEndModelIndex = 3519, .speed = 1.0f, .framerate = 30u };

    // npc_cyborg_enforcer
    modelAnimationClips[27][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 438, .frameStartModelIndex = 3520, .frameEndModelIndex = 3958, .speed = 1.0f, .framerate = 24u };

    // npc_cyborgwarrior
    modelAnimationClips[28][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 221, .frameStartModelIndex = 3959, .frameEndModelIndex = 4180, .speed = 1.0f, .framerate = 24u };

    // npc_execbot
    modelAnimationClips[29][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 131, .frameStartModelIndex = 4181, .frameEndModelIndex = 4312, .speed = 1.0f, .framerate = 24u };

    // npc_flierbot
    modelAnimationClips[30][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 121, .frameStartModelIndex = 4313, .frameEndModelIndex = 4434, .speed = 1.0f, .framerate = 24u };

    // npc_gortiger
    modelAnimationClips[31][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 179, .frameStartModelIndex = 4435, .frameEndModelIndex = 4614, .speed = 1.0f, .framerate = 24u };

    // npc_hopper
    modelAnimationClips[32][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 248, .frameStartModelIndex = 4615, .frameEndModelIndex = 4863, .speed = 1.0f, .framerate = 24u };

    // npc_invisomut
    modelAnimationClips[33][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 102, .frameStartModelIndex = 4864, .frameEndModelIndex = 4966, .speed = 1.0f, .framerate = 24u };

    // npc_maintenancebot
    modelAnimationClips[34][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 169, .frameStartModelIndex = 4967, .frameEndModelIndex = 5136, .speed = 1.0f, .framerate = 24u };

    // npc_mutant_avian
    modelAnimationClips[35][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 119, .frameStartModelIndex = 5137, .frameEndModelIndex = 5256, .speed = 1.0f, .framerate = 15u };

    // npc_plantmutant
    modelAnimationClips[36][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 239, .frameStartModelIndex = 5257, .frameEndModelIndex = 5496, .speed = 1.0f, .framerate = 24u };

    // npc_repairbot
    modelAnimationClips[37][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 148, .frameStartModelIndex = 5497, .frameEndModelIndex = 5645, .speed = 1.0f, .framerate = 24u };

    // npc_sec1bot
    modelAnimationClips[38][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 95, .frameStartModelIndex = 5646, .frameEndModelIndex = 5741, .speed = 1.0f, .framerate = 24u };

    // npc_sec2bot
    modelAnimationClips[39][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 75, .frameStartModelIndex = 5742, .frameEndModelIndex = 5817, .speed = 1.0f, .framerate = 24u };

    // npc_servbot
    modelAnimationClips[40][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 84, .frameStartModelIndex = 5818, .frameEndModelIndex = 5902, .speed = 1.0f, .framerate = 24u };

    // npc_virusmutant
    modelAnimationClips[41][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 225, .frameStartModelIndex = 5903, .frameEndModelIndex = 6128, .speed = 1.0f, .framerate = 24u };

    // npc_zerogmut
    modelAnimationClips[42][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 156, .frameStartModelIndex = 6129, .frameEndModelIndex = 6285, .speed = 1.0f, .framerate = 24u };

    // puzzlepanel1
    modelAnimationClips[43][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 43, .frameStartModelIndex = 6286, .frameEndModelIndex = 6328, .speed = 1.0f, .framerate = 24u };

    // puzzlepanel2 (starts at 000000)
    modelAnimationClips[44][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 30, .frameStartModelIndex = 6329, .frameEndModelIndex = 6359, .speed = 1.0f, .framerate = 24u };

    // puzzlepanel3 (starts at 000000)
    modelAnimationClips[45][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 0, .frameEnd = 18, .frameStartModelIndex = 6360, .frameEndModelIndex = 6378, .speed = 1.0f, .framerate = 24u };

    // sparkingwire
    modelAnimationClips[46][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 100, .frameStartModelIndex = 6379, .frameEndModelIndex = 6478, .speed = 1.0f, .framerate = 24u };

    // switch4
    modelAnimationClips[47][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 7, .frameStartModelIndex = 6479, .frameEndModelIndex = 6485, .speed = 1.0f, .framerate = 24u };

    // switch5
    modelAnimationClips[48][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 12, .frameStartModelIndex = 6486, .frameEndModelIndex = 6497, .speed = 1.0f, .framerate = 24u };

    // v_pipe
    modelAnimationClips[49][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 25, .frameStartModelIndex = 6498, .frameEndModelIndex = 6522, .speed = 1.0f, .framerate = 24u };

    // v_rapier
    modelAnimationClips[50][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 23, .frameStartModelIndex = 6523, .frameEndModelIndex = 6545, .speed = 1.0f, .framerate = 24u };

    // npc_mutant_cyborg
    modelAnimationClips[51][ANIM_LOOP_ALL] = (AnimationClip){ .frameStart = 1, .frameEnd = 258, .frameStartModelIndex = 6546, .frameEndModelIndex = 6803, .speed = 1.0f, .framerate = 24u };
}

uint16_t loadedLights;
float lightMinIntensity[LIGHT_COUNT];
float lightMaxIntensity[LIGHT_COUNT];
bool lightOn[LIGHT_COUNT];
bool lightLerpOn[LIGHT_COUNT];
bool lightLerpUp[LIGHT_COUNT];
uint8_t lightCurrentStep[LIGHT_COUNT];
float lightLerpValue[LIGHT_COUNT];
float lightLerpTime[LIGHT_COUNT];
float lightLerpStepTime[LIGHT_COUNT];
float lightLerpStartTime[LIGHT_COUNT];
uint8_t lightIntervalStepsLength[LIGHT_COUNT];
float lightIntervalSteps[LIGHT_COUNT][30];
uint8_t lightIntervalStepIsLerpingLength[LIGHT_COUNT];
float intervalStepisLerping[LIGHT_COUNT][30];
bool lightCastsShadows[LIGHT_COUNT];
uint16_t numDoorsFound;

#define LINE_LEN_MAX 81920
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
void LoadLevel(uint8_t curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true;
    queuedLevelToLoad = 255u; // Reset any loading state that got us here.
    if (curlevel == LEVEL_CYBERSPACE) RenderLoadingProgress(100,"Loading cyberspace...");
    else RenderLoadingProgress(100,"Loading level...");

    if (!Sys_Global.levelCurrentlyLoading) memset(instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    Sys_Global.levelCurrentlyLoading = true;
    Sys_Global.currentLevel = curlevel;
    loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
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
    SetAnimationTables();
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        memset(modelIndexUsedForCurrentLevel,0,MODEL_IDX_MAX * sizeof(bool));
        memset(textureIndexUsedForCurrentLevel,0,MAX_VALID_TEXTURE * sizeof(bool));
    #endif
    memset(lightMinIntensity,0,LIGHT_COUNT * sizeof(float));
    memset(lightMaxIntensity,0,LIGHT_COUNT * sizeof(float));
    memset(lightOn,1,LIGHT_COUNT * sizeof(bool)); // Default all on, only off if level data specifies
    memset(lightCastsShadows,1,LIGHT_COUNT * sizeof(bool)); // Default all on, only off if level data specifies
    memset(lightLerpOn,0,LIGHT_COUNT * sizeof(bool));
    memset(lightLerpUp,0,LIGHT_COUNT * sizeof(bool));
    memset(lightCurrentStep,0,LIGHT_COUNT * sizeof(uint8_t));
    memset(lightLerpValue,0,LIGHT_COUNT * sizeof(float));
    memset(lightLerpTime,0,LIGHT_COUNT * sizeof(float));
    memset(lightLerpStepTime,0,LIGHT_COUNT * sizeof(float));
    memset(lightLerpStartTime,0,LIGHT_COUNT * sizeof(float));
    memset(lightIntervalStepsLength,0,LIGHT_COUNT * sizeof(uint8_t));
    memset(lightIntervalSteps,0,LIGHT_COUNT * 30 * sizeof(float));
    memset(lightIntervalStepIsLerpingLength,0,LIGHT_COUNT * sizeof(uint8_t));
    memset(intervalStepisLerping,0,LIGHT_COUNT * 30 * sizeof(float));
    if (curlevel >= Sys_Global.numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n", curlevel, Sys_Global.numLevels - 1); OS_Exit(1); }
    
    for (uint16_t idx = START_INDEX_LEVEL_INSTANCES;idx<INSTANCE_COUNT;idx++) { InitializeEntity(&instances[idx]); dirtyInstances[idx] = true; } // Start AFTER player indices and NULLENT
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    char filename[20]; // Minimum size for 0 through 13.
    snprintf(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); OS_Exit(1); }

    uint32_t lineNum = 0;
    int32_t instanceIdx = PLAYER2;
    int32_t lightsIdx = -1; // Start at 0 on first loop iteration, -1 here due to ++ positioning, as it needs to iterate before each blank or commented line skip
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
        bool overridePos = false;
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
            if (!key) { DualLogError("Invalid key-value pair at line %u (as viewed by text editor): %s\n", lineNum, initialLine); OS_Exit(1); }

            char trimmed_key[64];
            char trimmed_value[256];
            snprintf(trimmed_key, sizeof(trimmed_key), "%s", key);
            snprintf(trimmed_value, sizeof(trimmed_value), "%s", value);
            trimmed_key[sizeof(trimmed_key) - 1] = '\0';
            trimmed_value[sizeof(trimmed_value) - 1] = '\0';
            if (isLight) {
                     if (strcmp(trimmed_key, "localPosition.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSX] = parse_float(trimmed_value, initialLine, lineNum) + correctionX;
                else if (strcmp(trimmed_key, "localPosition.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSY] = parse_float(trimmed_value, initialLine, lineNum) + correctionY;
                else if (strcmp(trimmed_key, "localPosition.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = parse_float(trimmed_value, initialLine, lineNum) + correctionZ;
                else if (strcmp(trimmed_key, "localRotation.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intensity") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = parse_float(trimmed_value, initialLine, lineNum) * 0.35f;
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
                else if (strcmp(trimmed_key, "overridePosition") == 0) overridePos  = parse_bool(trimmed_value, initialLine, lineNum) + correctionZ;
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
                lightMaxIntensity[lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            } else {
                // Dynamic Animated light
                if (lightMinIntensity[lightsIdx] < 0.01f) lightMinIntensity[lightsIdx] = 0.01f;
                lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[lightsIdx];
                lightLerpUp[lightsIdx] = true;
                lights[litIdx + LIGHT_DATA_OFFSET_POSX] += correctionLightX;
                lights[litIdx + LIGHT_DATA_OFFSET_POSY] += correctionLightY;
                lights[litIdx + LIGHT_DATA_OFFSET_POSZ] += correctionLightZ;
            }

            if (lightMaxIntensity[lightsIdx] < 0.16f || lights[litIdx + LIGHT_DATA_OFFSET_RANGE] < 0.32f) { lightsIdx--; loadedLights--; }
            if (lightType == 1) {
                if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Spotlight %d on line %d loaded with spotAngle less than 5deg\n",lightsIdx,lineNum+1);
            } else if (lightType == 2) {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 180.0f; // Force to be a directional light
            } else {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            }
            
            lightCastsShadows[lightsIdx] = (lights[litIdx + LIGHT_DATA_OFFSET_RANGE] >= 0.32f);
        } else {
            uint16_t parent = instanceIdx; // Needed as adding children moves the instanceIdx.
            uint16_t entIdx = instances[parent].index;
            float posBeforeX, posBeforeY, posBeforeZ;
            if (overridePos) {
                posBeforeX = instances[parent].position.x;
                posBeforeY = instances[parent].position.y;
                posBeforeZ = instances[parent].position.z;
                instances[parent].overrideTest = true;
            }
            AddInstance(entIdx, parent);
            if (overridePos) {
                    instances[parent].position.x = posBeforeX;
                    instances[parent].position.y = posBeforeY;
                    instances[parent].position.z = posBeforeZ;
            }
            
            if (EntityIndexIsPortalBlockingDoor(entIdx)) {
                float nudgeAmount = entIdx == 499 || entIdx == 509 ? 3.84f : 0.32f; // Bulkhead and giant elevator door need to nudge further to be sure.
                instances[parent].portalIndex = numActivePortals;
                bool isOpen = (instances[parent].doorState != DoorState_Closed); // Allows for any of DoorState_Open, DoorState_Opening, or DoorState_Closing to be considered open as far as portals are concerned so we can draw objects between the door panels.
                float obj_x = instances[parent].position.x;
                float obj_z = instances[parent].position.z;
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
                
                if (instances[parent].index == 496) instances[parent].clip = ANIM_IDLE_CLOSED;
                numActivePortals++;
            }
            
            for (int i=0;i<MAX_CHILD_COUNT;++i) {
                if (instances[parent].child[i] < entityCount) {
                    if (entities[entIdx].child[i] != UINT16_MAX) { // Add child
                        instanceIdx++; // Increment head of the list an extra time for the child entity.
                        AddInstance(entities[entIdx].child[i], instanceIdx);
                        instances[instanceIdx].index = entities[entIdx].child[i];
                        instances[instanceIdx].position.x = instances[parent].position.x + entities[entIdx].child_offset[i].x;
                        instances[instanceIdx].position.y = instances[parent].position.y + entities[entIdx].child_offset[i].y;
                        instances[instanceIdx].position.z = instances[parent].position.z + entities[entIdx].child_offset[i].z;
                        instances[instanceIdx].scale.x = instances[parent].scale.x * entities[entIdx].child_scale[i].x;
                        instances[instanceIdx].scale.y = instances[parent].scale.y * entities[entIdx].child_scale[i].y;
                        instances[instanceIdx].scale.z = instances[parent].scale.z * entities[entIdx].child_scale[i].z;
                    }
                }
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
    DualLog("Loaded %d entities, %u static lights, %u doors for Level %d... took %f secs\n", loadedInstances, loadedLights, numActivePortals, curlevel, get_time() - start_time);
    DebugRAM("end of LoadLevel instances");
    LoadModels();
    // Set Physics
    for (int i=0;i<ARRSIZE;++i) { gridCellFloorHeight[i] = -FLT_MAX; gridCellCeilingHeight[i] = FLT_MAX;}
    for (int i=PLAYER1;i<loadedInstances;++i) {
        int32_t cellIdx = PosGetCellCoords(instances[i].position.x, instances[i].position.z);
        instances[i].cellIndex = cellIdx;
        if (i == PLAYER1 || i == PLAYER2 || ConstIndexIsDynamicObject(instances[i].index)) instances[i].gravity = 1.0f; // Normal gravity
        else instances[i].gravity = 0.0f;
        
        if (instances[i].modelIndex >= loadedModelsMaxIndex) continue;
        if (instances[i].collider == COLLIDER_TYPE_NONE) continue;
        if (instances[i].collider == COLLIDER_TYPE_CONVEXMESH && instances[i].colliderMeshIndex >= loadedModelsMaxIndex) continue;
        
        if (instances[i].collider == COLLIDER_TYPE_BOX) {
            Quaternion quat = instances[i].rotation;
            Quaternion upQuat = {0.0f, 0.0f, 0.0f, 1.0f};
            float floorangle = quat_angle_deg(quat,upQuat); // Get angle in degrees relative to up vector (floor normal)
            Quaternion downQuat = {0.0f, 0.0f, 0.0f, -1.0f};
            float ceilangle = quat_angle_deg(quat,downQuat); // Get angle in degrees relative to down vector (ceiling normal)
            float floorHeight = (floorangle <= 30.0f) ? instances[i].position.y - 1.28f : -FLT_MAX; // World cells are 2.56x2.56x2.56 with modular chunk origins at center, so offset by half cell size to get actual positions.
            if (floorHeight > -FLT_MAX && floorHeight > gridCellFloorHeight[cellIdx]) gridCellFloorHeight[cellIdx] = floorHeight; // Raise floor up until highest one is selected.
            float ceilHeight = (ceilangle <= 30.0f) ? instances[i].position.y + 1.28f : FLT_MAX;
            if (ceilHeight < FLT_MAX && ceilHeight < gridCellCeilingHeight[cellIdx]) gridCellCeilingHeight[cellIdx] = ceilHeight; // Raise floor up until highest one is selected.
            continue;
        }
//         
//         uint32_t handle = 0;
//         Vector3 pos = instances[i].position;
//         Vector3 offset = instances[i].colliderCenter;
//         Vector3 size = instances[i].colliderSize;
//         uint8_t layer = instances[i].layer;
//         float mass = instances[i].mass;
//         bool isStatic = !(instances[i].entflags & ENTFLAG_RIGIDBODY);
//         uint16_t mdx = instances[i].collider == COLLIDER_TYPE_CONVEXMESH ? instances[i].colliderMeshIndex : instances[i].modelIndex;
//         switch (instances[i].collider) {
//             case COLLIDER_TYPE_NONE:       handle = 0; break;
//             case COLLIDER_TYPE_SPHERE:     handle = Physics_CreateSphere(size.x, pos, layer, mass, isStatic); break;
//             case COLLIDER_TYPE_BOX:        handle = Physics_CreateBox(size, offset, pos, instances[i].rotation, layer, mass, isStatic); break;
//             case COLLIDER_TYPE_CAPSULE:    handle = Physics_CreateCapsule(size.x, size.y, pos, instances[i].rotation, layer, mass, isStatic); break;
//             case COLLIDER_TYPE_CONVEXMESH:
//                 if (modelVertexCounts[mdx] < 3 && modelTriangleCounts[mdx] > 0) DualLogError("Convex mesh collider on entity %u uses model %u with no vertices!\n", i, mdx);
//                 else handle = Physics_CreateConvexMesh(modelVertices[mdx], modelVertexCounts[mdx], pos, instances[i].rotation, layer, mass, isStatic);
//                 
//                 break;
//             case COLLIDER_TYPE_MESH:
//                 DualLog("Full mesh collider for %s\n",GetPrefabNameFromIndex(instances[i].index));
//                 if (modelVertexCounts[mdx] < 3 && modelTriangleCounts[mdx] > 0) DualLogError("Convex mesh collider on entity %u uses model %u with no vertices!\n", i, mdx);
//                 else handle = Physics_CreateMeshCollider(modelVertices[mdx], modelTriangles[mdx], modelVertexCounts[mdx], modelTriangleCounts[mdx], pos, instances[i].rotation, layer, mass, isStatic);
//                 
//                 break;
//         }
//     
//         instances[i].physics_handle = handle;
    }
    
    float levelMinFloor = FLT_MAX;
    float levelMaxCeil = -FLT_MAX;
    for (int i=0;i<ARRSIZE;++i) { //        Using 1.0f buffer for floating point innaccuracies
        if (gridCellFloorHeight[i] > (-FLT_MAX +  1.0f) && gridCellFloorHeight[i] < levelMinFloor) levelMinFloor = gridCellFloorHeight[i];
        if (gridCellCeilingHeight[i] < (FLT_MAX - 1.0f) && gridCellCeilingHeight[i] > levelMaxCeil) levelMaxCeil = gridCellCeilingHeight[i];
    }
    
    DualLog("Min floor level for %d: %f, Max ceil %f\n", curlevel, (double)levelMinFloor, (double)levelMaxCeil);
    for (int i=0;i<ARRSIZE;++i) { //         Using 1.0f buffer for floating point innaccuracies
        if (gridCellFloorHeight[i] <= (-FLT_MAX +  1.0f)) gridCellFloorHeight[i] = levelMinFloor;
        if (gridCellCeilingHeight[i] >= (FLT_MAX - 1.0f)) gridCellCeilingHeight[i] = levelMaxCeil;
    }

    LoadTextures();
    SortInstances(); // All instances loaded, sort them for render order: opaques, doublesideds, transparents.  REORDERS instances[] INDICES!!  CAREFUL!!
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; i++) UpdateInstanceMatrix(i);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) dirtyInstances[i] = true;
    for (uint16_t i = 0; i < loadedLights; i++) {
        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        lightDirty[i] = true;
        lightsNewPosition[i] = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        lightInPVS[i] = false;
    }
    memset(voxelLightLists, 0, VOXEL_COUNT * 24 * sizeof(uint32_t));
    memset(voxelLightListCounts,0, VOXEL_COUNT * sizeof(uint32_t));
    memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
    #ifdef DEBUG_ENTITIES
        uint16_t endOfModels = loadedInstances - invalidModelIndexCount;
        for (int i=START_INDEX_LEVEL_INSTANCES; i < endOfModels;++i) { if (instances[i].overrideTest) DualLogEntityInstance(i); }
    #endif
    //play_mp3("./Audio/music/THM1-19_medicalstart.mp3",((float)Sys_Settings.VolumeMusic/100.0f) * 0.4f,100);
    Sys_Global.levelCurrentlyLoading = false;
    DualLog("LoadLevel completed!\n");
}
#pragma GCC diagnostic pop
