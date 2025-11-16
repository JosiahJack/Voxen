#ifndef VOXEN_ENTITY_H
#define VOXEN_ENTITY_H
#include <float.h>
#include "voxen.h"

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
} Entity;

// Entities
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
extern Entity* entities; // Global array of entity definitions
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

void DualLogEntity(uint16_t idx);
void LoadEntities(void);
void LoadLevel(uint8_t curlevel);
void SortInstances(void);
#endif // VOXEN_ENTITY_H
