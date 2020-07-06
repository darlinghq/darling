#ifndef _XAR_EA_H_
#define _XAR_EA_H_

#include "xar.h"
#include "filetree.h"

typedef struct __xar_ea_t *xar_ea_t;

xar_ea_t xar_ea_new(xar_file_t f, const char *name);
int32_t xar_ea_pset(xar_file_t f, xar_ea_t e, const char *key, const char *value);
int32_t xar_ea_pget(xar_ea_t e, const char *key, const char **value);
xar_prop_t xar_ea_root(xar_ea_t e);
xar_prop_t xar_ea_find(xar_file_t f, const char *name);

#endif /* _XAR_EA_H_ */
