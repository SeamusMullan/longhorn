#pragma once

// GL 3.3 Core function pointers loaded via SDL_GL_GetProcAddress

#include <cstddef>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

// Types not in SDL_opengl.h
typedef char GLchar;
typedef std::ptrdiff_t GLsizeiptr;

// Shader
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;

// Program
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;

// Uniforms
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;

// VAO
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

// VBO
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;

// Vertex attribs
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

// Texture (glActiveTexture is GL 1.3+ but needs loading on some platforms)
extern PFNGLACTIVETEXTUREPROC glActiveTexture_;

bool load_gl_functions();
