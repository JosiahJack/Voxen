// init.c - Entity Initialization
#include "voxen.h"
#define FLT_MAX 3.402823466e+38F
void ButtonSwitchInit(void) {
    SELF.delayFinished = 0.0f; // prevent using targets on awake
    if (SELF.entflags & ENTFLAG_ACTIVE) SELF.tickFinished = Sys_Global.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ForceBridgeInit(void) {    
    SELF.tickFinished = Sys_Global.pauseRelativeTime + SELF.tickTime + (double)random_range(0.0f,1.0f);
    SELF.lerping = true;
    if (SELF.activatedScale.x <= 0.02f) SELF.activatedScale.x = 2.56f;
    if (SELF.activatedScale.y <= 0.02f) SELF.activatedScale.y = 0.08f;
    if (SELF.activatedScale.z <= 0.02f) SELF.activatedScale.z = 2.56f;
    if (!(SELF.entflags & ENTFLAG_ACTIVATED)) {
        flag_set(&SELF.entflags,ENTFLAG_VISIBLE,false);
        SELF.collider = COLLIDER_TYPE_NONE;
    }
    
    switch (SELF.fieldColor) {
        case ForceFieldColor_Red:      SELF.texIndex = 38; break;
        case ForceFieldColor_Green:    SELF.texIndex = 40; break;
        case ForceFieldColor_Blue:     SELF.texIndex = 39; break;
        case ForceFieldColor_Purple:   SELF.texIndex = 41; break;
        case ForceFieldColor_RedFaint: SELF.texIndex = 198; break;
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

void DriftUpdInit() {
    SELF..position = (Vector3){instances[i].position.x,startY,instances[i].position.z};
    if (fadeImage && img != null) {
        img.color = new Color(img.color.r,img.color.g,img.color.b,startFade);
    }

    SELF.tickFinished = Sys_Global.pauseRelativeTime;
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

void InitAfterLoad(void) { // Init entities after level load and after already having generic entity type fields set.
    for (int i=0;i<ARRSIZE;++i) { gridCellFloorHeight[i] = -FLT_MAX; gridCellCeilingHeight[i] = FLT_MAX;}
    for (int i=PLAYER1;i<loadedInstances;++i) {
        selfIdx = i;
        
        int32_t cellIdx = PosGetCellCoords(SELF.position.x, SELF.position.z);
        SELF.cellIndex = cellIdx;
        if (i == PLAYER1 || i == PLAYER2 || ConstIndexIsDynamicObject(SELF.index)) SELF.gravity = 1.0f; // Normal gravity
        else SELF.gravity = 0.0f;
        
        if (SELF.index < MAX_ENTITIES) flag_set(&SELF.entflags,ENTFLAG_ANIMATED,entities[SELF.index].entflags & ENTFLAG_ANIMATED);
        if (SELF.entflags & ENTFLAG_HAS_CAMERA_VIEW) AddCameraPosition(i);

        if (SELF.collider == COLLIDER_TYPE_BOX) {
            Quaternion quat = SELF.rotation;
            Quaternion upQuat = {0.0f, 0.0f, 0.0f, 1.0f};
            float floorangle = quat_angle_deg(quat,upQuat); // Get angle in degrees relative to up vector (floor normal)
            Quaternion downQuat = {0.0f, 0.0f, 0.0f, -1.0f};
            float ceilangle = quat_angle_deg(quat,downQuat); // Get angle in degrees relative to down vector (ceiling normal)
            float floorHeight = (floorangle <= 30.0f) ? SELF.position.y - 1.28f : -FLT_MAX; // World cells are 2.56x2.56x2.56 with modular chunk origins at center, so offset by half cell size to get actual positions.
            if (floorHeight > -FLT_MAX && floorHeight > gridCellFloorHeight[cellIdx]) gridCellFloorHeight[cellIdx] = floorHeight; // Raise floor up until highest one is selected.
            float ceilHeight = (ceilangle <= 30.0f) ? SELF.position.y + 1.28f : FLT_MAX;
            if (ceilHeight < FLT_MAX && ceilHeight < gridCellCeilingHeight[cellIdx]) gridCellCeilingHeight[cellIdx] = ceilHeight; // Raise floor up until highest one is selected.
        }
        
        // Entity Specific Inits
        if (ConstIndexIsButtonSwitch(SELF.index)) ButtonSwitchInit();
        if (!StringIsEmpty(SELF.targetname) && (SELF.ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE) && !(SELF.entflags & TARG_IOFLAGS_DISABLD_ONCE_4EVER)) flag_set(&SELF.entflags,ENTFLAG_ACTIVE,false);
        if (SELF.index == 700) { // logic_branch
            if ((SELF.ioflags & TARG_IOFLAGS_START_ON_SECOND) || (SELF.ioflags & TARG_IOFLAGS_ON_SECOND)) { StringCopyInto_A_From_B(SELF.currenttarget,SELF.target,TARGET_STRING_LENGTH); flag_set(&SELF.ioflags,TARG_IOFLAGS_ON_SECOND,false); }
            else { StringCopyInto_A_From_B(SELF.currenttarget,SELF.target2,TARGET_STRING_LENGTH); flag_set(&SELF.ioflags,TARG_IOFLAGS_ON_SECOND,true); }
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

    inventoryPlayer1.generalInventoryIndexRef[0] = 81;
    inventoryPlayer1.nitroTimeSetting = NITRO_DEFAULT_TIME;
    inventoryPlayer1.earthShakerTimeSetting = EARTH_SHAKER_DEFAULT_TIME;
    inventoryPlayer1.lastAddedIndex = -1;
    inventoryPlayer1.hasNewEmail = true;
    inventoryPlayer1.hasNewNotes = true;
    inventoryPlayer1.currentCyberItem = -1;
    inventoryPlayer1.isPulserNotDrill = true;
/*    inventoryPlayer1.weaponInventoryIndices = {-1,-1,-1,-1,-1,-1,-1};
    inventoryPlayer1.weaponInventoryAmmoIndices = {-1,-1,-1,-1,-1,-1,-1};*/	
    inventoryPlayer1.globalLookupIndex = -1;
}

void PlayerInit(uint16_t i) {
    selfIdx = i;
    SELF.index = 767;
    SELF.layer = 12; // PhysicsLayer_Player
    SELF.position = (Vector3) { .x = 10.52f, .y = -43.792f + 0.84f, .z = 20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f y for cam offset from center
    SELF.scale = (Vector3) { 1.0f, 1.0f, 1.0f };
    SELF.rotation = (Quaternion){ .x = 0.0f, .y = 0.7071f, .z = 0.0f, .w = 0.7071f }; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    SELF.entflags = ENTFLAG_ACTIVE | ENTFLAG_USEGRAVITY | ENTFLAG_RIGIDBODY;
    SELF.collider = COLLIDER_TYPE_CAPSULE;
    SELF.colliderCenter.y = 0.84f;
    SELF.colliderSize = (Vector3) { .x = 0.48f, .y = 2.0f, .z = 1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    SELF.mass = 1.0f;
    SELF.linearDrag = 8.0f;
    SELF.dynamicFriction = 0.6f;
    SELF.staticFriction = 0.8f;
    SELF.frictionCombine = PHYS_COMBINE_MUL;
    SELF.health = 200.0f;
    SELF.lastHealth = SELF.health;
    SELF.energyDrainTickFinished = Sys_Global.pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);
    SELF.energy = 54.0f;
    SELF.maxEnergy = 255.0f;
    SELF.resetAfterDeathTime = 0.5;
    SELF.painSoundFinished = Sys_Global.pauseRelativeTime;
    SELF.radSoundFinished = Sys_Global.pauseRelativeTime;
    SELF.radFXFinished = Sys_Global.pauseRelativeTime;
    SELF.noiseFinished = Sys_Global.pauseRelativeTime;
    if (i == PLAYER1) MFDInit(); // TODO Make this check if i == current player on this PC
    if (i == PLAYER1) InventoryInit(); // TODO Make this check if i == current player
}
