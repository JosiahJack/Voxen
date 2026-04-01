// data_models.c - Load 3D Models
#include "os.h"
#include "gl.h"
#include "voxen.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
uint8_t** modelVertices=NULL;
uint16_t** modelTriangles=NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0};
uint16_t modelTriangleCounts[MODEL_IDX_MAX] = {0};
float modelBounds[MODEL_IDX_MAX*BOUNDS_ATTRIBUTES_COUNT] = {0};
uint16_t loadedModelsMaxIndex = 0;
#define MAX_VERT_ELEMENT_SIZE 6964
#define MAX_OUTPUT_VERTS      20892
static float**    thread_temp_pos   = NULL;
static float**    thread_temp_nrm   = NULL;
static float**    thread_temp_uv    = NULL;
static float**    thread_out_verts  = NULL;
static uint16_t** thread_out_tris   = NULL;
static int        num_parse_threads = 0;
typedef struct { const char* data; int size; } RawOBJ;
typedef uint16_t half;
typedef struct { uint16_t index; bool animated; uint8_t animationNum; char path[128]; } ModelData;
typedef struct { ModelData* entries; uint32_t count; uint32_t capacity; } ModelDataParser;
static inline __attribute__((always_inline)) half float_to_half(float f){
	uint32_t x;__builtin_memcpy(&x,&f,4);
	uint32_t s=x>>31;
	uint32_t ue=(x>>23)&0xff;
	int32_t e=(int32_t)ue-127;
	uint32_t m=x&0x7fffff;
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

static inline __attribute__((always_inline)) int32_t fast_atoi(const char** p){
	int32_t val=0;int32_t sign=1;
	while(**p==' '||**p=='\t')(*p)++;
	if(**p=='-'){sign=-1;(*p)++;}
	while(**p>='0'&&**p<='9')val=val*10+(*(*p)++-'0');
	return val*sign;
}

typedef struct { uint32_t idx; uint32_t key; } TriSort;   // renamed sumv → key for clarity
int cmp(const void* a, const void* b) {
    uint32_t ka = ((const TriSort*)a)->key;
    uint32_t kb = ((const TriSort*)b)->key;
    return (ka < kb) ? -1 : (ka > kb);
}

static void OptimizeVertexCache(uint16_t* indices, uint32_t indexCount, uint32_t vertexCount) {
    if (indexCount < 3 || vertexCount == 0) return;
    uint32_t triCount = indexCount / 3;
    TriSort* tris = (TriSort*)OS_AllocateRAM(NULL, triCount * sizeof(TriSort), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    for (uint32_t i = 0; i < triCount; ++i) {
        uint16_t* t = indices + i * 3;
        uint32_t v0 = t[0], v1 = t[1], v2 = t[2];
        uint32_t minv = v0 < v1 ? v0 : v1;
        minv = minv < v2 ? minv : v2;
        tris[i].idx = i;
        tris[i].key = minv;
    }

    qsort(tris, triCount, sizeof(TriSort), cmp);
    uint16_t* newIndices = (uint16_t*)OS_AllocateRAM(NULL, indexCount * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    for (uint32_t i = 0; i < triCount; ++i) {
        uint32_t old = tris[i].idx;
        uint16_t* src = indices + old * 3;
        uint16_t* dst = newIndices + i * 3;
        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
    }

    __builtin_memcpy(indices, newIndices, indexCount * sizeof(uint16_t));
    OS_DeallocateRAM(newIndices, indexCount * sizeof(uint16_t));
    OS_DeallocateRAM(tris, triCount * sizeof(TriSort));
}

static uint8_t* OptimizeVertexFetch(uint8_t* vertices, uint32_t* vertexCount, uint16_t* indices, uint32_t indexCount, size_t vertexStride) {
    uint32_t oldCount = *vertexCount;
    if (oldCount == 0 || indexCount == 0) return vertices;
    uint32_t* remap = (uint32_t*)OS_AllocateRAM(NULL, oldCount * sizeof(uint32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    __builtin_memset(remap, 0xFF, oldCount * sizeof(uint32_t));
    uint32_t* firstUseOldId = (uint32_t*)OS_AllocateRAM(NULL, oldCount * sizeof(uint32_t),PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    uint32_t newCount = 0;
    for (uint32_t i = 0; i < indexCount; ++i) {
        uint32_t v = indices[i];
        if (v < oldCount && remap[v] == 0xFFFFFFFFU) {
            remap[v] = newCount;
            firstUseOldId[newCount] = v;
            ++newCount;
        }
    }

    uint8_t* newVertices = (uint8_t*)OS_AllocateRAM(NULL, newCount * vertexStride, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    for (uint32_t i = 0; i < newCount; ++i) __builtin_memcpy(newVertices + i * vertexStride,vertices + firstUseOldId[i] * vertexStride, vertexStride);
    for (uint32_t i = 0; i < indexCount; ++i) { uint32_t v = indices[i]; if (v < oldCount) { indices[i] = (uint16_t)remap[v]; } }
    *vertexCount = newCount;
    OS_DeallocateRAM(remap, oldCount * sizeof(uint32_t));
    OS_DeallocateRAM(firstUseOldId, oldCount * sizeof(uint32_t));
    return newVertices;
}

static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(const char* __restrict data,int file_size,float* __restrict temp_pos,float* __restrict temp_nrm,float* __restrict temp_uv,float* __restrict scratch_verts,uint16_t* __restrict scratch_tris,uint8_t** out_vertices,uint32_t* out_vertex_count,uint16_t** out_triangles,uint16_t* out_triangle_count,float* out_minx,float* out_miny,float* out_minz,float* out_maxx,float* out_maxy,float* out_maxz){
	*out_vertices=NULL;*out_triangles=NULL;
	*out_vertex_count=*out_triangle_count=0;
	if(unlikely(!data||file_size<=0))return false;

	uint32_t pos_count=0,norm_count=0,uv_count=0;
	uint32_t expanded_count=0;
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

            // Collect up to 8 indices per face (enough for typical models)
            uint32_t vert_ids[8] = {0}, tex_ids[8] = {0}, norm_ids[8] = {0};
            int num_verts = 0;

            while(num_verts < 8 && p < end && *p != '\n' && *p != '\r'){
                while(*p == ' ' || *p == '\t') ++p;
                if(*p == '\n' || *p == '\r' || *p == '#') break;

                long raw = fast_atoi(&p);
                uint32_t vidx = (raw > 0) ? (uint32_t)raw :
                                (raw < 0) ? (uint32_t)((int32_t)pos_count + raw) : 0;
                vert_ids[num_verts] = vidx;

                if(*p == '/'){
                    ++p;
                    if(*p != '/'){
                        raw = fast_atoi(&p);
                        uint32_t tidx = (raw > 0) ? (uint32_t)raw :
                                        (raw < 0) ? (uint32_t)((int32_t)uv_count + raw) : 0;
                        tex_ids[num_verts] = tidx;
                    }
                    if(*p == '/'){
                        ++p;
                        raw = fast_atoi(&p);
                        uint32_t nidx = (raw > 0) ? (uint32_t)raw :
                                        (raw < 0) ? (uint32_t)((int32_t)norm_count + raw) : 0;
                        norm_ids[num_verts] = nidx;
                    }
                }
                ++num_verts;
            }

            if(num_verts < 3) goto skip_face;   // invalid face

            // Triangulate: fan from first vertex (works well for convex faces)
            for(int k = 1; k < num_verts-1; ++k){
                if(unlikely(expanded_count + 3 > MAX_OUTPUT_VERTS)) return false;

                uint32_t triangle[3] = {0, (uint32_t)k, (uint32_t)(k+1)};

                for(int t = 0; t < 3; ++t){
                    int idx = triangle[t];
                    uint32_t vi_idx = vert_ids[idx] ? vert_ids[idx]-1 : 0;
                    uint32_t ti_idx = (tex_ids[idx] && tex_ids[idx] <= uv_count) ? tex_ids[idx]-1 : 0;
                    uint32_t ni_idx = (norm_ids[idx] && norm_ids[idx] <= norm_count) ? norm_ids[idx]-1 : 0;

                    float* dst = scratch_verts + (expanded_count << 3);
                    dst[0] = -temp_pos[vi_idx*3];
                    dst[1] =  temp_pos[vi_idx*3+1];
                    dst[2] =  temp_pos[vi_idx*3+2];
                    dst[3] = (ni_idx < norm_count) ? -temp_nrm[ni_idx*3]   : 0.0f;
                    dst[4] = (ni_idx < norm_count) ?  temp_nrm[ni_idx*3+1] : 0.0f;
                    dst[5] = (ni_idx < norm_count) ?  temp_nrm[ni_idx*3+2] : 0.0f;
                    dst[6] = (ti_idx < uv_count)   ?  temp_uv[ti_idx*2]    : 0.0f;
                    dst[7] = (ti_idx < uv_count)   ?  temp_uv[ti_idx*2+1]  : 0.0f;

                    // update bounds
                    float x = dst[0], y = dst[1], z = dst[2];
                    minx = (x < minx) ? x : minx; maxx = (x > maxx) ? x : maxx;
                    miny = (y < miny) ? y : miny; maxy = (y > maxy) ? y : maxy;
                    minz = (z < minz) ? z : minz; maxz = (z > maxz) ? z : maxz;

                    scratch_tris[expanded_count] = (uint16_t)expanded_count;
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
	uint32_t hash_table[HASH_SIZE];
	__builtin_memset(hash_table,0xFF,sizeof(hash_table));
	float* unique_verts=scratch_verts;
	uint32_t* remap=(uint32_t*)scratch_tris;
	uint32_t unique_cnt=0;
	for(uint32_t i=0;i<expanded_count;++i){
		const float* v=scratch_verts+(i<<3);
		uint64_t h=*(uint64_t*)(v+0)^*(uint64_t*)(v+2)^*(uint64_t*)(v+4)^*(uint64_t*)(v+6);
		uint32_t slot=(uint32_t)(h^(h>>32))&(HASH_SIZE-1);
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
	
	size_t vbytes=(size_t)unique_cnt*VERTEX_ATTRIBUTES_SIZE;
	uint8_t* final_verts=(uint8_t*)OS_AllocateRAM(NULL,vbytes,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	uint8_t* dst=final_verts;
	for(uint32_t i=0;i<unique_cnt;++i){
		const float* src=unique_verts+(i<<3);
		*(half*)dst=float_to_half(src[0]);dst+=2; // x
		*(half*)dst=float_to_half(src[1]);dst+=2; // y
		*(half*)dst=float_to_half(src[2]);dst+=2; // z
        *(half*)dst=float_to_half(src[3]);dst+=2; // nx
		*(half*)dst=float_to_half(src[4]);dst+=2; // ny
		*(half*)dst=float_to_half(src[5]);dst+=2; // nz
        *(half*)dst = float_to_half(src[6]); dst += 2; // u
        *(half*)dst = float_to_half(src[7]); dst += 2; // v
	}

	size_t ibytes=(size_t)expanded_count*sizeof(uint16_t);
	uint16_t* final_tris=(uint16_t*)OS_AllocateRAM(NULL,ibytes,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	for(uint32_t i=0;i<expanded_count;++i)final_tris[i]=(uint16_t)remap[i];
    OptimizeVertexCache(final_tris,expanded_count,unique_cnt);
    uint32_t oldVertexCount = unique_cnt;
    uint8_t* optimizedVerts = OptimizeVertexFetch(final_verts,&unique_cnt,final_tris,expanded_count,VERTEX_ATTRIBUTES_SIZE);
    OS_DeallocateRAM(final_verts,(size_t)oldVertexCount * VERTEX_ATTRIBUTES_SIZE);
	*out_vertices=optimizedVerts;
	*out_vertex_count=unique_cnt;
	*out_triangles=final_tris;
	*out_triangle_count=expanded_count/3;
	*out_minx=minx;*out_miny=miny;*out_minz=minz;
	*out_maxx=maxx;*out_maxy=maxy;*out_maxz=maxz;
	return true;
}

typedef struct{uint32_t start_model;uint32_t end_model;RawOBJ* raw_models;int32_t* index_to_parser;const ModelDataParser* data_parser;int thread_id;}ModelParseTask;

static void* ModelParsingWorker(void* argument){
	ModelParseTask* task=(ModelParseTask*)argument;
	for(uint32_t current_model=task->start_model;current_model<task->end_model;++current_model){
		int32_t parser_index=task->index_to_parser[current_model];
		if(unlikely(parser_index<0||parser_index>=(int32_t)task->data_parser->count))continue;

        const char* model_data=task->raw_models[current_model].data;
		int model_file_size=task->raw_models[current_model].size;
		if(unlikely(!model_data||model_file_size<=0))continue;
        
		int tid=task->thread_id;
		float min_x,min_y,min_z,max_x,max_y,max_z;
		if(unlikely(!ParseOBJ(model_data,model_file_size,thread_temp_pos[tid],thread_temp_nrm[tid],thread_temp_uv[tid],thread_out_verts[tid],thread_out_tris[tid],&modelVertices[current_model],&modelVertexCounts[current_model],&modelTriangles[current_model],&modelTriangleCounts[current_model],&min_x,&min_y,&min_z,&max_x,&max_y,&max_z)))continue;
		uint32_t bounds_base=current_model*BOUNDS_ATTRIBUTES_COUNT;
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_MINX]=min_x;
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_MINY]=min_y;
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_MINZ]=min_z;
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_MAXX]=max_x;
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_MAXY]=max_y;
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_MAXZ]=max_z;
		float radius=vmax(0.0f,vabs(min_x));
		radius=vmax(radius,vabs(min_y));
		radius=vmax(radius,vabs(min_z));
		radius=vmax(radius,max_x);
		radius=vmax(radius,max_y);
		radius=vmax(radius,max_z);
		modelBounds[bounds_base+BOUNDS_DATA_OFFSET_RADIUS]=radius;
	}
	return NULL;
}

bool ParseModelData(ModelDataParser *parser, uint16_t maxSize, const char *filename) {
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
    ModelData *new_entries = OS_AllocateRAM(NULL,entry_count * sizeof(ModelData),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);  
    parser->entries = new_entries;
    for (uint32_t i = 0; i < entry_count; ++i) { parser->entries[i] = (ModelData){ .index=UINT16_MAX, .animated=false, .animationNum=255, .path={0} }; }
    parser->capacity = entry_count;
    parser->count = entry_count;
    ModelData entry = (ModelData){ .index=UINT16_MAX, .animated=false, .animationNum=255, .path={0} };
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
            if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
            entry = (ModelData){ .index=UINT16_MAX, .animated=false, .animationNum=255, .path={0} };
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
    if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
    OS_DeallocateRAM(data,st_size);
    return true;
}

void LoadModels(void){
	if(unlikely(loadedModelsMaxIndex>0))return;
	double start_time=get_time();
	ModelDataParser mpars;
	if(unlikely(!ParseModelData(&mpars,MODEL_IDX_MAX,"./Data/models.txt"))){DualLogError("Could not parse ./Data/models.txt!\n");OS_Exit(1);}
	int32_t max_index=-1;
	for(uint32_t k=0;k<mpars.count;++k){
		if(mpars.entries[k].index>max_index&&mpars.entries[k].index!=UINT16_MAX)max_index=mpars.entries[k].index;
	}
	
	loadedModelsMaxIndex=(uint16_t)max_index+1U;
	DualLog("Loading models (%d) ...",mpars.count);
	modelVertices=OS_AllocateRAM(NULL,loadedModelsMaxIndex*sizeof(uint8_t*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	modelTriangles=OS_AllocateRAM(NULL,loadedModelsMaxIndex*sizeof(uint16_t*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	size_t index_map_size=loadedModelsMaxIndex*sizeof(int32_t);
	int32_t* index_to_parser=OS_AllocateRAM(NULL,index_map_size,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE,OS_INVALID_HANDLE);
	__builtin_memset(index_to_parser,-1,index_map_size);
	for(uint32_t k=0;k<mpars.count;++k){
		if(likely(mpars.entries[k].index!=UINT16_MAX))index_to_parser[mpars.entries[k].index]=(int32_t)k;
	}

	RawOBJ* raw_models=OS_AllocateRAM(NULL,loadedModelsMaxIndex*sizeof(RawOBJ),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	__builtin_memset(raw_models,0,loadedModelsMaxIndex*sizeof(RawOBJ));
	for(uint32_t i=0;i<loadedModelsMaxIndex;++i){
		int32_t parser_index=index_to_parser[i];
		if(unlikely(parser_index<0||parser_index>=(int32_t)mpars.count))continue;
		const char* path=mpars.entries[parser_index].path;
		OsFileHandle dummy_fd;
		int size=0;
		raw_models[i].data=(const char*)OS_OpenAndAllocateFileBufferReadonly(path,&dummy_fd,&size);
		raw_models[i].size=size;
	}

	num_parse_threads=OS_GetNumThreads();
	if(num_parse_threads<1)num_parse_threads=1;
	if(num_parse_threads>32)num_parse_threads=32;
	thread_temp_pos=(float**)OS_AllocateRAM(NULL,(size_t)num_parse_threads*sizeof(float*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	thread_temp_nrm=(float**)OS_AllocateRAM(NULL,(size_t)num_parse_threads*sizeof(float*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	thread_temp_uv=(float**)OS_AllocateRAM(NULL,(size_t)num_parse_threads*sizeof(float*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	thread_out_verts=(float**)OS_AllocateRAM(NULL,(size_t)num_parse_threads*sizeof(float*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	thread_out_tris=(uint16_t**)OS_AllocateRAM(NULL,(size_t)num_parse_threads*sizeof(uint16_t*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	for(int t=0;t<num_parse_threads;++t){
		thread_temp_pos[t]=(float*)OS_AllocateRAM(NULL,MAX_VERT_ELEMENT_SIZE*3*sizeof(float),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
		thread_temp_nrm[t]=(float*)OS_AllocateRAM(NULL,MAX_VERT_ELEMENT_SIZE*3*sizeof(float),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
		thread_temp_uv[t]=(float*)OS_AllocateRAM(NULL,MAX_VERT_ELEMENT_SIZE*2*sizeof(float),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
		thread_out_verts[t]=(float*)OS_AllocateRAM(NULL,MAX_OUTPUT_VERTS*8*sizeof(float),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
		thread_out_tris[t]=(uint16_t*)OS_AllocateRAM(NULL,MAX_OUTPUT_VERTS*sizeof(uint32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
	}

	ModelParseTask tasks[32];
	uint32_t chunk_size=(loadedModelsMaxIndex+(uint32_t)num_parse_threads-1U)/(uint32_t)num_parse_threads;
	for(int t=0;t<num_parse_threads;++t){
		tasks[t].start_model=(uint32_t)t*chunk_size;
		tasks[t].end_model=tasks[t].start_model+chunk_size;
		if(tasks[t].end_model>loadedModelsMaxIndex)tasks[t].end_model=loadedModelsMaxIndex;
		tasks[t].raw_models=raw_models;
		tasks[t].index_to_parser=index_to_parser;
		tasks[t].data_parser=&mpars;
		tasks[t].thread_id=t;
	}

	pthread_t worker_threads[32];
	for(int t=0;t<num_parse_threads;++t)pthread_create(&worker_threads[t],NULL,ModelParsingWorker,&tasks[t]);
	for(int t=0;t<num_parse_threads;++t)pthread_join(worker_threads[t],NULL);
	for(int t=0;t<num_parse_threads;++t){
		OS_DeallocateRAM(thread_temp_nrm[t],MAX_VERT_ELEMENT_SIZE*3*sizeof(float));
		OS_DeallocateRAM(thread_temp_uv[t],MAX_VERT_ELEMENT_SIZE*2*sizeof(float));
		OS_DeallocateRAM(thread_out_verts[t],MAX_OUTPUT_VERTS*8*sizeof(float));
		OS_DeallocateRAM(thread_out_tris[t],MAX_OUTPUT_VERTS*sizeof(uint32_t));
	}

	OS_DeallocateRAM(thread_temp_pos,(size_t)num_parse_threads*sizeof(float*));
	OS_DeallocateRAM(thread_temp_nrm,(size_t)num_parse_threads*sizeof(float*));
	OS_DeallocateRAM(thread_temp_uv,(size_t)num_parse_threads*sizeof(float*));
	OS_DeallocateRAM(thread_out_verts,(size_t)num_parse_threads*sizeof(float*));
	OS_DeallocateRAM(thread_out_tris,(size_t)num_parse_threads*sizeof(uint16_t*));

	for(uint32_t i=0;i<loadedModelsMaxIndex;++i){
		if(raw_models[i].data)OS_DeallocateRAM((void*)raw_models[i].data,(size_t)raw_models[i].size);
	}
	OS_DeallocateRAM(raw_models,loadedModelsMaxIndex*sizeof(RawOBJ));

	DebugRAM("after model load loop");
	OS_DeallocateRAM(index_to_parser,index_map_size);

	glGenBuffers(loadedModelsMaxIndex,Sys_Render.vbos);
	glGenBuffers(loadedModelsMaxIndex,Sys_Render.tbos);

	uint32_t total_vertices=0,total_tris=0;
	for(int i=0;i<loadedModelsMaxIndex;++i){
		if(unlikely(modelVertexCounts[i]==0))continue;
		size_t vert_size=(size_t)modelVertexCounts[i]*VERTEX_ATTRIBUTES_SIZE;
		total_vertices+=modelVertexCounts[i];
		size_t tri_size=(size_t)modelTriangleCounts[i]*3*sizeof(uint16_t);
		total_tris+=(uint32_t)tri_size;
		glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.vbos[i]);
		glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)vert_size,NULL,GL_STATIC_DRAW);
		void* ptr=glMapBufferRange(GL_ARRAY_BUFFER,0,(GLsizeiptr)vert_size,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
		__builtin_memcpy(ptr,modelVertices[i],vert_size);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)tri_size,NULL,GL_STATIC_DRAW);
		ptr=glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,0,(GLsizeiptr)tri_size,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
		__builtin_memcpy(ptr,modelTriangles[i],tri_size);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}

	DebugRAM("after to model to gpu transfer");
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	glFlush();glFinish();
	OS_DeallocateRAM(mpars.entries,mpars.count*sizeof(ModelData));
	DualLog(" total vertices: %u, total tris: %u, took %f secs\n",total_vertices,total_tris,get_time()-start_time);
	DebugRAM("After Load Models");
}
