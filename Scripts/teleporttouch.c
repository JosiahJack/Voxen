#include "mod.h"

static uint16_t TeleportTouch_allTeleportTouches[8];
static bool TeleportTouch_initialized;

void TeleportTouchInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!TeleportTouch_initialized) { for (uint8_t i = 0; i < 8; i++) TeleportTouch_allTeleportTouches[i] = UINT16_MAX; TeleportTouch_initialized = true; }
    if (e->teleportID >= 8) { DeleteInstance(self); return; }
    TeleportTouch_allTeleportTouches[e->teleportID] = self;
}

void TeleportTouchOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (!e->touchEnabled || other != PLAYER1) return;
    if (player->health <= 0.0f || e->justUsed >= Eng_Global->pauseRelativeTime) return;
    uint16_t dest = e->targetDestinationID < 8 ? TeleportTouch_allTeleportTouches[e->targetDestinationID] : UINT16_MAX;
    if (dest == UINT16_MAX) return;
    player->position = Eng_Global->instances[dest].position;
    Eng_Global->instances[dest].justUsed = Eng_Global->pauseRelativeTime + 1.0;
    play_wav(sounds[106],1.0f,Eng_Global->instances[dest].position,false);
}
