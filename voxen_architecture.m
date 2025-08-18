// System Architecture
// ============================================================================
// Order of Ops:
// Initializes various core systems (Unified Event Queue, Client-Server, OpenGL+Window)
// Loads data resources (textures, models, etc.)
// Loads scripting VM
// Parses all game/mod scripts
// Initializes data handling systems and parsers using all above data
// Level Load using gamedata definition to pick starting level
// Starts game loop:
//   Polls SDL2 input and enqueues input Events
//   Processes input and sets key states, mouselook
//   Iterates over all queued Server events (Physics, Game Logic scripts (VM))
//   Client-side rendering
// Exit with cleanup, conditionally cleaning up resources based on how we exited and when
//
// ----------------------------------------------------------------------------
// Systems:
// Unified Event Queue: All Server actions occur as events processed by the
//                      event queue that runs on just the Server's main thread.
//                      Journaling as it goes for debugging, doubles as a log
//                      feature and supports playback of logs similar to Quake
//                      demo files and uses same .dem extension but with a
//                      different custom format.
//
// Client-Server Architecture: All game sessions are coop sessions using Listen
//                             Server; singleplayer is local Listen Server with
//                             itself as only client.  Singleplayer is the same
//                             as starting a coop game before anyone has joined.
//                             Only 2 player coop is currently planned (4 maybe).
//                             Server handles all game functionality except
//                             loading in resources, keeping track of static
//                             level geometry and structures needed for rendering,
//                             and rendering are all Client-side.
// Data Resource Loading: All game assets are loaded as different types of data
//                        via first loading a definition text file from ./Data
//                        then populating a list from which the particular
//                        asset type is then loaded into fixed flat buffers for
//                        use either in CPU or GPU shaders.
//
//                        e.g. Textures load ./Data/textures.txt definition file
//                        then load all specified .png images from that text
//                        file out of the file path specified in the definition
//                        file... ideally ./Textues folder.  Images are loaded
//                        into a fixed buffer at the index specified by the
//                        textures.txt definition file.  These indices are used
//                        by all other systems that use textures (e.g. instaces).
// Entity - Instance System: All objects/items in the game are Instances that
//                           have an associated Entity type.  No instances exist
//                           without a type.  Some Entity types specified by the
//                           entities.txt file may be unused by a game/mod.
//                           Entity definition is loaded first to populate the
//                           types list.  The Instances are populated after as
//                           a product of the level load system.
// Scripting Virtual Machine (VM): Game/mod specific logic are located in
//                                 scripts in ./Scripts folder.  Scripts are raw
//                                 and parsed on load into the script VM engine
//                                 that runs the logic using predefined hooks.
//                                 Unique valid_keys list made per script for
//                                 all variables which are all always saved in
//                                 savegames or to level definition files.
//
//                                 Certain function names are reserved for
//                                 specific functionality.  "update()" in a
//                                 script will be called once per frame from a
//                                 single threaded loop over all instances that
//                                 have entity type defined that has scripts that
//                                 have an "update()" hook in them.  "init()" is
//                                 ran for the VM's first cycle to initialize
//                                 variables or perform anything needed at game
//                                 start by the game/mod scripts.
// Level Load System: Levels are specified in sets of files for each level data
//                    type: geometry, dynamic objects, lights.  Geometry are any
//                    immovable static mesh based rendered objects which may be
//                    walls, shelves, floors, ceilings, crates, windows, etc.
//                    Dynamic objects are anything that can move or change state
//                    and include even hidden game state tracking entity instances
//                    because they can change their state.  Geometry is guaranteed
//                    static after level load.  Lights are the 3rd system loaded
//                    for a level and are a list of defined light sources with
//                    their brightness, color, and other values (e.g. spot angle).
//                    The gamedata definition file specifies the first level
//                    index to load.  All level definition files are specific
//                    and use with a number for the level index.
//                    E.g. level3_geometery.txt, level3_lights.txt
//                    Levels use same specification as savegames, in plain text.
// Savegame System: All script variables are saved.  All instance states are
//                  saved.  All physics states are saved, referenced by instance
//                  index.  No systems rely on pointers and are indexed array
//                  based to ensure all links are preserved in saves.  All save
//                  data is in plaintext format using pipe delimiter | to split
//                  each key:value pair which are colon separated.  The key is
//                  given by the variable name, variable names pulled from the
//                  scripts on the instance based on its entity type.
// VXGI Lighting: Voxen wouldn't be called Voxen without Voxels.  The world is
//                overlayed with a sparse voxel representation for storing and
//                updating lighting information such as Global Illumination (GI)
//                and Shadows which include Ambient Occlusion.  This is
//                calculated on a separate thread then passed to GPU for actually
//                applying lighting/shadows.
// Screen Space Reflections: All specular surfaces get reflections.  There are
//                           no specular highlight fakeries to be found here.
//                           As this is "screenspace" it can only reflect what
//                           the player can see elsewhere in their screen. This
//                           may be augmented with the, albeit softer and
//                           blurrier, voxel results.  Also called SSR.
// Rendering System: Rendering uses a multipass system with deferred lighting.
//                   Pass 1: Unlit Rasterization - gets albedo, normals, depth
//                                                 world position, indices.
//                                                 This is standard vert+frag.
//                   Pass 2: Deferred Lighting - compute shader that determines
//                                               many lights' contributions and
//                                               combines with VXGI results.
//                   Pass 3: Screen Space Reflections (SSR): Compute shader full
//                                                           screen effect that
//                                                           is subtle.
//                   Pass 4: Final Blit - Takes the results of the compute
//                                        shaders and renders image as full
//                                        screen quad.
//                   Rendering leverages static buffers for minimal CPU->GPU
//                   data transfers and maximal performance with minimal state
//                   changes.
// Texturing System: Leveraging a unified single buffer for all texture colors,
//                   palettized by texture, allows for completely arbitrary
//                   unlimited texture sizes (up to VRAM) in any size with no
//                   rebinding overhead, only passing to GPU once ever.  All
//                   texture data is accessible GPU-side and not stored on CPU.
//                   ALL.  Normalmaps, glow maps, specular maps, UI.  ALL.
//                   One flat buffer of color, One flat buffer of palette
//                   offsets, one flat buffer of palette indices, one flat
//                   buffer of texture palette indices offsets.
// Mesh System: Models are loaded into one unified flat vertex buffer with
//              minimal data, just position, normal, and uv.
//
