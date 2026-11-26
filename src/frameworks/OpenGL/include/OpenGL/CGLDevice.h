#ifndef _CGLDEVICE_H
#define _CGLDEVICE_H

#include <OpenGL/CGLTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// The set of contexts whose objects (textures, buffers, shaders, ...) are
// shared with each other.
typedef struct CGLShareGroupRec *CGLShareGroupObj;

// The share group of a context: the same object for contexts that share
// objects (a context created with another as its share context joins that
// context's group, a context created without a share context starts its own),
// valid for as long as one of those contexts exists. NULL for a NULL context.
// Only the identity of the group is meaningful: compare share groups, do not
// dereference them.
CGL_EXPORT CGLShareGroupObj CGLGetShareGroup(CGLContextObj ctx);

#ifdef __cplusplus
}
#endif

#endif /* _CGLDEVICE_H */
