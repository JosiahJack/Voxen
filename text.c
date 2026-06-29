// text.c - Text and Font Rendering/Loading System
#include "stbtt.h"
#define T_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
int numPackedGlyphs=0,numPackedGlyphsStopD=0;
u32 fontAtlasTex,fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS],fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth=0.0f,fixedNumberAdvanceWidthStopD=0.0f;
static const char* fallbackFontPaths[]={"./Fonts/FreeSerifBold.ttf","./Fonts/cambriab.ttf","./Fonts/NotoSansCJK-Bold.ttc"}, *fontPaths[]={"./Fonts/SystemShockText.ttf","./Fonts/StopD.ttf"};
static stbtt_fontinfo fontInfo[5]; static u8 *fontData[5]; static char uiTextBuffer[T_BUFFER_SIZE];
typedef struct{char*path;u8*data;size_t size;stbtt_fontinfo info;}LoadedFont;
LoadedFont fallbackFonts[3];
typedef struct{i32 first,count,startIndex;}GlyphRange;
GlyphRange fontRanges[]     ={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
GlyphRange fontRangesStopD[]={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
i32 numFontRanges=sizeof(fontRanges)/sizeof(fontRanges[0]);
i32 CodepointToPackedIndex(i32 cp,int fontID){ if(cp<32){cp=32;} if(cp>=447){cp=446;} const GlyphRange*ranges=(fontID==FONT_STOPD)?fontRangesStopD:fontRanges; i32 total=(fontID==FONT_STOPD)?numPackedGlyphsStopD:numPackedGlyphs; for(i32 i=0;i<numFontRanges;i++){if(cp>=ranges[i].first&&cp<ranges[i].first+ranges[i].count){i32 idx=ranges[i].startIndex+vmax((cp-ranges[i].first),0);if(idx<total){return idx;}}} return 0; }
LoadedFont LoadFallbackFont(const char*path,int fii,int ci){
    FHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
    int off=stbtt_GetFontOffsetForIndex(fontData[fii],ci);if(off<0){DualLogError("Invalid collection index %d for font %s\n",ci,path);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[fii],fontData[fii],off)){DualLogError("Failed to init font at index %d in %s\n",ci,path);OS_Exit(1);}
    return (LoadedFont){(char*)path,fontData[fii],fsz,fontInfo[fii]};
}
int GetGlyphAndFont(u32 cp,stbtt_fontinfo**outFont,u8 fontID){ int g=stbtt_FindGlyphIndex(fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0],cp);if(g){*outFont=fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0];return g;} for(int i=0;i<3;i++){g=stbtt_FindGlyphIndex(&fallbackFonts[i].info,cp);if(g){*outFont=&fallbackFonts[i].info;return g;}} return 0; }
void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, u8* bmp);
void InitFontAtlasses(){
    DebugRAM("start font load");
    double t0=get_time();DualLog("Loading    5 fonts...");
    ttAllocs = OS_Alloc(4474 * sizeof(TAlloc));
    FHandle fd1,fd2;int sz1,sz2;
    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if(!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)){DualLogError("%s font init failed\n",fontPaths[0]);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)){DualLogError("%s font init failed\n",fontPaths[1]);OS_Exit(1);}
    fallbackFonts[0]=LoadFallbackFont(fallbackFontPaths[0],2,0);
    fallbackFonts[1]=LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2]=LoadFallbackFont(fallbackFontPaths[2],4,2);
    u8*bmp=OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Primary atlas
    stbtt_pack_context pc;stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc.h_oversample=3;pc.v_oversample=3;pc.skip_missing=1;numPackedGlyphs=0;
    for(int r=0;r<numFontRanges;++r){fontRanges[r].startIndex=numPackedGlyphs;
        for(int i=0;i<fontRanges[r].count;++i){if(numPackedGlyphs>=MAX_GLYPHS)break;u32 cp=fontRanges[r].first+i;stbtt_fontinfo*font=&fontInfo[0];u8*data=fontData[0];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_NORMAL);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=20.0f;if(font!=&fontInfo[0])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedChar[numPackedGlyphs],0,0};stbtt_PackFontRanges(&pc,data,0,&range,1);
            int idx=numPackedGlyphs++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidth=vmax(fixedNumberAdvanceWidth,fontPackedChar[idx].xadvance);
        }
    }
    TempFree(pc.pack_info);GenerateAndBindTexture(&fontAtlasTex,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,bmp);
    mset(bmp,0,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Secondary atlas
    stbtt_pack_context pc2;stbtt_PackBegin(&pc2,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc2.h_oversample=3;pc2.v_oversample=3;pc2.skip_missing=1;numPackedGlyphsStopD=0;
    for(int r=0;r<numFontRanges;++r){fontRangesStopD[r].startIndex=numPackedGlyphsStopD;
        for(int i=0;i<fontRangesStopD[r].count;++i){if(numPackedGlyphsStopD>=MAX_GLYPHS)break;u32 cp=fontRangesStopD[r].first+i;stbtt_fontinfo*font=&fontInfo[1];u8*data=fontData[1];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_STOPD);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=54.0f;if(font!=&fontInfo[1])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedCharStopD[numPackedGlyphsStopD],0,0};stbtt_PackFontRanges(&pc2,data,0,&range,1);
            int idx=numPackedGlyphsStopD++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidthStopD=vmax(fixedNumberAdvanceWidthStopD,fontPackedCharStopD[idx].xadvance);
        }
    }
    TempFree(pc2.pack_info);GenerateAndBindTexture(&fontAtlasTexStopD,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,bmp);
    OS_Free(bmp,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    OS_Free(fontData[0],sz1);
    OS_Free(fontData[1],sz2);
    OS_Free(fontData[2],fallbackFonts[0].size);
    OS_Free(fontData[3],fallbackFonts[1].size);
    OS_Free(fontData[4],fallbackFonts[2].size);
    OS_Free(ttAllocs,4474 * sizeof(TAlloc));
    DebugRAM("after font load");
    glUseProgram(textSP); glUniform1i(1,2);
    DualLog(" took %f s\n",get_time()-t0);
}

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
void LoadTextForLanguage(u8 lang){
    char tf[256]={0}; sCpy2aSubFromb(tf,255,localizations[lang<8?lang:0],256);
    FHandle dfd=INVALID_FHANDLE;int asz=0;
    if(Sys_Text.file_data){OS_Free(Sys_Text.file_data,Sys_Text.file_size);Sys_Text.file_data=NULL;Sys_Text.file_size=0;}
    Sys_Text.file_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.file_data||asz<=0){DualLogError("Failed to load text file: %s\n",tf);return;}
    Sys_Text.file_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){dp=3;}
    else{size_t nl=0;for(size_t i=1;i<Sys_Text.file_size&&i<1024;i+=2)if(Sys_Text.file_data[i]==0)nl++;if(nl*3>Sys_Text.file_size)utf16=1;}
    char line[T_LOCALIZATION_MAX_LENGTH];int ln=0;
    while(dp<Sys_Text.file_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.file_size){u16 ch=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.file_size){u16 nx=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.file_size){u8 c=Sys_Text.file_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.file_size&&Sys_Text.file_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls;if(ll==0){if(ln<T_STRING_COUNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(utf16)utf16le_to_utf8(&Sys_Text.file_data[ls],ll,line,sizeof(line));else{if(ll>=sizeof(line))ll=sizeof(line)-1; mcpy(line,&Sys_Text.file_data[ls],ll);line[ll]='\0';}
        size_t sl=slen(line);while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0';
        if(sl==0){if(ln<T_STRING_COUNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(ln<T_STRING_COUNT) {mcpy(Sys_Text.stringTable[ln],line,sl);Sys_Text.stringTable[ln][sl]='\0';++ln;} }
}

INLINE int s2i32Len(const char*str,size_t len){int v=0;for(size_t i=0;i<len&&str[i]>='0'&&str[i]<='9';++i)v=v*10+(str[i]-'0');return v;}
static const char* logLocalizations[8]={"./Data/logs_english.txt","./Data/logs_espanol.txt","./Data/logs_deutsch.txt","./Data/logs_francais.txt","./Data/logs_nihongo.txt","./Data/logs_russkiy.txt","./Data/logs_italiano.txt","./Data/logs_portugues.txt"};
void LoadLogTextForLanguage(u8 lang){
    mset(Sys_Text.audioLogImagesRefIndicesLH,0,T_LOGS_COUNT*sizeof(u16));mset(Sys_Text.audioLogImagesRefIndicesRH,0,T_LOGS_COUNT*sizeof(u16));mset(Sys_Text.audioLogType,0,T_LOGS_COUNT*sizeof(u8));mset(Sys_Text.audioLogLevelFound,0,T_LOGS_COUNT*sizeof(u8));
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
                case 0:  li=s2i32Len(st,tl); if (li<0||li>=T_LOGS_COUNT) goto nxt; break;
                case 1:  ilh=s2i32Len(st,tl); break;
                case 2:  irh=s2i32Len(st,tl); break;
                case 3:  if(li>=0&&li<T_LOGS_COUNT) sCpy2aSubFromb(World.audiologNames[li],tl,st,sizeof(World.audiologNames[0]));       break;
                case 4:  if(li>=0&&li<T_LOGS_COUNT) sCpy2aSubFromb(World.audiologSenders[li],tl,st,sizeof(World.audiologSenders[0]));   break;
                case 5:  if(li>=0&&li<T_LOGS_COUNT) sCpy2aSubFromb(World.audiologSubjects[li],tl,st,sizeof(World.audiologSubjects[0])); break;
                case 6:  lt=s2i32Len(st,tl); break;
                case 7:  lf=s2i32Len(st,tl); break;
                default: if(li>=0&&li<T_LOGS_COUNT){char*d=World.audioLogSpeech2Text[li]; size_t cur=slen(d); if(cur>0&&cur<T_LOCALIZATION_MAX_LENGTH*4-2){d[cur++]=',';d[cur]='\0';} size_t left=T_LOCALIZATION_MAX_LENGTH*4-cur-1; if(left>0){size_t cl=tl>left?left:tl;sCpy2aSubFromb(d+cur,cl,st,left+1);}}break;}
            if (*pos==',')++pos;fi++;}
        if (li>=0&&li<T_LOGS_COUNT) {Sys_Text.audioLogImagesRefIndicesLH[li]=(u16)ilh;Sys_Text.audioLogImagesRefIndicesRH[li]=(u16)irh;Sys_Text.audioLogType[li]=(u8)lt;Sys_Text.audioLogLevelFound[li]=(u8)lf;}
        nxt:continue;}
}

Color textColors[] = {{1.0f,1.0f,1.0f,1.0f},/* 0 White T_WHITE*/ {0.890196078f,0.874509804f,0.0f,1.0f},/* 1 Yellow T_YELLOW*/  {0.623529412f,0.611764706f,0.0f,1.0f},/* 2 Dark Yellow (Yellow * 0.7f) T_DARK_YELLOW*/ {0.372549020f,0.654901961f,0.168627451f,1.0f},/* 3 Green T_GREEN*/ {0.917647059f,0.137254902f,0.168627451f,1.0f},/* 4 Red T_RED*/
                      {1.0f,0.498039216f,0.0f,1.0f}, /* 5 Orange T_ORANGE*/ {0.674509804f,0.058823529f,0.070588235f,1.0f},/* 6 StopD Red T_STOPD_RED*/ {0.941176471f,0.282352941f,0.298039216f,1.0f},/* 7 StopD Red Highlight T_STOPD_RED_HIGHLIGHT*/ {0.909803922f,0.203921569f,0.219607843f,1.0f}, /* 8 StopD Red Pause Title T_STOPD_RED_PAUSETITLE*/
                      {0.470588235f,0.721568627f,0.172549020f,1.0f},/* 9 Green Menu Title T_GREEN_MENU*/ {0.137254902f,0.356862745f,0.109803922f,1.0f},/* 10 Green Menu Title Shadow T_GREEN_MENU_SHADOW*/ {0.239215686f,0.466666667f,0.129411765f,1.0f}, /* 11 Green Menu Title Glow T_GREEN_MENU_GLOW*/ {0.392156863f,0.031372549f,0.039215686f,1.0f} /* 12 Red Menu Text Dark T_RED_MENU*/ };
float textVertexData[8192];
void RenderFormattedText(i16 x,i16 y,u32 color,u8 fontID,float scale,const char* restrict format,...) {
    va_list args; __builtin_va_start(args,format); sFormatV(uiTextBuffer,T_BUFFER_SIZE,format,args); __builtin_va_end(args);
    glUseProgram(textSP); glEnable(GL_BLEND); glUniform4f(3,textColors[color].r,textColors[color].g,textColors[color].b,1.0f);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D,fontID==FONT_STOPD ? fontAtlasTexStopD : fontAtlasTex);
    float invatsz = 1.0f/(float)FONT_ATLAS_SIZE;
    glUniform2f(4,invatsz,invatsz); glUniform1ui(2,fontID); glBindVertexArray(textVAO);
    size_t vc=0; const char*p=uiTextBuffer; float xpos=x,ypos=y+(16*scale),ls=22*scale; aligned_quad q; int cc=0; float puv = 10.0f * invatsz, bw=2.0f;
    while(*p) {
        const u8*s=(const u8*)p; u32 cp=0;
        if (*s<0x80) { cp=*s++; }
        else if ((*s&0xE0)==0xC0) { cp=(*s&0x1F)<< 6; cp|=(s[1]&0x3F); s+=2; }
        else if ((*s&0xF0)==0xE0) { cp=(*s&0x0F)<<12; cp|=(s[1]&0x3F)<<6;  cp|=(s[2]&0x3F); s+=3; }
        else if ((*s&0xF8)==0xF0) { cp=(*s&0x07)<<18; cp|=(s[1]&0x3F)<<12; cp|=(s[2]&0x3F)<<6; cp|=(s[3]&0x3F); s+=4; }
        else s++;

        p = (const char*)s; cc++; if (cp=='\n'||cc>120) { xpos=x; ypos+=ls; cc=0; continue; }

        int idx=CodepointToPackedIndex(cp,fontID);
        stbtt_GetPackedQuad((fontID==FONT_STOPD) ? fontPackedCharStopD : fontPackedChar,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,idx,&xpos,&ypos,&q,1);
        float vx0=q.x0*scale-bw,vy0=q.y0*scale-bw,vx1=q.x1*scale+bw,vy1=q.y1*scale+bw; float s0=q.s0-puv,t0=q.t0-puv,s1=q.s1+puv,t1=q.t1+puv,z=0.0f; float tv[30] = { vx0,vy0,z,s0,t0,vx1,vy1,z,s1,t1,vx1,vy0,z,s1,t0,vx0,vy0,z,s0,t0,vx0,vy1,z,s0,t1,vx1,vy1,z,s1,t1 };
        mcpy(textVertexData+vc*30,tv,sizeof(tv)); vc++;
        if (cp>='0' && cp<='9') { if(fontID == FONT_STOPD){xpos=q.x0 + ((fontID == FONT_STOPD) ? fixedNumberAdvanceWidthStopD : fixedNumberAdvanceWidth);} }
    }
    
    if(vc){ glBindBuffer(GL_ARRAY_BUFFER,textVBO); glBufferData(GL_ARRAY_BUFFER,vc*30*sizeof(float),textVertexData,GL_DYNAMIC_DRAW); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,vc*6); }
}
