#ifndef OBJC_EXCEPTIONS_H
#define OBJC_EXCEPTIONS_H
#include "../visibility.h"

extern "C" {

DARLING_VISIBLE void __darwin_objc_exception_rethrow();
DARLING_VISIBLE void* __darwin_objc_begin_catch(void*);
DARLING_VISIBLE void __darwin_objc_end_catch();

}

#endif

