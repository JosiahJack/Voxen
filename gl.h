// gl.h - Combined gl,glad,glfw3.5 declarations
#pragma once
typedef void (*GLADapiproc)(void);
typedef GLADapiproc (*GLADloadfunc)(const char *name);
typedef GLADapiproc (*GLADuserptrloadfunc)(void *userptr, const char *name);
#define GL_ARRAY_BUFFER 0x8892
#define GL_BLEND 0x0BE2
#define GL_CCW 0x0901
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
#define GL_DYNAMIC_STORAGE_BIT 0x0100
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_EQUAL 0x0202
#define GL_FALSE 0
#define GL_FLOAT 0x1406
#define GL_FRAMEBUFFER 0x8D40
#define GL_NEAREST 0x2600
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
#define GL_TRIANGLES 0x0004
#define GL_HALF_FLOAT 0x140B
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
typedef unsigned int GLenum,GLbitfield; typedef unsigned char GLboolean; typedef int GLint;
typedef u32 GLuint; typedef int GLsizei; typedef float GLfloat; typedef u8 GLubyte;
typedef char GLchar; typedef intptr_t GLintptr; typedef size_t GLsizeiptr;
typedef void (  *PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (  *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (  *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (  *PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
typedef void (  *PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void (  *PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (  *PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (  *PFNGLBINDTEXTUREUNITPROC)(GLuint unit, GLuint texture);
typedef void (  *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (  *PFNGLBINDVERTEXBUFFERPROC)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (  *PFNGLBLENDFUNCSEPARATEPROC)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (  *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
typedef GLenum (  *PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
typedef void (  *PFNGLCLEARPROC)(GLbitfield mask);
typedef void (  *PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (  *PFNGLCOLORMASKPROC)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (  *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (  *PFNGLCOPYTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (  *PFNGLCREATEBUFFERSPROC)(GLsizei n, GLuint * buffers);
typedef GLuint (  *PFNGLCREATEPROGRAMPROC)(void);
typedef GLuint (  *PFNGLCREATESHADERPROC)(GLenum type);
typedef void (  *PFNGLCREATETEXTURESPROC)(GLenum target, GLsizei n, GLuint * textures);
typedef void (  *PFNGLCREATEVERTEXARRAYSPROC)(GLsizei n, GLuint * arrays);
typedef void (  *PFNGLCULLFACEPROC)(GLenum mode);
typedef void (  *PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void (  *PFNGLDEPTHMASKPROC)(GLboolean flag);
typedef void (  *PFNGLDISABLEPROC)(GLenum cap);
typedef void (  *PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (  *PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void (  *PFNGLDRAWBUFFERSPROC)(GLsizei n, const GLenum * bufs);
typedef void (  *PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void * indices);
typedef void (  *PFNGLENABLEPROC)(GLenum cap);
typedef void (  *PFNGLENABLEVERTEXARRAYATTRIBPROC)(GLuint vaobj, GLuint index);
typedef void (  *PFNGLFINISHPROC)(void);
typedef void (  *PFNGLFLUSHPROC)(void);
typedef void (  *PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (  *PFNGLFRONTFACEPROC)(GLenum mode);
typedef void (  *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint * buffers);
typedef void (  *PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint * framebuffers);
typedef void (  *PFNGLGENTEXTURESPROC)(GLsizei n, GLuint * textures);
typedef GLenum (  *PFNGLGETERRORPROC)(void);
typedef void (  *PFNGLGETINTEGERVPROC)(GLenum pname, GLint * data);
typedef void (  *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint * params);
typedef void (  *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
typedef void (  *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint * params);
typedef const GLubyte * (  *PFNGLGETSTRINGPROC)(GLenum name);
typedef const GLubyte * (  *PFNGLGETSTRINGIPROC)(GLenum name, GLuint index);
typedef void (  *PFNGLLINEWIDTHPROC)(GLfloat width);
typedef void (  *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void * (  *PFNGLMAPBUFFERRANGEPROC)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (  *PFNGLNAMEDBUFFERDATAPROC)(GLuint buffer, GLsizeiptr size, const void * data, GLenum usage);
typedef void (  *PFNGLNAMEDBUFFERSTORAGEPROC)(GLuint buffer, GLsizeiptr size, const void * data, GLbitfield flags);
typedef void (  *PFNGLNAMEDBUFFERSUBDATAPROC)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void * data);
typedef void (  *PFNGLPUSHDEBUGGROUPPROC)(GLenum source, GLuint id, GLsizei length, const GLchar * message);
typedef void (  *PFNGLREADBUFFERPROC)(GLenum src);
typedef void (  *PFNGLREADPIXELSPROC)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels);
typedef void (  *PFNGLSHADERBINARYPROC)(GLsizei count, const GLuint * shaders, GLenum binaryFormat, const void * binary, GLsizei length);
typedef void (  *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
typedef void (  *PFNGLSHADERSTORAGEBLOCKBINDINGPROC)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
typedef void (  *PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
typedef void (  *PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (  *PFNGLTEXTUREPARAMETERIPROC)(GLuint texture, GLenum pname, GLint param);
typedef void (  *PFNGLTEXTURESTORAGE2DPROC)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (  *PFNGLTEXTURESUBIMAGE2DPROC)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
typedef void (  *PFNGLTEXTUREVIEWPROC)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
typedef void (  *PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void (  *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void (  *PFNGLUNIFORM1UIPROC)(GLint location, GLuint v0);
typedef void (  *PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef void (  *PFNGLUNIFORM2UIPROC)(GLint location, GLuint v0, GLuint v1);
typedef void (  *PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (  *PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (  *PFNGLUNIFORMMATRIX3FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (  *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef GLboolean (  *PFNGLUNMAPBUFFERPROC)(GLenum target);
typedef void (  *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (  *PFNGLVERTEXARRAYATTRIBBINDINGPROC)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
typedef void (  *PFNGLVERTEXARRAYATTRIBFORMATPROC)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (  *PFNGLVERTEXARRAYVERTEXBUFFERPROC)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (  *PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (  *PFNGLCLEARBUFFERFVPROC)(GLenum buffer, GLint drawbuffer, const GLfloat * value);
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLCLEARPROC glClear;
PFNGLCOLORMASKPROC glColorMask;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
PFNGLDEPTHFUNCPROC glDepthFunc;
PFNGLDEPTHMASKPROC glDepthMask;
PFNGLDISABLEPROC glDisable;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLENABLEPROC glEnable;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
PFNGLFINISHPROC glFinish;
PFNGLFLUSHPROC glFlush;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRONTFACEPROC glFrontFace;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLGETERRORPROC glGetError;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLLINEWIDTHPROC glLineWidth;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;
PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup;
PFNGLREADBUFFERPROC glReadBuffer;
PFNGLREADPIXELSPROC glReadPixels;
PFNGLSHADERBINARYPROC glShaderBinary;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLTEXPARAMETERIPROC glTexParameteri;
PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
PFNGLTEXTUREVIEWPROC glTextureView;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
PFNGLVIEWPORTPROC glViewport;
PFNGLGETINTEGERVPROC glGetIntegerv;
PFNGLUNIFORM2UIPROC glUniform2ui;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARBUFFERFVPROC glClearBufferFv;
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
#define GLFW_KEY_WORLD_1            161 /* non-US #1 */
#define GLFW_KEY_WORLD_2            162 /* non-US #2 */
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
#define GLFW_KEY_F13                302
#define GLFW_KEY_F14                303
#define GLFW_KEY_F15                304
#define GLFW_KEY_F16                305
#define GLFW_KEY_F17                306
#define GLFW_KEY_F18                307
#define GLFW_KEY_F19                308
#define GLFW_KEY_F20                309
#define GLFW_KEY_F21                310
#define GLFW_KEY_F22                311
#define GLFW_KEY_F23                312
#define GLFW_KEY_F24                313
#define GLFW_KEY_F25                314
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
#define GLFW_MOD_SHIFT           0x0001
#define GLFW_MOD_CONTROL         0x0002
#define GLFW_MOD_ALT             0x0004
#define GLFW_MOD_SUPER           0x0008
#define GLFW_MOD_CAPS_LOCK       0x0010
#define GLFW_MOD_NUM_LOCK        0x0020
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
#define GLFW_GAMEPAD_BUTTON_A               0
#define GLFW_GAMEPAD_BUTTON_B               1
#define GLFW_GAMEPAD_BUTTON_X               2
#define GLFW_GAMEPAD_BUTTON_Y               3
#define GLFW_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define GLFW_GAMEPAD_BUTTON_BACK            6
#define GLFW_GAMEPAD_BUTTON_START           7
#define GLFW_GAMEPAD_BUTTON_GUIDE           8
#define GLFW_GAMEPAD_BUTTON_LEFT_THUMB      9
#define GLFW_GAMEPAD_BUTTON_RIGHT_THUMB     10
#define GLFW_GAMEPAD_BUTTON_DPAD_UP         11
#define GLFW_GAMEPAD_BUTTON_DPAD_RIGHT      12
#define GLFW_GAMEPAD_BUTTON_DPAD_DOWN       13
#define GLFW_GAMEPAD_BUTTON_DPAD_LEFT       14
#define GLFW_GAMEPAD_BUTTON_LAST            GLFW_GAMEPAD_BUTTON_DPAD_LEFT
#define GLFW_GAMEPAD_BUTTON_CROSS       GLFW_GAMEPAD_BUTTON_A
#define GLFW_GAMEPAD_BUTTON_CIRCLE      GLFW_GAMEPAD_BUTTON_B
#define GLFW_GAMEPAD_BUTTON_SQUARE      GLFW_GAMEPAD_BUTTON_X
#define GLFW_GAMEPAD_BUTTON_TRIANGLE    GLFW_GAMEPAD_BUTTON_Y
#define GLFW_GAMEPAD_AXIS_LEFT_X        0
#define GLFW_GAMEPAD_AXIS_LEFT_Y        1
#define GLFW_GAMEPAD_AXIS_RIGHT_X       2
#define GLFW_GAMEPAD_AXIS_RIGHT_Y       3
#define GLFW_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define GLFW_GAMEPAD_AXIS_LAST          GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
#define GLFW_ANY_POSITION           0x80000000
#define GLFW_DONT_CARE              -1
typedef void (*GLFWglproc)(void);
typedef struct GLFWmonitor GLFWmonitor;
typedef struct GLFWwindow GLFWwindow;
typedef void (* GLFWwindowfocusfun)(GLFWwindow* window, int focused);
typedef void (* GLFWframebuffersizefun)(GLFWwindow* window, int width, int height);
typedef void (* GLFWmonitorfun)(GLFWmonitor* monitor, int event);
typedef void (* GLFWjoystickfun)(int jid, int event);
typedef struct GLFWvidmode { int width,height,redBits,greenBits,blueBits,refreshRate; } GLFWvidmode;
typedef struct GLFWimage { int width,height; unsigned char* pixels; } GLFWimage;
typedef struct GLFWgamepadstate { unsigned char buttons[15]; float axes[6]; } GLFWgamepadstate;
