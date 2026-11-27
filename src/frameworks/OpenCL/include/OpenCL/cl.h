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

#ifndef _CL_H_
#define _CL_H_

#include <OpenCL/cl_platform.h>
#include <stdarg.h>

// Source compatibility with code written against the Khronos headers. On the
// platforms Darling targets these calling-convention and visibility macros
// are all empty.
#define CL_API_ENTRY
#define CL_API_CALL
#define CL_CALLBACK
#define CL_EXTENSION_WEAK_LINK

#ifdef __cplusplus
extern "C" {
#endif

// Opaque object handles.
typedef struct _cl_platform_id *cl_platform_id;
typedef struct _cl_device_id *cl_device_id;
typedef struct _cl_context *cl_context;
typedef struct _cl_command_queue *cl_command_queue;
typedef struct _cl_mem *cl_mem;
typedef struct _cl_program *cl_program;
typedef struct _cl_kernel *cl_kernel;
typedef struct _cl_event *cl_event;
typedef struct _cl_sampler *cl_sampler;

typedef cl_uint cl_bool;
typedef cl_ulong cl_bitfield;
typedef cl_bitfield cl_device_type;
typedef cl_uint cl_platform_info;
typedef cl_uint cl_device_info;
typedef cl_bitfield cl_device_fp_config;
typedef cl_uint cl_device_mem_cache_type;
typedef cl_uint cl_device_local_mem_type;
typedef cl_bitfield cl_device_exec_capabilities;
typedef cl_bitfield cl_command_queue_properties;
typedef intptr_t cl_queue_properties;
typedef intptr_t cl_device_partition_property;
typedef cl_bitfield cl_device_affinity_domain;
typedef intptr_t cl_context_properties;
typedef cl_uint cl_context_info;
typedef cl_uint cl_command_queue_info;
typedef cl_uint cl_channel_order;
typedef cl_uint cl_channel_type;
typedef cl_bitfield cl_mem_flags;
typedef cl_uint cl_mem_object_type;
typedef cl_uint cl_mem_info;
typedef cl_bitfield cl_mem_migration_flags;
typedef cl_uint cl_image_info;
typedef cl_uint cl_buffer_create_type;
typedef cl_uint cl_addressing_mode;
typedef cl_uint cl_filter_mode;
typedef cl_uint cl_sampler_info;
typedef cl_bitfield cl_map_flags;
typedef cl_uint cl_program_info;
typedef cl_uint cl_program_build_info;
typedef cl_uint cl_program_binary_type;
typedef cl_int cl_build_status;
typedef cl_uint cl_kernel_info;
typedef cl_uint cl_kernel_arg_info;
typedef cl_uint cl_kernel_arg_address_qualifier;
typedef cl_uint cl_kernel_arg_access_qualifier;
typedef cl_bitfield cl_kernel_arg_type_qualifier;
typedef cl_uint cl_kernel_work_group_info;
typedef cl_uint cl_event_info;
typedef cl_uint cl_command_type;
typedef cl_uint cl_profiling_info;

typedef struct _cl_image_format {
	cl_channel_order image_channel_order;
	cl_channel_type image_channel_data_type;
} cl_image_format;

typedef struct _cl_image_desc {
	cl_mem_object_type image_type;
	size_t image_width;
	size_t image_height;
	size_t image_depth;
	size_t image_array_size;
	size_t image_row_pitch;
	size_t image_slice_pitch;
	cl_uint num_mip_levels;
	cl_uint num_samples;
	cl_mem buffer;
} cl_image_desc;

typedef struct _cl_buffer_region {
	size_t origin;
	size_t size;
} cl_buffer_region;

#define CL_FALSE 0
#define CL_TRUE 1
#define CL_BLOCKING CL_TRUE
#define CL_NON_BLOCKING CL_FALSE

// Error codes.
#define CL_SUCCESS 0
#define CL_DEVICE_NOT_FOUND -1
#define CL_DEVICE_NOT_AVAILABLE -2
#define CL_COMPILER_NOT_AVAILABLE -3
#define CL_MEM_OBJECT_ALLOCATION_FAILURE -4
#define CL_OUT_OF_RESOURCES -5
#define CL_OUT_OF_HOST_MEMORY -6
#define CL_PROFILING_INFO_NOT_AVAILABLE -7
#define CL_MEM_COPY_OVERLAP -8
#define CL_IMAGE_FORMAT_MISMATCH -9
#define CL_IMAGE_FORMAT_NOT_SUPPORTED -10
#define CL_BUILD_PROGRAM_FAILURE -11
#define CL_MAP_FAILURE -12
#define CL_MISALIGNED_SUB_BUFFER_OFFSET -13
#define CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST -14
#define CL_COMPILE_PROGRAM_FAILURE -15
#define CL_LINKER_NOT_AVAILABLE -16
#define CL_LINK_PROGRAM_FAILURE -17
#define CL_DEVICE_PARTITION_FAILED -18
#define CL_KERNEL_ARG_INFO_NOT_AVAILABLE -19
#define CL_INVALID_VALUE -30
#define CL_INVALID_DEVICE_TYPE -31
#define CL_INVALID_PLATFORM -32
#define CL_INVALID_DEVICE -33
#define CL_INVALID_CONTEXT -34
#define CL_INVALID_QUEUE_PROPERTIES -35
#define CL_INVALID_COMMAND_QUEUE -36
#define CL_INVALID_HOST_PTR -37
#define CL_INVALID_MEM_OBJECT -38
#define CL_INVALID_IMAGE_FORMAT_DESCRIPTOR -39
#define CL_INVALID_IMAGE_SIZE -40
#define CL_INVALID_SAMPLER -41
#define CL_INVALID_BINARY -42
#define CL_INVALID_BUILD_OPTIONS -43
#define CL_INVALID_PROGRAM -44
#define CL_INVALID_PROGRAM_EXECUTABLE -45
#define CL_INVALID_KERNEL_NAME -46
#define CL_INVALID_KERNEL_DEFINITION -47
#define CL_INVALID_KERNEL -48
#define CL_INVALID_ARG_INDEX -49
#define CL_INVALID_ARG_VALUE -50
#define CL_INVALID_ARG_SIZE -51
#define CL_INVALID_KERNEL_ARGS -52
#define CL_INVALID_WORK_DIMENSION -53
#define CL_INVALID_WORK_GROUP_SIZE -54
#define CL_INVALID_WORK_ITEM_SIZE -55
#define CL_INVALID_GLOBAL_OFFSET -56
#define CL_INVALID_EVENT_WAIT_LIST -57
#define CL_INVALID_EVENT -58
#define CL_INVALID_OPERATION -59
#define CL_INVALID_GL_OBJECT -60
#define CL_INVALID_BUFFER_SIZE -61
#define CL_INVALID_MIP_LEVEL -62
#define CL_INVALID_GLOBAL_WORK_SIZE -63
#define CL_INVALID_PROPERTY -64
#define CL_INVALID_IMAGE_DESCRIPTOR -65
#define CL_INVALID_COMPILER_OPTIONS -66
#define CL_INVALID_LINKER_OPTIONS -67
#define CL_INVALID_DEVICE_PARTITION_COUNT -68

// Returned by ICD loaders that have no vendor ICD installed.
#ifndef CL_PLATFORM_NOT_FOUND_KHR
#define CL_PLATFORM_NOT_FOUND_KHR -1001
#endif

#define CL_VERSION_1_0 1
#define CL_VERSION_1_1 1
#define CL_VERSION_1_2 1

#define CL_PLATFORM_PROFILE 0x0900
#define CL_PLATFORM_VERSION 0x0901
#define CL_PLATFORM_NAME 0x0902
#define CL_PLATFORM_VENDOR 0x0903
#define CL_PLATFORM_EXTENSIONS 0x0904

#define CL_DEVICE_TYPE_DEFAULT ((cl_device_type) 1 << 0)
#define CL_DEVICE_TYPE_CPU ((cl_device_type) 1 << 1)
#define CL_DEVICE_TYPE_GPU ((cl_device_type) 1 << 2)
#define CL_DEVICE_TYPE_ACCELERATOR ((cl_device_type) 1 << 3)
#define CL_DEVICE_TYPE_CUSTOM ((cl_device_type) 1 << 4)
#define CL_DEVICE_TYPE_ALL ((cl_device_type) 0xFFFFFFFF)

#define CL_DEVICE_TYPE 0x1000
#define CL_DEVICE_VENDOR_ID 0x1001
#define CL_DEVICE_MAX_COMPUTE_UNITS 0x1002
#define CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS 0x1003
#define CL_DEVICE_MAX_WORK_GROUP_SIZE 0x1004
#define CL_DEVICE_MAX_WORK_ITEM_SIZES 0x1005
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR 0x1006
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT 0x1007
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT 0x1008
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG 0x1009
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT 0x100A
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE 0x100B
#define CL_DEVICE_MAX_CLOCK_FREQUENCY 0x100C
#define CL_DEVICE_ADDRESS_BITS 0x100D
#define CL_DEVICE_MAX_READ_IMAGE_ARGS 0x100E
#define CL_DEVICE_MAX_WRITE_IMAGE_ARGS 0x100F
#define CL_DEVICE_MAX_MEM_ALLOC_SIZE 0x1010
#define CL_DEVICE_IMAGE2D_MAX_WIDTH 0x1011
#define CL_DEVICE_IMAGE2D_MAX_HEIGHT 0x1012
#define CL_DEVICE_IMAGE3D_MAX_WIDTH 0x1013
#define CL_DEVICE_IMAGE3D_MAX_HEIGHT 0x1014
#define CL_DEVICE_IMAGE3D_MAX_DEPTH 0x1015
#define CL_DEVICE_IMAGE_SUPPORT 0x1016
#define CL_DEVICE_MAX_PARAMETER_SIZE 0x1017
#define CL_DEVICE_MAX_SAMPLERS 0x1018
#define CL_DEVICE_MEM_BASE_ADDR_ALIGN 0x1019
#define CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE 0x101A
#define CL_DEVICE_SINGLE_FP_CONFIG 0x101B
#define CL_DEVICE_GLOBAL_MEM_CACHE_TYPE 0x101C
#define CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE 0x101D
#define CL_DEVICE_GLOBAL_MEM_CACHE_SIZE 0x101E
#define CL_DEVICE_GLOBAL_MEM_SIZE 0x101F
#define CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE 0x1020
#define CL_DEVICE_MAX_CONSTANT_ARGS 0x1021
#define CL_DEVICE_LOCAL_MEM_TYPE 0x1022
#define CL_DEVICE_LOCAL_MEM_SIZE 0x1023
#define CL_DEVICE_ERROR_CORRECTION_SUPPORT 0x1024
#define CL_DEVICE_PROFILING_TIMER_RESOLUTION 0x1025
#define CL_DEVICE_ENDIAN_LITTLE 0x1026
#define CL_DEVICE_AVAILABLE 0x1027
#define CL_DEVICE_COMPILER_AVAILABLE 0x1028
#define CL_DEVICE_EXECUTION_CAPABILITIES 0x1029
#define CL_DEVICE_QUEUE_PROPERTIES 0x102A
#define CL_DEVICE_NAME 0x102B
#define CL_DEVICE_VENDOR 0x102C
#define CL_DRIVER_VERSION 0x102D
#define CL_DEVICE_PROFILE 0x102E
#define CL_DEVICE_VERSION 0x102F
#define CL_DEVICE_EXTENSIONS 0x1030
#define CL_DEVICE_PLATFORM 0x1031
#define CL_DEVICE_DOUBLE_FP_CONFIG 0x1032
#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF 0x1034
#define CL_DEVICE_HOST_UNIFIED_MEMORY 0x1035
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR 0x1036
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT 0x1037
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_INT 0x1038
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG 0x1039
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT 0x103A
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE 0x103B
#define CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF 0x103C
#define CL_DEVICE_OPENCL_C_VERSION 0x103D
#define CL_DEVICE_LINKER_AVAILABLE 0x103E
#define CL_DEVICE_BUILT_IN_KERNELS 0x103F
#define CL_DEVICE_IMAGE_MAX_BUFFER_SIZE 0x1040
#define CL_DEVICE_IMAGE_MAX_ARRAY_SIZE 0x1041
#define CL_DEVICE_PARENT_DEVICE 0x1042
#define CL_DEVICE_PARTITION_MAX_SUB_DEVICES 0x1043
#define CL_DEVICE_PARTITION_PROPERTIES 0x1044
#define CL_DEVICE_PARTITION_AFFINITY_DOMAIN 0x1045
#define CL_DEVICE_PARTITION_TYPE 0x1046
#define CL_DEVICE_REFERENCE_COUNT 0x1047
#define CL_DEVICE_PREFERRED_INTEROP_USER_SYNC 0x1048
#define CL_DEVICE_PRINTF_BUFFER_SIZE 0x1049

#define CL_FP_DENORM ((cl_device_fp_config) 1 << 0)
#define CL_FP_INF_NAN ((cl_device_fp_config) 1 << 1)
#define CL_FP_ROUND_TO_NEAREST ((cl_device_fp_config) 1 << 2)
#define CL_FP_ROUND_TO_ZERO ((cl_device_fp_config) 1 << 3)
#define CL_FP_ROUND_TO_INF ((cl_device_fp_config) 1 << 4)
#define CL_FP_FMA ((cl_device_fp_config) 1 << 5)
#define CL_FP_SOFT_FLOAT ((cl_device_fp_config) 1 << 6)
#define CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT ((cl_device_fp_config) 1 << 7)

#define CL_NONE 0
#define CL_READ_ONLY_CACHE 1
#define CL_READ_WRITE_CACHE 2
#define CL_LOCAL 1
#define CL_GLOBAL 2
#define CL_EXEC_KERNEL ((cl_device_exec_capabilities) 1 << 0)
#define CL_EXEC_NATIVE_KERNEL ((cl_device_exec_capabilities) 1 << 1)

#define CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ((cl_command_queue_properties) 1 << 0)
#define CL_QUEUE_PROFILING_ENABLE ((cl_command_queue_properties) 1 << 1)

#define CL_CONTEXT_REFERENCE_COUNT 0x1080
#define CL_CONTEXT_DEVICES 0x1081
#define CL_CONTEXT_PROPERTIES 0x1082
#define CL_CONTEXT_NUM_DEVICES 0x1083
#define CL_CONTEXT_PLATFORM ((cl_context_properties) 0x1084)
#define CL_CONTEXT_INTEROP_USER_SYNC ((cl_context_properties) 0x1085)

#define CL_DEVICE_PARTITION_EQUALLY ((cl_device_partition_property) 0x1086)
#define CL_DEVICE_PARTITION_BY_COUNTS ((cl_device_partition_property) 0x1087)
#define CL_DEVICE_PARTITION_BY_COUNTS_LIST_END ((cl_device_partition_property) 0)
#define CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN ((cl_device_partition_property) 0x1088)
#define CL_DEVICE_AFFINITY_DOMAIN_NUMA ((cl_device_affinity_domain) 1 << 0)
#define CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE ((cl_device_affinity_domain) 1 << 1)
#define CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE ((cl_device_affinity_domain) 1 << 2)
#define CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE ((cl_device_affinity_domain) 1 << 3)
#define CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE ((cl_device_affinity_domain) 1 << 4)
#define CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE ((cl_device_affinity_domain) 1 << 5)

#define CL_QUEUE_CONTEXT 0x1090
#define CL_QUEUE_DEVICE 0x1091
#define CL_QUEUE_REFERENCE_COUNT 0x1092
#define CL_QUEUE_PROPERTIES 0x1093

#define CL_MEM_READ_WRITE ((cl_mem_flags) 1 << 0)
#define CL_MEM_WRITE_ONLY ((cl_mem_flags) 1 << 1)
#define CL_MEM_READ_ONLY ((cl_mem_flags) 1 << 2)
#define CL_MEM_USE_HOST_PTR ((cl_mem_flags) 1 << 3)
#define CL_MEM_ALLOC_HOST_PTR ((cl_mem_flags) 1 << 4)
#define CL_MEM_COPY_HOST_PTR ((cl_mem_flags) 1 << 5)
#define CL_MEM_HOST_WRITE_ONLY ((cl_mem_flags) 1 << 7)
#define CL_MEM_HOST_READ_ONLY ((cl_mem_flags) 1 << 8)
#define CL_MEM_HOST_NO_ACCESS ((cl_mem_flags) 1 << 9)
#define CL_MIGRATE_MEM_OBJECT_HOST ((cl_mem_migration_flags) 1 << 0)
#define CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED ((cl_mem_migration_flags) 1 << 1)

#define CL_R 0x10B0
#define CL_A 0x10B1
#define CL_RG 0x10B2
#define CL_RA 0x10B3
#define CL_RGB 0x10B4
#define CL_RGBA 0x10B5
#define CL_BGRA 0x10B6
#define CL_ARGB 0x10B7
#define CL_INTENSITY 0x10B8
#define CL_LUMINANCE 0x10B9
#define CL_Rx 0x10BA
#define CL_RGx 0x10BB
#define CL_RGBx 0x10BC
#define CL_DEPTH 0x10BD
#define CL_DEPTH_STENCIL 0x10BE
#define CL_sRGB 0x10BF
#define CL_sRGBx 0x10C0
#define CL_sRGBA 0x10C1
#define CL_sBGRA 0x10C2
#define CL_ABGR 0x10C3

#define CL_SNORM_INT8 0x10D0
#define CL_SNORM_INT16 0x10D1
#define CL_UNORM_INT8 0x10D2
#define CL_UNORM_INT16 0x10D3
#define CL_UNORM_SHORT_565 0x10D4
#define CL_UNORM_SHORT_555 0x10D5
#define CL_UNORM_INT_101010 0x10D6
#define CL_SIGNED_INT8 0x10D7
#define CL_SIGNED_INT16 0x10D8
#define CL_SIGNED_INT32 0x10D9
#define CL_UNSIGNED_INT8 0x10DA
#define CL_UNSIGNED_INT16 0x10DB
#define CL_UNSIGNED_INT32 0x10DC
#define CL_HALF_FLOAT 0x10DD
#define CL_FLOAT 0x10DE
#define CL_UNORM_INT_101010_2 0x10E0

#define CL_MEM_OBJECT_BUFFER 0x10F0
#define CL_MEM_OBJECT_IMAGE2D 0x10F1
#define CL_MEM_OBJECT_IMAGE3D 0x10F2
#define CL_MEM_OBJECT_IMAGE2D_ARRAY 0x10F3
#define CL_MEM_OBJECT_IMAGE1D 0x10F4
#define CL_MEM_OBJECT_IMAGE1D_ARRAY 0x10F5
#define CL_MEM_OBJECT_IMAGE1D_BUFFER 0x10F6

#define CL_MEM_TYPE 0x1100
#define CL_MEM_FLAGS 0x1101
#define CL_MEM_SIZE 0x1102
#define CL_MEM_HOST_PTR 0x1103
#define CL_MEM_MAP_COUNT 0x1104
#define CL_MEM_REFERENCE_COUNT 0x1105
#define CL_MEM_CONTEXT 0x1106
#define CL_MEM_ASSOCIATED_MEMOBJECT 0x1107
#define CL_MEM_OFFSET 0x1108

#define CL_IMAGE_FORMAT 0x1110
#define CL_IMAGE_ELEMENT_SIZE 0x1111
#define CL_IMAGE_ROW_PITCH 0x1112
#define CL_IMAGE_SLICE_PITCH 0x1113
#define CL_IMAGE_WIDTH 0x1114
#define CL_IMAGE_HEIGHT 0x1115
#define CL_IMAGE_DEPTH 0x1116
#define CL_IMAGE_ARRAY_SIZE 0x1117
#define CL_IMAGE_BUFFER 0x1118
#define CL_IMAGE_NUM_MIP_LEVELS 0x1119
#define CL_IMAGE_NUM_SAMPLES 0x111A

#define CL_ADDRESS_NONE 0x1130
#define CL_ADDRESS_CLAMP_TO_EDGE 0x1131
#define CL_ADDRESS_CLAMP 0x1132
#define CL_ADDRESS_REPEAT 0x1133
#define CL_ADDRESS_MIRRORED_REPEAT 0x1134
#define CL_FILTER_NEAREST 0x1140
#define CL_FILTER_LINEAR 0x1141

#define CL_SAMPLER_REFERENCE_COUNT 0x1150
#define CL_SAMPLER_CONTEXT 0x1151
#define CL_SAMPLER_NORMALIZED_COORDS 0x1152
#define CL_SAMPLER_ADDRESSING_MODE 0x1153
#define CL_SAMPLER_FILTER_MODE 0x1154

#define CL_MAP_READ ((cl_map_flags) 1 << 0)
#define CL_MAP_WRITE ((cl_map_flags) 1 << 1)
#define CL_MAP_WRITE_INVALIDATE_REGION ((cl_map_flags) 1 << 2)

#define CL_PROGRAM_REFERENCE_COUNT 0x1160
#define CL_PROGRAM_CONTEXT 0x1161
#define CL_PROGRAM_NUM_DEVICES 0x1162
#define CL_PROGRAM_DEVICES 0x1163
#define CL_PROGRAM_SOURCE 0x1164
#define CL_PROGRAM_BINARY_SIZES 0x1165
#define CL_PROGRAM_BINARIES 0x1166
#define CL_PROGRAM_NUM_KERNELS 0x1167
#define CL_PROGRAM_KERNEL_NAMES 0x1168
#define CL_PROGRAM_BUILD_STATUS 0x1181
#define CL_PROGRAM_BUILD_OPTIONS 0x1182
#define CL_PROGRAM_BUILD_LOG 0x1183
#define CL_PROGRAM_BINARY_TYPE 0x1184
#define CL_PROGRAM_BINARY_TYPE_NONE 0x0
#define CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT 0x1
#define CL_PROGRAM_BINARY_TYPE_LIBRARY 0x2
#define CL_PROGRAM_BINARY_TYPE_EXECUTABLE 0x4

#define CL_BUILD_SUCCESS 0
#define CL_BUILD_NONE -1
#define CL_BUILD_ERROR -2
#define CL_BUILD_IN_PROGRESS -3

#define CL_KERNEL_FUNCTION_NAME 0x1190
#define CL_KERNEL_NUM_ARGS 0x1191
#define CL_KERNEL_REFERENCE_COUNT 0x1192
#define CL_KERNEL_CONTEXT 0x1193
#define CL_KERNEL_PROGRAM 0x1194
#define CL_KERNEL_ATTRIBUTES 0x1195
#define CL_KERNEL_ARG_ADDRESS_QUALIFIER 0x1196
#define CL_KERNEL_ARG_ACCESS_QUALIFIER 0x1197
#define CL_KERNEL_ARG_TYPE_NAME 0x1198
#define CL_KERNEL_ARG_TYPE_QUALIFIER 0x1199
#define CL_KERNEL_ARG_NAME 0x119A
#define CL_KERNEL_ARG_ADDRESS_GLOBAL 0x119B
#define CL_KERNEL_ARG_ADDRESS_LOCAL 0x119C
#define CL_KERNEL_ARG_ADDRESS_CONSTANT 0x119D
#define CL_KERNEL_ARG_ADDRESS_PRIVATE 0x119E
#define CL_KERNEL_ARG_ACCESS_READ_ONLY 0x11A0
#define CL_KERNEL_ARG_ACCESS_WRITE_ONLY 0x11A1
#define CL_KERNEL_ARG_ACCESS_READ_WRITE 0x11A2
#define CL_KERNEL_ARG_ACCESS_NONE 0x11A3
#define CL_KERNEL_ARG_TYPE_NONE 0
#define CL_KERNEL_ARG_TYPE_CONST ((cl_kernel_arg_type_qualifier) 1 << 0)
#define CL_KERNEL_ARG_TYPE_RESTRICT ((cl_kernel_arg_type_qualifier) 1 << 1)
#define CL_KERNEL_ARG_TYPE_VOLATILE ((cl_kernel_arg_type_qualifier) 1 << 2)

#define CL_KERNEL_WORK_GROUP_SIZE 0x11B0
#define CL_KERNEL_COMPILE_WORK_GROUP_SIZE 0x11B1
#define CL_KERNEL_LOCAL_MEM_SIZE 0x11B2
#define CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE 0x11B3
#define CL_KERNEL_PRIVATE_MEM_SIZE 0x11B4
#define CL_KERNEL_GLOBAL_WORK_SIZE 0x11B5

#define CL_EVENT_COMMAND_QUEUE 0x11D0
#define CL_EVENT_COMMAND_TYPE 0x11D1
#define CL_EVENT_REFERENCE_COUNT 0x11D2
#define CL_EVENT_COMMAND_EXECUTION_STATUS 0x11D3
#define CL_EVENT_CONTEXT 0x11D4

#define CL_COMMAND_NDRANGE_KERNEL 0x11F0
#define CL_COMMAND_TASK 0x11F1
#define CL_COMMAND_NATIVE_KERNEL 0x11F2
#define CL_COMMAND_READ_BUFFER 0x11F3
#define CL_COMMAND_WRITE_BUFFER 0x11F4
#define CL_COMMAND_COPY_BUFFER 0x11F5
#define CL_COMMAND_READ_IMAGE 0x11F6
#define CL_COMMAND_WRITE_IMAGE 0x11F7
#define CL_COMMAND_COPY_IMAGE 0x11F8
#define CL_COMMAND_COPY_IMAGE_TO_BUFFER 0x11F9
#define CL_COMMAND_COPY_BUFFER_TO_IMAGE 0x11FA
#define CL_COMMAND_MAP_BUFFER 0x11FB
#define CL_COMMAND_MAP_IMAGE 0x11FC
#define CL_COMMAND_UNMAP_MEM_OBJECT 0x11FD
#define CL_COMMAND_MARKER 0x11FE
#define CL_COMMAND_ACQUIRE_GL_OBJECTS 0x11FF
#define CL_COMMAND_RELEASE_GL_OBJECTS 0x1200
#define CL_COMMAND_READ_BUFFER_RECT 0x1201
#define CL_COMMAND_WRITE_BUFFER_RECT 0x1202
#define CL_COMMAND_COPY_BUFFER_RECT 0x1203
#define CL_COMMAND_USER 0x1204
#define CL_COMMAND_BARRIER 0x1205
#define CL_COMMAND_MIGRATE_MEM_OBJECTS 0x1206
#define CL_COMMAND_FILL_BUFFER 0x1207
#define CL_COMMAND_FILL_IMAGE 0x1208

#define CL_COMPLETE 0x0
#define CL_RUNNING 0x1
#define CL_SUBMITTED 0x2
#define CL_QUEUED 0x3

#define CL_BUFFER_CREATE_TYPE_REGION 0x1220
#define CL_PROFILING_COMMAND_QUEUED 0x1280
#define CL_PROFILING_COMMAND_SUBMIT 0x1281
#define CL_PROFILING_COMMAND_START 0x1282
#define CL_PROFILING_COMMAND_END 0x1283

// Callback types used by the object lifetime and event APIs.
typedef void (*cl_context_destructor_callback)(const char *errinfo, const void *private_info,
                                               size_t cb, void *user_data);
typedef void (*cl_mem_destructor_callback)(cl_mem memobj, void *user_data);
typedef void (*cl_event_callback)(cl_event event, cl_int event_command_exec_status, void *user_data);
typedef void (*cl_native_kernel_callback)(void *user_data);

// Platform API.
cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms);
cl_int clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size,
                         void *param_value, size_t *param_value_size_ret);

// Device APIs.
cl_int clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries,
                      cl_device_id *devices, cl_uint *num_devices);
cl_int clGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size,
                       void *param_value, size_t *param_value_size_ret);
cl_int clCreateSubDevices(cl_device_id in_device, const cl_device_partition_property *properties,
                          cl_uint num_devices, cl_device_id *out_devices, cl_uint *num_devices_ret);
cl_int clRetainDevice(cl_device_id device);
cl_int clReleaseDevice(cl_device_id device);

// Context APIs.
cl_context clCreateContext(const cl_context_properties *properties, cl_uint num_devices,
                           const cl_device_id *devices, cl_context_destructor_callback pfn_notify,
                           void *user_data, cl_int *errcode_ret);
cl_context clCreateContextFromType(const cl_context_properties *properties, cl_device_type device_type,
                                   cl_context_destructor_callback pfn_notify, void *user_data,
                                   cl_int *errcode_ret);
cl_int clRetainContext(cl_context context);
cl_int clReleaseContext(cl_context context);
cl_int clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size,
                        void *param_value, size_t *param_value_size_ret);

// Command queue APIs.
cl_command_queue clCreateCommandQueue(cl_context context, cl_device_id device,
                                      cl_command_queue_properties properties, cl_int *errcode_ret);
cl_int clRetainCommandQueue(cl_command_queue command_queue);
cl_int clReleaseCommandQueue(cl_command_queue command_queue);
cl_int clGetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name,
                             size_t param_value_size, void *param_value, size_t *param_value_size_ret);
cl_int clSetCommandQueueProperty(cl_command_queue command_queue,
                                 cl_command_queue_properties properties, cl_bool enable,
                                 cl_command_queue_properties *old_properties);
cl_command_queue clCreateCommandQueueWithProperties(cl_context context, cl_device_id device,
                                                    const cl_queue_properties *properties,
                                                    cl_int *errcode_ret);

// Memory object APIs.
cl_mem clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr,
                      cl_int *errcode_ret);
cl_mem clCreateSubBuffer(cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type,
                         const void *buffer_create_info, cl_int *errcode_ret);
cl_mem clCreateImage(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                     const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret);
cl_mem clCreateImage2D(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                       size_t image_width, size_t image_height, size_t image_row_pitch,
                       void *host_ptr, cl_int *errcode_ret);
cl_mem clCreateImage3D(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                       size_t image_width, size_t image_height, size_t image_depth,
                       size_t image_row_pitch, size_t image_slice_pitch, void *host_ptr,
                       cl_int *errcode_ret);
cl_int clRetainMemObject(cl_mem memobj);
cl_int clReleaseMemObject(cl_mem memobj);
cl_int clGetSupportedImageFormats(cl_context context, cl_mem_flags flags,
                                  cl_mem_object_type image_type, cl_uint num_entries,
                                  cl_image_format *image_formats, cl_uint *num_image_formats);
cl_int clGetMemObjectInfo(cl_mem memobj, cl_mem_info param_name, size_t param_value_size,
                          void *param_value, size_t *param_value_size_ret);
cl_int clGetImageInfo(cl_mem image, cl_image_info param_name, size_t param_value_size,
                      void *param_value, size_t *param_value_size_ret);
cl_int clSetMemObjectDestructorCallback(cl_mem memobj, cl_mem_destructor_callback pfn_notify,
                                        void *user_data);

// Sampler APIs.
cl_sampler clCreateSampler(cl_context context, cl_bool normalized_coords,
                           cl_addressing_mode addressing_mode, cl_filter_mode filter_mode,
                           cl_int *errcode_ret);
cl_int clRetainSampler(cl_sampler sampler);
cl_int clReleaseSampler(cl_sampler sampler);
cl_int clGetSamplerInfo(cl_sampler sampler, cl_sampler_info param_name, size_t param_value_size,
                        void *param_value, size_t *param_value_size_ret);

// Program APIs.
cl_program clCreateProgramWithSource(cl_context context, cl_uint count, const char **strings,
                                     const size_t *lengths, cl_int *errcode_ret);
cl_program clCreateProgramWithBinary(cl_context context, cl_uint num_devices,
                                     const cl_device_id *device_list, const size_t *lengths,
                                     const unsigned char **binaries, cl_int *binary_status,
                                     cl_int *errcode_ret);
cl_program clCreateProgramWithBuiltInKernels(cl_context context, cl_uint num_devices,
                                             const cl_device_id *device_list,
                                             const char *kernel_names, cl_int *errcode_ret);
cl_int clRetainProgram(cl_program program);
cl_int clReleaseProgram(cl_program program);
cl_int clBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id *device_list,
                      const char *options, void (*pfn_notify)(cl_program, void *), void *user_data);
cl_int clCompileProgram(cl_program program, cl_uint num_devices, const cl_device_id *device_list,
                        const char *options, cl_uint num_input_headers, const cl_program *headers,
                        const char **header_include_names,
                        void (*pfn_notify)(cl_program, void *), void *user_data);
cl_program clLinkProgram(cl_context context, cl_uint num_devices, const cl_device_id *device_list,
                         const char *options, cl_uint num_input_programs,
                         const cl_program *input_programs,
                         void (*pfn_notify)(cl_program, void *), void *user_data,
                         cl_int *errcode_ret);
cl_int clUnloadCompiler(void);
cl_int clUnloadPlatformCompiler(cl_platform_id platform);
cl_int clGetProgramInfo(cl_program program, cl_program_info param_name, size_t param_value_size,
                        void *param_value, size_t *param_value_size_ret);
cl_int clGetProgramBuildInfo(cl_program program, cl_device_id device,
                             cl_program_build_info param_name, size_t param_value_size,
                             void *param_value, size_t *param_value_size_ret);

// Kernel APIs.
cl_kernel clCreateKernel(cl_program program, const char *kernel_name, cl_int *errcode_ret);
cl_int clCreateKernelsInProgram(cl_program program, cl_uint num_kernels, cl_kernel *kernels,
                                cl_uint *num_kernels_ret);
cl_int clRetainKernel(cl_kernel kernel);
cl_int clReleaseKernel(cl_kernel kernel);
cl_int clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);
cl_int clGetKernelInfo(cl_kernel kernel, cl_kernel_info param_name, size_t param_value_size,
                       void *param_value, size_t *param_value_size_ret);
cl_int clGetKernelArgInfo(cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info param_name,
                          size_t param_value_size, void *param_value, size_t *param_value_size_ret);
cl_int clGetKernelWorkGroupInfo(cl_kernel kernel, cl_device_id device,
                                cl_kernel_work_group_info param_name, size_t param_value_size,
                                void *param_value, size_t *param_value_size_ret);

// Event APIs.
cl_int clWaitForEvents(cl_uint num_events, const cl_event *event_list);
cl_int clGetEventInfo(cl_event event, cl_event_info param_name, size_t param_value_size,
                      void *param_value, size_t *param_value_size_ret);
cl_event clCreateUserEvent(cl_context context, cl_int *errcode_ret);
cl_int clRetainEvent(cl_event event);
cl_int clReleaseEvent(cl_event event);
cl_int clSetUserEventStatus(cl_event event, cl_int execution_status);
cl_int clSetEventCallback(cl_event event, cl_int command_exec_callback_type,
                          cl_event_callback pfn_notify, void *user_data);
cl_int clGetEventProfilingInfo(cl_event event, cl_profiling_info param_name,
                               size_t param_value_size, void *param_value,
                               size_t *param_value_size_ret);

// Flush and finish.
cl_int clFlush(cl_command_queue command_queue);
cl_int clFinish(cl_command_queue command_queue);

// Enqueued commands.
cl_int clEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read,
                           size_t offset, size_t size, void *ptr, cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write,
                            size_t offset, size_t size, const void *ptr,
                            cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                            cl_event *event);
cl_int clEnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer,
                           size_t src_offset, size_t dst_offset, size_t size,
                           cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                           cl_event *event);
cl_int clEnqueueReadBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read,
                               const size_t buffer_offset[3], const size_t host_offset[3],
                               const size_t region[3], size_t buffer_row_pitch,
                               size_t buffer_slice_pitch, size_t host_row_pitch,
                               size_t host_slice_pitch, void *ptr,
                               cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                               cl_event *event);
cl_int clEnqueueWriteBufferRect(cl_command_queue command_queue, cl_mem buffer,
                                cl_bool blocking_write, const size_t buffer_offset[3],
                                const size_t host_offset[3], const size_t region[3],
                                size_t buffer_row_pitch, size_t buffer_slice_pitch,
                                size_t host_row_pitch, size_t host_slice_pitch, const void *ptr,
                                cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                                cl_event *event);
cl_int clEnqueueCopyBufferRect(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer,
                               const size_t src_origin[3], const size_t dst_origin[3],
                               const size_t region[3], size_t src_row_pitch,
                               size_t src_slice_pitch, size_t dst_row_pitch,
                               size_t dst_slice_pitch, cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read,
                          const size_t origin[3], const size_t region[3], size_t row_pitch,
                          size_t slice_pitch, void *ptr, cl_uint num_events_in_wait_list,
                          const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write,
                           const size_t origin[3], const size_t region[3], size_t input_row_pitch,
                           size_t input_slice_pitch, const void *ptr,
                           cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                           cl_event *event);
cl_int clEnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image,
                          const size_t src_origin[3], const size_t dst_origin[3],
                          const size_t region[3], cl_uint num_events_in_wait_list,
                          const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer,
                                  const size_t src_origin[3], const size_t region[3],
                                  size_t dst_offset, cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image,
                                  size_t src_offset, const size_t dst_origin[3],
                                  const size_t region[3], cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list, cl_event *event);
void *clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map,
                         cl_map_flags map_flags, size_t offset, size_t size,
                         cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                         cl_event *event, cl_int *errcode_ret);
void *clEnqueueMapImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map,
                        cl_map_flags map_flags, const size_t origin[3], const size_t region[3],
                        size_t *image_row_pitch, size_t *image_slice_pitch,
                        cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                        cl_event *event, cl_int *errcode_ret);
cl_int clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr,
                               cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                               cl_event *event);
cl_int clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim,
                              const size_t *global_work_offset, const size_t *global_work_size,
                              const size_t *local_work_size, cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueTask(cl_command_queue command_queue, cl_kernel kernel,
                     cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                     cl_event *event);
cl_int clEnqueueNativeKernel(cl_command_queue command_queue,
                             cl_native_kernel_callback user_func, void *args, size_t cb_args,
                             cl_uint num_mem_objects, const cl_mem *mem_list,
                             const void **args_mem_loc, cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueMarker(cl_command_queue command_queue, cl_event *event);
cl_int clEnqueueWaitForEvents(cl_command_queue command_queue, cl_uint num_events,
                              const cl_event *event_list);
cl_int clEnqueueBarrier(cl_command_queue command_queue);
cl_int clEnqueueFillBuffer(cl_command_queue command_queue, cl_mem buffer, const void *pattern,
                           size_t pattern_size, size_t offset, size_t size,
                           cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                           cl_event *event);
cl_int clEnqueueFillImage(cl_command_queue command_queue, cl_mem image, const void *fill_color,
                          const size_t origin[3], const size_t region[3],
                          cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                          cl_event *event);
cl_int clEnqueueMigrateMemObjects(cl_command_queue command_queue, cl_uint num_mem_objects,
                                  const cl_mem *mem_objects, cl_mem_migration_flags flags,
                                  cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                                  cl_event *event);
cl_int clEnqueueMarkerWithWaitList(cl_command_queue command_queue, cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueBarrierWithWaitList(cl_command_queue command_queue, cl_uint num_events_in_wait_list,
                                    const cl_event *event_wait_list, cl_event *event);

// Extension function lookup.
void *clGetExtensionFunctionAddress(const char *func_name);
void *clGetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char *func_name);

// Apple's argument batching. It is layered on clSetKernelArg and is not an ICD entry point.
cl_int clSetKernelArgsVaListAPPLE(cl_kernel kernel, cl_uint num_args, va_list args);
cl_int clSetKernelArgsListAPPLE(cl_kernel kernel, cl_uint num_args, ...);

#ifdef __cplusplus
}
#endif

#endif
