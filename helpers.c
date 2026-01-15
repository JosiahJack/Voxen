// helpers.c - Helper Functions for various things
#include <time.h>
#include <malloc.h>
#include "os.h"
#include "voxen.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "External/stb_image_write.h"

double get_time(void) {
    #ifdef WINDOWS
        static LARGE_INTEGER frequency;
        static BOOL initialized = FALSE;
        if (!initialized) { QueryPerformanceFrequency(&frequency); initialized = TRUE; }
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / frequency.QuadPart;
    #else
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) { DualLogError("clock_gettime failed\n"); return 0.0; }
        return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Full time in seconds
    #endif
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

bool ConstIndexInBounds(int constdex) { return (constdex >= 0 && constdex <= 760); }
bool ConstIndexIsGeometry(int constdex) { return (constdex >= 0 && constdex <= 306 && constdex != 112 && constdex != 279) || constdex == 760; }
bool ConstIndexIsDoor(int constdex) { return (constdex >= 496 && constdex < 515); }
bool ConstIndexIsLightStaticSaveable(int constdex) { return constdex == 748; }
bool ConstIndexIsGenericTransform(int constdex) { return constdex == 749; }
bool ConstIndexIsNPC(int constdex) { return (constdex >= 419 && constdex < 448); }
bool ConstIndexIsHardware(int constdex) { return (constdex >= 328) && (constdex <= 339); }
bool ConstIndexIsAmbient(int constdex) { return (constdex >= 621 && constdex <= 655); }
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

void Screenshot(void) {
    OS_MakeFolder("Screenshots");
    unsigned char* pixels = OS_AllocateRAM(NULL, Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);//malloc(Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char));
    glReadPixels(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    char timestamp[32];
    char filename[96];
    time_t now = time(NULL);
    struct tm *utc_time = localtime(&now);    
    if (utc_time) strftime(timestamp, sizeof(timestamp), "%d%b%Y_%H_%M_%S", utc_time);
    snprintf(filename, sizeof(filename), "Screenshots/%s_x%.2f_y%.2f_z%.2f__time_%.1f.bmp", timestamp, (double)instances[PLAYER1].position.x, (double)instances[PLAYER1].position.y, (double)instances[PLAYER1].position.z, get_time());
    if (!stbi_write_bmp(filename, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, 4, pixels)) DualLogError("Failed to save screenshot\n"); else DualLog("Saved screenshot %s\n", filename);
    OS_DeallocateRAM(pixels, Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char));
}

__attribute__((pure)) bool CursorVisible(void) {
    return (Sys_Global.inventoryMode || Sys_Global.menuActive || Sys_Global.gamePaused);
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
float LoadRelativeTimeDifferential(char* trimmed_value, char* initialLine, uint32_t lineNum) { return parse_float(trimmed_value, initialLine, lineNum) + (float)Sys_Global.pauseRelativeTime; }
