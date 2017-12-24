#ifndef __gltypes_H_
#define __gltypes_H_

#include <stdint.h>

typedef uint32_t GLbitfield;
typedef uint8_t  GLboolean;
typedef int8_t   GLbyte;
typedef float    GLclampf;
typedef uint32_t GLenum;
typedef float    GLfloat;
typedef int32_t  GLint;
typedef int16_t  GLshort;
typedef int32_t  GLsizei;
typedef uint8_t  GLubyte;
typedef uint32_t GLuint;
typedef uint16_t GLushort;
typedef void     GLvoid;

#if !defined(GL_VERSION_2_0)
typedef char     GLchar;
#endif
#if !defined(GL_ARB_shader_objects)
typedef char     GLcharARB;
typedef void    *GLhandleARB;
#endif
typedef double   GLdouble;
typedef double   GLclampd;
#if !defined(ARB_ES2_compatibility) && !defined(GL_VERSION_4_1)
typedef int32_t  GLfixed;
#endif
#if !defined(GL_ARB_half_float_vertex) && !defined(GL_VERSION_3_0)
typedef uint16_t GLhalf;
#endif
#if !defined(GL_ARB_half_float_pixel)
typedef uint16_t GLhalfARB;
#endif
#if !defined(GL_ARB_sync) && !defined(GL_VERSION_3_2)
typedef int64_t  GLint64;
typedef struct __GLsync *GLsync;
typedef uint64_t GLuint64;
#endif
#if !defined(GL_EXT_timer_query)
typedef int64_t  GLint64EXT;
typedef uint64_t GLuint64EXT;
#endif

#if !defined(GL_VERSION_1_5)
#ifdef OPENGL_LINUX_COMPAT
typedef ptrdiff_t GLintptr;
#else
typedef intptr_t GLintptr;
#endif

#ifdef OPENGL_LINUX_COMPAT
typedef ptrdiff_t GLsizeiptr;
#else
typedef intptr_t GLsizeiptr;
#endif
#endif

#if !defined(GL_ARB_vertex_buffer_object)
#ifdef OPENGL_LINUX_COMPAT
typedef ptrdiff_t GLintptrARB;
#else
typedef intptr_t GLintptrARB;
#endif
#ifdef OPENGL_LINUX_COMPAT
typedef ptrdiff_t GLsizeiptrARB;
#else
typedef intptr_t GLsizeiptrARB;
#endif

#endif

#endif
