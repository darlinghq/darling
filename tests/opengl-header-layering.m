// The OpenGL header layering of the SDK: <OpenGL/OpenGL.h>, <OpenGL/CGLIOSurface.h>
// and AppKit's OpenGL classes declare their API with the OpenGL types from
// <OpenGL/gltypes.h>, and leave the OpenGL API itself (<OpenGL/gl.h> and its
// extensions) to code that includes it. A program that includes only these
// headers can define its own GL entry points and extension types (as ANGLE
// does) without clashing with the system ones.
// Run on macOS 26: gl.h is not included, the types are declared.
#import <AppKit/AppKit.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLIOSurface.h>
#include <stdio.h>

#if defined(GL_VERSION_1_1) || defined(__gl_h_)
#error "<OpenGL/gl.h> was included by AppKit, <OpenGL/OpenGL.h> or <OpenGL/CGLIOSurface.h>"
#endif

// The type an OpenGL extension header would declare; defining it here must not clash.
typedef void (*GLDEBUGPROCKHR)(GLenum, GLenum, GLuint, GLenum, GLsizei, const char *, const void *);

int main(void)
{
    GLenum target = 0x0DE1;
    GLuint name = 1;
    GLsizei width = 2;
    GLint level = 0;
    GLboolean flag = 1;
    GLbitfield mask = 0x4000;
    GLfloat f = 0.5f;
    GLclampf c = 1.0f;
    GLDEBUGPROCKHR callback = NULL;
    (void)callback;
    if (sizeof(GLenum) != 4 || sizeof(GLuint) != 4 || sizeof(GLsizei) != 4 || sizeof(GLint) != 4
        || sizeof(GLboolean) != 1 || sizeof(GLbitfield) != 4 || sizeof(GLfloat) != 4 || sizeof(GLclampf) != 4)
    {
        printf("FAIL: OpenGL type sizes\n");
        return 1;
    }
    printf("PASS %u %u %d %d %u %u %g %g\n", target, name, width, level, flag, mask, f, c);
    return 0;
}
