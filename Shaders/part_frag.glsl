// part_frag.glsl - Particle Fragment Shader, with animated texture and soft blend support
in vec2 vUV;
in vec4 vColor;
flat in uint vTex;
layout(location=0) out vec4 outColor;
layout(location=5) uniform sampler2D sceneDepth;
layout(location=6) uniform vec2 screenSize;
layout(location=7) uniform float softScale;
layout(location=8) uniform vec2 projParams; // x=znear,y=zfar
layout(std430,binding=8) buffer TexPal { uint texPal[]; };
layout(std430,binding=9) buffer TexPalOfs { uint texPalOfs[]; };
layout(std430,binding=12) buffer ColBuf { uint colors[]; };
layout(std430,binding=14) buffer TexOfs { uint texOfs[]; };
layout(std430,binding=15) buffer TexSzs { ivec2 texSzs[]; };
const vec4 BTFRAC = vec4(1.0/255.0);
vec4 getTexColor(uint txi,ivec2 uv,int sx) { uint po=texOfs[txi]+uint(uv.y)*uint(sx)+uint(uv.x); uint slot=po>>2u,lo=po&3u; uint pi=(colors[slot]>>(lo<<3u))&0xFFu; uint c=texPal[texPalOfs[txi]+pi]; return vec4(c&0xFFu,(c>>8)&0xFFu,(c>>16)&0xFFu,c>>24)*BTFRAC; }
void main() {
    ivec2 ts=texSzs[vTex]; int sx=ts.x;
    ivec2 uv=ivec2(int(floor(vUV.x*float(sx))),int(floor(vUV.y*float(ts.y))));
    uv.x%=sx; uv.y%=ts.y;
    vec4 tc=getTexColor(vTex,uv,sx);
    vec4 col=tc*vColor;
    if(col.a<0.02)discard;
    if(softScale>0.0) {
        vec2 ssc=gl_FragCoord.xy/screenSize;
        float sd=texture(sceneDepth,ssc).r;
        float ld=gl_FragCoord.z;
        float ln=projParams.x,lf=projParams.y,nf2=2.0*ln*lf,df=lf-ln;
        float ldd=nf2/(lf+ln-ld*df);
        float sdd=nf2/(lf+ln-sd*df);
        col.a*=clamp((sdd-ldd)/softScale,0.0,1.0);
    }
    outColor=col;
}
