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
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* clBuildProgram(void)
{
    if (verbose) puts("STUB: clBuildProgram called");
    return NULL;
}

void* clCompileProgram(void)
{
    if (verbose) puts("STUB: clCompileProgram called");
    return NULL;
}

void* clCreateBuffer(void)
{
    if (verbose) puts("STUB: clCreateBuffer called");
    return NULL;
}

void* clCreateCommandQueue(void)
{
    if (verbose) puts("STUB: clCreateCommandQueue called");
    return NULL;
}

void* clCreateCommandQueueWithPropertiesAPPLE(void)
{
    if (verbose) puts("STUB: clCreateCommandQueueWithPropertiesAPPLE called");
    return NULL;
}

void* clCreateContext(void)
{
    if (verbose) puts("STUB: clCreateContext called");
    return NULL;
}

void* clCreateContextAndCommandQueueAPPLE(void)
{
    if (verbose) puts("STUB: clCreateContextAndCommandQueueAPPLE called");
    return NULL;
}

void* clCreateContextFromType(void)
{
    if (verbose) puts("STUB: clCreateContextFromType called");
    return NULL;
}

void* clCreateDAGAPPLE(void)
{
    if (verbose) puts("STUB: clCreateDAGAPPLE called");
    return NULL;
}

void* clCreateEventFromGLsyncKHR(void)
{
    if (verbose) puts("STUB: clCreateEventFromGLsyncKHR called");
    return NULL;
}

void* clCreateFromGLBuffer(void)
{
    if (verbose) puts("STUB: clCreateFromGLBuffer called");
    return NULL;
}

void* clCreateFromGLRenderbuffer(void)
{
    if (verbose) puts("STUB: clCreateFromGLRenderbuffer called");
    return NULL;
}

void* clCreateFromGLTexture(void)
{
    if (verbose) puts("STUB: clCreateFromGLTexture called");
    return NULL;
}

void* clCreateFromGLTexture2D(void)
{
    if (verbose) puts("STUB: clCreateFromGLTexture2D called");
    return NULL;
}

void* clCreateFromGLTexture3D(void)
{
    if (verbose) puts("STUB: clCreateFromGLTexture3D called");
    return NULL;
}

void* clCreateImage(void)
{
    if (verbose) puts("STUB: clCreateImage called");
    return NULL;
}

void* clCreateImage2D(void)
{
    if (verbose) puts("STUB: clCreateImage2D called");
    return NULL;
}

void* clCreateImage3D(void)
{
    if (verbose) puts("STUB: clCreateImage3D called");
    return NULL;
}

void* clCreateImageFromIOSurface2DAPPLE(void)
{
    if (verbose) puts("STUB: clCreateImageFromIOSurface2DAPPLE called");
    return NULL;
}

void* clCreateImageFromIOSurfaceWithPropertiesAPPLE(void)
{
    if (verbose) puts("STUB: clCreateImageFromIOSurfaceWithPropertiesAPPLE called");
    return NULL;
}

void* clCreateKernel(void)
{
    if (verbose) puts("STUB: clCreateKernel called");
    return NULL;
}

void* clCreateKernelFromDAGAPPLE(void)
{
    if (verbose) puts("STUB: clCreateKernelFromDAGAPPLE called");
    return NULL;
}

void* clCreateKernelsInProgram(void)
{
    if (verbose) puts("STUB: clCreateKernelsInProgram called");
    return NULL;
}

void* clCreateProgramAndKernelsWithSourceAPPLE(void)
{
    if (verbose) puts("STUB: clCreateProgramAndKernelsWithSourceAPPLE called");
    return NULL;
}

void* clCreateProgramPortableBinaryAPPLE(void)
{
    if (verbose) puts("STUB: clCreateProgramPortableBinaryAPPLE called");
    return NULL;
}

void* clCreateProgramWithBinary(void)
{
    if (verbose) puts("STUB: clCreateProgramWithBinary called");
    return NULL;
}

void* clCreateProgramWithBuiltInKernels(void)
{
    if (verbose) puts("STUB: clCreateProgramWithBuiltInKernels called");
    return NULL;
}

void* clCreateProgramWithSource(void)
{
    if (verbose) puts("STUB: clCreateProgramWithSource called");
    return NULL;
}

void* clCreateSampler(void)
{
    if (verbose) puts("STUB: clCreateSampler called");
    return NULL;
}

void* clCreateSubBuffer(void)
{
    if (verbose) puts("STUB: clCreateSubBuffer called");
    return NULL;
}

void* clCreateSubDevices(void)
{
    if (verbose) puts("STUB: clCreateSubDevices called");
    return NULL;
}

void* clCreateUserEvent(void)
{
    if (verbose) puts("STUB: clCreateUserEvent called");
    return NULL;
}

void* clEnqueueAcquireGLObjects(void)
{
    if (verbose) puts("STUB: clEnqueueAcquireGLObjects called");
    return NULL;
}

void* clEnqueueBarrier(void)
{
    if (verbose) puts("STUB: clEnqueueBarrier called");
    return NULL;
}

void* clEnqueueBarrierWithWaitList(void)
{
    if (verbose) puts("STUB: clEnqueueBarrierWithWaitList called");
    return NULL;
}

void* clEnqueueCopyBuffer(void)
{
    if (verbose) puts("STUB: clEnqueueCopyBuffer called");
    return NULL;
}

void* clEnqueueCopyBufferRect(void)
{
    if (verbose) puts("STUB: clEnqueueCopyBufferRect called");
    return NULL;
}

void* clEnqueueCopyBufferToImage(void)
{
    if (verbose) puts("STUB: clEnqueueCopyBufferToImage called");
    return NULL;
}

void* clEnqueueCopyImage(void)
{
    if (verbose) puts("STUB: clEnqueueCopyImage called");
    return NULL;
}

void* clEnqueueCopyImageToBuffer(void)
{
    if (verbose) puts("STUB: clEnqueueCopyImageToBuffer called");
    return NULL;
}

void* clEnqueueFillBuffer(void)
{
    if (verbose) puts("STUB: clEnqueueFillBuffer called");
    return NULL;
}

void* clEnqueueFillImage(void)
{
    if (verbose) puts("STUB: clEnqueueFillImage called");
    return NULL;
}

void* clEnqueueMapBuffer(void)
{
    if (verbose) puts("STUB: clEnqueueMapBuffer called");
    return NULL;
}

void* clEnqueueMapImage(void)
{
    if (verbose) puts("STUB: clEnqueueMapImage called");
    return NULL;
}

void* clEnqueueMarker(void)
{
    if (verbose) puts("STUB: clEnqueueMarker called");
    return NULL;
}

void* clEnqueueMarkerWithWaitList(void)
{
    if (verbose) puts("STUB: clEnqueueMarkerWithWaitList called");
    return NULL;
}

void* clEnqueueMigrateMemObjects(void)
{
    if (verbose) puts("STUB: clEnqueueMigrateMemObjects called");
    return NULL;
}

void* clEnqueueNDRangeKernel(void)
{
    if (verbose) puts("STUB: clEnqueueNDRangeKernel called");
    return NULL;
}

void* clEnqueueNativeKernel(void)
{
    if (verbose) puts("STUB: clEnqueueNativeKernel called");
    return NULL;
}

void* clEnqueueReadBuffer(void)
{
    if (verbose) puts("STUB: clEnqueueReadBuffer called");
    return NULL;
}

void* clEnqueueReadBufferRect(void)
{
    if (verbose) puts("STUB: clEnqueueReadBufferRect called");
    return NULL;
}

void* clEnqueueReadImage(void)
{
    if (verbose) puts("STUB: clEnqueueReadImage called");
    return NULL;
}

void* clEnqueueReleaseGLObjects(void)
{
    if (verbose) puts("STUB: clEnqueueReleaseGLObjects called");
    return NULL;
}

void* clEnqueueTask(void)
{
    if (verbose) puts("STUB: clEnqueueTask called");
    return NULL;
}

void* clEnqueueUnmapMemObject(void)
{
    if (verbose) puts("STUB: clEnqueueUnmapMemObject called");
    return NULL;
}

void* clEnqueueWaitForEvents(void)
{
    if (verbose) puts("STUB: clEnqueueWaitForEvents called");
    return NULL;
}

void* clEnqueueWriteBuffer(void)
{
    if (verbose) puts("STUB: clEnqueueWriteBuffer called");
    return NULL;
}

void* clEnqueueWriteBufferRect(void)
{
    if (verbose) puts("STUB: clEnqueueWriteBufferRect called");
    return NULL;
}

void* clEnqueueWriteImage(void)
{
    if (verbose) puts("STUB: clEnqueueWriteImage called");
    return NULL;
}

void* clFinish(void)
{
    if (verbose) puts("STUB: clFinish called");
    return NULL;
}

void* clFlush(void)
{
    if (verbose) puts("STUB: clFlush called");
    return NULL;
}

void* clGetCommandQueueInfo(void)
{
    if (verbose) puts("STUB: clGetCommandQueueInfo called");
    return NULL;
}

void* clGetContextInfo(void)
{
    if (verbose) puts("STUB: clGetContextInfo called");
    return NULL;
}

void* clGetDAGNodeAPPLE(void)
{
    if (verbose) puts("STUB: clGetDAGNodeAPPLE called");
    return NULL;
}

void* clGetDeviceIDs(void)
{
    if (verbose) puts("STUB: clGetDeviceIDs called");
    return NULL;
}

void* clGetDeviceInfo(void)
{
    if (verbose) puts("STUB: clGetDeviceInfo called");
    return NULL;
}

void* clGetEventInfo(void)
{
    if (verbose) puts("STUB: clGetEventInfo called");
    return NULL;
}

void* clGetEventProfilingInfo(void)
{
    if (verbose) puts("STUB: clGetEventProfilingInfo called");
    return NULL;
}

void* clGetExtensionFunctionAddress(void)
{
    if (verbose) puts("STUB: clGetExtensionFunctionAddress called");
    return NULL;
}

void* clGetExtensionFunctionAddressForPlatform(void)
{
    if (verbose) puts("STUB: clGetExtensionFunctionAddressForPlatform called");
    return NULL;
}

void* clGetGLContextInfoAPPLE(void)
{
    if (verbose) puts("STUB: clGetGLContextInfoAPPLE called");
    return NULL;
}

void* clGetGLObjectInfo(void)
{
    if (verbose) puts("STUB: clGetGLObjectInfo called");
    return NULL;
}

void* clGetGLTextureInfo(void)
{
    if (verbose) puts("STUB: clGetGLTextureInfo called");
    return NULL;
}

void* clGetImageInfo(void)
{
    if (verbose) puts("STUB: clGetImageInfo called");
    return NULL;
}

void* clGetKernelArgInfo(void)
{
    if (verbose) puts("STUB: clGetKernelArgInfo called");
    return NULL;
}

void* clGetKernelInfo(void)
{
    if (verbose) puts("STUB: clGetKernelInfo called");
    return NULL;
}

void* clGetKernelWorkGroupInfo(void)
{
    if (verbose) puts("STUB: clGetKernelWorkGroupInfo called");
    return NULL;
}

void* clGetMemObjectInfo(void)
{
    if (verbose) puts("STUB: clGetMemObjectInfo called");
    return NULL;
}

void* clGetMemoryAnnotationListAPPLE(void)
{
    if (verbose) puts("STUB: clGetMemoryAnnotationListAPPLE called");
    return NULL;
}

void* clGetPlatformIDs(void)
{
    if (verbose) puts("STUB: clGetPlatformIDs called");
    return NULL;
}

void* clGetPlatformInfo(void)
{
    if (verbose) puts("STUB: clGetPlatformInfo called");
    return NULL;
}

void* clGetProgramBuildInfo(void)
{
    if (verbose) puts("STUB: clGetProgramBuildInfo called");
    return NULL;
}

void* clGetProgramInfo(void)
{
    if (verbose) puts("STUB: clGetProgramInfo called");
    return NULL;
}

void* clGetSamplerInfo(void)
{
    if (verbose) puts("STUB: clGetSamplerInfo called");
    return NULL;
}

void* clGetSupportedImageFormats(void)
{
    if (verbose) puts("STUB: clGetSupportedImageFormats called");
    return NULL;
}

void* clLinkProgram(void)
{
    if (verbose) puts("STUB: clLinkProgram called");
    return NULL;
}

void* clLogMessagesToStderrAPPLE(void)
{
    if (verbose) puts("STUB: clLogMessagesToStderrAPPLE called");
    return NULL;
}

void* clLogMessagesToStdoutAPPLE(void)
{
    if (verbose) puts("STUB: clLogMessagesToStdoutAPPLE called");
    return NULL;
}

void* clLogMessagesToSystemLogAPPLE(void)
{
    if (verbose) puts("STUB: clLogMessagesToSystemLogAPPLE called");
    return NULL;
}

void* clReleaseCommandQueue(void)
{
    if (verbose) puts("STUB: clReleaseCommandQueue called");
    return NULL;
}

void* clReleaseContext(void)
{
    if (verbose) puts("STUB: clReleaseContext called");
    return NULL;
}

void* clReleaseDAGAPPLE(void)
{
    if (verbose) puts("STUB: clReleaseDAGAPPLE called");
    return NULL;
}

void* clReleaseDevice(void)
{
    if (verbose) puts("STUB: clReleaseDevice called");
    return NULL;
}

void* clReleaseEvent(void)
{
    if (verbose) puts("STUB: clReleaseEvent called");
    return NULL;
}

void* clReleaseKernel(void)
{
    if (verbose) puts("STUB: clReleaseKernel called");
    return NULL;
}

void* clReleaseMemObject(void)
{
    if (verbose) puts("STUB: clReleaseMemObject called");
    return NULL;
}

void* clReleaseProgram(void)
{
    if (verbose) puts("STUB: clReleaseProgram called");
    return NULL;
}

void* clReleaseSampler(void)
{
    if (verbose) puts("STUB: clReleaseSampler called");
    return NULL;
}

void* clRetainCommandQueue(void)
{
    if (verbose) puts("STUB: clRetainCommandQueue called");
    return NULL;
}

void* clRetainContext(void)
{
    if (verbose) puts("STUB: clRetainContext called");
    return NULL;
}

void* clRetainDevice(void)
{
    if (verbose) puts("STUB: clRetainDevice called");
    return NULL;
}

void* clRetainEvent(void)
{
    if (verbose) puts("STUB: clRetainEvent called");
    return NULL;
}

void* clRetainKernel(void)
{
    if (verbose) puts("STUB: clRetainKernel called");
    return NULL;
}

void* clRetainMemObject(void)
{
    if (verbose) puts("STUB: clRetainMemObject called");
    return NULL;
}

void* clRetainProgram(void)
{
    if (verbose) puts("STUB: clRetainProgram called");
    return NULL;
}

void* clRetainSampler(void)
{
    if (verbose) puts("STUB: clRetainSampler called");
    return NULL;
}

void* clSetCommandQueueProperty(void)
{
    if (verbose) puts("STUB: clSetCommandQueueProperty called");
    return NULL;
}

void* clSetEventCallback(void)
{
    if (verbose) puts("STUB: clSetEventCallback called");
    return NULL;
}

void* clSetKernelArg(void)
{
    if (verbose) puts("STUB: clSetKernelArg called");
    return NULL;
}

void* clSetKernelArgByNameAPPLE(void)
{
    if (verbose) puts("STUB: clSetKernelArgByNameAPPLE called");
    return NULL;
}

void* clSetKernelArgsListAPPLE(void)
{
    if (verbose) puts("STUB: clSetKernelArgsListAPPLE called");
    return NULL;
}

void* clSetKernelArgsVaListAPPLE(void)
{
    if (verbose) puts("STUB: clSetKernelArgsVaListAPPLE called");
    return NULL;
}

void* clSetMemObjectDestructorAPPLE(void)
{
    if (verbose) puts("STUB: clSetMemObjectDestructorAPPLE called");
    return NULL;
}

void* clSetMemObjectDestructorCallback(void)
{
    if (verbose) puts("STUB: clSetMemObjectDestructorCallback called");
    return NULL;
}

void* clSetUserEventStatus(void)
{
    if (verbose) puts("STUB: clSetUserEventStatus called");
    return NULL;
}

void* clUnloadCompiler(void)
{
    if (verbose) puts("STUB: clUnloadCompiler called");
    return NULL;
}

void* clUnloadPlatformCompiler(void)
{
    if (verbose) puts("STUB: clUnloadPlatformCompiler called");
    return NULL;
}

void* clWaitForEvents(void)
{
    if (verbose) puts("STUB: clWaitForEvents called");
    return NULL;
}

void* gclBuildProgramAPPLE(void)
{
    if (verbose) puts("STUB: gclBuildProgramAPPLE called");
    return NULL;
}

void* gclBuildProgramBinaryAPPLE(void)
{
    if (verbose) puts("STUB: gclBuildProgramBinaryAPPLE called");
    return NULL;
}

void* gclCreateArgsAPPLE(void)
{
    if (verbose) puts("STUB: gclCreateArgsAPPLE called");
    return NULL;
}

void* gclDeleteArgsAPPLE(void)
{
    if (verbose) puts("STUB: gclDeleteArgsAPPLE called");
    return NULL;
}

void* gclExecKernelAPPLE(void)
{
    if (verbose) puts("STUB: gclExecKernelAPPLE called");
    return NULL;
}

void* gclRegisterBlockKernelMap(void)
{
    if (verbose) puts("STUB: gclRegisterBlockKernelMap called");
    return NULL;
}

void* gclReleaseSampler(void)
{
    if (verbose) puts("STUB: gclReleaseSampler called");
    return NULL;
}

void* gclSetKernelArgAPPLE(void)
{
    if (verbose) puts("STUB: gclSetKernelArgAPPLE called");
    return NULL;
}

void* gclSetKernelArgMemAPPLE(void)
{
    if (verbose) puts("STUB: gclSetKernelArgMemAPPLE called");
    return NULL;
}

void* gclSetKernelArgSamplerAPPLE(void)
{
    if (verbose) puts("STUB: gclSetKernelArgSamplerAPPLE called");
    return NULL;
}

void* gcl_copy_image(void)
{
    if (verbose) puts("STUB: gcl_copy_image called");
    return NULL;
}

void* gcl_copy_image_to_ptr(void)
{
    if (verbose) puts("STUB: gcl_copy_image_to_ptr called");
    return NULL;
}

void* gcl_copy_ptr_to_image(void)
{
    if (verbose) puts("STUB: gcl_copy_ptr_to_image called");
    return NULL;
}

void* gcl_create_buffer_from_ptr(void)
{
    if (verbose) puts("STUB: gcl_create_buffer_from_ptr called");
    return NULL;
}

void* gcl_create_dispatch_queue(void)
{
    if (verbose) puts("STUB: gcl_create_dispatch_queue called");
    return NULL;
}

void* gcl_create_image(void)
{
    if (verbose) puts("STUB: gcl_create_image called");
    return NULL;
}

void* gcl_create_kernel_from_block(void)
{
    if (verbose) puts("STUB: gcl_create_kernel_from_block called");
    return NULL;
}

void* gcl_free(void)
{
    if (verbose) puts("STUB: gcl_free called");
    return NULL;
}

void* gcl_get_context(void)
{
    if (verbose) puts("STUB: gcl_get_context called");
    return NULL;
}

void* gcl_get_device_id_with_dispatch_queue(void)
{
    if (verbose) puts("STUB: gcl_get_device_id_with_dispatch_queue called");
    return NULL;
}

void* gcl_get_kernel_block_workgroup_info(void)
{
    if (verbose) puts("STUB: gcl_get_kernel_block_workgroup_info called");
    return NULL;
}

void* gcl_get_supported_image_formats(void)
{
    if (verbose) puts("STUB: gcl_get_supported_image_formats called");
    return NULL;
}

void* gcl_gl_create_image_from_renderbuffer(void)
{
    if (verbose) puts("STUB: gcl_gl_create_image_from_renderbuffer called");
    return NULL;
}

void* gcl_gl_create_image_from_texture(void)
{
    if (verbose) puts("STUB: gcl_gl_create_image_from_texture called");
    return NULL;
}

void* gcl_gl_create_ptr_from_buffer(void)
{
    if (verbose) puts("STUB: gcl_gl_create_ptr_from_buffer called");
    return NULL;
}

void* gcl_gl_set_sharegroup(void)
{
    if (verbose) puts("STUB: gcl_gl_set_sharegroup called");
    return NULL;
}

void* gcl_log_cl_error(void)
{
    if (verbose) puts("STUB: gcl_log_cl_error called");
    return NULL;
}

void* gcl_log_cl_fatal(void)
{
    if (verbose) puts("STUB: gcl_log_cl_fatal called");
    return NULL;
}

void* gcl_log_error(void)
{
    if (verbose) puts("STUB: gcl_log_error called");
    return NULL;
}

void* gcl_log_fatal(void)
{
    if (verbose) puts("STUB: gcl_log_fatal called");
    return NULL;
}

void* gcl_log_warning(void)
{
    if (verbose) puts("STUB: gcl_log_warning called");
    return NULL;
}

void* gcl_malloc(void)
{
    if (verbose) puts("STUB: gcl_malloc called");
    return NULL;
}

void* gcl_map_image(void)
{
    if (verbose) puts("STUB: gcl_map_image called");
    return NULL;
}

void* gcl_map_ptr(void)
{
    if (verbose) puts("STUB: gcl_map_ptr called");
    return NULL;
}

void* gcl_memcpy(void)
{
    if (verbose) puts("STUB: gcl_memcpy called");
    return NULL;
}

void* gcl_memcpy_rect(void)
{
    if (verbose) puts("STUB: gcl_memcpy_rect called");
    return NULL;
}

void* gcl_release_image(void)
{
    if (verbose) puts("STUB: gcl_release_image called");
    return NULL;
}

void* gcl_retain_image(void)
{
    if (verbose) puts("STUB: gcl_retain_image called");
    return NULL;
}

void* gcl_set_finalizer(void)
{
    if (verbose) puts("STUB: gcl_set_finalizer called");
    return NULL;
}

void* gcl_start_timer(void)
{
    if (verbose) puts("STUB: gcl_start_timer called");
    return NULL;
}

void* gcl_stop_timer(void)
{
    if (verbose) puts("STUB: gcl_stop_timer called");
    return NULL;
}

void* gcl_unmap(void)
{
    if (verbose) puts("STUB: gcl_unmap called");
    return NULL;
}
