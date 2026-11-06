#import <Foundation/Foundation.h>
#import <OpenGL/OpenGL.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
    @autoreleasepool {
        if (argc>1 && strcmp(argv[1],"flush")==0) {
            CGLError error=CGLFlushDrawable(NULL);
            printf("null flush error=%d expected=%d\n",error,kCGLBadContext);
            return error!=kCGLBadContext;
        }
        // Without a registered display, creation uses an offscreen default
        // display. If no EGL display can be opened at all, it fails with
        // kCGLBadConnection and must clear the output.
        CGLContextObj context=(CGLContextObj)(uintptr_t)1;
        CGLError error=CGLCreateContext(NULL,NULL,&context);
        int failures=error==kCGLNoError ? context==NULL || context==(CGLContextObj)(uintptr_t)1
                                        : (error!=kCGLBadConnection || context!=NULL);
        printf("no-display create error=%d cleared=%d\n",error,context==NULL);
        if (error==kCGLNoError)
            CGLDestroyContext(context);
        error=CGLCreateContext(NULL,NULL,NULL);
        failures+=(error!=kCGLBadAddress);
        printf("null output error=%d expected=%d\n",error,kCGLBadAddress);
        printf("creation checks=2 failures=%d\n",failures);
        return failures!=0;
    }
}
