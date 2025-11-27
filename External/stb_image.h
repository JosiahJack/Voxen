// stb_image.h - PNG Load System
extern unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels);

#ifdef STB_IMAGE_IMPLEMENTATION
typedef unsigned short stbi__uint16;
typedef   signed short stbi__int16;
typedef unsigned int   stbi__uint32;
typedef   signed int   stbi__int32;
typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

typedef struct {
   stbi__uint32 img_x, img_y;
   int img_n, img_out_n;
   unsigned char *img_buffer, *img_buffer_end;
} stbi__context;

#include <malloc.h>
#include <stdlib.h>
void *memcpy(void *s1, const void *s2, size_t n); // #include <string.h>
void *memset(void *s, int c, size_t n);

typedef struct {
   stbi__context *s;
   stbi_uc *idata, *expanded, *out;
   int depth;
} stbi__png;

enum {
   STBI__F_none=0,
   STBI__F_sub=1,
   STBI__F_up=2,
   STBI__F_avg=3,
   STBI__F_paeth=4,
   // synthetic filters used for first scanline to avoid needing a dummy row of 0s
   STBI__F_avg_first,
   STBI__F_paeth_first
};

inline static stbi_uc stbi__get8(stbi__context *s) {
   if (s->img_buffer < s->img_buffer_end) return *s->img_buffer++;
   return 0;
}

inline static stbi__uint32 stbi__get32be(stbi__context *s) {
   stbi__uint32 z = (stbi__get8(s) << 8) + stbi__get8(s);
   return (z << 16) + (stbi__get8(s) << 8) + stbi__get8(s);
}

#define STBI__BYTECAST(x)  ((stbi_uc) ((x) & 255))  // truncate int to byte without warnings

typedef struct {
   stbi__uint16 fast[1 << 9];
   stbi__uint16 firstcode[16];
   int maxcode[17];
   stbi__uint16 firstsymbol[16];
   stbi_uc  size[288];
   stbi__uint16 value[288];
} stbi__zhuffman;

typedef struct {
   stbi_uc *zbuffer, *zbuffer_end;
   int num_bits;
   stbi__uint32 code_buffer;
   char *zout;
   char *zout_start;
   char *zout_end;
   stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

inline static int stbi__bitreverse16(int n) {
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

inline static int stbi__bit_reverse(int v, int bits) {
   return stbi__bitreverse16(v) >> (16-bits);
}

static int stbi__zbuild_huffman(stbi__zhuffman *z, const stbi_uc *sizelist, int num) {
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 0, sizeof(z->fast));
   if (num != 32) {
      for (i=0; i < num; ++i) ++sizes[sizelist[i]];      
   }
   
   sizes[0] = 0;
   for (i=1; i < 16; ++i) {
      if (sizes[i] > (1 << i)) return 0;
   }
   
   code = 0;
   
   #pragma GCC unroll 16
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (stbi__uint16) code;
      z->firstsymbol[i] = (stbi__uint16) k;
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
         stbi__uint16 fastv = (stbi__uint16) ((s << 9) | i);
         z->size [c] = (stbi_uc     ) s;
         z->value[c] = (stbi__uint16) i;
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

inline static stbi_uc stbi__zget8(stbi__zbuf *z) {
   return (z->zbuffer >= z->zbuffer_end) ? 0 : *z->zbuffer++;
}

static inline void stbi__fill_bits(stbi__zbuf *z) {
   do {
      if (z->code_buffer >= (1U << z->num_bits)) {
        z->zbuffer = z->zbuffer_end;  /* treat this as EOF so we fail. */
        return;
      }
      z->code_buffer |= (unsigned int) stbi__zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

inline static unsigned int stbi__zreceive(stbi__zbuf *z, int n) {
   unsigned int k;
   if (z->num_bits < n) stbi__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int stbi__zhuffman_decode_slowpath(stbi__zbuf *a, stbi__zhuffman *z) {
   int b,s,k;
   k = stbi__bit_reverse(a->code_buffer, 16);
   for (s=10; ;++s) { if (k < z->maxcode[s]) break; }
   if (s >= 16) return -1;
   
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   if (b >= 288) return -1; // some data was corrupt somewhere!
   if (z->size[b] != s) return -1;  // was originally an assert, but report failure instead.
   
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

inline static int stbi__zhuffman_decode(stbi__zbuf *a, stbi__zhuffman *z) {
   int b,s;
   if (a->num_bits < 16) {
      if ((a->zbuffer >= a->zbuffer_end)) return -1;   /* report error for unexpected end of data. */

      stbi__fill_bits(a);
   }
   
   b = z->fast[a->code_buffer & ((1 << 9) - 1)];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
   
   return stbi__zhuffman_decode_slowpath(a, z);
}

static const int stbi__zlength_base[31] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258,0,0 };
static const int stbi__zlength_extra[31] = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
static const int stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
static const int stbi__zdist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int stbi__parse_huffman_block(stbi__zbuf *a) {
   char *zout = a->zout;
   for(;;) {
      int z = stbi__zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return 0; // error in huffman codes
         if (zout >= a->zout_end) return 0;
         
         *zout++ = (char) z;
      } else {
         stbi_uc *p;
         int len,dist;
         if (z == 256) {
            a->zout = zout;
            return 1;
         }
         if (z >= 286) return 0; // per DEFLATE, length codes 286 and 287 must not appear in compressed data
         z -= 257;
         len = stbi__zlength_base[z];
         if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
         z = stbi__zhuffman_decode(a, &a->z_distance);
         if (z < 0 || z >= 30) return 0; // per DEFLATE, distance codes 30 and 31 must not appear in compressed data
         dist = stbi__zdist_base[z];
         if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
         if (zout - a->zout_start < dist) return 0;
         if (zout + len > a->zout_end) return 0;

         p = (stbi_uc *) (zout - dist);
         if (dist == 1) { // run of one byte; common in images.
            stbi_uc v = *p;
            if (len) { do *zout++ = v; while (--len); }
         } else {
            if (len) { do *zout++ = *p++; while (--len); }
         }
      }
   }
}

static int stbi__compute_huffman_codes(stbi__zbuf *a) {
   static const stbi_uc length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   stbi__zhuffman z_codelength;
   stbi_uc lencodes[286+32+137];
   stbi_uc codelength_sizes[19];
   int i,n;
   int hlit  = stbi__zreceive(a,5) + 257;
   int hdist = stbi__zreceive(a,5) + 1;
   int hclen = stbi__zreceive(a,4) + 4;
   int ntot  = hlit + hdist;
   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = stbi__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (stbi_uc) s;
   }
   
   if (!stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < ntot) {
      int c = stbi__zhuffman_decode(a, &z_codelength);
      if (c < 0 || c >= 19) return 0;
      
      if (c < 16) {
         lencodes[n++] = (stbi_uc) c;
      } else {
         stbi_uc fill = 0;
         if (c == 16) {
            c = stbi__zreceive(a,2)+3;
            if (n == 0) return 0;
            
            fill = lencodes[n-1];
         } else if (c == 17) {
            c = stbi__zreceive(a,3)+3;
         } else if (c == 18) {
            c = stbi__zreceive(a,7)+11;
         } else return 0;
         if (ntot - n < c) return 0;
         
         memset(lencodes+n, fill, c);
         n += c;
      }
   }
   
   if (n != ntot) return 0;
   if (!stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int stbi__parse_uncompressed_block(stbi__zbuf *a) {
   stbi_uc header[4];
   int len,nlen,k;
   if (a->num_bits & 7) stbi__zreceive(a, a->num_bits & 7);
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (stbi_uc) (a->code_buffer & 255);
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   
   if (a->num_bits < 0) return 0;

   while (k < 4) header[k++] = stbi__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return 0;
   if (a->zbuffer + len > a->zbuffer_end) return 0;
   if (a->zout + len > a->zout_end) return 0;
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static const stbi_uc stbi__zdefault_length[288] = {
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

static int stbi__parse_zlib(stbi__zbuf *a) {
   int cmf   = stbi__zget8(a);
   int cm    = cmf & 15;
   int flg   = stbi__zget8(a);
   if ((a->zbuffer >= a->zbuffer_end)) return 0;
   if ((cmf*256+flg) % 31 != 0) return 0;
   if (flg & 32) return 0;
   if (cm != 8) return 0;
   
   a->num_bits = 0;
   a->code_buffer = 0;
   int finalOne, type;
   do {
      finalOne = stbi__zreceive(a,1);
      type = stbi__zreceive(a,2);
      if (type == 0) {
         if (!stbi__parse_uncompressed_block(a)) return 0;
      } else if (type == 3) {
         return 0;
      } else {
         if (type == 1) {
            // use fixed code lengths
            if (!stbi__zbuild_huffman(&a->z_length  , stbi__zdefault_length  , 288)) return 0;
            if (!stbi__zbuild_huffman(&a->z_distance, NULL,  32)) return 0;
         } else {
            if (!stbi__compute_huffman_codes(a)) return 0;
         }
         if (!stbi__parse_huffman_block(a)) return 0;
      }
   } while (!finalOne);
   return 1;
}

char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen) {
   stbi__zbuf a;
   char *p = (char *)malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer + len;
   a.zout_start = p;
   a.zout       = p;
   a.zout_end   = p + initial_size;
   if (stbi__parse_zlib(&a)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else return NULL;
}

static int stbi__check_png_header(stbi__context *s) {
   static const stbi_uc png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
      if (stbi__get8(s) != png_sig[i]) return 0;
   return 1;
}

static stbi_uc first_row_filter[5] = {
   STBI__F_none,
   STBI__F_sub,
   STBI__F_none,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

inline static int stbi__paeth(int a, int b, int c) {
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

static const stbi_uc stbi__depth_scale_table[9] = { 0, 0xff, 0x55, 0, 0x11, 0,0,0, 0x01 };

// create the png data from post-deflated data
static int stbi__create_png_image_raw(stbi__png *a, stbi_uc *raw, stbi__uint32 raw_len, int out_n, stbi__uint32 x, stbi__uint32 y) {
   stbi__context *s = a->s;
   stbi__uint32 i,j,stride = x*out_n;
   stbi__uint32 img_len, img_width_bytes;
   int k;
   int img_n = s->img_n; // copy it into a local for later
   int output_bytes = out_n;
   int filter_bytes = img_n;
   int width = x;
   a->out = (stbi_uc *)malloc(x * y * output_bytes); // extra bytes to write off the end into
   img_width_bytes = (((img_n * x * 8) + 7) >> 3);
   img_len = (img_width_bytes + 1) * y;
   if (raw_len < img_len) return 0;

   for (j=0; j < y; ++j) {
      stbi_uc *cur = a->out + stride*j;
      stbi_uc *prior;
      int filter = *raw++;
      if (filter > 4) return 0;

      prior = cur - stride; // bugfix: need to compute this after 'cur +=' computation above
      if (j == 0) filter = first_row_filter[filter]; // if first row, use special filter that doesn't sample previous row
      for (k=0; k < filter_bytes; ++k) { // handle first byte explicitly
         switch (filter) {
            case STBI__F_none       : cur[k] = raw[k]; break;
            case STBI__F_sub        : cur[k] = raw[k]; break;
            case STBI__F_up         : cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
            case STBI__F_avg        : cur[k] = STBI__BYTECAST(raw[k] + (prior[k]>>1)); break;
            case STBI__F_paeth      : cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(0,prior[k],0)); break;
            case STBI__F_avg_first  : cur[k] = raw[k]; break;
            case STBI__F_paeth_first: cur[k] = raw[k]; break;
         }
      }

      if (img_n != out_n) cur[img_n] = 255; // first pixel
      raw += img_n;
      cur += out_n;
      prior += out_n;
      if (img_n == out_n) { // this is a little gross, so that we don't switch per-pixel or per-component
         int nk = (width - 1)*filter_bytes;
         #define STBI__CASE(f) \
             case f:     \
                for (k=0; k < nk; ++k)
         switch (filter) {
            // "none" filter turns into a memcpy here; make that explicit.
            case STBI__F_none:         memcpy(cur, raw, nk); break;
            STBI__CASE(STBI__F_sub)          { cur[k] = STBI__BYTECAST(raw[k] + cur[k-filter_bytes]); } break;
            STBI__CASE(STBI__F_up)           { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg)          { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k-filter_bytes])>>1)); } break;
            STBI__CASE(STBI__F_paeth)        { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],prior[k],prior[k-filter_bytes])); } break;
            STBI__CASE(STBI__F_avg_first)    { cur[k] = STBI__BYTECAST(raw[k] + (cur[k-filter_bytes] >> 1)); } break;
            STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],0,0)); } break;
         }
         #undef STBI__CASE
         raw += nk;
      } else {
         #define STBI__CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, cur[filter_bytes]=255,raw+=filter_bytes,cur+=output_bytes,prior+=output_bytes) \
                   for (k=0; k < filter_bytes; ++k)
         switch (filter) {
            STBI__CASE(STBI__F_none)         { cur[k] = raw[k]; } break;
            STBI__CASE(STBI__F_sub)          { cur[k] = STBI__BYTECAST(raw[k] + cur[k- output_bytes]); } break;
            STBI__CASE(STBI__F_up)           { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg)          { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k- output_bytes])>>1)); } break;
            STBI__CASE(STBI__F_paeth)        { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k- output_bytes],prior[k],prior[k- output_bytes])); } break;
            STBI__CASE(STBI__F_avg_first)    { cur[k] = STBI__BYTECAST(raw[k] + (cur[k- output_bytes] >> 1)); } break;
            STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k- output_bytes],0,0)); } break;
         }
         #undef STBI__CASE
      }
   }

   return 1;
}

#define STBI__PNG_TYPE(a,b,c,d)  (((unsigned) (a) << 24) + ((unsigned) (b) << 16) + ((unsigned) (c) << 8) + (unsigned) (d))

extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp) {
   stbi__context s;
   s.img_buffer = (stbi_uc *) buffer;
   s.img_buffer_end = (stbi_uc *)buffer+len;
   void* result = NULL;
   stbi__png z;
   z.s = &s;
   if (req_comp < 0 || req_comp > 4) return 0;
   
   stbi_uc palette[1024], pal_img_n=0;
   stbi_uc has_trans=0, tc[3]={0};
   stbi__uint32 ioff=0, idata_limit=0, i, pal_len=0;
   int first=1, color=0;
   z.expanded = NULL;
   z.idata = NULL;
   z.out = NULL;
   if (!stbi__check_png_header(&s)) goto Label_parsefail;

   for (;;) {
      stbi__uint32 length = stbi__get32be(&s);
      stbi__uint32 type   = stbi__get32be(&s);
      switch (type) {
         case STBI__PNG_TYPE('I','H','D','R'): {
            int comp,filter;
            if (!first) goto Label_parsefail;
            
            first = 0;
            if (length != 13) goto Label_parsefail;
            
            s.img_x = stbi__get32be(&s);
            s.img_y = stbi__get32be(&s);
            z.depth = stbi__get8(&s);  if (z.depth != 8) goto Label_parsefail;
            color = stbi__get8(&s);  if (color > 6)         goto Label_parsefail;
            if (color == 3 && z.depth == 16)                  goto Label_parsefail;
            if (color == 3) pal_img_n = 3; else if (color & 1) goto Label_parsefail;
            comp  = stbi__get8(&s);  if (comp) goto Label_parsefail;
            filter= stbi__get8(&s);  if (filter) goto Label_parsefail;
            stbi__get8(&s); // Shift the interlace byte
            if (!s.img_x || !s.img_y) goto Label_parsefail;
            
            if (!pal_img_n) {
               s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
               if ((1 << 30) / s.img_x / s.img_n < s.img_y) goto Label_parsefail;
            } else {
               // if paletted, then pal_n is our final components, and
               s.img_n = 1; // img_n is # components to decompress/filter.
               if ((1 << 30) / s.img_x / 4 < s.img_y) goto Label_parsefail;
            }
           
            break; // even with SCAN_header, have to scan to see if we have a tRNS
         }

         case STBI__PNG_TYPE('P','L','T','E'):  {
            if (first) goto Label_parsefail;
            if (length > 256*3) goto Label_parsefail;
            
            pal_len = length / 3;
            if (pal_len * 3 != length) goto Label_parsefail;
            
            for (i=0; i < pal_len; ++i) {
               palette[i*4+0] = stbi__get8(&s);
               palette[i*4+1] = stbi__get8(&s);
               palette[i*4+2] = stbi__get8(&s);
               palette[i*4+3] = 255;
            }
            break;
         }

         case STBI__PNG_TYPE('t','R','N','S'): {
            if (first) goto Label_parsefail;
            if (z.idata) goto Label_parsefail;
            
            if (pal_img_n) {
               if (pal_len == 0) goto Label_parsefail;
               if (length > pal_len) goto Label_parsefail;
               
               pal_img_n = 4;
               for (i=0; i < length; ++i) palette[i*4+3] = stbi__get8(&s);
            }
            break;
         }

         case STBI__PNG_TYPE('I','D','A','T'): {
            if (first) goto Label_parsefail;
            if (pal_img_n && !pal_len) goto Label_parsefail;
            if (length > (1u << 30)) goto Label_parsefail;
            if ((int)(ioff + length) < (int)ioff) goto Label_parsefail;
            
            if (ioff + length > idata_limit) {
               stbi_uc *p;
               if (idata_limit == 0) idata_limit = length > 4096 ? length : 4096;
               while (ioff + length > idata_limit) idata_limit *= 2;
               p = (stbi_uc *)realloc(z.idata,idata_limit);
               z.idata = p;
            }

            if (s.img_buffer + length <= s.img_buffer_end) {
               memcpy(z.idata + ioff, s.img_buffer, length);
               s.img_buffer += length;
            } else goto Label_parsefail;

            ioff += length;
            break;
         }

         case STBI__PNG_TYPE('I','E','N','D'): {
            stbi__uint32 raw_len, bpl;
            if (first) goto Label_parsefail;
            if (z.idata == NULL) goto Label_parsefail;
            
            // initial guess for decoded data size to avoid unnecessary reallocs
            bpl = (s.img_x * 8 + 7) / 8; // bytes per line, per component
            raw_len = bpl * s.img_y * s.img_n /* pixels */ + s.img_y /* filter mode per row */;
            z.expanded = (stbi_uc *)stbi_zlib_decode_malloc_guesssize_headerflag((char *) z.idata, ioff, raw_len, (int *) &raw_len);
            if (z.expanded == NULL) goto Label_parsefail; // zlib should set error
            
            free(z.idata); z.idata = NULL;
            if ((req_comp == s.img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
               s.img_out_n = s.img_n+1;
            else
               s.img_out_n = s.img_n;
            if (!stbi__create_png_image_raw(&z, z.expanded, raw_len, s.img_out_n, s.img_x, s.img_y)) goto Label_parsefail;
            
            if (has_trans) {
               stbi__uint32 i, pixel_count = s.img_x * s.img_y;
               stbi_uc *p = z.out;
               if (s.img_out_n == 2) {
                  for (i=0; i < pixel_count; ++i) {
                     p[1] = (p[0] == tc[0] ? 0 : 255);
                     p += 2;
                  }
               } else {
                  for (i=0; i < pixel_count; ++i) {
                     if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2]) p[3] = 0;
                     p += 4;
                  }
               }
            }
            
            if (pal_img_n) { // pal_img_n == 3 or 4
               s.img_n = pal_img_n; // record the actual colors we had
               s.img_out_n = pal_img_n;
               if (req_comp >= 3) s.img_out_n = req_comp;
               stbi__uint32 i, pixel_count = s.img_x * s.img_y;
               stbi_uc *p, *temp_out, *orig = z.out;
               p = (stbi_uc *)malloc(pixel_count * s.img_out_n);
               temp_out = p;
               if (s.img_out_n == 3) {
                  for (i=0; i < pixel_count; ++i) {
                     int n = orig[i]*4;
                     p[0] = palette[n  ];
                     p[1] = palette[n+1];
                     p[2] = palette[n+2];
                     p += 3;
                  }
               } else {
                  for (i=0; i < pixel_count; ++i) {
                     int n = orig[i]*4;
                     p[0] = palette[n  ];
                     p[1] = palette[n+1];
                     p[2] = palette[n+2];
                     p[3] = palette[n+3];
                     p += 4;
                  }
               }
               
               free(z.out);
               z.out = temp_out;
            } else if (has_trans) { // non-paletted image with tRNS -> source image has (constant) alpha
               ++s.img_n;
            }

            free(z.expanded); z.expanded = NULL;
            stbi__get32be(&s); // end of PNG chunk, read and skip CRC
            goto Label_parsesuccess;
         }

         default:
            // if critical, fail
            if (first) goto Label_parsefail;
            if ((type & (1 << 29)) == 0) goto Label_parsefail;

            if (length != 0) s.img_buffer += length;
            break;
      }
      
      stbi__get32be(&s); // end of PNG chunk, read and skip CRC
   }

   Label_parsesuccess:
   if (z.depth > 8) return 0;
   
   result = z.out;
   z.out = NULL;
   *x = z.s->img_x;
   *y = z.s->img_y;
   if (comp) *comp = z.s->img_n;
   Label_parsefail:
   free(z.out);      z.out      = NULL;
   free(z.expanded); z.expanded = NULL;
   free(z.idata);    z.idata    = NULL;
   if (result == NULL) return NULL;
   return (unsigned char *)result;
}

#endif // STB_IMAGE_IMPLEMENTATION
