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

#import <Foundation/Foundation.h>


void* clBuildProgram(void);
void* clCompileProgram(void);
void* clCreateBuffer(void);
void* clCreateCommandQueue(void);
void* clCreateCommandQueueWithPropertiesAPPLE(void);
void* clCreateContext(void);
void* clCreateContextAndCommandQueueAPPLE(void);
void* clCreateContextFromType(void);
void* clCreateDAGAPPLE(void);
void* clCreateEventFromGLsyncKHR(void);
void* clCreateFromGLBuffer(void);
void* clCreateFromGLRenderbuffer(void);
void* clCreateFromGLTexture(void);
void* clCreateFromGLTexture2D(void);
void* clCreateFromGLTexture3D(void);
void* clCreateImage(void);
void* clCreateImage2D(void);
void* clCreateImage3D(void);
void* clCreateImageFromIOSurface2DAPPLE(void);
void* clCreateImageFromIOSurfaceWithPropertiesAPPLE(void);
void* clCreateKernel(void);
void* clCreateKernelFromDAGAPPLE(void);
void* clCreateKernelsInProgram(void);
void* clCreateProgramAndKernelsWithSourceAPPLE(void);
void* clCreateProgramPortableBinaryAPPLE(void);
void* clCreateProgramWithBinary(void);
void* clCreateProgramWithBuiltInKernels(void);
void* clCreateProgramWithSource(void);
void* clCreateSampler(void);
void* clCreateSubBuffer(void);
void* clCreateSubDevices(void);
void* clCreateUserEvent(void);
void* clEnqueueAcquireGLObjects(void);
void* clEnqueueBarrier(void);
void* clEnqueueBarrierWithWaitList(void);
void* clEnqueueCopyBuffer(void);
void* clEnqueueCopyBufferRect(void);
void* clEnqueueCopyBufferToImage(void);
void* clEnqueueCopyImage(void);
void* clEnqueueCopyImageToBuffer(void);
void* clEnqueueFillBuffer(void);
void* clEnqueueFillImage(void);
void* clEnqueueMapBuffer(void);
void* clEnqueueMapImage(void);
void* clEnqueueMarker(void);
void* clEnqueueMarkerWithWaitList(void);
void* clEnqueueMigrateMemObjects(void);
void* clEnqueueNDRangeKernel(void);
void* clEnqueueNativeKernel(void);
void* clEnqueueReadBuffer(void);
void* clEnqueueReadBufferRect(void);
void* clEnqueueReadImage(void);
void* clEnqueueReleaseGLObjects(void);
void* clEnqueueTask(void);
void* clEnqueueUnmapMemObject(void);
void* clEnqueueWaitForEvents(void);
void* clEnqueueWriteBuffer(void);
void* clEnqueueWriteBufferRect(void);
void* clEnqueueWriteImage(void);
void* clFinish(void);
void* clFlush(void);
void* clGetCommandQueueInfo(void);
void* clGetContextInfo(void);
void* clGetDAGNodeAPPLE(void);
void* clGetDeviceIDs(void);
void* clGetDeviceInfo(void);
void* clGetEventInfo(void);
void* clGetEventProfilingInfo(void);
void* clGetExtensionFunctionAddress(void);
void* clGetExtensionFunctionAddressForPlatform(void);
void* clGetGLContextInfoAPPLE(void);
void* clGetGLObjectInfo(void);
void* clGetGLTextureInfo(void);
void* clGetImageInfo(void);
void* clGetKernelArgInfo(void);
void* clGetKernelInfo(void);
void* clGetKernelWorkGroupInfo(void);
void* clGetMemObjectInfo(void);
void* clGetMemoryAnnotationListAPPLE(void);
void* clGetPlatformIDs(void);
void* clGetPlatformInfo(void);
void* clGetProgramBuildInfo(void);
void* clGetProgramInfo(void);
void* clGetSamplerInfo(void);
void* clGetSupportedImageFormats(void);
void* clLinkProgram(void);
void* clLogMessagesToStderrAPPLE(void);
void* clLogMessagesToStdoutAPPLE(void);
void* clLogMessagesToSystemLogAPPLE(void);
void* clReleaseCommandQueue(void);
void* clReleaseContext(void);
void* clReleaseDAGAPPLE(void);
void* clReleaseDevice(void);
void* clReleaseEvent(void);
void* clReleaseKernel(void);
void* clReleaseMemObject(void);
void* clReleaseProgram(void);
void* clReleaseSampler(void);
void* clRetainCommandQueue(void);
void* clRetainContext(void);
void* clRetainDevice(void);
void* clRetainEvent(void);
void* clRetainKernel(void);
void* clRetainMemObject(void);
void* clRetainProgram(void);
void* clRetainSampler(void);
void* clSetCommandQueueProperty(void);
void* clSetEventCallback(void);
void* clSetKernelArg(void);
void* clSetKernelArgByNameAPPLE(void);
void* clSetKernelArgsListAPPLE(void);
void* clSetKernelArgsVaListAPPLE(void);
void* clSetMemObjectDestructorAPPLE(void);
void* clSetMemObjectDestructorCallback(void);
void* clSetUserEventStatus(void);
void* clUnloadCompiler(void);
void* clUnloadPlatformCompiler(void);
void* clWaitForEvents(void);
void* gclBuildProgramAPPLE(void);
void* gclBuildProgramBinaryAPPLE(void);
void* gclCreateArgsAPPLE(void);
void* gclDeleteArgsAPPLE(void);
void* gclExecKernelAPPLE(void);
void* gclRegisterBlockKernelMap(void);
void* gclReleaseSampler(void);
void* gclSetKernelArgAPPLE(void);
void* gclSetKernelArgMemAPPLE(void);
void* gclSetKernelArgSamplerAPPLE(void);
void* gcl_copy_image(void);
void* gcl_copy_image_to_ptr(void);
void* gcl_copy_ptr_to_image(void);
void* gcl_create_buffer_from_ptr(void);
void* gcl_create_dispatch_queue(void);
void* gcl_create_image(void);
void* gcl_create_kernel_from_block(void);
void* gcl_free(void);
void* gcl_get_context(void);
void* gcl_get_device_id_with_dispatch_queue(void);
void* gcl_get_kernel_block_workgroup_info(void);
void* gcl_get_supported_image_formats(void);
void* gcl_gl_create_image_from_renderbuffer(void);
void* gcl_gl_create_image_from_texture(void);
void* gcl_gl_create_ptr_from_buffer(void);
void* gcl_gl_set_sharegroup(void);
void* gcl_log_cl_error(void);
void* gcl_log_cl_fatal(void);
void* gcl_log_error(void);
void* gcl_log_fatal(void);
void* gcl_log_warning(void);
void* gcl_malloc(void);
void* gcl_map_image(void);
void* gcl_map_ptr(void);
void* gcl_memcpy(void);
void* gcl_memcpy_rect(void);
void* gcl_release_image(void);
void* gcl_retain_image(void);
void* gcl_set_finalizer(void);
void* gcl_start_timer(void);
void* gcl_stop_timer(void);
void* gcl_unmap(void);

#endif
