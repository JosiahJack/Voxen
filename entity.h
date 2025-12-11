#pragma once
#include "voxen.h"

#define INSTANCE_COUNT 10000 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define MAX_CHILD_COUNT 8
#define NULLENT 0u
#define PLAYER1 1u
#define PLAYER2 2u
#define START_INDEX_LEVEL_INSTANCES 3
#define ENTFLAG_ACTIVE        1u
#define ENTFLAG_CARDCHUNK     2u
#define ENTFLAG_GROUNDED      4u
#define ENTFLAG_USEGRAVITY    8u
#define ENTFLAG_KINEMATIC    16u
#define ENTFLAG_RIGIDBODY    32u
#define ENTFLAG_DOUBLESIDED  64u
#define ENTFLAG_TRANSPARENT 128u

typedef struct {
    uint16_t index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    uint32_t entflags;
    uint16_t modelIndex;
    uint8_t animated;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    Vector3 velocity;
    Vector3 angularVelocity;
    BodyState bodyState;
    ColliderType collider;
    Vector3 colliderCenter; // Offset relative to .position's global worldspace xyz location
    Vector3 colliderSize; // x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
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
    uint16_t   child[MAX_CHILD_COUNT];
    Vector3    child_offset[MAX_CHILD_COUNT];
    Quaternion child_rotation[MAX_CHILD_COUNT];
    Vector3    child_scale[MAX_CHILD_COUNT];
    char path[MAX_PATH];
} Entity;

typedef struct {
    Entity* entries;
    uint32_t count;
    uint32_t capacity;
} DataParser;

void ParseGameData(void);
bool parse_data_file(DataParser *parser, const char *filename);
extern Entity entities[MAX_ENTITIES]; // Global array of entity definitions
extern Entity instances[INSTANCE_COUNT];
extern uint16_t loadedInstances;
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
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
void DualLogEntityInstance(uint16_t idx);
void DualLogEntity(Entity ent);
void LoadEntities(void);
void LoadLevel(uint8_t curlevel);
