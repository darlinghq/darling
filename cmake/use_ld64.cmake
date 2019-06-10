set(dylib_paths "")
FUNCTION(use_ld64 target)
	get_property(ld_dylib_paths GLOBAL PROPERTY ld_dylib_paths)
	set_property(TARGET ${target} APPEND_STRING PROPERTY
		LINK_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ld64/src/ \
-B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ \
-target x86_64-apple-darwin11 -Wl,-Z \
-Wl,-dylib_file,/usr/lib/system/libsystem_c.dylib:${CMAKE_BINARY_DIR}/src/libc/libsystem_c_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_kernel.dylib:${CMAKE_BINARY_DIR}/src/kernel/libsystem_kernel_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libunwind.dylib:${CMAKE_BINARY_DIR}/src/libunwind/libunwind_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_m.dylib:${CMAKE_BINARY_DIR}/src/libm/libsystem_m_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libcompiler_rt.dylib:${CMAKE_BINARY_DIR}/src/external/compiler-rt/lib/builtins/libcompiler_rt_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libdyld.dylib:${CMAKE_BINARY_DIR}/src/dyld/libsystem_dyld_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_info.dylib:${CMAKE_BINARY_DIR}/src/libinfo/libsystem_info_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_blocks.dylib:${CMAKE_BINARY_DIR}/src/external/libclosure/libsystem_blocks_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/libc++abi.dylib:${CMAKE_BINARY_DIR}/src/external/libcxxabi/libc++abi.dylib \
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
-Wl,-dylib_file,/usr/lib/system/libcorecrypto.dylib:${CMAKE_BINARY_DIR}/src/external/corecrypto/libcorecrypto_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_coretls.dylib:${CMAKE_BINARY_DIR}/src/external/coretls/libsystem_coretls.dylib \
-Wl,-dylib_file,/usr/lib/system/libcommonCrypto.dylib:${CMAKE_BINARY_DIR}/src/external/commoncrypto/libcommonCrypto.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_asl.dylib:${CMAKE_BINARY_DIR}/src/external/syslog/libsystem_asl.tproj/libsystem_asl_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/libresolv.9.dylib:${CMAKE_BINARY_DIR}/src/libresolv/libresolv.9.dylib \
-Wl,-dylib_file,/usr/lib/system/libxpc.dylib:${CMAKE_BINARY_DIR}/src/external/libxpc/libxpc_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/libc++.1.dylib:${CMAKE_BINARY_DIR}/src/external/libcxx/libc++.1.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_malloc.dylib:${CMAKE_BINARY_DIR}/src/libmalloc/libsystem_malloc_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/libobjc.A.dylib:${CMAKE_BINARY_DIR}/src/external/objc4/runtime/libobjc.A.dylib \
-Wl,-dylib_file,/usr/lib/libicucore.A.dylib:${CMAKE_BINARY_DIR}/src/external/icu/icuSources/libicucore.A.dylib \
-Wl,-dylib_file,/usr/lib/libncurses.5.4.dylib:${CMAKE_BINARY_DIR}/src/ncurses/ncurses/libncurses.5.4.dylib \
-Wl,-dylib_file,/usr/lib/libSystem.B.dylib:${CMAKE_BINARY_DIR}/src/libsystem/libSystem.B.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Python.framework/Versions/2.6/Python:${CMAKE_BINARY_DIR}/src/external/python/2.6/Python-2.6.9/Python \
-Wl,-dylib_file,/System/Library/Frameworks/Python.framework/Versions/2.7/Python:${CMAKE_BINARY_DIR}/src/external/python/2.7/Python-2.7.10/Python \
-Wl,-dylib_file,/System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation:${CMAKE_BINARY_DIR}/src/external/corefoundation/CoreFoundation \
-Wl,-dylib_file,/System/Library/Frameworks/Ruby.framework/Versions/2.0/Ruby:${CMAKE_BINARY_DIR}/src/external/ruby/Ruby \
-Wl,-dylib_file,/usr/lib/libiconv.2.dylib:${CMAKE_BINARY_DIR}/src/libiconv/libiconv.2.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CFNetwork.framework/Versions/A/CFNetwork:${CMAKE_BINARY_DIR}/src/external/cfnetwork/src/CFNetwork \
-Wl,-dylib_file,/usr/lib/libxml2.2.dylib:${CMAKE_BINARY_DIR}/src/external/libxml2/libxml2.2.dylib \
-Wl,-dylib_file,/usr/lib/libpam.2.dylib:${CMAKE_BINARY_DIR}/src/external/openpam/libpam.2.dylib \
-Wl,-dylib_file,/usr/lib/libbsm.0.dylib:${CMAKE_BINARY_DIR}/src/external/bsm/libbsm.0.dylib \
-Wl,-dylib_file,/usr/lib/libauto.dylib:${CMAKE_BINARY_DIR}/src/external/libauto/libauto.dylib \
-Wl,-dylib_file,/usr/lib/libxar.1.dylib:${CMAKE_BINARY_DIR}/src/xar/libxar.1.dylib \
-Wl,-dylib_file,/usr/lib/libsqlite3.0.dylib:${CMAKE_BINARY_DIR}/src/external/sqlite/libsqlite3.0.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CryptoTokenKit.framework/Versions/A/CryptoTokenKit:${CMAKE_BINARY_DIR}/src/CryptoTokenKit/CryptoTokenKit \
-Wl,-dylib_file,/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit:${CMAKE_BINARY_DIR}/src/external/IOKitUser/IOKit \
-Wl,-dylib_file,/System/Library/Frameworks/SystemConfiguration.framework/Versions/A/SystemConfiguration:${CMAKE_BINARY_DIR}/src/SystemConfiguration/SystemConfiguration	\
-Wl,-dylib_file,/usr/lib/libbz2.1.0.dylib:${CMAKE_BINARY_DIR}/src/external/bzip2/libbz2.1.0.dylib \
-Wl,-dylib_file,/usr/lib/libssl.0.9.8.dylib:${CMAKE_BINARY_DIR}/src/external/openssl/src/libssl.0.9.8.dylib \
-Wl,-dylib_file,/usr/lib/libcrypto.0.9.8.dylib:${CMAKE_BINARY_DIR}/src/external/openssl/src/libcrypto.0.9.8.dylib \
-Wl,-dylib_file,/usr/lib/native/libGL.dylib:${CMAKE_BINARY_DIR}/src/native/libGL.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CoreImage.framework/Versions/A/CoreImage:${CMAKE_BINARY_DIR}/src/CoreImage/CoreImage \
-Wl,-dylib_file,/System/Library/Frameworks/CoreVideo.framework/Versions/A/CoreVideo:${CMAKE_BINARY_DIR}/src/CoreVideo/CoreVideo \
${ld_dylib_paths}")

	add_dependencies(${target} x86_64-apple-darwin11-ld)

ENDFUNCTION(use_ld64)

function(reexport reexporter reexportee)
	add_dependencies(${reexporter} ${reexportee})
	set(reexportee_binary_dir "$<TARGET_PROPERTY:${reexportee},BINARY_DIR>")
	set(reexportee_output_name "$<TARGET_PROPERTY:${reexportee},OUTPUT_NAME>")
	#message("binary dir: ${reexportee_binary_dir}, output name: ${reexportee_output_name}")
	if(NOT DEFINED reexportee_output_name)
		set(reexportee_output_name "$<TARGET_PROPERTY:${reexportee},DYLIB_BUILD_NAME>")
		if(NOT DEFINED reexportee_output_name)
			message(FATAL_ERROR "failed to get reexportee path")
		endif(NOT DEFINED reexportee_output_name)
	endif(NOT DEFINED reexportee_output_name)
	set(reexportee_output "${reexportee_binary_dir}/${reexportee_output_name}")
	set(reexportee_install_name "$<TARGET_PROPERTY:${reexportee},DYLIB_INSTALL_NAME>")
	set_property(TARGET ${reexporter} APPEND PROPERTY
		LINK_OPTIONS " -Wl,-reexport_library,${reexportee_output} ")
	#set_property(GLOBAL APPEND_STRING PROPERTY ld_dylib_paths " -Wl,-dylib_file,${reexportee_install_name}:${reexportee_output} ")
endfunction(reexport)
