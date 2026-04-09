// stb_truetype.h - Font Load System
#pragma once
typedef struct { void* ptr; size_t sz; } TAlloc;
TAlloc ttAllocs[4474]; int tallocCount = 0;
void* TempAlloc(size_t amount) {
    if (tallocCount >= 4474) { DualLogError("TempAlloc too many!\n"); return NULL; }
    void* ptr = OS_Alloc(amount);
    if (!ptr) { DualLogError("TempAlloc: OS_Alloc failed!\n"); return NULL; }
    ttAllocs[tallocCount++] = (TAlloc){ptr, amount};
    return ptr;
}

void TempFree(void* ptr) { if (!ptr || tallocCount == 0 || ttAllocs[tallocCount-1].ptr != ptr) {return;} OS_DeallocateRAM(ptr,ttAllocs[tallocCount-1].sz); tallocCount--; }
#define STBTT_malloc(size)  TempAlloc(size)
#define STBTT_free(ptr)     TempFree(ptr)
typedef struct { unsigned short x0,y0,x1,y1;/* coordinates of bbox in bitmap*/ float xoff,yoff,xadvance; float xoff2,yoff2; } stbtt_packedchar;
typedef struct { float x0,y0,s0,t0; float x1,y1,s1,t1; } stbtt_aligned_quad; // 0 is top-left, 1 is bottom-right
void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int align_to_integer) {
   float ipw = 1.0f / (float)pw, iph = 1.0f / (float)ph;
   const stbtt_packedchar *b = chardata + char_index;
   if (align_to_integer) {
      float x = vfloor((*xpos + b->xoff) + 0.5f); float y = vfloor((*ypos + b->yoff) + 0.5f);
      q->x0 = x; q->y0 = y; q->x1 = x + b->xoff2 - b->xoff; q->y1 = y + b->yoff2 - b->yoff;
   } else { q->x0 = *xpos + b->xoff; q->y0 = *ypos + b->yoff; q->x1 = *xpos + b->xoff2; q->y1 = *ypos + b->yoff2; }
   q->s0 = b->x0 * ipw; q->t0 = b->y0 * iph; q->s1 = b->x1 * ipw; q->t1 = b->y1 * iph;
   *xpos += b->xadvance;
}

typedef struct { unsigned char *data; int cursor; int size; } stbtt__buf;
typedef struct stbtt_pack_context stbtt_pack_context; typedef struct stbtt_fontinfo stbtt_fontinfo; typedef struct stbrp_rect stbrp_rect;
extern int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int width, int height, int stride_in_bytes, int padding, void *alloc_context);
typedef struct {float font_size; int first_unicode_codepoint_in_range; int* array_of_unicode_codepoints; int num_chars; stbtt_packedchar *chardata_for_range; unsigned char h_oversample, v_oversample;} stbtt_pack_range;
struct stbtt_pack_context { void* user_allocator_context; void* pack_info; int width,height,stride_in_bytes,padding,skip_missing; unsigned int h_oversample,v_oversample; unsigned char *pixels; };
struct stbtt_fontinfo { void* userdata; unsigned char  * data; int fontstart,numGlyphs,loca,head,glyf,hhea,hmtx,kern,gpos,svg,index_map,indexToLocFormat; stbtt__buf cff,charstrings,gsubrs,subrs,fontdicts,fdselect; };
enum {STBTT_vmove=1,STBTT_vline,STBTT_vcurve,STBTT_vcubic};
#define stbtt_vertex_type short // can't use i16 because that's not visible in the header file
typedef struct { stbtt_vertex_type x,y,cx,cy,cx1,cy1; unsigned char type,padding; } stbtt_vertex;
extern int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **vertices);
typedef struct { int w,h,stride; unsigned char *pixels; } stbtt__bitmap;
enum { STBTT_MS_EID_SYMBOL=0, STBTT_MS_EID_UNICODE_BMP=1, STBTT_MS_EID_SHIFTJIS=2, STBTT_MS_EID_UNICODE_FULL=10 };
#define STBTT_MAX_OVERSAMPLE 8
static u8 stbtt__buf_get8(stbtt__buf *b) { return (b->cursor >= b->size) ? 0 : b->data[b->cursor++]; }
static u8 stbtt__buf_peek8(stbtt__buf *b) { return (b->cursor >= b->size) ? 0 : b->data[b->cursor]; }
static void stbtt__buf_seek(stbtt__buf *b, int o) { b->cursor = (o > b->size || o < 0) ? b->size : o; }
static void stbtt__buf_skip(stbtt__buf *b, int o) { stbtt__buf_seek(b, b->cursor + o); }
static u32 stbtt__buf_get(stbtt__buf *b, int n) { u32 v = 0; for (int i = 0; i < n; i++) { v = (v << 8) | stbtt__buf_get8(b); } return v; }
static stbtt__buf stbtt__new_buf(const void *p, size_t size) { stbtt__buf r; r.data=(u8*) p; r.size=(int)size; r.cursor=0; return r; }
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

static u32 stbtt__cff_int(stbtt__buf *b) {
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

static void stbtt__dict_get_ints(stbtt__buf *b, int key, int outcount, u32 *out) { stbtt__buf operands = stbtt__dict_get(b, key); for (int i=0;i<outcount && operands.cursor<operands.size;++i) { out[i] = stbtt__cff_int(&operands); }}
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

#define ttBYTE(p)     (* (u8 *) (p))
#define ttCHAR(p)     (* (i8 *) (p))
#define ttFixed(p)    ttLONG(p)
static u16 ttUSHORT(u8 *p) { return p[0]*256 + p[1]; }
static i16 ttSHORT(u8 *p)   { return p[0]*256 + p[1]; }
static u32 ttULONG(u8 *p)  { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
static i32 ttLONG(u8 *p)    { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
#define stbtt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str)           stbtt_tag4(p,str[0],str[1],str[2],str[3])
static int stbtt__isfont(u8 *font) {
   // check the version number
   if (stbtt_tag4(font, '1',0,0,0))  return 1; // TrueType 1
   if (stbtt_tag(font, "typ1"))   return 1; // TrueType with type 1 font -- we don't support this!
   if (stbtt_tag(font, "OTTO"))   return 1; // OpenType with CFF
   if (stbtt_tag4(font, 0,1,0,0)) return 1; // OpenType 1.0
   if (stbtt_tag(font, "true"))   return 1; // Apple specification for TrueType fonts
   return 0;
}

// @OPTIMIZE: binary search
static u32 stbtt__find_table(u8 *data, u32 fontstart, const char *tag)
{
   i32 num_tables = ttUSHORT(data+fontstart+4);
   u32 tabledir = fontstart + 12;
   i32 i;
   for (i=0; i < num_tables; ++i) {
      u32 loc = tabledir + 16*i;
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
         i32 n = ttLONG(font_collection+8);
         if (index >= n)
            return -1;
         return ttULONG(font_collection+12+index*4);
      }
   }
   return -1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
__attribute__((pure)) extern int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index) { return stbtt_GetFontOffsetForIndex_internal((unsigned char*)data,index); }
#pragma GCC diagnostic pop

static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict) {
   u32 subrsoff = 0, private_loc[2] = { 0, 0 };
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
   u32 cmap, t;
   i32 i,numTables;
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
      u32 cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
      u32 cff;
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
      u32 encoding_record = cmap + 4 + 8 * i;
      // find an encoding we understand:
      switch(ttUSHORT(data+encoding_record)) {
         case 3: // MICROSOFT
            switch (ttUSHORT(data+encoding_record+2)) {
               case 1: // BMP
               case 10: info->index_map = cmap + ttULONG(data+encoding_record+4); break; // FULL MS/Unicode
            }
            break;
        case 0: info->index_map = cmap + ttULONG(data+encoding_record+4); break; // UNICODE Mac/iOS has these, all the encodingIDs are unicode, so we don't bother to check it
      }
   }
   if (info->index_map == 0) return 0;

   info->indexToLocFormat = ttUSHORT(data+info->head + 50);
   return 1;
}

__attribute__((pure)) extern int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint) {
   u8 *data=info->data; u32 index_map=info->index_map; u16 format=ttUSHORT(data + index_map + 0);
   if (format == 0) { // apple byte encoding
      i32 bytes = ttUSHORT(data + index_map + 2);
      if (unicode_codepoint < bytes-6) return ttBYTE(data + index_map + 6 + unicode_codepoint);
      return 0;
   } else if (format == 6) {
      u32 first = ttUSHORT(data + index_map + 6);
      u32 count = ttUSHORT(data + index_map + 8);
      if ((u32) unicode_codepoint >= first && (u32) unicode_codepoint < first+count)
         return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first)*2);
      return 0;
   } else if (format == 2) { // @TODO: high-byte mapping for japanese/chinese/korean
      return 0;
   } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
      u16 segcount = ttUSHORT(data+index_map+6) >> 1;
      u16 searchRange = ttUSHORT(data+index_map+8) >> 1;
      u16 entrySelector = ttUSHORT(data+index_map+10);
      u16 rangeShift = ttUSHORT(data+index_map+12) >> 1;
      u32 endCount = index_map + 14; // do a binary search of the segments
      u32 search = endCount;
      if (unicode_codepoint > 0xffff) return 0;

      if (unicode_codepoint >= ttUSHORT(data + search + rangeShift*2)) search += rangeShift*2; // they lie from endCount .. endCount + segCount but searchRange is the nearest power of two, so...
      search -= 2; // now decrement to bias correctly to find smallest
      while (entrySelector) {
         u16 end;
         searchRange >>= 1;
         end = ttUSHORT(data + search + searchRange*2);
         if (unicode_codepoint > end)
            search += searchRange*2;
         --entrySelector;
      }
      search += 2;

      {
         u16 offset, start, last;
         u16 item = (u16) ((search - endCount) >> 1);
         start = ttUSHORT(data + index_map + 14 + segcount*2 + 2 + 2*item);
         last = ttUSHORT(data + endCount + 2*item);
         if (unicode_codepoint < start || unicode_codepoint > last) return 0;

         offset = ttUSHORT(data + index_map + 14 + segcount*6 + 2 + 2*item);
         if (offset == 0) return (u16) (unicode_codepoint + ttSHORT(data + index_map + 14 + segcount*4 + 2 + 2*item));
         return ttUSHORT(data + offset + (unicode_codepoint-start)*2 + index_map + 14 + segcount*6 + 2 + 2*item);
      }
   } else if (format == 12 || format == 13) {
      u32 ngroups = ttULONG(data+index_map+12);
      i32 low,high;
      low = 0; high = (i32)ngroups;
      // Binary search the right group.
      while (low < high) {
         i32 mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
         u32 start_char = ttULONG(data+index_map+16+mid*12);
         u32 end_char = ttULONG(data+index_map+16+mid*12+4);
         if ((u32) unicode_codepoint < start_char)
            high = mid;
         else if ((u32) unicode_codepoint > end_char)
            low = mid+1;
         else {
            u32 start_glyph = ttULONG(data+index_map+16+mid*12+8);
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

static void stbtt_setvertex(stbtt_vertex *v, u8 type, i32 x, i32 y, i32 cx, i32 cy) { v->type = type; v->x = (i16) x; v->y = (i16) y; v->cx = (i16) cx; v->cy = (i16) cy; }
static int stbtt__GetGlyfOffset(const stbtt_fontinfo *info, int glyph_index) {
   if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
   if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

   int g1,g2;
   if (info->indexToLocFormat == 0) { g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2; g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
   } else { g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4); g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4); }

   return g1==g2 ? -1 : g1; // if length is 0, return -1
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);
extern int stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1) {
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

static int stbtt__close_shape(stbtt_vertex *vertices, int num_vertices, int was_off, int start_off, i32 sx, i32 sy, i32 scx, i32 scy, i32 cx, i32 cy) {
   if (start_off) {
      if (was_off) stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+scx)>>1, (cy+scy)>>1, cx,cy);
      stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx,sy,scx,scy);
   } else {
      if (was_off) stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve,sx,sy,cx,cy);
      else stbtt_setvertex(&vertices[num_vertices++], STBTT_vline,sx,sy,0,0);
   }
   return num_vertices;
}

static int stbtt__GetGlyphShapeTT(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices) {
    u8 *data = info->data; stbtt_vertex *vertices = 0; int num_vertices = 0, g = stbtt__GetGlyfOffset(info,glyph_index);
    *pvertices = NULL;
    if (g < 0) return 0;

    i16 numberOfContours = ttSHORT(data + g);
    if (numberOfContours > 0) {
        u8 *endPtsOfContours = data + g + 10;
        int ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
        u8 *points = data + g + 10 + numberOfContours * 2 + 2 + ins;
        int n = 1 + ttUSHORT(endPtsOfContours + numberOfContours*2-2);
        int m = n + 2*numberOfContours;
        vertices = (stbtt_vertex*)STBTT_malloc(m * sizeof(vertices[0]));
        if (!vertices) return 0;

        int off = m - n;
        u8 flags = 0, flagcount = 0;
        for (int i = 0; i < n; ++i) {
            if (flagcount == 0) { flags = *points++; if (flags & 8) flagcount = *points++; }
            else --flagcount;
            vertices[off+i].type = flags;
        }

        i32 x = 0;
        for (int i = 0; i < n; ++i) {
            flags = vertices[off+i].type;
            if (flags & 2) { i16 dx = *points++; x += (flags & 16) ? dx : -dx; }
            else if (!(flags & 16)) { x += (i16)(points[0]*256 + points[1]); points += 2; }
            vertices[off+i].x = (i16)x;
        }

        i32 y = 0;
        for (int i = 0; i < n; ++i) {
            flags = vertices[off+i].type;
            if (flags & 4) { i16 dy = *points++; y += (flags & 32) ? dy : -dy; }
            else if (!(flags & 32)) { y += (i16)(points[0]*256 + points[1]); points += 2; }
            vertices[off+i].y = (i16)y;
        }

        i32 sx=0,sy=0,cx=0,cy=0,scx=0,scy=0;
        int was_off=0, start_off=0, next_move=0, j=0;
        for (int i = 0; i < n; ++i) {
            flags = vertices[off+i].type;
            x = (i16)vertices[off+i].x;
            y = (i16)vertices[off+i].y;
            if (next_move == i) {
                if (i) num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
                start_off = !(flags & 1);
                if (start_off) {
                    scx = x; scy = y;
                    if (!(vertices[off+i+1].type & 1)) {
                        sx = (x + (i32)vertices[off+i+1].x) >> 1;
                        sy = (y + (i32)vertices[off+i+1].y) >> 1;
                    } else { sx = vertices[off+i+1].x; sy = vertices[off+i+1].y; ++i; }
                } else { sx = x; sy = y; }
                stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove, sx, sy, 0, 0);
                was_off = 0;
                next_move = 1 + ttUSHORT(endPtsOfContours + j++*2);
            } else {
                if (!(flags & 1)) {
                    if (was_off) stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
                    cx = x; cy = y; was_off = 1;
                } else {
                    stbtt_setvertex(&vertices[num_vertices++],was_off ? STBTT_vcurve : STBTT_vline,x,y,was_off ? cx : 0,was_off ? cy : 0);
                    was_off = 0;
                }
            }
        }
        num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
    } else if (numberOfContours < 0) {
        u8 *comp = data + g + 10;
        int more = 1;
        while (more) {
            stbtt_vertex *comp_verts = 0, *tmp = 0;
            float mtx[6] = {1,0,0,1,0,0};
            u16 flags = ttSHORT(comp); comp += 2;
            u16 gidx  = ttSHORT(comp); comp += 2;
            if (flags & 2) {
                if (flags & 1) { mtx[4] = ttSHORT(comp); comp+=2; mtx[5] = ttSHORT(comp); comp+=2; }
                else           { mtx[4] = ttCHAR(comp);  comp+=1; mtx[5] = ttCHAR(comp);  comp+=1; }
            }
            if      (flags & (1<<3)) { mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2; mtx[1] = mtx[2] = 0; }
            else if (flags & (1<<6)) { mtx[0] = ttSHORT(comp)/16384.0f; comp+=2; mtx[1] = mtx[2] = 0; mtx[3] = ttSHORT(comp)/16384.0f; comp+=2; }
            else if (flags & (1<<7)) { mtx[0] = ttSHORT(comp)/16384.0f; comp+=2; mtx[1] = ttSHORT(comp)/16384.0f; comp+=2; mtx[2] = ttSHORT(comp)/16384.0f; comp+=2; mtx[3] = ttSHORT(comp)/16384.0f; comp+=2; }

            float fm = vsqrtf(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
            float fn = vsqrtf(mtx[2]*mtx[2] + mtx[3]*mtx[3]);
            int comp_num_verts = stbtt_GetGlyphShape(info, gidx, &comp_verts);
            if (comp_num_verts > 0) {
                for (int i = 0; i < comp_num_verts; ++i) {
                    stbtt_vertex *v = &comp_verts[i];
                    stbtt_vertex_type vx = v->x, vy = v->y;
                    v->x = (stbtt_vertex_type)(fm * (mtx[0]*vx + mtx[2]*vy + mtx[4]));
                    v->y = (stbtt_vertex_type)(fn * (mtx[1]*vx + mtx[3]*vy + mtx[5]));
                    vx = v->cx; vy = v->cy;
                    v->cx = (stbtt_vertex_type)(fm * (mtx[0]*vx + mtx[2]*vy + mtx[4]));
                    v->cy = (stbtt_vertex_type)(fn * (mtx[1]*vx + mtx[3]*vy + mtx[5]));
                }
                tmp = (stbtt_vertex*)STBTT_malloc((num_vertices + comp_num_verts) * sizeof(stbtt_vertex));
                if (!tmp) { STBTT_free(vertices); STBTT_free(comp_verts); return 0; }
                if (num_vertices > 0 && vertices) __builtin_memcpy(tmp, vertices, num_vertices*sizeof(stbtt_vertex));
                __builtin_memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(stbtt_vertex));
                STBTT_free(vertices); STBTT_free(comp_verts);
                vertices = tmp;
                num_vertices += comp_num_verts;
            }
            more = flags & (1<<5);
        }
    }

    *pvertices = vertices;
    return num_vertices;
}

typedef struct {int bounds,started; float first_x,first_y,x,y; i32 min_x,max_x,min_y,max_y; stbtt_vertex *pvertices; int num_vertices; } stbtt__csctx;

#define STBTT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}
static void stbtt__track_vertex(stbtt__csctx *c, i32 x, i32 y) {
   if (x > c->max_x || !c->started) c->max_x = x;
   if (y > c->max_y || !c->started) c->max_y = y;
   if (x < c->min_x || !c->started) c->min_x = x;
   if (y < c->min_y || !c->started) c->min_y = y;
   c->started = 1;
}

static void stbtt__csctx_v(stbtt__csctx *c, u8 type, i32 x, i32 y, i32 cx, i32 cy, i32 cx1, i32 cy1) {
   if (c->bounds) {
      stbtt__track_vertex(c, x, y);
      if (type == STBTT_vcubic) {
         stbtt__track_vertex(c, cx, cy);
         stbtt__track_vertex(c, cx1, cy1);
      }
   } else {
      stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
      c->pvertices[c->num_vertices].cx1 = (i16) cx1;
      c->pvertices[c->num_vertices].cy1 = (i16) cy1;
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
    int in_header=1, maskbits=0, subr_stack_height=0, sp=0, has_subrs=0, i, b0; float s[48], f;
    stbtt__buf subr_stack[10], subrs=info->subrs, b=stbtt__cff_index_get(info->charstrings,glyph_index);
#define ERR(x) return 0
#define CHK(n) if(sp<(n)) ERR(#n)
    while (b.cursor < b.size) {
        int clear_stack = 1;
        i = 0;
        b0 = stbtt__buf_get8(&b);
        switch (b0) {
        case 0x13: case 0x14:               // hintmask, cntrmask
            if (in_header) maskbits += sp/2;
            in_header = 0;
            stbtt__buf_skip(&b, (maskbits+7)/8);
            break;
        case 0x01: case 0x03: case 0x12: case 0x17: // h/vstem, h/vstemhm
            maskbits += sp/2;
            break;
        case 0x15: in_header=0; CHK(2); stbtt__csctx_rmove_to(c, s[sp-2], s[sp-1]); break; // rmoveto
        case 0x04: in_header=0; CHK(1); stbtt__csctx_rmove_to(c, 0,       s[sp-1]); break; // vmoveto
        case 0x16: in_header=0; CHK(1); stbtt__csctx_rmove_to(c, s[sp-1], 0      ); break; // hmoveto
        case 0x05: CHK(2); for(;i+1<sp;i+=2) stbtt__csctx_rline_to(c,s[i],s[i+1]); break; // rlineto
        case 0x07: CHK(1); goto vlineto;                                                     // vlineto
        case 0x06: CHK(1);                                                                   // hlineto
            for(;;) {
                if(i>=sp) break; stbtt__csctx_rline_to(c,s[i++],0);
                vlineto:
                if(i>=sp) break; stbtt__csctx_rline_to(c,0,s[i++]);
            } break;
        case 0x1F: CHK(4); goto hvcurveto;                                                   // hvcurveto
        case 0x1E: CHK(4);                                                                   // vhcurveto
            for(;;) {
                if(i+3>=sp) break;
                stbtt__csctx_rccurve_to(c, 0,s[i],s[i+1],s[i+2],s[i+3],(sp-i==5)?s[i+4]:0); i+=4;
                hvcurveto:
                if(i+3>=sp) break;
                stbtt__csctx_rccurve_to(c, s[i],0,s[i+1],s[i+2],(sp-i==5)?s[i+4]:0,s[i+3]); i+=4;
            } break;
        case 0x08: CHK(6); for(;i+5<sp;i+=6) stbtt__csctx_rccurve_to(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]); break; // rrcurveto
        case 0x18: CHK(8);                                                                   // rcurveline
            for(;i+5<sp-2;i+=6) stbtt__csctx_rccurve_to(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);
            stbtt__csctx_rline_to(c,s[i],s[i+1]); break;
        case 0x19: CHK(8);                                                                   // rlinecurve
            for(;i+1<sp-6;i+=2) stbtt__csctx_rline_to(c,s[i],s[i+1]);
            stbtt__csctx_rccurve_to(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]); break;
        case 0x1A: case 0x1B: CHK(4);                                                       // vvcurveto, hhcurveto
            f=0; if(sp&1){f=s[i++];}
            for(;i+3<sp;i+=4,f=0)
                stbtt__csctx_rccurve_to(c, b0==0x1B?s[i]:f, b0==0x1B?f:s[i],
                                           s[i+1],s[i+2], b0==0x1B?s[i+3]:0, b0==0x1B?0:s[i+3]);
            break;
        case 0x0A:                                                                           // callsubr
            if(!has_subrs){ if(info->fdselect.size) subrs=stbtt__cid_get_glyph_subrs(info,glyph_index); has_subrs=1; }
            // FALLTHROUGH
        case 0x1D: CHK(1);                                                                   // callgsubr
            if(subr_stack_height>=10) ERR("recursion");
            subr_stack[subr_stack_height++]=b;
            b=stbtt__get_subr(b0==0x0A?subrs:info->gsubrs,(int)s[--sp]);
            if(!b.size) ERR("subr not found");
            b.cursor=0; clear_stack=0; break;
        case 0x0B:                                                                           // return
            if(subr_stack_height<=0) ERR("return outside subr");
            b=subr_stack[--subr_stack_height]; clear_stack=0; break;
        case 0x0E:                                                                           // endchar
            stbtt__csctx_close_shape(c); return 1;
        case 0x0C: {                                                                         // two-byte escape
            int b1=stbtt__buf_get8(&b);
            switch(b1) {
            case 0x22: CHK(7);  // hflex
                stbtt__csctx_rccurve_to(c,s[0],0,s[1],s[2],s[3],0);
                stbtt__csctx_rccurve_to(c,s[4],0,s[5],-s[2],s[6],0); break;
            case 0x23: CHK(13); // flex
                stbtt__csctx_rccurve_to(c,s[0],s[1],s[2],s[3],s[4],s[5]);
                stbtt__csctx_rccurve_to(c,s[6],s[7],s[8],s[9],s[10],s[11]); break;
            case 0x24: CHK(9);  // hflex1
                stbtt__csctx_rccurve_to(c,s[0],s[1],s[2],s[3],s[4],0);
                stbtt__csctx_rccurve_to(c,s[5],0,s[6],s[7],s[8],-(s[1]+s[3]+s[7])); break;
            case 0x25: CHK(11); { // flex1
                float dx=s[0]+s[2]+s[4]+s[6]+s[8], dy=s[1]+s[3]+s[5]+s[7]+s[9];
                float d6x=s[10], d6y=s[10];
                if(vabs(dx)>vabs(dy)) d6y=-dy; else d6x=-dx;
                stbtt__csctx_rccurve_to(c,s[0],s[1],s[2],s[3],s[4],s[5]);
                stbtt__csctx_rccurve_to(c,s[6],s[7],s[8],s[9],d6x,d6y); break;
            }
            default: ERR("unimplemented escape");
            }
        } break;
        default:
            if(b0!=255 && b0!=28 && b0<32) ERR("reserved");
            f = (b0==255) ? (float)(i32)stbtt__buf_get32(&b)/0x10000
                          : (stbtt__buf_skip(&b,-1), (float)(i16)stbtt__cff_int(&b));
            if(sp>=48) ERR("stack overflow");
            s[sp++]=f; clear_stack=0; break;
        }
        if(clear_stack) sp=0;
    }
    ERR("no endchar");
#undef ERR
#undef CHK
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

extern int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices) {
   if (!info->cff.size) return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
   else                 return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

extern void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing) {
   u16 numOfLongHorMetrics = ttUSHORT(info->data+info->hhea + 34);
   if (glyph_index < numOfLongHorMetrics) {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*glyph_index);
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*glyph_index + 2);
   } else {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*(numOfLongHorMetrics-1));
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
   }
}

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
static void stbtt__hheap_cleanup(stbtt__hheap *hh) { stbtt__hheap_chunk *c = hh->head; while (c) { stbtt__hheap_chunk *n = c->next; if (c) {STBTT_free(c);} c = n; } }

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
            if (x0 >= 0) { stbtt__handle_clipped_edge(scanline,(int)x0,e,x0,y_top,x0,y_bottom); stbtt__handle_clipped_edge(scanline_fill-1,(int) x0+1,e,x0,y_top,x0,y_bottom); }
            else stbtt__handle_clipped_edge(scanline_fill-1,0,e,x0,y_top,x0,y_bottom);
         }
      } else {
         float x0=e->fx,dx=e->fdx,dy = e->fdy;
         float xb=x0 + dx,x_top,x_bottom,sy0,sy1;
         if (e->sy > y_top) { x_top = x0 + dx * (e->sy - y_top); sy0 = e->sy; }
         else { x_top = x0; sy0 = y_top; }

         if (e->ey < y_bottom) { x_bottom = x0 + dx * (e->ey - y_top); sy1 = e->ey; }
         else { x_bottom = xb; sy1 = y_bottom; }

         if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
            if ((int)x_top == (int)x_bottom) {
               int x = (int)x_top; float height = (sy1 - sy0) * e->direction;
               scanline[x]      += stbtt__position_trapezoid_area(height,x_top,(float)x + 1.0f,x_bottom,(float)x + 1.0f);
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
               for (x = x1+1; x < x2; ++x) { scanline[x] += area + step/2; area += step; }
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
   if (scanline != scanline_data) { if (scanline) {STBTT_free(scanline);} }
}

#define STBTT__COMPARE(a,b) ((a)->y0 < (b)->y0)
#define STBTT__SWAP(a,b)    { stbtt__edge t_=(a); (a)=(b); (b)=t_; }
static void stbtt__sort_edges_ins_sort(stbtt__edge *p, int n) {
    for (int i=1; i<n; ++i) {
        stbtt__edge t=p[i]; int j=i;
        while (j>0 && STBTT__COMPARE(&t,&p[j-1])) { p[j]=p[j-1]; --j; }
        p[j]=t;
    }
}

static void stbtt__sort_edges_quicksort(stbtt__edge *p, int n) {
    while (n>12) {
        int m=n>>1;
        int c01=STBTT__COMPARE(&p[0],&p[m]);
        int c12=STBTT__COMPARE(&p[m],&p[n-1]);
        if (c01!=c12) { int z=(STBTT__COMPARE(&p[0],&p[n-1])==c12)?0:n-1; STBTT__SWAP(p[z],p[m]); }
        STBTT__SWAP(p[0],p[m]);
        int i=1, j=n-1;
        for(;;) {
            while (STBTT__COMPARE(&p[i],&p[0])) ++i;
            while (STBTT__COMPARE(&p[0],&p[j])) --j;
            if (i>=j) break;
            STBTT__SWAP(p[i],p[j]); ++i; --j;
        }
        if (j<(n-i)) { stbtt__sort_edges_quicksort(p,j); p+=i; n-=i; }
        else          { stbtt__sort_edges_quicksort(p+i,n-i); n=j; }
    }
}

static void stbtt__sort_edges(stbtt__edge *p, int n) { stbtt__sort_edges_quicksort(p,n); stbtt__sort_edges_ins_sort(p,n); }
static void stbtt__rasterize(stbtt__bitmap *result, Vector2 *pts, int *wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert) {
   float y_scale_inv = invert ? -scale_y : scale_y;
   stbtt__edge *e;
   int n=0,i,j,k;
   for (i=0; i < windings; ++i) n += wcount[i];
   e = (stbtt__edge*)STBTT_malloc(sizeof(*e) * ((size_t)n+1)); // add an extra one as a sentinel
   if (e == 0) return;
   
   n = 0; int m=0;
   for (i=0; i < windings; ++i) {
      Vector2 *p = pts + m;
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
   if (e) STBTT_free(e);
}

static void stbtt__add_point(Vector2 *p, int n, float x, float y) { if (p) { p[n].x=x; p[n].y=y; } }
static int stbtt__tesselate_curve(Vector2 *pts, int *np, float x0, float y0, float x1, float y1, float x2, float y2, float flat_sq, int n) {
    float mx=(x0+2*x1+x2)/4, my=(y0+2*y1+y2)/4;
    float dx=(x0+x2)/2-mx,   dy=(y0+y2)/2-my;
    if (n>16 || dx*dx+dy*dy<=flat_sq) { stbtt__add_point(pts,(*np)++,x2,y2); return 1; }
    stbtt__tesselate_curve(pts,np, x0,y0,(x0+x1)/2,(y0+y1)/2,mx,my,flat_sq,n+1);
    stbtt__tesselate_curve(pts,np, mx,my,(x1+x2)/2,(y1+y2)/2,x2,y2,flat_sq,n+1);
    return 1;
}

static void stbtt__tesselate_cubic(Vector2 *pts, int *np, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float flat_sq, int n) {
    float dx0=x1-x0,dy0=y1-y0, dx1=x2-x1,dy1=y2-y1, dx2=x3-x2,dy2=y3-y2, dx=x3-x0,dy=y3-y0;
    float ll=vsqrtf(dx0*dx0+dy0*dy0)+vsqrtf(dx1*dx1+dy1*dy1)+vsqrtf(dx2*dx2+dy2*dy2), sl=vsqrtf(dx*dx+dy*dy);
    if (n>16 || ll*ll-sl*sl<=flat_sq) { stbtt__add_point(pts,(*np)++,x3,y3); return; }
    float x01=(x0+x1)/2,y01=(y0+y1)/2, x12=(x1+x2)/2,y12=(y1+y2)/2, x23=(x2+x3)/2,y23=(y2+y3)/2;
    float xa=(x01+x12)/2,ya=(y01+y12)/2, xb=(x12+x23)/2,yb=(y12+y23)/2, mx=(xa+xb)/2,my=(ya+yb)/2;
    stbtt__tesselate_cubic(pts,np, x0,y0,x01,y01,xa,ya,mx,my,flat_sq,n+1);
    stbtt__tesselate_cubic(pts,np, mx,my,xb,yb,x23,y23,x3,y3,flat_sq,n+1);
}

static Vector2 *stbtt_FlattenCurves(stbtt_vertex *v, int nv, float flatness, int **contour_lengths, int *num_contours) {
    float flat_sq=flatness*flatness;
    int n=0; for (int i=0;i<nv;++i) if (v[i].type==STBTT_vmove) ++n;
    *num_contours=n; if (!n) return 0;
    
    *contour_lengths=(int*)STBTT_malloc(sizeof(int)*(size_t)n);
    if (!*contour_lengths) { *num_contours=0; return 0; }

    Vector2 *points=0; int num_points=0;
    for (int pass=0; pass<2; ++pass) {
        float x=0,y=0; int start=0; n=-1;
        if (pass==1) { points=(Vector2*)STBTT_malloc((size_t)num_points*sizeof(Vector2)); if (!points) goto error; }
        num_points=0;
        for (int i=0; i<nv; ++i) {
            switch (v[i].type) {
            case STBTT_vmove: if (n>=0) {(*contour_lengths)[n]=num_points-start;} start=num_points; ++n; x=v[i].x; y=v[i].y; stbtt__add_point(points,num_points++,x,y); break;
            case STBTT_vline: x=v[i].x; y=v[i].y; stbtt__add_point(points,num_points++,x,y); break;
            case STBTT_vcurve: stbtt__tesselate_curve(points,&num_points,x,y,v[i].cx,v[i].cy,v[i].x,v[i].y,flat_sq,0); x=v[i].x; y=v[i].y; break;
            case STBTT_vcubic: stbtt__tesselate_cubic(points,&num_points,x,y,v[i].cx,v[i].cy,v[i].cx1,v[i].cy1,v[i].x,v[i].y,flat_sq,0); x=v[i].x; y=v[i].y; break;
            }
        }
        (*contour_lengths)[n]=num_points-start;
    }
    return points;
    error:
    STBTT_free(points); STBTT_free(*contour_lengths);
    *contour_lengths=0; *num_contours=0; return NULL;
}

extern void stbtt_Rasterize(stbtt__bitmap *result, float flatness_in_pixels, stbtt_vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert) {
   float scale            = scale_x > scale_y ? scale_y : scale_x;
   int winding_count      = 0;
   int *winding_lengths   = NULL;
   Vector2 *windings = stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count);
   if (windings) {
      stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert);
      if (winding_lengths) STBTT_free(winding_lengths);
      if (windings) STBTT_free(windings);
   }
}

extern void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph) {
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
   if (vertices) STBTT_free(vertices);
}

typedef int stbrp_coord;
typedef struct {int width,height; int x,y,bottom_y; } stbrp_context;
typedef struct { unsigned char x; } stbrp_node;
struct stbrp_rect { stbrp_coord x,y; int id,w,h,was_packed; };
static void stbrp_pack_rects(stbrp_context *con, stbrp_rect *rects, int num_rects) {
   int i;
   for (i=0; i < num_rects; ++i) {
      if (con->x + rects[i].w > con->width) { con->x = 0; con->y = con->bottom_y; }
      if (con->y + rects[i].h > con->height) break;
      
      rects[i].x = con->x;
      rects[i].y = con->y;
      rects[i].was_packed = 1;
      con->x += rects[i].w;
      if (con->y + rects[i].h > con->bottom_y) con->bottom_y = con->y + rects[i].h;
   }

   for (; i < num_rects; ++i) rects[i].was_packed = 0;
}

int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int pw, int ph, int stride, int padding, void *alloc) {
   stbrp_context *ctx = (stbrp_context*)STBTT_malloc(sizeof(*ctx));
   *ctx = (stbrp_context){pw-padding,ph-padding,0,0,0};
   if(pixels) __builtin_memset(pixels,0,(size_t)(pw*ph));
   return *spc = (stbtt_pack_context){alloc,ctx,pw,ph,stride?stride:pw,padding,0,1,1,pixels},1;
}


#define STBTT__OVER_MASK  (STBTT_MAX_OVERSAMPLE-1)
static void stbtt__h_prefilter(unsigned char *p, int w, int h, int stride, unsigned int kw) {
   for (int j = 0; j < h; ++j, p += stride) {
      unsigned char buf[STBTT_MAX_OVERSAMPLE] = {0}; int total=0;
      for (int i=0; i < w; ++i) {
         if (i <= w - (int)kw) {
            total += p[i] - buf[i & STBTT__OVER_MASK];
            buf[(i + kw) & STBTT__OVER_MASK] = p[i];
         } else total -= buf[i & STBTT__OVER_MASK];
         p[i] = (unsigned char)(total / kw);
      }
   }
}

static void stbtt__v_prefilter(unsigned char *p, int w, int h, int stride, unsigned int kw) {
   for (int j=0;j<w;++j,++p) {
      unsigned char buf[STBTT_MAX_OVERSAMPLE] = {0}; int total=0;
      for (int i=0;i<h;++i) {
         if (i <= h - (int)kw) {
            total += p[i*stride] - buf[i & STBTT__OVER_MASK];
            buf[(i+kw) & STBTT__OVER_MASK] = p[i*stride];
         } else total -= buf[i & STBTT__OVER_MASK];
         p[i*stride] = (unsigned char)(total / kw);
      }
   }
}

static float stbtt__oversample_shift(int oversample) { if (!oversample) {return 0.0f;} return (float)-(oversample - 1) / (2.0f * (float)oversample); }

// rects array must be big enough to accommodate all characters in the given ranges
extern int stbtt_PackFontRangesGatherRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects) {
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
            stbtt_GetGlyphBitmapBoxSubpixel(info,glyph,scale * (float)spc->h_oversample,scale * (float)spc->v_oversample,0,0,&x0,&y0,&x1,&y1);
            rects[k].w = (stbrp_coord) (x1-x0 + spc->padding + (int)spc->h_oversample-1);
            rects[k].h = (stbrp_coord) (y1-y0 + spc->padding + (int)spc->v_oversample-1);
            if (glyph == 0) missing_glyph_added = 1;
         }
         ++k;
      }
   }

   return k;
}

// rects array must be big enough to accommodate all characters in the given ranges
extern int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects) {
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
            bc->x0 = (unsigned short)r->x; bc->y0 = (unsigned short)r->y;
            bc->x1 = (unsigned short)(r->x + r->w); bc->y1 = (unsigned short)(r->y + r->h);
            bc->xadvance = scale * (float)advance;
            bc->xoff = (float)x0 * recip_h + sub_x; bc->yoff = (float)y0 * recip_v + sub_y;
            bc->xoff2 = (float)(x0 + r->w) * recip_h + sub_x; bc->yoff2 = (float)(y0 + r->h) * recip_v + sub_y;
            if (glyph == 0) missing_glyph = j;
         } else if (spc->skip_missing) {
            return_value = 0;
         } else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
            ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
         } else return_value = 0; // if any fail, report failure
         ++k;
      }
   }

   spc->h_oversample = (unsigned int)old_h_over; spc->v_oversample = (unsigned int)old_v_over; // restore original values
   return return_value;
}

extern int stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges) {
   stbtt_fontinfo info; int i,j,n = 0,return_value=1; stbrp_rect *rects;
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
   if (rects) STBTT_free(rects);
   return return_value;
}

extern int stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float sz, int frst, int num, stbtt_packedchar *d) { stbtt_pack_range range = (stbtt_pack_range){sz,frst,NULL,num,d,0,0}; return stbtt_PackFontRanges(spc,fontdata,font_index,&range,1); }
