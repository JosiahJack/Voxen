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
 * -fopenmp

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
Build completed in 323 ms
Voxen v0.7.4 by W. Josiah Jack, MIT-0 licensed
Window positioned (windowed, centered) on monitor: DVI-I-1 (primary) at 2717,141
Using GLFW 3.5.0 Wayland X11 GLX Null EGL OSMesa monotonic, OpenGL Version: 4.3.0 NVIDIA 550.144.03, GPU: NVIDIA GeForce GTX 970/PCIe/SSE2CPU: AMD Ryzen 5 5500 | Logical cores: 12
Loading game definition from ./Data/gamedata.txt... loaded Game Definition for Citadel:: num levels: 14, start level: 1... took 0.000256 secs
Loaded    2 fonts...in 0.019 s
Parallel Inits and window init took 0.141993 secs
Loading textures(1271) with max index 1270, using stb_image version:  2.28... total pallete colors: 80227, total pixels: 34152900... took 0.136199 secs
Loading   models( 698) with max index  697 ... total vertices: 57506112, total tris: 7188264, took 0.064184 secs
Loading  756 entities... took 0.000910 secs
Loaded 6056 geometry chunks and 893 static lights for Level 1... took 0.021124 secs
Sorting entity instances... opaque: 5461, double-sided: 42, transparent: 40, invisible: 510... took 0.007388 secs
Culling...found 1170 open cells, closed edges N: 368, S: 364, E: 337, W: 341... took 0.129602 secs
Number of lights with shadows: 893
0 dynamic lights in level 1
Game Initialized in 0.553212 secs
```

Log output from one run with DEBUG_RAM_OUTPUT declared in voxen.h:

```
Compiling voxen...
Shaders converted to string constants in 12 ms
Linking completed in 131 ms
Build completed in 1763 ms
Memory at prior to event system init: Heap usage 76560 bytes (74 KB | 0.07 MB), USS 1748992 bytes (1708 KB | 1.67 MB)
Voxen v0.7.2 by W. Josiah Jack, MIT-0 licensed
Memory at InitializeEnvironment start: Heap usage 82192 bytes (80 KB | 0.08 MB), USS 3018752 bytes (2948 KB | 2.88 MB)
Memory at window init: Heap usage 6744064 bytes (6586 KB | 6.43 MB), USS 18780160 bytes (18340 KB | 17.91 MB)
Window positioned (windowed, centered) on monitor: DVI-I-1 (primary) at 2717,141
OpenGL Version: 4.3.0 NVIDIA 550.144.03
GPU: NVIDIA GeForce GTX 970/PCIe/SSE2
Memory at after vao chunk bind: Heap usage 7894848 bytes (7709 KB | 7.53 MB), USS 20209664 bytes (19736 KB | 19.27 MB)
Memory at after ui image vao chunk bind: Heap usage 7898192 bytes (7713 KB | 7.53 MB), USS 20209664 bytes (19736 KB | 19.27 MB)
Memory at setup gbuffer end: Heap usage 8197424 bytes (8005 KB | 7.82 MB), USS 20271104 bytes (19796 KB | 19.33 MB)
Memory at end of font init: Heap usage 12419904 bytes (12128 KB | 11.84 MB), USS 28778496 bytes (28104 KB | 27.45 MB)
Loading fonts(2)... took 0.033038
Memory at audio init: Heap usage 13831920 bytes (13507 KB | 13.19 MB), USS 30437376 bytes (29724 KB | 29.03 MB)
Loading game definition from ./Data/gamedata.txt... loaded Game Definition for Citadel:: num levels: 14, start level: 1
Window and GL Init took 0.140204 seconds
Loading texturesMemory at start of LoadTextures: Heap usage 13919808 bytes (13593 KB | 13.27 MB), USS 35745792 bytes (34908 KB | 34.09 MB)
(1267) with max index 1266, using stb_image version:  2.28... total pallete colors: 79391, totalPixels was: 33438148... 
 took 0.253006 seconds
Memory at After LoadTextures: Heap usage 14753376 bytes (14407 KB | 14.07 MB), USS 53211136 bytes (51964 KB | 50.75 MB)
Memory at start of LoadModels: Heap usage 14753376 bytes (14407 KB | 14.07 MB), USS 53211136 bytes (51964 KB | 50.75 MB)
Loading   models( 672) with max index  671, using    Assimp version: 6.0.2... took 0.613524 seconds
Memory at After Load Models: Heap usage 160327680 bytes (156570 KB | 152.90 MB), USS 142864384 bytes (139516 KB | 136.25 MB)
Loading  755 entities... took 0.000354 seconds
Memory at after loading all entities: Heap usage 160435680 bytes (156675 KB | 153.00 MB), USS 142987264 bytes (139636 KB | 136.36 MB)
Memory at start of LoadLevel: Heap usage 160436144 bytes (156675 KB | 153.00 MB), USS 142987264 bytes (139636 KB | 136.36 MB)
Loaded 6054 geometry chunks and 892 static lights for Level 1... took 0.022155 seconds
Memory at end of LoadLevel: Heap usage 160436144 bytes (156675 KB | 153.00 MB), USS 144412672 bytes (141028 KB | 137.72 MB)
Sorting instances... took 0.002449 secs
Total opaque instances: 5427, double-sided: 42, transparent: 40, invisible: 545
Culling...Memory at start of Cull_Init: Heap usage 160444640 bytes (156684 KB | 153.01 MB), USS 144920576 bytes (141524 KB | 138.21 MB)
Memory at Start of DetermineClosedEdges: Heap usage 160444640 bytes (156684 KB | 153.01 MB), USS 144920576 bytes (141524 KB | 138.21 MB)
Found 1170 open cells for level 1, Found closed edges north: 368, south: 364, east: 337, west: 341...Memory at end of dynamic culling DetermineClosedEdges: Heap usage 160444640 bytes (156684 KB | 153.01 MB), USS 144924672 bytes (141528 KB | 138.21 MB)
 took 0.094322 seconds
Memory at end of Cull_Init: Heap usage 160444640 bytes (156684 KB | 153.01 MB), USS 147034112 bytes (143588 KB | 140.22 MB)
Generating voxel lighting data... took 0.016658 seconds, total list size: 1008105
Memory at InitializeEnvironment end: Heap usage 166905776 bytes (162993 KB | 159.17 MB), USS 162381824 bytes (158576 KB | 154.86 MB)
Memory at prior to game loop: Heap usage 166905776 bytes (162993 KB | 159.17 MB), USS 162381824 bytes (158576 KB | 154.86 MB)
Rendering shadowmaps...Memory at Start of RenderShadowmaps: Heap usage 166905776 bytes (162993 KB | 159.17 MB), USS 162381824 bytes (158576 KB | 154.86 MB)
 took 0.454595 seconds to render 892 static shadow maps
Memory at After rendering all shadowmaps: Heap usage 167185136 bytes (163266 KB | 159.44 MB), USS 162447360 bytes (158640 KB | 154.92 MB)
Game Initialized in 1.626723 secs
Memory at after 4 frames of running: Heap usage 167258208 bytes (163338 KB | 159.51 MB), USS 162844672 bytes (159028 KB | 155.30 MB)
Memory at after 100 frames of running: Heap usage 167235360 bytes (163315 KB | 159.49 MB), USS 158351360 bytes (154640 KB | 151.02 MB)
Memory at after 200 frames of running: Heap usage 167233344 bytes (163313 KB | 159.49 MB), USS 158351360 bytes (154640 KB | 151.02 MB)
```

Heap impacts:

```
❯ grep -rIn  "alloc("
voxen.c:410:    unsigned char* pixels = malloc(screen_width * screen_height * 4 * sizeof(char));
voxen.c:525:    uint32_t* voxelLightListsRaw = malloc(VOXEL_COUNT * 4 * sizeof(uint32_t));
voxen.c:526:    uint32_t* voxelLightListIndices = malloc(VOXEL_COUNT * 2 * sizeof(uint32_t));
dynamic_culling.c:45:    uint8_t* file_buffer = malloc(maxFileSize);
data_fonts.c:13:    char *p = malloc(len);
data_fonts.c:151:    unsigned char *data = malloc(size);
data_fonts.c:220:    primaryFontData = malloc(primarySize);
data_fonts.c:237:    unsigned char *secondaryFontData = malloc(secondarySize);
data_fonts.c:254:    unsigned char *atlasBitmap = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
data_fonts.c:311:    unsigned char *atlasBitmapStopD = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
data_parser.c:129:        ResourceEntry *new_entries = realloc(parser->entries, entry_count * sizeof(ResourceEntry));  
data_parser.c:350:    image_data            =   malloc(loadedTextures * sizeof(unsigned char*));
data_parser.c:351:    textureOffsets        = calloc(loadedTextures, sizeof(uint32_t));
data_parser.c:352:    textureSizes          = calloc(loadedTextures * 2, sizeof(int));
data_parser.c:353:    texturePaletteOffsets = calloc(loadedTextures, sizeof(uint32_t));
data_parser.c:354:    doubleSidedTexture    = calloc(loadedTextures,sizeof(bool));
data_parser.c:355:    transparentTexture    = calloc(loadedTextures,sizeof(bool));
data_parser.c:357:    texturePalettes             = malloc(totalPaletteColorsExtraSized * sizeof(uint32_t));
data_parser.c:358:    int32_t* widths             = malloc(loadedTextures * sizeof(int32_t));
data_parser.c:359:    int32_t* heights            = malloc(loadedTextures * sizeof(int32_t));
data_parser.c:360:    int32_t* matchedParserIdxes = malloc(loadedTextures * sizeof(int32_t));
data_parser.c:382:            uint8_t* file_buffer = malloc(file_size);
data_parser.c:408:    ColorEntry* color_pool = malloc(loadedTextures * MAX_PALETTE_SIZE * sizeof(ColorEntry));
data_parser.c:409:    uint32_t* pool_indices = malloc(loadedTextures * sizeof(uint32_t));
data_parser.c:411:    uint32_t** per_texture_palettes = malloc(loadedTextures * sizeof(uint32_t*));
data_parser.c:412:    uint32_t* per_texture_palette_sizes = malloc(loadedTextures * sizeof(uint32_t));
data_parser.c:413:    uint8_t* all_indices = malloc(max_total_pixels * sizeof(uint8_t));
data_parser.c:414:    uint32_t* index_offsets = malloc(loadedTextures * sizeof(uint32_t));
data_parser.c:417:        per_texture_palettes[i] = malloc(MAX_PALETTE_SIZE * sizeof(uint32_t));
data_parser.c:557:    modelVertexCounts   = calloc(loadedModels, sizeof(uint32_t));
data_parser.c:558:    modelTriangleCounts = calloc(loadedModels, sizeof(uint32_t));
data_parser.c:559:    modelVertices       = calloc(loadedModels, sizeof(float*));
data_parser.c:560:    modelTriangles      = calloc(loadedModels, sizeof(uint32_t*));
data_parser.c:561:    modelBounds         = calloc(loadedModels * BOUNDS_ATTRIBUTES_COUNT, sizeof(float));
data_parser.c:565:    int32_t* indexToParser = calloc(loadedModels, sizeof(int32_t));
data_parser.c:617:            modelVertices[i]  = calloc(vertexCount * VERTEX_ATTRIBUTES_COUNT,sizeof(float));
data_parser.c:618:            modelTriangles[i] = calloc(triCount * 3,sizeof(uint32_t));
data_parser.c:673:    vbos = calloc(loadedModels, sizeof(GLuint));
data_parser.c:674:    tbos = calloc(loadedModels, sizeof(GLuint));
data_parser.c:946:    modelTypeCountsOpaque = calloc(loadedModels,sizeof(uint16_t)); // Zero out all arrays and counters
data_parser.c:947:    modelTypeCountsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
data_parser.c:948:    modelTypeCountsTransparent = calloc(loadedModels,sizeof(uint16_t));
data_parser.c:949:    modelTypeOffsetsOpaque = calloc(loadedModels,sizeof(uint16_t));
data_parser.c:950:    modelTypeOffsetsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
data_parser.c:951:    modelTypeOffsetsTransparent = calloc(loadedModels,sizeof(uint16_t));
NOTE: Excluded miniaudio, stb_image ./External alloc calls
```

Binary static variable impacts:

```
❯ size ./voxen
   text    data     bss     dec     hex filename
2703444   34128 5476360 8213932  7d55ac ./voxen
```

Individual static variable impacts:

```
❯ nm -S --size-sort -r ./voxen | grep ' [BD] '
0000000000324420 0000000000200000 B precomputedVisibleCellsFromHere
000000000056a560 000000000009c400 B modelMatrices
0000000000606960 00000000000927c0 B instances
000000000029f100 0000000000038000 B fontPackedCharStopD
00000000002d7100 0000000000038000 B fontPackedChar
0000000000699360 0000000000023140 B _glfw
0000000000547ce0 0000000000014500 B lights
000000000052d5e0 000000000000ee00 B wav_sounds
000000000030f280 000000000000b400 B entities
000000000053d0c0 0000000000009c40 B eventJournal
00000000003197e0 0000000000009c40 B cellIndexForInstance
0000000000676dc0 0000000000007000 B uiImages
00000000005285c0 0000000000004000 B uiImageVertexData
00000000005245c0 0000000000004000 B textVertexData
000000000067f820 0000000000002710 B instanceIsLODArray
0000000000682180 0000000000002710 B dirtyInstances
000000000067de80 0000000000001900 B lightIsDynamic
0000000000523420 0000000000001000 B gridCellStates
0000000000661040 0000000000000fa0 B eventQueue
000000000029be00 0000000000000c58 D _glfwDefaultMappings
000000000053b3e0 0000000000000770 B mp3_sounds
000000000029b1e0 0000000000000640 D lightDirty
000000000053bb60 0000000000000520 B audio_engine
00000000006769a0 0000000000000400 B uiTextBuffer
0000000000676540 0000000000000400 B statusText
000000000029b840 0000000000000400 D consoleEntryText
000000000029df20 00000000000001c0 B playerMovement
0000000000524440 000000000000015e B keys
0000000000682060 0000000000000100 B global_modname
000000000029bc40 0000000000000090 D textColors
000000000029b160 0000000000000060 D orientationQuaternion
0000000000676960 0000000000000040 B uiOrthoProjection
0000000000681f60 0000000000000040 B shadowmapsPerspectiveProjection
0000000000681fa0 0000000000000040 B rasterPerspectiveProjection
0000000000682020 0000000000000034 B questData
000000000029b040 0000000000000030 D fontRangesStopD
000000000029b080 0000000000000030 D fontRanges
00000000002982c0 0000000000000030 D dataDeviceListener
0000000000297e40 0000000000000028 D wp_fractional_scale_manager_v1_interface
0000000000297960 0000000000000028 D _glfw_zxdg_toplevel_decoration_v1_interface
00000000002978a0 0000000000000028 D _glfw_zxdg_decoration_manager_v1_interface
0000000000297b80 0000000000000028 D _glfw_zwp_relative_pointer_v1_interface
0000000000297b00 0000000000000028 D _glfw_zwp_relative_pointer_manager_v1_interface
0000000000297c20 0000000000000028 D _glfw_zwp_pointer_constraints_v1_interface
0000000000297d00 0000000000000028 D _glfw_zwp_locked_pointer_v1_interface
0000000000298120 0000000000000028 D _glfw_zwp_idle_inhibitor_v1_interface
00000000002980c0 0000000000000028 D _glfw_zwp_idle_inhibit_manager_v1_interface
0000000000297dc0 0000000000000028 D _glfw_zwp_confined_pointer_v1_interface
0000000000297300 0000000000000028 D _glfw_xdg_wm_base_interface
0000000000297720 0000000000000028 D _glfw_xdg_toplevel_interface
0000000000297520 0000000000000028 D _glfw_xdg_surface_interface
0000000000297440 0000000000000028 D _glfw_xdg_positioner_interface
0000000000297820 0000000000000028 D _glfw_xdg_popup_interface
0000000000297f60 0000000000000028 D _glfw_xdg_activation_v1_interface
0000000000298040 0000000000000028 D _glfw_xdg_activation_token_v1_interface
0000000000297a80 0000000000000028 D _glfw_wp_viewport_interface
00000000002979e0 0000000000000028 D _glfw_wp_viewporter_interface
0000000000297ec0 0000000000000028 D _glfw_wp_fractional_scale_v1_interface
0000000000296f40 0000000000000028 D _glfw_wl_touch_interface
0000000000296aa0 0000000000000028 D _glfw_wl_surface_interface
0000000000297240 0000000000000028 D _glfw_wl_subsurface_interface
0000000000297160 0000000000000028 D _glfw_wl_subcompositor_interface
00000000002961c0 0000000000000028 D _glfw_wl_shm_pool_interface
0000000000296240 0000000000000028 D _glfw_wl_shm_interface
00000000002968e0 0000000000000028 D _glfw_wl_shell_surface_interface
0000000000296740 0000000000000028 D _glfw_wl_shell_interface
0000000000296b80 0000000000000028 D _glfw_wl_seat_interface
0000000000296040 0000000000000028 D _glfw_wl_registry_interface
00000000002970e0 0000000000000028 D _glfw_wl_region_interface
0000000000296d20 0000000000000028 D _glfw_wl_pointer_interface
0000000000297040 0000000000000028 D _glfw_wl_output_interface
0000000000296e20 0000000000000028 D _glfw_wl_keyboard_interface
0000000000295fa0 0000000000000028 D _glfw_wl_display_interface
0000000000296520 0000000000000028 D _glfw_wl_data_source_interface
00000000002963e0 0000000000000028 D _glfw_wl_data_offer_interface
00000000002966e0 0000000000000028 D _glfw_wl_data_device_manager_interface
0000000000296660 0000000000000028 D _glfw_wl_data_device_interface
0000000000296120 0000000000000028 D _glfw_wl_compositor_interface
00000000002960a0 0000000000000028 D _glfw_wl_callback_interface
00000000002962c0 0000000000000028 D _glfw_wl_buffer_interface
00000000007b35e0 0000000000000010 B _ZN6Assimp13DefaultLogger13s_pNullLoggerE
00000000003197a0 0000000000000010 B texture_parser
0000000000319790 0000000000000010 B model_parser
0000000000319770 0000000000000010 B lights_parser
0000000000319780 0000000000000010 B entity_parser
000000000029bcf0 0000000000000010 D cam_rotation
000000000029bda8 0000000000000008 D _ZN6Assimp13DefaultLogger9m_pLoggerE
00000000007b3498 0000000000000008 B window
000000000052c5c8 0000000000000008 B voxelLightListsRaw
000000000052c5c0 0000000000000008 B voxelLightListIndices
000000000029bd70 0000000000000008 D vertexShaderSource
00000000003196a8 0000000000000008 B vbos
0000000000319718 0000000000000008 B transparentTexture
00000000007b3468 0000000000000008 B time_PhysicsStep
000000000029bd80 0000000000000008 D textVertexShaderSource
0000000000319730 0000000000000008 B textureSizes
0000000000319740 0000000000000008 B texturePalettes
0000000000319748 0000000000000008 B texturePaletteOffsets
0000000000319750 0000000000000008 B textureOffsets
000000000029bd78 0000000000000008 D textFragmentShaderSource
00000000003196a0 0000000000000008 B tbos
000000000029dc48 0000000000000008 B stdout@GLIBC_2.2.5
000000000029de00 0000000000000008 B stderr@GLIBC_2.2.5
000000000029bd40 0000000000000008 D ssr_computeShader
000000000029bd60 0000000000000008 D shadowmapVertexShaderSource
000000000029bd38 0000000000000008 D shadowmaps_clear_computeShader
000000000029bd58 0000000000000008 D shadowmapFragmentShaderSource
00000000007b3470 0000000000000008 B screenshotTimeout
000000000029bd50 0000000000000008 D quadVertexShaderSource
000000000029bd48 0000000000000008 D quadFragmentShaderSource
00000000005245a0 0000000000000008 B physicsProcessingTime
00000000003196b8 0000000000000008 B modelVertices
0000000000319708 0000000000000008 B modelVertexCounts
00000000003196c8 0000000000000008 B modelTypeOffsetsTransparent
00000000003196d8 0000000000000008 B modelTypeOffsetsOpaque
00000000003196d0 0000000000000008 B modelTypeOffsetsDoubleSided
00000000003196e8 0000000000000008 B modelTypeCountsTransparent
00000000003196f8 0000000000000008 B modelTypeCountsOpaque
00000000003196f0 0000000000000008 B modelTypeCountsDoubleSided
00000000003196b0 0000000000000008 B modelTriangles
0000000000319700 0000000000000008 B modelTriangleCounts
0000000000319690 0000000000000008 B modelBounds
0000000000661fe8 0000000000000008 B manualLogName
000000000053c090 0000000000000008 B last_time
0000000000681fe8 0000000000000008 B last_mouse_y
0000000000681ff0 0000000000000008 B last_mouse_x
0000000000662000 0000000000000008 B lastJournalWriteTime
00000000007b3480 0000000000000008 B lastFrameSecCountTime
0000000000319710 0000000000000008 B image_data
000000000029ddd0 0000000000000008 B __glewVertexAttribFormat
000000000029de60 0000000000000008 B __glewVertexAttribBinding
000000000029de40 0000000000000008 B __glewVertexArrayVertexBuffer
000000000029dc40 0000000000000008 B __glewVertexArrayAttribFormat
000000000029de30 0000000000000008 B __glewVertexArrayAttribBinding
000000000029dea0 0000000000000008 B __glewUseProgram
000000000029def8 0000000000000008 B __glewUnmapBuffer
000000000029dd68 0000000000000008 B __glewUniformMatrix4fv
000000000029dd20 0000000000000008 B __glewUniformMatrix3fv
000000000029dde8 0000000000000008 B __glewUniform3f
000000000029dce8 0000000000000008 B __glewUniform1ui
000000000029dd90 0000000000000008 B __glewUniform1i
000000000029dd08 0000000000000008 B __glewUniform1f
000000000029ddf8 0000000000000008 B __glewTextureSubImage2D
000000000029ddc0 0000000000000008 B __glewTextureStorage2D
000000000029dc68 0000000000000008 B __glewTextureParameteri
000000000029dcc8 0000000000000008 B __glewShaderSource
000000000029dd70 0000000000000008 B __glewProgramUniformMatrix4fv
000000000029dc70 0000000000000008 B __glewProgramUniform4f
000000000029de70 0000000000000008 B __glewProgramUniform3f
000000000029dd00 0000000000000008 B __glewProgramUniform2f
000000000029de50 0000000000000008 B __glewProgramUniform1ui
000000000029dd60 0000000000000008 B __glewProgramUniform1i
000000000029dc60 0000000000000008 B __glewProgramUniform1f
000000000029dcb8 0000000000000008 B __glewNamedBufferData
000000000029dcb0 0000000000000008 B __glewMinSampleShading
000000000029dda0 0000000000000008 B __glewMemoryBarrier
000000000029dc98 0000000000000008 B __glewMapBufferRange
000000000029de90 0000000000000008 B __glewLinkProgram
000000000029dc50 0000000000000008 B __glewGetUniformLocation
000000000029dc80 0000000000000008 B __glewGetShaderiv
000000000029dd10 0000000000000008 B __glewGetShaderInfoLog
000000000029dea8 0000000000000008 B __glewGetProgramiv
000000000029dce0 0000000000000008 B __glewGetProgramInfoLog
000000000029de98 0000000000000008 B __glewGenVertexArrays
000000000029dee0 0000000000000008 B __glewGenFramebuffers
000000000029dcc0 0000000000000008 B __glewGenBuffers
000000000029ddc8 0000000000000008 B __glewFramebufferTexture2D
000000000029df10 0000000000000008 B __glewEnableVertexAttribArray
000000000029ddf0 0000000000000008 B __glewEnableVertexArrayAttrib
000000000029def0 0000000000000008 B __glewDrawBuffers
000000000029dd40 0000000000000008 B __glewDispatchCompute
000000000029de08 0000000000000008 B __glewDeleteShader
000000000029de78 0000000000000008 B __glewDeleteBuffers
000000000029dd88 0000000000000008 B __glewCreateVertexArrays
000000000029dc88 0000000000000008 B __glewCreateTextures
000000000029dcd0 0000000000000008 B __glewCreateShader
000000000029dca0 0000000000000008 B __glewCreateProgram
000000000029de88 0000000000000008 B __glewCreateBuffers
000000000029df08 0000000000000008 B __glewCopyBufferSubData
000000000029dc90 0000000000000008 B __glewCompileShader
000000000029df00 0000000000000008 B __glewCheckFramebufferStatus
000000000029de38 0000000000000008 B __glewBufferData
000000000029de80 0000000000000008 B __glewBindVertexBuffer
000000000029dcf0 0000000000000008 B __glewBindVertexArray
000000000029dd78 0000000000000008 B __glewBindTextureUnit
000000000029ded0 0000000000000008 B __glewBindImageTexture
000000000029dec8 0000000000000008 B __glewBindFramebuffer
000000000029dc58 0000000000000008 B __glewBindBufferBase
000000000029dde0 0000000000000008 B __glewBindBuffer
000000000029dec0 0000000000000008 B __glewAttachShader
000000000029de20 0000000000000008 B __glewActiveTexture
000000000029bd68 0000000000000008 D fragmentShaderTraditional
000000000029a360 0000000000000008 D fractionalScaleListener
0000000000319720 0000000000000008 B doubleSidedTexture
000000000053c088 0000000000000008 B current_time
000000000053c080 0000000000000008 B cpuTime
00000000007b3488 0000000000000008 B console_log_file
0000000000661ff0 0000000000000008 B activeLogFile
000000000029bd20 0000000000000004 D worstFPS
000000000067ddf0 0000000000000004 B worldMin_zLoc_imageBlit
000000000067f7d8 0000000000000004 B worldMin_zLoc_chunk
00000000003197c0 0000000000000004 B worldMin_z
000000000067ddf4 0000000000000004 B worldMin_xLoc_imageBlit
000000000067f7dc 0000000000000004 B worldMin_xLoc_chunk
00000000003197c4 0000000000000004 B worldMin_x
000000000052c5d0 0000000000000004 B wav_count
000000000067f7a0 0000000000000004 B viewProjMatrixLoc_shadowmaps
000000000067f800 0000000000000004 B viewProjLoc_chunk
000000000067de5c 0000000000000004 B viewProjectionLoc_ssr
000000000067ddec 0000000000000004 B viewProjectionLoc_imageBlit
0000000000681f30 0000000000000004 B verticesRenderedThisFrame
000000000067f804 0000000000000004 B vao_chunk
000000000067f7b4 0000000000000004 B unlitLoc_chunk
0000000000676da0 0000000000000004 B uiImageVBO
0000000000676da4 0000000000000004 B uiImageVAO
0000000000681f38 0000000000000004 B uiImageDrawCallsRenderedThisFrame
0000000000676da8 0000000000000004 B uiImageCount
000000000031973c 0000000000000004 B totalPixels
0000000000319738 0000000000000004 B totalPaletteColors
000000000067de04 0000000000000004 B timeValLoc_imageBlit
000000000067ddd8 0000000000000004 B textVBO
000000000067dddc 0000000000000004 B textVAO
0000000000319764 0000000000000004 B textureSizesID
000000000031975c 0000000000000004 B texturePalettesID
0000000000319758 0000000000000004 B texturePaletteOffsetsID
0000000000319760 0000000000000004 B textureOffsetsID
000000000067ddcc 0000000000000004 B textTextureLoc_text
000000000067dde0 0000000000000004 B textShaderProgram
0000000000681f3c 0000000000000004 B textDrawCallsRenderedThisFrame
000000000067ddd0 0000000000000004 B textColorLoc_text
000000000067de40 0000000000000004 B texLoc_quadblit
000000000067f79c 0000000000000004 B texIndexLoc_shadowmaps
000000000067f7f8 0000000000000004 B texIndexLoc_chunk
000000000067ddc8 0000000000000004 B texelSizeLoc_text
000000000029bd2c 0000000000000004 D stbi_write_tga_with_rle
000000000029bd30 0000000000000004 D stbi_write_png_compression_level
000000000029bd28 0000000000000004 D stbi_write_force_png_filter
000000000029b820 0000000000000004 D statusTextLengthWithoutNullTerminator
0000000000676524 0000000000000004 B statusTextDecayFinished
000000000067de1c 0000000000000004 B stationShieldVisibleLoc_imageBlit
000000000067de68 0000000000000004 B ssrShaderProgram
000000000067f78c 0000000000000004 B ssbo_indexBaseLoc_shadowmaps
000000000067de28 0000000000000004 B skyVisibleLoc_imageBlit
000000000067ddfc 0000000000000004 B shadowsSettingLoc_imageBlit
000000000067f7bc 0000000000000004 B shadowsEnabledLoc_chunk
000000000067f7a8 0000000000000004 B shadowmapsShaderProgram
000000000067f784 0000000000000004 B shadowMapSSBO
000000000067f788 0000000000000004 B shadowmapSizeLoc_shadowmaps
000000000067ddf8 0000000000000004 B shadowmapSizeLoc_imageBlit
000000000067f7c4 0000000000000004 B shadowmapSizeLoc_chunk
000000000067de50 0000000000000004 B shadowmapsClearShaderProgram
000000000067f7ac 0000000000000004 B shadowFBO
0000000000681f34 0000000000000004 B shadowDrawCallsRenderedThisFrame
000000000067f7b0 0000000000000004 B shadowCubeMap
000000000067de64 0000000000000004 B screenWidthLoc_ssr
000000000067de34 0000000000000004 B screenWidthLoc_imageBlit
000000000067f7e4 0000000000000004 B screenWidthLoc_chunk
000000000067de60 0000000000000004 B screenHeightLoc_ssr
000000000067de30 0000000000000004 B screenHeightLoc_imageBlit
000000000067f7e0 0000000000000004 B screenHeightLoc_chunk
000000000067de18 0000000000000004 B reflectionsEnabledLoc_imageBlit
000000000067f7c0 0000000000000004 B reflectionsEnabledLoc_chunk
000000000029b144 0000000000000004 D random_range_rng
000000000067de44 0000000000000004 B quadVBO
000000000067de48 0000000000000004 B quadVAO
000000000067ddd4 0000000000000004 B projectionLoc_text
000000000067de24 0000000000000004 B planetaryBodiesVisibleLoc_imageBlit
0000000000682054 0000000000000004 B pauseRelativeTime
000000000067de54 0000000000000004 B outputImageLoc_ssr
000000000067de2c 0000000000000004 B outputImageLoc_imageBlit
000000000067f80c 0000000000000004 B outputImageID
000000000030e108 0000000000000004 B numPackedGlyphsStopD
000000000030e10c 0000000000000004 B numPackedGlyphs
000000000029b020 0000000000000004 D numFontRanges
000000000067f794 0000000000000004 B normInstanceIndexLoc_shadowmaps
000000000067f7e8 0000000000000004 B normInstanceIndexLoc_chunk
000000000029b0f0 0000000000000004 D move_speed
000000000029b0e8 0000000000000004 D mouse_sensitivity
000000000067f7a4 0000000000000004 B modelMatrixLoc_shadowmaps
0000000000319698 0000000000000004 B modelBoundsID
0000000000662008 0000000000000004 B maxEventCount_debug
000000000067f7fc 0000000000000004 B matrixLoc_chunk
0000000000682160 0000000000000004 B matricesBuffer
000000000029bd14 0000000000000004 D lodRangeSqrd
0000000000676520 0000000000000004 B lightsID
000000000067f790 0000000000000004 B lightPosLoc_shadowmaps
00000000007b347c 0000000000000004 B lastFrameSecCount
000000000067f7b8 0000000000000004 B isUILoc_chunk
000000000067dde4 0000000000000004 B invViewRotLoc_imageBlit
0000000000682164 0000000000000004 B instancesBuffer
000000000067f814 0000000000000004 B inputWorldPosID
000000000067f81c 0000000000000004 B inputImageID
000000000067f818 0000000000000004 B inputDepthID
000000000067de4c 0000000000000004 B imageBlitShaderProgram
000000000067de20 0000000000000004 B groveShieldVisibleLoc_imageBlit
000000000067f798 0000000000000004 B glowSpecIndexLoc_shadowmaps
000000000067f7ec 0000000000000004 B glowSpecIndexLoc_chunk
0000000000661ff8 0000000000000004 B globalFrameNum
000000000029b0b0 0000000000000004 D genericTextWidthFacStopD
000000000029b0b8 0000000000000004 D genericTextWidthFac
000000000029b0b4 0000000000000004 D genericTextHeightFacStopD
000000000029b0bc 0000000000000004 D genericTextHeightFac
000000000067f810 0000000000000004 B gBufferFBO
00000000007b3478 0000000000000004 B framesPerLastSecond
000000000067de0c 0000000000000004 B fovLoc_imageBlit
000000000067ddc4 0000000000000004 B fontTypeLoc_text
000000000030e100 0000000000000004 B fontAtlasTexStopD
000000000030e104 0000000000000004 B fontAtlasTex
000000000067f7d0 0000000000000004 B fogColorRLoc_chunk
000000000029bce0 0000000000000004 D fogColorR
000000000067f7cc 0000000000000004 B fogColorGLoc_chunk
000000000029bcdc 0000000000000004 D fogColorG
000000000067f7c8 0000000000000004 B fogColorBLoc_chunk
000000000029bcd8 0000000000000004 D fogColorB
000000000029e0e0 0000000000000004 B fixedNumberAdvanceWidthStopD
000000000029e0e4 0000000000000004 B fixedNumberAdvanceWidth
000000000053c098 0000000000000004 B eventQueueEnd
000000000053c0a0 0000000000000004 B eventJournalIndex
000000000053c09c 0000000000000004 B eventIndex
000000000030e264 0000000000000004 B entityCount
0000000000681f40 0000000000000004 B drawCallsRenderedThisFrame
000000000067de3c 0000000000000004 B debugViewLoc_quadblit
000000000067f7f4 0000000000000004 B debugViewLoc_chunk
0000000000681fe4 0000000000000004 B debugView
000000000067de38 0000000000000004 B debugValueLoc_quadblit
000000000067f7f0 0000000000000004 B debugValueLoc_chunk
0000000000681fe0 0000000000000004 B debugValue
000000000029bcd0 0000000000000004 D cursorPosition_y
000000000029bcd4 0000000000000004 D cursorPosition_x
0000000000676944 0000000000000004 B currentEntryLength
0000000000319768 0000000000000004 B colorBufferID
000000000067f808 0000000000000004 B chunkShaderProgram
000000000029bd04 0000000000000004 D cam_z
000000000029bd00 0000000000000004 D cam_yaw
000000000029bd08 0000000000000004 D cam_y
000000000029bd0c 0000000000000004 D cam_x
000000000067de08 0000000000000004 B camRotLoc_imageBlit
0000000000682010 0000000000000004 B cam_roll
0000000000681ff8 0000000000000004 B cam_rightz
0000000000681ffc 0000000000000004 B cam_righty
0000000000682000 0000000000000004 B cam_rightx
000000000067de58 0000000000000004 B camPosLoc_ssr
000000000067dde8 0000000000000004 B camPosLoc_imageBlit
000000000067f7d4 0000000000000004 B camPosLoc_chunk
0000000000682014 0000000000000004 B cam_pitch
000000000029bcec 0000000000000004 D cam_fov
0000000000682004 0000000000000004 B cam_forwardz
0000000000682008 0000000000000004 B cam_forwardy
000000000068200c 0000000000000004 B cam_forwardx
000000000067de10 0000000000000004 B brightnessSettingLoc_imageBlit
000000000067de00 0000000000000004 B aspectLoc_imageBlit
000000000029bce8 0000000000000004 D aspect3D
000000000029bce4 0000000000000004 D aspect2D
000000000067de14 0000000000000004 B aaEnabledLoc_imageBlit
0000000000319680 0000000000000002 B transparentInstancesHead
000000000029b0da 0000000000000002 D startOfTransparentInstances
000000000029b0dc 0000000000000002 D startOfDoubleSidedInstances
000000000029bd26 0000000000000002 D screen_width
000000000029bd24 0000000000000002 D screen_height
000000000031968a 0000000000000002 B renderableCount
00000000003197ca 0000000000000002 B playerCellIdx_z
00000000003197cc 0000000000000002 B playerCellIdx_y
00000000003197ce 0000000000000002 B playerCellIdx_x
00000000003197d0 0000000000000002 B playerCellIdx
000000000030e260 0000000000000002 B physHead
00000000003196c0 0000000000000002 B opaqueInstancesHead
00000000003197c8 0000000000000002 B numCellsVisible
0000000000524420 0000000000000002 B mouse_y
0000000000524422 0000000000000002 B mouse_x
0000000000319728 0000000000000002 B loadedTextures
0000000000319686 0000000000000002 B loadedModels
0000000000319684 0000000000000002 B loadedLights
0000000000319688 0000000000000002 B loadedInstances
00000000003196e0 0000000000000002 B invalidModelIndexCount
0000000000319682 0000000000000002 B doubleSidedInstancesHead
000000000052459e 0000000000000001 B window_has_focus
000000000029bd10 0000000000000001 D startLevel
000000000067f780 0000000000000001 B shadowMapsRendered
00000000007b3460 0000000000000001 B settings_Vsync
000000000029bd18 0000000000000001 D settings_VolumeMusic
000000000029bd1b 0000000000000001 D settings_Shadows
000000000029bd1c 0000000000000001 D settings_Reflections
000000000029bd19 0000000000000001 D settings_Brightness
000000000029bd1a 0000000000000001 D settings_AntiAliasing
000000000029bd11 0000000000000001 D numLevels
000000000029b0ec 0000000000000001 D noclip
0000000000682058 0000000000000001 B menuActive
0000000000661fe0 0000000000000001 B log_playback
000000000029b1c0 0000000000000001 D journalFirstWrite
00000000007b3490 0000000000000001 B inventoryMode
000000000068205b 0000000000000001 B global_modIsCitadel
000000000029dd80 0000000000000001 B glewExperimental
0000000000682059 0000000000000001 B gamePaused
000000000067ddc0 0000000000000001 B cursorVisible
000000000068205a 0000000000000001 B currentLevel
0000000000676940 0000000000000001 B consoleActive
```

cloc --by-file --exclude-dir=temp_build,.git,Audio,Data,Fonts,Models,Screenshots,Scripts,Shaders,Textures,Tools,External ./
      31 text files.
      31 unique files.                              
       5 files ignored.

github.com/AlDanial/cloc v 1.90  T=0.02 s (1086.0 files/s, 439666.1 lines/s)
----------------------------------------------------------------------------------
File                                           blank        comment           code
----------------------------------------------------------------------------------
./voxen.c                                        172            151           1476
./entity.c                                        73             12            714
./dynamic_culling.c                               89             30            588
./physics.c                                       69            116            542
./voxen.h                                         40             57            461
./data_fonts.c                                    49              8            380
./console.c                                       20             80            372
./data_models.c                                   37              3            306
./data_text.c                                     32              3            303
./citadel_enumerations.h                          28             28            297
./data_parser.c                                   37             11            263
./data_textures.c                                 22              5            214
./event.c                                         36             16            187
./helpers.c                                       25              5            179
./input.c                                         29              5            167
./audio.c                                         26              7            140
./entity.h                                        13              6             95
./build.sh                                         5              3             53
./event.h                                          6              4             48
./matvecquat.c                                     4              3             41
./vmath.h                                          2              0             38
./.github/workflows/main.yml                       0              0             28
./matvecquat.h                                     0              0             24
./test_voxen_restart_x_times.sh                    2              0             17
./patches.c                                        2             91             15
----------------------------------------------------------------------------------
SUM:                                             818            644           6948
----------------------------------------------------------------------------------
