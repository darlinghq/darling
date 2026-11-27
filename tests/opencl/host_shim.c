/*
  Host-side contract test for the OpenCL framework's host loader shim.

  The real OpenCLHost.c is linked with an elfcalls table whose dlopen always
  fails. That exercises the documented argument checks and the error returned
  when no ICD loader is available, without requiring a Darling prefix or an
  installed OpenCL ICD.
*/

#include <OpenCL/OpenCL.h>
#include <elfcalls.h>

struct elf_calls *_elfcalls;

static void *no_dlopen(const char *name)
{
    (void) name;
    return NULL;
}

static void *no_dlsym(void *library, const char *name)
{
    (void) library;
    (void) name;
    return NULL;
}

static struct elf_calls unavailable_host = {
    .dlopen = no_dlopen,
    .dlsym = no_dlsym,
};

__attribute__((constructor))
static void install_host_calls(void)
{
    _elfcalls = &unavailable_host;
}

int main(void)
{
    cl_uint count = 42;
    cl_platform_id platform = NULL;

    if (clGetPlatformIDs(0, NULL, &count) != CL_PLATFORM_NOT_FOUND_KHR || count != 0)
        return 1;
    if (clGetPlatformIDs(0, &platform, &count) != CL_INVALID_VALUE)
        return 2;

    count = 42;
    if (clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &count) != CL_INVALID_PLATFORM)
        return 3;
    if (clGetDeviceIDs(NULL, 0, 0, NULL, &count) != CL_INVALID_PLATFORM)
        return 4;

    cl_int err = CL_SUCCESS;
    if (clCreateBuffer(NULL, CL_MEM_READ_ONLY, 1, NULL, &err) != NULL)
        return 5;
    if (err != CL_INVALID_OPERATION)
        return 6;

    if (clSetKernelArg(NULL, 0, 0, NULL) != CL_INVALID_OPERATION)
        return 7;
    if (clEnqueueBarrier(NULL) != CL_INVALID_OPERATION)
        return 8;
    if (clGetExtensionFunctionAddress("clCreateBuffer") != NULL)
        return 9;

    return 0;
}
