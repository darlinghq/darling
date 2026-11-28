# Build a private, static OpenSSL 3 for CPython 3.13.
#
# Python 2 continues to use Darling's LibreSSL-backed ssl46/crypto44 targets.
# PEP 644 removed LibreSSL support from CPython, so the Python 3 extension
# modules need an OpenSSL 3 build of their own. The build is per architecture
# and the thin archives are joined with the same lipo helper Darling uses for
# its other Mach-O libraries.

include(ExternalProject)
include(architecture)

if (NOT PYTHON3_OPENSSL3_INCLUDED)
	set(PYTHON3_OPENSSL3_INCLUDED TRUE)

	find_program(PYTHON3_PERL_EXECUTABLE NAMES perl)
	find_program(PYTHON3_MAKE_EXECUTABLE NAMES make gmake)
	find_program(PYTHON3_LLVM_AR_EXECUTABLE NAMES llvm-ar llvm-ar-19 llvm-ar-18 llvm-ar-17)
	find_program(PYTHON3_LLVM_RANLIB_EXECUTABLE NAMES llvm-ranlib llvm-ranlib-19 llvm-ranlib-18 llvm-ranlib-17)
	if (NOT PYTHON3_PERL_EXECUTABLE OR NOT PYTHON3_MAKE_EXECUTABLE OR NOT PYTHON3_LLVM_AR_EXECUTABLE OR NOT PYTHON3_LLVM_RANLIB_EXECUTABLE)
		message(FATAL_ERROR "Building Python 3's OpenSSL 3 requires perl, make, llvm-ar and llvm-ranlib")
	endif ()

	set(PYTHON3_OPENSSL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openssl")
	set(PYTHON3_OPENSSL_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}/openssl")
	# OpenSSL keeps static headers in the source tree and generates the rest
	# (notably opensslv.h and opensslconf.h) into the build tree. Both are
	# needed when compiling CPython's ssl/hashlib modules. The generated
	# headers are selected per slice because opensslconf.h encodes the target
	# word size; see PYTHON3_OPENSSL_ARCH_INCLUDE_OPTIONS below.
	set(PYTHON3_OPENSSL_INCLUDE_DIRS
		"${PYTHON3_OPENSSL_SOURCE_DIR}/include"
	)
	set(PYTHON3_OPENSSL_ARCH_INCLUDE_OPTIONS "")
	foreach (arch IN LISTS DARLING_ARCHS)
		list(APPEND PYTHON3_OPENSSL_ARCH_INCLUDE_OPTIONS
			"-Xarch_${arch}" "-I${PYTHON3_OPENSSL_BUILD_ROOT}/${arch}/include")
	endforeach ()

	file(MAKE_DIRECTORY "${PYTHON3_OPENSSL_BUILD_ROOT}")

	set(PYTHON3_OPENSSL_CRYPTO_SLICES "")
	set(PYTHON3_OPENSSL_SSL_SLICES "")

	foreach (arch IN LISTS DARLING_ARCHS)
		darling_arch_triple("${arch}" triple)

		if (arch STREQUAL "i386")
			set(openssl_target "darwin-i386-cc")
		elseif (arch STREQUAL "x86_64")
			set(openssl_target "darwin64-x86_64-cc")
		elseif (arch STREQUAL "arm64")
			set(openssl_target "darwin64-arm64-cc")
		else ()
			message(FATAL_ERROR "No OpenSSL 3 configuration for architecture ${arch}")
		endif ()

		set(slice_build_dir "${PYTHON3_OPENSSL_BUILD_ROOT}/${arch}")
		set(cc_wrapper "${slice_build_dir}/cc")
		set(build_script "${slice_build_dir}/build.sh")
		file(MAKE_DIRECTORY "${slice_build_dir}")

		# Do not disable additional method constructors. CPython 3.13 still
		# references fixed-version SSL/TLS method constructors that OpenSSL 4
		# removes; that is an upstream CPython migration item and will be handled
		# when the bundled CPython and OpenSSL are bumped together. No
		# Darling-owned code uses those removed APIs. OpenSSL's own default
		# policy (such as no-ssl3-method) still applies.
		#
		# OpenSSL's Makefile takes the compiler from CC as a single string. A wrapper
		# avoids quoting a multi-word clang invocation in that variable and keeps
		# every slice on the matching Darling target triple and SDK.
		file(WRITE "${cc_wrapper}"
			"#!/bin/sh\n"
			"exec \"${CMAKE_C_COMPILER}\" -target \"${triple}\" -isysroot \"${CMAKE_SOURCE_DIR}/${DARLING_SDK_RELATIVE_PATH}\" -mmacosx-version-min=\"${CMAKE_OSX_DEPLOYMENT_TARGET}\" \"$@\"\n")
		file(WRITE "${build_script}"
			"#!/bin/sh\n"
			"set -e\n"
			"cd \"${slice_build_dir}\"\n"
			"\"${PYTHON3_MAKE_EXECUTABLE}\" build_generated\n"
			"\"${PYTHON3_MAKE_EXECUTABLE}\" libcrypto.a libssl.a\n")
		execute_process(COMMAND "${CMAKE_COMMAND}" -E chmod +x "${cc_wrapper}" "${build_script}")

		ExternalProject_Add(py3_openssl_${arch}
			SOURCE_DIR "${PYTHON3_OPENSSL_SOURCE_DIR}"
			BINARY_DIR "${slice_build_dir}"
			DOWNLOAD_COMMAND ""
			UPDATE_COMMAND ""
			PATCH_COMMAND ""
			CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E env
				"CC=${cc_wrapper}"
				"AR=${PYTHON3_LLVM_AR_EXECUTABLE}"
				"RANLIB=${PYTHON3_LLVM_RANLIB_EXECUTABLE}"
				"${PYTHON3_PERL_EXECUTABLE}" "<SOURCE_DIR>/Configure"
					"${openssl_target}" no-shared no-tests no-engine no-asm
			BUILD_COMMAND "${CMAKE_COMMAND}" -E env
				"AR=${PYTHON3_LLVM_AR_EXECUTABLE}"
				"RANLIB=${PYTHON3_LLVM_RANLIB_EXECUTABLE}"
				"${build_script}"
			INSTALL_COMMAND ""
			BUILD_BYPRODUCTS
				"${slice_build_dir}/libcrypto.a"
				"${slice_build_dir}/libssl.a"
		)

		add_library(py3_openssl_crypto_${arch} STATIC IMPORTED GLOBAL)
		set_target_properties(py3_openssl_crypto_${arch} PROPERTIES
			IMPORTED_LOCATION "${slice_build_dir}/libcrypto.a")
		add_dependencies(py3_openssl_crypto_${arch} py3_openssl_${arch})

		add_library(py3_openssl_ssl_${arch} STATIC IMPORTED GLOBAL)
		set_target_properties(py3_openssl_ssl_${arch} PROPERTIES
			IMPORTED_LOCATION "${slice_build_dir}/libssl.a")
		add_dependencies(py3_openssl_ssl_${arch} py3_openssl_${arch})

		list(APPEND PYTHON3_OPENSSL_CRYPTO_SLICES py3_openssl_crypto_${arch})
		list(APPEND PYTHON3_OPENSSL_SSL_SLICES py3_openssl_ssl_${arch})
	endforeach ()

	darling_lipo(py3_openssl_crypto IMPORTED STATIC
		OUTPUT "${PYTHON3_OPENSSL_BUILD_ROOT}/libcrypto.a"
		SLICES ${PYTHON3_OPENSSL_CRYPTO_SLICES})
	darling_lipo(py3_openssl_ssl IMPORTED STATIC
		OUTPUT "${PYTHON3_OPENSSL_BUILD_ROOT}/libssl.a"
		SLICES ${PYTHON3_OPENSSL_SSL_SLICES})
endif ()
