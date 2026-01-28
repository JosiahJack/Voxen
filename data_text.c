#include "os.h"
#include "voxen.h"
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Voxen_Text voxen_Text;
char** audiologNames = NULL;
char** audiologSubjects = NULL;
char** audiologSenders = NULL;
char** audioLogSpeech2Text = NULL;
uint16_t logImages = 1272; // Start index of first index 0 logImages[0] which is blank1.png

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

void LoadTextForLanguage(uint8_t lang) {
    char textFile[256];
    switch (lang) {
        case 1:  StringCopyInto_A_From_B(textFile, "./Data/text_espanol.txt", 256); break;
        case 2:  StringCopyInto_A_From_B(textFile, "./Data/text_deutsch.txt", 256); break;
        case 3:  StringCopyInto_A_From_B(textFile, "./Data/text_francais.txt", 256); break;
        case 4:  StringCopyInto_A_From_B(textFile, "./Data/text_nihongo.txt", 256); break;
        case 5:  StringCopyInto_A_From_B(textFile, "./Data/text_russkiy.txt", 256); break;
        case 6:  StringCopyInto_A_From_B(textFile, "./Data/text_italiano.txt", 256); break;
        case 7:  StringCopyInto_A_From_B(textFile, "./Data/text_portugues.txt", 256); break;
        default: StringCopyInto_A_From_B(textFile, "./Data/text_english.txt", 256); break;
    }

    FILE* fp = fopen(textFile, "rb"); if (!fp) { DualLog("Failed to open text file: %s\n", textFile); return; }
    fseek(fp, 0, SEEK_END); size_t file_size = (size_t)ftell(fp); fseek(fp, 0, SEEK_SET);
    if (fread(voxen_Text.file_data, 1, file_size, fp) != file_size) { DualLogError("Failed to read %s?\n",textFile); OS_Exit(1); }
    fclose(fp);
    size_t data_pos = 0;
    int is_utf16le = 0;
    int is_utf8    = 0;
    if (file_size >= 2 && voxen_Text.file_data[0] == 0xFF && voxen_Text.file_data[1] == 0xFE) {          // UTF-16-LE BOM
        data_pos = 2;
        is_utf16le = 1;
    } else if (file_size >= 3 && voxen_Text.file_data[0] == 0xEF && voxen_Text.file_data[1] == 0xBB && voxen_Text.file_data[2] == 0xBF) {
        data_pos = 3;
        is_utf8 = 1;
    } else {                                   // No BOM → heuristic
        size_t null_bytes = 0;
        for (size_t i = 1; i < (size_t)file_size && i < 1024; i += 2) {
            if (voxen_Text.file_data[i] == 0) ++null_bytes;
        }
        if (null_bytes * 3 > file_size) {      // >33% null bytes → UTF-16-LE
            is_utf16le = 1;
        } else {
            is_utf8 = 1;
        }
    }

    int lineNum = 0, totalLines = 0;
    char utf8_line[TEXT_LOCALIZATION_MAX_LENGTH];
    while (data_pos < (size_t)file_size) {
        ++totalLines; size_t line_start = data_pos;
        if (is_utf8) {                         // ----- UTF-8 path -----
            while (data_pos < (size_t)file_size) {
                uint8_t c = voxen_Text.file_data[data_pos];
                if (c == '\r') {
                    ++data_pos;
                    if (data_pos < (size_t)file_size && voxen_Text.file_data[data_pos] == '\n') ++data_pos;
                    break;
                }
                if (c == '\n') { ++data_pos; break; }
                
                ++data_pos;
            }
            size_t len = data_pos - line_start;
            if (len == 0) {                         // blank line
                if (lineNum < TEXT_STRING_COUNT) voxen_Text.stringTable[lineNum][0] = '\0';                
                ++lineNum;
                continue;
            }
            if (len >= sizeof(utf8_line)) len = sizeof(utf8_line) - 1;
            memcpy(utf8_line, &voxen_Text.file_data[line_start], len);
            utf8_line[len] = '\0';
        } else if (is_utf16le) {                               // ----- UTF-16-LE path -----
            while (data_pos + 1 < (size_t)file_size) {
                uint16_t code = (uint16_t)voxen_Text.file_data[data_pos + 1] << 8 | voxen_Text.file_data[data_pos];
                data_pos += 2;
                if (code == 0x000D) {
                    if (data_pos + 1 < (size_t)file_size) {
                        uint16_t next = (uint16_t)voxen_Text.file_data[data_pos + 1] << 8 | voxen_Text.file_data[data_pos];
                        if (next == 0x000A) data_pos += 2;
                    }
                    break;
                }
                if (code == 0x000A) break;
            }
            size_t utf16_len = data_pos - line_start;
            if (utf16_len == 0) continue;
            utf8_line[0] = '\0';
            utf16le_to_utf8(&voxen_Text.file_data[line_start], utf16_len, utf8_line, sizeof(utf8_line));
        } else {
            DualLogError("Unknown encoding, not UTF-8 nor UTF-16LE for %s\n",textFile);
            OS_Exit(1);
        }

        size_t len = GetStringLength(utf8_line);
        while (len > 0 && (utf8_line[len - 1] == '\n' || utf8_line[len - 1] == '\r')) utf8_line[--len] = '\0';
        if (len == 0) {
            if (lineNum < TEXT_STRING_COUNT) voxen_Text.stringTable[lineNum][0] = '\0';                
            ++lineNum;
            continue;
        }

        if (lineNum < TEXT_STRING_COUNT) {
            memcpy(voxen_Text.stringTable[lineNum], utf8_line, len);
            voxen_Text.stringTable[lineNum][len] = '\0';
            ++lineNum;
        }
    }
}

void LoadLogTextForLanguage(uint8_t lang) {    
    // Unload language for when changing languages at runtime from settings.
    #define FREE_ARRAY(ptr, count) do { \
        if (ptr) { \
            for (int i = 0; i < (count); ++i) { \
                if ((ptr)[i]) { free((ptr)[i]); (ptr)[i] = NULL; } \
            } \
            free(ptr); ptr = NULL; \
        } \
    } while (0)

    FREE_ARRAY(audiologNames,       TEXT_LOGS_COUNT);
    FREE_ARRAY(audiologSenders,     TEXT_LOGS_COUNT);
    FREE_ARRAY(audiologSubjects,    TEXT_LOGS_COUNT);
    FREE_ARRAY(audioLogSpeech2Text, TEXT_LOGS_COUNT);
    #if defined(LINUX) || defined(ANDROID)
        malloc_trim(0);
    #endif
    memset(voxen_Text.audioLogImagesRefIndicesLH,0,TEXT_LOGS_COUNT * sizeof(uint16_t));
    memset(voxen_Text.audioLogImagesRefIndicesRH,0,TEXT_LOGS_COUNT * sizeof(uint16_t));
    memset(voxen_Text.audioLogType,0,TEXT_LOGS_COUNT * sizeof(uint8_t));
    memset(voxen_Text.audioLogLevelFound,0,TEXT_LOGS_COUNT * sizeof(uint8_t));
    audiologNames       = calloc(TEXT_LOGS_COUNT, sizeof(char*));
    audiologSenders     = calloc(TEXT_LOGS_COUNT, sizeof(char*));
    audiologSubjects    = calloc(TEXT_LOGS_COUNT, sizeof(char*));
    audioLogSpeech2Text = calloc(TEXT_LOGS_COUNT, sizeof(char*));
    char textFile[256];
    switch (lang) {
        case 1:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_espanol.txt", 256); break;
        case 2:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_deutsch.txt", 256); break;
        case 3:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_francais.txt", 256); break;
        case 4:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_nihongo.txt", 256); break;
        case 5:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_russkiy.txt", 256); break;
        case 6:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_italiano.txt", 256); break;
        case 7:  StringCopyInto_A_From_B(textFile, "./Data/logs_text_portugues.txt", 256); break;
        default: StringCopyInto_A_From_B(textFile, "./Data/logs_text_english.txt", 256); break;
    }

    FILE *fp = fopen(textFile, "rb");
    if (!fp) { DualLog("Failed to open logs text file: %s\n", textFile); OS_Exit(1); }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fread(voxen_Text.file_data, 1, (size_t)file_size, fp) != (size_t)file_size) { DualLogError("Failed to read %s\n", textFile); fclose(fp); OS_Exit(1); return; } // Suppress -fanalyzer warning about double free by including unnecessary `return;` here.
    fclose(fp);
    size_t data_pos = 0;
    int is_utf16le = 0, is_utf8 = 0;

    if (file_size >= 2 && voxen_Text.file_data[0] == 0xFF && voxen_Text.file_data[1] == 0xFE) { data_pos = 2; is_utf16le = 1; }
    else if (file_size >= 3 && voxen_Text.file_data[0] == 0xEF && voxen_Text.file_data[1] == 0xBB && voxen_Text.file_data[2] == 0xBF) { data_pos = 3; is_utf8 = 1; }
    else {
        int null_bytes = 0;
        for (size_t i = 1; i < (size_t)file_size && i < 1024; i += 2)
            if (voxen_Text.file_data[i] == 0) ++null_bytes;
        if (null_bytes > (file_size / 3)) is_utf16le = 1;
        else                               is_utf8    = 1;
    }

    int lineNum = 0, totalLines = 0;
    char utf8_line[TEXT_LOCALIZATION_MAX_LENGTH]; // 1207
    while (data_pos < (size_t)file_size) {
        ++totalLines;
        size_t line_start = data_pos;
        if (is_utf8) {
            while (data_pos < (size_t)file_size) {
                uint8_t c = voxen_Text.file_data[data_pos];
                if (c == '\r') { ++data_pos; if (data_pos < (size_t)file_size && voxen_Text.file_data[data_pos] == '\n') ++data_pos; break; }
                if (c == '\n') { ++data_pos; break; }
                ++data_pos;
            }
            size_t len = data_pos - line_start;
            if (len == 0) continue;
            if (len >= sizeof(utf8_line)) len = sizeof(utf8_line) - 1;
            memcpy(utf8_line, &voxen_Text.file_data[line_start], len);
            utf8_line[len] = '\0';
        } else if (is_utf16le) {
            while (data_pos + 1 < (size_t)file_size) {
                uint16_t code = (uint16_t)voxen_Text.file_data[data_pos + 1] << 8 | voxen_Text.file_data[data_pos];
                data_pos += 2;
                if (code == 0x000D) {
                    if (data_pos + 1 < (size_t)file_size) {
                        uint16_t next = (uint16_t)voxen_Text.file_data[data_pos + 1] << 8 | voxen_Text.file_data[data_pos];
                        if (next == 0x000A) data_pos += 2;
                    }
                    break;
                }
                if (code == 0x000A) break;
            }
            size_t utf16_len = data_pos - line_start;
            if (utf16_len == 0) continue;
            utf8_line[0] = '\0';
            utf16le_to_utf8(&voxen_Text.file_data[line_start], utf16_len, utf8_line, sizeof(utf8_line));
        } else { DualLogError("Unknown encoding for %s\n", textFile); OS_Exit(1); }

        size_t len = GetStringLength(utf8_line);
        while (len > 0 && (utf8_line[len - 1] == '\n' || utf8_line[len - 1] == '\r')) utf8_line[--len] = '\0';
        if (len == 0) { ++lineNum; continue; }   /* blank line -> skip */

        char logline[TEXT_LOCALIZATION_MAX_LENGTH];
        memcpy(logline, utf8_line, sizeof(logline) - 1);
        logline[sizeof(logline) - 1] = '\0';
        
        char fields[32][TEXT_LOCALIZATION_MAX_LENGTH];
        int  num_fields = 0;
        char *saveptr = NULL;
        char *token = strtok_r(logline, ",", &saveptr);

        while (token && num_fields < 32) {
            if (token[0] == '"') ++token;
            size_t tlen = GetStringLength(token);
            if (tlen && token[tlen - 1] == '"') token[--tlen] = '\0';
            memcpy(fields[num_fields], token, sizeof(fields[0]) - 1);
            fields[num_fields][sizeof(fields[0]) - 1] = '\0';
            
            ++num_fields;
            token = strtok_r(NULL, ",", &saveptr);
        }

        int  readIndexOfLog      = (num_fields > 0) ? StringToInt(fields[0]) : -1;
        int  readLogImageLHIndex = (num_fields > 1) ? StringToInt(fields[1]) : -1;
        int  readLogImageRHIndex = (num_fields > 2) ? StringToInt(fields[2]) : -1;
        char readLogName[64]     = {0};
        char readLogSender[64]   = {0};
        char readLogSubject[256] = {0};
        char readLogText[TEXT_LOCALIZATION_MAX_LENGTH * 4] = {0};
        int  readLogType         = (num_fields > 6) ? StringToInt(fields[6]) : 0;
        int  readLogLevelFound   = (num_fields > 7) ? StringToInt(fields[7]) : 0;
        if (num_fields > 3) memcpy(readLogName,    fields[3], sizeof(readLogName) - 1);
        if (num_fields > 4) memcpy(readLogSender,  fields[4], sizeof(readLogSender) - 1);
        if (num_fields > 5) memcpy(readLogSubject, fields[5], sizeof(readLogSubject) - 1);
        for (int f = 8; f < num_fields; ++f) {
            if (f > 8) StringConcatenate(readLogText, ",", TEXT_LOCALIZATION_MAX_LENGTH * 4);
            StringConcatenate(readLogText, fields[f], TEXT_LOCALIZATION_MAX_LENGTH * 4);
        }

        if (readIndexOfLog >= 0 && readIndexOfLog < TEXT_LOGS_COUNT) {
            voxen_Text.audioLogImagesRefIndicesLH[readIndexOfLog] = (uint16_t)readLogImageLHIndex;
            voxen_Text.audioLogImagesRefIndicesRH[readIndexOfLog] = (uint16_t)readLogImageRHIndex;
            voxen_Text.audioLogType[readIndexOfLog]               = (uint8_t)readLogType;
            voxen_Text.audioLogLevelFound[readIndexOfLog]         = (uint8_t)readLogLevelFound;
            #define REPLACE_STR(dst, src) do { \
                if ((dst)[readIndexOfLog]) free((dst)[readIndexOfLog]); \
                size_t slen = GetStringLength(src); \
                (dst)[readIndexOfLog] = malloc(slen + 1); \
                if ((dst)[readIndexOfLog]) StringCopyInto_A_From_B((dst)[readIndexOfLog], src, slen + 1); \
                else (dst)[readIndexOfLog] = NULL; \
            } while (0)

            REPLACE_STR(audiologNames,       readLogName);
            REPLACE_STR(audiologSenders,     readLogSender);
            REPLACE_STR(audiologSubjects,    readLogSubject);
            REPLACE_STR(audioLogSpeech2Text, readLogText);
            ++lineNum;
        }
    }
}
