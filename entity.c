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
void AddInstance(uint16_t entIdx, uint16_t i) {
    if (entIdx >= entityCount) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d\n",entIdx,entityCount); OS_Exit(1); }
        
    instances[i].index = entIdx;
//     if (ConstIndexIsNPC(entIdx)) InitializeAI(i); TODO
    bool isCardChunk = (entities[entIdx].entflags & ENTFLAG_CARDCHUNK);
    instances[i].modelIndex = entities[entIdx].modelIndex;
    instances[i].colliderMeshIndex = entities[entIdx].colliderMeshIndex;
    flag_set(&instances[i].entflags, ENTFLAG_ANIMATED, modelHasAnimation[instances[i].modelIndex]);
    instances[i].numclips = entities[entIdx].numclips;
    instances[i].animationNum = entities[entIdx].animationNum;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[i].modelIndex < MODEL_IDX_MAX) modelIndexUsedForCurrentLevel[instances[i].modelIndex] = true;
        if (instances[i].colliderMeshIndex < MODEL_IDX_MAX) modelIndexUsedForCurrentLevel[instances[i].colliderMeshIndex] = true;
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
    
    instances[i].texIndex = entities[entIdx].texIndex;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[i].texIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[i].texIndex] = true;
    #endif
    
    instances[i].glowIndex = entities[entIdx].glowIndex;
    if (instances[i].glowIndex >= MAX_VALID_TEXTURE) instances[i].glowIndex = 0;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[i].glowIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[i].glowIndex] = true;
    #endif
    
    instances[i].specIndex = entities[entIdx].specIndex;
    if (instances[i].specIndex >= MAX_VALID_TEXTURE) instances[i].specIndex = 0;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[i].specIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[i].specIndex] = true;
    #endif

    instances[i].normIndex = entities[entIdx].normIndex;
    if (instances[i].normIndex >= MAX_VALID_TEXTURE) instances[i].normIndex = 0;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        if (instances[i].normIndex < MAX_VALID_TEXTURE) textureIndexUsedForCurrentLevel[instances[i].normIndex] = true;
    #endif

    instances[i].lodIndex = entities[entIdx].lodIndex;
    flag_set(&instances[i].entflags, ENTFLAG_CARDCHUNK,  isCardChunk); // Decided `instances[i].entflags = entities[entIdx].entflags;` was dangerous/error-prone, commented out in lieu of these explicit sets to better preserve the loaded data:
    flag_set(&instances[i].entflags, ENTFLAG_USEGRAVITY,  entities[entIdx].entflags & ENTFLAG_USEGRAVITY);
    flag_set(&instances[i].entflags, ENTFLAG_KINEMATIC,  entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&instances[i].entflags, ENTFLAG_RIGIDBODY,  entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    flag_set(&instances[i].entflags, ENTFLAG_NO_SHADOWS,  entities[entIdx].entflags & ENTFLAG_NO_SHADOWS);
    instances[i].collider = entities[entIdx].collider;
    instances[i].colliderCenter = entities[entIdx].colliderCenter;
    instances[i].colliderSize = entities[entIdx].colliderSize;
    instances[i].mass = entities[entIdx].mass > 0.0f ? entities[entIdx].mass : 1.0f; // Nonzero fallback.
    instances[i].linearDrag = entities[entIdx].linearDrag > 0.0f ? entities[entIdx].linearDrag : 0.0f;
    instances[i].angularDrag = entities[entIdx].angularDrag > 0.0f ? entities[entIdx].angularDrag : 0.05f;
    for (int c=0;c<MAX_CHILD_COUNT;++c) {
        instances[i].child[c] = entities[entIdx].child[c];
        instances[i].child_offset[c] = entities[entIdx].child_offset[c];
        instances[i].child_rotation[c] = entities[entIdx].child_rotation[c];
        instances[i].child_scale[c] = isCardChunk ? entities[entIdx].child_scale[c] : (Vector3){ 1.0f, 1.0f, 1.0f };
    }
    
    ApplyUnityHierarchyCorrectionAtLevelLoad(i, entIdx); // TODO: Manually fix these all up to not be needed.
    dirtyInstances[i] = true;
    loadedInstances++;
}

void DeleteInstance(uint16_t i) {
    if (i <= PLAYER2) return; // Don't delete null ent, player 1, nor player 2.
    if (i >= loadedInstances) return; // Already gone.
    
    // Shift render state markers.
    if (i <= startOfDoubleSidedInstances) --startOfDoubleSidedInstances;
    if (i <= startOfTransparentInstances) --startOfTransparentInstances;
    if (i <= endOfModels)                 --endOfModels;
    if (InstanceIsNonRenderable(i))      --invalidModelIndexCount;
    
    // Shift entire list
    uint16_t endInstance = vmax(vmin(INSTANCE_COUNT - 1, loadedInstances - 1),START_INDEX_LEVEL_INSTANCES);
    for (;i<endInstance;++i) instances[i] = instances[i + 1]; // Shift the list down, overwriting the entity we're deleting at starting i
    --loadedInstances; // Shift final marker.  It's history!
}

// Name,AtkTyp1,2,3,Dmg1,2,3,Range1,2,3,Health,CybHealth,Percp,Disrp,Armr,Def,Movtyp,Yawspd,FOV,FOVAtk,FOVStartMov,DistToSeeBehind,SightRange,WalkSpd,RunSpd,AtkSpd1,2,3,AtkForce3,AtkRad3,TtPain,TbwPain,TtDead,TtActualAtk1,2,3,TbwAtk1,2,3,TEnemChg,TIdleSFXMin,TIdleSFXMax,TAtk1WaitMin,TAtk1WaitMax,TAtk1WaitChnc,TAtk2WaitMin,TAtk2WaitMax,TAtk2WaitChnc,TAtk3WaitMin,TAtk3WaitMax,TAtk3WaitChnc,ProjType1,2,3,ProjSpd1,2,3,HasLaser1,2,3,ExplodeOn3,PreActMeleCols,THunt,FlightHeight,FlightHeightIsPerc,SwitchMatOnDie,RangeHear,TTranq,Hops,NPCType,AtkProj1,2,3
NPCTable npcTable[NUM_AI_TYPES] = {
 { "AUTOBOMB"              ,0,0,1,  0,  0,200, 0,0,2.4,50,0,1,0.5,40,1,1,300,180,120,55,3.84,50,2.5,2.5,0,0,0,100,6,0,0,0.1,0,0,0,0,0,0,3,5,12,0.5,1,0.1,1,3,0.5,0,0,0,0,0,0,0,0,0,0,0,0,1,0,20,0,0,0,10,3,0,2,0,0,0 },
 { "CYBORG ASSASSIN"       ,0,4,7, 30, 50, 35, 3.3,10,20,65,0,2,0.6,5,4,1,180,180,80,15,3.2,50,2,2,0,0,0,0,0,0.45,5,2.083,0,0.25,0.2,0.91,0.91,1.58,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,3,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,489 },
 { "AVIAN MUTANT"          ,1,0,0, 40, 40,  0, 3.3,10,20,125,0,1,0.25,0,2,2,180,180,80,15,5.12,50,2,2,3.5,0,0,0,0,2,5,1,0.1,0,0,1,0,0,3,5,12,0.5,1,0.1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,60,0.65,1,0,10,3,0,1,0,0,0 },
 { "EXEC-BOT"              ,0,4,0, 30, 35,  0, 3.3,10,20,225,0,1,0.2,40,2,1,200,180,15,30,4.12,50,1.5,1.5,0,0,0,0,0,0.45,7,0.15,0,0.2,0,0,1.5,0,3,5,12,0,0,0,0.97,2,0.3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "CYBORG DRONE"          ,0,4,0, 20, 20, 20, 3.3,25,50,60,0,1,0.3,0,2,1,65,180,80,15,3.2,50,1.6,2.2,0,0,0,0,0,0.542,15,0.958,0,0.1,0,0,1,0,3,20,45,0,0,0,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,0 },
 { "CORTEX REAVER"         ,0,4,7, 80,325,125, 3.3,20,30,580,0,1,0.1,40,2,1,180,180,80,15,3.84,50,2,2,0,0,0,0,0,0.583,5,0.333,0,0.35244,0.324,0,1,1,3,15,30,0,0,0,0.2,1,0.5,8,15,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,372 },
 { "CYBORG WARRIOR"        ,0,4,7, 35, 35,150, 3.3,20,20,120,0,1,0.1,5,4,1,180,180,30,15,3.2,50,2.4,2.4,0,0,0,0,0,0.5,5,2.2,0,0.339,0.201,0,0.83,0.542,3,15,30,0,0,0,1,2,0.5,10,20,1,0,0,0,0,0,10,0,0,0,0,0,180,0,0,0,10,3,0,3,0,0,370 },
 { "CYBORG ENFORCER"       ,1,4,7, 60, 60, 80, 3.3,15,30,285,0,1,0.1,30,5,1,180,180,80,15,3.2,50,2.8,2.8,2.8,0,0.3,0,0,2,5,1.5,0.23471,0.393738,0.313266,0.958,0.958,0.958,5,15,30,0.1,0.3,0.1,0.1,0.5,0.5,10,25,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,4,0,0,387 },
 { "CYBORG ELITE GUARD"    ,1,7,4, 70, 75,  0, 3.3,10,50,380,0,1,0.05,50,6,1,180,180,80,15,3.2,50,3,3,1.5,0,0,0,0,0.4665,5,1.5,0.5,0.2653,0.117045,0.733,0.7,0.867,5,15,30,0.05,0.2,0.1,0.5,2,0.8,2,3,0.5,0,0,0,0,2,0,0,1,0,0,0,600,0,0,0,15,3,0,4,0,490,0 },
 { "CYBORG OF EDWARD DIEGO",1,7,0, 80, 95,  0, 3.3,40,50,900,0,2,0,55,6,1,180,180,80,15,3.2,50,2.8,2.8,0,0,0,0,0,0,0,0,0.28,0.363188,0.2,1.4,0.833,3,5,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.5,0,0,0,0,0,1,600,0,0,0,15,3,0,4,0,490,0 },
 { "SECURITY-1 ROBOT"      ,0,4,0, 35, 35,  0, 3.3,10,20,170,0,1,0.15,40,4,2,180,180,80,15,4.12,50,2.5,2.5,1.5,0,0,0,0,2,5,0.05,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,600,1.28,0,0,10,3,0,2,0,0,0 },
 { "SECURITY-2 ROBOT"      ,0,4,4, 65, 65, 15, 3.3,5,35,300,0,2,0.05,50,5,1,180,180,60,25,4.12,50,1.5,1.5,1.5,0,0,0,0,0.75,5,0.25,0.5,0.39,0.1,1.2,1,1.5,3,5,12,0.5,1,0.1,3,3.5,1,2.5,3.5,1,0,0,0,0,0,0,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,0 },
 { "MAINTENANCE ROBOT"     ,1,0,0, 25, 25,  0, 3.3,3.3,20,75,0,1,0.3,40,3,1,180,180,80,15,3.84,50,2.2,2.6,0.02,0.02,0,0,0,0,0,1.6,3,0.7,0.2,2,1.3,3,3,5,12,0.5,1,0.1,1,2,0.3,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "MUTANT CYBORG"         ,1,7,0, 35, 75, 50, 2,30,49,340,0,1,0.2,15,6,1,180,180,60,15,3.2,50,1.5,1.5,0,0,0,0,0,0.583,3.5,3.41,0.265,0.285,0.2,0.625,0.75,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.8,0,0,0,0,0,0,180,0,0,0,10,3,0,5,0,491,0 },
 { "HOPPER"                ,0,4,0, 35, 35,  0, 0,17.92,17.92,150,0,1,0.25,35,4,1,180,160,80,15,3.84,50,7,7,0,0,0,0,0,0.708,5,0,0.5,0.1,0.5,0.5,0.5,0.5,3,5,12,0.5,1,0.1,0.5,1,0.5,1,2,0.5,0,0,0,0,0,0,0,1,0,0,0,180,0,0,0,10,3,1,2,0,0,0 },
 { "HUMANOID MUTANT"       ,1,0,0, 12, 12,  0, 3.3,10,20,50,0,0,0.4,0,3,1,60,180,80,15,2.56,50,1.4,2,0.5,0,0,0,0,0.42,5,0.967,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,0,0 },
 { "INVISIBLE MUTANT"      ,0,7,0, 10, 35,  0, 3.3,20,20,350,0,1,0.05,0,2,2,180,180,80,15,2.56,50,0.7,0.7,1.5,0.7,0.7,0,0,0.875,5,1.125,0.875,0.4,0.2,1.2,0.875,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,0.32,0,1,10,3,0,0,0,486,0 },
 { "VIRUS MUTANT"          ,0,7,0, 45, 30,  0, 3.3,20,20,140,0,0,0.1,0,3,1,180,180,80,15,2.56,50,2.5,2.5,2.5,0.3,0,0,0,0.542,3,1.792,0.2874,0.2874,0.2874,0.958,0.958,0.958,3,5,12,0.5,1,0.1,0.5,0,0.5,1,2,0.5,0,0,0,0,1.75,0,0,0,0,0,0,20,0,0,0,10,3,0,1,0,481,0 },
 { "SERV-BOT"              ,1,0,0,  8,  0,  0, 3.3,10,20,20,0,1,0.5,20,2,1,180,180,80,15,3.84,50,2,2,1.2,0,0,0,0,1.125,2,0.98,0.2,0.1,0.2,0.834,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "FLIER BOT"             ,0,4,7, 30,150,  0, 3.3,35,40,75,0,1,0.3,30,2,2,180,180,80,15,5.12,50,1.5,1.5,1.5,0,0,0,0,1.375,5,0.6,0.1,0.1,0.2,1,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,10,12,1,0,0,0,0,0,10,0,0,0,0,0,180,0.85,1,0,10,3,0,2,0,0,404 },
 { "ZERO-G MUTANT"         ,0,7,0, 20, 20,  0, 3.3,20,20,90,0,1,0.5,0,2,2,180,180,80,15,2.56,50,0.8,1.4,0,0.8,0,0,0,0.1,0,0.1,0.5,0.05,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,1.96,0,0,10,3,0,0,0,488,0 },
 { "GORILLA TIGER MUTANT"  ,1,0,0, 60, 60,  0, 3.3,3.84,20,200,0,1,0.1,0,3,1,180,180,80,15,2.56,50,3,3.5,1,2,0,0,0,0.667,5,1.625,0.5,0.1,0.2,0.958,1.042,3,3,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,1,0,0,0 },
 { "REPAIR BOT"            ,0,4,0, 12, 12,  0, 3.3,3.3,20,65,0,1,0.4,25,3,1,180,180,80,15,3.84,50,2.25,3,0.5,0,0,0,0,0,0,0.05,0.2,0.1,0.2,1.25,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "PLANT MUTANT"          ,0,7,0, 35, 25,  0, 3.3,20,20,115,0,1,0.3,0,1,1,180,180,80,15,2.56,50,0.8,1.2,0.1,0,0,0,0,0.375,2,2.208,0.89,0.82,0.2,1.91,1.027,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,3.5,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,487,0 },
 { "CYBER DOG"             ,0,7,0,  0, 25,  0, 0,20,0,0,20,1,0.5,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.3,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1.5,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
 { "CYBER GUARD"           ,0,7,0,  0, 25,  0, 0,20,0,0,35,1,0.4,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,0.8,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
 { "CYBER RAM"             ,0,7,0,  0, 35,  0, 0,20,0,0,40,1,0.25,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,1.2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
 { "CYBER CORTEX REAVER"   ,0,7,0,  0, 45,  0, 0,20,0,0,80,1,0.1,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
 { "SHODAN"                ,0,7,0,  0, 55,  0, 0,20,0,0,500,2,0,0,1,4,360,280,280,15,20.48,25.6,0,0,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.05,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 }
};
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//TARGET ID: Type-LevelNum(0#)EnemyNum(###),,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Example: Mutant-06003,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//EXCEPTIONS: Cyborg-00001 is Edward Diego,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

//                             NPC Sounds       0,   1,   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
int sfxIdle[NUM_AI_TYPES] =           {  -1,  -1,  -1, -1, 58, -1, 59, -1, 59, 52, -1, -1, -1, -1, -1, -1,121, -1, -1, -1,121,118, -1, -1, -1, -1, -1, -1, -1};
int sfxSightSound[NUM_AI_TYPES] =     {  -1,  -1, 111,150, 58,150, 59,152,152, -1,150,150,151,152,150, -1,121, -1,151,150,121,119,151, -1, -1, -1, -1, -1, -1};
int sfxAttack1[NUM_AI_TYPES] =        {  -1,  -1, 108, -1, -1,146, -1,146,252,247, -1, -1, -1, -1, -1,122, -1,108,146, -1, -1,118, -1,125,258,258,258,258,258};
int sfxAttack2[NUM_AI_TYPES] =        {  -1, 256,  -1,148, 50, 50, 50, 50, 50,250, 50, 50,146,259,148, -1,121, -1, -1,147, -1, -1,146, -1,258,258,258,258,258};
int sfxAttack3[NUM_AI_TYPES] =        {  -1,  -1,  -1, -1, -1,244,244,244,245, -1, -1,149, -1, -1, -1, -1, -1, -1, -1,244, -1, -1, -1, -1,258,258,258,258,258};
int sfxDeath[NUM_AI_TYPES] =          {  -1,  48, 110,143, 48,145, 48, 51, 47, 47,142,143,144, 47,162,123,120,134,144,144,120,117,144,124, -1, -1, -1, -1, -1};
float deathBurstTimer[NUM_AI_TYPES] = {0.0f,0.0f, 0.1f,0.0f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.0f,0.45f,0.75f,0.1f,0.0f,0.0f,0.1f,0.224f,0.9f,0.0f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f};
