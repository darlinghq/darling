project(python313)

cmake_minimum_required(VERSION 3.13)

include(CMakeParseArguments)
include(darling_exe)
include(darling_framework)
include(InstallSymlink)

# PYTHON3_VERSION is defined in the top-level CMakeLists.txt so shared helpers
# can configure /usr/bin/python before the Python 2 framework is added.
set(PYTHON3_FULL_VERSION "3.13.18")
set(PYTHON3_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpython")
set(PYTHON3_FRAMEWORK_REL "System/Library/Frameworks/Python.framework")
set(PYTHON3_FRAMEWORK_DIR "/${PYTHON3_FRAMEWORK_REL}/Versions/${PYTHON3_VERSION}")
set(PYTHON3_INSTALL_DIR "libexec/darling${PYTHON3_FRAMEWORK_DIR}")
set(PYTHON3_STDLIB_DIR "${PYTHON3_INSTALL_DIR}/lib/python${PYTHON3_VERSION}")
set(PYTHON3_DYNLOAD_DIR "${PYTHON3_STDLIB_DIR}/lib-dynload")

# Frozen bytecode is interpreted by the host Python at build time. It has to
# match the guest interpreter's bytecode version exactly, so a plain python3
# from the host is only usable when it happens to be 3.13.
find_program(PYTHON3_HOST_EXECUTABLE NAMES python3.13 python3)
if (NOT PYTHON3_HOST_EXECUTABLE)
	message(FATAL_ERROR "A host Python 3.13 interpreter is required to build the Python 3 framework")
endif ()
execute_process(
	COMMAND "${PYTHON3_HOST_EXECUTABLE}" -c "import sys; print('%d.%d' % sys.version_info[:2])"
	OUTPUT_VARIABLE PYTHON3_HOST_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE PYTHON3_HOST_VERSION_RESULT
)
if (NOT PYTHON3_HOST_VERSION_RESULT EQUAL 0 OR NOT PYTHON3_HOST_VERSION STREQUAL "3.13")
	message(FATAL_ERROR "The Python 3 framework requires a host Python 3.13 interpreter, found '${PYTHON3_HOST_VERSION}' at ${PYTHON3_HOST_EXECUTABLE}")
endif ()

find_program(PYTHON3_PERL_EXECUTABLE NAMES perl)
if (NOT PYTHON3_PERL_EXECUTABLE)
	message(FATAL_ERROR "Perl is required to generate CPython's builtin module table")
endif ()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python/frozen_modules")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Modules")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pyconfig.h"
	COMMAND "${PYTHON3_HOST_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_pyconfig.py"
		"${PYTHON3_SOURCE_DIR}/pyconfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/pyconfig.h"
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_pyconfig.py"
		"${PYTHON3_SOURCE_DIR}/pyconfig.h.in"
	COMMENT "Generating pyconfig.h for the Darwin CPython 3.13 build"
	VERBATIM
)

# CPython's makesetup script is the source of truth for the builtin module
# table. Only the bootstrap module list is static; the standard library
# extensions are built separately as Mach-O bundles below.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Modules/Setup.local"
	"# No local modules are added by the Darling build.\n")
file(READ "${PYTHON3_SOURCE_DIR}/Modules/Setup.bootstrap.in" PYTHON3_SETUP_BOOTSTRAP)
string(REPLACE "@MODULE_PWD_TRUE@" "" PYTHON3_SETUP_BOOTSTRAP "${PYTHON3_SETUP_BOOTSTRAP}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Modules/Setup.bootstrap" "${PYTHON3_SETUP_BOOTSTRAP}")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config.c"
	COMMAND "${PYTHON3_PERL_EXECUTABLE}" "${PYTHON3_SOURCE_DIR}/Modules/makesetup"
		-m - -c "${PYTHON3_SOURCE_DIR}/Modules/config.c.in" -s Modules
		Modules/Setup.local Modules/Setup.bootstrap "${PYTHON3_SOURCE_DIR}/Modules/Setup"
	DEPENDS
		"${PYTHON3_SOURCE_DIR}/Modules/makesetup"
		"${PYTHON3_SOURCE_DIR}/Modules/config.c.in"
		"${PYTHON3_SOURCE_DIR}/Modules/Setup"
		"${CMAKE_CURRENT_BINARY_DIR}/Modules/Setup.local"
		"${CMAKE_CURRENT_BINARY_DIR}/Modules/Setup.bootstrap"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	COMMENT "Generating CPython 3.13 builtin module table"
	VERBATIM
)

set(PYTHON3_FROZEN_HEADERS "")

function(py3_freeze name source)
	set(output "${CMAKE_CURRENT_BINARY_DIR}/Python/frozen_modules/${name}.h")
	add_custom_command(OUTPUT "${output}"
		COMMAND "${PYTHON3_HOST_EXECUTABLE}" "${PYTHON3_SOURCE_DIR}/Programs/_freeze_module.py"
			"${name}" "${source}" "${output}"
		DEPENDS
			"${PYTHON3_SOURCE_DIR}/Programs/_freeze_module.py"
			"${source}"
		COMMENT "Freezing ${name} for CPython 3.13"
		VERBATIM
	)
	set(PYTHON3_FROZEN_HEADERS "${PYTHON3_FROZEN_HEADERS}" "${output}" PARENT_SCOPE)
endfunction()

py3_freeze(getpath "${PYTHON3_SOURCE_DIR}/Modules/getpath.py")
py3_freeze(importlib._bootstrap "${PYTHON3_SOURCE_DIR}/Lib/importlib/_bootstrap.py")
py3_freeze(importlib._bootstrap_external "${PYTHON3_SOURCE_DIR}/Lib/importlib/_bootstrap_external.py")
py3_freeze(zipimport "${PYTHON3_SOURCE_DIR}/Lib/zipimport.py")
py3_freeze(abc "${PYTHON3_SOURCE_DIR}/Lib/abc.py")
py3_freeze(codecs "${PYTHON3_SOURCE_DIR}/Lib/codecs.py")
py3_freeze(io "${PYTHON3_SOURCE_DIR}/Lib/io.py")
py3_freeze(_collections_abc "${PYTHON3_SOURCE_DIR}/Lib/_collections_abc.py")
py3_freeze(_sitebuiltins "${PYTHON3_SOURCE_DIR}/Lib/_sitebuiltins.py")
py3_freeze(genericpath "${PYTHON3_SOURCE_DIR}/Lib/genericpath.py")
py3_freeze(ntpath "${PYTHON3_SOURCE_DIR}/Lib/ntpath.py")
py3_freeze(posixpath "${PYTHON3_SOURCE_DIR}/Lib/posixpath.py")
py3_freeze(os "${PYTHON3_SOURCE_DIR}/Lib/os.py")
py3_freeze(site "${PYTHON3_SOURCE_DIR}/Lib/site.py")
py3_freeze(stat "${PYTHON3_SOURCE_DIR}/Lib/stat.py")
py3_freeze(importlib.util "${PYTHON3_SOURCE_DIR}/Lib/importlib/util.py")
py3_freeze(importlib.machinery "${PYTHON3_SOURCE_DIR}/Lib/importlib/machinery.py")
py3_freeze(runpy "${PYTHON3_SOURCE_DIR}/Lib/runpy.py")
py3_freeze(__hello__ "${PYTHON3_SOURCE_DIR}/Lib/__hello__.py")
py3_freeze(__phello__ "${PYTHON3_SOURCE_DIR}/Lib/__phello__/__init__.py")
py3_freeze(__phello__.ham "${PYTHON3_SOURCE_DIR}/Lib/__phello__/ham/__init__.py")
py3_freeze(__phello__.ham.eggs "${PYTHON3_SOURCE_DIR}/Lib/__phello__/ham/eggs.py")
py3_freeze(__phello__.spam "${PYTHON3_SOURCE_DIR}/Lib/__phello__/spam.py")
py3_freeze(frozen_only "${PYTHON3_SOURCE_DIR}/Tools/freeze/flag.py")

add_custom_target(py3_generated DEPENDS
	"${CMAKE_CURRENT_BINARY_DIR}/pyconfig.h"
	"${CMAKE_CURRENT_BINARY_DIR}/config.c"
	${PYTHON3_FROZEN_HEADERS}
)

function(py3_target_setup target)
	target_include_directories(${target} BEFORE PRIVATE
		"${CMAKE_CURRENT_BINARY_DIR}"
		"${CMAKE_CURRENT_BINARY_DIR}/Python"
		"${PYTHON3_SOURCE_DIR}/Include"
		"${PYTHON3_SOURCE_DIR}/Include/internal"
		"${PYTHON3_SOURCE_DIR}"
	)
	target_compile_definitions(${target} PRIVATE
		HAVE_CONFIG_H
		NDEBUG
		MACOSX
		PLATFORM="darwin"
		SOABI=""
	)

	# CPython only passes these to Modules/getpath.c. Defining PREFIX for
	# every translation unit collides with identifiers in bundled libraries
	# such as Expat, which has a struct named PREFIX.
	set_source_files_properties("${PYTHON3_SOURCE_DIR}/Modules/getpath.c" PROPERTIES
		COMPILE_DEFINITIONS
			"VERSION=\"${PYTHON3_VERSION}\";PREFIX=\"${PYTHON3_FRAMEWORK_DIR}\";EXEC_PREFIX=\"${PYTHON3_FRAMEWORK_DIR}\";VPATH=\".\";PLATLIBDIR=\"lib\";PYTHONPATH=\":plat-darwin:plat-mac:lib-dynload\";PYTHONFRAMEWORK=\"Python\""
	)
	target_compile_options(${target} PRIVATE
		-fno-common
		-fno-strict-aliasing
		-fwrapv
		-w
	)
endfunction()

# This list mirrors the LIBRARY_OBJS variable generated by CPython's own
# configure for 3.13. Python/asm_trampoline.S is Linux-only and config.c is
# generated above; everything else is compiled straight from the submodule.
set(PYTHON3_CORE_RELATIVE_SOURCES
	Modules/getbuildinfo.c
	Parser/token.c
	Parser/pegen.c
	Parser/pegen_errors.c
	Parser/action_helpers.c
	Parser/parser.c
	Parser/string_parser.c
	Parser/peg_api.c
	Parser/lexer/buffer.c
	Parser/lexer/lexer.c
	Parser/lexer/state.c
	Parser/tokenizer/file_tokenizer.c
	Parser/tokenizer/readline_tokenizer.c
	Parser/tokenizer/string_tokenizer.c
	Parser/tokenizer/utf8_tokenizer.c
	Parser/tokenizer/helpers.c
	Parser/myreadline.c
	Objects/abstract.c
	Objects/boolobject.c
	Objects/bytes_methods.c
	Objects/bytearrayobject.c
	Objects/bytesobject.c
	Objects/call.c
	Objects/capsule.c
	Objects/cellobject.c
	Objects/classobject.c
	Objects/codeobject.c
	Objects/complexobject.c
	Objects/descrobject.c
	Objects/enumobject.c
	Objects/exceptions.c
	Objects/genericaliasobject.c
	Objects/genobject.c
	Objects/fileobject.c
	Objects/floatobject.c
	Objects/frameobject.c
	Objects/funcobject.c
	Objects/iterobject.c
	Objects/listobject.c
	Objects/longobject.c
	Objects/dictobject.c
	Objects/odictobject.c
	Objects/memoryobject.c
	Objects/methodobject.c
	Objects/moduleobject.c
	Objects/namespaceobject.c
	Objects/object.c
	Objects/obmalloc.c
	Objects/picklebufobject.c
	Objects/rangeobject.c
	Objects/setobject.c
	Objects/sliceobject.c
	Objects/structseq.c
	Objects/tupleobject.c
	Objects/typeobject.c
	Objects/typevarobject.c
	Objects/unicodeobject.c
	Objects/unicodectype.c
	Objects/unionobject.c
	Objects/weakrefobject.c
	Python/_warnings.c
	Python/Python-ast.c
	Python/Python-tokenize.c
	Python/asdl.c
	Python/assemble.c
	Python/ast.c
	Python/ast_opt.c
	Python/ast_unparse.c
	Python/bltinmodule.c
	Python/brc.c
	Python/ceval.c
	Python/codecs.c
	Python/compile.c
	Python/context.c
	Python/critical_section.c
	Python/crossinterp.c
	Python/dynamic_annotations.c
	Python/errors.c
	Python/flowgraph.c
	Python/frame.c
	Python/frozenmain.c
	Python/future.c
	Python/gc.c
	Python/gc_free_threading.c
	Python/gc_gil.c
	Python/getargs.c
	Python/getcompiler.c
	Python/getcopyright.c
	Python/getplatform.c
	Python/getversion.c
	Python/ceval_gil.c
	Python/hamt.c
	Python/hashtable.c
	Python/import.c
	Python/importdl.c
	Python/initconfig.c
	Python/interpconfig.c
	Python/instrumentation.c
	Python/instruction_sequence.c
	Python/intrinsics.c
	Python/jit.c
	Python/legacy_tracing.c
	Python/lock.c
	Python/marshal.c
	Python/modsupport.c
	Python/mysnprintf.c
	Python/mystrtoul.c
	Python/object_stack.c
	Python/optimizer.c
	Python/optimizer_analysis.c
	Python/optimizer_symbols.c
	Python/parking_lot.c
	Python/pathconfig.c
	Python/preconfig.c
	Python/pyarena.c
	Python/pyctype.c
	Python/pyfpe.c
	Python/pyhash.c
	Python/pylifecycle.c
	Python/pymath.c
	Python/pystate.c
	Python/pythonrun.c
	Python/pytime.c
	Python/qsbr.c
	Python/bootstrap_hash.c
	Python/specialize.c
	Python/structmember.c
	Python/symtable.c
	Python/sysmodule.c
	Python/thread.c
	Python/traceback.c
	Python/tracemalloc.c
	Python/getopt.c
	Python/pystrcmp.c
	Python/pystrtod.c
	Python/pystrhex.c
	Python/dtoa.c
	Python/formatter_unicode.c
	Python/fileutils.c
	Python/suggestions.c
	Python/perf_trampoline.c
	Python/perf_jit_trampoline.c
	Python/dynload_shlib.c
	Modules/main.c
	Modules/gcmodule.c
	Modules/atexitmodule.c
	Modules/faulthandler.c
	Modules/posixmodule.c
	Modules/signalmodule.c
	Modules/_tracemalloc.c
	Modules/_suggestions.c
	Modules/_codecsmodule.c
	Modules/_collectionsmodule.c
	Modules/errnomodule.c
	Modules/_io/_iomodule.c
	Modules/_io/iobase.c
	Modules/_io/fileio.c
	Modules/_io/bytesio.c
	Modules/_io/bufferedio.c
	Modules/_io/textio.c
	Modules/_io/stringio.c
	Modules/itertoolsmodule.c
	Modules/_sre/sre.c
	Modules/_sysconfig.c
	Modules/_threadmodule.c
	Modules/timemodule.c
	Modules/_typingmodule.c
	Modules/_weakref.c
	Modules/_abc.c
	Modules/_functoolsmodule.c
	Modules/_localemodule.c
	Modules/_operator.c
	Modules/_stat.c
	Modules/symtablemodule.c
	Modules/pwdmodule.c
	Modules/getpath.c
	Python/frozen.c
)

# This CMakeLists is a wrapper around the CPython submodule, so source paths
# relative to the submodule root have to be made absolute before they are
# passed to CMake's target commands.
set(PYTHON3_CORE_SOURCES "")
foreach (src IN LISTS PYTHON3_CORE_RELATIVE_SOURCES)
	list(APPEND PYTHON3_CORE_SOURCES "${PYTHON3_SOURCE_DIR}/${src}")
endforeach ()
list(APPEND PYTHON3_CORE_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/config.c")

set(DYLIB_COMPAT_VERSION "3.13.0")
set(DYLIB_CURRENT_VERSION "${PYTHON3_FULL_VERSION}")
add_framework(Python
	FAT
	CURRENT_VERSION
	VERSION "${PYTHON3_VERSION}"
	TARGET_NAME "Python_${PYTHON3_VERSION}"
	SOURCES
		${PYTHON3_CORE_SOURCES}
	DEPENDENCIES
		system
	RESOURCES
		Info.plist Info.plist
)
unset(DYLIB_COMPAT_VERSION)
unset(DYLIB_CURRENT_VERSION)

py3_target_setup(Python_${PYTHON3_VERSION})
target_compile_definitions(Python_${PYTHON3_VERSION} PRIVATE Py_BUILD_CORE)
add_dependencies(Python_${PYTHON3_VERSION} py3_generated)

add_darling_object_library(python313exe_obj "${PYTHON3_SOURCE_DIR}/Programs/python.c")
py3_target_setup(python313exe_obj)
add_dependencies(python313exe_obj py3_generated)

add_darling_executable(python313exe $<TARGET_OBJECTS:python313exe_obj>)
target_link_libraries(python313exe system Python_${PYTHON3_VERSION})
set_target_properties(python313exe PROPERTIES OUTPUT_NAME "python${PYTHON3_VERSION}")
make_fat(python313exe)

install(TARGETS python313exe DESTINATION "${PYTHON3_INSTALL_DIR}/bin")
# /usr/bin/python is the versioner wrapper installed by the Python 2.7
# framework; its default is now PYTHON3_VERSION. Python 2.7 stays available as
# python2, while python3 and python3.13 point directly at this interpreter.
InstallSymlink("../../System/Library/Frameworks/Python.framework/Versions/${PYTHON3_VERSION}/bin/python${PYTHON3_VERSION}"
	"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/python${PYTHON3_VERSION}")
InstallSymlink("python${PYTHON3_VERSION}"
	"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/python3")
InstallSymlink("../../System/Library/Frameworks/Python.framework/Versions/2.7/bin/python2.7"
	"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/python2")

set(EXENAME "${PYTHON3_FRAMEWORK_DIR}/bin/python${PYTHON3_VERSION}")
configure_file("${PYTHON3_SOURCE_DIR}/Misc/python-config.in"
	"${CMAKE_CURRENT_BINARY_DIR}/python-config" @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/python-config"
	DESTINATION libexec/darling/usr/bin RENAME "python${PYTHON3_VERSION}-config")
InstallSymlink("python${PYTHON3_VERSION}-config"
	"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/python3-config")
# The unversioned python-config belongs to the default interpreter too. The
# Python 2 framework also installs a placeholder here; this symlink is emitted
# afterwards and replaces it.
InstallSymlink("python${PYTHON3_VERSION}-config"
	"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/python-config")
InstallSymlink("python2.7-config"
	"${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/python2-config")

#
# OpenSSL 3 for the Python 3 ssl/hashlib modules.
#
include("${CMAKE_CURRENT_SOURCE_DIR}/openssl3.cmake")

function(py3_module name)
	cmake_parse_arguments(PY3MOD "" "" "SOURCES;LIBRARIES;INCLUDES;DEFINITIONS;DEPENDS;FLAGS" ${ARGN})

	if (NOT PY3MOD_SOURCES)
		message(FATAL_ERROR "py3_module(${name}): SOURCES are required")
	endif ()

	set(target "py313_${name}")
	set(module_sources "")
	foreach (src IN LISTS PY3MOD_SOURCES)
		if (IS_ABSOLUTE "${src}")
			list(APPEND module_sources "${src}")
		else ()
			list(APPEND module_sources "${PYTHON3_SOURCE_DIR}/${src}")
		endif ()
	endforeach ()
	add_darling_bundle("${target}" "" ${module_sources})
	py3_target_setup("${target}")
	set_property(TARGET "${target}" APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-undefined,dynamic_lookup ")
	set_target_properties("${target}" PROPERTIES
		OUTPUT_NAME "${name}"
		PREFIX ""
		SUFFIX ".so"
	)
	target_link_libraries("${target}" system ${PY3MOD_LIBRARIES})
	if (PY3MOD_INCLUDES)
		target_include_directories("${target}" BEFORE PRIVATE ${PY3MOD_INCLUDES})
	endif ()
	if (PY3MOD_DEFINITIONS)
		target_compile_definitions("${target}" PRIVATE ${PY3MOD_DEFINITIONS})
	endif ()
	if (PY3MOD_FLAGS)
		target_compile_options("${target}" PRIVATE ${PY3MOD_FLAGS})
	endif ()
	add_dependencies("${target}" python313exe py3_generated ${PY3MOD_DEPENDS})
	make_fat("${target}")
	install(TARGETS "${target}" DESTINATION "${PYTHON3_DYNLOAD_DIR}")
endfunction()

py3_module(array SOURCES Modules/arraymodule.c)
py3_module(binascii SOURCES Modules/binascii.c)
py3_module(cmath SOURCES Modules/cmathmodule.c Modules/_math.c)
py3_module(math SOURCES Modules/mathmodule.c Modules/_math.c)
py3_module(select SOURCES Modules/selectmodule.c)
py3_module(termios SOURCES Modules/termios.c)
py3_module(unicodedata SOURCES Modules/unicodedata.c)

py3_module(_asyncio SOURCES Modules/_asynciomodule.c)
py3_module(_curses SOURCES Modules/_cursesmodule.c LIBRARIES ncurses)
py3_module(_curses_panel SOURCES Modules/_curses_panel.c LIBRARIES ncurses panel)
py3_module(_bisect SOURCES Modules/_bisectmodule.c)
py3_module(_blake2 SOURCES
	Modules/_blake2/blake2module.c
	Modules/_blake2/blake2b_impl.c
	Modules/_blake2/blake2s_impl.c
)
py3_module(_bz2 SOURCES Modules/_bz2module.c LIBRARIES bz2)
py3_module(_codecs_cn SOURCES Modules/cjkcodecs/_codecs_cn.c)
py3_module(_codecs_hk SOURCES Modules/cjkcodecs/_codecs_hk.c)
py3_module(_codecs_iso2022 SOURCES Modules/cjkcodecs/_codecs_iso2022.c)
py3_module(_codecs_jp SOURCES Modules/cjkcodecs/_codecs_jp.c)
py3_module(_codecs_kr SOURCES Modules/cjkcodecs/_codecs_kr.c)
py3_module(_codecs_tw SOURCES Modules/cjkcodecs/_codecs_tw.c)
py3_module(_contextvars SOURCES Modules/_contextvarsmodule.c)
py3_module(_csv SOURCES Modules/_csv.c)
py3_module(_ctypes SOURCES
	Modules/_ctypes/_ctypes.c
	Modules/_ctypes/callbacks.c
	Modules/_ctypes/callproc.c
	Modules/_ctypes/cfield.c
	Modules/_ctypes/stgdict.c
	LIBRARIES ffi
)
py3_module(_datetime SOURCES Modules/_datetimemodule.c)
py3_module(_dbm SOURCES Modules/_dbmmodule.c
	LIBRARIES berkeley_db
	INCLUDES "${CMAKE_SOURCE_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/BerkeleyDB"
	DEFINITIONS USE_BERKDB DB_DBM_HSEARCH=1
)
py3_module(_elementtree SOURCES Modules/_elementtree.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/expat"
)
py3_module(_heapq SOURCES Modules/_heapqmodule.c)
py3_module(_interpchannels SOURCES Modules/_interpchannelsmodule.c)
py3_module(_interpreters SOURCES Modules/_interpretersmodule.c)
py3_module(_interpqueues SOURCES Modules/_interpqueuesmodule.c)
py3_module(_json SOURCES Modules/_json.c)
py3_module(_lsprof SOURCES Modules/_lsprof.c Modules/rotatingtree.c)
py3_module(_lzma SOURCES Modules/_lzmamodule.c LIBRARIES lzma)
py3_module(_multibytecodec SOURCES Modules/cjkcodecs/multibytecodec.c)
py3_module(_multiprocessing SOURCES
	Modules/_multiprocessing/multiprocessing.c
	Modules/_multiprocessing/semaphore.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_multiprocessing"
)
py3_module(_opcode SOURCES Modules/_opcode.c)
py3_module(_pickle SOURCES Modules/_pickle.c)
py3_module(_posixshmem SOURCES Modules/_multiprocessing/posixshmem.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_multiprocessing"
)
py3_module(_queue SOURCES Modules/_queuemodule.c)
py3_module(_random SOURCES Modules/_randommodule.c)
py3_module(_scproxy SOURCES Modules/_scproxy.c LIBRARIES SystemConfiguration)
py3_module(_socket SOURCES Modules/socketmodule.c)
py3_module(_sqlite3 SOURCES
	Modules/_sqlite/blob.c
	Modules/_sqlite/connection.c
	Modules/_sqlite/cursor.c
	Modules/_sqlite/microprotocols.c
	Modules/_sqlite/module.c
	Modules/_sqlite/prepare_protocol.c
	Modules/_sqlite/row.c
	Modules/_sqlite/statement.c
	Modules/_sqlite/util.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_sqlite"
	LIBRARIES sqlite3
	DEFINITIONS "MODULE_NAME=\"sqlite3\""
)
py3_module(_statistics SOURCES Modules/_statisticsmodule.c)
py3_module(_struct SOURCES Modules/_struct.c)
py3_module(_uuid SOURCES Modules/_uuidmodule.c)
py3_module(_zoneinfo SOURCES Modules/_zoneinfo.c)

py3_module(_md5 SOURCES
	Modules/md5module.c
	Modules/_hacl/Hacl_Hash_MD5.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_hacl/include"
	DEFINITIONS _BSD_SOURCE _DEFAULT_SOURCE
)
py3_module(_sha1 SOURCES
	Modules/sha1module.c
	Modules/_hacl/Hacl_Hash_SHA1.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_hacl/include"
	DEFINITIONS _BSD_SOURCE _DEFAULT_SOURCE
)
py3_module(_sha2 SOURCES
	Modules/sha2module.c
	Modules/_hacl/Hacl_Hash_SHA2.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_hacl/include"
	DEFINITIONS _BSD_SOURCE _DEFAULT_SOURCE
)
py3_module(_sha3 SOURCES
	Modules/sha3module.c
	Modules/_hacl/Hacl_Hash_SHA3.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_hacl/include"
	DEFINITIONS _BSD_SOURCE _DEFAULT_SOURCE
)

py3_module(_hashlib SOURCES Modules/_hashopenssl.c
	INCLUDES ${PYTHON3_OPENSSL_INCLUDE_DIRS}
	FLAGS ${PYTHON3_OPENSSL_ARCH_INCLUDE_OPTIONS}
	LIBRARIES py3_openssl_ssl py3_openssl_crypto
	DEPENDS
		py3_openssl_ssl_lipo
		py3_openssl_crypto_lipo
)
py3_module(_ssl SOURCES Modules/_ssl.c
	INCLUDES ${PYTHON3_OPENSSL_INCLUDE_DIRS}
	FLAGS ${PYTHON3_OPENSSL_ARCH_INCLUDE_OPTIONS}
	LIBRARIES py3_openssl_ssl py3_openssl_crypto
	DEPENDS
		py3_openssl_ssl_lipo
		py3_openssl_crypto_lipo
)

py3_module(fcntl SOURCES Modules/fcntlmodule.c)
py3_module(grp SOURCES Modules/grpmodule.c)
py3_module(mmap SOURCES Modules/mmapmodule.c)
py3_module(resource SOURCES Modules/resource.c)
py3_module(readline SOURCES Modules/readline.c LIBRARIES ncurses edit)
py3_module(syslog SOURCES Modules/syslogmodule.c)
py3_module(pyexpat SOURCES
	Modules/pyexpat.c
	Modules/expat/xcs.c
	Modules/expat/xmlparse.c
	Modules/expat/xmlrole.c
	Modules/expat/xmltok.c
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/expat"
)
py3_module(zlib SOURCES Modules/zlibmodule.c LIBRARIES z)

file(GLOB PYTHON3_DECIMAL_SOURCES "${PYTHON3_SOURCE_DIR}/Modules/_decimal/libmpdec/*.c")
list(REMOVE_ITEM PYTHON3_DECIMAL_SOURCES "${PYTHON3_SOURCE_DIR}/Modules/_decimal/libmpdec/mpsignal.c")
darling_xarch_flags(PYTHON3_DECIMAL_32_FLAGS ARCHS ${DARLING_ARCHS_32BIT}
	OPTIONS "-DCONFIG_64=0" "-UHAVE_UINT128_T")
darling_xarch_flags(PYTHON3_DECIMAL_64_FLAGS ARCHS ${DARLING_ARCHS_64BIT}
	OPTIONS "-DCONFIG_64=1" "-DHAVE_UINT128_T=1")
set_source_files_properties(${PYTHON3_DECIMAL_SOURCES} PROPERTIES COMPILE_FLAGS
	"-DANSI=1 ${PYTHON3_DECIMAL_32_FLAGS} ${PYTHON3_DECIMAL_64_FLAGS}")
set(PYTHON3_DECIMAL_MODULE_SOURCE "${PYTHON3_SOURCE_DIR}/Modules/_decimal/_decimal.c")
set_source_files_properties("${PYTHON3_DECIMAL_MODULE_SOURCE}" PROPERTIES COMPILE_FLAGS
	"-DANSI=1 ${PYTHON3_DECIMAL_32_FLAGS} ${PYTHON3_DECIMAL_64_FLAGS}")
py3_module(_decimal SOURCES "${PYTHON3_DECIMAL_MODULE_SOURCE}" ${PYTHON3_DECIMAL_SOURCES}
	INCLUDES "${PYTHON3_SOURCE_DIR}/Modules/_decimal/libmpdec"
)

#
# Standard library, headers and installation data.
#
file(GLOB_RECURSE PYTHON3_STDLIB_FILES RELATIVE "${PYTHON3_SOURCE_DIR}/Lib"
	"${PYTHON3_SOURCE_DIR}/Lib/*.py")
list(FILTER PYTHON3_STDLIB_FILES EXCLUDE REGEX "^site-packages/")
list(FILTER PYTHON3_STDLIB_FILES EXCLUDE REGEX "(^|/)test/")

install(DIRECTORY "${PYTHON3_SOURCE_DIR}/Lib/"
	DESTINATION "${PYTHON3_STDLIB_DIR}"
	FILES_MATCHING PATTERN "*.py"
	PATTERN "site-packages" EXCLUDE
	PATTERN "test" EXCLUDE
	PATTERN "__pycache__" EXCLUDE
)

set(PYTHON3_STDLIB_ABSOLUTE_FILES "")
foreach (pyfile IN LISTS PYTHON3_STDLIB_FILES)
	list(APPEND PYTHON3_STDLIB_ABSOLUTE_FILES "${PYTHON3_SOURCE_DIR}/Lib/${pyfile}")
endforeach ()
pyc(py313_stdlib_pyc
	PYTHON_VERSION 3.13
	ROOT "${PYTHON3_SOURCE_DIR}/Lib"
	SOURCES ${PYTHON3_STDLIB_ABSOLUTE_FILES}
	DESTINATION "${PYTHON3_STDLIB_DIR}"
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/_sysconfigdata__darwin_darwin.py"
	DESTINATION "${PYTHON3_STDLIB_DIR}")
pyc(py313_sysconfigdata_pyc
	PYTHON_VERSION 3.13
	ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
	SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/_sysconfigdata__darwin_darwin.py"
	DESTINATION "${PYTHON3_STDLIB_DIR}"
)

InstallSymlink("include/python${PYTHON3_VERSION}"
	"${CMAKE_INSTALL_PREFIX}${PYTHON3_FRAMEWORK_DIR}/Headers")
install(DIRECTORY "${PYTHON3_SOURCE_DIR}/Include/"
	DESTINATION "${PYTHON3_INSTALL_DIR}/include/python${PYTHON3_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pyconfig.h"
	DESTINATION "${PYTHON3_INSTALL_DIR}/include/python${PYTHON3_VERSION}")

install(DIRECTORY DESTINATION "${PYTHON3_INSTALL_DIR}/Extras/lib/python")
install(FILES "${PYTHON3_SOURCE_DIR}/Lib/site-packages/README.txt"
	DESTINATION libexec/darling/Library/Python/${PYTHON3_VERSION}/site-packages)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Extras-3.13.pth"
	DESTINATION libexec/darling/Library/Python/${PYTHON3_VERSION}/site-packages
	RENAME Extras.pth)

set(PYTHON3_CONFIG_DIR "${PYTHON3_INSTALL_DIR}/lib/python${PYTHON3_VERSION}/config-${PYTHON3_VERSION}-darwin")
install(FILES "${PYTHON3_SOURCE_DIR}/Makefile.pre.in"
	DESTINATION "${PYTHON3_CONFIG_DIR}"
	RENAME Makefile)
install(FILES
		"${PYTHON3_SOURCE_DIR}/Modules/Setup"
		"${PYTHON3_SOURCE_DIR}/Modules/config.c.in"
		"${PYTHON3_SOURCE_DIR}/Modules/makesetup"
		"${CMAKE_CURRENT_BINARY_DIR}/config.c"
		"${CMAKE_CURRENT_BINARY_DIR}/Modules/Setup.local"
		"${CMAKE_CURRENT_BINARY_DIR}/Modules/Setup.bootstrap"
	DESTINATION "${PYTHON3_CONFIG_DIR}")
install(FILES $<TARGET_OBJECTS:python313exe_obj>
	DESTINATION "${PYTHON3_CONFIG_DIR}"
	RENAME python.o)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap"
	DESTINATION "${PYTHON3_INSTALL_DIR}/Modules")
