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
straightforward. (TBD shaders part of mod/gamecode?)

Using GLFW3.5(what's left of it) and OpenGL 4.3+, this engine attempts to achieve
maximum compatibility and maximum performance with minimal footprint.  Heavy use of 
SSBOs is made though this is still compatible with old hardware and GL drivers from
15yrs ago; further very few GL extensions are used to further widen compatibility.
Careful handling of CPU to GPU transfers is made to minimize VRAM and to prevent 
naughty GL drivers duplicating that VRAM into the CPU RAM space which is also kept
minimal.

All texture and model data is loaded from disk directly for ease of development
and full mod support by design.  Any intermediate format is internal to the engine.

Minimizing hierarchical layers and leveraging sensibly named globals to cut out
fluff and overhead is important.  Minimal dependencies and leveraging tried and
true systems is important.

The gamecode for mod as game lives in a separately compiled hermetic dll/so file
that has no libc usage, no external linking, and strict API interop with the engine.

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
 * zig cc for cross compilation and libc underversioning for compatibility
 * OpenGL(Linux) / OpenGL32(Win)
 * pthread

Single command:

```bash
sudo apt install zig libgl1-mesa-dev
```

Notes: Glfw remnants still present, bundled together in glfw.c as I ever continue to reduce dependencies.

## System Architecture

Order of Ops:
Initializes various core systems (OpenGL+Window)
Loads data resources (textures, models, etc.)
Loads scripting VM
Parses all game/mod scripts
Initializes data handling systems and parsers using all above data
Level Load using gamedata definition to pick starting level
Starts game loop:
  Polls input
  Processes input and applies movement key states, mouselook
  Animation (done prior to physics such that physics can respond properly)
  Physics
  Game Logic Update Loop
  Render Shadowmaps
  Render Depth Prepass
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
Compiling voxen, total iterations today 85 (2026-03-29)...
Built engine successfully.
Built mod gamecode successfully.
Built engine and mod in 954 ms
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
GLFW init took 0.010394029 secs
Load Config.ini, glfw create window and GL context took 0.081719108 secs
GL function loading took 0.000441692 secs
Set monitor, Set GLFW callbacks, Compile shaders took 0.004927384 secs
GL buffer definitions took 0.000161770 secs
Loading game definition... Citadel:: num levels: 14, start level: 1... took 0.000048480 secs
Reloading mod code...dlopen-ing...done!
Loading    5 fonts...took 0.023699606 secs
GL SSBOs and Settings Apply... took 0.011501173 secs
Loading models (5989) ... total vertices: 11670811, total tris: 65659044, took 0.699694164 secs
Loading textures (1579) ... total palette colors: 100535, total pixels: 29804440... took 0.126002 secs
InitializeEnvironment completed
Game Initialized in 1.263755100 secs
Loading new game...
Rendered screen saying "Loading new game..."
Cam view added.  Count at 1
Cam view added.  Count at 2
Loaded 17791 entities, 1058 static lights for Level 1... took 0.101106322 secs
Entity instances initialized after load
Culling...found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341... took 0.080589886 secs
Player named "" started the game!
```

Log output from one run with DEBUG_RAM_OUTPUT declared in voxen.h:

```
Compiling voxen, total iterations today 87 (2026-03-29)...
Built engine successfully.
Built mod gamecode successfully.
Built engine and mod in 395 ms
Memory at program start: Heap 0 bytes (0 KB | 0.00 MB), USS 35565568 bytes (34732 KB | 33.92 MB)
Memory at InitializeEnvironment start: Heap 0 bytes (0 KB | 0.00 MB), USS 35565568 bytes (34732 KB | 33.92 MB)
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
GLFW init took 0.011530377 secs
Load Config.ini, glfw create window and GL context took 0.073844631 secs
GL function loading took 0.000458672 secs
Set monitor, Set GLFW callbacks, Compile shaders took 0.004821350 secs
GL buffer definitions took 0.000160161 secs
Loading game definition... Citadel:: num levels: 14, start level: 1... took 0.000090910 secs
Reloading mod code...dlopen-ing...done!
Memory at after loading mod: Heap 9699328 bytes (9472 KB | 9.25 MB), USS 53764096 bytes (52504 KB | 51.27 MB)
Loading    5 fonts...took 0.023276584 secs
Memory at after freeing window bar icon: Heap 9699328 bytes (9472 KB | 9.25 MB), USS 116690944 bytes (113956 KB | 111.29 MB)
GL SSBOs and Settings Apply... took 0.013768522 secs
Loading models (5989) ...Memory at after model load loop: Heap 12599296 bytes (12304 KB | 12.02 MB), USS 399085568 bytes (389732 KB | 380.60 MB)
Memory at after to model to gpu transfer: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 459104256 bytes (448344 KB | 437.84 MB)
 total vertices: 11670811, total tris: 65659044, took 0.701599961 secs
Memory at After Load Models: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 452018176 bytes (441424 KB | 431.08 MB)
Memory at start of LoadTextures: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 452018176 bytes (441424 KB | 431.08 MB)
Loading textures (1579) ... Memory at After loop for load textures: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 509079552 bytes (497148 KB | 485.50 MB)
total palette colors: 100535, total pixels: 29804440... took 0.130844 secs
Memory at After LoadTextures and after deallocation: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 482234368 bytes (470932 KB | 459.89 MB)
Memory at InitializeEnvironment end: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 483131392 bytes (471808 KB | 460.75 MB)
InitializeEnvironment completed
Memory at prior to game loop: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 483131392 bytes (471808 KB | 460.75 MB)
Game Initialized in 1.301660107 secs
Memory at after 4 frames: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 492355584 bytes (480816 KB | 469.55 MB)
Memory at after 100 frames: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 460451840 bytes (449660 KB | 439.12 MB)
Memory at after 200 frames: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 460451840 bytes (449660 KB | 439.12 MB)
Memory at after 500 frames: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 460451840 bytes (449660 KB | 439.12 MB)
Memory at after 1000 frames: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 460455936 bytes (449664 KB | 439.13 MB)
Loading new game...
Rendered screen saying "Loading new game..."
Memory at start of LoadLevel: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 460468224 bytes (449676 KB | 439.14 MB)
Cam view added.  Count at 1
Cam view added.  Count at 2
Loaded 17791 entities, 1058 static lights for Level 1... took 0.096384954 secs
Memory at end of LoadLevel instances: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 462393344 bytes (451556 KB | 440.97 MB)
Entity instances initialized after load
Culling...Memory at start of Cull_Init: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 462397440 bytes (451560 KB | 440.98 MB)
found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341...Memory at end of dynamic culling DetermineClosedEdges: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 462417920 bytes (451580 KB | 440.00 MB)
 took 0.088219065 secs
Memory at end of Cull_Init: Heap 221216768 bytes (216032 KB | 210.97 MB), USS 464531456 bytes (453644 KB | 443.01 MB)
Player named "" started the game!
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

Binary size eval:

```
❯ size ./voxen
   text     data      bss      dec     hex filename
 909806 34310772 20152784 55373362 34cee32 ./voxen
```

```
ls *.* | grep -vE 'miniaudio\.h|gl\.h|glad\.c|glfw3\.h|Citadel.dll|voxen.upx|README.md|glfw_defines.h|voxen.lib|builds.csv|voxen.exe|Citadel.so|voxen.log|glfw3.dll|build.sh' | xargs wc -l 2>/dev/null | sort -nr | head -n 50
 12176 total
  2611 stb_truetype.h
  2248 voxen.c
  1438 common.h
   943 data_textures.c
   678 dynamic_culling.c
   621 physics.c
   582 helpers.c
   542 data_models.c
   498 console.c
   386 credits.h
   372 voxen.h
   289 os.h
   265 data_fonts.c
   230 input.c
   200 data_text.c
   164 interop.h
    88 level.c
    21 miniaudio.c
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
