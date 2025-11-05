#include "voxen.h"
#include <stdio.h>
#include <math.h>

// Player
#define PLAYER_CAPSULE_TOTAL_HEIGHT 2.0f
#define PLAYER_CAPSULE_RADIUS 0.48f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
#define PLAYER_CAMERA_OFFSET_Y 0.84f
float move_speed = 0.06;
bool noclip = false;
float testLight_x, testLight_y, testLight_z;

// ================================= Particle System ==================================
int32_t ParticleSystemStep(void) {
    if (gamePaused || menuActive) return 0; // No particle movement on the menu or paused
    
    return 0;
}

// ================================= Vector Logic ==================================
void normalize_vector(float* x, float* y, float* z) {
    float len = sqrtf(*x * *x + *y * *y + *z * *z);
    if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } // Length check to avoid division by zero.
}

static inline Vector3 sub_vector3(Vector3 a, Vector3 b) {
    Vector3 res = {a.x - b.x, a.y - b.y, a.z - b.z};
    return res;
}

static inline Vector3 add_vector3(Vector3 a, Vector3 b) {
    Vector3 res = {a.x + b.x, a.y + b.y, a.z + b.z};
    return res;
}

static inline Vector3 scale_vector3(Vector3 v, float s) {
    Vector3 res = {v.x * s, v.y * s, v.z * s};
    return res;
}

float dot(float x1, float y1, float z1, float x2, float y2, float z2) {
    return x1 * x2 + y1 * y2 + z1 * z2;
}

static inline float dot_vector3(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float dist_sq_vector3(Vector3 a, Vector3 b) {
    Vector3 d = sub_vector3(a, b);
    return dot_vector3(d, d);
}

static inline Vector3 cross_vector3(Vector3 a, Vector3 b) {
    Vector3 res;
    res.x = a.y * b.z - a.z * b.y;
    res.y = a.z * b.x - a.x * b.z;
    res.z = a.x * b.y - a.y * b.x;
    return res;
}

static inline float length_vector3(Vector3 v) {
    return sqrtf(dot_vector3(v, v));
}

static inline Vector3 normalize_vector3(Vector3 v) {
    float len = length_vector3(v);
    if (len > 1e-6f) {
        v.x /= len; v.y /= len; v.z /= len;
    }
    return v;
}

static inline float clampf(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

static inline Vector3 mul_mat4_vector3(const float* mat, Vector3 v) {
    // Assume homogeneous, w=1
    Vector3 res;
    res.x = mat[0] * v.x + mat[4] * v.y + mat[8] * v.z + mat[12];
    res.y = mat[1] * v.x + mat[5] * v.y + mat[9] * v.z + mat[13];
    res.z = mat[2] * v.x + mat[6] * v.y + mat[10] * v.z + mat[14];
    return res;
}

void UpdatePlayerFacingAngles() {
    float rotation[16]; // Extract forward and right vectors from quaternion
    quat_to_matrix(&cam_rotation, rotation);
    cam_forwardx = rotation[8];  // Forward X
    cam_forwardy = rotation[9];  // Forward Y
    cam_forwardz = rotation[10]; // Forward Z
    cam_rightx = rotation[0];  // Right X
    cam_righty = rotation[1];  // Right Y
    cam_rightz = rotation[2];  // Right Z
    normalize_vector(&cam_forwardx, &cam_forwardy, &cam_forwardz); // Normalize forward
    normalize_vector(&cam_rightx, &cam_righty, &cam_rightz); // Normalize strafe
}

// ================================= Collision Detection ==================================
// Point to segment squared distance, outputs closest points
static float dist_point_segment_sq(Vector3 p, Vector3 a, Vector3 b, Vector3* closest_on_seg, Vector3* closest_on_p) {
    Vector3 ab = sub_vector3(b, a);
    float len2 = dot_vector3(ab, ab);
    Vector3 ap = sub_vector3(p, a);
    float proj = dot_vector3(ap, ab);
    float t = 0.0f;
    if (len2 > 1e-6f) {
        t = clampf(proj / len2, 0.0f, 1.0f);
    }
    *closest_on_seg = add_vector3(a, scale_vector3(ab, t));
    *closest_on_p = p;  // Point to itself
    Vector3 diff = sub_vector3(p, *closest_on_seg);
    return dot_vector3(diff, diff);
}

// Segment to segment squared distance (clamped)
static float dist_segment_segment_sq(Vector3 a0, Vector3 a1, Vector3 b0, Vector3 b1, Vector3* closest_a, Vector3* closest_b) {
    Vector3 d1 = sub_vector3(a1, a0);
    Vector3 d2 = sub_vector3(b1, b0);
    Vector3 d0 = sub_vector3(a0, b0);
    float a = dot_vector3(d1, d1);
    float e = dot_vector3(d2, d2);
    float f = dot_vector3(d2, d1);
    float c = dot_vector3(d1, d0);
    float d = dot_vector3(d2, d0);
    float det = a * e - f * f;
    float s = 0.5f, t = 0.5f;
    if (det > 1e-6f) {
        s = clampf((f * d - e * c) / det, 0.0f, 1.0f);
        t = clampf((a * d - f * c) / det, 0.0f, 1.0f);
    }
    *closest_a = add_vector3(a0, scale_vector3(d1, s));
    *closest_b = add_vector3(b0, scale_vector3(d2, t));
    Vector3 diff = sub_vector3(*closest_a, *closest_b);
    return dot_vector3(diff, diff);
}

// Point to triangle squared distance (barycentric), outputs closest on tri
static float point_tri_dist_sq(Vector3 p, Vector3 a, Vector3 b, Vector3 c, Vector3* closest) {
    Vector3 ab = sub_vector3(b, a);
    Vector3 ac = sub_vector3(c, a);
    Vector3 ap = sub_vector3(p, a);
    float d00 = dot_vector3(ab, ab);
    float d01 = dot_vector3(ab, ac);
    float d11 = dot_vector3(ac, ac);
    float d20 = dot_vector3(ap, ab);
    float d21 = dot_vector3(ap, ac);
    float denom = d00 * d11 - d01 * d01;
    float v = 0.0f, w = 0.0f;
    if (fabsf(denom) > 1e-6f) {
        v = clampf((d11 * d20 - d01 * d21) / denom, 0.0f, 1.0f);
        w = clampf((d00 * d21 - d01 * d20) / denom, 0.0f, 1.0f);
        if (v + w > 1.0f) {
            float vv = (v + w - 1.0f);
            v = clampf(v - vv, 0.0f, 1.0f);
            w = clampf(w - vv, 0.0f, 1.0f);
        }
    }
    *closest = add_vector3(a, add_vector3(scale_vector3(ab, v), scale_vector3(ac, w)));
    Vector3 diff = sub_vector3(p, *closest);
    return dot_vector3(diff, diff);
}

// Capsule vs triangle: Returns true if colliding, updates cap_center by pushing out
static bool capsule_vs_tri(Vector3* cap_center, float half_h, float r, Vector3 t0, Vector3 t1, Vector3 t2) {
    Vector3 cap_bot = {cap_center->x, cap_center->y - half_h, cap_center->z};
    Vector3 cap_top = {cap_center->x, cap_center->y + half_h, cap_center->z};
    Vector3 seg_closest, tri_closest;
    tri_closest = t0;
    float min_dist_sq = 1e30f;

    // Vert to segment (3)
    Vector3 tris[3] = {t0, t1, t2};
    for (int i = 0; i < 3; i++) {
        Vector3 dummy;
        float d = dist_point_segment_sq(tris[i], cap_bot, cap_top, &seg_closest, &dummy);
        if (d < min_dist_sq) {
            min_dist_sq = d;
            tri_closest = tris[i];
        }
    }

    // Edges to segment (3)
    Vector3 edges[3][2] = {{t0, t1}, {t1, t2}, {t2, t0}};
    for (int i = 0; i < 3; i++) {
        Vector3 dummy_b;
        float d = dist_segment_segment_sq(cap_bot, cap_top, edges[i][0], edges[i][1], &seg_closest, &dummy_b);
        if (d < min_dist_sq) {
            min_dist_sq = d;
            tri_closest = dummy_b;
        }
    }

    // Segment to plane clamped (1)
    Vector3 n = normalize_vector3(cross_vector3(sub_vector3(t1, t0), sub_vector3(t2, t0)));
    float d0 = dot_vector3(sub_vector3(cap_bot, t0), n);
    float d1 = dot_vector3(sub_vector3(cap_top, t0), n);
    if (fabsf(d0) > 1e-6f || fabsf(d1) > 1e-6f) {
        float t = clampf(-d0 / (d1 - d0), 0.0f, 1.0f);
        Vector3 foot = add_vector3(cap_bot, scale_vector3(sub_vector3(cap_top, cap_bot), t));
        Vector3 cp;
        float d = point_tri_dist_sq(foot, t0, t1, t2, &cp);
        if (d < min_dist_sq) {
            min_dist_sq = d;
            seg_closest = foot;
            tri_closest = cp;
        }
    }

    float min_dist = sqrtf(min_dist_sq);
    if (min_dist > r) return false;  // No collision

    // Resolve: Push along connecting vector
    Vector3 push_dir = sub_vector3(seg_closest, tri_closest);
    push_dir = normalize_vector3(push_dir);
    float pen = r - min_dist + 0.001f;  // Epsilon
    *cap_center = add_vector3(*cap_center, scale_vector3(push_dir, pen));
    return true;
}

int32_t absi(int32_t input) {
    if (input >= 0) return input;
    return input * -1;
}

// Check if instance is in 3x3 grid around object
static inline bool is_instance_in_neighbor_cells(uint32_t instanceCellIdx, uint32_t objectCellIdx) {
    int32_t inst_x = instanceCellIdx % WORLDX; // Convert 1D indices to 2D (x, z)
    int32_t inst_z = instanceCellIdx / WORLDZ;
    int32_t object_x = objectCellIdx % WORLDX;
    int32_t object_z = objectCellIdx / WORLDZ;
    return absi(inst_x - object_x) <= 1 && absi(inst_z - object_z) <= 1; // 0 means same cell, 1 means one of the 8 neighbors, accept all.
}

// ================================= Physics ==================================
void PlayerPhysics(void) {
    // Player Movement from Input
    if (window_has_focus) { // Move the player based on input first, then bound it below...
        UpdatePlayerFacingAngles();
        ProcessInput();
    }

    // Player Physics: Capsule-triangle naive collision
    if (noclip) return;
    
    // Apply gravity to camera (affects bottom of capsule)
    if (instances[PLAYER1].entflags & ENTFLAG_USEGRAVITY) instances[PLAYER1].position.y -= 0.01f;
    
    // Capsule setup: radius=0.48, height=2.0, center at instances[PLAYER1].position.y - 1.84
    float capsule_offset = currentLevel == LEVEL_CYBERSPACE ? 0.0f : PLAYER_CAMERA_OFFSET_Y;  // Center below camera (1.84 below, 0.16 above for total capsule heights including end radii).
    Vector3 cap_center = {instances[PLAYER1].position.x, instances[PLAYER1].position.y - capsule_offset, instances[PLAYER1].position.z};          // Cyberspace in Unity version of Citadel had sphere collider at 0.84f offset to center on camera,
                                                                          // here we center the camera on the capsule center which is the player center for cyberspace, simpler.
    float half_height = currentLevel == LEVEL_CYBERSPACE ? PLAYER_CAPSULE_RADIUS : (2.0f - (PLAYER_CAPSULE_RADIUS * 2.0f)) * 0.5f;  // Half of 2.0f height

//     float body_state_add = 0.0f;
//     if (currentLevel != LEVEL_CYBERSPACE) {
//         switch (playerMovement.bodyState) {
//             case BodyState_Standing: body_state_add = 0.32f; break;//(PLAYER_HEIGHT * 0.5f); break; TODO
//             // Add cases for crouch/prone: adjust half_height, offset
//         }
//         // For now, assume fixed; extend as needed
//     }
    
    // Naive loop over all instances and their triangles
    uint32_t numTrisProcessed = 0;
    for (uint32_t i = 3; i < loadedInstances; i++) { // Skip player indices and start at 3
        if (instances[i].modelIndex > loadedModels) continue;
        if (ConstIndexIsDynamicObject(instances[i].index)) continue;
        if (!is_instance_in_neighbor_cells(cellIndexForInstance[i],playerCellIdx)) continue;
        
        int32_t mid = instances[i].entflags & ENTFLAG_CARDCHUNK ? GEOMETRY_LOD_CARD_MODEL_IDX : instances[i].modelIndex;
        if (mid > loadedModels || mid < 0) continue;
        if (modelVertexCounts[mid] < 3 || modelTriangleCounts[mid] == 0) continue;

        const float* world_mat = &modelMatrices[i * 16];
        uint32_t num_tris = modelTriangleCounts[mid];
        for (uint32_t t = 0; t < num_tris; t++) {
            uint32_t i0 = modelTriangles[mid][t * 3 + 0] * VERTEX_ATTRIBUTES_COUNT;
            uint32_t i1 = modelTriangles[mid][t * 3 + 1] * VERTEX_ATTRIBUTES_COUNT;
            uint32_t i2 = modelTriangles[mid][t * 3 + 2] * VERTEX_ATTRIBUTES_COUNT;
            
            // Transform positions to world
            Vector3 v0 = mul_mat4_vector3(world_mat, (Vector3){ modelVertices[mid][i0 + 0], modelVertices[mid][i0 + 1], modelVertices[mid][i0 + 2] });
            Vector3 v1 = mul_mat4_vector3(world_mat, (Vector3){ modelVertices[mid][i1 + 0], modelVertices[mid][i1 + 1], modelVertices[mid][i1 + 2] });
            Vector3 v2 = mul_mat4_vector3(world_mat, (Vector3){ modelVertices[mid][i2 + 0], modelVertices[mid][i2 + 1], modelVertices[mid][i2 + 2] });
            numTrisProcessed++;
            
            // Test and resolve
            capsule_vs_tri(&cap_center, half_height, PLAYER_CAPSULE_RADIUS, v0, v1, v2);
        }
    }

    instances[PLAYER1].position.x = cap_center.x; // Update camera from resolved capsule center
    instances[PLAYER1].position.y = cap_center.y + capsule_offset;  // Restore offset
    instances[PLAYER1].position.z = cap_center.z;
}

int32_t Physics(void) {
    if (gamePaused || menuActive) return 0; // No physics on the menu or paused
    
    PlayerPhysics();
    return 0;
}
