// interop.h - Mod/Gamecode Functions::
#pragma once
#define MOD_FUNCTION_LIST(X) \
    X(void, ModLink, (GlobalContext* ctx, CheatsSystem* cheats, SettingsSystem* settings, TextSystem* text, SystemUI* ui)) \
    X(void, ModUpdate, (void)) \
    X(bool, Forward, (void)) \
    X(bool, StrafeLeft, (void)) \
    X(bool, Backpedal, (void)) \
    X(bool, StrafeRight, (void)) \
    X(bool, Jump, (void)) \
    X(bool, JumpDown, (void)) \
    X(bool, Crouch, (void)) \
    X(bool, Prone, (void)) \
    X(bool, LeanLeft, (void)) \
    X(bool, LeanRight, (void)) \
    X(bool, Sprint, (void)) \
    X(bool, TurnLeft, (void)) \
    X(bool, TurnRight, (void)) \
    X(bool, LookUp, (void)) \
    X(bool, LookDown, (void)) \
    X(bool, RecentLog, (void)) \
    X(bool, Biomonitor, (void)) \
    X(bool, Sensaround, (void)) \
    X(bool, Lantern, (void)) \
    X(bool, Shield, (void)) \
    X(bool, Infrared, (void)) \
    X(bool, Booster, (void)) \
    X(bool, Jumpjets, (void)) \
    X(bool, Attack, (void)) \
    X(bool, Use, (void)) \
    X(bool, Menu, (void)) \
    X(bool, ToggleMode, (void)) \
    X(bool, Reload, (void)) \
    X(bool, WeaponCycUp, (void)) \
    X(bool, WeaponCycDown, (void)) \
    X(bool, Grenade, (void)) \
    X(bool, GrenadeCycUp, (void)) \
    X(bool, GrenadeCycDown, (void)) \
    X(bool, ChangeAmmoType, (void)) \
    X(bool, Patch, (void)) \
    X(bool, PatchCycUp, (void)) \
    X(bool, PatchCycDown, (void)) \
    X(bool, Map, (void)) \
    X(bool, SwimUp, (void)) \
    X(bool, SwimDn, (void)) \
    X(bool, Console, (void)) \
    X(float,GetBasePlayerSpeed, (u16, bool)) \
    X(void, InitializeAIAfterLoad, (u16)) \
    X(bool, TakeScreenshot, (void)) \
    X(void, UpdateMusic, (void)) \
    X(void, UpdateAmbientSounds, (void)) \
    X(void, PlayMenuMusic, (void)) \
    X(void, PlayGameMusic, (void)) \
    X(void, ResetLevelMusic, (void)) \
    X(void, ResetLevelAudio, (void)) \
    X(void, UpdateAnims, (void)) \
    X(u16, SpawnDynamicObject, (int, bool)) \
    X(void, TextureSequenceInit, (u16, char*)) \
    X(void, ModNewGame, (void)) \
    X(void, ModInitAfterLoad, (void)) \
    X(void, ModEntityDefinitionsInitAfterLoad, (void)) \
    X(void, PlayerInit, (u16)) \
    X(void, ProcessInput, (void)) \
    X(void, CheckAndTakeScreenshot, (void)) \
    X(u16, GetCursorTexture, (void)) \
    X(void, LoadLevelMod, (u8)) \
    X(void, SetModFatigue, (float)) \
    X(void, ForceShootMode, (void)) \
    X(bool, ModRequestsGrayscale, (void)) \
    X(const char**, GetCreditsText, (void))

#ifdef MOD_INTEROP_MOD  // mod.h usage:
    // Interop - To Engine
    #if defined(_WIN32)
        #define MOD_TO_ENGINE __declspec(dllexport) __cdecl
    #else
        #define MOD_TO_ENGINE __attribute__((visibility("default")))
    #endif
    #define X(ret, name, params) MOD_TO_ENGINE ret name params;
    MOD_FUNCTION_LIST(X)
    #undef X
#else                              // voxen.h usage:
    // Interop - From Mod
    #if defined(_WIN32)
        #define MOD_TO_ENGINE
        #define CALL_CONV     __cdecl
        #define X(ret, name, params) MOD_TO_ENGINE ret (CALL_CONV *name) params;
        MOD_FUNCTION_LIST(X)
        #undef X
    #else
        #ifdef MOD_INTEROP_ENGINE // voxen.c imported this for the actual dlopen/dlsym
            #define MOD_TO_ENGINE // This is the definition
        #else
            #define MOD_TO_ENGINE extern // Shared declaration
        #endif
        #define X(ret, name, params) MOD_TO_ENGINE ret (*name) params;
        MOD_FUNCTION_LIST(X)
        #undef X
    #endif
#endif
    
// ----------------------------------------------------------------------------
// Engine Functions::
#ifdef MOD_INTEROP_MOD // mod.h usage:
    // Interop - To Engine
    #define ENGINE_TO_MOD extern
#else                             // voxen.h usage, MOD_INTEROP_ENGINE:
    // Interop - To Mod
    #if defined(_WIN32)
        #define ENGINE_TO_MOD __declspec(dllexport) __cdecl
    #else
        #define ENGINE_TO_MOD __attribute__((visibility("default")))
    #endif
#endif
ENGINE_TO_MOD void* MemSetToVForNBytes(void *dst, int c, size_t n);
ENGINE_TO_MOD void DualLog(const char* fmt, ...);
ENGINE_TO_MOD void DualLogWarn(const char* fmt, ...);
ENGINE_TO_MOD void DualLogError(const char* fmt, ...);
ENGINE_TO_MOD u8 random_range_u8(u8 a, u8 b);
ENGINE_TO_MOD u32 random_range_u32(u32 a, u32 b);
ENGINE_TO_MOD i32 random_range_i32(i32 a, i32 b);
ENGINE_TO_MOD float random_range(float a, float b);
ENGINE_TO_MOD double get_time(void);
ENGINE_TO_MOD float lerp(float min, float max, float val);
ENGINE_TO_MOD float inverse_lerp(float min, float max, float val);
ENGINE_TO_MOD float smooth_damp(float current, float target, float *current_velocity, float smooth_time);
ENGINE_TO_MOD void play_wav(const char* path, float volume, Vector3 pos, bool positional);
ENGINE_TO_MOD void play_message(const char* path);
ENGINE_TO_MOD void play_mp3(const char* path, i32 fade_in_ms);
ENGINE_TO_MOD void mp3_clear(void);
ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound* sound);
ENGINE_TO_MOD float GetMP3RemainingTime(void);
ENGINE_TO_MOD void MP3Pause(void);
ENGINE_TO_MOD void MP3Resume(void);
ENGINE_TO_MOD bool StringIsEmpty(const char* a);
ENGINE_TO_MOD bool StringsEqual(const char* a, const char* b);
ENGINE_TO_MOD void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize);
ENGINE_TO_MOD RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, u32 layerMask);
ENGINE_TO_MOD void RaycastAll(Vector3 origin, Vector3 dir, float distance, u32 layerMask, RaycastHit* hits, u16 maxCount);
ENGINE_TO_MOD RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, u32 layerMask, bool hitTriggers);
ENGINE_TO_MOD bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, u32 layerMask);
ENGINE_TO_MOD void AddDebugLine(Vector3 start, Vector3 end, Color col);
ENGINE_TO_MOD i32 PosGetCellCoords(float pos_x, float pos_z);
ENGINE_TO_MOD int StringFormatV(char* buffer, size_t bufferSize, const char* format, va_list args); // vsnprintf replacement
ENGINE_TO_MOD int StringFormat(char* buffer, size_t bufferSize, const char* format, ...); // snprintf replacement
ENGINE_TO_MOD bool PositionVisibleFromPlayerCell(float x, float z);
ENGINE_TO_MOD void SoundUninit(ma_sound* snd);
ENGINE_TO_MOD i32 SoundInit(const char* path, ma_sound* pSound);
ENGINE_TO_MOD void SoundSetLooping(ma_sound* pSound, i32 isLooping);
ENGINE_TO_MOD void SoundSetVolume(ma_sound* pSound, float volume);
ENGINE_TO_MOD i32 SoundStart(ma_sound* pSound);
ENGINE_TO_MOD i32 SoundStop(ma_sound* pSound);
ENGINE_TO_MOD float SoundGetLength(ma_sound* pSound);
ENGINE_TO_MOD i32 SoundGetCurrentFrameCursor(const ma_sound* pSound, u64* pCursor);
ENGINE_TO_MOD void Screenshot(void);
ENGINE_TO_MOD void ToggleConsole(void);
ENGINE_TO_MOD void MenuGoBack(void);
ENGINE_TO_MOD void IgnoreNextMouseDelta(void);
ENGINE_TO_MOD void ApplyPlayerMovements(void);
ENGINE_TO_MOD void AddForce(u16 idx, Vector3 force, bool isImpulse);
ENGINE_TO_MOD void CenterStatusPrint(const char* fmt, ...);
ENGINE_TO_MOD void PortalCulling(void);
ENGINE_TO_MOD char* GetLevelFileNextStringUpToNewlineOrEOF(char* buf, int size);
ENGINE_TO_MOD void LoadLevel(u8 curlevel);
ENGINE_TO_MOD void LoadFieldIntoLight(char* trimmed_key, char* trimmed_value, char* initialLine, u32 lineNum, Light* lit, LightAnimation* lan, u16 lightIdx);
ENGINE_TO_MOD i32 AddLight(Light* lit, LightAnimation* lanim);
ENGINE_TO_MOD void UpdateLight(u16 lightsIdx, Vector3 pos, Color3 col, float range, float intensity, float maxIntensity, float minIntensity, float spotAng, Quaternion spotDir, bool on, bool shadOn);
ENGINE_TO_MOD void InitializeEntity(Entity* entry);
ENGINE_TO_MOD size_t GetStringLength(const char* s);
ENGINE_TO_MOD char* StringFindFirstCharWithin(const char *s, char c);
ENGINE_TO_MOD bool CharacterIsEmpty(const char c);
ENGINE_TO_MOD void AddDoorPortal(u16 entIdx, u16 parent);
ENGINE_TO_MOD bool ToggleDoorPortal(u8 portalIdx, u16 doorIdx, u16 closedModelIndex);
ENGINE_TO_MOD Vector3 GetEntityLocalSpawnPointFromUnrotatedOffsetVector(Entity* originator, Vector3 offsetFromOriginator);
ENGINE_TO_MOD void TurnLightOff(u16 litIdx);
ENGINE_TO_MOD void AddCamView(Vector3 pos, Quaternion rot, u8 fov, u16 width, u16 height, float near, float far);
ENGINE_TO_MOD void SetPosition(Entity* e, Vector3 newpos, bool teleport);

// Common inlines that need to span both engine and gamecode
static inline __attribute__((always_inline)) u32 parse_numberu32(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (CharacterIsEmpty((char)*str)) str++;
    while (CharacterIsEmpty(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10uL + (unsigned long)digit;
        str++;
    }

    return (u32)result;
}

static inline __attribute__((always_inline)) u16 parse_numberu16(const char* str, const char* line, u32 lineNum) {
    u32 retval = parse_numberu32(str, line, lineNum);
    if (retval > U16_MAX) { DualLogError("Value %u out of range for u16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (u16)retval;
}

static inline __attribute__((always_inline)) u8 parse_numberu8(const char* str, const char* line, u32 lineNum) {
    u32 retval = parse_numberu32(str, line, lineNum);
    if (retval > U8_MAX) { DualLogError("Value %u out of range for u8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (u8)retval;
}

static inline __attribute__((always_inline)) bool parse_bool(const char* str, const char* line, u32 lineNum) {
    u32 parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);
    return parseval > 0 ? true : false;
}

static inline __attribute__((always_inline)) i32 parse_numberi32(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (CharacterIsEmpty((char)*str)) str++;
    bool negative = false;
    if (*str == '+') str++;
    else if (*str == '-') { negative = true; str++; }
    long result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10L + (*str - '0');
        str++;
    }
    return (i32)(negative ? -result : result);
}
static inline __attribute__((always_inline)) i16 parse_numberi16(const char* str, const char* line, u32 lineNum) {
    i32 retval = parse_numberi32(str, line, lineNum);
    if (retval < -32768 || retval > 32767) { DualLogError("Value %d out of range for i16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (i16)retval;
}
static inline __attribute__((always_inline)) i8 parse_numberi8(const char* str, const char* line, u32 lineNum) {
    i32 retval = parse_numberi32(str, line, lineNum);
    if (retval < -128 || retval > 127) { DualLogError("Value %d out of range for i8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (i8)retval;
}

static inline __attribute__((always_inline)) float parse_float(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    
    while (CharacterIsEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }

    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { // Integer part
        value = value * 10.0 + (*str - '0');
        str++;
        has_digit = true;
    }

    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') {
            frac += (*str - '0') * place;
            place *= 0.1;
            str++;
            has_digit = true;
        }

        value += frac;
    }

    if (!has_digit) return 0.0f;

    if (negative) value = -value;
    return (float)value;
}
