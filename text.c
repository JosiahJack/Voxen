// text.c - Text and Font Rendering/Loading System
#include "common.h"
// stb_truetype.h - v1.26 - public domain, authored from 2009-2021 by Sean Barrett / RAD Game Tools...Heavily gutted by Josiah Jack
typedef struct { void* ptr; size_t sz; } TAlloc;
static TAlloc* ttAllocs = NULL;
static int tallocCount=0;
static void* ttalloc(size_t n) { if (tallocCount>=4674) {DualLogError("ttalloc too many!\n"); return NULL;} void*p=OS_Alloc(n); ttAllocs[tallocCount++]=(TAlloc){p,n}; return p; }
static void  ttfree (void* p) { if(!p||tallocCount==0||ttAllocs[tallocCount-1].ptr!=p)return;OS_Free(p,ttAllocs[tallocCount-1].sz);tallocCount--; }
static u16 ttUSHORT(u8*p) {return p[0]*256 + p[1];} 
static i16 ttSHORT (u8*p) {return p[0]*256 + p[1];}
static u32 ttULONG (u8*p) {return((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];}
#define stbtt_tag4(p,a,b,c,d) ((p)[0]==(a) && (p)[1]==(b) && (p)[2]==(c) && (p)[3]==(d))
#define stbtt_tag(p,s) ((p)[0]==(s[0]) && (p)[1]==(s[1]) && (p)[2]==(s[2]) && (p)[3]==(s[3]))
typedef struct { u8*data; int cursor,size; } stbtt__buf;
static stbtt__buf stbtt__new_buf(const void*p,size_t s){stbtt__buf r; r.data=(u8*)p; r.size=(int)s; r.cursor=0; return r;}
static u8  _bg8(stbtt__buf*b){return b->cursor>=b->size?0:b->data[b->cursor++];}
static u8  _bp8(stbtt__buf*b){return b->cursor>=b->size?0:b->data[b->cursor];}
static void _bsk(stbtt__buf*b,int o) { b->cursor = (o>b->size||o<0) ? b->size : o; }
static void _bskip(stbtt__buf*b,int o){_bsk(b,b->cursor+o);}
static u32 _bg(stbtt__buf*b,int n){u32 v=0;for(int i=0;i<n;i++)v=(v<<8)|_bg8(b); return v;}
static stbtt__buf _brange(const stbtt__buf*b,int o,int s){stbtt__buf r=stbtt__new_buf(NULL,0); if(o<0||s<0||o>b->size||s>b->size-o)return r; r.data=b->data+o; r.size=s; return r;}
static stbtt__buf _cff_idx(stbtt__buf*b){int c=b->cursor,n=_bg(b,2);if(n){int os=_bg8(b); _bskip(b,os*n); _bskip(b,_bg(b,os)-1);}return _brange(b,c,b->cursor-c);}
static u32 _cff_int(stbtt__buf*b){int b0=_bg8(b);if(b0>=32&&b0<=246)return b0-139;if(b0>=247&&b0<=250)return(b0-247)*256+_bg8(b)+108;if(b0>=251&&b0<=254)return-(b0-251)*256-_bg8(b)-108;if(b0==28)return _bg(b,2);if(b0==29)return _bg(b,4);return 0;}
static void _cff_skip_op(stbtt__buf*b){if(_bp8(b)==30){_bskip(b,1);while(b->cursor<b->size){int v=_bg8(b);if((v&0xF)==0xF||(v>>4)==0xF)break;}}else _cff_int(b);}
static stbtt__buf _dict_get(stbtt__buf*b, int key) { b->cursor = (0 > b->size) ? b->size : 0; while(b->cursor < b->size) { int e,op,s=b->cursor; while(_bp8(b) >= 28) _cff_skip_op(b); e = b->cursor; op = _bg8(b); if(op==12) op = _bg8(b) | 0x100; if(op==key) return _brange(b,s,e-s); } return _brange(b,0,0); }
static void _dict_ints(stbtt__buf*b,int key,int n,u32*out) { stbtt__buf op = _dict_get(b,key); for (int i=0;i<n && op.cursor<op.size;++i) {out[i] = (u32)_cff_int(&op);} }
static stbtt__buf _cff_idx_get(stbtt__buf b,int i){_bsk(&b,0);int n=_bg(&b,2),os=_bg8(&b);_bskip(&b,i*os);int s=_bg(&b,os),e=_bg(&b,os);return _brange(&b,2+(n+1)*os+s,e-s);}
enum{STBTT_vmove=1,STBTT_vline,STBTT_vcurve,STBTT_vcubic};
typedef struct{i16 x,y,cx,cy,cx1,cy1;u8 type,padding;}stbtt_vertex;
typedef struct{void*userdata;u8*data;int fontstart,numGlyphs,loca,head,glyf,hhea,hmtx,index_map,indexToLocFormat;stbtt__buf cff,charstrings,gsubrs,subrs,fontdicts,fdselect;}stbtt_fontinfo;
static u32 _find_table(u8*d,u32 fs,const char*tag){i32 n=ttUSHORT(d+fs+4);u32 td=fs+12;for(i32 i=0;i<n;++i){u32 l=td+16*i;if(stbtt_tag(d+l+0,tag))return ttULONG(d+l+8);}return 0;}
static stbtt__buf _get_subrs(stbtt__buf cff,stbtt__buf fd){u32 so=0,pl[2]={0,0};_dict_ints(&fd,18,2,pl);if(!pl[1]||!pl[0])return stbtt__new_buf(NULL,0);stbtt__buf pd=_brange(&cff,pl[1],pl[0]);_dict_ints(&pd,19,1,&so);if(!so)return stbtt__new_buf(NULL,0);_bsk(&cff,pl[1]+so);return _cff_idx(&cff);}
static int stbtt_InitFont_internal(stbtt_fontinfo* info, u8* data, int fs) {
    u32 cmap,t,i,nt;info->data=data;info->fontstart=fs;info->cff=stbtt__new_buf(NULL,0);
    cmap=_find_table(data,fs,"cmap"); info->loca=_find_table(data,fs,"loca"); info->head=_find_table(data,fs,"head");
    info->glyf=_find_table(data,fs,"glyf"); info->hhea=_find_table(data,fs,"hhea"); info->hmtx=_find_table(data,fs,"hmtx");
    if(!cmap || !info->head || !info->hhea || !info->hmtx) return 0;
    if(info->glyf){ if(!info->loca)return 0; }
    else{
        u32 cs=2,chstr=0,fda=0,fds=0,cff=_find_table(data,fs,"CFF "); if(!cff)return 0;
        info->fontdicts=stbtt__new_buf(NULL,0);info->fdselect=stbtt__new_buf(NULL,0);
        info->cff=stbtt__new_buf(data+cff,16*1024*1024);stbtt__buf b=info->cff;
        _bskip(&b,2);_bsk(&b,_bg8(&b)); _cff_idx(&b);
        stbtt__buf tdi=_cff_idx(&b),td=_cff_idx_get(tdi,0);_cff_idx(&b);info->gsubrs=_cff_idx(&b);
        _dict_ints(&td,17,1,&chstr); _dict_ints(&td,0x100|6,1,&cs);_dict_ints(&td,0x100|36,1,&fda);_dict_ints(&td,0x100|37,1,&fds);
        info->subrs=_get_subrs(b,td);
        if (cs!=2||chstr==0) return 0;
        if (fda) { if(!fds) {return 0;} _bsk(&b,fda);info->fontdicts=_cff_idx(&b);info->fdselect=_brange(&b,fds,b.size-fds); }
        _bsk(&b,chstr);info->charstrings=_cff_idx(&b);
    }
    t=_find_table(data,fs,"maxp"); info->numGlyphs = t ? ttUSHORT(data+t+4) : 0xffff;
    nt=ttUSHORT(data+cmap+2);info->index_map=0;
    for(i=0;i<nt;++i){u32 er=cmap+4+8*i;switch(ttUSHORT(data+er)){case 3:switch(ttUSHORT(data+er+2)){case 1:case 10:info->index_map=cmap+ttULONG(data+er+4);}break;case 0:info->index_map=cmap+ttULONG(data+er+4);break;}}
    if(!info->index_map)return 0;
    info->indexToLocFormat=ttUSHORT(data+info->head+50);return 1;
}

int _font_offset(u8*d,int idx){ if(stbtt_tag4(d,'1',0,0,0)||stbtt_tag(d,"typ1")||stbtt_tag(d,"OTTO")||stbtt_tag4(d,0,1,0,0)||stbtt_tag(d,"true")){return idx==0?0:-1;} if(stbtt_tag(d,"ttcf")&&(ttULONG(d+4)==0x00010000||ttULONG(d+4)==0x00020000)){i32 n=((i32)d[8]<<24)|((i32)d[9]<<16)|((i32)d[10]<<8)|d[11]; if(idx>=n){return -1;} return ttULONG(d+12+idx*4); } return -1; }
int stbtt_GetFontOffsetForIndex(const u8*d,int i){return _font_offset((u8*)d,i);}
int stbtt_FindGlyphIndex(const stbtt_fontinfo*info,int cp){
    u8*d=info->data;u32 im=info->index_map;u16 fmt=ttUSHORT(d+im);
    if(fmt==0) { i32 b=ttUSHORT(d+im+2); return cp<b-6 ? (*(u8*)(d+im+6+cp)) : 0; }
    if(fmt==6) { u32 f=ttUSHORT(d+im+6),n=ttUSHORT(d+im+8); return(u32)cp>=f&&(u32)cp<f+n?ttUSHORT(d+im+10+(cp-f)*2):0;}
    if(fmt==2)return 0;
    if(fmt==4){
        u16 sc=ttUSHORT(d+im+6)>>1,sr=ttUSHORT(d+im+8)>>1,es=ttUSHORT(d+im+10),rs=ttUSHORT(d+im+12)>>1; u32 ec=im+14,s=ec;if(cp>0xffff)return 0;
        if (cp>=ttUSHORT(d+s+rs*2)) s+=rs*2;s-=2;
        while(es) { sr>>=1; u16 e=ttUSHORT(d+s+sr*2); if(cp>e) {s+=sr*2;} --es; }
        s+=2;{u16 it=(u16)((s-ec)>>1),st=ttUSHORT(d+im+14+sc*2+2+2*it),la=ttUSHORT(d+ec+2*it); if(cp<st||cp>la)return 0;u16 off=ttUSHORT(d+im+14+sc*6+2+2*it);
        return off?ttUSHORT(d+off+(cp-st)*2+im+14+sc*6+2+2*it):(u16)(cp+ttSHORT(d+im+14+sc*4+2+2*it));}
    }
    if(fmt==12||fmt==13){u32 ng=ttULONG(d+im+12);i32 lo=0,hi=(i32)ng;
        while(lo<hi){i32 m=lo+((hi-lo)>>1);u32 sc=ttULONG(d+im+16+m*12),ec=ttULONG(d+im+16+m*12+4);
        if((u32)cp<sc)hi=m;else if((u32)cp>ec)lo=m+1;else{u32 sg=ttULONG(d+im+16+m*12+8);return fmt==12?sg+cp-sc:sg;}}return 0;}
    return 0;
}

void _sv(stbtt_vertex*v,u8 t,i32 x,i32 y,i32 cx,i32 cy){v->type=t;v->x=(i16)x;v->y=(i16)y;v->cx=(i16)cx;v->cy=(i16)cy;}
int _glyf_off(const stbtt_fontinfo*info,int gi){ if(gi>=info->numGlyphs||info->indexToLocFormat>=2){return-1;} int g1,g2;if(info->indexToLocFormat==0){g1=info->glyf+ttUSHORT(info->data+info->loca+gi*2)*2;g2=info->glyf+ttUSHORT(info->data+info->loca+gi*2+2)*2;} else{g1=info->glyf+ttULONG(info->data+info->loca+gi*4);g2=info->glyf+ttULONG(info->data+info->loca+gi*4+4);} return g1==g2?-1:g1; }
int _close_shape(stbtt_vertex*v,int n,int wo,int so,i32 sx,i32 sy,i32 scx,i32 scy,i32 cx,i32 cy){
    if(so){if(wo)_sv(&v[n++],STBTT_vcurve,(cx+scx)>>1,(cy+scy)>>1,cx,cy);_sv(&v[n++],STBTT_vcurve,sx,sy,scx,scy);}
    else{if(wo)_sv(&v[n++],STBTT_vcurve,sx,sy,cx,cy);else _sv(&v[n++],STBTT_vline,sx,sy,0,0);}
    return n;
}

int _GetGlyphShapeT2(const stbtt_fontinfo*,int,stbtt_vertex**);
int stbtt_GetGlyphShape(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv);
int _GetGlyphShapeTT(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){
    u8*d=info->data;stbtt_vertex*verts=0;int nv=0,g=_glyf_off(info,gi);*pv=NULL;if(g<0)return 0;
    i16 nc=ttSHORT(d+g);
    if(nc>0){
        u8*ep=d+g+10;int ins=ttUSHORT(d+g+10+nc*2);u8*pts=d+g+10+nc*2+2+ins;
        int n=1+ttUSHORT(ep+nc*2-2),m=n+2*nc;verts=(stbtt_vertex*)ttalloc(m*sizeof(verts[0]));if(!verts)return 0;
        int off=m-n;u8 fl=0,fc=0;
        for(int i=0;i<n;++i){if(fc==0){fl=*pts++;if(fl&8)fc=*pts++;}else--fc;verts[off+i].type=fl;}
        i32 x=0;for(int i=0;i<n;++i){fl=verts[off+i].type;if(fl&2){i16 dx=*pts++;x+=(fl&16)?dx:-dx;}else if(!(fl&16)){x+=(i16)(pts[0]*256+pts[1]);pts+=2;}verts[off+i].x=(i16)x;}
        i32 y=0;for(int i=0;i<n;++i){fl=verts[off+i].type;if(fl&4){i16 dy=*pts++;y+=(fl&32)?dy:-dy;}else if(!(fl&32)){y+=(i16)(pts[0]*256+pts[1]);pts+=2;}verts[off+i].y=(i16)y;}
        i32 sx=0,sy=0,cx=0,cy=0,scx=0,scy=0;int wo=0,so=0,nm=0,j=0;
        for(int i=0;i<n;++i){fl=verts[off+i].type;x=(i16)verts[off+i].x;y=(i16)verts[off+i].y;
            if(nm==i){if(i)nv=_close_shape(verts,nv,wo,so,sx,sy,scx,scy,cx,cy);so=!(fl&1);
                if(so){scx=x;scy=y;if(!(verts[off+i+1].type&1)){sx=(x+(i32)verts[off+i+1].x)>>1;sy=(y+(i32)verts[off+i+1].y)>>1;}else{sx=verts[off+i+1].x;sy=verts[off+i+1].y;++i;}}else{sx=x;sy=y;}
                _sv(&verts[nv++],STBTT_vmove,sx,sy,0,0);wo=0;nm=1+ttUSHORT(ep+j++*2);
            }else{
                if(!(fl&1)){
                    if(wo)_sv(&verts[nv++],STBTT_vcurve,(cx+x)>>1,(cy+y)>>1,cx,cy); cx=x; cy=y; wo=1;
                } else { _sv(&verts[nv++],wo ? STBTT_vcurve : STBTT_vline,x,y,wo ? cx : 0, wo ? cy : 0); wo=0; }
            }
        }
        nv=_close_shape(verts,nv,wo,so,sx,sy,scx,scy,cx,cy);
    }else if(nc<0){
        u8*comp=d+g+10;int more=1;
        while(more){stbtt_vertex*cv=0,*tmp=0;float mtx[6]={1,0,0,1,0,0};
            u16 fl=ttSHORT(comp);comp+=2;u16 gidx=ttSHORT(comp);comp+=2;
            if(fl&2) { if(fl&1) { mtx[4] = ttSHORT(comp); comp+=2; mtx[5]=ttSHORT(comp); comp+=2; } else { mtx[4]=(*(i8*)(comp)); comp++; mtx[5]=(*(i8*)(comp)); comp++; }}
            if(fl&(1<<3)){mtx[0]=mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=mtx[2]=0;}
            else if(fl&(1<<6)){mtx[0]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=mtx[2]=0;mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;}
            else if(fl&(1<<7)){mtx[0]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=ttSHORT(comp)/16384.0f;comp+=2;mtx[2]=ttSHORT(comp)/16384.0f;comp+=2;mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;}
            float fm=vsqrtf(mtx[0]*mtx[0]+mtx[1]*mtx[1]),fn=vsqrtf(mtx[2]*mtx[2]+mtx[3]*mtx[3]);
            int cn=stbtt_GetGlyphShape(info,gidx,&cv);
            if(cn>0){for(int i=0;i<cn;++i){stbtt_vertex*v=&cv[i];i16 vx=v->x,vy=v->y;v->x=(i16)(fm*(mtx[0]*vx+mtx[2]*vy+mtx[4]));v->y=(i16)(fn*(mtx[1]*vx+mtx[3]*vy+mtx[5]));vx=v->cx;vy=v->cy;v->cx=(i16)(fm*(mtx[0]*vx+mtx[2]*vy+mtx[4]));v->cy=(i16)(fn*(mtx[1]*vx+mtx[3]*vy+mtx[5]));}
                tmp=(stbtt_vertex*)ttalloc((nv+cn)*sizeof(stbtt_vertex));if(!tmp){ttfree(verts);ttfree(cv);return 0;}
                if(nv>0&&verts) mcpy(tmp,verts,nv*sizeof(stbtt_vertex)); mcpy(tmp+nv,cv,cn*sizeof(stbtt_vertex));ttfree(verts);ttfree(cv);verts=tmp;nv+=cn;}
            more=fl&(1<<5);}
    }
    *pv=verts;return nv;
}

typedef struct{int bounds,started;float first_x,first_y,x,y;i32 min_x,max_x,min_y,max_y;stbtt_vertex*pvertices;int num_vertices;}stbtt__csctx;
#define CSCTX_INIT(b) {b,0,0,0,0,0,0,0,0,0,NULL,0}
void _trk(stbtt__csctx*c,i32 x,i32 y){if(x>c->max_x||!c->started)c->max_x=x;if(y>c->max_y||!c->started)c->max_y=y;if(x<c->min_x||!c->started)c->min_x=x;if(y<c->min_y||!c->started)c->min_y=y;c->started=1;}
void _csv(stbtt__csctx*c,u8 t,i32 x,i32 y,i32 cx,i32 cy,i32 cx1,i32 cy1){if(c->bounds){_trk(c,x,y);if(t==STBTT_vcubic){_trk(c,cx,cy);_trk(c,cx1,cy1);}}else{_sv(&c->pvertices[c->num_vertices],t,x,y,cx,cy);c->pvertices[c->num_vertices].cx1=(i16)cx1;c->pvertices[c->num_vertices].cy1=(i16)cy1;}c->num_vertices++;}
void _csclose(stbtt__csctx*c){if(c->first_x!=c->x||c->first_y!=c->y)_csv(c,STBTT_vline,(int)c->first_x,(int)c->first_y,0,0,0,0);}
void _csmove(stbtt__csctx*c,float dx,float dy){_csclose(c);c->first_x=c->x=c->x+dx;c->first_y=c->y=c->y+dy;_csv(c,STBTT_vmove,(int)c->x,(int)c->y,0,0,0,0);}
void _csline(stbtt__csctx*c,float dx,float dy){c->x+=dx;c->y+=dy;_csv(c,STBTT_vline,(int)c->x,(int)c->y,0,0,0,0);}
void _cscurve(stbtt__csctx*c,float d1,float e1,float d2,float e2,float d3,float e3){float cx1=c->x+d1,cy1=c->y+e1,cx2=cx1+d2,cy2=cy1+e2;c->x=cx2+d3;c->y=cy2+e3;_csv(c,STBTT_vcubic,(int)c->x,(int)c->y,(int)cx1,(int)cy1,(int)cx2,(int)cy2);}
stbtt__buf _subr(stbtt__buf idx,int n){ _bsk(&idx,0); int c = _bg(&idx,2); int bias = (c >= 33900) ? 32768 : ((c >= 1240) ? 1131 : 107); n+=bias; return (n<0 || n>=c) ? stbtt__new_buf(NULL,0) : _cff_idx_get(idx,n); }
stbtt__buf _cid_subrs(const stbtt_fontinfo*info,int gi){stbtt__buf fd=info->fdselect;int nr,st,end,v,fmt,sel=-1,i;_bsk(&fd,0);fmt=_bg8(&fd); if(fmt==0){_bskip(&fd,gi);sel=_bg8(&fd);} else if(fmt==3){nr=_bg(&fd,2);st=_bg(&fd,2);for(i=0;i<nr;i++){v=_bg8(&fd);end=_bg(&fd,2);if(gi>=st&&gi<end){sel=v;break;}st=end;}} if(sel==-1){return stbtt__new_buf(NULL,0);}return _get_subrs(info->cff,_cff_idx_get(info->fontdicts,sel));}
int _run_cs(const stbtt_fontinfo*info,int gi,stbtt__csctx*c){
    int hdr=1,mb=0,ssh=0,sp=0,hs=0,i,b0;float s[48],f;
    stbtt__buf ss[10],subrs=info->subrs,b=_cff_idx_get(info->charstrings,gi);
#define ERR(x) return 0
#define CHK(n) if(sp<(n))ERR(#n)
    while(b.cursor<b.size){int cs=1;i=0;b0=_bg8(&b);
        switch(b0){
        case 0x13:case 0x14:if(hdr)mb+=sp/2;hdr=0;_bskip(&b,(mb+7)/8);break;
        case 0x01:case 0x03:case 0x12:case 0x17:mb+=sp/2;break;
        case 0x15:hdr=0;CHK(2);_csmove(c,s[sp-2],s[sp-1]);break;
        case 0x04:hdr=0;CHK(1);_csmove(c,0,s[sp-1]);break;
        case 0x16:hdr=0;CHK(1);_csmove(c,s[sp-1],0);break;
        case 0x05:CHK(2);for(;i+1<sp;i+=2)_csline(c,s[i],s[i+1]);break;
        case 0x07:CHK(1);goto vlt;
        case 0x06:CHK(1);for(;;){if(i>=sp)break;_csline(c,s[i++],0);vlt:if(i>=sp)break;_csline(c,0,s[i++]);}break;
        case 0x1F:CHK(4);goto hvc;
        case 0x1E:CHK(4);for(;;){if(i+3>=sp)break;_cscurve(c,0,s[i],s[i+1],s[i+2],s[i+3],(sp-i==5)?s[i+4]:0);i+=4;hvc:if(i+3>=sp)break;_cscurve(c,s[i],0,s[i+1],s[i+2],(sp-i==5)?s[i+4]:0,s[i+3]);i+=4;}break;
        case 0x08:CHK(6);for(;i+5<sp;i+=6)_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);break;
        case 0x18:CHK(8);for(;i+5<sp-2;i+=6)_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);_csline(c,s[i],s[i+1]);break;
        case 0x19:CHK(8);for(;i+1<sp-6;i+=2)_csline(c,s[i],s[i+1]);_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);break;
        case 0x1A:case 0x1B:CHK(4);f=0;if(sp&1)f=s[i++];for(;i+3<sp;i+=4,f=0)_cscurve(c,b0==0x1B?s[i]:f,b0==0x1B?f:s[i],s[i+1],s[i+2],b0==0x1B?s[i+3]:0,b0==0x1B?0:s[i+3]);break;
        case 0x0A:if(!hs){if(info->fdselect.size)subrs=_cid_subrs(info,gi);hs=1;}
        case 0x1D:CHK(1);if(ssh>=10)ERR("recursion");ss[ssh++]=b;b=_subr(b0==0x0A?subrs:info->gsubrs,(int)s[--sp]);if(!b.size)ERR("subr");b.cursor=0;cs=0;break;
        case 0x0B:if(ssh<=0)ERR("return");b=ss[--ssh];cs=0;break;
        case 0x0E:_csclose(c);return 1;
        case 0x0C:{int b1=_bg8(&b);switch(b1){
            case 0x22:CHK(7);_cscurve(c,s[0],0,s[1],s[2],s[3],0);_cscurve(c,s[4],0,s[5],-s[2],s[6],0);break;
            case 0x23:CHK(13);_cscurve(c,s[0],s[1],s[2],s[3],s[4],s[5]);_cscurve(c,s[6],s[7],s[8],s[9],s[10],s[11]);break;
            case 0x24:CHK(9);_cscurve(c,s[0],s[1],s[2],s[3],s[4],0);_cscurve(c,s[5],0,s[6],s[7],s[8],-(s[1]+s[3]+s[7]));break;
            case 0x25:CHK(11);{float dx=s[0]+s[2]+s[4]+s[6]+s[8],dy=s[1]+s[3]+s[5]+s[7]+s[9],d6x=s[10],d6y=s[10];if(vabs(dx)>vabs(dy))d6y=-dy;else d6x=-dx;_cscurve(c,s[0],s[1],s[2],s[3],s[4],s[5]);_cscurve(c,s[6],s[7],s[8],s[9],d6x,d6y);}break;
            default:ERR("escape");}}break;
        default:if(b0!=255&&b0!=28&&b0<32)ERR("reserved");f=(b0==255)?(float)(i32)_bg(&b,4)/0x10000:(_bskip(&b,-1),(float)(i16)_cff_int(&b));if(sp>=48)ERR("overflow");s[sp++]=f;cs=0;break;}
        if(cs)sp=0;}ERR("no endchar");
#undef ERR
#undef CHK
}

int stbtt_GetGlyphShape(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){return info->cff.size?_GetGlyphShapeT2(info,gi,pv):_GetGlyphShapeTT(info,gi,pv);}
int _GetGlyphShapeT2(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){stbtt__csctx cc=CSCTX_INIT(1),oc=CSCTX_INIT(0);if(_run_cs(info,gi,&cc)){*pv=(stbtt_vertex*)ttalloc(cc.num_vertices*sizeof(stbtt_vertex));oc.pvertices=*pv;if(_run_cs(info,gi,&oc))return oc.num_vertices;}*pv=NULL;return 0;}
int _GetGlyphInfoT2(const stbtt_fontinfo*info,int gi,int*x0,int*y0,int*x1,int*y1){stbtt__csctx c=CSCTX_INIT(1);int r=_run_cs(info,gi,&c);if(x0)*x0=r?c.min_x:0;if(y0)*y0=r?c.min_y:0;if(x1)*x1=r?c.max_x:0;if(y1)*y1=r?c.max_y:0;return r?c.num_vertices:0;}
int stbtt_GetGlyphBox(const stbtt_fontinfo*info,int gi,int*x0,int*y0,int*x1,int*y1){
    if(info->cff.size){_GetGlyphInfoT2(info,gi,x0,y0,x1,y1);}
    else{int g=_glyf_off(info,gi);if(g<0)return 0;if(x0)*x0=ttSHORT(info->data+g+2);if(y0)*y0=ttSHORT(info->data+g+4);if(x1)*x1=ttSHORT(info->data+g+6);if(y1)*y1=ttSHORT(info->data+g+8);}
    return 1;
}

void stbtt_GetGlyphHMetrics(const stbtt_fontinfo*info,int gi,int*adv,int*lsb){
    u16 n=ttUSHORT(info->data+info->hhea+34);
    if(gi<n){if(adv)*adv=ttSHORT(info->data+info->hmtx+4*gi);if(lsb)*lsb=ttSHORT(info->data+info->hmtx+4*gi+2);}
    else{if(adv)*adv=ttSHORT(info->data+info->hmtx+4*(n-1));if(lsb)*lsb=ttSHORT(info->data+info->hmtx+4*n+2*(gi-n));}
}

float stbtt_ScaleForPixelHeight(const stbtt_fontinfo*info,float h){return h/(float)(ttSHORT(info->data+info->hhea+4)-ttSHORT(info->data+info->hhea+6));}
float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo*info,float px){return px/(float)ttUSHORT(info->data+info->head+18);}
void GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo*font,int g,float sx,float sy,float shx,float shy,int*ix0,int*iy0,int*ix1,int*iy1){ int x0=0,y0=0,x1,y1;if(!stbtt_GetGlyphBox(font,g,&x0,&y0,&x1,&y1)){if(ix0)*ix0=0;if(iy0)*iy0=0;if(ix1)*ix1=0;if(iy1)*iy1=0;} else{if(ix0)*ix0=(int)vfloor(x0*sx+shx);if(iy0)*iy0=(int)vfloor(-y1*sy+shy);if(ix1)*ix1=(int)vceil(x1*sx+shx);if(iy1)*iy1=(int)vceil(-y0*sy+shy);} }
typedef struct{int w,h,stride;u8*pixels;}stbtt__bitmap; typedef struct tt_heapchk{ struct tt_heapchk* next; }tt_heapchk; typedef struct{ tt_heapchk* head; void* first_free; int remaining; }stbtt__hheap;
void* _hha(stbtt__hheap* hh,size_t sz) { if(hh->first_free){void*p=hh->first_free;hh->first_free=*(void**)p;return p;} if(!hh->remaining) {int c=sz<32?2000:sz<128?800:100;tt_heapchk*ck=(tt_heapchk*)ttalloc(sizeof(*ck)+sz*c); if(!ck){return NULL;} ck->next=hh->head; hh->head=ck; hh->remaining=c;} --hh->remaining; return(char*)hh->head+sizeof(tt_heapchk)+sz*hh->remaining; }
void _hhf(stbtt__hheap* hh,void*p) { *(void**)p=hh->first_free;hh->first_free=p; }
typedef struct{ float x0,y0,x1,y1; int invert; }stbtt__edge; typedef struct stbtt__active_edge{ struct stbtt__active_edge*next; float fx,fdx,fdy,direction,sy,ey; }stbtt__active_edge;
void _hce(float*sl,int x,stbtt__active_edge*e,float x0,float y0,float x1,float y1){
    if(y0==y1||y0>e->ey||y1<e->sy)return;if(y0<e->sy){x0+=(x1-x0)*(e->sy-y0)/(y1-y0);y0=e->sy;}if(y1>e->ey){x1+=(x1-x0)*(e->ey-y1)/(y1-y0);y1=e->ey;}
    if(x0<=x&&x1<=x)sl[x]+=e->direction*(y1-y0);else if(x0>=x+1&&x1>=x+1);else sl[x]+=e->direction*(y1-y0)*(1.0f-((x0-(float)x)+(x1-(float)x))/2.0f);
}

float _ptz(float h,float t0,float t1,float b0,float b1){ return ((t1-t0)+(b1-b0))/2.0f*h; }
void _fae(float*sl,float*sf,int len,stbtt__active_edge*e,float yt){
    float yb = yt + 1;
    while(e) {
        if(e->fdx==0){float x0=e->fx;if(x0<len){if(x0>=0){_hce(sl,(int)x0,e,x0,yt,x0,yb);_hce(sf-1,(int)x0+1,e,x0,yt,x0,yb);}else _hce(sf-1,0,e,x0,yt,x0,yb);}}
        else{float x0=e->fx,dx=e->fdx,dy=e->fdy,xb=x0+dx,xt,xbt,sy0,sy1;
            if(e->sy>yt){xt=x0+dx*(e->sy-yt);sy0=e->sy;}else{xt=x0;sy0=yt;}
            if(e->ey<yb){xbt=x0+dx*(e->ey-yt);sy1=e->ey;}else{xbt=xb;sy1=yb;}
            if(xt>=0&&xbt>=0&&xt<len&&xbt<len){
                if((int)xt==(int)xbt){int x=(int)xt;float h=(sy1-sy0)*e->direction;sl[x]+=_ptz(h,xt,(float)x+1.0f,xbt,(float)x+1.0f);sf[x]+=h;}
                else{float yc,yf,step,sign,area;
                    if(xt>xbt){float t;sy0=yb-(sy0-yt);sy1=yb-(sy1-yt);t=sy0;sy0=sy1;sy1=t;t=xbt;xbt=xt;xt=t;dx=-dx;dy=-dy;t=x0;x0=xb;xb=t;}
                    int x1=(int)xt,x2=(int)xbt;yc=yt+dy*((float)(x1+1)-x0);yf=yt+dy*((float)x2-x0);if(yc>yb)yc=yb;sign=e->direction;area=sign*(yc-sy0);
                    sl[x1] += area*((float)(x1+1)-xt)/2;
                    if(yf>yb){yf=yb;dy=(yf-yc)/((float)x2-(float)(x1+1));}
                    step=sign*dy;for(int x=x1+1;x<x2;++x){sl[x]+=area+step/2;area+=step;}
                    sl[x2]+=area+sign*_ptz(sy1-yf,(float)x2,(float)x2+1.0f,xbt,(float)x2+1.0f);sf[x2]+=sign*(sy1-sy0);}
            }else{for(int x=0;x<len;++x){float y0=yt,x1f=(float)x,x2f=(float)(x+1),x3=xb,y3=yb;float y1=((float)x-x0)/dx+yt,y2=((float)(x+1)-x0)/dx+yt;
                if(x0<x1f&&x3>x2f){_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else if(x3<x1f&&x0>x2f){_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if ((x0<x1f&&x3>x1f) || (x3<x1f&&x0>x1f)) {_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if ((x0<x2f&&x3>x2f) || (x3<x2f&&x0>x2f)) {_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else _hce(sl,x,e,x0,y0,x3,y3);}}
        }e=e->next;}
}

void _rse(stbtt__bitmap*res,stbtt__edge*e,int n,int ox,int oy){
    stbtt__hheap hh={0,0,0}; stbtt__active_edge*active=NULL; int y,j=0,i; float sd[129],*sl,*sl2; if(res->w>64)sl=(float*)ttalloc((size_t)(res->w*2+1)*sizeof(float));else sl=sd; sl2=sl+res->w;y=oy;e[n].y0=(float)(oy+res->h)+1;
    while(j<res->h){float syt=(float)y,syb=(float)y+1;stbtt__active_edge**step=&active;
        mset(sl,0,(size_t)res->w*sizeof(sl[0]));mset(sl2,0,((size_t)res->w+1)*sizeof(sl[0]));
        while(*step){stbtt__active_edge*z=*step;if(z->ey<=syt){*step=z->next;z->direction=0;_hhf(&hh,z);}else step=&(*step)->next;}
        while(e->y0<=syb){
            if(e->y0!=e->y1){
                stbtt__active_edge* z=(stbtt__active_edge*)_hha(&hh,sizeof(*z));
                if(z) { float dxdy = (e->x1-e->x0)/(e->y1-e->y0); z->fdx = dxdy; z->fdy = dxdy ? 1.0f/dxdy : 0; z->fx = e->x0 + dxdy * (syt - e->y0) - (float)ox; z->direction = e->invert ? 1.0f : -1.0f; z->sy = e->y0; z->ey = e->y1; z->next = 0; if(j == 0 && oy != 0 && z->ey < syt){z->ey=syt;} z->next = active; active = z; }
            }++e;
        }
        if(active)_fae(sl,sl2+1,res->w,active,syt);
        {float sum=0;for(i=0;i<res->w;++i){float k;int m;sum+=sl2[i];k=(float)vabs(sl[i]+sum)*255.0f+0.5f;m=(int)k;if(m>255)m=255;res->pixels[j*res->stride+i]=(u8)m;}}
        step=&active;while(*step){stbtt__active_edge*z=*step;z->fx+=z->fdx;step=&(*step)->next;}++y;++j;}
    tt_heapchk* c = hh.head; while(c){ tt_heapchk* hp = c->next; ttfree(c); c = hp;} if(sl != sd) ttfree(sl);
}

#define _CMP(a,b) ((a)->y0<(b)->y0)
#define _SWP(a,b) {stbtt__edge t_=(a);(a)=(b);(b)=t_;}
void _eis(stbtt__edge*p,int n){for(int i=1;i<n;++i){stbtt__edge t=p[i];int j=i;while(j>0&&_CMP(&t,&p[j-1])){p[j]=p[j-1];--j;}p[j]=t;}}
void _eqs(stbtt__edge*p,int n){while(n>12){int m=n>>1,c01=_CMP(&p[0],&p[m]),c12=_CMP(&p[m],&p[n-1]);if(c01!=c12){int z=(_CMP(&p[0],&p[n-1])==c12)?0:n-1;_SWP(p[z],p[m]);}_SWP(p[0],p[m]);int i=1,j=n-1;for(;;){while(_CMP(&p[i],&p[0]))++i;while(_CMP(&p[0],&p[j]))--j;if(i>=j)break;_SWP(p[i],p[j]);++i;--j;}if(j<n-i){_eqs(p,j);p+=i;n-=i;}else{_eqs(p+i,n-i);n=j;}}}
void _esort(stbtt__edge*p,int n){_eqs(p,n);_eis(p,n);}
void _add_pt(V2*p,int n,float x,float y){if(p){p[n].x=x;p[n].y=y;}}
int _tess_c(V2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float fsq,int n){
    float mx=(x0+2*x1+x2)/4,my=(y0+2*y1+y2)/4,dx=(x0+x2)/2-mx,dy=(y0+y2)/2-my;
    if(n>16||dx*dx+dy*dy<=fsq){_add_pt(pts,(*np)++,x2,y2);return 1;}
    _tess_c(pts,np,x0,y0,(x0+x1)/2,(y0+y1)/2,mx,my,fsq,n+1);_tess_c(pts,np,mx,my,(x1+x2)/2,(y1+y2)/2,x2,y2,fsq,n+1);return 1;
}

void _tess_cb(V2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float fsq,int n){
    float d0=vsqrtf((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)),d1=vsqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)),d2=vsqrtf((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2)),ds=vsqrtf((x3-x0)*(x3-x0)+(y3-y0)*(y3-y0)),ll=d0+d1+d2;
    if(n>16||ll*ll-ds*ds<=fsq){_add_pt(pts,(*np)++,x3,y3);return;}
    float x01=(x0+x1)/2,y01=(y0+y1)/2,x12=(x1+x2)/2,y12=(y1+y2)/2,x23=(x2+x3)/2,y23=(y2+y3)/2,xa=(x01+x12)/2,ya=(y01+y12)/2,xb=(x12+x23)/2,yb=(y12+y23)/2,mx=(xa+xb)/2,my=(ya+yb)/2;
    _tess_cb(pts,np,x0,y0,x01,y01,xa,ya,mx,my,fsq,n+1);_tess_cb(pts,np,mx,my,xb,yb,x23,y23,x3,y3,fsq,n+1);
}

static V2* _flatten(stbtt_vertex*v,int nv,float flat,int**cl,int*nc){
    float fsq=flat*flat;int n=0;for(int i=0;i<nv;++i)if(v[i].type==STBTT_vmove)++n;
    *nc=n;if(!n)return 0;*cl=(int*)ttalloc(sizeof(int)*(size_t)n);V2*pts=0;int np=0;
    for (int pass=0;pass<2;++pass) {
        float x=0,y=0;int start=0;n=-1;if(pass==1){pts=(V2*)ttalloc((size_t)np*sizeof(V2));if(!pts)goto err;}np=0;
        for(int i=0;i<nv;++i){
            switch(v[i].type) {
                case STBTT_vmove:if(n>=0)(*cl)[n]=np-start;start=np;++n;x=v[i].x;y=v[i].y;_add_pt(pts,np++,x,y);break;
                case STBTT_vline:x=v[i].x;y=v[i].y;_add_pt(pts,np++,x,y);break;
                case STBTT_vcurve:_tess_c(pts,&np,x,y,v[i].cx,v[i].cy,v[i].x,v[i].y,fsq,0);x=v[i].x;y=v[i].y;break;
                case STBTT_vcubic:_tess_cb(pts,&np,x,y,v[i].cx,v[i].cy,v[i].cx1,v[i].cy1,v[i].x,v[i].y,fsq,0);x=v[i].x;y=v[i].y;break;
            }
        }
        (*cl)[n]=np-start;
    }
    return pts;
    err: ttfree(pts); ttfree(*cl); *cl=0; *nc=0; return NULL;
}

static void _rasterize(stbtt__bitmap*res,V2*pts,int*wc,int nw,float sx,float sy,float shx,float shy,int ox,int oy,int inv){
    float ysi=inv?-sy:sy;stbtt__edge*e;int n=0,i,j,k;for(i=0;i<nw;++i)n+=wc[i];
    e=(stbtt__edge*)ttalloc(sizeof(*e)*((size_t)n+1));if(!e)return;n=0;int m=0;
    for(i=0;i<nw;++i){V2*p=pts+m;m+=wc[i];j=wc[i]-1;for(k=0;k<wc[i];j=k++){int a=k,b=j;if(p[j].y==p[k].y)continue;e[n].invert=0;if(inv?p[j].y>p[k].y:p[j].y<p[k].y){e[n].invert=1;a=j;b=k;}e[n].x0=p[a].x*sx+shx;e[n].y0=p[a].y*ysi+shy;e[n].x1=p[b].x*sx+shx;e[n].y1=p[b].y*ysi+shy;++n;}}
    _esort(e,n);_rse(res,e,n,ox,oy);ttfree(e);
}

void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo*info,u8*out,int ow,int oh,int ostr,float sx,float sy,float shx,float shy,int g){
    stbtt_vertex*v;int ix0,iy0,nv=stbtt_GetGlyphShape(info,g,&v);stbtt__bitmap gbm;
    GetGlyphBitmapBoxSubpixel(info,g,sx,sy,shx,shy,&ix0,&iy0,0,0);gbm.pixels=out;gbm.w=ow;gbm.h=oh;gbm.stride=ostr;
    float scale=sx>sy?sy:sx;int wc=0;int*wl=NULL;V2*win=_flatten(v,nv,0.35f/scale,&wl,&wc);
    if(win){_rasterize(&gbm,win,wl,wc,sx,sy,shx,shy,ix0,iy0,1);ttfree(wl);ttfree(win);}if(v)ttfree(v);
}

typedef int stbrp_coord; typedef struct{int width,height,x,y,bottom_y;}stbrp_context;
typedef struct{u8 x;}stbrp_node; typedef struct{stbrp_coord x,y;int id,w,h,was_packed;}stbrp_rect;
void stbrp_pack_rects(stbrp_context*con,stbrp_rect*rects,int n){int i;for(i=0;i<n;++i){if(con->x+rects[i].w>con->width){con->x=0;con->y=con->bottom_y;}if(con->y+rects[i].h>con->height)break;rects[i].x=con->x;rects[i].y=con->y;rects[i].was_packed=1;con->x+=rects[i].w;if(con->y+rects[i].h>con->bottom_y)con->bottom_y=con->y+rects[i].h;}for(;i<n;++i)rects[i].was_packed=0;}
typedef struct{void*uac;void*pack_info;int width,height,stride_in_bytes,padding,skip_missing;u32 h_oversample,v_oversample;u8*pixels;}stbtt_pack_context;
typedef struct{u16 x0,y0,x1,y1;float xoff,yoff,xadvance,xoff2,yoff2;}stbtt_packedchar;
typedef struct{float font_size;int first_unicode_codepoint_in_range;int*array_of_unicode_codepoints;int num_chars;stbtt_packedchar*chardata_for_range;u8 h_oversample,v_oversample;}FPackRange;
int stbtt_PackBegin(stbtt_pack_context*spc,u8* px, int pw, int ph, int str, int pad, void* a){ stbrp_context*ctx=(stbrp_context*)ttalloc(sizeof(*ctx)); *ctx=(stbrp_context){pw-pad,ph-pad,0,0,0}; if(px){mset(px,0,(size_t)(pw*ph));} return *spc=(stbtt_pack_context){a,ctx,pw,ph,str ? str : pw,pad,0,1,1,px},1; }
void _pre(u8*p,int w,int h,int str,u32 kw,int vert){ int outer=vert?w:h, inner=vert?h:w, os=vert?1:str, is=vert?str:1; for(int j=0;j<outer;++j,p+=os){u8 buf[8]={0};int tot=0;for(int i=0;i<inner;++i){if(i<=inner-(int)kw){tot+=p[i*is]-buf[i&7];buf[(i+kw)&7]=p[i*is];}else tot-=buf[i&7];p[i*is]=(u8)(tot/kw);}} }
float _oshift(int os){return os?-(float)(os-1)/(2.0f*(float)os):0.0f;}
int stbtt_PackFontRanges(stbtt_pack_context*spc,const u8*fontdata,int fi,FPackRange*ranges,int nr){
    stbtt_fontinfo info;int n=0;stbrp_rect*rects;
    for (int i=0;i<nr;++i) { for(int j=0;j<ranges[i].num_chars;++j) ranges[i].chardata_for_range[j].x0 = ranges[i].chardata_for_range[j].y0 = ranges[i].chardata_for_range[j].x1 = ranges[i].chardata_for_range[j].y1 = 0; }
    for (int i=0;i<nr;++i) n+=ranges[i].num_chars;
    rects=(stbrp_rect*)ttalloc(sizeof(*rects)*(size_t)n);if(!rects)return 0;
    info.userdata = spc->uac;
    stbtt_InitFont_internal(&info,(u8*)fontdata,stbtt_GetFontOffsetForIndex(fontdata,fi));
    int mga=0,k=0;
    for (int i=0;i<nr;++i) {
        float fh=ranges[i].font_size,sc=fh>0 ? stbtt_ScaleForPixelHeight(&info,fh) : stbtt_ScaleForMappingEmToPixels(&info,-fh); ranges[i].h_oversample = (u8)spc->h_oversample; ranges[i].v_oversample=(u8)spc->v_oversample;
        for (int j=0;j<ranges[i].num_chars;++j){
            int x0,y0,x1,y1,cp=ranges[i].array_of_unicode_codepoints ? ranges[i].array_of_unicode_codepoints[j] : ranges[i].first_unicode_codepoint_in_range+j, g=stbtt_FindGlyphIndex(&info,cp);
            if (g == 0 && (spc->skip_missing || mga)) { rects[k].w=rects[k].h=0;
            } else {
                GetGlyphBitmapBoxSubpixel(&info,g,sc*(float)spc->h_oversample,sc*(float)spc->v_oversample,0,0,&x0,&y0,&x1,&y1);
                rects[k].w=(stbrp_coord)(x1-x0+spc->padding+(int)spc->h_oversample-1);
                rects[k].h=(stbrp_coord)(y1-y0+spc->padding+(int)spc->v_oversample-1);
                if(g==0)mga=1;
            }
            ++k;
        }
    }
    n = k; k=0; stbrp_pack_rects(spc->pack_info,rects,n);
    int i,j,mg=-1,rv=1,oh=spc->h_oversample,ov=spc->v_oversample;
    for (i = 0; i < nr; ++i) {
        FPackRange* rng = &ranges[i];
        float sc = rng->font_size > 0 ? stbtt_ScaleForPixelHeight(&info,rng->font_size) : stbtt_ScaleForMappingEmToPixels(&info,-rng->font_size);
        spc->h_oversample = rng->h_oversample; spc->v_oversample = rng->v_oversample;
        for (j = 0; j < rng->num_chars; ++j, ++k) {
            stbrp_rect* r = &rects[k];
            if (r->was_packed && r->w && r->h) {
                int cp = rng->array_of_unicode_codepoints ? rng->array_of_unicode_codepoints[j] : rng->first_unicode_codepoint_in_range + j;
                int g = stbtt_FindGlyphIndex(&info,cp);
                stbtt_packedchar* bc = &rng->chardata_for_range[j];
                r->x += spc->padding; r->y += spc->padding; r->w -= spc->padding; r->h -= spc->padding;
                int adv,lsb; stbtt_GetGlyphHMetrics(&info,g,&adv,&lsb);
                int x0,y0,x1,y1;
                GetGlyphBitmapBoxSubpixel(&info,g,sc * spc->h_oversample,sc * spc->v_oversample,0,0,&x0,&y0,&x1,&y1);
                u8* p_pixels = spc->pixels + r->x + r->y * spc->stride_in_bytes;
                stbtt_MakeGlyphBitmapSubpixel(&info,p_pixels,r->w - spc->h_oversample + 1,r->h - spc->v_oversample + 1,spc->stride_in_bytes,sc * spc->h_oversample,sc * spc->v_oversample,0,0,g);
                if (spc->h_oversample > 1) _pre(p_pixels,r->w,r->h,spc->stride_in_bytes,spc->h_oversample,0);
                if (spc->v_oversample > 1) _pre(p_pixels,r->w,r->h,spc->stride_in_bytes,spc->v_oversample,1);
                bc->x0 = r->x; bc->y0 = r->y; bc->x1 = r->x + r->w; bc->y1 = r->y + r->h;
                bc->xadvance = sc * adv;
                bc->xoff  = x0 * (1.0f / spc->h_oversample) + _oshift(spc->h_oversample);
                bc->yoff  = y0 * (1.0f / spc->v_oversample) + _oshift(spc->v_oversample);
                bc->xoff2 = (x0 + r->w) * (1.0f / spc->h_oversample) + _oshift(spc->h_oversample);
                bc->yoff2 = (y0 + r->h) * (1.0f / spc->v_oversample) + _oshift(spc->v_oversample);
                if (!g) mg = j;
            } else if (r->was_packed && !r->w && !r->h && mg >= 0) rng->chardata_for_range[j] = rng->chardata_for_range[mg];
            else if (!spc->skip_missing) rv = 0;
        }
    }
    spc->h_oversample = oh; spc->v_oversample = ov;
    return rv;
}
// Font Loading System
int numPackedGlyphs=0,numPackedGlyphsStopD=0; extern u32 textVAO,textVBO,textSP;
u32 fontAtlasTex,fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS],fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth=0.0f,fixedNumberAdvanceWidthStopD=0.0f;
static const char* fallbackFontPaths[]={"./Fonts/FreeSerifBold.ttf","./Fonts/cambriab.ttf","./Fonts/NotoSansCJK-Subset.ttf"}, *fontPaths[]={"./Fonts/SystemShockText.ttf","./Fonts/StopD.ttf"};
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
void BuildAtlas(u32* atlasTex, GlyphRange* ranges, int* numPacked, stbtt_packedchar* packedChars, float* fixedNumAdv, float baseH, int fontIdx, u8 fontID, u8* bmp) {
    stbtt_pack_context pc; stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL); pc.h_oversample=pc.v_oversample=3; pc.skip_missing=1; *numPacked=0;
    for(int r=0; r<numFontRanges; ++r) { ranges[r].startIndex = *numPacked;
        for(int i=0; i<ranges[r].count; ++i) {
            if(*numPacked >= MAX_GLYPHS) break;
            u32 cp = ranges[r].first + i; stbtt_fontinfo* font = &fontInfo[fontIdx]; u8* data = fontData[fontIdx]; int g = stbtt_FindGlyphIndex(font, cp);
            if(!g) { g = GetGlyphAndFont(cp, &font, fontID); if(!g){continue;} data = (font == &fontInfo[fontIdx]) ? fontData[fontIdx] : ((LoadedFont*)((char*)font - __builtin_offsetof(LoadedFont, info)))->data; }
            float h = baseH; if(font != &fontInfo[fontIdx]) h *= 1.2f;
            FPackRange range = {h, cp, NULL, 1, &packedChars[*numPacked], 0, 0}; stbtt_PackFontRanges(&pc, data, 0, &range, 1);
            int idx = (*numPacked)++; if(cp >= '0' && cp <= '9') *fixedNumAdv = vmax(*fixedNumAdv,packedChars[idx].xadvance);
        }
    } ttfree(pc.pack_info); GenerateAndBindTexture(atlasTex, 0x8229, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0x1903, GL_UNSIGNED_BYTE, 0x2601, bmp);
}

void InitFontAtlasses() {
    DebugRAM("start font load");
    double t0=get_time();DualLog("Loading    5 fonts...");
    ttAllocs = OS_Alloc(4674 * sizeof(TAlloc));
    FHandle fd1,fd2;int sz1,sz2;
    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if(!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)){DualLogError("%s font init failed\n",fontPaths[0]);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)){DualLogError("%s font init failed\n",fontPaths[1]);OS_Exit(1);}
    fallbackFonts[0]=LoadFallbackFont(fallbackFontPaths[0],2,0);
    fallbackFonts[1]=LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2]=LoadFallbackFont(fallbackFontPaths[2],4,0);
    u8* bmp = OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Primary atlas
    BuildAtlas(&fontAtlasTex, fontRanges, &numPackedGlyphs, fontPackedChar, &fixedNumberAdvanceWidth, 20.0f, 0, FONT_NORMAL, bmp);
    mset(bmp, 0, FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Secondary atlas
    BuildAtlas(&fontAtlasTexStopD, fontRangesStopD, &numPackedGlyphsStopD, fontPackedCharStopD, &fixedNumberAdvanceWidthStopD, 54.0f, 1, FONT_STOPD, bmp);
    OS_Free(bmp,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    OS_Free(fontData[0],sz1);
    OS_Free(fontData[1],sz2);
    OS_Free(fontData[2],fallbackFonts[0].size);
    OS_Free(fontData[3],fallbackFonts[1].size);
    OS_Free(fontData[4],fallbackFonts[2].size);
    OS_Free(ttAllocs,4674 * sizeof(TAlloc));
    DebugRAM("after font load");
    glUseProgram(textSP); glUniform1i(1,2);
    DualLog(" took %f s\n",get_time()-t0);
}
// Localization
size_t utf16le_to_utf8(const u8*src,size_t slen,char*dst,size_t dlen){
    size_t dp=0,sp=0;
    while(sp<slen&&dp<dlen-4){
        if(sp+1>=slen)break;u32 c=(u32)src[sp+1]<<8|src[sp];sp+=2;
        if(c<0x80){dst[dp++]=(char)c;} else if(c<0x800){dst[dp++]=(char)(0xC0|(c>>6));dst[dp++]=(char)(0x80|(c&0x3F));} else if(c<0x10000){dst[dp++]=(char)(0xE0|(c>>12));dst[dp++]=(char)(0x80|((c>>6)&0x3F));dst[dp++]=(char)(0x80|(c&0x3F));} else continue;
    }
    dst[dp]='\0';return dp;
}

static const char* localizations[8]={"./Data/text_english.txt","./Data/text_espanol.txt","./Data/text_deutsch.txt","./Data/text_francais.txt","./Data/text_nihongo.txt","./Data/text_russkiy.txt","./Data/text_italiano.txt","./Data/text_portugues.txt"};
u8* LoadTextFile(const char* path, size_t* out_size, size_t* out_dp, int* out_utf16) {
    FHandle dfd = INVALID_FHANDLE; int asz = 0;
    u8* data = OS_OpenAndAllocateFileBufferReadonly(path, &dfd, &asz); if(!data || asz <= 0) { DualLogError("Failed to load text file: %s\n", path); *out_size = 0; return NULL; }
    *out_size = (size_t)asz; *out_dp = 0; *out_utf16 = 0;
    if(asz >= 2 && data[0]==0xFF && data[1]==0xFE) { *out_dp = 2; *out_utf16 = 1; }
    else if(asz >= 3 && data[0]==0xEF && data[1]==0xBB && data[2]==0xBF) { *out_dp = 3; }
    else { int nl=0; for(size_t i=1; i<(size_t)asz && i<1024; i+=2) if(data[i]==0) nl++; if(nl*3 > asz) *out_utf16 = 1; }
    return data;
}

void LoadTextForLanguage(u8 lang) {
    char tf[256]={0}; sCpy2aSubFromb(tf,255,localizations[lang<8?lang:0],256);
    char line[T_LOGSTR_MAX]; size_t dp=0; int utf16=0,ln=0; Sys_Text.file_data = LoadTextFile(tf,&Sys_Text.file_size,&dp,&utf16);    
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
    char line[1024]; size_t dp=0;int utf16=0; Sys_Text.filelog_data = LoadTextFile(tf,&Sys_Text.filelog_size,&dp,&utf16);  
    while(dp<Sys_Text.filelog_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.filelog_size){u16 ch=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.filelog_size){u16 nx=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.filelog_size){u8 c=Sys_Text.filelog_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.filelog_size&&Sys_Text.filelog_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls; if(!ll)continue;
        if(utf16)utf16le_to_utf8(&Sys_Text.filelog_data[ls],ll,line,sizeof(line)); else { if (ll>=sizeof(line)) ll=sizeof(line)-1; mcpy(line,&Sys_Text.filelog_data[ls],ll);line[ll]='\0'; }
        size_t sl=slen(line); while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0'; if(!sl)continue;
        int li=-1,ilh=-1,irh=-1,lt=0,lf=0,fi=0;char*pos=line;
        while(*pos&&fi<32){while(*pos==' ')++pos;char*st=pos;int q=(*pos=='"');if(q)++pos;while(*pos){if(*pos==','&&!q)break;if(*pos=='"'&&q){if(pos[1]==','){pos++;break;}if(pos[1]=='"'){pos+=2;continue;}}++pos;}char*en=pos;if(q&&*en=='"')--en;size_t tl=(size_t)(en-st);if(!tl){if(*pos==',')++pos;fi++;continue;}
            switch(fi){
                case 0:  li=s2i32Len(st,tl); if (li<0||li>=LOGCNT) goto nxt; break;                 case 1:  ilh=s2i32Len(st,tl); break; case 2: irh=s2i32Len(st,tl); break;
                case 3:  if(li>=0&&li<LOGCNT) sCpy2aSubFromb(   World.audiologNames[li],tl,st,sizeof(World.audiologNames[0]));    break; case 4: if(li>=0&&li<LOGCNT) sCpy2aSubFromb(World.audiologSenders[li],tl,st,sizeof(World.audiologSenders[0])); break;
                case 5:  if(li>=0&&li<LOGCNT) sCpy2aSubFromb(World.audiologSubjects[li],tl,st,sizeof(World.audiologSubjects[0])); break; case 6:  lt=s2i32Len(st,tl); break;    case 7:  lf=s2i32Len(st,tl); break;
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
        const stbtt_packedchar *b = ((fontID==FONT_STOPD) ? fontPackedCharStopD : fontPackedChar) + idx;
        float qx0 = vfloor((xpos + b->xoff) + 0.5f), qy0 = vfloor((ypos + b->yoff) + 0.5f);
        float qs0 = b->x0 * invatsz, qt0 = b->y0 * invatsz, qs1 = b->x1 * invatsz, qt1 = b->y1 * invatsz;
        float vx0 = qx0*scale - bw, vy0 = qy0*scale - bw, vx1 = (qx0 + b->xoff2 - b->xoff)*scale + bw, vy1 = (qy0 + b->yoff2 - b->yoff)*scale + bw;
        float s0 = qs0 - puv, t0 = qt0 - puv, s1 = qs1 + puv, t1 = qt1 + puv, z = 0.0f;
        float tv[30] = { vx0,vy0,z,s0,t0, vx1,vy1,z,s1,t1, vx1,vy0,z,s1,t0, vx0,vy0,z,s0,t0, vx0,vy1,z,s0,t1, vx1,vy1,z,s1,t1 };
        mcpy(textVertexData + vc * 30,tv,sizeof(tv)); vc++;
        if (cp >= '0' && cp <= '9' && fontID == FONT_STOPD){xpos = qx0 + fixedNumberAdvanceWidthStopD;}
        else xpos += b->xadvance;
    }
    if (vc) { glBindBuffer(GL_ARRAY_BUFFER,textVBO); glBufferData(GL_ARRAY_BUFFER,vc*30*sizeof(float),textVertexData,GL_DYNAMIC_DRAW); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,vc*6); }
}
