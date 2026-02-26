#include "os.h" // Operating System calls shim layer.
#include "voxen.h"
uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (CharacterIsEmpty((char)*str)) str++;
    while (CharacterIsEmpty(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10uL + (unsigned long)digit;
        str++;
    }

    return (uint32_t)result;
}

uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT16_MAX) { DualLogError("Value %u out of range for uint16_t from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint16_t)retval;
}

uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT8_MAX) { DualLogError("Value %u out of range for uint8_t from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint8_t)retval;
}

bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    uint32_t parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);
    return parseval > 0 ? true : false;
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    
    while (CharacterIsEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }

    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { // Integer part
        value = value * 10.0 + (*str - '0');
        str++;
        has_digit = true;
    }

    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') {
            frac += (*str - '0') * place;
            place *= 0.1;
            str++;
            has_digit = true;
        }

        value += frac;
    }

    if (!has_digit) return 0.0f;

    if (negative) value = -value;
    return (float)value;
}

bool parse_data_file(DataParser *parser, uint16_t maxSize, const char *filename) {
    OsFileHandle fd; int st_size; char* data = OS_OpenAndAllocateFileBufferReadonly(filename,&fd,&st_size);
    char* cursor = data; char* end = data + st_size;
    uint32_t lineNum = 0, max_index = 0;
    while (cursor < end) { // First pass: count entries and find max index
        char* start = cursor;
        while (cursor < end && *cursor != '\n' && *cursor != '\r') cursor++;
        size_t lineLen = cursor - start;
        lineNum++;
        if (lineLen <= 0) { cursor++; continue; }

        while (CharacterIsEmpty(*start)) start++; // Trim leading whitespace
        char *lineend = start + lineLen - 1;
        while (lineend > start && CharacterIsEmpty(*lineend)) lineend--; // Trim trailing whitespace
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue; // Skip empty lines and commented lines
        if (start[0] == '#') { continue; } // Skip entry start marker, only count ones with valid index thereafter in the key|value block lines

        char *colon = StringFindFirstCharWithin(start, ':');
        if (colon && StringCompareUpToLength(start, "index", colon - start) == 0) {
            char *value = colon + 1;
            while (CharacterIsEmpty(*value)) value++;
            uint32_t idx = parse_numberu32(value, start, lineNum);
            if (idx > max_index) max_index = idx;
       }
       
       if (cursor < end && *cursor == '\r') cursor++;
       if (cursor < end && *cursor == '\n') cursor++;
    }
    
    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); OS_DeallocateRAM(data,st_size); return true; }
    if (max_index >= maxSize) { DualLogWarn("Too large of index found in %s, %u exceeds limit %u\n", filename, max_index, maxSize); OS_DeallocateRAM(data,st_size); return true; }

    uint32_t entry_count = max_index + 1;
    Entity *new_entries = OS_AllocateRAM(NULL, entry_count * sizeof(Entity), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);  
    parser->entries = new_entries;
    for (uint32_t i = parser->capacity; i < entry_count; ++i) InitializeEntity(&parser->entries[i]);
    parser->capacity = entry_count;
    parser->count = entry_count;
    Entity entry;
    InitializeEntity(&entry);
    lineNum = 0;
    int32_t currentChild = -1;
    cursor = data; end = data + st_size; // Rewind
    while (cursor < end) {
        char* start = cursor;
        while (cursor < end && *cursor != '\n' && *cursor != '\r') cursor++;
        size_t lineLen = cursor - start;
        lineNum++;
        if (lineLen < 3) { cursor++; continue; } // Must have at least k:v, skip if shorter

        while (CharacterIsEmpty(*start)) start++; // Trim leading whitespace
        char *lineend = start + lineLen - 1;
        while (lineend > start && CharacterIsEmpty(*lineend)) lineend--; // Trim trailing whitespace
        if (start[0] == '/' && start[1] == '/') continue; // Skip comment(ed out) line

        if (*start == '#') {
            if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
            InitializeEntity(&entry);
            if (lineend > start) {
                size_t actualLen = lineend - (start + 1) + 1;
                if (actualLen >= sizeof(entry.path)) actualLen = sizeof(entry.path) - 1;
                __builtin_memcpy(entry.path, start + 1, actualLen);
                entry.path[actualLen] = '\0';
            }
            continue;
        }

        // Handle key-value pair
        char *colon = StringFindFirstCharWithin(start, ':');
        if (colon) {
            char *key = start;
            char *value = colon + 1;
            while (CharacterIsEmpty(*key) && key < colon) key++;
            while (CharacterIsEmpty(*value) && value < lineend) value++;
            size_t keylen = colon - key; size_t vallen = (lineend >= value) ? (lineend - value + 1) : 0;
            if (keylen > 0 && vallen > 0) {
                char trimmed_key[256];
                char trimmed_value[256];
                StringCopyInto_A_SubstringFrom_B(trimmed_key, keylen, key, 256);
                StringCopyInto_A_SubstringFrom_B(trimmed_value, vallen, value, 256);
                char *key_end = trimmed_key + GetStringLength(trimmed_key) - 1;
                char *val_end = trimmed_value + GetStringLength(trimmed_value) - 1;
                while (key_end > trimmed_key && CharacterIsEmpty(*key_end)) *key_end-- = '\0';
                while (val_end > trimmed_value && CharacterIsEmpty(*val_end)) *val_end-- = '\0';
                     if (StringsAreEqual(trimmed_key, "index"))             entry.index = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "persistent"))        flag_set(&entry.entflags,ENTFLAG_TEST_PERSISTENT,parse_bool(trimmed_value, start, lineNum));
                
                else if (StringsAreEqual(trimmed_key, "model"))             entry.modelIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "animationNum")) {    entry.animationNum = parse_numberu16(trimmed_value, start, lineNum); entry.entflags |= ENTFLAG_ANIMATED; }
                else if (StringsAreEqual(trimmed_key, "animated"))          flag_set(&entry.entflags,ENTFLAG_ANIMATED,parse_numberu8(trimmed_value, start, lineNum));

                else if (StringsAreEqual(trimmed_key, "texture"))           entry.texIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "alttexture"))        entry.altTexIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "glowtexture"))       entry.glowIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "altglowtexture"))    entry.altGlowIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "spectexture"))       entry.specIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "normtexture"))       entry.normIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "doublesided"))       flag_set(&entry.entflags,ENTFLAG_DOUBLESIDED,parse_bool(trimmed_value, start, lineNum));
                else if (StringsAreEqual(trimmed_key, "transparent"))       flag_set(&entry.entflags,ENTFLAG_TRANSPARENT,parse_bool(trimmed_value, start, lineNum));
                else if (StringsAreEqual(trimmed_key, "cardchunk"))         flag_set(&entry.entflags,ENTFLAG_CARDCHUNK,  parse_bool(trimmed_value, start, lineNum));

                else if (StringsAreEqual(trimmed_key, "collider"))          entry.collider = parse_numberu8(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "collider_centerx"))  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "collider_centery"))  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "collider_centerz"))  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "collider_sizex"))    entry.colliderSize.x = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "collider_sizey"))    entry.colliderSize.y = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "collider_sizez"))    entry.colliderSize.z = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "colliderMeshIndex")) entry.colliderMeshIndex = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "mass"))              entry.mass = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "linearDrag"))        entry.linearDrag = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "angularDrag"))       entry.angularDrag = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "kinematic"))         flag_set(&entry.entflags,ENTFLAG_KINEMATIC, parse_bool(trimmed_value, start, lineNum));
                else if (StringsAreEqual(trimmed_key, "useGravity"))        flag_set(&entry.entflags,ENTFLAG_USEGRAVITY,parse_bool(trimmed_value, start, lineNum));
                else if (StringsAreEqual(trimmed_key, "bounciness"))        entry.bounciness = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "dynamicFriction"))   entry.dynamicFriction = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "frictionCombine"))   entry.frictionCombine = parse_numberu8(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "bounceCombine"))     entry.bounceCombine = parse_numberu8(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "numclips"))          entry.numclips = parse_numberu8(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "changeMatOnActive")) flag_set(&entry.entflags,ENTFLAG_CHANGE_TEX_ON_ACTIVE,parse_bool(trimmed_value, start, lineNum));
                else if (StringsAreEqual(trimmed_key, "blinkWhenActive"))   flag_set(&entry.entflags,ENTFLAG_BLINK_TEX_ON_ACTIVE,parse_bool(trimmed_value, start, lineNum));
                else if (StringsAreEqual(trimmed_key, "noshadows"))         flag_set(&entry.entflags,ENTFLAG_NO_SHADOWS,parse_bool(trimmed_value, start, lineNum));

                else if (StringsAreEqual(trimmed_key, "volume"))            entry.volume = parse_float(trimmed_value, start, lineNum);
                
                else if (StringsAreEqual(trimmed_key, "##child")) {
                    ++currentChild;
                    if (currentChild >= MAX_CHILD_COUNT) { DualLogError("Too many children %u! Minivan is full!!\n", currentChild); OS_Exit(1); }
                    
                    entry.child[currentChild] = parse_numberu16(trimmed_value, start, lineNum);
                } else if (StringsAreEqual(trimmed_key, "child_offsetx"))    entry.child_offset[currentChild].x = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "child_offsety"))    entry.child_offset[currentChild].y = parse_float(trimmed_value, start, lineNum);
                else if (StringsAreEqual(trimmed_key, "child_offsetz"))    entry.child_offset[currentChild].z = parse_float(trimmed_value, start, lineNum);
            } else DualLogWarn("Invalid key-value pair at line %u: %s\n", lineNum, start);
        } else DualLogWarn("No colon found in line %u: %s\n", lineNum, start);
    }

    // Store last entry
    if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
    OS_DeallocateRAM(data,st_size);
    return true;
}
