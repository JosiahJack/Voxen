#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "render.h"
#include "constants.h"
#include "transform.h"
#include "player.h"
#include "quaternion.h"

// Function to convert quaternion to 3x3 rotation matrix
void quat_to_mat3(float q[4], float mat[9]) {
    float x2 = q[0] * q[0];
    float y2 = q[1] * q[1];
    float z2 = q[2] * q[2];
    float xy = q[0] * q[1];
    float xz = q[0] * q[2];
    float yz = q[1] * q[2];
    float wx = q[3] * q[0];
    float wy = q[3] * q[1];
    float wz = q[3] * q[2];

    mat[0] = 1.0f - 2.0f * (y2 + z2); mat[3] = 2.0f * (xy + wz);      mat[6] = 2.0f * (xz - wy);
    mat[1] = 2.0f * (xy - wz);      mat[4] = 1.0f - 2.0f * (x2 + z2); mat[7] = 2.0f * (yz + wx);
    mat[2] = 2.0f * (xz + wy);      mat[5] = 2.0f * (yz - wx);      mat[8] = 1.0f - 2.0f * (x2 + y2);
}

// Helper function to rotate a vector by a quaternion
void rotate_by_quaternion(float* vec, float qx, float qy, float qz, float qw, float* result) {
    float vx = vec[0], vy = vec[1], vz = vec[2];

    // Quaternion multiplication: q * v * q^{-1}, where v = (0, vx, vy, vz)
    // Compute q * v
    float temp_w = -qx * vx - qy * vy - qz * vz;
    float temp_x = qw * vx + qy * vz - qz * vy;
    float temp_y = qw * vy - qx * vz + qz * vx;
    float temp_z = qw * vz + qx * vy - qy * vx;

    // Compute (q * v) * q^{-1}, where q^{-1} = (-qx, -qy, -qz, qw) since q is unit-length
    result[0] = -temp_w * qx + temp_x * qw - temp_y * qz + temp_z * qy;
    result[1] = -temp_w * qy + temp_x * qz + temp_y * qw - temp_z * qx;
    result[2] = -temp_w * qz - temp_x * qy + temp_y * qx + temp_z * qw;
}

// Main function to transform vertices to screen space
void transform_vertices(float *verticesIn, float *verticesOut, uint32_t vertexCount, InstanceData *instances, unsigned int *modelVertexCounts, unsigned int *vbo_offsets) {
    // Convert field of view from degrees to radians
    float halfFovH_rad = (cam_fovH / 2.0f) * (M_PI / 180.0f);
    float halfFovV_rad = (cam_fovV / 2.0f) * (M_PI / 180.0f);
    float tan_half_fovH = tanf(halfFovH_rad);
    float tan_half_fovV = tanf(halfFovV_rad);

    // Define near and far planes (adjust as needed)
    float near = 0.1f;
    float far = 100.0f;
    for (unsigned int instanceID = 0; instanceID < INSTANCE_COUNT; instanceID++) {
        InstanceData instance = instances[instanceID];
        uint32_t modelIdx = instance.modelIndex;
        uint32_t vertCount = modelVertexCounts[modelIdx];
        GLuint baseOffset = vbo_offsets[modelIdx];

        for (uint32_t localIdx = 0; localIdx < vertCount; localIdx++) {
            // Calculate indices
            uint32_t globalOutIdx = (instanceID * vertexCount) + localIdx;
            uint32_t inIdx = (baseOffset + localIdx) * 9;

            // Load input vertex data
            float position[3] = {verticesIn[inIdx + 0], verticesIn[inIdx + 1], verticesIn[inIdx + 2]};
            float uv[2] = {verticesIn[inIdx + 6], verticesIn[inIdx + 7]};
            float texIndex = verticesIn[inIdx + 8];

            // 1. Model Transformation
            // Scale
            float scaledPos[3] = {
                position[0] * instance.scale[0],
                position[1] * instance.scale[1],
                position[2] * instance.scale[2]
            };

            // Rotate using instance.rotation quaternion
            float rotatedPos[3];
            rotate_by_quaternion(scaledPos, instance.rotation[0], instance.rotation[1], instance.rotation[2], instance.rotation[3], rotatedPos);

            // Translate
            float worldPos[3] = {
                rotatedPos[0] + instance.position[0],
                rotatedPos[1] + instance.position[1],
                rotatedPos[2] + instance.position[2]
            };

            // 2. View Transformation (world space to camera space)
            // Translate relative to camera position
            float relativePos[3] = {
                worldPos[0] - cam_x,
                worldPos[1] - cam_y,
                worldPos[2] - cam_z
            };

            // Apply inverse camera rotation
            float cameraSpacePos[3];
             // Pass quaternion of camera's inverse rotation (conjugate of cam_rotation)
            rotate_by_quaternion(relativePos, -cam_rotation.x, -cam_rotation.y, -cam_rotation.z, cam_rotation.w, cameraSpacePos);

            // 3. Projection Transformation
            float x = cameraSpacePos[0]; // X+ right
            float y = cameraSpacePos[1]; // Y+ forward (depth)
            float z = cameraSpacePos[2]; // Z+ up

            // Handle perspective division safely
            float screenX, screenY, depth;
            if (y > 0.0001f) { // Ensure vertex is in front of camera
                float inv_y = 1.0f / y;

                // Project to NDC
                float ndc_x = (1.0f / tan_half_fovH) * x * inv_y;
                float ndc_y = (1.0f / tan_half_fovV) * z * inv_y;
                float ndc_z = ((far + near) / (far - near)) - ((2.0f * far * near) / (far - near)) / y;

                // Map NDC to screen space (assuming (0,0) at top-left, Y+ down)
                screenX = (ndc_x + 1.0f) * (screen_width / 2.0f);
                screenY = (1.0f - ndc_y) * (screen_height / 2.0f);
                depth = ndc_z; // Depth in [-1, 1] for z-buffering
            } else {
                // Vertex behind camera or on near plane
                screenX = 0.0f;
                screenY = 0.0f;
                depth = 1.0f; // Push to far plane
            }

            // 4. Write to output buffer
            unsigned int outIdx = globalOutIdx * 6;
            verticesOut[outIdx + 0] = screenX;    // Screen X
            verticesOut[outIdx + 1] = screenY;    // Screen Y
            verticesOut[outIdx + 2] = depth;      // Depth
            verticesOut[outIdx + 3] = uv[0];      // U
            verticesOut[outIdx + 4] = uv[1];      // V
            verticesOut[outIdx + 5] = texIndex;   // Texture Index
        }
    }
}
