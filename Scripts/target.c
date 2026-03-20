#include "mod.h"

void UseTargets(uint16_t activator, const char* argvalue, const char* targetname) {
    bool succeeded = false;
    if (StringIsEmpty(targetname)) return;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; i++) {
        if (!StringsAreEqual(Eng_Global->instances[i].targetname,targetname)) continue;
        Targetted(activator,i,argvalue);
        succeeded = true;
    }
    if (!succeeded) DualLogWarn("Failed to find a matching targetname for %s\n",targetname);
}

void Targetted(uint16_t activator, uint16_t self, const char* argvalue) {
    Entity* e = &Eng_Global->instances[self];
    Entity* a = &Eng_Global->instances[activator];
    if (argvalue && !StringIsEmpty(argvalue)) StringCopyInto_A_From_B(e->argvalue,argvalue,TARGET_STRING_LENGTH);
    if (e->index == 708) { Eng_Global->gameFinished = true; return; }
    if ((a->ioflags & TARG_IOFLAGS_SEND_EMAIL) && EntityDefIs(self,"info_email")) EmailTargetted(self,activator,argvalue);
    if (a->ioflags & TARG_IOFLAGS_TRIPTRIGGER) {
        if (e->index == 598 || e->index == 600) TriggerTargetted(self,activator);
        else if (e->index == 594) TriggerCounterTargetted(self,activator,argvalue);
    }
    if ((a->ioflags & TARG_IOFLAGS_SWITCHTRIGGER) && ConstIndexIsButtonSwitch(e->index)) ButtonSwitchTargetted(self,activator,argvalue);
    if ((a->ioflags & TARG_IOFLAGS_DOOROPEN) && ConstIndexIsDoor(e->index)) DoorForceOpen(self);
    if ((a->ioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && ConstIndexIsDoor(e->index) && !EntityLocked(e) && (e->requiredAccessCard == AccessCardType_None || e->accessCardUsedByPlayer || (Eng_Global->inventoryPlayer1.accessCardOwned & (1u << e->requiredAccessCard)))) DoorForceOpen(self);
    if ((a->ioflags & TARG_IOFLAGS_DOOR_TOGGLE) && ConstIndexIsDoor(e->index)) DoorActuate(self);
    if ((a->ioflags & TARG_IOFLAGS_DOORCLOSE) && ConstIndexIsDoor(e->index)) DoorForceClose(self);
    if ((a->ioflags & TARG_IOFLAGS_DOORLOCK) && ConstIndexIsDoor(e->index)) DoorLock(self);
    if ((a->ioflags & TARG_IOFLAGS_DOORUNLOCK) && ConstIndexIsDoor(e->index)) DoorUnlock(self);
    if ((a->ioflags & TARG_IOFLAGS_TOG_DORACESOVERIDE) && ConstIndexIsDoor(e->index)) DoorToggleAccessCardOverride(self);
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self,false);
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self,false);
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    if (a->ioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) GravityLiftToggle(self);
    if (a->ioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (a->ioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self,activator,argvalue);
    if (a->ioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) ButtonSwitchToggleLocked(self);
    if (a->ioflags & TARG_IOFLAGS_UNLOCK_SWITCH) EntitySetLocked(e,false);
    if (a->ioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags,ENTFLAG_ACTIVE,true);
    if (a->ioflags & TARG_IOFLAGS_INST_DEACTIVATE) flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
    if (a->ioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags,ENTFLAG_ACTIVE,!(e->entflags & ENTFLAG_ACTIVE));
}
