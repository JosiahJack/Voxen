// update.c - Entity Update logic for all entities thinking
void ButtonSwitchUpdate(void) {
    if ((SELF.delayFinished < Sys_Global.pauseRelativeTime) && SELF.delayFinished != 0) {
        SELF.delayFinished = 0;
//         UseTargets(); TODO
    }

    if (SELF.entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) {
        if (SELF.entflags & ENTFLAG_ACTIVE) {
            if (SELF.tickFinished < Sys_Global.pauseRelativeTime) {
//                 if (alternateOn) SetMaterialToAlternate(); TODO
//                 else SetMaterialToNormal();
                
                SELF.alternateOn = !SELF.alternateOn;
                SELF.tickFinished = Sys_Global.pauseRelativeTime + SELF.tickTime;
            }
        }
    }
}
	
void Update(uint16_t i) {
    selfIdx = i;
    
    
    if (!(SELF.entflags & ENTFLAG_ACTIVE)) return; // ACTIVE BARRIER =========================

    if (ConstIndexIsButtonSwitch(SELF.index)) ButtonSwitchUpdate();
}
