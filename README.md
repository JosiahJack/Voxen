# Voxen

### The OpenGL Voxel Lit Engine

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
high speed with low RAM usage.  The voxel volume is limited to a local space
and moves with the player camera, updating only the voxels that change at the
edges of the volume (for similar technique, see SEGI).  This engine is focused
on interior spaces similar to Quake, Half-Life, and other classic games.  The
voxel volume may work fine for outdoor environments but Voxen is not intended
to be used for large open world games.

Based on SDL2 and OpenGL 4.5+, this engine attempts to leverage low latency and
GPU driven rendering methods with minimal state changes and maximum flexibility
with user customizable entities.

All texture and model and audio data is loaded from disk directly for ease of
development and full mod support by design.

The engine uses a unified event system queue for debuggability, logging, and
log file playback in a very similar manner to Quake 1 demo files.  This engine
uses the same .dem file extension but in a custom binary format specific to
Voxen's event format with event type and payload variables.  All engine actions
run through the queue.

Minimizing hierarchical layers and leveraging sensibly named globals to cut out
fluff and overhead is important.  Minimal dependencies and leveraging tried and
true systems is important.

### Supported Platforms

- **Linux (64-bit)**: Primarily Debian-based distros (e.g., Kubuntu, Xubuntu) with X11. Wayland support is not intentional.
- **Windows (64-bit)**: Supports Windows 7+. Windows 11 support is not intentional.
- **MacOS (64-bit)**: Not supported due to OpenGL deprecation in favor of Metal.  Metal not supported at this time. TBD.

#### Test Systems

* AMD Ryzen 5000 + Nvidia GTX970, Linux 64bit Kubuntu 20.04, 16GB RAM 3200mhz (MAIN RIG)
* AMD Ryzen 1600X + Nvidia GTX550Ti, Linux 64bit Xubuntu 20.04, 32GB RAM 1866mhz (QUAKE MAP COMPILER RIG)
* Intel 4400 + Mesa integrated APU, Linux 64bit Xubuntu 20.04, 4GB RAM (WORK TRIP POTATO)

### Building

This is first and foremost a Linux based project.  Cross compile for Windows is TBD.
Build by calling ./build.sh build script.

#### Prerequisites

Project must be linked against the following libraries which your system must install
 * -lSDL2 (`sudo apt install libsdl2-dev`)
 * -lSDL2_ttf (`sudo apt install libsdl2-ttf-dev`)
 * -lSDL2_image (`sudo apt install libsdl2-image-dev`)
 * -lGLEW (`sudo apt install libglew-dev`)
 * -lGL (`sudo apt install libgl1-mesa-dev`)
 * -lm (`sudo apt install libsdl2-dev`)
 * -lrt (`sudo apt install libsdl2-dev`)
 * -lassimp (`sudo apt install libassimp-dev`)

Single command:

```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libglew-dev libgl1-mesa-dev libassimp-dev

### License

MIT-0
