project(DebugSymbols)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(DebugSymbols_sources
    DebugSymbols.mm
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(DebugSymbols SHARED ${DebugSymbols_sources})

install(TARGETS DebugSymbols DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")
