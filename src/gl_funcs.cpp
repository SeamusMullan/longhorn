#include <longhorn/gl_funcs.hpp>

#define LOAD(name) name = reinterpret_cast<decltype(name)>(SDL_GL_GetProcAddress(#name))

PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;

PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;

PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;

PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;

PFNGLACTIVETEXTUREPROC glActiveTexture_ = nullptr;

bool load_gl_functions() {
    LOAD(glCreateShader);
    LOAD(glShaderSource);
    LOAD(glCompileShader);
    LOAD(glGetShaderiv);
    LOAD(glGetShaderInfoLog);
    LOAD(glDeleteShader);

    LOAD(glCreateProgram);
    LOAD(glAttachShader);
    LOAD(glLinkProgram);
    LOAD(glGetProgramiv);
    LOAD(glGetProgramInfoLog);
    LOAD(glDeleteProgram);
    LOAD(glUseProgram);

    LOAD(glGetUniformLocation);
    LOAD(glUniform1f);
    LOAD(glUniform1i);
    LOAD(glUniform2fv);
    LOAD(glUniform4fv);

    LOAD(glGenVertexArrays);
    LOAD(glBindVertexArray);
    LOAD(glDeleteVertexArrays);

    LOAD(glGenBuffers);
    LOAD(glBindBuffer);
    LOAD(glBufferData);
    LOAD(glDeleteBuffers);

    LOAD(glEnableVertexAttribArray);
    LOAD(glVertexAttribPointer);

    glActiveTexture_ = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(SDL_GL_GetProcAddress("glActiveTexture"));

    return glCreateShader && glCreateProgram && glGenVertexArrays && glGenBuffers;
}
