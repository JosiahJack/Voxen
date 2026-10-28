// text.c - Text and Font Rendering/Loading System
#include "common.h"
// stb_truetype.h - v1.26 - public domain, authored from 2009-2021 by Sean Barrett / RAD Game Tools...Heavily gutted by Josiah Jack
typedef struct { void* ptr; size_t sz; } TAlloc; static TAlloc* ttAllocs = NULL; static int tallocCount=0; typedef struct { u8*data; int cursor,size; } stbtt__buf;
static void* ttalloc(size_t n) { if (tallocCount>=4674) {DualLogError("ttalloc too many!\n"); return NULL;} void*p=OS_Alloc(n); if(!p||p==(void*)-1){DualLogError("ttalloc failed\n");return NULL;} ttAllocs[tallocCount++]=(TAlloc){p,n}; return p; }
static void ttfree (void* p) { if(!p)return; for(int i=tallocCount-1;i>=0;--i){ if(ttAllocs[i].ptr==p){ OS_Free(p,ttAllocs[i].sz); ttAllocs[i]=ttAllocs[--tallocCount]; return; } } }
static u16 ttUSHORT(u8*p) {return p[0]*256 + p[1];} 
static i16 ttSHORT (u8*p) {return p[0]*256 + p[1];}
static u32 ttULONG (u8*p) {return((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];}
#define stbtt_tag4(p,a,b,c,d) ((p)[0]==(a) && (p)[1]==(b) && (p)[2]==(c) && (p)[3]==(d))
#define stbtt_tag(p,s) ((p)[0]==(s[0]) && (p)[1]==(s[1]) && (p)[2]==(s[2]) && (p)[3]==(s[3]))
enum{STBTT_vmove=1,STBTT_vline,STBTT_vcurve,STBTT_vcubic};
typedef struct{i16 x,y,cx,cy,cx1,cy1;u8 type,padding;}stbtt_vertex;
typedef struct{void*userdata;u8*data;int fontstart,numGlyphs,loca,head,glyf,hhea,hmtx,index_map,indexToLocFormat;}stbtt_fontinfo;
static u32 _find_table(u8*d,u32 fs,const char*tag){i32 n=ttUSHORT(d+fs+4);u32 td=fs+12;for(i32 i=0;i<n;++i){u32 l=td+16*i;if(stbtt_tag(d+l+0,tag))return ttULONG(d+l+8);}return 0;}
static int stbtt_InitFont_internal(stbtt_fontinfo* info, u8* data, int fs) {
    u32 cmap,t,i,nt;info->data=data;info->fontstart=fs; 
    cmap=_find_table(data,fs,"cmap"); info->loca=_find_table(data,fs,"loca"); info->head=_find_table(data,fs,"head"); info->glyf=_find_table(data,fs,"glyf"); info->hhea=_find_table(data,fs,"hhea"); info->hmtx=_find_table(data,fs,"hmtx"); if(!cmap || !info->head || !info->hhea || !info->hmtx) return 0;
    if(info->glyf){ if(!info->loca)return 0; }
    else{ return 0; }
    t=_find_table(data,fs,"maxp"); info->numGlyphs = t ? ttUSHORT(data+t+4) : 0xffff; nt=ttUSHORT(data+cmap+2);info->index_map=0; for(i=0;i<nt;++i){u32 er=cmap+4+8*i;switch(ttUSHORT(data+er)){case 3:switch(ttUSHORT(data+er+2)){case 1:case 10:info->index_map=cmap+ttULONG(data+er+4);}break;case 0:info->index_map=cmap+ttULONG(data+er+4);break;}} if(!info->index_map)return 0; info->indexToLocFormat=ttUSHORT(data+info->head+50);return 1;
}

int _font_offset(u8*d,int idx){ if(stbtt_tag4(d,'1',0,0,0)||stbtt_tag4(d,0,1,0,0)||stbtt_tag(d,"true")){return idx==0?0:-1;} if(stbtt_tag(d,"ttcf")&&(ttULONG(d+4)==0x00010000||ttULONG(d+4)==0x00020000)){i32 n=((i32)d[8]<<24)|((i32)d[9]<<16)|((i32)d[10]<<8)|d[11]; if(idx>=n){return -1;} return ttULONG(d+12+idx*4); } return -1; }
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

INLINE void _sv(stbtt_vertex*v,u8 t,i32 x,i32 y,i32 cx,i32 cy){v->type=t;v->x=(i16)x;v->y=(i16)y;v->cx=(i16)cx;v->cy=(i16)cy;}
static int _glyf_off(const stbtt_fontinfo*info,int gi){ if(gi>=info->numGlyphs||info->indexToLocFormat>=2){return-1;} int g1,g2;if(info->indexToLocFormat==0){g1=info->glyf+ttUSHORT(info->data+info->loca+gi*2)*2;g2=info->glyf+ttUSHORT(info->data+info->loca+gi*2+2)*2;} else{g1=info->glyf+ttULONG(info->data+info->loca+gi*4);g2=info->glyf+ttULONG(info->data+info->loca+gi*4+4);} return g1==g2?-1:g1; }
static int _close_shape(stbtt_vertex*v,int n,int wo,int so,i32 sx,i32 sy,i32 scx,i32 scy,i32 cx,i32 cy){
    if(so){if(wo)_sv(&v[n++],STBTT_vcurve,(cx+scx)>>1,(cy+scy)>>1,cx,cy);_sv(&v[n++],STBTT_vcurve,sx,sy,scx,scy);}
    else{if(wo)_sv(&v[n++],STBTT_vcurve,sx,sy,cx,cy);else _sv(&v[n++],STBTT_vline,sx,sy,0,0);}
    return n;
}

static int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int gi, stbtt_vertex **pv){
    u8*d=info->data; stbtt_vertex*verts=0; int nv=0, g=_glyf_off(info,gi); *pv=NULL; if(g<0){return 0;}
        i16 nc=ttSHORT(d+g);
        if (nc > 0) {
            u8*ep=d+g+10;int ins=ttUSHORT(d+g+10+nc*2);u8*pts=d+g+10+nc*2+2+ins;
            int n=1+ttUSHORT(ep+nc*2-2), m=n+2*nc; verts=(stbtt_vertex*)ttalloc(m*sizeof(verts[0])); if(!verts){return 0;}
            int off=m-n; u8 fl=0,fc=0;
            for(int i=0;i<n;++i){if(fc==0){fl=*pts++; if(fl&8)fc=*pts++;}else{--fc;} verts[off+i].type=fl;}
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
        } else if (nc < 0) {
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
                    tmp=(stbtt_vertex*)ttalloc((nv+cn)*sizeof(stbtt_vertex));if(!tmp){ttfree(cv);ttfree(verts);return 0;}
                    if(nv>0&&verts){mcpy(tmp,verts,nv*sizeof(stbtt_vertex));} mcpy(tmp+nv,cv,cn*sizeof(stbtt_vertex)); ttfree(cv); ttfree(verts); verts=tmp; nv+=cn;}
                more=fl&(1<<5);}
        }
        *pv = verts;
        return nv;
    }

static void GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo*font,int g,float sx,float sy,float shx,float shy,int*ix0,int*iy0,int*ix1,int*iy1){
    int x0=0,y0=0,x1=0,y1=0,has=0;
    int g1=_glyf_off(font,g);if(g1>=0) { has = 1; x0 = ttSHORT(font->data + g1 + 2); y0 = ttSHORT(font->data + g1 + 4); x1 = ttSHORT(font->data + g1 + 6); y1 = ttSHORT(font->data + g1 + 8);}
    if(ix0) *ix0=has?(int)vfloor(x0*sx+shx):0;
    if(iy0) *iy0=has?(int)vfloor(-y1*sy+shy):0;
    if(ix1) *ix1=has?(int)vceil(x1*sx+shx):0;
    if(iy1) *iy1=has?(int)vceil(-y0*sy+shy):0;
}

typedef struct tt_heapchk{ struct tt_heapchk* next; }tt_heapchk;
typedef struct{ tt_heapchk* head; void* first_free; int remaining; }stbtt__hheap;
static void* _hha(stbtt__hheap* hh,size_t sz) { if(hh->first_free){void*p=hh->first_free;hh->first_free=*(void**)p;return p;} if(!hh->remaining) {int c=sz<32?2000:sz<128?800:100;tt_heapchk*ck=(tt_heapchk*)ttalloc(sizeof(*ck)+sz*c); if(!ck){return NULL;} ck->next=hh->head; hh->head=ck; hh->remaining=c;} --hh->remaining; return(char*)hh->head+sizeof(tt_heapchk)+sz*hh->remaining; }
static void _hhf(stbtt__hheap* hh,void*p) { *(void**)p=hh->first_free;hh->first_free=p; }
typedef struct{ float x0,y0,x1,y1; int invert; }stbtt__edge; typedef struct stbtt__active_edge{ struct stbtt__active_edge*next; float fx,fdx,fdy,direction,sy,ey; }stbtt__active_edge;
static void _hce(float*sl,int x,stbtt__active_edge*e,float x0,float y0,float x1,float y1){
    if(y0==y1||y0>e->ey||y1<e->sy)return;if(y0<e->sy){x0+=(x1-x0)*(e->sy-y0)/(y1-y0);y0=e->sy;}if(y1>e->ey){x1+=(x1-x0)*(e->ey-y1)/(y1-y0);y1=e->ey;}
    if(x0<=x&&x1<=x)sl[x]+=e->direction*(y1-y0);else if(x0>=x+1&&x1>=x+1);else sl[x]+=e->direction*(y1-y0)*(1.0f-((x0-(float)x)+(x1-(float)x))/2.0f);
}

INLINE float _ptz(float h,float t0,float t1,float b0,float b1){ return ((t1-t0)+(b1-b0))/2.0f*h; }
static void _eqs(stbtt__edge*p,int n) {
    while (n > 12) {
        int m = n >> 1;
        int c01 = ((&p[0])->y0 < (&p[m])->y0);
        int c12 = ((&p[m])->y0 < (&p[n - 1])->y0);
        if(c01!=c12){
            int z = (((&p[0])->y0 < (&p[n-1])->y0) == c12) ? 0 : n - 1;
            stbtt__edge t_ = p[z]; p[z] = p[m]; p[m] = t_;;
        }
        stbtt__edge t_ = p[0]; p[0] = p[m]; p[m] = t_;
        int i = 1, j = n - 1;
        for(;;){
            while (((&p[i])->y0 < (&p[0])->y0)) ++i;
            while (((&p[0])->y0 < (&p[j])->y0)) --j;
            if(i>=j)break;
            
            t_ = p[i]; p[i] = p[j]; p[j] = t_;
            ++i;
            --j;
        }
        if(j<n-i) { _eqs(p,j); p += i; n -= i; }
        else      { _eqs(p + i,n - i); n = j;  }
    }
}
static void _add_pt(V2*p,int n,float x,float y){if(p){p[n].x=x;p[n].y=y;}}
static int _tess_c(V2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float fsq,int n) {
    float mx=(x0+2*x1+x2)/4,my=(y0+2*y1+y2)/4,dx=(x0+x2)/2-mx,dy=(y0+y2)/2-my; if(n>16||dx*dx+dy*dy<=fsq){_add_pt(pts,(*np)++,x2,y2);return 1;}
    
    _tess_c(pts,np,x0,y0,(x0+x1)/2,(y0+y1)/2,mx,my,fsq,n+1);
    _tess_c(pts,np,mx,my,(x1+x2)/2,(y1+y2)/2,x2,y2,fsq,n+1);return 1;
}
    
static void _tess_cb(V2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float fsq,int n) {
    float d0=vsqrtf((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)),d1=vsqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)),d2=vsqrtf((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2)),ds=vsqrtf((x3-x0)*(x3-x0)+(y3-y0)*(y3-y0)),ll=d0+d1+d2;
    if(n>16||ll*ll-ds*ds<=fsq){_add_pt(pts,(*np)++,x3,y3);return;}
    
    float x01=(x0+x1)/2,y01=(y0+y1)/2,x12=(x1+x2)/2,y12=(y1+y2)/2,x23=(x2+x3)/2,y23=(y2+y3)/2,xa=(x01+x12)/2,ya=(y01+y12)/2,xb=(x12+x23)/2,yb=(y12+y23)/2,mx=(xa+xb)/2,my=(ya+yb)/2;
    _tess_cb(pts,np,x0,y0,x01,y01,xa,ya,mx,my,fsq,n+1);
    _tess_cb(pts,np,mx,my,xb,yb,x23,y23,x3,y3,fsq,n+1);
}

static void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo* info, u8* out, int ow, int oh, int ostr, float sx, float sy, float shx, float shy, int g) {
    stbtt_vertex *v; int ix0, iy0, nv = stbtt_GetGlyphShape(info, g, &v);
    GetGlyphBitmapBoxSubpixel(info, g, sx, sy, shx, shy, &ix0, &iy0, 0, 0);
    float scale = sx > sy ? sy : sx;
    int wc = 0; int *wl = NULL; V2 *win = NULL;
    float flat = 0.35f / scale; float fsq = flat * flat; int n = 0;
    for (int i = 0; i < nv; ++i) { if (v[i].type == STBTT_vmove) { ++n; } }
    wc = n;
    if (n != 0) {
        wl = (int*)ttalloc(sizeof(int) * (size_t)n); if (!wl) { wc = 0; goto err_flatten; }
        int np = 0;
        for (int pass = 0; pass < 2; ++pass) {
            float x = 0, y = 0;
            int start = 0;
            n = -1;
            if (pass == 1) { win = (V2*)ttalloc((size_t)np * sizeof(V2)); if (!win) goto err_flatten; }
            
            np = 0;
            for (int i = 0; i < nv; ++i) {
                switch (v[i].type) {
                    case STBTT_vmove: if (n >= 0) wl[n] = np - start; start = np; ++n; x = v[i].x; y = v[i].y; _add_pt(win, np++, x, y);               break;
                    case STBTT_vline: x = v[i].x; y = v[i].y; _add_pt(win, np++, x, y);                                                                break;
                    case STBTT_vcurve: _tess_c(win, &np, x, y, v[i].cx, v[i].cy, v[i].x, v[i].y, fsq, 0); x = v[i].x; y = v[i].y;                      break;
                    case STBTT_vcubic: _tess_cb(win, &np, x, y, v[i].cx, v[i].cy, v[i].cx1, v[i].cy1, v[i].x, v[i].y, fsq, 0); x = v[i].x; y = v[i].y; break;
                }
            }
            wl[n] = np - start;
        }
    }
    goto after_flatten;
    err_flatten: ttfree(win); ttfree(wl); wl = NULL; wc = 0; win = NULL; 
    after_flatten:;
    if (win) {
        int inv = 1; float ysi = inv ? -sy : sy; stbtt__edge* e; int n_edges = 0;
        for (int i = 0; i < wc; ++i) { n_edges += wl[i]; }
        e = (stbtt__edge*)ttalloc(sizeof(*e) * ((size_t)n_edges + 1));
        if (e) {
            n_edges = 0; int m = 0;
            for (int i = 0; i < wc; ++i) {
                V2* p = win + m; m += wl[i]; int j = wl[i] - 1;
                for (int k = 0; k < wl[i]; j = k++) {
                    int a = k, b = j; if (p[j].y == p[k].y) continue;
                    
                    e[n_edges].invert = 0;
                    if (inv ? p[j].y > p[k].y : p[j].y < p[k].y) { e[n_edges].invert = 1; a = j; b = k; }
                    e[n_edges].x0 = p[a].x * sx + shx;
                    e[n_edges].y0 = p[a].y * ysi + shy;
                    e[n_edges].x1 = p[b].x * sx + shx;
                    e[n_edges].y1 = p[b].y * ysi + shy;
                    ++n_edges;
                }
            }
            _eqs(e, n_edges);
            for (int i = 1; i < n_edges; ++i) { stbtt__edge t = e[i]; int j = i; while (j > 0 && ((&t)->y0 < (&e[j - 1])->y0)) { e[j] = e[j - 1]; --j; } e[j] = t; }
            e[n_edges].y0=(float)(iy0+oh)+1;
            stbtt__hheap hh={0}; stbtt__active_edge*active=NULL; stbtt__edge*ep=e; float sd[129],*sl=sd,*sl2; int y=iy0,j=0,i;
            if(ow>64)sl=(float*)ttalloc((size_t)(ow*2+1)*sizeof(float));
            if(sl){
                sl2=sl+ow;
                while(j<oh){float syt=(float)y,syb=(float)y+1;stbtt__active_edge**step=&active;
                    mset(sl,0,(size_t)ow*sizeof(sl[0]));mset(sl2,0,((size_t)ow+1)*sizeof(sl[0]));
                    while(*step){stbtt__active_edge*z=*step;if(z->ey<=syt){*step=z->next;z->direction=0;_hhf(&hh,z);}else step=&(*step)->next;}
                    while(ep->y0<=syb){
                        if(ep->y0!=ep->y1){
                            stbtt__active_edge*z=(stbtt__active_edge*)_hha(&hh,sizeof(*z));
                            if(z){float dxdy=(ep->x1-ep->x0)/(ep->y1-ep->y0);z->fdx=dxdy;z->fdy=dxdy?1.0f/dxdy:0;z->fx=ep->x0+dxdy*(syt-ep->y0)-(float)ix0;z->direction=ep->invert?1.0f:-1.0f;z->sy=ep->y0;z->ey=ep->y1;if(j==0&&iy0!=0&&z->ey<syt)z->ey=syt;z->next=active;active=z;}
                        }++ep;
                    }
                    for(stbtt__active_edge*ae=active;ae;ae=ae->next){
                        if(ae->fdx==0){float x0=ae->fx;if(x0<ow){if(x0>=0){_hce(sl,(int)x0,ae,x0,syt,x0,syb);_hce(sl2,(int)x0+1,ae,x0,syt,x0,syb);}else _hce(sl2,0,ae,x0,syt,x0,syb);}}
                        else{float x0=ae->fx,dx=ae->fdx,dy=ae->fdy,xb=x0+dx,xt,xbt,sy0,sy1;
                            if(ae->sy>syt){xt=x0+dx*(ae->sy-syt);sy0=ae->sy;}else{xt=x0;sy0=syt;}
                            if(ae->ey<syb){xbt=x0+dx*(ae->ey-syt);sy1=ae->ey;}else{xbt=xb;sy1=syb;}
                            if(xt>=0&&xbt>=0&&xt<ow&&xbt<ow){
                                if((int)xt==(int)xbt){int x=(int)xt;float h=(sy1-sy0)*ae->direction;sl[x]+=_ptz(h,xt,(float)x+1.0f,xbt,(float)x+1.0f);sl2[x+1]+=h;}
                                else{float yc,yf,stp,sign,area;
                                    if(xt>xbt){float t;sy0=syb-(sy0-syt);sy1=syb-(sy1-syt);t=sy0;sy0=sy1;sy1=t;t=xbt;xbt=xt;xt=t;dx=-dx;dy=-dy;t=x0;x0=xb;xb=t;}
                                    int x1=(int)xt,x2=(int)xbt;yc=syt+dy*((float)(x1+1)-x0);yf=syt+dy*((float)x2-x0);if(yc>syb)yc=syb;sign=ae->direction;area=sign*(yc-sy0);
                                    sl[x1]+=area*((float)(x1+1)-xt)/2;
                                    if(yf>syb){yf=syb;dy=(yf-yc)/((float)x2-(float)(x1+1));}
                                    stp=sign*dy;for(int x=x1+1;x<x2;++x){sl[x]+=area+stp/2;area+=stp;}
                                    sl[x2]+=area+sign*_ptz(sy1-yf,(float)x2,(float)x2+1.0f,xbt,(float)x2+1.0f);sl2[x2+1]+=sign*(sy1-sy0);}
                            }else{for(int x=0;x<ow;++x){float y0=syt,x1f=(float)x,x2f=(float)(x+1),x3=xb,y3=syb;float y1=((float)x-x0)/dx+syt,y2=((float)(x+1)-x0)/dx+syt;
                                if(x0<x1f&&x3>x2f){_hce(sl,x,ae,x0,y0,x1f,y1);_hce(sl,x,ae,x1f,y1,x2f,y2);_hce(sl,x,ae,x2f,y2,x3,y3);}
                                else if(x3<x1f&&x0>x2f){_hce(sl,x,ae,x0,y0,x2f,y2);_hce(sl,x,ae,x2f,y2,x1f,y1);_hce(sl,x,ae,x1f,y1,x3,y3);}
                                else if((x0<x1f&&x3>x1f)||(x3<x1f&&x0>x1f)){_hce(sl,x,ae,x0,y0,x1f,y1);_hce(sl,x,ae,x1f,y1,x3,y3);}
                                else if((x0<x2f&&x3>x2f)||(x3<x2f&&x0>x2f)){_hce(sl,x,ae,x0,y0,x2f,y2);_hce(sl,x,ae,x2f,y2,x3,y3);}
                                else _hce(sl,x,ae,x0,y0,x3,y3);}}
                        }
                    }
                    {float sum=0;for(i=0;i<ow;++i){sum+=sl2[i];float cov=(float)vabs(sl[i]+sum)*255.0f+0.5f;int px=(int)cov;if(px>255)px=255;out[j*ostr+i]=(u8)px;}}
                    step=&active;while(*step){stbtt__active_edge*z=*step;z->fx+=z->fdx;step=&(*step)->next;}++y;++j;}
                for(tt_heapchk*c=hh.head,*hp;c;c=hp){hp=c->next;ttfree(c);}
                if(sl!=sd)ttfree(sl);
            }
            ttfree(e);
        }
        ttfree(wl);
        ttfree(win);
    }
    if (v) ttfree(v);
}

typedef struct {u16 x0,y0,x1,y1; float xoff,yoff,xadvance,xoff2,yoff2;} stbtt_packedchar;
INLINE void _pre(u8 *p, int w, int h, int str, u32 kw, int v) {
    int out = v ? w : h, in = v ? h : w, os = v ? 1 : str, is = v ? str : 1;
    for (int j=0;j<out;++j,p+=os) {
        u8 buf[8] = {0}; int tot = 0;
        for (int i=0;i<in;++i) {
            if (i <= in - (int)kw) { tot += p[i * is] - buf[i & 7]; buf[(i + kw) & 7] = p[i * is]; } else { tot -= buf[i & 7]; }
            p[i * is] = (u8)(tot / kw);
        }
    }
}
// Font Loading System
int numPackedGlyphs=0,numPackedGlyphsStopD=0; extern u32 textVAO,textVBO,textSP;
u32 fontAtlasTex,fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS],fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth=0.0f,fixedNumberAdvanceWidthStopD=0.0f;
static const char* fallbackFontPaths[]={"./Fonts/FreeSerifBold.ttf","./Fonts/cambriab.ttf","./Fonts/NotoSansCJK-Subset.ttf"}, *fontPaths[]={"./Fonts/SystemShockText.ttf","./Fonts/StopD.ttf"};
static stbtt_fontinfo fontInfo[5]; static u8 *fontData[5]; static char uiTextBuffer[T_BUFFER_SIZE];
static u16 cpPackedIndex[2][65536]; // Codepoint -> packed glyph index per atlas; 0 = blank(space)
static const u16 packCodepoints[]={/*1303 codepoints used by Data/text_*.txt and Data/logs_*.txt; generated by Tools/gen_pack_codepoints.py*/
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x0023/*#*/,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,0x0028/*(*/,0x0029/*)*/,0x002A/* * */,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,0x0040/*@*/,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x005B/*[*/,0x005C/*\*/,0x005D/*]*/,0x005E/*^*/,0x005F/*_*/,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,0x0068/*h*/,
    0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00A1/*¡*/,0x00AB/*«*/,0x00B0/*°*/,0x00BA/*º*/,0x00BB/*»*/,0x00BF/*¿*/,
    0x00C0/*À*/,0x00C1/*Á*/,0x00C2/*Â*/,0x00C3/*Ã*/,0x00C4/*Ä*/,0x00C7/*Ç*/,0x00C8/*È*/,0x00C9/*É*/,0x00CA/*Ê*/,0x00CC/*Ì*/,0x00CD/*Í*/,0x00CE/*Î*/,0x00D1/*Ñ*/,0x00D3/*Ó*/,0x00D4/*Ô*/,0x00D5/*Õ*/,0x00D6/*Ö*/,0x00DA/*Ú*/,0x00DC/*Ü*/,0x00DF/*ß*/,0x00E0/*à*/,0x00E1/*á*/,0x00E2/*â*/,0x00E3/*ã*/,
    0x00E4/*ä*/,0x00E7/*ç*/,0x00E8/*è*/,0x00E9/*é*/,0x00EA/*ê*/,0x00EC/*ì*/,0x00ED/*í*/,0x00EE/*î*/,0x00EF/*ï*/,0x00F1/*ñ*/,0x00F2/*ò*/,0x00F3/*ó*/,0x00F4/*ô*/,0x00F5/*õ*/,0x00F6/*ö*/,0x00F9/*ù*/,0x00FA/*ú*/,0x00FB/*û*/,0x00FC/*ü*/,0x0153/*œ*/,0x0401/*Ё*/,0x0410/*А*/,0x0411/*Б*/,0x0412/*В*/,
    0x0413/*Г*/,0x0414/*Д*/,0x0415/*Е*/,0x0416/*Ж*/,0x0417/*З*/,0x0418/*И*/,0x0419/*Й*/,0x041A/*К*/,0x041B/*Л*/,0x041C/*М*/,0x041D/*Н*/,0x041E/*О*/,0x041F/*П*/,0x0420/*Р*/,0x0421/*С*/,0x0422/*Т*/,0x0423/*У*/,0x0424/*Ф*/,0x0425/*Х*/,0x0426/*Ц*/,0x0427/*Ч*/,0x0428/*Ш*/,0x0429/*Щ*/,0x042A/*Ъ*/,
    0x042B/*Ы*/,0x042C/*Ь*/,0x042D/*Э*/,0x042E/*Ю*/,0x042F/*Я*/,0x0430/*а*/,0x0431/*б*/,0x0432/*в*/,0x0433/*г*/,0x0434/*д*/,0x0435/*е*/,0x0436/*ж*/,0x0437/*з*/,0x0438/*и*/,0x0439/*й*/,0x043A/*к*/,0x043B/*л*/,0x043C/*м*/,0x043D/*н*/,0x043E/*о*/,0x043F/*п*/,0x0440/*р*/,0x0441/*с*/,0x0442/*т*/,
    0x0443/*у*/,0x0444/*ф*/,0x0445/*х*/,0x0446/*ц*/,0x0447/*ч*/,0x0448/*ш*/,0x0449/*щ*/,0x044A/*ъ*/,0x044B/*ы*/,0x044C/*ь*/,0x044D/*э*/,0x044E/*ю*/,0x044F/*я*/,0x0451/*ё*/,0x2014/*—*/,0x2019/*’*/,0x2026/*…*/,0x2116/*№*/,0x3001/*、*/,0x3002/*。*/,0x3005/*々*/,0x300C/*「*/,0x300D/*」*/,0x300E/*『*/,
    0x300F/*』*/,0x301C/*〜*/,0x3042/*あ*/,0x3044/*い*/,0x3046/*う*/,0x3048/*え*/,0x304A/*お*/,0x304B/*か*/,0x304C/*が*/,0x304D/*き*/,0x304E/*ぎ*/,0x304F/*く*/,0x3050/*ぐ*/,0x3051/*け*/,0x3052/*げ*/,0x3053/*こ*/,0x3054/*ご*/,0x3055/*さ*/,0x3056/*ざ*/,0x3057/*し*/,0x3058/*じ*/,0x3059/*す*/,0x305A/*ず*/,0x305B/*せ*/,
    0x305C/*ぜ*/,0x305D/*そ*/,0x305E/*ぞ*/,0x305F/*た*/,0x3060/*だ*/,0x3061/*ち*/,0x3063/*っ*/,0x3064/*つ*/,0x3065/*づ*/,0x3066/*て*/,0x3067/*で*/,0x3068/*と*/,0x3069/*ど*/,0x306A/*な*/,0x306B/*に*/,0x306C/*ぬ*/,0x306D/*ね*/,0x306E/*の*/,0x306F/*は*/,0x3070/*ば*/,0x3071/*ぱ*/,0x3072/*ひ*/,0x3073/*び*/,0x3075/*ふ*/,
    0x3076/*ぶ*/,0x3078/*へ*/,0x3079/*べ*/,0x307B/*ほ*/,0x307C/*ぼ*/,0x307E/*ま*/,0x307F/*み*/,0x3080/*む*/,0x3081/*め*/,0x3082/*も*/,0x3083/*ゃ*/,0x3084/*や*/,0x3086/*ゆ*/,0x3087/*ょ*/,0x3088/*よ*/,0x3089/*ら*/,0x308A/*り*/,0x308B/*る*/,0x308C/*れ*/,0x308D/*ろ*/,0x308F/*わ*/,0x3092/*を*/,0x3093/*ん*/,0x30A1/*ァ*/,
    0x30A2/*ア*/,0x30A3/*ィ*/,0x30A4/*イ*/,0x30A6/*ウ*/,0x30A7/*ェ*/,0x30A8/*エ*/,0x30A9/*ォ*/,0x30AA/*オ*/,0x30AB/*カ*/,0x30AC/*ガ*/,0x30AD/*キ*/,0x30AE/*ギ*/,0x30AF/*ク*/,0x30B0/*グ*/,0x30B1/*ケ*/,0x30B2/*ゲ*/,0x30B3/*コ*/,0x30B4/*ゴ*/,0x30B5/*サ*/,0x30B6/*ザ*/,0x30B7/*シ*/,0x30B8/*ジ*/,0x30B9/*ス*/,0x30BA/*ズ*/,
    0x30BB/*セ*/,0x30BC/*ゼ*/,0x30BD/*ソ*/,0x30BE/*ゾ*/,0x30BF/*タ*/,0x30C0/*ダ*/,0x30C1/*チ*/,0x30C3/*ッ*/,0x30C4/*ツ*/,0x30C6/*テ*/,0x30C7/*デ*/,0x30C8/*ト*/,0x30C9/*ド*/,0x30CA/*ナ*/,0x30CB/*ニ*/,0x30CD/*ネ*/,0x30CE/*ノ*/,0x30CF/*ハ*/,0x30D0/*バ*/,0x30D1/*パ*/,0x30D2/*ヒ*/,0x30D3/*ビ*/,0x30D4/*ピ*/,0x30D5/*フ*/,
    0x30D6/*ブ*/,0x30D7/*プ*/,0x30D8/*ヘ*/,0x30D9/*ベ*/,0x30DA/*ペ*/,0x30DB/*ホ*/,0x30DC/*ボ*/,0x30DD/*ポ*/,0x30DE/*マ*/,0x30DF/*ミ*/,0x30E0/*ム*/,0x30E1/*メ*/,0x30E2/*モ*/,0x30E3/*ャ*/,0x30E4/*ヤ*/,0x30E5/*ュ*/,0x30E6/*ユ*/,0x30E7/*ョ*/,0x30E9/*ラ*/,0x30EA/*リ*/,0x30EB/*ル*/,0x30EC/*レ*/,0x30ED/*ロ*/,0x30EF/*ワ*/,
    0x30F3/*ン*/,0x30F4/*ヴ*/,0x30F6/*ヶ*/,0x30FB/*・*/,0x30FC/*ー*/,0x4E00/*一*/,0x4E01/*丁*/,0x4E07/*万*/,0x4E08/*丈*/,0x4E09/*三*/,0x4E0A/*上*/,0x4E0B/*下*/,0x4E0D/*不*/,0x4E0E/*与*/,0x4E18/*丘*/,0x4E21/*両*/,0x4E2D/*中*/,0x4E3B/*主*/,0x4E45/*久*/,0x4E57/*乗*/,0x4E71/*乱*/,0x4E80/*亀*/,0x4E86/*了*/,0x4E88/*予*/,
    0x4E89/*争*/,0x4E8B/*事*/,0x4E8C/*二*/,0x4E95/*井*/,0x4EA4/*交*/,0x4EBA/*人*/,0x4ECA/*今*/,0x4ED5/*仕*/,0x4ED6/*他*/,0x4ED8/*付*/,0x4EE3/*代*/,0x4EE4/*令*/,0x4EE5/*以*/,0x4EEE/*仮*/,0x4EF2/*仲*/,0x4EF6/*件*/,0x4F01/*企*/,0x4F0F/*伏*/,0x4F11/*休*/,0x4F1A/*会*/,0x4F1D/*伝*/,0x4F4D/*位*/,0x4F4E/*低*/,0x4F4F/*住*/,
    0x4F53/*体*/,0x4F55/*何*/,0x4F59/*余*/,0x4F5C/*作*/,0x4F7F/*使*/,0x4F8B/*例*/,0x4F9B/*供*/,0x4F9D/*依*/,0x4FA1/*価*/,0x4FB5/*侵*/,0x4FBF/*便*/,0x4FC2/*係*/,0x4FDD/*保*/,0x4FE1/*信*/,0x4FEE/*修*/,0x4FFA/*俺*/,0x5009/*倉*/,0x500B/*個*/,0x5012/*倒*/,0x5024/*値*/,0x502B/*倫*/,0x505C/*停*/,0x5074/*側*/,0x5099/*備*/,
    0x50B2/*傲*/,0x50B7/*傷*/,0x50BE/*傾*/,0x50CD/*働*/,0x50CF/*像*/,0x512A/*優*/,0x5143/*元*/,0x5145/*充*/,0x5148/*先*/,0x5149/*光*/,0x514D/*免*/,0x5165/*入*/,0x5168/*全*/,0x5175/*兵*/,0x5177/*具*/,0x5185/*内*/,0x518D/*再*/,0x5192/*冒*/,0x5197/*冗*/,0x51B7/*冷*/,0x51CD/*凍*/,0x51E0/*几*/,0x51E6/*処*/,0x51FA/*出*/,
    0x5206/*分*/,0x5207/*切*/,0x5212/*划*/,0x5217/*列*/,0x521D/*初*/,0x5224/*判*/,0x5225/*別*/,0x5229/*利*/,0x5230/*到*/,0x5236/*制*/,0x523A/*刺*/,0x523B/*刻*/,0x524A/*削*/,0x524D/*前*/,0x5264/*剤*/,0x5265/*剥*/,0x526F/*副*/,0x5270/*剰*/,0x5272/*割*/,0x5275/*創*/,0x529B/*力*/,0x529F/*功*/,0x52A0/*加*/,0x52A9/*助*/,
    0x52B4/*労*/,0x52B9/*効*/,0x52D5/*動*/,0x52DD/*勝*/,0x52E2/*勢*/,0x52E4/*勤*/,0x52E7/*勧*/,0x5316/*化*/,0x5317/*北*/,0x533A/*区*/,0x533B/*医*/,0x5341/*十*/,0x5348/*午*/,0x534A/*半*/,0x5354/*協*/,0x5357/*南*/,0x5358/*単*/,0x535A/*博*/,0x5371/*危*/,0x5373/*即*/,0x5374/*却*/,0x539A/*厚*/,0x539F/*原*/,0x53B3/*厳*/,
    0x53BB/*去*/,0x53C2/*参*/,0x53CA/*及*/,0x53CD/*反*/,0x53CE/*収*/,0x53D6/*取*/,0x53D7/*受*/,0x53E3/*口*/,0x53E4/*古*/,0x53E5/*句*/,0x53EB/*叫*/,0x53EC/*召*/,0x53EF/*可*/,0x53F0/*台*/,0x53F3/*右*/,0x53F7/*号*/,0x5404/*各*/,0x5408/*合*/,0x540C/*同*/,0x540D/*名*/,0x5410/*吐*/,0x5411/*向*/,0x5420/*吠*/,0x5426/*否*/,
    0x542B/*含*/,0x5438/*吸*/,0x5439/*吹*/,0x544A/*告*/,0x5468/*周*/,0x5473/*味*/,0x547D/*命*/,0x54C0/*哀*/,0x54C1/*品*/,0x54E1/*員*/,0x5506/*唆*/,0x552F/*唯*/,0x5546/*商*/,0x554F/*問*/,0x5584/*善*/,0x559C/*喜*/,0x5642/*噂*/,0x5668/*器*/,0x5674/*噴*/,0x56DE/*回*/,0x56E0/*因*/,0x56E3/*団*/,0x56F2/*囲*/,0x56F3/*図*/,
    0x56FD/*国*/,0x571F/*土*/,0x5727/*圧*/,0x5728/*在*/,0x5730/*地*/,0x57DF/*域*/,0x57F7/*執*/,0x57F9/*培*/,0x5831/*報*/,0x5834/*場*/,0x585A/*塚*/,0x585E/*塞*/,0x5869/*塩*/,0x586B/*填*/,0x5883/*境*/,0x5893/*墓*/,0x5897/*増*/,0x589C/*墜*/,0x58A8/*墨*/,0x58C1/*壁*/,0x58CA/*壊*/,0x58EB/*士*/,0x58EE/*壮*/,0x58F0/*声*/,
    0x58F2/*売*/,0x5909/*変*/,0x5916/*外*/,0x591A/*多*/,0x591C/*夜*/,0x5927/*大*/,0x5929/*天*/,0x592B/*夫*/,0x592E/*央*/,0x5931/*失*/,0x5947/*奇*/,0x5949/*奉*/,0x5951/*契*/,0x596A/*奪*/,0x5973/*女*/,0x5974/*奴*/,0x5999/*妙*/,0x59A8/*妨*/,0x59CB/*始*/,0x5B50/*子*/,0x5B57/*字*/,0x5B58/*存*/,0x5B66/*学*/,0x5B85/*宅*/,
    0x5B87/*宇*/,0x5B88/*守*/,0x5B89/*安*/,0x5B8C/*完*/,0x5B99/*宙*/,0x5B9A/*定*/,0x5B9F/*実*/,0x5BA4/*室*/,0x5BAE/*宮*/,0x5BB3/*害*/,0x5BB4/*宴*/,0x5BB6/*家*/,0x5BB9/*容*/,0x5BDF/*察*/,0x5BEE/*寮*/,0x5BFE/*対*/,0x5C01/*封*/,0x5C02/*専*/,0x5C04/*射*/,0x5C0B/*尋*/,0x5C0E/*導*/,0x5C0F/*小*/,0x5C11/*少*/,0x5C1A/*尚*/,
    0x5C3D/*尽*/,0x5C40/*局*/,0x5C45/*居*/,0x5C48/*屈*/,0x5C4A/*届*/,0x5C4B/*屋*/,0x5C51/*屑*/,0x5C5E/*属*/,0x5C64/*層*/,0x5DE3/*巣*/,0x5DE5/*工*/,0x5DE6/*左*/,0x5DE8/*巨*/,0x5DFE/*巾*/,0x5E02/*市*/,0x5E0C/*希*/,0x5E1D/*帝*/,0x5E2D/*席*/,0x5E2F/*帯*/,0x5E38/*常*/,0x5E72/*干*/,0x5E74/*年*/,0x5E78/*幸*/,0x5E79/*幹*/,
    0x5E83/*広*/,0x5E8A/*床*/,0x5E95/*底*/,0x5E9C/*府*/,0x5EA6/*度*/,0x5EA7/*座*/,0x5EAB/*庫*/,0x5EC3/*廃*/,0x5ECA/*廊*/,0x5EF6/*延*/,0x5EFA/*建*/,0x5F01/*弁*/,0x5F0F/*式*/,0x5F15/*引*/,0x5F31/*弱*/,0x5F35/*張*/,0x5F37/*強*/,0x5F3E/*弾*/,0x5F53/*当*/,0x5F62/*形*/,0x5F71/*影*/,0x5F79/*役*/,0x5F7C/*彼*/,0x5F81/*征*/,
    0x5F85/*待*/,0x5F8C/*後*/,0x5F92/*徒*/,0x5F93/*従*/,0x5F97/*得*/,0x5FA1/*御*/,0x5FA9/*復*/,0x5FAA/*循*/,0x5FAE/*微*/,0x5FB9/*徹*/,0x5FC3/*心*/,0x5FC5/*必*/,0x5FDC/*応*/,0x5FE0/*忠*/,0x5FF5/*念*/,0x6016/*怖*/,0x601D/*思*/,0x6025/*急*/,0x6027/*性*/,0x602A/*怪*/,0x602F/*怯*/,0x604B/*恋*/,0x6050/*恐*/,0x6069/*恩*/,
    0x6075/*恵*/,0x60A3/*患*/,0x60A9/*悩*/,0x60AA/*悪*/,0x60B2/*悲*/,0x60C5/*情*/,0x60D1/*惑*/,0x60F3/*想*/,0x610F/*意*/,0x611A/*愚*/,0x611B/*愛*/,0x611F/*感*/,0x614B/*態*/,0x6162/*慢*/,0x6163/*慣*/,0x616E/*慮*/,0x6182/*憂*/,0x61A9/*憩*/,0x61C7/*懇*/,0x61F8/*懸*/,0x6210/*成*/,0x6226/*戦*/,0x623B/*戻*/,0x6240/*所*/,
    0x624B/*手*/,0x6253/*打*/,0x6255/*払*/,0x6280/*技*/,0x6291/*抑*/,0x6295/*投*/,0x6297/*抗*/,0x629C/*抜*/,0x629E/*択*/,0x62B1/*抱*/,0x62B5/*抵*/,0x62BC/*押*/,0x62BD/*抽*/,0x62CD/*拍*/,0x62D2/*拒*/,0x62D8/*拘*/,0x62E0/*拠*/,0x62F6/*拶*/,0x62FE/*拾*/,0x6301/*持*/,0x6307/*指*/,0x6311/*挑*/,0x631F/*挟*/,0x6328/*挨*/,
    0x632F/*振*/,0x6355/*捕*/,0x6357/*捗*/,0x635C/*捜*/,0x6368/*捨*/,0x6392/*排*/,0x6398/*掘*/,0x639B/*掛*/,0x63A1/*採*/,0x63A2/*探*/,0x63A5/*接*/,0x63A8/*推*/,0x63D0/*提*/,0x63DB/*換*/,0x63EE/*揮*/,0x640D/*損*/,0x6483/*撃*/,0x64CD/*操*/,0x652F/*支*/,0x6539/*改*/,0x653B/*攻*/,0x653E/*放*/,0x653F/*政*/,0x6545/*故*/,
    0x654F/*敏*/,0x6551/*救*/,0x6557/*敗*/,0x6559/*教*/,0x6563/*散*/,0x6570/*数*/,0x6574/*整*/,0x6575/*敵*/,0x6587/*文*/,0x6599/*料*/,0x65AD/*断*/,0x65B0/*新*/,0x65B9/*方*/,0x65BD/*施*/,0x65E2/*既*/,0x65E5/*日*/,0x65E7/*旧*/,0x65E9/*早*/,0x660E/*明*/,0x660F/*昏*/,0x6613/*易*/,0x661F/*星*/,0x6620/*映*/,0x6628/*昨*/,
    0x6642/*時*/,0x666E/*普*/,0x666F/*景*/,0x6697/*暗*/,0x66B4/*暴*/,0x66DC/*曜*/,0x66F2/*曲*/,0x66F4/*更*/,0x66F8/*書*/,0x66FF/*替*/,0x6700/*最*/,0x6708/*月*/,0x6709/*有*/,0x670D/*服*/,0x671B/*望*/,0x671D/*朝*/,0x671F/*期*/,0x672A/*未*/,0x672B/*末*/,0x672C/*本*/,0x673A/*机*/,0x6750/*材*/,0x675F/*束*/,0x6765/*来*/,
    0x676F/*杯*/,0x6771/*東*/,0x6790/*析*/,0x679C/*果*/,0x67B6/*架*/,0x67D3/*染*/,0x67D4/*柔*/,0x67F1/*柱*/,0x67FB/*査*/,0x6804/*栄*/,0x682A/*株*/,0x683C/*格*/,0x6841/*桁*/,0x6848/*案*/,0x68B0/*械*/,0x68C4/*棄*/,0x68DA/*棚*/,0x68EE/*森*/,0x6905/*椅*/,0x690D/*植*/,0x691C/*検*/,0x696D/*業*/,0x6975/*極*/,0x697D/*楽*/,
    0x69CB/*構*/,0x69D8/*様*/,0x6A19/*標*/,0x6A21/*模*/,0x6A29/*権*/,0x6A2A/*横*/,0x6A4B/*橋*/,0x6A5F/*機*/,0x6B20/*欠*/,0x6B21/*次*/,0x6B53/*歓*/,0x6B62/*止*/,0x6B63/*正*/,0x6B66/*武*/,0x6B69/*歩*/,0x6B6A/*歪*/,0x6B7B/*死*/,0x6B8B/*残*/,0x6B96/*殖*/,0x6BB5/*段*/,0x6BBA/*殺*/,0x6BBF/*殿*/,0x6BD2/*毒*/,0x6C0F/*氏*/,
    0x6C17/*気*/,0x6C38/*永*/,0x6C42/*求*/,0x6C5A/*汚*/,0x6C7A/*決*/,0x6C8C/*沌*/,0x6CB9/*油*/,0x6CBB/*治*/,0x6CC1/*況*/,0x6CD5/*法*/,0x6CE3/*泣*/,0x6CE8/*注*/,0x6D3B/*活*/,0x6D3E/*派*/,0x6D41/*流*/,0x6D44/*浄*/,0x6D6E/*浮*/,0x6D74/*浴*/,0x6D88/*消*/,0x6DF1/*深*/,0x6DF7/*混*/,0x6E09/*渉*/,0x6E21/*渡*/,0x6E2C/*測*/,
    0x6E80/*満*/,0x6E90/*源*/,0x6E96/*準*/,0x6EB6/*溶*/,0x6ED1/*滑*/,0x6EDE/*滞*/,0x6EE1/*满*/,0x6F0F/*漏*/,0x6F14/*演*/,0x6F5C/*潜*/,0x6F64/*潤*/,0x6FC0/*激*/,0x6FE1/*濡*/,0x706B/*火*/,0x706F/*灯*/,0x707D/*災*/,0x7089/*炉*/,0x70B9/*点*/,0x70BA/*為*/,0x7121/*無*/,0x7136/*然*/,0x713C/*焼*/,0x7167/*照*/,0x71B1/*熱*/,
    0x71C3/*燃*/,0x7206/*爆*/,0x722A/*爪*/,0x7247/*片*/,0x7269/*物*/,0x7272/*牲*/,0x7279/*特*/,0x72A0/*犠*/,0x72B6/*状*/,0x72C2/*狂*/,0x72D9/*狙*/,0x72ED/*狭*/,0x7344/*獄*/,0x7372/*獲*/,0x7384/*玄*/,0x7387/*率*/,0x738B/*王*/,0x73AF/*环*/,0x73FE/*現*/,0x7403/*球*/,0x7406/*理*/,0x74A7/*璧*/,0x74B0/*環*/,0x751F/*生*/,
    0x7523/*産*/,0x7528/*用*/,0x7531/*由*/,0x7532/*甲*/,0x753B/*画*/,0x7559/*留*/,0x7565/*略*/,0x756A/*番*/,0x7570/*異*/,0x7591/*疑*/,0x75AB/*疫*/,0x75B2/*疲*/,0x75B9/*疹*/,0x75BE/*疾*/,0x75C5/*病*/,0x75C7/*症*/,0x75D2/*痒*/,0x75D5/*痕*/,0x75DB/*痛*/,0x75FA/*痺*/,0x760D/*瘍*/,0x7642/*療*/,0x7652/*癒*/,0x767A/*発*/,
    0x767B/*登*/,0x767D/*白*/,0x7684/*的*/,0x7686/*皆*/,0x76AE/*皮*/,0x76CA/*益*/,0x76D7/*盗*/,0x76E3/*監*/,0x76EE/*目*/,0x76F4/*直*/,0x76F8/*相*/,0x76FE/*盾*/,0x7720/*眠*/,0x7740/*着*/,0x7761/*睡*/,0x77E5/*知*/,0x77ED/*短*/,0x77F3/*石*/,0x7814/*研*/,0x7815/*砕*/,0x7832/*砲*/,0x7834/*破*/,0x78BA/*確*/,0x78C1/*磁*/,
    0x78E8/*磨*/,0x793A/*示*/,0x793C/*礼*/,0x793E/*社*/,0x7948/*祈*/,0x795D/*祝*/,0x795E/*神*/,0x7981/*禁*/,0x79C1/*私*/,0x79D1/*科*/,0x79D2/*秒*/,0x79D8/*秘*/,0x79F0/*称*/,0x79FB/*移*/,0x7A0B/*程*/,0x7A0E/*税*/,0x7A3C/*稼*/,0x7A4D/*積*/,0x7A4F/*穏*/,0x7A74/*穴*/,0x7A76/*究*/,0x7A7A/*空*/,0x7A81/*突*/,0x7A93/*窓*/,
    0x7ACB/*立*/,0x7AEF/*端*/,0x7AF6/*競*/,0x7B2C/*第*/,0x7B54/*答*/,0x7BA1/*管*/,0x7BB1/*箱*/,0x7BC0/*節*/,0x7BC4/*範*/,0x7BC9/*築*/,0x7C21/*簡*/,0x7C89/*粉*/,0x7C98/*粘*/,0x7CBE/*精*/,0x7CFB/*系*/,0x7D04/*約*/,0x7D0D/*納*/,0x7D19/*紙*/,0x7D1A/*級*/,0x7D20/*素*/,0x7D22/*索*/,0x7D30/*細*/,0x7D42/*終*/,0x7D44/*組*/,
    0x7D4C/*経*/,0x7D50/*結*/,0x7D61/*絡*/,0x7D66/*給*/,0x7D76/*絶*/,0x7D9A/*続*/,0x7DAD/*維*/,0x7DCA/*緊*/,0x7DD2/*緒*/,0x7DDA/*線*/,0x7DE0/*締*/,0x7DE8/*編*/,0x7E41/*繁*/,0x7E54/*織*/,0x7F60/*罠*/,0x7F6E/*置*/,0x7F8E/*美*/,0x7FA4/*群*/,0x7FA9/*義*/,0x7FD2/*習*/,0x7FFC/*翼*/,0x8003/*考*/,0x8005/*者*/,0x8010/*耐*/,
    0x8056/*聖*/,0x805E/*聞*/,0x8074/*聴*/,0x8077/*職*/,0x80A2/*肢*/,0x80B2/*育*/,0x80CC/*背*/,0x80FD/*能*/,0x8131/*脱*/,0x8133/*脳*/,0x8155/*腕*/,0x816B/*腫*/,0x8170/*腰*/,0x8179/*腹*/,0x81BF/*膿*/,0x81E8/*臨*/,0x81EA/*自*/,0x81F4/*致*/,0x8208/*興*/,0x820C/*舌*/,0x821E/*舞*/,0x822C/*般*/,0x826F/*良*/,0x82BD/*芽*/,
    0x8349/*草*/,0x8377/*荷*/,0x843D/*落*/,0x8449/*葉*/,0x84B8/*蒸*/,0x84C4/*蓄*/,0x84CB/*蓋*/,0x8513/*蔓*/,0x8535/*蔵*/,0x853D/*蔽*/,0x85AC/*薬*/,0x8650/*虐*/,0x866B/*虫*/,0x86CD/*蛍*/,0x8840/*血*/,0x884C/*行*/,0x8853/*術*/,0x885B/*衛*/,0x885D/*衝*/,0x8868/*表*/,0x88AB/*被*/,0x88C2/*裂*/,0x88C5/*装*/,0x88CF/*裏*/,
    0x88DC/*補*/,0x88FD/*製*/,0x8907/*複*/,0x8972/*襲*/,0x897F/*西*/,0x8981/*要*/,0x8986/*覆*/,0x898B/*見*/,0x898F/*規*/,0x8996/*視*/,0x899A/*覚*/,0x89A7/*覧*/,0x89B3/*観*/,0x89C4/*规*/,0x89E3/*解*/,0x89E6/*触*/,0x8A00/*言*/,0x8A08/*計*/,0x8A17/*託*/,0x8A18/*記*/,0x8A2A/*訪*/,0x8A2D/*設*/,0x8A31/*許*/,0x8A3A/*診*/,
    0x8A3C/*証*/,0x8A55/*評*/,0x8A66/*試*/,0x8A70/*詰*/,0x8A71/*話*/,0x8A73/*詳*/,0x8A87/*誇*/,0x8A8D/*認*/,0x8A93/*誓*/,0x8A98/*誘*/,0x8A9E/*語*/,0x8AAC/*説*/,0x8AAD/*読*/,0x8AB0/*誰*/,0x8AB2/*課*/,0x8ABF/*調*/,0x8AC7/*談*/,0x8ACB/*請*/,0x8AD6/*論*/,0x8B0E/*謎*/,0x8B58/*識*/,0x8B66/*警*/,0x8B70/*議*/,0x8B77/*護*/,
    0x8B90/*讐*/,0x8C4A/*豊*/,0x8C61/*象*/,0x8CA0/*負*/,0x8CA7/*貧*/,0x8CAF/*貯*/,0x8CB4/*貴*/,0x8CBB/*費*/,0x8CC7/*資*/,0x8CEA/*質*/,0x8D64/*赤*/,0x8D70/*走*/,0x8D77/*起*/,0x8D8A/*越*/,0x8DB3/*足*/,0x8DDD/*距*/,0x8DE1/*跡*/,0x8DEF/*路*/,0x8EAB/*身*/,0x8ECA/*車*/,0x8ECC/*軌*/,0x8ECD/*軍*/,0x8EE2/*転*/,0x8EFD/*軽*/,
    0x8F2A/*輪*/,0x8F38/*輸*/,0x8FBC/*込*/,0x8FCE/*迎*/,0x8FD1/*近*/,0x8FF7/*迷*/,0x8FFD/*追*/,0x9000/*退*/,0x9001/*送*/,0x9003/*逃*/,0x900F/*透*/,0x9014/*途*/,0x9019/*這*/,0x901A/*通*/,0x901F/*速*/,0x9020/*造*/,0x9023/*連*/,0x902E/*逮*/,0x9031/*週*/,0x9032/*進*/,0x9042/*遂*/,0x9045/*遅*/,0x904A/*遊*/,0x904B/*運*/,
    0x904E/*過*/,0x9053/*道*/,0x9054/*達*/,0x9055/*違*/,0x9060/*遠*/,0x9063/*遣*/,0x9069/*適*/,0x906D/*遭*/,0x9078/*選*/,0x907A/*遺*/,0x907F/*避*/,0x90AA/*邪*/,0x90CE/*郎*/,0x90E8/*部*/,0x90FD/*都*/,0x914D/*配*/,0x9154/*酔*/,0x916C/*酬*/,0x91CD/*重*/,0x91CE/*野*/,0x91CF/*量*/,0x91D1/*金*/,0x9244/*鉄*/,0x925B/*鉛*/,
    0x9280/*銀*/,0x9285/*銅*/,0x92FC/*鋼*/,0x9320/*錠*/,0x9332/*録*/,0x9375/*鍵*/,0x9396/*鎖*/,0x9577/*長*/,0x9580/*門*/,0x9589/*閉*/,0x958B/*開*/,0x9593/*間*/,0x95A2/*関*/,0x95C7/*闇*/,0x95D8/*闘*/,0x9632/*防*/,0x963B/*阻*/,0x964D/*降*/,0x9650/*限*/,0x9662/*院*/,0x9664/*除*/,0x9678/*陸*/,0x967A/*険*/,0x9685/*隅*/,
    0x968A/*隊*/,0x968E/*階*/,0x969B/*際*/,0x969C/*障*/,0x96A0/*隠*/,0x96B7/*隷*/,0x96C6/*集*/,0x96C7/*雇*/,0x96D1/*雑*/,0x96E2/*離*/,0x96E3/*難*/,0x96F7/*雷*/,0x96FB/*電*/,0x9727/*霧*/,0x9759/*静*/,0x975E/*非*/,0x9762/*面*/,0x97F3/*音*/,0x97FF/*響*/,0x9803/*頃*/,0x9805/*項*/,0x9806/*順*/,0x9810/*預*/,0x9811/*頑*/,
    0x9818/*領*/,0x982D/*頭*/,0x983C/*頼*/,0x984C/*題*/,0x9858/*願*/,0x985E/*類*/,0x98DB/*飛*/,0x98DF/*食*/,0x98F2/*飲*/,0x990A/*養*/,0x9928/*館*/,0x9952/*饒*/,0x9996/*首*/,0x99AC/*馬*/,0x99C5/*駅*/,0x9A13/*験*/,0x9AA8/*骨*/,0x9AD8/*高*/,0x9B42/*魂*/,0x9B54/*魔*/,0x9CF4/*鳴*/,0x9E7F/*鹿*/,0x9EBB/*麻*/,0x9ED2/*黒*/,
    0xFF01/*！*/,0xFF05/*％*/,0xFF08/*（*/,0xFF09/*）*/,0xFF1A/*：*/,0xFF1B/*；*/,0xFF1F/*？*/
};
i32 numPackCodepoints=sizeof(packCodepoints)/sizeof(packCodepoints[0]);
static i32 CodepointToPackedIndex(i32 cp,int fontID){ return ((u32)cp<65536u)?cpPackedIndex[fontID][cp]:0; }
void LoadFallbackFont(const char*path,int fii,int ci){
    FHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
    int off=_font_offset((u8*)fontData[fii],ci); if(off<0){DualLogError("Invalid collection index %d for font %s\n",ci,path);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[fii],fontData[fii],off)){DualLogError("Failed to init font at index %d in %s\n",ci,path);OS_Exit(1);}
}

void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, u8* bmp);
u8 *fontBmp1, *fontBmp2;
void BuildAtlasCPU(int* numPacked, stbtt_packedchar* packedChars, float* fixedNumAdv, float baseH, int fontIdx, u8 fontID, u8* bmp, u32 sz) { /*no GL*/
    const int S=(int)sz, lim=S-16; int cx=0,cy=0,cb=0; /*shelf packer: pen x, row top y, row bottom y; every rect carries 16px padding*/
    mset(bmp,0,(size_t)S*S); *numPacked=0;
    for(int i=0; i<numPackCodepoints; ++i) {
        if(*numPacked >= MAX_GLYPHS) break;
        u32 cp = packCodepoints[i]; int fi = fontIdx, g = stbtt_FindGlyphIndex(&fontInfo[fi], cp);
        if(!g) for(fi=2; fi<5 && !(g = stbtt_FindGlyphIndex(&fontInfo[fi], cp)); ++fi) {} /*fallbacks live in fontInfo[2..4]*/
        if(!g) continue;
        const stbtt_fontinfo* font = &fontInfo[fi]; const int fb = fi != fontIdx;
        float h = fb ? baseH*1.2f : baseH; int os = fb ? 2 : 3; /* Lower supersampling for fallback glyphs: they are the bulk of the atlas and 2x still keeps CJK legible at these sizes. */
        float sc = h/(float)(ttSHORT(font->data+font->hhea+4)-ttSHORT(font->data+font->hhea+6)), sco = sc*os, rc = 1.0f/os, sh = -(float)(os-1)/(2.0f*(float)os);
        stbtt_packedchar* bc = &packedChars[*numPacked]; bc->x0=bc->y0=bc->x1=bc->y1=0;
        int bx0,by0,bx1,by1; GetGlyphBitmapBoxSubpixel(font,g,sco,sco,0,0,&bx0,&by0,&bx1,&by1);
        int rw = bx1-bx0+os-1, rh = by1-by0+os-1;
        if(cx+rw+16 > lim){cx=0;cy=cb;}
        if(cy+rh+16 <= lim) {
            int rx=cx+16, ry=cy+16, nhm=ttUSHORT(font->data+font->hhea+34); u8* dst=bmp+rx+ry*S;
            cx+=rw+16; if(cy+rh+16 > cb) cb=cy+rh+16;
            stbtt_MakeGlyphBitmapSubpixel(font,dst,bx1-bx0,by1-by0,S,sco,sco,0,0,g);
            _pre(dst,rw,rh,S,os,0); _pre(dst,rw,rh,S,os,1);
            bc->x0=rx; bc->y0=ry; bc->x1=rx+rw; bc->y1=ry+rh;
            bc->xadvance = sc*ttSHORT(font->data+font->hmtx+4*(g<nhm?g:nhm-1));
            bc->xoff=bx0*rc+sh; bc->yoff=by0*rc+sh; bc->xoff2=(bx0+rw)*rc+sh; bc->yoff2=(by0+rh)*rc+sh;
        }
        int idx=(*numPacked)++; cpPackedIndex[fontID][cp]=(u16)idx; if(cp>='0' && cp<='9') *fixedNumAdv=vmax(*fixedNumAdv,bc->xadvance);
    }
}

void DumpFontAtlasBmp(const char* filename, u8* atlas, int size) { size_t n = (size_t)size * size; u32* rgba = OS_Alloc(n * 4);  for (int row = 0; row < size; ++row) { const u8* srcRow = atlas + row * size; u32* dstRow = rgba + (size - 1 - row) * size; for (int col = 0; col < size; ++col) { u8 v = srcRow[col]; dstRow[col] = v | (v<<8) | (v<<16) | (v<<24); } } BmpWrite(filename, size, size, rgba); OS_Free(rgba, n * 4); }
static int CountBlankGlyphs(const stbtt_packedchar* chars, int n) { int b=0; for(int i=0;i<n;++i) if(!chars[i].x1 && !chars[i].y1) ++b; return b; }
void InitFontAtlasses() {
    DebugRAM("start font load");
    double t0=get_time();DualLog("Loading    fonts (   5) ...");
    ttAllocs = OS_AllocScratch(4674 * sizeof(TAlloc));
    FHandle fd1,fd2;int sz1,sz2;
    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if(!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)){DualLogError("%s font init failed\n",fontPaths[0]); OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)){DualLogError("%s font init failed\n",fontPaths[1]); OS_Exit(1);}
    LoadFallbackFont(fallbackFontPaths[0],2,0);
    LoadFallbackFont(fallbackFontPaths[1],3,0);
    LoadFallbackFont(fallbackFontPaths[2],4,0);
    u8* bmp = fontBmp1 = OS_AllocScratch(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    BuildAtlasCPU(&numPackedGlyphs,fontPackedChar,&fixedNumberAdvanceWidth,20.0f,0,FONT_NORMAL,bmp,FONT_ATLAS_SIZE);
    DumpFontAtlasBmp("Screenshots/atlas_normal.bmp",bmp,FONT_ATLAS_SIZE);
    u8* bmp2 = fontBmp2 = OS_AllocScratch(FONT_ATLAS_SIZE2*FONT_ATLAS_SIZE2);
    BuildAtlasCPU(&numPackedGlyphsStopD,fontPackedCharStopD,&fixedNumberAdvanceWidthStopD,54.0f,1,FONT_STOPD,bmp2,FONT_ATLAS_SIZE2);
    DumpFontAtlasBmp("Screenshots/atlas_stopd.bmp",bmp2,FONT_ATLAS_SIZE2);
    DualLog("(CPU, %d/%d glyphs packed, blanks %d/%d)...........took %.6f secs\n",numPackedGlyphs,numPackedGlyphsStopD,CountBlankGlyphs(fontPackedChar,numPackedGlyphs),CountBlankGlyphs(fontPackedCharStopD,numPackedGlyphsStopD),get_time() - t0);
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
    FHandle dfd = INVALID_FHANDLE; int asz = 0; u8* data = OS_OpenAndAllocateFileBufferReadonly(path, &dfd, &asz); if(!data || asz <= 0) { DualLogError("Failed to load text file: %s\n", path); *out_size = 0; return NULL; } *out_size = (size_t)asz; *out_dp = 0; *out_utf16 = 0;
    if(asz >= 2 && data[0]==0xFF && data[1]==0xFE) { *out_dp = 2; *out_utf16 = 1; } else if(asz >= 3 && data[0]==0xEF && data[1]==0xBB && data[2]==0xBF) { *out_dp = 3; } else { int nl=0; for(size_t i=1; i<(size_t)asz && i<1024; i+=2) if(data[i]==0) nl++; if(nl*3 > asz) *out_utf16 = 1; } return data;
}

void LoadTextForLanguage(u8 lang) {
    char tf[256]={0}; sCpy2aSubFromb(tf,255,localizations[lang<8?lang:0],256);
    char line[T_LOGSTR_MAX]; size_t dp=0; int utf16=0,ln=0; Sys_Text.file_data = LoadTextFile(tf,&Sys_Text.file_size,&dp,&utf16);    
    while(dp<Sys_Text.file_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.file_size){u16 ch=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.file_size){u16 nx=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}} else{while(dp<Sys_Text.file_size){u8 c=Sys_Text.file_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.file_size&&Sys_Text.file_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
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
                default: if(li>=0&&li<LOGCNT){char*d=World.audioLogSpeech2Text[li]; size_t cur=slen(d); if(cur>0&&cur<T_LOGSTR_MAX-2){d[cur++]=',';d[cur]='\0';} size_t left=T_LOGSTR_MAX-cur-1; if(left>0){size_t cl=tl>left?left:tl;sCpy2aSubFromb(d+cur,cl,st,left+1);}}break;}
            if (*pos==',')++pos;fi++;}
        if (li>=0&&li<LOGCNT) {Sys_Text.audioLogImagesRefIndicesLH[li]=(u16)ilh;Sys_Text.audioLogImagesRefIndicesRH[li]=(u16)irh;Sys_Text.audioLogType[li]=(u8)lt;Sys_Text.audioLogLevelFound[li]=(u8)lf;}
        nxt:continue;}
}

static float textVertexData[TEXT_VTX_FLOATS]; extern Color textColors[]; enum {TALIGN_LEFT=0,TALIGN_CENTER=1,TALIGN_RIGHT=2};
float MeasureLineAdvance(const char* p, u8 fontID) {
    float w=0; int cc=0;
    while (*p) {
        const u8*s=(const u8*)p; u32 cp=0;
        if (*s<0x80) { cp=*s++; }
        else if ((*s&0xE0)==0xC0) { if (!s[1]) break; cp=(*s&0x1F)<< 6; cp|=(s[1]&0x3F); s+=2; }
        else if ((*s&0xF0)==0xE0) { if (!s[1] || !s[2]) break; cp=(*s&0x0F)<<12; cp|=(s[1]&0x3F)<<6; cp|=(s[2]&0x3F); s+=3; }
        else if ((*s&0xF8)==0xF0) { if (!s[1] || !s[2] || !s[3]) break; cp=(*s&0x07)<<18; cp|=(s[1]&0x3F)<<12; cp|=(s[2]&0x3F)<<6; cp|=(s[3]&0x3F); s+=4; }
        else s++;
        p=(const char*)s;
        cc++; if (cp=='\n'||cc>120) break;
        const stbtt_packedchar *b = ((fontID==FONT_STOPD) ? fontPackedCharStopD : fontPackedChar) + CodepointToPackedIndex(cp,fontID); if (cp >= '0' && cp <= '9' && fontID == FONT_STOPD) w = vfloor((w + b->xoff) + 0.5f) + fixedNumberAdvanceWidthStopD; else w += b->xadvance;
    } return w;
}

void RenderFormattedText(i16 x, i16 y, u32 color, u8 fontID, float scale, u8 align, const char* restrict format, va_list args) {
    va_list c; __builtin_va_copy(c,args); sFormatV(uiTextBuffer,T_BUFFER_SIZE,format,c); __builtin_va_end(c); glUseProgram(textSP); glEnable(GL_BLEND); glUniform4f(3,textColors[color].r,textColors[color].g,textColors[color].b,1.0f); glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D,fontID==FONT_STOPD ? fontAtlasTexStopD : fontAtlasTex); float invatsz = 1.0f/(fontID==FONT_STOPD ? (float)FONT_ATLAS_SIZE2 : (float)FONT_ATLAS_SIZE);
    glUniform2f(4,invatsz,invatsz); glUniform1ui(2,fontID); glBindVertexArray(textVAO); float xUsed=x, yUsed=y; if (scale < 1.0f) { xUsed *= (1/scale); yUsed *= (1/scale); } float alignMul = (align==TALIGN_CENTER) ? 0.5f : (align==TALIGN_RIGHT) ? 1.0f : 0.0f; /*pen starts this far (as a fraction of line width) left of xUsed*/
    size_t vc=0; const char*p=uiTextBuffer; float xpos=(alignMul ? xUsed - MeasureLineAdvance(p,fontID)*alignMul : xUsed), ypos=yUsed+(16*scale),ls=22*scale; int cc=0; float puv = 10.0f * invatsz, bw=2.0f; 
    while(*p) {
        const u8*s=(const u8*)p; u32 cp=0; if (*s<0x80) { cp=*s++; }else if ((*s&0xE0)==0xC0) { if (!s[1]) break; cp=(*s&0x1F)<< 6; cp|=(s[1]&0x3F); s+=2; }else if ((*s&0xF0)==0xE0) { if (!s[1] || !s[2]) break; cp=(*s&0x0F)<<12; cp|=(s[1]&0x3F)<<6; cp|=(s[2]&0x3F); s+=3; }else if ((*s&0xF8)==0xF0) { if (!s[1] || !s[2] || !s[3]) break; cp=(*s&0x07)<<18; cp|=(s[1]&0x3F)<<12; cp|=(s[2]&0x3F)<<6; cp|=(s[3]&0x3F); s+=4; }else s++;
        p = (const char*)s; cc++; if (cp=='\n'||cc>120) { xpos=(alignMul ? xUsed - MeasureLineAdvance(p,fontID)*alignMul : xUsed); ypos+=ls; cc=0; continue; } int idx=CodepointToPackedIndex(cp,fontID); const stbtt_packedchar *b = ((fontID==FONT_STOPD) ? fontPackedCharStopD : fontPackedChar) + idx; float qx0=vfloor((xpos+b->xoff)+0.5f),qy0=vfloor((ypos+b->yoff)+0.5f); float qs0=b->x0*invatsz, qt0=b->y0*invatsz, qs1=b->x1*invatsz, qt1=b->y1*invatsz;
        float vx0 = qx0*scale - bw, vy0 = qy0*scale - bw, vx1 = (qx0 + b->xoff2 - b->xoff)*scale + bw, vy1 = (qy0 + b->yoff2 - b->yoff)*scale + bw; float s0 = qs0 - puv, t0 = qt0 - puv, s1 = qs1 + puv, t1 = qt1 + puv, z = 0.0f; float tv[30] = { vx0,vy0,z,s0,t0, vx1,vy1,z,s1,t1, vx1,vy0,z,s1,t0, vx0,vy0,z,s0,t0, vx0,vy1,z,s0,t1, vx1,vy1,z,s1,t1 }; if (vc >= TEXT_VTX_FLOATS/30) break; mcpy(textVertexData + vc * 30,tv,sizeof(tv)); vc++;
        if (cp >= '0' && cp <= '9' && fontID == FONT_STOPD){xpos = qx0 + fixedNumberAdvanceWidthStopD;}else xpos += b->xadvance;
    } if (vc) { glBindBuffer(GL_ARRAY_BUFFER,textVBO); glBufferSubData(GL_ARRAY_BUFFER,0,vc*30*sizeof(float),textVertexData); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,vc*6); }
}

void RenderTextL(i16 x, i16 y, u32 color, u8 f, float scale, const char* restrict s,...) { va_list a; __builtin_va_start(a,s); RenderFormattedText(x,y,color,f,scale,TALIGN_LEFT,s,a); __builtin_va_end(a); }
void RenderTextC(i16 x, i16 y, u32 color, u8 f, float scale, const char* restrict s,...) { va_list a; __builtin_va_start(a,s); RenderFormattedText(x,y,color,f,scale,TALIGN_CENTER,s,a); __builtin_va_end(a); }
void RenderText3DC(V3 worldPos, u32 color, u8 f, float scale, const char* restrict s, ...) { va_list a; __builtin_va_start(a,s); RenderFormattedText((i16)worldPos.x,(i16)worldPos.y,color,f,scale,TALIGN_CENTER,s,a); __builtin_va_end(a); }
static u16 F32ToHalf(float f){u32 x; mcpy(&x,&f,4); u32 sign=(x>>16)&0x8000u,exp=(x>>23)&0xFFu, mant=x&0x7FFFFFu; if(exp==0xFFu)return (u16)(sign|0x7C00u|(mant?0x200u:0)); int e=(int)exp-127+15; if (e>=31)return (u16)(sign|0x7C00u); if(e<=0){if(e<-10){return (u16)sign;} mant|=0x800000u; u32 shift=(u32)(14-e); u32 h=mant>>shift; if((mant>>(shift-1))&1u)h++; return (u16)(sign|h);} u32 h=((u32)e<<10)|(mant>>13); if(mant&0x1000u)h++; return (u16)(sign|h);}
void BuildTextDecalMeshes(void) {
    mset(textDecalVBO,0,sizeof(textDecalVBO)); mset(textDecalVertexCount,0,sizeof(textDecalVertexCount)); const float DECAL_SCALE_NORMAL = 5.0f*0.25f/0.7f, DECAL_SCALE_STOPD = 2.5f*9.0f/11.5f, DECAL_PX_NORMAL = 0.2f/20.0f, DECAL_PX_STOPD = 9.0f/54.0f; const float DECAL_PX_N = DECAL_SCALE_NORMAL*DECAL_PX_NORMAL, DECAL_PX_S = DECAL_SCALE_STOPD*DECAL_PX_STOPD;
    for (u8 lev=0; lev<World.numLevels; ++lev) {
        for (u16 i=INSTS_1ST_IDX; i<World.levelInstCount[lev]; ++i) {
            Entity* e = &World.levelInstances[lev][i]; if (e->index!=592 && e->index!=593) continue; u8 fontID = (e->index==593) ? FONT_STOPD : FONT_NORMAL; const char* str = NULL; for (u16 k=0;k<decalInlineTextCount;++k) { if (decalInlineTextLevel[k]==lev && decalInlineTextInst[k]==i) { str=decalInlineText[k]; break; } } // literal level file text for decals with no (or invalid) lingdex
            if (!str) { u16 li=e->messageLingdex; str=(li>0 && li<T_LOGSTR_CNT) ? Sys_Text.stringTable[li] : NULL; } if (!str || !str[0]) continue; float invatsz = 1.0f/(fontID==FONT_STOPD ? (float)FONT_ATLAS_SIZE2 : (float)FONT_ATLAS_SIZE); float puv=10.0f*invatsz, bw=2.0f; float decalPx = (fontID==FONT_STOPD) ? DECAL_PX_S : DECAL_PX_N; u16 halfVerts[720*8]; u32 vIdx=0;
            const int dix=(fontID==FONT_STOPD) ? 1 : 0;  const float DECAL_ASC_PX[2]={15.0f,45.6f},DECAL_DESC_PX[2]={5.0f,8.4f}, DECAL_LINEH_PX[2]={21.8f,54.0f}; u8 anchor=0,algn=0; float lineSp=1.0f;
            for (u16 k=0;k<decalStyleCount;++k){ if (decalStyles[k].level==lev && decalStyles[k].inst==i){ anchor=decalStyles[k].anchor; algn=decalStyles[k].align; lineSp=decalStyles[k].lineSp; break; } } float blockW=MeasureLineAdvance(str,fontID); int nLines=1; for (const char* q=str; *q; ++q){ if (*q=='\n'){ ++nLines; float lw=MeasureLineAdvance(q+1,fontID); if (lw>blockW) blockW=lw; } }
            const float blockH=(nLines-1)*DECAL_LINEH_PX[dix]*lineSp + DECAL_ASC_PX[dix] + DECAL_DESC_PX[dix]; const u8 arow=anchor/3, acol=anchor%3;/*TextAnchor rows Upper/Middle/Lower, cols Left/Center/Right*/ const float xLeft=-(float)acol*0.5f*blockW, yTop=-(float)arow*0.5f*blockH, lineH=DECAL_LINEH_PX[dix]*lineSp; const float alignMul=(algn==2)?1.0f:((algn==1)?0.5f:0.0f);
            const char* p = str; float xpos=xLeft+alignMul*(blockW-MeasureLineAdvance(p,fontID)), ypos=yTop+DECAL_ASC_PX[dix]; int cc=0; float x0=0,y0=0,x1=0,y1=0,s0=0,t0=0,s1=0,t1=0;
            while(*p) {
                const u8* s=(const u8*)p; u32 cp=0; if (*s<0x80){cp=*s++;}else if ((*s&0xE0)==0xC0){ if(!s[1])break; cp=(*s&0x1F)<<6; cp|=(s[1]&0x3F); s+=2; }else if ((*s&0xF0)==0xE0){ if(!s[1]||!s[2])break; cp=(*s&0x0F)<<12; cp|=(s[1]&0x3F)<<6; cp|=(s[2]&0x3F); s+=3; }else if ((*s&0xF8)==0xF0){ if(!s[1]||!s[2]||!s[3])break; cp=(*s&0x07)<<18; cp|=(s[1]&0x3F)<<12; cp|=(s[2]&0x3F)<<6; cp|=(s[3]&0x3F); s+=4; }else s++;
                p=(const char*)s; cc++; if (cp=='\n'||cc>120){ xpos=xLeft+alignMul*(blockW-MeasureLineAdvance(p,fontID)); ypos+=lineH; cc=0; continue; } int idx=CodepointToPackedIndex((i32)cp,fontID); const stbtt_packedchar* b = ((fontID==FONT_STOPD)?fontPackedCharStopD:fontPackedChar)+idx; float qx0=vfloor((xpos+b->xoff)+0.5f), qy0=vfloor((ypos+b->yoff)+0.5f); float qs0=b->x0*invatsz, qt0=b->y0*invatsz, qs1=b->x1*invatsz, qt1=b->y1*invatsz;
                x0=qx0-bw; x1=(qx0+b->xoff2-b->xoff)+bw; y0=qy0-bw; y1=(qy0+b->yoff2-b->yoff)+bw; s0=qs0-puv; t0=qt0-puv; s1=qs1+puv; t1=qt1+puv;/*6 verts (2 tris) per glyph; local space only, live model matrix applies rotation/scale/position*/ const float L[6][5] = {{x0,y0,s0,t0},{x1,y1,s1,t1},{x1,y0,s1,t0},{x0,y0,s0,t0},{x0,y1,s0,t1},{x1,y1,s1,t1}};
                for(int k=0;k<6;k++){float lx=L[k][0]*decalPx,ly=-L[k][1]*decalPx,lz=0;/*negate y: glyph quads are y-down, world +Y is up*/ halfVerts[vIdx++]=F32ToHalf(lx); halfVerts[vIdx++]=F32ToHalf(ly); halfVerts[vIdx++]=F32ToHalf(lz); halfVerts[vIdx++]=F32ToHalf(0.0f); halfVerts[vIdx++]=F32ToHalf(0.0f); halfVerts[vIdx++]=F32ToHalf(-1.0f); halfVerts[vIdx++]=F32ToHalf(L[k][2]); halfVerts[vIdx++]=F32ToHalf(L[k][3]);}
                if (vIdx+8 > 720*8) break; if (cp>='0'&&cp<='9'&&fontID==FONT_STOPD){ xpos=qx0+fixedNumberAdvanceWidthStopD; }else xpos += b->xadvance;
            }
            u32 vertCount = vIdx/8; if (vertCount==0) { textDecalVBO[lev][i]=0; textDecalVertexCount[lev][i]=0; continue; } if (textDecalVBO[lev][i]==0) glGenBuffers(1,&textDecalVBO[lev][i]); glBindBuffer(GL_ARRAY_BUFFER,textDecalVBO[lev][i]); glBufferData(GL_ARRAY_BUFFER,vertCount*16,(const void*)halfVerts,GL_STATIC_DRAW); glBindBuffer(GL_ARRAY_BUFFER,0); textDecalVertexCount[lev][i]=vertCount;
        }
    }
}
