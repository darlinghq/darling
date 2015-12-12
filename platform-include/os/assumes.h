#ifndef _OS_ASSUMES_H
#define _OS_ASSUMES_H

#define posix_assumes_zero_ctx(a, b, c) (c)
#define os_assumes_zero_ctx(a, b, c) (c)
#define os_assumes_zero(a) (a);
#define os_assumes_ctx(a, b, c) (c)
#define posix_assumes_zero(a) (a)
#define __OS_COMPILETIME_ASSERT__(a)

#endif

