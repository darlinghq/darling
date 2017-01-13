SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR "x86-64")

SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
SET(BITS 64)

if (EXISTS "/etc/debian_version")
	SET (CMAKE_INSTALL_LIBDIR "lib/x86_64-linux-gnu")
else (EXISTS "/etc/debian_version")
	SET (CMAKE_INSTALL_LIBDIR "lib64")
endif (EXISTS "/etc/debian_version")
