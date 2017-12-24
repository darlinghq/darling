#ifndef _CGLCONTEXT_H
#define _CGLCONTEXT_H

#include <OpenGL/gliContext.h>
#include <OpenGL/gliDispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CGLPrivateObject *CGLPrivateObj;

struct _CGLContextObject {
	GLIContext            rend;
	GLIFunctionDispatch   disp;
	CGLPrivateObj         priv;
	void                 *stak;
};


#ifdef __cplusplus
}
#endif

#endif