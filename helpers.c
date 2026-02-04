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
bool ConstIndexIsButtonSwitch(int constdex) { return ((constdex >= 688 && constdex <= 692) || constdex == 694 || constdex == 695); }
bool ConstIndexIsDynamicObject(uint16_t constIndex) {
    return     (constIndex >= 307 && constIndex <= 404) ||  constIndex == 417 || (constIndex >= 419 && constIndex <= 428)
            || (constIndex >= 430 && constIndex <= 437) || (constIndex >= 440 && constIndex <= 442)
            || (constIndex >= 458 && constIndex <= 463) || (constIndex >= 465 && constIndex <= 476);
}

bool ConstIndexIsStaticObjectSaveable(int constdex) {
	return (constdex == 112 || constdex == 279 || (constdex >= 448 && constdex < 458) || constdex == 480 || constdex == 516
			|| (constdex >= 518 && constdex <= 526) || constdex == 530 || constdex == 531 || constdex == 546
			|| constdex == 555 || constdex == 594 || constdex == 596 || constdex == 598 || (constdex >= 600 && constdex < 603)
			|| (constdex >= 604 && constdex < 616) || (constdex >= 688 && constdex < 693) || constdex == 694 || constdex == 695
			|| (constdex >= 699 && constdex < 704) || (constdex >= 741 && constdex < 746));
}

bool ConstIndexIsStaticObjectImmutable(int constdex) {
	return ((constdex >= 527 && constdex < 530) || (constdex >= 532 && constdex < 546) || (constdex >= 547 && constdex < 553)
			|| constdex == 554 || (constdex >= 556 && constdex < 594) || constdex == 595 || constdex == 597 || constdex == 599
			|| constdex == 601 || constdex == 603 || (constdex >= 616 && constdex < 688) || constdex == 693 || constdex == 696 || constdex == 697
			|| constdex == 698 || (constdex >= 704 && constdex < 717) || constdex == 720 || (constdex >= 733 && constdex < 736)
			|| (constdex >= 737 && constdex < 739) || constdex == 746 || constdex == 747 || (constdex >= 750 && constdex <= 759 && constdex != 755));
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

uint8_t random_range(float a, float b) {
    return a + ((b - a) * ((float)rand() / RAND_MAX));
}

float lerp(float min, float max, float val) { return min + (max - min) * vclamp(val,0.0f,1.0f); }
float inverse_lerp(float min, float max, float val) { return (min == max) ? 0.0f : vclamp((val - min) / (max - min),0.0f,1.0f); }

float smooth_damp(float current, float target, float *current_velocity, float smooth_time) { 
    if (smooth_time < 0.0001f) smooth_time = 0.0001f;
    float omega = 2.0f / smooth_time;
    float x = omega * (float)Sys_Global.timeSinceLastPhysicsTick;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float original_to = target;
    target = current - change;
    float temp = (*current_velocity + omega * change) * (float)Sys_Global.timeSinceLastPhysicsTick;
    *current_velocity = (*current_velocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;
    if ((original_to - current > 0.0f) == (output > original_to)) {
        output = original_to;
        *current_velocity = (output - original_to) / (float)Sys_Global.timeSinceLastPhysicsTick;
    }

    return output;
}

size_t GetStringLength(const char *s) {
    if (s == NULL) return 0;
    
    const char *p = s;
    while (*(p++));
    return (size_t)(p - s - 1);
}

char* data_parser_trim(char* s) {
    while (CharacterIsEmpty((unsigned char)*s)) s++;
    if (*s == 0) return s;

    char* e = s + GetStringLength(s) - 1;
    while (e > s && CharacterIsEmpty((unsigned char)*e)) e--;
    e[1] = 0;
    return s;
}

int32_t StringToInt(const char *str) { // atoi replacement
    while (CharacterIsEmpty(*str)) str++;
    int sign = 1;
         if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;

    if (*str < '0' || *str > '9') return 0;
    int64_t result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        if (result > (2147483647 - digit) / 10) return (sign == 1) ? 2147483647 : -2147483648;

        result = result * 10 + digit;
        str++;
    }

    return (int32_t)(sign * result);
}

bool CharacterIsEmpty(const char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; } // isspace replacement
bool StringIsEmpty(const char* a) { // C# String.IsNullOrWhiteSpace replacement
    size_t size = GetStringLength(a);
    for(size_t i=0;i < size;++i) {
        if (a[i] == '\0') break;
        if (!CharacterIsEmpty(a[i])) return false;
    }
    
    return true;
}

bool StringsAreEqual(const char* a, const char* b) { // !strcmp replacement (hated its inverted logic)
    size_t size  = GetStringLength(a);
    size_t size2 = GetStringLength(b);
    if (size != size2) return false;
    
    for (size_t i=0;i<size;++i) {
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') break;
    }
    
    return true;
}

bool StringsAreEqualLimitedBy(const char* a, const char* b, size_t limit) { // !strcmp replacement (hated its inverted logic)
    size_t size  = GetStringLength(a);
    size_t size2 = GetStringLength(b);
    if (size != size2) return false;
    
    for (size_t i=0;i<limit;++i) {
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') break;
    }
    
    return true;
}

void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize) { // strcpy replacement
    size_t size2=GetStringLength(b);
    if (size2>=bufferSize) { DualLogError("Error attempting string copy from B into A but B is bigger than buffer limit %u! A: ",bufferSize); DualLogError("%s, B: %s\n",a,b); OS_Exit(1); }
    
    for (size_t i=0;i<size2;++i) a[i] = b[i];
    a[size2] = '\0';
}

void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize) { // strcpy replacement (hopefully my mnemonic "SubstringFrom_B" will help me remember substringSize comes before be in the args passed)
    if (substringSize >= bufferSize) { DualLogError("Substring too large for buffer!\n");  OS_Exit(1); }
    
    for (size_t i=0;i<substringSize;++i) a[i] = b[i];
    a[substringSize] = '\0';
}

void StringConcatenate(char* a, const char* b, size_t bufferSize) { // strcat replacement
    size_t size  = GetStringLength(a);
    size_t size2 = GetStringLength(b);
    if (size + size2 >= bufferSize) { DualLogError("Strings to large to concat, will overflow buffer: GetStringLength(%s{%u} + %s{%u} > %u)\n",a,size,b,size2,bufferSize); OS_Exit(1); }

    char* dest = a + size;
    for (size_t i=0;i<size2; ++i) dest[i] = b[i];
    dest[size2] = '\0';
}

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
uint8_t GetCurrentLevelSecurity() { return (Sys_Global.difficultyMission < 1 || Sys_Cheats.superoverride) ? 0u : Sys_Global.levelSecurity[Sys_Global.currentLevel]; }

uint16_t GetImpactType(uint16_t instanceIdx) {
    switch (instances[instanceIdx].bloodType) {
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

int hardware14fromConstdex(int constdex) { return clamp(constdex - 21,0,14); }
