# Voxen

## The Voxel Lit Open Source Engine

This is a pure C (C11) rendering engine with a focus on high performance and 
simplicity for first person shooter games.  This project was developed with 
Citadel: The System Shock Fan Remake in mind but should be reasonably
modifiable for anything, being based on FOSS MIT licensing and principles.
Please take this and make it your own for your own projects.

The "Voxel Lit" portion of Voxen is in the representation of lighting 
information using a voxel format that is an invisible layer of data
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
straightforward.  Shaders are embedded in the engine.

Using OpenGL 4.3+, this engine attempts to achieve maximum compatibility and maximum 
performance with minimal footprint.  Heavy use of SSBOs is made though this is still 
compatible with old hardware and GL drivers from 15yrs ago; further very few GL extensions 
are used to further widen compatibility.  Careful handling of CPU to GPU transfers 
is made to minimize VRAM and to prevent naughty GL drivers duplicating that VRAM into 
the CPU RAM space which is also kept minimal.

All texture and model data is loaded from disk directly for ease of development
and full mod support by design.  Any intermediate format is internal to the engine.

Minimizing hierarchical layers and leveraging sensibly named globals to cut out
fluff and overhead is important.  Minimal dependencies and leveraging tried and
true systems is important.

## Supported Platforms

- **Linux (64-bit)**: Primarily Debian-based distros (e.g., Kubuntu, Xubuntu) with X11. Wayland support is not intentional.  YMMV via XWayland.
- **Windows (64-bit)**: Supports Windows 7+.  Tested on Windows 10 and 11.
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

### Table of Contents (Kind of):

```
❯ ls *.* ./Shaders/*.glsl ./Shaders/*.compute | grep -vE 'README.md|builds.csv|voxen.exe|voxen.log|build.sh|Citadel.pdb|*.bin' | xargs perl -MList::Util=max -lne '$first{$ARGV} //= $_; $count{$ARGV} = $.; if(eof){$total += $.; $. = 0;} END { $max = max map {length} keys %first; printf "99999999 %7d total\n", $total; printf "%8d %-${max}s  %s\n", $count{$_}, $_, $first{$_} for keys %first }' 2>/dev/null | sort -nr | head -n 51 | sed 's/^99999999 //'
  13450 total
    1352 entity.c                            // entity.c - Entity Definitions and Save Load System for levels and savegames
    1300 models.c                            // models.c - 3D Models Loading System, Animation, Convex Edge Adjacency, Mesh Optimization
    1199 citadel.c                           // citadel.c - Game logic.
    1134 voxen.c                             // voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Core renderer.  OS Shim Layer.
     932 physics.c                           // physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
     925 audio.c                             // audio.c - Audio System supporting .mp3 + .wav filetypes only, uses Windows WASAPI or Linux ALSA("default" to work on PulseAudio and PipeWire or ALSA+dmix, w/ raw ioctl fallback).  Mixes synthesized sounds/music.
     887 ai.c                                // ai.c - AI logic control for NPC's enemies in the game.
     821 winput.c                            // winput.c - WinSys Windowing System and Input System interfacing with the OS.
     537 common.h                            // common.h - Shared items between engine and gamecode (e.g. enums)
     529 text.c                              // text.c - Text and Font Rendering/Loading System
     494 textures.c                          // textures.c - 2D Texture Loading System
     399 synth.c                             // synth.c — Procedural Audio Engine
     391 ./Shaders/composite_frag.glsl       // composite.glsl - Composite rendered view + UI overlay, custom AA, VHS blur (subtle, magic!), SSR with tapped blur, Procedural skybox w/ stars + saturn + sun + station shield (if on!) that rotate, berserk color hallucinations, EMP screen rolling, fog, infrared grayscale.
     387 credits.h                           // credits.h - Credits for Citadel: The System Shock Fan Remake, salt the fries!
     372 weapons.c                           // weapons.c - Weapon System
     361 ui.c                                // ui.c - User Interface(UI) aka HUD
     307 biomonitor.c                        // biomonotor.c - Biomonitor Graph and Text displays.
     288 culling.c                           // culling.c - XZ 2D World Grid Cell Culling System 64x64 matching System Shock 1.
     229 ./Shaders/chunk_frag.glsl           // chunk_frag.glsl: Generic shader for all world objects
     229 lib.c                               // lib.c - LibC replacement functions and other misc helpers.
      83 ./Shaders/voxels.compute            // voxels.compute - Compute shader for determining light lists for voxels and updating voxel tables 
      70 ./Shaders/ssr.compute               // ssr.compute - Compute shader for Screen Space Reflections 
      54 ./Shaders/text_frag.glsl            // text_frag.glsl - Text Fragment shader, supports both SystemShock font with black border around every character and StopD font with 3d drop shadow and top edge highlights
      33 ./Shaders/shadowmap_frag.glsl       // shadowmap_frag.glsl - Shadowmap Fragment Shader, uses alpha cutout on textures for {fence style shadows.  Writes into SSBO via atomicMin on typecast float dist with * 100000 scaling.
      32 ./Shaders/depth_prepass.glsl        // depth_prepass.glsl: Renders all opaque + cutout objects prior to main forward+ pass
      29 ./Shaders/ui_frag.glsl              // ui_frag.glsl: Generic shader for unlit textured UI images (mostly cutouts)
      18 ./Shaders/chunk_vert.glsl           // chunk_vert.glsl: Generic shader for unlit textured surfaces (all world geometry, items, enemies, doors, etc., without transparency for first pass prior to lighting.
      15 ./Shaders/depth_prepass_vert.glsl   // depth_prepass_vert.glsl: vertex shader for depth prepass
      10 ./Shaders/shadowmap_vert.glsl       // shadowmap_vert.glsl - Shadowmap Vertex shader
       6 ./Shaders/ui_vert.glsl              // ui_vert.glsl: Generic shader for unlit textured surfaces (all world geometry, items, enemies, doors, etc., without transparency for first pass prior to lighting.
       6 ./Shaders/text_vert.glsl            // text_vert.glsl - Text Vertex Shader
       6 ./Shaders/shadowmaps_clear.compute  // shadowmaps_clear.compute - Compute shader for clearing the distances for shadowmaps in the SSBO to 0xFFFFFFFF
       6 ./Shaders/debugunlit_vert.glsl      // debugunlit_vert.glsl - Wireline Vertex Shader
       5 ./Shaders/composite_vert.glsl       // imageblit.glsl - Full screen quad unlit textured for presenting image buffers such as results from compute shaders, image effects, post-processing, etc..
       4 ./Shaders/debugunlit_frag.glsl      // debugunlit_frag.glsl - Wireline Fragment Shader, colored wirelines used for physics wireframe view of colliders, velocity debug vectors, angular velocity debug vector and arc for orientation, raycast debug vector, and weapon lasers  
```

### Install Footprint
Size key: b = bit, n = nibble, B = byte, k = kilobyte, m = megabyte, g = we don't go there, t = haha
```
 85,644k Audio/
 51,726k Textures/
 52,321k Models/
 17,170k Data/
  1,815k Fonts/
  1,589k loose files (binaries, autosplitter notes)
      0k Screenshots/
210,265k
```
Citadel.7z (LZMA Max Compressed) sitting at 116.8mb

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
Compiling voxen, total iterations today 228 (2026-07-25)...
Built engine as game in 127 ms
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
Entity size: 900
Loading    5 fonts... took 0.123607973 s
Loading textures (1989) ... total palette colors: 120019, total pixels: 38000755... took 0.162262 secs
Loading   models (5989) ... vertices: 11975224, tris: 11351562, 0.643557074 secs
Loading new game...
Entity counts::0:3831|1:5937|2:5076|3:3274|4:3389|5:4270|6:5773|7:6081|8:4756|9:4469|10:1721|11:1608|12:1314|13:8416
 Light counts::0:1289|1:1059|2:1138|3:490|4:870|5:1149|6:1225|7:2016|8:903|9:1526|10:719|11:525|12:487|13:3
Load all levels... took 0.189141671 secs
Switched to Level 1
Culling found 1178 open cells... took 0.090702681 secs
Generating edge adjacency lists for 28 convex meshes...took 0.002452750 secs
Game Initialized in 1.631655835 secs
Loading new game...
Entity counts::0:3831|1:5937|2:5076|3:3274|4:3389|5:4270|6:5773|7:6081|8:4756|9:4469|10:1721|11:1608|12:1314|13:8416
 Light counts::0:1289|1:1059|2:1138|3:490|4:870|5:1149|6:1225|7:2016|8:903|9:1526|10:719|11:525|12:487|13:3
Load all levels... took 0.156872861 secs
Switched to Level 1
Culling found 1178 open cells... took 0.089528178 secs
Generating edge adjacency lists for 28 convex meshes...took 0.002449103 secs
Player named "" started the game!
```

Log output from one run with lib.c DebugRAM() internals uncommented:

```
Compiling voxen, total iterations today 229 (2026-07-25)...
Built engine as game in 363 ms
Mem at program start: Heap 0b(0KB|0.00MB), USS 487424b(476KB|0.46MB)
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
Entity size: 900
Mem at start font load: Heap 8282112b(8088KB|7.90MB), USS 16257024b(15876KB|15.50MB)
Loading    5 fonts...Mem at after font load: Heap 8282112b(8088KB|7.90MB), USS 95985664b(93736KB|91.54MB)
 took 0.147640197 s
Loading textures (1989) ... total palette colors: 120019, total pixels: 38000755... took 0.127588 secs
Mem at After LoadTextures and after deallocation: Heap 10797056b(10544KB|10.30MB), USS 140636160b(137340KB|134.12MB)
Loading   models (5989) ... vertices: 11975224, tris: 11351562, 0.665408164 secs
Mem at After LoadModels: Heap 281055232b(274468KB|268.04MB), USS 496721920b(485080KB|473.71MB)
Loading new game...
Mem at start of LoadAllLevels: Heap 281055232b(274468KB|268.04MB), USS 497909760b(486240KB|474.84MB)
Entity counts::0:3831|1:5937|2:5076|3:3274|4:3389|5:4270|6:5773|7:6081|8:4756|9:4469|10:1721|11:1608|12:1314|13:8416
 Light counts::0:1289|1:1059|2:1138|3:490|4:870|5:1149|6:1225|7:2016|8:903|9:1526|10:719|11:525|12:487|13:3
Load all levels... took 0.196210112 secs
Mem at end of LoadAllLevels: Heap 281600000b(275000KB|268.55MB), USS 634777600b(619900KB|605.37MB)
Mem at start of LoadLevel: Heap 281600000b(275000KB|268.55MB), USS 634777600b(619900KB|605.37MB)
Switched to Level 1
Culling found 1178 open cells... took 0.091125611 secs
Mem at end of LoadLevel: Heap 281600000b(275000KB|268.55MB), USS 637509632b(622568KB|607.98MB)
Generating edge adjacency lists for 28 convex meshes...took 0.002214804 secs
Mem at InitializeEnvironment end: Heap 281600000b(275000KB|268.55MB), USS 637845504b(622896KB|608.30MB)
Game Initialized in 1.726753756 secs
Mem at frame 4: Heap 282288128b(275672KB|269.21MB), USS 643543040b(628460KB|613.73MB)
Mem at frame 100: Heap 282288128b(275672KB|269.21MB), USS 639348736b(624364KB|609.73MB)
Mem at frame 200: Heap 282288128b(275672KB|269.21MB), USS 639348736b(624364KB|609.73MB)
Mem at frame 500: Heap 282288128b(275672KB|269.21MB), USS 639352832b(624368KB|609.73MB)
Mem at frame 1000: Heap 282288128b(275672KB|269.21MB), USS 639356928b(624372KB|609.74MB)
Loading new game...
Mem at start of LoadAllLevels: Heap 282288128b(275672KB|269.21MB), USS 639361024b(624376KB|609.74MB)
Entity counts::0:3831|1:5937|2:5076|3:3274|4:3389|5:4270|6:5773|7:6081|8:4756|9:4469|10:1721|11:1608|12:1314|13:8416
 Light counts::0:1289|1:1059|2:1138|3:490|4:870|5:1149|6:1225|7:2016|8:903|9:1526|10:719|11:525|12:487|13:3
Load all levels... took 0.167074728 secs
Mem at end of LoadAllLevels: Heap 282288128b(275672KB|269.21MB), USS 639361024b(624376KB|609.74MB)
Mem at start of LoadLevel: Heap 282288128b(275672KB|269.21MB), USS 639361024b(624376KB|609.74MB)
Switched to Level 1
Culling found 1178 open cells... took 0.089577006 secs
Mem at end of LoadLevel: Heap 282288128b(275672KB|269.21MB), USS 639295488b(624312KB|609.68MB)
Generating edge adjacency lists for 28 convex meshes...took 0.002515498 secs
Player named "" started the game!
```

```
❯ grep -rIn "Alloc"
voxen.c:11:    typedef struct { union { u32 dwOemId; struct { u16 wProcessorArchitecture,wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; u32 dwPageSize; void* lpMinimumApplicationAddress,*lpMaximumApplicationAddress; u64 dwActiveProcessorMask; u32 dwNumberOfProcessors,dwProcessorType,dwAllocationGranularity; u16 wProcessorLevel,wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
voxen.c:12:    DLL_IMP void* WINAPI CreateFileMappingA(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,const char*); DLL_IMP i32 WINAPI VirtualFree(void*,u64,u32);    DLL_IMP void* WINAPI VirtualAlloc(void*,u64,u32,u32);         DLL_IMP i32 WINAPI ReadFile(void*,void*,u32,u32*,LPOVERLAPPED);    DLL_IMP i32 WINAPI GetFileSizeEx(void*,PLARGE_INTEGER);          DLL_IMP i32 WINAPI UnmapViewOfFile(void*); DLL_IMP FARPROC WINAPI GetProcAddress(HINSTANCE,const char*);
voxen.c:17:    void* OS_AllocateRAM(size_t l,i32 p,i32 f,FHandle fd) { (void)f; if (fd==(void*)-1) return VirtualAlloc(NULL,l,0x3000,(p&2)?4:2); void* m = CreateFileMappingW(fd,NULL,(p&2) ? 4 : 2,(u32)(l>>32),(u32)l,NULL); void* r=MapViewOfFileEx(m,(p&2)?2:4,0,0,l,NULL); return CloseHandle(m),r;}    
voxen.c:24:    void* OS_AllocateFileBackedRAMReadonly(size_t s,FHandle fd, char* path) { void* m; void* r; return(fd==(void*)-1||!s||!(m=CreateFileMappingA(fd,NULL,2,0,0,NULL))) ? DualLogError("CreateFileMappingA failed for %s\n",path),NULL : (r=MapViewOfFile(m,4,0,0,s)) ? (CloseHandle(m),r) : (DualLogError("Failed to allocate %s\n",path),CloseHandle(m),NULL);}
voxen.c:30:    void* __stdcall GetProcessHeap(); void* __stdcall HeapAlloc(void* hHeap, u32 dwFlags, size_t dwBytes); i32 __stdcall HeapFree(void* hHeap, u32 dwFlags, void* lpMem); void __stdcall Sleep(u32 dwMilliseconds); u32 __stdcall WaitForSingleObject(void* hHandle, u32 dwMilliseconds);
voxen.c:34:    int OS_ThreadCreate(OS_Thread* out, void*(*fn)(void*), void* arg) { void** b=(void**)HeapAlloc(GetProcessHeap(),0,2 * sizeof(void*)); b[0]=(void*)fn; b[1]=arg; out->handle=CreateThread(NULL,THRSTACKSZ,thrtramp,b,0,NULL); if(!out->handle){HeapFree(GetProcessHeap(),0,b); return -1;} return 0; }
voxen.c:45:    void* OS_AllocateRAM(size_t len, i32 prot, i32 flags, FHandle fd) { long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
voxen.c:49:    void* OS_AllocateFileBackedRAMReadonly(size_t s, FHandle fd, char* path) { void* r=OS_AllocateRAM(s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
voxen.c:62:    int OS_ThreadCreate(OS_Thread* out, void*(*fn)(void*), void* arg) { void* base = OS_AllocateRAM(THRSTACKSZ,0x1|0x2,0x02|0x20,INVALID_FHANDLE); if (!base || base == (void*)-1) return -1; struct OS_ThreadHead* head = (struct OS_ThreadHead*)((char*)base + THRSTACKSZ) - 1; head->trampoline = thrtramp; head->fn=fn; head->arg=arg; head->join_futex=0; head->_pad=0; long tid = OS_CloneSyscall(head); if(tid < 0){OS_Free(base,THRSTACKSZ); return (int)tid;} out->head=head; out->stack_base=base; return 0; } // Multithreading taken from https://github.com/skeeto/scratch/blob/master/misc/stack_head.c Ref: https://nullprogram.com/blog/2023/03/23/ This is free and unencumbered software released into the public domain.
voxen.c:67:void* OS_Alloc(size_t amount) { return OS_AllocateRAM(amount,0x1|0x2,0x02|0x20,INVALID_FHANDLE); } 
voxen.c:68:INLINE void* OS_Calloc(size_t amount, size_t count) { return OS_Alloc(amount * count); }
voxen.c:70:void* OS_OpenAndAllocateFileBufferReadonly(const char* p,FHandle* f,int* s) {void* r;return((*f=OS_OpenReadonly(p))==(FHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
voxen.c:71:void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (mcpy(n,old,olds),OS_Free(old,olds),n) : 0; }
lib.c:135:    u8* pixels = OS_Alloc(w * h * 4 * sizeof(char));
entity.c:1208:    FHandle fh; int fsize; void* fbuf = OS_OpenAndAllocateFileBufferReadonly(filename, &fh, &fsize); if (!fbuf) { OS_Exit(1); }
entity.c:1344:    size_t sz=sizeof(GlobalContext); size_t maxCompSize=GetMaxCompressedSize(sz); u8* b=(u8*)OS_Alloc(maxCompSize); size_t finalCompSize=VoidSquasher((const u8*)&World,sz,b,maxCompSize);
entity.c:1356:    u8* b = (u8*)OS_Alloc(header.compressedSize);
textures.c:11:void PngArenaInit(PngArena* arena) { if (!arena->base) { arena->base = OS_Alloc(16777216); arena->cursor = arena->base; arena->end = arena->base + 16777216; } }
textures.c:12:void* PngArenaAlloc(PngArena* a, size_t s) { if(!a->base||a->cursor+s>a->end)return NULL; void* p=a->cursor; a->cursor+=s; return p; }
textures.c:62:    pngzbuf a = {0}; u8* p = (u8*)PngArenaAlloc(arena, initial_size), d_len[288]; i32 f, t;
textures.c:81:    a->out = (u8*)PngArenaAlloc(arena, (size_t)x * y * out_n);
textures.c:121:            case 0x49444154: if (!z.idata) { z.idata = (u8*)PngArenaAlloc(arena, len + 16); ioff = 0; } mcpy(z.idata + ioff, s.img_buffer, length); s.img_buffer += length; ioff += length; break;
textures.c:138:        FHandle dummy_fd; int sz=0; const char* d =(const char*)OS_OpenAndAllocateFileBufferReadonly(t->parser->entries[pIdx].path,&dummy_fd,&sz);
textures.c:141:        u32 nP = (u32)w * h; u8 *idx = (u8*)OS_Alloc(nP); u32 *pal = (u32*)OS_Alloc(256 * sizeof(u32)); u32 pSz = 0; 
textures.c:181:    FHandle fd; int sz; char *data = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz), *cur = data, *end = data + sz;
textures.c:197:    p->entries = OS_Alloc((p->count = p->capacity = m_idx + 1) * sizeof(TextureData));
textures.c:240:    i32* parsIdx = OS_Alloc(texCnt * sizeof(i32));
textures.c:244:    thread_png_arenas = (PngArena*)OS_Alloc((size_t)threadCnt * sizeof(PngArena));
textures.c:246:    TexResult* texResults = OS_Alloc(texCnt * sizeof(TexResult)); // Unified result struct allocation
textures.c:256:    void* arena = OS_AllocateRAM(arena_size, 0x1|0x2, 0x20|0x02|0x08000, INVALID_FHANDLE);
textures.c:259:    i32* textureSizes = OS_Alloc(texCnt * 2 * sizeof(i32));
textures.c:260:    u32* texturePaletteOffsets = OS_Alloc(texCnt * sizeof(u32));
textures.c:295:    u8* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize,fp,WIN_ICON);    
text.c:19:    FHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
text.c:29:    ttAllocs = OS_Alloc(4674 * sizeof(TAlloc));
text.c:31:    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
text.c:32:    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
text.c:38:    u8*bmp=OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Primary atlas
text.c:64:    OS_Free(ttAllocs,4674 * sizeof(TAlloc));
text.c:85:    Sys_Text.file_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.file_data||asz<=0){DualLogError("Failed to load text file: %s\n",tf);return;}
text.c:109:    Sys_Text.filelog_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.filelog_data||asz<=0){DualLogError("Failed to load log text file: %s\n",tf);return;}
culling.c:24:    u8* cullingFileBuffer=OS_Alloc(MAX_CULL_FILESIZE * sizeof(u8));
winput.c:11:void InputMonitor(WinSysmonitor*,int,int); const FBC* ChooseFBConfig(const FBC*, u32); WinSysmonitor* AllocMonitor(const char*,int,int);
winput.c:138:        DeleteDC(dc); m = AllocMonitor(name,wMM,hMM); OS_Free(name,nameSize);
winput.c:231:    typedef void(*__GLXextproc)();                            typedef XSizeHints*(*PFN_XAllocSizeHints)();                       typedef int(*PFN_XChangeProperty)(Display*,XID,Atom,Atom,int,int,const u8*,int);   typedef void(*PFN_XCID)(XcursorImage*);                         typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                    typedef XID(*PFN_XCreateColormap)(Display*,XID,Visual*,int);
winput.c:247:                                     struct { void* handle; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XChangeWindowAttributes ChangeWindowAttributes; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
winput.c:262:    static void updateNormalHints(WinSyswindow* w, int w_, int h) { XSizeHints* hs=WinSys.x11.xlib.AllocSizeHints(); i64 s; WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,hs,&s); hs->flags &= ~((1L<<4)|(1L<<5)|(1L<<7)); hs->flags|=((1L<<4)|(1L<<5)); hs->min_width=hs->max_width=w_; hs->min_height=hs->max_height=h; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,hs); WinSys.x11.xlib.Free(hs); }
winput.c:303:            WinSysmonitor* m = AllocMonitor(oi->name, wMM, hMM);
winput.c:415:        if (!WindowVisible()) { i64 s; XSizeHints* h=WinSys.x11.xlib.AllocSizeHints(); if (WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,h,&s)) {h->flags|=(1L<<2); h->x=h->y=0; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,h);} WinSys.x11.xlib.Free(h); } 
winput.c:450:        XSizeHints* sz=WinSys.x11.xlib.AllocSizeHints();
winput.c:486:            X(AllocSizeHints) X(ChangeProperty) X(CheckTypedWindowEvent) X(CreateColormap) X(CreateWindow) X(ChangeWindowAttributes) X(DefineCursor) X(DeleteProperty) X(DisplayKeycodes) X(FilterEvent) X(FindContext) X(Free) X(UngrabPointer) X(FreeEventData) X(GetInputFocus) X(GetKeyboardMapping) X(GetWMNormalHints) X(GetWindowAttributes) X(GetWindowProperty)
winput.c:535:size_t monitorAllocationSize = 0;
winput.c:539:        WinSys.monitors = WinSys.monitors ? OS_Realloc(WinSys.monitors,monitorAllocationSize,sizeof(WinSysmonitor*) * WinSys.monitorCount) : OS_Alloc(WinSys.monitorCount * sizeof(WinSysmonitor*));
winput.c:540:        monitorAllocationSize = WinSys.monitorCount * sizeof(WinSysmonitor*);
winput.c:550:WinSysmonitor* AllocMonitor(const char* n, int w, int h) { WinSysmonitor* m = OS_Calloc(1, sizeof(WinSysmonitor)); m->widthMM = w; m->heightMM = h; scpy_to_a_from_b(m->name,n,sizeof(m->name)); return m; }
audio.c:655:    u32 sf = *frames, df = (u32)((u64)sf*AUDIO_RATE/src_rate); float *dst = (float*)OS_Alloc(df*2*sizeof(float)); *sz = df*2*sizeof(float); float ratio = (float)sf/(float)df;
audio.c:663:    u64 frames = wav.totalPCMFrameCount; float *buf = (float*)OS_Alloc(frames*AUDIO_CHANNELS*sizeof(float)); size_t bufSize = frames*AUDIO_CHANNELS*sizeof(float); u64 got = WavReadPCMFrames(&wav,frames,buf);
models.c:88:    u16* final_t = OS_Alloc(ec * sizeof(u16)); // Allocate final_t early so we can use it instead of ft_scratch
models.c:100:    float* final_verts = (float*)OS_Alloc((size_t)ucnt * CPU_VRT_SZ);
models.c:173:    modelBVHNodes[m] = (BvhNode*)OS_Alloc(ctx->nodeCount * sizeof(BvhNode));
models.c:177:        modelBVHTriOrder[m] = (u16*)OS_Alloc(ctx->triCount * sizeof(u16));
models.c:193:    FHandle fd; int sz; char* buf = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz);
models.c:203:    ModelData* ents = OS_Alloc(cnt * sizeof(ModelData));
models.c:274:    float* weldedPos = (float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); // worst case: no duplicates at all
models.c:293:    u16* weldedTris = (u16*)OS_Alloc((size_t)tc * 3 * sizeof(u16));
models.c:307:    vPos = OS_Alloc(mdlsCnt * sizeof(float*)); modelTriangles = OS_Alloc(mdlsCnt * sizeof(u16*));
models.c:308:    modelBVHNodes = (BvhNode**)OS_Alloc(mdlsCnt * sizeof(BvhNode*)); modelBVHTriOrder = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
models.c:311:    void* arena_base = OS_Alloc(arena); char* p = arena_base;
models.c:316:    for (u32 i=0; i<mdlsCnt; ++i) { i32 pi = idxmap[i]; if(pi >= 0){ FHandle d; int sz=0; raw[i].data=(const char*)OS_OpenAndAllocateFileBufferReadonly(mp.entries[pi].path,&d,&sz); raw[i].size=sz; raw[i].name=mp.entries[pi].path;} }
models.c:348:    physPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*));
models.c:349:    physTris = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
models.c:350:    physVertCounts = (u32*)OS_Alloc(mdlsCnt * sizeof(u32));
models.c:458:    cvxAdjOffsets=OS_Alloc(MAX_UNIQUE_CVX_MESHES * sizeof(u32*)); cvxAdjLists=OS_Alloc(MAX_UNIQUE_CVX_MESHES * sizeof(u16*));
models.c:477:        u32 edgeCount = 0; u32* tempEdges = OS_Alloc(tCount * 3 * sizeof(u32));
models.c:486:        u32* degree=OS_Alloc(vCount * sizeof(u32)); 
models.c:488:        u32* offsets=OS_Alloc((vCount + 1) * sizeof(u32)); offsets[0]=0; for(u32 i=0;i<vCount;++i){offsets[i+1]=offsets[i] + degree[i];}
models.c:489:        u16* adjList = OS_Alloc(uniqueEdgeCount * 2 * sizeof(u16));
models.c:490:        u32* writePos = OS_Alloc(vCount * sizeof(u32));
common.h:239:void *OS_AllocateRAM(size_t,i32,i32,FHandle),*OS_AllocateFileBackedRAMReadonly(size_t,FHandle,char*),*OS_Realloc(void*,size_t,size_t),OS_Close(FHandle),OS_Write(FHandle,const void*,size_t,const char*),OS_ThreadJoin(OS_Thread* t),OS_USleep(u32 usec);
common.h:246:void UseTargets(u16,const char*),AddForce(u16,V3,bool),CenterStatusPrint(const char * restrict fmt, ...),* OS_Alloc(size_t),OS_Free(void*,size_t),*OS_OpenAndAllocateFileBufferReadonly(const char*,FHandle*,int*),DebugRAM(const char*),
stbtt.h:4:typedef struct { void* ptr; size_t sz; } TAlloc;
stbtt.h:5:static TAlloc* ttAllocs = NULL;
stbtt.h:7:static void* ttalloc(size_t n) { if (tallocCount>=4674) {DualLogError("ttalloc too many!\n"); return NULL;} void*p=OS_Alloc(n); ttAllocs[tallocCount++]=(TAlloc){p,n}; return p; }
stbtt.h:8:static void  ttfree (void* p) { if(!p||tallocCount==0||ttAllocs[tallocCount-1].ptr!=p)return;OS_Free(p,ttAllocs[tallocCount-1].sz);tallocCount--; }
synth.c:5:static SynthVoice* SynAlloc(void) { for (u32 i=0;i<MAX_SYNTH_VOICES;i++) if (!syn_ch[i].active) return &syn_ch[i]; return NULL; }
synth.c:6:static SynthVoice* SynTrigger(SynthFn fn,float seconds,float vol) { SynthVoice* v=SynAlloc(); if(!v) return NULL; *v=(SynthVoice){.fn=fn,.frames=(u32)(AUDIO_RATE*seconds),.vol=vol,.active=true}; return v; }
```

Binary size eval:

```
❯ size ./voxen
   text  data       bss       dec     hex filename
 506018 33428 192529865 193069311 b8200ff ./voxen
```

```
❯ nm -S --size-sort -t d ./voxen | grep -i ' [tw] ' | tail -n 200
0000000016978736 0000000000000392 t CastRayCellCheck
0000000017081328 0000000000000392 t OptimizeVertexFetch
0000000017055312 0000000000000393 t Screenshot
0000000016924928 0000000000000395 t mp3_read_pcm_frames_f32
0000000016922144 0000000000000403 t resample_stereo
0000000017294320 0000000000000407 t ConsoleEmulator
0000000017290976 0000000000000411 t CenterWindowOnMonitor
0000000017294736 0000000000000420 t quat_from_yaw_pitch_roll
0000000016950656 0000000000000443 t ScreenPointToRay
0000000017074368 0000000000000447 t cgltf_load_buffer_base64
0000000016977584 0000000000000447 t LoadCullPNG
0000000016957056 0000000000000447 t UseTargets
0000000017219904 0000000000000460 t _eqs
0000000016965408 0000000000000462 t TeleportAway
0000000017303440 0000000000000464 t ParseLevelArg
0000000016924448 0000000000000469 t mp3_seek_to_pcm_frame
0000000016902736 0000000000000476 t play_synth
0000000017301904 0000000000000478 t LoadConfig
0000000017231360 0000000000000490 t LoadFallbackFont
0000000017220368 0000000000000491 t _tess_c
0000000017208896 0000000000000492 t _cff_idx_get
0000000017220864 0000000000000498 t _tess_cb
0000000017377648 0000000000000498 t WeaponsUpdate
0000000017009872 0000000000000506 t SetLevelPointers
0000000016982720 0000000000000515 t PortalCulling
0000000017349600 0000000000000519 t UpdateInstanceMatrix4x4s
0000000017290448 0000000000000526 t InputKey
0000000016979696 0000000000000531 t CastStraightX
0000000016914384 0000000000000533 t UpdateMusic
0000000016955568 0000000000000534 t FuncWallUpdate
0000000016923904 0000000000000536 t mp3_get_pcm_frame_count
0000000016920832 0000000000000537 t GenRing
0000000017209760 0000000000000543 t _get_subrs
0000000017041120 0000000000000546 t LoadGame
0000000016918496 0000000000000549 t GenTapCase
0000000016979136 0000000000000551 t CastStraightZ
0000000016966224 0000000000000564 t PatchUpdate
0000000017148144 0000000000000565 t _supA_box
0000000016971280 0000000000000572 t UseEntity
0000000016917536 0000000000000578 t GenFootstep
0000000017054704 0000000000000579 t BmpWrite
0000000017048432 0000000000000580 t DualLogMain
0000000016915728 0000000000000580 t GenDoor
0000000016900176 0000000000000582 t AICheckPain
0000000016903216 0000000000000582 t play_wav
0000000016978032 0000000000000587 t AddDoorPortal
0000000016953200 0000000000000588 t AddWeaponToInventory
0000000017142336 0000000000000588 t RunGJK
0000000017128576 0000000000000595 t SampleQuat
0000000016967984 0000000000000605 t DoorUse
0000000017313088 0000000000000612 t UI_Slider
0000000017270112 0000000000000613 t DrawVelocityVector
0000000016957920 0000000000000614 t ButtonSwitchUse
0000000016915104 0000000000000617 t GenLaserSS1
0000000017269232 0000000000000618 t TextureSequenceUpdate
0000000017124176 0000000000000629 t cgltf_parse_json_accessor_sparse
0000000016919456 0000000000000634 t GenSparkSmall
0000000017142928 0000000000000643 t SeedEPA
0000000017132128 0000000000000646 t Entity_GetCap
0000000017278656 0000000000000650 t SetWindowIcon
0000000016906928 0000000000000654 t play_mp3
0000000017010384 0000000000000656 t CopyPlayerState
0000000017051616 0000000000000662 t VoidSquasher
0000000017040448 0000000000000666 t SaveGame
0000000016956384 0000000000000671 t ForceBridgeUpdate
0000000016984352 0000000000000672 t CullInit
0000000016902048 0000000000000675 t synth_reverb_apply
0000000017343104 0000000000000680 t GetProjections
0000000017309488 0000000000000688 t CompileAnyShader
0000000017371936 0000000000000689 t MeleeHitUpdate
0000000017158736 0000000000000690 t ApplyInvTensor
0000000016913360 0000000000000690 t GetCorrespondingLevelClip
0000000016899472 0000000000000704 t InitNPC
0000000016967264 0000000000000713 t DoorActuate
0000000017038960 0000000000000719 t LoadAllLevels
0000000016900768 0000000000000720 t GetFootstepTypeForPrefab
0000000016962576 0000000000000746 t GrenadeExplode
0000000016951152 0000000000000754 t DropHeldItem
0000000016921376 0000000000000756 t GenBeakerThud
0000000017311328 0000000000000760 t mul_mat4
0000000017202912 0000000000000767 t stbtt_FindGlyphIndex
0000000017039680 0000000000000768 t LoadLevel
0000000017144880 0000000000000806 t BvhWalkSphMsh
0000000017310496 0000000000000829 t ExtractFrustumPlanes
0000000017124816 0000000000000831 t cgltf_parse_json_sparse_part
0000000017249504 0000000000000831 t PngHuf
0000000016959328 0000000000000846 t ApplyImpactForceSphere
0000000017136448 0000000000000847 t HullSupport
0000000017372640 0000000000000894 t FireMelee
0000000017272416 0000000000000898 t DrawSphereWireframe
0000000017145696 0000000000000901 t CapMsh
0000000017044048 0000000000000901 t double2str
0000000017050672 0000000000000919 t trinkle
0000000017152896 0000000000000926 t CvxMshFillExtraPoints
0000000017370992 0000000000000936 t CreateStandardImpactMarks
0000000017273376 0000000000000958 t DrawMeshCollider
0000000016948032 0000000000000965 t mp3L3_imdct36
0000000016973680 0000000000000966 t ObjectDeath
0000000017280416 0000000000000970 t InputMonitor
0000000017374880 0000000000001020 t FireWeapon
0000000017127312 0000000000001024 t NodeGlobalMatrixAtTime
0000000017303904 0000000000001036 t ProcessConsoleCommand
0000000016905872 0000000000001052 t SndInit
0000000017153824 0000000000001060 t BvhWalkAABB_CvxTri
0000000016983248 0000000000001092 t CullCore
0000000017279312 0000000000001095 t PollMonitors
0000000017340320 0000000000001131 t DrawEntity
0000000016911952 0000000000001137 t MixAmbs
0000000017008720 0000000000001144 t AddInstance
0000000017081728 0000000000001155 t FinalizeParsedMesh
0000000016980240 0000000000001181 t CircleFanRays
0000000017080128 0000000000001192 t OptimizeVertexCache
0000000017198928 0000000000001202 t CantStand
0000000017292736 0000000000001203 t ChangeFullScreenWindowed
0000000016949424 0000000000001223 t Push
0000000017267648 0000000000001234 t TextureParsingWorker
0000000017074816 0000000000001249 t cgltf_load_buffers
0000000017143584 0000000000001284 t ExpandEPA
0000000017277360 0000000000001288 t DrawAngularVelocity
0000000017049376 0000000000001290 t qsort_new
0000000016981424 0000000000001292 t DetermineVisibleCells
0000000017056176 0000000000001312 t scycle
0000000017288736 0000000000001319 t SetGLContext_GetFunctionPointers
0000000016964080 0000000000001327 t Death
0000000017291392 0000000000001336 t UpdateScreenSize
0000000017146608 0000000000001337 t PrimitiveCvx
0000000016922560 0000000000001338 t mp3_init_file
0000000016953792 0000000000001477 t AddItemToInventory
0000000017356048 0000000000001508 t main
0000000017240064 0000000000001539 t RenderFormattedText
0000000017125648 0000000000001548 t GltfMeshFreePartial
0000000017281680 0000000000001590 t processEvent
0000000017350119 0000000000001624 t NewGame
0000000017218272 0000000000001626 t _rse
0000000017341456 0000000000001645 t mat4_inverse
0000000017129520 0000000000001667 t BvhBuildOctree
0000000017270736 0000000000001676 t DrawBoxColliderColored
0000000016971856 0000000000001707 t ModUpdate
0000000017235616 0000000000001755 t LoadTextForLanguage
0000000017052288 0000000000001777 t BlowBubblesOfVoid
0000000016910080 0000000000001803 t InitAudio
0000000017072560 0000000000001808 t jsmn_parse
0000000016903808 0000000000001813 t load_wav
0000000017156816 0000000000001888 t CvxCvx
0000000017154896 0000000000001917 t CvxMsh
0000000017058368 0000000000001985 t cgltf_element_read_float
0000000017006256 0000000000002011 t LoadFieldIntoLight
0000000017327376 0000000000002018 t RenderPausedUI
0000000017283280 0000000000002073 t VCreateWindow
0000000017140192 0000000000002134 t SphTriTest
0000000017113504 0000000000002172 t UpdateAnims
0000000017115744 0000000000002184 t GenerateConvexAdjacencyLists
0000000016907872 0000000000002207 t AudioUpdate
0000000017118224 0000000000002305 t cgltf_parse_json_node
0000000017137296 0000000000002332 t GJKNextSimplex
0000000016968928 0000000000002350 t Targetted
0000000017090784 0000000000002365 t GltfBakeWorker
0000000016960176 0000000000002385 t TakeDamage
0000000017036512 0000000000002392 t LoadLevelData
0000000017162384 0000000000002450 t PrepareSolverContact
0000000017200144 0000000000002566 t ApplyPlayerMovements
0000000016974960 0000000000002621 t DetermineClosedEdges
0000000017237376 0000000000002674 t LoadLogTextForLanguage
0000000017228672 0000000000002687 t stbtt_InitFont_internal
0000000017110720 0000000000002777 t PhysGeomWorker
0000000017120544 0000000000002871 t cgltf_parse_json_animation
0000000017298800 0000000000002930 t InputProcessing
0000000017159440 0000000000002933 t SolveGlobalContacts
0000000017274336 0000000000003022 t DrawCapsuleCollider
0000000017133312 0000000000003045 t ComputeConvexMeshInertiaTensor
0000000017044960 0000000000003052 t sFormatV
0000000017329408 0000000000003172 t UpdateLights
0000000017285488 0000000000003243 t WindowInit
0000000017149056 0000000000003305 t CvxTriTest
0000000017221376 0000000000003323 t stbtt_MakeGlyphBitmapSubpixel
0000000017210304 0000000000003422 t _run_cs
0000000017214672 0000000000003599 t _fae
0000000017250336 0000000000003700 t PngLoad
0000000017231856 0000000000003750 t InitFontAtlasses
0000000017224704 0000000000003953 t stbtt_PackFontRanges
0000000017076080 0000000000004048 t cgltf_validate
0000000017203680 0000000000004225 t _GetGlyphShapeTT
0000000017351792 0000000000004252 t InitalizeEnvironment
0000000017093152 0000000000004469 t ParseModelData
0000000017304944 0000000000004529 t Raycast
0000000017098000 0000000000005003 t LoadModels
0000000017344128 0000000000005460 t Render
0000000017254048 0000000000006331 t CreatePngImageArena
0000000017242448 0000000000007052 t PngDecode
0000000017260384 0000000000007249 t LoadTextures
0000000017332592 0000000000007651 t RenderShadowmaps
0000000017103008 0000000000007710 t ModelParsingWorker
0000000017082896 0000000000007878 t LoadGLTFAnimatedBlocks
0000000017359632 0000000000010741 t RenderUI
0000000017060368 0000000000012181 t cgltf_parse
0000000017314528 0000000000012840 t RenderMenu
0000000016985840 0000000000019907 t ModEDefsInitAfterLoad
0000000016925328 0000000000022693 t mp3_decode_next_frame_ex
0000000017011040 0000000000025043 t LoadLevelMod
0000000017164848 0000000000034074 t Physics
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

FPS: 289
ms: 3.43
RAM: 751mb (mostly the animation system duplicating models for every frame, stupid but works great!)
VRAM: 434mb
Build 8.4secs
Init 1.449secs

CPU: 3.43ms bottlneck
GPU: 3.29ms

PMD copy-paste-detector usage
/home/qmaster/Downloads/pmd/bin/pmd cpd --minimum-tokens 100 --language cpp /home/qmaster/Github/Voxen

Quick command to print out all unused .obj files in ./Models that aren't specified in ./Data/models.txt
ls ./Models/*.obj | grep -vFf <(sed -n 's/^#//p' ./Data/models.txt)

Quick command to clean out Blender comments from obj file:
sed -i '/^#/d' ./Models/*.obj

mp3 crush command:
❯ find . -type f -name "*.mp3" -exec sh -c 'for f; do ffmpeg -loglevel fatal -y -i "$f" -f mp3 -map_metadata -1 -q:a 8.4 -ar 24000 "${f}.tmp" && mv "${f}.tmp" "$f"; done' _ {} +
