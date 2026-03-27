// interop.h - Mod/Gamecode Functions::
#pragma once
#define MOD_FUNCTION_LIST(X) \
    X(void, ModLink, (GlobalContext* ctx, CheatsSystem* cheats, SettingsSystem* settings, TextSystem* text, SystemUI* ui)) \
    X(void, ModUpdate, (bool)) \
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
    X(float,GetBasePlayerSpeed, (uint16_t, bool)) \
    X(void, InitializeAIAfterLoad, (uint16_t)) \
    X(bool, TakeScreenshot, (void)) \
    X(void, UpdateMusic, (void)) \
    X(void, UpdateAmbientSounds, (void)) \
    X(void, PlayMenuMusic, (void)) \
    X(void, PlayGameMusic, (void)) \
    X(void, ResetLevelMusic, (void)) \
    X(void, ResetLevelAudio, (void)) \
    X(void, UpdateAnims, (void)) \
    X(uint16_t, SpawnDynamicObject, (int, bool)) \
    X(void, TextureSequenceInit, (uint16_t, char*)) \
    X(void, LoadEntities, (void)) \
    X(void, ModNewGame, (void)) \
    X(void, ModInitAfterLoad, (void)) \
    X(void, ModEntityDefinitionsInitAfterLoad, (DataParser*)) \
    X(void, PlayerInit, (uint16_t)) \
    X(void, ProcessInput, (void)) \
    X(void, CheckAndTakeScreenshot, (void)) \
    X(uint16_t, GetCursorTexture, (void)) \
    X(void, LoadLevelMod, (uint8_t))

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
    #if defined(_WIN32) || defined(__CYGWIN__)
        #define MOD_TO_ENGINE
        #define CALL_CONV     __cdecl
        #define X(ret, name, params) MOD_TO_ENGINE ret (CALL_CONV *name) params;
        MOD_FUNCTION_LIST(X)
        #undef X
    #else
        #ifdef MOD_INTEROP // voxen.c imported this for the actual dlopen/dlsym
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
ENGINE_TO_MOD bool StringsAreEqual(const char* a, const char* b);
ENGINE_TO_MOD void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize);
ENGINE_TO_MOD RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, uint32_t layerMask);
ENGINE_TO_MOD void AddDebugLine(Vector3 start, Vector3 end);
ENGINE_TO_MOD int32_t PosGetCellCoords(float pos_x, float pos_z);
ENGINE_TO_MOD int StringFormatV(char* buffer, size_t bufferSize, const char* format, va_list args); // vsnprintf replacement
ENGINE_TO_MOD int StringFormat(char* buffer, size_t bufferSize, const char* format, ...); // snprintf replacement
ENGINE_TO_MOD bool PositionVisibleFromPlayerCell(float x, float z);
ENGINE_TO_MOD void SoundUninit(ma_sound* snd);
ENGINE_TO_MOD ma_result SoundInit(const char* path, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound);
ENGINE_TO_MOD void SoundSetLooping(ma_sound* pSound, ma_bool32 isLooping);
ENGINE_TO_MOD void SoundSetVolume(ma_sound* pSound, float volume);
ENGINE_TO_MOD ma_result SoundStart(ma_sound* pSound);
ENGINE_TO_MOD ma_result SoundStop(ma_sound* pSound);
ENGINE_TO_MOD float SoundGetLength(ma_sound* pSound);
ENGINE_TO_MOD ma_result SoundGetCurrentFrameCursor(const ma_sound* pSound, ma_uint64* pCursor);
ENGINE_TO_MOD void Screenshot(void);
ENGINE_TO_MOD void ToggleConsole(void);
ENGINE_TO_MOD void MenuGoBack(void);
ENGINE_TO_MOD void IgnoreNextMouseDelta(void);
ENGINE_TO_MOD void ApplyPlayerMovements(void);
ENGINE_TO_MOD void AddForce(uint16_t idx, Vector3 force, bool isImpulse);
ENGINE_TO_MOD void DeleteInstance(uint16_t i);
ENGINE_TO_MOD void CenterStatusPrint(const char* fmt, ...);
ENGINE_TO_MOD void play_wav(const char* path, float volume, Vector3 pos, bool positional);
ENGINE_TO_MOD void PortalCulling(void);
ENGINE_TO_MOD void AddInstance(uint16_t entIdx, uint16_t i);
ENGINE_TO_MOD bool parse_data_file(DataParser *parser, uint16_t maxSize, const char *filename);
ENGINE_TO_MOD char* GetLevelFileNextStringUpToNewlineOrEOF(char* buf, int size);
ENGINE_TO_MOD void LoadFieldIntoLight(char* trimmed_key, char* trimmed_value, char* initialLine, uint32_t lineNum, uint32_t lightsIdx, bool* lightOnRead, uint8_t* lightType);
ENGINE_TO_MOD void AddLightFromLoad(bool lightOnRead, int32_t* lightsIdx, uint8_t lightType, uint32_t lineNum);
ENGINE_TO_MOD int32_t AddLight(Vector3 pos, Color col, float range, float intensity, float maxIntensity, float minIntensity, float spotAng, Quaternion spotDir, bool on, bool shadOn);
ENGINE_TO_MOD void UpdateLight(uint16_t lightsIdx, Vector3 pos, Color col, float range, float intensity, float maxIntensity, float minIntensity, float spotAng, Quaternion spotDir, bool on, bool shadOn);
ENGINE_TO_MOD void InitializeEntity(Entity* entry);
ENGINE_TO_MOD size_t GetStringLength(const char* s);
ENGINE_TO_MOD char* StringFindFirstCharWithin(const char *s, char c);
ENGINE_TO_MOD bool CharacterIsEmpty(const char c);
ENGINE_TO_MOD void AddDoorPortal(uint16_t entIdx, uint16_t parent);
ENGINE_TO_MOD bool ToggleDoorPortal(uint8_t portalIdx, uint16_t doorIdx, uint16_t closedModelIndex);
extern EntityField entityFields[NUM_ENTITY_FIELDS];
