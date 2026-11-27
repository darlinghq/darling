// Compile-time check that the SDK exposes documented OpenCL 1.2 signatures,
// not the old void *(void) placeholder declarations.

#include <OpenCL/OpenCL.h>

#include <type_traits>

typedef decltype(&clGetPlatformIDs) get_platform_ids_fn;
typedef decltype(&clGetDeviceIDs) get_device_ids_fn;
typedef decltype(&clCreateBuffer) create_buffer_fn;
typedef decltype(&clSetKernelArg) set_kernel_arg_fn;
typedef decltype(&clEnqueueNDRangeKernel) enqueue_ndrange_fn;
typedef decltype(&clCreateFromGLBuffer) create_from_gl_buffer_fn;

static_assert(std::is_same<get_platform_ids_fn,
                           cl_int (*)(cl_uint, cl_platform_id *, cl_uint *)>::value,
              "clGetPlatformIDs has the wrong signature");
static_assert(std::is_same<get_device_ids_fn,
                           cl_int (*)(cl_platform_id, cl_device_type, cl_uint, cl_device_id *,
                                      cl_uint *)>::value,
              "clGetDeviceIDs has the wrong signature");
static_assert(std::is_same<create_buffer_fn,
                           cl_mem (*)(cl_context, cl_mem_flags, size_t, void *, cl_int *)>::value,
              "clCreateBuffer has the wrong signature");
static_assert(std::is_same<set_kernel_arg_fn,
                           cl_int (*)(cl_kernel, cl_uint, size_t, const void *)>::value,
              "clSetKernelArg has the wrong signature");
static_assert(std::is_same<enqueue_ndrange_fn,
                           cl_int (*)(cl_command_queue, cl_kernel, cl_uint, const size_t *,
                                      const size_t *, const size_t *, cl_uint, const cl_event *,
                                      cl_event *)>::value,
              "clEnqueueNDRangeKernel has the wrong signature");
static_assert(std::is_same<create_from_gl_buffer_fn,
                           cl_mem (*)(cl_context, cl_mem_flags, cl_GLuint, cl_int *)>::value,
              "clCreateFromGLBuffer has the wrong signature");

int main()
{
    return 0;
}
