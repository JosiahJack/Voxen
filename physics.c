// physics.cpp - Full Jolt Physics integration for Voxen
#include "voxen.h"
#include "types.h"
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

float fatigue;

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

float GetBasePlayerSpeed(bool running) {
    bool isSprinting = keyStates[GLFW_KEY_LEFT_SHIFT].down; // TODO handle keybind
    if (voxen_Cheats.noclip && isSprinting) return PLAYER_MAX_CYBER_SPEED * 2.5f;
    if (voxen_Cheats.noclip) return PLAYER_MAX_CYBER_SPEED * 1.5f;
    if (voxen_globalContext.currentLevel == LEVEL_CYBERSPACE) return PLAYER_MAX_CYBER_SPEED; //Cyber space speed

    float retval = PLAYER_MAX_WALK_SPEED;
    float bonus = 0.0f;
    if (boosterActive > 0u) bonus = PLAYER_BOOSTER_SPEED_BOOST;
    BodyState bodyState = instances[PLAYER1].bodyState;
    switch (bodyState) {
        case BodyState_Standing:      retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Crouch:        retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_CrouchingDown: retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_StandingUp:    retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Prone:         retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningDown:   retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningUp:     retval = PLAYER_MAX_PRONE_SPEED;  break;
    }

    if ((isSprinting || boosterActive) && running) {
        if (fatigue > 80.0f && boosterActive) retval = PLAYER_MAX_SPRINT_SPEED_FATIGUED;
        else                                           retval = PLAYER_MAX_SPRINT_SPEED;

        if (bodyState == BodyState_Standing || bodyState == BodyState_Crouch || bodyState == BodyState_CrouchingDown) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_CROUCH_SPEED) * 1.5f); // Subtract off the difference in speed between walking and crouching from the sprint speed
        } else if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown || bodyState == BodyState_ProningUp) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_PRONE_SPEED) * 2.0f); // Subtract off the difference in speed between walking and proning from the sprint speed.
        }
    }

    return retval + bonus;
}

void ApplyPlayerMovements(void) {
    Vector3 forward = instances[PLAYER1].forward;
    Vector3 right = instances[PLAYER1].right;
    Vector3 input = {0};    
    if (keyStates[GLFW_KEY_F].down)     input = Vector3_A_plus_B(input, (Vector3){forward.x, 0, forward.z});
    if (keyStates[GLFW_KEY_S].down)     input = Vector3_A_minus_B(input, (Vector3){forward.x, 0, forward.z});
    if (keyStates[GLFW_KEY_D].down)     input = Vector3_A_plus_B(input, (Vector3){right.x, 0, right.z});
    if (keyStates[GLFW_KEY_A].down)     input = Vector3_A_minus_B(input, (Vector3){right.x, 0, right.z});
    if (keyStates[GLFW_KEY_C].down/* && voxen_Cheats.noclip*/) input.y -= 1.0f; // Temporarily allow for now until I have collision working
    if (keyStates[GLFW_KEY_V].down/* && voxen_Cheats.noclip*/) input.y += 1.0f;
    input = normalize_vector3(input);
    float intent = magnitude_vector3(input);
    float speed = GetBasePlayerSpeed(intent > 0.1f);
    Vector3 wishVel = scale_vector3(input, speed);
    Vector3 currentVel = instances[PLAYER1].velocity;
    float accel = boosterActive ? 1.0f : 3.0f;
    Vector3 deltaVel = Vector3_A_minus_B(wishVel, currentVel);
    Vector3 appliedVel = Vector3_A_plus_B(currentVel, scale_vector3(deltaVel, (accel * (float)voxen_globalContext.timeSinceLastPhysicsTick)));
    instances[PLAYER1].velocity = appliedVel; // Gravity applied elsewhere same as everything else.
}

const Vector3 gravityVelocity = { 0.0f, -9.81f, 0.0f };

void UpdateVelocityFromGravity(void) {
    for (int32_t i=PLAYER1;i<loadedInstances;++i) {
        if (i > loadedInstances) return;
        if (instances[i].gravity < 0.01f && instances[i].gravity > -0.01f) continue;
        if (i <= PLAYER2 && voxen_Cheats.noclip) continue;
        
        instances[i].velocity = Vector3_A_plus_B(instances[i].velocity, scale_vector3(gravityVelocity, instances[i].gravity * (float)voxen_globalContext.timeSinceLastPhysicsTick));
    }
}

float reboundVelocity = 0.1f;

void ApplyVelocityUntilCollision(uint16_t i) {
    Vector3 currentPosition = instances[i].position;
    instances[i].cellIndex = PosGetCellCoords(currentPosition.x, currentPosition.z);
    float mag = magnitude_vector3(instances[i].velocity);
    if (i > loadedInstances) return;
    if (!(instances[i].index != PLAYER1 || ConstIndexIsDynamicObject(instances[i].index))) return;
    if (mag < 0.05f) return;
    
    Vector3 dir = normalize_vector3(instances[i].velocity);
    Vector3 currentHitPos = Vector3_A_plus_B(instances[i].position, scale_vector3(dir, PLAYER_RADIUS));
                                                                                  
    Vector3 newPosition = Vector3_A_plus_B(currentPosition, scale_vector3(instances[i].velocity, (float)voxen_globalContext.timeSinceLastPhysicsTick));
    Vector3 newHitPos = Vector3_A_plus_B(currentHitPos, scale_vector3(instances[i].velocity, (float)voxen_globalContext.timeSinceLastPhysicsTick));
    if (i <= PLAYER2 && voxen_Cheats.noclip) { instances[i].position = newPosition; return; }
    
    int32_t cellCoordsCurrentX = PosGetCellCoordX(currentPosition.x);
    int32_t cellCoordsCurrentZ = PosGetCellCoordZ(currentPosition.z);
    int32_t cellCoordsCurrentHitPosX = PosGetCellCoordX(currentHitPos.x);
    int32_t cellCoordsCurrentHitPosZ = PosGetCellCoordZ(currentHitPos.z);
    if (cellCoordsCurrentHitPosX != cellCoordsCurrentX) cellCoordsCurrentX = cellCoordsCurrentHitPosX;
    if (cellCoordsCurrentHitPosZ != cellCoordsCurrentZ) cellCoordsCurrentZ = cellCoordsCurrentHitPosZ;
    int32_t cellCoordsX = PosGetCellCoordX(newHitPos.x);
    int32_t cellCoordsZ = PosGetCellCoordZ(newHitPos.z);
    int32_t cellCoordsCurrent = (cellCoordsCurrentZ * WORLDX) + cellCoordsCurrentX;
    int32_t cellCoords = (cellCoordsZ * WORLDX) + cellCoordsX;
    if (cellCoordsZ > cellCoordsCurrentZ && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDNORTH)) { instances[i].velocity.z = -reboundVelocity; return; } // blocked north
    if (cellCoordsZ < cellCoordsCurrentZ && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDSOUTH)) { instances[i].velocity.z = reboundVelocity; return; } // blocked south
    if (cellCoordsX > cellCoordsCurrentX && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDEAST)) { instances[i].velocity.x = -reboundVelocity; return; } // blocked east
    if (cellCoordsX < cellCoordsCurrentX && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDWEST)) { instances[i].velocity.x = reboundVelocity; return; } // blocked west
    if (newHitPos.y < gridCellFloorHeight[cellCoords]) { instances[i].velocity.y = reboundVelocity; return; } // floor blocked
    if (newHitPos.y > gridCellCeilingHeight[cellCoords]) { instances[i].velocity.y = -reboundVelocity; return; } // ceiling blocked
    if (!(gridCellStates[cellCoords] & CELL_OPEN)) { instances[i].velocity = scale_vector3(dir,-reboundVelocity); return; } // void blocked
        
    instances[i].position = newPosition; // It moves! It lives!!
}

void UpdatePositions(void) {
    for (int32_t i=PLAYER1;i<loadedInstances;++i) ApplyVelocityUntilCollision(i);
}

void ClampVelocity(void) {
    for (int32_t i=START_INDEX_LEVEL_INSTANCES;i<loadedInstances;++i) {
        Vector3 curvel = instances[i].velocity;
        if (magnitude_vector3(curvel) > TERMINAL_VELOCITY) {
            Vector3 dir = normalize_vector3(curvel);
            instances[i].velocity = scale_vector3(dir, TERMINAL_VELOCITY);
        }
    }
}

int32_t Physics(void) {
    UpdateVelocityFromGravity();
    ClampVelocity();
    UpdatePositions();
    return 0; // Ok.
}

// uint32_t Physics_CreateConvexMesh(const float* vertices, uint32_t vertexCount, Vector3 position, Quaternion rotation, uint8_t layer, float mass, bool isStatic) {
//     if (vertexCount > 255 || vertexCount < 3) {  DualLogError("Invalid vert count for convex hull: %d (max ~255)\n", vertexCount); return 0; }
//     
//     uint8_t objectLayer = layer;
//     if (isStatic) objectLayer = Layers::NON_MOVING;
//     JPH::Array<JPH::Vec3> joltVerts;
//     joltVerts.reserve(vertexCount);
//     uint8_t stride = 8; // x,y,z,nx,ny,nz,u,v flat buffer
//     uint16_t floatCount = vertexCount * stride;
//     for (int i = 0; i < floatCount; i += stride) joltVerts.push_back(JPH::Vec3(vertices[i + 0], vertices[i + 1], vertices[i + 2]));
//     JPH::ConvexHullShapeSettings shape(joltVerts, JPH::cDefaultConvexRadius, nullptr);
//     shape.SetEmbedded();
//     JPH::BodyCreationSettings settings(&shape, GetVoxenToJoltPosition(position), GetVoxenToJoltQuaternion(rotation), isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, objectLayer);
//     settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
//     if (!isStatic) settings.mMassPropertiesOverride.mMass = mass;
//     JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::Activate);
//     uint32_t handle = gNextBodyHandle++; gBodyMap[handle] = bodyID;
//     return handle;
// }

// uint32_t Physics_CreateMeshCollider(const float* vertices, const uint32_t* indices, uint32_t vertexCount, uint32_t triCount, Vector3 position, Quaternion rotation, uint8_t layer, float mass, bool isStatic) {
//     if (!vertices || !indices || vertexCount < 3 || triCount == 0) {
//         DualLogError("Invalid mesh data (null ptr or too small)\n");
//         return 0;
//     }
//     if (!isStatic) {
//         isStatic = true; // Force to true for now for testing meshes.
//         DualLogError("MeshShape only for static, marked as static\n");
//     }
// 
//     JPH::VertexList vertList;
//     vertList.reserve(vertexCount);
//     for (uint32_t i = 0; i < vertexCount; ++i) {
//         uint32_t offset = i * VERTEX_ATTRIBUTES_COUNT;
//         float x = vertices[offset + 0];
//         float y = vertices[offset + 1];
//         float z = vertices[offset + 2];
//         if (!isfinite(x + y + z)) {  // Catch NaN/inf
//             DualLogError("Bad vertex %u\n", i);
//             return 0;
//         }
//         vertList.push_back(JPH::Float3(x, y, -z));
//     }
// 
//     JPH::IndexedTriangleList triList;
//     triList.reserve(triCount);
//     for (uint32_t t = 0; t < triCount; ++t) {
//         uint32_t a = indices[t*3 + 0];
//         uint32_t b = indices[t*3 + 1];
//         uint32_t c = indices[t*3 + 2];
//         if (a >= vertexCount || b >= vertexCount || c >= vertexCount) { DualLogError("Bad index in tri %u\n", t); return 0; }
// 
//         triList.push_back(JPH::IndexedTriangle(a, b, c, 0));
//     }
// 
//     JPH::MeshShapeSettings shape(vertList, triList);
//     shape.Sanitize();
// 
//     JPH::ShapeSettings::ShapeResult result = shape.Create();
//     if (!result.IsValid()) { DualLogError("MeshShape.Create() failed: %s\n", result.GetError().c_str()); return 0; }
// 
//     JPH::BodyCreationSettings settings(result.Get(), GetVoxenToJoltPosition(position), GetVoxenToJoltQuaternion(rotation), JPH::EMotionType::Static, Layers::NON_MOVING);
//     JPH::BodyID bodyID = gPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::DontActivate);
//     if (bodyID.IsInvalid()) return 0;
// 
//     uint32_t handle = gNextBodyHandle++;
//     gBodyMap[handle] = bodyID;
//     return handle;
// }

// RaycastHit Raycast(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask) {
//     JPH::RRayCast ray{JPH::RVec3(origin.x, origin.y, origin.z), JPH::Vec3(dir.x, dir.y, dir.z) * distance};
//     JPH::RayCastResult hit;
//     LayerFilter layerFilter(layerMask);
//     if (gPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), layerFilter)) {
//         JPH::RVec3 hitPos = ray.GetPointOnRay(hit.mFraction * distance);
//         Vector3 point = {(float)hitPos.GetX(), (float)hitPos.GetY(), (float)hitPos.GetZ()};
//         Vector3 normal = {0,0,1};
//         { JPH::BodyLockRead lock(gPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
//           if (lock.Succeeded()) {
//               const JPH::Body& body = lock.GetBody();
//               JPH::Vec3 n = body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, hitPos);
//               normal = {(float)n.GetX(), (float)n.GetY(), (float)n.GetZ()};
//           }
//         }
//         uint16_t hitIdx = UINT16_MAX;
//         for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) {
//             if (instances[i].physics_handle && gBodyMap[instances[i].physics_handle] == hit.mBodyID) { hitIdx = i; break; }
//         }
//         return (RaycastHit){.point = point, .normal = normal, .distance = hit.mFraction * distance, .hitInstanceIndex = hitIdx, .hit = true};
//     }
//     return (RaycastHit){.hit = false};
// }
/*
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
}*/

// } // extern "C"

