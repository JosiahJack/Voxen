// init.c - Entity Initialization
#include "mod.h"
// From Engine
GlobalContext* Eng_Global = 0; CheatsSystem* Eng_Cheats; SettingsSystem* Eng_Settings;
MOD_TO_ENGINE void ModLink(GlobalContext* globals, CheatsSystem* cheats, SettingsSystem* settings) {Eng_Global = globals; Eng_Cheats = cheats; Eng_Settings = settings; }

uint8_t GetCurrentLevelSecurity(void) { return (Eng_Global->difficultyMission < 1 || Eng_Cheats->superoverride) ? 0u : Eng_Global->levelSecurity[Eng_Global->currentLevel]; }
uint16_t GetImpactType(uint16_t instanceIdx) {
    switch (Eng_Global->instances[instanceIdx].bloodType) {
        case BloodType_None:         return 729; // SparksSmall
        case BloodType_Red:          return 724; // BloodSpurtSmall
        case BloodType_Yellow:       return 723; // BloodSpurtSmallYellow
        case BloodType_Green:        return 722; // BloodSpurtSmallGreen
        case BloodType_Robot:        return 730; // SparksSmallBlue
        case BloodType_Leaf:         return 756; // LeafBurst
        case BloodType_Mutation:     return 757; // MutationBurst
        case BloodType_GrayMutation: return 758; // GraytationBurst
    }

    return 729; // SparksSmall
}

void ButtonSwitchInit(uint16_t self) {
    Eng_Global->instances[self].delayFinished = 0.0f; // prevent using targets on awake
    if (Eng_Global->instances[self].entflags & ENTFLAG_ACTIVE) Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ForceBridgeInit(uint16_t self) {    
    Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[self].tickTime + (double)random_range(0.0f,1.0f);
    Eng_Global->instances[self].lerping = true;
    if (Eng_Global->instances[self].activatedScale.x <= 0.02f) Eng_Global->instances[self].activatedScale.x = 2.56f;
    if (Eng_Global->instances[self].activatedScale.y <= 0.02f) Eng_Global->instances[self].activatedScale.y = 0.08f;
    if (Eng_Global->instances[self].activatedScale.z <= 0.02f) Eng_Global->instances[self].activatedScale.z = 2.56f;
    if (!(Eng_Global->instances[self].entflags & ENTFLAG_ACTIVATED)) {
        flag_set(&Eng_Global->instances[self].entflags,ENTFLAG_VISIBLE,false);
        Eng_Global->instances[self].collider = COLLIDER_TYPE_NONE;
    }
    
    switch (Eng_Global->instances[self].fieldColor) {
        case ForceFieldColor_Red:      Eng_Global->instances[self].texIndex = 38; break;
        case ForceFieldColor_Green:    Eng_Global->instances[self].texIndex = 40; break;
        case ForceFieldColor_Blue:     Eng_Global->instances[self].texIndex = 39; break;
        case ForceFieldColor_Purple:   Eng_Global->instances[self].texIndex = 41; break;
        case ForceFieldColor_RedFaint: Eng_Global->instances[self].texIndex = 198; break;
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
    Eng_Global->instances[self]..position = (Vector3){Eng_Global->instances[i].position.x,startY,Eng_Global->instances[i].position.z};
    if (fadeImage && img != null) {
        img.color = new Color(img.color.r,img.color.g,img.color.b,startFade);
    }

    Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime;
}*/

//=============================================================================

// void MFDInit(SystemUI* ui) {
//     ui->lastMultiMediaTabOpened = MULTI_MEDIA_TAB_EMAIL_TABLE;
//     ui->logFinished = Eng_Global->pauseRelativeTime;
//     ui->tickFinished = ui->centerTabsTickFinished = Eng_Global->current_time + 0.1 + (double)random_range(0.0f,1.0f);
//     ui->blinkFinished = 1.0 + Eng_Global->pauseRelativeTime;
//     ui->beepFinished = 3.0 + Eng_Global->pauseRelativeTime;
// }

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
    inv->generalInventoryIndexRef[0] = 81;
    for (int i = 1; i < HW_COUNT; i++) inv->generalInventoryIndexRef[i] = -1;
    for (int i=0;i<HW_COUNT;++i) inv->hardwareVersion[i] = 0;
    for (int i=0;i<HW_COUNT;++i) inv->hardwareVersionSetting[i] = 0;
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
    inv->justFired = (Eng_Global->pauseRelativeTime - 31.0); // Set less than 30s before pauseRelativeTime so we don't immediately play action music.
}

MOD_TO_ENGINE void PlayerInit(uint16_t i) {
    DualLog("Entered mod function PlayerInit()\n");
    Eng_Global->instances[i].index = 767;
    Eng_Global->instances[i].layer = 12; // PhysicsLayer_Player
    Eng_Global->instances[i].position = (Vector3) { .x = 10.52f, .y = -43.792f + 0.84f, .z = 20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f y for cam offset from center
    Eng_Global->instances[i].scale = (Vector3) { 1.0f, 1.0f, 1.0f };
    Eng_Global->instances[i].rotation = (Quaternion){ .x = 0.0f, .y = 0.7071f, .z = 0.0f, .w = 0.7071f }; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    Eng_Global->instances[i].entflags = ENTFLAG_ACTIVE | ENTFLAG_USEGRAVITY | ENTFLAG_RIGIDBODY;
    Eng_Global->instances[i].collider = COLLIDER_TYPE_CAPSULE;
    Eng_Global->instances[i].colliderCenter.y = 0.84f;
    Eng_Global->instances[i].colliderSize = (Vector3){.x = 0.48f, .y = 2.0f, .z = 1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    Eng_Global->instances[i].mass = 1.0f;
    return;
    Eng_Global->instances[i].linearDrag = 8.0f;
    Eng_Global->instances[i].velocity = (Vector3){0.0f,0.0f,0.0f};
    Eng_Global->instances[i].dynamicFriction = 0.6f;
    Eng_Global->instances[i].staticFriction = 0.8f;
    Eng_Global->instances[i].frictionCombine = PHYS_COMBINE_MUL;
    Eng_Global->instances[i].health = 200.0f;
    Eng_Global->instances[i].lastHealth = Eng_Global->instances[i].health;
    Eng_Global->instances[i].energyDrainTickFinished = Eng_Global->pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);
    Eng_Global->instances[i].energy = 54.0f;
    Eng_Global->instances[i].maxEnergy = 255.0f;
    Eng_Global->instances[i].resetAfterDeathTime = 0.5;
    Eng_Global->instances[i].painSoundFinished = Eng_Global->pauseRelativeTime;
    Eng_Global->instances[i].radSoundFinished = Eng_Global->pauseRelativeTime;
    Eng_Global->instances[i].radFXFinished = Eng_Global->pauseRelativeTime;
    Eng_Global->instances[i].noiseFinished = Eng_Global->pauseRelativeTime;
//     if (i == PLAYER1) MFDInit(&Sys_UIPlayer1);
//     else if (i == PLAYER2) MFDInit(&Sys_UIPlayer2);
    
    if (i == PLAYER1) InventoryInit(&Eng_Global->inventoryPlayer1);
    else if (i == PLAYER2) InventoryInit(&Eng_Global->inventoryPlayer2);
}

#define GEOMETRY_LOD_CARD_MODEL_IDX 178
MOD_TO_ENGINE void ModEntityDefinitionsInitAfterLoad(DataParser* entity_parser) {
    for (int32_t i = 0; i < Eng_Global->entityCount; i++) {
        if (entity_parser->entries[i].index == UINT16_MAX) continue;

        Eng_Global->entities[i] = entity_parser->entries[i];
        flag_set(&Eng_Global->entities[i].entflags, ENTFLAG_ACTIVE, true);
        flag_set(&Eng_Global->entities[i].entflags, ENTFLAG_GROUNDED, false);
        flag_set(&Eng_Global->entities[i].entflags, ENTFLAG_RIGIDBODY, ConstIndexIsDynamicObject(Eng_Global->entities[i].index));
        if (entity_parser->entries[i].entflags & ENTFLAG_CARDCHUNK) {
            Eng_Global->entities[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX; // Generic LOD card
            Eng_Global->entities[i].collider = COLLIDER_TYPE_BOX;
            Eng_Global->entities[i].colliderCenter = (Vector3){ .x = 0.0f, .y = 1.44f, .z = 0.0f };
            Eng_Global->entities[i].colliderSize = (Vector3){ .x = 2.56f, .y = 0.32f, .z = 2.56f };
        }
        
        if (ConstIndexIsButtonSwitch(Eng_Global->entities[i].index)) {
            Eng_Global->entities[i].lockedMessageLingdex = 193; // ButtonSwitch
            Eng_Global->entities[i].tickTime = 1.5;
        }
    }
}

MOD_TO_ENGINE void ModInitAfterLoad(void) {
    for (int i=PLAYER1;i<(Eng_Global->loadedInstances);++i) {        
        if (i == PLAYER1 || i == PLAYER2 || ConstIndexIsDynamicObject(Eng_Global->instances[i].index)) Eng_Global->instances[i].gravity = 1.0f; // Normal gravity
        else Eng_Global->instances[i].gravity = 0.0f;
        
        if (Eng_Global->instances[i].index < MAX_ENTITIES) flag_set(&Eng_Global->instances[i].entflags,ENTFLAG_ANIMATED,Eng_Global->entities[Eng_Global->instances[i].index].entflags & ENTFLAG_ANIMATED);
        if (Eng_Global->instances[i].entflags & ENTFLAG_HAS_CAMERA_VIEW) AddCameraPosition(i);
        
        // Entity Specific Inits
        if (ConstIndexIsGeometry(Eng_Global->instances[i].index)) Eng_Global->instances[i].layer = PhysicsLayer_Geometry;
        if (ConstIndexIsDoor(Eng_Global->instances[i].index)) Eng_Global->instances[i].layer = PhysicsLayer_Door;
        if (ConstIndexIsNPC(Eng_Global->instances[i].index)) Eng_Global->instances[i].layer = PhysicsLayer_NPC;
        if (ConstIndexIsButtonSwitch(Eng_Global->instances[i].index)) ButtonSwitchInit(i);
        if (!StringIsEmpty(Eng_Global->instances[i].targetname) && (Eng_Global->instances[i].ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE) && !(Eng_Global->instances[i].entflags & TARG_IOFLAGS_DISABLD_ONCE_4EVER)) flag_set(&Eng_Global->instances[i].entflags,ENTFLAG_ACTIVE,false);
        if (Eng_Global->instances[i].index == 700) { // logic_branch
            if ((Eng_Global->instances[i].ioflags & TARG_IOFLAGS_START_ON_SECOND) || (Eng_Global->instances[i].ioflags & TARG_IOFLAGS_ON_SECOND)) {
                StringCopyInto_A_From_B(Eng_Global->instances[i].currenttarget,Eng_Global->instances[i].target,TARGET_STRING_LENGTH);
                flag_set(&Eng_Global->instances[i].ioflags,TARG_IOFLAGS_ON_SECOND,false);
            } else { StringCopyInto_A_From_B(Eng_Global->instances[i].currenttarget,Eng_Global->instances[i].target2,TARGET_STRING_LENGTH); flag_set(&Eng_Global->instances[i].ioflags,TARG_IOFLAGS_ON_SECOND,true); }
        }
    }
}
