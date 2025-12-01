#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "entity.h"
#include "voxen.h"
#include "vmath.h"

#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
Entity* entities = NULL; // Global array of entity definitions
int32_t entityCount = 0;            // Number of entities loaded
DataParser entity_parser;
uint16_t invalidModelIndexCount;
uint16_t* modelTypeCountsOpaque = NULL;
uint16_t* modelTypeCountsDoubleSided = NULL;
uint16_t* modelTypeCountsTransparent = NULL;
uint16_t* modelTypeOffsetsOpaque = NULL;
uint16_t* modelTypeOffsetsDoubleSided = NULL;
uint16_t* modelTypeOffsetsTransparent = NULL;
uint16_t opaqueInstancesHead = 0;
uint16_t renderableCount = 0;
uint16_t loadedInstances = 0;
uint16_t startOfDoubleSidedInstances = INSTANCE_COUNT - 1;
uint16_t startOfTransparentInstances = INSTANCE_COUNT - 1;
uint16_t doubleSidedInstancesHead = 0;
uint16_t transparentInstancesHead = 0;
float correctionX, correctionY, correctionZ;
float correctionNPCX, correctionNPCY, correctionNPCZ;
float correctionDoorsX, correctionDoorsY, correctionDoorsZ;
float correctionDynamicsX, correctionDynamicsY, correctionDynamicsZ;
float correctionLightsSaveableX, correctionLightsSaveableY, correctionLightsSaveableZ;
float correctionStaticImmutableX, correctionStaticImmutableY, correctionStaticImmutableZ;
float correctionStaticSaveableX, correctionStaticSaveableY, correctionStaticSaveableZ;
float correctionLightX, correctionLightY, correctionLightZ;
bool lightIsDynamic[LIGHT_COUNT];
uint16_t loadedLights = 0;

void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; } // TODO: Resave levels with the offsets applied.
    
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.MedicalLevel
        case 0:  *ofsx = 3.6f; *ofsy = -4.10195f; *ofsz = 1.0f; break;
        case 1:  *ofsx = 25.56f; *ofsy = -48.64f; *ofsz = -5.2f; break;
        case 2:  *ofsx = -2.6f; *ofsy = 0.0f; *ofsz = -7.7f; break;
        case 3:  *ofsx = -45.12f; *ofsy = -0.700374f; *ofsz = -16.32f; break;
        case 4:  *ofsx = -20.4f; *ofsy = 0.0f; *ofsz = 11.48f; break;
        case 5:  *ofsx = -10.14f; *ofsy = 0.065f; *ofsz = -0.0383f; break;
        case 6:  *ofsx = -0.6728f; *ofsy = 0.1725f; *ofsz = 3.76f; break;
        case 7: *ofsx = -6.7f; *ofsy = 0.24443f; *ofsz = 1.16f; break;
        case 8:  *ofsx = 1.08f; *ofsy = -0.935f; *ofsz = 0.8f; break;
        case 9:  *ofsx = 3.6f; *ofsy = 0.0f; *ofsz = -1.28f; break;
        case 10: *ofsx = 107.37f; *ofsy = 101.2f; *ofsz = 35.48f; break;
        case 11: *ofsx = 15.05f; *ofsy = 129.9f; *ofsz = -77.94f; break;
        case 12:  *ofsx = 19.04f; *ofsy = 162.2f; *ofsz = 95.8f; break;
        case LEVEL_CYBERSPACE: *ofsx = 164.7f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_Dynamic_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DynamicObjectsSaveableInstantiated
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.84f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.07f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.04f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.16f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.08f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.32f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.2f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = -14.528f; *ofsy = 48.269f; *ofsz = -26.836f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_DoorsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DoorsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_NPCsSaveableInstantiated_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.NPCsSaveableInstantiated
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -33.28f; *ofsy = 48.64f; *ofsz = 7.679996f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
void DualLogEntity(Entity ent) {
    DualLog("Entity::\n"
            "    index: %u\n"
            "    entflags: %u [\n      ACTIVE:     %u\n      CARDCHUNK:  %u\n      GROUNDED:   %u\n      USEGRAVITY: %u\n      KINEMATIC:  %u\n      RIGIDBODY:  %u\n            ]\n"
            "    modelIndex: %u\n"
            "    texIndex:   %u\n"
            "    glowIndex:  %u\n"
            "    specIndex:  %u\n"
            "    normIndex:  %u\n"
            "    lodIndex:  %u\n"
            "    position.x: %f, .y: %f, .z: %f\n"
            "    rotation.x: %f, .y: %f, .z: %f, .w: %f\n"
            "    scale.x: %f, .y: %f, .z: %f\n"
            "    velocity.x: %f, .y: %f, .z: %f\n"
            "    angularVelocity.x: %f, .y: %f, .z: %f\n"
            "    bodyState: %u\n"
            "    collider: %u\n"
            "    colliderCenter.x: %f, .y: %f, .z: %f\n"
            "    colliderSize.x: %f, .y: %f, .z: %f\n"
            "    colliderMeshIndex: %u,\n"
            "    mass: %f\n"
            "    linearDrag: %f\n"
            "    angularDrag: %f\n"
            "    inertia: %f\n"
            "    accumulatedForce.x: %f, .y: %f, .z: %f\n"
            "    accumulatedTorque.x: %f, .y: %f, .z: %f\n"
            "    bounciness: %f\n"
            "    dynamicFriction: %f\n"
            "    staticFriction: %f\n"
            "    frictionCombine: %u\n"
            "    bounceCombine: %u\n"
            "    volume: %f\n"
            "    child0: %u\n"
            "    child0_offset.x: %f, .y: %f, .z: %f\n"
            "    child0_rotation.x: %f, .y: %f, .z: %f, .w: %f\n"
            "    child0_scale.x: %f, .y: %f, .z: %f\n"
            "    child1: %u\n"
            "    child1_offset.x: %f, .y: %f, .z: %f\n"
            "    child1_rotation.x: %f, .y: %f, .z: %f, .w: %f\n"
            "    child1_scale.x: %f, .y: %f, .z: %f\n"
            ,
            ent.index,
            ent.entflags,
                (ent.entflags & ENTFLAG_ACTIVE) > 0,
                (ent.entflags & ENTFLAG_CARDCHUNK) > 0,
                (ent.entflags & ENTFLAG_GROUNDED) > 0,
                (ent.entflags & ENTFLAG_USEGRAVITY) > 0,
                (ent.entflags & ENTFLAG_KINEMATIC) > 0,
                (ent.entflags & ENTFLAG_RIGIDBODY) > 0,
            ent.modelIndex,
            ent.texIndex,
            ent.glowIndex,
            ent.specIndex,
            ent.normIndex,
            ent.lodIndex,
            ent.position.x, ent.position.y, ent.position.z,
            ent.rotation.x, ent.rotation.y, ent.rotation.z, ent.rotation.w,
            ent.scale.x, ent.scale.y, ent.scale.z,
            ent.velocity.x, ent.velocity.y, ent.velocity.z,
            ent.angularVelocity.x, ent.angularVelocity.y, ent.angularVelocity.z,
            ent.bodyState,
            ent.collider,
            ent.colliderCenter.x, ent.colliderCenter.y, ent.colliderCenter.z,
            ent.colliderSize.x, ent.colliderSize.y, ent.colliderSize.z,
            ent.colliderMeshIndex,
            ent.mass,
            ent.linearDrag,
            ent.angularDrag,
            ent.inertia,
            ent.accumulatedForce.x, ent.accumulatedForce.y, ent.accumulatedForce.z,
            ent.accumulatedTorque.x, ent.accumulatedTorque.y, ent.accumulatedTorque.z,
            ent.dynamicFriction,
            ent.staticFriction,
            ent.bounciness,
            ent.frictionCombine,
            ent.bounceCombine,
            ent.volume,
            ent.child0,
            ent.child0_offset.x, ent.child0_offset.y, ent.child0_offset.z,
            ent.child0_rotation.x, ent.child0_rotation.y, ent.child0_rotation.z, ent.child0_rotation.w,
            ent.child0_scale.x, ent.child0_scale.y, ent.child0_scale.z,
            ent.child1,
            ent.child1_offset.x, ent.child1_offset.y, ent.child1_offset.z,
            ent.child1_rotation.x, ent.child1_rotation.y, ent.child1_rotation.z, ent.child1_rotation.w,
            ent.child1_scale.x, ent.child1_scale.y, ent.child1_scale.z);
}

void DualLogEntityInstance(uint16_t idx) {
    DualLog("Logging instance[%u] ",idx);
    DualLogEntity(instances[idx]);
}
#pragma GCC diagnostic pop

void InitializeEntity(Entity* entry) {
    entry->index = UINT16_MAX; // memset here would be harmful as only a handful of fields are the same.
    entry->entflags = ENTFLAG_KINEMATIC; // Zeroes the rest out.
    entry->modelIndex = MODEL_IDX_MAX;
    entry->texIndex = entry->glowIndex = entry->specIndex = entry->normIndex = MATERIAL_IDX_MAX;
    entry->lodIndex  = MODEL_IDX_MAX;
    entry->rotation.x = entry->rotation.y = entry->rotation.z = 0.0f; entry->rotation.w = 1.0f; // Quaternion identity
    entry->scale.x = entry->scale.y = entry->scale.z = 1.0f;
    entry->collider = COLLIDER_TYPE_NONE;
    entry->colliderMeshIndex = MODEL_IDX_MAX;
    entry->mass = 1.0f;
    entry->angularDrag = 0.05f;
    entry->dynamicFriction = entry->staticFriction = 0.6f;
    entry->frictionCombine = entry->bounceCombine = PHYS_COMBINE_AVG;
    entry->volume = 1.0f;
    entry->child0 = UINT16_MAX;
    entry->child0_rotation.x = entry->child0_rotation.y = entry->child0_rotation.z = 0.0f; entry->child0_rotation.w = 1.0f;
    entry->child0_scale.x = entry->child0_scale.y = entry->child0_scale.z = 1.0f;
    entry->child1 = UINT16_MAX;
    entry->child1_rotation.x = entry->child1_rotation.y = entry->child1_rotation.z = 0.0f; entry->child1_rotation.w = 1.0f;
    entry->child1_scale.x = entry->child1_scale.y = entry->child1_scale.z = 1.0f;
    entry->path[0] = '\0';    
}

void LoadEntities(void) {
    double start_time = get_time();
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); exit(1); }
    
    entityCount = entity_parser.count;
    entities = calloc(entityCount,sizeof(Entity));
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); exit(1); }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); exit(1); }

    DualLog("Loading  %d entities...", entityCount);
    for (int32_t i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        entities[i] = entity_parser.entries[i];
        flag_enable(&entities[i].entflags, ENTFLAG_ACTIVE);
        flag_set(&entities[i].entflags,    ENTFLAG_GROUNDED, false);
        flag_set(&entities[i].entflags,    ENTFLAG_RIGIDBODY, ConstIndexIsDynamicObject(entities[i].index));
        if (entity_parser.entries[i].entflags & ENTFLAG_CARDCHUNK) {
            entities[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX; // Generic LOD card
            entities[i].collider = COLLIDER_TYPE_BOX;
            entities[i].colliderCenter.x = 0.0f;
            entities[i].colliderCenter.y = 1.44f;
            entities[i].colliderCenter.z = 0.0f;
            entities[i].colliderSize.x = 2.56f;
            entities[i].colliderSize.y = 0.32f;
            entities[i].colliderSize.z = 2.56f;
        }

        entities[i].inertia = 0.0f;
        entities[i].accumulatedForce.x = 0.0f;
        entities[i].accumulatedForce.y = 0.0f;
        entities[i].accumulatedForce.z = 0.0f;
        entities[i].accumulatedTorque.x = 0.0f;
        entities[i].accumulatedTorque.y = 0.0f;
        entities[i].accumulatedTorque.z = 0.0f;
//         DualLogEntity(entities[i]);
    }

    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

void CopyInstanceRegion(uint16_t head, uint16_t* instanceTypeArray, Entity* tempInstances, uint16_t* targetIndex, uint16_t nextRegionStart) {
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        for (uint16_t j = 0; j < head; j++) {
            uint16_t i = instanceTypeArray[j];
            if (tempInstances[i].modelIndex == modelIdx) {
                if (*targetIndex >= nextRegionStart) { DualLogError("Instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, *targetIndex); exit(1); }
                
                instances[*targetIndex] = tempInstances[i];
                (*targetIndex) += 1;
            }
        }
    }
}

void SortInstances(void) { // Reorder instances such that each type is grouped opaque->doublesided->transparent in that order in instances[].
    double start_time = get_time();
    DualLog("Sorting entity instances... ");
    if (modelTypeCountsOpaque      ) { free(modelTypeCountsOpaque      ); }   modelTypeCountsOpaque = calloc(loadedModels,sizeof(uint16_t)); // Zero out all arrays and counters
    if (modelTypeCountsDoubleSided ) { free(modelTypeCountsDoubleSided ); }   modelTypeCountsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeCountsTransparent ) { free(modelTypeCountsTransparent ); }   modelTypeCountsTransparent = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeOffsetsOpaque     ) { free(modelTypeOffsetsOpaque     ); }   modelTypeOffsetsOpaque = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeOffsetsDoubleSided) { free(modelTypeOffsetsDoubleSided); }   modelTypeOffsetsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
    if (modelTypeOffsetsTransparent) { free(modelTypeOffsetsTransparent); }   modelTypeOffsetsTransparent = calloc(loadedModels,sizeof(uint16_t));
    uint16_t opaqueInstances[INSTANCE_COUNT] = {0};
    uint16_t doubleSidedInstances[INSTANCE_COUNT] = {0};
    uint16_t transparentInstances[INSTANCE_COUNT] = {0};
    opaqueInstancesHead = doubleSidedInstancesHead = transparentInstancesHead = invalidModelIndexCount = 0;
    for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { // Skip player instances and NULLENT by starting at 3.
        if (instances[i].texIndex >= loadedTextures && instances[i].texIndex != MATERIAL_IDX_MAX) { DualLogError("Invalid texIndex %u for instance %u\n", instances[i].texIndex, i); invalidModelIndexCount++; continue; }
        if (instances[i].modelIndex >= loadedModels || instances[i].modelIndex == UINT16_MAX) { invalidModelIndexCount++; continue; }
        if (instances[i].index >= MAX_ENTITIES) { DualLogError("Invalid entity index %u for instance %u\n", instances[i].index, i); invalidModelIndexCount++; continue; }

        bool is_double_sided = isDoubleSided(instances[i].texIndex) || instances[i].scale.x < 0.0f || instances[i].scale.y < 0.0f || instances[i].scale.z < 0.0f;
        if (isTransparent(instances[i].texIndex)) {
            if (transparentInstancesHead >= INSTANCE_COUNT) { DualLogError("Transparent instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            transparentInstances[transparentInstancesHead++] = i;
            modelTypeCountsTransparent[instances[i].modelIndex]++;
        } else if (is_double_sided) {
            if (doubleSidedInstancesHead >= INSTANCE_COUNT) { DualLogError("Double-sided instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            doubleSidedInstances[doubleSidedInstancesHead++] = i;
            modelTypeCountsDoubleSided[instances[i].modelIndex]++;
        } else {
            if (opaqueInstancesHead >= INSTANCE_COUNT) { DualLogError("Opaque instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            opaqueInstances[opaqueInstancesHead++] = i;
            modelTypeCountsOpaque[instances[i].modelIndex]++;
        }
    }

    // Compute offsets
    uint16_t currentOffset = START_INDEX_LEVEL_INSTANCES;
    uint16_t i = 0;
    for (; i < loadedModels; i++) { modelTypeOffsetsOpaque[i] = currentOffset; currentOffset += modelTypeCountsOpaque[i]; }
    startOfDoubleSidedInstances = currentOffset;
    for (i = 0; i < loadedModels; i++) { modelTypeOffsetsDoubleSided[i] = currentOffset; currentOffset += modelTypeCountsDoubleSided[i]; }
    startOfTransparentInstances = currentOffset;
    for (i = 0; i < loadedModels; i++) { modelTypeOffsetsTransparent[i] = currentOffset; currentOffset += modelTypeCountsTransparent[i]; }
    if ((startOfTransparentInstances + transparentInstancesHead) > (loadedInstances - invalidModelIndexCount)) { DualLogError("Transparent range overflow: start %u, head %u, limit %u\n", startOfTransparentInstances, transparentInstancesHead, loadedInstances - invalidModelIndexCount); exit(1); }

    Entity tempInstances[INSTANCE_COUNT];
    memcpy(tempInstances, instances, loadedInstances * sizeof(Entity));
    uint16_t targetIdx = START_INDEX_LEVEL_INSTANCES;
    CopyInstanceRegion(opaqueInstancesHead,           opaqueInstances, tempInstances, &targetIdx, startOfDoubleSidedInstances); // Copy opaque instances
    CopyInstanceRegion(doubleSidedInstancesHead, doubleSidedInstances, tempInstances, &targetIdx, startOfTransparentInstances); // Copy doublesided instances
    CopyInstanceRegion(transparentInstancesHead, transparentInstances, tempInstances, &targetIdx,             loadedInstances); // Copy transparent instances
    for (i = 0; i < loadedInstances; ++i) { // Put all the invisible entities at the end of the list now
        if (tempInstances[i].modelIndex > loadedModels) { instances[targetIdx] = tempInstances[i]; targetIdx++; }
    }

    // Update cellIndexForInstance
    for (i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) { // Skip player index and start at 3?
        float x = instances[i].position.x;
        float z = instances[i].position.z;
        int32_t cellX = (int32_t)vfloor((x - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t cellZ = (int32_t)vfloor((z - worldMin_z) / WORLDCELL_WIDTH_F);
        cellX = clamp(cellX, 0, 63);
        cellZ = clamp(cellZ, 0, 63);
        cellIndexForInstance[i] = cellZ * 64 + cellX;
    }
    
    DualLog("opaque: %u, double-sided: %u, transparent: %u, invisible: %u...", opaqueInstancesHead, doubleSidedInstancesHead, transparentInstancesHead, invalidModelIndexCount);
    DualLog(" took %f secs\n", get_time() - start_time);
    loadedAmbients = 0;
    for (i = opaqueInstancesHead + doubleSidedInstancesHead + transparentInstancesHead; i<loadedInstances;++i) {
        uint16_t entIdx = instances[i].index;
        if (ConstIndexIsAmbient(entIdx)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); exit(1); }
            
            instances[i].volume = entities[entIdx].volume * 0.5f;
        }
    }
}

void AddInstance(uint16_t entIdx, uint16_t instanceIdx, uint32_t lineNum) {
    if (entIdx >= entityCount) { DualLogError("\nEntity index when loading level geometry object %d was %d, exceeds max defined entity count of %d\n",lineNum,entIdx,entityCount); exit(1); }
            
    instances[instanceIdx].index = entIdx;
    instances[instanceIdx].modelIndex = entities[entIdx].modelIndex;
    if (instances[instanceIdx].modelIndex < loadedModels) renderableCount++;
    instances[instanceIdx].texIndex = entities[entIdx].texIndex;
    instances[instanceIdx].glowIndex = entities[entIdx].glowIndex;
    if (instances[instanceIdx].glowIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].glowIndex = BLACK_TEXTURE_IDX;
    instances[instanceIdx].specIndex = entities[entIdx].specIndex;
    if (instances[instanceIdx].specIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].specIndex = BLACK_TEXTURE_IDX;
    instances[instanceIdx].normIndex = entities[entIdx].normIndex;
    if (instances[instanceIdx].normIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].normIndex = BLACK_TEXTURE_IDX;
    instances[instanceIdx].lodIndex = entities[entIdx].lodIndex;
//     instances[instanceIdx].entflags = entities[entIdx].entflags; // Decided this was dangerous/error-prone, commented out in lieu of these explicit sets to better preserve the loaded data:
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_CARDCHUNK,  entities[entIdx].entflags & ENTFLAG_CARDCHUNK);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_USEGRAVITY,  entities[entIdx].entflags & ENTFLAG_USEGRAVITY);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_KINEMATIC,  entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_RIGIDBODY,  entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    instances[instanceIdx].collider = entities[entIdx].collider;
    instances[instanceIdx].colliderCenter.x = entities[entIdx].colliderCenter.x;
    instances[instanceIdx].colliderCenter.y = entities[entIdx].colliderCenter.y;
    instances[instanceIdx].colliderCenter.z = entities[entIdx].colliderCenter.z;
    instances[instanceIdx].colliderSize.x = entities[entIdx].colliderSize.x;
    instances[instanceIdx].colliderSize.y = entities[entIdx].colliderSize.y;
    instances[instanceIdx].colliderSize.z = entities[entIdx].colliderSize.z;
    instances[instanceIdx].colliderMeshIndex = entities[entIdx].colliderMeshIndex;
    instances[instanceIdx].mass = entities[entIdx].mass > 0.0f ? entities[entIdx].mass : 1.0f; // Nonzero fallback.
    instances[instanceIdx].linearDrag = entities[entIdx].linearDrag > 0.0f ? entities[entIdx].linearDrag : 0.0f;
    instances[instanceIdx].angularDrag = entities[entIdx].angularDrag > 0.0f ? entities[entIdx].angularDrag : 0.05f;
    
    // Apply the Unity hierarchy nonsense, TODO: Save out level#.txt from the engine just once and then delete all this.
    if (levelCurrentlyLoading && entIdx != 755 && entIdx != 590) { // Adjusted for in the level data directly, no correction.
        instances[instanceIdx].position.x += correctionX;   
        instances[instanceIdx].position.y += correctionY;
        instances[instanceIdx].position.z += correctionZ;
        if (ConstIndexIsDoor(entIdx)) {
            instances[instanceIdx].position.x += correctionDoorsX;
            instances[instanceIdx].position.y += correctionDoorsY;
            instances[instanceIdx].position.z += correctionDoorsZ;
        } else if (ConstIndexIsNPC(entIdx)) {
            instances[instanceIdx].position.x += correctionNPCX;
            instances[instanceIdx].position.y += correctionNPCY - 1.0f; // Offset to center them up in their capsule
            instances[instanceIdx].position.z += correctionNPCZ;
            
            Vector3 axis = (Vector3){0.0f, 0.0f, 1.0f}; // X-axis
            instances[instanceIdx].rotation = axis_angle_quaternion(axis, deg2rad(-90.0f));
        } else if (ConstIndexIsLightStaticSaveable(entIdx)) {
            instances[instanceIdx].position.x += correctionLightX;
            instances[instanceIdx].position.y += correctionLightY;
            instances[instanceIdx].position.z += correctionLightZ;
        } else if (ConstIndexIsStaticObjectSaveable(entIdx)) {
            instances[instanceIdx].position.x += correctionStaticSaveableX;
            instances[instanceIdx].position.y += correctionStaticSaveableY;
            instances[instanceIdx].position.z += correctionStaticSaveableZ;
        } else if (ConstIndexIsStaticObjectImmutable(entIdx)) {
            instances[instanceIdx].position.x += correctionStaticImmutableX;
            instances[instanceIdx].position.y += correctionStaticImmutableY;
            instances[instanceIdx].position.z += correctionStaticImmutableZ;
        } else if (ConstIndexIsDynamicObject(entIdx)) { // MUST BE LAST AS IT OVERLAPS WITH NPC AND LIGHTS SAVEABLE!
            instances[instanceIdx].position.x += correctionDynamicsX;
            instances[instanceIdx].position.y += correctionDynamicsY;
            instances[instanceIdx].position.z += correctionDynamicsZ;
        } 
    }

    dirtyInstances[instanceIdx] = true;
    loadedInstances++;
}

void AddChild0(uint16_t child, uint16_t parent, uint16_t entIdx, int32_t* instanceIdx, uint32_t lineNum) {
    if (child == UINT16_MAX) return;
    
    (*instanceIdx)++; // Increment head of the list an extra time for the child entity
    AddInstance(child, *instanceIdx, lineNum);
    instances[*instanceIdx].index = child;
    instances[*instanceIdx].position.x = instances[parent].position.x + entities[entIdx].child0_offset.x;
    instances[*instanceIdx].position.y = instances[parent].position.y + entities[entIdx].child0_offset.y;
    instances[*instanceIdx].position.z = instances[parent].position.z + entities[entIdx].child0_offset.z;
    instances[*instanceIdx].scale.x = instances[parent].scale.x * entities[entIdx].child0_scale.x;
    instances[*instanceIdx].scale.y = instances[parent].scale.y * entities[entIdx].child0_scale.y;
    instances[*instanceIdx].scale.z = instances[parent].scale.z * entities[entIdx].child0_scale.z;
}

void AddChild1(uint16_t child, uint16_t parent, uint16_t entIdx, int32_t* instanceIdx, uint32_t lineNum) {
    if (child == UINT16_MAX) return;

    (*instanceIdx)++; // Increment head of the list an extra time for the child entity
    AddInstance(child, *instanceIdx, lineNum);
    instances[*instanceIdx].index = child;
    instances[*instanceIdx].position.x = instances[parent].position.x + entities[entIdx].child1_offset.x;
    instances[*instanceIdx].position.y = instances[parent].position.y + entities[entIdx].child1_offset.y;
    instances[*instanceIdx].position.z = instances[parent].position.z + entities[entIdx].child1_offset.z;
    instances[*instanceIdx].scale.x = instances[parent].scale.x * entities[entIdx].child1_scale.x;
    instances[*instanceIdx].scale.y = instances[parent].scale.y * entities[entIdx].child1_scale.y;
    instances[*instanceIdx].scale.z = instances[parent].scale.z * entities[entIdx].child1_scale.z;
}

void LoadLevel(uint8_t curlevel) {
    double start_time = get_time();
    if (!levelCurrentlyLoading) memset(instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    levelCurrentlyLoading = true;
    DebugRAM("start of LoadLevel");
    currentLevel = curlevel;
    renderableCount = 0;
    loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    loadedLights = 0;
    loadedAmbients = 0;
    if (curlevel >= numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); exit(1); }
    
    for (uint16_t idx = START_INDEX_LEVEL_INSTANCES;idx<INSTANCE_COUNT;idx++) { InitializeEntity(&instances[idx]); dirtyInstances[idx] = true; } // Start AFTER player indices and NULLENT
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    char filename[20]; // Minimum size for 0 through 13.
    snprintf(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); exit(1); }

    int32_t lineNum = -1; // Start at 0 on first loop iteration, as it needs to iterate before each blank or commented line skip
    int32_t instanceIdx = PLAYER2;
    int32_t lightsIdx = -1;
    size_t lineLengthMax = 81920; 
    char lineSpace[lineLengthMax];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[lineLengthMax];
    GetLevel_Transform_Offsets(curlevel, &correctionX, &correctionY, &correctionZ);
    GetLevel_Dynamic_ContainerOffsets(curlevel, &correctionDynamicsX, &correctionDynamicsY, &correctionDynamicsZ);
    GetLevel_LightsStaticSaveable_ContainerOffsets(curlevel, &correctionLightsSaveableX, &correctionLightsSaveableY, &correctionLightsSaveableZ);
    GetLevel_StaticObjectsSaveable_ContainerOffsets(curlevel, &correctionStaticSaveableX, &correctionStaticSaveableY, &correctionStaticSaveableZ);
    GetLevel_StaticObjectsImmutable_ContainerOffsets(curlevel, &correctionStaticImmutableX, &correctionStaticImmutableY, &correctionStaticImmutableZ);
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel, &correctionLightX, &correctionLightY, &correctionLightZ);
    GetLevel_DoorsStaticSaveable_ContainerOffsets(curlevel, &correctionDoorsX, &correctionDoorsY, &correctionDoorsZ);
    GetLevel_NPCsSaveableInstantiated_ContainerOffsets(curlevel, &correctionNPCX, &correctionNPCY, &correctionNPCZ);
    while (fgets(lineSpace, lineLengthMax, file)) {
        size_t len = strlen(lineSpace);
        while (len && (lineSpace[len - 1] == '\n' || lineSpace[len - 1] == '\r'))
        lineSpace[--len] = '\0';
        line = lineSpace;
        snprintf(initialLine, sizeof(initialLine), "%s", line);
        memcpy(firstKeyCheck,line,10); firstKeyCheck[10] = '\0';
        lineNum++;
        bool isLight = true;
        if (strcmp(firstKeyCheck, "constIndex") == 0) isLight = false;  // constIndex specified indicating this is a real entity?
        if (isLight) {
            lightsIdx++;
            lightIsDynamic[lightsIdx] = false;
            if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",lightsIdx,curlevel); exit(1); }
        } else {
            instanceIdx++;
            if (instanceIdx >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n",instanceIdx,curlevel); exit(1); }
        }
        
        int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
        uint8_t lightType = 0u; // Point
        while(line[0] != '\0') {
            // Guaranteed no leading whitespaces,k comments, or blank lines, so don't bother
            char* pipe = strchr(line,'|');
            char* kvString = line; // key:value pair before the pipe as a string
            if (pipe) {
                *pipe = '\0';          // Split string at the pipe
                line = pipe + 1;       // Point to rest of the line after the pipe
            } else { // Else this is the last string after the last pipe with last kv pair
                line += strlen(line);
            }
           
            if (kvString[0] == '\0' || strchr(kvString, ':') == NULL) continue;
            
            char *colon = strchr(kvString, ':');
            if (colon[1] == '\0') continue; // Don't care about the name of the Unity gameobject from when this data used to be over there.  Need to skip this in the middle, but this also handles the very end
            
            *colon = '\0';           // Split string at the colon
            char *key = kvString;    // Assign key to before colon
            char *value = colon + 1; // Assing value to after colon
            if (!key || !value) { DualLogError("Invalid key-value pair at line %u (as viewed by text editor): %s\n", lineNum+1, initialLine); exit(1); }

            char trimmed_key[64];
            char trimmed_value[256];
            snprintf(trimmed_key, sizeof(trimmed_key), "%s", key);
            snprintf(trimmed_value, sizeof(trimmed_value), "%s", value);
            trimmed_key[sizeof(trimmed_key) - 1] = '\0';
            trimmed_value[sizeof(trimmed_value) - 1] = '\0';
//             sanitize_utf8_ascii(trimmed_key);
//             sanitize_utf8_ascii(trimmed_value);
            if (isLight) {
                     if (strcmp(trimmed_key, "localPosition.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSX] = parse_float(trimmed_value, initialLine, lineNum) + correctionX + correctionLightX;
                else if (strcmp(trimmed_key, "localPosition.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSY] = parse_float(trimmed_value, initialLine, lineNum) + correctionY + correctionLightY;
                else if (strcmp(trimmed_key, "localPosition.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = parse_float(trimmed_value, initialLine, lineNum) + correctionZ + correctionLightZ;
                else if (strcmp(trimmed_key, "localRotation.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intensity") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = parse_float(trimmed_value, initialLine, lineNum) * 0.4; // Adjustment, globally applied from Citadel's Unity to Custom Game Engine (Voxen) conversion.
                else if (strcmp(trimmed_key, "range") == 0)           lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "spotAngle") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "type") == 0) {
                    if ((strcmp(trimmed_value, "Spot") == 0)) lightType = 1u;
                    else if ((strcmp(trimmed_value, "Directional") == 0)) lightType = 2u;
                }
                else if (strcmp(trimmed_key, "color.r") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_R] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "color.g") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_G] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "color.b") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_B] = parse_float(trimmed_value, initialLine, lineNum);
            } else {
                     if (strcmp(trimmed_key, "constIndex") == 0)      instances[instanceIdx].index = parse_numberu16(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.x") == 0) instances[instanceIdx].position.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.y") == 0) instances[instanceIdx].position.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.z") == 0) instances[instanceIdx].position.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.x") == 0) instances[instanceIdx].rotation.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) instances[instanceIdx].rotation.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) instances[instanceIdx].rotation.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) instances[instanceIdx].rotation.w = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.x") == 0)    instances[instanceIdx].scale.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.y") == 0)    instances[instanceIdx].scale.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.z") == 0)    instances[instanceIdx].scale.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "go.activeSelf") == 0)   flag_set(&instances[instanceIdx].entflags, ENTFLAG_ACTIVE, parse_bool(trimmed_value, initialLine, lineNum));
            }
        }
        
        if (isLight) {
            loadedLights++;
            lightsRangeSquared[lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_RANGE] * lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            // TODO: Set lightIsDynamic[lightsIdx] = true when light has animation data values set from file
            if (lightType == 1) {
                if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Spotlight %d on line %d loaded with spotAngle less than 5deg\n",lightsIdx,lineNum+1);
            } else if (lightType == 2) {
                // TODO: Handle directional lights for cyberspace
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            } else {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            }
        } else {
            uint16_t parent = instanceIdx; // Needed as adding children moves the instanceIdx.
            uint16_t entIdx = instances[parent].index;
            AddInstance(entIdx, parent, lineNum);
            AddChild0(entities[entIdx].child0, parent, entIdx, &instanceIdx, lineNum);
            AddChild1(entities[entIdx].child1, parent, entIdx, &instanceIdx, lineNum);
        }
    }

    fclose(file);
    
    // Set Fog
    switch(curlevel) {
        case  0: fogColorR = 0.3207547f;  fogColorG = 0.29200783f;  fogColorB = 0.29200783f;  fogBaseDensityForLevel = 0.07f;  break;
        case  1: fogColorR = 0.34509805f; fogColorG = 0.38431373f;  fogColorB = 0.49019608f;  fogBaseDensityForLevel = 0.055f; break;
        case  2: fogColorR = 0.47058824f; fogColorG = 0.3882353f;   fogColorB = 0.3928334f;   fogBaseDensityForLevel = 0.05f;  break;
        case  3: fogColorR = 0.32941177f; fogColorG = 0.29411766f;  fogColorB = 0.2509804f;   fogBaseDensityForLevel = 0.065f; break;
        case  4: fogColorR = 0.3882353f;  fogColorG = 0.452415f;    fogColorB = 0.47058824f;  fogBaseDensityForLevel = 0.075f; break;
        case  5: fogColorR = 0.3882353f;  fogColorG = 0.4117647f;   fogColorB = 0.47058824f;  fogBaseDensityForLevel = 0.03f;  break;
        case  6: fogColorR = 0.3f;        fogColorG = 0.24f;        fogColorB = 0.33f;        fogBaseDensityForLevel = 0.07f;  break;
        case  7: fogColorR = 0.38679248f; fogColorG = 0.3471719f;   fogColorB = 0.3302332f;   fogBaseDensityForLevel = 0.07f;  break;
        case  8: fogColorR = 0.44708973f; fogColorG = 0.45681614f;  fogColorB = 0.4811321f;   fogBaseDensityForLevel = 0.04f;  break;
        case  9: fogColorR = 0.4056604f;  fogColorG = 0.3992963f;   fogColorB = 0.36930403f;  fogBaseDensityForLevel = 0.05f;  break;
        case 10: fogColorR = 0.48235294f; fogColorG = 0.58431375f;  fogColorB = 0.5176471f;   fogBaseDensityForLevel = 0.04f;  break;
        case 11: fogColorR = 0.52872473f; fogColorG = 0.58431375f;  fogColorB = 0.48235294f;  fogBaseDensityForLevel = 0.04f;  break;
        case 12: fogColorR = 0.48235294f; fogColorG = 0.58431375f;  fogColorB = 0.5176471f;   fogBaseDensityForLevel = 0.05f;  break;
        case 13: fogColorR = 0.0f;        fogColorG = 0.0f;         fogColorB = 0.0f;         fogBaseDensityForLevel = 0.005f; break;
    }

    fogBaseDensityForLevel *= 4.0f; // Global multiplier to get it to look similar to Unity's
    SetFog();
    malloc_trim(0);
    DualLog("Loaded %d geometry chunks and %u static lights for Level %d... took %f secs\n", loadedInstances, loadedLights, curlevel, get_time() - start_time);
    DebugRAM("end of LoadLevel instances");
    SortInstances(); // All instances loaded, sort them for render order: opaques, doublesideds, transparents.  REORDERS instances[] INDICES!!  CAREFUL!!
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    VoxelLists();
    uint32_t shadowmapPixelCount = SHADOW_MAP_SIZE_SQD * 6u;
    totalShadowmapPixels = MAX_SHADOWMAPS * shadowmapPixelCount;
    shadowMapSSBO = SetupSSBO(shadowMapSSBO, 5, totalShadowmapPixels * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    glUseProgram(shadowmapsClearShaderProgram);
    GLuint groupX_shadClear = (totalShadowmapPixels + 31) / 32;
    glDispatchCompute(groupX_shadClear,1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    numDynamicLights = 0;
    for (int i=0;i<loadedLights;++i) { if (lightIsDynamic[i]) numDynamicLights++; }
    DualLog("%u dynamic lights in level %u\n", numDynamicLights, currentLevel);
    //play_mp3("./Audio/music/THM1-19_medicalstart.mp3",((float)settings_VolumeMusic/100.0f) * 0.4f,100);
    RenderShadowmaps();
    Input_MouselookApply();
    levelCurrentlyLoading = false;
}
