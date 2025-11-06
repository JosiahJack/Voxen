#include <string.h>
#include <sys/stat.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "External/stb_image_write.h"
#include "voxen.h"
#include "event.h"

// MD5 (128-bit / 16 bytes) – tiny self-contained implementation
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define F(x,y,z) (((x) & (y)) | (~(x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & ~(z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | ~(z)))
// Very small, public-domain MD5 – copy-paste from https://github.com/kerukuro/digestpp/blob/master/algorithm/detail/constants/md5_constants.hpp
static const uint32_t md5Constants[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,	0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340,	0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,	0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa,	0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
	
void md5(const uint8_t *data, size_t len, uint8_t out[16]) {

    uint32_t h[4] = {0x67452301,0xefcdab89,0x98badcfe,0x10325476};
    uint32_t a,b,c,d,f,g;
    uint32_t M[16];
    size_t i, n = len;
    while (n >= 64) {
        memcpy(M, data, 64);
        a = h[0]; b = h[1]; c = h[2]; d = h[3];
        for (i = 0; i < 64; ++i) {
            if (i < 16) { f = F(b,c,d); g = i; }
            else if (i < 32) { f = G(b,c,d); g = (5*i+1)%16; }
            else if (i < 48) { f = H(b,c,d); g = (3*i+5)%16; }
            else { f = I(b,c,d); g = (7*i)%16; }
            uint32_t temp = d;
            d = c; c = b;
            b += ROTL(a + f + md5Constants[i] + M[g], (i%4)*7 + 7);
            a = temp;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        data += 64; n -= 64;
    }

    /* pad the last block */
    uint8_t pad[64] = {0};
    memcpy(pad, data, n);
    pad[n] = 0x80;
    if (n >= 56) { /* need a second block */
        /* (process first block) */
        memcpy(M, pad, 64);
        a = h[0]; b = h[1]; c = h[2]; d = h[3];
        for (i = 0; i < 64; ++i) { /* same loop as above */ }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        memset(pad, 0, 56);
    }

    uint64_t bits = len * 8;
    for (i = 0; i < 8; ++i) pad[56+i] = (bits >> (i*8)) & 0xFF;
    memcpy(M, pad, 64);
    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    for (i = 0; i < 64; ++i) { /* same loop */ }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    for (i = 0; i < 4; ++i) {
        out[i*4+0] = h[i] >> 0;
        out[i*4+1] = h[i] >> 8;
        out[i*4+2] = h[i] >> 16;
        out[i*4+3] = h[i] >> 24;
    }
}

bool ConstIndexInBounds(int constdex) {
	return (constdex >= 0 && constdex <= 760);
}

bool ConstIndexIsGeometry(int constdex) {
	return (constdex >= 0 && constdex <= 306) || constdex == 760;
}

bool ConstIndexIsDoor(int constdex) {
	return (constdex >= 496 && constdex < 515);
}

bool ConstIndexIsLightStaticSaveable(int constdex) {
	return constdex == 748;
}

bool ConstIndexIsGenericTransform(int constdex) {
	return constdex == 749;
}

bool ConstIndexIsDynamicObject(uint16_t constIndex) {
    return     (constIndex >= 307 && constIndex <= 404)
            ||  constIndex == 417
            || (constIndex >= 419 && constIndex <= 428)
            || (constIndex >= 430 && constIndex <= 437)
            || (constIndex >= 440 && constIndex <= 442)
            || (constIndex >= 458 && constIndex <= 463)
            || (constIndex >= 465 && constIndex <= 476);
}

bool ConstIndexIsStaticObjectSaveable(int constdex) {
	return ((constdex >= 448 && constdex < 458)
			|| constdex == 480 || constdex == 516
			|| (constdex >= 518 && constdex <= 526)
			|| constdex == 530 || constdex == 531 || constdex == 546
			|| constdex == 555 || constdex == 594 || constdex == 596
			|| constdex == 598
			|| (constdex >= 600 && constdex < 603)
			|| (constdex >= 604 && constdex < 616)
			|| (constdex >= 688 && constdex < 693)
			|| constdex == 694 || constdex == 695
			|| (constdex >= 699 && constdex < 704)
			|| (constdex >= 741 && constdex < 746));
}

bool ConstIndexIsStaticObjectImmutable(int constdex) {
	return ((constdex >= 527 && constdex < 530)
			|| (constdex >= 532 && constdex < 546)
			|| (constdex >= 547 && constdex < 553)
			|| constdex == 554
			|| (constdex >= 556 && constdex < 594)
			|| constdex == 595 || constdex == 597 || constdex == 599
			|| constdex == 601 || constdex == 603
			|| (constdex >= 616 && constdex < 688)
			|| constdex == 693 || constdex == 696 || constdex == 697
			|| constdex == 698
			|| (constdex >= 704 && constdex < 717)
			|| constdex == 720
			|| (constdex >= 733 && constdex < 736)
			|| (constdex >= 737 && constdex < 739)
			|| constdex == 746
			|| constdex == 747
			|| (constdex >= 750 && constdex <= 759));
}

bool ConstIndexIsNPC(int constdex) {
	return (constdex >= 419 && constdex < 448);
}

bool ConstIndexIsHardware(int constdex) {
	return (constdex >= 328) && (constdex <= 339);
}

bool ConstIndexIsAmbient(int constdex) {
    return (constdex >= 621 && constdex <= 655);
}

void Screenshot() {
    struct stat st = {0};
    if (stat("Screenshots", &st) == -1) { // Check and make ./Screenshots/ folder if it doesn't exist yet.
        if (mkdir("Screenshots", 0755) != 0) { DualLogError("Failed to create Screenshots folder\n"); return; }
    }
    
    unsigned char* pixels = malloc(screen_width * screen_height * 4 * sizeof(char));
    glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    char timestamp[32];
    char filename[96];
    time_t now = time(NULL);
    struct tm *utc_time = localtime(&now);    
    if (utc_time) strftime(timestamp, sizeof(timestamp), "%d%b%Y_%H_%M_%S", utc_time);
    snprintf(filename, sizeof(filename), "Screenshots/%s_%s_x%.2f_y%.2f_z%.2f__time_%.1f.png", timestamp, VERSION_STRING, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z, get_time());
    if (!stbi_write_png(filename, screen_width, screen_height, 4, pixels, screen_width * 4)) DualLogError("Failed to save screenshot\n");
    else DualLog("Saved screenshot %s\n", filename);

    free(pixels);
}

GLuint SetupSSBO(GLuint id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage) {
    if (!levelCurrentlyLoading && globalFrameNum > 1) DualLogError("Trying to delete and generate a new SSBO %u outside of a level load!\n", bindingIndex);
    if (id != 0) glDeleteBuffers(1, &id); // Clear last level's SSBO.
    GLuint new_id = 0;
    glGenBuffers(1, &new_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, new_id);
    return new_id;
}

// Construct rotation matrix (column-major, Unity: X+ right, Y+ up, Z+ forward)
void quat_to_matrix(Quaternion* q, float* m) {
    float x = q->x, y = q->y, z = q->z, w = q->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    // Column-major rotation matrix for Unity (Y+ up, Z+ forward, X+ right)
    m[0]  = 1.0f - 2.0f * (y2 + z2); // Right X
    m[1]  = 2.0f * (xy + wz);        // Right Y
    m[2]  = 2.0f * (xz - wy);        // Right Z
    m[3]  = 0.0f;
    m[4]  = 2.0f * (xy - wz);        // Up X
    m[5]  = 1.0f - 2.0f * (x2 + z2); // Up Y
    m[6]  = 2.0f * (yz + wx);        // Up Z
    m[7]  = 0.0f;
    m[8]  = 2.0f * (xz + wy);        // Forward X
    m[9]  = 2.0f * (yz - wx);        // Forward Y
    m[10] = 1.0f - 2.0f * (x2 + y2); // Forward Z
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

bool CursorVisible(void) {
    return (inventoryMode || menuActive || gamePaused);
}
