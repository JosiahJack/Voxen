// helpers.c - Helper Functions for various things
#include <sys/stat.h>
#include <time.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "External/stb_image_write.h"
#include "entity.h"
#include "voxen.h"
#ifdef DEBUG_RAM_OUTPUT
#include <malloc.h>
#endif
#include "event.h"

double get_time(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        DualLogError("clock_gettime failed\n");
        return 0.0;
    }

    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Full time in seconds
}

// Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void DebugRAM(const char *context) {
#ifdef DEBUG_RAM_OUTPUT
    struct mallinfo2 info = mallinfo2();
    size_t uss_bytes = 0;
    FILE *fp = fopen("/proc/self/smaps_rollup", "r");
    if (fp) {
        char line[256];
        size_t val;
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "Private_Clean: %zu kB", &val) == 1)      uss_bytes += val * 1024;
            else if (sscanf(line, "Private_Dirty: %zu kB", &val) == 1) uss_bytes += val * 1024;
        }
        fclose(fp);
    } else DualLogError("Failed to open /proc/self/smaps_rollup\n");

    DualLog("Memory at %s: Heap usage %zu bytes (%zu KB | %.2f MB), USS %zu bytes (%zu KB | %.2f MB)\n",
            context, info.uordblks, info.uordblks / 1024, info.uordblks / 1024.0 / 1024.0,
            uss_bytes, uss_bytes / 1024, uss_bytes / 1024.0 / 1024.0);
#endif
}
#pragma GCC diagnostic pop

void print_bytes_no_newline(int32_t count) { DualLog("%d bytes | %f kb | %f Mb",count,(double)count / 1000.0,(double)count / 1000000.0); }

// MD5 (128-bit / 16 bytes) – tiny self-contained implementation
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define F(x,y,z) (((x) & (y)) | (~(x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & ~(z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | ~(z)))
// Very small, public-domain MD5 – copy-paste from https://github.com/kerukuro/digestpp/blob/master/algorithm/detail/constants/md5_constants.hpp
static const uint32_t md5Constants[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,	0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340,	0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,	0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa,	0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
	
void md5(const uint8_t *data, size_t len, uint8_t out[16]) {

    uint32_t h[4] = {0x67452301,0xefcdab89,0x98badcfe,0x10325476};
    uint32_t a,b,c,d,f,g;
    uint32_t M[16];
    size_t i, n = len;
    while (n >= 64) {
        memcpy(M, data, 64);
        a = h[0]; b = h[1]; c = h[2]; d = h[3];
        for (i = 0; i < 64; ++i) {
            if (i < 16) { f = F(b,c,d); g = i; }
            else if (i < 32) { f = G(b,c,d); g = (5*i+1)%16; }
            else if (i < 48) { f = H(b,c,d); g = (3*i+5)%16; }
            else { f = I(b,c,d); g = (7*i)%16; }
            uint32_t temp = d;
            d = c; c = b;
            b += ROTL(a + f + md5Constants[i] + M[g], (i%4)*7 + 7);
            a = temp;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        data += 64; n -= 64;
    }

    /* pad the last block */
    uint8_t pad[64] = {0};
    memcpy(pad, data, n);
    pad[n] = 0x80;
    if (n >= 56) { /* need a second block */
        /* (process first block) */
        memcpy(M, pad, 64);
        a = h[0]; b = h[1]; c = h[2]; d = h[3];
        for (i = 0; i < 64; ++i) { /* same loop as above */ }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        memset(pad, 0, 56);
    }

    uint64_t bits = len * 8;
    for (i = 0; i < 8; ++i) pad[56+i] = (bits >> (i*8)) & 0xFF;
    memcpy(M, pad, 64);
    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    for (i = 0; i < 64; ++i) { /* same loop */ }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    for (i = 0; i < 4; ++i) {
        out[i*4+0] = h[i] >> 0;
        out[i*4+1] = h[i] >> 8;
        out[i*4+2] = h[i] >> 16;
        out[i*4+3] = h[i] >> 24;
    }
}

bool ConstIndexInBounds(int constdex) {
	return (constdex >= 0 && constdex <= 760);
}

bool ConstIndexIsGeometry(int constdex) {
	return (constdex >= 0 && constdex <= 306 && constdex != 112 && constdex != 279) || constdex == 760;
}

bool ConstIndexIsDoor(int constdex) {
	return (constdex >= 496 && constdex < 515);
}

bool ConstIndexIsLightStaticSaveable(int constdex) {
	return constdex == 748;
}

bool ConstIndexIsGenericTransform(int constdex) {
	return constdex == 749;
}

bool ConstIndexIsDynamicObject(uint16_t constIndex) {
    return     (constIndex >= 307 && constIndex <= 404)
            ||  constIndex == 417
            || (constIndex >= 419 && constIndex <= 428)
            || (constIndex >= 430 && constIndex <= 437)
            || (constIndex >= 440 && constIndex <= 442)
            || (constIndex >= 458 && constIndex <= 463)
            || (constIndex >= 465 && constIndex <= 476);
}

bool ConstIndexIsStaticObjectSaveable(int constdex) {
	return (   constdex == 112 || constdex == 279
            || (constdex >= 448 && constdex < 458)
			|| constdex == 480 || constdex == 516
			|| (constdex >= 518 && constdex <= 526)
			|| constdex == 530 || constdex == 531 || constdex == 546
			|| constdex == 555 || constdex == 594 || constdex == 596
			|| constdex == 598
			|| (constdex >= 600 && constdex < 603)
			|| (constdex >= 604 && constdex < 616)
			|| (constdex >= 688 && constdex < 693)
			|| constdex == 694 || constdex == 695
			|| (constdex >= 699 && constdex < 704)
			|| (constdex >= 741 && constdex < 746));
}

bool ConstIndexIsStaticObjectImmutable(int constdex) {
	return ((constdex >= 527 && constdex < 530)
			|| (constdex >= 532 && constdex < 546)
			|| (constdex >= 547 && constdex < 553)
			|| constdex == 554
			|| (constdex >= 556 && constdex < 594)
			|| constdex == 595 || constdex == 597 || constdex == 599
			|| constdex == 601 || constdex == 603
			|| (constdex >= 616 && constdex < 688)
			|| constdex == 693 || constdex == 696 || constdex == 697
			|| constdex == 698
			|| (constdex >= 704 && constdex < 717)
			|| constdex == 720
			|| (constdex >= 733 && constdex < 736)
			|| (constdex >= 737 && constdex < 739)
			|| constdex == 746
			|| constdex == 747
			|| (constdex >= 750 && constdex <= 759 && constdex != 755));
}

bool ConstIndexIsNPC(int constdex) {
	return (constdex >= 419 && constdex < 448);
}

bool ConstIndexIsHardware(int constdex) {
	return (constdex >= 328) && (constdex <= 339);
}

bool ConstIndexIsAmbient(int constdex) {
    return (constdex >= 621 && constdex <= 655);
}

void Screenshot(void) {
    struct stat st = {0};
    if (stat("Screenshots", &st) == -1) { // Check and make ./Screenshots/ folder if it doesn't exist yet.
        if (mkdir("Screenshots", 0755) != 0) { DualLogError("Failed to create Screenshots folder\n"); return; }
    }
    
    unsigned char* pixels = malloc(voxen_Settings.ScreenWidth * voxen_Settings.ScreenHeight * 4 * sizeof(char));
    glReadPixels(0, 0, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    char timestamp[32];
    char filename[96];
    time_t now = time(NULL);
    struct tm *utc_time = localtime(&now);    
    if (utc_time) strftime(timestamp, sizeof(timestamp), "%d%b%Y_%H_%M_%S", utc_time);
    snprintf(filename, sizeof(filename), "Screenshots/%s_%s_x%.2f_y%.2f_z%.2f__time_%.1f.bmp", timestamp, VERSION_STRING, (double)instances[PLAYER1].position.x, (double)instances[PLAYER1].position.y, (double)instances[PLAYER1].position.z, get_time());
    if (!stbi_write_bmp(filename, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight, 4, pixels)) DualLogError("Failed to save screenshot\n");
    else DualLog("Saved screenshot %s\n", filename);

    free(pixels);
}

GLuint SetupSSBO(GLuint id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage) {
    if (!voxen_globalContext.levelCurrentlyLoading && voxen_Diagnostics.globalFrameNum > 1) DualLogError("Trying to delete and generate a new SSBO %u outside of a level load!\n", bindingIndex);
    if (id != 0) glDeleteBuffers(1, &id); // Clear last level's SSBO.
    GLuint new_id = 0;
    glGenBuffers(1, &new_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, new_id);
    return new_id;
}

__attribute__((pure)) bool CursorVisible(void) {
    return (voxen_globalContext.inventoryMode || voxen_globalContext.menuActive || voxen_globalContext.gamePaused);
}

uint32_t random_range_rng = 0x12345678u; // Global seed
uint32_t xs32(uint32_t *s) {
    uint32_t x=*s; x^=x<<13; x^=x>>17; x^=x<<5;
    return *s = x ? x : 0xdeadbeefu;
}

uint8_t random_range_u8(uint8_t a, uint8_t b) {
    uint8_t n = (uint8_t)(b - a + 1u);
    if (!n) return a; // handle wrap if a>b (undefined otherwise)
    uint8_t v, t = (uint8_t)(256u % n);
    do v = (uint8_t)xs32(&random_range_rng); while (v >= 256u - t);
    return (uint8_t)(a + (v % n));
}

int data_parser_isspace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; }

// Using "relative time" = pauseRelativeTime and "finished" = some
// script's timer float value, e.g. attackFinished, in the notes below...
//
// If the relative time is 123 when we save and finished is 156, then when
// we load and relative time is 160 and we set finished to 156, it will
// immediately finish.
//
// Need to take finished - relative time = 33 and save that.  Then on load
// take this differential value and do relative time + differential = 160 +
// 33 = 193, then the same condition is restored such that the finished 
// timer still has 33 before it is up.
//
// In the scenario where finished is less than relative time, if finished =
// 103 and relative time is still 123, then when we load and relative time
// is 160, all is still fine, timer is already up.
//
// Still can't hurt to do finished - relative time = -20. Then when we load
// the value do relative time 160 + -20 = 140.  This is arguably best just
// in case there is a whackado one-off instance of comparing (time - 
// finished) somewhere instead of (finished < time) which is my usual Quake
// derived timer pattern.
float LoadRelativeTimeDifferential(char* trimmed_value, char* initialLine, uint32_t lineNum) {
    return parse_float(trimmed_value, initialLine, lineNum) + (float)voxen_globalContext.pauseRelativeTime; // Add current instance's relative time to get same timer in context of current time.  See above notes.
}
