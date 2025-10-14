# Voxen

## The OpenGL Voxel Lit Engine

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

Based on SDL2 and OpenGL 4.5+, this engine attempts to leverage low latency and
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
 * -lSDL2 (`sudo apt install libsdl2-dev`)
 * -lSDL2_ttf (`sudo apt install libsdl2-ttf-dev`)
 * -lSDL2_image (`sudo apt install libsdl2-image-dev`)
 * -lGLEW (`sudo apt install libglew-dev`)
 * -lGL (`sudo apt install libgl1-mesa-dev`)
 * -lm
 * -lrt
 * -lpthread (for multithread support)
 * -lenet (for networking with Enet)
 * -L./External -l:libassimp.6.0.2.a -lz -lstdc++ -static-libstdc++ (Prebuilt, included in ./External/, for model loading from .fbx (for now))
 * -fopenmp (Will hopefully remove after only loading needed items for current level at a time)

Single command:

```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libglew-dev libgl1-mesa-dev libassimp-dev
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
  Polls SDL2 input and enqueues input Events
  Processes input and sets key states, mouselook
  Iterates over all queued Server events (Physics, Game Logic scripts (VM))
  Client-side rendering
Exit with cleanup, conditionally cleaning up resources based on how we exited and when

---

### Systems:

#### Unified Event Queue:

All Server actions occur as events processed by the
event queue that runs on just the Server's main thread.
Journaling as it goes for debugging, doubles as a log
feature and supports playback of logs similar to Quake
demo files and uses same .dem extension but with a
different custom format.


#### Client-Server Architecture:

All game sessions are coop sessions using Listen
Server; singleplayer is local Listen Server with
itself as only client.  Singleplayer is the same
as starting a coop game before anyone has joined.
Only 2 player coop is currently planned (4 maybe).
Server handles all game functionality except
loading in resources, keeping track of static
level geometry and structures needed for rendering,
and rendering are all Client-side.
Data Resource Loading:

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


#### Entity - Instance System:

All objects/items in the game are Instances that
have an associated Entity type.  No instances exist
without a type.  Some Entity types specified by the
entities.txt file may be unused by a game/mod.
Entity definition is loaded first to populate the
types list.  The Instances are populated after as
a product of the level load system.


#### Scripting Virtual Machine (VM):

TODO


#### Level Load System:

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


#### Savegame System:

All script variables are saved.  All instance states are
saved.  All physics states are saved, referenced by instance
index.  No systems rely on pointers and are indexed array
based to ensure all links are preserved in saves.  All save
data is in plaintext format using pipe delimiter | to split
each key:value pair which are colon separated.  The key is
given by the variable name, variable names pulled from the
scripts on the instance based on its entity type.


#### VXGI Lighting:

Voxen wouldn't be called Voxen without Voxels.  The world is
overlayed with a sparse voxel representation for storing and
updating lighting information such as Global Illumination (GI)
and Shadows which include Ambient Occlusion.  This is
calculated on a separate thread then passed to GPU for actually
applying lighting/shadows.


#### Screen Space Reflections:

All specular surfaces get reflections.  There are
no specular highlight fakeries to be found here.
As this is "screenspace" it can only reflect what
the player can see elsewhere in their screen. This
may be augmented with the, albeit softer and
blurrier, voxel results.  Also called SSR.

#### Rendering System:

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

#### Texturing System:

Leveraging a unified single buffer for all texture colors,
palettized by texture, allows for completely arbitrary
unlimited texture sizes (up to VRAM) in any size with no
rebinding overhead, only passing to GPU once ever.  All
texture data is accessible GPU-side and not stored on CPU.
ALL.  Normalmaps, glow maps, specular maps, UI.  ALL.
One flat buffer of color, One flat buffer of palette
offsets, one flat buffer of palette indices, one flat
buffer of texture palette indices offsets.

#### Mesh System:

Models are loaded into one unified flat vertex buffer with
minimal data, just position, normal, and uv.  Meshes are indexed triangles.

---

### License

MIT-0
