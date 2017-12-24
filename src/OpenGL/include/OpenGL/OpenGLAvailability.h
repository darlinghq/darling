#ifndef _OpenGLAvailability_H
#define _OpenGLAvailability_H

#include <Availability.h>

#if !defined(__has_extension)
	#define __has_extension(x) 0
#endif

#define OPENGL_AVAILABLE(from) __OSX_AVAILABLE_STARTING(__MAC_##from, __IPHONE_NA)
#define OPENGL_DEPRECATED(from, to) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_##from, __MAC_##to, __IPHONE_NA, __IPHONE_NA)
#define OPENGL_DEPRECATED_MSG(from, to, msg) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_##from, __MAC_##to, __IPHONE_NA, __IPHONE_NA, "" #msg "")

#if __has_extension(enumerator_attributes)
	#define OPENGL_ENUM_AVAILABLE(from) OPENGL_AVAILABLE(from)
	#define OPENGL_ENUM_DEPRECATED(from, to) OPENGL_DEPRECATED(from, to)
	#define OPENGL_ENUM_DEPRECATED_MSG(from, to, msg) OPENGL_DEPRECATED_MSG(from, to, msg)
#else
	#define OPENGL_ENUM_AVAILABLE(from)
	#define OPENGL_ENUM_DEPRECATED(from, to)
	#define OPENGL_ENUM_DEPRECATED_MSG(from, to, msg)
#endif

#endif