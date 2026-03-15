// init.c - Entity Initialization
#include "voxen.h"
#define FLT_MAX 3.402823466e+38F
void ButtonSwitchInit(uint16_t self) {
    Sys_Global.instances[self].delayFinished = 0.0f; // prevent using targets on awake
    if (Sys_Global.instances[self].entflags & ENTFLAG_ACTIVE) Sys_Global.instances[self].tickFinished = Sys_Global.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ForceBridgeInit(uint16_t self) {    
    Sys_Global.instances[self].tickFinished = Sys_Global.pauseRelativeTime + Sys_Global.instances[self].tickTime + (double)random_range(0.0f,1.0f);
    Sys_Global.instances[self].lerping = true;
    if (Sys_Global.instances[self].activatedScale.x <= 0.02f) Sys_Global.instances[self].activatedScale.x = 2.56f;
    if (Sys_Global.instances[self].activatedScale.y <= 0.02f) Sys_Global.instances[self].activatedScale.y = 0.08f;
    if (Sys_Global.instances[self].activatedScale.z <= 0.02f) Sys_Global.instances[self].activatedScale.z = 2.56f;
    if (!(Sys_Global.instances[self].entflags & ENTFLAG_ACTIVATED)) {
        flag_set(&Sys_Global.instances[self].entflags,ENTFLAG_VISIBLE,false);
        Sys_Global.instances[self].collider = COLLIDER_TYPE_NONE;
    }
    
    switch (Sys_Global.instances[self].fieldColor) {
        case ForceFieldColor_Red:      Sys_Global.instances[self].texIndex = 38; break;
        case ForceFieldColor_Green:    Sys_Global.instances[self].texIndex = 40; break;
        case ForceFieldColor_Blue:     Sys_Global.instances[self].texIndex = 39; break;
        case ForceFieldColor_Purple:   Sys_Global.instances[self].texIndex = 41; break;
        case ForceFieldColor_RedFaint: Sys_Global.instances[self].texIndex = 198; break;
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
    Sys_Global.instances[self]..position = (Vector3){Sys_Global.instances[i].position.x,startY,Sys_Global.instances[i].position.z};
    if (fadeImage && img != null) {
        img.color = new Color(img.color.r,img.color.g,img.color.b,startFade);
    }

    Sys_Global.instances[self].tickFinished = Sys_Global.pauseRelativeTime;
}*/

//=============================================================================
void ResetLevelAudio(void);
void InitAfterLoad(void) { // Init entities after level load and after already having generic entity type fields set.
    for (int i=0;i<ARRSIZE;++i) { gridCellFloorHeight[i] = -FLT_MAX; gridCellCeilingHeight[i] = FLT_MAX;}
    for (int i=PLAYER1;i<loadedInstances;++i) {        
        int32_t cellIdx = PosGetCellCoords(Sys_Global.instances[i].position.x, Sys_Global.instances[i].position.z);
        Sys_Global.instances[i].cellIndex = cellIdx;
        if (i == PLAYER1 || i == PLAYER2 || ConstIndexIsDynamicObject(Sys_Global.instances[i].index)) Sys_Global.instances[i].gravity = 1.0f; // Normal gravity
        else Sys_Global.instances[i].gravity = 0.0f;
        
        if (Sys_Global.instances[i].index < MAX_ENTITIES) flag_set(&Sys_Global.instances[i].entflags,ENTFLAG_ANIMATED,entities[Sys_Global.instances[i].index].entflags & ENTFLAG_ANIMATED);
        if (Sys_Global.instances[i].entflags & ENTFLAG_HAS_CAMERA_VIEW) AddCameraPosition(i);
        if (Sys_Global.instances[i].collider == COLLIDER_TYPE_BOX) {
            Quaternion quat = Sys_Global.instances[i].rotation;
            Quaternion upQuat = {0.0f, 0.0f, 0.0f, 1.0f};
            float floorangle = quat_angle_deg(quat,upQuat); // Get angle in degrees relative to up vector (floor normal)
            Quaternion downQuat = {0.0f, 0.0f, 0.0f, -1.0f};
            float ceilangle = quat_angle_deg(quat,downQuat); // Get angle in degrees relative to down vector (ceiling normal)
            float floorHeight = (floorangle <= 30.0f) ? Sys_Global.instances[i].position.y - 1.28f : -FLT_MAX; // World cells are 2.56x2.56x2.56 with modular chunk origins at center, so offset by half cell size to get actual positions.
            if (floorHeight > -FLT_MAX && floorHeight > gridCellFloorHeight[cellIdx]) gridCellFloorHeight[cellIdx] = floorHeight; // Raise floor up until highest one is selected.
            float ceilHeight = (ceilangle <= 30.0f) ? Sys_Global.instances[i].position.y + 1.28f : FLT_MAX;
            if (ceilHeight < FLT_MAX && ceilHeight < gridCellCeilingHeight[cellIdx]) gridCellCeilingHeight[cellIdx] = ceilHeight; // Raise floor up until highest one is selected.
        }
        
        // Entity Specific Inits
        if (ConstIndexIsGeometry(Sys_Global.instances[i].index)) Sys_Global.instances[i].layer = PhysicsLayer_Geometry;
        if (ConstIndexIsDoor(Sys_Global.instances[i].index)) Sys_Global.instances[i].layer = PhysicsLayer_Door;
        if (ConstIndexIsNPC(Sys_Global.instances[i].index)) Sys_Global.instances[i].layer = PhysicsLayer_NPC;
        if (ConstIndexIsButtonSwitch(Sys_Global.instances[i].index)) ButtonSwitchInit(i);
        if (!StringIsEmpty(Sys_Global.instances[i].targetname) && (Sys_Global.instances[i].ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE) && !(Sys_Global.instances[i].entflags & TARG_IOFLAGS_DISABLD_ONCE_4EVER)) flag_set(&Sys_Global.instances[i].entflags,ENTFLAG_ACTIVE,false);
        if (Sys_Global.instances[i].index == 700) { // logic_branch
            if ((Sys_Global.instances[i].ioflags & TARG_IOFLAGS_START_ON_SECOND) || (Sys_Global.instances[i].ioflags & TARG_IOFLAGS_ON_SECOND)) {
                StringCopyInto_A_From_B(Sys_Global.instances[i].currenttarget,Sys_Global.instances[i].target,TARGET_STRING_LENGTH);
                flag_set(&Sys_Global.instances[i].ioflags,TARG_IOFLAGS_ON_SECOND,false);
            } else { StringCopyInto_A_From_B(Sys_Global.instances[i].currenttarget,Sys_Global.instances[i].target2,TARGET_STRING_LENGTH); flag_set(&Sys_Global.instances[i].ioflags,TARG_IOFLAGS_ON_SECOND,true); }
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
    ResetLevelMusic();
    DualLog("Entity instances initialized after load\n");
}

void MFDInit(SystemUI* ui) {
    ui->lastMultiMediaTabOpened = MULTI_MEDIA_TAB_EMAIL_TABLE;
    ui->logFinished = Sys_Global.pauseRelativeTime;
    ui->tickFinished = ui->centerTabsTickFinished = Sys_Global.current_time + 0.1 + (double)random_range(0.0f,1.0f);
    ui->blinkFinished = 1.0 + Sys_Global.pauseRelativeTime;
    ui->beepFinished = 3.0 + Sys_Global.pauseRelativeTime;
}

void InventoryInit(InventorySystem* inv) {
    inv->hardwareInvReferenceIndex[0]  = 21; // Hardcoded lookup indices into the Const main table.
    inv->hardwareInvReferenceIndex[1]  = 22;
    inv->hardwareInvReferenceIndex[2]  = 23;
    inv->hardwareInvReferenceIndex[3]  = 24;
    inv->hardwareInvReferenceIndex[4]  = 25;
    inv->hardwareInvReferenceIndex[5]  = 26;
    inv->hardwareInvReferenceIndex[6]  = 27;
    inv->hardwareInvReferenceIndex[7]  = 28;
    inv->hardwareInvReferenceIndex[8]  = 29;
    inv->hardwareInvReferenceIndex[9]  = 30;
    inv->hardwareInvReferenceIndex[10] = 31;
    inv->hardwareInvReferenceIndex[11] = 32;
    inv->hardwareInvReferenceIndex[12] =  0;
    inv->hardwareInvReferenceIndex[13] =  0;
    for (int i = 0; i < 14; i++) {
        if (i != 0) inv->generalInventoryIndexRef[i] = -1;
    }
    
    for (int i=0;i<HW_COUNT;++i) inv->hardwareVersion[i] = 0;
    for (int i=0;i<HW_COUNT;++i) inv->hardwareVersionSetting[i] = 0;
    for (int i=0;i<HW_COUNT;++i) inv->hardwareInvReferenceIndex[i] = 0;
    inv->generalInventoryIndexRef[0] = 81;
    inv->nitroTimeSetting = NITRO_DEFAULT_TIME;
    inv->earthShakerTimeSetting = EARTH_SHAKER_DEFAULT_TIME;
    inv->lastAddedIndex = -1;
    inv->hasNewEmail = true;
    inv->hasNewNotes = true;
    inv->currentCyberItem = -1;
    inv->isPulserNotDrill = true;
    for (int i=0;i<7;++i) inv->weaponInventoryIndices[i]     = -1;
    for (int i=0;i<7;++i) inv->weaponInventoryAmmoIndices[i] = -1;
    inv->globalLookupIndex = -1;
    inv->sparqSetting = 50.0f;
    inv->ionSetting = 100.0f;
    inv->blasterSetting = 15.0f;
    inv->plasmaSetting = 40.0f;
    inv->stungunSetting = 20.0f;
    inv->justFired = (Sys_Global.pauseRelativeTime - 31.0); // Set less than 30s before pauseRelativeTime so we don't immediately play action music.
}

void PlayerInit(uint16_t i) {
    Sys_Global.instances[i].index = 767;
    Sys_Global.instances[i].layer = 12; // PhysicsLayer_Player
    Sys_Global.instances[i].position = (Vector3) { .x = 10.52f, .y = -43.792f + 0.84f, .z = 20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f y for cam offset from center
    Sys_Global.instances[i].scale = (Vector3) { 1.0f, 1.0f, 1.0f };
    Sys_Global.instances[i].rotation = (Quaternion){ .x = 0.0f, .y = 0.7071f, .z = 0.0f, .w = 0.7071f }; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    Sys_Global.instances[i].entflags = ENTFLAG_ACTIVE | ENTFLAG_USEGRAVITY | ENTFLAG_RIGIDBODY;
    Sys_Global.instances[i].collider = COLLIDER_TYPE_CAPSULE;
    Sys_Global.instances[i].colliderCenter.y = 0.84f;
    Sys_Global.instances[i].colliderSize = (Vector3){.x = 0.48f, .y = 2.0f, .z = 1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    Sys_Global.instances[i].mass = 1.0f;
    Sys_Global.instances[i].linearDrag = 8.0f;
    Sys_Global.instances[i].velocity = (Vector3){0.0f,0.0f,0.0f};
    Sys_Global.instances[i].dynamicFriction = 0.6f;
    Sys_Global.instances[i].staticFriction = 0.8f;
    Sys_Global.instances[i].frictionCombine = PHYS_COMBINE_MUL;
    Sys_Global.instances[i].health = 200.0f;
    Sys_Global.instances[i].lastHealth = Sys_Global.instances[i].health;
    Sys_Global.instances[i].energyDrainTickFinished = Sys_Global.pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);
    Sys_Global.instances[i].energy = 54.0f;
    Sys_Global.instances[i].maxEnergy = 255.0f;
    Sys_Global.instances[i].resetAfterDeathTime = 0.5;
    Sys_Global.instances[i].painSoundFinished = Sys_Global.pauseRelativeTime;
    Sys_Global.instances[i].radSoundFinished = Sys_Global.pauseRelativeTime;
    Sys_Global.instances[i].radFXFinished = Sys_Global.pauseRelativeTime;
    Sys_Global.instances[i].noiseFinished = Sys_Global.pauseRelativeTime;
    if (i == PLAYER1) MFDInit(&Sys_UIPlayer1);
    else if (i == PLAYER2) MFDInit(&Sys_UIPlayer2);
    if (i == PLAYER1) InventoryInit(&Sys_Global.inventoryPlayer1);
    else if (i == PLAYER2) InventoryInit(&Sys_Global.inventoryPlayer2);
}
