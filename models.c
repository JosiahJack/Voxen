// models.c - 3D Models Loading System, Animation, Convex Edge Adjacency, Mesh Optimization
#include "common.h"
enum{MAX_GLTF_JOINTS=96,MAX_GLTF_TRIS=MAX_OUTPUT_VERTS/3,MAX_GLTF_VERTS=MAX_OUTPUT_VERTS,MAX_GLTF_BLOCKS=64};
float **vPos, **thrd_pos, **thread_temp_nrm, **thrd_uv, **thrd_verts; u32 **thrd_ht, **thrd_ht_used, **thrd_remap_scratch; u8** thrd_cache_scratch;
typedef struct { const char *data,*name; int size; } RawOBJ; typedef struct { u16 index; bool animated; u8 animationNum; u16* frames; u32 frameCount; char path[128]; } ModelData; typedef struct { ModelData* entries; u32 count,capacity; } ModelDataParser;
typedef struct { u32 start,end; int tid; } PhysGeomTask;
BvhNode** modelBVHNodes; u16** modelBVHTriOrder; u32 modelBVHNodeCounts[MAX_MDLS],modelBVHTriOrderCounts[MAX_MDLS];
typedef struct { BvhNode* nodes; u8* triOctants; u16 *triOrder, *triScratch,*initialTris; u32 nodeCount,triCount; } BvhBuildCtx;
static BvhBuildCtx thrd_bvh_ctx[32];
u16 uniqueCvxMeshIndices[MAX_UNIQUE_CVX_MESHES]; u32 uniqueCvxMeshCount=0;
u32* cvxAdjOffsets[MAX_UNIQUE_CVX_MESHES]; u16* cvxAdjLists[MAX_UNIQUE_CVX_MESHES]; // CSR format adjacency data: cvxAdjOffsets[m] has vCount + 1 entries pointing into cvxAdjLists[m]
u16 cvxAdjStart[MAX_UNIQUE_CVX_MESHES];
INLINE float fast_atof(const char** p) { const char* c=*p; while (*c == ' ' || *c == '\t') {c++;} float s=1.0f; if(*c == '-'){s=-1.0f; c++;} float v=0.0f; while (*c >= '0' && *c <= '9') { v=v * 10.0f + (*c - '0'); c++; } if (*c == '.') { c++; float sub=0.1f; while (*c >= '0' && *c <= '9') { v += (*c - '0') * sub; sub*=0.1f; c++; } } *p=c; return s * v; }
INLINE i32 fast_atoi(const char** p) { const char* c = *p; while (*c == ' ' || *c == '\t') {c++;} i32 s=1; if(*c == '-'){s=-1; c++;} i32 v = 0; while (*c >= '0' && *c <= '9') { v = v * 10 + (*c - '0'); c++; } *p = c; return v * s; }
typedef enum{cgltf_result_success,cgltf_result_data_too_short,cgltf_result_unknown_format,cgltf_result_invalid_json,cgltf_result_invalid_gltf,cgltf_result_file_not_found,cgltf_result_io_error,cgltf_result_legacy_gltf}cgltf_result;
typedef enum{cgltf_buffer_view_type_invalid,cgltf_buffer_view_type_indices,cgltf_buffer_view_type_vertices,cgltf_buffer_view_type_max_enum}cgltf_buffer_view_type;
typedef enum{cgltf_attribute_type_invalid,cgltf_attribute_type_position,cgltf_attribute_type_normal,cgltf_attribute_type_texcoord,cgltf_attribute_type_joints,cgltf_attribute_type_weights,cgltf_attribute_type_custom,cgltf_attribute_type_max_enum}cgltf_attribute_type;
typedef enum{cgltf_component_type_invalid,cgltf_component_type_r_8,cgltf_component_type_r_8u,cgltf_component_type_r_16,cgltf_component_type_r_16u,cgltf_component_type_r_32u,cgltf_component_type_r_32f,cgltf_component_type_max_enum}cgltf_component_type;
typedef enum{cgltf_type_invalid,cgltf_type_scalar,cgltf_type_vec2,cgltf_type_vec3,cgltf_type_vec4,cgltf_type_mat2,cgltf_type_mat3,cgltf_type_mat4,cgltf_type_max_enum}cgltf_type;
typedef enum{cgltf_primitive_type_invalid,cgltf_primitive_type_points,cgltf_primitive_type_lines,cgltf_primitive_type_line_loop,cgltf_primitive_type_line_strip,cgltf_primitive_type_triangles,cgltf_primitive_type_triangle_strip,cgltf_primitive_type_triangle_fan,cgltf_primitive_type_max_enum}cgltf_primitive_type;
typedef enum{cgltf_animation_path_type_invalid,cgltf_animation_path_type_translation,cgltf_animation_path_type_rotation,cgltf_animation_path_type_scale,cgltf_animation_path_type_weights,cgltf_animation_path_type_max_enum}cgltf_animation_path_type;
typedef enum{cgltf_interpolation_type_linear,cgltf_interpolation_type_step,cgltf_interpolation_type_cubic_spline,cgltf_interpolation_type_max_enum}cgltf_interpolation_type;
typedef struct{ char*name; size_t size; char*uri; void*data; } cgltf_buffer; 
typedef struct{char*name; cgltf_buffer*buffer; size_t offset,size,stride; cgltf_buffer_view_type type; void*data; }cgltf_buffer_view;
typedef struct{size_t count;cgltf_buffer_view*indices_buffer_view;size_t indices_byte_offset;cgltf_component_type indices_component_type;cgltf_buffer_view*values_buffer_view;size_t values_byte_offset;}cgltf_accessor_sparse;
typedef struct{char*name;cgltf_component_type component_type;bool normalized;cgltf_type type;size_t offset,count,stride;cgltf_buffer_view*buffer_view;bool has_min,has_max,is_sparse;float min[16],max[16];cgltf_accessor_sparse sparse;} cgltf_accessor;
typedef struct{char*name;cgltf_attribute_type type;i32 index;cgltf_accessor*data;}cgltf_attribute;
typedef struct{cgltf_primitive_type type; cgltf_accessor*indices; cgltf_attribute*attributes; size_t attributes_count; }cgltf_primitive;
typedef struct{char *name; cgltf_primitive *primitives; size_t primitives_count; float*weights; size_t weights_count;} cgltf_mesh;
typedef struct cgltf_node cgltf_node;
typedef struct{char*name;cgltf_node**joints;size_t joints_count;cgltf_node*skeleton;cgltf_accessor*inverse_bind_matrices;}cgltf_skin;
struct cgltf_node{char*name;cgltf_node*parent,**children;size_t children_count;cgltf_skin*skin;cgltf_mesh*mesh;float*weights;size_t weights_count;bool has_translation,has_rotation,has_scale,has_matrix;float translation[3],rotation[4],scale[3],matrix[16];};
typedef struct{cgltf_accessor*input,*output;cgltf_interpolation_type interpolation;}cgltf_animation_sampler;
typedef struct{cgltf_animation_sampler*sampler;cgltf_node*target_node;cgltf_animation_path_type target_path;}cgltf_animation_channel;
typedef struct{char*name;cgltf_animation_sampler*samplers;size_t samplers_count;cgltf_animation_channel*channels;size_t channels_count;}cgltf_animation;
typedef struct{void*file_data;size_t file_size;cgltf_mesh*meshes;size_t meshes_count;cgltf_accessor*accessors;size_t accessors_count;cgltf_buffer_view*buffer_views;size_t buffer_views_count;cgltf_buffer*buffers;size_t buffers_count;cgltf_skin*skins;size_t skins_count;cgltf_node*nodes;size_t nodes_count;cgltf_animation*animations;size_t animations_count;const char*json;size_t json_size;const void*bin;size_t bin_size;}cgltf_data;
typedef enum{JSMN_UNDEFINED=0,JSMN_OBJECT=1,JSMN_ARRAY=2,JSMN_STRING=3,JSMN_PRIMITIVE=4}jsmntype_t;
enum{JSMN_ERROR_NOMEM=-1,JSMN_ERROR_INVAL=-2,JSMN_ERROR_PART=-3};
typedef struct{jsmntype_t type;i64 start,end;i32 size,parent;}jsmntok_t;
typedef struct{size_t pos;u32 toknext;i32 toksuper;}jsmn_parser;
static void Mat4FromTRS(const float* T, const float* R, const float* S, float* lm) {
	float tx=T[0],ty=T[1],tz=T[2],qx=R[0],qy=R[1],qz=R[2],qw=R[3],sx=S[0],sy=S[1],sz=S[2];
	lm[0]=(1-2*qy*qy-2*qz*qz)*sx; lm[1]=(2*qx*qy+2*qz*qw)*sx; lm[2]=(2*qx*qz-2*qy*qw)*sx; lm[3]=lm[7]=lm[11]=0.0f; lm[4]=(2*qx*qy-2*qz*qw)*sy; lm[5]=(1-2*qx*qx-2*qz*qz)*sy; lm[6]=(2*qy*qz+2*qx*qw)*sy; lm[8]=(2*qx*qz+2*qy*qw)*sz; lm[9]=(2*qy*qz-2*qx*qw)*sz; lm[10]=(1-2*qx*qx-2*qy*qy)*sz; lm[12]=tx; lm[13]=ty; lm[14]=tz; lm[15]=1.0f;
}

void cgltf_node_transform_local(const cgltf_node* n, float* m){ if(n->has_matrix){mcpy(m,n->matrix,64);return;} Mat4FromTRS(n->translation, n->rotation, n->scale, m); }
static u64 cgltf_component_read_integer(const void* i, cgltf_component_type t){return t==cgltf_component_type_r_16?*((const i16*)i):t==cgltf_component_type_r_16u?*((const u16*)i):t==cgltf_component_type_r_32u?*((const u32*)i):t==cgltf_component_type_r_8?*((const i8*)i):t==cgltf_component_type_r_8u?*((const u8*)i):0;}
static size_t cgltf_component_read_index(const void* i, cgltf_component_type t){return t==cgltf_component_type_r_16u?*((const u16*)i):t==cgltf_component_type_r_32u?*((const u32*)i):t==cgltf_component_type_r_8u?*((const u8*)i):0;}
static float cgltf_component_read_float(const void* i, cgltf_component_type t, bool n) {
    if(t==cgltf_component_type_r_32f) return *((const float*)i);
    if(n) return t==cgltf_component_type_r_16?*((const i16*)i)/32767.f:t==cgltf_component_type_r_16u?*((const u16*)i)/65535.f:t==cgltf_component_type_r_8?*((const i8*)i)/127.f:t==cgltf_component_type_r_8u?*((const u8*)i)/255.f:0;
    return (float)cgltf_component_read_integer(i, t);
}

size_t cgltf_num_components(cgltf_type t){return t==cgltf_type_vec2?2:t==cgltf_type_vec3?3:t==cgltf_type_vec4?4:t==cgltf_type_mat2?4:t==cgltf_type_mat3?9:t==cgltf_type_mat4?16:1;}
size_t cgltf_component_size(cgltf_component_type ct){return ct==cgltf_component_type_r_8||ct==cgltf_component_type_r_8u?1:ct==cgltf_component_type_r_16||ct==cgltf_component_type_r_16u?2:ct==cgltf_component_type_r_32u||ct==cgltf_component_type_r_32f?4:0;}
static bool cgltf_element_read_float(const u8* e, cgltf_type ty, cgltf_component_type ct, bool n, float* o, size_t es){
    size_t nc=cgltf_num_components(ty); if(es<nc) return 0;
    size_t cs=cgltf_component_size(ct);
    if(ty==cgltf_type_mat2&&cs==1){o[0]=cgltf_component_read_float(e,ct,n);o[1]=cgltf_component_read_float(e+1,ct,n);o[2]=cgltf_component_read_float(e+4,ct,n);o[3]=cgltf_component_read_float(e+5,ct,n);return 1;}
    if(ty==cgltf_type_mat3&&cs==1){o[0]=cgltf_component_read_float(e,ct,n);o[1]=cgltf_component_read_float(e+1,ct,n);o[2]=cgltf_component_read_float(e+2,ct,n);o[3]=cgltf_component_read_float(e+4,ct,n);o[4]=cgltf_component_read_float(e+5,ct,n);o[5]=cgltf_component_read_float(e+6,ct,n);o[6]=cgltf_component_read_float(e+8,ct,n);o[7]=cgltf_component_read_float(e+9,ct,n);o[8]=cgltf_component_read_float(e+10,ct,n);return 1;}
    if(ty==cgltf_type_mat3&&cs==2){o[0]=cgltf_component_read_float(e,ct,n);o[1]=cgltf_component_read_float(e+2,ct,n);o[2]=cgltf_component_read_float(e+4,ct,n);o[3]=cgltf_component_read_float(e+8,ct,n);o[4]=cgltf_component_read_float(e+10,ct,n);o[5]=cgltf_component_read_float(e+12,ct,n);o[6]=cgltf_component_read_float(e+16,ct,n);o[7]=cgltf_component_read_float(e+18,ct,n);o[8]=cgltf_component_read_float(e+20,ct,n);return 1;}
    for(size_t i=0;i<nc;++i) o[i]=cgltf_component_read_float(e+cs*i,ct,n);
    return 1;
}

const u8* cgltf_buffer_view_data(const cgltf_buffer_view* v){if(v->data)return(const u8*)v->data;if(!v->buffer->data)return NULL;return(const u8*)v->buffer->data+v->offset;}
const cgltf_accessor* cgltf_find_accessor(const cgltf_primitive* p, cgltf_attribute_type t, i32 idx){for(size_t i=0;i<p->attributes_count;++i){const cgltf_attribute*a=&p->attributes[i];if(a->type==t&&a->index==idx)return a->data;}return NULL;}
static const u8* cgltf_find_sparse_index(const cgltf_accessor* a, size_t n){
    const cgltf_accessor_sparse*s=&a->sparse;const u8*id=cgltf_buffer_view_data(s->indices_buffer_view),*vd=cgltf_buffer_view_data(s->values_buffer_view);
    if(!id||!vd)return NULL;
    id+=s->indices_byte_offset;vd+=s->values_byte_offset;size_t is=cgltf_component_size(s->indices_component_type),o=0,l=s->count;
    while(l){size_t r=l%2;l/=2;size_t idx=cgltf_component_read_index(id+(o+l)*is,s->indices_component_type);o+=idx<n?l+r:0;}
    if(o==s->count)return NULL;
    size_t idx=cgltf_component_read_index(id+o*is,s->indices_component_type);
    return idx==n?vd+o*a->stride:NULL;
}

bool cgltf_accessor_read_float(const cgltf_accessor* a, size_t i, float* o, size_t es){
    if(a->is_sparse){const u8*e=cgltf_find_sparse_index(a,i);if(e)return cgltf_element_read_float(e,a->type,a->component_type,a->normalized,o,es);}
    if(!a->buffer_view){mset(o,0,es*sizeof(float));return 1;}
    const u8*e=cgltf_buffer_view_data(a->buffer_view);if(!e)return 0;
    e+=a->offset+a->stride*i;
    return cgltf_element_read_float(e,a->type,a->component_type,a->normalized,o,es);
}

size_t cgltf_accessor_read_index(const cgltf_accessor* a, size_t i){
    if(a->is_sparse){const u8*e=cgltf_find_sparse_index(a,i);if(e)return cgltf_component_read_index(e,a->component_type);}
    if(!a->buffer_view)return 0;
    const u8*e=cgltf_buffer_view_data(a->buffer_view);if(!e)return 0;
    e+=a->offset+a->stride*i;
    return cgltf_component_read_index(e,a->component_type);
}

#define CGLTF_ERROR_JSON -1
#define CGLTF_ERROR_LEGACY -3
#define CGLTF_CHECK_TOKTYPE(t, ty) if((t).type!=(ty))return CGLTF_ERROR_JSON;
#define CGLTF_CHECK_TOKTYPE_RET(t, ty, r) if((t).type!=(ty))return r;
#define CGLTF_CHECK_KEY(t) if((t).type!=JSMN_STRING||(t).size==0)return CGLTF_ERROR_JSON;
#define CGLTF_PTRINDEX(ty, idx) (ty*)((size_t)idx+1)
#define CGLTF_PTRFIXUP(v, d, s) if(v){if((size_t)v>s)return CGLTF_ERROR_JSON;v=&d[(size_t)v-1];}
#define CGLTF_PTRFIXUP_REQ(v, d, s) if(!v||(size_t)v>s)return CGLTF_ERROR_JSON;v=&d[(size_t)v-1];
static int cgltf_json_strcmp(jsmntok_t const* t, const u8* j, const char* s){CGLTF_CHECK_TOKTYPE(*t, JSMN_STRING); size_t sl=slen(s),nl=(size_t)(t->end-t->start); return sl==nl?sCompUpToLen((const char*)j+t->start,s,sl)==0:0; }
static int cgltf_json_to_int(jsmntok_t const* t, const u8* j){
    CGLTF_CHECK_TOKTYPE(*t, JSMN_PRIMITIVE);i32 r=0;const char*p=(const char*)j+t->start,*e=(const char*)j+t->end;bool n=0;
    if(*p=='-'){n=1;p++;}while(p<e&&*p>='0'&&*p<='9'){r=r*10+*p-'0';p++;}return n?-r:r;
}
static size_t cgltf_json_to_size(jsmntok_t const* t, const u8* j){
    CGLTF_CHECK_TOKTYPE_RET(*t, JSMN_PRIMITIVE, 0);size_t r=0;const char*p=(const char*)j+t->start,*e=(const char*)j+t->end;
    while(p<e&&*p>='0'&&*p<='9'){r=r*10+*p-'0';p++;}return r;
}
static float cgltf_json_to_float(jsmntok_t const* t, const u8* j){
    CGLTF_CHECK_TOKTYPE(*t, JSMN_PRIMITIVE);float r=0,f=0;int s=1;const char*p=(const char*)j+t->start,*e=(const char*)j+t->end;
    if(*p=='-'){s=-1;p++;}while(p<e&&*p>='0'&&*p<='9'){r=r*10+*p-'0';p++;}
    if(p<e&&*p=='.'){p++;float d=0.1f;while(p<e&&*p>='0'&&*p<='9'){f+=(*p-'0')*d;d*=0.1f;p++;}}
    return s*(r+f);
}
static bool cgltf_json_to_bool(jsmntok_t const* t, const u8* j){int sz=(int)(t->end-t->start);return sz==4&&sCompUpToLen((const char*)j+t->start,"true",4)==0;}
static int cgltf_skip_json(jsmntok_t const* t, int i){int e=i+1;while(i<e){switch(t[i].type){case JSMN_OBJECT:e+=t[i].size*2;break;case JSMN_ARRAY:e+=t[i].size;break;case JSMN_PRIMITIVE:case JSMN_STRING:break;default:return -1;}i++;}return i;}
static int cgltf_parse_json_float_array(jsmntok_t const* t, int i, const u8* j, float* o, int s){CGLTF_CHECK_TOKTYPE(t[i], JSMN_ARRAY);if(t[i].size!=s)return CGLTF_ERROR_JSON;++i;for(int k=0;k<s;++k){CGLTF_CHECK_TOKTYPE(t[i], JSMN_PRIMITIVE);o[k]=cgltf_json_to_float(t+i,j);++i;}return i;}
size_t cgltf_total_alloc = 0; // track cgltf parse tree leak
static int cgltf_parse_json_string(jsmntok_t const* t, int i, const u8* j, char** out){ CGLTF_CHECK_TOKTYPE(t[i], JSMN_STRING);if(*out)return CGLTF_ERROR_JSON; int sz=(int)(t[i].end-t[i].start);char*r=(char*)OS_AllocScratch(sz+1);cgltf_total_alloc+=sz+1;sCpy2aSubFromb(r,sz,(const char*)j+t[i].start,sz+1);*out=r;return i+1; }
static int cgltf_parse_json_array(jsmntok_t const* t, int i, const u8* j, size_t es, void** out, size_t* os){ (void)j;if(t[i].type!=JSMN_ARRAY)return CGLTF_ERROR_JSON;if(*out)return CGLTF_ERROR_JSON;int sz=t[i].size;*out=OS_AllocScratch(es*sz);cgltf_total_alloc+=es*sz;*os=sz;return i+1; }
typedef int (*cgltf_parse_item_func)(jsmntok_t const* t, int i, const u8* j, void* out);
static int cgltf_parse_json_array_generic(jsmntok_t const* t, int i, const u8* j, size_t elem_size, void** out_array, size_t* out_count, cgltf_parse_item_func parse_item) { i = cgltf_parse_json_array(t, i, j, elem_size, out_array, out_count); if (i < 0) return i; for (size_t k = 0; k < *out_count; ++k) { i = parse_item(t, i, j, (char*)*out_array + k * elem_size); if (i < 0) return i; } return i; }
static cgltf_component_type cgltf_json_to_component_type(jsmntok_t const* t, const u8* j){ int ty=cgltf_json_to_int(t,j); return ty==5120?cgltf_component_type_r_8:ty==5121?cgltf_component_type_r_8u:ty==5122?cgltf_component_type_r_16:ty==5123?cgltf_component_type_r_16u:ty==5125?cgltf_component_type_r_32u:ty==5126?cgltf_component_type_r_32f:cgltf_component_type_invalid; }
static int cgltf_parse_json_sparse_part(jsmntok_t const* t, int i, const u8* j, cgltf_buffer_view** out_bv, size_t* out_offset, cgltf_component_type* out_ct) {
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);
    int sz = t[i].size; ++i;
    for (int m = 0; m < sz; ++m) {
        CGLTF_CHECK_KEY(t[i]);
        if (cgltf_json_strcmp(t+i, j, "bufferView")) { ++i; *out_bv = CGLTF_PTRINDEX(cgltf_buffer_view, cgltf_json_to_int(t+i, j)); ++i; }
        else if (cgltf_json_strcmp(t+i, j, "byteOffset")) { ++i; *out_offset = cgltf_json_to_size(t+i, j); ++i; }
        else if (out_ct && cgltf_json_strcmp(t+i, j, "componentType")) { ++i; *out_ct = cgltf_json_to_component_type(t+i, j); ++i; }
        else i = cgltf_skip_json(t, i+1);
        if (i < 0) return i;
    }
    return i;
}

static int cgltf_parse_json_node_array(jsmntok_t const* t, int i, const u8* j, cgltf_node*** out, size_t* out_count) { i = cgltf_parse_json_array(t, i, j, sizeof(cgltf_node*), (void**)out, out_count); if (i < 0) return i; for (size_t m = 0; m < *out_count; ++m) { (*out)[m] = CGLTF_PTRINDEX(cgltf_node, cgltf_json_to_int(t+i, j)); ++i; } return i; }
static int cgltf_parse_json_float_array_alloc(jsmntok_t const* t, int i, const u8* j, float** out, size_t* out_count) { i = cgltf_parse_json_array(t, i, j, sizeof(float), (void**)out, out_count); if (i < 0) return i; return cgltf_parse_json_float_array(t, i-1, j, *out, (int)*out_count); }
static void cgltf_parse_attribute_type(const char* n, cgltf_attribute_type* ot, int* oi){
    if(*n=='_'){*ot=cgltf_attribute_type_custom;return;}
    const char* us=StringFindFirstCharWithin(n,'_');size_t l=us?(size_t)(us-n):slen(n);
    *ot = l==8&&sCompUpToLen(n,"POSITION",8)==0?cgltf_attribute_type_position: l==6&&sCompUpToLen(n,"NORMAL",6)==0?cgltf_attribute_type_normal: l==8&&sCompUpToLen(n,"TEXCOORD",8)==0?cgltf_attribute_type_texcoord: l==6&&sCompUpToLen(n,"JOINTS",6)==0?cgltf_attribute_type_joints: l==7&&sCompUpToLen(n,"WEIGHTS",7)==0?cgltf_attribute_type_weights:cgltf_attribute_type_invalid;
    if(us&&*ot!=cgltf_attribute_type_invalid){*oi=s2i32(us+1);if(*oi<0){*ot=cgltf_attribute_type_invalid;*oi=0;}}
}

static int cgltf_parse_json_attribute_list(jsmntok_t const* t, int i, const u8* j, cgltf_attribute** out, size_t* oc){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);if(*out)return CGLTF_ERROR_JSON;
    *oc=t[i].size;*out=(cgltf_attribute*)OS_Alloc(sizeof(cgltf_attribute)**oc);cgltf_total_alloc+=sizeof(cgltf_attribute)**oc;++i;
    for(size_t k=0;k<*oc;++k){
        CGLTF_CHECK_KEY(t[i]);i=cgltf_parse_json_string(t,i,j,&(*out)[k].name);if(i<0)return CGLTF_ERROR_JSON;
        cgltf_parse_attribute_type((*out)[k].name,&(*out)[k].type,&(*out)[k].index);
        (*out)[k].data=CGLTF_PTRINDEX(cgltf_accessor,cgltf_json_to_int(t+i,j));++i;
    }
    return i;
}

static cgltf_primitive_type cgltf_json_to_primitive_type(jsmntok_t const* t, const u8* j){
    int ty=cgltf_json_to_int(t,j);
    return ty==0?cgltf_primitive_type_points:ty==1?cgltf_primitive_type_lines:ty==2?cgltf_primitive_type_line_loop:ty==3?cgltf_primitive_type_line_strip:ty==4?cgltf_primitive_type_triangles:ty==5?cgltf_primitive_type_triangle_strip:ty==6?cgltf_primitive_type_triangle_fan:cgltf_primitive_type_invalid;
}

static int cgltf_parse_json_primitive(jsmntok_t const* t, int i, const u8* j, cgltf_primitive* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);out->type=cgltf_primitive_type_triangles;int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
             if(cgltf_json_strcmp(t+i,j,"mode")){++i;out->type=cgltf_json_to_primitive_type(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"indices")){++i;out->indices=CGLTF_PTRINDEX(cgltf_accessor,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"attributes"))i=cgltf_parse_json_attribute_list(t,i+1,j,&out->attributes,&out->attributes_count);
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_mesh(jsmntok_t const* t, int i, const u8* j, cgltf_mesh* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
             if(cgltf_json_strcmp(t+i,j,      "name")){i=cgltf_parse_json_string(t,i+1,j,&out->name);}
        else if(cgltf_json_strcmp(t+i,j,"primitives")){i=cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_primitive), (void**)&out->primitives, &out->primitives_count, (cgltf_parse_item_func)cgltf_parse_json_primitive);}
        else if(cgltf_json_strcmp(t+i,j,   "weights")){i=cgltf_parse_json_float_array_alloc(t, i+1, j, &out->weights, &out->weights_count);}
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_accessor_sparse(jsmntok_t const* t, int i, const u8* j, cgltf_accessor_sparse* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT); int sz = t[i].size; ++i;
    for (int k = 0; k < sz; ++k) {
        CGLTF_CHECK_KEY(t[i]);
        if (cgltf_json_strcmp(t+i, j, "count")) { ++i; out->count = cgltf_json_to_size(t+i, j); ++i; }
        else if (cgltf_json_strcmp(t+i, j, "indices")) { ++i; i = cgltf_parse_json_sparse_part(t, i, j, &out->indices_buffer_view, &out->indices_byte_offset, &out->indices_component_type); }
        else if (cgltf_json_strcmp(t+i, j, "values")) { ++i; i = cgltf_parse_json_sparse_part(t, i, j, &out->values_buffer_view, &out->values_byte_offset, NULL); }
        else i = cgltf_skip_json(t, i+1);
        if (i < 0) return i;
    } return i;
}

static int cgltf_parse_json_accessor(jsmntok_t const* t, int i, const u8* j, cgltf_accessor* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
        if(cgltf_json_strcmp(t+i,j,"name"))i=cgltf_parse_json_string(t,i+1,j,&out->name);
        else if(cgltf_json_strcmp(t+i,j,"bufferView")){++i;out->buffer_view=CGLTF_PTRINDEX(cgltf_buffer_view,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"byteOffset")){++i;out->offset=cgltf_json_to_size(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"componentType")){++i;out->component_type=cgltf_json_to_component_type(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"normalized")){++i;out->normalized=cgltf_json_to_bool(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"count")){++i;out->count=cgltf_json_to_size(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"type")){
            ++i;
            out->type = cgltf_json_strcmp(t+i,j,"SCALAR")?cgltf_type_scalar:cgltf_json_strcmp(t+i,j,"VEC2")?cgltf_type_vec2:cgltf_json_strcmp(t+i,j,"VEC3")?cgltf_type_vec3:cgltf_json_strcmp(t+i,j,"VEC4")?cgltf_type_vec4:cgltf_json_strcmp(t+i,j,"MAT2")?cgltf_type_mat2:cgltf_json_strcmp(t+i,j,"MAT3")?cgltf_type_mat3:cgltf_json_strcmp(t+i,j,"MAT4")?cgltf_type_mat4:cgltf_type_invalid;
            ++i;
        } else if(cgltf_json_strcmp(t+i,j,"min")){++i;out->has_min=1;int ms=t[i].size>16?16:t[i].size;i=cgltf_parse_json_float_array(t,i,j,out->min,ms);}
        else if(cgltf_json_strcmp(t+i,j,"max")){++i;out->has_max=1;int ms=t[i].size>16?16:t[i].size;i=cgltf_parse_json_float_array(t,i,j,out->max,ms);}
        else if(cgltf_json_strcmp(t+i,j,"sparse")){out->is_sparse=1;i=cgltf_parse_json_accessor_sparse(t,i+1,j,&out->sparse);}
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_buffer_view(jsmntok_t const* t, int i, const u8* j, cgltf_buffer_view* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
        if(cgltf_json_strcmp(t+i,j,"name"))i=cgltf_parse_json_string(t,i+1,j,&out->name);
        else if(cgltf_json_strcmp(t+i,j,"buffer")){++i;out->buffer=CGLTF_PTRINDEX(cgltf_buffer,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"byteOffset")){++i;out->offset=cgltf_json_to_size(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"byteLength")){++i;out->size=cgltf_json_to_size(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"byteStride")){++i;out->stride=cgltf_json_to_size(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"target")){++i;int ty=cgltf_json_to_int(t+i,j);out->type=ty==34962?cgltf_buffer_view_type_vertices:ty==34963?cgltf_buffer_view_type_indices:cgltf_buffer_view_type_invalid;++i;}
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}


static int cgltf_parse_json_buffer(jsmntok_t const* t, int i, const u8* j, cgltf_buffer* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
        if(cgltf_json_strcmp(t+i,j,"name"))i=cgltf_parse_json_string(t,i+1,j,&out->name);
        else if(cgltf_json_strcmp(t+i,j,"byteLength")){++i;out->size=cgltf_json_to_size(t+i,j);++i;}
        else if(cgltf_json_strcmp(t+i,j,"uri"))i=cgltf_parse_json_string(t,i+1,j,&out->uri);
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_skin(jsmntok_t const* t, int i, const u8* j, cgltf_skin* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
             if (cgltf_json_strcmp(t+i,j,"name")){i=cgltf_parse_json_string(t,i+1,j,&out->name);}
        else if (cgltf_json_strcmp(t+i,j,"joints")) { ++i; i = cgltf_parse_json_node_array(t,i,j,&out->joints,&out->joints_count); }
        else if (cgltf_json_strcmp(t+i,j,"skeleton")) {++i;CGLTF_CHECK_TOKTYPE(t[i],JSMN_PRIMITIVE);out->skeleton=CGLTF_PTRINDEX(cgltf_node,cgltf_json_to_int(t+i,j));++i;}
        else if (cgltf_json_strcmp(t+i,j,"inverseBindMatrices")) {++i;CGLTF_CHECK_TOKTYPE(t[i],JSMN_PRIMITIVE);out->inverse_bind_matrices=CGLTF_PTRINDEX(cgltf_accessor,cgltf_json_to_int(t+i,j));++i;}
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_node(jsmntok_t const* t, int i, const u8* j, cgltf_node* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);
    out->rotation[3]=1.0f;out->scale[0]=1.0f;out->scale[1]=1.0f;out->scale[2]=1.0f;out->matrix[0]=1.0f;out->matrix[5]=1.0f;out->matrix[10]=1.0f;out->matrix[15]=1.0f;
    int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
        if(cgltf_json_strcmp(t+i,j,"name"))i=cgltf_parse_json_string(t,i+1,j,&out->name);
        else if (cgltf_json_strcmp(t+i,j,"children")) { ++i; i = cgltf_parse_json_node_array(t, i, j, &out->children, &out->children_count); }
        else if(cgltf_json_strcmp(t+i,j,"mesh")){++i;CGLTF_CHECK_TOKTYPE(t[i], JSMN_PRIMITIVE);out->mesh=CGLTF_PTRINDEX(cgltf_mesh,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"skin")){++i;CGLTF_CHECK_TOKTYPE(t[i], JSMN_PRIMITIVE);out->skin=CGLTF_PTRINDEX(cgltf_skin,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"translation")){out->has_translation=1;i=cgltf_parse_json_float_array(t,i+1,j,out->translation,3);}
        else if(cgltf_json_strcmp(t+i,j,"rotation")){out->has_rotation=1;i=cgltf_parse_json_float_array(t,i+1,j,out->rotation,4);}
        else if(cgltf_json_strcmp(t+i,j,"scale")){out->has_scale=1;i=cgltf_parse_json_float_array(t,i+1,j,out->scale,3);}
        else if(cgltf_json_strcmp(t+i,j,"matrix")){out->has_matrix=1;i=cgltf_parse_json_float_array(t,i+1,j,out->matrix,16);}
        else if (cgltf_json_strcmp(t+i,j,"weights")) {i = cgltf_parse_json_float_array_alloc(t,i+1,j,&out->weights,&out->weights_count);}
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_animation_sampler(jsmntok_t const* t, int i, const u8* j, cgltf_animation_sampler* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
        if(cgltf_json_strcmp(t+i,j,"input")){++i;out->input=CGLTF_PTRINDEX(cgltf_accessor,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"output")){++i;out->output=CGLTF_PTRINDEX(cgltf_accessor,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"interpolation")){ ++i; out->interpolation = cgltf_json_strcmp(t+i,j,"LINEAR")?cgltf_interpolation_type_linear:cgltf_json_strcmp(t+i,j,"STEP")?cgltf_interpolation_type_step:cgltf_json_strcmp(t+i,j,"CUBICSPLINE")?cgltf_interpolation_type_cubic_spline:cgltf_interpolation_type_linear; ++i; } else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_animation_channel(jsmntok_t const* t, int i, const u8* j, cgltf_animation_channel* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
        if(cgltf_json_strcmp(t+i,j,"sampler")){++i;out->sampler=CGLTF_PTRINDEX(cgltf_animation_sampler,cgltf_json_to_int(t+i,j));++i;}
        else if(cgltf_json_strcmp(t+i,j,"target")){
            ++i;CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int tsz=t[i].size;++i;
            for(int m=0;m<tsz;++m){
                CGLTF_CHECK_KEY(t[i]);
                if(cgltf_json_strcmp(t+i,j,"node")){++i;out->target_node=CGLTF_PTRINDEX(cgltf_node,cgltf_json_to_int(t+i,j));++i;}
                else if(cgltf_json_strcmp(t+i,j,"path")){ ++i; out->target_path = cgltf_json_strcmp(t+i,j,"translation") ? cgltf_animation_path_type_translation : cgltf_json_strcmp(t+i,j,"rotation") ? cgltf_animation_path_type_rotation : cgltf_json_strcmp(t+i,j,"scale") ? cgltf_animation_path_type_scale : cgltf_animation_path_type_invalid; ++i; }
                else i=cgltf_skip_json(t,i+1);
                if(i<0)return i;
            }
        } else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    } return i;
}

static int cgltf_parse_json_animation(jsmntok_t const* t, int i, const u8* j, cgltf_animation* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT);int sz=t[i].size;++i;
    for(int k=0;k<sz;++k){
        CGLTF_CHECK_KEY(t[i]);
             if(cgltf_json_strcmp(t+i,j,"name"))i=cgltf_parse_json_string(t,i+1,j,&out->name);
        else if(cgltf_json_strcmp(t+i,j,"samplers")){i=cgltf_parse_json_array_generic(t,i+1,j,sizeof(cgltf_animation_sampler),(void**)&out->samplers,&out->samplers_count,(cgltf_parse_item_func)cgltf_parse_json_animation_sampler); } 
        else if(cgltf_json_strcmp(t+i,j,"channels")){i=cgltf_parse_json_array(t,i+1,j,sizeof(cgltf_animation_channel),(void**)&out->channels,&out->channels_count);if(i<0)return i; for(size_t m=0;m<out->channels_count;++m){i=cgltf_parse_json_animation_channel(t,i,j,&out->channels[m]);if(i<0)return i;} }
        else i=cgltf_skip_json(t,i+1);
        if(i<0)return i;
    }
    return i;
}

static int cgltf_parse_json_root(jsmntok_t const* t, int i, const u8* j, cgltf_data* out){
    CGLTF_CHECK_TOKTYPE(t[i], JSMN_OBJECT); int sz = t[i].size; ++i;
    for (int k = 0; k < sz; ++k) {
        CGLTF_CHECK_KEY(t[i]);
        if (cgltf_json_strcmp(t+i, j, "meshes")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_mesh), (void**)&out->meshes, &out->meshes_count, (cgltf_parse_item_func)cgltf_parse_json_mesh);
        else if (cgltf_json_strcmp(t+i, j, "accessors")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_accessor), (void**)&out->accessors, &out->accessors_count, (cgltf_parse_item_func)cgltf_parse_json_accessor);
        else if (cgltf_json_strcmp(t+i, j, "bufferViews")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_buffer_view), (void**)&out->buffer_views, &out->buffer_views_count, (cgltf_parse_item_func)cgltf_parse_json_buffer_view);
        else if (cgltf_json_strcmp(t+i, j, "buffers")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_buffer), (void**)&out->buffers, &out->buffers_count, (cgltf_parse_item_func)cgltf_parse_json_buffer);
        else if (cgltf_json_strcmp(t+i, j, "skins")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_skin), (void**)&out->skins, &out->skins_count, (cgltf_parse_item_func)cgltf_parse_json_skin);
        else if (cgltf_json_strcmp(t+i, j, "nodes")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_node), (void**)&out->nodes, &out->nodes_count, (cgltf_parse_item_func)cgltf_parse_json_node);
        else if (cgltf_json_strcmp(t+i, j, "animations")) i = cgltf_parse_json_array_generic(t, i+1, j, sizeof(cgltf_animation), (void**)&out->animations, &out->animations_count, (cgltf_parse_item_func)cgltf_parse_json_animation);
        else i = cgltf_skip_json(t, i+1);
        if (i < 0) return i;
    } return i;
}

static void jsmn_init(jsmn_parser* p){p->pos=0;p->toknext=0;p->toksuper=-1;}
static jsmntok_t* jsmn_alloc_token(jsmn_parser* p, jsmntok_t* t, size_t n){if(p->toknext>=n)return NULL;jsmntok_t* tok=&t[p->toknext++];tok->start=tok->end=-1;tok->size=0;tok->parent=-1;return tok;}
static void jsmn_fill_token(jsmntok_t* t, jsmntype_t ty, size_t s, size_t e){t->type=ty;t->start=s;t->end=e;t->size=0;}
static int jsmn_parse_primitive(jsmn_parser* p, const char* js, size_t l, jsmntok_t* t, size_t n){
    size_t s=p->pos; for(;p->pos<l&&js[p->pos];p->pos++){switch(js[p->pos]){case ':':case '\t':case '\r':case '\n':case ' ':case ',':case ']':case '}':goto f;}}
    p->pos=s;return JSMN_ERROR_PART; f:if(!t){p->pos--;return 0;} jsmntok_t* tok=jsmn_alloc_token(p,t,n);if(!tok){p->pos=s;return JSMN_ERROR_NOMEM;} jsmn_fill_token(tok,JSMN_PRIMITIVE,s,p->pos);tok->parent=p->toksuper;p->pos--;return 0;
}

static int jsmn_parse_string(jsmn_parser* p, const char* js, size_t l, jsmntok_t* t, size_t n){
    size_t s=p->pos;p->pos++;
    for(;p->pos<l&&js[p->pos];p->pos++){
        char c=js[p->pos];
        if(c=='\"'){if(!t)return 0;jsmntok_t* tok=jsmn_alloc_token(p,t,n);if(!tok){p->pos=s;return JSMN_ERROR_NOMEM;}jsmn_fill_token(tok,JSMN_STRING,s+1,p->pos);tok->parent=p->toksuper;return 0;}
        if(c=='\\'&&p->pos+1<l){p->pos++;switch(js[p->pos]){case '\"':case '/':case '\\':case 'b':case 'f':case 'r':case 'n':case 't':break;case 'u':p->pos++;for(int i=0;i<4&&p->pos<l&&js[p->pos];i++){if(!((js[p->pos]>=48&&js[p->pos]<=57)||(js[p->pos]>=65&&js[p->pos]<=70)||(js[p->pos]>=97&&js[p->pos]<=102))){p->pos=s;return JSMN_ERROR_INVAL;}p->pos++;}p->pos--;break;default:p->pos=s;return JSMN_ERROR_INVAL;}}
    }
    p->pos=s;return JSMN_ERROR_PART;
}

static int jsmn_parse(jsmn_parser* p, const char* js, size_t l, jsmntok_t* t, size_t n){
    int r,i,cnt=p->toknext;
    for(;p->pos<l&&js[p->pos];p->pos++){
        char c=js[p->pos];jsmntype_t ty;
        switch(c){
        case '{':case '[':
            cnt++;if(!t)break;jsmntok_t* tok=jsmn_alloc_token(p,t,n);if(!tok)return JSMN_ERROR_NOMEM;
            if(p->toksuper!=-1){t[p->toksuper].size++;tok->parent=p->toksuper;}
            tok->type=(c=='{'?JSMN_OBJECT:JSMN_ARRAY);tok->start=p->pos;p->toksuper=p->toknext-1;break;
        case '}':case ']':
            if(!t)break;ty=(c=='}'?JSMN_OBJECT:JSMN_ARRAY);if(p->toknext<1)return JSMN_ERROR_INVAL;
            tok=&t[p->toknext-1];
            for(;;){if(tok->start!=-1&&tok->end==-1){if(tok->type!=ty)return JSMN_ERROR_INVAL;tok->end=p->pos+1;p->toksuper=tok->parent;break;}if(tok->parent==-1){if(tok->type!=ty||p->toksuper==-1)return JSMN_ERROR_INVAL;break;}tok=&t[tok->parent];}
            break;
        case '\"':
            r=jsmn_parse_string(p,js,l,t,n);if(r<0)return r;cnt++;if(p->toksuper!=-1&&t)t[p->toksuper].size++;break;
        case '\t':case '\r':case '\n':case ' ':break;
        case ':':p->toksuper=p->toknext-1;break;
        case ',':
            if(t&&p->toksuper!=-1&&t[p->toksuper].type!=JSMN_ARRAY&&t[p->toksuper].type!=JSMN_OBJECT){p->toksuper=t[p->toksuper].parent;for(i=p->toknext-1;i>=0;i--){if(t[i].type==JSMN_ARRAY||t[i].type==JSMN_OBJECT){if(t[i].start!=-1&&t[i].end==-1){p->toksuper=i;break;}}}}
            break;
        case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case 't':case 'f':case 'n':
            if(t&&p->toksuper!=-1){jsmntok_t* ot=&t[p->toksuper];if(ot->type==JSMN_OBJECT||(ot->type==JSMN_STRING&&ot->size!=0))return JSMN_ERROR_INVAL;}
            r=jsmn_parse_primitive(p,js,l,t,n);if(r<0)return r;cnt++;if(p->toksuper!=-1&&t)t[p->toksuper].size++;break;
        default:return JSMN_ERROR_INVAL;
        }
    }
    if(t){for(i=p->toknext-1;i>=0;i--){if(t[i].start!=-1&&t[i].end==-1)return JSMN_ERROR_PART;}}
    return cnt;
}

size_t cgltf_calc_size(cgltf_type ty, cgltf_component_type ct){size_t cs=cgltf_component_size(ct);if(ty==cgltf_type_mat2&&cs==1)return 8*cs;if(ty==cgltf_type_mat3&&(cs==1||cs==2))return 12*cs;return cs*cgltf_num_components(ty);}
cgltf_result cgltf_parse(const void* d, size_t sz, cgltf_data** out_data) {
    if(sz<12){DualLogError("Data passed too short for glb\n");OS_Exit(1);}
    u32 tmp;mcpy(&tmp,d,4);if(tmp!=0x46546C67){DualLogError("invalid glb file\n");OS_Exit(1);}
    const u8* ptr=(const u8*)d;mcpy(&tmp,ptr+4,4);mcpy(&tmp,ptr+8,4);if(tmp>sz){DualLogError("Data too short for glb\n");OS_Exit(1);}
    const u8* jc=ptr+12;if(20>sz){DualLogError("Data too short for glb\n");OS_Exit(1);}
    u32 jl;mcpy(&jl,jc,4);if(jl>sz-20){DualLogError("Data too short for glb\n");OS_Exit(1);}
    mcpy(&tmp,jc+4,4);if(tmp!=0x4E4F534A){DualLogError("Unknown format for glb\n");OS_Exit(1);}
    jc+=8;const void* bin=NULL;size_t bsz=0;
    if(8<=sz-20-jl){
        const u8* bc=jc+jl;u32 bl;mcpy(&bl,bc,4);
        if(bl>sz-20-jl-8){DualLogError("Data too short for glb\n");OS_Exit(1);}
        mcpy(&tmp,bc+4,4);if(tmp!=0x004E4942){DualLogError("Unknown format for glb\n");OS_Exit(1);}
        bc+=8;bin=bc;bsz=bl;
    }
    jsmn_parser p={0,0,0};int tc=jsmn_parse(&p,(const char*)jc,jl,NULL,0);if(tc<=0){DualLogError("No tokens in glb\n");OS_Exit(1);}
    jsmntok_t* t=(jsmntok_t*)OS_AllocScratch(sizeof(jsmntok_t)*(tc+1));jsmn_init(&p);
    tc=jsmn_parse(&p,(const char*)jc,jl,t,tc);if(tc<=0){DualLogError("No tokens in glb\n");OS_Exit(1);}
    t[tc].type=JSMN_UNDEFINED;
    cgltf_data* data=(cgltf_data*)OS_AllocScratch(sizeof(cgltf_data)); cgltf_total_alloc += sizeof(cgltf_data);
    int i=cgltf_parse_json_root(t,0,jc,data);
    if(i<0){DualLogError("Error parsing json in glb\n");OS_Exit(1);}
    for(size_t m=0;m<data->meshes_count;++m)
        for(size_t n=0;n<data->meshes[m].primitives_count;++n){ CGLTF_PTRFIXUP(data->meshes[m].primitives[n].indices,data->accessors,data->accessors_count); for(size_t k=0;k<data->meshes[m].primitives[n].attributes_count;++k){CGLTF_PTRFIXUP_REQ(data->meshes[m].primitives[n].attributes[k].data,data->accessors,data->accessors_count);} }
    for(size_t m=0;m<data->accessors_count;++m){
        CGLTF_PTRFIXUP(data->accessors[m].buffer_view,data->buffer_views,data->buffer_views_count);
        if(data->accessors[m].is_sparse){CGLTF_PTRFIXUP_REQ(data->accessors[m].sparse.indices_buffer_view,data->buffer_views,data->buffer_views_count);CGLTF_PTRFIXUP_REQ(data->accessors[m].sparse.values_buffer_view,data->buffer_views,data->buffer_views_count);}
        if(data->accessors[m].buffer_view)data->accessors[m].stride=data->accessors[m].buffer_view->stride;
        if(data->accessors[m].stride==0){data->accessors[m].stride=cgltf_calc_size(data->accessors[m].type,data->accessors[m].component_type);}
    }
    for(size_t m=0;m<data->buffer_views_count;++m){CGLTF_PTRFIXUP_REQ(data->buffer_views[m].buffer,data->buffers,data->buffers_count);}
    for(size_t m=0;m<data->skins_count;++m){for(size_t n=0;n<data->skins[m].joints_count;++n){CGLTF_PTRFIXUP_REQ(data->skins[m].joints[n],data->nodes,data->nodes_count);}CGLTF_PTRFIXUP(data->skins[m].skeleton,data->nodes,data->nodes_count);CGLTF_PTRFIXUP(data->skins[m].inverse_bind_matrices,data->accessors,data->accessors_count);}
    for(size_t m=0;m<data->nodes_count;++m){
        for(size_t n=0;n<data->nodes[m].children_count;++n){CGLTF_PTRFIXUP_REQ(data->nodes[m].children[n],data->nodes,data->nodes_count);if(data->nodes[m].children[n]->parent){DualLogError("JSON error when attempting to fixup pointers\n");OS_Exit(1);}data->nodes[m].children[n]->parent=&data->nodes[m];}
        CGLTF_PTRFIXUP(data->nodes[m].mesh,data->meshes,data->meshes_count); CGLTF_PTRFIXUP(data->nodes[m].skin,data->skins,data->skins_count);
    }
    for(size_t m=0;m<data->animations_count;++m){
        for(size_t n=0;n<data->animations[m].samplers_count;++n){CGLTF_PTRFIXUP_REQ(data->animations[m].samplers[n].input,data->accessors,data->accessors_count);CGLTF_PTRFIXUP_REQ(data->animations[m].samplers[n].output,data->accessors,data->accessors_count);}
        for(size_t n=0;n<data->animations[m].channels_count;++n){CGLTF_PTRFIXUP_REQ(data->animations[m].channels[n].sampler,data->animations[m].samplers,data->animations[m].samplers_count);CGLTF_PTRFIXUP(data->animations[m].channels[n].target_node,data->nodes,data->nodes_count);}
    }
    data->json=(const char*)jc;data->json_size=jl;*out_data=data; (*out_data)->bin=bin;(*out_data)->bin_size=bsz; return cgltf_result_success;
}

static void cgltf_combine_paths(char* p, const char* b, const char* u) { const char* s0=StringFindLastChar(b,'/'),*s1=StringFindLastChar(b,'\\'),*sl=s0?(s1&&s1>s0?s1:s0):s1; size_t sz=0; if(sl){sz=sl-b+1;for(size_t i=0;i<sz;++i)p[i]=b[i];} for(size_t i=0;u[i];++i)p[sz+i]=u[i];p[sz+slen(u)]=0; }
static int cgltf_unhex(char c){return(u8)(c-'0')<10?c-'0':(u8)(c-'A')<6?c-'A'+10:(u8)(c-'a')<6?c-'a'+10:-1;}
size_t cgltf_decode_uri(char* u){char*w=u,*i=u;while(*i){if(*i=='%'){int h1=cgltf_unhex(i[1]);if(h1>=0){int h2=cgltf_unhex(i[2]);if(h2>=0){*w++=(char)(h1*16+h2);i+=3;continue;}}}*w++=*i++;}*w=0;return w-u;}
cgltf_result cgltf_load_buffer_base64(size_t sz, const char* b64, void** out) {
    u8* d=(u8*)OS_Alloc(sz);cgltf_total_alloc+=sz;u32 buf=0,bb=0;
    for(size_t i=0;i<sz;++i){ while(bb<8){ char c=*b64++;int idx=(u8)(c-'A')<26?c-'A':(u8)(c-'a')<26?c-'a'+26:(u8)(c-'0')<10?c-'0'+52:c=='+'?62:c=='/'?63:-1; if(idx<0){OS_FreeInitPhaseInner(sz);return cgltf_result_io_error;} buf=(buf<<6)|idx;bb+=6; } d[i]=(u8)(buf>>(bb-8));bb-=8; }
    *out=d;return cgltf_result_success;
}

cgltf_result cgltf_load_buffers(cgltf_data* data, const char* gltf_path) {
    if(data->buffers_count&&data->buffers[0].data==NULL&&data->buffers[0].uri==NULL&&data->bin){if(data->bin_size<data->buffers[0].size)return cgltf_result_data_too_short;data->buffers[0].data=(void*)data->bin;}
    for(size_t i=0;i<data->buffers_count;++i){
        if(data->buffers[i].data)continue;
        const char* uri=data->buffers[i].uri;if(!uri)continue;
        if(sCompUpToLen(uri,"data:",5)){
            const char* comma=StringFindFirstCharWithin(uri,',');
            if(comma&&comma-uri>=7&&sCompUpToLen(comma-7,";base64",7)){cgltf_result r=cgltf_load_buffer_base64(data->buffers[i].size,comma+1,&data->buffers[i].data);if(r!=cgltf_result_success)return r;}
            else return cgltf_result_unknown_format;
        } else if(sFindSub(uri,"://")==NULL&&gltf_path){
            size_t psz=slen(uri)+slen(gltf_path)+1;char* path=(char*)OS_AllocScratch(psz);
            cgltf_combine_paths(path,gltf_path,uri);
            cgltf_decode_uri(path+slen(path)-slen(uri));
            FHandle fp=OS_OpenReadonly(path);
            int fsz=OS_FileSize(fp);
            u8* fb=OS_AllocateFileBackedRAMReadonly(fsz,fp,path);
            OS_Close(fp);OS_FreeInitPhaseInner(psz);data->buffers[i].data=fb;
        } else return cgltf_result_unknown_format;
    } return cgltf_result_success;
}

static size_t cgltf_calc_index_bound(cgltf_buffer_view* bv, size_t off, cgltf_component_type ct, size_t cnt) {
    char* d=(char*)bv->buffer->data+off+bv->offset;size_t b=0;
    if(ct==cgltf_component_type_r_8u)for(size_t i=0;i<cnt;++i)b=b>((u8*)d)[i]?b:((u8*)d)[i];
    else if(ct==cgltf_component_type_r_16u)for(size_t i=0;i<cnt;++i)b=b>((u16*)d)[i]?b:((u16*)d)[i];
    else if(ct==cgltf_component_type_r_32u)for(size_t i=0;i<cnt;++i)b=b>((u32*)d)[i]?b:((u32*)d)[i];
    return b;
}

cgltf_result cgltf_validate(cgltf_data* data) {
    for(size_t i=0;i<data->accessors_count;++i){
        cgltf_accessor* a=&data->accessors[i];
        if(a->component_type==cgltf_component_type_invalid||a->type==cgltf_type_invalid)return cgltf_result_invalid_gltf;
        size_t es=cgltf_calc_size(a->type,a->component_type);
        if(a->buffer_view){size_t rq=a->offset+a->stride*(a->count-1)+es;if(a->buffer_view->size<rq)return cgltf_result_data_too_short;}
        if(a->is_sparse){
            cgltf_accessor_sparse* s=&a->sparse;size_t ics=cgltf_component_size(s->indices_component_type);
            size_t irq=s->indices_byte_offset+ics*s->count,vrq=s->values_byte_offset+es*s->count;
            if(s->indices_buffer_view->size<irq||s->values_buffer_view->size<vrq)return cgltf_result_data_too_short;
            if(s->indices_component_type!=cgltf_component_type_r_8u&&s->indices_component_type!=cgltf_component_type_r_16u&&s->indices_component_type!=cgltf_component_type_r_32u)return cgltf_result_invalid_gltf;
            if(s->indices_buffer_view->buffer->data){size_t ib=cgltf_calc_index_bound(s->indices_buffer_view,s->indices_byte_offset,s->indices_component_type,s->count);if(ib>=a->count)return cgltf_result_data_too_short;}
        }
    }
    for(size_t i=0;i<data->buffer_views_count;++i){ size_t rq=data->buffer_views[i].offset+data->buffer_views[i].size; if(data->buffer_views[i].buffer&&data->buffer_views[i].buffer->size<rq)return cgltf_result_data_too_short; }
    for(size_t i=0;i<data->meshes_count;++i){
        for(size_t j=0;j<data->meshes[i].primitives_count;++j){
            if(data->meshes[i].primitives[j].type==cgltf_primitive_type_invalid)return cgltf_result_invalid_gltf;
            if(data->meshes[i].primitives[j].attributes_count==0)return cgltf_result_invalid_gltf;
            cgltf_accessor* f=data->meshes[i].primitives[j].attributes[0].data;
            if(f->count==0)return cgltf_result_invalid_gltf;
            for(size_t k=0;k<data->meshes[i].primitives[j].attributes_count;++k)if(data->meshes[i].primitives[j].attributes[k].data->count!=f->count)return cgltf_result_invalid_gltf;
            cgltf_accessor* idx=data->meshes[i].primitives[j].indices;
            if(idx&&(idx->component_type!=cgltf_component_type_r_8u&&idx->component_type!=cgltf_component_type_r_16u&&idx->component_type!=cgltf_component_type_r_32u))return cgltf_result_invalid_gltf;
            if(idx&&idx->type!=cgltf_type_scalar)return cgltf_result_invalid_gltf;
            if(idx&&idx->stride!=cgltf_component_size(idx->component_type))return cgltf_result_invalid_gltf;
            if(idx&&idx->buffer_view&&idx->buffer_view->buffer->data){size_t ib=cgltf_calc_index_bound(idx->buffer_view,idx->offset,idx->component_type,idx->count);if(ib>=f->count)return cgltf_result_data_too_short;}
        }
    }
    for(size_t i=0;i<data->nodes_count;++i){cgltf_node* p1=data->nodes[i].parent,*p2=p1?p1->parent:NULL;while(p1&&p2){if(p1==p2)return cgltf_result_invalid_gltf;p1=p1->parent;p2=p2->parent?p2->parent->parent:NULL;}}
    for(size_t i=0;i<data->animations_count;++i){
        for(size_t j=0;j<data->animations[i].channels_count;++j){ cgltf_animation_channel* c=&data->animations[i].channels[j];if(!c->target_node)continue; size_t comp=1,vals=c->sampler->interpolation==cgltf_interpolation_type_cubic_spline?3:1; if(c->sampler->input->count*comp*vals!=c->sampler->output->count)return cgltf_result_invalid_gltf; }
    }
    return cgltf_result_success;
}
typedef struct {u32 idx,key;} TriSort;
int cmp(const void* a, const void* b) { u32 ka=((const TriSort*)a)->key, kb=((const TriSort*)b)->key; return (ka > kb) - (ka < kb); } // branchless 1 or -1
void OptimizeVertexCache(u16* idx, u32 ic, u32 vc, u8* scratch) {
    if (ic < 3 || !vc) return;
    u32 tc = ic / 3;
    TriSort* t = (TriSort*)scratch;
    TriSort* t_tmp = (TriSort*)(scratch + (tc * sizeof(TriSort)));
    u16* n = (u16*)(scratch + (tc * sizeof(TriSort) * 2));
    for (u32 i = 0; i < tc; ++i) { u16* p = idx + i * 3; u32 m = p[0] < p[1] ? p[0] : p[1]; m = m < p[2] ? m : p[2]; t[i].idx = i; t[i].key = (u16)m; }
    if (tc >= 2) {
        u32 b0[256]={0}, b1[256]={0};
        for (u32 i=0;i<tc;++i) {u16 key = t[i].key; b0[key & 0xFF]++; b1[(key >> 8) & 0xFF]++;}
        u32 sum0=0, sum1=0;
        for (u32 i=0;i<256;++i) { u32 t0 = b0[i]; u32 t1 = b1[i]; b0[i] = sum0; b1[i] = sum1; sum0 += t0; sum1 += t1; }
        for (u32 i=0;i<tc;++i) {u32 radix0 = t[i].key & 0xFF; u32 dest = b0[radix0]++; t_tmp[dest] = t[i];}
        for (u32 i=0;i<tc;++i) { u32 radix1 = (t_tmp[i].key >> 8) & 0xFF; u32 dest = b1[radix1]++; t[dest] = t_tmp[i]; }
    }
    for (u32 i = 0; i < tc; ++i) { u16* s = idx + t[i].idx * 3; u16* d = n + i * 3; d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; }
    mcpy(idx, n, ic * sizeof(u16));
}

u8* OptimizeVertexFetch(u8* v, u32* vc, u16* idx, u32 ic, size_t stride, u32* remap, u8* nv) {
    u32 oc = *vc; if (!oc || !ic) return v; mset(remap,0xFF,oc * sizeof(u32)); u32 nc = 0; for(u32 i=0;i<ic;++i) { u32 id = idx[i]; if (id < oc && remap[id] == 0xFFFFFFFFU) { remap[id]=nc; ++nc; } }
    mset(remap,0xFF,oc * sizeof(u32)); u32 write_ptr=0; for(u32 i=0;i<ic;++i) { u32 id = idx[i]; if (id < oc) { if (remap[id] == 0xFFFFFFFFU) { remap[id]=write_ptr; mcpy(nv + write_ptr * stride, v + id * stride, stride); write_ptr++; } idx[i]=(u16)remap[id]; } }
    *vc = nc; return nv;
}

#define _mm_min_ps(A, B) ((__m128)__builtin_ia32_minps((__v4sf)(A), (__v4sf)(B)))
#define _mm_max_ps(A, B) ((__m128)__builtin_ia32_maxps((__v4sf)(A), (__v4sf)(B)))
__attribute__((hot)) bool FinalizeParsedMesh(u32 mindex, float* __restrict sv, u32 ec, u32* __restrict ht, u32* __restrict ht_used, u32* __restrict remap_scr, u8* __restrict cache_scr, float** __restrict ov_pos, u32* ovc, u16** ot, u16* otc, __m128 mn_v, __m128 mx_v) {
    if (unlikely(!ec)){return false;} u16* final_t = OS_Alloc(ec * sizeof(u16)); /*Allocate final_t early so we can use it instead of ft_scratch*/ u32 used_slots_count = 0; u32* rem = (u32*)remap_scr; /*Reuse remap_scr for the 'rem' array!*/ u32 ucnt = 0;
    for (u32 i=0; i<ec; ++i) {
        const float* v = sv + (i<<3); const u32* uv = (const u32*)v; u32 h0 = uv[0] ^ uv[1] ^ uv[2] ^ uv[3]; u32 h1 = uv[4] ^ uv[5] ^ uv[6] ^ uv[7]; u32 s = (h0 ^ h1) & (WELD_HASH_SIZE-1);
        while (ht[s] != 0xFFFFFFFFU) { if (mcmp(sv+(ht[s]<<3), v, 32) == 0) { rem[i] = ht[s]; goto nxt; } s = (s+1) & (WELD_HASH_SIZE-1); }
        ht[s] = ucnt; rem[i] = ucnt; ht_used[used_slots_count++] = s; mcpy(sv+(ucnt<<3), v, 32); ++ucnt; nxt:;
    }
    for (u32 i=0;i<ec;++i) final_t[i] = (u16)rem[i];
    OptimizeVertexCache(final_t,ec,ucnt,cache_scr); float* final_verts = (float*)OS_Alloc((size_t)ucnt * CPU_VRT_SZ);
    OptimizeVertexFetch((u8*)sv,&ucnt,final_t,ec,CPU_VRT_SZ,remap_scr,(u8*)final_verts); *ov_pos = final_verts; *ovc = ucnt; *ot = final_t; *otc = ec/3;
    float mn_arr[4], mx_arr[4]; _mm_storeu_ps(mn_arr,mn_v); _mm_storeu_ps(mx_arr,mx_v); modelBounds[mindex] = vmax(vabs(mn_arr[0]),vmax(vabs(mn_arr[1]),vmax(vabs(mn_arr[2]),vmax(mx_arr[0],vmax(mx_arr[1],mx_arr[2]))))); for (u32 i = 0; i < used_slots_count; ++i) {ht[ht_used[i]] = 0xFFFFFFFFU;}
    return true;
}

typedef struct { u16 j[4]; float w[4]; } VtxSkin;
typedef struct { float *pos,*nrm,*uv; VtxSkin* skin; u32 vertCount,*indices,triCount; cgltf_node* jointNodes[MAX_GLTF_JOINTS]; float invBind[MAX_GLTF_JOINTS][16]; u32 jointCount; cgltf_animation* anim; cgltf_data* gltf; bool isTransformAnim; cgltf_node** meshNodes; float **subPos,**subNrm,**subUv; u32 *subVertCount,**subIndices,*subTriCount,submeshCount; } GltfMesh;
GltfMesh* gBlockMeshes = NULL; // scratch-allocated in LoadGLTFAnimatedBlocks (init-only, 441 KB)
static u32 gBlockMeshCount = 0;
static void Mat4Identity(float* m) { mset(m, 0, sizeof(float) * 16); m[0] = m[5] = m[10] = m[15] = 1.0f; }
static void Mat4Mul(const float* __restrict a, const float* __restrict b, float* __restrict out) { for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) { float s = 0.0f; for (int k = 0; k < 4; ++k) s += a[k*4+r] * b[c*4+k]; out[c*4+r] = s; } }
static void Mat4TransformPoint(const float* __restrict m, const float* __restrict v, float* __restrict out) { out[0] = m[0]*v[0] + m[4]*v[1] + m[8]*v[2]  + m[12]; out[1] = m[1]*v[0] + m[5]*v[1] + m[9]*v[2]  + m[13]; out[2] = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]; }
static void Mat4TransformDir(const float* __restrict m, const float* __restrict v, float* __restrict out) { float x = m[0]*v[0] + m[4]*v[1] + m[8]*v[2]; float y = m[1]*v[0] + m[5]*v[1] + m[9]*v[2]; float z = m[2]*v[0] + m[6]*v[1] + m[10]*v[2]; float len = vsqrtf(x*x + y*y + z*z), inv = (len > 1e-8f) ? 1.0f/len : 0.0f; out[0] = x*inv; out[1] = y*inv; out[2] = z*inv; }
static bool ParseGLTFStatic(u32 mindex, const u8* bytes, size_t size, float* __restrict sv, u32* __restrict ht, u32* __restrict ht_used, u32* __restrict remap_scr, u8* __restrict cache_scr, float** __restrict ov_pos, u32* ovc, u16** ot, u16* otc) {
    *ov_pos=NULL; *ot=NULL; *ovc=*otc=0; cgltf_data* data = NULL; cgltf_parse(bytes, size, &data); cgltf_load_buffers(data, NULL); cgltf_validate(data);
    if (data->meshes_count == 0 || data->nodes_count == 0) { DualLogError("gltf_static: no mesh/nodes in glb\n"); OS_Exit(1); }
    cgltf_node* meshNode = NULL;
    for (size_t i = 0; i < data->nodes_count; ++i) { if (data->nodes[i].mesh && !data->nodes[i].skin) { meshNode = &data->nodes[i]; break; } }
    if (!meshNode) {  for (size_t i = 0; i < data->nodes_count; ++i) { if(data->nodes[i].mesh){meshNode = &data->nodes[i]; break;} }  }
    if (!meshNode) { DualLogError("gltf_static: no mesh node in glb\n"); OS_Exit(1); }
    cgltf_mesh* mesh = meshNode->mesh;
    if (mesh->primitives_count == 0) { DualLogError("gltf_static: mesh has no primitives\n"); OS_Exit(1); }
    float gm[16]; Mat4Identity(gm); const cgltf_node* parents[32]; int parentCount = 0; const cgltf_node* curr = meshNode;
    while (curr && parentCount < 32) { parents[parentCount++] = curr; curr = curr->parent; }
    for (int i = parentCount - 1; i >= 0; --i) { float local[16]; cgltf_node_transform_local(parents[i], local); float next[16]; Mat4Mul(gm, local, next); mcpy(gm, next, sizeof(float) * 16); } // Multiply in reverse order (root to child)
    __m128 mn_v=_mm_set1_ps(1e9f), mx_v=_mm_set1_ps(-1e9f);
    u32 ec = 0;
    for (size_t p = 0; p < mesh->primitives_count; ++p) {
        cgltf_primitive* prim = &mesh->primitives[p];
        if (prim->type != cgltf_primitive_type_triangles) continue;
        const cgltf_accessor* posAcc = cgltf_find_accessor(prim, cgltf_attribute_type_position, 0);
        const cgltf_accessor* nrmAcc = cgltf_find_accessor(prim, cgltf_attribute_type_normal, 0);
        const cgltf_accessor* uvAcc  = cgltf_find_accessor(prim, cgltf_attribute_type_texcoord, 0);
        if (!posAcc) { DualLogError("gltf_static: primitive missing POSITION\n"); OS_Exit(1); }
        u32 vc = (u32)posAcc->count;
        u32 ic = prim->indices ? (u32)prim->indices->count : vc;
        if (ic == 0 || ec + ic > MAX_OUTPUT_VERTS) { DualLogError("gltf_static: vert count %u out of range or overflow\n", ic); OS_Exit(1); }
        for (u32 k = 0; k < ic; ++k) {
            u32 vi = prim->indices ? (u32)cgltf_accessor_read_index(prim->indices, k) : k;
            if (vi >= vc) continue;
            float pt[3]={0,0,0}, n[3]={0,1,0}, uv[2]={0,0};
            cgltf_accessor_read_float(posAcc, vi, pt, 3);
            Mat4TransformPoint(gm, pt, pt);
            if (nrmAcc) { cgltf_accessor_read_float(nrmAcc, vi, n, 3); Mat4TransformDir(gm, n, n); }
            if (uvAcc) cgltf_accessor_read_float(uvAcc, vi, uv, 2);
            float* dst = sv + (ec<<3);
            dst[0]=pt[0]; dst[1]=pt[2]; dst[2]=pt[1];   dst[3]=n[0]; dst[4]=n[2]; dst[5]=n[1];   dst[6]=uv[0]; dst[7]=1.0f - uv[1];
            __m128 pos_v=_mm_loadu_ps(dst); mn_v=_mm_min_ps(mn_v,pos_v); mx_v=_mm_max_ps(mx_v,pos_v); 
            ++ec;
        }
    }
    return FinalizeParsedMesh(mindex, sv, ec, ht, ht_used, remap_scr, cache_scr, ov_pos, ovc, ot, otc, mn_v, mx_v);
}

static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(u32 mindex, const char* __restrict d, int fs, float* __restrict tp, float* __restrict tn, float* __restrict tu, float* __restrict sv, u32* __restrict ht, u32* __restrict ht_used, u32* __restrict remap_scr, u8* __restrict cache_scr, float** __restrict ov_pos, u32* ovc, u16** ot, u16* otc) {
    *ov_pos=NULL; *ot=NULL; *ovc=*otc=0; u32 pc=0,nc=0,uc=0,ec=0; __m128 mn_v=_mm_set1_ps(1e9f), mx_v=_mm_set1_ps(-1e9f); const char *p=d, *e=d+fs;
    while (likely(p < e)) {
        while (p < e && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
        if (p >= e) break;
        if (*p == '#') { while (p < e && *p != '\n') ++p; continue; }
        if (*p == 'v') {
            ++p;
            if (*p == ' ') { if (unlikely(pc >= MAX_VERT_ELEMENT_SIZE)) {return false;} ++p; tp[pc*3] = fast_atof(&p); tp[pc*3+1] = fast_atof(&p); tp[pc*3+2] = fast_atof(&p); ++pc; }
            else if (*p == 'n' && p[1] == ' ') { p += 2; if (unlikely(nc >= MAX_VERT_ELEMENT_SIZE)) {return false;} tn[nc*3] = fast_atof(&p); tn[nc*3+1] = fast_atof(&p); tn[nc*3+2] = fast_atof(&p); ++nc; }
            else if (*p == 't' && p[1] == ' ') { p += 2; if (unlikely(uc >= MAX_VERT_ELEMENT_SIZE)) {return false;} tu[uc*2] = fast_atof(&p); tu[uc*2+1] = fast_atof(&p); ++uc; }
        } else if (*p == 'f' && p[1] == ' ') {
            p += 2;
            u32 vi[8]={0}, ti[8]={0}, ni[8]={0}; int nv = 0;
            while (nv < 8 && p < e && *p != '\n' && *p != '\r') {
                while (*p == ' ' || *p == '\t') ++p;
                if (*p == '\n' || *p == '\r' || *p == '#') break;
                long r = fast_atoi(&p);
                u32 v = (r>0) ? (u32)r : (r<0) ? (u32)((i32)pc + r) : 0; vi[nv] = v;
                if (*p == '/') {
                    ++p;
                    if (*p != '/') { r = fast_atoi(&p); u32 t = (r>0)?(u32)r:(r<0)?(u32)((i32)uc+r):0; ti[nv]=t; }
                    if (*p == '/') { ++p; r = fast_atoi(&p); u32 n = (r>0)?(u32)r:(r<0)?(u32)((i32)nc+r):0; ni[nv]=n; }
                }
                ++nv;
            }
            if (nv < 3) goto skip;
            for (int k=1; k<nv-1; ++k) {
                if (unlikely(ec + 3 > MAX_OUTPUT_VERTS)) {DualLogError("vert overflow!\n"); return false;}
                u32 tri[3] = {0, (u32)k, (u32)(k+1)};
                for (int t=0; t<3; ++t) {
                    int ix = tri[t]; u32 v = (vi[ix] && vi[ix] <= pc) ? vi[ix]-1 : 0; u32 tex = (ti[ix] && ti[ix] <= uc) ? ti[ix]-1 : 0; u32 nrm = (ni[ix] && ni[ix] <= nc) ? ni[ix]-1 : 0; float* dst = sv + (ec<<3);
                    dst[0]=-tp[v*3]; dst[1]=tp[v*3+1]; dst[2]=tp[v*3+2]; dst[3]=(nrm < nc) ? -tn[nrm*3] : 0; dst[4]=(nrm < nc) ? tn[nrm*3+1] : 0; dst[5]=(nrm < nc) ? tn[nrm*3+2] : 0; dst[6]=(tex < uc) ? tu[tex*2] : 0; dst[7]=(tex < uc) ? tu[tex*2+1] : 0;
                    __m128 pos_v=_mm_loadu_ps(dst); mn_v=_mm_min_ps(mn_v,pos_v); mx_v=_mm_max_ps(mx_v,pos_v); ++ec;
                }
            }
        skip:;
        } else while (p < e && *p != '\n') ++p;
    }
    return FinalizeParsedMesh(mindex, sv, ec, ht, ht_used, remap_scr, cache_scr, ov_pos, ovc, ot, otc, mn_v, mx_v);
}

static void FindBracket(const cgltf_animation_sampler* samp, float t, u32* i0, float* frac) {
	u32 n = (u32)samp->input->count;
	float t0, tn; cgltf_accessor_read_float(samp->input, 0, &t0, 1); cgltf_accessor_read_float(samp->input, n-1, &tn, 1);
	if (n <= 1 || t <= t0) { *i0 = 0; *frac = 0.0f; return; }
	if (t >= tn) { *i0 = n-2; *frac = 1.0f; return; }
	for (u32 i = 0; i < n-1; ++i) {
		float ta, tb; cgltf_accessor_read_float(samp->input, i, &ta, 1); cgltf_accessor_read_float(samp->input, i+1, &tb, 1);
		if (t >= ta && t <= tb) { *i0 = i; *frac = (tb > ta) ? (t-ta)/(tb-ta) : 0.0f; return; }
	}
	*i0 = n-2; *frac = 1.0f;
}
 
static void ReadSamplerValue(const cgltf_animation_sampler* samp, u32 keyIdx, u32 numComp, float* out) { u32 elemIdx = (samp->interpolation == cgltf_interpolation_type_cubic_spline) ? keyIdx*3 + 1 : keyIdx; cgltf_accessor_read_float(samp->output, elemIdx, out, numComp); }
static void SampleVec3(const cgltf_animation_sampler* samp, float t, float* out3) {
	u32 i0; float frac; FindBracket(samp, t, &i0, &frac);
	float v0[3]; ReadSamplerValue(samp, i0, 3, v0);
	if (frac <= 0.0f || samp->interpolation == cgltf_interpolation_type_step) { out3[0]=v0[0]; out3[1]=v0[1]; out3[2]=v0[2]; return; }
	float v1[3]; ReadSamplerValue(samp, i0+1, 3, v1);
	out3[0] = v0[0] + (v1[0]-v0[0])*frac; out3[1] = v0[1] + (v1[1]-v0[1])*frac; out3[2] = v0[2] + (v1[2]-v0[2])*frac;
}
 
static void SampleQuat(const cgltf_animation_sampler* samp, float t, float* outq /* xyzw */) {
	u32 i0; float frac; FindBracket(samp, t, &i0, &frac);
	float q0[4]; ReadSamplerValue(samp, i0, 4, q0);
	if (frac <= 0.0f || samp->interpolation == cgltf_interpolation_type_step) { mcpy(outq, q0, sizeof(float)*4); return; }
	float q1[4]; ReadSamplerValue(samp, i0+1, 4, q1);
	float d = q0[0]*q1[0] + q0[1]*q1[1] + q0[2]*q1[2] + q0[3]*q1[3];
	float qb[4];
	if (d < 0.0f) { qb[0]=-q1[0]; qb[1]=-q1[1]; qb[2]=-q1[2]; qb[3]=-q1[3]; d = -d; } else mcpy(qb, q1, sizeof(float)*4);
	if (d > 0.9995f) { for (int c = 0; c < 4; ++c) outq[c] = q0[c] + (qb[c]-q0[c])*frac; } // nearly parallel: nlerp
	else { float theta0 = vacosf(d), theta = theta0*frac; float s1 = sinf(theta) / sinf(theta0), s0 = cosf(theta) - d*s1; for(int c = 0; c < 4; ++c){outq[c]=q0[c]*s0 + qb[c]*s1;} }
	float len = vsqrtf(outq[0]*outq[0]+outq[1]*outq[1]+outq[2]*outq[2]+outq[3]*outq[3]);
	if (len > 1e-8f) { float inv = 1.0f/len; for (int c = 0; c < 4; ++c) outq[c] *= inv; }
}

static void NodeLocalMatrixAtTime(const GltfMesh* gm, cgltf_node* node, float t, float* outM) {
    float T[3] = {node->translation[0], node->translation[1], node->translation[2]};
    float R[4] = {node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]};
    float S[3] = {node->scale[0], node->scale[1], node->scale[2]};
    bool animated = false;
    if (gm->isTransformAnim) { // Transform based animations (e.g. doors)
        for (size_t a = 0; a < gm->gltf->animations_count; ++a) {
            cgltf_animation* anim = &gm->gltf->animations[a];
            for (size_t c = 0; c < anim->channels_count; ++c) {
                const cgltf_animation_channel* ch = &anim->channels[c];
                if (ch->target_node != node) continue;
                animated = true;
                if (ch->target_path == cgltf_animation_path_type_translation) SampleVec3(ch->sampler, t, T);
                else if (ch->target_path == cgltf_animation_path_type_rotation) SampleQuat(ch->sampler, t, R); 
                else if (ch->target_path == cgltf_animation_path_type_scale) SampleVec3(ch->sampler, t, S);
            }
        }
    } else { // For skinned meshes, stick to the single selected animation clip
        for (size_t c = 0; c < gm->anim->channels_count; ++c) {
            const cgltf_animation_channel* ch = &gm->anim->channels[c];
            if (ch->target_node != node) continue;
            animated = true;
            if (ch->target_path == cgltf_animation_path_type_translation) SampleVec3(ch->sampler, t, T);
            else if (ch->target_path == cgltf_animation_path_type_rotation) SampleQuat(ch->sampler, t, R);
            else if (ch->target_path == cgltf_animation_path_type_scale) SampleVec3(ch->sampler, t, S);
        }
    }
    if (!animated) { cgltf_node_transform_local(node, outM); return; }
    Mat4FromTRS(T, R, S, outM);
}
 
static void NodeGlobalMatrixAtTime(const GltfMesh* gm, cgltf_node* node, float t, float* outM) { float l[16]; NodeLocalMatrixAtTime(gm,node,t,l); if(!node->parent){mcpy(outM,l,sizeof(l)); return;} float p[16]; NodeGlobalMatrixAtTime(gm,node->parent,t,p); Mat4Mul(p,l,outM); }
bool IsGLTFSourcePath(const char* path) { if (!path){return false;} const char* dot=StringFindLastChar(path, '.'); return dot && sEqual(dot,".glb"); }
static void GltfMeshFreePartial(GltfMesh* gm) {
    if(gm->pos){OS_Free(gm->pos,(size_t)gm->vertCount * 3 * sizeof(float));}
    if(gm->nrm){OS_Free(gm->nrm,(size_t)gm->vertCount * 3 * sizeof(float));}
    if(gm->uv) {OS_Free(gm->uv, (size_t)gm->vertCount * 2 * sizeof(float));}
    if(gm->skin){OS_Free(gm->skin,(size_t)gm->vertCount * sizeof(VtxSkin));}
    if(gm->indices){OS_Free(gm->indices,(size_t)gm->triCount * 3 * sizeof(u32));}
    if(gm->isTransformAnim) {
        for(u32 s = 0; s < gm->submeshCount; ++s) {
            if(gm->subPos[s])     OS_Free(gm->subPos[s],     (size_t)gm->subVertCount[s] * 3 * sizeof(float));
            if(gm->subNrm[s])     OS_Free(gm->subNrm[s],     (size_t)gm->subVertCount[s] * 3 * sizeof(float));
            if(gm->subUv[s])      OS_Free(gm->subUv[s],       (size_t)gm->subVertCount[s] * 2 * sizeof(float));
            if(gm->subIndices[s]) OS_Free(gm->subIndices[s],  (size_t)gm->subTriCount[s]  * 3 * sizeof(u32));
        }
        if(gm->meshNodes)    OS_Free(gm->meshNodes,    gm->submeshCount * sizeof(cgltf_node*));
        if(gm->subPos)       OS_Free(gm->subPos,       gm->submeshCount * sizeof(float*));
        if(gm->subNrm)       OS_Free(gm->subNrm,       gm->submeshCount * sizeof(float*));
        if(gm->subUv)        OS_Free(gm->subUv,        gm->submeshCount * sizeof(float*));
        if(gm->subVertCount) OS_Free(gm->subVertCount, gm->submeshCount * sizeof(u32));
        if(gm->subIndices)   OS_Free(gm->subIndices,   gm->submeshCount * sizeof(u32*));
        if(gm->subTriCount)  OS_Free(gm->subTriCount,  gm->submeshCount * sizeof(u32));
    }
}
 
static bool GltfMeshLoad(const u8* bytes, size_t size, GltfMesh* out) {
    mset(out,0,sizeof(*out)); cgltf_data* data = NULL;
    cgltf_parse(bytes,size,&data);
    cgltf_load_buffers(data,NULL);
    cgltf_validate(data);
    if (data->animations_count == 0) { DualLogError("gltf_anim: glb has no animation\n"); OS_Exit(1); }
    cgltf_animation* bestAnim = &data->animations[0];
    size_t maxChannels = 0;
    for (size_t a = 0; a < data->animations_count; ++a) { if (data->animations[a].channels_count > maxChannels) { maxChannels = data->animations[a].channels_count; bestAnim = &data->animations[a]; } }
    out->anim = bestAnim;
    out->gltf = data;
    if (out->anim->channels_count) { for (size_t c = 0; c < out->anim->channels_count; ++c) { if (out->anim->channels[c].sampler->interpolation == cgltf_interpolation_type_cubic_spline) { DualLogWarn("gltf_anim: CUBICSPLINE channel present -- tangents ignored, degrading to linear-between-keys\n"); break; } } }
    cgltf_node* skinNode = NULL;
    for (size_t i = 0; i < data->nodes_count; ++i) if (data->nodes[i].skin && data->nodes[i].mesh) { skinNode = &data->nodes[i]; break; }
    if (skinNode) { // Skeletal mesh animation (skinned)
        out->isTransformAnim = false;
        cgltf_mesh* mesh = skinNode->mesh;
        if (mesh->primitives_count == 0) { DualLogError("gltf_anim: skinned mesh has no primitives\n"); OS_Exit(1); }
        cgltf_primitive* prim = &mesh->primitives[0];
        if (prim->type != cgltf_primitive_type_triangles) { DualLogError("gltf_anim: primitive is not a triangle list\n"); OS_Exit(1); }
        const cgltf_accessor* posAcc = cgltf_find_accessor(prim, cgltf_attribute_type_position, 0);
        const cgltf_accessor* nrmAcc = cgltf_find_accessor(prim, cgltf_attribute_type_normal, 0);
        const cgltf_accessor* uvAcc  = cgltf_find_accessor(prim, cgltf_attribute_type_texcoord, 0);
        const cgltf_accessor* jntAcc = cgltf_find_accessor(prim, cgltf_attribute_type_joints, 0);
        const cgltf_accessor* wgtAcc = cgltf_find_accessor(prim, cgltf_attribute_type_weights, 0);
        if (!posAcc || !jntAcc || !wgtAcc) { DualLogError("gltf_anim: primitive missing POSITION/JOINTS_0/WEIGHTS_0\n"); OS_Exit(1); }
        u32 vc = (u32)posAcc->count;
        if (vc == 0 || vc > MAX_GLTF_VERTS) { DualLogError("gltf_anim: vertex count %u out of range (max %u)\n", vc, (u32)MAX_GLTF_VERTS); OS_Exit(1); }
        out->vertCount = vc;
        out->pos=(float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); out->nrm=(float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); out->uv=(float*)OS_Alloc((size_t)vc * 2 * sizeof(float)); out->skin=(VtxSkin*)OS_Alloc((size_t)vc * sizeof(VtxSkin));
        for (u32 i = 0; i < vc; ++i) {
            cgltf_accessor_read_float(posAcc, i, &out->pos[i*3], 3);
            if (nrmAcc) cgltf_accessor_read_float(nrmAcc, i, &out->nrm[i*3], 3);
            else { out->nrm[i*3]=0.0f; out->nrm[i*3+1]=1.0f; out->nrm[i*3+2]=0.0f; }
            if (uvAcc) { cgltf_accessor_read_float(uvAcc, i, &out->uv[i*2], 2); out->uv[i*2+1] = 1.0f - out->uv[i*2+1]; /*Flip V: glTF bottom-left, engine top-left*/ }
            else { out->uv[i*2]=0.0f; out->uv[i*2+1]=0.0f; }
            float jf[4]={0,0,0,0}, wf[4]={0,0,0,0};
            cgltf_accessor_read_float(jntAcc,i,jf,4); cgltf_accessor_read_float(wgtAcc,i,wf,4); float wsum = wf[0]+wf[1]+wf[2]+wf[3], winv = (wsum > 1e-6f) ? 1.0f/wsum : 0.0f;
            for (int k=0;k<4;++k){i32 jj=(i32)jf[k]; out->skin[i].j[k]=(jj >= 0 && jj < MAX_GLTF_JOINTS) ? (u16)jj : 0; out->skin[i].w[k]=wf[k]*winv;}
        }
        u32 tc;
        if (prim->indices) {
            tc = (u32)(prim->indices->count / 3);
            if (tc == 0 || tc > MAX_GLTF_TRIS) { DualLogError("gltf_anim: triangle count %u out of range (max %u)\n", tc, (u32)MAX_GLTF_TRIS); GltfMeshFreePartial(out); OS_Exit(1); }
            out->indices = (u32*)OS_Alloc((size_t)tc * 3 * sizeof(u32));
            for (u32 k = 0; k < tc*3; ++k) out->indices[k] = (u32)cgltf_accessor_read_index(prim->indices, k);
        } else {
            tc = vc / 3;
            if (tc == 0 || tc > MAX_GLTF_TRIS) { DualLogError("gltf_anim: (non-indexed) triangle count %u out of range\n", tc); GltfMeshFreePartial(out); OS_Exit(1); }
            out->indices = (u32*)OS_Alloc((size_t)tc * 3 * sizeof(u32));
            for (u32 k = 0; k < tc*3; ++k) out->indices[k] = k;
        }
        out->triCount = tc;
        cgltf_skin* skin = skinNode->skin;
        if (skin->joints_count == 0 || skin->joints_count > MAX_GLTF_JOINTS) { DualLogError("gltf_anim: joint count %u out of range (max %u)\n", (u32)skin->joints_count, (u32)MAX_GLTF_JOINTS); GltfMeshFreePartial(out); OS_Exit(1); }
        out->jointCount = (u32)skin->joints_count;
        for (u32 j = 0; j < out->jointCount; ++j) {
            out->jointNodes[j] = skin->joints[j];
            if (skin->inverse_bind_matrices) cgltf_accessor_read_float(skin->inverse_bind_matrices, j, out->invBind[j], 16);
            else Mat4Identity(out->invBind[j]);
        }
        return true;
    }
    u32 submeshCount=0; out->isTransformAnim=true; // Transform-based Animation (node TRS)
    for (size_t i=0;i<data->nodes_count;++i) { if(data->nodes[i].mesh){for(size_t p = 0; p < data->nodes[i].mesh->primitives_count; ++p){ if(data->nodes[i].mesh->primitives[p].type == cgltf_primitive_type_triangles){++submeshCount;} }} }
    if (submeshCount == 0) { DualLogError("gltf_anim: no mesh primitives in glb\n"); OS_Exit(1); }
    out->submeshCount = submeshCount;
    out->meshNodes    = (cgltf_node**)OS_Alloc(submeshCount * sizeof(cgltf_node*));
    out->subPos       = (float**)    OS_Alloc(submeshCount * sizeof(float*));
    out->subNrm       = (float**)    OS_Alloc(submeshCount * sizeof(float*));
    out->subUv        = (float**)    OS_Alloc(submeshCount * sizeof(float*));
    out->subVertCount = (u32*)       OS_Alloc(submeshCount * sizeof(u32));
    out->subIndices   = (u32**)      OS_Alloc(submeshCount * sizeof(u32*));
    out->subTriCount  = (u32*)       OS_Alloc(submeshCount * sizeof(u32));
    for (u32 s = 0; s < submeshCount; ++s) { out->subPos[s]=NULL; out->subNrm[s]=NULL; out->subUv[s]=NULL; out->subIndices[s]=NULL; }
    u32 si = 0;
    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* node = &data->nodes[i];
        if (!node->mesh) continue;
        cgltf_mesh* mesh = node->mesh;
        for (size_t p = 0; p < mesh->primitives_count; ++p) {
            cgltf_primitive* prim = &mesh->primitives[p];
            if (prim->type != cgltf_primitive_type_triangles) continue;
            const cgltf_accessor* posAcc = cgltf_find_accessor(prim, cgltf_attribute_type_position, 0);
            const cgltf_accessor* nrmAcc = cgltf_find_accessor(prim, cgltf_attribute_type_normal, 0);
            const cgltf_accessor* uvAcc  = cgltf_find_accessor(prim, cgltf_attribute_type_texcoord, 0);
            if (!posAcc) { DualLogError("gltf_anim: primitive missing POSITION\n"); GltfMeshFreePartial(out); OS_Exit(1); }
            u32 vc = (u32)posAcc->count;
            if (vc == 0 || vc > MAX_GLTF_VERTS) { DualLogError("gltf_anim: vertex count %u out of range (max %u)\n", vc, (u32)MAX_GLTF_VERTS); GltfMeshFreePartial(out); OS_Exit(1); }
            out->meshNodes[si] = node; out->subVertCount[si] = vc;
            out->subPos[si] = (float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); out->subNrm[si] = (float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); out->subUv[si]  = (float*)OS_Alloc((size_t)vc * 2 * sizeof(float));
            for (u32 v = 0; v < vc; ++v) {
                cgltf_accessor_read_float(posAcc, v, &out->subPos[si][v*3], 3);
                if (nrmAcc) cgltf_accessor_read_float(nrmAcc, v, &out->subNrm[si][v*3], 3);
                else { out->subNrm[si][v*3]=0.0f; out->subNrm[si][v*3+1]=1.0f; out->subNrm[si][v*3+2]=0.0f; }
                if (uvAcc) {
                    cgltf_accessor_read_float(uvAcc, v, &out->subUv[si][v*2], 2);
                    out->subUv[si][v*2+1] = 1.0f - out->subUv[si][v*2+1]; // Flip V
                } else { out->subUv[si][v*2]=0.0f; out->subUv[si][v*2+1]=0.0f; }
            }
            u32 tc;
            if (prim->indices) {
                tc = (u32)(prim->indices->count / 3);
                if (tc > MAX_GLTF_TRIS) { DualLogError("gltf_anim: triangle count %u out of range (max %u)\n", tc, (u32)MAX_GLTF_TRIS); GltfMeshFreePartial(out); OS_Exit(1); }
                out->subIndices[si] = (u32*)OS_Alloc((size_t)tc * 3 * sizeof(u32));
                for (u32 k = 0; k < tc*3; ++k) out->subIndices[si][k] = (u32)cgltf_accessor_read_index(prim->indices, k);
            } else {
                tc = vc / 3;
                out->subIndices[si] = (u32*)OS_Alloc((size_t)tc * 3 * sizeof(u32));
                for (u32 k = 0; k < tc*3; ++k) out->subIndices[si][k] = k;
            }
            out->subTriCount[si] = tc;
            ++si;
        }
    }
    return true;
}
 
static void SkinFrameToScratch(GltfMesh* __restrict gm, float t, float* __restrict posedPos, float* __restrict posedNrm, float* __restrict sv, u32* outEc, __m128* outMn, __m128* outMx) {
	float skinMat[MAX_GLTF_JOINTS][16];
	for (u32 j = 0; j < gm->jointCount; ++j) { float g[16]; NodeGlobalMatrixAtTime(gm, gm->jointNodes[j], t, g); Mat4Mul(g, gm->invBind[j], skinMat[j]); }
	for (u32 v = 0; v < gm->vertCount; ++v) {
		const VtxSkin* sk = &gm->skin[v];
		float blended[16] = {0};
		for (int k = 0; k < 4; ++k) { float w = sk->w[k]; if (w <= 0.0f){continue;} const float* m = skinMat[sk->j[k]]; for (int e = 0; e < 16; ++e) blended[e] += m[e] * w; }
		Mat4TransformPoint(blended, &gm->pos[v*3], &posedPos[v*3]); Mat4TransformDir(blended, &gm->nrm[v*3], &posedNrm[v*3]);
	}
	__m128 mn_v = _mm_set1_ps(1e9f), mx_v = _mm_set1_ps(-1e9f);
	u32 ec = 0, cornerCount = gm->triCount * 3;
	for (u32 k = 0; k < cornerCount; ++k) {
		if (unlikely(ec + 1 > MAX_OUTPUT_VERTS)) { DualLogError("gltf_anim: frame vertex overflow, truncating\n"); break; }
		u32 vi = gm->indices[k];
		float* dst = sv + (ec << 3); dst[0] = -posedPos[vi*3+0]; dst[1] = posedPos[vi*3+1]; dst[2] = posedPos[vi*3+2]; dst[3] = -posedNrm[vi*3+0]; dst[4] = posedNrm[vi*3+1]; dst[5] = posedNrm[vi*3+2]; dst[6] = gm->uv[vi*2+0];    dst[7] = gm->uv[vi*2+1];
		__m128 pos_v = _mm_loadu_ps(dst); mn_v = _mm_min_ps(mn_v, pos_v); mx_v = _mm_max_ps(mx_v, pos_v); ++ec;
	}
	*outEc = ec; *outMn = mn_v; *outMx = mx_v;
}

static void TransformFrameToScratch(GltfMesh* __restrict gm, float t, float* __restrict sv, u32* outEc, __m128* outMn, __m128* outMx) {
    __m128 mn_v = _mm_set1_ps(1e9f), mx_v = _mm_set1_ps(-1e9f);
    u32 ec = 0;
    for (u32 s = 0; s < gm->submeshCount; ++s) {
        float gm_mat[16]; NodeGlobalMatrixAtTime(gm, gm->meshNodes[s], t, gm_mat);
        const float* __restrict spos = gm->subPos[s]; const float* __restrict snrm = gm->subNrm[s]; const float* __restrict suv  = gm->subUv[s]; const u32*   __restrict sidx = gm->subIndices[s]; u32 cornerCount = gm->subTriCount[s] * 3;
        for (u32 k = 0; k < cornerCount; ++k) {
            if (unlikely(ec + 1 > MAX_OUTPUT_VERTS)) { DualLogError("gltf_anim: transform frame vertex overflow, truncating\n"); goto done; }
            u32 vi = sidx[k]; float pt[3], n[3]; Mat4TransformPoint(gm_mat, &spos[vi*3], pt); Mat4TransformDir  (gm_mat, &snrm[vi*3], n);
            float* dst = sv + (ec << 3); dst[0] = -pt[0]; dst[1] = pt[1]; dst[2] = pt[2]; dst[3] = -n[0]; dst[4] = n[1]; dst[5] = n[2]; dst[6] = suv[vi*2+0]; dst[7] = suv[vi*2+1];
            __m128 pos_v = _mm_loadu_ps(dst); mn_v = _mm_min_ps(mn_v, pos_v); mx_v = _mm_max_ps(mx_v, pos_v); ++ec;
        }
    }
    done: *outEc = ec; *outMn = mn_v; *outMx = mx_v;
}
 
typedef struct { GltfMesh* mesh; u32 modelIndex; float timelineFrame; } GltfFrameTask;
typedef struct { GltfFrameTask* tasks; u32 start, end; int tid; } GltfBakeTask;
static void* GltfBakeWorker(void* arg) {
    GltfBakeTask* bt = (GltfBakeTask*)arg; float* posedPos = thrd_pos[bt->tid]; float* posedNrm = thread_temp_nrm[bt->tid]; float* sv = thrd_verts[bt->tid];
    for (u32 i = bt->start; i < bt->end; ++i) {
        GltfFrameTask* t = &bt->tasks[i]; u32 ec; __m128 mn_v, mx_v; if (t->mesh->isTransformAnim) TransformFrameToScratch(t->mesh, t->timelineFrame, sv, &ec, &mn_v, &mx_v); else SkinFrameToScratch(t->mesh, t->timelineFrame, posedPos, posedNrm, sv, &ec, &mn_v, &mx_v);
        FinalizeParsedMesh(t->modelIndex, sv, ec, thrd_ht[bt->tid], thrd_ht_used[bt->tid], thrd_remap_scratch[bt->tid], thrd_cache_scratch[bt->tid],&vPos[t->modelIndex], &modelVertexCounts[t->modelIndex], &modelTriangles[t->modelIndex], &modelTriangleCounts[t->modelIndex], mn_v, mx_v);
    } return NULL;
}

void LoadGLTFAnimatedBlocks(ModelData* entries, u32 entryCount, RawOBJ* raw) {
    gBlockMeshCount = 0; u32 maxTasks = 0; for (u32 i = 0; i < entryCount; ++i) { if(!entries[i].animated || !IsGLTFSourcePath(entries[i].path)){continue;} maxTasks += entries[i].frameCount; }   if(!maxTasks){return;}
    gBlockMeshes = (GltfMesh*)OS_AllocScratch((size_t)MAX_GLTF_BLOCKS * sizeof(GltfMesh));
    GltfFrameTask* tasks = (GltfFrameTask*)OS_Alloc((size_t)maxTasks * sizeof(GltfFrameTask)); u32 taskCount = 0;
    for (u32 i = 0; i < entryCount; ++i) {
        if (!entries[i].animated || !IsGLTFSourcePath(entries[i].path)) {continue;}
        u32 baseIdx = entries[i].index;
        if (baseIdx >= MAX_MDLS || !raw[baseIdx].data || raw[baseIdx].size <= 0) { DualLogError("gltf_anim: '%s' (index %u) has no loaded data\n", entries[i].path, baseIdx); continue; }
        if (gBlockMeshCount >= MAX_GLTF_BLOCKS) { DualLogError("gltf_anim: exceeded MAX_GLTF_BLOCKS (%u), skipping '%s'\n", (u32)MAX_GLTF_BLOCKS, entries[i].path); continue; }
        GltfMesh* gm = &gBlockMeshes[gBlockMeshCount];
        if (!GltfMeshLoad((const u8*)raw[baseIdx].data, (size_t)raw[baseIdx].size, gm)) { DualLogError("gltf_anim: failed to load '%s'\n", entries[i].path); continue; }
        ++gBlockMeshCount; u16 a = entries[i].animationNum; float framerate = 0.0f;
        if (a < MAX_ANIMS) {  for(u32 c = 0; c < MAX_ANIMCLIPS; ++c){ if(modelAnimationClips[a][c].framerate > 0.0f){framerate = modelAnimationClips[a][c].framerate; break;} }  }
        if (framerate <= 0.0f) framerate = 30.0f; // Fallback if undefined
        for (u32 fi = 0; fi < entries[i].frameCount; ++fi) {
            u32 frameNum = entries[i].frames[fi];
            u32 modelIdx = baseIdx + fi; // Contiguous index, sparse frame
            if (modelIdx >= mdlsCnt) { DualLogWarn("gltf_anim: frame %u -> model index %u exceeds mdlsCnt\n", frameNum, modelIdx); continue; }
            bool dup = false; for (u32 k = 0; k < taskCount; ++k) { if (tasks[k].modelIndex == modelIdx) { dup = true; break; } }   if(dup){continue;}
            tasks[taskCount].mesh = gm; tasks[taskCount].modelIndex = modelIdx; tasks[taskCount].timelineFrame = (float)frameNum / framerate; ++taskCount;
        }
    }
    if (!taskCount) { OS_Free(tasks, (size_t)maxTasks * sizeof(GltfFrameTask)); OS_FreeInitPhaseInner((size_t)MAX_GLTF_BLOCKS * sizeof(GltfMesh)); gBlockMeshes = NULL; return; }
    GltfBakeTask btasks[32]; OS_Thread bth[32];
    u32 chunk = (taskCount + threadCnt - 1) / threadCnt;
    for (int t = 0; t < threadCnt; ++t) { u32 s = (u32)t * chunk, e = ((u32)t+1) * chunk > taskCount ? taskCount : ((u32)t+1) * chunk; btasks[t] = (GltfBakeTask){ tasks, s, e, t }; }
    if (threadCnt > 1) { for(int t=0;t<threadCnt;++t){OS_ThreadCreate(&bth[t],GltfBakeWorker,&btasks[t]);}  for(int t=0;t<threadCnt;++t){OS_ThreadJoin(&bth[t]);} } else {  for (int t = 0; t < threadCnt; ++t) GltfBakeWorker(&btasks[t]);  }
    OS_Free(tasks, (size_t)maxTasks * sizeof(GltfFrameTask));
    OS_FreeInitPhaseInner((size_t)MAX_GLTF_BLOCKS * sizeof(GltfMesh));
    gBlockMeshes = NULL;
}

// Recursive(ew) centroid-based, each tri goes into exactly one octant containing its centroid, no tri dupes. The node AABB is the union of its tri AABBs (NOT the octant AABB) — guarantees any query that overlaps a tri also overlaps its ancestor nodes, so traversal never misses a tri. triIdxArray is modified in-place: on return it is partitioned by octant so that each child's triangles are contiguous (matches the leaf ranges written to ctx->triOrder).
static i32 BvhBuildOctree(BvhBuildCtx* __restrict ctx, u16 m, const float* __restrict pos, const u16* __restrict tris, u16* triIdxArray, u32 triCount, u32 depth) {
    if (triCount == 0){return -1;}
    if (ctx->nodeCount >= BVH_MAX_NODES_PER_MDL){depth = BVH_MAX_DEPTH;}
    i32 nodeIdx = ctx->nodeCount++; BvhNode* node = &ctx->nodes[nodeIdx]; node->triStart = 0; node->triCount = 0;
    for(int i = 0; i < 8; i++){node->children[i]=-1;}
    __m128 mn_v=_mm_set1_ps(1e9f); __m128 mx_v=_mm_set1_ps(-1e9f);
    for (u32 i = 0; i < triCount; i++){u32 triIdx=triIdxArray[i]; u32 i0=tris[triIdx*3+0],i1=tris[triIdx*3+1],i2=tris[triIdx*3+2]; __m128 v0=_mm_loadu_ps(pos+(size_t)i0*3); __m128 v1=_mm_loadu_ps(pos+(size_t)i1*3); __m128 v2=_mm_loadu_ps(pos+(size_t)i2*3); mn_v=_mm_min_ps(mn_v,_mm_min_ps(_mm_min_ps(v0,v1),v2)); mx_v=_mm_max_ps(mx_v,_mm_max_ps(_mm_max_ps(v0,v1),v2));}
    float mn_arr[4], mx_arr[4]; _mm_storeu_ps(mn_arr, mn_v); _mm_storeu_ps(mx_arr, mx_v); node->mn = (V3){mn_arr[0], mn_arr[1], mn_arr[2]}; node->mx = (V3){mx_arr[0], mx_arr[1], mx_arr[2]};
    if (depth >= 3 || triCount <= BVH_LEAF_MAX_TRIS || ctx->nodeCount + 8 > BVH_MAX_NODES_PER_MDL) { u32 startIdx = ctx->triCount; for (u32 i = 0; i < triCount && ctx->triCount < BVH_MAX_TRIS_PER_MDL; i++) { ctx->triOrder[ctx->triCount++] = triIdxArray[i]; } node->triStart = startIdx; node->triCount = (u16)triCount; return nodeIdx; }
    V3 center = V3_ScaleByF(V3_AplusB(node->mn, node->mx), 0.5f); __m128 center_v = _mm_setr_ps(center.x, center.y, center.z, 0.0f); __m128 third = _mm_set1_ps(1.0f/3.0f); u32 octantCounts[8] = {0};
    for (u32 i=0;i<triCount;++i) {
        u32 triIdx = triIdxArray[i];
        u32 i0 = tris[triIdx*3+0], i1 = tris[triIdx*3+1], i2 = tris[triIdx*3+2];
        __m128 v0 = _mm_loadu_ps(pos + (size_t)i0*3); __m128 v1 = _mm_loadu_ps(pos + (size_t)i1*3); __m128 v2 = _mm_loadu_ps(pos + (size_t)i2*3);
        __m128 centroid = _mm_mul_ps(_mm_add_ps(_mm_add_ps(v0, v1), v2), third);
        __v4si ge = (__v4si)(centroid >= center_v); // Compare centroid >= center. Returns a vector mask (all 1s if true, all 0s if false)
        u8 oct = (u8)((ge[0] & 1) | ((ge[1] & 1) << 1) | ((ge[2] & 1) << 2)); // Extract bits 0, 1, 2 from the mask
        ctx->triOctants[i] = oct; octantCounts[oct]++;
    }
    u32 octantStarts[8], total=0; for (int o = 0; o < 8; o++) { octantStarts[o] = total; total += octantCounts[o]; } u32 octantFill[8] = {0};
    for (u32 i = 0; i < triCount; i++) { u8 o = ctx->triOctants[i]; ctx->triScratch[octantStarts[o] + octantFill[o]++] = triIdxArray[i]; }
    for (u32 i = 0; i < triCount; i++) { triIdxArray[i] = ctx->triScratch[i]; }
    for (int o = 0; o < 8; o++) { if (octantCounts[o] == 0) {continue;} i32 childIdx = BvhBuildOctree(ctx,m,pos,tris,triIdxArray + octantStarts[o],octantCounts[o],depth + 1);/*We re curse a little*/ if (childIdx >= 0) {node->children[o] = (i16)childIdx;} }
    return nodeIdx;
}

static void BuildModelBVH(BvhBuildCtx* ctx, u16 m) {
    if (m >= mdlsCnt || m >= MAX_MDLS) return;
    modelBVHNodes[m] = NULL; modelBVHTriOrder[m] = NULL; modelBVHNodeCounts[m] = modelBVHTriOrderCounts[m] = 0; u32 triCount = modelTriangleCounts[m];
    if (triCount == 0 || triCount > BVH_MAX_TRIS_PER_MDL) { DualLogError("Too many verts on model %u!  Could not build a BVH!\n",m); return;}
    if (!physPos[m] || !physTris[m]) return;
    ctx->nodeCount = ctx->triCount = 0; u16* initialTris = ctx->initialTris;
    for (u32 i = 0; i < triCount; i++) initialTris[i] = (u16)i;
    i32 rootIdx = BvhBuildOctree(ctx,m,physPos[m],physTris[m],initialTris,triCount,0); if (rootIdx < 0 || ctx->nodeCount == 0) return;
    modelBVHNodes[m] = (BvhNode*)OS_Alloc(ctx->nodeCount * sizeof(BvhNode)); mcpy(modelBVHNodes[m], ctx->nodes, ctx->nodeCount * sizeof(BvhNode)); modelBVHNodeCounts[m] = ctx->nodeCount;
    if (ctx->triCount > 0) { modelBVHTriOrder[m] = (u16*)OS_Alloc(ctx->triCount * sizeof(u16)); if (modelBVHTriOrder[m]) { mcpy(modelBVHTriOrder[m],ctx->triOrder,ctx->triCount * sizeof(u16)); modelBVHTriOrderCounts[m] = ctx->triCount; } }
}

typedef struct { u32 start, end; RawOBJ* raw; const bool* isGLTFAnimSrc; const bool* isGLTFStaticSrc; int tid; } ModelParseTask;
static void* ModelParsingWorker(void* arg) {
    ModelParseTask* t = arg;
    for (u32 i = t->start; i < t->end; ++i) {
        if (t->isGLTFAnimSrc[i]) continue; // filled by LoadGLTFAnimatedBlocks instead
        RawOBJ obj = t->raw[i]; if (unlikely(!obj.data || obj.size <= 0)) continue;
        if (t->isGLTFStaticSrc[i]) { ParseGLTFStatic(i,(const u8*)obj.data,(size_t)obj.size,thrd_verts[t->tid],thrd_ht[t->tid],thrd_ht_used[t->tid],thrd_remap_scratch[t->tid],thrd_cache_scratch[t->tid],&vPos[i],&modelVertexCounts[i],&modelTriangles[i],&modelTriangleCounts[i]); continue; }
        if (!ParseOBJ(i,obj.data,obj.size,thrd_pos[t->tid],thread_temp_nrm[t->tid],thrd_uv[t->tid],thrd_verts[t->tid],thrd_ht[t->tid],thrd_ht_used[t->tid],thrd_remap_scratch[t->tid],thrd_cache_scratch[t->tid],&vPos[i],&modelVertexCounts[i],&modelTriangles[i],&modelTriangleCounts[i])) continue;
    }
    return NULL;
}

bool ParseModelData(ModelDataParser *p, u16 maxSz, const char *fn) {
    FHandle fd; int sz; char* buf = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz);
    char *c = buf, *e = buf + sz; u32 maxidx = 0, ln = 0;
    while (c < e) { // first pass - find max index
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        if (c - s > 5) { while (cEmpty(*s)) {++s;} char* col = StringFindFirstCharWithin(s, ':'); if (col && sCompUpToLen(s, "index", col - s) == 0) { u32 idx = parse_numberu32(col+1, s, ln); if(idx > maxidx){maxidx=idx;} } }
        if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c; ++ln;
    }
    if (!maxidx) { DualLogWarn("No entries in %s\n", fn); OS_Free(buf,sz); return true; }
    if (maxidx >= maxSz) { DualLogWarn("Index too large in %s\n", fn); OS_Free(buf,sz); return true; }
    u32 cnt = maxidx + 1;
    ModelData* ents = OS_AllocScratch(cnt * sizeof(ModelData));
    p->entries = ents; p->capacity = p->count = cnt;
    for (u32 i=0; i<cnt; ++i) {ents[i] = (ModelData){U16_MAX,false,255,NULL,0,{0}};}
    ModelData cur = {U16_MAX,false,255,NULL,0,{0}}; c = buf; e = buf+sz; ln = 0;
    while (c < e) {
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        size_t len = c - s; ++ln;
        if (len < 3) { if (c<e && (*c=='\r'||*c=='\n')) ++c; continue; }
        while (cEmpty(*s)) ++s;
        char* le = s + len - 1; while (le > s && cEmpty(*le)) --le;
        if (*s == '/' && s[1] == '/') goto next;
        if (*s == '#') {
            if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
            cur = (ModelData){U16_MAX,false,255,NULL,0,{0}};
            if (le > s) { size_t pl=le - s; if(pl >= sizeof(cur.path)){pl=sizeof(cur.path)-1;} mcpy(cur.path,s+1,pl); cur.path[pl] = 0; }
            goto next;
        }
        char* col = StringFindFirstCharWithin(s, ':');
        if (col) {
            char k[256]={0}, v[256]={0}; sCpy2aSubFromb(k, col-s, s, 256); sCpy2aSubFromb(v, le-col, col+1, 256);
            if (sEqual(k,"index")) cur.index = parse_numberu16(v,s,ln);
            //else if (sEqual(k,"frame")) {u16 f = parse_numberu16(v,s,ln); cur.frames = OS_Realloc(cur.frames,cur.frameCount*sizeof(u16),(cur.frameCount+1) * sizeof(u16)); cur.frames[cur.frameCount++] = f;}
            else if (sEqual(k,"frame")) {
                const char* vp = v; while (*vp == ' ' || *vp == '\t') ++vp; u16 f0 = parse_numberu16(vp, s, ln); while (*vp && ((*vp >= '0' && *vp <= '9') || *vp == '-' || *vp == '+')) ++vp; // skip first number
                while (*vp == ' ' || *vp == '\t') ++vp;
                if (*vp >= '0' && *vp <= '9') { // second number present -> range
                    u16 f1 = parse_numberu16(vp, s, ln); if (f1 < f0) { u16 t = f0; f0 = f1; f1 = t; } // be forgiving
                    for (u16 f = f0; f <= f1; ++f) { cur.frames = OS_Realloc(cur.frames, cur.frameCount*sizeof(u16), (cur.frameCount+1) * sizeof(u16)); cur.frames[cur.frameCount++] = f; }
                } else { cur.frames = OS_Realloc(cur.frames, cur.frameCount*sizeof(u16), (cur.frameCount+1) * sizeof(u16)); cur.frames[cur.frameCount++] = f0; }
            }
            else if (sEqual(k,"animationNum")) cur.animationNum = parse_numberu16(v,s,ln);
            else if (sEqual(k,"animated")) cur.animated = parse_numberu8(v,s,ln);
        }
        next: if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c;
    }
    if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
    OS_Free(buf, sz);
    return true;
}

float BvhRayAABBHit(V3 origin, V3 dir, V3 mn, V3 mx, float maxDist) { // Ray-vs-AABB slab test. Returns entry t (>=0) if the ray hits the AABB within [0, maxDist], or -1.0f if no hit. Handles axis-aligned rays (zero direction component) correctly.
    float tmin = 0.0f, tmax = maxDist;
    if (vabs(dir.x) < 1e-8f) { if (origin.x < mn.x || origin.x > mx.x) return -1.0f; } else { float inv=1.0f/dir.x; float t1=(mn.x-origin.x) * inv, t2=(mx.x-origin.x) * inv; if(t1 > t2){float t=t1; t1=t2; t2=t;} if(t1 > tmin){tmin=t1;} if(t2 < tmax){tmax=t2;} if (tmin > tmax) return -1.0f; } // X slab
    if (vabs(dir.y) < 1e-8f) { if (origin.y < mn.y || origin.y > mx.y) return -1.0f; } else { float inv=1.0f/dir.y; float t1=(mn.y-origin.y) * inv, t2=(mx.y-origin.y) * inv; if(t1 > t2){float t=t1; t1=t2; t2=t;} if(t1 > tmin){tmin=t1;} if(t2 < tmax){tmax=t2;} if (tmin > tmax) return -1.0f; } // Y slab
    if (vabs(dir.z) < 1e-8f) { if (origin.z < mn.z || origin.z > mx.z) return -1.0f; } else { float inv=1.0f/dir.z; float t1=(mn.z-origin.z) * inv, t2=(mx.z-origin.z) * inv; if(t1 > t2){float t=t1; t1=t2; t2=t;} if(t1 > tmin){tmin=t1;} if(t2 < tmax){tmax=t2;} if (tmin > tmax) return -1.0f; } // Z slab
    return tmin;
}

INLINE u32 WeldHash(i32 x, i32 y, i32 z) { u32 h = ((u32)x * 0x8DA6B343u) ^ ((u32)y * 0xD8163841u) ^ ((u32)z * 0xCB1AB31Fu); return h & (WELD_HASH_SIZE - 1); }
static void WeldModelPositions(u16 m, u32* weldHt, u32* weldHtUsed, u16* remap) {
    u32 vc = modelVertexCounts[m], tc = modelTriangleCounts[m];
    if (!vc || !tc) { physPos[m] = NULL; physTris[m] = NULL; physVertCounts[m] = 0; return; }
    const float* src = vPos[m]; mset(weldHt,0xFF,WELD_HASH_SIZE * sizeof(u32)); u32 usedSlots=0, weldedCount=0;
    float* weldedPos = (float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); // worst case: no duplicates at all
    for (u32 i = 0; i < vc; ++i) {
        float x = src[i*8+0], y = src[i*8+1], z = src[i*8+2]; i32 cx = (i32)vfloor(x * 10000), cy = (i32)vfloor(y * 10000), cz = (i32)vfloor(z * 10000); u32 found = 0xFFFFFFFFU;
        for (i32 dz = -1; dz <= 1 && found == 0xFFFFFFFFU; ++dz)
            for (i32 dy = -1; dy <= 1 && found == 0xFFFFFFFFU; ++dy)
                for (i32 dx = -1; dx <= 1 && found == 0xFFFFFFFFU; ++dx) {
                    u32 slot = WeldHash(cx+dx, cy+dy, cz+dz);
                    while (weldHt[slot] != 0xFFFFFFFFU) { u32 cand = weldHt[slot]; float ddx = weldedPos[cand*3+0]-x, ddy = weldedPos[cand*3+1]-y, ddz = weldedPos[cand*3+2]-z; if (ddx*ddx + ddy*ddy + ddz*ddz <= 0.0001f * 0.0001f) { found = cand; break; } slot = (slot + 1) & (WELD_HASH_SIZE - 1); }
                }
        if (found == 0xFFFFFFFFU) { found = weldedCount; weldedPos[weldedCount*3+0]=x; weldedPos[weldedCount*3+1]=y; weldedPos[weldedCount*3+2]=z; ++weldedCount; u32 slot = WeldHash(cx, cy, cz); while (weldHt[slot] != 0xFFFFFFFFU) slot = (slot + 1) & (WELD_HASH_SIZE - 1); weldHt[slot] = found; weldHtUsed[usedSlots++] = slot; }
        remap[i] = (u16)found;
    }
    for (u32 i = 0; i < usedSlots; ++i) weldHt[weldHtUsed[i]] = 0xFFFFFFFFU; // reset shared scratch for the next model on this thread
    float* exactPos = (float*)OS_Alloc((size_t)weldedCount * 3 * sizeof(float));
    mcpy(exactPos,weldedPos, (size_t)weldedCount * 3 * sizeof(float));
    OS_Free(weldedPos,(size_t)vc * 3 * sizeof(float));
    u16* weldedTris = (u16*)OS_Alloc((size_t)tc * 3 * sizeof(u16));
    const u16* srcTris = modelTriangles[m];
    for (u32 i = 0; i < tc * 3; ++i) weldedTris[i] = remap[srcTris[i]];
    physPos[m] = exactPos; physTris[m] = weldedTris; physVertCounts[m] = weldedCount;
}

static void* PhysGeomWorker(void* a) { PhysGeomTask* t=a; BvhBuildCtx* bvhCtx=&thrd_bvh_ctx[t->tid]; u32* ht = thrd_ht[t->tid]; u32* u=thrd_ht_used[t->tid]; u16* sc=(u16*)thrd_remap_scratch[t->tid]; for (u32 m = t->start; m < t->end; ++m) { if(m >= mdlsCnt || !modelVertexCounts[m] || !modelTriangleCounts[m]){physPos[m]=NULL; physTris[m]=NULL; physVertCounts[m]=0; continue;}  WeldModelPositions((u16)m,ht,u,sc); BuildModelBVH(bvhCtx,(u16)m); }  return NULL; }

#define _mm256_cvtps_ph(A, imm) ((__m128i)__builtin_ia32_vcvtps2ph256((__v8sf)(__m256)(A), (int)(imm)))
void LoadModels() {
    double startModelTime = get_time();
    ModelDataParser mp = {0};
    if (!ParseModelData(&mp, MAX_MDLS,"./Data/models.txt")) { DualLogError("Failed models.txt\n"); OS_Exit(1); }
    u32 maxid = 0, totalActual = 0;
    for (u32 i=0; i<mp.count; ++i) {
        if (mp.entries[i].index == U16_MAX) continue;
        totalActual++; if (mp.entries[i].index > maxid) maxid = mp.entries[i].index;
        if (mp.entries[i].animated && IsGLTFSourcePath(mp.entries[i].path)) { u32 blockMax=mp.entries[i].index + (mp.entries[i].frameCount > 0 ? (mp.entries[i].frameCount - 1) : 0); if(blockMax > maxid){maxid=blockMax;} }
    }
    DualLog("Loading   models (%d) ...",totalActual); mdlsCnt = (u16)maxid + 1; if ((u16)maxid > MAX_MDLS){DualLogError("Too many models!  Exceeds %u!\n",MAX_MDLS); OS_Exit(1);}
    vPos = OS_AllocScratch(mdlsCnt * sizeof(float*)); modelTriangles = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    modelBVHNodes = (BvhNode**)OS_Alloc(mdlsCnt * sizeof(BvhNode*)); modelBVHTriOrder = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    size_t remap_sz = (size_t)MAX_OUTPUT_VERTS * sizeof(u32), cache_sz = ((MAX_OUTPUT_VERTS/3) * sizeof(TriSort)) * 2 + (MAX_OUTPUT_VERTS * sizeof(u16)); size_t bvh_nodes_sz = (size_t)BVH_MAX_NODES_PER_MDL * sizeof(BvhNode); size_t bvh_u8_sz = (size_t)BVH_MAX_TRIS_PER_MDL * sizeof(u8); size_t bvh_u16_sz = (size_t)BVH_MAX_TRIS_PER_MDL * sizeof(u16);
    size_t arena = mdlsCnt*sizeof(i32) + mdlsCnt*sizeof(RawOBJ) + 16*threadCnt*sizeof(void*) + (size_t)threadCnt * ((MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*2)*sizeof(float) + MAX_OUTPUT_VERTS*8*sizeof(float) + WELD_HASH_SIZE*sizeof(u32) + MAX_OUTPUT_VERTS*sizeof(u32) + remap_sz + cache_sz + bvh_nodes_sz + bvh_u8_sz + 3*bvh_u16_sz);
    void* arena_base = OS_AllocScratch(arena); char* p = arena_base;
    i32* idxmap = (i32*)p; p += mdlsCnt*sizeof(i32);
    mset(idxmap, -1, mdlsCnt*sizeof(i32));
    for (u32 i=0; i<mp.count; ++i) if (mp.entries[i].index != U16_MAX) idxmap[mp.entries[i].index] = (i32)i;
    RawOBJ* raw = (RawOBJ*)p; p += mdlsCnt*sizeof(RawOBJ);
    for (u32 i=0; i<mdlsCnt; ++i) { i32 pi = idxmap[i]; if(pi >= 0){ FHandle d; int sz=0; raw[i].data=(const char*)OS_OpenAndAllocateFileBufferReadonly(mp.entries[pi].path,&d,&sz); raw[i].size=sz; raw[i].name=mp.entries[pi].path;} }
    bool* isGLTFAnimSrc = (bool*)OS_AllocScratch(mdlsCnt * sizeof(bool));
    bool* isGLTFStaticSrc = (bool*)OS_AllocScratch(mdlsCnt * sizeof(bool));
    for (u32 i=0; i<mp.count; ++i) { if (mp.entries[i].index == U16_MAX || !IsGLTFSourcePath(mp.entries[i].path)){continue;} if (mp.entries[i].animated) isGLTFAnimSrc[mp.entries[i].index] = true; else isGLTFStaticSrc[mp.entries[i].index] = true; }
    float **pos = (float**)p; p += threadCnt*sizeof(float*); float **nrm = (float**)p; p += threadCnt*sizeof(float*); float **uv = (float**)p; p += threadCnt*sizeof(float*);  float **ov = (float**)p; p += threadCnt*sizeof(float*);
    u32 **ht = (u32**)p; p += threadCnt*sizeof(u32*); u32 **ht_used = (u32**)p; p += threadCnt*sizeof(u32*); u32 **remap_scr = (u32**)p; p += threadCnt*sizeof(u32*); u8 **cache_scr = (u8**)p; p += threadCnt*sizeof(u8*);
    BvhNode **bvh_nodes_p = (BvhNode**)p; p += threadCnt*sizeof(BvhNode*); u8 **bvh_oct_p = (u8**)p; p += threadCnt*sizeof(u8*); u16 **bvh_order_p = (u16**)p; p += threadCnt*sizeof(u16*); u16 **bvh_scr_p = (u16**)p; p += threadCnt*sizeof(u16*); u16 **bvh_init_p = (u16**)p; p += threadCnt*sizeof(u16*);
    size_t psz = MAX_VERT_ELEMENT_SIZE*3*sizeof(float), usz = MAX_OUTPUT_VERTS*8*sizeof(float);
    for (int i=0; i<threadCnt; ++i) { 
        pos[i]=(float*)p; p+=psz; nrm[i] = (float*)p; p += psz; uv[i] = (float*)p; p+=MAX_VERT_ELEMENT_SIZE*2*sizeof(float); 
        ov[i]=(float*)p; p+=usz; 
        ht[i]=(u32*)p; p+=WELD_HASH_SIZE*sizeof(u32); ht_used[i] = (u32*)p; p+=MAX_OUTPUT_VERTS*sizeof(u32);
        remap_scr[i]=(u32*)p; p+=remap_sz; cache_scr[i]=(u8*)p; p+=cache_sz; bvh_nodes_p[i]=(BvhNode*)p; p += bvh_nodes_sz; bvh_oct_p[i]=(u8*)p; p += bvh_u8_sz; bvh_order_p[i]=(u16*)p; p += bvh_u16_sz; bvh_scr_p[i]=(u16*)p; p += bvh_u16_sz; bvh_init_p[i]=(u16*)p; p += bvh_u16_sz;
        mset(ht[i],0xFF,WELD_HASH_SIZE * sizeof(u32));
        thrd_bvh_ctx[i]=(BvhBuildCtx){.nodes=bvh_nodes_p[i], .triOctants=bvh_oct_p[i], .triOrder=bvh_order_p[i], .triScratch=bvh_scr_p[i], .initialTris=bvh_init_p[i], .nodeCount=0, .triCount=0};
    }
    thrd_pos = pos; thread_temp_nrm = nrm; thrd_uv = uv; thrd_verts = ov;
    thrd_ht = ht; thrd_ht_used = ht_used;
    thrd_remap_scratch = remap_scr; thrd_cache_scratch = cache_scr;
    ModelParseTask tasks[32]; u32 chunk = (mdlsCnt + threadCnt - 1) / threadCnt; OS_Thread th[32];
    for (int i=0;i<threadCnt;++i) tasks[i] = (ModelParseTask){i*chunk,(i+1)*chunk > mdlsCnt ? mdlsCnt : (i+1)*chunk,raw,isGLTFAnimSrc,isGLTFStaticSrc,i};
    if (threadCnt > 1) { // Each worker now parses its model range AND builds each model's BVH right after that model is parsed, all within the same thread -- overlapping BVH build cost with other threads' OBJ parsing instead of a separate serial post-pass.
        for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&th[i],ModelParsingWorker,&tasks[i]);
        for (int i=0;i<threadCnt;++i) OS_ThreadJoin(&th[i]);
    } else { for (int t=0;t<threadCnt;++t) ModelParsingWorker(&tasks[t]); /*Single threaded fallback*/ }
    LoadGLTFAnimatedBlocks(mp.entries,mp.count,raw);
    OS_FreeInitPhaseInner(mdlsCnt * sizeof(bool));
    OS_FreeInitPhaseInner(mdlsCnt * sizeof(bool));
    physPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*));
    physTris = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    physVertCounts = (u32*)OS_Alloc(mdlsCnt * sizeof(u32));
    PhysGeomTask ptasks[32]; OS_Thread pth[32];
    for (int i=0;i<threadCnt;++i) ptasks[i] = (PhysGeomTask){i*chunk,(i+1)*chunk > mdlsCnt ? mdlsCnt : (i+1)*chunk,i};
    if (threadCnt > 1) {     for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&pth[i],PhysGeomWorker,&ptasks[i]); } // Sneak the physics deduplication passes underneath the GPU upload ;)
    glGenBuffers(mdlsCnt,vbos); glGenBuffers(mdlsCnt,tbos); u32 tv=0,tt=0;
    for (int i=0; i<mdlsCnt; ++i) {
        if (raw[i].data) OS_Free((void*)raw[i].data,raw[i].size);
        if (!modelVertexCounts[i]) {continue;/*Skip unused index slots*/}
        tv += modelVertexCounts[i]; tt += modelTriangleCounts[i]; size_t vcz = (size_t)modelVertexCounts[i] * VRT_ATT_SZ, tcz = (size_t)modelTriangleCounts[i] * 3 * sizeof(u16);
        glBindBuffer(GL_ARRAY_BUFFER,vbos[i]); glBufferData(GL_ARRAY_BUFFER,vcz,NULL,GL_STATIC_DRAW);
        half* mpv = (half*)glMapBufferRange(GL_ARRAY_BUFFER,0,vcz,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/);
        u32 vc = modelVertexCounts[i]; const float *verts=vPos[i];
        for (u32 k = 0; k < vc; ++k) { __m256 v_in=(*(__m256_u const *)(&verts[k*8])); __m128i v_half=_mm256_cvtps_ph(v_in,0x00/*_MM_FROUND_TO_NEAREST_INT*/|0x08/*_MM_FROUND_NO_EXC*/); _mm_storeu_si128((__m128i*)&mpv[k*8],v_half); }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[i]); glBufferData(GL_ELEMENT_ARRAY_BUFFER,tcz,NULL,GL_STATIC_DRAW); void* mpt = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,0,tcz,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/); mcpy(mpt,modelTriangles[i],tcz); glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }
    glBindBuffer(GL_ARRAY_BUFFER,0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    if (threadCnt > 1) { for(int i=0;i<threadCnt;++i){OS_ThreadJoin(&pth[i]);}    } else { for(int t=0;t<threadCnt;++t){PhysGeomWorker(&ptasks[t]);} /*Single threaded fallback*/} // Regroup the physics deduplication passes after GPU upload, this does save about 0.18secs!
    for (u32 m = 0; m < mdlsCnt; ++m) {
        if (vPos[m]) { OS_Free(vPos[m],(size_t)modelVertexCounts[m] * CPU_VRT_SZ); OS_Free(modelTriangles[m],(size_t)modelTriangleCounts[m] * 3 * sizeof(u16)); vPos[m]=physPos[m]; modelTriangles[m]=physTris[m]; modelVertexCounts[m]=physVertCounts[m]; }
    }
    OS_FreeInitPhaseInner(mdlsCnt * sizeof(float*)); OS_FreeInitPhaseInner(arena); OS_FreeInitPhaseInner(mp.count * sizeof(ModelData));
    OS_FreeInitPhase();
    DualLog(" vertices: %u, tris: %u, %f secs\n",tv,tt,get_time() - startModelTime);
}

AnimationClip modelAnimationClips[MAX_ANIMS][MAX_ANIMCLIPS] = { // speed, frameStart, frameEnd, frameStartModelIndex, framerate
    [0]={[ANIM_IDLE_CLOSED]={1.0f,2,2,699,24},[ANIM_OPENING]={1.0f,2,11,699,24},[ANIM_IDLE_OPEN]={1.0f,11,11,708,24},[ANIM_CLOSING]={1.0f,12,21,709,24}}, // doorB (door2)
    [1]={[ANIM_IDLE_CLOSED]={1.0f,2,2,719,24},[ANIM_OPENING]={1.0f,2,12,719,24},[ANIM_IDLE_OPEN]={1.0f,12,12,729,24},[ANIM_CLOSING]={1.0f,14,24,731,24}}, // doorA (door1)
    [2]={[ANIM_IDLE]={1.0f,0,37,742,30},[ANIM_WALK]={1.0f,50,99,780,30},[ANIM_RUN]={1.1f,50,99,792,30},[ANIM_ATTACK1]={0.75f,111,136,830,30},[ANIM_PAIN]={0.5f,138,150,856,30},[ANIM_DYING]={0.75f,153,176,869,30}}, // npc_humanoid_mutant
    [3]={[ANIM_IDLE]={1.0f,1,207,893,24},[ANIM_ATTACK1]={1.0f,219,239,1100,24},[ANIM_WALK]={1.0f,252,308,1121,24},[ANIM_RUN]={1.0f,252,308,1121,24},[ANIM_PAIN]={1.0f,321,330,1177,24},[ANIM_PAIN2]={1.0f,331,344,1187,24},[ANIM_DYING]={1.0f,345,369,1201,24}}, // npc_cyborg_drone 
    [4]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1234,24},[ANIM_OPENING]={1.5f,2,44,1234,24},[ANIM_IDLE_OPEN]={1.0f,44,44,1276,24},[ANIM_CLOSING]={1.75f,46,96,1277,24}}, // doorD (door4, bulkhead 1)
    [5]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1328,24},[ANIM_OPENING]={1.0f,2,25,1328,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1351,24},[ANIM_CLOSING]={1.0f,27,44,1352,24}}, // doorC (door3)
    [6]={[ANIM_IDLE_CLOSED]={1.0f,1,1,1370,24},[ANIM_OPENING]={1.2f,1,30,1370,24},[ANIM_IDLE_OPEN]={1.0f,30,30,1399,24},[ANIM_CLOSING]={1.2f,32,66,1400,24}}, // doorJ (xdoor1)
    [7]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1435,24},[ANIM_OPENING]={1.2f,3,24,1435,24},[ANIM_IDLE_OPEN]={1.0f,26,26,1457,24},[ANIM_CLOSING]={1.2f,27,49,1458,24}}, // doorK (xdoor2)
    [8]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1481,24},[ANIM_OPENING]={1.2f,3,27,1481,24},[ANIM_IDLE_OPEN]={1.0f,27,27,1505,24},[ANIM_CLOSING]={1.2f,30,51,1506,24}}, // doorL (door10)
    [9]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1528,24},[ANIM_OPENING]={1.0f,3,15,1528,24},[ANIM_IDLE_OPEN]={1.0f,28,28,1541,24},[ANIM_CLOSING]={1.0f,28,39,1541,24}}, // doorE (door5)
    [10]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1553,24},[ANIM_OPENING]={1.0f,2,23,1553,24},[ANIM_IDLE_OPEN]={1.0f,23,23,1574,24},[ANIM_CLOSING]={1.0f,27,45,1575,24}}, // doorF (door6)
    [11]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1594,24},[ANIM_OPENING]={1.0f,3,22,1594,24},[ANIM_IDLE_OPEN]={1.0f,22,22,1613,24},[ANIM_CLOSING]={1.0f,25,42,1614,24}}, // doorG (door7)
    [12]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1632,24},[ANIM_OPENING]={1.0f,2,25,1632,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1655,24},[ANIM_CLOSING]={1.0f,27,49,1656,24}}, // doorH (door8)
    [13]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1679,24},[ANIM_OPENING]={1.0f,2,24,1679,24},[ANIM_IDLE_OPEN]={1.0f,24,24,1691,24},[ANIM_CLOSING]={1.0f,26,52,1692,24}}, // doorI (door9)
    [14]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1719,24},[ANIM_OPENING]={1.0f,2,20,1719,24},[ANIM_IDLE_OPEN]={1.0f,20,20,1737,24},[ANIM_CLOSING]={1.0f,22,41,1738,24}}, // door_elevator1
    [15]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1758,24},[ANIM_OPENING]={1.5f,2,21,1758,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1777,24},[ANIM_CLOSING]={1.5f,23,41,1778,24}}, // door_elevator2
    [16]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1797,24},[ANIM_OPENING]={1.0f,2,22,1797,24},[ANIM_IDLE_OPEN]={1.0f,22,22,1817,24},[ANIM_CLOSING]={1.0f,24,43,1818,24}}, // door_elevator3
    [17]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1838,24},[ANIM_OPENING]={2.0f,2,32,1838,24},[ANIM_IDLE_OPEN]={1.0f,32,32,1868,24},[ANIM_CLOSING]={2.0f,34,62,1869,24}}, // door_elevator4
    [18]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1898,24},[ANIM_OPENING]={1.0f,2,21,1898,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1917,24},[ANIM_CLOSING]={1.0f,23,41,1918,24}}, // door_secret2 (door_wall1)
    [19]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1937,24},[ANIM_OPENING]={1.0f,2,21,1937,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1956,24},[ANIM_CLOSING]={1.0f,23,41,1957,24}}, // door_secret1 (door_wall2)
    [20]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1976,24},[ANIM_OPENING]={1.0f,2,17,1976,24},[ANIM_IDLE_OPEN]={1.0f,17,17,1991,24},[ANIM_CLOSING]={1.0f,19,33,1992,24}}, // door_secret3 (door_wall3)
    [21]={[ANIM_LOOP_ALL]={1.0f,1,47,2007,24}}, // chunk_eng2_6 (eng_wallpump)
    [22]={[ANIM_LOOP_ALL]={1.0f,1,50,2054,24}}, // flight_fanwall
    [23]={[ANIM_IDLE]={1.0f,3,3,2104,24},[ANIM_WALK]={1.0f,3,36,2104,24},[ANIM_ATTACK1]={1.0f,38,56,2138,24},[ANIM_ATTACK2]={1.0f,58,81,2156,24},[ANIM_ATTACK3]={1.0f,58,81,2156,24},[ANIM_RUN]={1.0f,3,36,2104,24},[ANIM_PAIN]={1.0f,84,96,2180,24},[ANIM_DYING]={1.0f,99,106,2192,24}}, // npc_bot_cortex_reaver
    [24]={[ANIM_IDLE]={1.0f,1,60,2200,24},[ANIM_ATTACK2]={1.0f,62,83,2260,24},[ANIM_ATTACK3]={1.0f,86,122,2282,24},[ANIM_RUN]={1.0f,143,182,2319,24},[ANIM_WALK]={1.0f,143,182,2319,24},[ANIM_PAIN]={1.0f,204,214,2359,24},[ANIM_PAIN2]={1.0f,216,227,2370,24},[ANIM_DYING]={1.0f,229,268,2382,24}}, // npc_cyborgassassin
    [25]={[ANIM_IDLE]={1.0f,1,155,2422,30},[ANIM_RUN]={1.0f,190,243,2577,30},[ANIM_WALK]={1.0f,190,243,2577,30},[ANIM_ATTACK2]={1.0f,265,289,2631,30},[ANIM_ATTACK1]={1.0f,291,332,2656,30},[ANIM_DYING]={1.0f,334,417,2698,30}}, // npc_cyborg_diego
    [26]={[ANIM_IDLE]={1.0f,1,68,2782,30},[ANIM_WALK]={1.0f,90,173,2850,30},[ANIM_RUN]={1.0f,90,173,2850,30},[ANIM_ATTACK2]={1.0f,194,214,2934,30},[ANIM_PAIN]={1.0f,216,233,2955,24},[ANIM_PAIN2]={1.0f,235,244,2973,24},[ANIM_DYING]={1.0f,319,386,2983,30},[ANIM_ATTACK1]={1.0f,401,422,3051,30},[ANIM_ATTACK3]={1.0f,424,450,3073,24}}, // npc_cyborg_elite
    [27]={[ANIM_IDLE]={1.0f,1,219,3100,24},[ANIM_WALK]={1.0f,240,286,3319,24},[ANIM_RUN]={1.0f,240,286,3319,24},[ANIM_PAIN]={1.0f,306,327,3366,24},[ANIM_ATTACK1]={1.0f,329,351,3388,24},[ANIM_ATTACK2]={1.0f,353,377,3411,24},[ANIM_DYING]={1.0f,380,402,3436,24},[ANIM_ATTACK3]={1.0f,416,438,3459,24}}, // npc_cyborg_enforcer
    [28]={[ANIM_IDLE]={1.0f,1,66,3482,24},[ANIM_ATTACK3]={1.0f,68,80,3548,24},[ANIM_ATTACK2]={1.0f,82,101,3561,24},[ANIM_PAIN]={1.0f,103,114,3581,24},[ANIM_WALK]={1.0f,122,157,3593,24},[ANIM_RUN]={1.0f,122,157,3593,24},[ANIM_DYING]={1.0f,169,217,3629,24}}, // npc_cyborgwarrior
    [29]={[ANIM_IDLE]={1.0f,3,3,3678,24},[ANIM_WALK]={1.0f,15,68,3679,24},[ANIM_RUN]={1.0f,15,68,3679,24},[ANIM_PAIN]={1.0f,82,92,3732,24},[ANIM_ATTACK2]={1.0f,94,117,3743,24},[ANIM_DYING]={1.0f,119,127,3767,24}}, // npc_execbot
    [30]={[ANIM_IDLE]={1.0f,1,39,3776,24},[ANIM_WALK]={2.0f,1,39,3776,24},[ANIM_RUN]={2.0f,1,39,3776,24},[ANIM_PAIN]={1.0f,41,73,3815,24},[ANIM_PAIN2]={0.5384f,75,95,3848,24},[ANIM_ATTACK2]={1.0f,97,121,3869,24},[ANIM_ATTACK3]={1.0f,106,121,3878,24}}, // npc_flierbot
    [31]={[ANIM_IDLE]={1.0f,1,73,3894,24},[ANIM_WALK]={1.0f,88,130,3967,24},[ANIM_RUN]={1.0f,88,130,3967,24},[ANIM_PAIN]={1.0f,144,159,4010,24},[ANIM_ATTACK1]={1.0f,162,183,4026,24},[ANIM_ATTACK2]={1.0f,186,209,4048,24},[ANIM_DYING]={1.0f,212,237,4072,24}}, // npc_gortiger
    [32]={[ANIM_IDLE]={1.0f,1,47,4098,24},[ANIM_WALK]={1.0f,49,87,4145,24},[ANIM_RUN]={1.0f,49,87,4145,24},[ANIM_PAIN]={1.0f,88,107,4184,24},[ANIM_PAIN2]={1.0f,109,125,4204,24},[ANIM_PAIN3]={1.0f,127,144,4221,24},[ANIM_ATTACK2]={1.0f,145,157,4239,24},[ANIM_DYING]={1.0f,160,239,4252,24}}, // npc_hopper
    [33]={[ANIM_IDLE]={1.0f,1,30,4332,24},[ANIM_WALK]={1.0f,1,30,4332,24},[ANIM_RUN]={1.0f,1,30,4332,24},[ANIM_PAIN]={1.0f,35,51,4362,24},[ANIM_ATTACK2]={1.0f,52,72,4379,24},[ANIM_DYING]={1.0f,79,103,4400,24}}, // npc_invisomut
    [34]={[ANIM_IDLE]={1.0f,2,2,4425,24},[ANIM_ATTACK1]={2.0f,2,71,4425,24},[ANIM_WALK]={2.0f,80,107,4495,24},[ANIM_RUN]={2.0f,80,107,4495,24},[ANIM_DYING]={1.0f,117,150,4523,24}}, // npc_maintenancebot
    [35]={[ANIM_IDLE]={2.5f,1,59,4557,24},[ANIM_WALK]={2.5f,1,59,4557,24},[ANIM_RUN]={2.5f,1,59,4557,24},[ANIM_ATTACK1]={1.0f,61,79,4616,24},[ANIM_PAIN]={1.0f,81,93,4635,24},[ANIM_DYING]={1.0f,94,119,4648,24}}, // npc_mutant_avian
    [36]={[ANIM_IDLE]={1.0f,1,78,4674,24},[ANIM_WALK]={1.0f,90,129,4752,24},[ANIM_RUN]={1.0f,90,129,4752,24},[ANIM_ATTACK2]={1.0f,142,185,4792,24},[ANIM_DYING]={1.0f,188,225,4836,24},[ANIM_PAIN]={1.0f,227,235,4874,24}}, // npc_plantmutant
    [37]={[ANIM_IDLE]={1.0f,1,42,4883,24},[ANIM_WALK]={2.0f,58,85,4925,24},[ANIM_RUN]={2.0f,58,85,4925,24},[ANIM_ATTACK1]={1.0f,102,123,4953,24},[ANIM_ATTACK2]={1.0f,126,148,4975,24}}, // npc_repairbot
    [38]={[ANIM_IDLE]={1.0f,1,54,4998,24},[ANIM_WALK]={1.0f,1,54,4998,24},[ANIM_RUN]={1.0f,58,95,5052,24}}, // npc_sec1bot
    [39]={[ANIM_IDLE]={0.333f,1,17,5090,24},[ANIM_WALK]={0.333f,19,38,5107,24},[ANIM_RUN]={0.333f,19,38,5107,24},[ANIM_ATTACK2]={0.25f,39,48,5127,24},[ANIM_ATTACK3]={1.0f,49,56,5137,24},[ANIM_PAIN]={1.0f,58,63,5145,24},[ANIM_DYING]={0.2f,65,66,5151,24}}, // npc_sec2bot
    [40]={[ANIM_IDLE]={0.18f,1,9,5153,24},[ANIM_WALK]={0.333f,1,9,5153,24},[ANIM_RUN]={0.333f,1,9,5153,24},[ANIM_ATTACK1]={0.5f,18,28,5162,24},[ANIM_PAIN]={0.333f,54,63,5173,24},[ANIM_DYING]={0.333f,77,85,5183,24}}, // npc_servbot
    [41]={[ANIM_IDLE]={1.0f,1,66,5192,24},[ANIM_WALK]={2.0f,79,132,5258,24},[ANIM_RUN]={2.5f,79,132,5258,24},[ANIM_PAIN]={1.0f,145,157,5312,24},[ANIM_ATTACK2]={1.0f,159,181,5325,24},[ANIM_DYING]={1.0f,183,221,5348,24}}, // npc_virusmutant
    [42]={[ANIM_IDLE]={1.0f,1,121,5387,24},[ANIM_DYING]={1.0f,121,157,5507,24}}, // npc_zerogmut
    [43]={[ANIM_IDLE_CLOSED]={1.0f,1,1,5544,24},[ANIM_OPENING]={1.2f,2,21,5545,24},[ANIM_IDLE_OPEN]={1.0f,21,21,5564,24}}, // puzzlepanel1
    [44]={[ANIM_IDLE_CLOSED]={1.0f,0,0,5565,24},[ANIM_OPENING]={1.2f,1,17,5566,24},[ANIM_IDLE_OPEN]={1.0f,17,17,5582,24},[ANIM_INSTALL]={1.0f,19,30,5584,24},[ANIM_INSTALLED]={1.0f,18,18,5583,24}}, // puzzlepanel2
    [45]={[ANIM_IDLE_CLOSED]={1.0f,0,0,5596,24},[ANIM_OPENING]={1.2f,1,17,5597,24},[ANIM_IDLE_OPEN]={1.0f,17,17,5613,24},[ANIM_INSTALLED]={1.0f,18,18,5614,24}}, // puzzlepanel3
    [46]={[ANIM_LOOP_ALL]={1.0f,1,100,5615,24}}, // sparkingwire
    [47]={[ANIM_INACTIVE]={1.0f,2,2,5715,24},[ANIM_ACTIVATE]={1.2f,2,4,5715,24},[ANIM_ACTIVATED]={1.0f,4,4,5717,24},[ANIM_DEACTIVATE]={1.0f,5,6,5718,24}}, // switch4
    [48]={[ANIM_INACTIVE]={1.0f,2,2,5720,24},[ANIM_ACTIVATE]={1.2f,2,6,5720,24},[ANIM_ACTIVATED]={1.0f,6,6,5724,24},[ANIM_DEACTIVATE]={1.0f,8,10,5725,24}}, // switch5
    [49]={[ANIM_IDLE]={1.0f,1,1,5728,24},[ANIM_ATTACK_MISS]={1.0f,1,13,5728,24},[ANIM_ATTACK_HIT]={1.0f,18,24,5741,24}}, // v_pipe
    [50]={[ANIM_IDLE]={1.0f,1,1,5748,24},[ANIM_ATTACK_MISS]={0.5f,4,22,5749,24},[ANIM_ATTACK_HIT]={1.0f,4,22,5749,24}}, // v_rapier
    [51]={[ANIM_IDLE]={1.0f,1,65,5768,24},[ANIM_WALK]={1.0f,75,98,5833,24},[ANIM_RUN]={1.0f,75,98,5833,24},[ANIM_ATTACK2]={1.0f,109,126,5857,24},[ANIM_ATTACK1]={1.0f,128,142,5875,24},[ANIM_PAIN]={1.0f,144,159,5890,24},[ANIM_PAIN2]={1.0f,161,174,5906,24},[ANIM_DYING]={1.0f,176,243,5920,24}}, // npc_mutant_cyborg
    [52]={[ANIM_LOOP_ALL]={1.0f,1,40,5989,24}}, // g_energmine
    // TODO Add cyber exit and item anims
};

void PortalCulling(); bool ToggleDoorPortal(u32,u16,u16);
void UpdateAnims(void) {
    if (World.paused || World.menuActive) return;
    static double lastPauseTime = 0.0; if (lastPauseTime == 0.0) lastPauseTime = World.pauseRelativeTime;
    double animDT = World.pauseRelativeTime - lastPauseTime; lastPauseTime = World.pauseRelativeTime;
    if (animDT > 0.1) animDT = 0.1; if (animDT <= 0.0) return;
    bool portalsNeedUpdated = false;
    u8 animTest = Cheats.animTest;
    for (u16 i = INSTS_1ST_IDX; i < INSTANCE_COUNT; ++i) {
        Entity* e = &World.instances[i]; if (e->modelIndex >= MAX_MDLS || !(e->entflags & EF_ACTIVE) || e->animationNum >= MAX_ANIMS || e->numclips == 0) continue;
        if (animTest == 1) {
            u8 validClips[MAX_ANIMCLIPS], numValid=0, targetClipIdx=0; double totalDuration = 0.0;
            for (u8 c = 0; c < MAX_ANIMCLIPS; ++c) { AnimationClip* test = &modelAnimationClips[e->animationNum][c]; if (test->framerate > 0 && test->speed > 0) { validClips[numValid++] = c; double dur = (double)(test->frameEnd - test->frameStart + 1) / ((double)test->framerate * test->speed); totalDuration += dur; } } if (numValid == 0){continue;} 
            e->currentFrameFinished += animDT;
            while (e->currentFrameFinished >= totalDuration) { e->currentFrameFinished -= totalDuration; } if (e->currentFrameFinished < 0.0){e->currentFrameFinished = 0.0;} double t = e->currentFrameFinished, clipStartTime=0.0; AnimationClip* activeClip = &modelAnimationClips[e->animationNum][validClips[0]];
            for (u8 j = 0; j < numValid; ++j) {
                activeClip = &modelAnimationClips[e->animationNum][validClips[j]]; double activeDur = (double)(activeClip->frameEnd - activeClip->frameStart + 1) / ((double)activeClip->framerate * activeClip->speed);
                if (t < clipStartTime + activeDur) { targetClipIdx = j; break; } clipStartTime += activeDur; if (j == numValid - 1) { targetClipIdx = j; t = clipStartTime; }
            }
            u8 targetClip = validClips[targetClipIdx]; double timeInClip = t - clipStartTime, timePerFrame = 1.0 / ((double)activeClip->framerate * activeClip->speed);
            u32 frameCount = activeClip->frameEnd - activeClip->frameStart + 1, frameOffset = (u32)(timeInClip / timePerFrame); if (frameOffset >= frameCount) frameOffset = frameCount - 1;
            u32 newFrame = activeClip->frameStart + frameOffset; u16 newModel = activeClip->frameStartModelIndex + frameOffset; bool frameUpdated = (e->clip != targetClip || e->frame != newFrame || e->modelIndex != newModel);
            e->clip = targetClip; e->frame = newFrame; e->modelIndex = newModel;
            if (frameUpdated && IdxIsPortalBlockingDoor(e->index) && ToggleDoorPortal(e->portalIndex, i, modelAnimationClips[e->animationNum][ANIM_IDLE_CLOSED].frameStartModelIndex)) { portalsNeedUpdated = true; }
        } else if (animTest == 2) {
            if (Sys_Input.keyStates[KEY_1].pressed || Sys_Input.keyStates[KEY_2].pressed) {
                u8 validClips[MAX_ANIMCLIPS], numValid=0;
                for (u8 c = 0; c < MAX_ANIMCLIPS; ++c) { AnimationClip* test = &modelAnimationClips[e->animationNum][c]; if (test->framerate > 0 && test->speed > 0) { validClips[numValid++] = c; } }
                if (numValid > 0) { // Locate the current clip within the valid list (default to first if out of range)
                    u8 currentValidIdx = 0;
                    for (u8 j = 0; j < numValid; ++j) { if (validClips[j] == e->clip) { currentValidIdx = j; break; } }
                    AnimationClip* activeClip = &modelAnimationClips[e->animationNum][validClips[currentValidIdx]];
                    u32 frameCount = activeClip->frameEnd - activeClip->frameStart + 1;
                    u32 currentOffset = (e->frame >= activeClip->frameStart && (e->frame - activeClip->frameStart) < frameCount) ? (e->frame - activeClip->frameStart) : 0;
                    if (Sys_Input.keyStates[KEY_1].pressed) { currentOffset++; if (currentOffset >= frameCount) { currentOffset = 0; currentValidIdx = (currentValidIdx + 1 >= numValid) ? 0 : (currentValidIdx + 1); activeClip = &modelAnimationClips[e->animationNum][validClips[currentValidIdx]]; } }
                    else { if (currentOffset == 0) { currentValidIdx = (currentValidIdx == 0) ? (numValid - 1) : (currentValidIdx - 1); activeClip = &modelAnimationClips[e->animationNum][validClips[currentValidIdx]]; frameCount = activeClip->frameEnd - activeClip->frameStart + 1; currentOffset = frameCount - 1; } else { currentOffset--; } }
                    u8 targetClip = validClips[currentValidIdx]; u32 newFrame = activeClip->frameStart + currentOffset; u16 newModel = activeClip->frameStartModelIndex + currentOffset;
                    bool frameUpdated = (e->clip != targetClip || e->frame != newFrame || e->modelIndex != newModel);
                    e->clip = targetClip; e->frame = newFrame; e->modelIndex = newModel; e->currentFrameFinished = 0.0; if (frameUpdated && IdxIsPortalBlockingDoor(e->index) && ToggleDoorPortal(e->portalIndex, i, modelAnimationClips[e->animationNum][ANIM_IDLE_CLOSED].frameStartModelIndex)) { portalsNeedUpdated = true; }
                }
            }
        }
        if (animTest || e->clip >= e->numclips) continue;
        AnimationClip* clip = (AnimationClip*)&modelAnimationClips[e->animationNum][e->clip]; if (clip->framerate <= 0 || clip->speed <= 0) continue;
        e->currentFrameFinished += animDT * clip->speed; double timePerFrame = 1.0 / (double)clip->framerate;
        if (e->currentFrameFinished >= timePerFrame) {
            u32 framesToAdvance = (u32)(e->currentFrameFinished / timePerFrame), frameCount = clip->frameEnd - clip->frameStart + 1;
            u16 prevFrame = e->frame;
            e->currentFrameFinished -= (double)framesToAdvance * timePerFrame; e->frame = (frameCount <= 1) ? clip->frameStart : clip->frameStart + ((e->frame - clip->frameStart + framesToAdvance) % frameCount); e->modelIndex = clip->frameStartModelIndex + (e->frame - clip->frameStart);
            if (e->frame != prevFrame) e->animFinished = World.current_time; // hysteresis: stamp ONLY on an actual frame change (single-frame idle clips don't restamp, so they don't hold neighbors awake)
            if (IdxIsPortalBlockingDoor(e->index) && ToggleDoorPortal(e->portalIndex, i, modelAnimationClips[e->animationNum][ANIM_IDLE_CLOSED].frameStartModelIndex)) portalsNeedUpdated = true;
        }
    }
    if (portalsNeedUpdated) PortalCulling();
}

void ChangeAnim(Entity* e, u8 clip) { e->clip = clip; e->currentFrameFinished = 0.0; AnimationClip* c = (AnimationClip*)&modelAnimationClips[e->animationNum][e->clip]; e->frame = c->frameStart; } // TODO actually use this!}
