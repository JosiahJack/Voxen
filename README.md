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
  Run compute shader for either of Screenspace Reflections or Raytraced Reflections (dep on setting)
  Render UI
  Composite shader to combine main render, reflections, effects, procedural sky, and UI, and apply AA + VHS blur
Exit with zero cleanup, let the OS handle it; does immediate fastest exit as user's time is important.

### Table of Contents (Kind of):

```
❯ ls *.* ./Shaders/*.glsl ./Shaders/*.compute | grep -vE 'README.md|builds.csv|*.patch|voxen.exe|voxen.log|*.sh|Citadel.pdb|*.bin' | xargs perl -MList::Util=max -lne '$first{$ARGV} //= $_; $count{$ARGV} = $.; if(eof){$total += $.; $. = 0;} END { $max = max map {length} keys %first; printf "99999999 %7d total\n", $total; printf "%8d %-${max}s  %s\n", $count{$_}, $_, $first{$_} for keys %first }' 2>/dev/null | sort -nr | head -n 51 | sed 's/^99999999 //'
  11699 total
    1988 voxen.c                            // voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Main loop.  Core rendering engine.  OS Shim Layer.  Windowing and Input.  Config.  Cheats Console.  Core systems init.
    1261 ui.c                               // ui.c - User Interface(UI) aka HUD
     910 citadel.c                          // citadel.c - Game logic.
     891 models.c                           // models.c - 3D Models Loading System, Animation, Convex Edge Adjacency, Mesh Optimization
     843 entity.c                           // entity.c - Entity Definitions and Save Load System for levels and savegames
     823 physics.c                          // physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
     698 audio.c                            // audio.c - Audio System supporting .mp3 + .wav filetypes only, uses Windows WASAPI or Linux ALSA("default" to work on PulseAudio and PipeWire or ALSA + dmix).  Mixes synthesized sounds/music.
     592 ai.c                               // ai.c - NPC logic
     547 common.h                           // common.h - Shared items between engine and gamecode (e.g. enums)
     529 textures.c                         // textures.c - 2D Texture Loading System and Texture Animations
     383 credits.h                          // credits.h - Credits for Citadel: The System Shock Fan Remake, salt the fries!
     349 particles.c                        // particles.c - CPU-simulated, GPU-instanced particle system for Voxen
     345 text.c                             // text.c - Text and Font Rendering/Loading System
     287 weapons.c                          // weapons.c - Weapon System
     232 ./Shaders/rt.compute               // rt.compute
     182 culling.c                          // culling.c - XZ 2D World Grid Cell Culling System 64x64 matching System Shock 1.
     173 ./Shaders/chunk_frag.glsl          // chunk_frag.glsl: Generic shader for all world objects
     147 ./Shaders/composite_frag.glsl      // composite.glsl - Composite rendered view + UI overlay, custom AA, VHS blur (subtle, magic!), SSR with tapped blur, Procedural skybox w/ stars + saturn + sun + station shield (if on!) that rotate, berserk color hallucinations, EMP screen rolling, fog, infrared grayscale.
     140 lib.c                              // lib.c - LibC replacement functions and other misc helpers.
      88 ./Shaders/voxels.compute           // voxels.compute - Compute shader for determining light lists for voxels and updating voxel tables 
      56 ./Shaders/ssr.compute              // ssr.compute - Compute shader for Screen Space Reflections 
      32 ./Shaders/depth_prepass.glsl       // depth_prepass.glsl: Renders all opaque + cutout objects prior to main forward+ pass
      30 ./Shaders/ui_frag.glsl             // ui_frag.glsl: Generic shader for unlit textured UI images (mostly cutouts)
      29 ./Shaders/particle_frag.glsl       // particle_frag.glsl - Particle fragment shader
      25 ./Shaders/particle_vert.glsl       // particle_vert.glsl - Instanced particle billboard vertex shader
      21 ./Shaders/trail_vert.glsl          // trail_vert.glsl - Camera-facing ribbon trail vertex shader (welded shared edges)
      21 ./Shaders/text_frag.glsl           // text_frag.glsl - Text Fragment shader, supports both SystemShock font with black border around every character and StopD font with 3d drop shadow and top edge highlights
      20 ./Shaders/trail_frag.glsl          // trail_frag.glsl - Trail fragment shader (SSBO palette texture lookup)
      10 ./Shaders/rtcopy.compute           // rtcopy.compute - Builds the raytracing geometry triangle/position/UV sections for one model entirely on the GPU from that model's already-uploaded render buffers, so raytraced reflections need no CPU-side duplicates.
       9 ./Shaders/depth_prepass_vert.glsl  // depth_prepass_vert.glsl: vertex shader for depth prepass
       9 ./Shaders/chunk_vert.glsl          // chunk_vert.glsl: Generic shader for unlit textured surfaces (all world geometry, items, enemies, doors, etc., without transparency for first pass prior to lighting.
       8 ./Shaders/debugunlit_vert.glsl     // debugunlit_vert.glsl - Wireline Vertex Shader
       6 ./Shaders/ui_vert.glsl             // ui_vert.glsl: Generic shader for unlit textured surfaces (all world geometry, items, enemies, doors, etc., without transparency for first pass prior to lighting.
       6 ./Shaders/text_vert.glsl           // text_vert.glsl - Text Vertex Shader
       5 ./Shaders/composite_vert.glsl      // imageblit.glsl - Full screen quad unlit textured for presenting image buffers such as results from compute shaders, image effects, post-processing, etc..
       4 ./Shaders/debugunlit_frag.glsl     // debugunlit_frag.glsl - Wireline Fragment Shader, colored wirelines used for physics wireframe view of colliders, velocity debug vectors, angular velocity debug vector and arc for orientation, raycast debug vector, and weapon lasers 
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

Log ouput from standard run (all settings maxed):
```
Compiling voxen, total iterations today 98 (2026-11-11)...
Built engine as game in 4643 ms
=========== Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed ===========
Game: Citadel, the System Shock (1994) Fan Remake                                1/03/2013 - 11/5/2026
OpenGL 4.3+, libc 2.7(Linux) / none(Windows), depdendencies: ALSA/WASAPI, X11/GDI, dlopen+dlsym / none
Window creation and GL context......................................................took 0.169706 secs
GL shaders and buffer setup.........................................................took 0.056797 secs
SSBO and shadow buffer setup........................................................took 0.000942 secs
Loading textures (3967 pngs) total palette colors: 38154, total pixels: 40202776....took 0.190721 secs
Parsing   models ( 903) ... total verts: 13582566, tris: 12059144...................took 0.124541 secs
Automap and biomonitor buffers......................................................took 0.000805 secs
Loading new game prep...............................................................took 0.000606 secs
Load all levels.....................................................................took 0.129937 secs
Loading   models ( 778 persistent)  vertices: 912446, tris: 709741..................took 0.149912 secs
Loading models for lev 1, count 1859 ...............................................took 0.206403 secs
Culling found 1178 open cells.......................................................took 0.014758 secs
Convex mesh and automap init........................................................took 0.000062 secs
Raytraced reflections upload........................................................took 0.098004 secs
Audio and Menu init.................................................................took 0.238817 secs
Scratch arena total peak: 91 MB, Peak: 534284KB|521.76MB.......Game Initialization TOTAL 1.397995 secs
Mem at init: Heap 0b(0KB|0.00MB), USS 339582976b(331624KB|323.85MB), MMAP 135451KB|132.28MB
Frm    4 timings:::GPU ms::All:0.00 Shad:0.00 Pre:0.00 Main:0.00 SSR:0.00 Comp:0.00 | CPU ms::Shad:1.657 Phys:5.492 Subs:2 Rend:0.327 PrePhys:0.040 Logic:0.197 Raycasts:0.000(0) | Shadmap draws:6588
Frm  100 timings:::GPU ms::All:11.05 Shad:2.97 Pre:0.60 Main:2.32 SSR:4.57 Comp:0.60 | CPU ms::Shad:1.664 Phys:1.668 Subs:2 Rend:0.297 PrePhys:0.044 Logic:0.246 Raycasts:0.000(0) | Shadmap draws:6588
Frm  200 timings:::GPU ms::All:12.22 Shad:2.68 Pre:0.58 Main:2.48 SSR:5.88 Comp:0.60 | CPU ms::Shad:1.605 Phys:1.362 Subs:2 Rend:0.392 PrePhys:0.049 Logic:0.272 Raycasts:0.000(0) | Shadmap draws:6588
Frm  500 timings:::GPU ms::All:11.89 Shad:2.59 Pre:0.64 Main:2.33 SSR:5.72 Comp:0.60 | CPU ms::Shad:1.585 Phys:1.448 Subs:2 Rend:0.296 PrePhys:0.042 Logic:0.274 Raycasts:0.000(0) | Shadmap draws:6588
Frm 1000 timings:::GPU ms::All:11.08 Shad:2.97 Pre:0.61 Main:2.35 SSR:4.55 Comp:0.60 | CPU ms::Shad:1.626 Phys:1.418 Subs:2 Rend:0.293 PrePhys:0.041 Logic:0.285 Raycasts:0.000(0) | Shadmap draws:6588
```

Binary size eval (when the strip line in build.sh is commented out, else it prints 0's):

```
❯ size ./voxen
   text   data       bss       dec     hex filename
 734011 140372 154289433 155163816 93f9ca8 ./voxen

```

```
❯ nm -S --size-sort -t d ./voxen | grep -i ' [tw] ' | tail -n 100
0000000017393520 0000000000001127 t UI_PuzzleGridCell
0000000017038375 0000000000001135 t ObjectDeath
0000000017403373 0000000000001136 t DrawSensaroundQuad
0000000017171756 0000000000001144 t SphTriTest
0000000016956120 0000000000001150 t AICheckIfEnemyInSight
0000000016989616 0000000000001150 t CyberWallUpdate
0000000017319562 0000000000001151 t mp3L3_decode
0000000017442364 0000000000001154 t mat4_inverse
0000000017163097 0000000000001159 t trigger_gravitylift_touch
0000000017006666 0000000000001162 t ApplyImpactForceSphere
0000000017282787 0000000000001168 t CreatePngImageArena
0000000017142446 0000000000001169 t NodeLocalMatrixAtTime
0000000017185288 0000000000001226 t CvxMsh
0000000017291420 0000000000001229 t DrawAngularVelocity
0000000017501364 0000000000001243 t SpawnBeamTrail
0000000017020680 0000000000001258 t PuzzlePanelUse
0000000017511852 0000000000001290 t FireWeapon
0000000017327972 0000000000001326 t mp3d_synth
0000000017321508 0000000000001330 t mp3L3_huffman
0000000017437962 0000000000001352 t RenderSunShadowmap
0000000017011383 0000000000001375 t Death
0000000017096701 0000000000001380 t jsmn_parse
0000000017472642 0000000000001387 t RtSwapPatchModels
0000000017326563 0000000000001409 t mp3d_DCT_II
0000000017465261 0000000000001419 t MdlBuildLevelConvexAdjacency
0000000017333226 0000000000001445 t mp3_init_file
0000000016987068 0000000000001449 t InventoryUpdate
0000000017506927 0000000000001462 t MeleeHitUpdate
0000000017293773 0000000000001496 t processEvent
0000000017019169 0000000000001511 t FrobHeldItemIntoPanel
0000000017297505 0000000000001590 t SetGLContext_GetFunctionPointers
0000000017080295 0000000000001609 t LoadLevel
0000000017285207 0000000000001612 t DrawBoxColliderColored
0000000017253553 0000000000001620 t LoadLogTextForLanguage
0000000017463627 0000000000001621 t UpdateInstanceMatrix4x4s
0000000017295503 0000000000001630 t VCreateWindow
0000000017323675 0000000000001640 t mp3dec_decode_frame
0000000017256772 0000000000001640 t RenderFormattedText
0000000017470457 0000000000001641 t EnsureReflectionResources
0000000017139883 0000000000001660 t PhysGeomWorker
0000000017505096 0000000000001681 t HitScanFire
0000000017138160 0000000000001723 t GltfBakeWorker
0000000017051673 0000000000001805 t AddInstance
0000000017484087 0000000000001907 t main
0000000017400293 0000000000001917 t RenderGeneralItem
0000000017264866 0000000000001938 t vid_worker
0000000017186930 0000000000001950 t CvxCvx
0000000017169258 0000000000001967 t GJKNextSimplex
0000000017271335 0000000000001967 t LoadPng
0000000017017002 0000000000001984 t Targetted
0000000017279428 0000000000002082 t TextureParsingWorker
0000000017261565 0000000000002092 t font_GetGlyphShape
0000000017003548 0000000000002092 t TakeDamage
0000000017342331 0000000000002106 t BioMonitorUpdate
0000000017094574 0000000000002127 t glb_parse
0000000017159561 0000000000002201 t CyberItemOnTriggerEnter
0000000017046484 0000000000002202 t CullInit
0000000017122432 0000000000002203 t UpdateAnims
0000000017166272 0000000000002299 t ComputeConvexMeshInertiaTensor
0000000017330641 0000000000002309 t audioCacheAcquire
0000000017105822 0000000000002363 t ParseModelData
0000000017258885 0000000000002376 t BuildTextDecalMeshes
0000000016976699 0000000000002392 t ProjectileRaycast
0000000017180055 0000000000002461 t CvxTriTest
0000000017119764 0000000000002554 t LoadModels
0000000017188899 0000000000002682 t SolveGlobalContacts
0000000017336137 0000000000002804 t AudioUpdateMix
0000000017288576 0000000000002844 t DrawCapsuleCollider
0000000017175274 0000000000002941 t PrimitiveCvx
0000000017466680 0000000000002972 t NewGame
0000000017439348 0000000000003016 t DrawEntity
0000000017241878 0000000000003091 t AreaBlockedAt
0000000017267386 0000000000003421 t PngDecode
0000000017150408 0000000000003531 t PSys_Render
0000000017146557 0000000000003561 t PSysAddInternal
0000000017116125 0000000000003639 t ParseAllModels
0000000017307528 0000000000004028 t InputProcessing
0000000017075472 0000000000004260 t LoadAllLevels
0000000016969309 0000000000004311 t AIAttack
0000000017154090 0000000000005021 t PSys_Update
0000000017311712 0000000000005233 t UI_ProcessNavigation
0000000017191581 0000000000005336 t PrepareSolverContact
0000000017246611 0000000000005368 t BuildAtlasCPU
0000000017099926 0000000000005896 t LoadGLBAnimatedSources
0000000017273302 0000000000006126 t LoadTextures
0000000017109788 0000000000006337 t MdlLoadModelSet
0000000017235276 0000000000006602 t ApplyPlayerMovements
0000000017347973 0000000000006790 t Raycast
0000000017418700 0000000000006917 t CenterMFD
0000000017130993 0000000000007033 t ModelParsingWorker
0000000016959257 0000000000007830 t AIControllerUpdate
0000000017029519 0000000000008324 t ModUpdate
0000000017474029 0000000000009916 t InitalizeEnvironment
0000000017426440 0000000000010344 t RenderShadowmaps
0000000017377364 0000000000010881 t RenderMenu
0000000017404509 0000000000013000 t SideMFD
0000000017357907 0000000000016257 t AutomapTick
0000000017054868 0000000000019685 t LoadLevelMod
0000000017443829 0000000000019798 t Render
0000000017197270 0000000000036035 t Physics
```

Helper bash commands to generate frame sequences in models.txt:
start_index=2104
for i in $(seq 2 50); do
  printf "#Models/flight_fanwall_%06d.obj\nindex: %d\n" "$i" "$start_index"
  ((start_index++))
done

Main Rig.  Settings:
#define SSR_RES 4 // Ratio is (1 / SSR_RES) * render resolution.
VoxenSettings voxen_Settings = {
    .ScreenWidth = 1366u,
    .ScreenHeight = 768u,
    .Shadows = 1u,
    .AntiAliasing = 1u, // Default 1
    .Brightness = 100u, // Default 100 (for %)
    .VolumeMusic = 20u,
    .Language = 0, // English default
    .FOV = 65.0f,
    .Reflections = 2u, // Default 1
    .Vsync = false
};

FPS: 83
ms: 12.04
RAM: 476mb (mostly the animation system duplicating models for every frame plus the CPU then copying it for raycasts, stupid but works great!)
VRAM: 607mb
Build 4.3secs
Init 1.43secs

CPU: 4.13ms (up from prior 1.7 due to mobo failure, I'm back on my old Ryzen 1600X boo much sad!)
GPU: 12.04ms bottlneck

PMD copy-paste-detector usage
/home/qmaster/Downloads/pmd/bin/pmd cpd --minimum-tokens 100 --language cpp /home/qmaster/Github/Voxen

Quick command to print out all unused .obj files in ./Models that aren't specified in ./Data/models.txt
ls ./Models/*.obj | grep -vFf <(sed -n 's/^#//p' ./Data/models.txt)

Quick command to clean out Blender comments from obj file:
sed -i '/^#/d' ./Models/*.obj

mp3 crush command:
❯ find . -type f -name "*.mp3" -exec sh -c 'for f; do ffmpeg -loglevel fatal -y -i "$f" -f mp3 -map_metadata -1 -q:a 8.4 -ar 24000 "${f}.tmp" && mv "${f}.tmp" "$f"; done' _ {} +
