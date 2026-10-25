#import <AppKit/AppKit.h>
#import <CoreGraphics/CGSubWindow.h>
#import <OpenGL/CGLInternal.h>
#import <OpenGL/gl.h>
#include <stdio.h>
#include <stdlib.h>

@interface NSWindow (DepthFixture)
- (CGSubWindow *) _createSubWindowWithFrame:(CGRect)frame;
@end

@interface DepthFixture : NSObject
- (void) draw:(NSTimer *)timer;
@end

@implementation DepthFixture
- (void) draw:(NSTimer *)timer {
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(30,30,160,100)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    [window setReleasedWhenClosed:NO];
    [window makeKeyAndOrderFront:nil];
    CGSubWindow *child = [[window _createSubWindowWithFrame:NSMakeRect(0,0,160,100)] retain];
    CGLContextObj context = NULL;
    CGLWindowRef drawable = NULL;
    int result = 1;
    if (!child || CGLCreateContext(NULL,NULL,&context) != kCGLNoError || !context)
        goto cleanup;
    drawable = CGLGetWindowForContext(context,[child nativeWindow]);
    if (!drawable || CGLContextMakeCurrentAndAttachToWindow(context,drawable) != kCGLNoError)
        goto cleanup;
    GLint depth = 0;
    glGetIntegerv(GL_DEPTH_BITS,&depth);
    glViewport(0,0,160,100);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glDepthMask(GL_TRUE);
    glClearColor(0,0,0,1); glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    // Paint the nearer red surface first. The farther blue surface must not cover it.
    glColor4f(1,0,0,1);
    glBegin(GL_QUADS);
    glVertex3f(-1,-1,-0.5); glVertex3f(1,-1,-0.5);
    glVertex3f(1,1,-0.5); glVertex3f(-1,1,-0.5);
    glEnd();
    glColor4f(0,0,1,1);
    glBegin(GL_QUADS);
    glVertex3f(-1,-1,0.5); glVertex3f(1,-1,0.5);
    glVertex3f(1,1,0.5); glVertex3f(-1,1,0.5);
    glEnd();
    unsigned char pixel[4] = {0};
    glReadPixels(10,10,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
    GLenum error = glGetError();
    result = depth < 24 || pixel[0] != 255 || pixel[1] != 0 || pixel[2] != 0 || error != GL_NO_ERROR;
    printf("depth=%d red-surface-visible=%d gl-error=%u %s\n",depth,
        pixel[0]==255 && pixel[1]==0 && pixel[2]==0,error,result?"FAIL":"PASS");
cleanup:
    CGLSetCurrentContext(NULL);
    if (drawable) CGLDestroyWindow(drawable);
    if (context) CGLReleaseContext(context);
    [child release]; [window close]; [window release];
    fflush(stdout);
    exit(result);
}
@end

int main(void) {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    [NSApplication sharedApplication];
    [NSScreen screens];
    DepthFixture *driver = [DepthFixture new];
    [NSTimer scheduledTimerWithTimeInterval:0.2 target:driver selector:@selector(draw:)
        userInfo:nil repeats:NO];
    [NSApp run];
    [driver release]; [pool drain];
    return 1;
}
