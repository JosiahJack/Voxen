#include "common.h"
static u8 mgPx[AM_W * AM_H * 4];
static u32 mgTexId, mgFBO;
static bool mgDirty = true;
static u8 mg15num[17];
static bool mgSliding = false, mg15solved = false;
static int mgSlideFrom, mgSlideTo, mgHoverCell = -1;
static float mgSlideX, mgSlideY, mgSlideToX, mgSlideToY, mgSlideDirX, mgSlideDirY;
static double mgSlideTick = 0.0;
static const u16 mgFontCps[] = {32, 33, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 67, 68, 69, 71, 73, 75, 76, 78, 79, 80, 83, 84, 85, 86, 89, 90};
void MgViewRect(bool rh, int* x, int* y, int* w, int* h) { *x = rh ? 1022 : 24; *y = 524; *w = AM_W; *h = AM_H; }
void MinigameInit(void) {
    Font0CacheBuild(mgFontCps, sizeof(mgFontCps) / sizeof(mgFontCps[0]));
    GenerateAndBindTexture(&mgTexId, GL_RGBA8, AM_W, AM_H, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, NULL);
    glGenFramebuffers(1, &mgFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mgFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mgTexId, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, uiFBO);
}
static void MgFillRect(int x, int y, int w, int h, u8 r, u8 g, u8 b) {
    int x0 = vmax(x, 0), y0 = vmax(y, 0), x1 = vmin(x + w, AM_W), y1 = vmin(y + h, AM_H);
    for (int yy = y0; yy < y1; ++yy) {
        u8* p = mgPx + ((size_t)yy * AM_W + (u32)x0) * 4;
        for (int xx = x0; xx < x1; ++xx, p += 4) { p[0] = r; p[1] = g; p[2] = b; p[3] = 255; }
    }
}
static void MgBlitImage(u16 tex, int dx, int dy, int dw, int dh) {
    int sw = 0, sh = 0;
    const u8* src = CpuResidentPixels(tex, &sw, &sh);
    if (!src || sw < 1 || sh < 1 || dw < 1 || dh < 1) return;
    for (int y = 0; y < dh; ++y) {
        int py = dy + y;
        if (py < 0 || py >= AM_H) continue;
        int sy = (y * sh) / dh;
        const u8* srow = src + (size_t)sy * sw * 4;
        u8* drow = mgPx + ((size_t)py * AM_W) * 4;
        for (int x = 0; x < dw; ++x) {
            int px = dx + x;
            if (px < 0 || px >= AM_W) continue;
            int sx = (x * sw) / dw;
            const u8* s = srow + (size_t)sx * 4;
            u8 a = s[3];
            if (!a) continue;
            u8* d = drow + (size_t)px * 4;
            if (a == 255) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = 255; }
            else { u32 ia = 255u - a; d[0] = (u8)((s[0] * a + d[0] * ia) / 255u); d[1] = (u8)((s[1] * a + d[1] * ia) / 255u); d[2] = (u8)((s[2] * a + d[2] * ia) / 255u); d[3] = 255; }
        }
    }
}
static void MgDrawText(int x, int baselineY, u8 r, u8 g, u8 b, const char* s) {
    float penX = (float)x;
    while (*s) {
        u32 cp = (u8)*s++;
        const u8* bmp = NULL;
        int gw = 0, gh = 0;
        float xoff = 0.0f, yoff = 0.0f, xadv = 0.0f;
        if (Font0CacheGet(cp, &bmp, &gw, &gh, &xoff, &yoff, &xadv) && bmp && gw > 0 && gh > 0) {
            int ox = (int)(penX + xoff + 0.5f), oy = (int)((float)baselineY + yoff + 0.5f);
            for (int yy = 0; yy < gh; ++yy) {
                int py = oy + yy;
                if (py < 0 || py >= AM_H) continue;
                const u8* srow = bmp + (size_t)yy * gw;
                u8* drow = mgPx + ((size_t)py * AM_W) * 4;
                for (int xx = 0; xx < gw; ++xx) {
                    int px = ox + xx;
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
static void MgDrawTextC(int baselineY, u8 r, u8 g, u8 b, const char* s) {
    float w = MeasureLineAdvance(s, FONT_NORMAL);
    MgDrawText((int)((AM_W - w) * 0.5f + 0.5f), baselineY, r, g, b, s);
}
static void Mg15CellXY(int cell, int* x, int* y) {
    *x = 96 + ((cell - 1) % 4) * 32;
    *y = 36 + ((cell - 1) / 4) * 32;
}
static void Mg15DrawTile(int x, int y, int num, bool hover) {
    if (num == 16) { MgFillRect(x, y, 28, 28, 128, 102, 102); return; }
    if (hover) { MgFillRect(x, y, 28, 28, 255, 153, 128); MgBlitImage(908, x, y, 28, 28); }
    else { MgFillRect(x, y, 28, 28, 204, 191, 184); MgBlitImage(1087, x, y, 28, 28); }
    char nb[4];
    sFormat(nb, sizeof(nb), "%u", (u32)num);
    const u8* bmp = NULL;
    int gw = 0, gh = 0;
    float xoff = 0.0f, yoff = 0.0f, xadv = 0.0f;
    float w = MeasureLineAdvance(nb, FONT_NORMAL);
    int tx = (int)(x + (28.0f - w) * 0.5f + 0.5f);
    int baseline = y + 21;
    if (Font0CacheGet((u32)nb[0], &bmp, &gw, &gh, &xoff, &yoff, &xadv) && bmp) baseline = (int)(y + (28 - gh) * 0.5f - yoff + 0.5f);
    MgDrawText(tx, baseline, 0, 0, 0, nb);
}
static void Mg15Render(void) {
    MgFillRect(0, 0, AM_W, AM_H, 6, 25, 11);
    if (mg15solved) {
        MgDrawTextC(24, 255, 255, 0, "PUZZLE SOLVED!");
        MgDrawTextC(190, 255, 255, 255, "CLICK TO PLAY AGAIN");
    } else {
        MgDrawTextC(24, 255, 255, 255, "15 PUZZLE");
    }
    for (int cell = 1; cell <= 16; ++cell) {
        int cx = 0, cy = 0;
        Mg15CellXY(cell, &cx, &cy);
        if (mgSliding && (cell == mgSlideFrom || cell == mgSlideTo)) { MgFillRect(cx, cy, 28, 28, 128, 102, 102); continue; }
        Mg15DrawTile(cx, cy, mg15num[cell], !mgSliding && !mg15solved && cell == mgHoverCell);
    }
    if (mgSliding) Mg15DrawTile((int)(mgSlideX + 0.5f), (int)(mgSlideY + 0.5f), mg15num[mgSlideTo], false);
    mgDirty = false;
}
static int Mg15EmptyNeighbor(int cell) {
    int lt = cell - 1, rt = cell + 1, up = cell - 4, dn = cell + 4;
    int lti = 0, rti = 0, upi = 0, dni = 0;
    if (lt > 0 && lt < 17) lti = mg15num[lt];
    if (rt > 0 && rt < 17) rti = mg15num[rt];
    if (up > 0 && up < 17) upi = mg15num[up];
    if (dn > 0 && dn < 17) dni = mg15num[dn];
    if (lti == 16) return lt;
    if (rti == 16) return rt;
    if (upi == 16) return up;
    if (dni == 16) return dn;
    return 0;
}
static void Mg15Slide(int from, int to, bool resetting) {
    if (from < 1 || to < 1 || from > 16 || to > 16) return;
    int fromNum = mg15num[from], toNum = mg15num[to];
    mg15num[to] = fromNum;
    mg15num[from] = toNum;
    if (resetting) return;
    int fx = 0, fy = 0, tx = 0, ty = 0;
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
    mgSlideTick = World.pauseRelativeTime + 0.1;
    mgDirty = true;
}
static void Mg15Evaluate(void) {
    for (int i = 1; i <= 15; ++i) if (mg15num[i] != i) return;
    mg15solved = true;
    mgDirty = true;
    play_wav(sounds[46], AppliedFXVol(1.0f), (V3){0}, false);
}
void Mg15Reset(void) {
    for (int i = 1; i <= 16; ++i) mg15num[i] = (u8)i;
    mgSliding = false;
    mg15solved = false;
    mgHoverCell = -1;
    int shuffleIter = World.diffPuz * 5;
    while (shuffleIter > 0) {
        int randIter = 32;
        while (randIter > 0) {
            int r = (int)random_range_u32(1, 16);
            int emp = Mg15EmptyNeighbor(r);
            if (emp) { Mg15Slide(r, emp, true); break; }
            randIter--;
        }
        shuffleIter--;
    }
    mgDirty = true;
}
static int Mg15CellAt(int lx, int ly) {
    if (lx < 96 || ly < 36 || lx >= 224 || ly >= 164) return 0;
    return ((ly - 36) / 32) * 4 + ((lx - 96) / 32) + 1;
}
void Mg15Click(bool rh, int x, int y) {
    int vx = 0, vy = 0, vw = 0, vh = 0;
    MgViewRect(rh, &vx, &vy, &vw, &vh);
    int lx = x - vx, ly = y - vy;
    if (lx < 0 || ly < 0 || lx >= vw || ly >= vh) return;
    if (mg15solved) { Mg15Reset(); return; }
    if (mgSliding) return;
    int cell = Mg15CellAt(lx, ly);
    if (!cell) return;
    int emp = Mg15EmptyNeighbor(cell);
    if (emp) Mg15Slide(cell, emp, false);
    Mg15Evaluate();
}
void MinigameTick(void) {
    if (World.Sys_UI.mg_current != 1 || !mgSliding) return;
    double now = World.pauseRelativeTime;
    if (now < mgSlideTick) return;
    double tdiff = now - mgSlideTick;
    float shift = (float)(tdiff / 0.04) * 12.0f;
    mgSlideTick = now + 0.04;
    mgSlideX += mgSlideDirX * shift;
    mgSlideY += mgSlideDirY * shift;
    bool done = false;
    if (mgSlideDirX > 0.0f && mgSlideX >= mgSlideToX) done = true;
    if (mgSlideDirX < 0.0f && mgSlideX <= mgSlideToX) done = true;
    if (mgSlideDirY > 0.0f && mgSlideY >= mgSlideToY) done = true;
    if (mgSlideDirY < 0.0f && mgSlideY <= mgSlideToY) done = true;
    if (done) { mgSlideX = mgSlideToX; mgSlideY = mgSlideToY; mgSliding = false; }
    mgDirty = true;
}
static int Mg15HoverCell(bool rh) {
    int vx = 0, vy = 0, vw = 0, vh = 0;
    MgViewRect(rh, &vx, &vy, &vw, &vh);
    int lx = World.cursorPos_x - vx, ly = World.cursorPos_y - vy;
    if (lx < 0 || ly < 0 || lx >= vw || ly >= vh) return -1;
    int cell = Mg15CellAt(lx, ly);
    if (!cell || mg15num[cell] == 16) return -1;
    return cell;
}
void MgBlitToUI(bool isRH) {
    if (World.Sys_UI.mg_current != 1) return;
    int hov = (mgSliding || mg15solved) ? -1 : Mg15HoverCell(isRH);
    if (hov != mgHoverCell) { mgHoverCell = hov; mgDirty = true; }
    if (mgDirty) {
        Mg15Render();
        glBindTexture(GL_TEXTURE_2D, mgTexId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, AM_W, AM_H, GL_RGBA, GL_UNSIGNED_BYTE, mgPx);
    }
    int vx = 0, vy = 0, vw = 0, vh = 0;
    MgViewRect(isRH, &vx, &vy, &vw, &vh);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mgFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, uiFBO);
    glBlitFramebuffer(0, AM_H, AM_W, 0, vx, UI_H - vy - vh, vx + vw, UI_H - vy, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, uiFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, uiFBO);
}
