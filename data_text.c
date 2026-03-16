#include "os.h"
#include "voxen.h"
char *strncpy(char *dest, const char *src, size_t n);
Voxen_Text Sys_Text;
char audiologNames[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
char audiologSubjects[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
char audiologSenders[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
char audioLogSpeech2Text[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
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

void LoadTextForLanguage(uint8_t lang){
    DualLog("Loading text for language...\n");
    char textFile[256]={0};
    switch(lang){case 1:strncpy(textFile,"./Data/text_espanol.txt",256);break;case 2:strncpy(textFile,"./Data/text_deutsch.txt",256);break;case 3:strncpy(textFile,"./Data/text_francais.txt",256);break;case 4:strncpy(textFile,"./Data/text_nihongo.txt",256);break;case 5:strncpy(textFile,"./Data/text_russkiy.txt",256);break;case 6:strncpy(textFile,"./Data/text_italiano.txt",256);break;case 7:strncpy(textFile,"./Data/text_portugues.txt",256);break;default:strncpy(textFile,"./Data/text_english.txt",256);break;}
    OsFileHandle fp=OS_OpenReadonly(textFile);if(fp==OS_INVALID_HANDLE){DualLogError("Cannot open %s\n",textFile);return;}
    OS_Seek(fp,0,SEEK_END);size_t file_size=(size_t)OS_Tell(fp);OS_Seek(fp,0,SEEK_SET);
    if(OS_Read(fp,Sys_Text.file_data,file_size)!=(long)file_size){DualLogError("Failed to read %s\n",textFile);OS_Close(fp);OS_Exit(1);}
    OS_Close(fp);
    size_t data_pos=0;int is_utf16le=0;
    if(file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){data_pos=2;is_utf16le=1;}
    else if(file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){data_pos=3;}
    else{size_t nulls=0;for(size_t i=1;i<file_size&&i<1024;i+=2)if(Sys_Text.file_data[i]==0)nulls++;if(nulls*3>file_size)is_utf16le=1;}
    char utf8_line[TEXT_LOCALIZATION_MAX_LENGTH];int lineNum=0;
    while(data_pos<file_size){
        size_t line_start=data_pos;
        if(is_utf16le){
            while(data_pos+1<file_size){
                uint16_t ch=Sys_Text.file_data[data_pos]|(Sys_Text.file_data[data_pos+1]<<8);data_pos+=2;
                if(ch=='\r'||ch=='\n'){if(ch=='\r'&&data_pos+1<file_size){uint16_t n=Sys_Text.file_data[data_pos]|(Sys_Text.file_data[data_pos+1]<<8);if(n=='\n')data_pos+=2;}break;}
            }
        }else{
            while(data_pos<file_size){
                uint8_t c=Sys_Text.file_data[data_pos];
                if(c=='\r'||c=='\n'){if(c=='\r'&&data_pos+1<file_size&&Sys_Text.file_data[data_pos+1]=='\n')++data_pos;++data_pos;break;}
                ++data_pos;
            }
        }
        size_t len=data_pos-line_start;if(len==0){if(lineNum<TEXT_STRING_COUNT)Sys_Text.stringTable[lineNum][0]='\0';++lineNum;continue;}
        if(is_utf16le){utf16le_to_utf8(&Sys_Text.file_data[line_start],len,utf8_line,sizeof(utf8_line));}
        else{if(len>=sizeof(utf8_line))len=sizeof(utf8_line)-1;CopyMemoryFromBtoAForNBytes(utf8_line,&Sys_Text.file_data[line_start],len);utf8_line[len]='\0';}
        len=GetStringLength(utf8_line);while(len>0&&(utf8_line[len-1]=='\r'||utf8_line[len-1]=='\n'))utf8_line[--len]='\0';
        if(len==0){if(lineNum<TEXT_STRING_COUNT)Sys_Text.stringTable[lineNum][0]='\0';++lineNum;continue;}
        if(lineNum<TEXT_STRING_COUNT){
            CopyMemoryFromBtoAForNBytes(Sys_Text.stringTable[lineNum],utf8_line,len);
            Sys_Text.stringTable[lineNum][len]='\0';++lineNum;
        }
    }
    DualLog("Text loaded!\n");
}

static inline __attribute__((always_inline)) int StringToIntLen(const char *str, size_t len) {
    int value = 0;
    size_t i = 0;
    while (i < len) {
        char c = str[i];
        if (c < '0' || c > '9') break;

        value = value * 10 + (c - '0');
        ++i;
    }
    return value;
}

void LoadLogTextForLanguage(uint8_t lang){
    DualLog("Loading log text for language...\n");
    SetMemoryToValueForNBytes(Sys_Text.audioLogImagesRefIndicesLH,0,TEXT_LOGS_COUNT*sizeof(uint16_t));
    SetMemoryToValueForNBytes(Sys_Text.audioLogImagesRefIndicesRH,0,TEXT_LOGS_COUNT*sizeof(uint16_t));
    SetMemoryToValueForNBytes(Sys_Text.audioLogType,0,TEXT_LOGS_COUNT*sizeof(uint8_t));
    SetMemoryToValueForNBytes(Sys_Text.audioLogLevelFound,0,TEXT_LOGS_COUNT*sizeof(uint8_t));
    char textFile[256]={0};
    switch(lang){
        case 1:StringCopyInto_A_From_B(textFile,"./Data/logs_text_espanol.txt",256);break;
        case 2:StringCopyInto_A_From_B(textFile,"./Data/logs_text_deutsch.txt",256);break;
        case 3:StringCopyInto_A_From_B(textFile,"./Data/logs_text_francais.txt",256);break;
        case 4:StringCopyInto_A_From_B(textFile,"./Data/logs_text_nihongo.txt",256);break;
        case 5:StringCopyInto_A_From_B(textFile,"./Data/logs_text_russkiy.txt",256);break;
        case 6:StringCopyInto_A_From_B(textFile,"./Data/logs_text_italiano.txt",256);break;
        case 7:StringCopyInto_A_From_B(textFile,"./Data/logs_text_portugues.txt",256);break;
        default:StringCopyInto_A_From_B(textFile,"./Data/logs_text_english.txt",256);break;
    }
    
    OsFileHandle fp=OS_OpenReadonly(textFile);
    if(!fp) { DualLogError("Cannot open %s\n",textFile); return; }
    
    OS_Seek(fp,0,SEEK_END);long file_size=OS_Tell(fp);OS_Seek(fp,0,SEEK_SET);
    if(OS_Read(fp,Sys_Text.file_data,(size_t)file_size)!=file_size){ DualLogError("Failed to read %s\n",textFile); OS_Close(fp); OS_Exit(1); }
    
    OS_Close(fp);
    size_t data_pos=0;int is_utf16le=0;
    if(file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){data_pos=2;is_utf16le=1;}
    else if(file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){data_pos=3;}
    else{int nulls=0;for(size_t i=1;i<(size_t)file_size&&i<2048;i+=2)if(Sys_Text.file_data[i]==0)nulls++;if(nulls>(int)(file_size/5))is_utf16le=1;}
    char utf8_line[1024];
    while(data_pos<(size_t)file_size){
        size_t line_start=data_pos;
        if(is_utf16le){
            while(data_pos+1<(size_t)file_size){
                uint16_t ch=Sys_Text.file_data[data_pos]|(Sys_Text.file_data[data_pos+1]<<8);data_pos+=2;
                if(ch=='\r'||ch=='\n'){if(ch=='\r'&&data_pos+1<(size_t)file_size){uint16_t next=Sys_Text.file_data[data_pos]|(Sys_Text.file_data[data_pos+1]<<8);if(next=='\n')data_pos+=2;}break;}
            }
        }else{
            while(data_pos<(size_t)file_size){
                uint8_t c=Sys_Text.file_data[data_pos];
                if(c=='\r'||c=='\n'){if(c=='\r'&&data_pos+1<(size_t)file_size&&Sys_Text.file_data[data_pos+1]=='\n')++data_pos;++data_pos;break;}
                ++data_pos;
            }
        }
        size_t line_len=data_pos-line_start;if(line_len==0)continue;
        if(is_utf16le){utf16le_to_utf8(&Sys_Text.file_data[line_start],line_len,utf8_line,sizeof(utf8_line));}
        else{if(line_len>=sizeof(utf8_line))line_len=sizeof(utf8_line)-1;CopyMemoryFromBtoAForNBytes(utf8_line,&Sys_Text.file_data[line_start],line_len);utf8_line[line_len]='\0';}
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
                case 3:if(log_index>=0&&log_index<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(audiologNames[log_index],tok_len,start,sizeof(audiologNames[0]));break;
                case 4:if(log_index>=0&&log_index<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(audiologSenders[log_index],tok_len,start,sizeof(audiologSenders[0]));break;
                case 5:if(log_index>=0&&log_index<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(audiologSubjects[log_index],tok_len,start,sizeof(audiologSubjects[0]));break;
                case 6:log_type=StringToIntLen(start,tok_len);break;
                case 7:level_found=StringToIntLen(start,tok_len);break;
                default:if(log_index>=0&&log_index<TEXT_LOGS_COUNT){char*dst=audioLogSpeech2Text[log_index];size_t cur=GetStringLength(dst);if(cur>0&&cur<TEXT_LOCALIZATION_MAX_LENGTH*4-2){dst[cur++]=',';dst[cur]='\0';}size_t left=TEXT_LOCALIZATION_MAX_LENGTH*4-cur-1;if(left>0){size_t cl=tok_len;if(cl>left)cl=left;StringCopyInto_A_SubstringFrom_B(dst+cur,cl,start,left+1);}}break;
            }if(*pos==',')++pos;field_idx++;
        }
        if(log_index>=0&&log_index<TEXT_LOGS_COUNT){
            Sys_Text.audioLogImagesRefIndicesLH[log_index]=(uint16_t)img_lh;
            Sys_Text.audioLogImagesRefIndicesRH[log_index]=(uint16_t)img_rh;
            Sys_Text.audioLogType[log_index]=(uint8_t)log_type;
            Sys_Text.audioLogLevelFound[log_index]=(uint8_t)level_found;
        }
    next_line:continue;
    }
    DualLog("Log text loaded!\n");
}
