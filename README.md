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

Using on GLFW and OpenGL 4.3+, this engine attempts to leverage low latency and
GPU driven rendering methods with minimal state changes and maximum flexibility
with user customizable entities.  Heavy use of SSBOs is made though this is still
compatible with old hardware and GL drivers from 15yrs ago; further very few GL
extensions are used to further widen compatibility.  Careful handling of CPU to GPU
transfers is made to minimize VRAM and to prevent naughty GL drivers duplicating
that VRAM into the CPU RAM space which is also kept minimal.

All texture and model data is loaded from disk directly for ease of development
and full mod support by design.  Any intermediate format is internal to the engine.

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
Initializes various core systems (OpenGL+Window)
Loads data resources (textures, models, etc.)
Loads scripting VM
Parses all game/mod scripts
Initializes data handling systems and parsers using all above data
Level Load using gamedata definition to pick starting level
Starts game loop:
  Polls GLFW input
  Processes input and applies movement key states, mouselook
  Animation (done prior to physics such that physics can respond properly)
  Physics
  Game Logic Update Loop
  Render Shadowmaps
  Render Opaques + Doublesided
  Render Transparents
  Render UI
Exit with zero cleanup, let the OS handle it; does immediate fastest exit as user's time is important.

---

### Systems:

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

#### VXGI Lighting (TODO, lol)

Voxen wouldn't be called Voxen without Voxels.  The world is
overlayed with a sparse voxel representation for storing and
updating lighting information such as Global Illumination (GI)
and Shadows which include Ambient Occlusion.  This is
calculated on a separate thread then passed to GPU for actually
applying lighting/shadows. TODO

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
Compiling voxen, total iterations today 97 (2026-02-03)...
Build completed in 440 ms
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
Window moved to monitor 0: DVI-D-0 at x: 2717, y: 531
OpenGL Version: 4.3.0 NVIDIA 550.144.03, GPU: NVIDIA GeForce GTX 1070/PCIe/SSE2CPU: AMD Ryzen 5 5500 | Cores: 12
Loaded    5 fonts...in 0.035 s
Loading game definition... Citadel:: num levels: 14, start level: 1... took 0.000050 secs
Screen size updated to 1366 x 768 from input values 1366 x 768
Applied configuration settings
GL buffers, FBO, fonts, audio, localization, and window init took 0.446896 secs
Loading  768 entities... took 0.003628 secs
Loaded 5935 entities, 949 static lights, 34 doors for Level 1... took 0.070558 secs
Loading   models( 5988/5988) with max index  5987 ... total vertices: 12872615, total tris: 148688016, took 0.460439 secs
Loading textures( 1310/1310), using stb_image version: 2.28, total palette colors: 76033, total pixels: 24595125... took 0.401226 secs
Culling...found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341... took 0.134402 secs
LoadLevel completed!
Game Initialized in 1.529632 secs
```

Log output from one run with DEBUG_RAM_OUTPUT declared in voxen.h:

```
Compiling voxen, total iterations today 98 (2026-02-03)...
Build completed in 422 ms
Memory at program start: Heap usage 83616 bytes (81 KB | 0.08 MB), USS 4345856 bytes (4244 KB | 4.14 MB)
Memory at prior to event system init: Heap usage 89248 bytes (87 KB | 0.09 MB), USS 4354048 bytes (4252 KB | 4.15 MB)
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
Window moved to monitor 0: DVI-D-0 at x: 2717, y: 531
OpenGL Version: 4.3.0 NVIDIA 550.144.03, GPU: NVIDIA GeForce GTX 1070/PCIe/SSE2CPU: AMD Ryzen 5 5500 | Cores: 12
Loaded    5 fonts...in 0.038 s
Loading game definition... Citadel:: num levels: 14, start level: 1... took 0.000052 secs
Screen size updated to 1366 x 768 from input values 1366 x 768
Applied configuration settings
Memory at after freeing window bar icon: Heap usage 9549856 bytes (9326 KB | 9.11 MB), USS 51523584 bytes (50316 KB | 49.14 MB)
GL buffers, FBO, fonts, audio, localization, and window init took 0.461477 secs
Loading  768 entities... took 0.003600 secs
Memory at after loading all entities: Heap usage 9550336 bytes (9326 KB | 9.11 MB), USS 52887552 bytes (51648 KB | 50.44 MB)
Memory at start of LoadLevel: Heap usage 11801408 bytes (11524 KB | 11.25 MB), USS 73465856 bytes (71744 KB | 70.06 MB)
Loaded 5935 entities, 949 static lights, 34 doors for Level 1... took 0.070774 secs
Memory at end of LoadLevel instances: Heap usage 11801264 bytes (11524 KB | 11.25 MB), USS 74633216 bytes (72884 KB | 71.18 MB)
Memory at start of LoadModels: Heap usage 11801616 bytes (11525 KB | 11.25 MB), USS 74633216 bytes (72884 KB | 71.18 MB)
Loading   models( 5988/5988) with max index  5987 ...Memory at after main OS_AllocateRAM block: Heap usage 11801616 bytes (11525 KB | 11.25 MB), USS 85323776 bytes (83324 KB | 81.37 MB)
Memory at prior to model load loop: Heap usage 11802256 bytes (11525 KB | 11.26 MB), USS 85409792 bytes (83408 KB | 81.45 MB)
Memory at after model load loop: Heap usage 11802256 bytes (11525 KB | 11.26 MB), USS 606507008 bytes (592292 KB | 578.41 MB)
Memory at after to model to gpu transfer: Heap usage 583969440 bytes (570282 KB | 556.92 MB), USS 719843328 bytes (702972 KB | 686.50 MB)
 total vertices: 12872615, total tris: 148688016, took 0.419001 secs
Memory at After Load Models: Heap usage 583969440 bytes (570282 KB | 556.92 MB), USS 709156864 bytes (692536 KB | 676.30 MB)
Memory at start of LoadTextures: Heap usage 583970128 bytes (570283 KB | 556.92 MB), USS 709173248 bytes (692552 KB | 676.32 MB)
Loading textures( 1310/1310), using stb_image version: 2.28, Memory at After loop for load textures: Heap usage 583970128 bytes (570283 KB | 556.92 MB), USS 744620032 bytes (727168 KB | 710.12 MB)
total palette colors: 76033, total pixels: 24595125... took 0.426592 secs
Memory at After LoadTextures and after deallocation of LoadTextures arena and stbi arena: Heap usage 583952320 bytes (570265 KB | 556.90 MB), USS 734101504 bytes (716896 KB | 700.09 MB)
Culling...Memory at start of Cull_Init: Heap usage 583952896 bytes (570266 KB | 556.90 MB), USS 734138368 bytes (716932 KB | 700.13 MB)
found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341...Memory at end of dynamic culling DetermineClosedEdges: Heap usage 583952896 bytes (570266 KB | 556.90 MB), USS 734175232 bytes (716968 KB | 700.16 MB)
 took 0.159101 secs
Memory at end of Cull_Init: Heap usage 583953120 bytes (570266 KB | 556.90 MB), USS 736296960 bytes (719040 KB | 702.19 MB)
LoadLevel completed!
Memory at InitializeEnvironment end: Heap usage 583963088 bytes (570276 KB | 556.91 MB), USS 736342016 bytes (719084 KB | 702.23 MB)
Memory at prior to game loop: Heap usage 583963088 bytes (570276 KB | 556.91 MB), USS 736342016 bytes (719084 KB | 702.23 MB)
Game Initialized in 1.621018 secs
Memory at after 4 frames of running: Heap usage 583783056 bytes (570100 KB | 556.74 MB), USS 741511168 bytes (724132 KB | 707.16 MB)
Memory at after 100 frames of running: Heap usage 583797008 bytes (570114 KB | 556.75 MB), USS 743608320 bytes (726180 KB | 709.16 MB)
Memory at after 200 frames of running: Heap usage 583780736 bytes (570098 KB | 556.74 MB), USS 743608320 bytes (726180 KB | 709.16 MB)
Memory at after 500 frames of running: Heap usage 583805744 bytes (570122 KB | 556.76 MB), USS 743608320 bytes (726180 KB | 709.16 MB)
Memory at after 1000 frames of running: Heap usage 583793760 bytes (570111 KB | 556.75 MB), USS 743608320 bytes (726180 KB | 709.16 MB)
```

Heap impacts:

```
❯ grep -rIn  "alloc("

```
(NOTE: Excluded miniaudio, stb_truetype ./External alloc calls)


```
❯ grep -rIn  "OS_AllocateRAM("
data_fonts.c:214:    unsigned char *bmp = OS_AllocateRAM(NULL, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
os.h:35:    static inline void* OS_AllocateRAM(void* addr, size_t length, int prot, int flags, OsFileHandle fd) {
os.h:77:    static inline void* OS_AllocateRAM(void* addr, size_t length, int prot, int flags, OsFileHandle fd) {
os.h:155:        void* ramSpacePointer = OS_AllocateRAM(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor);
data_parser.c:103:    Entity *new_entries = OS_AllocateRAM(NULL, entry_count * sizeof(Entity), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);  
helpers.c:85:    unsigned char* pixels = OS_AllocateRAM(NULL, Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);//malloc(Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char));
External/stb_image.h:15:        stbi__arena_base = OS_AllocateRAM(NULL, STBI_ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
data_textures.c:45:    void* arena = OS_AllocateRAM(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
data_models.c:46:    uint8_t *buf = OS_AllocateRAM(NULL, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
data_models.c:80:    modelVertices[i]  = fromCache ? (float*)cached_verts : OS_AllocateRAM(NULL, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
data_models.c:81:    modelTriangles[i] =  fromCache ? (uint32_t*)cached_idx : OS_AllocateRAM(NULL, triCount * 3 * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
data_models.c:151:    modelVertices  = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(float*),    PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
data_models.c:152:    modelTriangles = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
data_models.c:155:    int32_t* indexToParser = OS_AllocateRAM(NULL, indexToParser_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INV
```

Binary static variable impacts:

```
❯ size ./voxen
   text    data     bss        dec            hex filename
1266448   30536 28902252  30199236        1cccdc4  ./voxen
```


Individual static variable impacts:

```
❯ nm -S --size-sort -r ./voxen | grep ' [BD] '
0000000000a23620 000000000116c000 B instances
00000000003737e0 0000000000200000 B precomputedVisibleCellsFromHere
00000000001a2600 0000000000154578 B Sys_Text
0000000000581520 000000000014e800 B entities
0000000001b8f800 0000000000105c68 B bioMonitor
000000000075ede0 00000000000e1000 B lightFrustumPlanes
0000000000983620 00000000000a0000 B modelMatrices
00000000002f6ea0 000000000007a120 B cullingFileBuffer
0000000000722f80 000000000002ee00 B lightIntervalSteps
00000000006f3b40 000000000002ee00 B intervalStepisLerping
0000000000164e00 000000000002e84c B modelBounds
0000000001cac440 0000000000023140 B _glfw
000000000014a4a0 000000000001a950 B mmap_cleanup
000000000096c420 0000000000014500 B lights
00000000006cfd40 0000000000014000 B visibleInstances
0000000001c95520 000000000000d544 B Sys_Render
00000000006e63a0 0000000000008000 B textVertexData
000000000019bb60 0000000000006a54 B modelVertexCounts
0000000000195100 0000000000006a54 B modelTriangleCounts
00000000006ee3a0 0000000000004b00 B lightsNewPosition
00000000001418a0 00000000000045e4 B fontPackedCharStopD
0000000000145ea0 00000000000045e4 B fontPackedChar
000000000057b7e0 0000000000004000 B gridCellStates
00000000005777e0 0000000000004000 B gridCellFloorHeight
00000000005737e0 0000000000004000 B gridCellCeilingHeight
0000000000370fc0 0000000000002800 B instanceIsLODArray
0000000000980e20 0000000000002800 B dirtyInstances
000000000013f6c0 0000000000001c00 B history
0000000000193660 0000000000001a95 B modelHasAnimation
000000000075d2c0 0000000000001a90 B voxen_Shadow_System
000000000075b9c0 0000000000001900 B lightMinIntensity
000000000075a0c0 0000000000001900 B lightMaxIntensity
0000000000756ec0 0000000000001900 B lightLerpValue
00000000007555c0 0000000000001900 B lightLerpTime
0000000000753cc0 0000000000001900 B lightLerpStepTime
00000000007523c0 0000000000001900 B lightLerpStartTime
00000000006e3d40 0000000000001000 B creditStats
000000000013d800 0000000000000c58 D _glfwDefaultMappings
00000000006e5340 0000000000000c00 B shadows_nearMeshes
0000000000580b80 0000000000000998 B Sys_Input
000000000013c920 0000000000000950 D inputElements
0000000001cab400 0000000000000800 B transparentTexture
0000000001cabc00 0000000000000800 B doubleSidedTexture
0000000000137a60 0000000000000690 D configTable
0000000000759a80 0000000000000640 B lightOn
0000000000758e00 0000000000000640 B lightLerpUp
0000000000759440 0000000000000640 B lightLerpOn
0000000000751d80 0000000000000640 B lightIntervalStepsLength
0000000000722940 0000000000000640 B lightIntervalStepIsLerpingLength
00000000006f2ea0 0000000000000640 B lightInPVS
000000000096bde0 0000000000000640 B lightDirty
00000000007587c0 0000000000000640 B lightCurrentStep
00000000006f3500 0000000000000640 B lightCastsShadows
00000000006e4d40 0000000000000600 B shadows_nearMeshRadii
0000000001caaee0 0000000000000520 B audio_engine
0000000000980960 0000000000000400 B uiTextBuffer
00000000006e5fa0 0000000000000400 B statusText
000000000013c480 0000000000000400 D consoleEntryText
00000000002f6ba0 0000000000000300 B activePortals
00000000001412e0 0000000000000228 B fallbackFonts
000000000013d520 00000000000001d8 D Sys_Global
0000000001b8f620 00000000000001d8 B inventoryPlayer1
0000000001caadc0 0000000000000100 B ambientRegistry
000000000013d280 00000000000000d0 D Sys_Settings
000000000013d700 00000000000000b0 D creditPages
000000000013d360 0000000000000090 D textColors
0000000001c95480 0000000000000088 B Sys_UI
000000000013d4a0 0000000000000078 D Sys_Dx
000000000075ed60 0000000000000060 B playerFrustumPlanes
00000000006e5f40 0000000000000060 B debugLineBuffer
000000000013d400 0000000000000060 D cubemapOrientationQuaternion
0000000000980920 0000000000000040 B uiOrthoProjection
0000000000980d60 0000000000000040 B shadowmapsPerspectiveProjection
0000000000980da0 0000000000000040 B rasterPerspectiveProjection
000000000013c8a0 0000000000000030 D fontRangesStopD
000000000013c8e0 0000000000000030 D fontRanges
000000000013ae60 0000000000000030 D dataDeviceListener
000000000013a700 0000000000000028 D wp_fractional_scale_manager_v1_interface
000000000013a220 0000000000000028 D _glfw_zxdg_toplevel_decoration_v1_interface
000000000013a160 0000000000000028 D _glfw_zxdg_decoration_manager_v1_interface
000000000013a440 0000000000000028 D _glfw_zwp_relative_pointer_v1_interface
000000000013a3c0 0000000000000028 D _glfw_zwp_relative_pointer_manager_v1_interface
000000000013a4e0 0000000000000028 D _glfw_zwp_pointer_constraints_v1_interface
000000000013a5c0 0000000000000028 D _glfw_zwp_locked_pointer_v1_interface
000000000013a9e0 0000000000000028 D _glfw_zwp_idle_inhibitor_v1_interface
000000000013a980 0000000000000028 D _glfw_zwp_idle_inhibit_manager_v1_interface
000000000013a680 0000000000000028 D _glfw_zwp_confined_pointer_v1_interface
0000000000139bc0 0000000000000028 D _glfw_xdg_wm_base_interface
0000000000139fe0 0000000000000028 D _glfw_xdg_toplevel_interface
0000000000139de0 0000000000000028 D _glfw_xdg_surface_interface
0000000000139d00 0000000000000028 D _glfw_xdg_positioner_interface
000000000013a0e0 0000000000000028 D _glfw_xdg_popup_interface
000000000013a820 0000000000000028 D _glfw_xdg_activation_v1_interface
000000000013a900 0000000000000028 D _glfw_xdg_activation_token_v1_interface
000000000013a340 0000000000000028 D _glfw_wp_viewport_interface
000000000013a2a0 0000000000000028 D _glfw_wp_viewporter_interface
000000000013a780 0000000000000028 D _glfw_wp_fractional_scale_v1_interface
0000000000139800 0000000000000028 D _glfw_wl_touch_interface
0000000000139360 0000000000000028 D _glfw_wl_surface_interface
0000000000139b00 0000000000000028 D _glfw_wl_subsurface_interface
0000000000139a20 0000000000000028 D _glfw_wl_subcompositor_interface
0000000000138a80 0000000000000028 D _glfw_wl_shm_pool_interface
0000000000138b00 0000000000000028 D _glfw_wl_shm_interface
00000000001391a0 0000000000000028 D _glfw_wl_shell_surface_interface
0000000000139000 0000000000000028 D _glfw_wl_shell_interface
0000000000139440 0000000000000028 D _glfw_wl_seat_interface
0000000000138900 0000000000000028 D _glfw_wl_registry_interface
00000000001399a0 0000000000000028 D _glfw_wl_region_interface
00000000001395e0 0000000000000028 D _glfw_wl_pointer_interface
0000000000139900 0000000000000028 D _glfw_wl_output_interface
00000000001396e0 0000000000000028 D _glfw_wl_keyboard_interface
0000000000138860 0000000000000028 D _glfw_wl_display_interface
0000000000138de0 0000000000000028 D _glfw_wl_data_source_interface
0000000000138ca0 0000000000000028 D _glfw_wl_data_offer_interface
0000000000138fa0 0000000000000028 D _glfw_wl_data_device_manager_interface
0000000000138f20 0000000000000028 D _glfw_wl_data_device_interface
00000000001389e0 0000000000000028 D _glfw_wl_compositor_interface
0000000000138960 0000000000000028 D _glfw_wl_callback_interface
0000000000138b80 0000000000000028 D _glfw_wl_buffer_interface
000000000013d478 000000000000000d D Sys_Cheats
00000000002f6b78 000000000000000a B uncullingCameras
0000000001cac410 0000000000000008 B stbi__arena_end
0000000001cac418 0000000000000008 B stbi__arena_cursor
0000000001cac420 0000000000000008 B stbi__arena_base
0000000000164df0 0000000000000008 B props
00000000006cfd20 0000000000000008 B monitorSwitchTime
00000000001a25c0 0000000000000008 B modelVertices
00000000001a25b8 0000000000000008 B modelTriangles
000000000057f7e0 0000000000000008 B glad_glWaitSync
000000000057f7e8 0000000000000008 B glad_glViewportIndexedfv
000000000057f7f0 0000000000000008 B glad_glViewportIndexedf
000000000057f7f8 0000000000000008 B glad_glViewportArrayv
000000000057f800 0000000000000008 B glad_glViewport
000000000057f808 0000000000000008 B glad_glVertexBindingDivisor
000000000057f810 0000000000000008 B glad_glVertexAttribPointer
000000000057f818 0000000000000008 B glad_glVertexAttribP4uiv
000000000057f820 0000000000000008 B glad_glVertexAttribP4ui
000000000057f828 0000000000000008 B glad_glVertexAttribP3uiv
000000000057f830 0000000000000008 B glad_glVertexAttribP3ui
000000000057f838 0000000000000008 B glad_glVertexAttribP2uiv
000000000057f840 0000000000000008 B glad_glVertexAttribP2ui
000000000057f848 0000000000000008 B glad_glVertexAttribP1uiv
000000000057f850 0000000000000008 B glad_glVertexAttribP1ui
000000000057f858 0000000000000008 B glad_glVertexAttribLPointer
000000000057f860 0000000000000008 B glad_glVertexAttribLFormat
000000000057f868 0000000000000008 B glad_glVertexAttribL4dv
000000000057f870 0000000000000008 B glad_glVertexAttribL4d
000000000057f878 0000000000000008 B glad_glVertexAttribL3dv
000000000057f880 0000000000000008 B glad_glVertexAttribL3d
000000000057f888 0000000000000008 B glad_glVertexAttribL2dv
000000000057f890 0000000000000008 B glad_glVertexAttribL2d
000000000057f898 0000000000000008 B glad_glVertexAttribL1dv
000000000057f8a0 0000000000000008 B glad_glVertexAttribL1d
000000000057f8a8 0000000000000008 B glad_glVertexAttribIPointer
000000000057f8b0 0000000000000008 B glad_glVertexAttribIFormat
000000000057f8b8 0000000000000008 B glad_glVertexAttribI4usv
000000000057f8c0 0000000000000008 B glad_glVertexAttribI4uiv
000000000057f8c8 0000000000000008 B glad_glVertexAttribI4ui
000000000057f8d0 0000000000000008 B glad_glVertexAttribI4ubv
000000000057f8d8 0000000000000008 B glad_glVertexAttribI4sv
000000000057f8e0 0000000000000008 B glad_glVertexAttribI4iv
000000000057f8e8 0000000000000008 B glad_glVertexAttribI4i
000000000057f8f0 0000000000000008 B glad_glVertexAttribI4bv
000000000057f8f8 0000000000000008 B glad_glVertexAttribI3uiv
000000000057f900 0000000000000008 B glad_glVertexAttribI3ui
000000000057f908 0000000000000008 B glad_glVertexAttribI3iv
000000000057f910 0000000000000008 B glad_glVertexAttribI3i
000000000057f918 0000000000000008 B glad_glVertexAttribI2uiv
000000000057f920 0000000000000008 B glad_glVertexAttribI2ui
000000000057f928 0000000000000008 B glad_glVertexAttribI2iv
000000000057f930 0000000000000008 B glad_glVertexAttribI2i
000000000057f938 0000000000000008 B glad_glVertexAttribI1uiv
000000000057f940 0000000000000008 B glad_glVertexAttribI1ui
000000000057f948 0000000000000008 B glad_glVertexAttribI1iv
000000000057f950 0000000000000008 B glad_glVertexAttribI1i
000000000057f958 0000000000000008 B glad_glVertexAttribFormat
000000000057f960 0000000000000008 B glad_glVertexAttribDivisor
000000000057f968 0000000000000008 B glad_glVertexAttribBinding
000000000057f970 0000000000000008 B glad_glVertexAttrib4usv
000000000057f978 0000000000000008 B glad_glVertexAttrib4uiv
000000000057f980 0000000000000008 B glad_glVertexAttrib4ubv
000000000057f988 0000000000000008 B glad_glVertexAttrib4sv
000000000057f990 0000000000000008 B glad_glVertexAttrib4s
000000000057f9c8 0000000000000008 B glad_glVertexAttrib4Nusv
000000000057f9d0 0000000000000008 B glad_glVertexAttrib4Nuiv
000000000057f9d8 0000000000000008 B glad_glVertexAttrib4Nubv
000000000057f9e0 0000000000000008 B glad_glVertexAttrib4Nub
000000000057f9e8 0000000000000008 B glad_glVertexAttrib4Nsv
000000000057f9f0 0000000000000008 B glad_glVertexAttrib4Niv
000000000057f9f8 0000000000000008 B glad_glVertexAttrib4Nbv
000000000057f998 0000000000000008 B glad_glVertexAttrib4iv
000000000057f9a0 0000000000000008 B glad_glVertexAttrib4fv
000000000057f9a8 0000000000000008 B glad_glVertexAttrib4f
000000000057f9b0 0000000000000008 B glad_glVertexAttrib4dv
000000000057f9b8 0000000000000008 B glad_glVertexAttrib4d
000000000057f9c0 0000000000000008 B glad_glVertexAttrib4bv
000000000057fa00 0000000000000008 B glad_glVertexAttrib3sv
000000000057fa08 0000000000000008 B glad_glVertexAttrib3s
000000000057fa10 0000000000000008 B glad_glVertexAttrib3fv
000000000057fa18 0000000000000008 B glad_glVertexAttrib3f
000000000057fa20 0000000000000008 B glad_glVertexAttrib3dv
000000000057fa28 0000000000000008 B glad_glVertexAttrib3d
000000000057fa30 0000000000000008 B glad_glVertexAttrib2sv
000000000057fa38 0000000000000008 B glad_glVertexAttrib2s
000000000057fa40 0000000000000008 B glad_glVertexAttrib2fv
000000000057fa48 0000000000000008 B glad_glVertexAttrib2f
000000000057fa50 0000000000000008 B glad_glVertexAttrib2dv
000000000057fa58 0000000000000008 B glad_glVertexAttrib2d
000000000057fa60 0000000000000008 B glad_glVertexAttrib1sv
000000000057fa68 0000000000000008 B glad_glVertexAttrib1s
000000000057fa70 0000000000000008 B glad_glVertexAttrib1fv
000000000057fa78 0000000000000008 B glad_glVertexAttrib1f
000000000057fa80 0000000000000008 B glad_glVertexAttrib1dv
000000000057fa88 0000000000000008 B glad_glVertexAttrib1d
000000000057fa90 0000000000000008 B glad_glVertexArrayVertexBuffers
000000000057fa98 0000000000000008 B glad_glVertexArrayVertexBuffer
000000000057faa0 0000000000000008 B glad_glVertexArrayElementBuffer
000000000057faa8 0000000000000008 B glad_glVertexArrayBindingDivisor
000000000057fab0 0000000000000008 B glad_glVertexArrayAttribLFormat
000000000057fab8 0000000000000008 B glad_glVertexArrayAttribIFormat
000000000057fac0 0000000000000008 B glad_glVertexArrayAttribFormat
000000000057fac8 0000000000000008 B glad_glVertexArrayAttribBinding
000000000057fad0 0000000000000008 B glad_glValidateProgramPipeline
000000000057fad8 0000000000000008 B glad_glValidateProgram
000000000057fae0 0000000000000008 B glad_glUseProgramStages
000000000057fae8 0000000000000008 B glad_glUseProgram
000000000057faf0 0000000000000008 B glad_glUnmapNamedBuffer
000000000057faf8 0000000000000008 B glad_glUnmapBuffer
000000000057fb00 0000000000000008 B glad_glUniformSubroutinesuiv
000000000057fb08 0000000000000008 B glad_glUniformMatrix4x3fv
000000000057fb10 0000000000000008 B glad_glUniformMatrix4x3dv
000000000057fb18 0000000000000008 B glad_glUniformMatrix4x2fv
000000000057fb20 0000000000000008 B glad_glUniformMatrix4x2dv
000000000057fb28 0000000000000008 B glad_glUniformMatrix4fv
000000000057fb30 0000000000000008 B glad_glUniformMatrix4dv
000000000057fb38 0000000000000008 B glad_glUniformMatrix3x4fv
000000000057fb40 0000000000000008 B glad_glUniformMatrix3x4dv
000000000057fb48 0000000000000008 B glad_glUniformMatrix3x2fv
000000000057fb50 0000000000000008 B glad_glUniformMatrix3x2dv
000000000057fb58 0000000000000008 B glad_glUniformMatrix3fv
000000000057fb60 0000000000000008 B glad_glUniformMatrix3dv
000000000057fb68 0000000000000008 B glad_glUniformMatrix2x4fv
000000000057fb70 0000000000000008 B glad_glUniformMatrix2x4dv
000000000057fb78 0000000000000008 B glad_glUniformMatrix2x3fv
000000000057fb80 0000000000000008 B glad_glUniformMatrix2x3dv
000000000057fb88 0000000000000008 B glad_glUniformMatrix2fv
000000000057fb90 0000000000000008 B glad_glUniformMatrix2dv
000000000057fb98 0000000000000008 B glad_glUniformBlockBinding
000000000057fba0 0000000000000008 B glad_glUniform4uiv
000000000057fba8 0000000000000008 B glad_glUniform4ui
000000000057fbb0 0000000000000008 B glad_glUniform4iv
000000000057fbb8 0000000000000008 B glad_glUniform4i
000000000057fbc0 0000000000000008 B glad_glUniform4fv
000000000057fbc8 0000000000000008 B glad_glUniform4f
000000000057fbd0 0000000000000008 B glad_glUniform4dv
000000000057fbd8 0000000000000008 B glad_glUniform4d
000000000057fbe0 0000000000000008 B glad_glUniform3uiv
000000000057fbe8 0000000000000008 B glad_glUniform3ui
000000000057fbf0 0000000000000008 B glad_glUniform3iv
000000000057fbf8 0000000000000008 B glad_glUniform3i
000000000057fc00 0000000000000008 B glad_glUniform3fv
000000000057fc08 0000000000000008 B glad_glUniform3f
000000000057fc10 0000000000000008 B glad_glUniform3dv
000000000057fc18 0000000000000008 B glad_glUniform3d
000000000057fc20 0000000000000008 B glad_glUniform2uiv
000000000057fc28 0000000000000008 B glad_glUniform2ui
000000000057fc30 0000000000000008 B glad_glUniform2iv
000000000057fc38 0000000000000008 B glad_glUniform2i
000000000057fc40 0000000000000008 B glad_glUniform2fv
000000000057fc48 0000000000000008 B glad_glUniform2f
000000000057fc50 0000000000000008 B glad_glUniform2dv
000000000057fc58 0000000000000008 B glad_glUniform2d
000000000057fc60 0000000000000008 B glad_glUniform1uiv
000000000057fc68 0000000000000008 B glad_glUniform1ui
000000000057fc70 0000000000000008 B glad_glUniform1iv
000000000057fc78 0000000000000008 B glad_glUniform1i
000000000057fc80 0000000000000008 B glad_glUniform1fv
000000000057fc88 0000000000000008 B glad_glUniform1f
000000000057fc90 0000000000000008 B glad_glUniform1dv
000000000057fc98 0000000000000008 B glad_glUniform1d
000000000057fca0 0000000000000008 B glad_glTransformFeedbackVaryings
000000000057fca8 0000000000000008 B glad_glTransformFeedbackBufferRange
000000000057fcb0 0000000000000008 B glad_glTransformFeedbackBufferBase
000000000057fcb8 0000000000000008 B glad_glTextureView
000000000057fcc0 0000000000000008 B glad_glTextureSubImage3D
000000000057fcc8 0000000000000008 B glad_glTextureSubImage2D
000000000057fcd0 0000000000000008 B glad_glTextureSubImage1D
000000000057fcd8 0000000000000008 B glad_glTextureStorage3DMultisample
000000000057fce0 0000000000000008 B glad_glTextureStorage3D
000000000057fce8 0000000000000008 B glad_glTextureStorage2DMultisample
000000000057fcf0 0000000000000008 B glad_glTextureStorage2D
000000000057fcf8 0000000000000008 B glad_glTextureStorage1D
000000000057fd00 0000000000000008 B glad_glTextureParameteriv
000000000057fd20 0000000000000008 B glad_glTextureParameterIuiv
000000000057fd28 0000000000000008 B glad_glTextureParameterIiv
000000000057fd08 0000000000000008 B glad_glTextureParameteri
000000000057fd10 0000000000000008 B glad_glTextureParameterfv
000000000057fd18 0000000000000008 B glad_glTextureParameterf
000000000057fd30 0000000000000008 B glad_glTextureBufferRange
000000000057fd38 0000000000000008 B glad_glTextureBuffer
000000000057fd40 0000000000000008 B glad_glTexSubImage3D
000000000057fd48 0000000000000008 B glad_glTexSubImage2D
000000000057fd50 0000000000000008 B glad_glTexSubImage1D
000000000057fd58 0000000000000008 B glad_glTexStorage3DMultisample
000000000057fd60 0000000000000008 B glad_glTexStorage3D
000000000057fd68 0000000000000008 B glad_glTexStorage2DMultisample
000000000057fd70 0000000000000008 B glad_glTexStorage2D
000000000057fd78 0000000000000008 B glad_glTexStorage1D
000000000057fd80 0000000000000008 B glad_glTexParameteriv
000000000057fda0 0000000000000008 B glad_glTexParameterIuiv
000000000057fda8 0000000000000008 B glad_glTexParameterIiv
000000000057fd88 0000000000000008 B glad_glTexParameteri
000000000057fd90 0000000000000008 B glad_glTexParameterfv
000000000057fd98 0000000000000008 B glad_glTexParameterf
000000000057fdb0 0000000000000008 B glad_glTexImage3DMultisample
000000000057fdb8 0000000000000008 B glad_glTexImage3D
000000000057fdc0 0000000000000008 B glad_glTexImage2DMultisample
000000000057fdc8 0000000000000008 B glad_glTexImage2D
000000000057fdd0 0000000000000008 B glad_glTexImage1D
000000000057fdd8 0000000000000008 B glad_glTexBufferRange
000000000057fde0 0000000000000008 B glad_glTexBuffer
000000000057fde8 0000000000000008 B glad_glStencilOpSeparate
000000000057fdf0 0000000000000008 B glad_glStencilOp
000000000057fdf8 0000000000000008 B glad_glStencilMaskSeparate
000000000057fe00 0000000000000008 B glad_glStencilMask
000000000057fe08 0000000000000008 B glad_glStencilFuncSeparate
000000000057fe10 0000000000000008 B glad_glStencilFunc
000000000057fe18 0000000000000008 B glad_glShaderStorageBlockBinding
000000000057fe20 0000000000000008 B glad_glShaderSource
000000000057fe28 0000000000000008 B glad_glShaderBinary
000000000057fe30 0000000000000008 B glad_glScissorIndexedv
000000000057fe38 0000000000000008 B glad_glScissorIndexed
000000000057fe40 0000000000000008 B glad_glScissorArrayv
000000000057fe48 0000000000000008 B glad_glScissor
000000000057fe50 0000000000000008 B glad_glSamplerParameteriv
000000000057fe70 0000000000000008 B glad_glSamplerParameterIuiv
000000000057fe78 0000000000000008 B glad_glSamplerParameterIiv
000000000057fe58 0000000000000008 B glad_glSamplerParameteri
000000000057fe60 0000000000000008 B glad_glSamplerParameterfv
000000000057fe68 0000000000000008 B glad_glSamplerParameterf
000000000057fe80 0000000000000008 B glad_glSampleMaski
000000000057fe88 0000000000000008 B glad_glResumeTransformFeedback
000000000057fe90 0000000000000008 B glad_glRenderbufferStorageMultisample
000000000057fe98 0000000000000008 B glad_glRenderbufferStorage
000000000057fea0 0000000000000008 B glad_glReleaseShaderCompiler
000000000057fea8 0000000000000008 B glad_glReadPixels
000000000057feb0 0000000000000008 B glad_glReadBuffer
000000000057feb8 0000000000000008 B glad_glQueryCounter
000000000057fec0 0000000000000008 B glad_glProvokingVertex
000000000057fec8 0000000000000008 B glad_glProgramUniformMatrix4x3fv
000000000057fed0 0000000000000008 B glad_glProgramUniformMatrix4x3dv
000000000057fed8 0000000000000008 B glad_glProgramUniformMatrix4x2fv
000000000057fee0 0000000000000008 B glad_glProgramUniformMatrix4x2dv
000000000057fee8 0000000000000008 B glad_glProgramUniformMatrix4fv
000000000057fef0 0000000000000008 B glad_glProgramUniformMatrix4dv
000000000057fef8 0000000000000008 B glad_glProgramUniformMatrix3x4fv
000000000057ff00 0000000000000008 B glad_glProgramUniformMatrix3x4dv
000000000057ff08 0000000000000008 B glad_glProgramUniformMatrix3x2fv
000000000057ff10 0000000000000008 B glad_glProgramUniformMatrix3x2dv
000000000057ff18 0000000000000008 B glad_glProgramUniformMatrix3fv
000000000057ff20 0000000000000008 B glad_glProgramUniformMatrix3dv
000000000057ff28 0000000000000008 B glad_glProgramUniformMatrix2x4fv
000000000057ff30 0000000000000008 B glad_glProgramUniformMatrix2x4dv
000000000057ff38 0000000000000008 B glad_glProgramUniformMatrix2x3fv
000000000057ff40 0000000000000008 B glad_glProgramUniformMatrix2x3dv
000000000057ff48 0000000000000008 B glad_glProgramUniformMatrix2fv
000000000057ff50 0000000000000008 B glad_glProgramUniformMatrix2dv
000000000057ff58 0000000000000008 B glad_glProgramUniform4uiv
000000000057ff60 0000000000000008 B glad_glProgramUniform4ui
000000000057ff68 0000000000000008 B glad_glProgramUniform4iv
000000000057ff70 0000000000000008 B glad_glProgramUniform4i
000000000057ff78 0000000000000008 B glad_glProgramUniform4fv
000000000057ff80 0000000000000008 B glad_glProgramUniform4f
000000000057ff88 0000000000000008 B glad_glProgramUniform4dv
000000000057ff90 0000000000000008 B glad_glProgramUniform4d
000000000057ff98 0000000000000008 B glad_glProgramUniform3uiv
000000000057ffa0 0000000000000008 B glad_glProgramUniform3ui
000000000057ffa8 0000000000000008 B glad_glProgramUniform3iv
000000000057ffb0 0000000000000008 B glad_glProgramUniform3i
000000000057ffb8 0000000000000008 B glad_glProgramUniform3fv
000000000057ffc0 0000000000000008 B glad_glProgramUniform3f
000000000057ffc8 0000000000000008 B glad_glProgramUniform3dv
000000000057ffd0 0000000000000008 B glad_glProgramUniform3d
000000000057ffd8 0000000000000008 B glad_glProgramUniform2uiv
000000000057ffe0 0000000000000008 B glad_glProgramUniform2ui
000000000057ffe8 0000000000000008 B glad_glProgramUniform2iv
000000000057fff0 0000000000000008 B glad_glProgramUniform2i
000000000057fff8 0000000000000008 B glad_glProgramUniform2fv
0000000000580000 0000000000000008 B glad_glProgramUniform2f
0000000000580008 0000000000000008 B glad_glProgramUniform2dv
0000000000580010 0000000000000008 B glad_glProgramUniform2d
0000000000580018 0000000000000008 B glad_glProgramUniform1uiv
0000000000580020 0000000000000008 B glad_glProgramUniform1ui
0000000000580028 0000000000000008 B glad_glProgramUniform1iv
0000000000580030 0000000000000008 B glad_glProgramUniform1i
0000000000580038 0000000000000008 B glad_glProgramUniform1fv
0000000000580040 0000000000000008 B glad_glProgramUniform1f
0000000000580048 0000000000000008 B glad_glProgramUniform1dv
0000000000580050 0000000000000008 B glad_glProgramUniform1d
0000000000580058 0000000000000008 B glad_glProgramParameteri
0000000000580060 0000000000000008 B glad_glProgramBinary
0000000000580068 0000000000000008 B glad_glPrimitiveRestartIndex
0000000000580070 0000000000000008 B glad_glPolygonOffset
0000000000580078 0000000000000008 B glad_glPolygonMode
0000000000580080 0000000000000008 B glad_glPointSize
0000000000580088 0000000000000008 B glad_glPointParameteriv
0000000000580090 0000000000000008 B glad_glPointParameteri
0000000000580098 0000000000000008 B glad_glPointParameterfv
00000000005800a0 0000000000000008 B glad_glPointParameterf
00000000005800a8 0000000000000008 B glad_glPixelStorei
00000000005800b0 0000000000000008 B glad_glPixelStoref
00000000005800b8 0000000000000008 B glad_glPauseTransformFeedback
00000000005800c0 0000000000000008 B glad_glPatchParameteri
00000000005800c8 0000000000000008 B glad_glPatchParameterfv
00000000005800d0 0000000000000008 B glad_glObjectPtrLabel
00000000005800d8 0000000000000008 B glad_glObjectLabel
00000000005800e0 0000000000000008 B glad_glNamedRenderbufferStorageMultisample
00000000005800e8 0000000000000008 B glad_glNamedRenderbufferStorage
00000000005800f0 0000000000000008 B glad_glNamedFramebufferTextureLayer
00000000005800f8 0000000000000008 B glad_glNamedFramebufferTexture
0000000000580100 0000000000000008 B glad_glNamedFramebufferRenderbuffer
0000000000580108 0000000000000008 B glad_glNamedFramebufferReadBuffer
0000000000580110 0000000000000008 B glad_glNamedFramebufferParameteri
0000000000580118 0000000000000008 B glad_glNamedFramebufferDrawBuffers
0000000000580120 0000000000000008 B glad_glNamedFramebufferDrawBuffer
0000000000580128 0000000000000008 B glad_glNamedBufferSubData
0000000000580130 0000000000000008 B glad_glNamedBufferStorage
0000000000580138 0000000000000008 B glad_glNamedBufferData
0000000000580140 0000000000000008 B glad_glMultiDrawElementsIndirect
0000000000580148 0000000000000008 B glad_glMultiDrawElementsBaseVertex
0000000000580150 0000000000000008 B glad_glMultiDrawElements
0000000000580158 0000000000000008 B glad_glMultiDrawArraysIndirect
0000000000580160 0000000000000008 B glad_glMultiDrawArrays
0000000000580168 0000000000000008 B glad_glMinSampleShading
0000000000580170 0000000000000008 B glad_glMemoryBarrier
0000000000580178 0000000000000008 B glad_glMapNamedBufferRange
0000000000580180 0000000000000008 B glad_glMapNamedBuffer
0000000000580188 0000000000000008 B glad_glMapBufferRange
0000000000580190 0000000000000008 B glad_glMapBuffer
0000000000580198 0000000000000008 B glad_glLogicOp
00000000005801a0 0000000000000008 B glad_glLinkProgram
00000000005801a8 0000000000000008 B glad_glLineWidth
00000000005801b0 0000000000000008 B glad_glIsVertexArray
00000000005801b8 0000000000000008 B glad_glIsTransformFeedback
00000000005801c0 0000000000000008 B glad_glIsTexture
00000000005801c8 0000000000000008 B glad_glIsSync
00000000005801d0 0000000000000008 B glad_glIsShader
00000000005801d8 0000000000000008 B glad_glIsSampler
00000000005801e0 0000000000000008 B glad_glIsRenderbuffer
00000000005801e8 0000000000000008 B glad_glIsQuery
00000000005801f0 0000000000000008 B glad_glIsProgramPipeline
00000000005801f8 0000000000000008 B glad_glIsProgram
0000000000580200 0000000000000008 B glad_glIsFramebuffer
0000000000580208 0000000000000008 B glad_glIsEnabledi
0000000000580210 0000000000000008 B glad_glIsEnabled
0000000000580218 0000000000000008 B glad_glIsBuffer
0000000000580220 0000000000000008 B glad_glInvalidateTexSubImage
0000000000580228 0000000000000008 B glad_glInvalidateTexImage
0000000000580230 0000000000000008 B glad_glInvalidateSubFramebuffer
0000000000580238 0000000000000008 B glad_glInvalidateNamedFramebufferSubData
0000000000580240 0000000000000008 B glad_glInvalidateNamedFramebufferData
0000000000580248 0000000000000008 B glad_glInvalidateFramebuffer
0000000000580250 0000000000000008 B glad_glInvalidateBufferSubData
0000000000580258 0000000000000008 B glad_glInvalidateBufferData
0000000000580260 0000000000000008 B glad_glHint
0000000000580280 0000000000000008 B glad_glGetVertexAttribPointerv
0000000000580288 0000000000000008 B glad_glGetVertexAttribLdv
0000000000580268 0000000000000008 B glad_glGetVertexAttribiv
0000000000580290 0000000000000008 B glad_glGetVertexAttribIuiv
0000000000580298 0000000000000008 B glad_glGetVertexAttribIiv
0000000000580270 0000000000000008 B glad_glGetVertexAttribfv
0000000000580278 0000000000000008 B glad_glGetVertexAttribdv
00000000005802a0 0000000000000008 B glad_glGetVertexArrayiv
00000000005802a8 0000000000000008 B glad_glGetVertexArrayIndexediv
00000000005802b0 0000000000000008 B glad_glGetVertexArrayIndexed64iv
00000000005802b8 0000000000000008 B glad_glGetUniformuiv
00000000005802d8 0000000000000008 B glad_glGetUniformSubroutineuiv
00000000005802e0 0000000000000008 B glad_glGetUniformLocation
00000000005802c0 0000000000000008 B glad_glGetUniformiv
00000000005802e8 0000000000000008 B glad_glGetUniformIndices
00000000005802c8 0000000000000008 B glad_glGetUniformfv
00000000005802d0 0000000000000008 B glad_glGetUniformdv
00000000005802f0 0000000000000008 B glad_glGetUniformBlockIndex
0000000000580310 0000000000000008 B glad_glGetTransformFeedbackVarying
00000000005802f8 0000000000000008 B glad_glGetTransformFeedbackiv
0000000000580300 0000000000000008 B glad_glGetTransformFeedbacki_v
0000000000580308 0000000000000008 B glad_glGetTransformFeedbacki64_v
0000000000580318 0000000000000008 B glad_glGetTextureParameteriv
0000000000580328 0000000000000008 B glad_glGetTextureParameterIuiv
0000000000580330 0000000000000008 B glad_glGetTextureParameterIiv
0000000000580320 0000000000000008 B glad_glGetTextureParameterfv
0000000000580338 0000000000000008 B glad_glGetTextureLevelParameteriv
0000000000580340 0000000000000008 B glad_glGetTextureLevelParameterfv
0000000000580348 0000000000000008 B glad_glGetTextureImage
0000000000580350 0000000000000008 B glad_glGetTexParameteriv
0000000000580360 0000000000000008 B glad_glGetTexParameterIuiv
0000000000580368 0000000000000008 B glad_glGetTexParameterIiv
0000000000580358 0000000000000008 B glad_glGetTexParameterfv
0000000000580370 0000000000000008 B glad_glGetTexLevelParameteriv
0000000000580378 0000000000000008 B glad_glGetTexLevelParameterfv
0000000000580380 0000000000000008 B glad_glGetTexImage
0000000000580388 0000000000000008 B glad_glGetSynciv
0000000000580390 0000000000000008 B glad_glGetSubroutineUniformLocation
0000000000580398 0000000000000008 B glad_glGetSubroutineIndex
00000000005803a0 0000000000000008 B glad_glGetStringi
00000000005803a8 0000000000000008 B glad_glGetString
00000000005803b8 0000000000000008 B glad_glGetShaderSource
00000000005803c0 0000000000000008 B glad_glGetShaderPrecisionFormat
00000000005803b0 0000000000000008 B glad_glGetShaderiv
00000000005803c8 0000000000000008 B glad_glGetShaderInfoLog
00000000005803d0 0000000000000008 B glad_glGetSamplerParameteriv
00000000005803e0 0000000000000008 B glad_glGetSamplerParameterIuiv
00000000005803e8 0000000000000008 B glad_glGetSamplerParameterIiv
00000000005803d8 0000000000000008 B glad_glGetSamplerParameterfv
00000000005803f0 0000000000000008 B glad_glGetRenderbufferParameteriv
0000000000580400 0000000000000008 B glad_glGetQueryObjectuiv
0000000000580408 0000000000000008 B glad_glGetQueryObjectui64v
0000000000580410 0000000000000008 B glad_glGetQueryObjectiv
0000000000580418 0000000000000008 B glad_glGetQueryObjecti64v
00000000005803f8 0000000000000008 B glad_glGetQueryiv
0000000000580420 0000000000000008 B glad_glGetQueryIndexediv
0000000000580428 0000000000000008 B glad_glGetQueryBufferObjectuiv
0000000000580430 0000000000000008 B glad_glGetQueryBufferObjectui64v
0000000000580438 0000000000000008 B glad_glGetQueryBufferObjectiv
0000000000580440 0000000000000008 B glad_glGetQueryBufferObjecti64v
0000000000580450 0000000000000008 B glad_glGetProgramStageiv
0000000000580460 0000000000000008 B glad_glGetProgramResourceName
0000000000580468 0000000000000008 B glad_glGetProgramResourceLocationIndex
0000000000580470 0000000000000008 B glad_glGetProgramResourceLocation
0000000000580458 0000000000000008 B glad_glGetProgramResourceiv
0000000000580478 0000000000000008 B glad_glGetProgramResourceIndex
0000000000580480 0000000000000008 B glad_glGetProgramPipelineiv
0000000000580488 0000000000000008 B glad_glGetProgramPipelineInfoLog
0000000000580448 0000000000000008 B glad_glGetProgramiv
0000000000580490 0000000000000008 B glad_glGetProgramInterfaceiv
0000000000580498 0000000000000008 B glad_glGetProgramInfoLog
00000000005804a0 0000000000000008 B glad_glGetProgramBinary
00000000005804a8 0000000000000008 B glad_glGetPointerv
00000000005804b0 0000000000000008 B glad_glGetObjectPtrLabel
00000000005804b8 0000000000000008 B glad_glGetObjectLabel
00000000005804c0 0000000000000008 B glad_glGetNamedRenderbufferParameteriv
00000000005804c8 0000000000000008 B glad_glGetNamedFramebufferParameteriv
00000000005804d0 0000000000000008 B glad_glGetNamedFramebufferAttachmentParameteriv
00000000005804d8 0000000000000008 B glad_glGetNamedBufferSubData
00000000005804e0 0000000000000008 B glad_glGetNamedBufferPointerv
00000000005804e8 0000000000000008 B glad_glGetNamedBufferParameteriv
00000000005804f0 0000000000000008 B glad_glGetNamedBufferParameteri64v
00000000005804f8 0000000000000008 B glad_glGetMultisamplefv
0000000000580500 0000000000000008 B glad_glGetInternalformativ
0000000000580508 0000000000000008 B glad_glGetInternalformati64v
0000000000580510 0000000000000008 B glad_glGetIntegerv
0000000000580518 0000000000000008 B glad_glGetIntegeri_v
0000000000580520 0000000000000008 B glad_glGetInteger64v
0000000000580528 0000000000000008 B glad_glGetInteger64i_v
0000000000580530 0000000000000008 B glad_glGetFramebufferParameteriv
0000000000580538 0000000000000008 B glad_glGetFramebufferAttachmentParameteriv
0000000000580540 0000000000000008 B glad_glGetFragDataLocation
0000000000580548 0000000000000008 B glad_glGetFragDataIndex
0000000000580550 0000000000000008 B glad_glGetFloatv
0000000000580558 0000000000000008 B glad_glGetFloati_v
0000000000580560 0000000000000008 B glad_glGetError
0000000000580568 0000000000000008 B glad_glGetDoublev
0000000000580570 0000000000000008 B glad_glGetDoublei_v
0000000000580578 0000000000000008 B glad_glGetBufferSubData
0000000000580580 0000000000000008 B glad_glGetBufferPointerv
0000000000580588 0000000000000008 B glad_glGetBufferParameteriv
0000000000580590 0000000000000008 B glad_glGetBufferParameteri64v
0000000000580598 0000000000000008 B glad_glGetBooleanv
00000000005805a0 0000000000000008 B glad_glGetBooleani_v
00000000005805a8 0000000000000008 B glad_glGetAttribLocation
00000000005805b0 0000000000000008 B glad_glGetAttachedShaders
00000000005805b8 0000000000000008 B glad_glGetActiveUniformsiv
00000000005805c0 0000000000000008 B glad_glGetActiveUniformName
00000000005805d0 0000000000000008 B glad_glGetActiveUniformBlockName
00000000005805c8 0000000000000008 B glad_glGetActiveUniformBlockiv
00000000005805d8 0000000000000008 B glad_glGetActiveUniform
00000000005805e8 0000000000000008 B glad_glGetActiveSubroutineUniformName
00000000005805e0 0000000000000008 B glad_glGetActiveSubroutineUniformiv
00000000005805f0 0000000000000008 B glad_glGetActiveSubroutineName
00000000005805f8 0000000000000008 B glad_glGetActiveAttrib
0000000000580600 0000000000000008 B glad_glGetActiveAtomicCounterBufferiv
0000000000580618 0000000000000008 B glad_glGenVertexArrays
0000000000580620 0000000000000008 B glad_glGenTransformFeedbacks
0000000000580628 0000000000000008 B glad_glGenTextures
0000000000580630 0000000000000008 B glad_glGenSamplers
0000000000580638 0000000000000008 B glad_glGenRenderbuffers
0000000000580640 0000000000000008 B glad_glGenQueries
0000000000580648 0000000000000008 B glad_glGenProgramPipelines
0000000000580650 0000000000000008 B glad_glGenFramebuffers
0000000000580608 0000000000000008 B glad_glGenerateTextureMipmap
0000000000580610 0000000000000008 B glad_glGenerateMipmap
0000000000580658 0000000000000008 B glad_glGenBuffers
0000000000580660 0000000000000008 B glad_glFrontFace
0000000000580668 0000000000000008 B glad_glFramebufferTextureLayer
0000000000580670 0000000000000008 B glad_glFramebufferTexture3D
0000000000580678 0000000000000008 B glad_glFramebufferTexture2D
0000000000580680 0000000000000008 B glad_glFramebufferTexture1D
0000000000580688 0000000000000008 B glad_glFramebufferTexture
0000000000580690 0000000000000008 B glad_glFramebufferRenderbuffer
0000000000580698 0000000000000008 B glad_glFramebufferParameteri
00000000005806a0 0000000000000008 B glad_glFlushMappedNamedBufferRange
00000000005806a8 0000000000000008 B glad_glFlushMappedBufferRange
00000000005806b0 0000000000000008 B glad_glFlush
00000000005806b8 0000000000000008 B glad_glFinish
00000000005806c0 0000000000000008 B glad_glFenceSync
00000000005806c8 0000000000000008 B glad_glEndTransformFeedback
00000000005806d0 0000000000000008 B glad_glEndQueryIndexed
00000000005806d8 0000000000000008 B glad_glEndQuery
00000000005806e0 0000000000000008 B glad_glEndConditionalRender
00000000005806f0 0000000000000008 B glad_glEnableVertexAttribArray
00000000005806f8 0000000000000008 B glad_glEnableVertexArrayAttrib
00000000005806e8 0000000000000008 B glad_glEnablei
0000000000580700 0000000000000008 B glad_glEnable
0000000000580708 0000000000000008 B glad_glDrawTransformFeedbackStreamInstanced
0000000000580710 0000000000000008 B glad_glDrawTransformFeedbackStream
0000000000580718 0000000000000008 B glad_glDrawTransformFeedbackInstanced
0000000000580720 0000000000000008 B glad_glDrawTransformFeedback
0000000000580728 0000000000000008 B glad_glDrawRangeElementsBaseVertex
0000000000580730 0000000000000008 B glad_glDrawRangeElements
0000000000580738 0000000000000008 B glad_glDrawElementsInstancedBaseVertexBaseInstance
0000000000580740 0000000000000008 B glad_glDrawElementsInstancedBaseVertex
0000000000580748 0000000000000008 B glad_glDrawElementsInstancedBaseInstance
0000000000580750 0000000000000008 B glad_glDrawElementsInstanced
0000000000580758 0000000000000008 B glad_glDrawElementsIndirect
0000000000580760 0000000000000008 B glad_glDrawElementsBaseVertex
0000000000580768 0000000000000008 B glad_glDrawElements
0000000000580770 0000000000000008 B glad_glDrawBuffers
0000000000580778 0000000000000008 B glad_glDrawBuffer
0000000000580780 0000000000000008 B glad_glDrawArraysInstancedBaseInstance
0000000000580788 0000000000000008 B glad_glDrawArraysInstanced
0000000000580790 0000000000000008 B glad_glDrawArraysIndirect
0000000000580798 0000000000000008 B glad_glDrawArrays
00000000005807a0 0000000000000008 B glad_glDispatchComputeIndirect
00000000005807a8 0000000000000008 B glad_glDispatchCompute
00000000005807b8 0000000000000008 B glad_glDisableVertexAttribArray
00000000005807c0 0000000000000008 B glad_glDisableVertexArrayAttrib
00000000005807b0 0000000000000008 B glad_glDisablei
00000000005807c8 0000000000000008 B glad_glDisable
00000000005807d0 0000000000000008 B glad_glDetachShader
00000000005807e0 0000000000000008 B glad_glDepthRangeIndexed
00000000005807d8 0000000000000008 B glad_glDepthRangef
00000000005807e8 0000000000000008 B glad_glDepthRangeArrayv
00000000005807f0 0000000000000008 B glad_glDepthRange
00000000005807f8 0000000000000008 B glad_glDepthMask
0000000000580800 0000000000000008 B glad_glDepthFunc
0000000000580808 0000000000000008 B glad_glDeleteVertexArrays
0000000000580810 0000000000000008 B glad_glDeleteTransformFeedbacks
0000000000580818 0000000000000008 B glad_glDeleteTextures
0000000000580820 0000000000000008 B glad_glDeleteSync
0000000000580828 0000000000000008 B glad_glDeleteShader
0000000000580830 0000000000000008 B glad_glDeleteSamplers
0000000000580838 0000000000000008 B glad_glDeleteRenderbuffers
0000000000580840 0000000000000008 B glad_glDeleteQueries
0000000000580848 0000000000000008 B glad_glDeleteProgramPipelines
0000000000580850 0000000000000008 B glad_glDeleteProgram
0000000000580858 0000000000000008 B glad_glDeleteFramebuffers
0000000000580860 0000000000000008 B glad_glDeleteBuffers
0000000000580868 0000000000000008 B glad_glCullFace
0000000000580870 0000000000000008 B glad_glCreateVertexArrays
0000000000580878 0000000000000008 B glad_glCreateTransformFeedbacks
0000000000580880 0000000000000008 B glad_glCreateTextures
0000000000580888 0000000000000008 B glad_glCreateShaderProgramv
0000000000580890 0000000000000008 B glad_glCreateShader
0000000000580898 0000000000000008 B glad_glCreateSamplers
00000000005808a0 0000000000000008 B glad_glCreateRenderbuffers
00000000005808a8 0000000000000008 B glad_glCreateQueries
00000000005808b0 0000000000000008 B glad_glCreateProgramPipelines
00000000005808b8 0000000000000008 B glad_glCreateProgram
00000000005808c0 0000000000000008 B glad_glCreateFramebuffers
00000000005808c8 0000000000000008 B glad_glCreateBuffers
00000000005808d0 0000000000000008 B glad_glCopyTextureSubImage3D
00000000005808d8 0000000000000008 B glad_glCopyTextureSubImage2D
00000000005808e0 0000000000000008 B glad_glCopyTextureSubImage1D
00000000005808e8 0000000000000008 B glad_glCopyTexSubImage3D
00000000005808f0 0000000000000008 B glad_glCopyTexSubImage2D
00000000005808f8 0000000000000008 B glad_glCopyTexSubImage1D
0000000000580900 0000000000000008 B glad_glCopyTexImage2D
0000000000580908 0000000000000008 B glad_glCopyTexImage1D
0000000000580910 0000000000000008 B glad_glCopyNamedBufferSubData
0000000000580918 0000000000000008 B glad_glCopyImageSubData
0000000000580920 0000000000000008 B glad_glCopyBufferSubData
0000000000580928 0000000000000008 B glad_glCompileShader
0000000000580930 0000000000000008 B glad_glColorMaski
0000000000580938 0000000000000008 B glad_glColorMask
0000000000580940 0000000000000008 B glad_glClientWaitSync
0000000000580948 0000000000000008 B glad_glClearNamedFramebufferuiv
0000000000580950 0000000000000008 B glad_glClearNamedFramebufferiv
0000000000580958 0000000000000008 B glad_glClearNamedFramebufferfv
0000000000580960 0000000000000008 B glad_glClearNamedFramebufferfi
0000000000580968 0000000000000008 B glad_glClearNamedBufferSubData
0000000000580970 0000000000000008 B glad_glClearNamedBufferData
0000000000580978 0000000000000008 B glad_glClearDepthf
0000000000580980 0000000000000008 B glad_glClearDepth
0000000000580988 0000000000000008 B glad_glClearColor
0000000000580990 0000000000000008 B glad_glClearBufferuiv
00000000005809b0 0000000000000008 B glad_glClearBufferSubData
0000000000580998 0000000000000008 B glad_glClearBufferiv
00000000005809a0 0000000000000008 B glad_glClearBufferfv
00000000005809a8 0000000000000008 B glad_glClearBufferfi
00000000005809b8 0000000000000008 B glad_glClearBufferData
00000000005809c0 0000000000000008 B glad_glClear
00000000005809c8 0000000000000008 B glad_glClampColor
00000000005809d0 0000000000000008 B glad_glCheckNamedFramebufferStatus
00000000005809d8 0000000000000008 B glad_glCheckFramebufferStatus
00000000005809e0 0000000000000008 B glad_glBufferSubData
00000000005809e8 0000000000000008 B glad_glBufferStorage
00000000005809f0 0000000000000008 B glad_glBufferData
00000000005809f8 0000000000000008 B glad_glBlitNamedFramebuffer
0000000000580a00 0000000000000008 B glad_glBlitFramebuffer
0000000000580a10 0000000000000008 B glad_glBlendFuncSeparatei
0000000000580a18 0000000000000008 B glad_glBlendFuncSeparate
0000000000580a08 0000000000000008 B glad_glBlendFunci
0000000000580a20 0000000000000008 B glad_glBlendFunc
0000000000580a30 0000000000000008 B glad_glBlendEquationSeparatei
0000000000580a38 0000000000000008 B glad_glBlendEquationSeparate
0000000000580a28 0000000000000008 B glad_glBlendEquationi
0000000000580a40 0000000000000008 B glad_glBlendEquation
0000000000580a48 0000000000000008 B glad_glBlendColor
0000000000580a50 0000000000000008 B glad_glBindVertexBuffer
0000000000580a58 0000000000000008 B glad_glBindVertexArray
0000000000580a60 0000000000000008 B glad_glBindTransformFeedback
0000000000580a68 0000000000000008 B glad_glBindTextureUnit
0000000000580a70 0000000000000008 B glad_glBindTexture
0000000000580a78 0000000000000008 B glad_glBindSampler
0000000000580a80 0000000000000008 B glad_glBindRenderbuffer
0000000000580a88 0000000000000008 B glad_glBindProgramPipeline
0000000000580a90 0000000000000008 B glad_glBindImageTexture
0000000000580a98 0000000000000008 B glad_glBindFramebuffer
0000000000580aa0 0000000000000008 B glad_glBindFragDataLocationIndexed
0000000000580aa8 0000000000000008 B glad_glBindFragDataLocation
0000000000580ab0 0000000000000008 B glad_glBindBufferRange
0000000000580ab8 0000000000000008 B glad_glBindBufferBase
0000000000580ac0 0000000000000008 B glad_glBindBuffer
0000000000580ac8 0000000000000008 B glad_glBindAttribLocation
0000000000580ad0 0000000000000008 B glad_glBeginTransformFeedback
0000000000580ad8 0000000000000008 B glad_glBeginQueryIndexed
0000000000580ae0 0000000000000008 B glad_glBeginQuery
0000000000580ae8 0000000000000008 B glad_glBeginConditionalRender
0000000000580af0 0000000000000008 B glad_glAttachShader
0000000000580af8 0000000000000008 B glad_glActiveTexture
0000000000580b00 0000000000000008 B glad_glActiveShaderProgram
000000000013af00 0000000000000008 D fractionalScaleListener
000000000013d6f8 0000000000000008 D EngineName
0000000001c95468 0000000000000008 B console_log_file
0000000000980e00 0000000000000008 B berserkFinished
00000000001a25d8 0000000000000008 B audiologSubjects
00000000001a25c8 0000000000000008 B audioLogSpeech2Text
00000000001a25d0 0000000000000008 B audiologSenders
00000000001a25e0 0000000000000008 B audiologNames
00000000003737c0 0000000000000004 B worldMin_z
00000000003737c4 0000000000000004 B worldMin_x
000000000075ed50 0000000000000004 B voxelMinCenterZ
000000000075ed54 0000000000000004 B voxelMinCenterX
0000000001cac408 0000000000000004 B totalPixels
0000000001cac404 0000000000000004 B totalPaletteColors
000000000013d350 0000000000000004 D reboundVelocity
000000000013c91c 0000000000000004 D random_range_rng
000000000014a48c 0000000000000004 B numPackedGlyphsStopD
000000000014a490 0000000000000004 B numPackedGlyphs
000000000013c880 0000000000000004 D numFontRanges
000000000014a494 0000000000000004 B mmap_cleanup_count
0000000000580b24 0000000000000004 B GLAD_GL_VERSION_4_3
0000000000580b28 0000000000000004 B GLAD_GL_VERSION_4_2
0000000000580b2c 0000000000000004 B GLAD_GL_VERSION_4_1
0000000000580b30 0000000000000004 B GLAD_GL_VERSION_4_0
0000000000580b34 0000000000000004 B GLAD_GL_VERSION_3_3
0000000000580b38 0000000000000004 B GLAD_GL_VERSION_3_2
0000000000580b3c 0000000000000004 B GLAD_GL_VERSION_3_1
0000000000580b40 0000000000000004 B GLAD_GL_VERSION_3_0
0000000000580b44 0000000000000004 B GLAD_GL_VERSION_2_1
0000000000580b48 0000000000000004 B GLAD_GL_VERSION_2_0
0000000000580b4c 0000000000000004 B GLAD_GL_VERSION_1_5
0000000000580b50 0000000000000004 B GLAD_GL_VERSION_1_4
0000000000580b54 0000000000000004 B GLAD_GL_VERSION_1_3
0000000000580b58 0000000000000004 B GLAD_GL_VERSION_1_2
0000000000580b5c 0000000000000004 B GLAD_GL_VERSION_1_1
0000000000580b60 0000000000000004 B GLAD_GL_VERSION_1_0
0000000000580b08 0000000000000004 B GLAD_GL_ARB_texture_view
0000000000580b0c 0000000000000004 B GLAD_GL_ARB_texture_storage
0000000000580b10 0000000000000004 B GLAD_GL_ARB_shader_storage_buffer_object
0000000000580b14 0000000000000004 B GLAD_GL_ARB_map_buffer_range
0000000000580b18 0000000000000004 B GLAD_GL_ARB_direct_state_access
0000000000580b1c 0000000000000004 B GLAD_GL_ARB_copy_buffer
0000000000580b20 0000000000000004 B GLAD_GL_ARB_buffer_storage
000000000013c910 0000000000000004 D genericTextWidthFacStopD
000000000013c914 0000000000000004 D genericTextHeightFacStopD
000000000013c918 0000000000000004 D genericTextHeightFac
000000000014a484 0000000000000004 B fontAtlasTexStopD
000000000014a488 0000000000000004 B fontAtlasTex
0000000000580b64 0000000000000004 B fogFac
0000000000980dec 0000000000000004 B fogColorR
0000000000980de8 0000000000000004 B fogColorG
0000000000980de4 0000000000000004 B fogColorB
0000000000980de0 0000000000000004 B fogBaseDensityForLevel
0000000000141880 0000000000000004 B fixedNumberAdvanceWidthStopD
0000000000141884 0000000000000004 B fixedNumberAdvanceWidth
000000000013d464 0000000000000004 D cursorPosition_y
000000000013d468 0000000000000004 D cursorPosition_x
000000000013d354 0000000000000004 D currentMonitorIndex
000000000013d46c 0000000000000004 D cam_yaw
0000000000980df0 0000000000000004 B cam_roll
0000000000980df4 0000000000000004 B cam_pitch
0000000000980df8 0000000000000004 B berserkSeedTime
000000000013d470 0000000000000004 D aspect3D
00000000006f34e0 0000000000000002 B selfIdx
00000000003737ca 0000000000000002 B playerCellIdx
00000000003737c8 0000000000000002 B numCellsVisible
0000000001cac400 0000000000000002 B loadedTexturesMaxIndex
0000000000164df8 0000000000000002 B loadedModelsMaxIndex
000000000075edc2 0000000000000002 B loadedLights
0000000000581518 0000000000000002 B loadedInstances
0000000001caaec0 0000000000000002 B loadedAmbients
000000000058151a 0000000000000002 B entityCount
000000000075edc0 0000000000000002 B editModeTestEntityDefinition
000000000013d460 0000000000000002 D editModeSelection
000000000013d474 0000000000000001 D queuedLevelToLoad
00000000002f6b82 0000000000000001 B numActivePortals
00000000006f34e2 0000000000000001 B boosterActive
```

```
❯ wc -l *.[ch] 2>/dev/null | sort -nr | head -n 50
 17595 total
  1988 weapons.c
  1815 voxen.h
  1764 ai.c
  1327 voxen.c
  1115 animation.c
   998 audio.c
   802 menu.c
   695 dynamic_culling.c
   586 update.c
   585 automap.c
   553 input.c
   528 level.c
   499 console.c
   397 saveload.c
   386 credits.h
   368 target.c
   332 entlogic.c
   327 use.c
   321 biomonitor.c
   292 data_text.c
   291 data_fonts.c
   279 helpers.c
   254 os.h
   230 data_models.c
   218 physics.c
   213 data_parser.c
   192 init.c
   133 data_textures.c
   107 todo.c
```

Helper bash commands to generate frame sequences in models.txt:
start_index=2104                                                                                                                                                                               ✔  0.25  │ 9.8G    19:43:49  01/24/2026  ▓▒░
for i in $(seq 2 50); do
  printf "#Models/flight_fanwall_%06d.obj\nindex: %d\n" "$i" "$start_index"
  ((start_index++))
done


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

FPS: 544
ms: 1.83
RAM: 774mb (mostly the animation system duplicating models for every frame, stupid but works great!)
VRAM: 677mb
Build 438ms
Init 1.529secs

CPU: 0.23ms
GPU: 1.83ms
