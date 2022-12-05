// i'm not sure where to put this header, since it's neither a "basic" header
// nor a framework header (it doesn't even have it's own library from what i can tell)

#ifndef _SOFTLINKING_SOFTLINKING_H_
#define _SOFTLINKING_SOFTLINKING_H_

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <dispatch/dispatch.h>

// certain aspects of this file (such as framework, function, and constant loading)
// can be implemented in plain C, but because it can also load Objective-C classes,
// we have to use `NSBundle`s too, which are Objective-C only
//
// that's okay, though, because it this only seems to be used in Objective-C code

#define SOFT_LINK_STRINGIFY_INTERNAL2(x) #x
#define SOFT_LINK_STRINGIFY_INTERNAL1(x) SOFT_LINK_STRINGIFY_INTERNAL2(x)
#define SOFT_LINK_NSSTRINGIFY(x) @ SOFT_LINK_STRINGIFY_INTERNAL1(x)
#define SOFT_LINK_CFSTRINGIFY(x) CFSTR(SOFT_LINK_STRINGIFY_INTERNAL1(x))

// STUB/HACK: For now just redirect to SOFT_LINK_FRAMEWORK
#define SOFT_LINK_OPTIONAL_FRAMEWORK(_location, _name) SOFT_LINK_FRAMEWORK(_location,_name)

#define SOFT_LINK_FRAMEWORK(_location, _name) \
	static CFBundleRef softlink_get_cfbundle_ ## _name () { \
		static CFBundleRef bundle = NULL; \
		static dispatch_once_t once_handle; \
		dispatch_once(&once_handle, ^{ \
			CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/System/Library/" #_location "/" #_name ".framework"), kCFURLPOSIXPathStyle, true); \
			if (!url) \
				return; \
			bundle = CFBundleCreate(kCFAllocatorDefault, url); \
			CFRelease(url); \
		}); \
		return bundle; \
	}; \
	static NSBundle* softlink_get_nsbundle_ ## _name () { \
		static NSBundle* bundle = nil; \
		static dispatch_once_t once_handle; \
		dispatch_once(&once_handle, ^{ \
			NSString* path = @"/System/Library/" #_location "/" #_name ".framework"; \
			bundle = [NSBundle bundleWithPath: path]; \
		}); \
		return bundle; \
	}; \
	static int is ## _name ## Available () { \
		/* this is probably not the best way to do this */ \
		if (softlink_get_cfbundle_ ## _name ()) \
			return 1; \
		return 0; \
	};

// not sure what the difference between safe and regular is
#define SOFT_LINK_FRAMEWORK_SAFE(_location, _name) SOFT_LINK_FRAMEWORK(_location, _name)

#define SOFT_LINK_CLASS(_framework, _name) \
	static Class get ## _name ## Class () { \
		static Class class = Nil; \
		static dispatch_once_t once_handle; \
		dispatch_once(&once_handle, ^{ \
			NSBundle* bundle = softlink_get_nsbundle_ ## _framework (); \
			class = [bundle classNamed: SOFT_LINK_NSSTRINGIFY(_name)]; \
		}); \
		return class; \
	};

#define SOFT_LINK_CONSTANT(_framework, _name, _type) \
	static _type get ## _name () { \
		static _type* constant = NULL; \
		static dispatch_once_t once_handle; \
		dispatch_once(&once_handle, ^{ \
			CFBundleRef bundle = softlink_get_cfbundle_ ## _framework (); \
			if (bundle) \
				constant = CFBundleGetDataPointerForName(bundle, SOFT_LINK_CFSTRINGIFY(_name)); \
		}); \
		/*
		i'm not sure what to do if the symbol can't be found since
		there's no way to tell the caller that an error occurred
		*/ \
		return *constant; \
	};

// i'm unsure about that last macro parameter: `_call_pattern`
// it seems to just be a repetition of the parameter names
// but i'm guessing that that would be used to modify the arguments to the "soft" variant before passing
// them onto the "hard" function
#define SOFT_LINK_FUNCTION(_framework, _name, _soft_name, _return_type, _parameters, _call_pattern) \
	static _return_type _soft_name _parameters { \
		static _return_type (*func) _parameters = NULL; \
		static dispatch_once_t once_handle; \
		dispatch_once(&once_handle, ^{ \
			CFBundleRef bundle = softlink_get_cfbundle_ ## _framework (); \
			if (bundle) \
				func = CFBundleGetFunctionPointerForName(bundle, SOFT_LINK_CFSTRINGIFY(_name)); \
		}); \
		/* again, here i'm unsure about what to do if the symbol's not there */ \
		return func _call_pattern; \
	};

#endif // _SOFTLINKING_SOFTLINKING_H_
