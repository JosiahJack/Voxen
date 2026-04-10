// data_models.c - Load 3D Models
#include "os.h"
#include "gl.h"
#include "voxen.h"
void qsort(void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*));
u8** modelVertices=NULL; u16** modelTriangles=NULL;
u32 modelVertexCounts[MODEL_IDX_MAX] = {0}; u16 modelTriangleCounts[MODEL_IDX_MAX] = {0};
float modelBounds[MODEL_IDX_MAX] = {0}; u16 loadedModelsMaxIndex = 0;
#define MAX_VERT_ELEMENT_SIZE 6964
#define MAX_OUTPUT_VERTS      20892
static float **thread_temp_pos=NULL, **thread_temp_nrm=NULL, **thread_temp_uv =NULL, **thread_out_verts=NULL;
static u16** thread_out_tris=NULL; static int num_parse_threads=0;
typedef struct { const char* data; int size; } RawOBJ;
typedef u16 half;
typedef struct { u16 index; bool animated; u8 animationNum; char path[128]; } ModelData;
typedef struct { ModelData* entries; u32 count; u32 capacity; } ModelDataParser;
static inline __attribute__((always_inline)) half float_to_half(float f){
	u32 x;__builtin_memcpy(&x,&f,4);
	u32 s=x>>31;
	u32 ue=(x>>23)&0xff;
	i32 e=(i32)ue-127;
	u32 m=x&0x7fffff;
	if(ue==0xff){
		if(m)return(half)(0x7e00|(m>>13)|(s<<15));
		return(half)((s<<15)|0x7c00);
	}
	if(ue==0&&m==0)return(half)(s<<15);
	if(e<=-24)return(half)(s<<15);
	if(e<=-14){m=(m|0x800000)>>(-e-1);return(half)((s<<15)|(m>>13));}
	if(e<=15){m+=0x1000;if(m>=0x800000){m=0;e++;}return(half)((s<<15)|((e+15)<<10)|(m>>13));}
	return(half)((s<<15)|0x7c00);
}

static inline __attribute__((always_inline)) float fast_atof(const char** p){
	float value=0.0f,sign=1.0f;
	while(**p==' '||**p=='\t')(*p)++;
	if(**p=='-'){sign=-1.0f;(*p)++;}
	while(**p>='0'&&**p<='9')value=value*10.0f+(*(*p)++-'0');
	if(**p=='.'){(*p)++;float sub=0.1f;while(**p>='0'&&**p<='9'){value+=(*(*p)++-'0')*sub;sub*=0.1f;}}
	return sign*value;
}

static inline __attribute__((always_inline)) i32 fast_atoi(const char** p){
	i32 val=0;i32 sign=1;
	while(**p==' '||**p=='\t')(*p)++;
	if(**p=='-'){sign=-1;(*p)++;}
	while(**p>='0'&&**p<='9')val=val*10+(*(*p)++-'0');
	return val*sign;
}

typedef struct { u32 idx; u32 key; } TriSort;   // renamed sumv → key for clarity
int cmp(const void* a, const void* b) { u32 ka = ((const TriSort*)a)->key, kb = ((const TriSort*)b)->key; return (ka < kb) ? -1 : (ka > kb); }
static void OptimizeVertexCache(u16* indices, u32 indexCount, u32 vertexCount) {
    if (indexCount < 3 || vertexCount == 0) return;
    u32 triCount = indexCount / 3;
    TriSort* tris = (TriSort*)OS_Alloc(triCount * sizeof(TriSort));
    for (u32 i = 0; i < triCount; ++i) {
        u16* t = indices + i * 3;
        u32 v0 = t[0], v1 = t[1], v2 = t[2];
        u32 minv = v0 < v1 ? v0 : v1;
        minv = minv < v2 ? minv : v2;
        tris[i].idx = i; tris[i].key = minv;
    }

    qsort(tris, triCount, sizeof(TriSort), cmp);
    u16* newIndices = (u16*)OS_Alloc(indexCount * sizeof(u16));
    for (u32 i = 0; i < triCount; ++i) {
        u32 old = tris[i].idx;
        u16* src = indices + old * 3; u16* dst = newIndices + i * 3;
        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
    }

    __builtin_memcpy(indices,newIndices,indexCount * sizeof(u16));
    OS_DeallocateRAM(newIndices,indexCount * sizeof(u16));
    OS_DeallocateRAM(tris,triCount * sizeof(TriSort));
}

static u8* OptimizeVertexFetch(u8* vertices, u32* vertexCount, u16* indices, u32 indexCount, size_t vertexStride) {
    u32 oldCount = *vertexCount;
    if (oldCount == 0 || indexCount == 0) return vertices;
    u32* remap = (u32*)OS_Alloc(oldCount * sizeof(u32));
    __builtin_memset(remap, 0xFF, oldCount * sizeof(u32));
    u32* firstUseOldId = (u32*)OS_Alloc(oldCount * sizeof(u32));
    u32 newCount = 0;
    for (u32 i = 0; i < indexCount; ++i) {
        u32 v = indices[i];
        if (v < oldCount && remap[v] == 0xFFFFFFFFU) {
            remap[v] = newCount;
            firstUseOldId[newCount] = v;
            ++newCount;
        }
    }

    u8* newVertices = (u8*)OS_Alloc(newCount * vertexStride);
    for (u32 i = 0; i < newCount; ++i) __builtin_memcpy(newVertices + i * vertexStride,vertices + firstUseOldId[i] * vertexStride, vertexStride);
    for (u32 i = 0; i < indexCount; ++i) { u32 v = indices[i]; if (v < oldCount) { indices[i] = (u16)remap[v]; } }
    *vertexCount = newCount;
    OS_DeallocateRAM(remap,oldCount * sizeof(u32));
    OS_DeallocateRAM(firstUseOldId,oldCount * sizeof(u32));
    return newVertices;
}

static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(const char* __restrict data,int file_size,float* __restrict temp_pos,float* __restrict temp_nrm,float* __restrict temp_uv,float* __restrict scratch_verts,u16* __restrict scratch_tris,u8** out_vertices,u32* out_vertex_count,u16** out_triangles,u16* out_triangle_count,float* out_minx,float* out_miny,float* out_minz,float* out_maxx,float* out_maxy,float* out_maxz){
	*out_vertices=NULL;*out_triangles=NULL;
	*out_vertex_count=*out_triangle_count=0;
	if(unlikely(!data||file_size<=0))return false;

	u32 pos_count=0,norm_count=0,uv_count=0;
	u32 expanded_count=0;
	float minx=1e9f,miny=1e9f,minz=1e9f;
	float maxx=-1e9f,maxy=-1e9f,maxz=-1e9f;
	const char* p=data;
	const char* const end=data+file_size;
	while(likely(p<end)){
		while(p<end&&(*p==' '||*p=='\t'||*p=='\r'||*p=='\n'))++p;
		if(p>=end)break;
		if(*p=='#'){while(p<end&&*p!='\n')++p;continue;}
		if(*p=='v'){
			++p;
			if(*p==' '){
				if(unlikely(pos_count>=MAX_VERT_ELEMENT_SIZE))return false;
				++p;
				temp_pos[pos_count*3]=fast_atof(&p);
				temp_pos[pos_count*3+1]=fast_atof(&p);
				temp_pos[pos_count*3+2]=fast_atof(&p);
				++pos_count;
			}else if(*p=='n'&&p[1]==' '){
				if(unlikely(norm_count>=MAX_VERT_ELEMENT_SIZE))return false;
				p+=2;
				temp_nrm[norm_count*3]=fast_atof(&p);
				temp_nrm[norm_count*3+1]=fast_atof(&p);
				temp_nrm[norm_count*3+2]=fast_atof(&p);
				++norm_count;
			}else if(*p=='t'&&p[1]==' '){
				if(unlikely(uv_count>=MAX_VERT_ELEMENT_SIZE))return false;
				p+=2;
				temp_uv[uv_count*2]=fast_atof(&p);
				temp_uv[uv_count*2+1]=fast_atof(&p);
				++uv_count;
			}
		}else if(*p=='f' && p[1]==' '){
            p += 2;
            u32 vert_ids[8] = {0}, tex_ids[8] = {0}, norm_ids[8] = {0}; // Up to 8 indices per face
            int num_verts = 0;
            while(num_verts < 8 && p < end && *p != '\n' && *p != '\r'){
                while(*p == ' ' || *p == '\t') ++p;
                if(*p == '\n' || *p == '\r' || *p == '#') break;

                long raw = fast_atoi(&p);
                u32 vidx = (raw > 0) ? (u32)raw : (raw < 0) ? (u32)((i32)pos_count + raw) : 0;
                vert_ids[num_verts] = vidx;

                if(*p == '/'){
                    ++p;
                    if(*p != '/'){
                        raw = fast_atoi(&p);
                        u32 tidx = (raw > 0) ? (u32)raw : (raw < 0) ? (u32)((i32)uv_count + raw) : 0;
                        tex_ids[num_verts] = tidx;
                    }
                    if(*p == '/'){
                        ++p;
                        raw = fast_atoi(&p);
                        u32 nidx = (raw > 0) ? (u32)raw : (raw < 0) ? (u32)((i32)norm_count + raw) : 0;
                        norm_ids[num_verts] = nidx;
                    }
                }
                ++num_verts;
            }

            if(num_verts < 3) goto skip_face;   // invalid face

            // Triangulate: fan from first vertex (works well for convex faces)
            for(int k = 1; k < num_verts-1; ++k){
                if(unlikely(expanded_count + 3 > MAX_OUTPUT_VERTS)) return false;

                u32 triangle[3] = {0, (u32)k, (u32)(k+1)};
                for(int t = 0; t < 3; ++t){
                    int idx = triangle[t];
                    u32 vi_idx = vert_ids[idx] ? vert_ids[idx]-1 : 0;
                    u32 ti_idx = (tex_ids[idx] && tex_ids[idx] <= uv_count) ? tex_ids[idx]-1 : 0;
                    u32 ni_idx = (norm_ids[idx] && norm_ids[idx] <= norm_count) ? norm_ids[idx]-1 : 0;
                    float* dst = scratch_verts + (expanded_count << 3);
                    dst[0] = -temp_pos[vi_idx*3];
                    dst[1] =  temp_pos[vi_idx*3+1];
                    dst[2] =  temp_pos[vi_idx*3+2];
                    dst[3] = (ni_idx < norm_count) ? -temp_nrm[ni_idx*3]   : 0.0f;
                    dst[4] = (ni_idx < norm_count) ?  temp_nrm[ni_idx*3+1] : 0.0f;
                    dst[5] = (ni_idx < norm_count) ?  temp_nrm[ni_idx*3+2] : 0.0f;
                    dst[6] = (ti_idx < uv_count)   ?  temp_uv[ti_idx*2]    : 0.0f;
                    dst[7] = (ti_idx < uv_count)   ?  temp_uv[ti_idx*2+1]  : 0.0f;
                    float x = dst[0], y = dst[1], z = dst[2];
                    minx = (x < minx) ? x : minx; maxx = (x > maxx) ? x : maxx;
                    miny = (y < miny) ? y : miny; maxy = (y > maxy) ? y : maxy;
                    minz = (z < minz) ? z : minz; maxz = (z > maxz) ? z : maxz;
                    scratch_tris[expanded_count] = (u16)expanded_count;
                    ++expanded_count;
                }
            }

        skip_face:;
        }else{
			while(p<end&&*p!='\n')++p;
		}
	}
	if(unlikely(expanded_count==0))return false;

    #define HASH_SIZE 32768
	u32 hash_table[HASH_SIZE];
	__builtin_memset(hash_table,0xFF,sizeof(hash_table));
	float* unique_verts=scratch_verts;
	u32* remap=(u32*)scratch_tris;
	u32 unique_cnt=0;
	for(u32 i=0;i<expanded_count;++i){
		const float* v=scratch_verts+(i<<3);
		u64 h=*(u32*)(v+0)^*(u32*)(v+2)^*(u32*)(v+4)^*(u32*)(v+6);
		u32 slot=(u32)(h^(h>>32))&(HASH_SIZE-1);
		while(hash_table[slot]!=0xFFFFFFFF){
			const float* candidate=unique_verts+(hash_table[slot]<<3);
			if(__builtin_memcmp(candidate,v,32)==0){
				remap[i]=hash_table[slot];
				goto next_vertex;
			}
			slot=(slot+1)&(HASH_SIZE-1);
		}
		hash_table[slot]=unique_cnt;
		remap[i]=unique_cnt;
		__builtin_memcpy(unique_verts+(unique_cnt<<3),v,32);
		++unique_cnt;
        next_vertex:;
	}
	
	u8* final_verts=(u8*)OS_Alloc((size_t)unique_cnt*VERTEX_ATTRIBUTES_SIZE);
	u8* dst=final_verts;
	for(u32 i=0;i<unique_cnt;++i){
		const float* src=unique_verts+(i<<3);
        for (u32 j=0;j<8;++j) { *(half*)dst=float_to_half(src[j]);dst+=2; }
	}

	u16* final_tris=(u16*)OS_Alloc((size_t)expanded_count*sizeof(u16));
	for(u32 i=0;i<expanded_count;++i)final_tris[i]=(u16)remap[i];
    OptimizeVertexCache(final_tris,expanded_count,unique_cnt);
    u32 oldVertexCount = unique_cnt;
    u8* optimizedVerts = OptimizeVertexFetch(final_verts,&unique_cnt,final_tris,expanded_count,VERTEX_ATTRIBUTES_SIZE);    
    OS_DeallocateRAM(final_verts,(size_t)oldVertexCount * VERTEX_ATTRIBUTES_SIZE);
	*out_vertices=optimizedVerts;
	*out_vertex_count=unique_cnt;
	*out_triangles=final_tris;
	*out_triangle_count=expanded_count/3;
	*out_minx=minx;*out_miny=miny;*out_minz=minz;
	*out_maxx=maxx;*out_maxy=maxy;*out_maxz=maxz;
	return true;
}

typedef struct{u32 start_model;u32 end_model;RawOBJ* raw_models;i32* index_to_parser;const ModelDataParser* data_parser;int thread_id;}ModelParseTask;

static void* ModelParsingWorker(void* argument){
	ModelParseTask* task=(ModelParseTask*)argument;
	for(u32 current_model=task->start_model;current_model<task->end_model;++current_model){
		i32 parser_index=task->index_to_parser[current_model];
		if(unlikely(parser_index<0||parser_index>=(i32)task->data_parser->count))continue;

        const char* model_data=task->raw_models[current_model].data;
		int model_file_size=task->raw_models[current_model].size;
		if(unlikely(!model_data||model_file_size<=0))continue;
        
		int tid=task->thread_id;
		float min_x,min_y,min_z,max_x,max_y,max_z;
		if(unlikely(!ParseOBJ(model_data,model_file_size,thread_temp_pos[tid],thread_temp_nrm[tid],thread_temp_uv[tid],thread_out_verts[tid],thread_out_tris[tid],&modelVertices[current_model],&modelVertexCounts[current_model],&modelTriangles[current_model],&modelTriangleCounts[current_model],&min_x,&min_y,&min_z,&max_x,&max_y,&max_z)))continue;
		float radius=vmax(0.0f,vabs(min_x)); radius=vmax(radius,vabs(min_y)); radius=vmax(radius,vabs(min_z)); radius=vmax(radius,max_x); radius=vmax(radius,max_y); radius=vmax(radius,max_z);
		modelBounds[current_model]=radius;
	}
	return NULL;
}

bool ParseModelData(ModelDataParser *parser, u16 maxSize, const char *filename) {
    OsFileHandle fd; int st_size; char* data = OS_OpenAndAllocateFileBufferReadonly(filename,&fd,&st_size);
    char* cursor = data; char* end = data + st_size;
    u32 lineNum = 0, max_index = 0;
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
            u32 idx = parse_numberu32(value, start, lineNum);
            if (idx > max_index) max_index = idx;
       }
       
       if (cursor < end && *cursor == '\r') cursor++;
       if (cursor < end && *cursor == '\n') cursor++;
    }
    
    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); OS_DeallocateRAM(data,st_size); return true; }
    if (max_index >= maxSize) { DualLogWarn("Too large of index found in %s, %u exceeds limit %u\n", filename, max_index, maxSize); OS_DeallocateRAM(data,st_size); return true; }

    u32 entry_count = max_index + 1;
    ModelData *new_entries = OS_Alloc(entry_count * sizeof(ModelData));  
    parser->entries = new_entries;
    for (u32 i = 0; i < entry_count; ++i) { parser->entries[i] = (ModelData){ .index=U16_MAX, .animated=false, .animationNum=255, .path={0} }; }
    parser->capacity = entry_count;
    parser->count = entry_count;
    ModelData entry = (ModelData){ .index=U16_MAX, .animated=false, .animationNum=255, .path={0} };
    lineNum = 0;
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
            if (entry.path[0] && entry.index != U16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
            entry = (ModelData){ .index=U16_MAX, .animated=false, .animationNum=255, .path={0} };
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
                     if (StringsEqual(trimmed_key, "index"))        entry.index = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsEqual(trimmed_key, "animationNum")) entry.animationNum = parse_numberu16(trimmed_value, start, lineNum);
                else if (StringsEqual(trimmed_key, "animated"))     entry.animated = parse_numberu8(trimmed_value, start, lineNum);
            } else DualLogWarn("Invalid key-value pair at line %u: %s\n", lineNum, start);
        } else DualLogWarn("No colon found in line %u: %s\n", lineNum, start);
    }

    // Store last entry
    if (entry.path[0] && entry.index != U16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
    OS_DeallocateRAM(data,st_size);
    return true;
}

void LoadModels(void) {
    double start_time = get_time();
    ModelDataParser mpars;
    if (unlikely(!ParseModelData(&mpars,MODEL_IDX_MAX,"./Data/models.txt"))) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    DualLog("Loading models (%d) ...",mpars.count);
    i32 max_index = -1;
    for (u32 k=0;k<mpars.count;++k) {
        if (mpars.entries[k].index > max_index && mpars.entries[k].index != U16_MAX) max_index = mpars.entries[k].index;
    }
    
    loadedModelsMaxIndex = (u16)max_index + 1U;
    num_parse_threads = clamp(OS_GetNumThreads(),1,32);
    modelVertices = OS_Alloc(loadedModelsMaxIndex * sizeof(u8*));
    modelTriangles = OS_Alloc(loadedModelsMaxIndex * sizeof(u16*));
    size_t index_map_size         = (size_t)loadedModelsMaxIndex * sizeof(i32);
    size_t raw_models_size        = (size_t)loadedModelsMaxIndex * sizeof(RawOBJ);
    size_t thread_ptr_size        = (size_t)num_parse_threads * sizeof(float*);
    size_t thread_ptr_arrays_size = 5 * thread_ptr_size;
    size_t per_thread_pos_size      = (size_t)MAX_VERT_ELEMENT_SIZE * 3 * sizeof(float);
    size_t per_thread_nrm_size      = per_thread_pos_size;
    size_t per_thread_uv_size       = (size_t)MAX_VERT_ELEMENT_SIZE * 2 * sizeof(float);
    size_t per_thread_out_verts_size = (size_t)MAX_OUTPUT_VERTS * 8 * sizeof(float);
    size_t per_thread_out_tris_size  = (size_t)MAX_OUTPUT_VERTS * sizeof(u32);
    size_t per_thread_total_size    = per_thread_pos_size + per_thread_nrm_size + per_thread_uv_size + per_thread_out_verts_size + per_thread_out_tris_size;
    size_t total_arena_size = index_map_size + raw_models_size + thread_ptr_arrays_size + (size_t)num_parse_threads * per_thread_total_size;
    void* arena_base = OS_Alloc(total_arena_size);
    char* arena_ptr  = (char*)arena_base;
    i32* index_to_parser = (i32*)arena_ptr;
    arena_ptr += index_map_size;
    __builtin_memset(index_to_parser, -1, index_map_size);
    for (u32 k = 0; k < mpars.count; ++k) {
        if (likely(mpars.entries[k].index != U16_MAX)) index_to_parser[mpars.entries[k].index] = (i32)k;
    }

    RawOBJ* raw_models = (RawOBJ*)arena_ptr;
    arena_ptr += raw_models_size;
    __builtin_memset(raw_models, 0, raw_models_size);
    for (u32 i = 0; i < loadedModelsMaxIndex; ++i) {
        i32 parser_index = index_to_parser[i];
        if (unlikely(parser_index < 0 || parser_index >= (i32)mpars.count)) continue;

        const char* path = mpars.entries[parser_index].path;
        OsFileHandle dummy_fd;
        int size = 0;
        raw_models[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path, &dummy_fd, &size);
        raw_models[i].size = size;
    }

    float**    l_thread_temp_pos   = (float**)arena_ptr;    arena_ptr += thread_ptr_size;
    float**    l_thread_temp_nrm   = (float**)arena_ptr;    arena_ptr += thread_ptr_size;
    float**    l_thread_temp_uv    = (float**)arena_ptr;    arena_ptr += thread_ptr_size;
    float**    l_thread_out_verts  = (float**)arena_ptr;    arena_ptr += thread_ptr_size;
    u16** l_thread_out_tris   = (u16**)arena_ptr; arena_ptr += thread_ptr_size;
    for (int t = 0; t < num_parse_threads; ++t) {
        l_thread_temp_pos[t]   = (float*)arena_ptr;    arena_ptr += per_thread_pos_size;
        l_thread_temp_nrm[t]   = (float*)arena_ptr;    arena_ptr += per_thread_nrm_size;
        l_thread_temp_uv[t]    = (float*)arena_ptr;    arena_ptr += per_thread_uv_size;
        l_thread_out_verts[t]  = (float*)arena_ptr;    arena_ptr += per_thread_out_verts_size;
        l_thread_out_tris[t]   = (u16*)arena_ptr; arena_ptr += per_thread_out_tris_size;
    }

    thread_temp_pos   = l_thread_temp_pos;
    thread_temp_nrm   = l_thread_temp_nrm;
    thread_temp_uv    = l_thread_temp_uv;
    thread_out_verts  = l_thread_out_verts;
    thread_out_tris   = l_thread_out_tris;
    ModelParseTask tasks[32];
    u32 chunk_size = (loadedModelsMaxIndex + (u32)num_parse_threads - 1U) / (u32)num_parse_threads;
    for (int t = 0; t < num_parse_threads; ++t) {
        tasks[t].start_model     = (u32)t * chunk_size;
        tasks[t].end_model       = tasks[t].start_model + chunk_size;
        if (tasks[t].end_model > loadedModelsMaxIndex) tasks[t].end_model = loadedModelsMaxIndex;
        tasks[t].raw_models      = raw_models;
        tasks[t].index_to_parser = index_to_parser;
        tasks[t].data_parser     = &mpars;
        tasks[t].thread_id       = t;
    }

    pthread_t worker_threads[32];
    for (int t = 0; t < num_parse_threads; ++t) pthread_create(&worker_threads[t], NULL, ModelParsingWorker, &tasks[t]);
    for (int t = 0; t < num_parse_threads; ++t) pthread_join(worker_threads[t], NULL);
    for (u32 i = 0; i < loadedModelsMaxIndex; ++i) {
        if (raw_models[i].data) OS_DeallocateRAM((void*)raw_models[i].data, (size_t)raw_models[i].size);
    }

    OS_DeallocateRAM(arena_base, total_arena_size);
    DebugRAM("after model load loop");
    glGenBuffers(loadedModelsMaxIndex,Sys_Render.vbos); glGenBuffers(loadedModelsMaxIndex,Sys_Render.tbos);
    u32 total_vertices=0,total_tris=0;
    for (int i=0;i<loadedModelsMaxIndex;++i) {
        if (unlikely(modelVertexCounts[i] == 0)) continue;
        size_t vert_size = (size_t)modelVertexCounts[i] * VERTEX_ATTRIBUTES_SIZE; total_vertices += modelVertexCounts[i];
        size_t  tri_size = (size_t)modelTriangleCounts[i] * 3 * sizeof(u16);     total_tris += (u32)tri_size;
        glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.vbos[i]); glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)vert_size,NULL,GL_STATIC_DRAW);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,0,(GLsizeiptr)vert_size,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr,modelVertices[i],vert_size);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[i]); glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)tri_size,NULL,GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,0,(GLsizeiptr)tri_size,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr,modelTriangles[i],tri_size);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER,0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glFlush(); glFinish();
    OS_DeallocateRAM(mpars.entries,mpars.count * sizeof(ModelData));
    DualLog(" total vertices: %u, total tris: %u, took %f secs\n",total_vertices,total_tris,get_time() - start_time);
    DebugRAM("After Load Models");
}
