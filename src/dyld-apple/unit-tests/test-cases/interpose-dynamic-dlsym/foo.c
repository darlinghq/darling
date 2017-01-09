#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <dlfcn.h>
#include <crt_externs.h>
#include <mach-o/dyld_priv.h>


int foo() { return 0; }

int alt_foo() { return 10; }




static const struct dyld_interpose_tuple sTable[] = { {&alt_foo, &foo} };


__attribute__((constructor))
void init()
{
  // switch main executable to use alt_foo() when it calls foo()
  dyld_dynamic_interpose((const struct mach_header*)_NSGetMachExecuteHeader(), sTable, 1);

}

