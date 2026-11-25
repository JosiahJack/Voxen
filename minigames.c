// minigames.c - HUD Minigames rendering and logic.
#include "common.h"
static float mgSlideX,mgSlideY,mgSlideToX,mgSlideToY,mgSlideDirX,mgSlideDirY,mgSlideTick;
static u32 mgTexId, mgFBO;
static i32 mgSlideFrom, mgSlideTo, mgHoverCell = -1;
static u8 mgPx[AM_W * AM_H * 4],mg15num[17];
static bool mgDirty = true, mgSliding = false, mg15solved = false;
static const u16 mgFontCps[] = {32, 33, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 67, 68, 69, 71, 73, 75, 76, 78, 79, 80, 83, 84, 85, 86, 89, 90};
void MgViewRect(bool rh, i32* x, i32* y, i32* w, i32* h) { *x = rh ? 1022 : 24; *y = 524; *w = AM_W; *h = AM_H; }
void MinigameInit() { Font0CacheBuild(mgFontCps, sizeof(mgFontCps) / sizeof(mgFontCps[0])); GenerateAndBindTexture(&mgTexId, GL_RGBA8, AM_W, AM_H, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, NULL); glGenFramebuffers(1, &mgFBO); glBindFramebuffer(GL_FRAMEBUFFER, mgFBO); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mgTexId, 0); glBindFramebuffer(GL_FRAMEBUFFER, uiFBO); }
static void MgFillRect(i32 x, i32 y, i32 w, i32 h, u8 r, u8 g, u8 b) { i32 x0 = vmax(x, 0), y0 = vmax(y, 0), x1 = vmin(x + w, AM_W), y1 = vmin(y + h, AM_H); for (i32 yy = y0; yy < y1; ++yy) { u8* p = mgPx + ((size_t)yy * AM_W + (u32)x0) * 4; for (i32 xx = x0; xx < x1; ++xx, p += 4) { p[0] = r; p[1] = g; p[2] = b; p[3] = 255; } } }
static void MgBlitImage(u16 tex, i32 dx, i32 dy, i32 dw, i32 dh) {
    i32 sw = 0, sh = 0;
    const u8* src = CpuResidentPixels(tex, &sw, &sh);
    if (!src || sw < 1 || sh < 1 || dw < 1 || dh < 1) return;
    for (i32 y = 0; y < dh; ++y) {
        i32 py = dy + y;
        if (py < 0 || py >= AM_H) continue;
        i32 sy = (y * sh) / dh;
        const u8* srow = src + (size_t)sy * sw * 4;
        u8* drow = mgPx + ((size_t)py * AM_W) * 4;
        for (i32 x = 0; x < dw; ++x) {
            i32 px = dx + x;
            if (px < 0 || px >= AM_W) continue;
            i32 sx = (x * sw) / dw;
            const u8* s = srow + (size_t)sx * 4;
            u8 a = s[3];
            if (!a) continue;
            u8* d = drow + (size_t)px * 4;
            if (a == 255) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = 255; }
            else { u32 ia = 255u - a; d[0] = (u8)((s[0] * a + d[0] * ia) / 255u); d[1] = (u8)((s[1] * a + d[1] * ia) / 255u); d[2] = (u8)((s[2] * a + d[2] * ia) / 255u); d[3] = 255; }
        }
    }
}
static void MgDrawText(i32 x, i32 baselineY, u8 r, u8 g, u8 b, const char* s) {
    float penX = (float)x;
    while (*s) {
        u32 cp = (u8)*s++;
        const u8* bmp = NULL;
        i32 gw = 0, gh = 0;
        float xoff = 0.0f, yoff = 0.0f, xadv = 0.0f;
        if (Font0CacheGet(cp, &bmp, &gw, &gh, &xoff, &yoff, &xadv) && bmp && gw > 0 && gh > 0) {
            i32 ox = (i32)(penX + xoff + 0.5f), oy = (i32)((float)baselineY + yoff + 0.5f);
            for (i32 yy = 0; yy < gh; ++yy) {
                i32 py = oy + yy;
                if (py < 0 || py >= AM_H) continue;
                const u8* srow = bmp + (size_t)yy * gw;
                u8* drow = mgPx + ((size_t)py * AM_W) * 4;
                for (i32 xx = 0; xx < gw; ++xx) {
                    i32 px = ox + xx;
                    if (px < 0 || px >= AM_W) continue;
                    u8 cov = srow[xx];
                    if (!cov) continue;
                    u8* d = drow + (size_t)px * 4;
                    if (cov == 255) { d[0] = r; d[1] = g; d[2] = b; d[3] = 255; }
                    else { u32 ia = 255u - cov; d[0] = (u8)((r * cov + d[0] * ia) / 255u); d[1] = (u8)((g * cov + d[1] * ia) / 255u); d[2] = (u8)((b * cov + d[2] * ia) / 255u); d[3] = 255; }
                }
            }
        }
        penX += xadv;
    }
}
static void MgDrawTextC(i32 baselineY, u8 r, u8 g, u8 b, const char* s) { float w = MeasureLineAdvance(s, FONT_NORMAL); MgDrawText((i32)((AM_W - w) * 0.5f + 0.5f), baselineY, r, g, b, s); }
static void Mg15CellXY(i32 cell, i32* x, i32* y) { *x = 96 + ((cell - 1) % 4) * 32; *y = 36 + ((cell - 1) / 4) * 32; }
static void Mg15DrawTile(i32 x, i32 y, i32 num, bool hover) {
    if (num == 16) { MgFillRect(x, y, 28, 28, 128, 102, 102); return; }
    if (hover) { MgFillRect(x, y, 28, 28, 255, 153, 128); MgBlitImage(908, x, y, 28, 28); }
    else { MgFillRect(x, y, 28, 28, 204, 191, 184); MgBlitImage(1087, x, y, 28, 28); }
    char nb[4];
    sFormat(nb, sizeof(nb), "%u", (u32)num);
    const u8* bmp = NULL;
    i32 gw = 0, gh = 0;
    float xoff = 0.0f, yoff = 0.0f, xadv = 0.0f, w = MeasureLineAdvance(nb,FONT_NORMAL);
    i32 tx = (i32)(x + (28.0f - w) * 0.5f + 0.5f);
    i32 baseline = y + 21;
    if (Font0CacheGet((u32)nb[0], &bmp, &gw, &gh, &xoff, &yoff, &xadv) && bmp) baseline = (i32)(y + (28 - gh) * 0.5f - yoff + 0.5f);
    MgDrawText(tx, baseline, 0, 0, 0, nb);
}
static void Mg15Render() {
    MgFillRect(0, 0, AM_W, AM_H, 6, 25, 11);
    if (mg15solved) { MgDrawTextC(24, 255, 255, 0, "PUZZLE SOLVED!"); MgDrawTextC(190, 255, 255, 255, "CLICK TO PLAY AGAIN"); } else { MgDrawTextC(24, 255, 255, 255, "15 PUZZLE"); }
    for (i32 cell = 1; cell <= 16; ++cell) { i32 cx = 0, cy = 0; Mg15CellXY(cell, &cx, &cy); if (mgSliding && (cell == mgSlideFrom || cell == mgSlideTo)) { MgFillRect(cx, cy, 28, 28, 128, 102, 102); continue; } Mg15DrawTile(cx, cy, mg15num[cell], !mgSliding && !mg15solved && cell == mgHoverCell); }
    if (mgSliding) Mg15DrawTile((i32)(mgSlideX + 0.5f), (i32)(mgSlideY + 0.5f), mg15num[mgSlideTo], false); 
    mgDirty = false;
}
static i32 Mg15EmptyNeighbor(i32 cell) { i32 lt = cell - 1, rt = cell + 1, up = cell - 4, dn = cell + 4; i32 lti = 0, rti = 0, upi = 0, dni = 0; if (lt > 0 && lt < 17) lti = mg15num[lt]; if (rt > 0 && rt < 17) rti = mg15num[rt]; if (up > 0 && up < 17) upi = mg15num[up]; if (dn > 0 && dn < 17) dni = mg15num[dn]; if (lti == 16) return lt; if (rti == 16) return rt; if (upi == 16) return up; if (dni == 16) return dn; return 0; }
static void Mg15Slide(i32 from, i32 to, bool resetting) {
    if (from < 1 || to < 1 || from > 16 || to > 16) return;
    i32 fromNum = mg15num[from], toNum = mg15num[to];
    mg15num[to] = fromNum;
    mg15num[from] = toNum;
    if (resetting) return;
    i32 fx = 0, fy = 0, tx = 0, ty = 0;
    Mg15CellXY(from, &fx, &fy);
    Mg15CellXY(to, &tx, &ty);
    mgSlideFrom = from;
    mgSlideTo = to;
    mgSlideX = (float)fx;
    mgSlideY = (float)fy;
    mgSlideToX = (float)tx;
    mgSlideToY = (float)ty;
    mgSlideDirX = (tx > fx) ? 1.0f : ((tx < fx) ? -1.0f : 0.0f);
    mgSlideDirY = (ty > fy) ? 1.0f : ((ty < fy) ? -1.0f : 0.0f);
    mgSliding = true;
    mgSlideTick = World.pauseRelativeTime + 0.1f;
    mgDirty = true;
}
static void Mg15Evaluate() { for (i32 i = 1; i <= 15; ++i) if (mg15num[i] != i) return; mg15solved = true; mgDirty = true; play_wav("buttons/keycard_success",AppliedFXVol(1.0f),(V3){0},false); }
void Mg15Reset() {
    for (i32 i = 1; i <= 16; ++i) mg15num[i] = (u8)i;
    mgSliding = false; mg15solved = false; mgHoverCell = -1; i32 shuffleIter = World.diffPuz * 5;
    while (shuffleIter > 0) { i32 randIter = 32; while (randIter > 0) { i32 r = (i32)random_range_u32(1, 16); i32 emp = Mg15EmptyNeighbor(r); if (emp) { Mg15Slide(r, emp, true); break; } randIter--; } shuffleIter--; }
    mgDirty = true;
}
static i32 Mg15CellAt(i32 lx, i32 ly) { if (lx < 96 || ly < 36 || lx >= 224 || ly >= 164) return 0; return ((ly - 36) / 32) * 4 + ((lx - 96) / 32) + 1; }
void Mg15Click(bool rh, i32 x, i32 y) { i32 vx = 0, vy = 0, vw = 0, vh = 0; MgViewRect(rh, &vx, &vy, &vw, &vh); i32 lx = x - vx, ly = y - vy; if (lx < 0 || ly < 0 || lx >= vw || ly >= vh) return; if (mg15solved) { Mg15Reset(); return; } if (mgSliding) return; i32 cell = Mg15CellAt(lx, ly); if (!cell) return; i32 emp = Mg15EmptyNeighbor(cell); if (emp) Mg15Slide(cell, emp, false); Mg15Evaluate(); }
void MinigameTick() {
    if (World.Sys_UI.mg_current != 1 || !mgSliding || World.pauseRelativeTime < mgSlideTick) return;
    float tdiff = World.pauseRelativeTime - mgSlideTick; float shift = (float)(tdiff / 0.04f) * 12.0f; mgSlideTick = World.pauseRelativeTime + 0.04f; mgSlideX += mgSlideDirX * shift; mgSlideY += mgSlideDirY * shift; bool done = false;
    if (mgSlideDirX > 0.0f && mgSlideX >= mgSlideToX) done = true;
    if (mgSlideDirX < 0.0f && mgSlideX <= mgSlideToX) done = true;
    if (mgSlideDirY > 0.0f && mgSlideY >= mgSlideToY) done = true;
    if (mgSlideDirY < 0.0f && mgSlideY <= mgSlideToY) done = true;
    if (done) { mgSlideX = mgSlideToX; mgSlideY = mgSlideToY; mgSliding = false; }
    mgDirty = true;
}
static i32 Mg15HoverCell(bool rh) { i32 vx = 0, vy = 0, vw = 0, vh = 0; MgViewRect(rh, &vx, &vy, &vw, &vh); i32 lx = World.cursorPos_x - vx, ly = World.cursorPos_y - vy; if (lx < 0 || ly < 0 || lx >= vw || ly >= vh) return -1; i32 cell = Mg15CellAt(lx, ly); if (!cell || mg15num[cell] == 16) return -1; return cell; }
void MgBlitToUI(bool isRH) {
    if (World.Sys_UI.mg_current != 1) return; i32 hov = (mgSliding || mg15solved) ? -1 : Mg15HoverCell(isRH); if (hov != mgHoverCell) { mgHoverCell = hov; mgDirty = true; } if (mgDirty) { Mg15Render(); glBindTexture(GL_TEXTURE_2D, mgTexId); glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, AM_W, AM_H, GL_RGBA, GL_UNSIGNED_BYTE, mgPx); }
    i32 vx = 0, vy = 0, vw = 0, vh = 0; MgViewRect(isRH, &vx, &vy, &vw, &vh); glBindFramebuffer(GL_READ_FRAMEBUFFER, mgFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, uiFBO); glBlitFramebuffer(0, AM_H, AM_W, 0, vx, UI_H - vy - vh, vx + vw, UI_H - vy, GL_COLOR_BUFFER_BIT, GL_LINEAR); glBindFramebuffer(GL_READ_FRAMEBUFFER, uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, uiFBO);
}
