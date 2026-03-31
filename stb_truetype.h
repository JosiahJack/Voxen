// stb_truetype.h - Font Load System
#pragma once
#define STBTT_DEF extern
#define STBTT_malloc(size)  OS_AllocateRAM(NULL, (size), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, (OsFileHandle)-1)
#define STBTT_free(ptr)     OS_DeallocateRAM((ptr),0)
typedef struct stbtt_pack_context stbtt_pack_context;
typedef struct stbtt_fontinfo stbtt_fontinfo;
typedef struct stbrp_rect stbrp_rect;
STBTT_DEF int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int width, int height, int stride_in_bytes, int padding, void *alloc_context);
typedef struct {float font_size; int first_unicode_codepoint_in_range; int* array_of_unicode_codepoints; int num_chars; stbtt_packedchar *chardata_for_range; unsigned char h_oversample, v_oversample;} stbtt_pack_range;
struct stbtt_pack_context { void* user_allocator_context; void* pack_info; int width,height,stride_in_bytes,padding,skip_missing; unsigned int h_oversample,v_oversample; unsigned char *pixels; };
struct stbtt_fontinfo { void* userdata; unsigned char  * data; int fontstart,numGlyphs,loca,head,glyf,hhea,hmtx,kern,gpos,svg,index_map,indexToLocFormat; stbtt__buf cff,charstrings,gsubrs,subrs,fontdicts,fdselect; };
typedef struct stbtt_kerningentry { int glyph1,glyph2,advance;} stbtt_kerningentry;
enum {STBTT_vmove=1,STBTT_vline,STBTT_vcurve,STBTT_vcubic};
#define stbtt_vertex_type short // can't use int16_t because that's not visible in the header file
typedef struct { stbtt_vertex_type x,y,cx,cy,cx1,cy1; unsigned char type,padding; } stbtt_vertex;
STBTT_DEF int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **vertices);
STBTT_DEF unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);
STBTT_DEF void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);
STBTT_DEF void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
STBTT_DEF void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);
typedef struct { int w,h,stride; unsigned char *pixels; } stbtt__bitmap;
STBTT_DEF void stbtt_Rasterize(stbtt__bitmap *result, float flatness_in_pixels, stbtt_vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert);
enum { STBTT_PLATFORM_ID_UNICODE=0, STBTT_PLATFORM_ID_MAC=1, STBTT_PLATFORM_ID_ISO=2, STBTT_PLATFORM_ID_MICROSOFT=3};
enum { STBTT_UNICODE_EID_UNICODE_1_0=0, STBTT_UNICODE_EID_UNICODE_1_1=1, STBTT_UNICODE_EID_ISO_10646=2, STBTT_UNICODE_EID_UNICODE_2_0_BMP=3, STBTT_UNICODE_EID_UNICODE_2_0_FULL=4 };
enum { STBTT_MS_EID_SYMBOL=0, STBTT_MS_EID_UNICODE_BMP=1, STBTT_MS_EID_SHIFTJIS=2, STBTT_MS_EID_UNICODE_FULL=10 };
#define STBTT_MAX_OVERSAMPLE 8
static uint8_t stbtt__buf_get8(stbtt__buf *b) {
   if (b->cursor >= b->size) return 0;
   return b->data[b->cursor++];
}

static uint8_t stbtt__buf_peek8(stbtt__buf *b) {
   if (b->cursor >= b->size) return 0;
   return b->data[b->cursor];
}

static void stbtt__buf_seek(stbtt__buf *b, int o) { b->cursor = (o > b->size || o < 0) ? b->size : o; }
static void stbtt__buf_skip(stbtt__buf *b, int o) { stbtt__buf_seek(b, b->cursor + o); }

static uint32_t stbtt__buf_get(stbtt__buf *b, int n) {
   uint32_t v = 0;
   int i;
   for (i = 0; i < n; i++) v = (v << 8) | stbtt__buf_get8(b);
   return v;
}

static stbtt__buf stbtt__new_buf(const void *p, size_t size) {
   stbtt__buf r;
   r.data = (uint8_t*) p;
   r.size = (int) size;
   r.cursor = 0;
   return r;
}

#define stbtt__buf_get16(b)  stbtt__buf_get((b), 2)
#define stbtt__buf_get32(b)  stbtt__buf_get((b), 4)
static stbtt__buf stbtt__buf_range(const stbtt__buf *b, int o, int s) {
   stbtt__buf r = stbtt__new_buf(NULL, 0);
   if (o < 0 || s < 0 || o > b->size || s > b->size - o) return r;
   r.data = b->data + o;
   r.size = s;
   return r;
}

static stbtt__buf stbtt__cff_get_index(stbtt__buf *b) {
   int count, start, offsize;
   start = b->cursor;
   count = stbtt__buf_get16(b);
   if (count) {
      offsize = stbtt__buf_get8(b);
      stbtt__buf_skip(b, offsize * count);
      stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
   }
   return stbtt__buf_range(b, start, b->cursor - start);
}

static uint32_t stbtt__cff_int(stbtt__buf *b) {
   int b0 = stbtt__buf_get8(b);
   if (b0 >= 32 && b0 <= 246)       return b0 - 139;
   else if (b0 >= 247 && b0 <= 250) return (b0 - 247)*256 + stbtt__buf_get8(b) + 108;
   else if (b0 >= 251 && b0 <= 254) return -(b0 - 251)*256 - stbtt__buf_get8(b) - 108;
   else if (b0 == 28)               return stbtt__buf_get16(b);
   else if (b0 == 29)               return stbtt__buf_get32(b);
   return 0;
}

static void stbtt__cff_skip_operand(stbtt__buf *b) {
   int v, b0 = stbtt__buf_peek8(b);
   if (b0 == 30) {
      stbtt__buf_skip(b, 1);
      while (b->cursor < b->size) {
         v = stbtt__buf_get8(b);
         if ((v & 0xF) == 0xF || (v >> 4) == 0xF) break;
      }
   } else stbtt__cff_int(b);
}

static stbtt__buf stbtt__dict_get(stbtt__buf *b, int key) {
   stbtt__buf_seek(b, 0);
   while (b->cursor < b->size) {
      int start = b->cursor, end, op;
      while (stbtt__buf_peek8(b) >= 28)
         stbtt__cff_skip_operand(b);
      end = b->cursor;
      op = stbtt__buf_get8(b);
      if (op == 12)  op = stbtt__buf_get8(b) | 0x100;
      if (op == key) return stbtt__buf_range(b, start, end-start);
   }
   return stbtt__buf_range(b, 0, 0);
}

static void stbtt__dict_get_ints(stbtt__buf *b, int key, int outcount, uint32_t *out) { stbtt__buf operands = stbtt__dict_get(b, key); for (int i=0;i<outcount && operands.cursor<operands.size;++i) { out[i] = stbtt__cff_int(&operands); }}
static int stbtt__cff_index_count(stbtt__buf *b) { stbtt__buf_seek(b,0); return stbtt__buf_get16(b); }
static stbtt__buf stbtt__cff_index_get(stbtt__buf b, int i) {
   int count, offsize, start, end;
   stbtt__buf_seek(&b, 0);
   count = stbtt__buf_get16(&b);
   offsize = stbtt__buf_get8(&b);
   stbtt__buf_skip(&b, i*offsize);
   start = stbtt__buf_get(&b, offsize);
   end = stbtt__buf_get(&b, offsize);
   return stbtt__buf_range(&b, 2+(count+1)*offsize+start, end - start);
}

#define ttBYTE(p)     (* (uint8_t *) (p))
#define ttCHAR(p)     (* (int8_t *) (p))
#define ttFixed(p)    ttLONG(p)
static uint16_t ttUSHORT(uint8_t *p) { return p[0]*256 + p[1]; }
static int16_t ttSHORT(uint8_t *p)   { return p[0]*256 + p[1]; }
static uint32_t ttULONG(uint8_t *p)  { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
static int32_t ttLONG(uint8_t *p)    { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
#define stbtt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str)           stbtt_tag4(p,str[0],str[1],str[2],str[3])
static int stbtt__isfont(uint8_t *font) {
   // check the version number
   if (stbtt_tag4(font, '1',0,0,0))  return 1; // TrueType 1
   if (stbtt_tag(font, "typ1"))   return 1; // TrueType with type 1 font -- we don't support this!
   if (stbtt_tag(font, "OTTO"))   return 1; // OpenType with CFF
   if (stbtt_tag4(font, 0,1,0,0)) return 1; // OpenType 1.0
   if (stbtt_tag(font, "true"))   return 1; // Apple specification for TrueType fonts
   return 0;
}

// @OPTIMIZE: binary search
static uint32_t stbtt__find_table(uint8_t *data, uint32_t fontstart, const char *tag)
{
   int32_t num_tables = ttUSHORT(data+fontstart+4);
   uint32_t tabledir = fontstart + 12;
   int32_t i;
   for (i=0; i < num_tables; ++i) {
      uint32_t loc = tabledir + 16*i;
      if (stbtt_tag(data+loc+0, tag))
         return ttULONG(data+loc+8);
   }
   return 0;
}

static int stbtt_GetFontOffsetForIndex_internal(unsigned char *font_collection, int index) {
   if (stbtt__isfont(font_collection)) return index == 0 ? 0 : -1; // if it's just a font, there's only one valid index

   // check if it's a TTC
   if (stbtt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         int32_t n = ttLONG(font_collection+8);
         if (index >= n)
            return -1;
         return ttULONG(font_collection+12+index*4);
      }
   }
   return -1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
__attribute__((pure)) STBTT_DEF int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index) { return stbtt_GetFontOffsetForIndex_internal((unsigned char*)data,index); }
#pragma GCC diagnostic pop

static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict) {
   uint32_t subrsoff = 0, private_loc[2] = { 0, 0 };
   stbtt__buf pdict;
   stbtt__dict_get_ints(&fontdict, 18, 2, private_loc);
   if (!private_loc[1] || !private_loc[0]) return stbtt__new_buf(NULL, 0);
   pdict = stbtt__buf_range(&cff, private_loc[1], private_loc[0]);
   stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);
   if (!subrsoff) return stbtt__new_buf(NULL, 0);
   stbtt__buf_seek(&cff, private_loc[1]+subrsoff);
   return stbtt__cff_get_index(&cff);
}

static int stbtt_InitFont_internal(stbtt_fontinfo *info, unsigned char *data, int fontstart) {
   uint32_t cmap, t;
   int32_t i,numTables;
   info->data = data;
   info->fontstart = fontstart;
   info->cff = stbtt__new_buf(NULL, 0);
   cmap = stbtt__find_table(data, fontstart, "cmap");       // required
   info->loca = stbtt__find_table(data, fontstart, "loca"); // required
   info->head = stbtt__find_table(data, fontstart, "head"); // required
   info->glyf = stbtt__find_table(data, fontstart, "glyf"); // required
   info->hhea = stbtt__find_table(data, fontstart, "hhea"); // required
   info->hmtx = stbtt__find_table(data, fontstart, "hmtx"); // required
   info->kern = stbtt__find_table(data, fontstart, "kern"); // not required
   info->gpos = stbtt__find_table(data, fontstart, "GPOS"); // not required
   if (!cmap || !info->head || !info->hhea || !info->hmtx)
      return 0;
   if (info->glyf) {
      // required for truetype
      if (!info->loca) return 0;
   } else {
      // initialization for CFF / Type2 fonts (OTF)
      stbtt__buf b, topdict, topdictidx;
      uint32_t cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
      uint32_t cff;
      cff = stbtt__find_table(data, fontstart, "CFF ");
      if (!cff) return 0;

      info->fontdicts = stbtt__new_buf(NULL, 0);
      info->fdselect = stbtt__new_buf(NULL, 0);
      info->cff = stbtt__new_buf(data+cff,512*1024*1024); // @TODO this should use size from table (not 512MB)
      b = info->cff;
      stbtt__buf_skip(&b,2); // read the header
      stbtt__buf_seek(&b,stbtt__buf_get8(&b)); // hdrsize
      stbtt__cff_get_index(&b);  // name INDEX @TODO the name INDEX could list multiple fonts, but we just use the first one.
      topdictidx = stbtt__cff_get_index(&b);
      topdict = stbtt__cff_index_get(topdictidx, 0);
      stbtt__cff_get_index(&b);  // string INDEX
      info->gsubrs = stbtt__cff_get_index(&b);
      stbtt__dict_get_ints(&topdict, 17, 1, &charstrings);
      stbtt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
      stbtt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
      stbtt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
      info->subrs = stbtt__get_subrs(b, topdict);
      if (cstype != 2) return 0; // we only support Type 2 charstrings
      if (charstrings == 0) return 0;

      if (fdarrayoff) {
         // looks like a CID font
         if (!fdselectoff) return 0;
         stbtt__buf_seek(&b, fdarrayoff);
         info->fontdicts = stbtt__cff_get_index(&b);
         info->fdselect = stbtt__buf_range(&b, fdselectoff, b.size-fdselectoff);
      }

      stbtt__buf_seek(&b, charstrings);
      info->charstrings = stbtt__cff_get_index(&b);
   }

   t = stbtt__find_table(data, fontstart, "maxp");
   if (t) info->numGlyphs = ttUSHORT(data+t+4);
   else info->numGlyphs = 0xffff;

   info->svg = -1; // find a cmap encoding table we understand *now* to avoid searching later. (todo: could make this installable) the same regardless of glyph.
   numTables = ttUSHORT(data + cmap + 2);
   info->index_map = 0;
   for (i=0; i < numTables; ++i) {
      uint32_t encoding_record = cmap + 4 + 8 * i;
      // find an encoding we understand:
      switch(ttUSHORT(data+encoding_record)) {
         case STBTT_PLATFORM_ID_MICROSOFT:
            switch (ttUSHORT(data+encoding_record+2)) {
               case STBTT_MS_EID_UNICODE_BMP:
               case STBTT_MS_EID_UNICODE_FULL:
                  // MS/Unicode
                  info->index_map = cmap + ttULONG(data+encoding_record+4);
                  break;
            }
            break;
        case STBTT_PLATFORM_ID_UNICODE:
            // Mac/iOS has these
            // all the encodingIDs are unicode, so we don't bother to check it
            info->index_map = cmap + ttULONG(data+encoding_record+4);
            break;
      }
   }
   if (info->index_map == 0) return 0;

   info->indexToLocFormat = ttUSHORT(data+info->head + 50);
   return 1;
}

__attribute__((pure)) STBTT_DEF int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint) {
   uint8_t *data=info->data; uint32_t index_map=info->index_map; uint16_t format=ttUSHORT(data + index_map + 0);
   if (format == 0) { // apple byte encoding
      int32_t bytes = ttUSHORT(data + index_map + 2);
      if (unicode_codepoint < bytes-6) return ttBYTE(data + index_map + 6 + unicode_codepoint);
      return 0;
   } else if (format == 6) {
      uint32_t first = ttUSHORT(data + index_map + 6);
      uint32_t count = ttUSHORT(data + index_map + 8);
      if ((uint32_t) unicode_codepoint >= first && (uint32_t) unicode_codepoint < first+count)
         return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first)*2);
      return 0;
   } else if (format == 2) { // @TODO: high-byte mapping for japanese/chinese/korean
      return 0;
   } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
      uint16_t segcount = ttUSHORT(data+index_map+6) >> 1;
      uint16_t searchRange = ttUSHORT(data+index_map+8) >> 1;
      uint16_t entrySelector = ttUSHORT(data+index_map+10);
      uint16_t rangeShift = ttUSHORT(data+index_map+12) >> 1;
      uint32_t endCount = index_map + 14; // do a binary search of the segments
      uint32_t search = endCount;
      if (unicode_codepoint > 0xffff) return 0;

      if (unicode_codepoint >= ttUSHORT(data + search + rangeShift*2)) search += rangeShift*2; // they lie from endCount .. endCount + segCount but searchRange is the nearest power of two, so...
      search -= 2; // now decrement to bias correctly to find smallest
      while (entrySelector) {
         uint16_t end;
         searchRange >>= 1;
         end = ttUSHORT(data + search + searchRange*2);
         if (unicode_codepoint > end)
            search += searchRange*2;
         --entrySelector;
      }
      search += 2;

      {
         uint16_t offset, start, last;
         uint16_t item = (uint16_t) ((search - endCount) >> 1);
         start = ttUSHORT(data + index_map + 14 + segcount*2 + 2 + 2*item);
         last = ttUSHORT(data + endCount + 2*item);
         if (unicode_codepoint < start || unicode_codepoint > last) return 0;

         offset = ttUSHORT(data + index_map + 14 + segcount*6 + 2 + 2*item);
         if (offset == 0) return (uint16_t) (unicode_codepoint + ttSHORT(data + index_map + 14 + segcount*4 + 2 + 2*item));
         return ttUSHORT(data + offset + (unicode_codepoint-start)*2 + index_map + 14 + segcount*6 + 2 + 2*item);
      }
   } else if (format == 12 || format == 13) {
      uint32_t ngroups = ttULONG(data+index_map+12);
      int32_t low,high;
      low = 0; high = (int32_t)ngroups;
      // Binary search the right group.
      while (low < high) {
         int32_t mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
         uint32_t start_char = ttULONG(data+index_map+16+mid*12);
         uint32_t end_char = ttULONG(data+index_map+16+mid*12+4);
         if ((uint32_t) unicode_codepoint < start_char)
            high = mid;
         else if ((uint32_t) unicode_codepoint > end_char)
            low = mid+1;
         else {
            uint32_t start_glyph = ttULONG(data+index_map+16+mid*12+8);
            if (format == 12)
               return start_glyph + unicode_codepoint-start_char;
            else // format == 13
               return start_glyph;
         }
      }
      return 0; // not found
   }

   return 0;
}

static void stbtt_setvertex(stbtt_vertex *v, uint8_t type, int32_t x, int32_t y, int32_t cx, int32_t cy) { v->type = type; v->x = (int16_t) x; v->y = (int16_t) y; v->cx = (int16_t) cx; v->cy = (int16_t) cy; }
static int stbtt__GetGlyfOffset(const stbtt_fontinfo *info, int glyph_index) {
   if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
   if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

   int g1,g2;
   if (info->indexToLocFormat == 0) {
      g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
      g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
   } else {
      g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4);
      g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4);
   }

   return g1==g2 ? -1 : g1; // if length is 0, return -1
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);
STBTT_DEF int stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1) {
   if (info->cff.size) {
      stbtt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
   } else {
      int g = stbtt__GetGlyfOffset(info, glyph_index);
      if (g < 0) return 0;

      if (x0) *x0 = ttSHORT(info->data + g + 2);
      if (y0) *y0 = ttSHORT(info->data + g + 4);
      if (x1) *x1 = ttSHORT(info->data + g + 6);
      if (y1) *y1 = ttSHORT(info->data + g + 8);
   }
   return 1;
}

static int stbtt__close_shape(stbtt_vertex *vertices, int num_vertices, int was_off, int start_off, int32_t sx, int32_t sy, int32_t scx, int32_t scy, int32_t cx, int32_t cy) {
   if (start_off) {
      if (was_off)
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+scx)>>1, (cy+scy)>>1, cx,cy);
      stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx,sy,scx,scy);
   } else {
      if (was_off)
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve,sx,sy,cx,cy);
      else
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vline,sx,sy,0,0);
   }
   return num_vertices;
}

static int stbtt__GetGlyphShapeTT(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices) {
   int16_t numberOfContours; uint8_t *endPtsOfContours; uint8_t *data = info->data;
   stbtt_vertex *vertices=0;
   int num_vertices=0; int g = stbtt__GetGlyfOffset(info, glyph_index); *pvertices = NULL;
   if (g < 0) return 0;

   numberOfContours = ttSHORT(data + g);
   if (numberOfContours > 0) {
      uint8_t flags=0,flagcount;
      int32_t ins, i,j=0,m,n, next_move, was_off=0, off, start_off=0;
      int32_t x,y,cx,cy,sx,sy, scx,scy;
      uint8_t *points;
      endPtsOfContours = (data + g + 10);
      ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
      points = data + g + 10 + numberOfContours * 2 + 2 + ins;
      n = 1+ttUSHORT(endPtsOfContours + numberOfContours*2-2);
      m = n + 2*numberOfContours;  // a loose bound on how many vertices we might need
      vertices = (stbtt_vertex*)STBTT_malloc(m * sizeof(vertices[0]));
      if (vertices == 0) return 0;

      next_move = 0;
      flagcount=0;
      off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated
      for (i=0; i < n; ++i) {
         if (flagcount == 0) {
            flags = *points++;
            if (flags & 8)
               flagcount = *points++;
         } else
            --flagcount;
         vertices[off+i].type = flags;
      }

      // now load x coordinates
      x=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 2) {
            int16_t dx = *points++;
            x += (flags & 16) ? dx : -dx; // ???
         } else {
            if (!(flags & 16)) {
               x = x + (int16_t) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].x = (int16_t) x;
      }

      // now load y coordinates
      y=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 4) {
            int16_t dy = *points++;
            y += (flags & 32) ? dy : -dy; // ???
         } else {
            if (!(flags & 32)) {
               y = y + (int16_t) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].y = (int16_t) y;
      }

      // now convert them to our format
      num_vertices=0;
      sx = sy = cx = cy = scx = scy = 0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         x     = (int16_t) vertices[off+i].x;
         y     = (int16_t) vertices[off+i].y;

         if (next_move == i) {
            if (i != 0)
               num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);

            // now start the new one
            start_off = !(flags & 1);
            if (start_off) {
               // if we start off with an off-curve point, then when we need to find a point on the curve
               // where we can start, and we need to save some state for when we wraparound.
               scx = x;
               scy = y;
               if (!(vertices[off+i+1].type & 1)) {
                  // next point is also a curve point, so interpolate an on-point curve
                  sx = (x + (int32_t) vertices[off+i+1].x) >> 1;
                  sy = (y + (int32_t) vertices[off+i+1].y) >> 1;
               } else {
                  // otherwise just use the next point as our start point
                  sx = (int32_t) vertices[off+i+1].x;
                  sy = (int32_t) vertices[off+i+1].y;
                  ++i; // we're using point i+1 as the starting point, so skip it
               }
            } else {
               sx = x;
               sy = y;
            }
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove,sx,sy,0,0);
            was_off = 0;
            next_move = 1 + ttUSHORT(endPtsOfContours+j*2);
            ++j;
         } else {
            if (!(flags & 1)) { // if it's a curve
               if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
               cx = x;
               cy = y;
               was_off = 1;
            } else {
               if (was_off)
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x,y, cx, cy);
               else
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x,y,0,0);
               was_off = 0;
            }
         }
      }
      num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
   } else if (numberOfContours < 0) {
      // Compound shapes.
      int more = 1;
      uint8_t *comp = data + g + 10;
      num_vertices = 0;
      vertices = 0;
      while (more) {
         uint16_t flags, gidx;
         int comp_num_verts = 0, i;
         stbtt_vertex *comp_verts = 0, *tmp = 0;
         float mtx[6] = {1,0,0,1,0,0}, m, n;
         flags = ttSHORT(comp); comp+=2;
         gidx = ttSHORT(comp); comp+=2;
         if (flags & 2) { // XY values
            if (flags & 1) { // shorts
               mtx[4] = ttSHORT(comp); comp+=2;
               mtx[5] = ttSHORT(comp); comp+=2;
            } else {
               mtx[4] = ttCHAR(comp); comp+=1;
               mtx[5] = ttCHAR(comp); comp+=1;
            }
         } //else { @TODO handle matching point

         if (flags & (1<<3)) { // WE_HAVE_A_SCALE
            mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
         } else if (flags & (1<<6)) { // WE_HAVE_AN_X_AND_YSCALE
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         } else if (flags & (1<<7)) { // WE_HAVE_A_TWO_BY_TWO
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[2] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         }

         // Find transformation scales.
         m = vsqrtf(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
         n = vsqrtf(mtx[2]*mtx[2] + mtx[3]*mtx[3]);

         // Get indexed glyph.
         comp_num_verts = stbtt_GetGlyphShape(info, gidx, &comp_verts);
         if (comp_num_verts > 0) {
            // Transform vertices.
            for (i = 0; i < comp_num_verts; ++i) {
               stbtt_vertex* v = &comp_verts[i];
               stbtt_vertex_type x,y;
               x=v->x; y=v->y;
               v->x = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->y = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
               x=v->cx; y=v->cy;
               v->cx = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->cy = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
            }
            // Append vertices.
            tmp = (stbtt_vertex*)STBTT_malloc((num_vertices+comp_num_verts)*sizeof(stbtt_vertex));
            if (!tmp) {
               if (vertices) STBTT_free(vertices);
               if (comp_verts) STBTT_free(comp_verts);
               return 0;
            }
            if (num_vertices > 0 && vertices) __builtin_memcpy(tmp, vertices, num_vertices*sizeof(stbtt_vertex));
            __builtin_memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(stbtt_vertex));
            if (vertices) STBTT_free(vertices);
            vertices = tmp;
            STBTT_free(comp_verts);
            num_vertices += comp_num_verts;
         }
         // More components ?
         more = flags & (1<<5);
      }
   } else {
      // numberOfCounters == 0, do nothing
   }

   *pvertices = vertices;
   return num_vertices;
}

typedef struct {int bounds,started; float first_x,first_y,x,y; int32_t min_x,max_x,min_y,max_y; stbtt_vertex *pvertices; int num_vertices; } stbtt__csctx;

#define STBTT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}
static void stbtt__track_vertex(stbtt__csctx *c, int32_t x, int32_t y) {
   if (x > c->max_x || !c->started) c->max_x = x;
   if (y > c->max_y || !c->started) c->max_y = y;
   if (x < c->min_x || !c->started) c->min_x = x;
   if (y < c->min_y || !c->started) c->min_y = y;
   c->started = 1;
}

static void stbtt__csctx_v(stbtt__csctx *c, uint8_t type, int32_t x, int32_t y, int32_t cx, int32_t cy, int32_t cx1, int32_t cy1) {
   if (c->bounds) {
      stbtt__track_vertex(c, x, y);
      if (type == STBTT_vcubic) {
         stbtt__track_vertex(c, cx, cy);
         stbtt__track_vertex(c, cx1, cy1);
      }
   } else {
      stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
      c->pvertices[c->num_vertices].cx1 = (int16_t) cx1;
      c->pvertices[c->num_vertices].cy1 = (int16_t) cy1;
   }
   c->num_vertices++;
}

static void stbtt__csctx_close_shape(stbtt__csctx *ctx) {
   if (ctx->first_x != ctx->x || ctx->first_y != ctx->y) stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void stbtt__csctx_rmove_to(stbtt__csctx *ctx, float dx, float dy) {
   stbtt__csctx_close_shape(ctx);
   ctx->first_x = ctx->x = ctx->x + dx;
   ctx->first_y = ctx->y = ctx->y + dy;
   stbtt__csctx_v(ctx, STBTT_vmove, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rline_to(stbtt__csctx *ctx, float dx, float dy) { ctx->x += dx; ctx->y += dy; stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0); }
static void stbtt__csctx_rccurve_to(stbtt__csctx *ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3) {
   float cx1 = ctx->x + dx1;
   float cy1 = ctx->y + dy1;
   float cx2 = cx1 + dx2;
   float cy2 = cy1 + dy2;
   ctx->x = cx2 + dx3;
   ctx->y = cy2 + dy3;
   stbtt__csctx_v(ctx, STBTT_vcubic, (int)ctx->x, (int)ctx->y, (int)cx1, (int)cy1, (int)cx2, (int)cy2);
}

static stbtt__buf stbtt__get_subr(stbtt__buf idx, int n) {
   int count = stbtt__cff_index_count(&idx);
   int bias = 107;
   if (count >= 33900)
      bias = 32768;
   else if (count >= 1240)
      bias = 1131;
   n += bias;
   if (n < 0 || n >= count)
      return stbtt__new_buf(NULL, 0);
   return stbtt__cff_index_get(idx, n);
}

static stbtt__buf stbtt__cid_get_glyph_subrs(const stbtt_fontinfo *info, int glyph_index) {
   stbtt__buf fdselect = info->fdselect;
   int nranges, start, end, v, fmt, fdselector = -1, i;
   stbtt__buf_seek(&fdselect, 0);
   fmt = stbtt__buf_get8(&fdselect);
   if (fmt == 0) {
      // untested
      stbtt__buf_skip(&fdselect, glyph_index);
      fdselector = stbtt__buf_get8(&fdselect);
   } else if (fmt == 3) {
      nranges = stbtt__buf_get16(&fdselect);
      start = stbtt__buf_get16(&fdselect);
      for (i = 0; i < nranges; i++) {
         v = stbtt__buf_get8(&fdselect);
         end = stbtt__buf_get16(&fdselect);
         if (glyph_index >= start && glyph_index < end) { fdselector = v; break; }
         
         start = end;
      }
   }
   if (fdselector == -1) stbtt__new_buf(NULL, 0);
   return stbtt__get_subrs(info->cff, stbtt__cff_index_get(info->fontdicts, fdselector));
}

static int stbtt__run_charstring(const stbtt_fontinfo *info, int glyph_index, stbtt__csctx *c) {
   int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
   int has_subrs = 0, clear_stack;
   float s[48];
   stbtt__buf subr_stack[10], subrs = info->subrs, b; float f;
#define STBTT__CSERR(s) (0)
   // this currently ignores the initial width value, which isn't needed if we have hmtx
   b = stbtt__cff_index_get(info->charstrings, glyph_index);
   while (b.cursor < b.size) {
      i = 0;
      clear_stack = 1;
      b0 = stbtt__buf_get8(&b);
      switch (b0) {
      // @TODO implement hinting
      case 0x13: // hintmask
      case 0x14: // cntrmask
         if (in_header) maskbits += (sp / 2); // implicit "vstem"
         in_header = 0;
         stbtt__buf_skip(&b, (maskbits + 7) / 8);
         break;
      case 0x01: // hstem
      case 0x03: // vstem
      case 0x12: // hstemhm
      case 0x17: // vstemhm
         maskbits += (sp / 2);
         break;
      case 0x15: // rmoveto
         in_header = 0;
         if (sp < 2) return STBTT__CSERR("rmoveto stack");
         stbtt__csctx_rmove_to(c, s[sp-2], s[sp-1]);
         break;
      case 0x04: // vmoveto
         in_header = 0;
         if (sp < 1) return STBTT__CSERR("vmoveto stack");
         stbtt__csctx_rmove_to(c, 0, s[sp-1]);
         break;
      case 0x16: // hmoveto
         in_header = 0;
         if (sp < 1) return STBTT__CSERR("hmoveto stack");
         stbtt__csctx_rmove_to(c, s[sp-1], 0);
         break;
      case 0x05: // rlineto
         if (sp < 2) return STBTT__CSERR("rlineto stack");
         for (; i + 1 < sp; i += 2) stbtt__csctx_rline_to(c, s[i], s[i+1]);
         break;
      // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
      // starting from a different place.
      case 0x07: // vlineto
         if (sp < 1) return STBTT__CSERR("vlineto stack");
         goto vlineto;
      case 0x06: // hlineto
         if (sp < 1) return STBTT__CSERR("hlineto stack");
         for (;;) {
            if (i >= sp) break;
            stbtt__csctx_rline_to(c, s[i], 0);
            i++;
      vlineto:
            if (i >= sp) break;
            stbtt__csctx_rline_to(c, 0, s[i]);
            i++;
         }
         break;
      case 0x1F: // hvcurveto
         if (sp < 4) return STBTT__CSERR("hvcurveto stack");
         goto hvcurveto;
      case 0x1E: // vhcurveto
         if (sp < 4) return STBTT__CSERR("vhcurveto stack");
         for (;;) {
            if (i + 3 >= sp) break;
            stbtt__csctx_rccurve_to(c, 0, s[i], s[i+1], s[i+2], s[i+3], (sp - i == 5) ? s[i + 4] : 0.0f);
            i += 4;
      hvcurveto:
            if (i + 3 >= sp) break;
            stbtt__csctx_rccurve_to(c, s[i], 0, s[i+1], s[i+2], (sp - i == 5) ? s[i+4] : 0.0f, s[i+3]);
            i += 4;
         }
         break;
      case 0x08: // rrcurveto
         if (sp < 6) return STBTT__CSERR("rcurveline stack");
         for (; i + 5 < sp; i += 6) stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;
      case 0x18: // rcurveline
         if (sp < 8) return STBTT__CSERR("rcurveline stack");
         for (; i + 5 < sp - 2; i += 6) stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         if (i + 1 >= sp) return STBTT__CSERR("rcurveline stack");
         stbtt__csctx_rline_to(c, s[i], s[i+1]);
         break;
      case 0x19: // rlinecurve
         if (sp < 8) return STBTT__CSERR("rlinecurve stack");
         for (; i + 1 < sp - 6; i += 2) stbtt__csctx_rline_to(c, s[i], s[i+1]);
         if (i + 5 >= sp) return STBTT__CSERR("rlinecurve stack");
         stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;
      case 0x1A: // vvcurveto
      case 0x1B: // hhcurveto
         if (sp < 4) return STBTT__CSERR("(vv|hh)curveto stack");
         f = 0.0;
         if (sp & 1) { f = s[i]; i++; }
         for (; i + 3 < sp; i += 4) {
            if (b0 == 0x1B) stbtt__csctx_rccurve_to(c, s[i], f, s[i+1], s[i+2], s[i+3], 0.0);
            else stbtt__csctx_rccurve_to(c, f, s[i], s[i+1], s[i+2], 0.0, s[i+3]);

            f = 0.0;
         }
         break;
      case 0x0A: // callsubr
         if (!has_subrs) {
            if (info->fdselect.size) subrs = stbtt__cid_get_glyph_subrs(info, glyph_index);
            has_subrs = 1;
         }
         // FALLTHROUGH
      case 0x1D: // callgsubr
         if (sp < 1) return STBTT__CSERR("call(g|)subr stack");
         v = (int) s[--sp];
         if (subr_stack_height >= 10) return STBTT__CSERR("recursion limit");
         subr_stack[subr_stack_height++] = b;
         b = stbtt__get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
         if (b.size == 0) return STBTT__CSERR("subr not found");
         b.cursor = 0;
         clear_stack = 0;
         break;
      case 0x0B: // return
         if (subr_stack_height <= 0) return STBTT__CSERR("return outside subr");
         b = subr_stack[--subr_stack_height];
         clear_stack = 0;
         break;
      case 0x0E: // endchar
         stbtt__csctx_close_shape(c);
         return 1;
      case 0x0C: { // two-byte escape
         float dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
         float dx, dy;
         int b1 = stbtt__buf_get8(&b);
         switch (b1) {
         // @TODO These "flex" implementations ignore the flex-depth and resolution,
         // and always draw beziers.
         case 0x22: // hflex
            if (sp < 7) return STBTT__CSERR("hflex stack");
            dx1 = s[0];
            dx2 = s[1]; dy2 = s[2];
            dx3 = s[3]; dx4 = s[4];
            dx5 = s[5]; dx6 = s[6];
            stbtt__csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
            stbtt__csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
            break;

         case 0x23: // flex
            if (sp < 13) return STBTT__CSERR("flex stack");
            dx1 = s[0]; dy1 = s[1];
            dx2 = s[2]; dy2 = s[3];
            dx3 = s[4]; dy3 = s[5];
            dx4 = s[6]; dy4 = s[7];
            dx5 = s[8]; dy5 = s[9];
            dx6 = s[10]; dy6 = s[11];
            //fd is s[12]
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         case 0x24: // hflex1
            if (sp < 9) return STBTT__CSERR("hflex1 stack");
            dx1 = s[0]; dy1 = s[1];
            dx2 = s[2]; dy2 = s[3];
            dx3 = s[4]; dx4 = s[5];
            dx5 = s[6]; dy5 = s[7];
            dx6 = s[8];
            stbtt__csctx_rccurve_to(c,dx1,dy1,dx2,dy2,dx3,0);
            stbtt__csctx_rccurve_to(c,dx4,0,dx5,dy5,dx6,-(dy1+dy2+dy5));
            break;

         case 0x25: // flex1
            if (sp < 11) return STBTT__CSERR("flex1 stack");
            dx1 = s[0]; dy1 = s[1];
            dx2 = s[2]; dy2 = s[3];
            dx3 = s[4]; dy3 = s[5];
            dx4 = s[6]; dy4 = s[7];
            dx5 = s[8]; dy5 = s[9];
            dx6 = dy6 = s[10];
            dx = dx1+dx2+dx3+dx4+dx5; dy = dy1+dy2+dy3+dy4+dy5;
            if (vabs(dx) > vabs(dy)) dy6 = -dy;
            else dx6 = -dx;

            stbtt__csctx_rccurve_to(c,dx1,dy1,dx2,dy2,dx3,dy3);
            stbtt__csctx_rccurve_to(c,dx4,dy4,dx5,dy5,dx6,dy6);
            break;

         default:
            return STBTT__CSERR("unimplemented");
         }
      } break;

      default:
         if (b0 != 255 && b0 != 28 && b0 < 32) return STBTT__CSERR("reserved operator");

         // push immediate
         if (b0 == 255) {
            f = (float)(int32_t)stbtt__buf_get32(&b) / 0x10000;
         } else {
            stbtt__buf_skip(&b, -1);
            f = (float)(int16_t)stbtt__cff_int(&b);
         }

         if (sp >= 48) return STBTT__CSERR("push stack overflow");
         s[sp++] = f;
         clear_stack = 0;
         break;
      }

      if (clear_stack) sp = 0;
   }
   return STBTT__CSERR("no endchar");

#undef STBTT__CSERR
}

static int stbtt__GetGlyphShapeT2(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices) {
   // runs the charstring twice, once to count and once to output (to avoid realloc)
   stbtt__csctx count_ctx = STBTT__CSCTX_INIT(1); stbtt__csctx output_ctx = STBTT__CSCTX_INIT(0);
   if (stbtt__run_charstring(info, glyph_index, &count_ctx)) {
      *pvertices = (stbtt_vertex*)STBTT_malloc(count_ctx.num_vertices*sizeof(stbtt_vertex));
      output_ctx.pvertices = *pvertices;
      if (stbtt__run_charstring(info, glyph_index, &output_ctx)) return output_ctx.num_vertices;
   }
   *pvertices = NULL;
   return 0;
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1) {
   stbtt__csctx c = STBTT__CSCTX_INIT(1);
   int r = stbtt__run_charstring(info,glyph_index,&c);
   if (x0) *x0 = r ? c.min_x : 0;
   if (y0) *y0 = r ? c.min_y : 0;
   if (x1) *x1 = r ? c.max_x : 0;
   if (y1) *y1 = r ? c.max_y : 0;
   return r ? c.num_vertices : 0;
}

STBTT_DEF int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices) {
   if (!info->cff.size) return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
   else                 return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

STBTT_DEF void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing) {
   uint16_t numOfLongHorMetrics = ttUSHORT(info->data+info->hhea + 34);
   if (glyph_index < numOfLongHorMetrics) {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*glyph_index);
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*glyph_index + 2);
   } else {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*(numOfLongHorMetrics-1));
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
   }
}

__attribute__((pure)) STBTT_DEF int stbtt_GetKerningTableLength(const stbtt_fontinfo *info) {
   uint8_t *data = info->data + info->kern;
   if (!info->kern) return 0; // we only look at the first table. it must be 'horizontal' and format 0.
   if (ttUSHORT(data+2) < 1) return 0; // number of tables, need at least 1
   if (ttUSHORT(data+8) != 1) return 0; // horizontal flag must be set in format
   return ttUSHORT(data+10);
}

STBTT_DEF int stbtt_GetKerningTable(const stbtt_fontinfo *info, stbtt_kerningentry* table, int table_length) {
   uint8_t *data = info->data + info->kern;
   int k, length;
   if (!info->kern) return 0; // we only look at the first table. it must be 'horizontal' and format 0.
   if (ttUSHORT(data+2) < 1) return 0; // number of tables, need at least 1
   if (ttUSHORT(data+8) != 1) return 0; // horizontal flag must be set in format
   
   length = ttUSHORT(data+10);
   if (table_length < length) length = table_length;
   for (k = 0; k < length; k++) {
      table[k].glyph1 = ttUSHORT(data+18+(k*6));
      table[k].glyph2 = ttUSHORT(data+20+(k*6));
      table[k].advance = ttSHORT(data+22+(k*6));
   }

   return length;
}

__attribute__((pure)) static int stbtt__GetGlyphKernInfoAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2) {
   if (!info->kern) return 0; // we only look at the first table. it must be 'horizontal' and format 0.

   uint8_t *data = info->data + info->kern;
   if (ttUSHORT(data+2) < 1) return 0; // number of tables, need at least 1
   if (ttUSHORT(data+8) != 1) return 0; // horizontal flag must be set in format

   int32_t l = 0; int32_t r = ttUSHORT(data+10) - 1; int32_t m;
   uint32_t needle = glyph1 << 16 | glyph2; uint32_t straw;
   while (l <= r) {
      m = (l + r) >> 1;
      straw = ttULONG(data+18+(m*6)); // note: unaligned read
      if (needle < straw) r = m - 1;
      else if (needle > straw) l = m + 1;
      else return ttSHORT(data+22+(m*6));
   }

   return 0;
}

__attribute__((pure)) static int32_t stbtt__GetCoverageIndex(uint8_t *coverageTable, int glyph) {
   uint16_t coverageFormat = ttUSHORT(coverageTable);
   switch (coverageFormat) {
      case 1: {
         uint16_t glyphCount = ttUSHORT(coverageTable + 2);
         int32_t l=0, r=glyphCount-1,m; int straw, needle=glyph;
         while (l <= r) { // Binary search.
            uint8_t *glyphArray = coverageTable + 4;
            uint16_t glyphID;
            m = (l + r) >> 1;
            glyphID = ttUSHORT(glyphArray + 2 * m);
            straw = glyphID;
            if (needle < straw)
               r = m - 1;
            else if (needle > straw)
               l = m + 1;
            else {
               return m;
            }
         }
         break;
      }
      case 2: {
         uint16_t rangeCount = ttUSHORT(coverageTable + 2);
         uint8_t *rangeArray = coverageTable + 4;
         int32_t l=0, r=rangeCount-1, m;
         int strawStart,strawEnd,needle=glyph;
         while (l <= r) { // Binary search.
            uint8_t *rangeRecord;
            m = (l + r) >> 1;
            rangeRecord = rangeArray + 6 * m;
            strawStart = ttUSHORT(rangeRecord);
            strawEnd = ttUSHORT(rangeRecord + 2);
            if (needle < strawStart) r = m - 1;
            else if (needle > strawEnd) l = m + 1;
            else { uint16_t startCoverageIndex = ttUSHORT(rangeRecord + 4); return startCoverageIndex + glyph - strawStart; }
         }
         break;
      }

      default: return -1; // unsupported
   }

   return -1;
}

__attribute__((pure)) static int32_t stbtt__GetGlyphClass(uint8_t *classDefTable, int glyph) {
   uint16_t classDefFormat = ttUSHORT(classDefTable);
   switch (classDefFormat) {
      case 1: {
         uint16_t startGlyphID = ttUSHORT(classDefTable + 2);
         uint16_t glyphCount = ttUSHORT(classDefTable + 4);
         uint8_t *classDef1ValueArray = classDefTable + 6;
         if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount) return (int32_t)ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID));
         break;
      }
      case 2: {
         uint16_t classRangeCount = ttUSHORT(classDefTable + 2);
         uint8_t *classRangeRecords = classDefTable + 4;
         int32_t l=0, r=classRangeCount-1, m;
         int strawStart, strawEnd, needle=glyph;
         while (l <= r) { // Binary search.
            uint8_t *classRangeRecord;
            m = (l + r) >> 1;
            classRangeRecord = classRangeRecords + 6 * m;
            strawStart = ttUSHORT(classRangeRecord);
            strawEnd = ttUSHORT(classRangeRecord + 2);
            if (needle < strawStart) r = m - 1;
            else if (needle > strawEnd) l = m + 1;
            else return (int32_t)ttUSHORT(classRangeRecord + 4);
         }
         break;
      }

      default: return -1; // Unsupported definition type, return an error.
   }

   return 0; // "All glyphs not assigned to a class fall into class 0". (OpenType spec)
}

__attribute__((pure)) static int32_t stbtt__GetGlyphGPOSInfoAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2) {
   uint16_t lookupListOffset,lookupCount; uint8_t *lookupList; uint8_t *data; int32_t i, sti;
   if (!info->gpos) return 0;

   data = info->data + info->gpos;
   if (ttUSHORT(data+0) != 1) return 0; // Major version 1
   if (ttUSHORT(data+2) != 0) return 0; // Minor version 0

   lookupListOffset = ttUSHORT(data+8);
   lookupList = data + lookupListOffset;
   lookupCount = ttUSHORT(lookupList);
   for (i=0; i<lookupCount; ++i) {
      uint16_t lookupOffset = ttUSHORT(lookupList + 2 + 2 * i);
      uint8_t *lookupTable = lookupList + lookupOffset;
      uint16_t lookupType = ttUSHORT(lookupTable);
      uint16_t subTableCount = ttUSHORT(lookupTable + 4);
      uint8_t *subTableOffsets = lookupTable + 6;
      if (lookupType != 2) continue; // Pair Adjustment Positioning Subtable

      for (sti=0; sti<subTableCount; sti++) {
         uint16_t subtableOffset = ttUSHORT(subTableOffsets + 2 * sti);
         uint8_t *table = lookupTable + subtableOffset;
         uint16_t posFormat = ttUSHORT(table);
         uint16_t coverageOffset = ttUSHORT(table + 2);
         int32_t coverageIndex = stbtt__GetCoverageIndex(table + coverageOffset, glyph1);
         if (coverageIndex == -1) continue;

         switch (posFormat) {
            case 1: {
               int32_t l, r, m;
               int straw, needle;
               uint16_t valueFormat1 = ttUSHORT(table + 4);
               uint16_t valueFormat2 = ttUSHORT(table + 6);
               if (valueFormat1 == 4 && valueFormat2 == 0) { // Support more formats?
                  int32_t valueRecordPairSizeInBytes = 2;
                  uint16_t pairSetCount = ttUSHORT(table + 8);
                  uint16_t pairPosOffset = ttUSHORT(table + 10 + 2 * coverageIndex);
                  uint8_t *pairValueTable = table + pairPosOffset;
                  uint16_t pairValueCount = ttUSHORT(pairValueTable);
                  uint8_t *pairValueArray = pairValueTable + 2;
                  if (coverageIndex >= pairSetCount) return 0;

                  needle=glyph2;
                  r=pairValueCount-1;
                  l=0;
                  while (l <= r) { // Binary search.
                     uint16_t secondGlyph;
                     uint8_t *pairValue;
                     m = (l + r) >> 1;
                     pairValue = pairValueArray + (2 + valueRecordPairSizeInBytes) * m;
                     secondGlyph = ttUSHORT(pairValue);
                     straw = secondGlyph;
                     if (needle < straw) r = m - 1;
                     else if (needle > straw) l = m + 1;
                     else { int16_t xAdvance = ttSHORT(pairValue + 2); return xAdvance; }
                  }
               } else return 0;
               break;
            }

            case 2: {
               uint16_t valueFormat1 = ttUSHORT(table + 4);
               uint16_t valueFormat2 = ttUSHORT(table + 6);
               if (valueFormat1 == 4 && valueFormat2 == 0) { // Support more formats?
                  uint16_t classDef1Offset = ttUSHORT(table + 8);
                  uint16_t classDef2Offset = ttUSHORT(table + 10);
                  int glyph1class = stbtt__GetGlyphClass(table + classDef1Offset, glyph1);
                  int glyph2class = stbtt__GetGlyphClass(table + classDef2Offset, glyph2);
                  uint16_t class1Count = ttUSHORT(table + 12);
                  uint16_t class2Count = ttUSHORT(table + 14);
                  uint8_t *class1Records, *class2Records;
                  int16_t xAdvance;
                  if (glyph1class < 0 || glyph1class >= class1Count) return 0; // malformed
                  if (glyph2class < 0 || glyph2class >= class2Count) return 0; // malformed

                  class1Records = table + 16;
                  class2Records = class1Records + 2 * (glyph1class * class2Count);
                  xAdvance = ttSHORT(class2Records + 2 * glyph2class);
                  return xAdvance;
               } else return 0;
               break;
            }

            default: return 0; // Unsupported position format
         }
      }
   }

   return 0;
}

__attribute__((pure)) extern int stbtt_GetGlyphKernAdvance(const stbtt_fontinfo *info, int g1, int g2) {
   int xAdvance = 0;
   if (info->gpos) xAdvance += stbtt__GetGlyphGPOSInfoAdvance(info, g1, g2);
   else if (info->kern) xAdvance += stbtt__GetGlyphKernInfoAdvance(info, g1, g2);

   return xAdvance;
}

extern void stbtt_GetFontBoundingBox(const stbtt_fontinfo *info, int *x0, int *y0, int *x1, int *y1) { *x0 = ttSHORT(info->data + info->head + 36); *y0 = ttSHORT(info->data + info->head + 38); *x1 = ttSHORT(info->data + info->head + 40); *y1 = ttSHORT(info->data + info->head + 42); }
__attribute__((pure)) extern float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float height) { int fheight = ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6); return (float) height / fheight; }
__attribute__((pure)) extern float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo *info, float pixels) { int unitsPerEm = ttUSHORT(info->data + info->head + 18); return pixels / unitsPerEm; }
extern void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1) {
   int x0=0,y0=0,x1,y1; // =0 suppresses compiler warning
   if (!stbtt_GetGlyphBox(font, glyph, &x0,&y0,&x1,&y1)) {
      // e.g. space character
      if (ix0) *ix0 = 0;
      if (iy0) *iy0 = 0;
      if (ix1) *ix1 = 0;
      if (iy1) *iy1 = 0;
   } else {
      // move to integral bboxes (treating pixels as little squares, what pixels get touched)?
      if (ix0) *ix0 = (int)vfloor( x0 * scale_x + shift_x);
      if (iy0) *iy0 = (int)vfloor(-y1 * scale_y + shift_y);
      if (ix1) *ix1 = (int)vceil(x1 * scale_x + shift_x);
      if (iy1) *iy1 = (int)vceil(-y0 * scale_y + shift_y);
   }
}

extern void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1) { stbtt_GetGlyphBitmapBoxSubpixel(font,glyph,scale_x,scale_y,0.0f,0.0f,ix0,iy0,ix1,iy1); }
typedef struct stbtt__hheap_chunk { struct stbtt__hheap_chunk *next; } stbtt__hheap_chunk;
typedef struct stbtt__hheap { struct stbtt__hheap_chunk *head; void *first_free; int num_remaining_in_head_chunk; } stbtt__hheap;
static void *stbtt__hheap_alloc(stbtt__hheap *hh, size_t size) {
   if (hh->first_free) {
      void *p = hh->first_free;
      hh->first_free = * (void **) p;
      return p;
   } else {
      if (hh->num_remaining_in_head_chunk == 0) {
         int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
         stbtt__hheap_chunk* c = (stbtt__hheap_chunk*)STBTT_malloc(sizeof(stbtt__hheap_chunk) + size * count);
         if (c == NULL) return NULL;
         
         c->next = hh->head;
         hh->head = c;
         hh->num_remaining_in_head_chunk = count;
      }
      
      --hh->num_remaining_in_head_chunk;
      return (char *) (hh->head) + sizeof(stbtt__hheap_chunk) + size * hh->num_remaining_in_head_chunk;
   }
}

static void stbtt__hheap_free(stbtt__hheap *hh, void *p) { *(void **)p = hh->first_free; hh->first_free = p; }
static void stbtt__hheap_cleanup(stbtt__hheap *hh) { stbtt__hheap_chunk *c = hh->head; while (c) { stbtt__hheap_chunk *n = c->next; STBTT_free(c); c = n; } }

typedef struct stbtt__edge { float x0,y0, x1,y1; int invert; } stbtt__edge;
typedef struct stbtt__active_edge { struct stbtt__active_edge *next; float fx,fdx,fdy,direction,sy,ey; } stbtt__active_edge;
static stbtt__active_edge *stbtt__new_active(stbtt__hheap *hh, stbtt__edge *e, int off_x, float start_point) {
   stbtt__active_edge *z = (stbtt__active_edge *)stbtt__hheap_alloc(hh, sizeof(*z));
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   if (!z) return z;
   
   z->fdx = dxdy;
   z->fdy = dxdy != 0.0f ? (1.0f/dxdy) : 0.0f;
   z->fx = e->x0 + dxdy * (start_point - e->y0);
   z->fx -= (float)off_x;
   z->direction = e->invert ? 1.0f : -1.0f;
   z->sy = e->y0; z->ey = e->y1;
   z->next = 0;
   return z;
}

// the edge passed in here does not cross the vertical line at x or the vertical line at x+1 (i.e. it has already been clipped to those)
static void stbtt__handle_clipped_edge(float *scanline, int x, stbtt__active_edge *e, float x0, float y0, float x1, float y1) {
   if (y0 == y1) return;
   if (y0 > e->ey) return;
   if (y1 < e->sy) return;
   if (y0 < e->sy) { x0 += (x1-x0) * (e->sy - y0) / (y1-y0); y0 = e->sy; }
   if (y1 > e->ey) { x1 += (x1-x0) * (e->ey - y1) / (y1-y0); y1 = e->ey; }
   if (x0 <= x && x1 <= x) scanline[x] += e->direction * (y1-y0);
   else if (x0 >= x+1 && x1 >= x+1) ;
   else { scanline[x] += e->direction * (y1 - y0) * (1.0f - ((x0 - (float)x) + (x1 - (float)x)) / 2.0f); } // coverage = 1 - average x position
}

static float stbtt__sized_trapezoid_area(float height, float top_width, float bottom_width) { return (top_width + bottom_width) / 2.0f * height; }
static float stbtt__position_trapezoid_area(float height, float tx0, float tx1, float bx0, float bx1) { return stbtt__sized_trapezoid_area(height, tx1 - tx0, bx1 - bx0); }
static float stbtt__sized_triangle_area(float height, float width) { return height * width / 2; }

static void stbtt__fill_active_edges_new(float *scanline, float *scanline_fill, int len, stbtt__active_edge *e, float y_top) {
   float y_bottom = y_top+1;
   while (e) {
      if (e->fdx == 0) {
         float x0 = e->fx;
         if (x0 < len) {
            if (x0 >= 0) {
               stbtt__handle_clipped_edge(scanline,(int) x0,e, x0,y_top, x0,y_bottom);
               stbtt__handle_clipped_edge(scanline_fill-1,(int) x0+1,e, x0,y_top, x0,y_bottom);
            } else {
               stbtt__handle_clipped_edge(scanline_fill-1,0,e, x0,y_top, x0,y_bottom);
            }
         }
      } else {
         float x0 = e->fx;
         float dx = e->fdx;
         float xb = x0 + dx;
         float x_top, x_bottom;
         float sy0,sy1;
         float dy = e->fdy;
         if (e->sy > y_top) {
            x_top = x0 + dx * (e->sy - y_top);
            sy0 = e->sy;
         } else {
            x_top = x0;
            sy0 = y_top;
         }
         if (e->ey < y_bottom) {
            x_bottom = x0 + dx * (e->ey - y_top);
            sy1 = e->ey;
         } else {
            x_bottom = xb;
            sy1 = y_bottom;
         }

         if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
            if ((int)x_top == (int)x_bottom) {
               float height;
               int x = (int) x_top;
               height = (sy1 - sy0) * e->direction;
               scanline[x]      += stbtt__position_trapezoid_area(height, x_top, (float)x + 1.0f, x_bottom, (float)x + 1.0f);
               scanline_fill[x] += height; // everything right of this pixel is filled
            } else {
               int x,x1,x2;
               float y_crossing, y_final, step, sign, area;
               // covers 2+ pixels
               if (x_top > x_bottom) {
                  // flip scanline vertically; signed area is the same
                  float t;
                  sy0 = y_bottom - (sy0 - y_top);
                  sy1 = y_bottom - (sy1 - y_top);
                  t = sy0, sy0 = sy1, sy1 = t;
                  t = x_bottom, x_bottom = x_top, x_top = t;
                  dx = -dx;
                  dy = -dy;
                  t = x0, x0 = xb, xb = t;
               }

               x1 = (int) x_top;
               x2 = (int) x_bottom;
               y_crossing = y_top + dy * ((float)(x1+1) - x0);
               y_final = y_top + dy * ((float)x2 - x0);
               if (y_crossing > y_bottom) y_crossing = y_bottom;
               sign = e->direction;
               area = sign * (y_crossing-sy0);
               scanline[x1] += stbtt__sized_triangle_area(area, (float)(x1+1) - x_top);
               if (y_final > y_bottom) {
                  y_final = y_bottom;
                  dy = (y_final - y_crossing ) / ((float)x2 - (float)(x1+1)); // if denom=0, y_final = y_crossing, so y_final <= y_bottom
               }

               step = sign * dy * 1;
               for (x = x1+1; x < x2; ++x) {
                  scanline[x] += area + step/2; // area of trapezoid is 1*step/2
                  area += step;
               }
               
               scanline[x2] += area + sign * stbtt__position_trapezoid_area(sy1-y_final, (float)x2, (float)x2 + 1.0f, x_bottom, (float)x2 + 1.0f);
               scanline_fill[x2] += sign * (sy1-sy0);
            }
         } else {
            for (int x=0; x < len; ++x) {
               float y0 = y_top;
               float x1 = (float)(x);
               float x2 = (float)(x+1);
               float x3 = xb;
               float y3 = y_bottom;
               float y1 = ((float)x - x0) / dx + y_top;
               float y2 = ((float)(x+1) - x0) / dx + y_top;
               if (x0 < x1 && x3 > x2) {         // three segments descending down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else {  // one segment
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x3,y3);
               }
            }
         }
      }
      e = e->next;
   }
}

// directly AA rasterize edges w/o supersampling
static void stbtt__rasterize_sorted_edges(stbtt__bitmap *result, stbtt__edge *e, int n, int off_x, int off_y) {
   stbtt__hheap hh = { 0, 0, 0 };
   stbtt__active_edge *active = NULL;
   int y,j=0, i;
   float scanline_data[129], *scanline, *scanline2;
   if (result->w > 64) scanline = (float*)STBTT_malloc((size_t)(result->w*2+1) * sizeof(float));
   else scanline = scanline_data;

   scanline2 = scanline + result->w;
   y = off_y;
   e[n].y0 = (float) (off_y + result->h) + 1;
   while (j < result->h) {
      float scan_y_top    = (float)y + 0.0f;
      float scan_y_bottom = (float)y + 1.0f;
      stbtt__active_edge **step = &active;
      __builtin_memset(scanline , 0, (size_t)result->w*sizeof(scanline[0]));
      __builtin_memset(scanline2, 0, ((size_t)result->w+1)*sizeof(scanline[0]));
      while (*step) {
         stbtt__active_edge * z = *step;
         if (z->ey <= scan_y_top) {
            *step = z->next; // delete from list
            z->direction = 0;
            stbtt__hheap_free(&hh, z);
         } else {
            step = &((*step)->next); // advance through list
         }
      }

      // insert all edges that start before the bottom of this scanline
      while (e->y0 <= scan_y_bottom) {
         if (e->y0 != e->y1) {
            stbtt__active_edge *z = stbtt__new_active(&hh, e, off_x, scan_y_top);
            if (z != NULL) {
               if (j == 0 && off_y != 0) {
                  if (z->ey < scan_y_top) {
                     // this can happen due to subpixel positioning and some kind of fp rounding error i think
                     z->ey = scan_y_top;
                  }
               }
               // insert at front
               z->next = active;
               active = z;
            }
         }
         ++e;
      }

      // now process all active edges
      if (active)
         stbtt__fill_active_edges_new(scanline, scanline2+1, result->w, active, scan_y_top);

      {
         float sum = 0;
         for (i=0; i < result->w; ++i) {
            float k;
            int m;
            sum += scanline2[i];
            k = scanline[i] + sum;
            k = (float)vabs(k) * 255.0f + 0.5f;
            m = (int)k;
            if (m > 255) m = 255;
            result->pixels[j*result->stride + i] = (unsigned char) m;
         }
      }
      // advance all the edges
      step = &active;
      while (*step) {
         stbtt__active_edge *z = *step;
         z->fx += z->fdx; // advance to position for current scanline
         step = &((*step)->next); // advance through list
      }

      ++y;
      ++j;
   }

   stbtt__hheap_cleanup(&hh);
   if (scanline != scanline_data) STBTT_free(scanline);
}

#define STBTT__COMPARE(a,b)  ((a)->y0 < (b)->y0)

static void stbtt__sort_edges_ins_sort(stbtt__edge *p, int n)
{
   int i,j;
   for (i=1; i < n; ++i) {
      stbtt__edge t = p[i], *a = &t;
      j = i;
      while (j > 0) {
         stbtt__edge *b = &p[j-1];
         int c = STBTT__COMPARE(a,b);
         if (!c) break;
         p[j] = p[j-1];
         --j;
      }
      if (i != j)
         p[j] = t;
   }
}

static void stbtt__sort_edges_quicksort(stbtt__edge *p, int n)
{
   /* threshold for transitioning to insertion sort */
   while (n > 12) {
      stbtt__edge t;
      int c01,c12,c,m,i,j;

      /* compute median of three */
      m = n >> 1;
      c01 = STBTT__COMPARE(&p[0],&p[m]);
      c12 = STBTT__COMPARE(&p[m],&p[n-1]);
      /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
      if (c01 != c12) {
         /* otherwise, we'll need to swap something else to middle */
         int z;
         c = STBTT__COMPARE(&p[0],&p[n-1]);
         /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
         /* 0<mid && mid>n:  0>n => 0; 0<n => n */
         z = (c == c12) ? 0 : n-1;
         t = p[z];
         p[z] = p[m];
         p[m] = t;
      }
      /* now p[m] is the median-of-three */
      /* swap it to the beginning so it won't move around */
      t = p[0];
      p[0] = p[m];
      p[m] = t;

      /* partition loop */
      i=1;
      j=n-1;
      for(;;) {
         /* handling of equality is crucial here */
         /* for sentinels & efficiency with duplicates */
         for (;;++i) {
            if (!STBTT__COMPARE(&p[i], &p[0])) break;
         }
         for (;;--j) {
            if (!STBTT__COMPARE(&p[0], &p[j])) break;
         }
         /* make sure we haven't crossed */
         if (i >= j) break;
         t = p[i];
         p[i] = p[j];
         p[j] = t;

         ++i;
         --j;
      }
      /* recurse on smaller side, iterate on larger */
      if (j < (n-i)) {
         stbtt__sort_edges_quicksort(p,j);
         p = p+i;
         n = n-i;
      } else {
         stbtt__sort_edges_quicksort(p+i, n-i);
         n = j;
      }
   }
}

static void stbtt__sort_edges(stbtt__edge *p, int n)
{
   stbtt__sort_edges_quicksort(p, n);
   stbtt__sort_edges_ins_sort(p, n);
}

typedef struct
{
   float x,y;
} stbtt__point;

static void stbtt__rasterize(stbtt__bitmap *result, stbtt__point *pts, int *wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert) {
   float y_scale_inv = invert ? -scale_y : scale_y;
   stbtt__edge *e;
   int n=0,i,j,k;
   for (i=0; i < windings; ++i) n += wcount[i];
   e = (stbtt__edge*)STBTT_malloc(sizeof(*e) * ((size_t)n+1)); // add an extra one as a sentinel
   if (e == 0) return;
   
   n = 0; int m=0;
   for (i=0; i < windings; ++i) {
      stbtt__point *p = pts + m;
      m += wcount[i];
      j = wcount[i]-1;
      for (k=0; k < wcount[i]; j=k++) {
         int a=k,b=j;
         if (p[j].y == p[k].y) continue;
         
         e[n].invert = 0;
         if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) { e[n].invert = 1; a=j,b=k; }
         e[n].x0 = p[a].x * scale_x + shift_x;
         e[n].y0 = (p[a].y * y_scale_inv + shift_y);
         e[n].x1 = p[b].x * scale_x + shift_x;
         e[n].y1 = (p[b].y * y_scale_inv + shift_y);
         ++n;
      }
   }

   stbtt__sort_edges(e, n);
   stbtt__rasterize_sorted_edges(result,e,n,off_x,off_y);
   STBTT_free(e);
}

static void stbtt__add_point(stbtt__point *points, int n, float x, float y) {
   if (!points) return; // during first pass, it's unallocated
   points[n].x = x;
   points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
static int stbtt__tesselate_curve(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n) {
   // midpoint
   float mx = (x0 + 2*x1 + x2)/4;
   float my = (y0 + 2*y1 + y2)/4;
   // versus directly drawn line
   float dx = (x0+x2)/2 - mx;
   float dy = (y0+y2)/2 - my;
   if (n > 16) // 65536 segments on one curve better be enough!
      return 1;
   if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
      stbtt__tesselate_curve(points, num_points, x0,y0, (x0+x1)/2.0f,(y0+y1)/2.0f, mx,my, objspace_flatness_squared,n+1);
      stbtt__tesselate_curve(points, num_points, mx,my, (x1+x2)/2.0f,(y1+y2)/2.0f, x2,y2, objspace_flatness_squared,n+1);
   } else {
      stbtt__add_point(points, *num_points,x2,y2);
      *num_points = *num_points+1;
   }
   return 1;
}

static void stbtt__tesselate_cubic(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n) {
   float dx0 = x1-x0; float dy0 = y1-y0; // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
   float dx1 = x2-x1; float dy1 = y2-y1;
   float dx2 = x3-x2; float dy2 = y3-y2;
   float dx = x3-x0; float dy = y3-y0;
   float longlen = vsqrtf(dx0*dx0+dy0*dy0) + vsqrtf(dx1*dx1+dy1*dy1) + vsqrtf(dx2*dx2+dy2*dy2);
   float shortlen = vsqrtf(dx*dx+dy*dy);
   float flatness_squared = longlen*longlen-shortlen*shortlen;
   if (n > 16) return; // 65536 segments on one curve better be enough!

   if (flatness_squared > objspace_flatness_squared) {
      float x01 = (x0+x1)/2; float y01 = (y0+y1)/2;
      float x12 = (x1+x2)/2; float y12 = (y1+y2)/2;
      float x23 = (x2+x3)/2; float y23 = (y2+y3)/2;
      float xa = (x01+x12)/2; float ya = (y01+y12)/2;
      float xb = (x12+x23)/2; float yb = (y12+y23)/2;
      float mx = (xa+xb)/2; float my = (ya+yb)/2;
      stbtt__tesselate_cubic(points,num_points,x0,y0,x01,y01,xa,ya,mx,my,objspace_flatness_squared,n+1);
      stbtt__tesselate_cubic(points,num_points,mx,my,xb,yb,x23,y23,x3,y3,objspace_flatness_squared,n+1);
   } else {
      stbtt__add_point(points,*num_points,x3,y3);
      *num_points = *num_points+1;
   }
}

// returns number of contours
static stbtt__point *stbtt_FlattenCurves(stbtt_vertex *vertices, int num_verts, float objspace_flatness, int **contour_lengths, int *num_contours) {
   stbtt__point *points=0;
   int num_points=0;
   float objspace_flatness_squared = objspace_flatness * objspace_flatness;
   int i,n=0,start=0, pass;
   for (i=0; i < num_verts; ++i) { if (vertices[i].type == STBTT_vmove) ++n; }
   *num_contours = n; if (n == 0) return 0;

   *contour_lengths = (int*)STBTT_malloc(sizeof(**contour_lengths) * (size_t)n);
   if (*contour_lengths == 0) { *num_contours = 0; return 0; }

   for (pass=0; pass < 2; ++pass) {
      float x=0,y=0;
      if (pass == 1) {
         points = (stbtt__point*)STBTT_malloc((size_t)num_points * sizeof(points[0])); if (points == NULL) goto error;
      }
      
      num_points = 0;
      n= -1;
      for (i=0; i < num_verts; ++i) {
         switch (vertices[i].type) {
            case STBTT_vmove:
               // start the next contour
               if (n >= 0) (*contour_lengths)[n] = num_points - start;
               ++n;
               start = num_points;
               x = vertices[i].x, y = vertices[i].y;
               stbtt__add_point(points, num_points++, x,y);
               break;
            case STBTT_vline:
               x = vertices[i].x, y = vertices[i].y;
               stbtt__add_point(points, num_points++, x, y);
               break;
            case STBTT_vcurve:
               stbtt__tesselate_curve(points,&num_points,x,y,vertices[i].cx,vertices[i].cy,vertices[i].x,vertices[i].y,objspace_flatness_squared,0);
               x = vertices[i].x, y = vertices[i].y;
               break;
            case STBTT_vcubic:
               stbtt__tesselate_cubic(points,&num_points,x,y,vertices[i].cx,vertices[i].cy,vertices[i].cx1,vertices[i].cy1,vertices[i].x,vertices[i].y,objspace_flatness_squared,0);
               x = vertices[i].x, y = vertices[i].y;
               break;
         }
      }
      (*contour_lengths)[n] = num_points - start;
   }

   return points;
   
   error:
   STBTT_free(points);
   STBTT_free(*contour_lengths);
   *contour_lengths = 0;
   *num_contours = 0;
   return NULL;
}

STBTT_DEF void stbtt_Rasterize(stbtt__bitmap *result, float flatness_in_pixels, stbtt_vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert) {
   float scale            = scale_x > scale_y ? scale_y : scale_x;
   int winding_count      = 0;
   int *winding_lengths   = NULL;
   stbtt__point *windings = stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count);
   if (windings) {
      stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert);
      STBTT_free(winding_lengths);
      STBTT_free(windings);
   }
}

STBTT_DEF unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff) {
   int ix0,iy0,ix1,iy1;
   stbtt__bitmap gbm;
   stbtt_vertex *vertices;
   int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);
   if (scale_x <= 0.000001f) scale_x = scale_y;
   if (scale_y <= 0.000001f) {
      if (scale_x <= 0.000001f) { STBTT_free(vertices); return NULL; }
      
      scale_y = scale_x;
   }

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,&ix1,&iy1);

   // now we get the size
   gbm.w = (ix1 - ix0);
   gbm.h = (iy1 - iy0);
   gbm.pixels = NULL; // in case we error
   if (width ) *width  = gbm.w;
   if (height) *height = gbm.h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;
   if (gbm.w && gbm.h) {
      gbm.pixels = (unsigned char*)STBTT_malloc((size_t)(gbm.w * gbm.h));
      if (gbm.pixels) {
         gbm.stride = gbm.w;
         stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1);
      }
   }
   
   STBTT_free(vertices);
   return gbm.pixels;
}

STBTT_DEF unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y, 0.0f, 0.0f, glyph, width, height, xoff, yoff);
}

STBTT_DEF void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph) {
   int ix0,iy0;
   stbtt_vertex *vertices;
   int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);
   stbtt__bitmap gbm;
   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,0,0);
   gbm.pixels = output;
   gbm.w = out_w;
   gbm.h = out_h;
   gbm.stride = out_stride;
   if (gbm.w && gbm.h) stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0,iy0, 1);
   STBTT_free(vertices);
}

STBTT_DEF void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph)
{
   stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, glyph);
}

#ifndef STB_RECT_PACK_VERSION

typedef int stbrp_coord;

typedef struct {
   int width,height;
   int x,y,bottom_y;
} stbrp_context;

typedef struct {
   unsigned char x;
} stbrp_node;

struct stbrp_rect {
   stbrp_coord x,y;
   int id,w,h,was_packed;
};

static void stbrp_pack_rects(stbrp_context *con, stbrp_rect *rects, int num_rects) {
   int i;
   for (i=0; i < num_rects; ++i) {
      if (con->x + rects[i].w > con->width) {
         con->x = 0;
         con->y = con->bottom_y;
      }

      if (con->y + rects[i].h > con->height) break;
      rects[i].x = con->x;
      rects[i].y = con->y;
      rects[i].was_packed = 1;
      con->x += rects[i].w;
      if (con->y + rects[i].h > con->bottom_y) con->bottom_y = con->y + rects[i].h;
   }

   for (; i < num_rects; ++i) rects[i].was_packed = 0;
}
#endif

STBTT_DEF int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int pw, int ph, int stride_in_bytes, int padding, void *alloc_context) {
   stbrp_context* context = (stbrp_context*)STBTT_malloc(sizeof(*context));
   spc->user_allocator_context = alloc_context;
   spc->width = pw;
   spc->height = ph;
   spc->pixels = pixels;
   spc->pack_info = context;
   spc->padding = padding;
   spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
   spc->h_oversample = 1;
   spc->v_oversample = 1;
   spc->skip_missing = 0;
   context->width  = pw-padding;
   context->height = ph-padding;
   context->x = 0; context->y = 0;
   context->bottom_y = 0;
   if (pixels) __builtin_memset(pixels, 0, (size_t)(pw*ph)); // background of 0 around pixels
   return 1;
}

#define STBTT__OVER_MASK  (STBTT_MAX_OVERSAMPLE-1)

static void stbtt__h_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width) {
   unsigned char buffer[STBTT_MAX_OVERSAMPLE];
   int safe_w = w - (int)kernel_width;
   __builtin_memset(buffer,0,STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (int j=0;j<h;++j) {
      int i;
      unsigned int total = 0; __builtin_memset(buffer,0,kernel_width);
      switch (kernel_width) { // make kernel_width a constant in common cases so compiler can optimize out the divide
         case 2:
            for (i=0;i<=safe_w;++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0;i<=safe_w;++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0;i<=safe_w;++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0;i<=safe_w;++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0;i<=safe_w;++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (;i<w;++i) {
         total -= buffer[i & STBTT__OVER_MASK];
         pixels[i] = (unsigned char) (total / kernel_width);
      }

      pixels += stride_in_bytes;
   }
}

static void stbtt__v_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width) {
   unsigned char buffer[STBTT_MAX_OVERSAMPLE]; int safe_h = h - (int)kernel_width;
   __builtin_memset(buffer,0,STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (int j=0; j < w; ++j) {
      int i;
      unsigned int total;
      __builtin_memset(buffer,0,kernel_width);
      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0;i<=safe_h;++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0;i<=safe_h;++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0;i<=safe_h;++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0;i<=safe_h;++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0;i<=safe_h;++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+(int)kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (;i<h;++i) {
         total -= buffer[i & STBTT__OVER_MASK];
         pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
      }

      pixels += 1;
   }
}

static float stbtt__oversample_shift(int oversample) { if (!oversample) {return 0.0f;} return (float)-(oversample - 1) / (2.0f * (float)oversample); }

// rects array must be big enough to accommodate all characters in the given ranges
STBTT_DEF int stbtt_PackFontRangesGatherRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects) {
   int missing_glyph_added = 0; int k=0;
   for (int i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
      ranges[i].h_oversample = (unsigned char) spc->h_oversample;
      ranges[i].v_oversample = (unsigned char) spc->v_oversample;
      for (int j=0; j < ranges[i].num_chars; ++j) {
         int x0,y0,x1,y1;
         int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
         int glyph = stbtt_FindGlyphIndex(info, codepoint);
         if (glyph == 0 && (spc->skip_missing || missing_glyph_added)) {
            rects[k].w = rects[k].h = 0;
         } else {
            stbtt_GetGlyphBitmapBoxSubpixel(info,glyph,
                                            scale * (float)spc->h_oversample,
                                            scale * (float)spc->v_oversample,
                                            0,0,
                                            &x0,&y0,&x1,&y1);
            rects[k].w = (stbrp_coord) (x1-x0 + spc->padding + (int)spc->h_oversample-1);
            rects[k].h = (stbrp_coord) (y1-y0 + spc->padding + (int)spc->v_oversample-1);
            if (glyph == 0)
               missing_glyph_added = 1;
         }
         ++k;
      }
   }

   return k;
}

// rects array must be big enough to accommodate all characters in the given ranges
STBTT_DEF int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects) {
   int i,j,k=0, missing_glyph = -1, return_value = 1;
   int old_h_over = (int)spc->h_oversample; int old_v_over = (int)spc->v_oversample; // save current values
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
      float recip_h,recip_v,sub_x,sub_y;
      spc->h_oversample = ranges[i].h_oversample; spc->v_oversample = ranges[i].v_oversample;
      recip_h = 1.0f / (float)spc->h_oversample; recip_v = 1.0f / (float)spc->v_oversample;
      sub_x = (float)stbtt__oversample_shift((int)spc->h_oversample); sub_y = (float)stbtt__oversample_shift((int)spc->v_oversample);
      for (j=0; j < ranges[i].num_chars; ++j) {
         stbrp_rect *r = &rects[k];
         if (r->was_packed && r->w != 0 && r->h != 0) {
            stbtt_packedchar *bc = &ranges[i].chardata_for_range[j];
            int advance, lsb, x0,y0,x1,y1;
            int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
            int glyph = stbtt_FindGlyphIndex(info, codepoint);
            stbrp_coord pad = (stbrp_coord) spc->padding;
            r->x += pad; r->y += pad; r->w -= pad; r->h -= pad; // pad on left and top
            stbtt_GetGlyphHMetrics(info,glyph,&advance,&lsb);
            stbtt_GetGlyphBitmapBox(info,glyph,scale * (float)spc->h_oversample,scale * (float)spc->v_oversample,&x0,&y0,&x1,&y1);
            stbtt_MakeGlyphBitmapSubpixel(info,spc->pixels + r->x + r->y*spc->stride_in_bytes,r->w - (int)spc->h_oversample+1,r->h - (int)spc->v_oversample+1,spc->stride_in_bytes,scale * (float)spc->h_oversample,scale * (float)spc->v_oversample,0,0,glyph);
            if (spc->h_oversample > 1) stbtt__h_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,r->w,r->h,spc->stride_in_bytes,spc->h_oversample);
            if (spc->v_oversample > 1) stbtt__v_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,r->w,r->h,spc->stride_in_bytes,spc->v_oversample);
            bc->x0       = (unsigned short)r->x;
            bc->y0       = (unsigned short)r->y;
            bc->x1       = (unsigned short)(r->x + r->w);
            bc->y1       = (unsigned short)(r->y + r->h);
            bc->xadvance =                scale * (float)advance;
            bc->xoff     =       (float)x0 * recip_h + sub_x;
            bc->yoff     =       (float)y0 * recip_v + sub_y;
            bc->xoff2    =                (float)(x0 + r->w) * recip_h + sub_x;
            bc->yoff2    =                (float)(y0 + r->h) * recip_v + sub_y;
            if (glyph == 0) missing_glyph = j;
         } else if (spc->skip_missing) {
            return_value = 0;
         } else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
            ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
         } else return_value = 0; // if any fail, report failure
         ++k;
      }
   }

   spc->h_oversample = (unsigned int)old_h_over; // restore original values
   spc->v_oversample = (unsigned int)old_v_over;
   return return_value;
}

STBTT_DEF int stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges) {
   stbtt_fontinfo info;
   int i,j,n = 0, return_value = 1;
   stbrp_rect *rects;
   for (i=0; i < num_ranges; ++i) { // flag all characters as NOT packed
      for (j=0; j < ranges[i].num_chars; ++j) ranges[i].chardata_for_range[j].x0 = ranges[i].chardata_for_range[j].y0 = ranges[i].chardata_for_range[j].x1 = ranges[i].chardata_for_range[j].y1 = 0;
   }

   for (i=0;i<num_ranges;++i) n += ranges[i].num_chars;
   rects = (stbrp_rect*)STBTT_malloc(sizeof(*rects) * (size_t)n);
   if (rects == NULL) return 0;
   info.userdata = spc->user_allocator_context;
   stbtt_InitFont_internal(&info,(unsigned char *)fontdata,stbtt_GetFontOffsetForIndex(fontdata,font_index));
   n = stbtt_PackFontRangesGatherRects(spc,&info,ranges,num_ranges,rects);
   stbrp_pack_rects(spc->pack_info,rects,n);
   return_value = stbtt_PackFontRangesRenderIntoRects(spc,&info,ranges,num_ranges,rects);
   STBTT_free(rects);
   return return_value;
}

STBTT_DEF int stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size, int first_unicode_codepoint_in_range, int num_chars_in_range, stbtt_packedchar *chardata_for_range) {
   stbtt_pack_range range;
   range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
   range.array_of_unicode_codepoints = NULL;
   range.num_chars                   = num_chars_in_range;
   range.chardata_for_range          = chardata_for_range;
   range.font_size                   = font_size;
   return stbtt_PackFontRanges(spc,fontdata,font_index,&range,1);
}
