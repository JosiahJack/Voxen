// voxen.c
// Description: A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#include "os.h" // Operating System calls shim layer.
#include "voxen.h"
#include "External/stb_image.h"
#include "Shaders/shaders.h"
#include "todo.h"
#include "data_models.c"
#include "dynamic_culling.c"
#include "credits.c"
const char* EngineName = "Voxen, the Voxel Lit Open Source Game Engine";
GlobalContext Sys_Global = { .menuActive = false, .screenshotTimeout = 1.0, .creditsPageIndex = 1, .difficultyCombat = 2, .difficultyCyber = 2, .difficultyPuzzle = 2, .difficultyMission = 2, .deaths = 0 };
DiagnosticsSystem Sys_Dx = { .worstFPS = UINT32_MAX };
CheatsSystem Sys_Cheats = { .god = false, .noclip = true, .showLocation = true, .showFPS = true, .editMode = true };
RenderSystem Sys_Render;
uint8_t queuedLevelToLoad = 3;
Entity instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
GLuint instancesBuffer;
QuestBits questData;
float berserkFinished, berserkSeedTime, aspect3D = 1.0f, cam_pitch, cam_yaw = 90.0f, cam_roll, fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
int32_t cursorPosition_x = 680, cursorPosition_y = 384; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
char uiTextBuffer[TEXT_BUFFER_SIZE];
float uiOrthoProjection[16];
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[LIGHT_COUNT];
static float lightView[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
static float lightViewProj[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6]; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
FrustumPlane playerFrustumPlanes[6];
uint16_t editModeSelection = 682; // Test instance
uint16_t editModeTestEntityDefinition = 0; // Test instance's model index
float voxelMinCenterX, voxelMinCenterZ;
VoxenShadowSystem voxen_Shadow_System;

uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid input blank string, from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (data_parser_isspace((char)*str)) str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    char* endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 10);
    if (errno != 0 || val > UINT32_MAX) { DualLogError("Invalid input %s\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    return (uint32_t)val;
}

uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT16_MAX) { DualLogError("Value out of range for uint16_t: %u\n      from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint16_t)retval;
}

uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT8_MAX) { DualLogError("Value out of range for uint8_t: %u\n      from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint8_t)retval;
}

bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    uint32_t parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u\n      in place where expected a boolean from line[%u]: %s\n",parseval, lineNum+1, line);
    return parseval > 0 ? true : false;
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid float input blank string, from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    char* endptr;
    errno = 0;
    float val = strtof(str, &endptr);
    if (errno != 0) { DualLogError("Invalid float input %s\n      from line[%d]: %s\n      gave errno %u\n", str, lineNum+1, line, errno); return 0.0f; }
    if (*endptr != '\0') { DualLogError("Invalid float input %s\n      from line[%d]: %s\n      missing null terminator, *endptr: %u\n", str, lineNum+1, line, *endptr); return 0.0f; }
    if (endptr == str) { DualLogError("Invalid float input %s\n      from line[%d]: %s\n      end is equal to start\n", str, lineNum+1, line); return 0.0f; }
    return val;
}

bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum) {
    *is_comment = false;
    *is_eof = false;
    *is_newline = false;
    size_t pos = 0;
    int32_t c;
    while ((c = fgetc(file)) != EOF && data_parser_isspace(c) && c != '\n');
    if (c == EOF) { *is_eof = true; return false; }
    if (c == '\n') { *is_newline = true; return false; }
    
    if (c == '/' && (c = fgetc(file)) == '/') {
        *is_comment = true;
        while ((c = fgetc(file)) != EOF && c != '\n');
        return false;
    }
    
    if (c != EOF) token[pos++] = c;
    while ((c = fgetc(file)) != EOF && c != delimiter && c != '\n' && pos < max_len - 1) { token[pos++] = c; }
    token[pos] = '\0';
    if (pos >= max_len - 1) DualLogError("Token truncated at line %u\n", *lineNum);
    if (c == EOF) *is_eof = 1u;
    else if (c == '\n') *is_newline = 1u;
    return pos > 0;
}

// Unique set separate from savedata path and resource data to keep it focussed
bool process_gamedata_key_value(Entity *entry, const char *key, const char *value, const char *line, uint32_t lineNum) {
    if (!key || !value) { DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, line); return false; }
    
    while (data_parser_isspace(*key)) key++;
    while (data_parser_isspace(*value)) value++;
    char trimmed_key[256];
    char trimmed_value[256];
    strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
    strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
    trimmed_key[sizeof(trimmed_key) - 1] = '\0';
    trimmed_value[sizeof(trimmed_value) - 1] = '\0';
    char *key_end = trimmed_key + strlen(trimmed_key) - 1;
    char *val_end = trimmed_value + strlen(trimmed_value) - 1;
    while (key_end > trimmed_key && data_parser_isspace(*key_end)) *key_end-- = '\0';
    while (val_end > trimmed_value && data_parser_isspace(*val_end)) *val_end-- = '\0';
    
         if (strcmp(trimmed_key, "modname") == 0)         { strncpy(Sys_Global.global_modname, trimmed_value, sizeof(Sys_Global.global_modname) - 1); Sys_Global.global_modname[sizeof(Sys_Global.global_modname) - 1] = '\0'; entry->index = 0; } // Game/Mod Definition enforces setting entry index to 0 here, at least one of these must do it.  The game definition only has one index, 0.
    else if (strcmp(trimmed_key, "levelcount") == 0)      Sys_Global.numLevels = parse_numberu8(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "startlevel") == 0)      Sys_Global.startLevel = parse_numberu8(trimmed_value, line, lineNum);
    else return false;
    return true;
}

// Load Game/Mod Definition
void ParseGameData(void) {
    double start_time = get_time();
    const char* filename = "./Data/gamedata.txt";
    DualLog("Loading game definition from %s...",filename);    
    Entity entry;
    InitializeEntity(&entry);
    FILE *gamedatfile = fopen(filename, "r");
    if (!gamedatfile) { DualLogError("\nCannot open %s\n", filename); DualLogError("Could not parse %s!\n", filename); OS_Exit(1); }
    
    uint32_t lineNum = 0;
    bool is_eof;
    while (!feof(gamedatfile)) {
        char token[256];
        bool is_comment, is_newline;
        if (!read_token(gamedatfile, token, sizeof(token), ':', &is_comment, &is_eof, &is_newline, &lineNum)) {
            if (is_comment || is_newline) { lineNum += is_newline; continue; }
        }
        
        char key[256];
        strncpy(key, token, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        if (!read_token(gamedatfile, token, sizeof(token), '\n', &is_comment, &is_eof, &is_newline, &lineNum)) continue;
        
        process_gamedata_key_value(&entry, key, token, key, lineNum);
        lineNum += 1;
    }
    
    fclose(gamedatfile);
    if (strcmp(Sys_Global.global_modname, "Citadel") == 0) Sys_Global.global_modIsCitadel = true;
    DualLog(" loaded Game Definition for %s:: num levels: %d, start level: %d... took %f secs\n",Sys_Global.global_modname, Sys_Global.numLevels, Sys_Global.startLevel, get_time() - start_time);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
bool parse_data_file(DataParser *parser, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); return false; }
    
    char line[1024];
    uint32_t lineNum = 0;
    uint32_t max_index = 0;
    while (fgets(line, sizeof(line), file)) { // First pass: count entries and find max index
        lineNum++;        
        char *start = line;
        while (data_parser_isspace(*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace(*end)) { *end = '\0'; end--; }
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue;
        if (line[0] == '#') { continue; }

        char *colon = strchr(start, ':');
        if (colon && strncmp(start, "index", colon - start) == 0) {
            char *value = colon + 1;
            while (data_parser_isspace(*value)) value++;
            uint32_t idx = parse_numberu32(value, line, lineNum);
            if (idx > max_index) max_index = idx;
       }
    }

    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); fclose(file); return true; }

    uint32_t entry_count = max_index + 1;
    if (entry_count > parser->capacity) {
        Entity *new_entries = realloc(parser->entries, entry_count * sizeof(Entity));  
        parser->entries = new_entries;
        for (uint32_t i = parser->capacity; i < entry_count; ++i) InitializeEntity(&parser->entries[i]);
        parser->capacity = entry_count;
    }
    
    parser->count = entry_count;
    rewind(file);
    Entity entry;
    InitializeEntity(&entry);
    int32_t entries_stored = 0;
    lineNum = 0;
    int32_t currentChild = -1;
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        char *start = line;
        if (strlen(start) < 3) continue; // Must have at least k:v, skip if shorter

        while (data_parser_isspace(*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace(*end)) { *end = '\0'; end--; }
        if (*start == '\0') continue; // Skip empty line
        if (start[0] == '/' && start[1] == '/') continue; // Skip comment(ed out) line

        if (*start == '#' && *(start + 1) != '#') {
            // Store previous entry if valid
            if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) {
                parser->entries[entry.index] = entry;
                entries_stored++;
            }
            
            // Start new entry
            InitializeEntity(&entry);
            strncpy(entry.path, start + 1, sizeof(entry.path) - 1);
            entry.path[sizeof(entry.path) - 1] = '\0';
            continue;
        }

        // Handle key-value pair
        char *colon = strchr(start, ':');
        if (colon) {
            *colon = '\0';
            char *key = start;
            char *value = colon + 1;
            while (data_parser_isspace(*key)) key++;
            while (data_parser_isspace(*value)) value++;
            if (*key && *value) {
                while (data_parser_isspace(*key)) key++;
                while (data_parser_isspace(*value)) value++;
                char trimmed_key[256];
                char trimmed_value[256];
                strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
                strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
                trimmed_key[sizeof(trimmed_key) - 1] = '\0';
                trimmed_value[sizeof(trimmed_value) - 1] = '\0';
                char *key_end = trimmed_key + strlen(trimmed_key) - 1;
                char *val_end = trimmed_value + strlen(trimmed_value) - 1;
                while (key_end > trimmed_key && data_parser_isspace(*key_end)) *key_end-- = '\0';
                while (val_end > trimmed_value && data_parser_isspace(*val_end)) *val_end-- = '\0';
                if (strncmp(trimmed_key, "chunk_", 6) == 0) {
                    strncpy(entry.path, trimmed_key, sizeof(entry.path) - 1);
                    entry.path[sizeof(entry.path) - 1] = '\0';
                } else {
                         if (strcmp(trimmed_key, "index") == 0)             entry.index = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "persistent") == 0)        entry.persistent = parse_bool(trimmed_value, start, lineNum); // Didn't feel worthy of being considered an entflag so left separte
                    else if (strcmp(trimmed_key, "model") == 0)             entry.modelIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "animated") == 0)          entry.animated = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "texture") == 0)           entry.texIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "alttexture") == 0)           entry.altTexIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "glowtexture") == 0)       entry.glowIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "altglowtexture") == 0)    entry.altGlowIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "spectexture") == 0)       entry.specIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "normtexture") == 0)       entry.normIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "doublesided") == 0)       flag_set(&entry.entflags,ENTFLAG_DOUBLESIDED,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "transparent") == 0)       flag_set(&entry.entflags,ENTFLAG_TRANSPARENT,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "cardchunk") == 0)         flag_set(&entry.entflags,ENTFLAG_CARDCHUNK,  parse_bool(trimmed_value, start, lineNum));

                    else if (strcmp(trimmed_key, "collider") == 0)          entry.collider = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_centerx") == 0)  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_centery") == 0)  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_centerz") == 0)  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_sizex") == 0)    entry.colliderSize.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_sizey") == 0)    entry.colliderSize.y = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_sizez") == 0)    entry.colliderSize.z = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "colliderMeshIndex") == 0) entry.colliderMeshIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "mass") == 0)              entry.mass = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "linearDrag") == 0)        entry.linearDrag = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "angularDrag") == 0)       entry.angularDrag = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "kinematic") == 0)         flag_set(&entry.entflags,ENTFLAG_KINEMATIC, parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "useGravity") == 0)        flag_set(&entry.entflags,ENTFLAG_USEGRAVITY,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "bounciness") == 0)        entry.bounciness = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "dynamicFriction") == 0)   entry.dynamicFriction = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "frictionCombine") == 0)   entry.frictionCombine = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "bounceCombine") == 0)     entry.bounceCombine = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "numclips") == 0)          entry.numclips = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "animationNum") == 0)      entry.animationNum = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "changeMatOnActive") == 0) flag_set(&entry.entflags,ENTFLAG_CHANGE_TEX_ON_ACTIVE,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "blinkWhenActive") == 0)   flag_set(&entry.entflags,ENTFLAG_BLINK_TEX_ON_ACTIVE,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "noshadows") == 0)         flag_set(&entry.entflags,ENTFLAG_NO_SHADOWS,parse_bool(trimmed_value, start, lineNum));

                    else if (strcmp(trimmed_key, "volume") == 0)            entry.volume = parse_float(trimmed_value, start, lineNum);
                    
                    else if (strcmp(trimmed_key, "##child") == 0) {
                        ++currentChild;
                        if (currentChild >= MAX_CHILD_COUNT) { DualLogError("Too many children %u! Minivan is full!!\n", currentChild); OS_Exit(1); }
                        
                        entry.child[currentChild] = parse_numberu16(trimmed_value, start, lineNum);
                    } else if (strcmp(trimmed_key, "child_offsetx") == 0)    entry.child_offset[currentChild].x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "child_offsety") == 0)    entry.child_offset[currentChild].y = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "child_offsetz") == 0)    entry.child_offset[currentChild].z = parse_float(trimmed_value, start, lineNum);
                }
            } else DualLogWarn("Invalid key-value pair at line %u: %s\n", lineNum, start);
        } else {
            DualLogWarn("No colon found in line %u: %s\n", lineNum, start);
        }
    }

    // Store last entry
    if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) {
        parser->entries[entry.index] = entry;
        entries_stored++;
    }

    fclose(file);
    return true;
}
#pragma GCC diagnostic pop

GLuint CompileShader(GLenum type, const char *source, const char *shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) { char infoLog[512]; glGetShaderInfoLog(shader, 512, NULL, infoLog); DualLogError("%s Compilation Failed: %s\n", shaderName, infoLog); OS_Exit(1); }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int32_t count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int32_t i = 0; i < count; i++) { glAttachShader(program, shaders[i]); glDeleteShader(shaders[i]); }
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) { char infoLog[512]; glGetProgramInfoLog(program, 512, NULL, infoLog); DualLogError("%s Linking Failed: %s\n", programName, infoLog); OS_Exit(1); }
    return program;
}

void CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader");
    Sys_Render.chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");
    
    vertShader = CompileShader(GL_VERTEX_SHADER, debugUnlitVertexShaderSource, "Debug Unlit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, debugUnlitFragmentShaderSource, "Debug Unlit Fragment Shader");
    Sys_Render.debugUnlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Debug Unlit Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER, shadowmapVertexShaderSource, "Shadowmaps Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, shadowmapFragmentShaderSource, "Shadowmaps Fragment Shader");
    Sys_Render.shadowmapsShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Shadowmaps Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader");
    Sys_Render.textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");
    
    computeShader = CompileShader(GL_COMPUTE_SHADER, ssr_computeShader, "Screen Space Reflections Compute Shader");
    Sys_Render.ssrShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Screen Space Reflections Shader Program");
    
    computeShader = CompileShader(GL_COMPUTE_SHADER, voxelUpdate_computeShader, "Voxel Update Compute Shader");
    Sys_Render.voxelUpdateShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Voxel Update Shader Program");
        
    computeShader = CompileShader(GL_COMPUTE_SHADER, shadowmaps_clear_computeShader, "Shadowmaps Clear Compute Shader");
    Sys_Render.shadowmapsClearShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Shadowmaps Clear Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");
    Sys_Render.imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program");
}

GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage) {
    glGenBuffers(1, id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, *id);
    return *id;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass from camera world position + orientation
void mat4_lookat_from(float* m, Quaternion* camRotation, Vector3 eye) { // Kept around for light views for shadowmap cubemap faces.
    float x = camRotation->x, y = camRotation->y, z = camRotation->z, w = camRotation->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    Vector3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
    Vector3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
    Vector3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
    m[0]  = right.x;   m[1]  = up.x;   m[2]  = -forward.x; m[3]  = 0.0f;
    m[4]  = right.y;   m[5]  = up.y;   m[6]  = -forward.y; m[7]  = 0.0f;
    m[8]  = right.z;   m[9]  = up.z;   m[10] = -forward.z; m[11] = 0.0f;
    m[12] = -dot_vector3(right, eye); m[13] = -dot_vector3(up, eye); m[14] = dot_vector3(forward, eye); m[15] = 1.0f;
}

__attribute__((pure)) bool SphereInFrustum(FrustumPlane* planes, Vector3 c, float radius) {
    if ((planes[0].normal.x * c.x + planes[0].normal.y * c.y + planes[0].normal.z * c.z + planes[0].d) < -radius) return false;
    if ((planes[1].normal.x * c.x + planes[1].normal.y * c.y + planes[1].normal.z * c.z + planes[1].d) < -radius) return false;
    if ((planes[2].normal.x * c.x + planes[2].normal.y * c.y + planes[2].normal.z * c.z + planes[2].d) < -radius) return false;
    if ((planes[3].normal.x * c.x + planes[3].normal.y * c.y + planes[3].normal.z * c.z + planes[3].d) < -radius) return false;
    if ((planes[4].normal.x * c.x + planes[4].normal.y * c.y + planes[4].normal.z * c.z + planes[4].d) < -radius) return false;
    if ((planes[5].normal.x * c.x + planes[5].normal.y * c.y + planes[5].normal.z * c.z + planes[5].d) < -radius) return false;
    return true;
}

void ExtractFrustumPlanes(float* m, FrustumPlane* planes) {
    planes[0].normal.x = m[3]  + m[0];  planes[0].normal.y = m[7]  + m[4];  planes[0].normal.z = m[11] + m[8];  planes[0].d = m[15] + m[12]; // Left
    planes[1].normal.x = m[3]  - m[0];  planes[1].normal.y = m[7]  - m[4];  planes[1].normal.z = m[11] - m[8];  planes[1].d = m[15] - m[12]; // Right
    planes[2].normal.x = m[3]  + m[1];  planes[2].normal.y = m[7]  + m[5];  planes[2].normal.z = m[11] + m[9];  planes[2].d = m[15] + m[13]; // Bottom
    planes[3].normal.x = m[3]  - m[1];  planes[3].normal.y = m[7]  - m[5];  planes[3].normal.z = m[11] - m[9];  planes[3].d = m[15] - m[13]; // Top
    planes[4].normal.x = m[3]  + m[2];  planes[4].normal.y = m[7]  + m[6];  planes[4].normal.z = m[11] + m[10]; planes[4].d = m[15] + m[14]; // Near
    planes[5].normal.x = m[3]  - m[2];  planes[5].normal.y = m[7]  - m[6];  planes[5].normal.z = m[11] - m[10]; planes[5].d = m[15] - m[14]; // Far
    for (int i = 0; i < 6; i++) {
        float len = vsqrtf(planes[i].normal.x*planes[i].normal.x + planes[i].normal.y*planes[i].normal.y + planes[i].normal.z*planes[i].normal.z);
        if (len > 0.0f) {
            planes[i].normal.x /= len; planes[i].normal.y /= len; planes[i].normal.z /= len; planes[i].d /= len; // Normalize
        }
    }
}

Quaternion cubemapOrientationQuaternion[6] = {
    {0.0f, 0.707106781f, 0.0f, 0.707106781f},  // +X: Right
    {0.0f, -0.707106781f, 0.0f, 0.707106781f}, // -X: Left
    {-0.707106781f, 0.0f, 0.0f, 0.707106781f}, // +Y: Up
    {0.707106781f, 0.0f, 0.0f, 0.707106781f},  // -Y: Down
    {0.0f, 0.0f, 0.0f, 1.0f},                  // +Z: Forward
    {0.0f, 1.0f, 0.0f, 0.0f}                   // -Z: Backward
};

uint32_t voxelLightLists[VOXEL_COUNT * 24];
uint32_t voxelLightListCounts[VOXEL_COUNT];
bool lightInPVS[LIGHT_COUNT];
Vector3 lightsNewPosition[LIGHT_COUNT];
void MoveLight(uint16_t lightIdx, Vector3 newPos) {
    lightsNewPosition[lightIdx] = newPos;
    lightDirty[lightIdx] = true;
}

bool UpdateLights(bool* voxelsNeedUpdated) {
    if (Sys_Global.gamePaused || Sys_Global.menuActive) return false;
    
    for (uint16_t lightIdx = 0; lightIdx < loadedLights; ++lightIdx) {        
        if (lightDirty[lightIdx]) { // Marked all as true at level load.
            *voxelsNeedUpdated = true;
            uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;

            // Remove light from all voxel lists at current location before moving light
            Vector3 lightPosOld = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
            uint16_t voxXold = (uint16_t)clamp((int32_t)vfloor((lightPosOld.x - voxelMinCenterX) / VOXEL_SIZE), 0, 511);
            uint16_t voxZold = (uint16_t)clamp((int32_t)vfloor((lightPosOld.z - voxelMinCenterZ) / VOXEL_SIZE), 0, 511);
            float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            int r1 = vceil(range * (1.0f / VOXEL_SIZE));
            int voxMinX = vmax(voxXold - r1,0);
            int voxMaxX = vmin((int)voxXold + r1,511);
            int voxMinZ = vmax(voxZold - r1,0);
            int voxMaxZ = vmin((int)voxZold + r1,511);
//             uint32_t subtractCount = 0;
            for (int ix = voxMinX; ix <= voxMaxX; ++ix) {
                for (int iz = voxMinZ; iz <= voxMaxZ; ++iz) {
                    uint32_t voxelIndex = (iz * 512) + ix;
                    for (int i=0; i<MAX_LIGHTS_PER_VOXEL; ++i) {
                        uint32_t currentVoxLightListIndex = (voxelIndex * MAX_LIGHTS_PER_VOXEL) + i;
                        if (voxelLightLists[currentVoxLightListIndex] == lightIdx) {
                            voxelLightLists[currentVoxLightListIndex] = VOXEL_LIGHT_IDX_CLEAR_VALUE; // Found this light, clear it.
                            for (uint32_t j=currentVoxLightListIndex;j<vmin(voxelLightListCounts[voxelIndex],MAX_LIGHTS_PER_VOXEL);++j) {
                                voxelLightLists[j] = voxelLightLists[j+1]; // Shift remaining list down
                            }
                            
                            if (voxelLightListCounts[voxelIndex] > 0) voxelLightListCounts[voxelIndex]--; // Decrease count for this list.
//                             subtractCount++;
                            break; // Light only should exist once in the list.
                        }
                    }
                }
            }
            
//             if (subtractCount > 0) DualLog("Light %u subtracted itself from %u voxels' lists with range %f, at voxel %u, %u with vox with vmins %u,%u and vmaxs %u,%u\n", lightIdx, subtractCount, (double)range, voxXold, voxZold, voxMinX, voxMinZ, voxMaxX, voxMaxZ);

            // Update to new position
            lights[litIdx + LIGHT_DATA_OFFSET_POSX] = lightsNewPosition[lightIdx].x;
            lights[litIdx + LIGHT_DATA_OFFSET_POSY] = lightsNewPosition[lightIdx].y;
            lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = lightsNewPosition[lightIdx].z;
            Vector3 lightPos = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };

            // Now update voxel light lists for voxels in range of new light position
            uint16_t voxX = (uint16_t)clamp((int32_t)vfloor((lightPos.x - voxelMinCenterX) / VOXEL_SIZE), 0, 511);
            uint16_t voxZ = (uint16_t)clamp((int32_t)vfloor((lightPos.z - voxelMinCenterZ) / VOXEL_SIZE), 0, 511);
            int r2 = vceil(range * (1.0f / VOXEL_SIZE));
            voxMinX = vmax(voxXold - r2,0);
            voxMaxX = vmin((int)voxX + r2,511);
            voxMinZ = vmax(voxZold - r2,0);
            voxMaxZ = vmin((int)voxZ + r2,511);
//             uint32_t voxelCountForLight = 0;
            for (int ix = voxMinX; ix <= voxMaxX; ++ix) {
                for (int iz = voxMinZ; iz <= voxMaxZ; ++iz) {
                    uint32_t voxelIndex = (iz * 512) + ix;
                    if (voxelLightListCounts[voxelIndex] >= MAX_LIGHTS_PER_VOXEL) continue; // Voxel is full, skip it.

                    uint32_t currentVoxLightListIndex = (voxelIndex * MAX_LIGHTS_PER_VOXEL) + voxelLightListCounts[voxelIndex];
                    voxelLightLists[currentVoxLightListIndex] = lightIdx; // Put light into the list for this voxel.
                    voxelLightListCounts[voxelIndex]++;
//                     voxelCountForLight++;
//                     if (voxelLightListCounts[voxelIndex] >= MAX_LIGHTS_PER_VOXEL) DualLogWarn("Voxel filled up at voxel %u, %u for light at %f, %f\n", ix, iz, (double)lightPos.x, (double)lightPos.z);
                }
            }
            
//             if (voxelCountForLight > 0) DualLog("Light %u added itself to %u voxels' lists with range %f, at voxel %u, %u with vox with vmins %u,%u and vmaxs %u,%u\n", lightIdx, voxelCountForLight, (double)range, voxXold, voxZold, voxMinX, voxMinZ, voxMaxX, voxMaxZ);

            #pragma GCC unroll 6
            for (int j=0;j<6;++j) {
                mat4_lookat_from((float*)lightView[lightIdx][j], &cubemapOrientationQuaternion[j], lightPos);
                mul_mat4((float*)lightViewProj[lightIdx][j], shadowmapsPerspectiveProjection, (float*)lightView[lightIdx][j]);
                ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j], lightFrustumPlanes[lightIdx][j]);
            }
            
            uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((lightPos.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
            uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((lightPos.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
            int lightCellIdx = (cellZ * WORLDX) + cellX;
            int r = vceil(range * (1.0f / WORLDCELL_WIDTH_F));
            lightInPVS[lightIdx] = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
            if (!lightInPVS[lightIdx]) {
                for (int ix = cellX - r; ix <= (int)cellX + r; ++ix) {
                    for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                        if (!XZPairInBounds(ix, iz)) continue;
                        
                        int subIdx = iz * WORLDX + ix;
                        if (get_cull_bit(precomputedVisibleCellsFromHere, lightCellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) {
                            lightInPVS[lightIdx] = true;
                            break;
                        }
                    }
                }
            }
        }
    }
        
    for (int i=0;i<loadedLights;++i) { // Just lerps/flickers in intensity
        if (lightIntervalStepsLength[i] < 1) continue;
        
        int litIdx = i * LIGHT_DATA_SIZE;
        if (!lightOn[i]) { lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[i]; continue; }

        float differenceInIntensity = (lightMaxIntensity[i] - lightMinIntensity[i]);
        if (lightLerpTime[i] < (float)Sys_Global.pauseRelativeTime) {
            lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpUp[i] ? lightMaxIntensity[i] : lightMinIntensity[i]; // Pick target to lerp towards
            lightLerpUp[i] = !lightLerpUp[i];
            lightCurrentStep[i]++;
            if (lightCurrentStep[i] >= lightIntervalStepsLength[i]) lightCurrentStep[i] = 0; // Wrap and start over continuous looping
            lightLerpStepTime[i] = lightIntervalSteps[i][lightCurrentStep[i]];
            lightLerpTime[i] = (float)Sys_Global.pauseRelativeTime + lightLerpStepTime[i];
            lightLerpStartTime[i] = (float)Sys_Global.pauseRelativeTime;
        } else if (lightLerpOn[i]) {
            if (lightCurrentStep[i] < lightIntervalStepIsLerpingLength[i]) {
                if (intervalStepisLerping[i][lightCurrentStep[i]]) {
                    lightLerpValue[i] = ((float)Sys_Global.pauseRelativeTime - lightLerpStartTime[i])/(lightLerpTime[i] - lightLerpStartTime[i]); // percent towards goal time
                    float lerpVal = lightLerpUp[i] ? (lightLerpValue[i]) : (1.0f - lightLerpValue[i]);
                    lightLerpValue[i] = lightMinIntensity[i] + (differenceInIntensity * lerpVal);
                    lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpValue[i];
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.lightsID); glBufferData(GL_SHADER_STORAGE_BUFFER, loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
//     if (*voxelsNeedUpdated) {
//         float px = instances[PLAYER1].position.x; float py = instances[PLAYER1].position.y; float pz = instances[PLAYER1].position.z;
//         float fx = instances[PLAYER1].forward.x;  float fy = instances[PLAYER1].forward.y;  float fz = instances[PLAYER1].forward.z;
//         glUseProgram(Sys_Render.voxelUpdateShaderProgram);
//         glUniform3f(5, px, py, pz);
//         glUniform3f(6, fx, fy, fz);
//         GLuint groupX_voxels = (512 + 31) / 32;
//         GLuint groupZ_voxels = (512 + 31) / 32; // Actually just a local size y, but for z axis voxels
//         glDispatchCompute(groupX_voxels,groupZ_voxels, 1);
//     }

    glNamedBufferData(Sys_Render.voxelLightListCountsID, VOXEL_COUNT * sizeof(uint32_t), voxelLightListCounts, GL_DYNAMIC_DRAW);
    glNamedBufferData(Sys_Render.uniqueLightListsID, VOXEL_COUNT * 24 * sizeof(uint32_t), voxelLightLists, GL_DYNAMIC_DRAW);
    return *voxelsNeedUpdated;
}

typedef struct {
    float depth;
    uint16_t index;
} DepthSort;

__attribute__((pure)) int32_t compareDepthSort(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth;
    float db = ((const DepthSort*)b)->depth;
    return (db > da) - (db < da);
}

__attribute__((pure)) int32_t compareDepthSortInverted(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth;
    float db = ((const DepthSort*)b)->depth;
    return (da > db) - (da < db);
}

// ============================================================================
// UI Rendering and Text
float GetScreenRelativeX(float percentage) { return (float)Sys_Settings.ScreenWidth * percentage; }
float GetScreenRelativeY(float percentage) { return (float)Sys_Settings.ScreenHeight * percentage; }

void RenderUIImage(float x, float y, float width, float height, uint32_t texIndex) {
    glEnable(GL_BLEND);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!  2nd Clear needed or UI dissappears/flickers!!
    glDisable(GL_CULL_FACE);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glBindVertexArray(Sys_Render.textVAO);
    glUniform1ui(1, 0);
    glUniform1ui(3, 1u);  // isUI true
    glUniform1ui(17, 1u); // unlit is true
    glUniform1ui(19, 0);
    glUniform1ui(20, 0);
    glUniformMatrix4fv(2, 1, GL_FALSE, uiOrthoProjection);
    glBindBuffer(GL_ARRAY_BUFFER, Sys_Render.textVBO);
    float x1 = x + width;
    float y1 = y + height;
    float z = 0.0f;
    float vertices[30] = { x, y1, z, 0.0f, 0.0f, x1,  y, z, 1.0f, 1.0f, x1, y1, z, 1.0f, 0.0f, x, y1, z, 0.0f, 0.0f, x,  y, z, 0.0f, 1.0f, x1,  y, z, 1.0f, 1.0f };
    glUniform1ui(18, texIndex);
    glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    Sys_Dx.drawCallsRenderedThisFrame++;
    Sys_Dx.uiImageDrawCallsRenderedThisFrame++;
    Sys_Dx.verticesRenderedThisFrame += 6;    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return (   cursorPosition_x >= startX && cursorPosition_x <= endX     // 0 == left
            && cursorPosition_y >= endY   && cursorPosition_y <= startY); // 0 == top
}

Color textColors[TEXT_COLOR_COUNT] = {
    {         1.0f,         1.0f,          1.0f, 1.0f}, // 0 White
    { 0.890196078f, 0.874509804f,          0.0f, 1.0f}, // 1 Yellow
    { 0.623529412f, 0.611764706f,          0.0f, 1.0f}, // 2 Dark Yellow 0.8902f * 0.7f, 0.8745f * 0.7f, 0f
    { 0.372549020f, 0.654901961f,  0.168627451f, 1.0f}, // 3 Green
    { 0.917647059f, 0.137254902f,  0.168627451f, 1.0f}, // 4 Red
    {         1.0f, 0.498039216f,          0.0f, 1.0f}, // 5 Orange
    { 0.674509804f, 0.058823529f,  0.070588235f, 1.0f}, // 6 StopD Red
    { 0.941176471f, 0.282352941f,  0.298039216f, 1.0f}, // 7 StopD Red Highlight
    { 0.909803922f, 0.203921569f,  0.219607843f, 1.0f}  // 8 StopD Red Pause Title
};
float textVertexData[8192]; // Reusable buffer for text vertices.  Most text only needs ~3000
void RenderFormattedText(float x, float y, uint32_t color, uint8_t fontID, const char * restrict format, ...) {
    va_list args;
    va_start(args, format); vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args); va_end(args);
    glUseProgram(Sys_Render.textShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, uiOrthoProjection);
    glUniform4f(3, textColors[color].r, textColors[color].g, textColors[color].b, textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6, fontAtlasTexStopD);
    else glBindTextureUnit(6, fontAtlasTex);
    
    glUniform2f(4, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glUniform1ui(2, fontID);
    glUniform1i(1, 6); // textTexture sampler2D
    glBindVertexArray(Sys_Render.textVAO);
    size_t vertexCount = 0;
    const char* p = uiTextBuffer;
    float xpos = x, ypos = y + GetScreenRelativeY(0.0211f);
    float lineSpacing = GetScreenRelativeY(0.03f); // Match RenderUI
    stbtt_aligned_quad q;
    int characterCount = 0;
    float paddingUV = 12.0f / (float)FONT_ATLAS_SIZE; // This is for the black outline around all text for readability.
    float borderWidthPixels = 2.0f;
    while (*p) {
        uint32_t codepoint = DecodeUTF8(&p);
        characterCount++;
        if (codepoint == '\n' || characterCount > 120) {
            xpos = x;
            ypos += lineSpacing;
            characterCount = 0;
            continue;
        }

        int idx = CodepointToPackedIndex(codepoint, fontID);
        if (idx < 0) continue;

        if (fontID == FONT_STOPD) stbtt_GetPackedQuad(fontPackedCharStopD, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        else stbtt_GetPackedQuad(fontPackedChar, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        float vx0 = q.x0 - borderWidthPixels;
        float vy0 = q.y0 - borderWidthPixels;
        float vx1 = q.x1 + borderWidthPixels;
        float vy1 = q.y1 + borderWidthPixels;
        float s0 = q.s0 - paddingUV;
        float t0 = q.t0 - paddingUV;
        float s1 = q.s1 + paddingUV;
        float t1 = q.t1 + paddingUV;
        float z = 0.0f;
        float textVertices[30] = { vx0, vy0, z, s0, t0, vx1, vy1, z, s1, t1, vx1, vy0, z, s1, t0, vx0, vy0, z, s0, t0, vx0, vy1, z, s0, t1, vx1, vy1, z, s1, t1 };
        memcpy(textVertexData + vertexCount * 30, textVertices, sizeof(textVertices));
        vertexCount++;
        if (codepoint >= '0' && codepoint <= '9') {
            if (fontID == FONT_STOPD) xpos = q.x0 + fixedNumberAdvanceWidthStopD;
            else xpos = q.x0 + fixedNumberAdvanceWidth;
        }
    }
    
    if (vertexCount > 0) {
        glNamedBufferData(Sys_Render.textVBO, vertexCount * 30 * sizeof(float), textVertexData, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
        Sys_Dx.drawCallsRenderedThisFrame++;
        Sys_Dx.textDrawCallsRenderedThisFrame++;
        Sys_Dx.verticesRenderedThisFrame += vertexCount * 6;
    }
}

void RenderLoadingProgress(int32_t offset, const char * restrict text) { // Only adds 0.01secs to game startup time.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderFormattedText(Sys_Settings.ScreenWidth / 2 - offset, Sys_Settings.ScreenHeight / 2 - 5, TEXT_WHITE, FONT_NORMAL, text);
    glfwSwapBuffers(Sys_Global.window);
}

char statusText[TEXT_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(statusText, TEXT_BUFFER_SIZE, fmt, args);
    va_end(args);
    DualLog("%s\n",statusText);
    Sys_Global.statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}

void NewGame(void) {
    RenderLoadingProgress(100,"Loading new game...");
    memset(&questData, 0, sizeof(QuestBits));
    questData.lev1SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent
    questData.lev2SecCode = random_range_u8(0u,9u); // these all being the same number.
    questData.lev3SecCode = random_range_u8(0u,9u);
    questData.lev4SecCode = random_range_u8(0u,9u);
    questData.lev5SecCode = random_range_u8(0u,9u);
    questData.lev6SecCode = random_range_u8(0u,9u);
    memset(instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    instances[PLAYER1].index = 767;
    instances[PLAYER1].layer = 12; // PhysicsLayer_Player
    instances[PLAYER1].position = (Vector3) { .x = 10.52f, .y = -43.792f + 0.84f, .z = 20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f y for cam offset from center
    instances[PLAYER1].scale = (Vector3) { 1.0f, 1.0f, 1.0f };
    instances[PLAYER1].rotation = (Quaternion){ .x = 0.0f, .y = 0.7071f, .z = 0.0f, .w = 0.7071f }; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    instances[PLAYER1].entflags = ENTFLAG_ACTIVE | ENTFLAG_USEGRAVITY | ENTFLAG_RIGIDBODY;
    instances[PLAYER1].collider = COLLIDER_TYPE_CAPSULE;
    instances[PLAYER1].colliderCenter.y = 0.84f;
    instances[PLAYER1].colliderSize = (Vector3) { .x = 0.48f, .y = 2.0f, .z = 1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    instances[PLAYER1].mass = 1.0f;
    instances[PLAYER1].linearDrag = 8.0f;
    instances[PLAYER1].dynamicFriction = 0.6f;
    instances[PLAYER1].staticFriction = 0.8f;
    instances[PLAYER1].frictionCombine = PHYS_COMBINE_MUL;
//     instances[PLAYER1].physics_handle = Physics_CreateCharacterCapsule(instances[PLAYER1].colliderSize.x, instances[PLAYER1].colliderSize.y, instances[PLAYER1].position, PhysicsLayer_Player, instances[PLAYER1].mass, false); // false == dynamic
//     Physics_CreatePlayer(instances[PLAYER1].position);
    LoadLevel(Sys_Global.startLevel); // Must be after entities!
    Sys_Global.pauseRelativeTime = 0.0;
    Sys_Global.last_physics_time = get_time();
    Sys_Global.last_topframe_time = Sys_Global.last_physics_time - 0.05;
}

#define MAX_DEBUG_LINE_VERTS 4096                // 2048 lines max per frame
float debugLineBuffer[MAX_DEBUG_LINE_VERTS * 3]; // xyz only

void InitializeEnvironment(int32_t argc, char* command, char* command_input1) {
    double init_start_time = get_time();
    Sys_Dx.globalFrameNum = 0;
    DebugRAM("prior to event system init");
    DualLog("%s by W. Josiah Jack, MIT-0 licensed\n", EngineName);
    EventSystemInit(argc,command,command_input1);
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); OS_Exit(1); }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, 0);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    LoadConfig(); // Get settings before setting window size.
    Sys_Global.window = glfwCreateWindow(Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, "Voxen", NULL, NULL);
    glfwSetFramebufferSizeCallback(Sys_Global.window, UpdateScreenSize);
    if (!Sys_Global.window) { DualLogError("glfwCreateWindow failed\n"); OS_Exit(1); }
        
    glfwMakeContextCurrent(Sys_Global.window);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { DualLogError("Failed to initialize GLAD\n"); OS_Exit(1); }
    
    CycleToNextMonitor(Sys_Global.window);
    glfwSetInputMode(Sys_Global.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    DualLog("OpenGL Version: %s, ", (const char*)glGetString(GL_VERSION));
    DualLog("GPU: %s", (const char*)glGetString(GL_RENDERER));
    OS_CPUInfo();
    Input_Init(Sys_Global.window);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Globally same alpha blending
    CompileShaders();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    GLuint vaos[4]; GLuint vbos[4];
    glCreateVertexArrays(4, vaos);
    glCreateBuffers(3, vbos);
    Sys_Render.quadVAO = vaos[0]; Sys_Render.vao_chunk = vaos[1]; Sys_Render.textVAO = vaos[2]; Sys_Render.debugLinesVAO = vaos[3];
    Sys_Render.quadVBO = vbos[0];                                     Sys_Render.textVBO = vbos[1]; Sys_Render.debugLinesVBO = vbos[2];
    float quadBlit_vertices[] = { 1.0f, -1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,    -1.0f,1.0f, 0.0f, 1.0f,   -1.0f, -1.0f, 0.0f, 0.0f }; // 4 verts, 4 floats each pos.xy, uv.xy
    glNamedBufferData(Sys_Render.quadVBO, sizeof(quadBlit_vertices), quadBlit_vertices, GL_STATIC_DRAW);

    glVertexArrayAttribFormat(Sys_Render.quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    glVertexArrayAttribFormat(Sys_Render.quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(Sys_Render.quadVAO, 0, Sys_Render.quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.quadVAO, i, 0); glEnableVertexArrayAttrib(Sys_Render.quadVAO, i); }
    
    glVertexArrayAttribFormat(Sys_Render.vao_chunk, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexArrayAttribFormat(Sys_Render.vao_chunk, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexArrayAttribFormat(Sys_Render.vao_chunk, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    for (uint8_t i = 0; i < 3; i++) { glVertexArrayAttribBinding(Sys_Render.vao_chunk, i, 0); glEnableVertexArrayAttrib(Sys_Render.vao_chunk, i); }
    
    glVertexArrayAttribFormat(Sys_Render.textVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(Sys_Render.textVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(Sys_Render.textVAO, 0, Sys_Render.textVBO, 0, 5 * sizeof(float));
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.textVAO, i, 0); glEnableVertexArrayAttrib(Sys_Render.textVAO, i); }
    
    glNamedBufferStorage(Sys_Render.debugLinesVBO, MAX_DEBUG_LINE_VERTS * 3 * sizeof(float), NULL, GL_DYNAMIC_STORAGE_BIT);  // persistent, client-writable
    glVertexArrayAttribFormat(Sys_Render.debugLinesVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(Sys_Render.debugLinesVAO, 0);
    glVertexArrayAttribBinding(Sys_Render.debugLinesVAO, 0, 0);
    glVertexArrayVertexBuffer(Sys_Render.debugLinesVAO, 0, Sys_Render.debugLinesVBO, 0, 3 * sizeof(float));

    float* m = shadowmapsPerspectiveProjection;
    m[0] = 1.0f; m[1] = 0.0f; m[2] =                                                                  0.0f; m[3] =  0.0f;
    m[4] = 0.0f; m[5] = 1.0f; m[6] =                                                                  0.0f; m[7] =  0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10]=      -(LIGHT_RANGE_MAX + NEAR_PLANE) / (LIGHT_RANGE_MAX - NEAR_PLANE); m[11]= -1.0f;
    m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * LIGHT_RANGE_MAX * NEAR_PLANE / (LIGHT_RANGE_MAX - NEAR_PLANE); m[15]=  0.0f;
    InitFontAtlasses();
    RenderLoadingProgress(80,"Loading...");
    InitializeAudio(); // Audio    
    ParseGameData();
    glGenFramebuffers(1, &Sys_Render.gBufferFBO);
    ApplySettings(); // After loading of text and game data.
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) DualLogError("Framebuffer incomplete: Error code %d\n", status);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Needed to render loading progress.
    glDepthMask(GL_TRUE); // Always true, set just once ever.
    glfwSetWindowTitle(Sys_Global.window, Sys_Global.global_modname);
    int fp = OS_OpenReadonly("./Textures/UI/menudot1.png");
    int windowIconFileSize = OS_FileSize(fp);
    uint8_t* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize, fp, "./Textures/UI/menudot1.png");
    OS_Close(fp);
    int w = 1, h = 1;
    stbi__arena_init();
    unsigned char* pixels = stbi_load_from_memory(file_buffer, windowIconFileSize, &w, &h);
    if (!pixels) { DualLogError("Failed to load icon: ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
    
    GLFWimage image;
    image.width  = w;
    image.height = h;
    image.pixels = pixels;
    glfwSetWindowIcon(Sys_Global.window, 1, &image);
    file_buffer = OS_DeallocateRAM(file_buffer, windowIconFileSize);
    stbi__arena_base = OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE);
    DebugRAM("after freeing window bar icon");
    DualLog("GL buffers, FBO, fonts, audio, localization, and window init took %f secs\n", get_time() - init_start_time);
    LoadEntities();
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(&modelMatrices[0], mat, 16 * sizeof(float)); // Null instance matrix used for UI
    Sys_Render.cellVisibleDataID       = SetupSSBO(&Sys_Render.cellVisibleDataID,        4, ARRSIZE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.shadowMapSSBO           = SetupSSBO(&Sys_Render.shadowMapSSBO,            5, TOTAL_SHADOWMAP_PIXELS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);    
    Sys_Render.voxelLightListCountsID  = SetupSSBO(&Sys_Render.voxelLightListCountsID,   6, VOXEL_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.shadowMapsIndirectionID = SetupSSBO(&Sys_Render.shadowMapsIndirectionID,  8, LIGHT_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.matricesBufferID        = SetupSSBO(&Sys_Render.matricesBufferID,        11, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices, GL_STATIC_DRAW);
    Sys_Render.colorBufferID           = SetupSSBO(&Sys_Render.colorBufferID,           12, MAX_TOTAL_PIXELS * sizeof(uint8_t), NULL, GL_STATIC_DRAW);
    Sys_Render.blueNoiseBuffer         = SetupSSBO(&Sys_Render.blueNoiseBuffer,         13, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
    Sys_Render.textureOffsetsID        = SetupSSBO(&Sys_Render.textureOffsetsID,        14, MAX_VALID_TEXTURE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.textureSizesID          = SetupSSBO(&Sys_Render.textureSizesID,          15, MAX_VALID_TEXTURE * 2 * sizeof(int32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.texturePalettesID       = SetupSSBO(&Sys_Render.texturePalettesID,       16, MAX_UNIQUE_COLORS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.texturePaletteOffsetsID = SetupSSBO(&Sys_Render.texturePaletteOffsetsID, 17, MAX_VALID_TEXTURE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.lightsID                = SetupSSBO(&Sys_Render.lightsID,                19, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_STATIC_DRAW);
    Sys_Render.uniqueLightListsID       = SetupSSBO(&Sys_Render.uniqueLightListsID,       27,  VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
//     play_mp3("./Audio/music/TITLOOP-00_menu.mp3",((float)Sys_Settings.VolumeMusic/100.0f) * 0.4f + 0.09f,1500);
    NewGame(); // TODO: Do this from menu not immediately lol
    DebugRAM("InitializeEnvironment end");
}

void DrawDebugLines(float* viewProj) {    
    glNamedBufferSubData(Sys_Render.debugLinesVBO, 0, Sys_Dx.debugLineVertCount * sizeof(float), debugLineBuffer);
    glUseProgram(Sys_Render.debugUnlitShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj);
    glLineWidth(10.0f);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(Sys_Render.debugLinesVAO);
    glDrawArrays(GL_LINES, 0, Sys_Dx.debugLineVertCount / 3);
    glEnable(GL_DEPTH_TEST);
    Sys_Dx.drawCallsRenderedThisFrame++;
    Sys_Dx.verticesRenderedThisFrame += Sys_Dx.debugLineVertCount / 3;
    Sys_Dx.debugLineVertCount = 0;
}

void AddDebugLine(Vector3 start, Vector3 end) {
    int32_t i = Sys_Dx.debugLineVertCount;
    debugLineBuffer[i++] = start.x; debugLineBuffer[i++] = start.y; debugLineBuffer[i++] = start.z;
    debugLineBuffer[i++] =   end.x; debugLineBuffer[i++] =   end.y; debugLineBuffer[i++] =   end.z;
    Sys_Dx.debugLineVertCount = i;
}

__attribute__((pure)) bool EntityIsAnimated(uint16_t entIdx) {
    return (   entIdx == 53
            || entIdx == 79
            || (entIdx >= 420 && entIdx <= 442)
            || (entIdx >= 496 && entIdx <= 514)
            || entIdx == 585
            || entIdx == 602
            || (entIdx >= 609 && entIdx <= 614)
            || (entIdx >= 741 && entIdx <= 745));
}

bool StepLoopingAnim(uint16_t i) {
    bool portalsNeedUpdated = false;
    AnimationClip currentClip = modelAnimationClips[instances[i].animationNum][instances[i].clip];
    if (instances[i].currentFrameFinished < Sys_Global.current_time) {
        instances[i].currentFrameFinished = Sys_Global.current_time + ((double)currentClip.speed * (1.0 / (double)currentClip.framerate));
        instances[i].frame++;
        if (instances[i].frame > currentClip.frameEnd) instances[i].frame = currentClip.frameStart;
        else if (instances[i].frame < currentClip.frameStart) instances[i].frame = currentClip.frameEnd;

        instances[i].modelIndex = (currentClip.frameStartModelIndex + (instances[i].frame - currentClip.frameStart));
        dirtyInstances[i] = true;
        if (EntityIndexIsPortalBlockingDoor(instances[i].index)) {
            uint8_t portalIdx = instances[i].portalIndex;
            if (portalIdx < MAX_PORTALS) {
                uint16_t closedModelIndex = 719;
                switch(instances[i].index) { // TODO Make this data driven from entities.txt on each, or models.txt
                    case 496: closedModelIndex =  719; break; // doorA
                    case 497: closedModelIndex =  699; break; // doorB
                    case 498: closedModelIndex = 1398; break; // doorC
                    case 499: closedModelIndex = 1301; break; // doorD
                    case 500: closedModelIndex = 1612; break; // doorE
                    case 501: closedModelIndex = 1652; break; // doorF
                    case 503: closedModelIndex = 1742; break; // doorH
                    case 504: closedModelIndex = 1792; break; // doorI
                    case 508: closedModelIndex = 1845; break; // door_elevator1
                    case 509: closedModelIndex = 1887; break; // door_elevator2
                    case 510: closedModelIndex = 1929; break; // door_elevator3
                    case 511: closedModelIndex = 1973; break; // door_elevator4
                    case 512: closedModelIndex = 2078; break; // door_secret1
                    case 513: closedModelIndex = 2036; break; // door_secret2
                    case 514: closedModelIndex = 2120; break; // door_secret3
                }
            
                bool currentState = activePortals[portalIdx].open;
                if (instances[i].modelIndex == closedModelIndex && currentState) {
                    activePortals[portalIdx].open = false;
                    activePortals[portalIdx].dirty = true;
                    portalsNeedUpdated = true;
                } else if (instances[i].modelIndex != closedModelIndex && !currentState) {
                    activePortals[portalIdx].open = true;
                    activePortals[portalIdx].dirty = true;
                    portalsNeedUpdated = true;
                }
            }
        }
    }
    
    return portalsNeedUpdated;
}

void PortalCulling(void);

void UpdateAnims(void) {
    bool portalsNeedUpdated = false;
    uint16_t endOfModels = loadedInstances - invalidModelIndexCount;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < endOfModels; ++i) {
        if (instances[i].animationNum >= MAX_ANIMATED_MODELS) continue; // Invalid animated model index
        if (instances[i].numclips >= MAX_ANIMATION_CLIPS_PER_MODEL) continue; // Invalid animation clip index
        if (instances[i].numclips == 0) continue; // Invalid animation clip index
        
        if (EntityIsAnimated(instances[i].index)) {
            if (StepLoopingAnim(i)) portalsNeedUpdated = true;
        }
    }
    
    if (portalsNeedUpdated) PortalCulling();
}

#define FROB_DISTANCE 4.9f
void Frob(Vector3 pos, Vector3 forward, Vector3 right) {
    float offsetX = cursorPosition_x - (Sys_Settings.ScreenWidth * 0.5f);
    float offsetY = cursorPosition_y - (Sys_Settings.ScreenHeight * 0.5f);
    float ndcX = offsetX / (Sys_Settings.ScreenWidth * 0.5f);
    float ndcY = -offsetY / (Sys_Settings.ScreenHeight * 0.5f);  // flip Y
    float tanFov = tanf((float)Sys_Settings.FOV * 0.5f * PI / 180.0f);
    Vector3 view = (Vector3){ ndcX * tanFov * aspect3D, ndcY * tanFov, -1.0f };
    view = normalize_vector3(view);
    Vector3 flipForward = (Vector3){ -forward.x, -forward.y, -forward.z};
    Vector3 up = normalize_vector3( cross_vector3(right, flipForward) );
    Vector3 dir = (Vector3){ view.x * right.x + view.y * up.x + view.z * (flipForward.x),
                             view.x * right.y + view.y * up.y + view.z * (flipForward.y),
                             view.x * right.z + view.y * up.z + view.z * (flipForward.z) };
                             
    Sys_Dx.debugLine_start = pos;
    Sys_Dx.debugLine_end   = (Vector3){ dir.x * FROB_DISTANCE + pos.x, dir.y * FROB_DISTANCE + pos.y, dir.z * FROB_DISTANCE + pos.z };
    RaycastHit tempHit = Raycast(pos, dir, FROB_DISTANCE, LAYER_MASK_PLAYER_FROB);
    if (tempHit.hit) {
        Sys_Dx.debugLine_end = tempHit.point;
        DualLog("Raycast hit!  Hit object %u named of entity type %s(%u) at hit point %f %f %f\n", tempHit.hitInstanceIndex, GetPrefabNameFromIndex(instances[tempHit.hitInstanceIndex].index), instances[tempHit.hitInstanceIndex].index, (double)tempHit.point.x, (double)tempHit.point.y, (double)tempHit.point.z);
    }
    
    Sys_Dx.debugLineFinished = Sys_Global.current_time + 3.0;
}

void UpdateGameplay(void) {    
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_2].released) Frob(instances[PLAYER1].position, instances[PLAYER1].forward, instances[PLAYER1].right);
    if (Sys_Global.current_time < Sys_Dx.debugLineFinished && (Sys_Dx.debugLineVertCount + 6) < (MAX_DEBUG_LINE_VERTS * 3)) AddDebugLine(Sys_Dx.debugLine_start, Sys_Dx.debugLine_end);
    UpdateAmbientSounds();
    UpdateAnims();
}

#define SHADOW_NEARMESH_MAX 384 // 350 was too low for light 712 on security atrium
#define SHADOW_LIGHT_THRESH 0.015f
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX]; // Found that this is typically around 172
float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];
bool UpdatedPlayerCell(void);

static inline bool CellNotVisible(uint16_t index) { return ((gridCellStates[index] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN); } // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(

typedef struct {
    uint16_t index; // Original index in lights array
    float distanceSquared; // Distance to camera squared
    float score; // Priority score (lower distance, higher intensity = higher priority)
    float radius;
    Vector3 position;
} LightCandidate;

void RenderShadowmaps(void) {    
    double shadowStartTime = get_time();
    glEnable(GL_DEPTH_TEST);
    LightCandidate candidates[MAX_SHADOWMAPS];
    uint16_t numberFoundLightCandidatesForShadows = 0;
    float bestScores[MAX_SHADOWMAPS];
    voxen_Shadow_System.numShadowsCouldRender = 0;
    Vector3 playerPos = instances[PLAYER1].position;
    float pfx = instances[PLAYER1].forward.x;    float pfy = instances[PLAYER1].forward.y;    float pfz = instances[PLAYER1].forward.z;
    for (uint16_t i = 0; i < loadedLights; ++i) { // Collect candidates: only lights that are enabled, within FAR_PLANE, and in PVS
        if (!lightCastsShadows[i]) continue;

        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        Vector3 lightPos = (Vector3){ lights[litIdx], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        float intensity = lightMaxIntensity[i];//lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        if (intensity < 0.1f) continue;
        
        float range =  lights[litIdx + LIGHT_DATA_OFFSET_RANGE] * 0.99f; // Discard 1% more lights/meshes for performance.
        float luminosity = (intensity / (range * range));
        if (luminosity < SHADOW_LIGHT_THRESH) continue;
        if (!lightInPVS[i]) continue;
        
        float dx = lightPos.x - playerPos.x;    float dy = lightPos.y - playerPos.y;    float dz = lightPos.z - playerPos.z;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        float dotResult = (dx*pfx + dy*pfy + dz*pfz);//dot_vector3(delta, instances[PLAYER1].forward);
        if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;
        
        float score = distSqrdToPlayer / vmax(intensity, 0.01f);
        if (dotResult > 0.5f || distSqrdToPlayer < 26.2144f) score *= 0.125f; // Favor lights in player's view cone or within 5.12 (2 world cells)
        else if (dotResult > 0.0f) score *= 0.25f; // Favor lights in player's view cone

        if (numberFoundLightCandidatesForShadows < MAX_SHADOWMAPS) {
            candidates[numberFoundLightCandidatesForShadows] = (LightCandidate){ i, distSqrdToPlayer, score, range, lightPos };
            bestScores[numberFoundLightCandidatesForShadows] = score;
            numberFoundLightCandidatesForShadows++;
        } else if (score < bestScores[0]) {  // Only compare against current worst
            // Find worst (highest score) and replace it
            int worstIdx = 0;
            for (uint32_t j = 1; j < numberFoundLightCandidatesForShadows; ++j) {
                if (bestScores[j] > bestScores[worstIdx]) worstIdx = j;
            }
            candidates[worstIdx] = (LightCandidate){ i, distSqrdToPlayer, score, range, lightPos };
            bestScores[worstIdx] = score;
        }

        voxen_Shadow_System.numShadowsCouldRender++;
    }

    uint32_t numLightsShadowmapsToRender = vmin(voxen_Shadow_System.numShadowsCouldRender, MAX_SHADOWMAPS);
    bool foundDirtyLight = false || Sys_Render.shadowmapsNeedUpdated;
    for (uint32_t i=0;i<numLightsShadowmapsToRender;++i) { if (lightDirty[candidates[i].index]) foundDirtyLight = true; }
    numLightsShadowmapsToRender = foundDirtyLight ? numLightsShadowmapsToRender : 0;
    if (numLightsShadowmapsToRender > 0) { // Added since there is now work between here and the for loop so this is beneficial to check.
        // Clear shadowmaps.  One might think that this would be less performant than standard shadowmap FBO with gl clears and textures but in fact this is faster on all but the oldest hardware (e.g. 10yrs old is fine, 13yrs suffers a small hit).
        glUseProgram(Sys_Render.shadowmapsClearShaderProgram); // Way faster
        for (uint32_t c=0;c<numLightsShadowmapsToRender;++c) {
            glUniform1ui(0, c);
            GLuint groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
            glDispatchCompute(groupX_shadClear,6,1);
        }

        Sys_Dx.shadowDrawCallsRenderedThisFrame = 0;
        memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsShaderProgram);
        uint32_t shadowmapOffsetHead = 0U;
        uint16_t endOfModels = loadedInstances - invalidModelIndexCount;
        uint16_t shadowCasterIndices[SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS];
        uint16_t numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<endOfModels;++i) {
            uint16_t mdx = instances[i].modelIndex;
            if (modelVertexCounts[mdx] < 3) continue;
            if (mdx >= loadedModelsMaxIndex) continue;
            if (instances[i].entflags & ENTFLAG_NO_SHADOWS) continue;

            bool cellNotVisible = CellNotVisible(PosGetCellCoords(instances[i].position.x, instances[i].position.z)); // Cache cell indices once per mesh rather than once per light.
            if (cellNotVisible && !(Sys_Global.currentLevel == 1 && (instances[i].index == 309 ||  instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
                if (EntityIndexIsPortalBlockingDoor(instances[i].index) && instances[i].portalIndex < MAX_PORTALS) {
                    Portal doorPortal = activePortals[instances[i].portalIndex];
                    uint16_t cellAIndex = (doorPortal.cellA.z * WORLDX) + doorPortal.cellA.x;
                    uint16_t cellBIndex = (doorPortal.cellA.z * WORLDX) + doorPortal.cellA.x;
                    if (CellNotVisible(cellAIndex) && CellNotVisible(cellBIndex)) continue; // Neither cell is visible for door
                } else continue;
            }

            shadowCasterIndices[numShadowCasters] = i;
            numShadowCasters++;
            if (numShadowCasters >= (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)) break; // Ran out of shadowcasters max for frame.
        }
        
        uint16_t numShadowingLightsHandled = 0, currentModelType = 0, currentTexIndex = 0;
        bool currentIsTransparent = 0;
        for (uint32_t c = 0; c < numLightsShadowmapsToRender; ++c) { // Render top MAX_SHADOWMAPS candidates
            uint16_t lightIdx = candidates[c].index;
            float effectiveRadius = vmin(candidates[c].radius, 15.36f);
            Vector3 lightPos = candidates[c].position;
            uint16_t nearbyMeshCount = 0;
            for (uint16_t shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) { // Skip player indices and start at 3
                uint16_t j = shadowCasterIndices[shadowCasterInstanceIdx];
                shadows_nearMeshRadii[nearbyMeshCount] = modelBounds[(instances[j].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 0.99f;
                Vector3 d = Vector3_A_minus_B(instances[j].position, lightPos);
                float distToLightSqrd = dot_vector3(d, d);
                float radSum = (effectiveRadius + shadows_nearMeshRadii[nearbyMeshCount]);
                if (distToLightSqrd >= radSum * radSum) continue;
                
                shadows_nearMeshes[nearbyMeshCount].index = j;
                shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++;
                if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }

            if (nearbyMeshCount < 1) continue;
            
//             glUseProgram(Sys_Render.shadowmapsClearShaderProgram); // Way faster
//             glUniform1ui(0, c);
//             GLuint groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
//             glDispatchCompute(groupX_shadClear,6,1);
          
//             glUseProgram(Sys_Render.shadowmapsShaderProgram);
            glUniform3f(3, candidates[c].position.x, candidates[c].position.y, candidates[c].position.z);
            voxen_Shadow_System.shadowmapIndirectionList[lightIdx] = numShadowingLightsHandled;
            bool lightPositionInPlayerFrustum = SphereInFrustum(playerFrustumPlanes, candidates[c].position, 0.64f); // Use some radius for floating point errors
            #pragma GCC unroll 6
            for (uint8_t face = 0; face < 6; face++) {                            
                if (!lightPositionInPlayerFrustum) { // Check if at least one of the four points of this cubemap face's frustum are within the player's frustum
                    bool faceOverlapsPlayerView = false;
                    switch (face) {
                        case 0: { // +X: Right
                                Vector3 corner0 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner0,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner1 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner1,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner2 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, -effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner2,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner3 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, -effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner3,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            }
                            break;
                        case 1: { // -X: Left
                                Vector3 corner4 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner4,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner5 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner5,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner6 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, -effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner6,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner7 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, -effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner7,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            }
                            break;
                        case 2: { // +Y: Up
                                Vector3 corner0 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner0,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner1 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner1,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner4 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner4,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner5 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner5,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            }
                            break;
                        case 3: { // -Y: Down
                                Vector3 corner2 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, -effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner2,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner3 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, -effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner3,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner6 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, -effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner6,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner7 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, -effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner7,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            }
                            break;
                        case 4: { // +Z: Forward
                                Vector3 corner0 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner0,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner2 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, -effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner2,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner4 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner4,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner6 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, -effectiveRadius, effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner6,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            }
                            break;
                        case 5: { // -Z: Backward
                                Vector3 corner1 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner1,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner3 = Vector3_A_plus_B(candidates[c].position, (Vector3){ effectiveRadius, -effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner3,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner5 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner5,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }

                                Vector3 corner7 = Vector3_A_plus_B(candidates[c].position, (Vector3){ -effectiveRadius, -effectiveRadius, -effectiveRadius });
                                if (dot_vector3(Vector3_A_minus_B(corner7,instances[PLAYER1].position), instances[PLAYER1].forward) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            }
                            break;
                    }

                    if (!faceOverlapsPlayerView) {
                        if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], playerPos, 0.48f)) continue;
                    }
                }
                
                glUniform1ui(2, face);
                glUniformMatrix4fv(1, 1, GL_FALSE, (float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7, shadowmapOffsetHead + (face * 36864));
                Sys_Dx.shadowDrawCallsRenderedThisFrame++;
                for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;            
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], instances[i].position, shadows_nearMeshRadii[j] * 1.41f)) continue;

                    int32_t modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && instances[i].lodIndex < loadedModelsMaxIndex ? instances[i].lodIndex : instances[i].modelIndex;
                    if (currentModelType != modelType) {
                        currentModelType = modelType;
                        glBindVertexBuffer(0, Sys_Render.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[currentModelType]);
                    }
                    
                    glUniform1ui(0, i);
                    if (currentTexIndex != instances[i].texIndex) { currentTexIndex = instances[i].texIndex; glUniform1ui(6, instances[i].texIndex); }
                    if (currentIsTransparent != isTransparent(instances[i].texIndex)) { currentIsTransparent = isTransparent(instances[i].texIndex); glUniform1ui(8, isTransparent(instances[i].texIndex)); }
                    glDrawElements(GL_TRIANGLES, modelTriangleCounts[currentModelType] * 3, GL_UNSIGNED_INT, 0);
                    Sys_Dx.drawCallsRenderedThisFrame++;
                    Sys_Dx.verticesRenderedThisFrame += modelTriangleCounts[currentModelType] * 3;
                }
            }

            shadowmapOffsetHead += (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) * 6;
            numShadowingLightsHandled++;
        }

        glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
        glNamedBufferData(Sys_Render.shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), voxen_Shadow_System.shadowmapIndirectionList, GL_DYNAMIC_DRAW);
    }
    
    Sys_Render.shadowmapsNeedUpdated = false;
    voxen_Shadow_System.shadowTime = get_time() - shadowStartTime;
}

static inline void RenderCompositePass(float px, float py, float pz, float * restrict viewProj, float * restrict invViewRot) {
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Sys_Render.inputImageID);
    glUniform1i(4, 4); // outputImage texture sampler2D
    float berserkTimeRemainingNormalized = berserkFinished > 0.0001f ? (berserkFinished - (float)Sys_Global.pauseRelativeTime) / PATCH_TIME_BERSERK : 0.0f;
    if (berserkFinished < (float)Sys_Global.pauseRelativeTime && berserkFinished > 0.0001f) berserkFinished = berserkTimeRemainingNormalized = 0.0f;
    glUniform1f(9, berserkTimeRemainingNormalized);
    glUniform1f(10, berserkSeedTime);
    glUniform1ui(11, Sys_Settings.Brightness);
    glUniform3f(12, deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
    glUniform3f(13, px, py, pz);
    glUniform1f(15, (float)Sys_Global.pauseRelativeTime * 0.1f);
    glUniform1ui(17, (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.currentLevel == LEVEL_CYBERSPACE);
    glUniform1ui(18, (gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.currentLevel != LEVEL_CYBERSPACE);
    glUniform1ui(19, ((Sys_Global.currentLevel >= 10 && Sys_Global.currentLevel < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    uint32_t shieldOnType = 0u; // No shield green tint.
    if (questData.ShieldActivated) {
        if (Sys_Global.currentLevel == 6 || Sys_Global.currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
        else if (Sys_Global.currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
    }
    
    glUniform1ui(20, shieldOnType);
    Color painStaticColor = GetPainStaticColor();
    glUniform3f(23, painStaticColor.r, painStaticColor.g, painStaticColor.b);
    glUniformMatrix4fv(24, 1, GL_FALSE, viewProj);
    glUniformMatrix3fv(25, 1, GL_FALSE, invViewRot);
    glUniform1i(27, 0); // Texture 0 for the rendered geometry color buffer
    glUniform1f(28, GetPainStatic());
    glBindVertexArray(Sys_Render.quadVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    Sys_Dx.drawCallsRenderedThisFrame++;
    Sys_Dx.verticesRenderedThisFrame += 4;
}

static inline double RenderUI(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Sys_Dx.drawCallsNormal = Sys_Dx.drawCallsRenderedThisFrame;
    float screenCenterX = (float)Sys_Settings.ScreenWidth / 2;
    float screenCenterY = (float)Sys_Settings.ScreenHeight / 2;
    float lineSpacing = GetScreenRelativeY(genericTextHeightFac);
    if (Sys_Global.gamePaused) {
        float pauseBGWidth = GetScreenRelativeX(0.24f), pauseBGHeight = GetScreenRelativeY(0.39f);
        float pauseBGX = screenCenterX - (pauseBGWidth * 0.5f);
        float pauseBGY = screenCenterY - (pauseBGHeight * 0.5f) + GetScreenRelativeY(0.08f);
        RenderUIImage(pauseBGX, pauseBGY, pauseBGWidth, pauseBGHeight, 1025); // Pause Menu background
        RenderUIImage(pauseBGX, pauseBGY, pauseBGWidth, pauseBGHeight, 1080); // Pause Menu background
        float quitGame_Height = GetScreenRelativeY(0.05f);
        RenderUIImage(pauseBGX, screenCenterY + GetScreenRelativeY(0.40f) - (quitGame_Height * 0.5f), pauseBGWidth, quitGame_Height, 950); // Pause Quit Game background
        
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 3.0f), screenCenterY - GetScreenRelativeY(0.3f), TEXT_STOPD_RED_PAUSETITLE, FONT_STOPD, "PAUSED");
        char* pauseButton_ResumeText = "RESUME";
        float pauseButton_ResumeWidth = (TextWidth(pauseButton_ResumeText,FONT_STOPD) * 0.5f);
        float pauseButton_ResumeHeight = GetScreenRelativeY(genericTextHeightFacStopD);
        float pauseButton_ResumeX = screenCenterX - pauseButton_ResumeWidth;
        float pauseButton_ResumeY = screenCenterY - GetScreenRelativeY(0.08f);
        uint8_t pauseButton_ResumeColor = TEXT_STOPD_RED;
        bool pauseButton_CursorIsAbove = CursorIsOverBounds(pauseButton_ResumeX - GetScreenRelativeX(genericTextWidthFacStopD), pauseButton_ResumeX + pauseButton_ResumeWidth,
                                                            pauseButton_ResumeY + (pauseButton_ResumeHeight * 0.5f), pauseButton_ResumeY - (pauseButton_ResumeHeight * 0.5f));
        
        if (pauseButton_CursorIsAbove) pauseButton_ResumeColor = TEXT_STOPD_RED_HIGHLIGHT;
        RenderFormattedText(pauseButton_ResumeX, pauseButton_ResumeY, pauseButton_ResumeColor, FONT_STOPD, "RESUME");
        
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 2.0f), screenCenterY + GetScreenRelativeY(0.00f), TEXT_STOPD_RED, FONT_STOPD, "LOAD");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 2.0f), screenCenterY + GetScreenRelativeY(0.08f), TEXT_STOPD_RED, FONT_STOPD, "SAVE");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 3.5f), screenCenterY + GetScreenRelativeY(0.16f), TEXT_STOPD_RED, FONT_STOPD, "OPTIONS");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 6.0f), screenCenterY + GetScreenRelativeY(0.24f), TEXT_STOPD_RED, FONT_STOPD, "QUIT TO MENU");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 4.5f), screenCenterY + GetScreenRelativeY(0.40f), TEXT_STOPD_RED, FONT_STOPD, "QUIT GAME");
    }
    
    // Diagnostics / Debugging
    float debugTextStartY = GetScreenRelativeY(0.075f);
    float leftPad = GetScreenRelativeX(0.0125f);
    if (!Sys_Cheats.noHUD && Sys_Cheats.showLocation) RenderFormattedText(leftPad, debugTextStartY, TEXT_WHITE, FONT_NORMAL, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f", (double)instances[PLAYER1].position.x, (double)instances[PLAYER1].position.y, (double)instances[PLAYER1].position.z, (double)instances[PLAYER1].rotation.x, (double)instances[PLAYER1].rotation.y, (double)instances[PLAYER1].rotation.z, (double)instances[PLAYER1].rotation.w);
    if (!Sys_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), TEXT_WHITE, FONT_NORMAL, "timeSinceLastPhysicsTick: %.6f, numShadowsCouldRender: %u, playerCellIdx: %u, numCellsVisible: %u", Sys_Global.timeSinceLastPhysicsTick, voxen_Shadow_System.numShadowsCouldRender, playerCellIdx, numCellsVisible);
    if (!Sys_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), TEXT_WHITE, FONT_NORMAL, "Player velocity: %.2f, %.2f, %.2f, accumulated force: %.2f, %.2f, %.2f", (double)instances[PLAYER1].velocity.x, (double)instances[PLAYER1].velocity.y, (double)instances[PLAYER1].velocity.z, (double)instances[PLAYER1].accumulatedForce.x, (double)instances[PLAYER1].accumulatedForce.y, (double)instances[PLAYER1].accumulatedForce.z);
    if (!Sys_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 3), TEXT_WHITE, FONT_NORMAL, "Test Entity %s Index: %u, Shadow cpu ms: %.3f", GetPrefabNameFromIndex(instances[editModeSelection].index), editModeTestEntityDefinition, voxen_Shadow_System.shadowTime * 1000);
    if (!Sys_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 4), TEXT_WHITE, FONT_NORMAL, "Player cell: %u, floor: %.3f, ceil: %.3f", instances[PLAYER1].cellIndex, (double)gridCellFloorHeight[instances[PLAYER1].cellIndex], (double)gridCellCeilingHeight[instances[PLAYER1].cellIndex]);
    if (Sys_Cheats.consoleActive) RenderFormattedText(leftPad, 0, TEXT_WHITE, FONT_NORMAL, "] %s",consoleEntryText);
    if (Sys_Global.statusTextDecayFinished > Sys_Global.current_time) RenderFormattedText(leftPad + (Sys_Settings.ScreenWidth / 2) - 220, screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), TEXT_WHITE, FONT_NORMAL, "%s",statusText);

    CreditsScroll();
    double time_now = get_time();
    if (Sys_Cheats.showFPS && !Sys_Cheats.noHUD) {
        double thisFrameTime = (time_now - Sys_Global.last_time) * 1000.0;
        double cpuFrameTime = Sys_Dx.cpuTime * 1000.0;
        uint8_t timingColor = TEXT_WHITE;
        if (vabs(thisFrameTime - cpuFrameTime) < 0.451) timingColor = TEXT_GREEN;
        if (thisFrameTime > 6.944444) timingColor = TEXT_RED;
        Sys_Dx.drawCallsRenderedThisFrame += 2; Sys_Dx.textDrawCallsRenderedThisFrame += 2; // Add two more for this text render ;)
        RenderFormattedText(leftPad, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL, "ms: %.2f, CPU %.2f", thisFrameTime,cpuFrameTime);
        RenderFormattedText(leftPad + 230.0f, debugTextStartY - lineSpacing, TEXT_WHITE, FONT_NORMAL, "(FPS: %d, Worst: %d), Drwclls: %d [G %d UI %d Txt %d Shd %d] Vrts: %d Edit:%u",
                            Sys_Dx.framesPerLastSecond, Sys_Dx.worstFPS, Sys_Dx.drawCallsRenderedThisFrame, Sys_Dx.drawCallsNormal, Sys_Dx.uiImageDrawCallsRenderedThisFrame,
                            Sys_Dx.textDrawCallsRenderedThisFrame, Sys_Dx.shadowDrawCallsRenderedThisFrame, Sys_Dx.verticesRenderedThisFrame, Sys_Cheats.editMode);
    }
    
    float shootModeWidth = GetScreenRelativeX(0.01639f), shootModeHeight = GetScreenRelativeX(0.01639f);
    float shootModePos_x = GetScreenRelativeX(0.5f) - (shootModeWidth * 0.5f);
    float shootModePos_y = 0.0f;
    if (!Sys_Global.gamePaused && !Sys_Cheats.noHUD) RenderUIImage(shootModePos_x, shootModePos_y, shootModeWidth, shootModeHeight, 1020); // Shoot mode button
    if (Sys_Global.inventoryMode) {
        if (CursorIsOverBounds(shootModePos_x, shootModePos_x + shootModeWidth, shootModePos_y + shootModeHeight, shootModePos_y)) {
            if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].released) {
                Sys_Global.inventoryMode = false;
                cursorPosition_x = Sys_Settings.ScreenWidth / 2;
                cursorPosition_y = Sys_Settings.ScreenHeight / 2;
            }
        }
    }
    
    // Cursor [ /// VERY LAST DRAWN OVER EVERYTHING ELSE! /// ]
    bool menuOrInventoryCursorStyle = (Sys_Global.gamePaused || Sys_Global.menuActive);
    uint16_t cursorTexture = menuOrInventoryCursorStyle ? 1261 : 1260;
    float cursorSize = (float)Sys_Settings.ScreenWidth * CURSOR_SCREEN_PERCENTAGE * (menuOrInventoryCursorStyle ? 3.0f : 1.0f);
    float cursorHalfSize = cursorSize * 0.5f;
    if (CursorVisible()) RenderUIImage(cursorPosition_x - cursorHalfSize, cursorPosition_y - cursorHalfSize, cursorSize, cursorSize, cursorTexture);
    else RenderUIImage(screenCenterX - cursorHalfSize, screenCenterY - cursorHalfSize, cursorSize, cursorSize, cursorTexture);
    
    return time_now;
}

DepthSort visibleInstances[INSTANCE_COUNT];

static inline void RenderInstancesBetween(uint16_t instancesStartIdx, uint16_t instancesEndIdx, Vector3 playerPos, bool transparents) {
    uint16_t visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0;
    for (uint16_t i = instancesStartIdx; i < instancesEndIdx; ++i) {
        Vector3 objPos = instances[i].position;
        uint16_t instCellIdx = PosGetCellCoords(objPos.x, objPos.z);
        Vector3 delta = Vector3_A_minus_B(objPos, playerPos);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        if (distSqrd >= FAR_PLANE_SQUARED) continue;

        if (EntityIndexIsPortalBlockingDoor(instances[i].index) && !transparents) { // Extra checks only needed for opaque portal blocking doors.
            bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
            if (!inPVS) {
                uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((objPos.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((objPos.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                int r = vfloor(5.12f * (1.0f / WORLDCELL_WIDTH_F));
                for (int ix = cellX - r; ix <= (int)cellX + r && !inPVS; ++ix) {
                    for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                        if (!XZPairInBounds(ix, iz)) continue;
                        int subIdx = iz * WORLDX + ix;
                        if (get_cull_bit(precomputedVisibleCellsFromHere, instCellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) {
                            inPVS = true;
                            break;
                        }
                    }
                }
            }
            if (!inPVS) continue;
        } else {
            if (!(Sys_Global.currentLevel == 1 && (instances[i].index == 309 ||  instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
                if (CellNotVisible(instCellIdx)) continue;
            }
            
            if (!(gridCellStates[instCellIdx] & CELL_OPEN) && distSqrd >= 943.7184f) continue; // 30.72 * 30.72, 12 cells
        }
        
        float dotResult = dot_vector3(delta, instances[PLAYER1].forward);
        float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f;
        if (dotResult < 0.0f && distSqrd > (radius * radius)) continue;
        
        visibleInstances[visibleCount].index = i;
        visibleInstances[visibleCount].depth = distSqrd;
        visibleCount++;
    }
    
    if (visibleCount > 1) qsort(visibleInstances, visibleCount, sizeof(DepthSort), transparents ? compareDepthSort : compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
    for (uint16_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        uint16_t i = visibleInstances[visibleIndex].index;
        glUniform1ui(0, i);
        if (currentNormIndex != (uint32_t)instances[i].normIndex || instances[i].normIndex == 0) { currentNormIndex = (uint32_t)instances[i].normIndex; glUniform1ui(1, currentNormIndex); }
        if (currentTexIndex  != (uint32_t)instances[i].texIndex)  { currentTexIndex  =  (uint32_t)instances[i].texIndex; glUniform1ui(18, currentTexIndex); }
        if (currentGlowIndex != (uint32_t)instances[i].glowIndex || instances[i].glowIndex == 0) { currentGlowIndex = (uint32_t)instances[i].glowIndex; glUniform1ui(19, currentGlowIndex); }
        if (currentSpecIndex != (uint32_t)instances[i].specIndex || instances[i].specIndex == 0) { currentSpecIndex = (uint32_t)instances[i].specIndex; glUniform1ui(20, currentSpecIndex); }
        int32_t modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && instances[i].lodIndex < loadedModelsMaxIndex ? instances[i].lodIndex : instances[i].modelIndex;
        if (currentModelType != modelType) {
            currentModelType = modelType;
            glBindVertexBuffer(0, Sys_Render.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[currentModelType]);
        }
        
        uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
        Sys_Dx.drawCallsRenderedThisFrame++;
        Sys_Dx.verticesRenderedThisFrame += vertCount;
    }
}

static inline void RenderInstances(float* viewProj, Vector3 playerPos) { // 4. Raterized Geometry, Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST); glDisable(GL_BLEND); // Opaques
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniformMatrix4fv(2, 1, GL_FALSE, viewProj);
    glUniform1ui(3, 0u); /* isUI false */    glUniform1ui(17, 0u); // unlit false
    glUniform1f(8, worldMin_x);   glUniform1f(9, worldMin_z);    glUniform3f(10, playerPos.x, playerPos.y, playerPos.z);
    RenderInstancesBetween(START_INDEX_LEVEL_INSTANCES, startOfDoubleSidedInstances, playerPos, false);
    
    glDisable(GL_CULL_FACE); glEnable(GL_BLEND); // Doublesided
    RenderInstancesBetween(startOfDoubleSidedInstances, startOfTransparentInstances, playerPos, false);
    
    glEnable(GL_CULL_FACE); glEnable(GL_BLEND); // Transparents (with sort)
    RenderInstancesBetween(startOfTransparentInstances, loadedInstances - invalidModelIndexCount, playerPos, true);
    
    if (Sys_Dx.debugLineVertCount > 0) DrawDebugLines(viewProj);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static inline void RenderSSR(float* viewProj, Vector3 playerPos) {    
    glUseProgram(Sys_Render.ssrShaderProgram);
    glUniformMatrix4fv(4, 1, GL_FALSE, viewProj);
    glUniform3f(3, playerPos.x, playerPos.y, playerPos.z);
    GLuint groupX_ssr = ((Sys_Settings.ScreenWidth  / Sys_Settings.SSR_RES) + 31) / 32;
    GLuint groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
    glDispatchCompute(groupX_ssr, groupY_ssr, 1);
}

void Render(void) {
    Sys_Dx.drawCallsRenderedThisFrame = Sys_Dx.textDrawCallsRenderedThisFrame = Sys_Dx.uiImageDrawCallsRenderedThisFrame = Sys_Dx.shadowDrawCallsRenderedThisFrame = Sys_Dx.verticesRenderedThisFrame = 0; // Reset per frame
    
    // Frame prep, View Matrix, and Projection Matrix
    float view[16]; // Also known as view matrix
    Vector3 playerPos = instances[PLAYER1].position;
    {// mat4_lookat_from(view,&instances[PLAYER1].rotation, playerPos); Manually inlined for performance
        float x = instances[PLAYER1].rotation.x, y = instances[PLAYER1].rotation.y, z = instances[PLAYER1].rotation.z, w = instances[PLAYER1].rotation.w;
        float x2 = x * x, y2 = y * y, z2 = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;
        Vector3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
        Vector3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
        Vector3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
        view[0]  = right.x; view[1]  = up.x; view[2]  = -forward.x; view[3]  = 0.0f;
        view[4]  = right.y; view[5]  = up.y; view[6]  = -forward.y; view[7]  = 0.0f;
        view[8]  = right.z; view[9]  = up.z; view[10] = -forward.z; view[11] = 0.0f;
        view[12] = -dot_vector3(right, playerPos); view[13] = -dot_vector3(up, playerPos); view[14] = dot_vector3(forward, playerPos); view[15] = 1.0f;
    }
    
    float viewProj[16]; // view-projection matrix
    mul_mat4(viewProj, rasterPerspectiveProjection, view);
    float invViewRot[9] = { view[0], view[4], view[8],    view[1], view[5], view[9],    view[2], view[6], view[10] };
    ExtractFrustumPlanes(viewProj, playerFrustumPlanes);
    if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
        glBindVertexArray(Sys_Render.vao_chunk); // Common vao for RenderShadowmaps and Rasterized Geometry
        if (Sys_Settings.Shadows > 0u) RenderShadowmaps();
        memset(    lightDirty,0    ,LIGHT_COUNT * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
        memset(dirtyInstances,0,loadedInstances * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
        RenderInstances(viewProj, playerPos);
        if (Sys_Settings.Reflections > 0u) RenderSSR(viewProj, playerPos); // Screen Space Reflections
    }

    RenderCompositePass(playerPos.x, playerPos.y, playerPos.z, viewProj, invViewRot);
    Sys_Global.last_time = RenderUI();
    if ((Sys_Global.last_time - Sys_Dx.lastFrameSecCountTime) >= 1.00) { // Update Diagnostic Poll
        Sys_Dx.lastFrameSecCountTime = Sys_Global.last_time;
        Sys_Dx.framesPerLastSecond = Sys_Dx.globalFrameNum - Sys_Dx.lastFrameSecCount;
        if (Sys_Dx.framesPerLastSecond < Sys_Dx.worstFPS && Sys_Dx.globalFrameNum > 2000) Sys_Dx.worstFPS = Sys_Dx.framesPerLastSecond; // After startup, keep track of worst framerate seen.
        Sys_Dx.lastFrameSecCount = Sys_Dx.globalFrameNum;
    }
    
    Sys_Dx.cpuTime = get_time() - Sys_Global.current_time; // Measure time over everything this frame before GPU swap buffers
    glfwSwapBuffers(Sys_Global.window); // Present frame
    CHECK_GL_ERROR();
}

static void UpdateVoxelsAndInstances(void) {
    Sys_Render.shadowmapsNeedUpdated = UpdatedPlayerCell();
    Sys_Render.shadowmapsNeedUpdated = UpdateLights(&Sys_Render.shadowmapsNeedUpdated);
    if (Sys_Render.shadowmapsNeedUpdated) CullCore(); // 1. Culling
    bool uploadInstances = false;
    for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { if (dirtyInstances[i]) { uploadInstances = true; Sys_Render.shadowmapsNeedUpdated = true; UpdateInstanceMatrix(i); } }
    if (uploadInstances) glNamedBufferData(Sys_Render.matricesBufferID, loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
}

int32_t main(int32_t argc, char* argv[]) {
    double game_start_time = get_time();
    random_range_rng = (uint32_t)game_start_time; // Seed global rand uniquely with time since system boot.
    OpenConsoleLogFile();
    DebugRAM("program start");
    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) { DualLog("-----------------------------------------------------------\n%s\nby W. Josiah Jack\nMIT-0 licensed\n", EngineName); return 0; }
    if ((argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))) {
        DualLog("%s\n-------------------------------------------------------------\n", EngineName);
        DualLog("   This is a game engine designed for optimized focused usage\n");
        DualLog("   of OpenGL, making heavy use of GPU Driven rendering\n");
        DualLog("   techniques, a unified event system for debugging and log\n");
        DualLog("   playback, full mod support loading all data from external\n");
        DualLog("   files and using definition files for what to do with the\n");
        DualLog("   data.\n\n");
        DualLog("   This project aims to have minimal overhead, profiling,\n");
        DualLog("   traceability, robustness, and low level control.\n\n\n");
        DualLog("Valid arguments:\n");
        DualLog(" < none >\n    Runs the engine as normal, loading data from \n    neighbor directories (./Textures, ./Models, etc.)\n\n");
        DualLog("-v, --version\n    Prints version information\n\n");
        DualLog("play <file>\n    Plays back recorded log from current directory\n\n");
        DualLog("record <file>\n    Records all engine events to designated log\n    as a .dem file\n\n");
        DualLog("dump <file.dem>\n    Dumps the specified log into ./log_dump.txt\n    as human readable text.  You must provide full\n    file name with extension\n\n");
        DualLog("-h, --help\n    Provides this help text.  Neat!\n-----------------------------------------------------------\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "dump") == 0) { DualLog("Converting log to plaintext: %s ...", argv[2]); JournalDump(argv[2]); DualLog("DONE!\n"); return 0; }

    InitializeEnvironment(argc,argv[1],argv[2]);
    DebugRAM("prior to game loop");
    DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    Sys_Global.absoluteTime = Sys_Global.pauseRelativeTime = get_time();
    while(1) { // Main Loop
        if (glfwWindowShouldClose(Sys_Global.window)) OS_Exit(0);
        
        // Update Time
        Sys_Global.current_time = get_time();
        double frame_time = Sys_Global.current_time - Sys_Global.last_topframe_time;
        Sys_Global.absoluteTime += frame_time;
        Sys_Global.last_topframe_time = Sys_Global.current_time;
        if (!Sys_Global.gamePaused) Sys_Global.pauseRelativeTime += frame_time;
    
        glfwPollEvents();
        ProcessInput(); // Calls ApplyPlayerMovements()
        if (!Sys_Global.gamePaused && !Sys_Global.menuActive) UpdatePlayerFacingAngles();
        InputClearRisingAndFallingEdges();
        // Update Events, calls Physics()
        Sys_Global.timeSinceLastPhysicsTick = Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time;
        if (!log_playback && !Sys_Global.gamePaused && !Sys_Global.menuActive && Sys_Global.timeSinceLastPhysicsTick > (1.0 / 144.0)) {
            Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime;
            EnqueueEvent(EV_PHYSICS_TICK,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
        }

        if (log_playback) { // Enqueue all logged events for the current frame.
            int32_t read_status = ReadActiveLog();
            if (read_status == 2) { // EOF reached, no more events
                DualLog("Log playback completed.  Control returned.\n");
            } else if (read_status == -1) { DualLogError("Error reading log file, exiting playback\n"); OS_Exit(1); }
        }

        if (EventQueueProcess()) OS_Exit(1); // Do everything
    
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad); queuedLevelToLoad = 255u; continue; }
        
        if (!Sys_Global.gamePaused && !Sys_Global.menuActive) UpdateGameplay();
        if (!Sys_Global.gamePaused && !Sys_Global.menuActive) UpdateVoxelsAndInstances();
        Render();
        Sys_Dx.globalFrameNum++;
        #ifdef DEBUG_RAM_OUTPUT
            if (Sys_Dx.globalFrameNum == 4) { DebugRAM("after 4 frames of running"); }
            else if (Sys_Dx.globalFrameNum == 100) { DebugRAM("after 100 frames of running"); }
            else if (Sys_Dx.globalFrameNum == 200) DebugRAM("after 200 frames of running");
            else if (Sys_Dx.globalFrameNum == 500) DebugRAM("after 500 frames of running");
            else if (Sys_Dx.globalFrameNum == 1000) DebugRAM("after 1000 frames of running");
        #endif
    } return 0;
}
