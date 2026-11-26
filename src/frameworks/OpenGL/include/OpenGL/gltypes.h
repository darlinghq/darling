#ifndef __gltypes_h_
#define __gltypes_h_

/*
 * The basic OpenGL types, without the OpenGL API. As on macOS, OpenGL.h, the
 * CGL headers and AppKit's OpenGL classes declare their functions with these
 * and leave the OpenGL API (gl.h and its extensions) to those who include it.
 *
 * The typedefs moved here unchanged from gl.h, which is Mesa's (MIT license,
 * see the notice in gl.h).
 */

typedef unsigned int	GLenum;
typedef unsigned char	GLboolean;
typedef unsigned int	GLbitfield;
typedef void		GLvoid;
typedef signed char	GLbyte;		/* 1-byte signed */
typedef short		GLshort;	/* 2-byte signed */
typedef int		GLint;		/* 4-byte signed */
typedef unsigned char	GLubyte;	/* 1-byte unsigned */
typedef unsigned short	GLushort;	/* 2-byte unsigned */
typedef unsigned int	GLuint;		/* 4-byte unsigned */
typedef int		GLsizei;	/* 4-byte signed */
typedef float		GLfloat;	/* single precision float */
typedef float		GLclampf;	/* single precision float in [0,1] */
typedef double		GLdouble;	/* double precision float */
typedef double		GLclampd;	/* double precision float in [0,1] */

#endif /* __gltypes_h_ */
