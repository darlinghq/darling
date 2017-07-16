project(darling-src)

cmake_minimum_required(VERSION 2.4.0)
cmake_policy(SET CMP0005 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(mig)

add_definitions(-D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE -DDARLING)
set(DARLING TRUE)

add_definitions(-D__APPLE__ -D__DYNAMIC__)

execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND git log -1 --format=%h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(darling-config.h.in darling-config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

include(compiler_include)
GetCompilerInclude(COMPILER_INC_PATH)
message(STATUS "Compiler include path detected as ${COMPILER_INC_PATH}")
include_directories(${COMPILER_INC_PATH})

add_subdirectory(bootstrap_cmds)
add_subdirectory(external/cctools-port/cctools/ld64/src)
add_subdirectory(external/cctools-port/cctools/ar)
#add_subdirectory(external/cctools-port/cctools/as)
add_subdirectory(external/cctools-port/cctools/misc)
#add_subdirectory(util)
#add_subdirectory(libmach-o)
#add_subdirectory(libdyld)
add_subdirectory(startup)

set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ranlib")


#if (FRAMEWORK_COREAUDIO)
#	add_subdirectory(AudioToolbox)
#	add_subdirectory(AudioUnit)
#endif (FRAMEWORK_COREAUDIO)
#if (FRAMEWORK_APPKIT)
#	add_subdirectory(external/AppKit)
#endif (FRAMEWORK_APPKIT)
#add_subdirectory(IOKit)

add_definitions(-target x86_64-apple-darwin11)
include(darling_lib)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/libc/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/libplatform/include
	${CMAKE_SOURCE_DIR}/platform-include
	${CMAKE_SOURCE_DIR}/platform-include/pthread
	${CMAKE_CURRENT_SOURCE_DIR}/libc/derived
	${CMAKE_CURRENT_SOURCE_DIR}/libc/include/NetBSD
	${CMAKE_CURRENT_SOURCE_DIR}/libc/include/FreeBSD
	${CMAKE_CURRENT_SOURCE_DIR}/kernel/libsyscall/wrappers
	${CMAKE_CURRENT_SOURCE_DIR}/kernel/include
	${CMAKE_SOURCE_DIR}/kernel-include
	${CMAKE_CURRENT_SOURCE_DIR}/duct/include
	${CMAKE_CURRENT_SOURCE_DIR}/libm/include
)

include_directories(AFTER
	${CMAKE_CURRENT_SOURCE_DIR}/external/SmartCardServices/include
	${CMAKE_CURRENT_SOURCE_DIR}/login/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/IOKitUser/include
	${CMAKE_CURRENT_SOURCE_DIR}/launchd/liblaunch/include
	${CMAKE_CURRENT_SOURCE_DIR}/sandbox/include
	${CMAKE_CURRENT_SOURCE_DIR}/CoreServices/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/coretls/include
	${CMAKE_SOURCE_DIR}/src/libmalloc/include
	${CMAKE_SOURCE_DIR}/src/libinfo/include
	${CMAKE_SOURCE_DIR}/src/external/syslog/libsystem_asl.tproj/include
	${CMAKE_SOURCE_DIR}/src/external/openssl/src/include
	${CMAKE_SOURCE_DIR}/kernel-include/iokit
	${CMAKE_SOURCE_DIR}/src/external/IOKitUser
	${CMAKE_CURRENT_SOURCE_DIR}/external/openpam/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite/include
	${CMAKE_CURRENT_SOURCE_DIR}/DebugSymbols/include
	${CMAKE_CURRENT_SOURCE_DIR}/OpenScripting/include
	${CMAKE_SOURCE_DIR}/src/CryptoTokenKit/include
	${CMAKE_SOURCE_DIR}/src/LocalAuthentication/include
	${CMAKE_CURRENT_SOURCE_DIR}/AppleSystemInfo/include
	${CMAKE_CURRENT_SOURCE_DIR}/SystemConfiguration/include
	${CMAKE_CURRENT_SOURCE_DIR}/libiconv/include
	${CMAKE_CURRENT_SOURCE_DIR}/ncurses/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/corefoundation/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/foundation/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/syslog/libsystem_asl.tproj/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/objc4/runtime/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/corecrypto/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/security/include
	${CMAKE_CURRENT_SOURCE_DIR}/xcselect/include
	${CMAKE_CURRENT_SOURCE_DIR}/libaks/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/icu/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/libpthread/include
	${CMAKE_CURRENT_SOURCE_DIR}/libremovefile/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/libdispatch/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/libclosure/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cfnetwork/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cfnetwork/private_include
	${CMAKE_CURRENT_SOURCE_DIR}/external/libxml2/include
	${CMAKE_CURRENT_BINARY_DIR}/external/libxml2/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/expat/include
	${CMAKE_CURRENT_SOURCE_DIR}/libedit/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/bzip2/include
	${CMAKE_CURRENT_SOURCE_DIR}/libnotify/include
	${CMAKE_CURRENT_SOURCE_DIR}/opendirectory/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/commoncrypto/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/coregraphics/Headers
	${CMAKE_CURRENT_SOURCE_DIR}/quarantine/include
	${CMAKE_CURRENT_SOURCE_DIR}/copyfile/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/include
	${CMAKE_CURRENT_SOURCE_DIR}/xar/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/DirectoryService/include
	${CMAKE_CURRENT_SOURCE_DIR}/DiskImages/include
)

add_subdirectory(external/libkqueue)


add_subdirectory(external/libplatform)
add_subdirectory(external/libpthread)
add_subdirectory(kernel)
add_subdirectory(libc)

add_definitions(-D_LIBC_NO_FEATURE_VERIFICATION)
add_subdirectory(external/darling-dmg)
add_subdirectory(libm)
add_subdirectory(libmacho)
add_subdirectory(libgcc)
add_subdirectory(copyfile)
add_subdirectory(libinfo)
add_subdirectory(quarantine)
add_subdirectory(libmalloc)
add_subdirectory(libunwind)
add_subdirectory(libsystem)
add_subdirectory(libsystem_coreservices)
add_subdirectory(libutil)
add_subdirectory(libnotify)
add_subdirectory(libremovefile)
add_subdirectory(launchd)
add_subdirectory(keymgr)
add_subdirectory(ncurses)
add_subdirectory(libiconv)
add_subdirectory(duct/src)
add_subdirectory(libresolv)
add_subdirectory(external/corecrypto)
add_subdirectory(external/coretls)
add_subdirectory(libffi)
add_subdirectory(xtrace)
add_subdirectory(dyld)
add_subdirectory(external/objc4/runtime)
add_subdirectory(external/syslog)
add_subdirectory(external/libdispatch)
add_subdirectory(external/zlib)
add_subdirectory(external/bzip2)
add_subdirectory(external/libxml2)
add_subdirectory(external/libxslt)
add_subdirectory(external/icu/icuSources)
add_subdirectory(external/commoncrypto)
add_subdirectory(external/corefoundation)
add_subdirectory(external/openssl/src)
#add_subdirectory(external/configd/SystemConfiguration.fproj)
add_subdirectory(external/foundation)
add_subdirectory(external/curl)
add_subdirectory(external/liblzma)
#add_subdirectory(external/cfnetwork/src)
add_subdirectory(external/pcre)
add_subdirectory(external/sqlite)
add_subdirectory(external/openpam)
add_subdirectory(external/libtelnet)
add_subdirectory(external/remote_cmds)
add_subdirectory(external/IOKitUser)
add_subdirectory(external/python_modules)
add_subdirectory(SystemConfiguration)
add_subdirectory(CoreServices)
add_subdirectory(ApplicationServices)
#add_subdirectory(VideoDecodeAcceleration)
add_subdirectory(DiskArbitration)
add_subdirectory(LocalAuthentication)
add_subdirectory(CryptoTokenKit)
add_subdirectory(xar)
add_subdirectory(libedit)
add_subdirectory(external/libclosure)
add_subdirectory(external/compiler-rt/lib/builtins)
add_subdirectory(csu)
add_subdirectory(external/python/2.6/Python-2.6.9)
add_subdirectory(external/python/2.7/Python-2.7.10)
add_subdirectory(external/ruby)
add_subdirectory(external/expat)
#add_subdirectory(external/libauto)
add_subdirectory(external/libarchive/libarchive)
add_subdirectory(external/apr)
add_subdirectory(sandbox)
add_subdirectory(xcselect)
add_subdirectory(ConfigurationProfiles)
add_subdirectory(AppleSystemInfo)
add_subdirectory(DiskImages)
add_subdirectory(PackageKit)
#add_subdirectory(Cocoa)

add_subdirectory(external/file/file)
add_subdirectory(external/libxpc)
add_subdirectory(external/openssl_certificates)

add_subdirectory(external/shell_cmds)
add_subdirectory(external/file_cmds)
add_subdirectory(external/text_cmds)
add_subdirectory(external/adv_cmds)
#add_subdirectory(external/network_cmds)
add_subdirectory(external/system_cmds)
add_subdirectory(external/bash)
add_subdirectory(external/tcsh)
add_subdirectory(external/less)
add_subdirectory(external/grep)
add_subdirectory(external/awk)
add_subdirectory(external/groff)
add_subdirectory(external/nano)
add_subdirectory(external/man)
add_subdirectory(external/bc)
add_subdirectory(external/vim)
add_subdirectory(external/zip/zip/zip30)
add_subdirectory(external/zip/unzip/unzip-5.52)
add_subdirectory(tools)
add_subdirectory(external/installer)
add_subdirectory(external/gnutar/gnutar)
add_subdirectory(external/gpatch)
add_subdirectory(external/gnudiff)
add_subdirectory(external/openssh)
#add_subdirectory(external/top)
add_subdirectory(external/perl)
add_subdirectory(external/mail_cmds)
add_subdirectory(external/screen)
add_subdirectory(unxip)
add_subdirectory(external/zsh)
add_subdirectory(external/doc_cmds)
add_subdirectory(external/basic_cmds)
add_subdirectory(external/misc_cmds)
add_subdirectory(external/patch_cmds)
add_subdirectory(external/DSTools)
add_subdirectory(external/DirectoryService)
add_subdirectory(ImageIO)
add_subdirectory(login)
add_subdirectory(DebugSymbols)
add_subdirectory(external/bsm)
# Requires a newer version of Security to build
#add_subdirectory(external/Heimdal)

# Subprojects after this line need C++ headers

include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/external/libcxxabi/include
)

# libstdc++ doesn't use the headers included below
add_subdirectory(libstdcxx)

include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx/include
)

add_subdirectory(external/libcxxabi)
add_subdirectory(external/libcxx)
# Requires a newer Security version
#add_subdirectory(external/SmartCardServices)
# Has build issues
add_subdirectory(external/security)
add_subdirectory(external/cctools)

add_subdirectory(lkm)
