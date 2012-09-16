project(cxxdarwin)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM-ATT)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -std=c99 -D__darwin_natural_t=long -I./include -I ../../include/darwin -fPIC -Ddladdr=__darwin_dladdr -gdwarf-2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I./include -stdlib=libc++ -fPIC -Ddl_info=Dl_info -Ddladdr=__darwin_dladdr -Wno-format -gdwarf-2")
set(CMAKE_SHARED_LINKER_FLAGS "-stdlib=libc++ -Wl,-Bsymbolic -Wl,-Bsymbolic-functions")

if (SUFFIX STREQUAL "32")
	set(FLAGS "-m32")
	SET(CMAKE_ASM-ATT_FLAGS "--32")
endif (SUFFIX STREQUAL "32")

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}")
ADD_CUSTOM_COMMAND(OUTPUT src-unwind/Registers.s
	COMMAND ${CMAKE_C_COMPILER} ARGS -E src-unwind/Registers.S ${FLAGS} -o src-unwind/Registers.s
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
ADD_CUSTOM_COMMAND(OUTPUT src-unwind/unw_getcontext.s
	COMMAND "${CMAKE_C_COMPILER}" ARGS -E src-unwind/unw_getcontext.S ${FLAGS} -o src-unwind/unw_getcontext.s
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set (cxxdarwin_SRCS
	src-unwind/UnwindLevel1.c  src-unwind/UnwindLevel1-gcc-ext.c
	src-unwind/Registers.s  src-unwind/unw_getcontext.s
	src-unwind/libuwind.cxx
)

set (abi_SRCS
	src-cxxabi/abort_message.cpp src-cxxabi/cxa_demangle.cpp src-cxxabi/cxa_guard.cpp src-cxxabi/cxa_personality.cpp  src-cxxabi/cxa_virtual.cpp src-cxxabi/stdexcept.cpp
	src-cxxabi/cxa_aux_runtime.cpp src-cxxabi/cxa_exception.cpp src-cxxabi/cxa_handlers.cpp src-cxxabi/cxa_unexpected.cpp src-cxxabi/exception.cpp src-cxxabi/typeinfo.cpp
	src-cxxabi/cxa_default_handlers.cpp src-cxxabi/cxa_exception_storage.cpp src-cxxabi/cxa_new_delete.cpp src-cxxabi/cxa_vector.cpp src-cxxabi/private_typeinfo.cpp
)

add_library(cxxdarwin SHARED ${cxxdarwin_SRCS} ${abi_SRCS})
install(TARGETS cxxdarwin DESTINATION lib${SUFFIX})

