project(darling-src)

unset(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
unset(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)

cmake_policy(SET CMP0005 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(mig)
include(pyc)

#
# start core components
#
# projects included here are strictly necessary for basic executables to function at all.
# it should be very clear which projects belong here. a project should only be here if it
# is one of the following or is necessary for one of the following:
#   * darlingserver
#   * dyld
#   * mldr
#   * libSystem
#   * the host-side `darling` executable
# this includes any build tools that are necessary to build any of those projects.
#
# these projects are always built
#

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "core")

#add_subdirectory(external/xcbuild)
add_subdirectory(bsdln)

add_definitions(
	-DDARLING
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

set(DARLING_SDK_RELATIVE_PATH "Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
set(DARLING_ROOT_RELATIVE_TO_SDK "../../../../../..")

add_subdirectory(external/cctools-port/cctools/ld64/src)
add_subdirectory(external/cctools-port/cctools/ar)
#add_subdirectory(external/cctools-port/cctools/as)
add_subdirectory(external/cctools-port/cctools/misc)

#add_subdirectory(util)
#add_subdirectory(libmach-o)
#add_subdirectory(libdyld)
add_subdirectory(buildtools)
add_subdirectory(hosttools)
add_subdirectory(libelfloader/wrapgen)
# add the Linux build of libsimple now
add_subdirectory(libsimple "${CMAKE_CURRENT_BINARY_DIR}/libsimple-linux")
add_subdirectory(external/darlingserver)
add_subdirectory(startup)

include_directories(${CMAKE_SOURCE_DIR}/basic-headers)

add_subdirectory(external/bootstrap_cmds)

add_definitions(
	-D__APPLE__
	-D__DYNAMIC__
	-D__MACH__
	-D_DARWIN_C_SOURCE
	-D_POSIX_C_SOURCE
	-DDARWIN
	-DTARGET_OS_MAC=1
	-DPLATFORM_MacOSX
)

set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/${APPLE_TARGET_TRIPLET_PRIMARY}-ar")
set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/${APPLE_TARGET_TRIPLET_PRIMARY}-ranlib")
#set(CMAKE_ASM_COMPILER "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/as/${APPLE_TARGET_TRIPLET_PRIMARY}-as")

# set macOS deployment version
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")

add_definitions(-target ${APPLE_TARGET_TRIPLET_PRIMARY})
include(darling_lib)
include(darling_static_lib)
include(darling_bundle)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include)

include_directories(
	${CMAKE_SOURCE_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include
	${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include
	${CMAKE_SOURCE_DIR}/framework-include
	${CMAKE_SOURCE_DIR}/framework-private-include
)

include_directories(AFTER
	${CMAKE_CURRENT_SOURCE_DIR}/external/lkm/include
	${CMAKE_CURRENT_SOURCE_DIR}/libDiagnosticMessagesClient/include
	${CMAKE_CURRENT_SOURCE_DIR}/libMobileGestalt/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib/include

	# necessary because this is the expected behavior when using `xml2-config --cflags`
	# it adds `$sdk_dir/libxml2` to the include path
	${CMAKE_SOURCE_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/libxml2
	${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/libxml2
)

# add the Mach-O build of libsimple now
add_subdirectory(libsimple "${CMAKE_CURRENT_BINARY_DIR}/libsimple-darling")

add_subdirectory(external/libkqueue)

# needs to come before libplatform because it generates mig headers that libplatform needs
add_subdirectory(external/xnu/darling/src/libsystem_kernel)

add_subdirectory(external/libplatform)
add_subdirectory(external/libpthread)
add_subdirectory(external/libtrace)
add_subdirectory(external/libc)
add_subdirectory(vchroot)

add_subdirectory(lib)

# libstdc++ doesn't use the libc++ headers included below
add_subdirectory(external/libstdcxx)

include_directories(BEFORE
	${CMAKE_SOURCE_DIR}/src/external/libcxx/include
)

add_definitions(-D_LIBC_NO_FEATURE_VERIFICATION)

add_subdirectory(launchd)
add_subdirectory(libm)
add_subdirectory(external/dyld)
add_subdirectory(external/libxpc)
add_subdirectory(libgcc)
add_subdirectory(external/copyfile)
add_subdirectory(external/Libinfo)
add_subdirectory(external/compiler-rt/lib/builtins)
add_subdirectory(external/libnotify)
add_subdirectory(duct/src)
add_subdirectory(external/syslog)
add_subdirectory(external/libclosure)
add_subdirectory(external/keymgr)
add_subdirectory(external/libunwind)
add_subdirectory(external/libmalloc)
add_subdirectory(external/coretls)
add_subdirectory(libsystem_coreservices)
add_subdirectory(external/removefile)
add_subdirectory(quarantine)
add_subdirectory(sandbox)
add_subdirectory(external/corecrypto)
add_subdirectory(external/commoncrypto)
add_subdirectory(networkextension)
add_subdirectory(libcache)
add_subdirectory(external/configd)
add_subdirectory(external/libsystem)
add_subdirectory(external/objc4/runtime)
add_subdirectory(external/corefoundation)
add_subdirectory(external/icu/icuSources)
add_subdirectory(external/csu)
add_subdirectory(external/libresolv)
add_subdirectory(external/usertemplate)
add_subdirectory(external/bsm)
add_subdirectory(external/zlib)
# shellspawn can actually spawn any executable we want within the prefix,
# so we include it in core to allow executables to be executed with the `darling` command.
add_subdirectory(shellspawn)

# typically considered CLI components, but required by launchctl (which is necessary for launchd to bootstrap)
add_subdirectory(external/libedit)
add_subdirectory(external/ncurses)

#
# end core components
#
# the rest of the projects are not all neatly separated like the core components are because
# some projects build multiple targets which may be part of different components
#

if (COMPONENT_system)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "system")

	add_subdirectory(external/libiconv)
	add_subdirectory(external/bzip2)
	add_subdirectory(external/libressl-2.8.3)
	add_subdirectory(external/libarchive/libarchive)
	add_subdirectory(external/liblzma)
	add_subdirectory(external/OpenDirectory)
	add_subdirectory(external/openssl_certificates)
	add_subdirectory(external/bash)
	add_subdirectory(external/zsh)
	add_subdirectory(external/tcsh)
	add_subdirectory(external/files)
	add_subdirectory(external/crontabs)
	add_subdirectory(external/swift)
endif()

if (COMPONENT_cli)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli")

	add_subdirectory(external/darling-dmg)
	add_subdirectory(external/libutil)
	add_subdirectory(xtrace)
	add_subdirectory(external/libxslt)
	add_subdirectory(external/openssl/src)
	add_subdirectory(external/nghttp2)
	add_subdirectory(external/passwordserver_sasl)
	add_subdirectory(external/curl)
	add_subdirectory(external/OpenLDAP)
	add_subdirectory(external/pcre)
	add_subdirectory(external/libtelnet)
	add_subdirectory(external/remote_cmds)
	add_subdirectory(external/IOKitTools)
	add_subdirectory(external/apr)
	add_subdirectory(xcselect)
	add_subdirectory(external/file/file)
	add_subdirectory(external/shell_cmds)
	add_subdirectory(external/file_cmds)
	add_subdirectory(external/text_cmds)
	add_subdirectory(external/adv_cmds)
	add_subdirectory(external/network_cmds)
	add_subdirectory(external/system_cmds)
	add_subdirectory(external/mail_cmds)
	add_subdirectory(external/doc_cmds)
	add_subdirectory(external/basic_cmds)
	add_subdirectory(external/misc_cmds)
	add_subdirectory(external/patch_cmds)
	add_subdirectory(external/less)
	add_subdirectory(external/grep)
	add_subdirectory(external/awk)
	add_subdirectory(external/groff)
	add_subdirectory(external/nano)
	add_subdirectory(external/man)
	add_subdirectory(external/bc)
	add_subdirectory(external/zip)
	add_subdirectory(external/installer)
	add_subdirectory(external/bind9)
	add_subdirectory(external/netcat)
	add_subdirectory(external/gpatch)
	add_subdirectory(external/gnudiff)
	add_subdirectory(external/openssh)
	add_subdirectory(external/top)
	add_subdirectory(external/screen)
	add_subdirectory(unxip)
	add_subdirectory(external/rsync)
	add_subdirectory(external/DSTools)
	add_subdirectory(libsysmon)
	add_subdirectory(PlistBuddy)
	add_subdirectory(libquit)
	add_subdirectory(external/lzfse)
	add_subdirectory(clt)
	add_subdirectory(diskutil)

	# these aren't used by anything we build (they're just included because they're also present in macOS)
	# TODO: maybe we should introduce another component (e.g. `lib_stock`) for libraries we don't need but are expected in a stock macOS installation.
	add_subdirectory(external/libressl-2.2.9)
	add_subdirectory(external/libressl-2.5.5)
	add_subdirectory(external/libressl-2.6.5)
endif()

if (COMPONENT_python OR COMPONENT_ruby)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "ffi")
	add_subdirectory(external/libffi)
endif()

if (COMPONENT_cli_dev)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev")
	add_subdirectory(libgmalloc)
endif()

if (COMPONENT_cli OR COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_gui_common")

	add_subdirectory(external/libxml2)
	add_subdirectory(external/foundation)
	add_subdirectory(external/cfnetwork/src)
	add_subdirectory(external/sqlite)
	add_subdirectory(external/openpam)
	add_subdirectory(external/energytrace)
	add_subdirectory(external/xnu/libkern/kxld)
	add_subdirectory(external/IOKitUser)
	add_subdirectory(external/xar)
	add_subdirectory(libpmenergy)
	add_subdirectory(external/DirectoryService)
	add_subdirectory(libacm)
	add_subdirectory(libaks)
	add_subdirectory(libcompression)
	add_subdirectory(external/Heimdal)
	add_subdirectory(libDiagnosticMessagesClient)
	add_subdirectory(libsandbox)
	add_subdirectory(opendirectory_internal)
	add_subdirectory(external/fmdb)
	add_subdirectory(external/libnetwork)
	add_subdirectory(libMobileGestalt)
	add_subdirectory(external/MITKerberosShim)
	add_subdirectory(MobileKeyBag)
endif()

if (COMPONENT_iokitd)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME iokitd)
	add_subdirectory(external/iokitd)
endif()

if (COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev_gui_common")

	add_subdirectory(libaccessibility)
	add_subdirectory(external/openjdk) # *should* be in `cli` component, but requires AppKit
endif()

if (COMPONENT_cli_extra)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_extra")

	add_subdirectory(external/gnutar/gnutar)
endif()

if (COMPONENT_gui)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "gui")

	add_subdirectory(external/cocotron)

	add_subdirectory(CoreAudio)
	add_subdirectory(external/cups)
	add_subdirectory(pboard)

	# not currently used by anything we build
	add_subdirectory(external/dbuskit)
endif()

if (COMPONENT_python)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "python")

	add_subdirectory(external/python_modules)
	add_subdirectory(external/python/2.7/Python-2.7.16)
endif()

if (COMPONENT_cli OR COMPONENT_python)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_python_common")

	add_subdirectory(external/BerkeleyDB)
	add_subdirectory(external/expat)
endif()

if (COMPONENT_gui_frameworks AND COMPONENT_gui_stubs AND COMPONENT_python)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "pyobjc")

	# TODO: pyobjc should only build individual modules if the respective components are enabled.
	#       right now, it's all-or-nothing.
	add_subdirectory(external/pyobjc)
endif()

if (COMPONENT_ruby)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "ruby")

	add_subdirectory(external/ruby)
endif()

if (COMPONENT_perl)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "perl")

	add_subdirectory(external/perl)
endif()

#add_subdirectory(external/mDNSResponder)
#add_subdirectory(external/OpenAL)
#add_subdirectory(VideoDecodeAcceleration)
#add_subdirectory(external/glut)

# not a core component, but we always add it regardless of the components that are enabled;
# it has its own component checks to determine which libraries to generate wrappers for.
add_subdirectory(native)

# has its own component checks
add_subdirectory(external/vim)

# has its own component checks
add_subdirectory(tools)

# these have their own component checks on a per-framework basis
add_subdirectory(frameworks)
add_subdirectory(private-frameworks)

# /Applications
#add_subdirectory(external/TextEdit)

# Subprojects after this line need C++ headers

include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/external/libcxxabi/include
)

#
# start core components with C++
#

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "core")

add_subdirectory(external/libcxxabi)
add_subdirectory(external/libcxx)

add_subdirectory(external/libdispatch)
add_subdirectory(external/cctools)

#
# end core components with C++
#

if (COMPONENT_cli OR COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_gui_common")

	add_subdirectory(external/SmartCardServices)
	add_subdirectory(external/security)
	add_subdirectory(external/SecurityTokend)
endif()

if (COMPONENT_cli)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli")

	add_subdirectory(external/dtrace)
	add_subdirectory(external/libauto)
endif()

if (COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev_gui_common")

	add_subdirectory(external/metal)
endif()

if (COMPONENT_jsc OR COMPONENT_webkit)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "jsc_webkit_common")

	add_subdirectory(external/WTF)
	add_subdirectory(external/bmalloc)
endif()

if (COMPONENT_jsc)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "jsc")

	add_subdirectory(external/JavaScriptCore)
endif()

#add_subdirectory(external/WebCore)
