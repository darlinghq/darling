#ifndef _EXT_FANOTIFY_H
#define _EXT_FANOTIFY_H
#include "../base.h"

#ifdef __cplusplus
extern "C" {
#endif

VISIBLE
int fanotify_init(unsigned flags, unsigned event_f_flags);

VISIBLE
int fanotify_mark(int fanotify_fd, unsigned flags, unsigned long long mask, int dfd, const char *pathname);


#ifdef __cplusplus
}
#endif

#endif
