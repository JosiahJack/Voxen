// gl.h - Combined gl,glad,glfw3.5 declarations
#pragma once
#define GL_ARRAY_BUFFER 0x8892
#define GL_BLEND 0x0BE2
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_DEPTH_TEST 0x0B71
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_FALSE 0
#define GL_FLOAT 0x1406
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_WRITE 0x88BA
#define GL_RGB 0x1907
#define GL_RG16F 0x822F
#define GL_RGB16F 0x881B
#define GL_RGBA 0x1908
#define GL_RGBA32F 0x8814
#define GL_RGBA8 0x8058
#define GL_SSBO 0x90D2 // Formerly GL_SHADER_STORAGE_BUFFER
#define GL_STATIC_DRAW 0x88E4
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE_2D 0x0DE1
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
typedef void(*PFNGLACTIVETEXTURE)(u32),(*PFNGLATTACHSHADER)(u32,u32),(*PFNGLBINDBUFFER)(u32,u32),(*PFNGLBINDBUFFERBASE)(u32,u32,u32),(*PFNGLBINDFRAMEBUFFER)(u32,u32);
typedef void(*PFNGLBINDIMAGETEXTURE)(u32,u32,i32,bool,i32,u32,u32),(*PFNGLBINDTEXTURE)(u32,u32),(*PFNGLBINDTEXTUREUNIT)(u32,u32),(*PFNGLBINDVERTEXARRAY)(u32),(*PFNGLBUFFERSUBDATA)(u32,intptr_t,intptr_t,const void*);
typedef void(*PFNGLBINDVERTEXBUFFER)(u32,u32,intptr_t,i32),(*PFNGLBLENDFUNCSEPARATE)(u32,u32,u32,u32),(*PFNGLBUFFERDATA)(u32,size_t,const void*,u32),(*PFNGLCLEAR)(u32);
typedef void(*PFNGLCLEARCOLOR)(float,float,float,float),(*PFNGLCOLORMASK)(bool,bool,bool,bool),(*PFNGLCOMPILESHADER)(u32),(*PFNGLCOPYTEXSUBIMAGE2D)(u32,i32,i32,i32,i32,i32,i32,i32);
typedef void(*PFNGLCREATEBUFFERS)(i32,u32*),(*PFNGLGENVERTEXARRAYS)(i32,u32*),(*PFNGLCULLFACE)(u32),(*PFNGLDEPTHFUNC)(u32),(*PFNGLDEPTHMASK)(bool);
typedef void(*PFNGLDISABLE)(u32),(*PFNGLDISPATCHCOMPUTE)(u32,u32,u32),(*PFNGLDRAWARRAYS)(u32,i32,i32),(*PFNGLDRAWBUFFERS)(i32,const u32*),(*PFNGLDRAWELEMENTS)(u32,i32,u32,const void*);
typedef void(*PFNGLENABLE)(u32),(*PFNGLFINISH)(void),(*PFNGLFLUSH)(void),(*PFNGLFRAMEBUFFERTEXTURE2D)(u32,u32,u32,u32,i32),(*PFNGLFRONTFACE)(u32);
typedef void(*PFNGLGENBUFFERS)(i32,u32*),(*PFNGLGENFRAMEBUFFERS)(i32,u32*),(*PFNGLGENTEXTURES)(i32,u32*),(*PFNGLGETINTEGERV)(u32,i32*),(*PFNGLGETPROGRAMIV)(u32,u32,i32*);
typedef void(*PFNGLGETSHADERINFOLOG)(u32,i32,i32*,char*),(*PFNGLGETSHADERIV)(u32,u32,i32*),(*PFNGLLINEWIDTH)(float),(*PFNGLLINKPROGRAM)(u32),(*PFNGLSHADERSOURCE)(u32,i32,const char*const*,const i32*);
typedef void(*PFNGLPUSHDEBUGGROUP)(u32,u32,i32,const char*),(*PFNGLREADBUFFER)(u32),(*PFNGLREADPIXELS)(i32,i32,i32,i32,u32,u32,void*),(*PFNGLSHADERBINARY)(i32,const u32*,u32,const void*,i32);
typedef void(*PFNGLSHADERSTORAGEBLOCKBINDING)(u32,u32,u32),(*PFNGLTEXIMAGE2D)(u32,i32,i32,i32,i32,i32,u32,u32,const void*),(*PFNGLTEXPARAMETERI)(u32,u32,i32),(*PFNGLTEXTUREVIEW)(u32,u32,u32,u32,u32,u32,u32,u32);
typedef void(*PFNGLUNIFORM1F)(i32,float),(*PFNGLUNIFORM1I)(i32,i32),(*PFNGLUNIFORM1UI)(i32,u32),(*PFNGLUNIFORM2F)(i32,float,float),(*PFNGLUNIFORM2UI)(i32,u32,u32),(*PFNGLENABLEVERTEXATTRIBARRAY)(u32);
typedef void(*PFNGLUNIFORM3F)(i32,float,float,float),(*PFNGLUNIFORM4F)(i32,float,float,float,float),(*PFNGLUNIFORMMATRIX3FV)(i32,i32,bool,const float*),(*PFNGLVERTEXATTRIBBINDING)(u32,u32);
typedef void(*PFNGLUNIFORMMATRIX4FV)(i32,i32,bool,const float*),(*PFNGLUSEPROGRAM)(u32);
typedef void(*PFNGLVIEWPORT)(i32,i32,i32,i32),(*PFNGLCLEARBUFFERFV)(u32,i32,const float*),(*PFNGLVERTEXATTRIBFORMAT)(u32,i32,u32,bool,u32);
typedef u32(*PFNGLCHECKFRAMEBUFFERSTATUS)(u32),(*PFNGLCREATEPROGRAM)(void),(*PFNGLCREATESHADER)(u32),(*PFNGLGETERROR)(void);
typedef void*(*PFNGLMAPBUFFERRANGE)(u32,intptr_t,size_t,u32); typedef bool(*PFNGLUNMAPBUFFER)(u32);
        PFNGLACTIVETEXTURE glActiveTexture;       PFNGLATTACHSHADER glAttachShader;                         PFNGLBINDBUFFER glBindBuffer;
      PFNGLBINDBUFFERBASE glBindBufferBase; PFNGLBINDFRAMEBUFFER glBindFramebuffer;             PFNGLBINDIMAGETEXTURE glBindImageTexture;
            PFNGLBINDTEXTURE glBindTexture; PFNGLBINDVERTEXARRAY glBindVertexArray;             PFNGLBINDVERTEXBUFFER glBindVertexBuffer;
PFNGLBLENDFUNCSEPARATE glBlendFuncSeparate;           PFNGLBUFFERDATA glBufferData; PFNGLCHECKFRAMEBUFFERSTATUS glCheckFramebufferStatus;
                        PFNGLCLEAR glClear;             PFNGLCOLORMASK glColorMask;                   PFNGLCOMPILESHADER glCompileShader;
        PFNGLCREATEPROGRAM glCreateProgram;       PFNGLCREATESHADER glCreateShader;               PFNGLGENVERTEXARRAYS glGenVertexArrays;
                PFNGLDEPTHFUNC glDepthFunc;             PFNGLDEPTHMASK glDepthMask;                               PFNGLDISABLE glDisable;
    PFNGLDISPATCHCOMPUTE glDispatchCompute;           PFNGLDRAWARRAYS glDrawArrays;                       PFNGLDRAWBUFFERS glDrawBuffers;
          PFNGLDRAWELEMENTS glDrawElements;                   PFNGLENABLE glEnable;                                 PFNGLFINISH glFinish;
                        PFNGLFLUSH glFlush;             PFNGLFRONTFACE glFrontFace;     PFNGLFRAMEBUFFERTEXTURE2D glFramebufferTexture2D;
                PFNGLFRONTFACE glFrontFace;           PFNGLGENBUFFERS glGenBuffers;               PFNGLGENFRAMEBUFFERS glGenFramebuffers;
            PFNGLGENTEXTURES glGenTextures;               PFNGLGETERROR glGetError;                     PFNGLGETPROGRAMIV glGetProgramiv;
  PFNGLGETSHADERINFOLOG glGetShaderInfoLog;         PFNGLGETSHADERIV glGetShaderiv;                           PFNGLLINEWIDTH glLineWidth;
            PFNGLLINKPROGRAM glLinkProgram;   PFNGLMAPBUFFERRANGE glMapBufferRange;                 PFNGLPUSHDEBUGGROUP glPushDebugGroup;
              PFNGLREADBUFFER glReadBuffer;           PFNGLREADPIXELS glReadPixels;                     PFNGLSHADERBINARY glShaderBinary;
          PFNGLSHADERSOURCE glShaderSource;           PFNGLTEXIMAGE2D glTexImage2D;                   PFNGLTEXPARAMETERI glTexParameteri;
PFNGLCOPYTEXSUBIMAGE2D glCopyTexSubImage2D;         PFNGLTEXTUREVIEW glTextureView;                           PFNGLUNIFORM1F glUniform1f;
                PFNGLUNIFORM1I glUniform1i;           PFNGLUNIFORM1UI glUniform1ui;                           PFNGLUNIFORM2F glUniform2f;
                PFNGLUNIFORM3F glUniform3f;             PFNGLUNIFORM4F glUniform4f;             PFNGLUNIFORMMATRIX3FV glUniformMatrix3fv;
  PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv;         PFNGLUNMAPBUFFER glUnmapBuffer;                         PFNGLUSEPROGRAM glUseProgram;
                  PFNGLVIEWPORT glViewport;         PFNGLGETINTEGERV glGetIntegerv;                         PFNGLUNIFORM2UI glUniform2ui;
              PFNGLCLEARCOLOR glClearColor;     PFNGLCLEARBUFFERFV glClearBufferFv;         PFNGLVERTEXATTRIBFORMAT glVertexAttribFormat;
  PFNGLBINDVERTEXBUFFER glBindVertexBuffer;     PFNGLBUFFERSUBDATA glBufferSubData;       PFNGLVERTEXATTRIBBINDING glVertexAttribBinding;
PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;                       PFNGLSHADERSTORAGEBLOCKBINDING glShaderStorageBlockBinding;
#define GLFW_RELEASE                0
#define GLFW_PRESS                  1
#define GLFW_REPEAT                 2
#define GLFW_HAT_CENTERED           0
#define GLFW_HAT_UP                 1
#define GLFW_HAT_RIGHT              2
#define GLFW_HAT_DOWN               4
#define GLFW_HAT_LEFT               8
#define GLFW_HAT_RIGHT_UP           (GLFW_HAT_RIGHT | GLFW_HAT_UP)
#define GLFW_HAT_RIGHT_DOWN         (GLFW_HAT_RIGHT | GLFW_HAT_DOWN)
#define GLFW_HAT_LEFT_UP            (GLFW_HAT_LEFT  | GLFW_HAT_UP)
#define GLFW_HAT_LEFT_DOWN          (GLFW_HAT_LEFT  | GLFW_HAT_DOWN)
#define GLFW_KEY_UNKNOWN            -1
#define GLFW_KEY_SPACE              32
#define GLFW_KEY_APOSTROPHE         39  /* ' */
#define GLFW_KEY_COMMA              44  /* , */
#define GLFW_KEY_MINUS              45  /* - */
#define GLFW_KEY_PERIOD             46  /* . */
#define GLFW_KEY_SLASH              47  /* / */
#define GLFW_KEY_0                  48
#define GLFW_KEY_1                  49
#define GLFW_KEY_2                  50
#define GLFW_KEY_3                  51
#define GLFW_KEY_4                  52
#define GLFW_KEY_5                  53
#define GLFW_KEY_6                  54
#define GLFW_KEY_7                  55
#define GLFW_KEY_8                  56
#define GLFW_KEY_9                  57
#define GLFW_KEY_SEMICOLON          59  /* ; */
#define GLFW_KEY_EQUAL              61  /* = */
#define GLFW_KEY_A                  65
#define GLFW_KEY_B                  66
#define GLFW_KEY_C                  67
#define GLFW_KEY_D                  68
#define GLFW_KEY_E                  69
#define GLFW_KEY_F                  70
#define GLFW_KEY_G                  71
#define GLFW_KEY_H                  72
#define GLFW_KEY_I                  73
#define GLFW_KEY_J                  74
#define GLFW_KEY_K                  75
#define GLFW_KEY_L                  76
#define GLFW_KEY_M                  77
#define GLFW_KEY_N                  78
#define GLFW_KEY_O                  79
#define GLFW_KEY_P                  80
#define GLFW_KEY_Q                  81
#define GLFW_KEY_R                  82
#define GLFW_KEY_S                  83
#define GLFW_KEY_T                  84
#define GLFW_KEY_U                  85
#define GLFW_KEY_V                  86
#define GLFW_KEY_W                  87
#define GLFW_KEY_X                  88
#define GLFW_KEY_Y                  89
#define GLFW_KEY_Z                  90
#define GLFW_KEY_LEFT_BRACKET       91  /* [ */
#define GLFW_KEY_BACKSLASH          92  /* \ */
#define GLFW_KEY_RIGHT_BRACKET      93  /* ] */
#define GLFW_KEY_GRAVE_ACCENT       96  /* ` */
#define GLFW_KEY_ESCAPE             256
#define GLFW_KEY_ENTER              257
#define GLFW_KEY_TAB                258
#define GLFW_KEY_BACKSPACE          259
#define GLFW_KEY_INSERT             260
#define GLFW_KEY_DELETE             261
#define GLFW_KEY_RIGHT              262
#define GLFW_KEY_LEFT               263
#define GLFW_KEY_DOWN               264
#define GLFW_KEY_UP                 265
#define GLFW_KEY_PAGE_UP            266
#define GLFW_KEY_PAGE_DOWN          267
#define GLFW_KEY_HOME               268
#define GLFW_KEY_END                269
#define GLFW_KEY_CAPS_LOCK          280
#define GLFW_KEY_SCROLL_LOCK        281
#define GLFW_KEY_NUM_LOCK           282
#define GLFW_KEY_PRINT_SCREEN       283
#define GLFW_KEY_PAUSE              284
#define GLFW_KEY_F1                 290
#define GLFW_KEY_F2                 291
#define GLFW_KEY_F3                 292
#define GLFW_KEY_F4                 293
#define GLFW_KEY_F5                 294
#define GLFW_KEY_F6                 295
#define GLFW_KEY_F7                 296
#define GLFW_KEY_F8                 297
#define GLFW_KEY_F9                 298
#define GLFW_KEY_F10                299
#define GLFW_KEY_F11                300
#define GLFW_KEY_F12                301
#define GLFW_KEY_KP_0               320
#define GLFW_KEY_KP_1               321
#define GLFW_KEY_KP_2               322
#define GLFW_KEY_KP_3               323
#define GLFW_KEY_KP_4               324
#define GLFW_KEY_KP_5               325
#define GLFW_KEY_KP_6               326
#define GLFW_KEY_KP_7               327
#define GLFW_KEY_KP_8               328
#define GLFW_KEY_KP_9               329
#define GLFW_KEY_KP_DECIMAL         330
#define GLFW_KEY_KP_DIVIDE          331
#define GLFW_KEY_KP_MULTIPLY        332
#define GLFW_KEY_KP_SUBTRACT        333
#define GLFW_KEY_KP_ADD             334
#define GLFW_KEY_KP_ENTER           335
#define GLFW_KEY_KP_EQUAL           336
#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341
#define GLFW_KEY_LEFT_ALT           342
#define GLFW_KEY_LEFT_SUPER         343
#define GLFW_KEY_RIGHT_SHIFT        344
#define GLFW_KEY_RIGHT_CONTROL      345
#define GLFW_KEY_RIGHT_ALT          346
#define GLFW_KEY_RIGHT_SUPER        347
#define GLFW_KEY_MENU               348
#define GLFW_KEY_LAST               GLFW_KEY_MENU
#define GLFW_MOUSE_BUTTON_1         0
#define GLFW_MOUSE_BUTTON_2         1
#define GLFW_MOUSE_BUTTON_3         2
#define GLFW_MOUSE_BUTTON_4         3
#define GLFW_MOUSE_BUTTON_5         4
#define GLFW_MOUSE_BUTTON_6         5
#define GLFW_MOUSE_BUTTON_7         6
#define GLFW_MOUSE_BUTTON_8         7
#define GLFW_MOUSE_BUTTON_LAST      GLFW_MOUSE_BUTTON_8
#define GLFW_MOUSE_BUTTON_LEFT      GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT     GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE    GLFW_MOUSE_BUTTON_3
#define GLFW_JOYSTICK_1             0
#define GLFW_JOYSTICK_2             1
#define GLFW_JOYSTICK_3             2
#define GLFW_JOYSTICK_4             3
#define GLFW_JOYSTICK_5             4
#define GLFW_JOYSTICK_6             5
#define GLFW_JOYSTICK_7             6
#define GLFW_JOYSTICK_8             7
#define GLFW_JOYSTICK_9             8
#define GLFW_JOYSTICK_10            9
#define GLFW_JOYSTICK_11            10
#define GLFW_JOYSTICK_12            11
#define GLFW_JOYSTICK_13            12
#define GLFW_JOYSTICK_14            13
#define GLFW_JOYSTICK_15            14
#define GLFW_JOYSTICK_16            15
#define GLFW_JOYSTICK_LAST          GLFW_JOYSTICK_16
#define GLFW_DONT_CARE              -1
typedef void (*GLFWglproc)(void);
typedef struct GLFWmonitor GLFWmonitor; typedef struct GLFWwindow GLFWwindow;
typedef struct GLFWvidmode { int width,height,redBits,greenBits,blueBits,refreshRate; } GLFWvidmode;
typedef struct GLFWimage { int width,height; unsigned char* pixels; } GLFWimage;
typedef struct GLFWgamepadstate { unsigned char buttons[15]; float axes[6]; } GLFWgamepadstate;
