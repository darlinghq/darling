FUNCTION(use_ld64 target)
	set_property(TARGET ${target} APPEND_STRING PROPERTY
		LINK_FLAGS " -fuse-ld=${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ld64/src/x86_64-apple-darwin11-ld ")

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
-Wl,-dylib_file,/usr/lib/system/libsystem_trace.dylib:${CMAKE_BINARY_DIR}/src/external/libtrace/libsystem_trace_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libcorecrypto.dylib:${CMAKE_BINARY_DIR}/src/external/corecrypto/libcorecrypto_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_dnssd.dylib:${CMAKE_BINARY_DIR}/src/lib/system/dnssd/libsystem_dnssd.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_networkextension.dylib:${CMAKE_BINARY_DIR}/src/networkextension/libsystem_networkextension.dylib \
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
-Wl,-dylib_file,/usr/lib/libDiagnosticMessagesClient.dylib:${CMAKE_BINARY_DIR}/src/libDiagnosticMessagesClient/libDiagnosticMessagesClient.dylib \
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
-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:${CMAKE_BINARY_DIR}/src/native/libGL.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CoreImage.framework/Versions/A/CoreImage:${CMAKE_BINARY_DIR}/src/frameworks/CoreImage/CoreImage \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/FSEvents.framework/Versions/A/FSEvents:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/FSEvents \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/LaunchServices:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/LaunchServices \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libvMisc.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/vMisc/libvMisc.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libvDSP.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/vDSP/libvDSP.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libBLAS.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/BLAS/libBLAS.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libLAPACK.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/LAPACK/libLAPACK.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libLinearAlgebra.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/LinearAlgebra/libLinearAlgebra.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libSparseBLAS.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/SparseBLAS/libSparseBLAS.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libQuadrature.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/Quadrature/libQuadrature.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libBNNS.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/BNNS/libBNNS.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libSparse.dylib\
:${CMAKE_BINARY_DIR}/src/frameworks/Accelerate/vecLib/Sparse/libSparse.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/ColorSync.framework/Versions/A/ColorSync:${CMAKE_BINARY_DIR}/src/frameworks/ColorSync/ColorSync \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ATS.framework/Versions/A/ATS\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/ATS/ATS \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ColorSyncLegacy.framework/Versions/A/ColorSyncLegacy\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/ColorSyncLegacy/ColorSyncLegacy \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/HIServices.framework/Versions/A/HIServices\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/HIServices/HIServices \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/LangAnalysis.framework/Versions/A/LangAnalysis\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/LangAnalysis/LangAnalysis \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/PrintCore.framework/Versions/A/PrintCore\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/PrintCore/PrintCore \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/QD.framework/Versions/A/QD\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/QD/QD \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/SpeechSynthesis.framework/Versions/A/SpeechSynthesis\
:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/SpeechSynthesis/SpeechSynthesis \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/CoreServices:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/CoreServices \
-Wl,-dylib_file,/System/Library/Frameworks/ApplicationServices.framework/Versions/A/ApplicationServices:${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/ApplicationServices \
-Wl,-dylib_file,/System/Library/Frameworks/CoreGraphics.framework/Versions/A/CoreGraphics:${CMAKE_BINARY_DIR}/src/external/cocotron/CoreGraphics/CoreGraphics \
-Wl,-dylib_file,/System/Library/Frameworks/CoreText.framework/Versions/A/CoreText:${CMAKE_BINARY_DIR}/src/external/cocotron/CoreText/CoreText \
-Wl,-dylib_file,/System/Library/Frameworks/ImageIO.framework/Versions/A/ImageIO:${CMAKE_BINARY_DIR}/src/ImageIO/ImageIO \
-Wl,-dylib_file,/System/Library/Frameworks/LocalAuthentication.framework/Versions/A/LocalAuthentication:${CMAKE_BINARY_DIR}/src/LocalAuthentication/LocalAuthentication \
-Wl,-dylib_file,/System/Library/Frameworks/CoreVideo.framework/Versions/A/CoreVideo:${CMAKE_BINARY_DIR}/src/CoreVideo/CoreVideo")

	add_dependencies(${target} x86_64-apple-darwin11-ld)

ENDFUNCTION(use_ld64)

function(reexport reexporter reexportee reexportee_output)
	add_dependencies(${reexporter} ${reexportee})
	set_property(TARGET ${reexporter} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-reexport_library,${reexportee_output} ")
endfunction(reexport)
