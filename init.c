// init.c - Entity Initialization
#include "voxen.h"
#define FLT_MAX 3.402823466e+38F
void ButtonSwitchInit(uint16_t self) {
    instances[self].delayFinished = 0.0f; // prevent using targets on awake
    if (instances[self].entflags & ENTFLAG_ACTIVE) instances[self].tickFinished = Sys_Global.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ForceBridgeInit(uint16_t self) {    
    instances[self].tickFinished = Sys_Global.pauseRelativeTime + instances[self].tickTime + (double)random_range(0.0f,1.0f);
    instances[self].lerping = true;
    if (instances[self].activatedScale.x <= 0.02f) instances[self].activatedScale.x = 2.56f;
    if (instances[self].activatedScale.y <= 0.02f) instances[self].activatedScale.y = 0.08f;
    if (instances[self].activatedScale.z <= 0.02f) instances[self].activatedScale.z = 2.56f;
    if (!(instances[self].entflags & ENTFLAG_ACTIVATED)) {
        flag_set(&instances[self].entflags,ENTFLAG_VISIBLE,false);
        instances[self].collider = COLLIDER_TYPE_NONE;
    }
    
    switch (instances[self].fieldColor) {
        case ForceFieldColor_Red:      instances[self].texIndex = 38; break;
        case ForceFieldColor_Green:    instances[self].texIndex = 40; break;
        case ForceFieldColor_Blue:     instances[self].texIndex = 39; break;
        case ForceFieldColor_Purple:   instances[self].texIndex = 41; break;
        case ForceFieldColor_RedFaint: instances[self].texIndex = 198; break;
    }
}
/*
// DriftUp for playerPizzaz on minigames TODO
float startY;
float endY;
float rate = 0.5f;
float fadeRate = 0.1f;
bool fadeImage;
Image img;
float startFade = 1.0f;
float endFade = 0.0f;
float tickFinished;

void DriftUpdInit(uint16_t self) {
    instances[self]..position = (Vector3){instances[i].position.x,startY,instances[i].position.z};
    if (fadeImage && img != null) {
        img.color = new Color(img.color.r,img.color.g,img.color.b,startFade);
    }

    instances[self].tickFinished = Sys_Global.pauseRelativeTime;
}*/

//=============================================================================
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
    for (int i=0;i<ARRSIZE;++i) { gridCellFloorHeight[i] = -FLT_MAX; gridCellCeilingHeight[i] = FLT_MAX;}
    for (int i=PLAYER1;i<loadedInstances;++i) {        
        int32_t cellIdx = PosGetCellCoords(instances[i].position.x, instances[i].position.z);
        instances[i].cellIndex = cellIdx;
        if (i == PLAYER1 || i == PLAYER2 || ConstIndexIsDynamicObject(instances[i].index)) instances[i].gravity = 1.0f; // Normal gravity
        else instances[i].gravity = 0.0f;
        
        if (instances[i].index < MAX_ENTITIES) flag_set(&instances[i].entflags,ENTFLAG_ANIMATED,entities[instances[i].index].entflags & ENTFLAG_ANIMATED);
        if (instances[i].entflags & ENTFLAG_HAS_CAMERA_VIEW) AddCameraPosition(i);
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
        }
        
        // Entity Specific Inits
        if (ConstIndexIsGeometry(instances[i].index)) instances[i].layer = PhysicsLayer_Geometry;
        if (ConstIndexIsDoor(instances[i].index)) instances[i].layer = PhysicsLayer_Door;
        if (ConstIndexIsNPC(instances[i].index)) instances[i].layer = PhysicsLayer_NPC;
        if (ConstIndexIsButtonSwitch(instances[i].index)) ButtonSwitchInit(i);
        if (!StringIsEmpty(instances[i].targetname) && (instances[i].ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE) && !(instances[i].entflags & TARG_IOFLAGS_DISABLD_ONCE_4EVER)) flag_set(&instances[i].entflags,ENTFLAG_ACTIVE,false);
        if (instances[i].index == 700) { // logic_branch
            if ((instances[i].ioflags & TARG_IOFLAGS_START_ON_SECOND) || (instances[i].ioflags & TARG_IOFLAGS_ON_SECOND)) {
                StringCopyInto_A_From_B(instances[i].currenttarget,instances[i].target,TARGET_STRING_LENGTH);
                flag_set(&instances[i].ioflags,TARG_IOFLAGS_ON_SECOND,false);
            } else { StringCopyInto_A_From_B(instances[i].currenttarget,instances[i].target2,TARGET_STRING_LENGTH); flag_set(&instances[i].ioflags,TARG_IOFLAGS_ON_SECOND,true); }
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
    
    ResetLevelAudio();
}

void MFDInit(void) {
    Sys_UI.lastMultiMediaTabOpened = MULTI_MEDIA_TAB_EMAIL_TABLE;
    Sys_UI.logFinished = Sys_Global.pauseRelativeTime;
    Sys_UI.tickFinished = Sys_UI.centerTabsTickFinished = Sys_Global.current_time + 0.1 + (double)random_range(0.0f,1.0f);
    Sys_UI.blinkFinished = 1.0 + Sys_Global.pauseRelativeTime;
    Sys_UI.beepFinished = 3.0 + Sys_Global.pauseRelativeTime;
}

void InventoryInit(void) {
    inventoryPlayer1.hardwareInvReferenceIndex[0]  = 21; // Hardcoded lookup indices into the Const main table.
    inventoryPlayer1.hardwareInvReferenceIndex[1]  = 22;
    inventoryPlayer1.hardwareInvReferenceIndex[2]  = 23;
    inventoryPlayer1.hardwareInvReferenceIndex[3]  = 24;
    inventoryPlayer1.hardwareInvReferenceIndex[4]  = 25;
    inventoryPlayer1.hardwareInvReferenceIndex[5]  = 26;
    inventoryPlayer1.hardwareInvReferenceIndex[6]  = 27;
    inventoryPlayer1.hardwareInvReferenceIndex[7]  = 28;
    inventoryPlayer1.hardwareInvReferenceIndex[8]  = 29;
    inventoryPlayer1.hardwareInvReferenceIndex[9]  = 30;
    inventoryPlayer1.hardwareInvReferenceIndex[10] = 31;
    inventoryPlayer1.hardwareInvReferenceIndex[11] = 32;
    inventoryPlayer1.hardwareInvReferenceIndex[12] =  0;
    inventoryPlayer1.hardwareInvReferenceIndex[13] =  0;
    for (int i = 0; i < 14; i++) {
        if (i != 0) inventoryPlayer1.generalInventoryIndexRef[i] = -1;
    }
    
    for (int i=0;i<HW_COUNT;++i) inventoryPlayer1.hardwareVersion[i] = 0;
    for (int i=0;i<HW_COUNT;++i) inventoryPlayer1.hardwareVersionSetting[i] = 0;
    for (int i=0;i<HW_COUNT;++i) inventoryPlayer1.hardwareInvReferenceIndex[i] = 0;
    inventoryPlayer1.generalInventoryIndexRef[0] = 81;
    inventoryPlayer1.nitroTimeSetting = NITRO_DEFAULT_TIME;
    inventoryPlayer1.earthShakerTimeSetting = EARTH_SHAKER_DEFAULT_TIME;
    inventoryPlayer1.lastAddedIndex = -1;
    inventoryPlayer1.hasNewEmail = true;
    inventoryPlayer1.hasNewNotes = true;
    inventoryPlayer1.currentCyberItem = -1;
    inventoryPlayer1.isPulserNotDrill = true;
    for (int i=0;i<7;++i) inventoryPlayer1.weaponInventoryIndices[i]     = -1;
    for (int i=0;i<7;++i) inventoryPlayer1.weaponInventoryAmmoIndices[i] = -1;
    inventoryPlayer1.globalLookupIndex = -1;
}

void PlayerInit(uint16_t i) {
    instances[i].index = 767;
    instances[i].layer = 12; // PhysicsLayer_Player
    instances[i].position = (Vector3) { .x = 10.52f, .y = -43.792f + 0.84f, .z = 20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f y for cam offset from center
    instances[i].scale = (Vector3) { 1.0f, 1.0f, 1.0f };
    instances[i].rotation = (Quaternion){ .x = 0.0f, .y = 0.7071f, .z = 0.0f, .w = 0.7071f }; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    instances[i].entflags = ENTFLAG_ACTIVE | ENTFLAG_USEGRAVITY | ENTFLAG_RIGIDBODY;
    instances[i].collider = COLLIDER_TYPE_CAPSULE;
    instances[i].colliderCenter.y = 0.84f;
    instances[i].colliderSize = (Vector3){.x = 0.48f, .y = 2.0f, .z = 1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    instances[i].mass = 1.0f;
    instances[i].linearDrag = 8.0f;
    instances[i].velocity = (Vector3){0.0f,0.0f,0.0f};
    instances[i].dynamicFriction = 0.6f;
    instances[i].staticFriction = 0.8f;
    instances[i].frictionCombine = PHYS_COMBINE_MUL;
    instances[i].health = 200.0f;
    instances[i].lastHealth = instances[i].health;
    instances[i].energyDrainTickFinished = Sys_Global.pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);
    instances[i].energy = 54.0f;
    instances[i].maxEnergy = 255.0f;
    instances[i].resetAfterDeathTime = 0.5;
    instances[i].painSoundFinished = Sys_Global.pauseRelativeTime;
    instances[i].radSoundFinished = Sys_Global.pauseRelativeTime;
    instances[i].radFXFinished = Sys_Global.pauseRelativeTime;
    instances[i].noiseFinished = Sys_Global.pauseRelativeTime;
    if (i == PLAYER1) MFDInit(); // TODO Make this check if i == current player on this PC
    if (i == PLAYER1) InventoryInit(); // TODO Make this check if i == current player
}
