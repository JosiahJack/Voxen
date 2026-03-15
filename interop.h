// Mod/Gamecode Functions::
#pragma once
#define MOD_FUNCTION_LIST(X) \
    X(void, ModInit, (GlobalContext* ctx, CheatsSystem* cheats, SettingsSystem* settings)) \
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
    X(bool, Email, (void)) \
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
    X(float,GetBasePlayerSpeed, (bool isSprinting)) \
    X(void, InitializeAIAfterLoad, (uint16_t entityIdx)) \
    X(bool, TakeScreenshot, (void)) \
    X(void, UpdateMusic, (void)) \
    X(void, PlayMenuMusic, (void)) \
    X(void, PlayGameMusic, (void)) \
    X(void, ResetLevelMusic, (void)) \
    X(void, ModInitAfterLoad, (void)) \
    X(void, ModEntityDefinitionsInitAfterLoad, (DataParser* parser)) \
    X(void, PlayerInit, (uint16_t playerIdx))

#ifdef MOD_INTEROP_IMPLEMENTATION  // mod.h usage:
    // Interop - To Engine
    #if defined(_WIN32) || defined(__CYGWIN__)
        #define MOD_TO_ENGINE __declspec(dllexport)
    #else
        #define MOD_TO_ENGINE __attribute__((visibility("default")))
    #endif
    #define X(ret, name, params) MOD_TO_ENGINE ret name params;
    MOD_FUNCTION_LIST(X)
    #undef X
#else                              // voxen.h usage:
    // Interop - From Mod
    #ifdef MOD_INTEROP
        #define MOD_TO_ENGINE // This is the definition
    #else
        #define MOD_TO_ENGINE extern // Shared declaration
    #endif
    #define X(ret, name, params) MOD_TO_ENGINE ret (*name) params;
    MOD_FUNCTION_LIST(X)
    #undef X
#endif
    
// ----------------------------------------------------------------------------
// Engine Functions::
#ifdef MOD_INTEROP_IMPLEMENTATION // mod.h usage:
    // Interop - To Engine
    #define ENGINE_TO_MOD extern
#else                             // voxen.h usage:
    // Interop - To Mod
    #if defined(_WIN32) || defined(__CYGWIN__)
        #define ENGINE_TO_MOD __declspec(dllexport)
    #else
        #define ENGINE_TO_MOD __attribute__((visibility("default")))
    #endif
#endif
ENGINE_TO_MOD void DualLog(const char* fmt, ...);
ENGINE_TO_MOD void DualLogWarn(const char* fmt, ...);
ENGINE_TO_MOD void DualLogError(const char* fmt, ...);
ENGINE_TO_MOD uint8_t random_range_u8(uint8_t a, uint8_t b);
ENGINE_TO_MOD uint32_t random_range_u32(uint32_t a, uint32_t b);
ENGINE_TO_MOD int32_t random_range_i32(int32_t a, int32_t b);
ENGINE_TO_MOD float random_range(float a, float b);
ENGINE_TO_MOD double random_rangedub(double a, double b);
ENGINE_TO_MOD double get_time(void);
ENGINE_TO_MOD float lerp(float min, float max, float val);
ENGINE_TO_MOD float inverse_lerp(float min, float max, float val);
ENGINE_TO_MOD void mp3_clear(void);
ENGINE_TO_MOD void play_mp3(const char* path, int32_t fade_in_ms);
ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound* sound);
ENGINE_TO_MOD float GetSoundRemainingTime(ma_sound* pSound);
ENGINE_TO_MOD void AddCameraPosition(uint16_t camIdx);
ENGINE_TO_MOD bool StringIsEmpty(const char* a);
ENGINE_TO_MOD void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize);
