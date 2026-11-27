/*
  This file is part of Darling.

  Copyright (C) 2019 Lubos Dolezel

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Darling is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <OpenCL/OpenCL.h>
#include <stdarg.h>

// The core cl* entry points live in OpenCLHost.c and reach the host's ICD
// loader. These symbols are Apple additions, or block-kernel helpers layered
// on the old Apple runtime; the host loader has no equivalent for them. They
// remain exported so that binaries linked against the Apple SDK still load,
// but they report that no such object could be created.

#define OPENCL_UNSUPPORTED_FUNCTIONS(X) \
    X(clCreateCommandQueueWithPropertiesAPPLE) \
    X(clCreateContextAndCommandQueueAPPLE) \
    X(clCreateDAGAPPLE) \
    X(clCreateImageFromIOSurface2DAPPLE) \
    X(clCreateImageFromIOSurfaceWithPropertiesAPPLE) \
    X(clCreateKernelFromDAGAPPLE) \
    X(clCreateProgramAndKernelsWithSourceAPPLE) \
    X(clCreateProgramPortableBinaryAPPLE) \
    X(clGetDAGNodeAPPLE) \
    X(clGetGLContextInfoAPPLE) \
    X(clGetMemoryAnnotationListAPPLE) \
    X(clLogMessagesToStderrAPPLE) \
    X(clLogMessagesToStdoutAPPLE) \
    X(clLogMessagesToSystemLogAPPLE) \
    X(clReleaseDAGAPPLE) \
    X(clSetKernelArgByNameAPPLE) \
    X(clSetMemObjectDestructorAPPLE) \
    X(gclBuildProgramAPPLE) \
    X(gclBuildProgramBinaryAPPLE) \
    X(gclCreateArgsAPPLE) \
    X(gclDeleteArgsAPPLE) \
    X(gclExecKernelAPPLE) \
    X(gclRegisterBlockKernelMap) \
    X(gclReleaseSampler) \
    X(gclSetKernelArgAPPLE) \
    X(gclSetKernelArgMemAPPLE) \
    X(gclSetKernelArgSamplerAPPLE) \
    X(gcl_copy_image) \
    X(gcl_copy_image_to_ptr) \
    X(gcl_copy_ptr_to_image) \
    X(gcl_create_buffer_from_ptr) \
    X(gcl_create_dispatch_queue) \
    X(gcl_create_image) \
    X(gcl_create_kernel_from_block) \
    X(gcl_free) \
    X(gcl_get_context) \
    X(gcl_get_device_id_with_dispatch_queue) \
    X(gcl_get_kernel_block_workgroup_info) \
    X(gcl_get_supported_image_formats) \
    X(gcl_gl_create_image_from_renderbuffer) \
    X(gcl_gl_create_image_from_texture) \
    X(gcl_gl_create_ptr_from_buffer) \
    X(gcl_gl_set_sharegroup) \
    X(gcl_log_cl_error) \
    X(gcl_log_cl_fatal) \
    X(gcl_log_error) \
    X(gcl_log_fatal) \
    X(gcl_log_warning) \
    X(gcl_malloc) \
    X(gcl_map_image) \
    X(gcl_map_ptr) \
    X(gcl_memcpy) \
    X(gcl_memcpy_rect) \
    X(gcl_release_image) \
    X(gcl_retain_image) \
    X(gcl_set_finalizer) \
    X(gcl_start_timer) \
    X(gcl_stop_timer) \
    X(gcl_unmap)

#define OPENCL_UNSUPPORTED_FUNCTION(name) \
    void *name(void) \
    { \
        return NULL; \
    }

OPENCL_UNSUPPORTED_FUNCTIONS(OPENCL_UNSUPPORTED_FUNCTION)

#undef OPENCL_UNSUPPORTED_FUNCTION

cl_int clSetKernelArgsVaListAPPLE(cl_kernel kernel, cl_uint num_args, va_list args)
{
    for (cl_uint i = 0; i < num_args; i++) {
        cl_uint index = va_arg(args, cl_uint);
        size_t size = va_arg(args, size_t);
        const void *value = va_arg(args, const void *);
        cl_int err = clSetKernelArg(kernel, index, size, value);
        if (err != CL_SUCCESS)
            return err;
    }
    return CL_SUCCESS;
}

cl_int clSetKernelArgsListAPPLE(cl_kernel kernel, cl_uint num_args, ...)
{
    va_list args;
    va_start(args, num_args);
    cl_int err = clSetKernelArgsVaListAPPLE(kernel, num_args, args);
    va_end(args);
    return err;
}
