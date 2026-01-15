# Voxen

## The Voxel Lit Open Source Engine

This is a pure C (C11) rendering engine with a focus on high performance and 
simplicity for first person shooter games.  This project was developed with 
Citadel: The System Shock Fan Remake in mind but should be reasonably
extendable and modifiable for anything, being based on FOSS MIT licensing and
principles.  Please take this and make it your own for your own projects.

The "Voxel Lit" portion of Voxen is in the representation of lighting 
information using a voxel format that is an invisible 3D layer of data
overlayed with the normal full 3D polygonal mesh world such that lighting
calculations for bounce lighting (GI), reflections, and other effects leverage 
the spacial voxel data to optimize lighting calculations for high fidelity at
high speed with low RAM usage.  The voxel volume is limited to a space that is
a world cell region 64x64x18 with each cell sized to 2.56x2.56x2.56. Voxels are
subtended as 8x8 regions on the x,z plane (for now) and used as light clusters
for Forward+ rendering pipeline. This engine is focused on interior spaces similar
to Quake, Half-Life, and other classic games.  The voxel volume may work fine for
outdoor environments but Voxen is not intended to be used for large open world games.
Further, the procedural sky is hardcoded and not intended to be a general sky system.
Modifications are of course welcome, however.  The hope is that everything is quite
straightforward.

Based on GLFW and OpenGL 4.3+, this engine attempts to leverage low latency and
GPU driven rendering methods with minimal state changes and maximum flexibility
with user customizable entities.  Heavy use of SSBOs is made though this is still
compatible with old hardware and GL drivers from 15yrs ago; further very few GL
extensions are used to further widen compatibility.  Careful handling of CPU to GPU
transfers is made to minimize VRAM and to prevent naughty GL drivers duplicating
that VRAM into the CPU RAM space which is also kept minimal.

All texture and model data is loaded from disk directly for ease of development
and full mod support by design.  Any intermediate format is internal to the engine.

The engine uses a unified event system queue for debuggability, logging, and
log file playback in a very similar manner to Quake 1 demo files.  This engine
uses the same .dem file extension but in a custom binary format specific to
Voxen's event format with event type and payload variables.  All engine actions
run through the queue.

Minimizing hierarchical layers and leveraging sensibly named globals to cut out
fluff and overhead is important.  Minimal dependencies and leveraging tried and
true systems is important.

## Supported Platforms

- **Linux (64-bit)**: Primarily Debian-based distros (e.g., Kubuntu, Xubuntu) with X11. Wayland support is not intentional.
- **Windows (64-bit)**: Supports Windows 7+. Windows 11 support is not intentional.
- **MacOS (64-bit)**: Not supported due to OpenGL deprecation in favor of Metal.  Metal not supported at this time. TBD.

### Test Systems

* AMD Ryzen 5000 + Nvidia GTX970, Linux 64bit Kubuntu 20.04, 16GB RAM 3200mhz (MAIN RIG)
* AMD Ryzen 1600X + Nvidia GTX550Ti, Linux 64bit Xubuntu 20.04, 32GB RAM 1866mhz (QUAKE MAP COMPILER RIG)
* Intel 4400 + Mesa integrated APU, Linux 64bit Xubuntu 20.04, 4GB RAM (WORK TRIP POTATO)

## Building

This is first and foremost a Linux based project.  Cross compile for Windows is TBD.
Build by calling ./build.sh build script.

### Prerequisites

Project must be linked against the following libraries which your system must install.  I'll continue to reduce these as much as I can:
 * mold linker
 * -lGL (`sudo apt install libgl1-mesa-dev`)
 * -L./External -l:libassimp.6.0.2.a -lz -lstdc++ -static-libstdc++ (Prebuilt, included in ./External/, for model loading from .fbx (for now))
 * libstdc++
 * -lfontconfig

Single command:

```bash
sudo apt install libglfw3-dev libgl1-mesa-dev libassimp-dev
```

## System Architecture

Order of Ops:
Initializes various core systems (Unified Event Queue, Client-Server, OpenGL+Window)
Loads data resources (textures, models, etc.)
Loads scripting VM
Parses all game/mod scripts
Initializes data handling systems and parsers using all above data
Level Load using gamedata definition to pick starting level
Starts game loop:
  Polls GLFW input and enqueues input Events
  Processes input and sets key states, mouselook
  Iterates over all queued Server events (Physics, Game Logic scripts (VM))
  Client-side rendering
Exit with cleanup, conditionally cleaning up resources based on how we exited and when

---

### Systems:

#### Unified Event Queue

All Server actions occur as events processed by the
event queue that runs on just the Server's main thread.
Journaling as it goes for debugging, doubles as a log
feature and supports playback of logs similar to Quake
demo files and uses same .dem extension but with a
different custom format.


#### Data Resource Loading

All game assets are loaded as different types of data
via first loading a definition text file from ./Data
then populating a list from which the particular
asset type is then loaded into fixed flat buffers for
use either in CPU or GPU shaders.

e.g. Textures load ./Data/textures.txt definition file
then load all specified .png images from that text
file out of the file path specified in the definition
file... ideally ./Textues folder.  Images are loaded
into a fixed buffer at the index specified by the
textures.txt definition file.  These indices are used
by all other systems that use textures (e.g. instaces).


#### Entity - Instance System

All objects/items in the game are Instances that
have an associated Entity type.  No instances exist
without a type.  Some Entity types specified by the
entities.txt file may be unused by a game/mod.
Entity definition is loaded first to populate the
types list.  The Instances are populated after as
a product of the level load system.

#### Level Load System

Levels are specified in sets of files for each level data
type: geometry, dynamic objects, lights.  Geometry are any
immovable static mesh based rendered objects which may be
walls, shelves, floors, ceilings, crates, windows, etc.
Dynamic objects are anything that can move or change state
and include even hidden game state tracking entity instances
because they can change their state.  Geometry is guaranteed
static after level load.  Lights are the 3rd system loaded
for a level and are a list of defined light sources with
their brightness, color, and other values (e.g. spot angle).
The gamedata definition file specifies the first level
index to load.  All level definition files are specific
and use with a number for the level index.
E.g. level3_geometery.txt, level3_lights.txt
Levels use same specification as savegames, in plain text.


#### Savegame System

All script variables are saved.  All instance states are
saved.  All physics states are saved, referenced by instance
index.  No systems rely on pointers and are indexed array
based to ensure all links are preserved in saves.  All save
data is in plaintext format using pipe delimiter | to split
each key:value pair which are colon separated.  The key is
given by the variable name, variable names pulled from the
scripts on the instance based on its entity type.


#### VXGI Lighting

Voxen wouldn't be called Voxen without Voxels.  The world is
overlayed with a sparse voxel representation for storing and
updating lighting information such as Global Illumination (GI)
and Shadows which include Ambient Occlusion.  This is
calculated on a separate thread then passed to GPU for actually
applying lighting/shadows.


#### Screen Space Reflections

All specular surfaces get reflections.  There are
no specular highlight fakeries to be found here.
As this is "screenspace" it can only reflect what
the player can see elsewhere in their screen. This
may be augmented with the, albeit softer and
blurrier, voxel results.  Also called SSR.

#### Rendering System

Rendering uses a multipass system with forward+ lighting with voxel light clusters (x,z voxel columns).
Pass 1: Forward+ Rasterization - gets albedo, normals, depth
                                 world position, indices.
                                 This is standard vert+frag.
                                 Applies shadows and lighting.

Pass 2: Screen Space Reflections (SSR): Compute shader full
                                        screen effect that
                                        is subtle.

Pass 3: Final Blit -  Takes the results of the compute
                      shaders and renders image as full
                      screen quad.  Applies Antialiasing and Post Processing.

Rendering leverages static buffers for minimal CPU->GPU
data transfers and maximal performance with minimal state
changes.

#### Texturing System

Leveraging a unified single buffer for all texture colors,
palettized by texture, allows for completely arbitrary
unlimited texture sizes (up to VRAM) in any size with no
rebinding overhead, only passing to GPU once ever.  All
texture data is accessible GPU-side and not stored on CPU.
ALL.  Normalmaps, glow maps, specular maps, UI.  ALL.
One flat buffer of color, One flat buffer of palette
offsets, one flat buffer of palette indices, one flat
buffer of texture palette indices offsets.

#### Mesh System

Models are loaded into one unified flat vertex buffer with
minimal data, just position, normal, and uv.  Meshes are indexed triangles.

---

### License

MIT-0

#### Stats

Log ouput from standard run:

```
Compiling voxen...
Build completed in 378 ms
Voxen v0.7.4 by W. Josiah Jack, MIT-0 licensed
Window positioned (windowed, centered) on monitor: DVI-I-1 (primary) at 2717,531
Using GLFW 3.5.0 Wayland X11 GLX Null EGL OSMesa monotonic, OpenGL Version: 4.3.0 NVIDIA 550.144.03, GPU: NVIDIA GeForce GTX 970/PCIe/SSE2CPU: AMD Ryzen 5 5500 | Logical cores: 12
Loaded    2 fonts...in 0.010 s
Loading game definition from ./Data/gamedata.txt... loaded Game Definition for Citadel:: num levels: 14, start level: 1... took 0.000032 secs
GL buffers, FBO, fonts, audio, localization, and window init took 0.154020 secs
Loading textures(1272), using stb_image version: 2.28, total palette colors: 72723, total pixels: 23737139... took 0.395442 secs
Loading   models( 698) with max index  697 ... total vertices: 1796046, total tris: 7184184, took 0.172740 secs
Loading  768 entities... took 0.000912 secs
Loaded 6056 geometry chunks and 893 static lights for Level 1... took 0.023691 secs
Sorting entity instances... opaque: 5477, double-sided: 42, transparent: 40, invisible: 494... took 0.006461 secs
Culling...found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341... took 0.136474 secs
Game Initialized in 0.926241 secs
```

Log output from one run with DEBUG_RAM_OUTPUT declared in voxen.h:

```
Compiling voxen...
Build completed in 371 ms
Memory at program start: Heap usage 73856 bytes (72 KB | 0.07 MB), USS 1609728 bytes (1572 KB | 1.54 MB)
Memory at prior to event system init: Heap usage 79488 bytes (77 KB | 0.08 MB), USS 1613824 bytes (1576 KB | 1.54 MB)
Voxen v0.7.4 by W. Josiah Jack, MIT-0 licensed
Window positioned (windowed, centered) on monitor: DVI-I-1 (primary) at 2717,531
Using GLFW 3.5.0 Wayland X11 GLX Null EGL OSMesa monotonic, OpenGL Version: 4.3.0 NVIDIA 550.144.03, GPU: NVIDIA GeForce GTX 970/PCIe/SSE2CPU: AMD Ryzen 5 5500 | Logical cores: 12
Memory at GL Buffer and shader setup: Heap usage 7101376 bytes (6934 KB | 6.77 MB), USS 17629184 bytes (17216 KB | 16.81 MB)
Memory at setup gbuffer end: Heap usage 9112192 bytes (8898 KB | 8.69 MB), USS 19599360 bytes (19140 KB | 18.69 MB)
Loaded    2 fonts...in 0.010 s
Memory at after InitFontAtlasses: Heap usage 9119072 bytes (8905 KB | 8.70 MB), USS 37638144 bytes (36756 KB | 35.89 MB)
Memory at after InitializeAudio: Heap usage 10523504 bytes (10276 KB | 10.04 MB), USS 38813696 bytes (37904 KB | 37.02 MB)
Memory at LoadTextForLanguage end: Heap usage 10523504 bytes (10276 KB | 10.04 MB), USS 40103936 bytes (39164 KB | 38.25 MB)
Memory at LoadLogTextForLanguage end: Heap usage 10573328 bytes (10325 KB | 10.08 MB), USS 40157184 bytes (39216 KB | 38.30 MB)
Loading game definition from ./Data/gamedata.txt... loaded Game Definition for Citadel:: num levels: 14, start level: 1... took 0.000019 secs
Memory at ParseGameData end: Heap usage 10573808 bytes (10325 KB | 10.08 MB), USS 40161280 bytes (39220 KB | 38.30 MB)
Memory at after freeing window bar icon: Heap usage 10575408 bytes (10327 KB | 10.09 MB), USS 40198144 bytes (39256 KB | 38.34 MB)
GL buffers, FBO, fonts, audio, localization, and window init took 0.160541 secs
Loading texturesMemory at start of LoadTextures: Heap usage 10575408 bytes (10327 KB | 10.09 MB), USS 40198144 bytes (39256 KB | 38.34 MB)
Memory at After loop for load textures: Heap usage 10575408 bytes (10327 KB | 10.09 MB), USS 72732672 bytes (71028 KB | 69.36 MB)
(1272), using stb_image version: 2.28, total palette colors: 72723, total pixels: 23737139... took 0.404324 secs
Memory at After LoadTextures and after munmap of LoadTextures arena and stbi arena: Heap usage 10892496 bytes (10637 KB | 10.39 MB), USS 64118784 bytes (62616 KB | 61.15 MB)
Loading   models( 698) with max index  697 ...Memory at after main mmap block: Heap usage 11166128 bytes (10904 KB | 10.65 MB), USS 64405504 bytes (62896 KB | 61.42 MB)
Memory at prior to parallel model load loop: Heap usage 11166560 bytes (10904 KB | 10.65 MB), USS 64409600 bytes (62900 KB | 61.43 MB)
Memory at after to parallel model load loop: Heap usage 11166560 bytes (10904 KB | 10.65 MB), USS 126656512 bytes (123688 KB | 120.79 MB)
Memory at after to model to gpu transfer: Heap usage 77180368 bytes (75371 KB | 73.60 MB), USS 115732480 bytes (113020 KB | 110.37 MB)
 total vertices: 1796046, total tris: 7184184, took 0.178472 secs
Memory at After Load Models: Heap usage 77201792 bytes (75392 KB | 73.63 MB), USS 51359744 bytes (50156 KB | 48.98 MB)
Loading  768 entities... took 0.000908 secs
Memory at after loading all entities: Heap usage 77803936 bytes (75980 KB | 74.20 MB), USS 51965952 bytes (50748 KB | 49.56 MB)
Memory at start of LoadLevel: Heap usage 80718288 bytes (78826 KB | 76.98 MB), USS 62992384 bytes (61516 KB | 60.07 MB)
Loaded 6056 geometry chunks and 893 static lights for Level 1... took 0.025371 secs
Memory at end of LoadLevel instances: Heap usage 80718288 bytes (78826 KB | 76.98 MB), USS 67129344 bytes (65556 KB | 64.02 MB)
Sorting entity instances... opaque: 5477, double-sided: 42, transparent: 40, invisible: 494... took 0.007188 secs
Culling...Memory at start of Cull_Init: Heap usage 80727600 bytes (78835 KB | 76.99 MB), USS 71147520 bytes (69480 KB | 67.85 MB)
found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341...Memory at end of dynamic culling DetermineClosedEdges: Heap usage 80727600 bytes (78835 KB | 76.99 MB), USS 71180288 bytes (69512 KB | 67.88 MB)
 took 0.138372 secs
Memory at end of Cull_Init: Heap usage 80727600 bytes (78835 KB | 76.99 MB), USS 73293824 bytes (71576 KB | 69.90 MB)
Memory at InitializeEnvironment end: Heap usage 84538784 bytes (82557 KB | 80.62 MB), USS 94359552 bytes (92148 KB | 89.99 MB)
Memory at prior to game loop: Heap usage 84538784 bytes (82557 KB | 80.62 MB), USS 94359552 bytes (92148 KB | 89.99 MB)
Game Initialized in 0.960962 secs
Memory at after 4 frames of running: Heap usage 84569504 bytes (82587 KB | 80.65 MB), USS 99037184 bytes (96716 KB | 94.45 MB)
Memory at after 100 frames of running: Heap usage 84596368 bytes (82613 KB | 80.68 MB), USS 99667968 bytes (97332 KB | 95.05 MB)
Memory at after 200 frames of running: Heap usage 84599296 bytes (82616 KB | 80.68 MB), USS 99667968 bytes (97332 KB | 95.05 MB)
Memory at after 500 frames of running: Heap usage 84569504 bytes (82587 KB | 80.65 MB), USS 99667968 bytes (97332 KB | 95.05 MB)
Memory at after 1000 frames of running: Heap usage 84586896 bytes (82604 KB | 80.67 MB), USS 99667968 bytes (97332 KB | 95.05 MB)
```

Heap impacts:

```
❯ grep -rIn  "alloc("
dynamic_culling.c:45:    uint8_t* file_buffer = malloc(maxFileSize);
entity.c:178:    entities = calloc(entityCount,sizeof(Entity));
entity.c:239:    if (modelTypeCountsOpaque      ) { free(modelTypeCountsOpaque      ); }   modelTypeCountsOpaque = calloc(loadedModels,sizeof(uint16_t)); // Zero out all arrays and counters
entity.c:240:    if (modelTypeCountsDoubleSided ) { free(modelTypeCountsDoubleSided ); }   modelTypeCountsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
entity.c:241:    if (modelTypeCountsTransparent ) { free(modelTypeCountsTransparent ); }   modelTypeCountsTransparent = calloc(loadedModels,sizeof(uint16_t));
entity.c:242:    if (modelTypeOffsetsOpaque     ) { free(modelTypeOffsetsOpaque     ); }   modelTypeOffsetsOpaque = calloc(loadedModels,sizeof(uint16_t));
entity.c:243:    if (modelTypeOffsetsDoubleSided) { free(modelTypeOffsetsDoubleSided); }   modelTypeOffsetsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
entity.c:244:    if (modelTypeOffsetsTransparent) { free(modelTypeOffsetsTransparent); }   modelTypeOffsetsTransparent = calloc(loadedModels,sizeof(uint16_t));
data_fonts.c:25:        char *p = malloc(len);
data_fonts.c:161:    unsigned char *data = malloc(size);
data_fonts.c:288:    primaryFontData = malloc(pri_sz);
data_fonts.c:294:    unsigned char *sec_data = malloc(sec_sz);
data_fonts.c:320:    unsigned char *bmp = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
data_fonts.c:358:    bmp = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
data_parser.c:166:        Entity *new_entries = realloc(parser->entries, entry_count * sizeof(Entity));  
helpers.c:201:    unsigned char* pixels = malloc(screen_width * screen_height * 4 * sizeof(char));
data_text.c:53:    stringTable = malloc(TEXT_STRING_COUNT * sizeof(char*));
data_text.c:69:    uint8_t* file_data = malloc(file_size);
data_text.c:112:                    stringTable[lineNum] = malloc(1 * sizeof(char));
data_text.c:152:                stringTable[lineNum] = malloc(1 * sizeof(char));
data_text.c:161:            stringTable[lineNum] = malloc((len+1) * sizeof(char));
data_text.c:192:    audioLogImagesRefIndicesLH = calloc(TEXT_LOGS_COUNT, sizeof(uint16_t));
data_text.c:193:    audioLogImagesRefIndicesRH = calloc(TEXT_LOGS_COUNT, sizeof(uint16_t));
data_text.c:194:    audioLogType               = calloc(TEXT_LOGS_COUNT, sizeof(uint8_t));
data_text.c:195:    audioLogLevelFound         = calloc(TEXT_LOGS_COUNT, sizeof(uint16_t));
data_text.c:196:    audiologNames       = calloc(TEXT_LOGS_COUNT, sizeof(char*));
data_text.c:197:    audiologSenders     = calloc(TEXT_LOGS_COUNT, sizeof(char*));
data_text.c:198:    audiologSubjects    = calloc(TEXT_LOGS_COUNT, sizeof(char*));
data_text.c:199:    audioLogSpeech2Text = calloc(TEXT_LOGS_COUNT, sizeof(char*));
data_text.c:220:    uint8_t *file_data = malloc((size_t)file_size);
data_text.c:324:                (dst)[readIndexOfLog] = malloc(slen + 1); \
NOTE: Excluded miniaudio, stb_image ./External alloc calls
```

Binary static variable impacts:

```
❯ size voxen
   text    data     bss       dec     hex filename
3183917   55952 56755608 59995477 3937555 voxen
```


Individual static variable impacts:

```
❯ nm -S --size-sort -r ./voxen | grep ' [BD] '
000000000103ef00 0000000000400000 B voxelLightListsRaw
000000000069f160 00000000003bd080 B tempInstances
00000000014e0f80 00000000003bd080 B instances
0000000000e3ef00 0000000000200000 B voxelLightListIndices
0000000000348b20 0000000000200000 B precomputedVisibleCellsFromHere
0000000000549b60 0000000000154578 B voxen_Text
0000000000b1a800 0000000000100000 B lightCounts
0000000000c1a800 00000000000e1000 B lightFrustumPlanes
0000000001444b80 000000000009c400 B modelMatrices
0000000000a9a080 000000000002ee00 B lightIntervalSteps
0000000000a6ac40 000000000002ee00 B intervalStepisLerping
000000000189eac0 0000000000023140 B _glfw
0000000000afb780 000000000001f060 B uiImageVertexData
0000000000e29100 0000000000014500 B lights
0000000000adff00 0000000000013880 B visibleInstances
0000000000ad3e60 0000000000009c40 B eventJournal
000000000033eee0 0000000000009c40 B cellIndexForInstance
0000000000af3780 0000000000008000 B textVertexData
0000000000331920 0000000000007000 B modelBounds
0000000000a5c1e0 0000000000004e20 B transparentInstances
0000000000a65e20 0000000000004e20 B opaqueInstances
0000000000a61000 0000000000004e20 B doubleSidedInstances
0000000000324d00 00000000000045e4 B fontPackedCharStopD
0000000000329300 00000000000045e4 B fontPackedChar
000000000032d900 0000000000004000 B mmap_cleanup
000000000031eec0 0000000000003b80 B wav_sounds
000000000143fae0 0000000000002710 B instanceIsLODArray
0000000001442460 0000000000002710 B dirtyInstances
0000000000322fa0 0000000000001c00 B history
0000000000e3d600 0000000000001900 B shadowmapIndirectionList
0000000000e27800 0000000000001900 B lightsRangeSquared
0000000000ad2480 0000000000001900 B lightMinIntensity
0000000000ad0b80 0000000000001900 B lightMaxIntensity
0000000000acdfc0 0000000000001900 B lightLerpValue
0000000000acc6c0 0000000000001900 B lightLerpTime
0000000000acadc0 0000000000001900 B lightLerpStepTime
0000000000ac94c0 0000000000001900 B lightLerpStartTime
0000000000339920 0000000000001000 B vbos
0000000000338920 0000000000001000 B tbos
000000000033b920 0000000000001000 B modelVertexCounts
000000000033a920 0000000000001000 B modelTriangleCounts
0000000000548b20 0000000000001000 B gridCellStates
0000000000addaa0 0000000000000fa0 B eventQueue
000000000033c960 0000000000000c80 B cellIndexForLightZ
000000000033d5e0 0000000000000c80 B cellIndexForLightX
000000000033e260 0000000000000c80 B cellIndexForLight
0000000000314a60 0000000000000c58 D _glfwDefaultMappings
000000000069e0e0 0000000000000800 B transparentTexture
000000000069e8e0 0000000000000800 B doubleSidedTexture
000000000143f380 0000000000000700 B uiImages
0000000000313ba0 0000000000000640 D lightOn
0000000000acff00 0000000000000640 B lightLerpUp
0000000000ad0540 0000000000000640 B lightLerpOn
0000000000ac8e80 0000000000000640 B lightIntervalStepsLength
0000000000a99a40 0000000000000640 B lightIntervalStepIsLerpingLength
0000000000314280 0000000000000640 D lightDirty
0000000000acf8c0 0000000000000640 B lightCurrentStep
000000000189e4a0 0000000000000600 B keyStates
0000000000322a40 0000000000000520 B audio_engine
000000000143ef60 0000000000000400 B uiTextBuffer
000000000189e000 0000000000000400 B statusText
00000000003136e0 0000000000000400 D consoleEntryText
00000000018c26c0 0000000000000118 B _ZSt4wcin
00000000018c2b40 0000000000000118 B _ZSt3cin
00000000018c25a0 0000000000000110 B _ZSt5wcout
00000000018c2360 0000000000000110 B _ZSt5wclog
00000000018c2480 0000000000000110 B _ZSt5wcerr
00000000018c2a20 0000000000000110 B _ZSt4cout
00000000018c27e0 0000000000000110 B _ZSt4clog
00000000018c2900 0000000000000110 B _ZSt4cerr
000000000030ede0 0000000000000110 D _ZNSt6locale17_S_twinned_facetsE
0000000001442340 0000000000000100 B global_modname
00000000003148e0 0000000000000090 D textColors
00000000003168c0 0000000000000070 D _ZNSt17__timepunct_cacheIwE12_S_timezonesE
0000000000316940 0000000000000070 D _ZNSt17__timepunct_cacheIcE12_S_timezonesE
0000000000314220 0000000000000060 D cubemapOrientationQuaternion
00000000018c21e0 0000000000000050 B _ZN14__gnu_internal14buf_wcout_syncE
00000000018c2120 0000000000000050 B _ZN14__gnu_internal14buf_wcerr_syncE
00000000018c2180 0000000000000050 B _ZN14__gnu_internal13buf_wcin_syncE
00000000018c2300 0000000000000050 B _ZN14__gnu_internal13buf_cout_syncE
00000000018c2240 0000000000000050 B _ZN14__gnu_internal13buf_cerr_syncE
00000000018c22a0 0000000000000050 B _ZN14__gnu_internal12buf_cin_syncE
000000000030ae40 0000000000000050 D z_errmsg
000000000143fa80 0000000000000050 B voxen_GL_Comms
000000000143ef20 0000000000000040 B uiOrthoProjection
0000000001442220 0000000000000040 B shadowmapsPerspectiveProjection
0000000001442260 0000000000000040 B rasterPerspectiveProjection
000000000031ee60 0000000000000040 B ambientRegistry
00000000014422e0 0000000000000034 B questData
0000000000313b00 0000000000000030 D fontRangesStopD
0000000000313b40 0000000000000030 D fontRanges
000000000030d9c0 0000000000000030 D dataDeviceListener
000000000030d260 0000000000000028 D wp_fractional_scale_manager_v1_interface
000000000030cd80 0000000000000028 D _glfw_zxdg_toplevel_decoration_v1_interface
000000000030ccc0 0000000000000028 D _glfw_zxdg_decoration_manager_v1_interface
000000000030cfa0 0000000000000028 D _glfw_zwp_relative_pointer_v1_interface
000000000030cf20 0000000000000028 D _glfw_zwp_relative_pointer_manager_v1_interface
000000000030d040 0000000000000028 D _glfw_zwp_pointer_constraints_v1_interface
000000000030d120 0000000000000028 D _glfw_zwp_locked_pointer_v1_interface
000000000030d540 0000000000000028 D _glfw_zwp_idle_inhibitor_v1_interface
000000000030d4e0 0000000000000028 D _glfw_zwp_idle_inhibit_manager_v1_interface
000000000030d1e0 0000000000000028 D _glfw_zwp_confined_pointer_v1_interface
000000000030c720 0000000000000028 D _glfw_xdg_wm_base_interface
000000000030cb40 0000000000000028 D _glfw_xdg_toplevel_interface
000000000030c940 0000000000000028 D _glfw_xdg_surface_interface
000000000030c860 0000000000000028 D _glfw_xdg_positioner_interface
000000000030cc40 0000000000000028 D _glfw_xdg_popup_interface
000000000030d380 0000000000000028 D _glfw_xdg_activation_v1_interface
000000000030d460 0000000000000028 D _glfw_xdg_activation_token_v1_interface
000000000030cea0 0000000000000028 D _glfw_wp_viewport_interface
000000000030ce00 0000000000000028 D _glfw_wp_viewporter_interface
000000000030d2e0 0000000000000028 D _glfw_wp_fractional_scale_v1_interface
000000000030c360 0000000000000028 D _glfw_wl_touch_interface
000000000030bec0 0000000000000028 D _glfw_wl_surface_interface
000000000030c660 0000000000000028 D _glfw_wl_subsurface_interface
000000000030c580 0000000000000028 D _glfw_wl_subcompositor_interface
000000000030b5e0 0000000000000028 D _glfw_wl_shm_pool_interface
000000000030b660 0000000000000028 D _glfw_wl_shm_interface
000000000030bd00 0000000000000028 D _glfw_wl_shell_surface_interface
000000000030bb60 0000000000000028 D _glfw_wl_shell_interface
000000000030bfa0 0000000000000028 D _glfw_wl_seat_interface
000000000030b460 0000000000000028 D _glfw_wl_registry_interface
000000000030c500 0000000000000028 D _glfw_wl_region_interface
000000000030c140 0000000000000028 D _glfw_wl_pointer_interface
000000000030c460 0000000000000028 D _glfw_wl_output_interface
000000000030c240 0000000000000028 D _glfw_wl_keyboard_interface
000000000030b3c0 0000000000000028 D _glfw_wl_display_interface
000000000030b940 0000000000000028 D _glfw_wl_data_source_interface
000000000030b800 0000000000000028 D _glfw_wl_data_offer_interface
000000000030bb00 0000000000000028 D _glfw_wl_data_device_manager_interface
000000000030ba80 0000000000000028 D _glfw_wl_data_device_interface
000000000030b540 0000000000000028 D _glfw_wl_compositor_interface
000000000030b4c0 0000000000000028 D _glfw_wl_callback_interface
000000000030b6e0 0000000000000028 D _glfw_wl_buffer_interface
000000000069f120 0000000000000018 B _ZN10ODDLParser7DDLNode16s_allocatedNodesE
000000000189e470 0000000000000018 B mouseButtons
000000000069f140 0000000000000010 B _ZN6Assimp13DefaultLogger13s_pNullLoggerE
00000000003149b0 0000000000000010 D voxen_Settings
000000000033c930 0000000000000010 B model_parser
0000000000ad3e30 0000000000000010 B entity_parser
0000000000314980 0000000000000010 D cam_rotation
0000000000314998 000000000000000d D voxen_Cheats
00000000018c2108 0000000000000008 B _ZNSt7codecvtIwc11__mbstate_tE2idE
00000000018c42f0 0000000000000008 B _ZNSt7codecvtIDsDu11__mbstate_tE2idE
00000000018c4300 0000000000000008 B _ZNSt7codecvtIDsc11__mbstate_tE2idE
00000000018c42e8 0000000000000008 B _ZNSt7codecvtIDiDu11__mbstate_tE2idE
00000000018c42f8 0000000000000008 B _ZNSt7codecvtIDic11__mbstate_tE2idE
00000000018c2110 0000000000000008 B _ZNSt7codecvtIcc11__mbstate_tE2idE
00000000018c2c70 0000000000000008 B _ZNSt6locale9_S_globalE
00000000018c2c60 0000000000000008 B _ZNSt6locale5facet11_S_c_localeE
00000000018c2c78 0000000000000008 B _ZNSt6locale10_S_classicE
00000000018c4520 0000000000000008 B _ZNSt5ctypeIwE2idE
00000000018c4528 0000000000000008 B _ZNSt5ctypeIcE2idE
00000000003168a8 0000000000000008 D _ZNSt10__num_base12_S_atoms_outE
00000000003168b0 0000000000000008 D _ZNSt10__num_base11_S_atoms_inE
00000000003168b8 0000000000000008 D _ZNSt10money_base8_S_atomsE
0000000000313b80 0000000000000008 D _ZN6Assimp13DefaultLogger9m_pLoggerE
00000000003168a0 0000000000000008 D _ZN10__cxxabiv120__unexpected_handlerE
0000000000316898 0000000000000008 D _ZN10__cxxabiv119__terminate_handlerE
000000000189e448 0000000000000008 B voxen_globalContext
0000000000314a08 0000000000000008 D vertexShaderSource
0000000000adfef0 0000000000000008 B timeSinceLastPhysicsTick
0000000000314a18 0000000000000008 D textVertexShaderSource
0000000000314a10 0000000000000008 D textFragmentShaderSource
000000000069f100 0000000000000008 B stbi__arena_end
000000000069f108 0000000000000008 B stbi__arena_cursor
000000000069f110 0000000000000008 B stbi__arena_base
00000000003149d8 0000000000000008 D ssr_computeShader
00000000003149f8 0000000000000008 D shadowmapVertexShaderSource
00000000003149d0 0000000000000008 D shadowmaps_clear_computeShader
00000000003149f0 0000000000000008 D shadowmapFragmentShaderSource
000000000189e468 0000000000000008 B scrollDelta
000000000189e400 0000000000000008 B screenshotTimeout
00000000003149e8 0000000000000008 D quadVertexShaderSource
00000000003149e0 0000000000000008 D quadFragmentShaderSource
0000000001442318 0000000000000008 B pauseRelativeTime
0000000000adfee0 0000000000000008 B monitorSwitchTime
000000000033c920 0000000000000008 B modelVertices
0000000000ad3df0 0000000000000008 B modelTypeOffsetsTransparent
0000000000ad3e00 0000000000000008 B modelTypeOffsetsOpaque
0000000000ad3df8 0000000000000008 B modelTypeOffsetsDoubleSided
0000000000ad3e08 0000000000000008 B modelTypeCountsTransparent
0000000000ad3e18 0000000000000008 B modelTypeCountsOpaque
0000000000ad3e10 0000000000000008 B modelTypeCountsDoubleSided
0000000000adea48 0000000000000008 B manualLogName
0000000000adfee8 0000000000000008 B last_topframe_time
000000000189e428 0000000000000008 B last_time
000000000189e458 0000000000000008 B last_mouse_y
000000000189e460 0000000000000008 B last_mouse_x
0000000000adea60 0000000000000008 B lastJournalWriteTime
000000000189e410 0000000000000008 B lastFrameSecCountTime
0000000000adeac0 0000000000000008 B glad_glWaitSync
0000000000adeac8 0000000000000008 B glad_glViewportIndexedfv
0000000000adead0 0000000000000008 B glad_glViewportIndexedf
0000000000adead8 0000000000000008 B glad_glViewportArrayv
0000000000adeae0 0000000000000008 B glad_glViewport
0000000000adeae8 0000000000000008 B glad_glVertexBindingDivisor
0000000000adeaf0 0000000000000008 B glad_glVertexAttribPointer
0000000000adeaf8 0000000000000008 B glad_glVertexAttribP4uiv
0000000000adeb00 0000000000000008 B glad_glVertexAttribP4ui
0000000000adeb08 0000000000000008 B glad_glVertexAttribP3uiv
0000000000adeb10 0000000000000008 B glad_glVertexAttribP3ui
0000000000adeb18 0000000000000008 B glad_glVertexAttribP2uiv
0000000000adeb20 0000000000000008 B glad_glVertexAttribP2ui
0000000000adeb28 0000000000000008 B glad_glVertexAttribP1uiv
0000000000adeb30 0000000000000008 B glad_glVertexAttribP1ui
0000000000adeb38 0000000000000008 B glad_glVertexAttribLPointer
0000000000adeb40 0000000000000008 B glad_glVertexAttribLFormat
0000000000adeb48 0000000000000008 B glad_glVertexAttribL4dv
0000000000adeb50 0000000000000008 B glad_glVertexAttribL4d
0000000000adeb58 0000000000000008 B glad_glVertexAttribL3dv
0000000000adeb60 0000000000000008 B glad_glVertexAttribL3d
0000000000adeb68 0000000000000008 B glad_glVertexAttribL2dv
0000000000adeb70 0000000000000008 B glad_glVertexAttribL2d
0000000000adeb78 0000000000000008 B glad_glVertexAttribL1dv
0000000000adeb80 0000000000000008 B glad_glVertexAttribL1d
0000000000adeb88 0000000000000008 B glad_glVertexAttribIPointer
0000000000adeb90 0000000000000008 B glad_glVertexAttribIFormat
0000000000adeb98 0000000000000008 B glad_glVertexAttribI4usv
0000000000adeba0 0000000000000008 B glad_glVertexAttribI4uiv
0000000000adeba8 0000000000000008 B glad_glVertexAttribI4ui
0000000000adebb0 0000000000000008 B glad_glVertexAttribI4ubv
0000000000adebb8 0000000000000008 B glad_glVertexAttribI4sv
0000000000adebc0 0000000000000008 B glad_glVertexAttribI4iv
0000000000adebc8 0000000000000008 B glad_glVertexAttribI4i
0000000000adebd0 0000000000000008 B glad_glVertexAttribI4bv
0000000000adebd8 0000000000000008 B glad_glVertexAttribI3uiv
0000000000adebe0 0000000000000008 B glad_glVertexAttribI3ui
0000000000adebe8 0000000000000008 B glad_glVertexAttribI3iv
0000000000adebf0 0000000000000008 B glad_glVertexAttribI3i
0000000000adebf8 0000000000000008 B glad_glVertexAttribI2uiv
0000000000adec00 0000000000000008 B glad_glVertexAttribI2ui
0000000000adec08 0000000000000008 B glad_glVertexAttribI2iv
0000000000adec10 0000000000000008 B glad_glVertexAttribI2i
0000000000adec18 0000000000000008 B glad_glVertexAttribI1uiv
0000000000adec20 0000000000000008 B glad_glVertexAttribI1ui
0000000000adec28 0000000000000008 B glad_glVertexAttribI1iv
0000000000adec30 0000000000000008 B glad_glVertexAttribI1i
0000000000adec38 0000000000000008 B glad_glVertexAttribFormat
0000000000adec40 0000000000000008 B glad_glVertexAttribDivisor
0000000000adec48 0000000000000008 B glad_glVertexAttribBinding
0000000000adec50 0000000000000008 B glad_glVertexAttrib4usv
0000000000adec58 0000000000000008 B glad_glVertexAttrib4uiv
0000000000adec60 0000000000000008 B glad_glVertexAttrib4ubv
0000000000adec68 0000000000000008 B glad_glVertexAttrib4sv
0000000000adec70 0000000000000008 B glad_glVertexAttrib4s
0000000000adeca8 0000000000000008 B glad_glVertexAttrib4Nusv
0000000000adecb0 0000000000000008 B glad_glVertexAttrib4Nuiv
0000000000adecb8 0000000000000008 B glad_glVertexAttrib4Nubv
0000000000adecc0 0000000000000008 B glad_glVertexAttrib4Nub
0000000000adecc8 0000000000000008 B glad_glVertexAttrib4Nsv
0000000000adecd0 0000000000000008 B glad_glVertexAttrib4Niv
0000000000adecd8 0000000000000008 B glad_glVertexAttrib4Nbv
0000000000adec78 0000000000000008 B glad_glVertexAttrib4iv
0000000000adec80 0000000000000008 B glad_glVertexAttrib4fv
0000000000adec88 0000000000000008 B glad_glVertexAttrib4f
0000000000adec90 0000000000000008 B glad_glVertexAttrib4dv
0000000000adec98 0000000000000008 B glad_glVertexAttrib4d
0000000000adeca0 0000000000000008 B glad_glVertexAttrib4bv
0000000000adece0 0000000000000008 B glad_glVertexAttrib3sv
0000000000adece8 0000000000000008 B glad_glVertexAttrib3s
0000000000adecf0 0000000000000008 B glad_glVertexAttrib3fv
0000000000adecf8 0000000000000008 B glad_glVertexAttrib3f
0000000000aded00 0000000000000008 B glad_glVertexAttrib3dv
0000000000aded08 0000000000000008 B glad_glVertexAttrib3d
0000000000aded10 0000000000000008 B glad_glVertexAttrib2sv
0000000000aded18 0000000000000008 B glad_glVertexAttrib2s
0000000000aded20 0000000000000008 B glad_glVertexAttrib2fv
0000000000aded28 0000000000000008 B glad_glVertexAttrib2f
0000000000aded30 0000000000000008 B glad_glVertexAttrib2dv
0000000000aded38 0000000000000008 B glad_glVertexAttrib2d
0000000000aded40 0000000000000008 B glad_glVertexAttrib1sv
0000000000aded48 0000000000000008 B glad_glVertexAttrib1s
0000000000aded50 0000000000000008 B glad_glVertexAttrib1fv
0000000000aded58 0000000000000008 B glad_glVertexAttrib1f
0000000000aded60 0000000000000008 B glad_glVertexAttrib1dv
0000000000aded68 0000000000000008 B glad_glVertexAttrib1d
0000000000aded70 0000000000000008 B glad_glVertexArrayVertexBuffers
0000000000aded78 0000000000000008 B glad_glVertexArrayVertexBuffer
0000000000aded80 0000000000000008 B glad_glVertexArrayElementBuffer
0000000000aded88 0000000000000008 B glad_glVertexArrayBindingDivisor
0000000000aded90 0000000000000008 B glad_glVertexArrayAttribLFormat
0000000000aded98 0000000000000008 B glad_glVertexArrayAttribIFormat
0000000000adeda0 0000000000000008 B glad_glVertexArrayAttribFormat
0000000000adeda8 0000000000000008 B glad_glVertexArrayAttribBinding
0000000000adedb0 0000000000000008 B glad_glValidateProgramPipeline
0000000000adedb8 0000000000000008 B glad_glValidateProgram
0000000000adedc0 0000000000000008 B glad_glUseProgramStages
0000000000adedc8 0000000000000008 B glad_glUseProgram
0000000000adedd0 0000000000000008 B glad_glUnmapNamedBuffer
0000000000adedd8 0000000000000008 B glad_glUnmapBuffer
0000000000adede0 0000000000000008 B glad_glUniformSubroutinesuiv
0000000000adede8 0000000000000008 B glad_glUniformMatrix4x3fv
0000000000adedf0 0000000000000008 B glad_glUniformMatrix4x3dv
0000000000adedf8 0000000000000008 B glad_glUniformMatrix4x2fv
0000000000adee00 0000000000000008 B glad_glUniformMatrix4x2dv
0000000000adee08 0000000000000008 B glad_glUniformMatrix4fv
0000000000adee10 0000000000000008 B glad_glUniformMatrix4dv
0000000000adee18 0000000000000008 B glad_glUniformMatrix3x4fv
0000000000adee20 0000000000000008 B glad_glUniformMatrix3x4dv
0000000000adee28 0000000000000008 B glad_glUniformMatrix3x2fv
0000000000adee30 0000000000000008 B glad_glUniformMatrix3x2dv
0000000000adee38 0000000000000008 B glad_glUniformMatrix3fv
0000000000adee40 0000000000000008 B glad_glUniformMatrix3dv
0000000000adee48 0000000000000008 B glad_glUniformMatrix2x4fv
0000000000adee50 0000000000000008 B glad_glUniformMatrix2x4dv
0000000000adee58 0000000000000008 B glad_glUniformMatrix2x3fv
0000000000adee60 0000000000000008 B glad_glUniformMatrix2x3dv
0000000000adee68 0000000000000008 B glad_glUniformMatrix2fv
0000000000adee70 0000000000000008 B glad_glUniformMatrix2dv
0000000000adee78 0000000000000008 B glad_glUniformBlockBinding
0000000000adee80 0000000000000008 B glad_glUniform4uiv
0000000000adee88 0000000000000008 B glad_glUniform4ui
0000000000adee90 0000000000000008 B glad_glUniform4iv
0000000000adee98 0000000000000008 B glad_glUniform4i
0000000000adeea0 0000000000000008 B glad_glUniform4fv
0000000000adeea8 0000000000000008 B glad_glUniform4f
0000000000adeeb0 0000000000000008 B glad_glUniform4dv
0000000000adeeb8 0000000000000008 B glad_glUniform4d
0000000000adeec0 0000000000000008 B glad_glUniform3uiv
0000000000adeec8 0000000000000008 B glad_glUniform3ui
0000000000adeed0 0000000000000008 B glad_glUniform3iv
0000000000adeed8 0000000000000008 B glad_glUniform3i
0000000000adeee0 0000000000000008 B glad_glUniform3fv
0000000000adeee8 0000000000000008 B glad_glUniform3f
0000000000adeef0 0000000000000008 B glad_glUniform3dv
0000000000adeef8 0000000000000008 B glad_glUniform3d
0000000000adef00 0000000000000008 B glad_glUniform2uiv
0000000000adef08 0000000000000008 B glad_glUniform2ui
0000000000adef10 0000000000000008 B glad_glUniform2iv
0000000000adef18 0000000000000008 B glad_glUniform2i
0000000000adef20 0000000000000008 B glad_glUniform2fv
0000000000adef28 0000000000000008 B glad_glUniform2f
0000000000adef30 0000000000000008 B glad_glUniform2dv
0000000000adef38 0000000000000008 B glad_glUniform2d
0000000000adef40 0000000000000008 B glad_glUniform1uiv
0000000000adef48 0000000000000008 B glad_glUniform1ui
0000000000adef50 0000000000000008 B glad_glUniform1iv
0000000000adef58 0000000000000008 B glad_glUniform1i
0000000000adef60 0000000000000008 B glad_glUniform1fv
0000000000adef68 0000000000000008 B glad_glUniform1f
0000000000adef70 0000000000000008 B glad_glUniform1dv
0000000000adef78 0000000000000008 B glad_glUniform1d
0000000000adef80 0000000000000008 B glad_glTransformFeedbackVaryings
0000000000adef88 0000000000000008 B glad_glTransformFeedbackBufferRange
0000000000adef90 0000000000000008 B glad_glTransformFeedbackBufferBase
0000000000adef98 0000000000000008 B glad_glTextureView
0000000000adefa0 0000000000000008 B glad_glTextureSubImage3D
0000000000adefa8 0000000000000008 B glad_glTextureSubImage2D
0000000000adefb0 0000000000000008 B glad_glTextureSubImage1D
0000000000adefb8 0000000000000008 B glad_glTextureStorage3DMultisample
0000000000adefc0 0000000000000008 B glad_glTextureStorage3D
0000000000adefc8 0000000000000008 B glad_glTextureStorage2DMultisample
0000000000adefd0 0000000000000008 B glad_glTextureStorage2D
0000000000adefd8 0000000000000008 B glad_glTextureStorage1D
0000000000adefe0 0000000000000008 B glad_glTextureParameteriv
0000000000adf000 0000000000000008 B glad_glTextureParameterIuiv
0000000000adf008 0000000000000008 B glad_glTextureParameterIiv
0000000000adefe8 0000000000000008 B glad_glTextureParameteri
0000000000adeff0 0000000000000008 B glad_glTextureParameterfv
0000000000adeff8 0000000000000008 B glad_glTextureParameterf
0000000000adf010 0000000000000008 B glad_glTextureBufferRange
0000000000adf018 0000000000000008 B glad_glTextureBuffer
0000000000adf020 0000000000000008 B glad_glTexSubImage3D
0000000000adf028 0000000000000008 B glad_glTexSubImage2D
0000000000adf030 0000000000000008 B glad_glTexSubImage1D
0000000000adf038 0000000000000008 B glad_glTexStorage3DMultisample
0000000000adf040 0000000000000008 B glad_glTexStorage3D
0000000000adf048 0000000000000008 B glad_glTexStorage2DMultisample
0000000000adf050 0000000000000008 B glad_glTexStorage2D
0000000000adf058 0000000000000008 B glad_glTexStorage1D
0000000000adf060 0000000000000008 B glad_glTexParameteriv
0000000000adf080 0000000000000008 B glad_glTexParameterIuiv
0000000000adf088 0000000000000008 B glad_glTexParameterIiv
0000000000adf068 0000000000000008 B glad_glTexParameteri
0000000000adf070 0000000000000008 B glad_glTexParameterfv
0000000000adf078 0000000000000008 B glad_glTexParameterf
0000000000adf090 0000000000000008 B glad_glTexImage3DMultisample
0000000000adf098 0000000000000008 B glad_glTexImage3D
0000000000adf0a0 0000000000000008 B glad_glTexImage2DMultisample
0000000000adf0a8 0000000000000008 B glad_glTexImage2D
0000000000adf0b0 0000000000000008 B glad_glTexImage1D
0000000000adf0b8 0000000000000008 B glad_glTexBufferRange
0000000000adf0c0 0000000000000008 B glad_glTexBuffer
0000000000adf0c8 0000000000000008 B glad_glStencilOpSeparate
0000000000adf0d0 0000000000000008 B glad_glStencilOp
0000000000adf0d8 0000000000000008 B glad_glStencilMaskSeparate
0000000000adf0e0 0000000000000008 B glad_glStencilMask
0000000000adf0e8 0000000000000008 B glad_glStencilFuncSeparate
0000000000adf0f0 0000000000000008 B glad_glStencilFunc
0000000000adf0f8 0000000000000008 B glad_glShaderStorageBlockBinding
0000000000adf100 0000000000000008 B glad_glShaderSource
0000000000adf108 0000000000000008 B glad_glShaderBinary
0000000000adf110 0000000000000008 B glad_glScissorIndexedv
0000000000adf118 0000000000000008 B glad_glScissorIndexed
0000000000adf120 0000000000000008 B glad_glScissorArrayv
0000000000adf128 0000000000000008 B glad_glScissor
0000000000adf130 0000000000000008 B glad_glSamplerParameteriv
0000000000adf150 0000000000000008 B glad_glSamplerParameterIuiv
0000000000adf158 0000000000000008 B glad_glSamplerParameterIiv
0000000000adf138 0000000000000008 B glad_glSamplerParameteri
0000000000adf140 0000000000000008 B glad_glSamplerParameterfv
0000000000adf148 0000000000000008 B glad_glSamplerParameterf
0000000000adf160 0000000000000008 B glad_glSampleMaski
0000000000adf168 0000000000000008 B glad_glResumeTransformFeedback
0000000000adf170 0000000000000008 B glad_glRenderbufferStorageMultisample
0000000000adf178 0000000000000008 B glad_glRenderbufferStorage
0000000000adf180 0000000000000008 B glad_glReleaseShaderCompiler
0000000000adf188 0000000000000008 B glad_glReadPixels
0000000000adf190 0000000000000008 B glad_glReadBuffer
0000000000adf198 0000000000000008 B glad_glQueryCounter
0000000000adf1a0 0000000000000008 B glad_glProvokingVertex
0000000000adf1a8 0000000000000008 B glad_glProgramUniformMatrix4x3fv
0000000000adf1b0 0000000000000008 B glad_glProgramUniformMatrix4x3dv
0000000000adf1b8 0000000000000008 B glad_glProgramUniformMatrix4x2fv
0000000000adf1c0 0000000000000008 B glad_glProgramUniformMatrix4x2dv
0000000000adf1c8 0000000000000008 B glad_glProgramUniformMatrix4fv
0000000000adf1d0 0000000000000008 B glad_glProgramUniformMatrix4dv
0000000000adf1d8 0000000000000008 B glad_glProgramUniformMatrix3x4fv
0000000000adf1e0 0000000000000008 B glad_glProgramUniformMatrix3x4dv
0000000000adf1e8 0000000000000008 B glad_glProgramUniformMatrix3x2fv
0000000000adf1f0 0000000000000008 B glad_glProgramUniformMatrix3x2dv
0000000000adf1f8 0000000000000008 B glad_glProgramUniformMatrix3fv
0000000000adf200 0000000000000008 B glad_glProgramUniformMatrix3dv
0000000000adf208 0000000000000008 B glad_glProgramUniformMatrix2x4fv
0000000000adf210 0000000000000008 B glad_glProgramUniformMatrix2x4dv
0000000000adf218 0000000000000008 B glad_glProgramUniformMatrix2x3fv
0000000000adf220 0000000000000008 B glad_glProgramUniformMatrix2x3dv
0000000000adf228 0000000000000008 B glad_glProgramUniformMatrix2fv
0000000000adf230 0000000000000008 B glad_glProgramUniformMatrix2dv
0000000000adf238 0000000000000008 B glad_glProgramUniform4uiv
0000000000adf240 0000000000000008 B glad_glProgramUniform4ui
0000000000adf248 0000000000000008 B glad_glProgramUniform4iv
0000000000adf250 0000000000000008 B glad_glProgramUniform4i
0000000000adf258 0000000000000008 B glad_glProgramUniform4fv
0000000000adf260 0000000000000008 B glad_glProgramUniform4f
0000000000adf268 0000000000000008 B glad_glProgramUniform4dv
0000000000adf270 0000000000000008 B glad_glProgramUniform4d
0000000000adf278 0000000000000008 B glad_glProgramUniform3uiv
0000000000adf280 0000000000000008 B glad_glProgramUniform3ui
0000000000adf288 0000000000000008 B glad_glProgramUniform3iv
0000000000adf290 0000000000000008 B glad_glProgramUniform3i
0000000000adf298 0000000000000008 B glad_glProgramUniform3fv
0000000000adf2a0 0000000000000008 B glad_glProgramUniform3f
0000000000adf2a8 0000000000000008 B glad_glProgramUniform3dv
0000000000adf2b0 0000000000000008 B glad_glProgramUniform3d
0000000000adf2b8 0000000000000008 B glad_glProgramUniform2uiv
0000000000adf2c0 0000000000000008 B glad_glProgramUniform2ui
0000000000adf2c8 0000000000000008 B glad_glProgramUniform2iv
0000000000adf2d0 0000000000000008 B glad_glProgramUniform2i
0000000000adf2d8 0000000000000008 B glad_glProgramUniform2fv
0000000000adf2e0 0000000000000008 B glad_glProgramUniform2f
0000000000adf2e8 0000000000000008 B glad_glProgramUniform2dv
0000000000adf2f0 0000000000000008 B glad_glProgramUniform2d
0000000000adf2f8 0000000000000008 B glad_glProgramUniform1uiv
0000000000adf300 0000000000000008 B glad_glProgramUniform1ui
0000000000adf308 0000000000000008 B glad_glProgramUniform1iv
0000000000adf310 0000000000000008 B glad_glProgramUniform1i
0000000000adf318 0000000000000008 B glad_glProgramUniform1fv
0000000000adf320 0000000000000008 B glad_glProgramUniform1f
0000000000adf328 0000000000000008 B glad_glProgramUniform1dv
0000000000adf330 0000000000000008 B glad_glProgramUniform1d
0000000000adf338 0000000000000008 B glad_glProgramParameteri
0000000000adf340 0000000000000008 B glad_glProgramBinary
0000000000adf348 0000000000000008 B glad_glPrimitiveRestartIndex
0000000000adf350 0000000000000008 B glad_glPolygonOffset
0000000000adf358 0000000000000008 B glad_glPolygonMode
0000000000adf360 0000000000000008 B glad_glPointSize
0000000000adf368 0000000000000008 B glad_glPointParameteriv
0000000000adf370 0000000000000008 B glad_glPointParameteri
0000000000adf378 0000000000000008 B glad_glPointParameterfv
0000000000adf380 0000000000000008 B glad_glPointParameterf
0000000000adf388 0000000000000008 B glad_glPixelStorei
0000000000adf390 0000000000000008 B glad_glPixelStoref
0000000000adf398 0000000000000008 B glad_glPauseTransformFeedback
0000000000adf3a0 0000000000000008 B glad_glPatchParameteri
0000000000adf3a8 0000000000000008 B glad_glPatchParameterfv
0000000000adf3b0 0000000000000008 B glad_glObjectPtrLabel
0000000000adf3b8 0000000000000008 B glad_glObjectLabel
0000000000adf3c0 0000000000000008 B glad_glNamedRenderbufferStorageMultisample
0000000000adf3c8 0000000000000008 B glad_glNamedRenderbufferStorage
0000000000adf3d0 0000000000000008 B glad_glNamedFramebufferTextureLayer
0000000000adf3d8 0000000000000008 B glad_glNamedFramebufferTexture
0000000000adf3e0 0000000000000008 B glad_glNamedFramebufferRenderbuffer
0000000000adf3e8 0000000000000008 B glad_glNamedFramebufferReadBuffer
0000000000adf3f0 0000000000000008 B glad_glNamedFramebufferParameteri
0000000000adf3f8 0000000000000008 B glad_glNamedFramebufferDrawBuffers
0000000000adf400 0000000000000008 B glad_glNamedFramebufferDrawBuffer
0000000000adf408 0000000000000008 B glad_glNamedBufferSubData
0000000000adf410 0000000000000008 B glad_glNamedBufferStorage
0000000000adf418 0000000000000008 B glad_glNamedBufferData
0000000000adf420 0000000000000008 B glad_glMultiDrawElementsIndirect
0000000000adf428 0000000000000008 B glad_glMultiDrawElementsBaseVertex
0000000000adf430 0000000000000008 B glad_glMultiDrawElements
0000000000adf438 0000000000000008 B glad_glMultiDrawArraysIndirect
0000000000adf440 0000000000000008 B glad_glMultiDrawArrays
0000000000adf448 0000000000000008 B glad_glMinSampleShading
0000000000adf450 0000000000000008 B glad_glMemoryBarrier
0000000000adf458 0000000000000008 B glad_glMapNamedBufferRange
0000000000adf460 0000000000000008 B glad_glMapNamedBuffer
0000000000adf468 0000000000000008 B glad_glMapBufferRange
0000000000adf470 0000000000000008 B glad_glMapBuffer
0000000000adf478 0000000000000008 B glad_glLogicOp
0000000000adf480 0000000000000008 B glad_glLinkProgram
0000000000adf488 0000000000000008 B glad_glLineWidth
0000000000adf490 0000000000000008 B glad_glIsVertexArray
0000000000adf498 0000000000000008 B glad_glIsTransformFeedback
0000000000adf4a0 0000000000000008 B glad_glIsTexture
0000000000adf4a8 0000000000000008 B glad_glIsSync
0000000000adf4b0 0000000000000008 B glad_glIsShader
0000000000adf4b8 0000000000000008 B glad_glIsSampler
0000000000adf4c0 0000000000000008 B glad_glIsRenderbuffer
0000000000adf4c8 0000000000000008 B glad_glIsQuery
0000000000adf4d0 0000000000000008 B glad_glIsProgramPipeline
0000000000adf4d8 0000000000000008 B glad_glIsProgram
0000000000adf4e0 0000000000000008 B glad_glIsFramebuffer
0000000000adf4e8 0000000000000008 B glad_glIsEnabledi
0000000000adf4f0 0000000000000008 B glad_glIsEnabled
0000000000adf4f8 0000000000000008 B glad_glIsBuffer
0000000000adf500 0000000000000008 B glad_glInvalidateTexSubImage
0000000000adf508 0000000000000008 B glad_glInvalidateTexImage
0000000000adf510 0000000000000008 B glad_glInvalidateSubFramebuffer
0000000000adf518 0000000000000008 B glad_glInvalidateNamedFramebufferSubData
0000000000adf520 0000000000000008 B glad_glInvalidateNamedFramebufferData
0000000000adf528 0000000000000008 B glad_glInvalidateFramebuffer
0000000000adf530 0000000000000008 B glad_glInvalidateBufferSubData
0000000000adf538 0000000000000008 B glad_glInvalidateBufferData
0000000000adf540 0000000000000008 B glad_glHint
0000000000adf560 0000000000000008 B glad_glGetVertexAttribPointerv
0000000000adf568 0000000000000008 B glad_glGetVertexAttribLdv
0000000000adf548 0000000000000008 B glad_glGetVertexAttribiv
0000000000adf570 0000000000000008 B glad_glGetVertexAttribIuiv
0000000000adf578 0000000000000008 B glad_glGetVertexAttribIiv
0000000000adf550 0000000000000008 B glad_glGetVertexAttribfv
0000000000adf558 0000000000000008 B glad_glGetVertexAttribdv
0000000000adf580 0000000000000008 B glad_glGetVertexArrayiv
0000000000adf588 0000000000000008 B glad_glGetVertexArrayIndexediv
0000000000adf590 0000000000000008 B glad_glGetVertexArrayIndexed64iv
0000000000adf598 0000000000000008 B glad_glGetUniformuiv
0000000000adf5b8 0000000000000008 B glad_glGetUniformSubroutineuiv
0000000000adf5c0 0000000000000008 B glad_glGetUniformLocation
0000000000adf5a0 0000000000000008 B glad_glGetUniformiv
0000000000adf5c8 0000000000000008 B glad_glGetUniformIndices
0000000000adf5a8 0000000000000008 B glad_glGetUniformfv
0000000000adf5b0 0000000000000008 B glad_glGetUniformdv
0000000000adf5d0 0000000000000008 B glad_glGetUniformBlockIndex
0000000000adf5f0 0000000000000008 B glad_glGetTransformFeedbackVarying
0000000000adf5d8 0000000000000008 B glad_glGetTransformFeedbackiv
0000000000adf5e0 0000000000000008 B glad_glGetTransformFeedbacki_v
0000000000adf5e8 0000000000000008 B glad_glGetTransformFeedbacki64_v
0000000000adf5f8 0000000000000008 B glad_glGetTextureParameteriv
0000000000adf608 0000000000000008 B glad_glGetTextureParameterIuiv
0000000000adf610 0000000000000008 B glad_glGetTextureParameterIiv
0000000000adf600 0000000000000008 B glad_glGetTextureParameterfv
0000000000adf618 0000000000000008 B glad_glGetTextureLevelParameteriv
0000000000adf620 0000000000000008 B glad_glGetTextureLevelParameterfv
0000000000adf628 0000000000000008 B glad_glGetTextureImage
0000000000adf630 0000000000000008 B glad_glGetTexParameteriv
0000000000adf640 0000000000000008 B glad_glGetTexParameterIuiv
0000000000adf648 0000000000000008 B glad_glGetTexParameterIiv
0000000000adf638 0000000000000008 B glad_glGetTexParameterfv
0000000000adf650 0000000000000008 B glad_glGetTexLevelParameteriv
0000000000adf658 0000000000000008 B glad_glGetTexLevelParameterfv
0000000000adf660 0000000000000008 B glad_glGetTexImage
0000000000adf668 0000000000000008 B glad_glGetSynciv
0000000000adf670 0000000000000008 B glad_glGetSubroutineUniformLocation
0000000000adf678 0000000000000008 B glad_glGetSubroutineIndex
0000000000adf680 0000000000000008 B glad_glGetStringi
0000000000adf688 0000000000000008 B glad_glGetString
0000000000adf698 0000000000000008 B glad_glGetShaderSource
0000000000adf6a0 0000000000000008 B glad_glGetShaderPrecisionFormat
0000000000adf690 0000000000000008 B glad_glGetShaderiv
0000000000adf6a8 0000000000000008 B glad_glGetShaderInfoLog
0000000000adf6b0 0000000000000008 B glad_glGetSamplerParameteriv
0000000000adf6c0 0000000000000008 B glad_glGetSamplerParameterIuiv
0000000000adf6c8 0000000000000008 B glad_glGetSamplerParameterIiv
0000000000adf6b8 0000000000000008 B glad_glGetSamplerParameterfv
0000000000adf6d0 0000000000000008 B glad_glGetRenderbufferParameteriv
0000000000adf6e0 0000000000000008 B glad_glGetQueryObjectuiv
0000000000adf6e8 0000000000000008 B glad_glGetQueryObjectui64v
0000000000adf6f0 0000000000000008 B glad_glGetQueryObjectiv
0000000000adf6f8 0000000000000008 B glad_glGetQueryObjecti64v
0000000000adf6d8 0000000000000008 B glad_glGetQueryiv
0000000000adf700 0000000000000008 B glad_glGetQueryIndexediv
0000000000adf708 0000000000000008 B glad_glGetQueryBufferObjectuiv
0000000000adf710 0000000000000008 B glad_glGetQueryBufferObjectui64v
0000000000adf718 0000000000000008 B glad_glGetQueryBufferObjectiv
0000000000adf720 0000000000000008 B glad_glGetQueryBufferObjecti64v
0000000000adf730 0000000000000008 B glad_glGetProgramStageiv
0000000000adf740 0000000000000008 B glad_glGetProgramResourceName
0000000000adf748 0000000000000008 B glad_glGetProgramResourceLocationIndex
0000000000adf750 0000000000000008 B glad_glGetProgramResourceLocation
0000000000adf738 0000000000000008 B glad_glGetProgramResourceiv
0000000000adf758 0000000000000008 B glad_glGetProgramResourceIndex
0000000000adf760 0000000000000008 B glad_glGetProgramPipelineiv
0000000000adf768 0000000000000008 B glad_glGetProgramPipelineInfoLog
0000000000adf728 0000000000000008 B glad_glGetProgramiv
0000000000adf770 0000000000000008 B glad_glGetProgramInterfaceiv
0000000000adf778 0000000000000008 B glad_glGetProgramInfoLog
0000000000adf780 0000000000000008 B glad_glGetProgramBinary
0000000000adf788 0000000000000008 B glad_glGetPointerv
0000000000adf790 0000000000000008 B glad_glGetObjectPtrLabel
0000000000adf798 0000000000000008 B glad_glGetObjectLabel
0000000000adf7a0 0000000000000008 B glad_glGetNamedRenderbufferParameteriv
0000000000adf7a8 0000000000000008 B glad_glGetNamedFramebufferParameteriv
0000000000adf7b0 0000000000000008 B glad_glGetNamedFramebufferAttachmentParameteriv
0000000000adf7b8 0000000000000008 B glad_glGetNamedBufferSubData
0000000000adf7c0 0000000000000008 B glad_glGetNamedBufferPointerv
0000000000adf7c8 0000000000000008 B glad_glGetNamedBufferParameteriv
0000000000adf7d0 0000000000000008 B glad_glGetNamedBufferParameteri64v
0000000000adf7d8 0000000000000008 B glad_glGetMultisamplefv
0000000000adf7e0 0000000000000008 B glad_glGetInternalformativ
0000000000adf7e8 0000000000000008 B glad_glGetInternalformati64v
0000000000adf7f0 0000000000000008 B glad_glGetIntegerv
0000000000adf7f8 0000000000000008 B glad_glGetIntegeri_v
0000000000adf800 0000000000000008 B glad_glGetInteger64v
0000000000adf808 0000000000000008 B glad_glGetInteger64i_v
0000000000adf810 0000000000000008 B glad_glGetFramebufferParameteriv
0000000000adf818 0000000000000008 B glad_glGetFramebufferAttachmentParameteriv
0000000000adf820 0000000000000008 B glad_glGetFragDataLocation
0000000000adf828 0000000000000008 B glad_glGetFragDataIndex
0000000000adf830 0000000000000008 B glad_glGetFloatv
0000000000adf838 0000000000000008 B glad_glGetFloati_v
0000000000adf840 0000000000000008 B glad_glGetError
0000000000adf848 0000000000000008 B glad_glGetDoublev
0000000000adf850 0000000000000008 B glad_glGetDoublei_v
0000000000adf858 0000000000000008 B glad_glGetBufferSubData
0000000000adf860 0000000000000008 B glad_glGetBufferPointerv
0000000000adf868 0000000000000008 B glad_glGetBufferParameteriv
0000000000adf870 0000000000000008 B glad_glGetBufferParameteri64v
0000000000adf878 0000000000000008 B glad_glGetBooleanv
0000000000adf880 0000000000000008 B glad_glGetBooleani_v
0000000000adf888 0000000000000008 B glad_glGetAttribLocation
0000000000adf890 0000000000000008 B glad_glGetAttachedShaders
0000000000adf898 0000000000000008 B glad_glGetActiveUniformsiv
0000000000adf8a0 0000000000000008 B glad_glGetActiveUniformName
0000000000adf8b0 0000000000000008 B glad_glGetActiveUniformBlockName
0000000000adf8a8 0000000000000008 B glad_glGetActiveUniformBlockiv
0000000000adf8b8 0000000000000008 B glad_glGetActiveUniform
0000000000adf8c8 0000000000000008 B glad_glGetActiveSubroutineUniformName
0000000000adf8c0 0000000000000008 B glad_glGetActiveSubroutineUniformiv
0000000000adf8d0 0000000000000008 B glad_glGetActiveSubroutineName
0000000000adf8d8 0000000000000008 B glad_glGetActiveAttrib
0000000000adf8e0 0000000000000008 B glad_glGetActiveAtomicCounterBufferiv
0000000000adf8f8 0000000000000008 B glad_glGenVertexArrays
0000000000adf900 0000000000000008 B glad_glGenTransformFeedbacks
0000000000adf908 0000000000000008 B glad_glGenTextures
0000000000adf910 0000000000000008 B glad_glGenSamplers
0000000000adf918 0000000000000008 B glad_glGenRenderbuffers
0000000000adf920 0000000000000008 B glad_glGenQueries
0000000000adf928 0000000000000008 B glad_glGenProgramPipelines
0000000000adf930 0000000000000008 B glad_glGenFramebuffers
0000000000adf8e8 0000000000000008 B glad_glGenerateTextureMipmap
0000000000adf8f0 0000000000000008 B glad_glGenerateMipmap
0000000000adf938 0000000000000008 B glad_glGenBuffers
0000000000adf940 0000000000000008 B glad_glFrontFace
0000000000adf948 0000000000000008 B glad_glFramebufferTextureLayer
0000000000adf950 0000000000000008 B glad_glFramebufferTexture3D
0000000000adf958 0000000000000008 B glad_glFramebufferTexture2D
0000000000adf960 0000000000000008 B glad_glFramebufferTexture1D
0000000000adf968 0000000000000008 B glad_glFramebufferTexture
0000000000adf970 0000000000000008 B glad_glFramebufferRenderbuffer
0000000000adf978 0000000000000008 B glad_glFramebufferParameteri
0000000000adf980 0000000000000008 B glad_glFlushMappedNamedBufferRange
0000000000adf988 0000000000000008 B glad_glFlushMappedBufferRange
0000000000adf990 0000000000000008 B glad_glFlush
0000000000adf998 0000000000000008 B glad_glFinish
0000000000adf9a0 0000000000000008 B glad_glFenceSync
0000000000adf9a8 0000000000000008 B glad_glEndTransformFeedback
0000000000adf9b0 0000000000000008 B glad_glEndQueryIndexed
0000000000adf9b8 0000000000000008 B glad_glEndQuery
0000000000adf9c0 0000000000000008 B glad_glEndConditionalRender
0000000000adf9d0 0000000000000008 B glad_glEnableVertexAttribArray
0000000000adf9d8 0000000000000008 B glad_glEnableVertexArrayAttrib
0000000000adf9c8 0000000000000008 B glad_glEnablei
0000000000adf9e0 0000000000000008 B glad_glEnable
0000000000adf9e8 0000000000000008 B glad_glDrawTransformFeedbackStreamInstanced
0000000000adf9f0 0000000000000008 B glad_glDrawTransformFeedbackStream
0000000000adf9f8 0000000000000008 B glad_glDrawTransformFeedbackInstanced
0000000000adfa00 0000000000000008 B glad_glDrawTransformFeedback
0000000000adfa08 0000000000000008 B glad_glDrawRangeElementsBaseVertex
0000000000adfa10 0000000000000008 B glad_glDrawRangeElements
0000000000adfa18 0000000000000008 B glad_glDrawElementsInstancedBaseVertexBaseInstance
0000000000adfa20 0000000000000008 B glad_glDrawElementsInstancedBaseVertex
0000000000adfa28 0000000000000008 B glad_glDrawElementsInstancedBaseInstance
0000000000adfa30 0000000000000008 B glad_glDrawElementsInstanced
0000000000adfa38 0000000000000008 B glad_glDrawElementsIndirect
0000000000adfa40 0000000000000008 B glad_glDrawElementsBaseVertex
0000000000adfa48 0000000000000008 B glad_glDrawElements
0000000000adfa50 0000000000000008 B glad_glDrawBuffers
0000000000adfa58 0000000000000008 B glad_glDrawBuffer
0000000000adfa60 0000000000000008 B glad_glDrawArraysInstancedBaseInstance
0000000000adfa68 0000000000000008 B glad_glDrawArraysInstanced
0000000000adfa70 0000000000000008 B glad_glDrawArraysIndirect
0000000000adfa78 0000000000000008 B glad_glDrawArrays
0000000000adfa80 0000000000000008 B glad_glDispatchComputeIndirect
0000000000adfa88 0000000000000008 B glad_glDispatchCompute
0000000000adfa98 0000000000000008 B glad_glDisableVertexAttribArray
0000000000adfaa0 0000000000000008 B glad_glDisableVertexArrayAttrib
0000000000adfa90 0000000000000008 B glad_glDisablei
0000000000adfaa8 0000000000000008 B glad_glDisable
0000000000adfab0 0000000000000008 B glad_glDetachShader
0000000000adfac0 0000000000000008 B glad_glDepthRangeIndexed
0000000000adfab8 0000000000000008 B glad_glDepthRangef
0000000000adfac8 0000000000000008 B glad_glDepthRangeArrayv
0000000000adfad0 0000000000000008 B glad_glDepthRange
0000000000adfad8 0000000000000008 B glad_glDepthMask
0000000000adfae0 0000000000000008 B glad_glDepthFunc
0000000000adfae8 0000000000000008 B glad_glDeleteVertexArrays
0000000000adfaf0 0000000000000008 B glad_glDeleteTransformFeedbacks
0000000000adfaf8 0000000000000008 B glad_glDeleteTextures
0000000000adfb00 0000000000000008 B glad_glDeleteSync
0000000000adfb08 0000000000000008 B glad_glDeleteShader
0000000000adfb10 0000000000000008 B glad_glDeleteSamplers
0000000000adfb18 0000000000000008 B glad_glDeleteRenderbuffers
0000000000adfb20 0000000000000008 B glad_glDeleteQueries
0000000000adfb28 0000000000000008 B glad_glDeleteProgramPipelines
0000000000adfb30 0000000000000008 B glad_glDeleteProgram
0000000000adfb38 0000000000000008 B glad_glDeleteFramebuffers
0000000000adfb40 0000000000000008 B glad_glDeleteBuffers
0000000000adfb48 0000000000000008 B glad_glCullFace
0000000000adfb50 0000000000000008 B glad_glCreateVertexArrays
0000000000adfb58 0000000000000008 B glad_glCreateTransformFeedbacks
0000000000adfb60 0000000000000008 B glad_glCreateTextures
0000000000adfb68 0000000000000008 B glad_glCreateShaderProgramv
0000000000adfb70 0000000000000008 B glad_glCreateShader
0000000000adfb78 0000000000000008 B glad_glCreateSamplers
0000000000adfb80 0000000000000008 B glad_glCreateRenderbuffers
0000000000adfb88 0000000000000008 B glad_glCreateQueries
0000000000adfb90 0000000000000008 B glad_glCreateProgramPipelines
0000000000adfb98 0000000000000008 B glad_glCreateProgram
0000000000adfba0 0000000000000008 B glad_glCreateFramebuffers
0000000000adfba8 0000000000000008 B glad_glCreateBuffers
0000000000adfbb0 0000000000000008 B glad_glCopyTextureSubImage3D
0000000000adfbb8 0000000000000008 B glad_glCopyTextureSubImage2D
0000000000adfbc0 0000000000000008 B glad_glCopyTextureSubImage1D
0000000000adfbc8 0000000000000008 B glad_glCopyTexSubImage3D
0000000000adfbd0 0000000000000008 B glad_glCopyTexSubImage2D
0000000000adfbd8 0000000000000008 B glad_glCopyTexSubImage1D
0000000000adfbe0 0000000000000008 B glad_glCopyTexImage2D
0000000000adfbe8 0000000000000008 B glad_glCopyTexImage1D
0000000000adfbf0 0000000000000008 B glad_glCopyNamedBufferSubData
0000000000adfbf8 0000000000000008 B glad_glCopyImageSubData
0000000000adfc00 0000000000000008 B glad_glCopyBufferSubData
0000000000adfc08 0000000000000008 B glad_glCompileShader
0000000000adfc10 0000000000000008 B glad_glColorMaski
0000000000adfc18 0000000000000008 B glad_glColorMask
0000000000adfc20 0000000000000008 B glad_glClientWaitSync
0000000000adfc28 0000000000000008 B glad_glClearNamedFramebufferuiv
0000000000adfc30 0000000000000008 B glad_glClearNamedFramebufferiv
0000000000adfc38 0000000000000008 B glad_glClearNamedFramebufferfv
0000000000adfc40 0000000000000008 B glad_glClearNamedFramebufferfi
0000000000adfc48 0000000000000008 B glad_glClearNamedBufferSubData
0000000000adfc50 0000000000000008 B glad_glClearNamedBufferData
0000000000adfc58 0000000000000008 B glad_glClearDepthf
0000000000adfc60 0000000000000008 B glad_glClearDepth
0000000000adfc68 0000000000000008 B glad_glClearColor
0000000000adfc70 0000000000000008 B glad_glClearBufferuiv
0000000000adfc90 0000000000000008 B glad_glClearBufferSubData
0000000000adfc78 0000000000000008 B glad_glClearBufferiv
0000000000adfc80 0000000000000008 B glad_glClearBufferfv
0000000000adfc88 0000000000000008 B glad_glClearBufferfi
0000000000adfc98 0000000000000008 B glad_glClearBufferData
0000000000adfca0 0000000000000008 B glad_glClear
0000000000adfca8 0000000000000008 B glad_glClampColor
0000000000adfcb0 0000000000000008 B glad_glCheckNamedFramebufferStatus
0000000000adfcb8 0000000000000008 B glad_glCheckFramebufferStatus
0000000000adfcc0 0000000000000008 B glad_glBufferSubData
0000000000adfcc8 0000000000000008 B glad_glBufferStorage
0000000000adfcd0 0000000000000008 B glad_glBufferData
0000000000adfcd8 0000000000000008 B glad_glBlitNamedFramebuffer
0000000000adfce0 0000000000000008 B glad_glBlitFramebuffer
0000000000adfcf0 0000000000000008 B glad_glBlendFuncSeparatei
0000000000adfcf8 0000000000000008 B glad_glBlendFuncSeparate
0000000000adfce8 0000000000000008 B glad_glBlendFunci
0000000000adfd00 0000000000000008 B glad_glBlendFunc
0000000000adfd10 0000000000000008 B glad_glBlendEquationSeparatei
0000000000adfd18 0000000000000008 B glad_glBlendEquationSeparate
0000000000adfd08 0000000000000008 B glad_glBlendEquationi
0000000000adfd20 0000000000000008 B glad_glBlendEquation
0000000000adfd28 0000000000000008 B glad_glBlendColor
0000000000adfd30 0000000000000008 B glad_glBindVertexBuffer
0000000000adfd38 0000000000000008 B glad_glBindVertexArray
0000000000adfd40 0000000000000008 B glad_glBindTransformFeedback
0000000000adfd48 0000000000000008 B glad_glBindTextureUnit
0000000000adfd50 0000000000000008 B glad_glBindTexture
0000000000adfd58 0000000000000008 B glad_glBindSampler
0000000000adfd60 0000000000000008 B glad_glBindRenderbuffer
0000000000adfd68 0000000000000008 B glad_glBindProgramPipeline
0000000000adfd70 0000000000000008 B glad_glBindImageTexture
0000000000adfd78 0000000000000008 B glad_glBindFramebuffer
0000000000adfd80 0000000000000008 B glad_glBindFragDataLocationIndexed
0000000000adfd88 0000000000000008 B glad_glBindFragDataLocation
0000000000adfd90 0000000000000008 B glad_glBindBufferRange
0000000000adfd98 0000000000000008 B glad_glBindBufferBase
0000000000adfda0 0000000000000008 B glad_glBindBuffer
0000000000adfda8 0000000000000008 B glad_glBindAttribLocation
0000000000adfdb0 0000000000000008 B glad_glBeginTransformFeedback
0000000000adfdb8 0000000000000008 B glad_glBeginQueryIndexed
0000000000adfdc0 0000000000000008 B glad_glBeginQuery
0000000000adfdc8 0000000000000008 B glad_glBeginConditionalRender
0000000000adfdd0 0000000000000008 B glad_glAttachShader
0000000000adfdd8 0000000000000008 B glad_glActiveTexture
0000000000adfde0 0000000000000008 B glad_glActiveShaderProgram
000000000189e438 0000000000000008 B game_start_time
0000000000314a00 0000000000000008 D fragmentShaderTraditional
000000000030da60 0000000000000008 D fractionalScaleListener
0000000000ad3e48 0000000000000008 B entities
000000000189e420 0000000000000008 B current_time
000000000189e418 0000000000000008 B cpuTime
0000000000adea50 0000000000000008 B console_log_file
0000000000549b38 0000000000000008 B audiologSubjects
0000000000549b28 0000000000000008 B audioLogSpeech2Text
0000000000549b30 0000000000000008 B audiologSenders
0000000000549b40 0000000000000008 B audiologNames
0000000000adea58 0000000000000008 B activeLogFile
00000000018c4630 0000000000000004 B _ZNSt8ios_base4Init11_S_refcountE
00000000018c2c68 0000000000000004 B _ZNSt6locale7_S_onceE
00000000018c2c5c 0000000000000004 B _ZNSt6locale5facet7_S_onceE
00000000018c2c58 0000000000000004 B _ZNSt6locale2id11_S_refcountE
00000000003149c0 0000000000000004 D worstFPS
000000000033c940 0000000000000004 B worldMin_z
000000000033c944 0000000000000004 B worldMin_x
000000000031eea4 0000000000000004 B wav_count
0000000000549b20 0000000000000004 B voxelMinCenterZ
0000000000549b24 0000000000000004 B voxelMinCenterX
000000000143ef04 0000000000000004 B voxelLightListsRawID
000000000143ef08 0000000000000004 B voxelLightListIndicesID
00000000014421f0 0000000000000004 B verticesRenderedThisFrame
00000000014421f8 0000000000000004 B uiImageDrawCallsRenderedThisFrame
000000000143f360 0000000000000004 B uiImageCount
000000000069f0e8 0000000000000004 B totalPixels
000000000069f0e4 0000000000000004 B totalPaletteColors
000000000069f0f8 0000000000000004 B textureSizesID
000000000069f0f0 0000000000000004 B texturePalettesID
000000000069f0ec 0000000000000004 B texturePaletteOffsetsID
000000000069f0f4 0000000000000004 B textureOffsetsID
00000000014421fc 0000000000000004 B textDrawCallsRenderedThisFrame
00000000003148c0 0000000000000004 D statusTextLengthWithoutNullTerminator
000000000143ef10 0000000000000004 B statusTextDecayFinished
000000000143ef00 0000000000000004 B shadowMapsIndirectionID
00000000014421f4 0000000000000004 B shadowDrawCallsRenderedThisFrame
0000000000314200 0000000000000004 D random_range_rng
0000000000b1a7e0 0000000000000004 B numShadowsCouldRender
000000000032d8ec 0000000000000004 B numPackedGlyphsStopD
000000000032d8f0 0000000000000004 B numPackedGlyphs
0000000000313ae0 0000000000000004 D numFontRanges
00000000003149c8 0000000000000004 D mouse_sensitivity
000000000032d8f4 0000000000000004 B mmap_cleanup_count
0000000001442440 0000000000000004 B matricesBuffer
000000000143ef0c 0000000000000004 B lightsID
000000000189e40c 0000000000000004 B lastFrameSecCount
000000000189e430 0000000000000004 B globalFrameNum
0000000000adfe04 0000000000000004 B GLAD_GL_VERSION_4_3
0000000000adfe08 0000000000000004 B GLAD_GL_VERSION_4_2
0000000000adfe0c 0000000000000004 B GLAD_GL_VERSION_4_1
0000000000adfe10 0000000000000004 B GLAD_GL_VERSION_4_0
0000000000adfe14 0000000000000004 B GLAD_GL_VERSION_3_3
0000000000adfe18 0000000000000004 B GLAD_GL_VERSION_3_2
0000000000adfe1c 0000000000000004 B GLAD_GL_VERSION_3_1
0000000000adfe20 0000000000000004 B GLAD_GL_VERSION_3_0
0000000000adfe24 0000000000000004 B GLAD_GL_VERSION_2_1
0000000000adfe28 0000000000000004 B GLAD_GL_VERSION_2_0
0000000000adfe2c 0000000000000004 B GLAD_GL_VERSION_1_5
0000000000adfe30 0000000000000004 B GLAD_GL_VERSION_1_4
0000000000adfe34 0000000000000004 B GLAD_GL_VERSION_1_3
0000000000adfe38 0000000000000004 B GLAD_GL_VERSION_1_2
0000000000adfe3c 0000000000000004 B GLAD_GL_VERSION_1_1
0000000000adfe40 0000000000000004 B GLAD_GL_VERSION_1_0
0000000000adfde8 0000000000000004 B GLAD_GL_ARB_texture_view
0000000000adfdec 0000000000000004 B GLAD_GL_ARB_texture_storage
0000000000adfdf0 0000000000000004 B GLAD_GL_ARB_shader_storage_buffer_object
0000000000adfdf4 0000000000000004 B GLAD_GL_ARB_map_buffer_range
0000000000adfdf8 0000000000000004 B GLAD_GL_ARB_direct_state_access
0000000000adfdfc 0000000000000004 B GLAD_GL_ARB_copy_buffer
0000000000adfe00 0000000000000004 B GLAD_GL_ARB_buffer_storage
0000000000313b70 0000000000000004 D genericTextWidthFacStopD
0000000000313b74 0000000000000004 D genericTextHeightFacStopD
0000000000313b78 0000000000000004 D genericTextHeightFac
000000000189e408 0000000000000004 B framesPerLastSecond
000000000032d8e4 0000000000000004 B fontAtlasTexStopD
000000000032d8e8 0000000000000004 B fontAtlasTex
000000000143fadc 0000000000000004 B fogColorR
000000000143fad8 0000000000000004 B fogColorG
000000000143fad4 0000000000000004 B fogColorB
000000000143fad0 0000000000000004 B fogBaseDensityForLevel
0000000000324ba4 0000000000000004 B fixedNumberAdvanceWidthStopD
0000000000324ba8 0000000000000004 B fixedNumberAdvanceWidth
0000000000adfec0 0000000000000004 B fatigue
0000000000ad3e54 0000000000000004 B eventQueueEnd
0000000000ad3e5c 0000000000000004 B eventJournalIndex
0000000000ad3e58 0000000000000004 B eventIndex
0000000000ad3e40 0000000000000004 B entityCount
0000000001442200 0000000000000004 B drawCallsRenderedThisFrame
00000000014422a4 0000000000000004 B debugView
00000000014422a0 0000000000000004 B debugValue
0000000000314970 0000000000000004 D cursorPosition_y
0000000000314974 0000000000000004 D cursorPosition_x
0000000000adfed8 0000000000000004 B currentMonitorIndex
0000000000ad3dd8 0000000000000004 B correctionZ
0000000000ad3ddc 0000000000000004 B correctionY
0000000000ad3de0 0000000000000004 B correctionX
0000000000ad3d90 0000000000000004 B correctionStaticSaveableZ
0000000000ad3d94 0000000000000004 B correctionStaticSaveableY
0000000000ad3d98 0000000000000004 B correctionStaticSaveableX
0000000000ad3d9c 0000000000000004 B correctionStaticImmutableZ
0000000000ad3da0 0000000000000004 B correctionStaticImmutableY
0000000000ad3da4 0000000000000004 B correctionStaticImmutableX
0000000000ad3dcc 0000000000000004 B correctionNPCZ
0000000000ad3dd0 0000000000000004 B correctionNPCY
0000000000ad3dd4 0000000000000004 B correctionNPCX
0000000000ad3d84 0000000000000004 B correctionLightZ
0000000000ad3d88 0000000000000004 B correctionLightY
0000000000ad3d8c 0000000000000004 B correctionLightX
0000000000ad3da8 0000000000000004 B correctionLightsSaveableZ
0000000000ad3dac 0000000000000004 B correctionLightsSaveableY
0000000000ad3db0 0000000000000004 B correctionLightsSaveableX
0000000000ad3db4 0000000000000004 B correctionDynamicsZ
0000000000ad3db8 0000000000000004 B correctionDynamicsY
0000000000ad3dbc 0000000000000004 B correctionDynamicsX
0000000000ad3dc0 0000000000000004 B correctionDoorsZ
0000000000ad3dc4 0000000000000004 B correctionDoorsY
0000000000ad3dc8 0000000000000004 B correctionDoorsX
000000000069f0fc 0000000000000004 B colorBufferID
0000000000314990 0000000000000004 D cam_yaw
00000000014422c8 0000000000000004 B cam_roll
00000000014422b0 0000000000000004 B cam_rightz
00000000014422b4 0000000000000004 B cam_righty
00000000014422b8 0000000000000004 B cam_rightx
00000000014422cc 0000000000000004 B cam_pitch
00000000014422bc 0000000000000004 B cam_forwardz
00000000014422c0 0000000000000004 B cam_forwardy
00000000014422c4 0000000000000004 B cam_forwardx
00000000014422a8 0000000000000004 B berserkSeedTime
00000000014422ac 0000000000000004 B berserkFinished
0000000000314978 0000000000000004 D aspect3D
0000000000ad3de4 0000000000000002 B transparentInstancesHead
00000000003141e0 0000000000000002 D startOfTransparentInstances
00000000003141e2 0000000000000002 D startOfDoubleSidedInstances
00000000003149c6 0000000000000002 D screen_width
00000000003149c4 0000000000000002 D screen_height
0000000000ad3dea 0000000000000002 B renderableCount
000000000033c94a 0000000000000002 B playerCellIdx_z
000000000033c94c 0000000000000002 B playerCellIdx_y
000000000033c94e 0000000000000002 B playerCellIdx_x
000000000033c950 0000000000000002 B playerCellIdx
0000000000ad3dec 0000000000000002 B opaqueInstancesHead
000000000033c948 0000000000000002 B numCellsVisible
000000000069f0e0 0000000000000002 B loadedTextures
0000000000331900 0000000000000002 B loadedModels
0000000000ad3d80 0000000000000002 B loadedLights
0000000000ad3de8 0000000000000002 B loadedInstances
000000000031eea0 0000000000000002 B loadedAmbients
0000000000b1a7e4 0000000000000002 B largestNearbyMeshCount
0000000000ad3e20 0000000000000002 B invalidModelIndexCount
0000000000ad3de6 0000000000000002 B doubleSidedInstancesHead
00000000003169b8 0000000000000001 D _ZNSt8ios_base4Init20_S_synced_with_stdioE
000000000189e450 0000000000000001 B window_has_focus
0000000000adea41 0000000000000001 B usingManualLog
0000000000314994 0000000000000001 D startLevel
0000000000314995 0000000000000001 D numLevels
0000000001442321 0000000000000001 B menuActive
0000000000adea40 0000000000000001 B log_playback
0000000001442320 0000000000000001 B levelCurrentlyLoading
00000000003141e4 0000000000000001 D journalFirstWrite
000000000189e440 0000000000000001 B inventoryMode
0000000000adfed4 0000000000000001 B ignore_next_mouse_delta
0000000001442324 0000000000000001 B global_modIsCitadel
0000000001442322 0000000000000001 B gamePaused
0000000001442323 0000000000000001 B currentLevel
0000000000adfe44 0000000000000001 B boosterActive
```

```
❯ cloc --by-file --exclude-dir=temp_build,.git,Audio,Data,Fonts,Models,Screenshots,Scripts,Textures,Tools,External --not-match-f='\.md$|\.csv$|\.diff$|\.sh$|\.yml|\.cginc$' ./
      41 text files.
      41 unique files.                              
       8 files ignored.

github.com/AlDanial/cloc v 1.90  T=0.02 s (1582.8 files/s, 392392.6 lines/s)
--------------------------------------------------------------------------------
File                                         blank        comment           code
--------------------------------------------------------------------------------
./voxen.c                                      117             19           1031
./entity.c                                      41              6            575
./dynamic_culling.c                             90             29            564
./voxen.h                                       33             48            516
./console.c                                     45              4            421
./Shaders/composite_frag.glsl                   38             31            416
./data_fonts.c                                  49              8            375
./Shaders/chunk_frag.glsl                       28              5            301
./citadel_enumerations.h                        28             28            297
./data_models.c                                 36              2            286
./todo.c                                        34             33            272
./data_text.c                                   23              1            271
./data_parser.c                                 34              7            252
./helpers.c                                     29             27            203
./event.c                                       36             16            194
./input.c                                       28              7            175
./physics.c                                     35             52            175
./audio.c                                       28              7            153
./data_textures.c                               16              1            143
./os.c                                          18              1            112
./entity.h                                       4              0             85
./matvecquat.c                                   6              2             53
./Shaders/shadowmap_frag.glsl                    7              3             51
./matvecquat.h                                   4              1             51
./Shaders/text_frag.glsl                         5              6             37
./vmath.h                                        2              0             37
./Shaders/chunk_vert.glsl                        5              2             26
./todo.h                                         0              0             20
./Shaders/shadowmap_vert.glsl                    4              5             16
./patches.c                                      2             91             15
./os.h                                           0              0             10
./Shaders/text_vert.glsl                         2              1              9
./Shaders/composite_vert.glsl                    2              3              8
./hardware.c                                     1              1              2
--------------------------------------------------------------------------------
SUM:                                           830            447           7152
--------------------------------------------------------------------------------
```

❯ wc -l *.[ch] 2>/dev/null | sort -nr | head -n 10
  8509 total
  1295 voxen.c
   788 input.c
   751 types.h
   672 level.c
   647 dynamic_culling.c
   640 voxen.h
   479 console.c
   335 todo.c
   317 data_par


Main Rig.  Settings:
#define SSR_RES 2 // Ratio is (1 / SSR_RES) * render resolution.
VoxenSettings voxen_Settings = {
    .ScreenWidth = 1366u,
    .ScreenHeight = 768u,
    .Shadows = 1u,
    .AntiAliasing = 1u, // Default 1
    .Brightness = 100u, // Default 100 (for %)
    .VolumeMusic = 20u,
    .Language = 0, // English default
    .FOV = 65.0f,
    .Reflections = 1u, // Default 1
    .Vsync = false
};

FPS: 270
ms: 3.71
RAM: 112mb
VRAM: 140mb
Build 347ms
Init 0.608443secs

Performance canvas:
Shaodowmapping: 1.265ms of total frametime
Reflections: 0.98ms of total frametime
All else: 1.465ms (Antialiasing only like 0.03ms)
TOTAL: 3.71ms

CPU: 0.99ms
GPU: 3.71ms
