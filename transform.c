#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "render.h"
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
void transform_vertices(float *verticesIn, float *verticesOut, uint32_t vertexCount, InstanceData *instances, unsigned int *modelVertexCounts, unsigned int *vbo_offsets) {
    float halfFovH = cam_fovH / 2.0f; // e.g. 90 / 2 = 45.0deg
    float minYaw = cam_yaw - halfFovH;
    float maxYaw = cam_yaw + halfFovH;

    float halfFovV = cam_fovV / 2.0f; // e.g. 65 / 2 = 32.5deg
    float minPitch = cam_pitch - halfFovV;
    float maxPitch = cam_pitch + halfFovV;
    printf("Transforming all verts with camera values of minYaw: %f, maxYaw: %f, minPitch: %f, maxPitch: %f\n",minYaw,maxYaw,minPitch,maxPitch);
    
    for (unsigned int instanceID = 0; instanceID < INSTANCE_COUNT; instanceID++) {
        InstanceData instance = instances[instanceID];
        uint32_t modelIdx = instance.modelIndex;
        uint32_t vertCount = modelVertexCounts[modelIdx];
        GLuint baseOffset = vbo_offsets[modelIdx];
        for (uint32_t localIdx = 0; localIdx < vertCount; localIdx++) {
            uint32_t globalOutIdx = (instanceID * vertexCount) + localIdx;
            uint32_t inIdx = (baseOffset + localIdx) * 9;

            // Load input vertex data
            float position[3] = {verticesIn[inIdx + 0], verticesIn[inIdx + 1], verticesIn[inIdx + 2]};
            // TODO pass in the normal too.  = {verticesIn[inIdx + 3], verticesIn[inIdx + 4], verticesIn[inIdx + 5]};
            float uv[2] = {verticesIn[inIdx + 6], verticesIn[inIdx + 7]};
            float texIndex = verticesIn[inIdx + 8];
            printf("\n----------------------------------------\nRaw vert %d: x %f, y %f, z %f\n",localIdx,position[0],position[1],position[2]);

            // Scale
            float scaledPos[3] = {
                position[0] * instance.scale[0],
                position[1] * instance.scale[1],
                position[2] * instance.scale[2]
            };
            
            // Rotate
            float rotMat[9];
            quat_to_mat3(instance.rotation, rotMat);
            float worldPos[3] = {// scaledPos[0], scaledPos[1], scaledPos[2] };
                rotMat[0] * scaledPos[0] + rotMat[3] * scaledPos[1] + rotMat[6] * scaledPos[2] + instance.position[0],
                rotMat[1] * scaledPos[0] + rotMat[4] * scaledPos[1] + rotMat[7] * scaledPos[2] + instance.position[1],
                rotMat[2] * scaledPos[0] + rotMat[5] * scaledPos[1] + rotMat[8] * scaledPos[2] + instance.position[2]
            };
            printf("World space for vert %d: x %f, y %f, z %f\n",localIdx,worldPos[0],worldPos[1],worldPos[2]);

            // Translate (treating camera position as 0, 0, 0)
            float vertexInCamSpace[3] = {
                worldPos[0] - cam_x,
                worldPos[1] - cam_y,
                worldPos[2] - cam_z
            };
            
            printf("vertexInCamSpace for vert %d: x %f, y %f, z %f\n",localIdx,vertexInCamSpace[0],vertexInCamSpace[1],vertexInCamSpace[2]);
            float distance = sqrtf(  vertexInCamSpace[0] * vertexInCamSpace[0]
                                   + vertexInCamSpace[1] * vertexInCamSpace[1]
                                   + vertexInCamSpace[2] * vertexInCamSpace[2]);

            if (fabs(distance) < 0.001f) distance = 0.001f; // Avoid division by zero for pitch

            // Compute yaw and pitch relative to camera (Z-up, adjust for camera orientation)
            float yaw = fabs(vertexInCamSpace[0]) < 0.000001f ? 0.0f : rad2deg(atan2f(vertexInCamSpace[1], vertexInCamSpace[0])); // Yaw around Z  atan2(y,x)
            float pitch = 90.0f + rad2deg(atan2f(vertexInCamSpace[2],fabs(vertexInCamSpace[1]) > 0.00001f ? vertexInCamSpace[1] : vertexInCamSpace[0]));
            printf("YawPitch raw vert %d: yaw %f, pitch %f\n",localIdx,yaw,pitch);
            
            // At this point yaw is the distance from screen center and pitch
            // is also distance from screen center.  Now we need to shift them
            // to be relative to lower left corner as 0,0.
            
            // Lower Left (LL) corner of screen is 0,0 for x,y
            yaw = yaw - minYaw; // Shift where yaw 0 maps to left screen edge
            pitch = pitch - minPitch; // Shift where pitch 0 maps to bottom screen edge
            printf("YawPitch shifted vert %d: yaw %f, pitch %f\n",localIdx,yaw,pitch);

            float screenX = ((yaw)   / cam_fovH) * screen_width ;
            float screenY = (pitch / cam_fovV) * screen_height;
            printf("screen final vert %d: screenX %f, screenY %f\n",localIdx,screenX,screenY);

            // Write transformed vertex data to output buffer
            unsigned int outIdx = globalOutIdx * 6;
            verticesOut[outIdx + 0] = screenX;  // x     Screen Coordinates in pixels
            verticesOut[outIdx + 1] = screenY;  // y
            verticesOut[outIdx + 2] = distance; // depth
            verticesOut[outIdx + 3] = uv[0];    // u
            verticesOut[outIdx + 4] = uv[1];    // v
            verticesOut[outIdx + 5] = texIndex; // texIndex
        }
    }
}
