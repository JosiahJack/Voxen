// text.c - Text and Font Rendering/Loading System
#include "common.h"
#include "lib.h"
extern u32 textVAO,textVBO,textSP; u32 fontAtlasTex, fontAtlasTexStopD; extern Color textColors[];
u8* textBinData = NULL; stbtt_packedchar* fontPackedChar = NULL; stbtt_packedchar* fontPackedCharStopD = NULL;
int numPackedGlyphs = 0, numPackedGlyphsStopD = 0; float fixedNumberAdvanceWidth = 0.0f, fixedNumberAdvanceWidthStopD = 0.0f;
static char uiTextBuffer[T_BUFFER_SIZE]; static float textVertexData[8192];
GlyphRange fontRanges[]     ={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
GlyphRange fontRangesStopD[]={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
i32 numFontRanges=sizeof(fontRanges)/sizeof(fontRanges[0]);
void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, u8* bmp);
void InitFontAtlasses() {
    DebugRAM("start font load");
    double t0=get_time();DualLog("Loading    5 fonts...");
    FHandle fd;
    int fileSize;
    textBinData = (u8*)OS_OpenAndAllocateFileBufferReadonly("text.bin",&fd,&fileSize);
    if (!textBinData || (size_t)fileSize < sizeof(TextBinHeader)) { DualLogError("Failed to load text.bin\n"); OS_Exit(1); }
    TextBinHeader* header = (TextBinHeader*)textBinData;
    if (header->magic != TEXT_BIN_MAGIC) { DualLogError("Invalid text.bin magic number\n"); OS_Exit(1); }
    numPackedGlyphs = header->numPackedGlyphs;
    numPackedGlyphsStopD = header->numPackedGlyphsStopD;
    fixedNumberAdvanceWidth = header->fixedNumberAdvanceWidth;
    fixedNumberAdvanceWidthStopD = header->fixedNumberAdvanceWidthStopD;
    u8* currentOffset = textBinData + sizeof(TextBinHeader);
    fontPackedChar = (stbtt_packedchar*)currentOffset;
    currentOffset += header->packedCharBytes;
    fontPackedCharStopD = (stbtt_packedchar*)currentOffset;
    currentOffset += header->packedCharStopDBytes;
    u8* primaryAtlasBmp = currentOffset;
    currentOffset += header->atlasBytes;
    u8* stopDAtlasBmp = currentOffset;
    GenerateAndBindTexture(&fontAtlasTex,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,primaryAtlasBmp);
    GenerateAndBindTexture(&fontAtlasTexStopD,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,stopDAtlasBmp);
    DebugRAM("after font load");
    glUseProgram(textSP);
    glUniform1i(1,2);
    DualLog(" took %f s\n",get_time()-t0);
}

i32 CodepointToPackedIndex(i32 cp,int fontID){ if(cp<32){cp=32;} if(cp>=447){cp=446;} const GlyphRange*ranges=(fontID==FONT_STOPD)?fontRangesStopD:fontRanges; i32 total=(fontID==FONT_STOPD)?numPackedGlyphsStopD:numPackedGlyphs; for(i32 i=0;i<numFontRanges;i++){if(cp>=ranges[i].first&&cp<ranges[i].first+ranges[i].count){i32 idx=ranges[i].startIndex+vmax((cp-ranges[i].first),0);if(idx<total){return idx;}}} return 0; }
size_t utf16le_to_utf8(const u8*src,size_t slen,char*dst,size_t dlen){
    size_t dp=0,sp=0;
    while(sp<slen&&dp<dlen-4){if(sp+1>=slen)break;u32 c=(u32)src[sp+1]<<8|src[sp];sp+=2;
        if(c<0x80){dst[dp++]=(char)c;}
        else if(c<0x800){dst[dp++]=(char)(0xC0|(c>>6));dst[dp++]=(char)(0x80|(c&0x3F));}
        else if(c<0x10000){dst[dp++]=(char)(0xE0|(c>>12));dst[dp++]=(char)(0x80|((c>>6)&0x3F));dst[dp++]=(char)(0x80|(c&0x3F));}
        else continue;}
    dst[dp]='\0';return dp;
}

static const char* localizations[8]={"./Data/text_english.txt","./Data/text_espanol.txt","./Data/text_deutsch.txt","./Data/text_francais.txt","./Data/text_nihongo.txt","./Data/text_russkiy.txt","./Data/text_italiano.txt","./Data/text_portugues.txt"};
void LoadTextForLanguage(u8 lang) {
    char tf[256]={0}; sCpy2aSubFromb(tf,255,localizations[lang<8?lang:0],256);
    FHandle dfd=INVALID_FHANDLE;int asz=0;
    if(Sys_Text.file_data){OS_Free(Sys_Text.file_data,Sys_Text.file_size);Sys_Text.file_data=NULL;Sys_Text.file_size=0;}
    Sys_Text.file_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.file_data||asz<=0){DualLogError("Failed to load text file: %s\n",tf);return;}
    Sys_Text.file_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){dp=3;}
    else{size_t nl=0;for(size_t i=1;i<Sys_Text.file_size&&i<1024;i+=2)if(Sys_Text.file_data[i]==0)nl++;if(nl*3>Sys_Text.file_size)utf16=1;}
    char line[T_LOGSTR_MAX];int ln=0;
    while(dp<Sys_Text.file_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.file_size){u16 ch=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.file_size){u16 nx=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.file_size){u8 c=Sys_Text.file_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.file_size&&Sys_Text.file_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls;if(ll==0){if(ln<T_LOGSTR_CNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(utf16)utf16le_to_utf8(&Sys_Text.file_data[ls],ll,line,sizeof(line));else{if(ll>=sizeof(line))ll=sizeof(line)-1; mcpy(line,&Sys_Text.file_data[ls],ll);line[ll]='\0';}
        size_t sl=slen(line);while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0';
        if(sl==0){if(ln<T_LOGSTR_CNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(ln<T_LOGSTR_CNT) {mcpy(Sys_Text.stringTable[ln],line,sl);Sys_Text.stringTable[ln][sl]='\0';++ln;} }
}

INLINE int s2i32Len(const char*str,size_t len){int v=0;for(size_t i=0;i<len&&str[i]>='0'&&str[i]<='9';++i)v=v*10+(str[i]-'0');return v;}
static const char* logLocalizations[8]={"./Data/logs_english.txt","./Data/logs_espanol.txt","./Data/logs_deutsch.txt","./Data/logs_francais.txt","./Data/logs_nihongo.txt","./Data/logs_russkiy.txt","./Data/logs_italiano.txt","./Data/logs_portugues.txt"};
void LoadLogTextForLanguage(u8 lang) {
    mset(Sys_Text.audioLogImagesRefIndicesLH,0,LOGCNT*sizeof(u16));mset(Sys_Text.audioLogImagesRefIndicesRH,0,LOGCNT*sizeof(u16));mset(Sys_Text.audioLogType,0,LOGCNT*sizeof(u8));mset(Sys_Text.audioLogLevelFound,0,LOGCNT*sizeof(u8));
    char tf[256]={0}; sCpy2aSubFromb(tf,255,logLocalizations[lang<8?lang:0],256);
    FHandle dfd=INVALID_FHANDLE;int asz=0;
    if(Sys_Text.filelog_data){OS_Free(Sys_Text.filelog_data,Sys_Text.filelog_size);Sys_Text.filelog_data=NULL;Sys_Text.filelog_size=0;}
    Sys_Text.filelog_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.filelog_data||asz<=0){DualLogError("Failed to load log text file: %s\n",tf);return;}
    Sys_Text.filelog_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.filelog_size>=2&&Sys_Text.filelog_data[0]==0xFF&&Sys_Text.filelog_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.filelog_size>=3&&Sys_Text.filelog_data[0]==0xEF&&Sys_Text.filelog_data[1]==0xBB&&Sys_Text.filelog_data[2]==0xBF){dp=3;}
    else{int nl=0;for(size_t i=1;i<Sys_Text.filelog_size&&i<2048;i+=2)if(Sys_Text.filelog_data[i]==0)nl++;if(nl>(int)(Sys_Text.filelog_size/5))utf16=1;}
    char line[1024];
    while(dp<Sys_Text.filelog_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.filelog_size){u16 ch=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.filelog_size){u16 nx=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.filelog_size){u8 c=Sys_Text.filelog_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.filelog_size&&Sys_Text.filelog_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls; if(!ll)continue;
        if(utf16)utf16le_to_utf8(&Sys_Text.filelog_data[ls],ll,line,sizeof(line)); else { if (ll>=sizeof(line)) ll=sizeof(line)-1; mcpy(line,&Sys_Text.filelog_data[ls],ll);line[ll]='\0'; }
        size_t sl=slen(line); while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0'; if(!sl)continue;
        int li=-1,ilh=-1,irh=-1,lt=0,lf=0,fi=0;char*pos=line;
        while(*pos&&fi<32){while(*pos==' ')++pos;char*st=pos;int q=(*pos=='"');if(q)++pos;while(*pos){if(*pos==','&&!q)break;if(*pos=='"'&&q){if(pos[1]==','){pos++;break;}if(pos[1]=='"'){pos+=2;continue;}}++pos;}char*en=pos;if(q&&*en=='"')--en;size_t tl=(size_t)(en-st);if(!tl){if(*pos==',')++pos;fi++;continue;}
            switch(fi){
                case 0:  li=s2i32Len(st,tl); if (li<0||li>=LOGCNT) goto nxt; break;
                case 1:  ilh=s2i32Len(st,tl); break;
                case 2:  irh=s2i32Len(st,tl); break;
                case 3:  if(li>=0&&li<LOGCNT) sCpy2aSubFromb(World.audiologNames[li],tl,st,sizeof(World.audiologNames[0]));       break;
                case 4:  if(li>=0&&li<LOGCNT) sCpy2aSubFromb(World.audiologSenders[li],tl,st,sizeof(World.audiologSenders[0]));   break;
                case 5:  if(li>=0&&li<LOGCNT) sCpy2aSubFromb(World.audiologSubjects[li],tl,st,sizeof(World.audiologSubjects[0])); break;
                case 6:  lt=s2i32Len(st,tl); break;
                case 7:  lf=s2i32Len(st,tl); break;
                default: if(li>=0&&li<LOGCNT){char*d=World.audioLogSpeech2Text[li]; size_t cur=slen(d); if(cur>0&&cur<T_LOGSTR_MAX*4-2){d[cur++]=',';d[cur]='\0';} size_t left=T_LOGSTR_MAX*4-cur-1; if(left>0){size_t cl=tl>left?left:tl;sCpy2aSubFromb(d+cur,cl,st,left+1);}}break;}
            if (*pos==',')++pos;fi++;}
        if (li>=0&&li<LOGCNT) {Sys_Text.audioLogImagesRefIndicesLH[li]=(u16)ilh;Sys_Text.audioLogImagesRefIndicesRH[li]=(u16)irh;Sys_Text.audioLogType[li]=(u8)lt;Sys_Text.audioLogLevelFound[li]=(u8)lf;}
        nxt:continue;}
}

static float textVertexData[8192]; extern Color textColors[];
void RenderFormattedText(i16 x,i16 y,u32 color,u8 fontID,float scale,const char* restrict format,...) {
    va_list args; __builtin_va_start(args,format); sFormatV(uiTextBuffer,T_BUFFER_SIZE,format,args); __builtin_va_end(args);
    glUseProgram(textSP); glEnable(GL_BLEND); glUniform4f(3,textColors[color].r,textColors[color].g,textColors[color].b,1.0f);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D,fontID==FONT_STOPD ? fontAtlasTexStopD : fontAtlasTex);
    float invatsz = 1.0f/(float)FONT_ATLAS_SIZE;
    glUniform2f(4,invatsz,invatsz); glUniform1ui(2,fontID); glBindVertexArray(textVAO);
    size_t vc=0; const char*p=uiTextBuffer; float xpos=x,ypos=y+(16*scale),ls=22*scale; int cc=0; float puv = 10.0f * invatsz, bw=2.0f;
    while(*p) {
        const u8*s=(const u8*)p; u32 cp=0;
        if (*s<0x80) { cp=*s++; }
        else if ((*s&0xE0)==0xC0) { cp=(*s&0x1F)<< 6; cp|=(s[1]&0x3F); s+=2; }
        else if ((*s&0xF0)==0xE0) { cp=(*s&0x0F)<<12; cp|=(s[1]&0x3F)<<6; cp|=(s[2]&0x3F); s+=3; }
        else if ((*s&0xF8)==0xF0) { cp=(*s&0x07)<<18; cp|=(s[1]&0x3F)<<12; cp|=(s[2]&0x3F)<<6; cp|=(s[3]&0x3F); s+=4; }
        else s++;
        p = (const char*)s; cc++; if (cp=='\n'||cc>120) { xpos=x; ypos+=ls; cc=0; continue; }
        int idx=CodepointToPackedIndex(cp,fontID);
        const stbtt_packedchar*b = ((fontID==FONT_STOPD) ? fontPackedCharStopD : fontPackedChar) + idx;
        float qx = vfloor((xpos + b->xoff) + 0.5f), qy = vfloor((ypos + b->yoff) + 0.5f);
        float qx0 = qx, qy0 = qy, qx1 = qx + b->xoff2 - b->xoff, qy1 = qy + b->yoff2 - b->yoff;
        float qs0 = b->x0 * invatsz, qt0 = b->y0 * invatsz, qs1 = b->x1 * invatsz, qt1 = b->y1 * invatsz;
        xpos += b->xadvance;
        float vx0 = qx0 * scale - bw, vy0 = qy0 * scale - bw, vx1 = qx1 * scale + bw, vy1 = qy1 * scale + bw;
        float s0 = qs0 - puv, t0 = qt0 - puv, s1 = qs1 + puv, t1 = qt1 + puv, z = 0.0f;
        float tv[30] = { vx0,vy0,z,s0,t0, vx1,vy1,z,s1,t1, vx1,vy0,z,s1,t0, vx0,vy0,z,s0,t0, vx0,vy1,z,s0,t1, vx1,vy1,z,s1,t1 };
        mcpy(textVertexData + vc * 30, tv, sizeof(tv)); vc++;
        if (cp >= '0' && cp <= '9' && fontID == FONT_STOPD)
            xpos = qx0 + fixedNumberAdvanceWidthStopD;
    }
    if (vc) { glBindBuffer(GL_ARRAY_BUFFER,textVBO); glBufferData(GL_ARRAY_BUFFER,vc*30*sizeof(float),textVertexData,GL_DYNAMIC_DRAW); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,vc*6); }
}
