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
calculations for bounce lighting (GI) TODO! lol, reflections, and other effects leverage 
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
   9996 total
     882 voxen.c                             // voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Core renderer.  OS Shim Layer.
     881 audio.c                             // audio.c - Audio System supporting .mp3 + .wav filetypes only, uses Windows WASAPI or Linux ALSA("default" to work on PulseAudio and PipeWire or ALSA+dmix, w/ raw ioctl fallback).  Mixes synthesized sounds/music.
     765 citadel.c                           // citadel.c - Game logic.
     762 entity.c                            // entity.c - Entity Definitions and Save Load System for levels and savegames
     754 winput.c                            // winput.c - WinSys Windowing System and Input System interfacing with the OS.
     706 models.c                            // models.c - 3D Models Loading System, Animation, Convex Edge Adjacency, Mesh Optimization
     646 physics.c                           // physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
     532 text.c                              // text.c - Text and Font Rendering/Loading System
     472 common.h                            // common.h - Shared items between engine and gamecode (e.g. enums)
     414 ai.c                                // ai.c - AI logic control for NPC's enemies in the game.
     398 ui.c                                // ui.c - User Interface(UI) aka HUD
     391 ./Shaders/composite_frag.glsl       // composite.glsl - Composite rendered view + UI overlay, custom AA, VHS blur (subtle, magic!), SSR with tapped blur, Procedural skybox w/ stars + saturn + sun + station shield (if on!) that rotate, berserk color hallucinations, EMP screen rolling, fog, infrared grayscale.
     387 textures.c                          // textures.c - 2D Texture Loading System
     387 credits.h                           // credits.h - Credits for Citadel: The System Shock Fan Remake, salt the fries!
     381 weapons.c                           // weapons.c - Weapon System
     229 ./Shaders/chunk_frag.glsl           // chunk_frag.glsl: Generic shader for all world objects
     215 lib.c                               // lib.c - LibC replacement functions and other misc helpers.
     176 culling.c                           // culling.c - XZ 2D World Grid Cell Culling System 64x64 matching System Shock 1.
     106 particles.c                         // particles.c - CPU-simulated, GPU-instanced particle system for Voxen
      93 biomonitor.c                        // biomonotor.c - Biomonitor Graph and Text displays.
      83 ./Shaders/voxels.compute            // voxels.compute - Compute shader for determining light lists for voxels and updating voxel tables 
      70 ./Shaders/ssr.compute               // ssr.compute - Compute shader for Screen Space Reflections 
      33 ./Shaders/shadowmap_frag.glsl       // shadowmap_frag.glsl - Shadowmap Fragment Shader, uses alpha cutout on textures for {fence style shadows.  Writes into SSBO via atomicMin on typecast float dist with * 100000 scaling.
      32 ./Shaders/depth_prepass.glsl        // depth_prepass.glsl: Renders all opaque + cutout objects prior to main forward+ pass
      29 ./Shaders/ui_frag.glsl              // ui_frag.glsl: Generic shader for unlit textured UI images (mostly cutouts)
      27 ./Shaders/particle_frag.glsl        // particle_frag.glsl - Particle fragment shader
      22 ./Shaders/text_frag.glsl            // text_frag.glsl - Text Fragment shader, supports both SystemShock font with black border around every character and StopD font with 3d drop shadow and top edge highlights
      18 ./Shaders/particle_vert.glsl        // particle_vert.glsl - Instanced particle billboard vertex shader
      18 ./Shaders/chunk_vert.glsl           // chunk_vert.glsl: Generic shader for unlit textured surfaces (all world geometry, items, enemies, doors, etc., without transparency for first pass prior to lighting.
      15 ./Shaders/trail_frag.glsl           // trail_frag.glsl - Trail fragment shader (SSBO palette texture lookup)
      15 ./Shaders/depth_prepass_vert.glsl   // depth_prepass_vert.glsl: vertex shader for depth prepass
      14 ./Shaders/trail_vert.glsl           // trail_vert.glsl - Camera-facing ribbon trail vertex shader (welded shared edges)
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
 11,264k Data/
  1,815k Fonts/
  1,589k loose files (binaries, autosplitter notes)
      0k Screenshots/
204,359k
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

All specular surfaces get reflections both from specular highlights
and from Screen Space Reflections. As this is "screenspace" it can
only reflect what the player can see elsewhere in their screen. This
may be augmented with the, albeit softer and blurrier, voxel results TODO!.

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
Uses separate deduplicated position only mesh representation for physics;
leverages edge adjacency for faster GJK algorithm.

---

### License

MIT-0

#### Stats

Log ouput from standard run:

```
Compiling voxen, total iterations today 44 (2026-09-06)...
Built engine as game in 1122 ms
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
Entity size: 692
Loading    5 fonts... took 0.131186370 s
Loading textures (2132) ... total palette colors: 48492, total pixels: 44325445... took 0.120746 secs
Loading   models (807) ... vertices: 13123482, tris: 11706371, 0.893375852 secs
Loading new game...
Entity counts::0:3835|1:5982|2:5121|3:3285|4:3390|5:4302|6:5780|7:6118|8:4761|9:4571|10:1726|11:1609|12:1315|13:8417
 Light counts::0:1229|1:1016|2:1107|3:457|4:867|5:1046|6:1130|7:1827|8:887|9:1408|10:699|11:502|12:464|13:3
Load all levels... took 0.127147783 secs
Switched to Level 1
Culling found 1178 open cells... took 0.109769619 secs
Game Initialized in 1.594466530 secs
Loading new game...
Entity counts::0:3835|1:5982|2:5121|3:3285|4:3390|5:4302|6:5780|7:6118|8:4761|9:4571|10:1726|11:1609|12:1315|13:8417
 Light counts::0:1229|1:1016|2:1107|3:457|4:867|5:1046|6:1130|7:1827|8:887|9:1408|10:699|11:502|12:464|13:3
Load all levels... took 0.101591044 secs
Switched to Level 1
Culling found 1178 open cells... took 0.107607252 secs
Player named "" started the game!
```

Log output from one run with lib.c DebugRAM() internals uncommented:

```
Compiling voxen, total iterations today 43 (2026-09-06)...
Built engine as game in 355 ms
Mem at program start: Heap 0b(0KB|0.00MB), USS 405504b(396KB|0.39MB)
Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed
Entity size: 692
Mem at start font load: Heap 8298496b(8104KB|7.91MB), USS 16326656b(15944KB|15.57MB)
Loading    5 fonts...Mem at after font load: Heap 8298496b(8104KB|7.91MB), USS 136011776b(132824KB|129.71MB)
 took 0.136365196 s
Mem at before LoadTextures: Heap 11677696b(11404KB|11.14MB), USS 139513856b(136244KB|133.05MB)
Mem at start LoadTextures: Heap 11677696b(11404KB|11.14MB), USS 139513856b(136244KB|133.05MB)
Loading textures (2132) ... total palette colors: 48492, total pixels: 44325445... took 0.148467 secs
Mem at after LoadTextures: Heap 11677696b(11404KB|11.14MB), USS 186159104b(181796KB|177.54MB)
Mem at before LoadModels: Heap 11677696b(11404KB|11.14MB), USS 186159104b(181796KB|177.54MB)
Loading   models (807) ... vertices: 13123482, tris: 11706371, 0.886417532 secs
Mem at after LoadModels: Heap 302555136b(295464KB|288.54MB), USS 531226624b(518776KB|506.62MB)
Loading new game...
Mem at before runtime LoadAllLevels: Heap 302555136b(295464KB|288.54MB), USS 532287488b(519812KB|507.63MB)
Mem at start of LoadAllLevels: Heap 302555136b(295464KB|288.54MB), USS 532287488b(519812KB|507.63MB)
Entity counts::0:3835|1:5982|2:5121|3:3285|4:3390|5:4302|6:5780|7:6118|8:4761|9:4571|10:1726|11:1609|12:1315|13:8417
 Light counts::0:1229|1:1016|2:1107|3:457|4:867|5:1046|6:1130|7:1827|8:887|9:1408|10:699|11:502|12:464|13:3
Load all levels... took 0.137906698 secs
Mem at end of LoadAllLevels: Heap 304005120b(296880KB|289.92MB), USS 628744192b(614008KB|599.62MB)
Mem at after runtime LoadAllLevels: Heap 304005120b(296880KB|289.92MB), USS 628744192b(614008KB|599.62MB)
Mem at start of LoadLevel: Heap 304005120b(296880KB|289.92MB), USS 628744192b(614008KB|599.62MB)
Switched to Level 1
Culling found 1178 open cells... took 0.110187051 secs
Mem at end of LoadLevel: Heap 304005120b(296880KB|289.92MB), USS 630902784b(616116KB|601.68MB)
Mem at after runtime LoadLevel: Heap 304005120b(296880KB|289.92MB), USS 630902784b(616116KB|601.68MB)
Mem at before edge adjacency: Heap 304005120b(296880KB|289.92MB), USS 630902784b(616116KB|601.68MB)
Mem at after edge adjacency: Heap 304005120b(296880KB|289.92MB), USS 631468032b(616668KB|602.21MB)
Game Initialized in 1.745898166 secs
Mem at InitializeEnvironment after scratch free: Heap 304005120b(296880KB|289.92MB), USS 573231104b(559796KB|546.68MB)
Target hit: lev1creepydoor on 5692 (lev 1), timestamp: 39793.573906016
Targetted a->ioflags:2 e:496 doorcond:1
Unlocking entity with index 5692
Mem at frame 4: Heap 304680960b(297540KB|290.57MB), USS 584269824b(570576KB|557.20MB)
Mem at frame 100: Heap 304680960b(297540KB|290.57MB), USS 544296960b(531540KB|519.08MB)
Mem at frame 200: Heap 304680960b(297540KB|290.57MB), USS 540237824b(527576KB|515.21MB)
Mem at frame 500: Heap 304680960b(297540KB|290.57MB), USS 540725248b(528052KB|515.68MB)
Mem at frame 1000: Heap 304680960b(297540KB|290.57MB), USS 542461952b(529748KB|517.33MB)
```

```
❯ grep -rIn "Alloc"
text.c:4:typedef struct { void* ptr; size_t sz; } TAlloc;
text.c:5:static TAlloc* ttAllocs = NULL;
text.c:7:static void* ttalloc(size_t n) { if (tallocCount>=4674) {DualLogError("ttalloc too many!\n"); return NULL;} void*p=OS_AllocScratch(n); ttAllocs[tallocCount++]=(TAlloc){p,n}; return p; }
text.c:8:static void  ttfree (void* p) { if(!p||tallocCount==0||ttAllocs[tallocCount-1].ptr!=p)return; tallocCount--; } // Make sure to pop off in reverse order!
text.c:373:    FHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
text.c:397:    ttAllocs = OS_AllocScratch(4674 * sizeof(TAlloc));
text.c:399:    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
text.c:400:    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
text.c:406:    u8* bmp = OS_AllocScratch(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Primary atlas
text.c:435:    u8* data = OS_OpenAndAllocateFileBufferReadonly(path, &dfd, &asz); if(!data || asz <= 0) { DualLogError("Failed to load text file: %s\n", path); *out_size = 0; return NULL; }
culling.c:20:    u8* cullingFileBuffer=OS_Alloc(MAX_CULL_FILESIZE * sizeof(u8)); OS_Seek(fp,0,0); long read_size = OS_Read(fp,cullingFileBuffer,size); OS_Close(fp); if ((size_t)read_size != size) { DualLogError("Failed to read %s\n",path); OS_Exit(1); }
winput.c:7:void InputCursorPos(double*,double*,double,double); void InputMonitor(WSMon*,int,int); const FBC* ChooseFBConfig(const FBC*, u32); static WSMon* AllocMonitor(const char*,int,int);
winput.c:129:    u16* CreateWideStringFromUTF8Win32(const char* s) { u16* t; int c = MultiByteToWideChar(65001,0,(char*)s,-1,NULL,0); t = OS_Alloc(c*sizeof(u16)); MultiByteToWideChar(65001,0,(char*)s,-1,t,c); return t; }
winput.c:130:    char* CreateUTF8FromWideStringWin32(const u16* s, int* sz) { *sz = WideCharToMultiByte(65001,0,(u16*)s,-1,NULL,0,NULL,NULL); char* t = OS_Alloc(*sz); WideCharToMultiByte(65001,0,(u16*)s,-1,t,*sz,NULL,NULL); return t; }
winput.c:141:        DeleteDC(dc); m = AllocMonitor(name,wMM,hMM); OS_Free(name,nameSize);
winput.c:151:        if (dC) { d = OS_Alloc(WinSys.monitorCount*sizeof(WSMon*)); mcpy(d,WinSys.monitors,WinSys.monitorCount * sizeof(WSMon*)); }
winput.c:186:        WSWin* w = OS_Alloc(sizeof(WSWin)); w->decorated = 1; w->cursorMode = 0x00034003; u32 style = 0x060A0000 | (w->decorated ? 0x00C00000 : 0x80000000);
winput.c:206:        FBC* usableConfigs = OS_Alloc(nativeCount*sizeof(FBC));
winput.c:234:    typedef void(*__GLXextproc)();                            typedef XSizeHints*(*PFN_XAllocSizeHints)();                       typedef int(*PFN_XChangeProperty)(Display*,XID,Atom,Atom,int,int,const u8*,int);   typedef void(*PFN_XCID)(XcursorImage*);                         typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                    typedef XID(*PFN_XCreateColormap)(Display*,XID,Visual*,int);
winput.c:251:                                     struct { void* handle; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XChangeWindowAttributes ChangeWindowAttributes; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
winput.c:271:    static void updateNormalHints(WSWin* w, int w_, int h) { XSizeHints* hs=WinSys.x11.xlib.AllocSizeHints(); i64 s; WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,hs,&s); hs->flags &= ~((1L<<4)|(1L<<5)|(1L<<7)); hs->flags|=((1L<<4)|(1L<<5)); hs->min_width=hs->max_width=w_; hs->min_height=hs->max_height=h; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,hs); WinSys.x11.xlib.Free(hs); }
winput.c:279:        int lC=2+image[0].width*image[0].height; u64* icon=OS_Alloc(lC*sizeof(u64)), *t=icon; *t++=image[0].width; *t++=image[0].height;
winput.c:295:        int dC = WinSys.monitorCount; WSMon** d = NULL; if (dC) { d = OS_Alloc(WinSys.monitorCount*sizeof(WSMon*)); mcpy(d,WinSys.monitors,WinSys.monitorCount * sizeof(WSMon*)); }
winput.c:306:            WSMon* m = AllocMonitor(oi->name, wMM, hMM);
winput.c:417:        if (!WindowVisible()) { i64 s; XSizeHints* h=WinSys.x11.xlib.AllocSizeHints(); if (WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,h,&s)) {h->flags|=(1L<<2); h->x=h->y=0; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,h);} WinSys.x11.xlib.Free(h); } 
winput.c:422:        WSWin* w = OS_Alloc(sizeof(WSWin)); w->decorated = 1; w->cursorMode = 0x00034003;
winput.c:433:        GLXFBConfig* nativeConfigs; FBC* usableConfigs; const FBC* closest; int nativeCount,usableCount; nativeConfigs=WinSys.glx.GetFBConfigs(WinSys.x11.display,WinSys.x11.screen,&nativeCount); usableConfigs=OS_Alloc(nativeCount*sizeof(FBC)); usableCount=0;
winput.c:452:        XSizeHints* sz=WinSys.x11.xlib.AllocSizeHints();
winput.c:488:            X(AllocSizeHints) X(ChangeProperty) X(CheckTypedWindowEvent) X(CreateColormap) X(CreateWindow) X(ChangeWindowAttributes) X(DefineCursor) X(DeleteProperty) X(DisplayKeycodes) X(FilterEvent) X(FindContext) X(Free) X(UngrabPointer) X(FreeEventData) X(GetInputFocus) X(GetKeyboardMapping) X(GetWMNormalHints) X(GetWindowAttributes) X(GetWindowProperty)
winput.c:545:size_t monitorAllocationSize = 0;
winput.c:548:        WinSys.monitorCount++; WinSys.monitors = WinSys.monitors ? OS_Realloc(WinSys.monitors,monitorAllocationSize,sizeof(WSMon*) * WinSys.monitorCount) : OS_Alloc(WinSys.monitorCount * sizeof(WSMon*)); monitorAllocationSize = WinSys.monitorCount * sizeof(WSMon*);
winput.c:553:static WSMon* AllocMonitor(const char* n, int w, int h) { WSMon* m = OS_Alloc(sizeof(WSMon)); m->widthMM = w; m->heightMM = h; scpy_to_a_from_b(m->name,n,sizeof(m->name)); return m; }
audio.c:620:    u32 sf=*frames, df=(u32)((u64)sf*AUDIO_RATE/src_rate); float *dst=(float*)OS_Alloc(df*2*sizeof(float)); *sz=df*2*sizeof(float); float ratio=(float)sf/(float)df;
audio.c:628:    u64 frames = wav.totalPCMFrameCount; float *buf = (float*)OS_Alloc(frames*AUDIO_CHANNELS*sizeof(float)); size_t bufSize = frames*AUDIO_CHANNELS*sizeof(float); u64 got = WavReadPCMFrames(&wav,frames,buf);
audio.c:635:    size_t bufSize = (size_t)total * AUDIO_CHANNELS * sizeof(float); float *buf = (float*)OS_Alloc(bufSize); mp3_seek_to_pcm_frame(&dec,0); u64 got = mp3_read_pcm_frames_f32(&dec,total,buf); mp3_uninit(&dec); if (got == 0) { OS_Free(buf,bufSize); return NULL; }
audio.c:660:static SynthVoice* SynAlloc(void) { for (u32 i = 0; i < MAX_SYNTH_VOICES; i++){ if (!syn_ch[i].active){return &syn_ch[i];} } return NULL; }
audio.c:727:void play_synth(SoundID id, float vol, float pitch){if((u32)id >= SND_COUNT){return;} const SynthPreset* pr=&SynthPresets[id]; SynthVoice* v=SynAlloc(); if(!v){return;} *v=(SynthVoice){.fn=pr->fn,.frames=(u32)(AUDIO_RATE*pr->dur),.vol=pr->vol*vol,.pitch=pitch,.active=true}; v->p[0]=pr->p[0]; v->p[1]=pr->p[1]; v->p[2]=pr->p[2]; v->p[3]=pr->p[3];}
audio.c:728:void play_synth_at(SoundID id, float vol, float pitch, V3 pos){if ((u32)id >= SND_COUNT){return;} const SynthPreset* pr = &SynthPresets[id]; SynthVoice* v = SynAlloc(); if(!v){return;} *v = (SynthVoice){.fn=pr->fn,.frames=(u32)(AUDIO_RATE*pr->dur),.vol=pr->vol*vol,.pitch=pitch,.positional=true,.pos=pos,.active=true}; v->p[0]=pr->p[0]; v->p[1]=pr->p[1]; v->p[2]=pr->p[2]; v->p[3]=pr->p[3]; }
audio.c:776:    log_msg_t *lm = (log_msg_t*)OS_Alloc(sizeof(log_msg_t)); lm->samples = load_wav(path,&lm->frame_count,&lm->allocSize); if (!lm->samples) { DualLogError("Failed to load %s\n",path); OS_Free(lm,sizeof(*lm)); return; }
audio.c:784:    mp3_channel_t *m = (mp3_channel_t*)OS_Alloc(sizeof(mp3_channel_t));
models.c:51:static int cgltf_parse_json_string(jsmntok_t const* t, int i, const u8* j, char** out){ CGLTF_CHECK_TOKTYPE(t[i], JSMN_STRING);if(*out)return -1; int sz=(int)(t[i].end-t[i].start);char*r=(char*)OS_AllocScratch(sz+1);cgltf_total_alloc+=sz+1;sCpy2aSubFromb(r,sz,(const char*)j+t[i].start,sz+1);*out=r;return i+1; }
models.c:52:static int cgltf_parse_json_array(jsmntok_t const* t, int i, const u8* j, size_t es, void** out, size_t* os){ (void)j;if(t[i].type!=JSMN_ARRAY)return -1;if(*out)return -1;int sz=t[i].size;*out=OS_AllocScratch(es*sz);cgltf_total_alloc+=es*sz;*os=sz;return i+1; }
models.c:65:    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);if(*out)return -1; *oc=t[i].size;*out=(cgltf_attribute*)OS_Alloc(sizeof(cgltf_attribute)**oc);cgltf_total_alloc+=sizeof(cgltf_attribute)**oc;++i;
models.c:256:    jsmntok_t* t=(jsmntok_t*)OS_AllocScratch(sizeof(jsmntok_t)*(tc+1));jsmn_init(&p);
models.c:259:    cgltf_data* data=(cgltf_data*)OS_AllocScratch(sizeof(cgltf_data)); cgltf_total_alloc += sizeof(cgltf_data);
models.c:286:    u8* d=(u8*)OS_Alloc(sz);cgltf_total_alloc+=sz;u32 buf=0,bb=0;
models.c:300:            size_t psz=slen(uri)+slen(gltf_path)+1;char* path=(char*)OS_AllocScratch(psz);
models.c:304:            u8* fb=OS_AllocateFileBackedRAMReadonly(fsz,fp,path);
models.c:341:    if (unlikely(!ec)){return false;} u16* final_t = OS_Alloc(ec * sizeof(u16)); /*Allocate final_t early so we can use it instead of ft_scratch*/ u32 used_slots_count = 0; u32* rem = (u32*)remap_scr; /*Reuse remap_scr for the 'rem' array!*/ u32 ucnt = 0;
models.c:348:    OptimizeVertexCache(final_t,ec,ucnt,cache_scr); float* final_verts = (float*)OS_Alloc((size_t)ucnt * CPU_VRT_SZ);
models.c:528:        out->pos=(float*)OS_AllocScratch((size_t)vc * 3 * sizeof(float)); out->nrm=(float*)OS_AllocScratch((size_t)vc * 3 * sizeof(float)); out->uv=(float*)OS_AllocScratch((size_t)vc * 2 * sizeof(float)); out->skin=(VtxSkin*)OS_AllocScratch((size_t)vc * sizeof(VtxSkin)); gltfScratch += (size_t)vc * (3+3+2) * sizeof(float) + (size_t)vc * sizeof(VtxSkin);
models.c:543:            out->indices = (u32*)OS_AllocScratch((size_t)tc * 3 * sizeof(u32)); gltfScratch += (size_t)tc * 3 * sizeof(u32);
models.c:548:            out->indices = (u32*)OS_AllocScratch((size_t)tc * 3 * sizeof(u32)); gltfScratch += (size_t)tc * 3 * sizeof(u32);
models.c:567:    out->meshNodes    = (cgltf_node**)OS_AllocScratch(submeshCount * sizeof(cgltf_node*)); gltfScratch += submeshCount * sizeof(cgltf_node*);
models.c:568:    out->subPos       = (float**)    OS_AllocScratch(submeshCount * sizeof(float*)); gltfScratch += submeshCount * sizeof(float*);
models.c:569:    out->subNrm       = (float**)    OS_AllocScratch(submeshCount * sizeof(float*)); gltfScratch += submeshCount * sizeof(float*);
models.c:570:    out->subUv        = (float**)    OS_AllocScratch(submeshCount * sizeof(float*)); gltfScratch += submeshCount * sizeof(float*);
models.c:571:    out->subVertCount = (u32*)       OS_AllocScratch(submeshCount * sizeof(u32)); gltfScratch += submeshCount * sizeof(u32);
models.c:572:    out->subIndices   = (u32**)      OS_AllocScratch(submeshCount * sizeof(u32*)); gltfScratch += submeshCount * sizeof(u32*);
models.c:573:    out->subTriCount  = (u32*)       OS_AllocScratch(submeshCount * sizeof(u32)); gltfScratch += submeshCount * sizeof(u32);
models.c:590:            out->subPos[si] = (float*)OS_AllocScratch((size_t)vc * 3 * sizeof(float)); out->subNrm[si] = (float*)OS_AllocScratch((size_t)vc * 3 * sizeof(float)); out->subUv[si]  = (float*)OS_AllocScratch((size_t)vc * 2 * sizeof(float)); gltfScratch += (size_t)vc * (3+3+2) * sizeof(float);
models.c:604:                out->subIndices[si] = (u32*)OS_AllocScratch((size_t)tc * 3 * sizeof(u32)); gltfScratch += (size_t)tc * 3 * sizeof(u32);
models.c:608:                out->subIndices[si] = (u32*)OS_AllocScratch((size_t)tc * 3 * sizeof(u32)); gltfScratch += (size_t)tc * 3 * sizeof(u32);
models.c:666:    gBlockMeshes = (GltfMesh*)OS_AllocScratch((size_t)MAX_GLTF_BLOCKS * sizeof(GltfMesh));
models.c:667:    GltfFrameTask* tasks = (GltfFrameTask*)OS_Alloc((size_t)maxTasks * sizeof(GltfFrameTask)); u32 taskCount = 0;
models.c:731:    modelBVHNodes[m] = (BvhNode*)OS_Alloc(ctx->nodeCount * sizeof(BvhNode)); mcpy(modelBVHNodes[m], ctx->nodes, ctx->nodeCount * sizeof(BvhNode)); modelBVHNodeCounts[m] = ctx->nodeCount;
models.c:732:    if (ctx->triCount > 0) { modelBVHTriOrder[m] = (u16*)OS_Alloc(ctx->triCount * sizeof(u16)); if (modelBVHTriOrder[m]) { mcpy(modelBVHTriOrder[m],ctx->triOrder,ctx->triCount * sizeof(u16)); modelBVHTriOrderCounts[m] = ctx->triCount; } }
models.c:748:    FHandle fd; int sz; char* buf = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz);
models.c:757:    u32 cnt = maxidx + 1; ModelData* ents = OS_AllocScratch(cnt * sizeof(ModelData)); p->entries = ents; p->count = cnt; for (u32 i=0; i<cnt; ++i) {ents[i] = (ModelData){U16_MAX,false,255,NULL,0,{0}};} ModelData cur = {U16_MAX,false,255,NULL,0,{0}}; c = buf; e = buf+sz; ln = 0;
models.c:806:    float* weldedPos = (float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); // worst case: no duplicates at all
models.c:819:    float* exactPos = (float*)OS_Alloc((size_t)weldedCount * 3 * sizeof(float));
models.c:822:    u16* weldedTris = (u16*)OS_Alloc((size_t)tc * 3 * sizeof(u16));
models.c:834:    vPos = OS_AllocScratch(mdlsCnt * sizeof(float*)); modelTriangles = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
models.c:835:    modelBVHNodes = (BvhNode**)OS_Alloc(mdlsCnt * sizeof(BvhNode*)); modelBVHTriOrder = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
models.c:838:    void* arena_base = OS_AllocScratch(arena + 4096); char* p = arena_base; // Fudge covers alignment padding between sections
models.c:843:    for (u32 i=0; i<mdlsCnt; ++i) { i32 pi = idxmap[i]; if(pi >= 0){ FHandle d; int sz=0; raw[i].data=(const char*)OS_OpenAndAllocateFileBufferReadonly(mp.entries[pi].path,&d,&sz); raw[i].size=sz;} }
models.c:844:    bool* isGLTFAnimSrc = (bool*)OS_AllocScratch(mdlsCnt * sizeof(bool));
models.c:845:    bool* isGLTFStaticSrc = (bool*)OS_AllocScratch(mdlsCnt * sizeof(bool));
models.c:864:    physPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*)); physTris = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*)); physVertCounts = (u32*)OS_Alloc(mdlsCnt * sizeof(u32));
```

Binary size eval:

```
❯ size ./voxen
   text  data       bss       dec     hex filename
 475561 81236 153749769 154306566 9328806 ./voxen
```

```
❯ nm -S --size-sort -t d ./voxen | grep -i ' [tw] ' | tail -n 200
0000000016920285 0000000000000363 t mp3d_synth_pair
0000000017025035 0000000000000364 t BvhRayAABBHit
0000000017064711 0000000000000369 t EPAContactPoint
0000000016935190 0000000000000371 t AddAudioLogToInventory
0000000017196152 0000000000000372 t CompileShaders
0000000016968160 0000000000000374 t UpdateLight
0000000017058577 0000000000000377 t trigger_gravitylift_touch
0000000016909791 0000000000000379 t synth_reverb_apply
0000000017147410 0000000000000383 t _dict_ints
0000000016925783 0000000000000383 t resample_stereo
0000000017070342 0000000000000384 t _supA_box
0000000017224271 0000000000000385 t GetProjections
0000000017045021 0000000000000393 t cgltf_parse_json_animation_sampler
0000000016961877 0000000000000395 t CastRayCellCheck
0000000017183011 0000000000000398 t ConsoleEmulator
0000000016941652 0000000000000405 t ButtonSwitchUse
0000000017212140 0000000000000418 t SideMFD
0000000017183409 0000000000000420 t quat_from_yaw_pitch_roll
0000000017006638 0000000000000420 t Screenshot
0000000016935875 0000000000000423 t AddWeaponToInventory
0000000017188211 0000000000000423 t ParseLevelArg
0000000016919343 0000000000000423 t PlayTrack
0000000016938370 0000000000000428 t FuncWallUpdateInner
0000000017007930 0000000000000431 t cgltf_accessor_read_float
0000000017142443 0000000000000435 t LoadTextFile
0000000017146674 0000000000000437 t MeasureLineAdvance
0000000016958849 0000000000000440 t ObjectDeath
0000000016933529 0000000000000443 t ScreenPointToRay
0000000017181829 0000000000000448 t ChangeFullScreenWindowed
0000000016910170 0000000000000448 t play_wav
0000000017196524 0000000000000452 t ExtractFrustumPlanes
0000000017179737 0000000000000455 t InputMonitor
0000000017189239 0000000000000457 t RayTriangle
0000000017162671 0000000000000459 t DrawVelocityVector
0000000016940213 0000000000000460 t UseTargets
0000000016950135 0000000000000463 t DoorActuate
0000000016969745 0000000000000464 t SetLevelPointers
0000000017257113 0000000000000467 t WeaponsUpdate
0000000017132758 0000000000000468 t _eqs
0000000017186135 0000000000000470 t LoadConfig
0000000016962798 0000000000000478 t CastStraightX
0000000017066946 0000000000000480 t SeedEPA
0000000017139303 0000000000000481 t LoadFallbackFont
0000000017133540 0000000000000481 t _tess_cb
0000000016997066 0000000000000496 t LoadGame
0000000017013921 0000000000000497 t OptimizeVertexCache
0000000017207845 0000000000000502 t GetWeaponAmmoText
0000000016924478 0000000000000509 t mp3L3_imdct36
0000000017151687 0000000000000509 t PngHuf
0000000017040388 0000000000000510 t cgltf_parse_json_buffer_view
0000000016913043 0000000000000512 t play_mp3
0000000016960556 0000000000000518 t LoadCullPNG
0000000016920657 0000000000000521 t mp3L3_decode_scalefactors
0000000016951605 0000000000000521 t QuestBitNoteSideEffects
0000000016946145 0000000000000522 t GrenadeExplode
0000000016962272 0000000000000526 t CastStraightZ
0000000016912262 0000000000000527 t SndInit
0000000017004344 0000000000000538 t trinkle
0000000016908809 0000000000000539 t mp3L3_decode
0000000016950598 0000000000000544 t DoorUse
0000000017198923 0000000000000545 t UI_Slider
0000000016953643 0000000000000549 t UseEntity
0000000016949001 0000000000000552 t PatchUpdate
0000000017234682 0000000000000553 t UpdateInstanceMatrix4x4s
0000000016902010 0000000000000560 t AICheckPain
0000000017002520 0000000000000561 t DualLogMain
0000000017006074 0000000000000563 t BmpWrite
0000000017164302 0000000000000563 t DrawSphereWireframe
0000000016964748 0000000000000571 t PortalCulling
0000000017161903 0000000000000579 t TextureSequenceUpdate
0000000017066363 0000000000000583 t RunGJK
0000000017046780 0000000000000584 t SampleQuat
0000000017121588 0000000000000598 t stbtt_FindGlyphIndex
0000000016917727 0000000000000599 t MixAmbs
0000000017188634 0000000000000605 t ProcessConsoleCommand
0000000017061800 0000000000000611 t HullSupport
0000000017142878 0000000000000624 t LoadTextForLanguage
0000000016970209 0000000000000626 t CopyPlayerState
0000000017068382 0000000000000638 t BvhWalkSphMsh
0000000017059099 0000000000000646 t Entity_GetCap
0000000016961074 0000000000000649 t AddDoorPortal
0000000016996417 0000000000000649 t SaveGame
0000000016918684 0000000000000659 t GetCorrespondingLevelClip
0000000016939544 0000000000000669 t ForceBridgeUpdate
0000000016967483 0000000000000677 t LoadFieldIntoLight
0000000016943493 0000000000000678 t ApplyImpactForceSphere
0000000017249503 0000000000000680 t CreateStandardImpactEffects
0000000017041513 0000000000000684 t cgltf_parse_json_node
0000000017114963 0000000000000685 t CantStand
0000000017000118 0000000000000693 t double2str
0000000017139895 0000000000000698 t BuildAtlas
0000000017014694 0000000000000701 t FinalizeParsedMesh
0000000016959847 0000000000000709 t DetermineClosedEdges
0000000016965557 0000000000000725 t CullInit
0000000017045507 0000000000000725 t NodeGlobalMatrixAtTime
0000000017252631 0000000000000738 t FireMelee
0000000016954218 0000000000000751 t DrawAIDebug
0000000017244970 0000000000000754 t PollMonitors
0000000017196976 0000000000000760 t mul_mat4
0000000017013152 0000000000000769 t cgltf_load_buffers
0000000016947389 0000000000000769 t Death
0000000017047711 0000000000000779 t BvhBuildOctree
0000000017165102 0000000000000810 t DrawMeshCollider
0000000017195336 0000000000000816 t CompileAnyShader
0000000017075515 0000000000000832 t BvhWalkAABB_CvxTri
0000000016995582 0000000000000835 t LoadLevel
0000000017003246 0000000000000844 t qsort_new
0000000016963731 0000000000000859 t DetermineVisibleCells
0000000017039526 0000000000000862 t cgltf_parse_json_accessor
0000000016907722 0000000000000882 t mp3L3_read_side_info
0000000017180939 0000000000000890 t UpdateScreenSize
0000000016906722 0000000000000892 t mp3dec_decode_frame
0000000016900972 0000000000000893 t InitNPC
0000000017042197 0000000000000901 t cgltf_parse_json_animation
0000000017251730 0000000000000901 t MeleeHitUpdate
0000000017254075 0000000000000914 t FireWeapon
0000000017248583 0000000000000920 t CreateStandardImpactMarks
0000000017067426 0000000000000956 t ExpandEPA
0000000017074549 0000000000000966 t CvxMshFillExtraPoints
0000000017211154 0000000000000986 t CenterMFD
0000000017138174 0000000000001002 t stbtt_InitFont_internal
0000000017043260 0000000000001024 t cgltf_parse_json_primitive
0000000016936376 0000000000001025 t AddItemToInventory
0000000017221758 0000000000001026 t DrawEntity
0000000017131727 0000000000001031 t _rse
0000000016928132 0000000000001087 t mp3_decode_next_frame_ex
0000000017206739 0000000000001106 t RenderPausedUI
0000000017065242 0000000000001121 t SphTriTest
0000000017069020 0000000000001153 t PrimitiveCvx
0000000017212636 0000000000001164 t UpdateLights
0000000017153930 0000000000001168 t CreatePngImageArena
0000000016968575 0000000000001170 t AddInstance
0000000017163130 0000000000001172 t DrawBoxColliderColored
0000000016930221 0000000000001184 t Push
0000000016926166 0000000000001185 t mp3_init_file
0000000017184758 0000000000001235 t InputProcessing
0000000017060476 0000000000001248 t ComputeConvexMeshInertiaTensor
0000000017168513 0000000000001282 t DrawAngularVelocity
0000000016952233 0000000000001307 t Targetted
0000000017130419 0000000000001308 t _fae
0000000017052638 0000000000001319 t PSys_Simulate
0000000017000811 0000000000001322 t sFormatV
0000000016904737 0000000000001326 t mp3d_synth
0000000016921178 0000000000001330 t mp3L3_huffman
0000000017051293 0000000000001345 t PSys_UpdateEmitters
0000000017250183 0000000000001389 t HitScanFire
0000000017011256 0000000000001406 t jsmn_parse
0000000017160164 0000000000001409 t TextureParsingWorker
0000000016903319 0000000000001418 t mp3d_DCT_II
0000000017178318 0000000000001419 t SetGLContext_GetFunctionPointers
0000000016910618 0000000000001426 t load_wav
0000000017076347 0000000000001435 t CvxMsh
0000000017222784 0000000000001487 t mat4_inverse
0000000017134021 0000000000001520 t stbtt_MakeGlyphBitmapSubpixel
0000000017143502 0000000000001553 t LoadLogTextForLanguage
0000000017145055 0000000000001619 t RenderFormattedText
0000000017078198 0000000000001644 t CvxCvx
0000000017140593 0000000000001660 t InitFontAtlasses
0000000017170634 0000000000001673 t processEvent
0000000016993897 0000000000001685 t LoadAllLevels
0000000017242781 0000000000001721 t main
0000000017035411 0000000000001726 t PhysGeomWorker
0000000017152196 0000000000001734 t PngLoad
0000000016944171 0000000000001788 t TakeDamage
0000000016915820 0000000000001851 t InitAudio
0000000017020516 0000000000001909 t GltfBakeWorker
0000000017172541 0000000000001923 t VCreateWindow
0000000017037205 0000000000001936 t UpdateAnims
0000000017136089 0000000000002085 t stbtt_PackFontRanges
0000000016931405 0000000000002124 t BioMonitorUpdate
0000000017122492 0000000000002127 t _GetGlyphShapeTT
0000000016913555 0000000000002265 t AudioUpdate
0000000017062411 0000000000002300 t GJKNextSimplex
0000000017208665 0000000000002489 t HardwareButtons
0000000017165912 0000000000002601 t DrawCapsuleCollider
0000000017126228 0000000000002605 t _run_cs
0000000017022425 0000000000002610 t ParseModelData
0000000017008585 0000000000002671 t cgltf_parse
0000000017079861 0000000000002681 t SolveGlobalContacts
0000000016991085 0000000000002766 t LoadLevelData
0000000017048490 0000000000002803 t PSysAdd
0000000017071024 0000000000003042 t CvxTriTest
0000000017148266 0000000000003421 t PngDecode
0000000017054480 0000000000003497 t PSys_Render
0000000017082542 0000000000003504 t PrepareSolverContact
0000000017235248 0000000000003521 t NewGame
0000000017174746 0000000000003572 t WindowInit
0000000016954969 0000000000003763 t ModUpdate
0000000017238882 0000000000003899 t InitalizeEnvironment
0000000017025399 0000000000004149 t LoadModels
0000000017015450 0000000000005066 t LoadGLTFAnimatedBlocks
0000000017155098 0000000000005066 t LoadTextures
0000000017189696 0000000000005640 t Raycast
0000000017115648 0000000000005749 t ApplyPlayerMovements
0000000017029548 0000000000005863 t ModelParsingWorker
0000000017200173 0000000000006566 t RenderMenu
0000000017214105 0000000000007619 t RenderShadowmaps
0000000017224970 0000000000009712 t Render
0000000016971023 0000000000019958 t LoadLevelMod
0000000017086046 0000000000027695 t Physics
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

FPS: 478
ms: 1.94
RAM: 587mb (mostly the animation system duplicating models for every frame, stupid but works great!)
VRAM: 1253mb (enough for all lights on largest level, prevents issues)
Build 1.355secs
Init 1.547secs

CPU: 1.36ms
GPU: 1.94ms bottlneck

PMD copy-paste-detector usage
/home/qmaster/Downloads/pmd/bin/pmd cpd --minimum-tokens 100 --language cpp /home/qmaster/Github/Voxen

Quick command to print out all unused .obj files in ./Models that aren't specified in ./Data/models.txt
ls ./Models/*.obj | grep -vFf <(sed -n 's/^#//p' ./Data/models.txt)

Quick command to clean out Blender comments from obj file:
sed -i '/^#/d' ./Models/*.obj

mp3 crush command:
❯ find . -type f -name "*.mp3" -exec sh -c 'for f; do ffmpeg -loglevel fatal -y -i "$f" -f mp3 -map_metadata -1 -q:a 8.4 -ar 24000 "${f}.tmp" && mv "${f}.tmp" "$f"; done' _ {} +
