project(libplatform)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__ -DPRIVATE)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/private
	${CMAKE_CURRENT_SOURCE_DIR}/internal
	${CMAKE_SOURCE_DIR}/src/libc/derived
	${CMAKE_SOURCE_DIR}/src/libc/darwin
	${CMAKE_SOURCE_DIR}/src/kernel/libsyscall
	${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)

set(platform_x86_64_sources
	src/setjmp/x86_64/_setjmp.S
	src/setjmp/x86_64/setjmp.S
	src/setjmp/x86_64/_sigtramp.S
	src/ucontext/x86_64/_ctx_start.S
	src/ucontext/x86_64/getcontext.S
	src/ucontext/x86_64/_setcontext.S
	src/cachecontrol/x86_64/cache.S
	src/atomics/x86_64/pfz.S
	src/atomics/x86_64/OSAtomic.S
)
add_library(platform_x86_64 OBJECT ${platform_x86_64_sources})
set_target_properties(platform_x86_64 PROPERTIES COMPILE_FLAGS "-arch x86_64")

set(platform_i386_sources
	src/setjmp/i386/_setjmp.S
	src/setjmp/i386/setjmp.S
	src/setjmp/i386/_sigtramp.S
	src/ucontext/i386/_ctx_start.S
	src/ucontext/i386/getcontext.S
	src/ucontext/i386/_setcontext.S
	src/atomics/i386/pfz.S
	src/atomics/i386/OSAtomic.S
	src/cachecontrol/i386/cache.S
)
add_library(platform_i386 OBJECT ${platform_i386_sources})
set_target_properties(platform_i386 PROPERTIES COMPILE_FLAGS "-arch i386")

set(platform_sources
	src/setjmp/generic/sigtramp.c
	src/setjmp/generic/setjmperr.c
	src/os/semaphore.c
	src/os/atomic_up.c
	src/os/lock.c
	src/os/lock_wfe.c
	src/os/lock_up.c
	src/os/resolver.c
	src/os/alloc_once.c
	src/os/atomic.c
	src/ucontext/generic/getmcontext.c
	src/ucontext/generic/makecontext.c
	src/ucontext/generic/swapcontext.c
	src/ucontext/generic/setcontext.c
	src/force_libplatform_to_build.c
	src/init.c
	src/cachecontrol/generic/cache.c
	src/string/generic/memccpy.c
	src/string/generic/memmove.c
	src/string/generic/strcmp.c
	src/string/generic/flsll.c
	src/string/generic/memcmp.c
	src/string/generic/strchr.c
	src/string/generic/ffsll.c
	src/string/generic/strncmp.c
	src/string/generic/bzero.c
	src/string/generic/memset_pattern.c
	src/string/generic/memchr.c
	src/atomics/common/MKGetTimeBaseInfo.c
	src/atomics/init.c
	src/simple/string_io.c
	src/simple/asl.c
	src/simple/getenv.c
	#src/introspection/introspection.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libplatform.dylib")
add_darling_library(platform SHARED ${platform_sources} $<TARGET_OBJECTS:platform_i386> $<TARGET_OBJECTS:platform_x86_64>)
make_fat(platform)

target_link_libraries(platform PRIVATE system_kernel system_dyld compiler_rt)

#set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
#set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ranlib")
#add_library(platform_static STATIC ${platform_sources})
#add_dependencies(platform_static ranlib x86_64-apple-darwin11-ar)

install(TARGETS platform DESTINATION libexec/darling/usr/lib/system)
