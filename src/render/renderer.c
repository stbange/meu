#include "render/renderer.h"
#include "core/matrix.h"
#include <SDL3/SDL.h>
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#define pCreateShader glCreateShader
#define pShaderSource glShaderSource
#define pCompileShader glCompileShader
#define pGetShaderiv glGetShaderiv
#define pGetShaderInfoLog glGetShaderInfoLog
#define pDeleteShader glDeleteShader
#define pCreateProgram glCreateProgram
#define pAttachShader glAttachShader
#define pLinkProgram glLinkProgram
#define pGetProgramiv glGetProgramiv
#define pGetProgramInfoLog glGetProgramInfoLog
#define pDeleteProgram glDeleteProgram
#define pUseProgram glUseProgram
#define pGetUniformLocation glGetUniformLocation
#define pUniformMatrix4fv glUniformMatrix4fv
#define pUniform3f glUniform3f
#define pGenVertexArrays glGenVertexArrays
#define pBindVertexArray glBindVertexArray
#define pDeleteVertexArrays glDeleteVertexArrays
#define pGenBuffers glGenBuffers
#define pBindBuffer glBindBuffer
#define pBufferData glBufferData
#define pDeleteBuffers glDeleteBuffers
#define pEnableVertexAttribArray glEnableVertexAttribArray
#define pVertexAttribPointer glVertexAttribPointer
#else
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#define GL_PROCEDURES(X) \
 X(CreateShader, PFNGLCREATESHADERPROC) X(ShaderSource, PFNGLSHADERSOURCEPROC) \
 X(CompileShader, PFNGLCOMPILESHADERPROC) X(GetShaderiv, PFNGLGETSHADERIVPROC) \
 X(GetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC) X(DeleteShader, PFNGLDELETESHADERPROC) \
 X(CreateProgram, PFNGLCREATEPROGRAMPROC) X(AttachShader, PFNGLATTACHSHADERPROC) \
 X(LinkProgram, PFNGLLINKPROGRAMPROC) X(GetProgramiv, PFNGLGETPROGRAMIVPROC) \
 X(GetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC) X(DeleteProgram, PFNGLDELETEPROGRAMPROC) \
 X(UseProgram, PFNGLUSEPROGRAMPROC) X(GetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC) \
 X(UniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC) X(Uniform3f, PFNGLUNIFORM3FPROC) \
 X(GenVertexArrays, PFNGLGENVERTEXARRAYSPROC) X(BindVertexArray, PFNGLBINDVERTEXARRAYPROC) \
 X(DeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC) X(GenBuffers, PFNGLGENBUFFERSPROC) \
 X(BindBuffer, PFNGLBINDBUFFERPROC) X(BufferData, PFNGLBUFFERDATAPROC) \
 X(DeleteBuffers, PFNGLDELETEBUFFERSPROC) X(EnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC) \
 X(VertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC)
#define DECLARE(name, type) static type p##name;
GL_PROCEDURES(DECLARE)
#undef DECLARE
#endif
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct { float position[3], color[3]; } Vertex;
static Vertex vertices[24000];
static unsigned vertex_count;
static GLuint program, buffer, vertex_array;
static GLint matrix_uniform, eye_uniform;

static void box(float x, float y, float z, float w, float h, float d, float r, float g, float b)
{
    static const unsigned corners[36] = {
        0,1,2,0,2,3, 5,4,7,5,7,6, 4,0,3,4,3,7,
        1,5,6,1,6,2, 3,2,6,3,6,7, 4,5,1,4,1,0
    };
    static const float shade[6] = {0.82f,0.72f,0.62f,0.88f,1.0f,0.5f};
    if (vertex_count + 36 > sizeof(vertices) / sizeof(vertices[0])) {
        SDL_Log("Scene geometry budget exceeded");
        abort();
    }
    for (unsigned i = 0; i < 36; ++i) {
        unsigned c = corners[i];
        Vertex v = {
            {x + ((c == 1 || c == 2 || c == 5 || c == 6) ? w : 0),
             y + ((c == 2 || c == 3 || c == 6 || c == 7) ? h : 0),
             z + ((c >= 4) ? d : 0)},
            {r * shade[i / 6], g * shade[i / 6], b * shade[i / 6]}
        };
        vertices[vertex_count++] = v;
    }
}

static void room(void)
{
    vertex_count = 0;
    box(-6.5f,-0.4f,-7,13,0.4f,16, 0.22f,0.28f,0.28f);
    box(-6.6f,0,-7,0.5f,6,16, 0.27f,0.32f,0.31f);
    box(6.1f,0,-7,0.5f,6,16, 0.27f,0.32f,0.31f);
    box(-6.5f,0,8.2f,13,6,0.5f, 0.26f,0.30f,0.30f);
    box(-6.5f,5.8f,-7,13,0.5f,16, 0.16f,0.20f,0.22f);
    /* A structural window, deliberately without a glass transparency pass. */
    box(-6.5f,0,-7.2f,13,0.7f,0.6f, 0.35f,0.40f,0.37f);
    box(-6.5f,4.8f,-7.2f,13,1.2f,0.6f, 0.27f,0.33f,0.31f);
    for (int i = -1; i <= 1; ++i)
        box((float)i * 5.7f - 0.12f,0.6f,-7.1f,0.24f,4.5f,0.4f, 0.38f,0.46f,0.42f);
    for (int z = -6; z < 9; z += 2) {
        box(-6,0.01f,(float)z,12,0.02f,0.035f, 0.33f,0.40f,0.38f);
        box(-6,4.8f,(float)z,0.14f,0.14f,0.9f, 0.93f,0.70f,0.36f);
        box(5.86f,4.8f,(float)z,0.14f,0.14f,0.9f, 0.93f,0.70f,0.36f);
        box(-6.05f,0.1f,(float)z,0.13f,4.6f,0.15f, 0.35f,0.41f,0.38f);
        box(5.92f,0.1f,(float)z,0.13f,4.6f,0.15f, 0.35f,0.41f,0.38f);
    }
    for (int x = -4; x <= 4; x += 2)
        box((float)x,0.01f,-7,0.035f,0.02f,15, 0.31f,0.37f,0.36f);
    box(-4.4f,0,-5.8f,2.4f,1.05f,0.8f, 0.25f,0.30f,0.30f);
    box(2.0f,0,-5.8f,2.4f,1.05f,0.8f, 0.25f,0.30f,0.30f);
    box(-4.2f,1.07f,-5.7f,1.9f,0.04f,0.55f, 0.25f,0.74f,0.63f);
    box(2.2f,1.07f,-5.7f,1.9f,0.04f,0.55f, 0.25f,0.74f,0.63f);
    /* Alien silhouettes supply a view beyond the test room, not a generated city. */
    box(-700,-80,-1000,1400,2,1200, 0.31f,0.32f,0.27f);
    for (int i = 0; i < 12; ++i) {
        float x = (float)(i - 6) * 42.0f;
        float z = -90.0f - (float)(i % 3) * 90;
        float h = 80.0f + (float)((i * 7) % 5) * 35;
        box(x,-78,z,18,h,23, 0.38f,0.46f,0.41f);
        box(x-6,-78+h-16,z-4,30,9,31, 0.49f,0.52f,0.43f);
        box(x+6,-18,z+23,1.5f,26,0.4f, 0.51f,0.76f,0.66f);
    }
    box(-35,30,-270,90,9,24, 0.48f,0.54f,0.48f);
    box(-35,30,-270,9,145,24, 0.48f,0.54f,0.48f);
    box(46,30,-270,9,145,24, 0.48f,0.54f,0.48f);
    box(-35,166,-270,90,9,24, 0.48f,0.54f,0.48f);
}

static GLuint shader(GLenum kind, const char *source)
{
    GLuint result = pCreateShader(kind);
    pShaderSource(result, 1, &source, NULL);
    pCompileShader(result);
    GLint ok = 0;
    pGetShaderiv(result, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char message[2048];
        pGetShaderInfoLog(result, sizeof(message), NULL, message);
        SDL_Log("Shader compilation failed: %s", message);
        pDeleteShader(result);
        return 0;
    }
    return result;
}

bool meu_renderer_init(void)
{
#ifndef __EMSCRIPTEN__
#define LOAD(name, type) p##name = (type)SDL_GL_GetProcAddress("gl" #name); \
    if (!p##name) { SDL_Log("Missing OpenGL entry: gl" #name); return false; }
    GL_PROCEDURES(LOAD)
#undef LOAD
#define GLSL_VERSION "#version 330 core\n"
#else
#define GLSL_VERSION "#version 300 es\nprecision highp float;\n"
#endif
    const char *vertex_source = GLSL_VERSION
        "layout(location=0) in vec3 position; layout(location=1) in vec3 color;\n"
        "uniform mat4 mvp; uniform vec3 eye; out vec3 tint; out float distance_to_eye;\n"
        "void main(){ tint=color; distance_to_eye=length(position-eye); gl_Position=mvp*vec4(position,1.0); }\n";
    const char *fragment_source = GLSL_VERSION
        "in vec3 tint; in float distance_to_eye; out vec4 result;\n"
        "void main(){ float fog=1.0-exp(-distance_to_eye*0.0018);"
        " result=vec4(mix(tint,vec3(0.26,0.34,0.33),fog),1.0); }\n";
    GLuint vertex = shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment = shader(GL_FRAGMENT_SHADER, fragment_source);
    if (!vertex || !fragment) {
        if (vertex) pDeleteShader(vertex);
        if (fragment) pDeleteShader(fragment);
        return false;
    }
    program = pCreateProgram();
    pAttachShader(program, vertex); pAttachShader(program, fragment);
    pLinkProgram(program);
    pDeleteShader(vertex); pDeleteShader(fragment);
    GLint ok = 0;
    pGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char message[2048];
        pGetProgramInfoLog(program, sizeof(message), NULL, message);
        SDL_Log("Shader link failed: %s", message);
        return false;
    }
    matrix_uniform = pGetUniformLocation(program, "mvp");
    eye_uniform = pGetUniformLocation(program, "eye");
    room();
    pGenVertexArrays(1, &vertex_array); pBindVertexArray(vertex_array);
    pGenBuffers(1, &buffer); pBindBuffer(GL_ARRAY_BUFFER, buffer);
    pBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertex_count * sizeof(Vertex)), vertices, GL_STATIC_DRAW);
    pEnableVertexAttribArray(0); pEnableVertexAttribArray(1);
    pVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(const void *)offsetof(Vertex,position));
    pVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(const void *)offsetof(Vertex,color));
    glEnable(GL_DEPTH_TEST);
    SDL_Log("OpenGL: %s | %s", (const char *)glGetString(GL_VERSION), (const char *)glGetString(GL_RENDERER));
    return glGetError() == GL_NO_ERROR;
}

bool meu_renderer_draw(const MeuSimulation *state, int width, int height)
{
    if (width <= 0 || height <= 0) return true;
    float projection[16], view[16], mvp[16];
    meu_matrix_perspective(projection, (float)width / (float)height);
    meu_matrix_view(view, state->x, state->y, state->z, state->yaw, state->pitch);
    meu_matrix_multiply(mvp, projection, view);
    glViewport(0,0,width,height);
    glClearColor(0.26f,0.34f,0.33f,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pUseProgram(program); pBindVertexArray(vertex_array);
    pUniformMatrix4fv(matrix_uniform,1,GL_FALSE,mvp);
    pUniform3f(eye_uniform,state->x,state->y,state->z);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)vertex_count);
    return glGetError() == GL_NO_ERROR;
}

int meu_renderer_variation(int width, int height)
{
    unsigned char pixel[4];
    int low = 765, high = 0;
    for (int y = 1; y <= 4; ++y) for (int x = 1; x <= 4; ++x) {
        glReadPixels(width * x / 5, height * y / 5, 1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
        int value = pixel[0] + pixel[1] + pixel[2];
        if (value < low) low = value;
        if (value > high) high = value;
    }
    return high - low;
}

bool meu_renderer_capture(const char *path, int width, int height)
{
    if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return false;
    size_t stride = (size_t)width * 4, size = stride * (size_t)height;
    unsigned char *pixels = malloc(size), *row = malloc(stride);
    if (!pixels || !row) { free(pixels); free(row); return false; }
    glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    for (int y = 0; y < height / 2; ++y) {
        unsigned char *top = pixels + (size_t)y * stride;
        unsigned char *bottom = pixels + (size_t)(height - y - 1) * stride;
        memcpy(row,top,stride); memcpy(top,bottom,stride); memcpy(bottom,row,stride);
    }
    SDL_Surface *surface = SDL_CreateSurfaceFrom(width,height,SDL_PIXELFORMAT_RGBA32,pixels,(int)stride);
    bool ok = surface && SDL_SaveBMP(surface,path);
    SDL_DestroySurface(surface); free(row); free(pixels);
    return ok;
}

void meu_renderer_destroy(void)
{
    if (buffer) pDeleteBuffers(1,&buffer);
    if (vertex_array) pDeleteVertexArrays(1,&vertex_array);
    if (program) pDeleteProgram(program);
    buffer = vertex_array = program = 0;
}
