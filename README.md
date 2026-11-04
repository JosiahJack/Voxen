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
❯ ls *.* ./Shaders/*.glsl ./Shaders/*.compute | grep -vE 'README.md|builds.csv|*.patch|voxen.exe|voxen.log|build.sh|Citadel.pdb|*.bin' | xargs perl -MList::Util=max -lne '$first{$ARGV} //= $_; $count{$ARGV} = $.; if(eof){$total += $.; $. = 0;} END { $max = max map {length} keys %first; printf "99999999 %7d total\n", $total; printf "%8d %-${max}s  %s\n", $count{$_}, $_, $first{$_} for keys %first }' 2>/dev/null | sort -nr | head -n 51 | sed 's/^99999999 //'
  15261 total
    2845 voxen.c                            // voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Main loop.  Core rendering engine.  OS Shim Layer.  Windowing and Input.  Config.  Cheats Console.  Core systems init.
    1772 entity.c                           // entity.c - Entity Definitions and Save Load System for levels and savegames
    1586 ui.c                               // ui.c - User Interface(UI) aka HUD
    1445 citadel.c                          // citadel.c - Game logic.
    1258 models.c                           // models.c - 3D Models Loading System, Animation, Convex Edge Adjacency, Mesh Optimization
     833 physics.c                          // physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
     696 audio.c                            // audio.c - Audio System supporting .mp3 + .wav filetypes only, uses Windows WASAPI or Linux ALSA("default" to work on PulseAudio and PipeWire or ALSA + dmix).  Mixes synthesized sounds/music.
     638 textures.c                         // textures.c - 2D Texture Loading System and Texture Animations
     593 ai.c                               // ai.c - NPC logic
     584 common.h                           // common.h - Shared items between engine and gamecode (e.g. enums)
     445 text.c                             // text.c - Text and Font Rendering/Loading System
     444 weapons.c                          // weapons.c - Weapon System
     383 credits.h                          // credits.h - Credits for Citadel: The System Shock Fan Remake, salt the fries!
     347 particles.c                        // particles.c - CPU-simulated, GPU-instanced particle system for Voxen
     243 ./Shaders/rt.compute               // rt.compute - Compute shader for ray-traced reflections (BVH per model)
     194 culling.c                          // culling.c - XZ 2D World Grid Cell Culling System 64x64 matching System Shock 1.
     157 ./Shaders/chunk_frag.glsl          // chunk_frag.glsl: Generic shader for all world objects
     150 lib.c                              // lib.c - LibC replacement functions and other misc helpers.
     147 ./Shaders/composite_frag.glsl      // composite.glsl - Composite rendered view + UI overlay, custom AA, VHS blur (subtle, magic!), SSR with tapped blur, Procedural skybox w/ stars + saturn + sun + station shield (if on!) that rotate, berserk color hallucinations, EMP screen rolling, fog, infrared grayscale.
      88 ./Shaders/voxels.compute           // voxels.compute - Compute shader for determining light lists for voxels and updating voxel tables 
      56 ./Shaders/ssr.compute              // ssr.compute - Compute shader for Screen Space Reflections 
      36 ./Shaders/rtcopy.compute           // rtcopy.compute - Builds the rtGeom triangle/position/UV sections for one model entirely on the GPU
      33 ./Shaders/shadowmap_frag.glsl      // shadowmap_frag.glsl - Shadowmap Fragment Shader, uses alpha cutout on textures for {fence style shadows.  Writes into SSBO via atomicMin on typecast float dist with * 100000 scaling.
      33 release.sh                         #!/bin/sh
      32 ./Shaders/depth_prepass.glsl       // depth_prepass.glsl: Renders all opaque + cutout objects prior to main forward+ pass
      30 ./Shaders/ui_frag.glsl             // ui_frag.glsl: Generic shader for unlit textured UI images (mostly cutouts)
      29 ./Shaders/particle_frag.glsl       // particle_frag.glsl - Particle fragment shader
      25 ./Shaders/particle_vert.glsl       // particle_vert.glsl - Instanced particle billboard vertex shader
      21 ./Shaders/trail_vert.glsl          // trail_vert.glsl - Camera-facing ribbon trail vertex shader (welded shared edges)
      21 ./Shaders/text_frag.glsl           // text_frag.glsl - Text Fragment shader, supports both SystemShock font with black border around every character and StopD font with 3d drop shadow and top edge highlights
      20 ./Shaders/trail_frag.glsl          // trail_frag.glsl - Trail fragment shader (SSBO palette texture lookup)
      20 gen_anim.sh                        #!/bin/bash
      10 ./Shaders/shadowmap_vert.glsl      // shadowmap_vert.glsl - Shadowmap Vertex shader
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

Log ouput from standard run:

```
Compiling voxen, total iterations today 62 (2026-11-03)...
Built engine as game in 3518 ms
=========== Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed ===========
Game: Citadel, the System Shock (1994) Fan Remake                               1/03/2013 - 10/26/2026

OpenGL 4.3+, libc 2.7(Linux) / none(Windows), depdendencies: ALSA/WASAPI, X11/GDI, dlopen+dlsym / none
Entity byte size:883
Windowing system init...............................................................took 0.002198 secs
Loading    fonts (   5) ...(glyphs packed, blanks 0/1303, 0/1303)...................took 0.088420 secs
Window creation and GL context......................................................took 0.119686 secs
Shader compilation..................................................................took 0.009126 secs
GL buffer setup.....................................................................took 0.024924 secs
Localization text loading...........................................................took 0.002765 secs
SSBO and shadow buffer setup........................................................took 0.000990 secs
Loading textures (3447) ... total palette colors: 37948, total pixels: 39252497.....took 0.140211 secs
Loading   models ( 902) ... vertices: 13561279, tris: 12059142......................took 0.440708 secs
Raytraced reflections upload........................................................took 0.278368 secs
Automap and biomonitor buffers......................................................took 0.003652 secs
Loading new game prep...............................................................took 0.001697 secs
Load all levels.....................................................................took 0.223480 secs
Difficulty item purge...............................................................took 0.000056 secs
Level entry setup...................................................................took 0.000064 secs
Culling found 1178 open cells.......................................................took 0.106524 secs
Level entry finalize................................................................took 0.000054 secs
Convex mesh and automap init........................................................took 0.005434 secs
Audio init..........................................................................took 0.002303 secs
Menu init...........................................................................took 0.000106 secs
Scratch arena total peak: 91 MB, Peak: 984624KB|961.55MB.....Game Initialization TOTAL 1.371932 secs
Mem at init: Heap 0b(0KB|0.00MB), USS 650825728b(635572KB|620.68MB), MMAP 292451KB|285.60MB

Frm    4 times|GPU ms::All:0.00 Shad:0.00 Pre:0.00 Main:0.00 SSR:0.00 Comp:0.00 | CPU ms::Shad:1.000 Phys:3.387 Subs:2 Rend:0.238 PrePhys:0.061 Logic:0.202 Raycasts:0.000(0) | Shadmap draws:5452
Frm  100 times|GPU ms::All:10.39 Shad:0.49 Pre:0.63 Main:2.22 SSR:6.41 Comp:0.64 | CPU ms::Shad:0.472 Phys:0.758 Subs:1 Rend:0.194 PrePhys:0.043 Logic:0.101 Raycasts:0.000(0) | Shadmap draws:1726
Frm  200 times|GPU ms::All:7.39 Shad:0.00 Pre:0.60 Main:2.20 SSR:3.95 Comp:0.63 | CPU ms::Shad:0.305 Phys:0.757 Subs:1 Rend:0.214 PrePhys:0.041 Logic:0.141 Raycasts:0.000(0) | Shadmap draws:0
Frm  500 times|GPU ms::All:7.71 Shad:0.19 Pre:0.60 Main:2.23 SSR:4.05 Comp:0.64 | CPU ms::Shad:0.273 Phys:0.585 Subs:1 Rend:0.225 PrePhys:0.036 Logic:0.145 Raycasts:0.000(0) | Shadmap draws:0
Frm 1000 times|GPU ms::All:9.52 Shad:0.19 Pre:0.62 Main:2.36 SSR:5.70 Comp:0.65 | CPU ms::Shad:0.354 Phys:0.516 Subs:1 Rend:0.184 PrePhys:0.031 Logic:0.146 Raycasts:0.000(0) | Shadmap draws:639
```

Binary size eval:

```
❯ size ./voxen
   text   data       bss       dec     hex filename
 712427 140260 171605873 172458560 a478240 ./voxen
```

```
❯ nm -S --size-sort -t d ./voxen | grep -i ' [tw] ' | tail -n 200
0000000016956263 0000000000000520 t AICheckPain
0000000017299439 0000000000000524 t LoadConfig
0000000017466768 0000000000000524 t WavReadPCMFrames
0000000017012182 0000000000000525 t QuestBitNoteSideEffects
0000000017479508 0000000000000529 t SpawnImpactEffectParticle
0000000017274719 0000000000000533 t GetMonitorWorkarea
0000000017086002 0000000000000538 t trinkle
0000000017036634 0000000000000539 t LoadCullPNG
0000000017124917 0000000000000541 t glb_parse_json_node
0000000017369436 0000000000000548 t RenderPausedUI
0000000017264595 0000000000000549 t TextureSequenceUpdate
0000000017321717 0000000000000550 t MixAmbs
0000000017476820 0000000000000554 t PollMonitors
0000000017451150 0000000000000556 t GoIntoGame
0000000016972411 0000000000000557 t AIHunt
0000000017355942 0000000000000558 t UI_Slider
0000000017326532 0000000000000561 t EnableCheatArsenal
0000000016982790 0000000000000561 t RemoveWeapon
0000000017087144 0000000000000563 t BmpWrite
0000000017268841 0000000000000563 t DrawSphereWireframe
0000000017474805 0000000000000567 t UILogWrap
0000000017130622 0000000000000572 t SampleV3
0000000017088452 0000000000000591 t DebugRAMActual
0000000017047407 0000000000000593 t CopyPlayerState
0000000017398566 0000000000000594 t PGComputePreview
0000000017375636 0000000000000597 t UI_WireNodeClick
0000000017228806 0000000000000598 t font_FindGlyphIndex
0000000016982166 0000000000000605 t UseCyberspaceItem
0000000017079520 0000000000000607 t LoadGame
0000000017043479 0000000000000610 t LoadFieldIntoLight
0000000017153571 0000000000000611 t HullSupport
0000000017037173 0000000000000612 t AddDoorPortal
0000000016998470 0000000000000628 t ChargeStationUse
0000000016951879 0000000000000630 t ai_effect_flash
0000000017159561 0000000000000637 t BvhWalkSphMsh
0000000016987218 0000000000000650 t UseTargets
0000000017491987 0000000000000653 t CheckUIStateAndAttack
0000000017298425 0000000000000657 t HwToggle
0000000017336902 0000000000000657 t RenderUIImageExtended
0000000016971741 0000000000000670 t AIGetSearchPoint
0000000016989808 0000000000000671 t ForceBridgeUpdate
0000000017078839 0000000000000681 t SaveGame
0000000017150633 0000000000000686 t Entity_GetCap
0000000017010305 0000000000000688 t DoorActuate
0000000016965656 0000000000000688 t SpawnNPCDeathBurst
0000000017021873 0000000000000690 t ElevatorPanelUse
0000000017467989 0000000000000690 t mp3_decode_next_frame
0000000017483084 0000000000000692 t CreateStandardImpactEffects
0000000017003864 0000000000000705 t GrenadeExplode
0000000016993661 0000000000000712 t ButtonSwitchUse
0000000017471288 0000000000000717 t amComputeCard
0000000017010993 0000000000000718 t DoorUse
0000000017008778 0000000000000718 t PatchUpdate
0000000017035910 0000000000000724 t DetermineClosedEdges
0000000017131194 0000000000000728 t FindBracket
0000000017046677 0000000000000730 t SetLevelPointers
0000000017040253 0000000000000741 t PortalCulling
0000000017284677 0000000000000743 t ChangeFullScreenWindowed
0000000016976149 0000000000000752 t ai_muzzle_particles
0000000017481306 0000000000000755 t SpawnProjectileImpactParticles
0000000017077526 0000000000000756 t UpdateSensaroundCamViews
0000000017269618 0000000000000763 t DrawMeshCollider
0000000017131922 0000000000000769 t BvhBuildOctree
0000000017123221 0000000000000771 t json_accessor
0000000017476044 0000000000000776 t EditFieldStep
0000000017383583 0000000000000776 t RenderSearch
0000000017371547 0000000000000781 t KeycodeKeypress
0000000017082265 0000000000000783 t double2str
0000000016952509 0000000000000783 t quat_look_rotation
0000000017302721 0000000000000795 t mp3L3_decode_scalefactors
0000000016976901 0000000000000798 t ai_muzzle_flash
0000000017158760 0000000000000801 t ExpandEPA
0000000017083048 0000000000000804 t sFormatV
0000000017169331 0000000000000812 t BvhWalkAABB_CvxTri
0000000017335150 0000000000000816 t CompileAnyShader
0000000017488553 0000000000000824 t FireBeachball
0000000017084904 0000000000000844 t qsort_new
0000000017320870 0000000000000847 t InitAudio
0000000017327776 0000000000000848 t ProcessConsoleCommand
0000000017094855 0000000000000854 t FinalizeParsedMesh
0000000017023666 0000000000000871 t DrawAIDebug
0000000017039236 0000000000000873 t DetermineVisibleCells
0000000017478627 0000000000000881 t SpawnImpactParticleForPrefab
0000000017307689 0000000000000882 t mp3L3_read_side_info
0000000017472005 0000000000000898 t amComputeModel
0000000017024537 0000000000000898 t SecurityCameraRotateUpdate
0000000017373006 0000000000000914 t PGEvalPuzzle
0000000016949916 0000000000000916 t InitNPC
0000000017126508 0000000000000958 t json_primitive
0000000017168365 0000000000000966 t CvxMshFillExtraPoints
0000000017487266 0000000000000969 t FireMelee
0000000017125458 0000000000000979 t glb_parse_json_animation
0000000016979130 0000000000000985 t PatchUse
0000000017283187 0000000000000993 t UpdateScreenSize
0000000017005734 0000000000000995 t ProjectileEffectImpactOnCollision
0000000017020711 0000000000001002 t UseEntity
0000000016991039 0000000000001009 t SpawnManagerUpdate
0000000017007330 0000000000001012 t Death
0000000017482061 0000000000001023 t CreateStandardImpactMarks
0000000017149262 0000000000001035 t trigger_gravitylift_touch
0000000017041235 0000000000001037 t CullInit
0000000017017704 0000000000001051 t RelayPanelUse
0000000016954924 0000000000001061 t AICheckIfPlayerInSight
0000000016964595 0000000000001061 t AIFace
0000000017406175 0000000000001073 t UpdateLights
0000000017002541 0000000000001099 t ApplyImpactForceSphere
0000000017033267 0000000000001107 t ObjectDeath
0000000017380179 0000000000001111 t RenderConsumableItem
0000000017374422 0000000000001130 t UI_PuzzleGridCell
0000000017384359 0000000000001136 t DrawSensaroundQuad
0000000017156680 0000000000001144 t SphTriTest
0000000016953779 0000000000001145 t AICheckIfEnemyInSight
0000000017301570 0000000000001151 t mp3L3_decode
0000000017423086 0000000000001158 t mat4_inverse
0000000017265214 0000000000001168 t CreatePngImageArena
0000000017267552 0000000000001172 t DrawBoxColliderColored
0000000017235583 0000000000001182 t InitFontAtlasses
0000000017129412 0000000000001210 t NodeLocalMatrixAtTime
0000000017170143 0000000000001226 t CvxMsh
0000000017016473 0000000000001231 t PuzzlePanelUse
0000000017480037 0000000000001243 t SpawnBeamTrail
0000000017238004 0000000000001265 t LoadLogTextForLanguage
0000000017272963 0000000000001282 t DrawAngularVelocity
0000000017490318 0000000000001290 t FireWeapon
0000000017309980 0000000000001326 t mp3d_synth
0000000017303516 0000000000001330 t mp3L3_huffman
0000000017152158 0000000000001337 t ComputeConvexMeshInertiaTensor
0000000017073955 0000000000001390 t LoadAllLevels
0000000017308571 0000000000001409 t mp3d_DCT_II
0000000017092417 0000000000001416 t jsmn_parse
0000000017315164 0000000000001445 t mp3_init_file
0000000016984231 0000000000001452 t InventoryUpdate
0000000017275369 0000000000001496 t processEvent
0000000017014963 0000000000001510 t FrobHeldItemIntoPanel
0000000017418873 0000000000001515 t RenderSunShadowmap
0000000017277099 0000000000001553 t VCreateWindow
0000000017279024 0000000000001579 t SetGLContext_GetFunctionPointers
0000000017485672 0000000000001594 t MeleeHitUpdate
0000000017445378 0000000000001609 t UpdateInstanceMatrix4x4s
0000000017305683 0000000000001640 t mp3dec_decode_frame
0000000017239781 0000000000001641 t RenderFormattedText
0000000017262254 0000000000001683 t TextureParsingWorker
0000000017116537 0000000000001717 t GltfBakeWorker
0000000017247942 0000000000001741 t vid_worker
0000000017483776 0000000000001746 t HitScanFire
0000000017044802 0000000000001847 t AddInstance
0000000017118254 0000000000001893 t PhysGeomWorker
0000000017381290 0000000000001902 t RenderGeneralItem
0000000017462113 0000000000001934 t main
0000000017171785 0000000000001950 t CvxCvx
0000000017012814 0000000000001966 t Targetted
0000000017154182 0000000000001967 t GJKNextSimplex
0000000017254231 0000000000001967 t LoadPng
0000000017146007 0000000000001988 t CyberItemOnTriggerEnter
0000000017244641 0000000000002092 t font_GetGlyphShape
0000000017324085 0000000000002106 t BioMonitorUpdate
0000000017090271 0000000000002146 t glb_parse
0000000017075345 0000000000002181 t LoadLevel
0000000017367102 0000000000002218 t EnsureReflectionResources
0000000017120437 0000000000002305 t UpdateAnims
0000000017312579 0000000000002309 t audioCacheAcquire
0000000016973839 0000000000002310 t ProjectileRaycast
0000000016999098 0000000000002311 t TakeDamage
0000000017241895 0000000000002349 t BuildTextDecalMeshes
0000000017164958 0000000000002461 t CvxTriTest
0000000017101378 0000000000002465 t ParseModelData
0000000017270381 0000000000002582 t DrawCapsuleCollider
0000000017420422 0000000000002664 t DrawEntity
0000000017173754 0000000000002682 t SolveGlobalContacts
0000000017318075 0000000000002795 t AudioUpdateMix
0000000017160198 0000000000002941 t PrimitiveCvx
0000000017225581 0000000000003091 t AreaBlockedAt
0000000017070627 0000000000003328 t LoadLevelData
0000000017250282 0000000000003421 t PngDecode
0000000017137002 0000000000003531 t PSys_Render
0000000017133151 0000000000003561 t PSysAddInternal
0000000016966813 0000000000003821 t AIAttack
0000000017288987 0000000000003999 t InputProcessing
0000000017447000 0000000000004024 t NewGame
0000000017176436 0000000000004734 t PrepareSolverContact
0000000017104207 0000000000004970 t LoadModels
0000000017140684 0000000000005021 t PSys_Update
0000000017293142 0000000000005283 t UI_ProcessNavigation
0000000017230215 0000000000005368 t BuildAtlasCPU
0000000017095753 0000000000005625 t LoadGLBAnimatedSources
0000000017329378 0000000000005772 t Raycast
0000000017256198 0000000000006056 t LoadTextures
0000000017218928 0000000000006653 t ApplyPlayerMovements
0000000017109409 0000000000006994 t ModelParsingWorker
0000000017399160 0000000000007015 t CenterMFD
0000000017025435 0000000000007186 t ModUpdate
0000000016956873 0000000000007722 t AIControllerUpdate
0000000017357684 0000000000009209 t RenderMenu
0000000017451942 0000000000010158 t InitalizeEnvironment
0000000017407553 0000000000010358 t RenderShadowmaps
0000000017385495 0000000000012754 t SideMFD
0000000017338159 0000000000016251 t AutomapTick
0000000017424555 0000000000020823 t Render
0000000017048188 0000000000021526 t LoadLevelMod
0000000017181170 0000000000035434 t Physics
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
