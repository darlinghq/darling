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

#ifndef _CL_GL_H_
#define _CL_GL_H_

#include <OpenCL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int cl_GLint;
typedef unsigned int cl_GLenum;
typedef unsigned int cl_GLuint;
typedef struct __GLsync *cl_GLsync;
typedef cl_uint cl_gl_context_info;
typedef cl_uint cl_gl_object_type;
typedef cl_uint cl_gl_texture_info;
typedef cl_uint cl_gl_platform_info;

#define CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR -1000
#define CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR 0x2006
#define CL_DEVICES_FOR_GL_CONTEXT_KHR 0x2007
#define CL_GL_CONTEXT_KHR ((cl_context_properties) 0x2008)
#define CL_EGL_DISPLAY_KHR ((cl_context_properties) 0x2009)
#define CL_GLX_DISPLAY_KHR ((cl_context_properties) 0x200A)
#define CL_WGL_HDC_KHR ((cl_context_properties) 0x200B)
#define CL_CGL_SHAREGROUP_KHR ((cl_context_properties) 0x200C)

#define CL_GL_OBJECT_BUFFER 0x2000
#define CL_GL_OBJECT_TEXTURE2D 0x2001
#define CL_GL_OBJECT_TEXTURE3D 0x2002
#define CL_GL_OBJECT_RENDERBUFFER 0x2003
#define CL_GL_OBJECT_TEXTURE2D_ARRAY 0x200E
#define CL_GL_OBJECT_TEXTURE1D 0x200F
#define CL_GL_OBJECT_TEXTURE1D_ARRAY 0x2010
#define CL_GL_OBJECT_TEXTURE_BUFFER 0x2011
#define CL_GL_TEXTURE_TARGET 0x2004
#define CL_GL_MIPMAP_LEVEL 0x2005
#define CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR 0x200D
#define CL_GL_NUM_SAMPLES 0x2012

cl_int clGetGLContextInfoKHR(const cl_context_properties *properties, cl_gl_context_info param_name,
                             size_t param_value_size, void *param_value,
                             size_t *param_value_size_ret);
cl_mem clCreateFromGLBuffer(cl_context context, cl_mem_flags flags, cl_GLuint bufobj,
                            cl_int *errcode_ret);
cl_mem clCreateFromGLTexture(cl_context context, cl_mem_flags flags, cl_GLenum target,
                             cl_GLint miplevel, cl_GLuint texture, cl_int *errcode_ret);
cl_mem clCreateFromGLTexture2D(cl_context context, cl_mem_flags flags, cl_GLenum target,
                               cl_GLint miplevel, cl_GLuint texture, cl_int *errcode_ret);
cl_mem clCreateFromGLTexture3D(cl_context context, cl_mem_flags flags, cl_GLenum target,
                               cl_GLint miplevel, cl_GLuint texture, cl_int *errcode_ret);
cl_mem clCreateFromGLRenderbuffer(cl_context context, cl_mem_flags flags, cl_GLuint renderbuffer,
                                  cl_int *errcode_ret);
cl_int clGetGLObjectInfo(cl_mem memobj, cl_gl_object_type *gl_object_type,
                         cl_GLuint *gl_object_name);
cl_int clGetGLTextureInfo(cl_mem memobj, cl_gl_texture_info param_name, size_t param_value_size,
                          void *param_value, size_t *param_value_size_ret);
cl_int clEnqueueAcquireGLObjects(cl_command_queue command_queue, cl_uint num_objects,
                                 const cl_mem *mem_objects, cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list, cl_event *event);
cl_int clEnqueueReleaseGLObjects(cl_command_queue command_queue, cl_uint num_objects,
                                 const cl_mem *mem_objects, cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list, cl_event *event);
cl_event clCreateEventFromGLsyncKHR(cl_context context, cl_GLsync sync, cl_int *errcode_ret);

#ifdef __cplusplus
}
#endif

#endif
