#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>
#define GL_GLEXT_PROTOTYPES 1
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#include <stdio.h>
#include <string.h>

// OpenGL/include/OpenGL/glext.h; some configured SDKs omit this alias.
extern void glTexSubImage3DEXT(GLenum, GLint, GLint, GLint, GLint, GLsizei,
                              GLsizei, GLsizei, GLenum, GLenum, const void *);

int main(void) {
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSScreen screens];
  NSOpenGLPixelFormatAttribute attrs[] = {NSOpenGLPFAAccelerated, 0};
  NSOpenGLPixelFormat *format =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  NSOpenGLContext *context =
      [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
  [context makeCurrentContext];
  if (!context || !CGLGetCurrentContext() ||
      CGLGetCurrentContext() != [context CGLContextObj]) {
    fputs("No current GL context\n", stderr);
    return 1;
  }
  GLuint texture = 0;
  unsigned char pixels[32], output[32];
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2, 2, 2, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  int failed = !texture || glGetError() != GL_NO_ERROR;
  for (int round = 0; round < 4 && !failed; round++) {
    for (int i = 0; i < 32; i++)
      pixels[i] = (unsigned char)(i * 7 + round * 13 + 3);
    memset(output, 0, sizeof(output));
    if (round & 1)
      glTexSubImage3DEXT(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 2, 2, 2, GL_RGBA,
                        GL_UNSIGNED_BYTE, pixels);
    else
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 2, 2, 2, GL_RGBA,
                     GL_UNSIGNED_BYTE, pixels);
    GLenum error = glGetError();
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, output);
    GLenum read_error = glGetError();
    failed = error != GL_NO_ERROR || read_error != GL_NO_ERROR ||
             memcmp(pixels, output, sizeof(pixels)) != 0;
    printf("round=%d upload=%x readback=%x matches=%d\n", round, error,
           read_error, memcmp(pixels, output, sizeof(pixels)) == 0);
  }
  glDeleteTextures(1, &texture);
  [NSOpenGLContext clearCurrentContext];
  [context release];
  [format release];
  [pool drain];
  return failed ? 1 : 0;
}
