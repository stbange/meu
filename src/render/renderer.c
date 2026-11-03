#include "render/renderer.h"
#include "core/matrix.h"
#include "render/mesh.h"
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

static GLuint program, buffer, dynamic_buffer, vertex_array;
static GLint matrix_uniform, eye_uniform, lighting_uniform;
static unsigned interior_count, exterior_count;

static void bind_mesh(GLuint id)
{
    pBindBuffer(GL_ARRAY_BUFFER,id);
    pVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(MeuVertex),(const void *)offsetof(MeuVertex,position));
    pVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(MeuVertex),(const void *)offsetof(MeuVertex,color));
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
        "in vec3 tint; in float distance_to_eye; uniform vec3 lighting; out vec4 result;\n"
        "void main(){ float fog=1.0-exp(-distance_to_eye*lighting.y);"
        " result=vec4(mix(tint*lighting.x,vec3(0.22,0.30,0.31),fog),1.0); }\n";
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
    lighting_uniform = pGetUniformLocation(program, "lighting");
    meu_vertex_count=0; meu_mesh_interior(); interior_count=meu_vertex_count;
    meu_mesh_exterior(); exterior_count=meu_vertex_count-interior_count;
    SDL_Log("Scene: %u interior triangles, %u exterior triangles",interior_count/3,exterior_count/3);
    pGenVertexArrays(1,&vertex_array); pBindVertexArray(vertex_array);
    pGenBuffers(1,&buffer); bind_mesh(buffer);
    pBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(meu_vertex_count*sizeof(MeuVertex)),meu_vertices,GL_STATIC_DRAW);
    pGenBuffers(1,&dynamic_buffer);
    pEnableVertexAttribArray(0); pEnableVertexAttribArray(1);
    glEnable(GL_DEPTH_TEST);
    SDL_Log("OpenGL: %s | %s", (const char *)glGetString(GL_VERSION), (const char *)glGetString(GL_RENDERER));
    return glGetError() == GL_NO_ERROR;
}

bool meu_renderer_draw(const MeuSimulation *state, int width, int height,int selection)
{
    (void)selection;
    if (width<=0 || height<=0) return true;
    float projection[16],view[16],mvp[16],inverse[16],world_mvp[16];
    meu_matrix_perspective(projection,(float)width/(float)height);
    MeuCamera eye=meu_sim_camera(state);
    meu_matrix_view(view,eye.x,eye.y,eye.z,eye.yaw,eye.pitch);
    meu_matrix_multiply(mvp,projection,view);
    meu_pose_inverse_matrix(state->robot,inverse);
    meu_matrix_multiply(world_mvp,mvp,inverse);
    glViewport(0,0,width,height);
    glClearColor(.22f,.30f,.31f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    pUseProgram(program); pBindVertexArray(vertex_array); bind_mesh(buffer);
    float light=1;
    if (meu_sim_room(state)==MEU_QUARTERS && !state->quarters_light) light=.58f;
    if (meu_sim_room(state)==MEU_ENGINEERING && !state->inspection_light) light=.60f;
    pUniform3f(lighting_uniform,light,.0009f,0);
    pUniformMatrix4fv(matrix_uniform,1,GL_FALSE,mvp);
    pUniform3f(eye_uniform,eye.x,eye.y,eye.z);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)interior_count);
    double world_eye[3]; meu_pose_to_world(state->robot,eye.x,eye.y,eye.z,world_eye);
    pUniform3f(eye_uniform,(float)world_eye[0],(float)world_eye[1],(float)world_eye[2]);
    pUniformMatrix4fv(matrix_uniform,1,GL_FALSE,world_mvp);
    pUniform3f(lighting_uniform,1,.0009f,0);
    glDrawArrays(GL_TRIANGLES,(GLint)interior_count,(GLsizei)exterior_count);
    meu_vertex_count=0; meu_mesh_lift(state); unsigned lift_count=meu_vertex_count;
#ifndef __EMSCRIPTEN__
    meu_mesh_hud(state,width,height,selection);
#endif
    bind_mesh(dynamic_buffer);
    pBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(meu_vertex_count*sizeof(MeuVertex)),meu_vertices,GL_STREAM_DRAW);
    pUniformMatrix4fv(matrix_uniform,1,GL_FALSE,mvp);
    pUniform3f(eye_uniform,eye.x,eye.y,eye.z);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)lift_count);
#ifndef __EMSCRIPTEN__
    float ortho[16]={0}; ortho[0]=2.0f/(float)width; ortho[5]=2.0f/(float)height;
    ortho[10]=1; ortho[12]=-1; ortho[13]=-1; ortho[15]=1;
    pUniformMatrix4fv(matrix_uniform,1,GL_FALSE,ortho);
    pUniform3f(lighting_uniform,1,0,0); glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES,(GLint)lift_count,(GLsizei)(meu_vertex_count-lift_count));
#endif
    return glGetError()==GL_NO_ERROR;
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
    if (dynamic_buffer) pDeleteBuffers(1,&dynamic_buffer);
    if (vertex_array) pDeleteVertexArrays(1,&vertex_array);
    if (program) pDeleteProgram(program);
    buffer = dynamic_buffer = vertex_array = program = 0;
}
