#include "mod.h"

void TextureChangerInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->currentTexture) return;
    e->texIndex = e->altTexIndex;
    if (e->altGlowIndex < MAX_VALID_TEXTURE) e->glowIndex = e->altGlowIndex;
}

void TextureChangerToggle(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->currentTexture) {
        e->texIndex = Eng_Global->entities[e->index].texIndex;
        e->glowIndex = Eng_Global->entities[e->index].glowIndex;
    } else {
        e->texIndex = e->altTexIndex;
        if (e->altGlowIndex < MAX_VALID_TEXTURE) e->glowIndex = e->altGlowIndex;
    }
    e->currentTexture = !e->currentTexture;
}
