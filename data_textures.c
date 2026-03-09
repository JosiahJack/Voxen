#include "os.h"
#include "gl.h"
#include "voxen.h"

// Custom stb_image stripped to barebones just .png just rgb/rgba, no variants, no fluff.
#define STBI_ARENA_SIZE 16 * 1024 * 1024
uint8_t* stbi__arena_base = NULL;
uint8_t* stbi__arena_cursor = NULL;
uint8_t* stbi__arena_end = NULL;

void stbi__arena_init(void) {
    if (!stbi__arena_base) {
        stbi__arena_base = OS_AllocateRAM(NULL, STBI_ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        stbi__arena_cursor = stbi__arena_base;
        stbi__arena_end    = stbi__arena_base + STBI_ARENA_SIZE;
    }
}

void* stbi__arena_alloc(size_t size) {
    if (!stbi__arena_base) { DualLogError("stbi__arena_base was invalid\n"); return NULL; }

    uint8_t* aligned = stbi__arena_cursor;
    if (aligned + size > stbi__arena_end) { DualLogError("stbi__arena_alloc failed buffer overflowed with %u vs %u\n",aligned + size,stbi__arena_end); return NULL; }

    stbi__arena_cursor = aligned + size;
    return aligned;
}

typedef struct {
   uint32_t img_x, img_y;
   int32_t img_n, img_out_n;
   uint8_t *img_buffer, *img_buffer_end;
} stbi__context;

typedef struct {
   stbi__context *s;
   uint8_t *idata, *expanded, *out;
} stbi__png;

enum {
   STBI__F_none=0,
   STBI__F_sub=1,
   STBI__F_up=2,
   STBI__F_avg=3,
   STBI__F_paeth=4,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

inline static uint32_t stbi__get32be(stbi__context *s) {
    const uint8_t *p = s->img_buffer;
    s->img_buffer += 4;
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

typedef struct {
   uint16_t fast[1 << 9];
   uint16_t firstcode[16];
   int32_t maxcode[17];
   uint16_t firstsymbol[16];
   uint8_t  size[288];
   uint16_t value[288];
} stbi__zhuffman;

typedef struct {
   uint8_t *zbuffer, *zbuffer_end;
   int32_t num_bits;
   uint32_t code_buffer;
   uint8_t *zout;
   uint8_t *zout_start;
   stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

inline static int32_t stbi__bit_reverse(int32_t n, int32_t bits) {   
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n >> (16-bits);
}

static int32_t stbi__zbuild_huffman(stbi__zhuffman *z, const uint8_t* sizelist, int32_t num) {
   int32_t i,k=0;
   int32_t next_code[16], sizes[17];
   __builtin_memset(sizes, 0, sizeof(sizes));
   __builtin_memset(z->fast, 0, sizeof(z->fast));
   if (num != 32) {
      for (i=0; i < num; ++i) ++sizes[sizelist[i]];      
   }
   
   sizes[0] = 0;
   for (i=1; i < 16; ++i) {
      if (sizes[i] > (1 << i)) return 0;
   }
   
   int32_t code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (uint16_t)code;
      z->firstsymbol[i] = (uint16_t)k;
      code = (code + sizes[i]);
      if (sizes[i]) {
         if (code-1 >= (1 << i)) return 0;
      }
      
      z->maxcode[i] = code << (16-i); // preshift for inner loop
      code <<= 1;
      k += sizes[i];
   }
   
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
      int s = num == 32 ? 5 : sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         uint16_t fastv = (uint16_t) ((s << 9) | i);
         z->size [c] = (unsigned char)s;
         z->value[c] = (uint16_t) i;
         if (s <= 9) {
            int j = stbi__bit_reverse(next_code[s],s);
            while (j < (1 << 9)) {
               z->fast[j] = fastv;
               j += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}


inline static uint32_t stbi__zreceive(stbi__zbuf *z, int n) {
   if (z->num_bits < n) {
      #pragma GCC unroll 24
      do {
         z->code_buffer |= (unsigned int)(*z->zbuffer++) << z->num_bits;
         z->num_bits += 8;
      } while (z->num_bits <= 24);
   }
   
   uint32_t k = z->code_buffer & ((1u << (uint32_t)n) - 1u);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

inline static uint32_t stbi__zhuffman_decode(stbi__zbuf *a, stbi__zhuffman *z) {
   int s;
   if (a->num_bits < 16) {
      #pragma GCC unroll 24
      do {
         a->code_buffer |= (unsigned int)(*a->zbuffer++) << a->num_bits;
         a->num_bits += 8;
      } while (a->num_bits <= 24);
   }
   int b = z->fast[a->code_buffer & ((1 << 9) - 1)];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
      
   int k = stbi__bit_reverse((int32_t)a->code_buffer, 16);
   for (s=10; ;++s) { if (k < z->maxcode[s]) break; }   
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   a->code_buffer >>= s;
   a->num_bits -= s;
   return (uint32_t)z->value[b];
}

static const int stbi__zlength_base[31] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258,0,0 };
static const int stbi__zlength_extra[31] = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
static const int stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
static const int stbi__zdist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
static int stbi__parse_huffman_block(stbi__zbuf* a) {
   uint8_t* zout = a->zout;
   for(;;) {
      int z = stbi__zhuffman_decode(a, &a->z_length);
      if (z < 256) {         
         *zout++ = (uint8_t)z;
      } else {
         int len,dist;
         if (z == 256) { a->zout = zout; return 1; }
         
         z -= 257;
         len = stbi__zlength_base[z];
         if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
         z = stbi__zhuffman_decode(a, &a->z_distance);
         dist = stbi__zdist_base[z];
         if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
         uint8_t* p = (uint8_t*)(zout - dist);
         if (dist == 1) {
            uint8_t v = *p;
            if (len) { do *zout++ = v; while (--len); }
         } else {
            if (len) { do *zout++ = *p++; while (--len); }
         }
      }
   }
}

static int stbi__compute_huffman_codes(stbi__zbuf *a) {
   static const uint8_t length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   stbi__zhuffman z_codelength;
   uint8_t lencodes[286+32+137];
   uint8_t codelength_sizes[19];
   uint32_t hlit  = stbi__zreceive(a,5) + 257;
   uint32_t hdist = stbi__zreceive(a,5) + 1;
   uint32_t hclen = stbi__zreceive(a,4) + 4;
   uint32_t ntot  = hlit + hdist;
   __builtin_memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (uint32_t i=0; i < hclen; ++i) {
      uint32_t s = stbi__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (uint8_t)s;
   }
   
   stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19);
   uint32_t n = 0;
   while (n < ntot) {
      uint32_t c = stbi__zhuffman_decode(a, &z_codelength);      
      if (c < 16) {
         lencodes[n++] = (uint8_t)c;
      } else {
         uint8_t fill = 0;
         if (c == 16) {
            c = stbi__zreceive(a,2)+3;
            fill = lencodes[n-1];
         } else if (c == 17) {
            c = stbi__zreceive(a,3)+3;
         } else if (c == 18) {
            c = stbi__zreceive(a,7)+11;
         } else return 0;
         
         __builtin_memset(lencodes+n, fill, c);
         n += c;
      }
   }
   
   stbi__zbuild_huffman(&a->z_length, lencodes, hlit);
   stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist);
   return 1;
}

static int stbi__parse_uncompressed_block(stbi__zbuf *a) {
   uint8_t header[4];
   if (a->num_bits & 7) stbi__zreceive(a, a->num_bits & 7);
   int32_t k = 0;
   while (a->num_bits > 0) {
      header[k] = (uint8_t)(a->code_buffer & 255);
      a->code_buffer >>= 8;
      a->num_bits -= 8;
      ++k;
   }

   if (k <= 0) header[0] = *a->zbuffer++;
   if (k <= 1) header[1] = *a->zbuffer++;
   if (k <= 2) header[2] = *a->zbuffer++;
   if (k <= 3) header[3] = *a->zbuffer++;
   int32_t len = header[1] * 256 + header[0];
   __builtin_memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static const uint8_t stbi__zdefault_length[288] = {
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8
};

uint8_t* stbi_zlib_decode_malloc_guesssize_headerflag(const uint8_t* buffer, int32_t len, int32_t initial_size, int32_t* outlen) {
   stbi__zbuf a = {0};
   uint8_t* p = (uint8_t*)stbi__arena_alloc(initial_size);
   a.zbuffer = (uint8_t*)buffer;
   a.zbuffer_end = (uint8_t*)buffer + len;
   a.zout_start = p;
   a.zout       = p;
   a.zbuffer++; // Skip CMF, don't bother checking
   a.zbuffer++; // Skip FLG, don't bother checking   
   a.num_bits = 0;
   a.code_buffer = 0;
   int32_t finalOne, type;
   do {
      finalOne = stbi__zreceive(&a,1);
      type = stbi__zreceive(&a,2);
      if (type == 0) stbi__parse_uncompressed_block(&a);
      else {
         if (type == 1) {
            stbi__zbuild_huffman(&a.z_length, stbi__zdefault_length, 288);
            stbi__zbuild_huffman(&a.z_distance, NULL, 32);
         } else {
            stbi__compute_huffman_codes(&a);
         }
         stbi__parse_huffman_block(&a);
      }
   } while (!finalOne);
   
   if (outlen) *outlen = (int32_t)(a.zout - a.zout_start);
   return a.zout_start;
}

static uint8_t first_row_filter[5] = {
   STBI__F_none,
   STBI__F_sub,
   STBI__F_none,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

inline static int32_t stbi__paeth(int32_t a, int32_t b, int32_t c) {
   int32_t p = a + b - c;
   int32_t pa = vabs(p-a);
   int32_t pb = vabs(p-b);
   int32_t pc = vabs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

// create the png data from post-deflated data
static int32_t stbi__create_png_image_raw(stbi__png* a, uint8_t* raw, uint32_t raw_len, int32_t out_n, uint32_t x, uint32_t y, int32_t img_n) {
   uint32_t i, stride = x*out_n;
   uint32_t img_len, img_width_bytes;
   int32_t k;
   int32_t output_bytes = out_n;
   int32_t filter_bytes = img_n;
   a->out = (uint8_t*)stbi__arena_alloc(x * y * output_bytes);
   img_width_bytes = (((img_n * x * 8) + 7) >> 3);
   img_len = (img_width_bytes + 1) * y;
   if (raw_len < img_len) return 0;

   for (uint32_t j=0; j < y; ++j) {
      uint8_t* cur = a->out + stride*j;
      uint8_t* prior;
      int filter = *raw++;
      if (filter > 4) return 0;

      if (j > 0) prior = cur - stride; // bugfix: need to compute this after 'cur +=' computation above
      else prior = a->out;
      
      if (j == 0) filter = first_row_filter[filter]; // if first row, use special filter that doesn't sample previous row
      for (k=0; k < filter_bytes; ++k) { // handle first byte explicitly
         switch (filter) {
            case STBI__F_none       : cur[k] = raw[k]; break;
            case STBI__F_sub        : cur[k] = raw[k]; break;
            case STBI__F_up         : cur[k] = (raw[k] + prior[k]); break;
            case STBI__F_avg        : cur[k] = (raw[k] + (prior[k]>>1)); break;
            case STBI__F_paeth      : cur[k] = (raw[k] + stbi__paeth(0,prior[k],0)); break;
            case STBI__F_avg_first  : cur[k] = raw[k]; break;
         }
      }

      if (img_n != out_n) cur[img_n] = 255; // first pixel
      raw += img_n;
      cur += out_n;
      prior += out_n;
      #define STBI__CASE(f) \
            case f:     \
               for (i=x-1; i >= 1; --i, cur[filter_bytes]=255,raw+=filter_bytes,cur+=output_bytes,prior+=output_bytes) \
                  for (k=0; k < filter_bytes; ++k)
      switch (filter) {
         STBI__CASE(STBI__F_none)         { cur[k] = raw[k]; } break;
         STBI__CASE(STBI__F_sub)          { cur[k] = (raw[k] + cur[k- output_bytes]); } break; // Supports RGB only, no A
         STBI__CASE(STBI__F_up)           { cur[k] = (raw[k] + prior[k]); } break;
         STBI__CASE(STBI__F_avg)          { cur[k] = (raw[k] + ((prior[k] + cur[k- output_bytes])>>1)); } break; // Also supports RGB only, no A
         STBI__CASE(STBI__F_paeth)        { cur[k] = (raw[k] + stbi__paeth(cur[k- output_bytes],prior[k],prior[k- output_bytes])); } break; // Also supports RGB only, no A
         STBI__CASE(STBI__F_avg_first)    { cur[k] = (raw[k] + (cur[k- output_bytes] >> 1)); } break;
      }
      #undef STBI__CASE
   }

   return 1;
}

extern uint8_t* stbi_load_from_memory(const uint8_t* buffer, int len, int *x, int *y) {
   if (stbi__arena_base) stbi__arena_cursor = stbi__arena_base;
   stbi__context s;
   s.img_n = s.img_out_n = 0;
   s.img_buffer = (uint8_t*)buffer;
   s.img_buffer_end = (uint8_t*)buffer+len;
   void* result = NULL;
   stbi__png z = {0};
   z.s = &s;
   uint32_t ioff = 0;
   z.expanded = NULL;
   z.idata = NULL;
   z.out = NULL;
   s.img_buffer += 8; // Skip header check and trust it's a .png
   s.img_x = s.img_y = 1;
   for (;;) {
      uint32_t length = stbi__get32be(&s);
      uint32_t type   = stbi__get32be(&s);
      switch (type) {
         case 0x49484452: { // IHDR
            s.img_x = stbi__get32be(&s);
            s.img_y = stbi__get32be(&s);
            s.img_buffer++;
            int32_t color = (*s.img_buffer++);
            s.img_buffer += 3;
            s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
            break;
         }
         
         case 0x49444154: { // IDAT
            if (!z.idata) {
               z.idata = stbi__arena_alloc(len + 16);
               ioff = 0;
            }
            
            __builtin_memcpy(z.idata + ioff, s.img_buffer, length);
            s.img_buffer += length;
            ioff += length;
            break;
         }

         case 0x49454E44: { // IEND
            uint32_t bpl = (s.img_x);
            uint32_t raw_len = bpl * s.img_y * s.img_n + s.img_y;
            z.expanded = (uint8_t*)stbi_zlib_decode_malloc_guesssize_headerflag((uint8_t*)z.idata, ioff, raw_len, (int32_t*)(&raw_len));
            if (s.img_n+1 == 4) s.img_out_n = s.img_n+1;
            else s.img_out_n = s.img_n;

            stbi__create_png_image_raw(&z, z.expanded, raw_len, s.img_out_n, s.img_x, s.img_y, z.s->img_n);
            stbi__get32be(&s); // end of PNG chunk, read and skip CRC
            goto Label_parsesuccess;
         }

         default: s.img_buffer += length; break;
      }
      
      stbi__get32be(&s); // end of PNG chunk, read and skip CRC
   }

   Label_parsesuccess:   
   result = z.out;
   z.out = NULL;
   *x = z.s->img_x;
   *y = z.s->img_y;
   return (unsigned char *)result;
}

void LoadTextures(void) {
    if (loadedTexturesMaxIndex > 0) return;

    double start_time = get_time();
    DebugRAM("start of LoadTextures");
    loadedTexturesMaxIndex = totalPixels = totalPaletteColors = 0u;
    DataParser texture_parser;
    if (!parse_data_file(&texture_parser, MAX_VALID_TEXTURE, "./Data/textures.txt")) { DualLogError("Could not parse ./Data/textures.txt!\n"); OS_Exit(1); }
    
    stbi__arena_init();
    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) maxIndex = texture_parser.entries[k].index;
    }

    loadedTexturesMaxIndex = maxIndex + 1;
    int32_t matchedParserIdxes[MAX_VALID_TEXTURE];
    for (uint16_t i = 0; i < loadedTexturesMaxIndex; ++i) matchedParserIdxes[i] = -1;
    for (uint32_t k = 0; k < texture_parser.count; k++) { // Match parser entries to indices ahead of loops
        if (texture_parser.entries[k].index < loadedTexturesMaxIndex) {
            matchedParserIdxes[texture_parser.entries[k].index] = k;
        }
    }
    
    if (loadedTexturesMaxIndex == 0) { DualLogError("No textures found in textures.txt\n"); OS_Exit(1); }
    DualLog("Loading textures( %u/%u), using stb_image version: 2.28, ", loadedTexturesMaxIndex, loadedTexturesMaxIndex);    
    totalPixels = 0U;
    totalPaletteColors = 0U;
    int32_t widths[MAX_VALID_TEXTURE]; __builtin_memset(widths,0,MAX_VALID_TEXTURE * sizeof(int32_t));
    int32_t heights[MAX_VALID_TEXTURE]; __builtin_memset(heights,0,MAX_VALID_TEXTURE * sizeof(int32_t));    
    size_t offsets_size          = loadedTexturesMaxIndex * sizeof(uint32_t);
    size_t sizes_size            = loadedTexturesMaxIndex * 2 * sizeof(int32_t);
    size_t palette_offsets_size  = loadedTexturesMaxIndex * sizeof(uint32_t);
    size_t palettes_size         = MAX_UNIQUE_COLORS * sizeof(uint32_t);
    size_t indices_size          = MAX_TOTAL_PIXELS * sizeof(uint8_t);
    size_t arena_size = offsets_size + sizes_size + palette_offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    uint8_t* cur = (uint8_t*)arena;
    uint32_t* textureOffsets        = (uint32_t*)cur; cur += offsets_size;
    int32_t*  textureSizes          = (int32_t*)cur;  cur += sizes_size;
    uint32_t* texturePaletteOffsets = (uint32_t*)cur; cur += palette_offsets_size;
    uint32_t* texturePalettes       = (uint32_t*)cur; cur += palettes_size;
    uint8_t*  all_indices           = (uint8_t*)cur;
    uint32_t pixel_base = 0u, color_base = 0u;
    for (uint16_t i = 0; i < loadedTexturesMaxIndex; ++i) {
        int32_t currentIndex = matchedParserIdxes[i];
        if (currentIndex < 0 || currentIndex > MAX_VALID_TEXTURE) continue;
        if (!texture_parser.entries[currentIndex].path || texture_parser.entries[currentIndex].path[0] == '\0') continue;
        
        doubleSidedTexture[currentIndex] = (texture_parser.entries[currentIndex].entflags & ENTFLAG_DOUBLESIDED) > 0 ? 1 : 0;
        transparentTexture[currentIndex] = (texture_parser.entries[currentIndex].entflags & ENTFLAG_TRANSPARENT) > 0 ? 1 : 0;
        OsFileHandle fd; int st_size; void* map = OS_OpenAndAllocateFileBufferReadonly(texture_parser.entries[currentIndex].path, &fd, &st_size);
        unsigned char* pixels = stbi_load_from_memory(map, (size_t)st_size, &widths[currentIndex], &heights[currentIndex]);
        OS_DeallocateRAM(map, (size_t)st_size);
        if (!pixels) { DualLogError("stbi_load failed for %s\n", texture_parser.entries[currentIndex].path); OS_Exit(1); }

        totalPixels += widths[currentIndex] * heights[currentIndex];
        int32_t numPixels = widths[currentIndex] * heights[currentIndex];
        uint8_t* indices = all_indices + pixel_base;
        uint32_t palette[256];
        uint8_t  remap[256] = {0};
        uint32_t pal_size = 0;
        int loopIter0 = 0;
        int loopIter1 = 0;
        for (int p = 0; p < numPixels; p++) {
            loopIter0++;
            if (loopIter0 > 16777216) break;
            
            uint32_t color = ((uint32_t*)pixels)[p];
            uint8_t idx = remap[color & 255];
            if (idx && palette[idx - 1] == color) { indices[p] = idx - 1; continue; }

            for (idx = 0; idx < pal_size; idx++) {
                loopIter1++;
                if (loopIter1 > 16777216) break;
                if (palette[idx] == color) {
                    indices[p] = idx;
                    remap[color & 255] = idx + 1;
                    goto Label_found;
                }
            }
                        
            if (pal_size >= 256) { DualLogError("Texture %s exceeded 256 colors %u\n", texture_parser.entries[currentIndex].path, pal_size); /*OS_Exit(1);*/ break; }
                        
            palette[pal_size] = color;
            indices[p] = pal_size;
            remap[color & 255] = pal_size + 1;
            pal_size++;
            Label_found:;
        }
        
        totalPaletteColors += pal_size;
        textureOffsets[currentIndex]        = pixel_base;
        texturePaletteOffsets[currentIndex] = color_base;
        textureSizes[currentIndex * 2]      = widths[currentIndex];
        textureSizes[currentIndex * 2 + 1]  = heights[currentIndex];
        __builtin_memcpy(texturePalettes + color_base, palette, pal_size * sizeof(uint32_t));
        pixel_base += numPixels; if (pixel_base > MAX_TOTAL_PIXELS) { DualLogError("Overflowed unique pixels buffer with %u, max size allowed: %u\n", pixel_base, MAX_TOTAL_PIXELS); OS_Exit(1); }
        color_base += pal_size;  if (color_base > MAX_UNIQUE_COLORS) { DualLogError("Overflowed palette buffer with %u, max size allowed: %u\n", color_base, MAX_UNIQUE_COLORS); OS_Exit(1); }
    }

    DebugRAM("After loop for load textures");
    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors, totalPixels);
    int32_t packed_size = ((int32_t)totalPixels + 3) / 4 * sizeof(uint32_t);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.colorBufferID);
    void* dst = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, packed_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    __builtin_memcpy(dst,all_indices,packed_size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColors * sizeof(uint32_t), texturePalettes, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.textureOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTexturesMaxIndex * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.textureSizesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTexturesMaxIndex * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.texturePaletteOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTexturesMaxIndex * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    glFinish();
    OS_DeallocateRAM(texture_parser.entries,texture_parser.count * sizeof(Entity));
    OS_DeallocateRAM(arena, arena_size); arena = NULL;
    OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE); stbi__arena_base = NULL;
    double end_time = get_time();
    DualLog(" took %.6f secs\n", end_time - start_time);
    DebugRAM("After LoadTextures and after deallocation of LoadTextures arena and stbi arena");
}
