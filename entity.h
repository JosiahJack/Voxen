#ifndef VOXEN_ENTITY_H
#define VOXEN_ENTITY_H
#include <float.h>
#include "voxen.h"

#define INSTANCE_COUNT 10000 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
#define NULLENT 0
#define PLAYER1 1
#define PLAYER2 2
#define START_INDEX_LEVEL_INSTANCES 3
#define ENTFLAG_ACTIVE        1
#define ENTFLAG_CARDCHUNK     2
#define ENTFLAG_GROUNDED      4
#define ENTFLAG_USEGRAVITY    8
#define ENTFLAG_KINEMATIC    16
#define ENTFLAG_RIGIDBODY    32
#define ENTFLAG_DOUBLESIDED  64
#define ENTFLAG_TRANSPARENT 128

typedef struct {
    uint16_t index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    uint32_t entflags;
    
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    
    Vector3 position; // global worldspace xyz location
    Quaternion rotation; // Orientation matching Unity convention
    Vector3 scale;
    
    Vector3 velocity;
    Vector3 angularVelocity;
    
    BodyState bodyState;
    
    ColliderType collider;
    Vector3 colliderCenter; // Offset relative to .position's global worldspace xyz location
    Vector3 colliderSize; // x,y,z for Box,
                          // x for Sphere radius,
                          // x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
    uint16_t colliderMeshIndex;
    float mass;
    float linearDrag;
    float angularDrag;
    float inertia;
    Vector3 accumulatedForce;
    Vector3 accumulatedTorque;
    float dynamicFriction;
    float staticFriction;
    float bounciness;
    PhysCombineType frictionCombine;
    PhysCombineType bounceCombine;
    
    float volume;
    
    uint16_t   child0;
    Vector3    child0_offset;
    Quaternion child0_rotation;
    Vector3    child0_scale;
    
    uint16_t   child1;
    Vector3    child1_offset;
    Quaternion child1_rotation;
    Vector3    child1_scale;
    
    char path[MAX_PATH];
} Entity;
// ----------------------------------------------------------------------------
// Data Parsing
typedef struct {
    Entity* entries;
    uint32_t count;
    uint32_t capacity;
} DataParser;

void ParseGameData();
bool parse_data_file(DataParser *parser, const char *filename);
// ----------------------------------------------------------------------------
// Entities
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
extern Entity* entities; // Global array of entity definitions
extern Entity instances[INSTANCE_COUNT];
extern uint16_t loadedInstances;
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
extern uint32_t cellIndexForInstance[INSTANCE_COUNT];
extern uint16_t cellIndexForLight[LIGHT_COUNT];
extern uint16_t cellIndexForLightX[LIGHT_COUNT];
extern uint16_t cellIndexForLightZ[LIGHT_COUNT];
extern int32_t entityCount;            // Number of entities loaded
extern uint16_t invalidModelIndexCount;
extern uint16_t* modelTypeCountsOpaque;
extern uint16_t* modelTypeCountsDoubleSided;
extern uint16_t* modelTypeCountsTransparent;
extern uint16_t* modelTypeOffsetsOpaque;
extern uint16_t* modelTypeOffsetsDoubleSided;
extern uint16_t* modelTypeOffsetsTransparent;
extern uint16_t opaqueInstancesHead;
extern uint16_t renderableCount;
extern uint16_t loadedInstances;
extern uint16_t startOfDoubleSidedInstances;
extern uint16_t startOfTransparentInstances;
extern uint16_t doubleSidedInstancesHead;
extern uint16_t transparentInstancesHead;

void InitializeEntity(Entity* entry);
void DualLogEntity(uint16_t idx);
void LoadEntities(void);
void LoadLevel(uint8_t curlevel);
#endif // VOXEN_ENTITY_H
