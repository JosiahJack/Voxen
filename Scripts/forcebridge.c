#include "mod.h"

void ForceBridgeInitBeforeLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->tickTime = 0.05f;
    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime + (double)random_range(0.0f,1.0f);
    e->lerping = true;
    if (e->activatedScale.x <= 0.02f) e->activatedScale.x = 2.56f;
    if (e->activatedScale.y <= 0.02f) e->activatedScale.y = 0.08f;
    if (e->activatedScale.z <= 0.02f) e->activatedScale.z = 2.56f;
}

void ForceBridgeInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & ENTFLAG_ACTIVATED)) {
        flag_set(&e->entflags,ENTFLAG_VISIBLE,false);
        e->collider = COLLIDER_TYPE_NONE;
    }
    switch (e->fieldColor) {
        case ForceFieldColor_Red:      e->texIndex = 38; break;
        case ForceFieldColor_Green:    e->texIndex = 40; break;
        case ForceFieldColor_Blue:     e->texIndex = 39; break;
        case ForceFieldColor_Purple:   e->texIndex = 41; break;
        case ForceFieldColor_RedFaint: e->texIndex = 198; break;
    }
}

void ForceBridgeActivate(uint16_t self, bool isSilent) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & ENTFLAG_ACTIVATED) return;
    if (!isSilent) play_wav(sounds[102],1.0f,e->position,true);
    flag_set(&e->entflags,ENTFLAG_VISIBLE,true);
    flag_set(&e->entflags,ENTFLAG_ACTIVATED,true);
    e->lerping = true;
    e->collider = COLLIDER_TYPE_BOX;
    e->scale = (Vector3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x, e->forceFieldDirectionY ? 0.1f : e->activatedScale.y, e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };
}

void ForceBridgeDeactivate(uint16_t self, bool isSilent) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & ENTFLAG_ACTIVATED)) return;
    if (!isSilent) play_wav(sounds[102],1.0f,e->position,true);
    flag_set(&e->entflags,ENTFLAG_ACTIVATED,false);
    e->lerping = true;
}

void ForceBridgeToggle(uint16_t self) {
    if (Eng_Global->instances[self].entflags & ENTFLAG_ACTIVATED) ForceBridgeDeactivate(self,false);
    else ForceBridgeActivate(self,false);
}

void ForceBridgeUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->gamePaused || Eng_Global->menuActive || e->tickFinished >= Eng_Global->pauseRelativeTime) return;
    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime;
    if (e->entflags & ENTFLAG_ACTIVATED) {
        if (!e->lerping) return;
        float sx = e->forceFieldDirectionX ? lerp(e->scale.x,e->activatedScale.x,e->tickTime * 2.0f) : e->scale.x;
        float sy = e->forceFieldDirectionY ? lerp(e->scale.y,e->activatedScale.y,e->tickTime * 2.0f) : e->scale.y;
        float sz = e->forceFieldDirectionZ ? lerp(e->scale.z,e->activatedScale.z,e->tickTime * 2.0f) : e->scale.z;
        e->scale = (Vector3){sx,sy,sz};
        if (vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f) { e->scale = e->activatedScale; e->lerping = false; }
    } else if (e->lerping) {
        float sx = e->forceFieldDirectionX ? lerp(e->scale.x,0.0f,e->tickTime * 2.0f) : e->scale.x;
        float sy = e->forceFieldDirectionY ? lerp(e->scale.y,0.0f,e->tickTime * 2.0f) : e->scale.y;
        float sz = e->forceFieldDirectionZ ? lerp(e->scale.z,0.0f,e->tickTime * 2.0f) : e->scale.z;
        e->scale = (Vector3){sx,sy,sz};
        if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->collider = COLLIDER_TYPE_NONE; e->lerping = false; }
    }
}
