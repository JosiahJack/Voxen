// physics.cpp - Full Jolt Physics integration for Voxen
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#define JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Renderer/DebugRendererPlayback.h>

#include <iostream>
#include <cstdarg>
#include <thread>
#include <vector>
#include <unordered_map>
using namespace JPH;
using namespace JPH::literals;
using namespace std;

#include "types.h"
extern "C" void ProcessInput(void);
extern "C" void DualLogError(const char* fmt, ...);
extern "C" void DualLog(const char* fmt, ...);
static inline float vsqrtf(float x) { union { float f; unsigned int i; } u = { x }; u.i = 0x1fbd1df5 + (u.i >> 1); return 0.5f * (u.f + x / u.f); } // Don't use glibc if we can help it, nor c++'s standard libraries.
static inline Vector3 Vector3_A_plus_B(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vector3 Vector3_A_minus_B(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vector3 scale_vector3(Vector3 v, float s) { Vector3 res = {v.x * s, v.y * s, v.z * s}; return res; }
static inline float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
static inline float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
static inline float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
static inline Vector3 normalize_vector3(Vector3 v) { float len = magnitude_vector3(v); return len > 0.000001f ? (Vector3){v.x / len, v.y / len, v.z / len} : v; }

static uint32_t GetCollisionMask(uint8_t layer) {
    switch (layer) {
        case PhysicsLayer_Default:           return 0x217efe07u;
        case PhysicsLayer_TransparentFX:     return 0x7c1e07u;
        case PhysicsLayer_IgnoreRaycast:     return 0x7c3e07u;
        case PhysicsLayer_Water:             return 0x0u;
        case PhysicsLayer_UI:                return 0x0u;
        case PhysicsLayer_GunViewModel:      return 0x0u;
        case PhysicsLayer_Geometry:          return 0x178fc07u;
        case PhysicsLayer_NPC:               return 0x6f61e07u;
        case PhysicsLayer_PlayerBullets:     return 0x217e6607u;
        case PhysicsLayer_Player:            return 0x5770607u;
        case PhysicsLayer_Corpse:            return 0x10c4a05u;
        case PhysicsLayer_PhysObjects:       return 0x17e6a01u;
        case PhysicsLayer_Sky:               return 0x201u;
        case PhysicsLayer_PlayerTriggerOnly: return 0x701000u;
        case PhysicsLayer_Trigger:           return 0x1785c01u;
        case PhysicsLayer_Door:              return 0x1707c07u;
        case PhysicsLayer_InterDebris:       return 0xa6a07u;
        case PhysicsLayer_Player2:           return 0x5675e07u;
        case PhysicsLayer_Player3:           return 0x5575e07u;
        case PhysicsLayer_Player4:           return 0x5375e07u;
        case PhysicsLayer_NPCTrigger:        return 0x400u;
        case PhysicsLayer_NPCBullet:         return 0x20767a01u;
        case PhysicsLayer_NPCClip:           return 0x400u;
        case PhysicsLayer_Clip:              return 0x701400u;
        case PhysicsLayer_Automap:           return 0x0u;
        case PhysicsLayer_Culling:           return 0x0u;
        case PhysicsLayer_CorpseSearchable:  return 0x1000801u;
    }
    
    return 0;
}

static void TraceImpl(const char *inFMT, ...) {
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);
	cout << buffer << endl;
}

static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine) {
	cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << endl;
	return true;
};

extern Voxen_GlobalContext voxen_globalContext;
extern uint16_t loadedInstances;

#define LAYER_MASK_PLAYER_COLLIDESWITH ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_TransparentFX) | (1u << PhysicsLayer_IgnoreRaycast) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_Player2) | (1u << PhysicsLayer_Player3) | (1u << PhysicsLayer_Player4) | (1u << PhysicsLayer_Trigger) | (1u << PhysicsLayer_PlayerTriggerOnly) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_NPCBullet) | (1u << PhysicsLayer_Clip))
#define LAYER_MASK_NPC_COLLIDESWITH ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_TransparentFX) | (1u << PhysicsLayer_IgnoreRaycast) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_PlayerBullets) | (1u << PhysicsLayer_Player) | (1u << PhysicsLayer_Player2) | (1u << PhysicsLayer_Player3) | (1u << PhysicsLayer_Player4) | (1u << PhysicsLayer_Trigger) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_NPCTrigger) | (1u << PhysicsLayer_NPCClip) | (1u << PhysicsLayer_Clip))

#define LAYER_MASK_NPC_SIGHT ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_InterDebris) \
	                          | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_Player))

#define LAYER_MASK_NPC_ATTACK ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_Door) \
							   | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_Player))

#define LAYER_MASK_NPC_COLLISION ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_TransparentFX) | (1u << PhysicsLayer_IgnoreRaycast) | (1u << PhysicsLayer_Geometry) \
							      | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_Player) \
							      | (1u << PhysicsLayer_Clip) | (1u << PhysicsLayer_NPCClip) | (1u << PhysicsLayer_PhysObjects))
#define LAYER_MASK_PLAYER_FROB ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_Water) | (1u << PhysicsLayer_Door) \
								| (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_CorpseSearchable))

#define LAYER_MASK_PLAYER_TARGET_ID_FROB ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_CorpseSearchable))
#define LAYER_MASK_PLAYER_ATTACK ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_PlayerBullets) \
								  | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_CorpseSearchable))

#define LAYER_MASK_EXPLOSION ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_PlayerBullets) | (1u << PhysicsLayer_Door) \
							  | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_Player) | (1u << PhysicsLayer_Player2) | (1u << PhysicsLayer_CorpseSearchable))

#define LAYER_MASK_PLAYER_FEET ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry))

#define START_INDEX_LEVEL_INSTANCES 3
extern Entity instances[INSTANCE_COUNT];
static Ref<JPH::CharacterVirtual> gPlayerCharacter;
static Ref<JPH::Shape> gPlayerStandingShape;

namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING     = 1;
    static constexpr uint NUM_LAYERS = 2;
}

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING = JPH::BroadPhaseLayer(0);
    static constexpr JPH::BroadPhaseLayer MOVING     = JPH::BroadPhaseLayer(1);
    static constexpr uint NUM_BROADPHASE_LAYERS = 2;
}

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        BPLayerInterfaceImpl() {
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING]     = BroadPhaseLayers::MOVING;
        }
        virtual uint                                   GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_BROADPHASE_LAYERS; }
        virtual JPH::BroadPhaseLayer                   GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { return mObjectToBroadPhase[inLayer]; }
    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override { return true; }
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override {
            uint32_t mask1 = GetCollisionMask((uint8_t)inLayer1);
            uint32_t mask2 = GetCollisionMask((uint8_t)inLayer2);
            return (mask1 & (1u << inLayer2)) || (mask2 & (1u << inLayer1));
        }
};

static JPH::PhysicsSystem*          gPhysicsSystem = nullptr;
static JPH::TempAllocatorImpl*      gTempAllocator = nullptr;
static JPH::JobSystemThreadPool*    gJobSystem = nullptr;
static std::unordered_map<uint32_t, JPH::BodyID> gBodyMap;
static uint32_t gNextBodyHandle = 1;

class LayerFilter : public JPH::ObjectLayerFilter {
    uint32_t mMask;
    public:
        explicit LayerFilter(uint32_t mask) : mMask(mask) {}
        virtual bool ShouldCollide(JPH::ObjectLayer layer) const override {
            return (mMask & (1u << layer)) != 0;
        }
};

static BPLayerInterfaceImpl broadPhaseLayerInterface;
static ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
static ObjectLayerPairFilterImpl objectLayerPairFilter;
#define NULLENT 0u
#define PLAYER1 1u
#define PLAYER2 2u
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define PLAYER_MAX_WALK_SPEED 3.2f
#define PLAYER_MAX_SPRINT_SPEED 8.8f
#define PLAYER_MAX_CYBER_SPEED 5.0f
#define PLAYER_MAX_CYBER_ULTIMATE_SPEED 12.0f
#define PLAYER_MAX_SPRINT_SPEED_FATIGUED 5.5f
#define PLAYER_MAX_CROUCH_SPEED 1.25f
#define PLAYER_MAX_PRONE_SPEED 0.5f
#define PLAYER_BOOSTER_SPEED_BOOST 1.2f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
#define PLAYER_RADIUS 0.48f
#define PLAYER_HEIGHT 2.00f
#define PLAYER_CAM_OFFSET_Y 0.84f // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84)
#define LEVEL_CYBERSPACE 13
extern KeyState keyStates[MAX_KEYS];
extern KeyState mouseButtons[MAX_MOUSE_BUTTONS];
extern float cam_yaw;
extern Voxen_Cheats voxen_Cheats;
float fatigue;
extern uint8_t boosterActive;

extern "C" {
void Physics_Init() {
    RegisterDefaultAllocator();
    Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
    Factory::sInstance = new Factory();
    RegisterTypes();
    gTempAllocator = new JPH::TempAllocatorImpl(32 * 1024 * 1024);
    gJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4); // 4 threads
    gPhysicsSystem = new JPH::PhysicsSystem();
    gPhysicsSystem->Init(
        65536,     // maxBodies
        0,         // numBodyMutexes
        10240,     // maxBodyPairs
        10240,     // maxContactConstraints
        broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter,
        objectLayerPairFilter
    );
    gPhysicsSystem->SetGravity(Vec3(0.0f, -9.81f, 0.0f));
    DualLog("Physics initialized successfully\n");
}

float GetBasePlayerSpeed(bool running) {
    bool isSprinting = keyStates[GLFW_KEY_LEFT_SHIFT].down; // TODO handle keybind
    if (voxen_Cheats.noclip && isSprinting) return PLAYER_MAX_CYBER_SPEED * 2.5f;
    if (voxen_Cheats.noclip) return PLAYER_MAX_CYBER_SPEED * 1.5f;
    if (voxen_globalContext.currentLevel == LEVEL_CYBERSPACE) return PLAYER_MAX_CYBER_SPEED; //Cyber space speed

    float retval = PLAYER_MAX_WALK_SPEED;
    float bonus = 0.0f;
    if (boosterActive > 0u) bonus = PLAYER_BOOSTER_SPEED_BOOST; // TODO proper booster hookup
    BodyState bodyState = instances[PLAYER1].bodyState;
    switch (bodyState) {
        case 0/*BodyState_Standing*/: 		retval = PLAYER_MAX_WALK_SPEED;   break;
        case 1/*BodyState_Crouch*/: 		retval = PLAYER_MAX_CROUCH_SPEED; break;
        case 2/*BodyState_CrouchingDown*/: 	retval = PLAYER_MAX_CROUCH_SPEED; break;
        case 3/*BodyState_StandingUp*/: 	retval = PLAYER_MAX_WALK_SPEED;   break;
        case 4/*BodyState_Prone*/: 			retval = PLAYER_MAX_PRONE_SPEED;  break;
        case 5/*BodyState_ProningDown*/: 	retval = PLAYER_MAX_PRONE_SPEED;  break;
        case 6/*BodyState_ProningUp*/: 		retval = PLAYER_MAX_PRONE_SPEED;  break;
    }

    if ((isSprinting/* || Inventory.a.BoosterActive()*/) && running) { // TODO proper booster hookup
        if (fatigue > 80.0f/* && !Inventory.a.BoosterActive()*/) retval = PLAYER_MAX_SPRINT_SPEED_FATIGUED; // TODO booster hookup
        else                                                 retval = PLAYER_MAX_SPRINT_SPEED;

        if (bodyState == BodyState_Standing || bodyState == BodyState_Crouch || bodyState == BodyState_CrouchingDown) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_CROUCH_SPEED) * 1.5f); // Subtract off the difference in speed between walking and crouching from the sprint speed
        } else if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown || bodyState == BodyState_ProningUp) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_PRONE_SPEED) * 2.0f); // Subtract off the difference in speed between walking and proning from the sprint speed.
        }
    }

    return retval + bonus;
}

void ApplyPlayerMovements(void) {
    Entity* player = &instances[PLAYER1];
    Vector3 input = {0};    
    if (keyStates[GLFW_KEY_F].down)     input = Vector3_A_plus_B(input, (Vector3){instances[PLAYER1].forward.x, 0, instances[PLAYER1].forward.z});
    if (keyStates[GLFW_KEY_S].down)     input = Vector3_A_minus_B(input, (Vector3){instances[PLAYER1].forward.x, 0, instances[PLAYER1].forward.z});
    if (keyStates[GLFW_KEY_D].down)     input = Vector3_A_plus_B(input, (Vector3){instances[PLAYER1].right.x,   0, instances[PLAYER1].right.z});
    if (keyStates[GLFW_KEY_A].down)     input = Vector3_A_minus_B(input, (Vector3){instances[PLAYER1].right.x,   0, instances[PLAYER1].right.z});
    if (keyStates[GLFW_KEY_C].down/* && voxen_Cheats.noclip*/) input.y -= 1.0f; // Temporarily allow for now until I have collision working
    if (keyStates[GLFW_KEY_V].down/* && voxen_Cheats.noclip*/) input.y += 1.0f;
    float sprintMul = keyStates[GLFW_KEY_LEFT_SHIFT].down ? 1.75f : 1.0f;
    const float moveForce = 1800.0f;
    input = normalize_vector3(input);
    float intent = magnitude_vector3(input);
    Vector3 wishDir = { 0.0f, 0.0f, 0.0f };
    float speed = GetBasePlayerSpeed(intent > 0.1f);
    Vector3 wishVel = scale_vector3(input, speed);
    Vec3 wishVelFinal = Vec3(wishVel.x, wishVel.y, wishVel.z);
    Vec3 currentVel = gPlayerCharacter->GetLinearVelocity();
    if (!voxen_Cheats.noclip) wishVelFinal.SetY(currentVel.GetY());
    float accel = 10.0f; // tweak for smooth coast
    Vec3 deltaVel = wishVelFinal - currentVel;
    Vec3 appliedVel = currentVel + deltaVel * (accel * voxen_globalContext.timeSinceLastPhysicsTick);
    gPlayerCharacter->SetLinearVelocity(appliedVel);
    CharacterVirtual::ExtendedUpdateSettings settings;
    settings.mStickToFloorStepDown = -gPlayerCharacter->GetUp();
    settings.mWalkStairsStepUp     =  gPlayerCharacter->GetUp();
}

void Physics_PreStep(void) {
    if (!gPlayerCharacter) return;

    CharacterVirtual::ExtendedUpdateSettings settings;
    settings.mStickToFloorStepDown = -gPlayerCharacter->GetUp();
    settings.mWalkStairsStepUp = gPlayerCharacter->GetUp();
    DefaultObjectLayerFilter object_filter = gPhysicsSystem->GetDefaultLayerFilter(PhysicsLayer_Player);
    gPlayerCharacter->ExtendedUpdate(voxen_globalContext.timeSinceLastPhysicsTick, -gPlayerCharacter->GetUp() * gPhysicsSystem->GetGravity().Length(), settings, gPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING), object_filter, { }, { }, *gTempAllocator);
}

int32_t Physics(void) {
    static float accumulator = 0.0f;
    static const float fixedStep = 1.0f / 60.0f; // Target framerate :smirk:
    accumulator += voxen_globalContext.timeSinceLastPhysicsTick;
    int steps = 0;
    Physics_PreStep();
    while (accumulator >= fixedStep && steps < 4) {
        gPhysicsSystem->Update(fixedStep, 1, gTempAllocator, gJobSystem);
        accumulator -= fixedStep;
        steps++;
    }
    
    RVec3 p = gPlayerCharacter->GetPosition();
    instances[PLAYER1].position.x = (float)p.GetX();
    instances[PLAYER1].position.y = (float)p.GetY();
    instances[PLAYER1].position.z = (float)p.GetZ();
    return 0; // 0 == it was okay
}

JPH::RVec3 GetVoxenToJoltPosition(Vector3 position) { return JPH::RVec3(position.x, position.y, position.z); }
JPH::Quat GetVoxenToJoltQuaternion(Quaternion rotation) { 
    JPH::Quat q(rotation.x, rotation.y, rotation.z, rotation.w);
    JPH::Quat y180 = JPH::Quat::sRotation(JPH::Vec3::sAxisY(), 3.14159265f);  // 180° around Y
    JPH::Quat final = (y180 * q).Normalized();
    float len = final.Length();
    if (len < 0.9f || len > 1.1f) {
        DualLogError("Final quat bad length %.6f\n", len);
        final = JPH::Quat::sIdentity();
    }
    return final;
}

uint32_t Physics_CreateSphere(float radius, Vector3 position, uint32_t layer, float mass, bool isStatic) {
    uint8_t objectLayer = layer;
    if (isStatic) objectLayer = Layers::NON_MOVING;
    JPH::SphereShapeSettings shape(radius);
    shape.SetEmbedded();
    JPH::BodyCreationSettings settings(&shape, GetVoxenToJoltPosition(position), JPH::Quat::sIdentity(), isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, objectLayer);
    if (!isStatic) settings.mMassPropertiesOverride.mMass = mass;
    if (settings.mMotionType == JPH::EMotionType::Dynamic) settings.mMassPropertiesOverride.mMass = mass;
    JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, settings.mMotionType == JPH::EMotionType::Dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
    uint32_t handle = gNextBodyHandle++; gBodyMap[handle] = bodyID;
    return handle;
}

uint32_t Physics_CreateBox(Vector3 colliderSize, Vector3 offset, Vector3 position, Quaternion rotation, uint8_t layer, float mass, bool isStatic) {
    uint8_t objectLayer = layer;
    if (isStatic) objectLayer = Layers::NON_MOVING;
    JPH::BoxShapeSettings shape(JPH::Vec3(colliderSize.x * 0.5f, colliderSize.y * 0.5f, colliderSize.z * 0.5f));
    shape.SetEmbedded();
    JPH::RVec3 com = GetVoxenToJoltPosition(position);
    JPH::RVec3 com_ofs = GetVoxenToJoltPosition(offset);
    com += com_ofs;
    DualLog("Created box collider at %f %f %f\n", com.GetX(), com.GetY(), com.GetZ());
    JPH::BodyCreationSettings settings(&shape, com, GetVoxenToJoltQuaternion(rotation), isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, objectLayer);
    if (!isStatic) settings.mMassPropertiesOverride.mMass = mass;
    JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::DontActivate);
    uint32_t handle = gNextBodyHandle++;
    gBodyMap[handle] = bodyID;
    return handle;
}

uint32_t Physics_CreateCapsule(float radius, float height, Vector3 position, Quaternion rotation, uint8_t layer, float mass, bool isStatic) {
    uint8_t objectLayer = layer;
    if (isStatic) objectLayer = Layers::NON_MOVING;
    float halfHeight = (height * 0.5f) - radius; // (2.0 * 0.5) - 0.48 = 1.0 - 0.48
    JPH::CapsuleShapeSettings shape(halfHeight, radius);
    shape.SetEmbedded();
    JPH::BodyCreationSettings settings(&shape, GetVoxenToJoltPosition(position), GetVoxenToJoltQuaternion(rotation), isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, objectLayer);
    settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ | JPH::EAllowedDOFs::RotationY;
    if (!isStatic) settings.mMassPropertiesOverride.mMass = mass;
    JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::Activate);
    uint32_t handle = gNextBodyHandle++;
    gBodyMap[handle] = bodyID;
    return handle;
}

uint32_t Physics_CreateConvexMesh(const float* vertices, uint32_t vertexCount, Vector3 position, Quaternion rotation, uint8_t layer, float mass, bool isStatic) {
    if (vertexCount > 255 || vertexCount < 3) {  DualLogError("Invalid vert count for convex hull: %d (max ~255)\n", vertexCount); return 0; }
    
    DualLog("Yay we got one\n");
    uint8_t objectLayer = layer;
    if (isStatic) objectLayer = Layers::NON_MOVING;
    JPH::Array<JPH::Vec3> joltVerts;
    joltVerts.reserve(vertexCount);
    uint8_t stride = 8; // x,y,z,nx,ny,nz,u,v flat buffer
    uint16_t floatCount = vertexCount * stride;
    for (int i = 0; i < floatCount; i += stride) joltVerts.push_back(JPH::Vec3(vertices[i + 0], vertices[i + 1], vertices[i + 2]));
    JPH::ConvexHullShapeSettings shape(joltVerts, JPH::cDefaultConvexRadius, nullptr);
    shape.SetEmbedded();
    JPH::BodyCreationSettings settings(&shape, GetVoxenToJoltPosition(position), GetVoxenToJoltQuaternion(rotation), isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, objectLayer);
    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
    if (!isStatic) settings.mMassPropertiesOverride.mMass = mass;
    JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::Activate);
    uint32_t handle = gNextBodyHandle++; gBodyMap[handle] = bodyID;
    return handle;
}

#define VERTEX_ATTRIBUTES_COUNT 8 // x,y,z,nx,ny,nz,u,v
uint32_t Physics_CreateMeshCollider(const float* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t triCount, Vector3 position, Quaternion rotation, uint8_t layer, float mass, bool isStatic) {
    if (!vertices || !indices || vertexCount < 3 || triCount == 0) {
        DualLogError("Invalid mesh data (null ptr or too small)\n");
        return 0;
    }
//     if (!isStatic) {
        isStatic = true; // Force to true for now for testing meshes.
//         DualLogError("MeshShape only for static\n");
//         return 0;
//     }

    JPH::VertexList vertList;
    vertList.reserve(vertexCount);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        uint32_t offset = i * VERTEX_ATTRIBUTES_COUNT;
        float x = vertices[offset + 0];
        float y = vertices[offset + 1];
        float z = vertices[offset + 2];
        if (!isfinite(x + y + z)) {  // Catch NaN/inf
            DualLogError("Bad vertex %u\n", i);
            return 0;
        }
        vertList.push_back(JPH::Float3(x, y, -z));
    }

    JPH::IndexedTriangleList triList;
    triList.reserve(triCount);
    for (uint32_t t = 0; t < triCount; ++t) {
        uint32_t a = indices[t*3 + 0];
        uint32_t b = indices[t*3 + 1];
        uint32_t c = indices[t*3 + 2];
        if (a >= vertexCount || b >= vertexCount || c >= vertexCount) { DualLogError("Bad index in tri %u\n", t); return 0; }

        triList.push_back(JPH::IndexedTriangle(a, b, c, 0));
    }

    JPH::MeshShapeSettings shape(vertList, triList);
    shape.Sanitize();

    JPH::ShapeSettings::ShapeResult result = shape.Create();
    if (!result.IsValid()) { DualLogError("MeshShape.Create() failed: %s\n", result.GetError().c_str()); return 0; }

    JPH::BodyCreationSettings settings(result.Get(), GetVoxenToJoltPosition(position), GetVoxenToJoltQuaternion(rotation), JPH::EMotionType::Static, Layers::NON_MOVING);
    JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::DontActivate);
    if (bodyID.IsInvalid()) return 0;

    uint32_t handle = gNextBodyHandle++;
    gBodyMap[handle] = bodyID;
    return handle;
}

void Physics_CreatePlayer(Vector3 position) {
    float radius = PLAYER_RADIUS;
    float height = PLAYER_HEIGHT;
    float halfHeight = height * 0.5f - radius;
    gPlayerStandingShape = new CapsuleShape(halfHeight, radius);
    Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
    settings->mShape = gPlayerStandingShape;
    settings->mMaxSlopeAngle = DegreesToRadians(50.0f);
    settings->mMaxStrength = 1000.0f;
    settings->mCharacterPadding = 0.02f;
    settings->mPenetrationRecoverySpeed = 0.5f;
    settings->mPredictiveContactDistance = 0.1f;
    settings->mSupportingVolume = Plane(Vec3::sAxisY(), -radius);
    settings->mBackFaceMode = EBackFaceMode::IgnoreBackFaces;
    gPlayerCharacter = new CharacterVirtual(settings, RVec3(position.x, position.y, position.z), Quat::sIdentity(), PLAYER1, gPhysicsSystem);
}

void Physics_DestroyBody(uint32_t handle) {
    auto it = gBodyMap.find(handle);
    if (it != gBodyMap.end()) {
        gPhysicsSystem->GetBodyInterface().RemoveBody(it->second);
        gPhysicsSystem->GetBodyInterface().DestroyBody(it->second);
        gBodyMap.erase(it);
    }
}

void Physics_SetVelocity(uint32_t handle, float vx, float vy, float vz) {
    auto it = gBodyMap.find(handle);
    if (it != gBodyMap.end()) {
        gPhysicsSystem->GetBodyInterface().SetLinearVelocity(it->second, JPH::Vec3(vx, vy, vz));
    }
}

void Physics_AddImpulse(uint32_t handle, float ix, float iy, float iz) {
    auto it = gBodyMap.find(handle);
    if (it != gBodyMap.end()) gPhysicsSystem->GetBodyInterface().AddImpulse(it->second, JPH::Vec3(ix, iy, iz));
}

void Physics_GetPosition(uint32_t handle, float* outX, float* outY, float* outZ) {
    auto it = gBodyMap.find(handle);
    if (it != gBodyMap.end() && outX && outY && outZ) {
        JPH::RVec3 pos = gPhysicsSystem->GetBodyInterface().GetPosition(it->second);
        *outX = (float)pos.GetX();
        *outY = (float)pos.GetY();
        *outZ = (float)pos.GetZ();
    }
}

RaycastHit Physics_Raycast(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask) {
    JPH::RRayCast ray{JPH::RVec3(origin.x, origin.y, origin.z), JPH::Vec3(dir.x, dir.y, dir.z) * distance};
    JPH::RayCastResult hit;
    LayerFilter layerFilter(layerMask);
    if (gPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), layerFilter)) {
        JPH::RVec3 hitPos = ray.GetPointOnRay(hit.mFraction * distance);
        Vector3 point = {(float)hitPos.GetX(), (float)hitPos.GetY(), (float)hitPos.GetZ()};
        Vector3 normal = {0,0,1};
        { JPH::BodyLockRead lock(gPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
          if (lock.Succeeded()) {
              const JPH::Body& body = lock.GetBody();
              JPH::Vec3 n = body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, hitPos);
              normal = {(float)n.GetX(), (float)n.GetY(), (float)n.GetZ()};
          }
        }
        uint16_t hitIdx = UINT16_MAX;
        for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) {
            if (instances[i].physics_handle && gBodyMap[instances[i].physics_handle] == hit.mBodyID) { hitIdx = i; break; }
        }
        return (RaycastHit){.point = point, .normal = normal, .distance = hit.mFraction * distance, .hitInstanceIndex = hitIdx, .hit = true};
    }
    return (RaycastHit){.hit = false};
}

class AnyHitCollideShapeCollector : public JPH::CollideShapeCollector {
    public:
        bool had_hit = false;
        virtual void AddHit(const JPH::CollideShapeResult&) override { had_hit = true; ResetEarlyOutFraction(0.0f); }
};

bool Physics_CheckCapsule(float posX, float posY, float posZ, float radius, float height, uint32_t layerMask) {
    float half = height * 0.5f - radius;
    JPH::CapsuleShape shape(half, radius);
    JPH::CollideShapeSettings settings;
    AnyHitCollideShapeCollector collector;
    LayerFilter layerFilter(layerMask);
    gPhysicsSystem->GetNarrowPhaseQuery().CollideShape(&shape, JPH::Vec3::sReplicate(1.0f), JPH::Mat44::sTranslation(JPH::Vec3(posX, posY + radius + half, posZ)), settings, JPH::RVec3::sZero(), collector, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), layerFilter);
    return collector.had_hit;
}

} // extern "C"

