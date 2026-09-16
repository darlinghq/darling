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
        CGLContextObj context=(CGLContextObj)(uintptr_t)1;
        CGLError error=CGLCreateContext(NULL,NULL,&context);
        int failures=(error!=kCGLBadContext || context!=NULL);
        printf("no-display create error=%d cleared=%d\n",error,context==NULL);
        error=CGLCreateContext(NULL,NULL,NULL);
        failures+=(error!=kCGLBadAddress);
        printf("null output error=%d expected=%d\n",error,kCGLBadAddress);
        printf("creation checks=2 failures=%d\n",failures);
        return failures!=0;
    }
}
