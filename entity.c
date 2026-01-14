#include <stdlib.h>
#include <malloc.h>
#include <float.h>
#include "os.h"
#include "voxen.h"

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
            entities[i].colliderCenter = (Vector3){ .x = 0.0f, .y = 1.44f, .z = 0.0f };
            entities[i].colliderSize = (Vector3){ .x = 2.56f, .y = 0.32f, .z = 2.56f };
        }
    }

    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

__attribute__((pure)) bool isDoubleSided(uint32_t texIndexToCheck) {
    if (texIndexToCheck >= MAX_VALID_TEXTURE) return false;
    return doubleSidedTexture[texIndexToCheck] > 0 ? 1 : 0;
}
__attribute__((pure)) bool isTransparent(uint32_t texIndexToCheck) {
    if (texIndexToCheck >= MAX_VALID_TEXTURE) return false;
    return transparentTexture[texIndexToCheck] > 0 ? 1 : 0;    
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
