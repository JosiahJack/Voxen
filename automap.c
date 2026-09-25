//automap.c - CPU automap, side MFD, 320x200
static u8 amPx[AM_W*AM_H*4],amFullPx[AM_FULL_W*AM_FULL_H*4],amBuiltLev=255,amBuiltZoom=255,amMdlState[MAX_MDLS]/*0=unknown,1=ok,2=no mesh*/,amMdlWall[MAX_MDLS]/*1=wall-like (draw), 0=floor-like (skip)*/,amHullN[MAX_MDLS],amCardState[MAX_MDLS]/*0=unknown,1=ok*/,amRadCells[ARRSIZE],amDiagCell[ARRSIZE],amDoorXR[256]/*1=X-running span, 0=Z-running*/; static bool amReady=false,amBuiltFull=false; static u32 amTexId=0,amFBO=0,amFullTexId=0,amFullFBO=0,amBuiltDoorHash=0; i32 amDoorN=0; static u16 amWedgeInst[ARRSIZE]; static int amDrawW,amDrawH; static u8* amDrawPx; static float amMapScaleX,amMapScaleY,amMapOffsetX;
typedef struct {float x,z;}AmPt;
static float amBuiltPX=1e30f,amBuiltPZ=1e30f,amBuiltFX=0.0f,amBuiltFZ=-1.0f,amWinX0,amWinZ1,amPxPerUnit,amHullX[MAX_MDLS][AM_MAXHULL],amHullZ[MAX_MDLS][AM_MAXHULL],amMinX[MAX_MDLS],amMaxX[MAX_MDLS],amMinZ[MAX_MDLS],amMaxZ[MAX_MDLS],amMinY[MAX_MDLS],amMaxY[MAX_MDLS],amCardX[MAX_MDLS][4],amCardY[MAX_MDLS][4],amCardZ[MAX_MDLS][4],amCardNx[MAX_MDLS],amCardNy[MAX_MDLS],amCardNz[MAX_MDLS],amFloorY[ARRSIZE],amDoorX0[256],amDoorZ0[256],amDoorX1[256],amDoorZ1[256];
INLINE int amNavVer() {/*nav hw ver: 0=none,1..3=v1..v3; gates zoom/overlays/cadence*/ return World.invP1.hwVers[HW_NAV_IDX]; }
static int amPtCmp(const void* a,const void* b) { const AmPt* pa=(const AmPt*)a,*pb=(const AmPt*)b; if(pa->x<pb->x){return -1;} if(pa->x>pb->x){return 1;} if(pa->z<pb->z){return -1;} if(pa->z>pb->z){return 1;} return 0; }/*monotone-chain hull of (x,z) verts; returns count*/
static u8 amComputeModel(u16 m) {
    if(amMdlState[m]){return amHullN[m];} amMdlState[m]=2; amHullN[m]=0; amMdlWall[m]=0; if(!physPos || !physVertCounts){return 0;} float* pos=physPos[m]; u32 vc=physVertCounts[m]; if(!pos||!vc){return 0;} float minX=1e30f,maxX=-1e30f,minY=1e30f,maxY=-1e30f,minZ=1e30f,maxZ=-1e30f; for(u32 i=0;i<vc;++i){float x=pos[i*3+0],y=pos[i*3+1],z=pos[i*3+2]; if(x<minX){minX=x;} if(x>maxX){maxX=x;} if(y<minY){minY=y;} if(y>maxY){maxY=y;} if(z<minZ){minZ=z;} if(z>maxZ){maxZ=z;}}
    float ySz=maxY-minY,xSz=maxX-minX,zSz=maxZ-minZ; amMinY[m]=minY; amMaxY[m]=maxY;/*cache Y extents even if floor-like (height detect)*/ if(ySz < 0.5f*vmin(xSz,zSz)){return 0;}/*floor/ceiling-like: no wall*/ static AmPt pts[2048]; u32 n=0; u32 stride=(vc>2048)?((vc+2047)/2048):1;  for (u32 i=0;i<vc && n<2048;i+=stride) { pts[n].x=pos[i*3+0]; pts[n].z=pos[i*3+2]; ++n; } if(n<3){return 0;} qsort_new(pts,n,sizeof(AmPt),amPtCmp); static AmPt hull[4096];
    u32 k=0; for (u32 i=0;i<n;++i) { while (k>=2) { AmPt a=hull[k-2],b=hull[k-1],c=pts[i]; float cr=(b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x); if(cr<=1e-9f){--k;} else break; } hull[k++]=pts[i]; } u32 t=k+1; for (i32 i=(i32)n-2;i>=0;--i) { while (k>=t) { AmPt a=hull[k-2],b=hull[k-1],c=pts[i]; float cr=(b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x); if(cr<=1e-9f){--k;} else break; } hull[k++]=pts[i]; } if(k>1){--k;}/*last point duplicates first*/
    if(k>AM_MAXHULL){k=AM_MAXHULL;} for (u32 i=0;i<k;++i) { amHullX[m][i]=hull[i].x; amHullZ[m][i]=hull[i].z; } amHullN[m]=(u8)k; amMdlWall[m]=1; amMdlState[m]=1; amMinX[m]=minX; amMaxX[m]=maxX; amMinZ[m]=minZ; amMaxZ[m]=maxZ; amMinY[m]=minY; amMaxY[m]=maxY; return amHullN[m];
}

INLINE u16 amCardModel(u16 m) { return (m < 306) ? 178 : m; }/*4 verts furthest from origin (card corners, no greebles). 1=ok,0=no mesh. normal via Newell.  model<306 uses 178 quad card. greebled geo ignored for diagonal/height; canonical card used*/
static u8 amComputeCard(u16 m) {
    if(amCardState[m]){return 1;} amCardState[m]=1;/*mark computed even on fail (no retry)*/ if(!physPos || !physVertCounts){return 0;} float* pos=physPos[m]; u32 vc=physVertCounts[m]; if(!pos || !vc || vc<4){return 0;}  float best[4]={-1,-1,-1,-1};/*4 largest dists from origin*/ u32 bidx[4]={0,0,0,0}; 
    for(u32 i=0;i<vc;++i){float x=pos[i*3+0],y=pos[i*3+1],z=pos[i*3+2]; float d2=x*x+y*y+z*z; for(int j=0;j<4;++j){if(d2>best[j]){for(int k=3;k>j;--k){best[k]=best[k-1]; bidx[k]=bidx[k-1];} best[j]=d2; bidx[j]=i; break;}}/*insertion sort best[4], descending*/}
    for (int j=0;j<4;++j) {u32 i=bidx[j]; amCardX[m][j]=pos[i*3+0]; amCardY[m][j]=pos[i*3+1]; amCardZ[m][j]=pos[i*3+2];}
    float nx=0,ny=0,nz=0; for (int j=0;j<4;++j){/*Newell normal*/ int k=(j+1)%4; float x0=amCardX[m][j],y0=amCardY[m][j],z0=amCardZ[m][j]; float x1=amCardX[m][k],y1=amCardY[m][k],z1=amCardZ[m][k]; nx+=(y0-y1)*(z0+z1); ny+=(z0-z1)*(x0+x1); nz+=(x0-x1)*(y0+y1);} float nl=vsqrtf(nx*nx+ny*ny+nz*nz); if(nl>1e-6f){ nx/=nl; ny/=nl; nz/=nl; } amCardNx[m]=nx; amCardNy[m]=ny; amCardNz[m]=nz; return 1;
}

INLINE void amPutPx(int x,int y,u8 r,u8 g,u8 b) {if((u32)x>=(u32)amDrawW || (u32)y>=(u32)amDrawH){return;} u8* p=&amDrawPx[((u32)y*amDrawW+(u32)x)*4]; p[0]=r; p[1]=g; p[2]=b; p[3]=255; }
static void amFillRect(int x0,int y0,int x1,int y1,u8 r,u8 g,u8 b) {/*x1,y1 exclusive*/if(x0<0){x0=0;} if(y0<0){y0=0;} if(x1>amDrawW){x1=amDrawW;} if(y1>amDrawH){y1=amDrawH;} for (int y=y0;y<y1;++y) for (int x=x0;x<x1;++x) { u8* p=&amDrawPx[((u32)y*amDrawW+(u32)x)*4]; p[0]=r; p[1]=g; p[2]=b; p[3]=255; } }
static void amFillTri(float ax,float ay,float bx,float by,float cx,float cy,u8 r,u8 g,u8 b) {
    float d=(by-cy)*(ax-cx)+(cx-bx)*(ay-cy); if(vabs(d)<1e-6f){return;} int x0=(int)vfloor(vmin(vmin(ax,bx),cx)),x1=(int)vceil(vmax(vmax(ax,bx),cx)); int y0=(int)vfloor(vmin(vmin(ay,by),cy)),y1=(int)vceil(vmax(vmax(ay,by),cy)); for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x){float px=(float)x+.5f,py=(float)y+.5f; float l1=((by-cy)*(px-cx)+(cx-bx)*(py-cy))/d,l2=((cy-ay)*(px-cx)+(ax-cx)*(py-cy))/d,l3=1.0f-l1-l2; if(l1>=-.001f && l2>=-.001f && l3>=-.001f){amPutPx(x,y,r,g,b);}}
}
static void amLine(float ax,float ay,float bx,float by,u8 r,u8 g,u8 b,int w) { float dx=bx-ax,dy=by-ay,len=vsqrtf(dx*dx+dy*dy); int n=(int)vceil(len); if(n<1){n=1;} int ww=w;/*fixed pixel width: line thickness is never scaled by map resolution*/ if(ww<1){ww=1;} int o0=-(ww/2),o1=o0+ww-1; for (int i=0;i<=n;++i) { float t=(float)i/(float)n; int x=(int)vround(ax+dx*t),y=(int)vround(ay+dy*t); for (int oy=o0;oy<=o1;++oy) for (int ox=o0;ox<=o1;++ox) amPutPx(x+ox,y+oy,r,g,b); }}
static void amCircle(float cx,float cy,float r,u8 cr,u8 cg,u8 cb) {int n=(int)(r*0.5f); if(n<16){n=16;} if(n>256){n=256;} float px=cx+r,py=cy; for (int i=1;i<=n;++i) {float a=(float)i/(float)n*6.2831853f; float x=cx+r*vcosf(a),y=cy+r*vsinf(a); amLine(px,py,x,y,cr,cg,cb,1); px=x; py=y;}}
INLINE float amWX(float x) { return (x-amWinX0)*amMapScaleX+amMapOffsetX; }
INLINE float amWZ(float z) { return (amWinZ1-z)*amMapScaleY; }
static u8 amEdgeKind(float x0,float z0,float x1,float z1,u8 lev) {/*edge kind: 0=closed-wall green, 1=height-step dark green*/
    float mx=(x0+x1)*0.5f,mz=(z0+z1)*0.5f; float x0c=World.worldMin_x[lev]-CELLXHALF,z0c=World.worldMin_z[lev]-CELLXHALF; int cx=(int)((mx-x0c)/CELLSZ),cz=(int)((mz-z0c)/CELLSZ); if(cx<0||cx>63||cz<0||cz>63){return 0;} float dx=x1-x0,dz=z1-z0; int ncx=cx,ncz=cz; u32 closedBit;
    if(vabs(dx)>=vabs(dz)){/*E-W: check N/S boundary*/float zb0=z0c+cz*CELLSZ,zb1=zb0+CELLSZ; if(vabs(mz-zb0)<0.35f){ closedBit=CELL_CLOSEDSOUTH; ncz=cz-1; } else if(vabs(mz-zb1)<0.35f){ closedBit=CELL_CLOSEDNORTH; ncz=cz+1; } else return 0; } else { float xb0=x0c+cx*CELLSZ,xb1=xb0+CELLSZ; if(vabs(mx-xb0)<0.35f){ closedBit=CELL_CLOSEDWEST; ncx=cx-1; } else if(vabs(mx-xb1)<0.35f){ closedBit=CELL_CLOSEDEAST; ncx=cx+1; } else return 0; }
    u32 cell=(u32)cz*64+(u32)cx; if(gridCellStates[cell]&closedBit){return 0;} if(ncx<0||ncx>63||ncz<0||ncz>63){return 0;} u32 ncell=(u32)ncz*64+(u32)ncx; if((gridCellStates[cell]&CELL_OPEN) && (gridCellStates[ncell]&CELL_OPEN)){return 1;} return 0;
}

INLINE int amEdgeHasDoor(int xrun,float fix,float a0,float a1) {
    for(int d=0;d<amDoorN;++d){if(amDoorXR[d]!=(u8)xrun){continue;} float midx=(amDoorX0[d]+amDoorX1[d])*0.5f, midz=(amDoorZ0[d]+amDoorZ1[d])*0.5f; float gap=xrun?vabs(midz-fix):vabs(midx-fix); if(gap>0.5f){continue;} float b0=xrun?vmin(amDoorX0[d],amDoorX1[d]):vmin(amDoorZ0[d],amDoorZ1[d]); float b1=xrun?vmax(amDoorX0[d],amDoorX1[d]):vmax(amDoorZ0[d],amDoorZ1[d]); float lo=a0>b0?a0:b0, hi=a1<b1?a1:b1; if(hi-lo>0.1f){return 1;} } return 0;
}

static void amNPCColor(NPCType t,u8* r,u8* g,u8* b) { if(t==NPCType_Cyborg||t==NPCType_Supercyborg||t==NPCType_MutantCyborg){ *r=159; *g=76; *b=77; }/*cyborg*/else if(t==NPCType_Mutant||t==NPCType_Supermutant){ *r=185; *g=134; *b=37; }/*mutant*/else { *r=104; *g=95; *b=166; }/*robot*/}
static int amNPCNeedVer(NPCType t) {if(t==NPCType_Robot){return 2;}if(t==NPCType_Cyborg||t==NPCType_Supercyborg||t==NPCType_MutantCyborg){return 3;}if(t==NPCType_Mutant||t==NPCType_Supermutant){return 3;}return 99;/*Cyber/unknown: no overlay*/}
static void amRaster(u8 zoom, bool full) {
    u8 lev=World.curLev; int nav=amNavVer(); int cells=16<<zoom;/*16,32,64; 0=closest*/ float x0c=World.worldMin_x[lev]-CELLXHALF,z0c=World.worldMin_z[lev]-CELLXHALF; float winW=(float)cells*CELLSZ; V3 app=World.position[PLAYER1]; float wantX0=app.x-winW*0.5f, wantZ1=app.z+winW*0.5f; float minX0=x0c,maxX0=x0c+64.0f*CELLSZ-winW; float minZ1=z0c+winW,maxZ1=z0c+64.0f*CELLSZ;
    amDrawW=full?AM_FULL_W:AM_W; amDrawH=full?AM_FULL_H:AM_H; amDrawPx=full?amFullPx:amPx; if(full){amWinX0=x0c; amWinZ1=z0c+64.0f*CELLSZ;}/*Unity: full map camera forced to center, never follows player*/ else {amWinX0=wantX0<minX0?minX0:(wantX0>maxX0?maxX0:wantX0); amWinZ1=wantZ1<minZ1?minZ1:(wantZ1>maxZ1?maxZ1:wantZ1); if(maxX0<minX0){amWinX0=minX0;} if(maxZ1<minZ1){amWinZ1=maxZ1;}} amPxPerUnit=(float)amDrawH/winW; amMapScaleX=full?(float)amDrawW/winW:amPxPerUnit; amMapScaleY=amPxPerUnit; amMapOffsetX=full?0.0f:(float)AM_XOFF;  u8* expl=World.automapExplored[lev]; for (u32 i=0;i<(u32)amDrawW*amDrawH*4;i+=4) { amDrawPx[i]=0; amDrawPx[i+1]=0; amDrawPx[i+2]=0; amDrawPx[i+3]=0; }/*transparent*/
    mset(amRadCells,0,sizeof(amRadCells)); mset(amWedgeInst,0xFF,sizeof(amWedgeInst)); mset(amDiagCell,0,sizeof(amDiagCell));
    if(nav>=3){/*rad triggers (nav v3+): expand bounds to overlapped cells*/
        for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
            Entity* e=&World.instances[i]; if(e->index!=601||!(e->entflags&EF_ACTIVE)){continue;} V3 c=World.colliderCenter[i],s=World.colliderSize[i],p=World.position[i]; float x0=p.x+c.x-s.x*0.5f,x1=p.x+c.x+s.x*0.5f,z0=p.z+c.z-s.z*0.5f,z1=p.z+c.z+s.z*0.5f;
            /*floor min, ceil max: fill touched cells*/ int ax=(int)vfloor((x0-x0c)/CELLSZ),bx=(int)vceil((x1-x0c)/CELLSZ); int az=(int)vfloor((z0-z0c)/CELLSZ),bz=(int)vceil((z1-z0c)/CELLSZ); if(ax<0){ax=0;} if(az<0){az=0;} if(bx>63){bx=63;} if(bz>63){bz=63;} for(int cz=az;cz<=bz;++cz)for(int cx=ax;cx<=bx;++cx)amRadCells[cz*64+cx]=1;
        }
    }
    for(u32 c=0;c<ARRSIZE;++c){amFloorY[c]=-1e30f;} float x0f=World.worldMin_x[lev]-CELLXHALF,z0f=World.worldMin_z[lev]-CELLXHALF;
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*Get floor heights for difference check to draw dark green line between cells of different heights.*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; u16 cm=amCardModel(m); if(!amComputeCard(cm)){continue;} V3 n=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]}); if(vabs(n.y)<0.7f){continue;}/*not horizontal*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; float sy=0,wx0=1e30f,wx1=-1e30f,wz0=1e30f,wz1=-1e30f;
        for(int k=0;k<4;++k){V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z}); sy+=ip.y+v.y; float wx=ip.x+v.x, wz=ip.z+v.z; if(wx<wx0){wx0=wx;} if(wx>wx1){wx1=wx;} if(wz<wz0){wz0=wz;} if(wz>wz1){wz1=wz;}}
        float topY=sy*0.25f; int ax=(int)vfloor((wx0-x0f)/CELLSZ), bx=(int)vceil((wx1-x0f)/CELLSZ); int az=(int)vfloor((wz0-z0f)/CELLSZ), bz=(int)vceil((wz1-z0f)/CELLSZ); if(ax<0){ax=0;} if(az<0){az=0;} if(bx>63){bx=63;} if(bz>63){bz=63;} for (int cz=az;cz<=bz;++cz) for(int cx=ax;cx<=bx;++cx){u32 cell=(u32)cz*64+(u32)cx; if(topY>amFloorY[cell]){amFloorY[cell]=topY;}}
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*wedge inst per cell*/Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; if(!amComputeModel(m) || !amMdlWall[m] || amHullN[m]!=3){continue;} int cx=PosGetCellCoordX(World.position[i].x),cz=PosGetCellCoordZ(World.position[i].z); if(cx>=0&&cx<64&&cz>=0&&cz<64){amWedgeInst[(u32)cz*64+(u32)cx]=(u16)i;}}
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*diag-wall cells (card-based). diag cells skip closed edges; neighbors draw backs*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; u16 cm=amCardModel(m); if(!amComputeCard(cm)){continue;} V3 wn=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]}); if(vabs(wn.y)>0.7f){continue;}/*horizontal: no wall*/ if(vabs(wn.x)<0.3f || vabs(wn.z)<0.3f){continue;}/*axis-aligned, not diagonal*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; float ccx=0,ccz=0; for (int k=0;k<4;++k) {V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z}); ccx+=ip.x+v.x; ccz+=ip.z+v.z;} ccx*=0.25f; ccz*=0.25f; int cx=PosGetCellCoordX(ccx),cz=PosGetCellCoordZ(ccz); if(cx>=0&&cx<64&&cz>=0&&cz<64){amDiagCell[(u32)cz*64+(u32)cx]=1;}
    }
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {/*cell fills: transparent; rad dark orange; wedge cells get access tri only*/
        u32 cell=(u32)cz*64+(u32)cx; if(!(gridCellStates[cell]&CELL_OPEN)||!expl[cell]){continue;} float wx0=x0c+(float)cx*CELLSZ,wx1=wx0+CELLSZ; float wz0=z0c+(float)cz*CELLSZ,wz1=wz0+CELLSZ; float rx0=amWX(wx0),rx1=amWX(wx1),ry0=amWZ(wz1),ry1=amWZ(wz0);/*top-left px*/ if(rx1<0.0f||rx0>(float)amDrawW||ry1<0.0f||ry0>(float)amDrawH){continue;}/*outside zoom window*/ bool rad=amRadCells[cell]!=0; if(!rad){continue;}/*black -> transparent: leave cleared alpha 0*/
        u8 r=170,g=85,b=0; u16 wi=amWedgeInst[cell]; 
        if(wi!=U16_MAX){/*access tri = empty corner + hypotenuse ends*/
            Entity* e=&World.instances[wi]; u16 m=e->modelIndex; float wx[3],wz[3]; for (u8 k=0;k<3;++k) { V3 v=quat_rot_v3(World.rotation[wi],(V3){amHullX[m][k]*World.scale[wi].x,0.0f,amHullZ[m][k]*World.scale[wi].z}); wx[k]=amWX(World.position[wi].x+v.x); wz[k]=amWZ(World.position[wi].z+v.z); } float gx=(wx[0]+wx[1]+wx[2])/3.0f,gy=(wz[0]+wz[1]+wz[2])/3.0f; float corns[4][2]={{rx0,ry0},{rx1,ry0},{rx1,ry1},{rx0,ry1}}; int ei=0; float bd=-1.0f;
            for (int k=0;k<4;++k) { float dx=corns[k][0]-gx,dy=corns[k][1]-gy,d2=dx*dx+dy*dy; if(d2>bd){bd=d2;ei=k;} } int h0=0,h1=1; float bl=-1.0f;/*longest wedge edge = hypotenuse*/ for (int k=0;k<3;++k) { int a=k,bb=(k+1)%3; float dx=wx[a]-wx[bb],dy=wz[a]-wz[bb],d2=dx*dx+dy*dy; if(d2>bl){bl=d2;h0=a;h1=bb;} } amFillTri(corns[ei][0],corns[ei][1],wx[h0],wz[h0],wx[h1],wz[h1],r,g,b);
        } else {amFillRect((int)rx0,(int)ry0,(int)(rx1+0.5f),(int)(ry1+0.5f),r,g,b);}
    }
    amDoorN=0;
    for (u32 i=INSTS_1ST_IDX;i<World.instCount && amDoorN<256;++i) {/*door spans, world XZ, explored only. skip wall lines under doors*/
        Entity* de=&World.instances[i]; if(!IdxIsDoor(de->index) || !(de->entflags&EF_ACTIVE) || de->modelIndex>=MAX_MDLS){continue;} u16 dm=de->modelIndex; if(!amComputeModel(dm)){continue;} bool dis2cell=(de->index==499||de->index==508||de->index==509||de->index==510||de->index==511); float dmaxW=dis2cell?5.12f:2.56f; float dlx0,dlz0,dlx1,dlz1;
        if((amMaxX[dm]-amMinX[dm]) >= (amMaxZ[dm]-amMinZ[dm])){float dw=amMaxX[dm]-amMinX[dm]; if(dw>dmaxW){dw=dmaxW;} float dcx=(amMinX[dm]+amMaxX[dm])*0.5f, dcz=(amMinZ[dm]+amMaxZ[dm])*0.5f; dlx0=dcx-dw*0.5f; dlz0=dcz; dlx1=dcx+dw*0.5f; dlz1=dcz;} else {float dw=amMaxZ[dm]-amMinZ[dm]; if(dw>dmaxW){dw=dmaxW;} float dcx=(amMinX[dm]+amMaxX[dm])*0.5f, dcz=(amMinZ[dm]+amMaxZ[dm])*0.5f; dlx0=dcx; dlz0=dcz-dw*0.5f; dlx1=dcx; dlz1=dcz+dw*0.5f;}
        V3 dip=World.position[i]; Quaternion diq=World.rotation[i]; V3 dis=World.scale[i]; V3 dva=quat_rot_v3(diq,(V3){dlx0*dis.x,0.0f,dlz0*dis.z}); V3 dvb=quat_rot_v3(diq,(V3){dlx1*dis.x,0.0f,dlz1*dis.z}); float dax=dip.x+dva.x,daz=dip.z+dva.z,dbx=dip.x+dvb.x,dbz=dip.z+dvb.z; int decx=PosGetCellCoordX((dax+dbx)*0.5f),decz=PosGetCellCoordZ((daz+dbz)*0.5f); if(!expl[(u32)decz*64+(u32)decx]){continue;}/*door not drawn: keep wall line*/
        amDoorX0[amDoorN]=dax; amDoorZ0[amDoorN]=daz; amDoorX1[amDoorN]=dbx; amDoorZ1[amDoorN]=dbz; amDoorXR[amDoorN]=(u8)(vabs(dbx-dax)>=vabs(dbz-daz)); ++amDoorN;
    }
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {/*walls from grid closed edges (light green). grid-implicit; hulls miss them. diag cells skip closed edges. door-coincident edges skipped*/
        u32 cell=(u32)cz*64+(u32)cx; if(!expl[cell]){continue;} if(amDiagCell[cell]){continue;}/*diag: diagonal only*/ u32 st=gridCellStates[cell]; if(!(st&(CELL_CLOSEDNORTH|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST|CELL_CLOSEDEAST))){continue;} /*cell px rect: rx0,ry0=top-left (NW)*/ float rx0=amWX(x0c+(float)cx*CELLSZ),rx1=amWX(x0c+(float)(cx+1)*CELLSZ); float ry0=amWZ(z0c+(float)(cz+1)*CELLSZ),ry1=amWZ(z0c+(float)cz*CELLSZ);
        if(rx1<0.0f||rx0>(float)amDrawW||ry1<0.0f||ry0>(float)amDrawH){continue;}/*outside zoom window*/ float ewx0=x0c+(float)cx*CELLSZ,ewx1=ewx0+CELLSZ; float ewz0=z0c+(float)cz*CELLSZ,ewz1=ewz0+CELLSZ; if(st&CELL_CLOSEDNORTH){ if(!amEdgeHasDoor(1,ewz1,ewx0,ewx1)){amLine(rx0,ry0,rx1,ry0,0,220,0,2);} } if(st&CELL_CLOSEDSOUTH){ if(!amEdgeHasDoor(1,ewz0,ewx0,ewx1)){amLine(rx0,ry1,rx1,ry1,0,220,0,2);} }
        if(st&CELL_CLOSEDWEST){ if(!amEdgeHasDoor(0,ewx0,ewz0,ewz1)){amLine(rx0,ry0,rx0,ry1,0,220,0,2);} } if(st&CELL_CLOSEDEAST){ if(!amEdgeHasDoor(0,ewx1,ewz0,ewz1)){amLine(rx1,ry0,rx1,ry1,0,220,0,2);} }
    }
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {/*height steps: dark green where neighbor floors differ. N+E neighbors only, no double-draw. closed walls skipped*/
        u32 cell=(u32)cz*64+(u32)cx; if(!expl[cell] || amFloorY[cell]<-1e29f){continue;} float rx0=amWX(x0c+(float)cx*CELLSZ),rx1=amWX(x0c+(float)(cx+1)*CELLSZ); float ry0=amWZ(z0c+(float)(cz+1)*CELLSZ),ry1=amWZ(z0c+(float)cz*CELLSZ); if(rx1<0.0f||rx0>(float)amDrawW||ry1<0.0f||ry0>(float)amDrawH){continue;}/*outside zoom window*/ u32 st=gridCellStates[cell];
        if(cz+1<64){/*N neighbor*/u32 ncell=(u32)(cz+1)*64+(u32)cx; u32 nst=gridCellStates[ncell]; if(!(st&CELL_CLOSEDNORTH) && !(nst&CELL_CLOSEDSOUTH) && expl[ncell] && amFloorY[ncell]>-1e29f){float dh=vabs(amFloorY[cell]-amFloorY[ncell]); if(dh>=0.16f){amLine(rx0,ry0,rx1,ry0,0,110,0,2);}/*north edge*/}}
        if(cx+1<64){/*E neighbor*/u32 ncell=(u32)cz*64+(u32)(cx+1); u32 nst=gridCellStates[ncell]; if(!(st&CELL_CLOSEDEAST) && !(nst&CELL_CLOSEDWEST) && expl[ncell] && amFloorY[ncell]>-1e29f){float dh=vabs(amFloorY[cell]-amFloorY[ncell]); if(dh>=0.16f){amLine(rx1,ry0,rx1,ry1,0,110,0,2);}/*east edge*/}}
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*diag walls: vertical cards with X+Z normal (non-axis). longest XZ span drawn. chunks use 178 card*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; u16 cm=amCardModel(m); if(!amComputeCard(cm)){continue;} /*world normal via instance rot*/ V3 wn=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]}); /*diag wall check: vertical, non-axis*/ if(vabs(wn.y)>0.7f){continue;}/*horizontal: no wall*/
        if(vabs(wn.x)<0.3f || vabs(wn.z)<0.3f){continue;}/*axis-aligned, grid handles it*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; float wx[4],wz[4]; for (int k=0;k<4;++k) { V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z}); wx[k]=ip.x+v.x; wz[k]=ip.z+v.z; }
        /*FoW: card center*/float ccx=(wx[0]+wx[1]+wx[2]+wx[3])*0.25f, ccz=(wz[0]+wz[1]+wz[2]+wz[3])*0.25f; int ecx=PosGetCellCoordX(ccx),ecz=PosGetCellCoordZ(ccz); if(!expl[(u32)ecz*64+(u32)ecx]){continue;}
        /*longest XZ span*/int bi=0,bj=1; float bd=-1.0f; for (int a=0;a<4;++a) for (int b=a+1;b<4;++b) {float dx=wx[a]-wx[b],dz=wz[a]-wz[b],d2=dx*dx+dz*dz; if(d2>bd){bd=d2;bi=a;bj=b;} }
        float x0=amWX(wx[bi]),y0=amWZ(wz[bi]),x1=amWX(wx[bj]),y1=amWZ(wz[bj]); amLine(x0,y0,x1,y1,0,220,0,2);
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*hull outlines: dark green open steps only. closed walls drawn from grid*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; if(!amComputeModel(m) || !amMdlWall[m]){continue;} u8 hn=amHullN[m]; if(hn<3){continue;} float px[AM_MAXHULL],pz[AM_MAXHULL]; V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
        for (u8 k=0;k<hn;++k) { V3 v=quat_rot_v3(iq,(V3){amHullX[m][k]*is.x,0.0f,amHullZ[m][k]*is.z}); px[k]=amWX(ip.x+v.x); pz[k]=amWZ(ip.z+v.z); }
        for (u8 k=0;k<hn;++k) {
            u8 a=k,b=(u8)((k+1)%hn); V3 va=quat_rot_v3(iq,(V3){amHullX[m][a]*is.x,0.0f,amHullZ[m][a]*is.z}); V3 vb=quat_rot_v3(iq,(V3){amHullX[m][b]*is.x,0.0f,amHullZ[m][b]*is.z}); float mx=(ip.x+va.x+ip.x+vb.x)*0.5f,mz=(ip.z+va.z+ip.z+vb.z)*0.5f; int ecx=PosGetCellCoordX(mx),ecz=PosGetCellCoordZ(mz); if(!expl[(u32)ecz*64+(u32)ecx]){continue;} if(amEdgeKind(ip.x+va.x,ip.z+va.z,ip.x+vb.x,ip.z+vb.z,lev)){amLine(px[a],pz[a],px[b],pz[b],0,110,0,2);}
        }
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*doors: closed=yellow span; open=yellow center ticks. bulkheads (499/509) + big elevator doors (508/510/511): end ticks only, 2 per cell. span=width axis, clamp 1 cell (2.56), 2-cell doors 2 cells (5.12). no card geo*/
        Entity* e=&World.instances[i]; if(!IdxIsDoor(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; if(!amComputeModel(m)){continue;} /*width-axis ends, local space, clamped*/bool is2cell=(e->index==499||e->index==508||e->index==509||e->index==510||e->index==511); float maxW=is2cell?5.12f:2.56f; float lx0,lz0,lx1,lz1;
        if((amMaxX[m]-amMinX[m]) >= (amMaxZ[m]-amMinZ[m])){float w=amMaxX[m]-amMinX[m]; if(w>maxW){w=maxW;} float cx=(amMinX[m]+amMaxX[m])*0.5f, cz=(amMinZ[m]+amMaxZ[m])*0.5f; lx0=cx-w*0.5f; lz0=cz; lx1=cx+w*0.5f; lz1=cz;}else {float w=amMaxZ[m]-amMinZ[m]; if(w>maxW){w=maxW;} float cx=(amMinX[m]+amMaxX[m])*0.5f, cz=(amMinZ[m]+amMaxZ[m])*0.5f; lx0=cx; lz0=cz-w*0.5f; lx1=cx; lz1=cz+w*0.5f;}
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; V3 va=quat_rot_v3(iq,(V3){lx0*is.x,0.0f,lz0*is.z}); V3 vb=quat_rot_v3(iq,(V3){lx1*is.x,0.0f,lz1*is.z}); int ecx=PosGetCellCoordX(ip.x+(va.x+vb.x)*0.5f),ecz=PosGetCellCoordZ(ip.z+(va.z+vb.z)*0.5f); if(!expl[(u32)ecz*64+(u32)ecx]){continue;}
        float ax=amWX(ip.x+va.x),ay=amWZ(ip.z+va.z),bx=amWX(ip.x+vb.x),by=amWZ(ip.z+vb.z); bool closed=(e->doorState==DoorState_Closed||e->doorState==DoorState_Closing); bool bulkhead=(e->index==499||e->index==509); if(closed){ amLine(ax,ay,bx,by,255,255,0,2); continue; }
        float dx=bx-ax,dy=by-ay,len=vsqrtf(dx*dx+dy*dy); if(len<1e-6f){continue;}
        dx/=len; dy/=len; float nx=-dy,ny=dx;/*span dir + perp*/ float cx=(ax+bx)*0.5f,cy=(ay+by)*0.5f; if(!bulkhead){amLine(cx-dx*1.0f,cy-dy*1.0f,cx+dx*1.0f,cy+dy*1.0f,255,255,0,2);}/*center*/ amLine(ax-nx*1.0f,ay-ny*1.0f,ax+nx*1.0f,ay+ny*1.0f,255,255,0,2);/*end ticks*/ amLine(bx-nx*1.0f,by-ny*1.0f,bx+nx*1.0f,by+ny*1.0f,255,255,0,2); if(bulkhead){amLine(cx-nx*1.0f,cy-ny*1.0f,cx+nx*1.0f,cy+ny*1.0f,255,255,0,2);}/*mid tick*/
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*overlays (nav-gated): NPC dots, red cameras/CPU nodes*/
        Entity* e=&World.instances[i]; if(!(e->entflags&EF_ACTIVE) || e->health<=0.0f){continue;} int ci=e->index; float x=amWX(World.position[i].x),y=amWZ(World.position[i].z); if(x<-12.0f||y<-12.0f||x>amDrawW+12.0f||y>amDrawH+12.0f){continue;} int ecx=PosGetCellCoordX(World.position[i].x),ecz=PosGetCellCoordZ(World.position[i].z); if(!expl[(u32)ecz*64+(u32)ecx]){continue;} 
        if(IdxIsNPC(ci)){NPCType t=npcTable[ci-419].type; if(nav<amNPCNeedVer(t)){continue;} u8 r,g,b; amNPCColor(t,&r,&g,&b); amFillRect((int)(x-5.0f),(int)(y-5.0f),(int)(x+5.0f),(int)(y+5.0f),r,g,b);}
        else if(ci==477){/*Cameras*/amFillRect((int)(x-1.5f),(int)(y-1.5f),(int)(x+1.5f),(int)(y+1.5f),255,0,0);}
        else if(ci==478||ci==479){/*Nodes*/amFillRect((int)(x-5.0f),(int)(y-5.0f),(int)(x+5.0f),(int)(y+5.0f),255,0,0);}
    }
    {V3 p=World.position[PLAYER1],f=World.instances[PLAYER1].forward; float x=amWX(p.x),y=amWZ(p.z); float fl=vsqrtf(f.x*f.x+f.z*f.z); float dx=0.0f,dy=-1.0f;
        if(fl>1e-4f){ dx=f.x/fl; dy=-f.z/fl; }/*screen: +x right, -z up*/
        float ox=dx*amMapScaleX/amPxPerUnit,oy=dy,ol=vsqrtf(ox*ox+oy*oy);
        if(ol>1e-4f){ox/=ol; oy/=ol;} else {ox=0.0f;oy=-1.0f;}
        float s=1.0f,nx=-oy,ny=ox;/*Unity: player icon is fixed-size UI overlay, not zoom-scaled*/ float tx=x+ox*8.0f*s,ty=y+oy*8.0f*s,lx=x-ox*5.0f*s+nx*5.0f*s,ly=y-oy*5.0f*s+ny*5.0f*s,rx=x-ox*5.0f*s-nx*5.0f*s,ry=y-oy*5.0f*s-ny*5.0f*s,ov=0.2f;/*hollow chevron: side edges overrun their tail verts by 20%, tip vertex unchanged*/ amLine(tx,ty,lx+(lx-tx)*ov,ly+(ly-ty)*ov,240,73,77,1); amLine(tx,ty,rx+(rx-tx)*ov,ry+(ry-ty)*ov,240,73,77,1); amLine(lx,ly,rx,ry,240,73,77,1);/*240,73,77 = 0.941f,0.286f,0.302f*/}
    if(nav>=3){V3 p=World.position[PLAYER1]; float x=amWX(p.x),y=amWZ(p.z); amCircle(x,y,14.08f*amPxPerUnit,0,200,200);/*small: cyan*/amCircle(x,y,20.48f*amPxPerUnit,0,200,200);/*large: cyan*/}
}

static bool amReveal() {
    u8 lev=World.curLev; V3 p=World.position[PLAYER1]; int pcx=PosGetCellCoordX(p.x),pcz=PosGetCellCoordZ(p.z); int r=(int)2; bool changed=false;
    for (int dz=-r;dz<=r;++dz) for (int dx=-r;dx<=r;++dx) {int cx=pcx+dx,cz=pcz+dz; if(cx<0||cx>63||cz<0||cz>63){continue;} float wx=World.worldMin_x[lev]+cx*CELLSZ-p.x,wz=World.worldMin_z[lev]+cz*CELLSZ-p.z; if(wx*wx+wz*wz>AM_FOW_RADIUS2){continue;} u32 cell=(u32)cz*64+(u32)cx; if(!(gridCellStates[cell]&CELL_OPEN)){continue;} if(!World.automapExplored[lev][cell]){ World.automapExplored[lev][cell]=1; changed=true; } } return changed;
}

static u32 amDoorHash() {u32 h=0; for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {Entity* e=&World.instances[i]; if(IdxIsDoor(e->index)){h=h*31u+(u32)(i*7u+(u32)e->doorState);}} return h;}
void AutomapInitGL() {if(amReady){return;} GenerateAndBindTexture(&amTexId,GL_RGBA8,AM_W,AM_H,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL); glGenFramebuffers(1,&amFBO); glBindFramebuffer(GL_FRAMEBUFFER,amFBO); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,amTexId,0); GenerateAndBindTexture(&amFullTexId,GL_RGBA8,AM_FULL_W,AM_FULL_H,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL); glGenFramebuffers(1,&amFullFBO); glBindFramebuffer(GL_FRAMEBUFFER,amFullFBO); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,amFullTexId,0); glBindFramebuffer(GL_FRAMEBUFFER,0); amReady=true;}
void AutomapNewGame() {mset(World.automapExplored,0,sizeof(World.automapExplored)); World.automapZoom=0; World.automapNextRaster=0.0; amBuiltLev=255; amBuiltPX=1e30f; amBuiltPZ=1e30f;}
void AutomapOnLoad() { amBuiltLev=255; World.automapNextRaster=0.0; amBuiltPX=1e30f; amBuiltPZ=1e30f; }/*force re-raster from loaded FoW*/
void AutomapTick() {
    if(!amReady || World.menuActive){return;} u8 lev=World.curLev; if(lev>=LEVEL_CYBERSPACE){return;} bool fowChanged=amReveal(); u32 dh=amDoorHash(); double now=get_time(); V3 tpp=World.position[PLAYER1]; V3 tpf=World.instances[PLAYER1].forward; bool full=World.Sys_UI.fullMapOpen[0] || World.Sys_UI.fullMapOpen[1]; u8 renderZoom=full ? 2 : World.automapZoom; int drawH=full?AM_FULL_H:AM_H; float wpp=((float)(16<<renderZoom)*CELLSZ)/(float)drawH;/*world/px*/ float mdx=tpp.x-amBuiltPX,mdz=tpp.z-amBuiltPZ,me=(0.25f*wpp)*(0.25f*wpp); float fdx=tpf.x-amBuiltFX,fdz=tpf.z-amBuiltFZ;
    bool moved=(mdx*mdx+mdz*mdz)>me || (fdx*fdx+fdz*fdz)>1e-6f;
    if(fowChanged || moved || dh!=amBuiltDoorHash || lev!=amBuiltLev || renderZoom!=amBuiltZoom || full!=amBuiltFull || now>=World.automapNextRaster){amRaster(renderZoom,full); amBuiltLev=lev; amBuiltDoorHash=dh; amBuiltZoom=renderZoom; amBuiltFull=full; amBuiltPX=tpp.x; amBuiltPZ=tpp.z; amBuiltFX=tpf.x; amBuiltFZ=tpf.z; int nav=amNavVer(); World.automapNextRaster=now+(nav>2?0:(nav>1?0.1:0.2)); glBindTexture(GL_TEXTURE_2D,full?amFullTexId:amTexId); glTexSubImage2D(GL_TEXTURE_2D,0,0,0,full?AM_FULL_W:AM_W,full?AM_FULL_H:AM_H,GL_RGBA,GL_UNSIGNED_BYTE,full?amFullPx:amPx);}
}

void AutomapBlitToUI() {
    if(!amReady || World.menuActive || World.paused || Cheats.noHUD || World.curLev>=LEVEL_CYBERSPACE){return;}
    int n=0; int dx0[2],dy0[2],dx1[2],dy1[2]; if(World.Sys_UI.MFD_LefTab==3){ dx0[n]=AMAP_UI_X_L; dy0[n]=AMAP_UI_Y; dx1[n]=AMAP_UI_X_L+AMAP_UI_W; dy1[n]=AMAP_UI_Y+AMAP_UI_H; ++n; } if(World.Sys_UI.MFD_RightTab==3){ dx0[n]=AMAP_UI_X_R; dy0[n]=AMAP_UI_Y; dx1[n]=AMAP_UI_X_R+AMAP_UI_W; dy1[n]=AMAP_UI_Y+AMAP_UI_H; ++n; } if(!n){return;}
    glBindFramebuffer(GL_READ_FRAMEBUFFER,amFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO); for (int k=0;k<n;++k) {/*UI y-down->GL y-up; row0=north=texture bottom: flip src*/int gx0=dx0[k],gy0=768-dy1[k],gx1=dx1[k],gy1=768-dy0[k]; glBlitFramebuffer(0,AM_H,AM_W,0, gx0,gy0,gx1,gy1, GL_COLOR_BUFFER_BIT,GL_LINEAR);} glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
}

void AutomapBlitFullToUI() {
    if(!amReady || World.menuActive || World.paused || Cheats.noHUD || World.curLev>=LEVEL_CYBERSPACE){return;}
    if(!World.Sys_UI.fullMapOpen[0] && !World.Sys_UI.fullMapOpen[1]){return;}
    i16 x=(i16)((UI_W-AM_FULL_W)/2),y=(i16)((UI_H-AM_FULL_H)/2); glBindFramebuffer(GL_READ_FRAMEBUFFER,amFullFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO); glBlitFramebuffer(0,AM_FULL_H,AM_FULL_W,0,x,768-(y+AM_FULL_H),x+AM_FULL_W,768-y,GL_COLOR_BUFFER_BIT,GL_LINEAR); glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
}

void AutomapDumpBMP() {
    if(!amReady){return;} if(World.Sys_UI.MFD_LefTab!=3 && World.Sys_UI.MFD_RightTab!=3){return;} OS_MakeFolder("Screenshots"); static u8 flip[AM_W*AM_H*4]; for(int y=0;y<AM_H;++y){u8* dst=flip+(size_t)y*AM_W*4,*src=amPx+(size_t)(AM_H-1-y)*AM_W*4; for(int x=0;x<AM_W;++x){u8* d=dst+(size_t)x*4,*s=src+(size_t)x*4; d[0]=s[0]; d[1]=s[1]; d[2]=s[2]; d[3]=s[3];}} char p[96]; sFormat(p,sizeof(p),"Screenshots/automap_%.2f.bmp",get_time()); BmpWrite(p,AM_W,AM_H,flip);
}
