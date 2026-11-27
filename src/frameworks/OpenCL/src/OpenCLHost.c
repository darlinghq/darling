/*
  This file is part of Darling.

  Copyright (C) 2026 Lubos Dolezel

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

// The core OpenCL API is implemented on the host's ICD loader. The loader is a
// normal ELF library, so it is opened and its entry points are resolved through
// elfcalls, the same way OpenGLES reaches the host's EGL and GLES libraries.
// Keeping a typed table here (instead of re-exporting the loader's Mach-O
// wrapper) lets the framework give every entry point its documented Apple SDK
// signature and keeps Apple-only entry points separate.

#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <pthread.h>
#include <elfcalls.h>

extern struct elf_calls *_elfcalls;

// name, return type, full parameter list, argument names, result when the host
// loader is unavailable
#define OPENCL_FORWARDED_FUNCTIONS(X) \
    X(clGetPlatformInfo, cl_int, (cl_platform_id platform, cl_platform_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (platform, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clGetDeviceInfo, cl_int, (cl_device_id device, cl_device_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (device, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clCreateSubDevices, cl_int, (cl_device_id in_device, \
        const cl_device_partition_property *properties, cl_uint num_devices, \
        cl_device_id *out_devices, cl_uint *num_devices_ret), \
        (in_device, properties, num_devices, out_devices, num_devices_ret), CL_INVALID_OPERATION) \
    X(clRetainDevice, cl_int, (cl_device_id device), (device), CL_INVALID_OPERATION) \
    X(clReleaseDevice, cl_int, (cl_device_id device), (device), CL_INVALID_OPERATION) \
    X(clCreateContext, cl_context, (const cl_context_properties *properties, cl_uint num_devices, \
        const cl_device_id *devices, cl_context_destructor_callback pfn_notify, \
        void *user_data, cl_int *errcode_ret), \
        (properties, num_devices, devices, pfn_notify, user_data, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateContextFromType, cl_context, (const cl_context_properties *properties, \
        cl_device_type device_type, cl_context_destructor_callback pfn_notify, \
        void *user_data, cl_int *errcode_ret), \
        (properties, device_type, pfn_notify, user_data, errcode_ret), unavailable_object(errcode_ret)) \
    X(clRetainContext, cl_int, (cl_context context), (context), CL_INVALID_OPERATION) \
    X(clReleaseContext, cl_int, (cl_context context), (context), CL_INVALID_OPERATION) \
    X(clGetContextInfo, cl_int, (cl_context context, cl_context_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (context, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clCreateCommandQueue, cl_command_queue, (cl_context context, cl_device_id device, \
        cl_command_queue_properties properties, cl_int *errcode_ret), \
        (context, device, properties, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateCommandQueueWithProperties, cl_command_queue, (cl_context context, \
        cl_device_id device, const cl_queue_properties *properties, cl_int *errcode_ret), \
        (context, device, properties, errcode_ret), unavailable_object(errcode_ret)) \
    X(clRetainCommandQueue, cl_int, (cl_command_queue command_queue), (command_queue), CL_INVALID_OPERATION) \
    X(clReleaseCommandQueue, cl_int, (cl_command_queue command_queue), (command_queue), CL_INVALID_OPERATION) \
    X(clGetCommandQueueInfo, cl_int, (cl_command_queue command_queue, \
        cl_command_queue_info param_name, size_t param_value_size, void *param_value, \
        size_t *param_value_size_ret), \
        (command_queue, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clSetCommandQueueProperty, cl_int, (cl_command_queue command_queue, \
        cl_command_queue_properties properties, cl_bool enable, \
        cl_command_queue_properties *old_properties), \
        (command_queue, properties, enable, old_properties), CL_INVALID_OPERATION) \
    X(clCreateBuffer, cl_mem, (cl_context context, cl_mem_flags flags, size_t size, \
        void *host_ptr, cl_int *errcode_ret), \
        (context, flags, size, host_ptr, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateSubBuffer, cl_mem, (cl_mem buffer, cl_mem_flags flags, \
        cl_buffer_create_type buffer_create_type, const void *buffer_create_info, \
        cl_int *errcode_ret), \
        (buffer, flags, buffer_create_type, buffer_create_info, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateImage, cl_mem, (cl_context context, cl_mem_flags flags, \
        const cl_image_format *image_format, const cl_image_desc *image_desc, \
        void *host_ptr, cl_int *errcode_ret), \
        (context, flags, image_format, image_desc, host_ptr, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateImage2D, cl_mem, (cl_context context, cl_mem_flags flags, \
        const cl_image_format *image_format, size_t image_width, size_t image_height, \
        size_t image_row_pitch, void *host_ptr, cl_int *errcode_ret), \
        (context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret), \
        unavailable_object(errcode_ret)) \
    X(clCreateImage3D, cl_mem, (cl_context context, cl_mem_flags flags, \
        const cl_image_format *image_format, size_t image_width, size_t image_height, \
        size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch, \
        void *host_ptr, cl_int *errcode_ret), \
        (context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, \
        image_slice_pitch, host_ptr, errcode_ret), unavailable_object(errcode_ret)) \
    X(clRetainMemObject, cl_int, (cl_mem memobj), (memobj), CL_INVALID_OPERATION) \
    X(clReleaseMemObject, cl_int, (cl_mem memobj), (memobj), CL_INVALID_OPERATION) \
    X(clGetSupportedImageFormats, cl_int, (cl_context context, cl_mem_flags flags, \
        cl_mem_object_type image_type, cl_uint num_entries, cl_image_format *image_formats, \
        cl_uint *num_image_formats), \
        (context, flags, image_type, num_entries, image_formats, num_image_formats), CL_INVALID_OPERATION) \
    X(clGetMemObjectInfo, cl_int, (cl_mem memobj, cl_mem_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (memobj, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clGetImageInfo, cl_int, (cl_mem image, cl_image_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (image, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clSetMemObjectDestructorCallback, cl_int, (cl_mem memobj, \
        cl_mem_destructor_callback pfn_notify, void *user_data), \
        (memobj, pfn_notify, user_data), CL_INVALID_OPERATION) \
    X(clCreateSampler, cl_sampler, (cl_context context, cl_bool normalized_coords, \
        cl_addressing_mode addressing_mode, cl_filter_mode filter_mode, cl_int *errcode_ret), \
        (context, normalized_coords, addressing_mode, filter_mode, errcode_ret), \
        unavailable_object(errcode_ret)) \
    X(clRetainSampler, cl_int, (cl_sampler sampler), (sampler), CL_INVALID_OPERATION) \
    X(clReleaseSampler, cl_int, (cl_sampler sampler), (sampler), CL_INVALID_OPERATION) \
    X(clGetSamplerInfo, cl_int, (cl_sampler sampler, cl_sampler_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (sampler, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clCreateProgramWithSource, cl_program, (cl_context context, cl_uint count, \
        const char **strings, const size_t *lengths, cl_int *errcode_ret), \
        (context, count, strings, lengths, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateProgramWithBinary, cl_program, (cl_context context, cl_uint num_devices, \
        const cl_device_id *device_list, const size_t *lengths, const unsigned char **binaries, \
        cl_int *binary_status, cl_int *errcode_ret), \
        (context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret), \
        unavailable_object(errcode_ret)) \
    X(clCreateProgramWithBuiltInKernels, cl_program, (cl_context context, cl_uint num_devices, \
        const cl_device_id *device_list, const char *kernel_names, cl_int *errcode_ret), \
        (context, num_devices, device_list, kernel_names, errcode_ret), unavailable_object(errcode_ret)) \
    X(clRetainProgram, cl_int, (cl_program program), (program), CL_INVALID_OPERATION) \
    X(clReleaseProgram, cl_int, (cl_program program), (program), CL_INVALID_OPERATION) \
    X(clBuildProgram, cl_int, (cl_program program, cl_uint num_devices, \
        const cl_device_id *device_list, const char *options, \
        void (*pfn_notify)(cl_program, void *), void *user_data), \
        (program, num_devices, device_list, options, pfn_notify, user_data), CL_INVALID_OPERATION) \
    X(clCompileProgram, cl_int, (cl_program program, cl_uint num_devices, \
        const cl_device_id *device_list, const char *options, cl_uint num_input_headers, \
        const cl_program *headers, const char **header_include_names, \
        void (*pfn_notify)(cl_program, void *), void *user_data), \
        (program, num_devices, device_list, options, num_input_headers, headers, \
        header_include_names, pfn_notify, user_data), CL_INVALID_OPERATION) \
    X(clLinkProgram, cl_program, (cl_context context, cl_uint num_devices, \
        const cl_device_id *device_list, const char *options, cl_uint num_input_programs, \
        const cl_program *input_programs, void (*pfn_notify)(cl_program, void *), \
        void *user_data, cl_int *errcode_ret), \
        (context, num_devices, device_list, options, num_input_programs, input_programs, \
        pfn_notify, user_data, errcode_ret), unavailable_object(errcode_ret)) \
    X(clUnloadCompiler, cl_int, (void), (), CL_INVALID_OPERATION) \
    X(clUnloadPlatformCompiler, cl_int, (cl_platform_id platform), (platform), CL_INVALID_OPERATION) \
    X(clGetProgramInfo, cl_int, (cl_program program, cl_program_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (program, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clGetProgramBuildInfo, cl_int, (cl_program program, cl_device_id device, \
        cl_program_build_info param_name, size_t param_value_size, void *param_value, \
        size_t *param_value_size_ret), \
        (program, device, param_name, param_value_size, param_value, param_value_size_ret), \
        CL_INVALID_OPERATION) \
    X(clCreateKernel, cl_kernel, (cl_program program, const char *kernel_name, \
        cl_int *errcode_ret), (program, kernel_name, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateKernelsInProgram, cl_int, (cl_program program, cl_uint num_kernels, \
        cl_kernel *kernels, cl_uint *num_kernels_ret), \
        (program, num_kernels, kernels, num_kernels_ret), CL_INVALID_OPERATION) \
    X(clRetainKernel, cl_int, (cl_kernel kernel), (kernel), CL_INVALID_OPERATION) \
    X(clReleaseKernel, cl_int, (cl_kernel kernel), (kernel), CL_INVALID_OPERATION) \
    X(clSetKernelArg, cl_int, (cl_kernel kernel, cl_uint arg_index, size_t arg_size, \
        const void *arg_value), (kernel, arg_index, arg_size, arg_value), CL_INVALID_OPERATION) \
    X(clGetKernelInfo, cl_int, (cl_kernel kernel, cl_kernel_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (kernel, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clGetKernelArgInfo, cl_int, (cl_kernel kernel, cl_uint arg_indx, \
        cl_kernel_arg_info param_name, size_t param_value_size, void *param_value, \
        size_t *param_value_size_ret), \
        (kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret), \
        CL_INVALID_OPERATION) \
    X(clGetKernelWorkGroupInfo, cl_int, (cl_kernel kernel, cl_device_id device, \
        cl_kernel_work_group_info param_name, size_t param_value_size, void *param_value, \
        size_t *param_value_size_ret), \
        (kernel, device, param_name, param_value_size, param_value, param_value_size_ret), \
        CL_INVALID_OPERATION) \
    X(clWaitForEvents, cl_int, (cl_uint num_events, const cl_event *event_list), \
        (num_events, event_list), CL_INVALID_OPERATION) \
    X(clGetEventInfo, cl_int, (cl_event event, cl_event_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (event, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clCreateUserEvent, cl_event, (cl_context context, cl_int *errcode_ret), \
        (context, errcode_ret), unavailable_object(errcode_ret)) \
    X(clRetainEvent, cl_int, (cl_event event), (event), CL_INVALID_OPERATION) \
    X(clReleaseEvent, cl_int, (cl_event event), (event), CL_INVALID_OPERATION) \
    X(clSetUserEventStatus, cl_int, (cl_event event, cl_int execution_status), \
        (event, execution_status), CL_INVALID_OPERATION) \
    X(clSetEventCallback, cl_int, (cl_event event, cl_int command_exec_callback_type, \
        cl_event_callback pfn_notify, void *user_data), \
        (event, command_exec_callback_type, pfn_notify, user_data), CL_INVALID_OPERATION) \
    X(clGetEventProfilingInfo, cl_int, (cl_event event, cl_profiling_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (event, param_name, param_value_size, param_value, param_value_size_ret), CL_INVALID_OPERATION) \
    X(clFlush, cl_int, (cl_command_queue command_queue), (command_queue), CL_INVALID_OPERATION) \
    X(clFinish, cl_int, (cl_command_queue command_queue), (command_queue), CL_INVALID_OPERATION) \
    X(clEnqueueReadBuffer, cl_int, (cl_command_queue command_queue, cl_mem buffer, \
        cl_bool blocking_read, size_t offset, size_t size, void *ptr, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, \
        event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueWriteBuffer, cl_int, (cl_command_queue command_queue, cl_mem buffer, \
        cl_bool blocking_write, size_t offset, size_t size, const void *ptr, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, \
        event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueCopyBuffer, cl_int, (cl_command_queue command_queue, cl_mem src_buffer, \
        cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t size, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, src_buffer, dst_buffer, src_offset, dst_offset, size, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueReadBufferRect, cl_int, (cl_command_queue command_queue, cl_mem buffer, \
        cl_bool blocking_read, const size_t buffer_offset[3], const size_t host_offset[3], \
        const size_t region[3], size_t buffer_row_pitch, size_t buffer_slice_pitch, \
        size_t host_row_pitch, size_t host_slice_pitch, void *ptr, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, buffer, blocking_read, buffer_offset, host_offset, region, \
        buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueWriteBufferRect, cl_int, (cl_command_queue command_queue, cl_mem buffer, \
        cl_bool blocking_write, const size_t buffer_offset[3], const size_t host_offset[3], \
        const size_t region[3], size_t buffer_row_pitch, size_t buffer_slice_pitch, \
        size_t host_row_pitch, size_t host_slice_pitch, const void *ptr, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, buffer, blocking_write, buffer_offset, host_offset, region, \
        buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueCopyBufferRect, cl_int, (cl_command_queue command_queue, cl_mem src_buffer, \
        cl_mem dst_buffer, const size_t src_origin[3], const size_t dst_origin[3], \
        const size_t region[3], size_t src_row_pitch, size_t src_slice_pitch, \
        size_t dst_row_pitch, size_t dst_slice_pitch, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, \
        src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, \
        event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueReadImage, cl_int, (cl_command_queue command_queue, cl_mem image, \
        cl_bool blocking_read, const size_t origin[3], const size_t region[3], \
        size_t row_pitch, size_t slice_pitch, void *ptr, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueWriteImage, cl_int, (cl_command_queue command_queue, cl_mem image, \
        cl_bool blocking_write, const size_t origin[3], const size_t region[3], \
        size_t input_row_pitch, size_t input_slice_pitch, const void *ptr, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, image, blocking_write, origin, region, input_row_pitch, \
        input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueCopyImage, cl_int, (cl_command_queue command_queue, cl_mem src_image, \
        cl_mem dst_image, const size_t src_origin[3], const size_t dst_origin[3], \
        const size_t region[3], cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, src_image, dst_image, src_origin, dst_origin, region, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueCopyImageToBuffer, cl_int, (cl_command_queue command_queue, cl_mem src_image, \
        cl_mem dst_buffer, const size_t src_origin[3], const size_t region[3], \
        size_t dst_offset, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, src_image, dst_buffer, src_origin, region, dst_offset, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueCopyBufferToImage, cl_int, (cl_command_queue command_queue, cl_mem src_buffer, \
        cl_mem dst_image, size_t src_offset, const size_t dst_origin[3], \
        const size_t region[3], cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, src_buffer, dst_image, src_offset, dst_origin, region, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueMapBuffer, void *, (cl_command_queue command_queue, cl_mem buffer, \
        cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t size, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, \
        cl_int *errcode_ret), \
        (command_queue, buffer, blocking_map, map_flags, offset, size, \
        num_events_in_wait_list, event_wait_list, event, errcode_ret), unavailable_object(errcode_ret)) \
    X(clEnqueueMapImage, void *, (cl_command_queue command_queue, cl_mem image, \
        cl_bool blocking_map, cl_map_flags map_flags, const size_t origin[3], \
        const size_t region[3], size_t *image_row_pitch, size_t *image_slice_pitch, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, \
        cl_int *errcode_ret), \
        (command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, \
        image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret), \
        unavailable_object(errcode_ret)) \
    X(clEnqueueUnmapMemObject, cl_int, (cl_command_queue command_queue, cl_mem memobj, \
        void *mapped_ptr, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event), \
        CL_INVALID_OPERATION) \
    X(clEnqueueNDRangeKernel, cl_int, (cl_command_queue command_queue, cl_kernel kernel, \
        cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, \
        const size_t *local_work_size, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueTask, cl_int, (cl_command_queue command_queue, cl_kernel kernel, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, kernel, num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueNativeKernel, cl_int, (cl_command_queue command_queue, \
        cl_native_kernel_callback user_func, void *args, size_t cb_args, \
        cl_uint num_mem_objects, const cl_mem *mem_list, const void **args_mem_loc, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, \
        num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueMarker, cl_int, (cl_command_queue command_queue, cl_event *event), \
        (command_queue, event), CL_INVALID_OPERATION) \
    X(clEnqueueWaitForEvents, cl_int, (cl_command_queue command_queue, cl_uint num_events, \
        const cl_event *event_list), (command_queue, num_events, event_list), CL_INVALID_OPERATION) \
    X(clEnqueueBarrier, cl_int, (cl_command_queue command_queue), (command_queue), CL_INVALID_OPERATION) \
    X(clEnqueueFillBuffer, cl_int, (cl_command_queue command_queue, cl_mem buffer, \
        const void *pattern, size_t pattern_size, size_t offset, size_t size, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, \
        event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueFillImage, cl_int, (cl_command_queue command_queue, cl_mem image, \
        const void *fill_color, const size_t origin[3], const size_t region[3], \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, image, fill_color, origin, region, num_events_in_wait_list, \
        event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueMigrateMemObjects, cl_int, (cl_command_queue command_queue, \
        cl_uint num_mem_objects, const cl_mem *mem_objects, cl_mem_migration_flags flags, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, \
        event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueMarkerWithWaitList, cl_int, (cl_command_queue command_queue, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clEnqueueBarrierWithWaitList, cl_int, (cl_command_queue command_queue, \
        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event), \
        (command_queue, num_events_in_wait_list, event_wait_list, event), CL_INVALID_OPERATION) \
    X(clGetGLContextInfoKHR, cl_int, (const cl_context_properties *properties, \
        cl_gl_context_info param_name, size_t param_value_size, void *param_value, \
        size_t *param_value_size_ret), \
        (properties, param_name, param_value_size, param_value, param_value_size_ret), \
        CL_INVALID_OPERATION) \
    X(clCreateFromGLBuffer, cl_mem, (cl_context context, cl_mem_flags flags, cl_GLuint bufobj, \
        cl_int *errcode_ret), (context, flags, bufobj, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateFromGLTexture, cl_mem, (cl_context context, cl_mem_flags flags, cl_GLenum target, \
        cl_GLint miplevel, cl_GLuint texture, cl_int *errcode_ret), \
        (context, flags, target, miplevel, texture, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateFromGLTexture2D, cl_mem, (cl_context context, cl_mem_flags flags, cl_GLenum target, \
        cl_GLint miplevel, cl_GLuint texture, cl_int *errcode_ret), \
        (context, flags, target, miplevel, texture, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateFromGLTexture3D, cl_mem, (cl_context context, cl_mem_flags flags, cl_GLenum target, \
        cl_GLint miplevel, cl_GLuint texture, cl_int *errcode_ret), \
        (context, flags, target, miplevel, texture, errcode_ret), unavailable_object(errcode_ret)) \
    X(clCreateFromGLRenderbuffer, cl_mem, (cl_context context, cl_mem_flags flags, \
        cl_GLuint renderbuffer, cl_int *errcode_ret), \
        (context, flags, renderbuffer, errcode_ret), unavailable_object(errcode_ret)) \
    X(clGetGLObjectInfo, cl_int, (cl_mem memobj, cl_gl_object_type *gl_object_type, \
        cl_GLuint *gl_object_name), (memobj, gl_object_type, gl_object_name), CL_INVALID_OPERATION) \
    X(clGetGLTextureInfo, cl_int, (cl_mem memobj, cl_gl_texture_info param_name, \
        size_t param_value_size, void *param_value, size_t *param_value_size_ret), \
        (memobj, param_name, param_value_size, param_value, param_value_size_ret), \
        CL_INVALID_OPERATION) \
    X(clEnqueueAcquireGLObjects, cl_int, (cl_command_queue command_queue, cl_uint num_objects, \
        const cl_mem *mem_objects, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), \
        CL_INVALID_OPERATION) \
    X(clEnqueueReleaseGLObjects, cl_int, (cl_command_queue command_queue, cl_uint num_objects, \
        const cl_mem *mem_objects, cl_uint num_events_in_wait_list, \
        const cl_event *event_wait_list, cl_event *event), \
        (command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event), \
        CL_INVALID_OPERATION) \
    X(clCreateEventFromGLsyncKHR, cl_event, (cl_context context, cl_GLsync sync, \
        cl_int *errcode_ret), (context, sync, errcode_ret), unavailable_object(errcode_ret)) \
    X(clGetExtensionFunctionAddress, void *, (const char *func_name), (func_name), NULL) \
    X(clGetExtensionFunctionAddressForPlatform, void *, (cl_platform_id platform, \
        const char *func_name), (platform, func_name), NULL)

#define OPENCL_HOST_FUNCTIONS(X) \
    OPENCL_FORWARDED_FUNCTIONS(X) \
    X(clGetPlatformIDs, cl_int, (cl_uint num_entries, cl_platform_id *platforms, \
        cl_uint *num_platforms), (num_entries, platforms, num_platforms), CL_INVALID_OPERATION) \
    X(clGetDeviceIDs, cl_int, (cl_platform_id platform, cl_device_type device_type, \
        cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices), \
        (platform, device_type, num_entries, devices, num_devices), CL_INVALID_OPERATION)

#define OPENCL_DECLARE_FUNCTION(name, ret, params, args, fallback) \
    typedef ret (*opencl_host_##name##_fn) params; \
    static opencl_host_##name##_fn opencl_host_##name;

OPENCL_HOST_FUNCTIONS(OPENCL_DECLARE_FUNCTION)

#undef OPENCL_DECLARE_FUNCTION

static void *opencl_host_library;
static pthread_once_t opencl_load_once = PTHREAD_ONCE_INIT;

static void opencl_load(void)
{
    if (_elfcalls == NULL || _elfcalls->dlopen == NULL || _elfcalls->dlsym == NULL)
        return;

    static const char *const names[] = { "libOpenCL.so.1", "libOpenCL.so", NULL };
    for (const char *const *name = names; *name != NULL; name++) {
        opencl_host_library = _elfcalls->dlopen(*name);
        if (opencl_host_library != NULL)
            break;
    }
    if (opencl_host_library == NULL)
        return;

#define OPENCL_LOAD_FUNCTION(name, ret, params, args, fallback) \
    opencl_host_##name = (opencl_host_##name##_fn) _elfcalls->dlsym(opencl_host_library, #name);

    OPENCL_HOST_FUNCTIONS(OPENCL_LOAD_FUNCTION)

#undef OPENCL_LOAD_FUNCTION
}

static int opencl_ready(void)
{
    pthread_once(&opencl_load_once, opencl_load);
    return opencl_host_library != NULL;
}

static void *unavailable_object(cl_int *errcode_ret)
{
    if (errcode_ret != NULL)
        *errcode_ret = CL_INVALID_OPERATION;
    return NULL;
}

#define OPENCL_DEFINE_FUNCTION(name, ret, params, args, fallback) \
    ret name params \
    { \
        if (!opencl_ready() || opencl_host_##name == NULL) \
            return (fallback); \
        return opencl_host_##name args; \
    }

OPENCL_FORWARDED_FUNCTIONS(OPENCL_DEFINE_FUNCTION)

#undef OPENCL_DEFINE_FUNCTION

cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
{
    if ((num_entries == 0 && platforms != NULL) || (platforms == NULL && num_platforms == NULL))
        return CL_INVALID_VALUE;
    if (!opencl_ready() || opencl_host_clGetPlatformIDs == NULL) {
        if (num_platforms != NULL)
            *num_platforms = 0;
        // An ICD loader without an installed ICD reports this extension error.
        return CL_PLATFORM_NOT_FOUND_KHR;
    }
    return opencl_host_clGetPlatformIDs(num_entries, platforms, num_platforms);
}

cl_int clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries,
                      cl_device_id *devices, cl_uint *num_devices)
{
    const cl_device_type valid_types = CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CPU |
                                       CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR |
                                       CL_DEVICE_TYPE_CUSTOM | CL_DEVICE_TYPE_ALL;
    if (platform == NULL)
        return CL_INVALID_PLATFORM;
    if ((device_type & ~valid_types) != 0 || device_type == 0)
        return CL_INVALID_DEVICE_TYPE;
    if ((num_entries == 0 && devices != NULL) || (devices == NULL && num_devices == NULL))
        return CL_INVALID_VALUE;
    if (!opencl_ready() || opencl_host_clGetDeviceIDs == NULL) {
        if (num_devices != NULL)
            *num_devices = 0;
        return CL_DEVICE_NOT_FOUND;
    }
    return opencl_host_clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}
