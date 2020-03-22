project(darling-src)

unset(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
unset(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)

cmake_minimum_required(VERSION 2.4.0)
cmake_policy(SET CMP0005 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(mig)
include(pyc)

#add_subdirectory(external/xcbuild)
add_subdirectory(libelfloader/native)
add_subdirectory(bsdln)

add_definitions(
	-D_DARWIN_C_SOURCE
	-D_POSIX_C_SOURCE
	-DDARLING
	-D__APPLE__
	-D__DYNAMIC__
)

set(DARLING TRUE)

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

configure_file(include/darling-config.h.in include/darling-config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

include(compiler_include)
GetCompilerInclude(COMPILER_INC_PATH)
message(STATUS "Compiler include path detected as ${COMPILER_INC_PATH}")
include_directories(SYSTEM ${COMPILER_INC_PATH})

find_package(BISON)
find_package(FLEX)

add_subdirectory(external/cctools-port/cctools/ld64/src)
add_subdirectory(external/cctools-port/cctools/ar)
#add_subdirectory(external/cctools-port/cctools/as)
add_subdirectory(external/cctools-port/cctools/misc)

#add_subdirectory(util)
#add_subdirectory(libmach-o)
#add_subdirectory(libdyld)
add_subdirectory(buildtools)
add_subdirectory(libelfloader/wrapgen)
add_subdirectory(startup)

add_definitions(
	-D_DARWIN_C_SOURCE
	-D_POSIX_C_SOURCE
	-D__APPLE__
	-D__DYNAMIC__
)

add_subdirectory(bootstrap_cmds)

set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ranlib")

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
	${CMAKE_CURRENT_SOURCE_DIR}/DiskArbitration/include
	${CMAKE_CURRENT_SOURCE_DIR}/CoreAudioOld/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/SmartCardServices/include
	${CMAKE_CURRENT_SOURCE_DIR}/login/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/IOKitUser/include
	${CMAKE_CURRENT_SOURCE_DIR}/launchd/liblaunch/include
	${CMAKE_CURRENT_SOURCE_DIR}/sandbox/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/coretls/include
	${CMAKE_CURRENT_SOURCE_DIR}/libmalloc/include
	${CMAKE_CURRENT_SOURCE_DIR}/libinfo/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/syslog/libsystem_asl.tproj/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/openssl/src/include
	${CMAKE_SOURCE_DIR}/kernel-include/iokit
	${CMAKE_CURRENT_SOURCE_DIR}/external/IOKitUser
	${CMAKE_CURRENT_SOURCE_DIR}/external/openpam/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite/include
	${CMAKE_CURRENT_SOURCE_DIR}/OpenScripting/include
	${CMAKE_CURRENT_SOURCE_DIR}/CryptoTokenKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/LocalAuthentication/include
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
	${CMAKE_CURRENT_SOURCE_DIR}/quarantine/include
	${CMAKE_CURRENT_SOURCE_DIR}/libpmenergy/include
	${CMAKE_CURRENT_SOURCE_DIR}/copyfile/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/include
	${CMAKE_CURRENT_SOURCE_DIR}/xar/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/DirectoryService/include
	${CMAKE_CURRENT_SOURCE_DIR}/ServiceManagement/include
	${CMAKE_CURRENT_SOURCE_DIR}/libresolv/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/AppKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/CoreText/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/QuartzCore/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/CoreGraphics/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/Onyx2D/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/CoreData/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/cocotron/CoreVideo/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/WTF/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/bmalloc/include
	${CMAKE_CURRENT_SOURCE_DIR}/SecurityFoundation/include
	${CMAKE_CURRENT_SOURCE_DIR}/libsysmon/include
	${CMAKE_CURRENT_SOURCE_DIR}/libcompression/include
	${CMAKE_CURRENT_SOURCE_DIR}/Metal/include
	${CMAKE_CURRENT_SOURCE_DIR}/kperf/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/dtrace/head
	${CMAKE_CURRENT_SOURCE_DIR}/ScriptingBridge/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/lkm/include
	${CMAKE_CURRENT_SOURCE_DIR}/libDiagnosticMessagesClient/include
	${CMAKE_CURRENT_SOURCE_DIR}/SecurityInterface/include
	${CMAKE_CURRENT_SOURCE_DIR}/DrawSprocket/include
	${CMAKE_CURRENT_SOURCE_DIR}/Quartz/include
	${CMAKE_CURRENT_SOURCE_DIR}/CoreVideo/include
	${CMAKE_CURRENT_SOURCE_DIR}/IOSurface/include
	${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/include
	${CMAKE_CURRENT_SOURCE_DIR}/libquit/include
	${CMAKE_CURRENT_SOURCE_DIR}/QuickLook/include
	${CMAKE_CURRENT_SOURCE_DIR}/WebKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/JavaVM/include
	${CMAKE_CURRENT_SOURCE_DIR}/DirectoryServices/include
	${CMAKE_CURRENT_SOURCE_DIR}/ImageIO/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/SecurityTokend/include
	${CMAKE_CURRENT_SOURCE_DIR}/LDAP/include
	${CMAKE_CURRENT_SOURCE_DIR}/ExceptionHandling/include
	${CMAKE_CURRENT_SOURCE_DIR}/EventKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/lzfse/include
	${CMAKE_CURRENT_SOURCE_DIR}/VideoToolbox/include
	${CMAKE_CURRENT_SOURCE_DIR}/QTKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/StoreKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/ForceFeedback/include
	${CMAKE_CURRENT_SOURCE_DIR}/GameController/include
	${CMAKE_CURRENT_SOURCE_DIR}/CoreWLAN/include
	${CMAKE_CURRENT_SOURCE_DIR}/IOBluetooth/include
	${CMAKE_CURRENT_SOURCE_DIR}/libsandbox/include
	${CMAKE_CURRENT_SOURCE_DIR}/OpenCL/include
	${CMAKE_CURRENT_SOURCE_DIR}/OpenDirectory/include
	${CMAKE_CURRENT_SOURCE_DIR}/MetalKit/include
	${CMAKE_CURRENT_SOURCE_DIR}/frameworks/include
	${CMAKE_CURRENT_SOURCE_DIR}/private-frameworks/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib/include
)

add_subdirectory(external/libkqueue)

add_subdirectory(external/libplatform)
add_subdirectory(external/libpthread)
add_subdirectory(external/libtrace)
add_subdirectory(kernel)
add_subdirectory(libc)
add_subdirectory(vchroot)

add_subdirectory(lib)

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
add_subdirectory(libelfloader)
add_subdirectory(duct/src)
add_subdirectory(libresolv)
add_subdirectory(external/corecrypto)
add_subdirectory(external/coretls)
add_subdirectory(xtrace)
add_subdirectory(libgmalloc)
add_subdirectory(dyld)
add_subdirectory(external/objc4/runtime)
add_subdirectory(external/syslog)
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
add_subdirectory(external/cfnetwork/src)
add_subdirectory(external/pcre)
add_subdirectory(external/sqlite)
add_subdirectory(external/openpam)
add_subdirectory(external/libtelnet)
add_subdirectory(external/remote_cmds)
add_subdirectory(external/IOKitUser)
add_subdirectory(external/python_modules)
add_subdirectory(SystemConfiguration)
#add_subdirectory(VideoDecodeAcceleration)
add_subdirectory(DiskArbitration)
add_subdirectory(LocalAuthentication)
add_subdirectory(CryptoTokenKit)
add_subdirectory(ServiceManagement)
add_subdirectory(xar)
add_subdirectory(libedit)
add_subdirectory(libpmenergy)
add_subdirectory(external/libclosure)
add_subdirectory(external/compiler-rt/lib/builtins)
add_subdirectory(csu)
add_subdirectory(external/python/2.6/Python-2.6.9)
add_subdirectory(external/python/2.7/Python-2.7.10)
add_subdirectory(external/ruby)
add_subdirectory(external/expat)
add_subdirectory(shellspawn)
add_subdirectory(external/libarchive/libarchive)
add_subdirectory(external/apr)
add_subdirectory(sandbox)
add_subdirectory(xcselect)
add_subdirectory(external/OpenDirectory)
add_subdirectory(external/iokitd)

add_subdirectory(native)

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
add_subdirectory(external/files)
add_subdirectory(external/crontabs)
add_subdirectory(external/zip/zip/zip30)
add_subdirectory(external/zip/unzip/unzip-5.52)
add_subdirectory(tools)
add_subdirectory(external/installer)
add_subdirectory(external/bind9)
add_subdirectory(external/gnutar/gnutar)
add_subdirectory(external/gpatch)
add_subdirectory(external/gnudiff)
add_subdirectory(external/openssh)
add_subdirectory(external/top)
add_subdirectory(external/perl)
add_subdirectory(external/mail_cmds)
add_subdirectory(external/screen)
add_subdirectory(unxip)
add_subdirectory(external/zsh)
add_subdirectory(external/rsync)
add_subdirectory(external/doc_cmds)
add_subdirectory(external/basic_cmds)
add_subdirectory(external/misc_cmds)
add_subdirectory(external/patch_cmds)
add_subdirectory(external/DSTools)
add_subdirectory(external/DirectoryService)
add_subdirectory(ImageIO)
add_subdirectory(login)
add_subdirectory(libaks)
add_subdirectory(SecurityFoundation)
add_subdirectory(libsysmon)
add_subdirectory(libcompression)
add_subdirectory(Metal)
add_subdirectory(kperf)
add_subdirectory(external/bsm)
add_subdirectory(external/Heimdal)
add_subdirectory(PlistBuddy)
add_subdirectory(ScriptingBridge)
add_subdirectory(libDiagnosticMessagesClient)
add_subdirectory(SecurityInterface)
add_subdirectory(Quartz)
add_subdirectory(CoreVideo)
add_subdirectory(IOSurface)
add_subdirectory(libquit)
add_subdirectory(QuickLook)
add_subdirectory(LDAP)
add_subdirectory(ExceptionHandling)
add_subdirectory(external/lzfse)
add_subdirectory(VideoToolbox)
add_subdirectory(QTKit)
add_subdirectory(StoreKit)
add_subdirectory(ForceFeedback)
add_subdirectory(GameController)
add_subdirectory(CoreWLAN)
add_subdirectory(IOBluetooth)
add_subdirectory(libsandbox)
add_subdirectory(OpenCL)
add_subdirectory(OpenDirectory)
add_subdirectory(opendirectory)
add_subdirectory(MetalKit)
add_subdirectory(external/cups)

add_subdirectory(clt)

# Just a stub
add_subdirectory(WebKit)
add_subdirectory(OpenGL)
add_subdirectory(JavaVM)
add_subdirectory(DirectoryServices)
add_subdirectory(DrawSprocket)
add_subdirectory(EventKit)
add_subdirectory(external/cocotron/CoreGraphics)

add_subdirectory(frameworks)
add_subdirectory(private-frameworks)
add_subdirectory(external/usertemplate)

# /Applications
#add_subdirectory(external/TextEdit)

# Subprojects after this line need C++ headers

include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/external/libcxxabi/include
)

# libstdc++ doesn't use the headers included below
add_subdirectory(libstdcxx)

include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx/include
	${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include
)

add_subdirectory(external/libcxxabi)
add_subdirectory(external/libcxx)

add_subdirectory(external/libdispatch)

# Requires a newer Security version
add_subdirectory(external/SmartCardServices)
add_subdirectory(external/security)
add_subdirectory(external/SecurityTokend)
add_subdirectory(external/cctools)
add_subdirectory(external/libauto)
add_subdirectory(external/WTF)
add_subdirectory(external/bmalloc)
add_subdirectory(external/dtrace)

add_subdirectory(external/lkm)

add_subdirectory(external/cocotron/Onyx2D)
add_subdirectory(external/cocotron/AppKit)
add_subdirectory(external/cocotron/CoreText)
add_subdirectory(external/cocotron/Cocoa)
add_subdirectory(external/cocotron/CoreData)
add_subdirectory(external/cocotron/QuartzCore)
add_subdirectory(pboard)

if(FULL_BUILD)
add_subdirectory(external/JavaScriptCore)
endif(FULL_BUILD)

#add_subdirectory(external/WebCore)
