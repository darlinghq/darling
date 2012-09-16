project(cxxdarwin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I./include -stdlib=libc++ -fPIC -gdwarf-2")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic -Wl,-Bsymbolic-functions -stdlib=libc++")
cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set (unwind_SRCS
#	emutls.c
	src-unwind/unwind-c.c
	src-unwind/unwind-compat.c
	src-unwind/unwind-dw2-fde-darwin.c
	src-unwind/unwind-dw2-fde-compat.c
	src-unwind/unwind-dw2.c
	src-unwind/darwin-crt2.c
#	src-unwind/unwind-sjlj.c
)

set (abi_SRCS
	src-cxxabi/abort_message.cpp src-cxxabi/cxa_demangle.cpp
	src-cxxabi/cxa_guard.cpp src-cxxabi/cxa_personality.cpp
	src-cxxabi/cxa_virtual.cpp src-cxxabi/stdexcept.cpp
	src-cxxabi/cxa_aux_runtime.cpp
	src-cxxabi/cxa_exception.cpp src-cxxabi/cxa_handlers.cpp
	src-cxxabi/cxa_unexpected.cpp src-cxxabi/exception.cpp
	src-cxxabi/typeinfo.cpp
	src-cxxabi/cxa_default_handlers.cpp
	src-cxxabi/cxa_exception_storage.cpp
	src-cxxabi/cxa_new_delete.cpp
	src-cxxabi/cxa_vector.cpp
	src-cxxabi/private_typeinfo.cpp
)


add_library(cxxdarwin SHARED ${unwind_SRCS} ${abi_SRCS})
#target_link_libraries(unwind-darwin -Tlink.cmd)
install(TARGETS cxxdarwin DESTINATION lib${SUFFIX})
