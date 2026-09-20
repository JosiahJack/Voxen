//automap.c - CPU-rasterized automap for the side MFD, 320x200
void AutomapTick(void),AutomapInitGL(void),AutomapBlitToUI(void),AutomapNewGame(void),AutomapOnLoad(void);
static u8 amPx[AM_W*AM_H*4];
static u32 amTexId=0,amFBO=0;
static bool amReady=false;
static u8 amBuiltLev=255,amBuiltZoom=255;
static u32 amBuiltDoorHash=0;
/*Current world->pixel window (set by zoom).*/
static float amWinX0,amWinZ1,amPxPerUnit;
/*Per-model footprint cache: local-space XZ convex hull + wall/floor classification.*/
static u8 amMdlState[MAX_MDLS];/*0=unknown,1=ok,2=no mesh*/
static u8 amMdlWall[MAX_MDLS];/*1=wall-like (draw), 0=floor-like (skip)*/
static u8 amHullN[MAX_MDLS];
static float amHullX[MAX_MDLS][AM_MAXHULL],amHullZ[MAX_MDLS][AM_MAXHULL];
static float amMinX[MAX_MDLS],amMaxX[MAX_MDLS],amMinZ[MAX_MDLS],amMaxZ[MAX_MDLS];/*local AABB*/
static float amMinY[MAX_MDLS],amMaxY[MAX_MDLS];/*local Y extents*/
/*Card corners: 4 verts furthest from local origin (the base quad, ignoring greebles).*/
static float amCardX[MAX_MDLS][4],amCardY[MAX_MDLS][4],amCardZ[MAX_MDLS][4];
static float amCardNx[MAX_MDLS],amCardNy[MAX_MDLS],amCardNz[MAX_MDLS];/*card normal*/
static u8 amCardState[MAX_MDLS];/*0=unknown,1=ok*/

/*Nav-unit hardware version: 0=none,1..3=v1..v3. Gates zoom/overlays/cadence.*/
/*TEMP TESTING: force v3 for full feature validation. Remove before commit.*/
#define AM_FORCE_V3 1
INLINE int amNavVer(void) {
#if AM_FORCE_V3
    return 3;
#else
    return World.invP1.hwVers[HW_NAV_IDX];
#endif
}

/*Monotone-chain convex hull of local (x,z) verts; returns hull point count.*/
typedef struct { float x,z; } AmPt;
static int amPtCmp(const void* a,const void* b) { const AmPt* pa=(const AmPt*)a,*pb=(const AmPt*)b;
    if (pa->x<pb->x) return -1; if (pa->x>pb->x) return 1;
    if (pa->z<pb->z) return -1; if (pa->z>pb->z) return 1; return 0; }
static u8 amComputeModel(u16 m) {
    if (amMdlState[m]) return amHullN[m];
    amMdlState[m]=2; amHullN[m]=0; amMdlWall[m]=0;
    if (!physPos || !physVertCounts) return 0;
    float* pos=physPos[m]; u32 vc=physVertCounts[m];
    if (!pos || !vc) return 0;
    float minX=1e30f,maxX=-1e30f,minY=1e30f,maxY=-1e30f,minZ=1e30f,maxZ=-1e30f;
    for (u32 i=0;i<vc;++i) { float x=pos[i*3+0],y=pos[i*3+1],z=pos[i*3+2];
        if (x<minX)minX=x; if (x>maxX)maxX=x; if (y<minY)minY=y; if (y>maxY)maxY=y; if (z<minZ)minZ=z; if (z>maxZ)maxZ=z; }
    float ySz=maxY-minY,xSz=maxX-minX,zSz=maxZ-minZ;
    /*Cache Y extents even for floor-like models (needed for height detection).*/
    amMinY[m]=minY; amMaxY[m]=maxY;
    if (ySz < 0.5f*vmin(xSz,zSz)) return 0;/*floor/ceiling-like: not a wall*/
    static AmPt pts[2048];
    u32 n=0; u32 stride=(vc>2048)?((vc+2047)/2048):1;
    for (u32 i=0;i<vc && n<2048;i+=stride) { pts[n].x=pos[i*3+0]; pts[n].z=pos[i*3+2]; ++n; }
    if (n<3) return 0;
    qsort_new(pts,n,sizeof(AmPt),amPtCmp);
    static AmPt hull[4096];
    u32 k=0;
    for (u32 i=0;i<n;++i) { while (k>=2) { AmPt a=hull[k-2],b=hull[k-1],c=pts[i]; float cr=(b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x); if (cr<=1e-9f) --k; else break; } hull[k++]=pts[i]; }
    u32 t=k+1;
    for (i32 i=(i32)n-2;i>=0;--i) { while (k>=t) { AmPt a=hull[k-2],b=hull[k-1],c=pts[i]; float cr=(b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x); if (cr<=1e-9f) --k; else break; } hull[k++]=pts[i]; }
    if (k>1) --k;/*last point duplicates first*/
    if (k>AM_MAXHULL) k=AM_MAXHULL;
    for (u32 i=0;i<k;++i) { amHullX[m][i]=hull[i].x; amHullZ[m][i]=hull[i].z; }
    amHullN[m]=(u8)k; amMdlWall[m]=1; amMdlState[m]=1;
    amMinX[m]=minX; amMaxX[m]=maxX; amMinZ[m]=minZ; amMaxZ[m]=maxZ;
    amMinY[m]=minY; amMaxY[m]=maxY;
    return amHullN[m];
}

/*Find the 4 verts furthest from the local origin (the card corners, ignoring greebles).
  Returns 1 on success, 0 if no mesh. Card normal is computed via Newell's method.*/
/*Chunk meshes (model index < 306) are treated as if using model 178 (plain quad card).
  Their actual greebled geometry (pipes, bumps) is NOT used for diagonal or height
  determination; the canonical card is used instead.*/
INLINE u16 amCardModel(u16 m) { return (m < 306) ? 178 : m; }
static u8 amComputeCard(u16 m) {
    if (amCardState[m]) return 1;
    amCardState[m]=1;/*mark as computed (even if failed, to avoid retry)*/
    if (!physPos || !physVertCounts) return 0;
    float* pos=physPos[m]; u32 vc=physVertCounts[m];
    if (!pos || !vc || vc<4) return 0;
    /*Find 4 largest distances from origin.*/
    float best[4]={-1,-1,-1,-1}; u32 bidx[4]={0,0,0,0};
    for (u32 i=0;i<vc;++i) {
        float x=pos[i*3+0],y=pos[i*3+1],z=pos[i*3+2];
        float d2=x*x+y*y+z*z;
        /*Insertion sort into best[4] (descending).*/
        for (int j=0;j<4;++j) {
            if (d2>best[j]) {
                for (int k=3;k>j;--k) { best[k]=best[k-1]; bidx[k]=bidx[k-1]; }
                best[j]=d2; bidx[j]=i;
                break;
            }
        }
    }
    /*Store the 4 corners.*/
    for (int j=0;j<4;++j) {
        u32 i=bidx[j];
        amCardX[m][j]=pos[i*3+0]; amCardY[m][j]=pos[i*3+1]; amCardZ[m][j]=pos[i*3+2];
    }
    /*Compute normal via Newell's method.*/
    float nx=0,ny=0,nz=0;
    for (int j=0;j<4;++j) {
        int k=(j+1)%4;
        float x0=amCardX[m][j],y0=amCardY[m][j],z0=amCardZ[m][j];
        float x1=amCardX[m][k],y1=amCardY[m][k],z1=amCardZ[m][k];
        nx+=(y0-y1)*(z0+z1); ny+=(z0-z1)*(x0+x1); nz+=(x0-x1)*(y0+y1);
    }
    float nl=vsqrtf(nx*nx+ny*ny+nz*nz);
    if (nl>1e-6f) { nx/=nl; ny/=nl; nz/=nl; }
    amCardNx[m]=nx; amCardNy[m]=ny; amCardNz[m]=nz;
    return 1;
}

/*--- CPU raster primitives (row 0 = top = north) ---*/
INLINE void amPutPx(int x,int y,u8 r,u8 g,u8 b) {
    if ((u32)x>=(u32)AM_W || (u32)y>=(u32)AM_H) return;
    u8* p=&amPx[((u32)y*AM_W+(u32)x)*4]; p[0]=r; p[1]=g; p[2]=b; p[3]=255;
}
static void amFillRect(int x0,int y0,int x1,int y1,u8 r,u8 g,u8 b) {/*x1,y1 exclusive*/
    if (x0<0)x0=0; if (y0<0)y0=0; if (x1>AM_W)x1=AM_W; if (y1>AM_H)y1=AM_H;
    for (int y=y0;y<y1;++y) for (int x=x0;x<x1;++x) { u8* p=&amPx[((u32)y*AM_W+(u32)x)*4]; p[0]=r; p[1]=g; p[2]=b; p[3]=255; }
}
static void amFillTri(float ax,float ay,float bx,float by,float cx,float cy,u8 r,u8 g,u8 b) {
    float d=(by-cy)*(ax-cx)+(cx-bx)*(ay-cy);
    if (vabs(d)<1e-6f) return;
    int x0=(int)vfloor(vmin(vmin(ax,bx),cx)),x1=(int)vceil(vmax(vmax(ax,bx),cx));
    int y0=(int)vfloor(vmin(vmin(ay,by),cy)),y1=(int)vceil(vmax(vmax(ay,by),cy));
    for (int y=y0;y<=y1;++y) for (int x=x0;x<=x1;++x) {
        float px=(float)x+0.5f,py=(float)y+0.5f;
        float l1=((by-cy)*(px-cx)+(cx-bx)*(py-cy))/d, l2=((cy-ay)*(px-cx)+(ax-cx)*(py-cy))/d, l3=1.0f-l1-l2;
        if (l1>=-0.001f && l2>=-0.001f && l3>=-0.001f) amPutPx(x,y,r,g,b);
    }
}
static void amLine(float ax,float ay,float bx,float by,u8 r,u8 g,u8 b,int w) {
    float dx=bx-ax,dy=by-ay,len=vsqrtf(dx*dx+dy*dy);
    int n=(int)vceil(len); if (n<1) n=1;
    int o0=-(w/2),o1=o0+w-1;
    for (int i=0;i<=n;++i) { float t=(float)i/(float)n; int x=(int)vround(ax+dx*t),y=(int)vround(ay+dy*t);
        for (int oy=o0;oy<=o1;++oy) for (int ox=o0;ox<=o1;++ox) amPutPx(x+ox,y+oy,r,g,b); }
}

/*Draw a circle outline centered at (cx,cy) with radius r (in pixels).*/
static void amCircle(float cx,float cy,float r,u8 cr,u8 cg,u8 cb) {
    int n=(int)(r*0.5f); if (n<16) n=16; if (n>256) n=256;
    float px=cx+r,py=cy;
    for (int i=1;i<=n;++i) {
        float a=(float)i/(float)n*6.2831853f;
        float x=cx+r*vcosf(a),y=cy+r*vsinf(a);
        amLine(px,py,x,y,cr,cg,cb,1);
        px=x; py=y;
    }
}

/*World -> raster pixel for the current zoom window.*/
INLINE float amWX(float x) { return (x-amWinX0)*amPxPerUnit+(float)AM_XOFF; }
INLINE float amWZ(float z) { return (amWinZ1-z)*amPxPerUnit; }

/*Classify a wall edge: 0=green closed wall, 1=dark green height-difference line.*/
static u8 amEdgeKind(float x0,float z0,float x1,float z1,u8 lev) {
    float mx=(x0+x1)*0.5f,mz=(z0+z1)*0.5f;
    float x0c=World.worldMin_x[lev]-CELLXHALF,z0c=World.worldMin_z[lev]-CELLXHALF;
    int cx=(int)((mx-x0c)/CELLSZ),cz=(int)((mz-z0c)/CELLSZ);
    if (cx<0||cx>63||cz<0||cz>63) return 0;
    float dx=x1-x0,dz=z1-z0;
    int ncx=cx,ncz=cz; u32 closedBit;
    if (vabs(dx)>=vabs(dz)) {/*runs east-west: check north/south boundary*/
        float zb0=z0c+cz*CELLSZ,zb1=zb0+CELLSZ;
        if (vabs(mz-zb0)<0.35f) { closedBit=CELL_CLOSEDSOUTH; ncz=cz-1; }
        else if (vabs(mz-zb1)<0.35f) { closedBit=CELL_CLOSEDNORTH; ncz=cz+1; }
        else return 0;
    } else {
        float xb0=x0c+cx*CELLSZ,xb1=xb0+CELLSZ;
        if (vabs(mx-xb0)<0.35f) { closedBit=CELL_CLOSEDWEST; ncx=cx-1; }
        else if (vabs(mx-xb1)<0.35f) { closedBit=CELL_CLOSEDEAST; ncx=cx+1; }
        else return 0;
    }
    u32 cell=(u32)cz*64+(u32)cx;
    if (gridCellStates[cell]&closedBit) return 0;
    if (ncx<0||ncx>63||ncz<0||ncz>63) return 0;
    u32 ncell=(u32)ncz*64+(u32)ncx;
    if ((gridCellStates[cell]&CELL_OPEN) && (gridCellStates[ncell]&CELL_OPEN)) return 1;
    return 0;
}

/*--- Map content ---*/
static u8 amRadCells[ARRSIZE];/*radiation overlap per cell, rebuilt each raster*/
static u16 amWedgeInst[ARRSIZE];/*instance idx of 45deg wedge chunk per cell, U16_MAX=none*/
static u8 amDiagCell[ARRSIZE];/*1=cell contains a 45deg diagonal wall, rebuilt each raster*/
static float amFloorY[ARRSIZE];/*floor height per cell, -1e30=unknown, rebuilt each raster*/

/*NPC dot colors by npcTable type (Unity overlay colors).*/
static void amNPCColor(NPCType t,u8* r,u8* g,u8* b) {
    if (t==NPCType_Cyborg||t==NPCType_Supercyborg||t==NPCType_MutantCyborg) { *r=159; *g=76; *b=77; }/*cyborg*/
    else if (t==NPCType_Mutant||t==NPCType_Supermutant) { *r=185; *g=134; *b=37; }/*mutant*/
    else { *r=104; *g=95; *b=166; }/*robot*/
}
/*Minimum nav-unit version for an NPC overlay category (Unity gating).*/
static int amNPCNeedVer(NPCType t) {
    if (t==NPCType_Robot) return 2;
    if (t==NPCType_Cyborg||t==NPCType_Supercyborg||t==NPCType_MutantCyborg) return 3;
    if (t==NPCType_Mutant||t==NPCType_Supermutant) return 3;
    return 99;/*Cyber/unknown: no overlay*/
}

static void amRaster(void) {
    u8 lev=World.curLev;
    int nav=amNavVer();
    int cells=16<<World.automapZoom;/*16,32,64; 0=closest*/
    int pcx=PosGetCellCoordX(World.position[PLAYER1].x),pcz=PosGetCellCoordZ(World.position[PLAYER1].z);
    int cx0=pcx-cells/2,cz0=pcz-cells/2;
    if (cx0<0)cx0=0; if (cz0<0)cz0=0;
    if (cx0>64-cells)cx0=64-cells; if (cz0>64-cells)cz0=64-cells;
    float x0c=World.worldMin_x[lev]-CELLXHALF,z0c=World.worldMin_z[lev]-CELLXHALF;
    amWinX0=x0c+cx0*CELLSZ; amWinZ1=z0c+(cz0+cells)*CELLSZ;
    amPxPerUnit=(float)AM_H/((float)cells*CELLSZ);
    u8* expl=World.automapExplored[lev];

    for (u32 i=0;i<sizeof(amPx);i+=4) { amPx[i]=0; amPx[i+1]=0; amPx[i+2]=0; amPx[i+3]=255; }/*opaque black*/

    mset(amRadCells,0,sizeof(amRadCells));
    mset(amWedgeInst,0xFF,sizeof(amWedgeInst));
    mset(amDiagCell,0,sizeof(amDiagCell));
    /*Radiation triggers (Unity: shown at nav v3): round bounds outward to full overlapped cells.*/
    if (nav>=3) {
        for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
            Entity* e=&World.instances[i];
            if (e->index!=601 || !(e->entflags&EF_ACTIVE)) continue;
            V3 c=World.colliderCenter[i],s=World.colliderSize[i],p=World.position[i];
            float x0=p.x+c.x-s.x*0.5f,x1=p.x+c.x+s.x*0.5f,z0=p.z+c.z-s.z*0.5f,z1=p.z+c.z+s.z*0.5f;
            /*Round min down (floor) and max up (ceil) to fill all touched cells.*/
            int ax=(int)vfloor((x0-x0c)/CELLSZ),bx=(int)vceil((x1-x0c)/CELLSZ);
            int az=(int)vfloor((z0-z0c)/CELLSZ),bz=(int)vceil((z1-z0c)/CELLSZ);
            if (ax<0)ax=0; if (az<0)az=0; if (bx>63)bx=63; if (bz>63)bz=63;
            for (int cz=az;cz<=bz;++cz) for (int cx=ax;cx<=bx;++cx) amRadCells[cz*64+cx]=1;
        }
    }

    float cellPx=(float)AM_H/(float)cells;/*px per cell at this zoom (square cells)*/
    /*Floor heights per cell: from horizontal cards (upward-facing). Used for height-step lines.
      Chunk meshes use the 178 canonical card; actual greebled geometry is ignored.*/
    for (u32 c=0;c<ARRSIZE;++c) amFloorY[c]=-1e30f;
    {
        float x0f=World.worldMin_x[lev]-CELLXHALF,z0f=World.worldMin_z[lev]-CELLXHALF;
        for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
            Entity* e=&World.instances[i];
            if (!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS) continue;
            u16 m=e->modelIndex;
            u16 cm=amCardModel(m);
            if (!amComputeCard(cm)) continue;
            /*Check if card is horizontal (floor/ceiling): normal mostly Y.*/
            /*Transform normal by instance rotation.*/
            V3 n=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]});
            if (vabs(n.y)<0.7f) continue;/*not horizontal*/
            /*Floor height = average Y of transformed card corners.*/
            V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
            float sy=0,wx0=1e30f,wx1=-1e30f,wz0=1e30f,wz1=-1e30f;
            for (int k=0;k<4;++k) {
                V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z});
                sy+=ip.y+v.y;
                float wx=ip.x+v.x, wz=ip.z+v.z;
                if (wx<wx0)wx0=wx; if (wx>wx1)wx1=wx;
                if (wz<wz0)wz0=wz; if (wz>wz1)wz1=wz;
            }
            float topY=sy*0.25f;
            /*Cells covered by this card.*/
            int ax=(int)vfloor((wx0-x0f)/CELLSZ), bx=(int)vceil((wx1-x0f)/CELLSZ);
            int az=(int)vfloor((wz0-z0f)/CELLSZ), bz=(int)vceil((wz1-z0f)/CELLSZ);
            if (ax<0)ax=0; if (az<0)az=0; if (bx>63)bx=63; if (bz>63)bz=63;
            for (int cz=az;cz<=bz;++cz) for (int cx=ax;cx<=bx;++cx) {
                u32 cell=(u32)cz*64+(u32)cx;
                if (topY>amFloorY[cell]) amFloorY[cell]=topY;
            }
        }
    }
    /*Pre-pass: record 45deg wedge chunk instance per cell (triangular hull).*/
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS) continue;
        u16 m=e->modelIndex;
        if (!amComputeModel(m) || !amMdlWall[m] || amHullN[m]!=3) continue;
        int cx=PosGetCellCoordX(World.position[i].x),cz=PosGetCellCoordZ(World.position[i].z);
        if (cx>=0&&cx<64&&cz>=0&&cz<64) amWedgeInst[(u32)cz*64+(u32)cx]=(u16)i;
    }
    /*Pre-pass: mark cells containing a 45deg diagonal wall (card-based). Diagonal cells
      skip closed-edge wall drawing; only the diagonal renders, neighbors draw the backs.*/
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS) continue;
        u16 m=e->modelIndex;
        u16 cm=amCardModel(m);
        if (!amComputeCard(cm)) continue;
        V3 wn=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]});
        if (vabs(wn.y)>0.7f) continue;/*horizontal, not a wall*/
        if (vabs(wn.x)<0.3f || vabs(wn.z)<0.3f) continue;/*axis-aligned, not diagonal*/
        /*Card center -> cell.*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
        float ccx=0,ccz=0;
        for (int k=0;k<4;++k) {
            V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z});
            ccx+=ip.x+v.x; ccz+=ip.z+v.z;
        }
        ccx*=0.25f; ccz*=0.25f;
        int cx=PosGetCellCoordX(ccx),cz=PosGetCellCoordZ(ccz);
        if (cx>=0&&cx<64&&cz>=0&&cz<64) amDiagCell[(u32)cz*64+(u32)cx]=1;
    }
    /*Cell fills: black, dark orange under radiation; accessible triangle only for 45deg cells.*/
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {
        u32 cell=(u32)cz*64+(u32)cx;
        if (!(gridCellStates[cell]&CELL_OPEN) || !expl[cell]) continue;
        float rx0=(cx-cx0)*cellPx+(float)AM_XOFF,ry0=(cz0+cells-1-cz)*cellPx;/*rx0,ry0 = top-left px*/
        bool rad=amRadCells[cell]!=0;
        u8 r=rad?170:0,g=rad?85:0,b=0;
        u16 wi=amWedgeInst[cell];
        if (wi!=U16_MAX) {
            /*Accessible triangle = empty cell corner + wedge hypotenuse endpoints.*/
            Entity* e=&World.instances[wi]; u16 m=e->modelIndex;
            float wx[3],wz[3];
            for (u8 k=0;k<3;++k) { V3 v=quat_rot_v3(World.rotation[wi],(V3){amHullX[m][k]*World.scale[wi].x,0.0f,amHullZ[m][k]*World.scale[wi].z});
                wx[k]=amWX(World.position[wi].x+v.x); wz[k]=amWZ(World.position[wi].z+v.z); }
            float gx=(wx[0]+wx[1]+wx[2])/3.0f,gy=(wz[0]+wz[1]+wz[2])/3.0f;
            float corns[4][2]={{rx0,ry0},{rx0+cellPx,ry0},{rx0+cellPx,ry0+cellPx},{rx0,ry0+cellPx}};
            int ei=0; float bd=-1.0f;
            for (int k=0;k<4;++k) { float dx=corns[k][0]-gx,dy=corns[k][1]-gy,d2=dx*dx+dy*dy; if (d2>bd){bd=d2;ei=k;} }
            int h0=0,h1=1; float bl=-1.0f;/*longest wedge edge = hypotenuse*/
            for (int k=0;k<3;++k) { int a=k,bb=(k+1)%3; float dx=wx[a]-wx[bb],dy=wz[a]-wz[bb],d2=dx*dx+dy*dy; if (d2>bl){bl=d2;h0=a;h1=bb;} }
            amFillTri(corns[ei][0],corns[ei][1],wx[h0],wz[h0],wx[h1],wz[h1],r,g,b);
        } else {
            amFillRect((int)rx0,(int)ry0,(int)(rx0+cellPx+0.5f),(int)(ry0+cellPx+0.5f),r,g,b);
        }
    }

    /*Walls: draw from grid closed-edge flags (light green). Walls are implicit in
      the grid, not separate instances, so the instance hull outlines miss them.
      Diagonal cells skip closed edges; only the diagonal renders.*/
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {
        u32 cell=(u32)cz*64+(u32)cx;
        if (!expl[cell]) continue;
        if (amDiagCell[cell]) continue;/*diagonal wall: draw diagonal only*/
        u32 st=gridCellStates[cell];
        if (!(st&(CELL_CLOSEDNORTH|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST|CELL_CLOSEDEAST))) continue;
        /*Cell pixel rect: rx0,ry0 = top-left (north-west).*/
        if (cx<cx0||cx>=cx0+cells||cz<cz0||cz>=cz0+cells) continue;/*outside zoom window*/
        float rx0=(cx-cx0)*cellPx+(float)AM_XOFF,ry0=(cz0+cells-1-cz)*cellPx;
        float rx1=rx0+cellPx,ry1=ry0+cellPx;
        if (st&CELL_CLOSEDNORTH) amLine(rx0,ry0,rx1,ry0,0,220,0,2);
        if (st&CELL_CLOSEDSOUTH) amLine(rx0,ry1,rx1,ry1,0,220,0,2);
        if (st&CELL_CLOSEDWEST) amLine(rx0,ry0,rx0,ry1,0,220,0,2);
        if (st&CELL_CLOSEDEAST) amLine(rx1,ry0,rx1,ry1,0,220,0,2);
    }
    /*Height differences: dark green lines where neighbor cells have different floor heights.
      Compare each cell with north and east neighbors to avoid double-draw.
      Skip edges that are closed walls (light green already draws there).*/
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {
        u32 cell=(u32)cz*64+(u32)cx;
        if (!expl[cell] || amFloorY[cell]<-1e29f) continue;
        if (cx<cx0||cx>=cx0+cells||cz<cz0||cz>=cz0+cells) continue;
        float rx0=(cx-cx0)*cellPx+(float)AM_XOFF,ry0=(cz0+cells-1-cz)*cellPx;
        float rx1=rx0+cellPx,ry1=ry0+cellPx;
        u32 st=gridCellStates[cell];
        /*North neighbor (cz+1).*/
        if (cz+1<64) {
            u32 ncell=(u32)(cz+1)*64+(u32)cx;
            u32 nst=gridCellStates[ncell];
            if (!(st&CELL_CLOSEDNORTH) && !(nst&CELL_CLOSEDSOUTH) &&
                expl[ncell] && amFloorY[ncell]>-1e29f) {
                float dh=vabs(amFloorY[cell]-amFloorY[ncell]);
                if (dh>=0.16f) amLine(rx0,ry0,rx1,ry0,0,110,0,2);/*north edge*/
            }
        }
        /*East neighbor (cx+1).*/
        if (cx+1<64) {
            u32 ncell=(u32)cz*64+(u32)(cx+1);
            u32 nst=gridCellStates[ncell];
            if (!(st&CELL_CLOSEDEAST) && !(nst&CELL_CLOSEDWEST) &&
                expl[ncell] && amFloorY[ncell]>-1e29f) {
                float dh=vabs(amFloorY[cell]-amFloorY[ncell]);
                if (dh>=0.16f) amLine(rx1,ry0,rx1,ry1,0,110,0,2);/*east edge*/
            }
        }
    }
/*Diagonal walls: card-based detection. A diagonal wall is a vertical card whose
      normal has significant X and Z components (not axis-aligned). Draw the card's
      longest XZ span as a light green line. Chunk meshes use the 178 canonical card.*/
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS) continue;
        u16 m=e->modelIndex;
        u16 cm=amCardModel(m);
        if (!amComputeCard(cm)) continue;
        /*Transform card normal to world space by instance rotation.*/
        V3 wn=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]});
        /*Check if card is a diagonal wall: vertical (|ny| small) and non-axis-aligned.*/
        if (vabs(wn.y)>0.7f) continue;/*horizontal = floor/ceiling, not a wall*/
        if (vabs(wn.x)<0.3f || vabs(wn.z)<0.3f) continue;/*axis-aligned, grid handles it*/
        /*Transform card corners to world XZ.*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
        float wx[4],wz[4];
        for (int k=0;k<4;++k) {
            V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z});
            wx[k]=ip.x+v.x; wz[k]=ip.z+v.z;
        }
        /*FoW check: use card center.*/
        float ccx=(wx[0]+wx[1]+wx[2]+wx[3])*0.25f, ccz=(wz[0]+wz[1]+wz[2]+wz[3])*0.25f;
        int ecx=PosGetCellCoordX(ccx),ecz=PosGetCellCoordZ(ccz);
        if (!expl[(u32)ecz*64+(u32)ecx]) continue;
        /*Find longest XZ span.*/
        int bi=0,bj=1; float bd=-1.0f;
        for (int a=0;a<4;++a) for (int b=a+1;b<4;++b) {
            float dx=wx[a]-wx[b],dz=wz[a]-wz[b],d2=dx*dx+dz*dz;
            if (d2>bd){bd=d2;bi=a;bj=b;}
        }
        float x0=amWX(wx[bi]),y0=amWZ(wz[bi]),x1=amWX(wx[bj]),y1=amWZ(wz[bj]);
        amLine(x0,y0,x1,y1,0,220,0,2);
    }
    /*Instance hull outlines: dark green for open height steps only.
      Closed walls are drawn from the grid above.*/
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS) continue;
        u16 m=e->modelIndex;
        if (!amComputeModel(m) || !amMdlWall[m]) continue;
        u8 hn=amHullN[m]; if (hn<3) continue;
        float px[AM_MAXHULL],pz[AM_MAXHULL];
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
        for (u8 k=0;k<hn;++k) { V3 v=quat_rot_v3(iq,(V3){amHullX[m][k]*is.x,0.0f,amHullZ[m][k]*is.z});
            px[k]=amWX(ip.x+v.x); pz[k]=amWZ(ip.z+v.z); }
        for (u8 k=0;k<hn;++k) {
            u8 a=k,b=(u8)((k+1)%hn);
            V3 va=quat_rot_v3(iq,(V3){amHullX[m][a]*is.x,0.0f,amHullZ[m][a]*is.z});
            V3 vb=quat_rot_v3(iq,(V3){amHullX[m][b]*is.x,0.0f,amHullZ[m][b]*is.z});
            float mx=(ip.x+va.x+ip.x+vb.x)*0.5f,mz=(ip.z+va.z+ip.z+vb.z)*0.5f;
            int ecx=PosGetCellCoordX(mx),ecz=PosGetCellCoordZ(mz);/*clamped to 0..63*/
            if (!expl[(u32)ecz*64+(u32)ecx]) continue;/*FoW: only explored portions*/
            if (amEdgeKind(ip.x+va.x,ip.z+va.z,ip.x+vb.x,ip.z+vb.z,lev)) amLine(px[a],pz[a],px[b],pz[b],0,110,0,2);
            /*else: closed wall, already drawn from grid above; skip to avoid double-draw*/
        }
    }
    /*Doors: closed = 2px yellow span; open = green center 4px line + end ticks.
      Double-wide bulkheads (499/509) and large elevator doors (508/510/511):
      outer/end ticks only, two per spanned cell.
      Span = door width axis (longer horizontal local AABB dimension), clamped to
      1 cell (2.56) unless 2-cell door, then clamped to 2 cells (5.12).
      Doors are NOT card chunks; do not use card geometry for them.*/
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (!IdxIsDoor(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS) continue;
        u16 m=e->modelIndex;
        if (!amComputeModel(m)) continue;
        /*Width axis endpoints in local space, clamped to cell size.*/
        bool is2cell=(e->index==499||e->index==508||e->index==509||e->index==510||e->index==511);
        float maxW=is2cell?5.12f:2.56f;
        float lx0,lz0,lx1,lz1;
        if ((amMaxX[m]-amMinX[m]) >= (amMaxZ[m]-amMinZ[m])) {
            float w=amMaxX[m]-amMinX[m]; if (w>maxW) w=maxW;
            float cx=(amMinX[m]+amMaxX[m])*0.5f, cz=(amMinZ[m]+amMaxZ[m])*0.5f;
            lx0=cx-w*0.5f; lz0=cz; lx1=cx+w*0.5f; lz1=cz;
        } else {
            float w=amMaxZ[m]-amMinZ[m]; if (w>maxW) w=maxW;
            float cx=(amMinX[m]+amMaxX[m])*0.5f, cz=(amMinZ[m]+amMaxZ[m])*0.5f;
            lx0=cx; lz0=cz-w*0.5f; lx1=cx; lz1=cz+w*0.5f;
        }
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
        V3 va=quat_rot_v3(iq,(V3){lx0*is.x,0.0f,lz0*is.z});
        V3 vb=quat_rot_v3(iq,(V3){lx1*is.x,0.0f,lz1*is.z});
        int ecx=PosGetCellCoordX(ip.x+(va.x+vb.x)*0.5f),ecz=PosGetCellCoordZ(ip.z+(va.z+vb.z)*0.5f);
        if (!expl[(u32)ecz*64+(u32)ecx]) continue;
        float ax=amWX(ip.x+va.x),ay=amWZ(ip.z+va.z),bx=amWX(ip.x+vb.x),by=amWZ(ip.z+vb.z);
        bool closed=(e->doorState==DoorState_Closed||e->doorState==DoorState_Closing);
        bool bulkhead=(e->index==499||e->index==509);
        if (closed) { amLine(ax,ay,bx,by,255,255,0,2); continue; }
        float dx=bx-ax,dy=by-ay,len=vsqrtf(dx*dx+dy*dy);
        if (len<1e-6f) continue;
        dx/=len; dy/=len; float nx=-dy,ny=dx;/*span dir + perpendicular*/
        float cx=(ax+bx)*0.5f,cy=(ay+by)*0.5f;
        if (!bulkhead) amLine(cx-dx*2.0f,cy-dy*2.0f,cx+dx*2.0f,cy+dy*2.0f,0,220,0,2);/*centered 4px line*/
        amLine(ax-nx*2.0f,ay-ny*2.0f,ax+nx*2.0f,ay+ny*2.0f,0,220,0,2);/*end ticks*/
        amLine(bx-nx*2.0f,by-ny*2.0f,bx+nx*2.0f,by+ny*2.0f,0,220,0,2);
        if (bulkhead) amLine(cx-nx*2.0f,cy-ny*2.0f,cx+nx*2.0f,cy+ny*2.0f,0,220,0,2);/*mid tick: 2 marks per spanned cell*/
    }
    /*Overlays: NPC dots by type (nav-gated), cameras 2x2 red, CPU nodes 10x10 red.*/
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (!(e->entflags&EF_ACTIVE) || e->health<=0.0f) continue;
        int ci=e->index;
        float x=amWX(World.position[i].x),y=amWZ(World.position[i].z);
        if (x<-12.0f||y<-12.0f||x>AM_W+12.0f||y>AM_H+12.0f) continue;
        int ecx=PosGetCellCoordX(World.position[i].x),ecz=PosGetCellCoordZ(World.position[i].z);
        if (!expl[(u32)ecz*64+(u32)ecx]) continue;
        if (IdxIsNPC(ci)) {
            NPCType t=npcTable[ci-419].type;
            if (nav<amNPCNeedVer(t)) continue;
            u8 r,g,b; amNPCColor(t,&r,&g,&b);
            amFillRect((int)(x-5.0f),(int)(y-5.0f),(int)(x+5.0f),(int)(y+5.0f),r,g,b);
        }
        else if (ci==477) amFillRect((int)(x-1.0f),(int)(y-1.0f),(int)(x+1.0f),(int)(y+1.0f),255,0,0);
        else if (ci==478||ci==479) amFillRect((int)(x-5.0f),(int)(y-5.0f),(int)(x+5.0f),(int)(y+5.0f),255,0,0);
    }
    /*Player marker: red arrowhead pointing along facing (like player_0.png).
      Rotated 90deg CW in screen space for correct orientation.*/
    {
        V3 p=World.position[PLAYER1],f=World.instances[PLAYER1].forward;
        float x=amWX(p.x),y=amWZ(p.z);
        float fl=vsqrtf(f.x*f.x+f.z*f.z);
        float dx=0.0f,dy=-1.0f;
        if (fl>1e-4f) { dx=f.x/fl; dy=-f.z/fl; }/*screen: +x right, -z up*/
        float nx=-dy,ny=dx;
        amFillTri(x+dx*8.0f,y+dy*8.0f, x-dx*5.0f+nx*5.0f,y-dy*5.0f+ny*5.0f, x-dx*5.0f-nx*5.0f,y-dy*5.0f-ny*5.0f, 255,32,32);
    }
    /*v3 nav: small and large range rings centered on player (Unity Automap.cs).
      v3 inner=14.08 (5*2.56+1.28), outer=20.48 (7.5*2.56+1.28) world units.*/
    if (nav>=3) {
        V3 p=World.position[PLAYER1];
        float x=amWX(p.x),y=amWZ(p.z);
        amCircle(x,y,14.08f*amPxPerUnit,0,200,200);/*small: cyan*/
        amCircle(x,y,20.48f*amPxPerUnit,0,200,200);/*large: cyan*/
    }
}

/*Fog-of-war reveal around the player; returns true if anything changed.*/
static bool amReveal(void) {
    u8 lev=World.curLev;
    V3 p=World.position[PLAYER1];
    int pcx=PosGetCellCoordX(p.x),pcz=PosGetCellCoordZ(p.z);
    int r=(int)(AM_FOW_RADIUS/CELLSZ)+1;
    bool changed=false;
    for (int dz=-r;dz<=r;++dz) for (int dx=-r;dx<=r;++dx) {
        int cx=pcx+dx,cz=pcz+dz;
        if (cx<0||cx>63||cz<0||cz>63) continue;
        float wx=World.worldMin_x[lev]+cx*CELLSZ-p.x,wz=World.worldMin_z[lev]+cz*CELLSZ-p.z;
        if (wx*wx+wz*wz>AM_FOW_RADIUS2) continue;
        u32 cell=(u32)cz*64+(u32)cx;
        if (!(gridCellStates[cell]&CELL_OPEN)) continue;
        if (!World.automapExplored[lev][cell]) { World.automapExplored[lev][cell]=1; changed=true; }
    }
    return changed;
}

static u32 amDoorHash(void) {
    u32 h=0;
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i];
        if (IdxIsDoor(e->index)) h=h*31u+(u32)(i*7u+(u32)e->doorState);
    }
    return h;
}

/*--- Public API ---*/
void AutomapInitGL(void) {
    if (amReady) return;
    GenerateAndBindTexture(&amTexId,GL_RGBA8,AM_W,AM_H,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL);
    glGenFramebuffers(1,&amFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,amFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,amTexId,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    amReady=true;
}
void AutomapNewGame(void) {
    mset(World.automapExplored,0,sizeof(World.automapExplored));
    World.automapZoom=0; World.automapNextRaster=0.0; amBuiltLev=255;
}
void AutomapOnLoad(void) { amBuiltLev=255; World.automapNextRaster=0.0; }/*force re-raster from loaded FoW*/

void AutomapTick(void) {
    if (!amReady || World.menuActive) return;
    u8 lev=World.curLev;
    if (lev>=LEVEL_CYBERSPACE) return;
    bool fowChanged=amReveal();
    u32 dh=amDoorHash();
    double now=get_time();
    if (fowChanged || dh!=amBuiltDoorHash || lev!=amBuiltLev || World.automapZoom!=amBuiltZoom || now>=World.automapNextRaster) {
        amRaster();
        amBuiltLev=lev; amBuiltDoorHash=dh; amBuiltZoom=World.automapZoom;
        int nav=amNavVer();/*Unity cadence: 0.2s base, 0.1s nav v2, 0.05s nav v3*/
        World.automapNextRaster=now+(nav>2?0.05:(nav>1?0.1:0.2));
        glBindTexture(GL_TEXTURE_2D,amTexId);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,AM_W,AM_H,GL_RGBA,GL_UNSIGNED_BYTE,amPx);
    }
}

void AutomapBlitToUI(void) {
    if (!amReady || World.menuActive || World.paused || Cheats.noHUD) return;
    if (World.curLev>=LEVEL_CYBERSPACE) return;
    int n=0; int dx0[2],dy0[2],dx1[2],dy1[2];
    if (World.Sys_UI.MFD_LefTab==3) { dx0[n]=AMAP_UI_X_L; dy0[n]=AMAP_UI_Y; dx1[n]=AMAP_UI_X_L+AMAP_UI_W; dy1[n]=AMAP_UI_Y+AMAP_UI_H; ++n; }
    if (World.Sys_UI.MFD_RightTab==3) { dx0[n]=AMAP_UI_X_R; dy0[n]=AMAP_UI_Y; dx1[n]=AMAP_UI_X_R+AMAP_UI_W; dy1[n]=AMAP_UI_Y+AMAP_UI_H; ++n; }
    if (!n) return;
    glBindFramebuffer(GL_READ_FRAMEBUFFER,amFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
    for (int k=0;k<n;++k) {
        /*UI y-down -> GL y-up; raster row 0 (north) is texture bottom -> flip src.*/
        int gx0=dx0[k],gy0=768-dy1[k],gx1=dx1[k],gy1=768-dy0[k];
        glBlitFramebuffer(0,AM_H,AM_W,0, gx0,gy0,gx1,gy1, GL_COLOR_BUFFER_BIT,GL_LINEAR);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
}

/*Dump the automap raster to Screenshots/ as a standalone BMP (north up).
Called alongside Screenshot() when an automap tab is open.*/
void AutomapDumpBMP(void) {
    if (!amReady) return;
    if (World.Sys_UI.MFD_LefTab!=3 && World.Sys_UI.MFD_RightTab!=3) return;
    OS_MakeFolder("Screenshots");
    /*amPx row 0 = north; BmpWrite stores row 0 at the bottom, so flip for north-up output.
      Fully transparent pixels become white for readability.*/
    static u8 flip[AM_W*AM_H*4];
    for (int y=0;y<AM_H;++y) {
        u8* dst=flip+(size_t)y*AM_W*4;
        u8* src=amPx+(size_t)(AM_H-1-y)*AM_W*4;
        for (int x=0;x<AM_W;++x) {
            u8* d=dst+(size_t)x*4, *s=src+(size_t)x*4;
            if (s[3]==0) { d[0]=255; d[1]=255; d[2]=255; d[3]=255; }
            else { d[0]=s[0]; d[1]=s[1]; d[2]=s[2]; d[3]=s[3]; }
        }
    }
    char filename[96]; sFormat(filename,sizeof(filename),"Screenshots/automap_%.2f.bmp",get_time());
    BmpWrite(filename,AM_W,AM_H,flip);
    CenterStatusPrint("Saved automap %s\n",filename);
}
