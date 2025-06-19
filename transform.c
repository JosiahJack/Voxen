#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "constants.h"
#include "transform.h"
#include "player.h"

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

// Transform vertex function (mimics the shader)
void transform_vertices(float *verticesIn, float *verticesOut, unsigned int vertexCount,
                       InstanceData *instances, unsigned int instanceCount,
                       unsigned int *modelVertexCounts, unsigned int *vbo_offsets,
                       float cameraPos[3], float cameraYaw, float cameraPitch,
                       float fovV, float fovH,
                       unsigned int screenWidth, unsigned int screenHeight) {
    
    verticesOut[0] = 200.0f; // x
    verticesOut[1] = 200.0f; // y
    verticesOut[2] = 1.0f;   // z
    verticesOut[3] = 0.0f;   // u
    verticesOut[4] = 0.0f;   // v
    verticesOut[5] = 0.0f;   // texIndex

    verticesOut[6] = 280.0f; // x
    verticesOut[7] = 200.0f; // y
    verticesOut[8] = 1.0f;   // z
    verticesOut[9] = 1.0f;   // u
    verticesOut[10] = 0.0f;   // v
    verticesOut[11] = 0.0f;   // texIndex
    
    verticesOut[12] = 300.0f; // x
    verticesOut[13] = 238.0f; // y
    verticesOut[14] = 1.0f;   // z
    verticesOut[15] = 1.0f;   // u
    verticesOut[16] = 1.0f;   // v
    verticesOut[17] = 0.0f;   // texIndex
    
    verticesOut[18] = 200.0f; // x // Changes to this don't affect anything.  Still only draws one tri.
    verticesOut[19] = 300.0f; // y
    verticesOut[20] = 1.0f;   // z
    verticesOut[21] = 0.0f;   // u
    verticesOut[22] = 1.0f;   // v
    verticesOut[23] = 0.0f;   // texIndex
    return;
    
    for (unsigned int instanceID = 0; instanceID < instanceCount; instanceID++) {
        InstanceData instance = instances[instanceID];
        unsigned int modelIdx = instance.modelIndex;
        unsigned int vertCount = modelVertexCounts[modelIdx];
        unsigned int baseOffset = vbo_offsets[modelIdx];

        for (unsigned int localIdx = 0; localIdx < vertCount; localIdx++) {
            unsigned int globalOutIdx = (instanceID * vertexCount) + localIdx;
            unsigned int inIdx = (baseOffset + localIdx) * 9;

            // Load input vertex data
            float position[3] = {verticesIn[inIdx + 0], verticesIn[inIdx + 1], verticesIn[inIdx + 2]};
            float uv[2] = {verticesIn[inIdx + 6], verticesIn[inIdx + 7]};
            float texIndex = verticesIn[inIdx + 8];

            // Instance transformation (model to world space)
            float scaledPos[3] = {
                position[0] * instance.scale[0],
                position[1] * instance.scale[1],
                position[2] * instance.scale[2]
            };
            float rotMat[9];
            quat_to_mat3(instance.rotation, rotMat);
            float worldPos[3] = {
                rotMat[0] * scaledPos[0] + rotMat[3] * scaledPos[1] + rotMat[6] * scaledPos[2] + instance.position[0],
                rotMat[1] * scaledPos[0] + rotMat[4] * scaledPos[1] + rotMat[7] * scaledPos[2] + instance.position[1],
                rotMat[2] * scaledPos[0] + rotMat[5] * scaledPos[1] + rotMat[8] * scaledPos[2] + instance.position[2]
            };

            // Compute vector from camera to vertex
            float toVertex[3] = {
                worldPos[0] - cameraPos[0],
                worldPos[1] - cameraPos[1],
                worldPos[2] - cameraPos[2]
            };
            float distance = sqrtf(toVertex[0] * toVertex[0] + toVertex[1] * toVertex[1] + toVertex[2] * toVertex[2]);
            if (distance < 0.001f) distance = 0.001f; // Avoid division by zero

            // Compute yaw and pitch relative to camera (Z-up, adjust for camera orientation)
            float yaw = atan2f(toVertex[0], toVertex[2]) * 180.0f / M_PI; // Yaw around Z
            float pitch = asin(toVertex[1] / distance) * 180.0f / M_PI; // Pitch from vertical (Z-up)

            // Apply camera rotation (in degrees)
            yaw += cameraYaw;
            pitch += cameraPitch; // Positive pitch should look up

            // Normalize yaw to [-180, 180]
            while (yaw > 180.0f) yaw -= 360.0f;
            while (yaw < -180.0f) yaw += 360.0f;

            // Map to screen coordinates
            float halfFovV = fovV / 2.0f;
            float halfFovH = fovH / 2.0f;
            float minYaw = cameraYaw - halfFovH;
            float maxYaw = cameraYaw + halfFovH;
            float minPitch = cameraPitch - halfFovV;
            float maxPitch = cameraPitch + halfFovV;
            float screenX, screenY;

            // Handle 360° FOV wrapping
            if (maxYaw - minYaw == 0.0f) {
                screenX = screenWidth / 2.0f;
            } else {
                screenX = (yaw - minYaw) / (maxYaw - minYaw) * screenWidth;
            }
            if (maxPitch - minPitch == 0.0f) {
                screenY = screenHeight / 2.0f;
            } else {
                screenY = (maxPitch - pitch) / (maxPitch - minPitch) * screenHeight; // Invert pitch for top-down view
            }

            // Clamp to screen bounds
            screenX = (screenX < 0.0f) ? 0.0f : (screenX > screenWidth - 1) ? screenWidth - 1 : screenX;
            screenY = (screenY < 0.0f) ? 0.0f : (screenY > screenHeight - 1) ? screenHeight - 1 : screenY;

            // Write transformed vertex data to output buffer
            unsigned int outIdx = globalOutIdx * 6;
            verticesOut[outIdx + 0] = screenX;
            verticesOut[outIdx + 1] = screenY;
            verticesOut[outIdx + 2] = distance;
            verticesOut[outIdx + 3] = uv[0];
            verticesOut[outIdx + 4] = uv[1];
            verticesOut[outIdx + 5] = texIndex;
        }
    }
}
