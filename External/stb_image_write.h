#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#include <stdint.h>
#include <stdlib.h>

// if STB_IMAGE_WRITE_STATIC causes problems, try defining STBIWDEF to 'inline' or 'static inline'
#ifndef STBIWDEF
#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF  static
#else
#ifdef __cplusplus
#define STBIWDEF  extern "C"
#else
#define STBIWDEF  extern
#endif
#endif
#endif

STBIWDEF int stbi_write_bmp(char const *filename, int w, int h, int comp, const void  *data);


#endif//INCLUDE_STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define STBIW_UCHAR(x) (unsigned char)((x) & 0xff)

typedef void stbi_write_func(void *context, void *data, int size);

typedef struct {
   stbi_write_func *func;
   void *context;
   unsigned char buffer[64];
   int buf_used;
} stbi__write_context;

static void stbi__start_write_callbacks(stbi__write_context *s, stbi_write_func *c, void *context) {
   s->func    = c;
   s->context = context;
}

static void stbi__stdio_write(void *context, void *data, int size) {
   fwrite(data,1,size,(FILE*) context);
}

static void stbiw__writefv(stbi__write_context *s, const char *fmt, va_list v) {
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = STBIW_UCHAR(va_arg(v, int));
                     s->func(s->context,&x,1);
                     break; }
         case '2': { int x = va_arg(v,int);
                     unsigned char b[2];
                     b[0] = STBIW_UCHAR(x);
                     b[1] = STBIW_UCHAR(x>>8);
                     s->func(s->context,b,2);
                     break; }
         case '4': { uint32_t x = va_arg(v,int);
                     unsigned char b[4];
                     b[0]=STBIW_UCHAR(x);
                     b[1]=STBIW_UCHAR(x>>8);
                     b[2]=STBIW_UCHAR(x>>16);
                     b[3]=STBIW_UCHAR(x>>24);
                     s->func(s->context,b,4);
                     break; }
         default: return;
      }
   }
}

static void stbiw__write_flush(stbi__write_context *s) {
   if (s->buf_used) {
      s->func(s->context, &s->buffer, s->buf_used);
      s->buf_used = 0;
   }
}

static void stbiw__write1(stbi__write_context *s, unsigned char a) {
   if ((size_t)s->buf_used + 1 > sizeof(s->buffer)) stbiw__write_flush(s);
   s->buffer[s->buf_used++] = a;
}

static void stbiw__write3(stbi__write_context *s, unsigned char a, unsigned char b, unsigned char c) {
   int n;
   if ((size_t)s->buf_used + 3 > sizeof(s->buffer)) stbiw__write_flush(s);
   n = s->buf_used;
   s->buf_used = n+3;
   s->buffer[n+0] = a;
   s->buffer[n+1] = b;
   s->buffer[n+2] = c;
}

static void stbiw__write_pixel(stbi__write_context *s, int rgb_dir, int comp, int write_alpha, int expand_mono, unsigned char *d) {
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   int k;
   if (write_alpha < 0) stbiw__write1(s, d[comp - 1]);
   switch (comp) {
      case 2: // 2 pixels = mono + alpha, alpha is written separately, so same as 1-channel case
      case 1:
         if (expand_mono)
            stbiw__write3(s, d[0], d[0], d[0]); // monochrome bmp
         else
            stbiw__write1(s, d[0]);  // monochrome TGA
         break;
      case 4:
         if (!write_alpha) {
            // composite against pink background
            for (k = 0; k < 3; ++k)
               px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
            stbiw__write3(s, px[1 - rgb_dir], px[1], px[1 + rgb_dir]);
            break;
         }
         /* FALLTHROUGH */
      case 3:
         stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
         break;
   }
   if (write_alpha > 0)
      stbiw__write1(s, d[comp - 1]);
}

static void stbiw__write_pixels(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono) {
   uint32_t zero = 0;
   int i,j, j_end;
   if (y <= 0) return;

   vdir *= -1;
   if (vdir < 0) {
      j_end = -1; j = y-1;
   } else {
      j_end =  y; j = 0;
   }

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         stbiw__write_pixel(s, rgb_dir, comp, write_alpha, expand_mono, d);
      }
      stbiw__write_flush(s);
      s->func(s->context, &zero, scanline_pad);
   }
}

static int stbiw__outfile(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...) {
   if (y < 0 || x < 0) return 0;

   va_list v;
   va_start(v, fmt);
   stbiw__writefv(s, fmt, v);
   va_end(v);
   stbiw__write_pixels(s,rgb_dir,vdir,x,y,comp,data,alpha,pad, expand_mono);
   return 1;
}

static int stbi_write_bmp_core(stbi__write_context *s, int x, int y, int comp, const void *data) {
   if (comp != 4) {
      // write RGB bitmap
      int pad = (-x*3) & 3;
      return stbiw__outfile(s,-1,-1,x,y,comp,1,(void *) data,0,pad,
              "11 4 22 4" "4 44 22 444444",
              'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
               40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
   } else {
      // RGBA bitmaps need a v4 header
      // use BI_BITFIELDS mode with 32bpp and alpha mask
      // (straight BI_RGB with alpha mask doesn't work in most readers)
      return stbiw__outfile(s,-1,-1,x,y,comp,1,(void *)data,1,0,
         "11 4 22 4" "4 44 22 444444 4444 4 444 444 444 444",
         'B', 'M', 14+108+x*y*4, 0, 0, 14+108, // file header
         108, x,y, 1,32, 3,0,0,0,0,0, 0xff0000,0xff00,0xff,0xff000000u, 0, 0,0,0, 0,0,0, 0,0,0, 0,0,0); // bitmap V4 header
   }
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data) {
   stbi__write_context s = { 0 };
   FILE *f = fopen(filename, "wb");
   stbi__start_write_callbacks(&s, stbi__stdio_write, (void *)f);
   if (f != NULL) {
      int r = stbi_write_bmp_core(&s, x, y, comp, data);
      fclose(f);
      return r;
   } else return 0;
}
#endif //!STBI_WRITE_NO_STDIO

#endif // STB_IMAGE_WRITE_IMPLEMENTATION
