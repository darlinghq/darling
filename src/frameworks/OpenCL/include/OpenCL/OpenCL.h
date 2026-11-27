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


#ifndef _OpenCL_H_
#define _OpenCL_H_

#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Apple-specific entry points and the block-kernel helpers are kept for binary
// compatibility with binaries that link them. The host ICD loader does not
// implement them, so the framework returns NULL instead of pretending that a
// kernel, IOSurface or GL sharegroup exists.

void *clCreateContextAndCommandQueueAPPLE(void);
void *clCreateDAGAPPLE(void);
void *clCreateImageFromIOSurface2DAPPLE(void);
void *clCreateImageFromIOSurfaceWithPropertiesAPPLE(void);
void *clCreateKernelFromDAGAPPLE(void);
void *clCreateProgramAndKernelsWithSourceAPPLE(void);
void *clCreateProgramPortableBinaryAPPLE(void);
void *clGetDAGNodeAPPLE(void);
void *clGetGLContextInfoAPPLE(void);
void *clGetMemoryAnnotationListAPPLE(void);
void *clLogMessagesToStderrAPPLE(void);
void *clLogMessagesToStdoutAPPLE(void);
void *clLogMessagesToSystemLogAPPLE(void);
void *clReleaseDAGAPPLE(void);
void *clSetKernelArgByNameAPPLE(void);
void *clSetMemObjectDestructorAPPLE(void);

void *gclBuildProgramAPPLE(void);
void *gclBuildProgramBinaryAPPLE(void);
void *gclCreateArgsAPPLE(void);
void *gclDeleteArgsAPPLE(void);
void *gclExecKernelAPPLE(void);
void *gclRegisterBlockKernelMap(void);
void *gclReleaseSampler(void);
void *gclSetKernelArgAPPLE(void);
void *gclSetKernelArgMemAPPLE(void);
void *gclSetKernelArgSamplerAPPLE(void);
void *gcl_copy_image(void);
void *gcl_copy_image_to_ptr(void);
void *gcl_copy_ptr_to_image(void);
void *gcl_create_buffer_from_ptr(void);
void *gcl_create_dispatch_queue(void);
void *gcl_create_image(void);
void *gcl_create_kernel_from_block(void);
void *gcl_free(void);
void *gcl_get_context(void);
void *gcl_get_device_id_with_dispatch_queue(void);
void *gcl_get_kernel_block_workgroup_info(void);
void *gcl_get_supported_image_formats(void);
void *gcl_gl_create_image_from_renderbuffer(void);
void *gcl_gl_create_image_from_texture(void);
void *gcl_gl_create_ptr_from_buffer(void);
void *gcl_gl_set_sharegroup(void);
void *gcl_log_cl_error(void);
void *gcl_log_cl_fatal(void);
void *gcl_log_error(void);
void *gcl_log_fatal(void);
void *gcl_log_warning(void);
void *gcl_malloc(void);
void *gcl_map_image(void);
void *gcl_map_ptr(void);
void *gcl_memcpy(void);
void *gcl_memcpy_rect(void);
void *gcl_release_image(void);
void *gcl_retain_image(void);
void *gcl_set_finalizer(void);
void *gcl_start_timer(void);
void *gcl_stop_timer(void);
void *gcl_unmap(void);

#ifdef __cplusplus
}
#endif

#endif
