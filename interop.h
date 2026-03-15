#ifdef MOD_INTEROP_IMPLEMENTATION  // mod.h usage:
    // Interop - To Engine
    #if defined(_WIN32) || defined(__CYGWIN__)
        #define MOD_TO_ENGINE __declspec(dllexport)
    #else
        #define MOD_TO_ENGINE __attribute__((visibility("default")))
    #endif
    MOD_TO_ENGINE bool Sprint(void);
    MOD_TO_ENGINE bool Attack(void);
    MOD_TO_ENGINE void UpdateMusic(void);
    MOD_TO_ENGINE void PlayMenuMusic(void);
    MOD_TO_ENGINE void PlayGameMusic(void);
    MOD_TO_ENGINE void ResetLevelMusic(void);
    MOD_TO_ENGINE void ModInitAfterLoad(void);
    MOD_TO_ENGINE void ModEntityDefinitionsInitAfterLoad(DataParser* entity_parser);
    MOD_TO_ENGINE void PlayerInit(uint16_t i);
#else                              // voxen.h usage:
    // Interop - From Mod
    #ifdef MOD_INTEROP
        #define MOD_TO_ENGINE // This is the definition
    #else
        #define MOD_TO_ENGINE extern // Shared declaration
    #endif
    MOD_TO_ENGINE void (*ModInit)(GlobalContext*,CheatsSystem*,SettingsSystem*);
    MOD_TO_ENGINE void (*ModUpdate)(void);
    MOD_TO_ENGINE bool (*Forward)(void);
    MOD_TO_ENGINE bool (*StrafeLeft)(void);
    MOD_TO_ENGINE bool (*Backpedal)(void);
    MOD_TO_ENGINE bool (*StrafeRight)(void);
    MOD_TO_ENGINE bool (*Jump)(void);
    MOD_TO_ENGINE bool (*JumpDown)(void);
    MOD_TO_ENGINE bool (*Crouch)(void);
    MOD_TO_ENGINE bool (*Prone)(void);
    MOD_TO_ENGINE bool (*LeanLeft)(void);
    MOD_TO_ENGINE bool (*LeanRight)(void);
    MOD_TO_ENGINE bool (*Sprint)(void);
    MOD_TO_ENGINE bool (*TurnLeft)(void);
    MOD_TO_ENGINE bool (*TurnRight)(void);
    MOD_TO_ENGINE bool (*LookUp)(void);
    MOD_TO_ENGINE bool (*LookDown)(void);
    MOD_TO_ENGINE bool (*RecentLog)(void);
    MOD_TO_ENGINE bool (*Biomonitor)(void);
    MOD_TO_ENGINE bool (*Sensaround)(void);
    MOD_TO_ENGINE bool (*Lantern)(void);
    MOD_TO_ENGINE bool (*Shield)(void);
    MOD_TO_ENGINE bool (*Infrared)(void);
    MOD_TO_ENGINE bool (*Email)(void);
    MOD_TO_ENGINE bool (*Booster)(void);
    MOD_TO_ENGINE bool (*Jumpjets)(void);
    MOD_TO_ENGINE bool (*Attack)(void);
    MOD_TO_ENGINE bool (*Use)(void);
    MOD_TO_ENGINE bool (*Menu)(void);
    MOD_TO_ENGINE bool (*ToggleMode)(void);
    MOD_TO_ENGINE bool (*Reload)(void);
    MOD_TO_ENGINE bool (*WeaponCycUp)(void);
    MOD_TO_ENGINE bool (*WeaponCycDown)(void);
    MOD_TO_ENGINE bool (*Grenade)(void);
    MOD_TO_ENGINE bool (*GrenadeCycUp)(void);
    MOD_TO_ENGINE bool (*GrenadeCycDown)(void);
    MOD_TO_ENGINE bool (*ChangeAmmoType)(void);
    MOD_TO_ENGINE bool (*Patch)(void);
    MOD_TO_ENGINE bool (*PatchCycUp)(void);
    MOD_TO_ENGINE bool (*PatchCycDown)(void);
    MOD_TO_ENGINE bool (*Map)(void);
    MOD_TO_ENGINE bool (*SwimUp)(void);
    MOD_TO_ENGINE bool (*SwimDn)(void);
    MOD_TO_ENGINE bool (*Console)(void);
    MOD_TO_ENGINE float (*GetBasePlayerSpeed)(bool);
    MOD_TO_ENGINE void (*InitializeAIAfterLoad)(uint16_t);
    MOD_TO_ENGINE bool (*TakeScreenshot)(void);
    MOD_TO_ENGINE void (*UpdateMusic)(void);
    MOD_TO_ENGINE void (*PlayMenuMusic)(void);
    MOD_TO_ENGINE void (*PlayGameMusic)(void);
    MOD_TO_ENGINE void (*ResetLevelMusic)(void);
    MOD_TO_ENGINE void (*ModInitAfterLoad)(void);
    MOD_TO_ENGINE void (*ModEntityDefinitionsInitAfterLoad)(DataParser*);
    MOD_TO_ENGINE void (*PlayerInit)(uint16_t);
#endif


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
