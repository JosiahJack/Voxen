// data_fonts.c - Load Font Atlasses
#include "os.h"
#include "gl.h"
#include "voxen.h"
#define DUMP_FONT_BITMAPS
#include "stb_truetype.h"
int numPackedGlyphs = 0;
int numPackedGlyphsStopD = 0;
GLuint fontAtlasTex;
GLuint fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS];
stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth = 0.0f; // Global for fixed-width number spacing
float fixedNumberAdvanceWidthStopD = 0.0f;
static const char* const fallbackFontPaths[] = { "./Fonts/FreeSerifBold.ttf", "./Fonts/cambriab.ttf", "./Fonts/NotoSansCJK-Bold.ttc" };
static const char* const fontPaths[] = { "./Fonts/SystemShockText.ttf", "./Fonts/StopD.ttf" };
static stbtt_fontinfo fontInfo[5];
static unsigned char *fontData[5];
typedef struct { char *path; unsigned char *data; size_t size; stbtt_fontinfo info; } LoadedFont;
LoadedFont fallbackFonts[3]; 
typedef struct { int32_t first; int32_t count; int32_t startIndex; } GlyphRange;
GlyphRange fontRanges[] = {{0x0020,0x7E - 0x20+1,0},/*ASCII 94*/ {0x00A0,0xFF - 0xA0+1, 95},/*Latin-1 95*/ {0x0400,0x04FF - 0x0400+1,95+96},/*Cyrillic 255*/ {0x3040,0x30FF - 0x3040+1,95+96+256},/*Hiragana/Katakana 191*/};
GlyphRange fontRangesStopD[] = {{0x0020,0x7E - 0x20+1,0},/*ASCII*/ {0x00A0,0xFF - 0xA0+1,95},/*Latin-1*/ {0x0400,0x04FF - 0x0400+1,95+96},/*Cyrillic*/ {0x3040,0x30FF - 0x3040+1,95+96+256},/*Hiragana/Katakana*/};
int32_t numFontRanges = sizeof(fontRanges)/sizeof(fontRanges[0]);
__attribute__((pure)) int32_t CodepointToPackedIndex(int32_t codepoint, int fontID) {
    if (codepoint < 32) codepoint = 32;
    if (codepoint >= 447) codepoint = 446;
    const GlyphRange* ranges = (fontID == FONT_STOPD) ? fontRangesStopD : fontRanges;
    int32_t total_packed = (fontID == FONT_STOPD) ? numPackedGlyphsStopD : numPackedGlyphs;
    for (int32_t i = 0; i < numFontRanges; i++) {
        if (codepoint >= ranges[i].first && codepoint < ranges[i].first + ranges[i].count) {
            int32_t idx = ranges[i].startIndex + vmax((codepoint - ranges[i].first),0);
            if (idx < total_packed) return idx;
        }
    }
    
    return 0;
}

static LoadedFont LoadFallbackFont(const char *path, int fontInfoIdx, int collection_index) {
    OsFileHandle fd; int fontFileSize; fontData[fontInfoIdx] = OS_OpenAndAllocateFileBufferReadonly(path, &fd, &fontFileSize);
    int offset = stbtt_GetFontOffsetForIndex(fontData[fontInfoIdx], collection_index);
    if (offset < 0) { DualLogError("Invalid collection index %d for font %s\n", collection_index, path); OS_Exit(1); }
    if (!stbtt_InitFont_internal(&fontInfo[fontInfoIdx], fontData[fontInfoIdx], offset)) { DualLogError("Failed to init font at index %d in %s\n", collection_index, path); OS_Exit(1); }
    return (LoadedFont){(char*)path,fontData[fontInfoIdx],fontFileSize,fontInfo[fontInfoIdx]};
}

static int GetGlyphAndFont(uint32_t codepoint, stbtt_fontinfo **outFont, uint8_t fontID) {
    int glyph = stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &fontInfo[1] : &fontInfo[0]), codepoint);
    if (glyph) { *outFont = (fontID == FONT_STOPD ? &fontInfo[1] : &fontInfo[0]); return glyph; }

    for (int i = 0; i < 3; i++) {        
        glyph = stbtt_FindGlyphIndex(&fallbackFonts[i].info, codepoint);
        if (glyph) { *outFont = &fallbackFonts[i].info; return glyph; }
    }

    return 0;
}

#ifdef DUMP_FONT_BITMAPS
    void stbi_write_bmp(char const *filename, int x, int y, const void *data);
    static void dump_atlas_bmp(const char *bmp_path, const unsigned char *atlas_data) {
        unsigned char *rgb = OS_AllocateRAM(NULL,(size_t)FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * 4,PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, OS_INVALID_HANDLE);
        for (int i = 0; i < FONT_ATLAS_SIZE * FONT_ATLAS_SIZE; ++i) {
            unsigned char v = atlas_data[i];
            rgb[i*4 + 0] = v;  // B
            rgb[i*4 + 1] = v;  // G
            rgb[i*4 + 2] = v;  // R
            rgb[i*4 + 3] = 255;// A
        }

        stbi_write_bmp(bmp_path, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, rgb);
        OS_DeallocateRAM(rgb, (size_t)FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * 4);
    }
#endif

void InitFontAtlasses(void) {
    double t0 = get_time();
    DualLog("Loading    5 fonts...");
    OsFileHandle fd1; int sz1; fontData[0] = OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    OsFileHandle fd2; int sz2; fontData[1] = OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if (!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)) { DualLogError("%s font init failed\n",fontPaths[0]); OS_Exit(1); }
    if (!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)) { DualLogError("%s font init failed\n",fontPaths[1]); OS_Exit(1); }

    fallbackFonts[0] = LoadFallbackFont(fallbackFontPaths[0],2,0); // Preload known fallback fonts for Cyrillic, Kanji, etc.
    fallbackFonts[1] = LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2] = LoadFallbackFont(fallbackFontPaths[2],4,2); // Index 2 for Japanese in NotoSansCJK-Bold.ttc

    // Primary
    unsigned char *bmp = OS_AllocateRAM(NULL,FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char),PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,OS_INVALID_HANDLE);
    stbtt_pack_context pc; stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);
    pc.h_oversample = 4; pc.v_oversample = 4; pc.skip_missing = 1; numPackedGlyphs = 0; float h = 20.0f;
    for (int r=0;r<numFontRanges;++r) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i=0;i<fontRanges[r].count;++i) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;
            
            uint32_t cp = fontRanges[r].first + i;
            int g = stbtt_FindGlyphIndex(&fontInfo[0], cp);
            stbtt_fontinfo *font = &fontInfo[0];
            unsigned char *data = fontData[0];
            if (!g) {
                g = GetGlyphAndFont(cp, &font, FONT_NORMAL);
                if (!g) continue;
                
                
                data = (font == &fontInfo[0]) ? fontData[0] : ((LoadedFont*)((char*)font - __builtin_offsetof(LoadedFont, info)))->data;
            }
            
            float height = h;
            if (font != &fontInfo[0]) height *= 1.2f;
            stbtt_PackFontRange(&pc,data,0,height,cp,1,&fontPackedChar[numPackedGlyphs]);
            int idx = numPackedGlyphs++;
            if (cp >= '0' && cp <= '9') fixedNumberAdvanceWidth = vmax(fixedNumberAdvanceWidth, fontPackedChar[idx].xadvance);
        }
    }
    
    free(pc.pack_info);
    glCreateTextures(GL_TEXTURE_2D,1,&fontAtlasTex);
    glTextureStorage2D(fontAtlasTex,1,GL_R8,FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTex,0,0,0,FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    #ifdef DUMP_FONT_BITMAPS
        dump_atlas_bmp("./Fonts/SystemShockText_atlas.bmp", bmp);
    #endif
        
    // Secondary
    __builtin_memset(bmp,0,FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char));
    stbtt_pack_context pc2; stbtt_PackBegin(&pc2,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);
    pc2.h_oversample = 4; pc2.v_oversample = 4; pc2.skip_missing = 1; numPackedGlyphsStopD = 0; float h2 = 54.0f;
    for (int r=0;r<numFontRanges;++r) {
        fontRangesStopD[r].startIndex = numPackedGlyphsStopD;
        for (int i = 0; i < fontRangesStopD[r].count; i++) {
            if (numPackedGlyphsStopD >= MAX_GLYPHS) break;
            uint32_t cp = fontRangesStopD[r].first + i;
            int g = stbtt_FindGlyphIndex(&fontInfo[1], cp);
            stbtt_fontinfo *font = &fontInfo[1];
            unsigned char *data = fontData[1];
            if (!g) {
                g = GetGlyphAndFont(cp, &font, FONT_STOPD);
                if (!g) continue;
                
                data = (font == &fontInfo[0]) ? fontData[0] : ((LoadedFont*)((char*)font - __builtin_offsetof(LoadedFont, info)))->data;
            }
            
            float height = h2;
            if (font != &fontInfo[1]) height *= 1.2f;
            stbtt_PackFontRange(&pc2, data, 0, height, cp, 1, &fontPackedCharStopD[numPackedGlyphsStopD]);
            int idx = numPackedGlyphsStopD++;
            if (cp >= '0' && cp <= '9') fixedNumberAdvanceWidthStopD = vmax(fixedNumberAdvanceWidthStopD, fontPackedCharStopD[idx].xadvance);
        }
    }
    
    free(pc2.pack_info);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTexStopD);
    glTextureStorage2D(fontAtlasTexStopD, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTexStopD, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    #ifdef DUMP_FONT_BITMAPS
        dump_atlas_bmp("./Fonts/StopD_atlas.bmp", bmp);
    #endif
    OS_DeallocateRAM(bmp,FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char));
    DualLog(" took %f s\n", get_time() - t0);
}

char *strncpy(char *dest, const char *src, size_t n);
TextSystem Sys_Text = { .file_data = NULL };
uint16_t logImages = 1272; // Start index of first index 0 logImages[0] is blank1.png
size_t utf16le_to_utf8(const uint8_t* src, size_t src_len, char* dst, size_t dst_len) {
    size_t dst_pos = 0; size_t src_pos = 0;
    while (src_pos < src_len && dst_pos < dst_len - 4) {
        if (src_pos + 1 >= src_len) break;
        uint32_t code = (uint32_t)src[src_pos + 1] << 8 | src[src_pos]; src_pos += 2;
        if (code < 0x80) {
            dst[dst_pos++] = (char)code;
        } else if (code < 0x800) {
            dst[dst_pos++] = (char)(0xC0 | (code >> 6));
            dst[dst_pos++] = (char)(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            dst[dst_pos++] = (char)(0xE0 | (code >> 12));
            dst[dst_pos++] = (char)(0x80 | ((code >> 6) & 0x3F));
            dst[dst_pos++] = (char)(0x80 | (code & 0x3F));
        } else continue; // Skip surrogates
    }
    dst[dst_pos] = '\0'; return dst_pos;
}

const char* localizations[8] = {"./Data/text_english.txt","./Data/text_espanol.txt","./Data/text_deutsch.txt","./Data/text_francais.txt",
                                "./Data/text_nihongo.txt","./Data/text_russkiy.txt","./Data/text_italiano.txt","./Data/text_portugues.txt"};
void LoadTextForLanguage(uint8_t lang) {
    char textFile[256] = {0};
    const char* filename = localizations[lang < 8 ? lang : 0];
    strncpy(textFile, filename, sizeof(textFile) - 1);
    textFile[sizeof(textFile) - 1] = '\0';
    OsFileHandle dummy_fd = OS_INVALID_HANDLE;
    int alloc_size = 0;
    if (Sys_Text.file_data) { // Free previous allocation if any (important when changing language)
        OS_DeallocateRAM(Sys_Text.file_data,Sys_Text.file_size);
        Sys_Text.file_data = NULL;
        Sys_Text.file_size = 0;
    }

    Sys_Text.file_data = (uint8_t*)OS_OpenAndAllocateFileBufferReadonly(textFile,&dummy_fd,&alloc_size);
    if (!Sys_Text.file_data || alloc_size <= 0) { DualLogError("Failed to load text file: %s\n", textFile); return; }

    Sys_Text.file_size = (size_t)alloc_size;
    size_t data_pos=0;int is_utf16le=0;
    if(Sys_Text.file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){data_pos=2;is_utf16le=1;}
    else if(Sys_Text.file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){data_pos=3;}
    else{size_t nulls=0;for(size_t i=1;i<Sys_Text.file_size&&i<1024;i+=2)if(Sys_Text.file_data[i]==0)nulls++;if(nulls*3>Sys_Text.file_size)is_utf16le=1;}
    char utf8_line[TEXT_LOCALIZATION_MAX_LENGTH];int lineNum=0;
    while(data_pos<Sys_Text.file_size){
        size_t line_start=data_pos;
        if(is_utf16le){
            while(data_pos+1<Sys_Text.file_size){
                uint16_t ch=Sys_Text.file_data[data_pos]|(Sys_Text.file_data[data_pos+1]<<8);data_pos+=2;
                if(ch=='\r'||ch=='\n'){if(ch=='\r'&&data_pos+1<Sys_Text.file_size){uint16_t n=Sys_Text.file_data[data_pos]|(Sys_Text.file_data[data_pos+1]<<8);if(n=='\n')data_pos+=2;}break;}
            }
        }else{
            while(data_pos<Sys_Text.file_size){
                uint8_t c=Sys_Text.file_data[data_pos];
                if(c=='\r'||c=='\n'){if(c=='\r'&&data_pos+1<Sys_Text.file_size&&Sys_Text.file_data[data_pos+1]=='\n')++data_pos;++data_pos;break;}
                ++data_pos;
            }
        }
        size_t len=data_pos-line_start;if(len==0){if(lineNum<TEXT_STRING_COUNT)Sys_Text.stringTable[lineNum][0]='\0';++lineNum;continue;}
        if(is_utf16le){utf16le_to_utf8(&Sys_Text.file_data[line_start],len,utf8_line,sizeof(utf8_line));}
        else{if(len>=sizeof(utf8_line))len=sizeof(utf8_line)-1;__builtin_memcpy(utf8_line,&Sys_Text.file_data[line_start],len);utf8_line[len]='\0';}
        len=GetStringLength(utf8_line);while(len>0&&(utf8_line[len-1]=='\r'||utf8_line[len-1]=='\n'))utf8_line[--len]='\0';
        if(len==0){if(lineNum<TEXT_STRING_COUNT)Sys_Text.stringTable[lineNum][0]='\0';++lineNum;continue;}
        if(lineNum<TEXT_STRING_COUNT){
            __builtin_memcpy(Sys_Text.stringTable[lineNum],utf8_line,len);
            Sys_Text.stringTable[lineNum][len]='\0';++lineNum;
        }
    }
}

static inline __attribute__((always_inline)) int StringToIntLen(const char *str, size_t len) {
    int value = 0;
    size_t i = 0;
    while (i < len) {
        char c = str[i];
        if (c < '0' || c > '9') break;

        value = value * 10 + (c - '0'); ++i;
    }
    
    return value;
}

const char* logLocalizations[8] = {"./Data/logs_text_english.txt","./Data/logs_text_espanol.txt","./Data/logs_text_deutsch.txt","./Data/logs_text_francais.txt",
                                  "./Data/logs_text_nihongo.txt","./Data/logs_text_russkiy.txt","./Data/logs_text_italiano.txt","./Data/logs_text_portugues.txt"};
void LoadLogTextForLanguage(uint8_t lang) {
    __builtin_memset(Sys_Text.audioLogImagesRefIndicesLH, 0, TEXT_LOGS_COUNT * sizeof(uint16_t));
    __builtin_memset(Sys_Text.audioLogImagesRefIndicesRH, 0, TEXT_LOGS_COUNT * sizeof(uint16_t));
    __builtin_memset(Sys_Text.audioLogType,               0, TEXT_LOGS_COUNT * sizeof(uint8_t));
    __builtin_memset(Sys_Text.audioLogLevelFound,         0, TEXT_LOGS_COUNT * sizeof(uint8_t));
    char textFile[256] = {0};
    const char* filename = logLocalizations[lang < 8 ? lang : 0];
    strncpy(textFile, filename, sizeof(textFile) - 1);
    textFile[sizeof(textFile) - 1] = '\0';
    OsFileHandle dummy_fd = OS_INVALID_HANDLE;
    int alloc_size = 0;
    if (Sys_Text.filelog_data) { // Free previous allocation if any
        OS_DeallocateRAM(Sys_Text.filelog_data,Sys_Text.filelog_size);
        Sys_Text.filelog_data = NULL;
        Sys_Text.filelog_size = 0;
    }

    Sys_Text.filelog_data = (uint8_t*) OS_OpenAndAllocateFileBufferReadonly(textFile,&dummy_fd,&alloc_size);
    if (!Sys_Text.filelog_data || alloc_size <= 0) { DualLogError("Failed to load log text file: %s\n", textFile); return; }

    Sys_Text.filelog_size = (size_t)alloc_size;
    size_t data_pos=0;int is_utf16le=0;
    if(Sys_Text.filelog_size>=2&&Sys_Text.filelog_data[0]==0xFF&&Sys_Text.filelog_data[1]==0xFE){data_pos=2;is_utf16le=1;}
    else if(Sys_Text.filelog_size>=3&&Sys_Text.filelog_data[0]==0xEF&&Sys_Text.filelog_data[1]==0xBB&&Sys_Text.filelog_data[2]==0xBF){data_pos=3;}
    else{int nulls=0;for(size_t i=1;i<(size_t)Sys_Text.filelog_size&&i<2048;i+=2)if(Sys_Text.filelog_data[i]==0)nulls++;if(nulls>(int)(Sys_Text.filelog_size/5))is_utf16le=1;}
    char utf8_line[1024];
    while(data_pos<(size_t)Sys_Text.filelog_size){
        size_t line_start=data_pos;
        if(is_utf16le){
            while(data_pos+1<(size_t)Sys_Text.filelog_size){
                uint16_t ch=Sys_Text.filelog_data[data_pos]|(Sys_Text.filelog_data[data_pos+1]<<8);data_pos+=2;
                if(ch=='\r'||ch=='\n'){if(ch=='\r'&&data_pos+1<(size_t)Sys_Text.filelog_size){uint16_t next=Sys_Text.filelog_data[data_pos]|(Sys_Text.filelog_data[data_pos+1]<<8);if(next=='\n')data_pos+=2;}break;}
            }
        }else{
            while(data_pos<(size_t)Sys_Text.filelog_size){
                uint8_t c=Sys_Text.filelog_data[data_pos];
                if(c=='\r'||c=='\n'){if(c=='\r'&&data_pos+1<(size_t)Sys_Text.filelog_size&&Sys_Text.filelog_data[data_pos+1]=='\n')++data_pos;++data_pos;break;}
                ++data_pos;
            }
        }
        size_t line_len=data_pos-line_start;if(line_len==0)continue;
        if(is_utf16le){utf16le_to_utf8(&Sys_Text.filelog_data[line_start],line_len,utf8_line,sizeof(utf8_line));}
        else{if(line_len>=sizeof(utf8_line))line_len=sizeof(utf8_line)-1;__builtin_memcpy(utf8_line,&Sys_Text.filelog_data[line_start],line_len);utf8_line[line_len]='\0';}
        size_t slen=GetStringLength(utf8_line);while(slen>0&&(utf8_line[slen-1]=='\r'||utf8_line[slen-1]=='\n'))utf8_line[--slen]='\0';if(slen==0)continue;
        int log_index=-1,img_lh=-1,img_rh=-1,log_type=0,level_found=0;char*pos=utf8_line;int field_idx=0;
        while(*pos&&field_idx<32){
            while(*pos==' ')++pos;char*start=pos;int quoted=(*pos=='"');if(quoted)++pos;
            while(*pos){if(*pos==','&&!quoted)break;if(*pos=='"'&&quoted){if(pos[1]==','){pos++;break;}if(pos[1]=='"'){pos+=2;continue;}}++pos;}
            char*end=pos;if(quoted&&*end=='"')--end;
            size_t tok_len=(size_t)(end-start);if(tok_len==0){if(*pos==',')++pos;field_idx++;continue;}
            switch(field_idx){
                case 0:log_index=StringToIntLen(start,tok_len);if(log_index<0||log_index>=TEXT_LOGS_COUNT)goto next_line;break;
                case 1:img_lh=StringToIntLen(start,tok_len);break;
                case 2:img_rh=StringToIntLen(start,tok_len);break;
                case 3:if(log_index>=0&&log_index<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologNames[log_index],tok_len,start,sizeof(Sys_Global.audiologNames[0]));break;
                case 4:if(log_index>=0&&log_index<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologSenders[log_index],tok_len,start,sizeof(Sys_Global.audiologSenders[0]));break;
                case 5:if(log_index>=0&&log_index<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologSubjects[log_index],tok_len,start,sizeof(Sys_Global.audiologSubjects[0]));break;
                case 6:log_type=StringToIntLen(start,tok_len);break;
                case 7:level_found=StringToIntLen(start,tok_len);break;
                default:if(log_index>=0&&log_index<TEXT_LOGS_COUNT){char*dst=Sys_Global.audioLogSpeech2Text[log_index];size_t cur=GetStringLength(dst);if(cur>0&&cur<TEXT_LOCALIZATION_MAX_LENGTH*4-2){dst[cur++]=',';dst[cur]='\0';}size_t left=TEXT_LOCALIZATION_MAX_LENGTH*4-cur-1;if(left>0){size_t cl=tok_len;if(cl>left)cl=left;StringCopyInto_A_SubstringFrom_B(dst+cur,cl,start,left+1);}}break;
            }
            
            if(*pos==',') ++pos;
            field_idx++;
        }
        
        if(log_index>=0&&log_index<TEXT_LOGS_COUNT){
            Sys_Text.audioLogImagesRefIndicesLH[log_index]=(uint16_t)img_lh;
            Sys_Text.audioLogImagesRefIndicesRH[log_index]=(uint16_t)img_rh;
            Sys_Text.audioLogType[log_index]=(uint8_t)log_type;
            Sys_Text.audioLogLevelFound[log_index]=(uint8_t)level_found;
        }
        
    next_line:continue;
    }
}
