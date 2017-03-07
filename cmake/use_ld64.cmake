FUNCTION(use_ld64 target)
	set_property(TARGET ${target} APPEND_STRING PROPERTY
		LINK_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ld64/src/ \
			-B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ \
	   		-target x86_64-apple-darwin11 \
			-Wl,-dylib_file,/usr/lib/system/libsystem_c.dylib:${CMAKE_BINARY_DIR}/src/libc/libsystem_c_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_kernel.dylib:${CMAKE_BINARY_DIR}/src/kernel/libsystem_kernel_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libunwind.dylib:${CMAKE_BINARY_DIR}/src/libunwind/libunwind_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_m.dylib:${CMAKE_BINARY_DIR}/src/libm/libsystem_m_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libcompiler_rt.dylib:${CMAKE_BINARY_DIR}/src/external/compiler-rt/lib/builtins/libcompiler_rt_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libdyld.dylib:${CMAKE_BINARY_DIR}/src/dyld-apple/libsystem_dyld_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_info.dylib:${CMAKE_BINARY_DIR}/src/libinfo/libsystem_info_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_blocks.dylib:${CMAKE_BINARY_DIR}/src/external/libclosure/libsystem_blocks_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/libc++abi.dylib:${CMAKE_BINARY_DIR}/src/external/libcxxabi/src/libc++abi.dylib \
			-Wl,-dylib_file,/usr/lib/system/liblaunch.dylib:${CMAKE_BINARY_DIR}/src/launchd/liblaunch/liblaunch_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libkeymgr.dylib:${CMAKE_BINARY_DIR}/src/keymgr/libkeymgr_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libquarantine.dylib:${CMAKE_BINARY_DIR}/src/quarantine/libquarantine.dylib \
			-Wl,-dylib_file,/usr/lib/system/libremovefile.dylib:${CMAKE_BINARY_DIR}/src/libremovefile/libremovefile.dylib \
			-Wl,-dylib_file,/usr/lib/system/libcopyfile.dylib:${CMAKE_BINARY_DIR}/src/copyfile/libcopyfile.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_coreservices.dylib:${CMAKE_BINARY_DIR}/src/libsystem_coreservices/libsystem_coreservices.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_notify.dylib:${CMAKE_BINARY_DIR}/src/libnotify/libsystem_notify_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libdispatch.dylib:${CMAKE_BINARY_DIR}/src/external/libdispatch/liblibdispatch_shared_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libmacho.dylib:${CMAKE_BINARY_DIR}/src/libmacho/libmacho_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_sandbox.dylib:${CMAKE_BINARY_DIR}/src/sandbox/libsystem_sandbox.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_duct.dylib:${CMAKE_BINARY_DIR}/src/duct/src/libsystem_duct_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_pthread.dylib:${CMAKE_BINARY_DIR}/src/external/libpthread/src/libsystem_pthread_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_platform.dylib:${CMAKE_BINARY_DIR}/src/external/libplatform/libplatform_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/libcommonCrypto.dylib:${CMAKE_BINARY_DIR}/src/CommonCrypto/libcommonCrypto.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_asl.dylib:${CMAKE_BINARY_DIR}/src/external/syslog/libsystem_asl.tproj/libsystem_asl_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/libresolv.9.dylib:${CMAKE_BINARY_DIR}/src/libresolv/libresolv.9.dylib \
			-Wl,-dylib_file,/usr/lib/system/libxpc.dylib:${CMAKE_BINARY_DIR}/src/external/libxpc/libxpc_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/libc++.1.dylib:${CMAKE_BINARY_DIR}/lib/libc++.1.dylib \
			-Wl,-dylib_file,/usr/lib/system/libsystem_malloc.dylib:${CMAKE_BINARY_DIR}/src/libmalloc/libsystem_malloc_firstpass.dylib \
			-Wl,-dylib_file,/usr/lib/libobjc.A.dylib:${CMAKE_BINARY_DIR}/src/external/objc4/runtime/libobjc.A.dylib \
			-Wl,-dylib_file,/usr/lib/libicucore.dylib:${CMAKE_BINARY_DIR}/src/external/icu/icuSources/libicucore.dylib \
			-Wl,-dylib_file,/usr/lib/libncurses.5.4.dylib:${CMAKE_BINARY_DIR}/src/ncurses/ncurses/libncurses.5.4.dylib \
			-Wl,-dylib_file,/System/Library/Frameworks/Python.framework/Versions/2.6/Python:${CMAKE_BINARY_DIR}/src/external/python/2.6/Python-2.6.9/Python \
			-Wl,-dylib_file,/System/Library/Frameworks/Python.framework/Versions/2.7/Python:${CMAKE_BINARY_DIR}/src/external/python/2.7/Python-2.7.10/Python \
			-Wl,-dylib_file,/System/Library/Frameworks/Ruby.framework/Versions/2.0/Ruby:${CMAKE_BINARY_DIR}/src/external/ruby/Ruby ")

	add_dependencies(${target} x86_64-apple-darwin11-ld)

ENDFUNCTION(use_ld64)

