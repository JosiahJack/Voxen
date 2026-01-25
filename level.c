#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <float.h>
#include "os.h"
#include "voxen.h"

void InitializeEntity(Entity* entry) {
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

Entity entities[MAX_ENTITIES]; // Global array of entity definitions
uint16_t entityCount; // Number of entities loaded
void LoadEntities(void) {
    double start_time = get_time();
    entityCount = 0;
    DataParser entity_parser;
    if (!parse_data_file(&entity_parser, MAX_ENTITIES, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); OS_Exit(1); }
    
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

    free(entity_parser.entries);
    #ifndef WINDOWS
        malloc_trim(0);
    #endif
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
    instances[i].texIndex = entities[entIdx].texIndex;
    instances[i].glowIndex = entities[entIdx].glowIndex;
    if (instances[i].glowIndex >= MAX_VALID_TEXTURE) instances[i].glowIndex = 0;
    instances[i].specIndex = entities[entIdx].specIndex;
    if (instances[i].specIndex >= MAX_VALID_TEXTURE) instances[i].specIndex = 0;
    instances[i].normIndex = entities[entIdx].normIndex;
    if (instances[i].normIndex >= MAX_VALID_TEXTURE) instances[i].normIndex = 0;
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
    
    dirtyInstances[i] = true;
    loadedInstances++;
}

void DeleteInstance(uint16_t i) {
    if (i <= PLAYER2 || i >= loadedInstances) return; // Don't delete null ent, player 1, nor player 2 or already empty slots.
    
    if (i <= startOfDoubleSidedInstances) --startOfDoubleSidedInstances; // Shift render state markers.
    if (i <= startOfTransparentInstances) --startOfTransparentInstances;
    if (i <= endOfModels)                 --endOfModels;
    if (InstanceIsNonRenderable(i))      --invalidModelIndexCount;
    uint16_t endInstance = vmax(vmin(INSTANCE_COUNT - 1, loadedInstances - 1),START_INDEX_LEVEL_INSTANCES);
    for (;i<endInstance;++i) instances[i] = instances[i + 1]; // Shift the entire list down, overwriting the entity we're deleting at starting i
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

//                             NPC Sounds       0,   1,   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
int sfxIdle[NUM_AI_TYPES] =           {  -1,  -1,  -1, -1, 58, -1, 59, -1, 59, 52, -1, -1, -1, -1, -1, -1,121, -1, -1, -1,121,118, -1, -1, -1, -1, -1, -1, -1};
int sfxSightSound[NUM_AI_TYPES] =     {  -1,  -1, 111,150, 58,150, 59,152,152, -1,150,150,151,152,150, -1,121, -1,151,150,121,119,151, -1, -1, -1, -1, -1, -1};
int sfxAttack1[NUM_AI_TYPES] =        {  -1,  -1, 108, -1, -1,146, -1,146,252,247, -1, -1, -1, -1, -1,122, -1,108,146, -1, -1,118, -1,125,258,258,258,258,258};
int sfxAttack2[NUM_AI_TYPES] =        {  -1, 256,  -1,148, 50, 50, 50, 50, 50,250, 50, 50,146,259,148, -1,121, -1, -1,147, -1, -1,146, -1,258,258,258,258,258};
int sfxAttack3[NUM_AI_TYPES] =        {  -1,  -1,  -1, -1, -1,244,244,244,245, -1, -1,149, -1, -1, -1, -1, -1, -1, -1,244, -1, -1, -1, -1,258,258,258,258,258};
int sfxDeath[NUM_AI_TYPES] =          {  -1,  48, 110,143, 48,145, 48, 51, 47, 47,142,143,144, 47,162,123,120,134,144,144,120,117,144,124, -1, -1, -1, -1, -1};
float deathBurstTimer[NUM_AI_TYPES] = {0.0f,0.0f, 0.1f,0.0f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.0f,0.45f,0.75f,0.1f,0.0f,0.0f,0.1f,0.224f,0.9f,0.0f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f};

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

bool InstanceIsNonRenderable(uint16_t i) { return (instances[i].modelIndex >= MODEL_IDX_MAX || instances[i].modelIndex == UINT16_MAX); }

uint16_t invalidModelIndexCount;
uint16_t endOfModels;
uint16_t startOfDoubleSidedInstances, startOfTransparentInstances;
uint16_t loadedInstances;
void SortInstances(void) { // Reorder instances such that each type is grouped opaque->doublesided->transparent in that order in instances[].
    double start_time = get_time();
    DualLog("Sorting entity instances... ");
    uint16_t* modelTypeCountsOpaque = calloc(MODEL_IDX_MAX,sizeof(uint16_t));
    uint16_t* modelTypeCountsDoubleSided = calloc(MODEL_IDX_MAX,sizeof(uint16_t));
    uint16_t* modelTypeCountsTransparent = calloc(MODEL_IDX_MAX,sizeof(uint16_t));
    uint16_t* opaqueInstances      = calloc(INSTANCE_COUNT,sizeof(uint16_t));
    uint16_t* doubleSidedInstances = calloc(INSTANCE_COUNT,sizeof(uint16_t));
    uint16_t* transparentInstances = calloc(INSTANCE_COUNT,sizeof(uint16_t));
    uint16_t opaqueInstancesHead = 0, doubleSidedInstancesHead = 0, transparentInstancesHead = 0; invalidModelIndexCount = 0; startOfDoubleSidedInstances = startOfTransparentInstances = INSTANCE_COUNT;
    for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { // Skip player instances and NULLENT by starting at 3.
        if (instances[i].texIndex >= MAX_VALID_TEXTURE && instances[i].texIndex != MAX_VALID_TEXTURE) { DualLogError("Invalid texIndex %u for instance %u\n", instances[i].texIndex, i); invalidModelIndexCount++; continue; }
        if (InstanceIsNonRenderable(i)) { invalidModelIndexCount++; continue; }
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
    
    endOfModels = loadedInstances - invalidModelIndexCount;
    uint16_t currentOffset = START_INDEX_LEVEL_INSTANCES; // Compute offsets
    uint16_t i = 0;
    for (; i < MODEL_IDX_MAX; i++) { currentOffset += modelTypeCountsOpaque[i]; }
    startOfDoubleSidedInstances = currentOffset;
    for (i = 0; i < MODEL_IDX_MAX; i++) { currentOffset += modelTypeCountsDoubleSided[i]; }
    startOfTransparentInstances = currentOffset;
    for (i = 0; i < MODEL_IDX_MAX; i++) { currentOffset += modelTypeCountsTransparent[i]; }
    if ((startOfTransparentInstances + transparentInstancesHead) > endOfModels) { DualLogError("Transparent range overflow: start %u, head %u, limit %u\n", startOfTransparentInstances, transparentInstancesHead, endOfModels); OS_Exit(1); }

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
    free(modelTypeCountsOpaque); free(modelTypeCountsDoubleSided); free(modelTypeCountsTransparent);
    #if defined(LINUX) || defined(ANDROID)
        malloc_trim(0);
    #endif
    DualLog("opaque: %u, double-sided: %u, transparent: %u, invisible: %u...", opaqueInstancesHead, doubleSidedInstancesHead, transparentInstancesHead, invalidModelIndexCount);
    DualLog(" took %f secs\n", get_time() - start_time);
    ResetLevelAudio();
}

#define LINE_LEN_MAX 81920
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
void LoadLevel(uint8_t curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true;
    queuedLevelToLoad = 255u; // Reset any loading state that got us here.
    RenderLoadingProgress(100,"Loading level...");
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
    if (!file) { DualLogError("Cannot open %s\n", filename); OS_Exit(1); }

    uint32_t lineNum = 0;
    int32_t instanceIdx = PLAYER2;
    int32_t lightsIdx = -1; // Start at 0 on first loop iteration, -1 here due to ++ positioning, as it needs to iterate before each blank or commented line skip
    char lineSpace[LINE_LEN_MAX];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[LINE_LEN_MAX];
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
        while(line[0] != '\0') { // Guaranteed no leading whitespaces,k comments, or blank lines, so don't bother
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
                     if (strcmp(trimmed_key, "localPosition.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = parse_float(trimmed_value, initialLine, lineNum);
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
                flag_set(&instances[parent].entflags,ENTFLAG_TEST_OVERRIDE_TEST,true);
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

    // Add instances for shield generators
    if (curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7) {
        AddInstance(754, instanceIdx);
        instances[instanceIdx].position = (Vector3){ -51.30664f, -47.42f, 56.42651f };
        instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 0 45
        instanceIdx++;
        AddInstance(754, instanceIdx);
        instances[instanceIdx].position = (Vector3){ 71.5f, -47.42f, -66.6f };
        instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 180 45
        instanceIdx++;
        AddInstance(754, instanceIdx);
        instances[instanceIdx].position = (Vector3){ -51.306650f, -47.42f, -66.66652f };
        instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 0 -45
        instanceIdx++;
        AddInstance(754, instanceIdx);
        instances[instanceIdx].position = (Vector3){ 71.78664f, -47.42f, 56.42651f };
        instances[instanceIdx].rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f }; // -90 180 -45
        instanceIdx++;
    }
    
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
    RenderLoadingProgress(110,"Loading models...");
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
    }
    
    float levelMinFloor = FLT_MAX;
    float levelMaxCeil = -FLT_MAX;
    for (int i=0;i<ARRSIZE;++i) { //        Using 1.0f buffer for floating point innaccuracies
        if (gridCellFloorHeight[i] > (-FLT_MAX +  1.0f) && gridCellFloorHeight[i] < levelMinFloor) levelMinFloor = gridCellFloorHeight[i];
        if (gridCellCeilingHeight[i] < (FLT_MAX - 1.0f) && gridCellCeilingHeight[i] > levelMaxCeil) levelMaxCeil = gridCellCeilingHeight[i];
    }
    
    for (int i=0;i<ARRSIZE;++i) { //         Using 1.0f buffer for floating point innaccuracies
        if (gridCellFloorHeight[i] <= (-FLT_MAX +  1.0f)) gridCellFloorHeight[i] = levelMinFloor;
        if (gridCellCeilingHeight[i] >= (FLT_MAX - 1.0f)) gridCellCeilingHeight[i] = levelMaxCeil;
    }

    RenderLoadingProgress(110,"Loading textures...");
    LoadTextures();
    SortInstances(); // All instances loaded, sort them for render order: opaques, doublesideds, transparents.  REORDERS instances[] INDICES!!  CAREFUL!!
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) dirtyInstances[i] = true;
    for (uint16_t i = 0; i < loadedLights; i++) {
        uint32_t litIdx = i * LIGHT_DATA_SIZE; // lightDirty[i] = true is already done in PortalCulling, leaving commented out here for confirmation.
        lightsNewPosition[i] = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        lightInPVS[i] = false;
    }
    memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
    DualLog("LoadLevel completed!\n");
}
#pragma GCC diagnostic pop
