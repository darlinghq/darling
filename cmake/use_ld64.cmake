FUNCTION(use_ld64 target)
	set_property(TARGET ${target} APPEND_STRING PROPERTY
		LINK_FLAGS " -fuse-ld=${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ld64/src/${APPLE_TARGET_TRIPLET_PRIMARY}-ld ")

	set_property(TARGET ${target} APPEND_STRING PROPERTY
		LINK_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ld64/src/ \
-B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ \
-target ${APPLE_TARGET_TRIPLET_PRIMARY} -Wl,-Z \
-Wl,-sdk_version,${CMAKE_OSX_DEPLOYMENT_TARGET} \
-Wl,-dylib_file,/usr/lib/system/libsystem_c.dylib:${CMAKE_BINARY_DIR}/src/external/libc/libsystem_c_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_darwin.dylib:${CMAKE_BINARY_DIR}/src/external/libc/libdarwin/libsystem_darwin.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_kernel.dylib:${CMAKE_BINARY_DIR}/src/kernel/libsystem_kernel_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libunwind.dylib:${CMAKE_BINARY_DIR}/src/libunwind/libunwind_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_m.dylib:${CMAKE_BINARY_DIR}/src/libm/libsystem_m_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libcompiler_rt.dylib:${CMAKE_BINARY_DIR}/src/external/compiler-rt/lib/builtins/libcompiler_rt_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libdyld.dylib:${CMAKE_BINARY_DIR}/src/external/dyld/libsystem_dyld_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_info.dylib:${CMAKE_BINARY_DIR}/src/external/Libinfo/libsystem_info_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_blocks.dylib:${CMAKE_BINARY_DIR}/src/external/libclosure/libsystem_blocks_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/libc++abi.dylib:${CMAKE_BINARY_DIR}/src/external/libcxxabi/libc++abi.dylib \
-Wl,-dylib_file,/usr/lib/system/liblaunch.dylib:${CMAKE_BINARY_DIR}/src/launchd/liblaunch/liblaunch_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libkeymgr.dylib:${CMAKE_BINARY_DIR}/src/keymgr/libkeymgr_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libquarantine.dylib:${CMAKE_BINARY_DIR}/src/quarantine/libquarantine.dylib \
-Wl,-dylib_file,/usr/lib/system/libremovefile.dylib:${CMAKE_BINARY_DIR}/src/libremovefile/libremovefile.dylib \
-Wl,-dylib_file,/usr/lib/system/libcopyfile.dylib:${CMAKE_BINARY_DIR}/src/copyfile/libcopyfile.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_coreservices.dylib:${CMAKE_BINARY_DIR}/src/libsystem_coreservices/libsystem_coreservices.dylib \
-Wl,-dylib_file,/usr/lib/system/libsystem_notify.dylib:${CMAKE_BINARY_DIR}/src/external/libnotify/libsystem_notify_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libdispatch.dylib:${CMAKE_BINARY_DIR}/src/external/libdispatch/liblibdispatch_shared_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/system/libmacho.dylib:${CMAKE_BINARY_DIR}/src/external/cctools/libmacho/libmacho_firstpass.dylib \
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
-Wl,-dylib_file,/usr/lib/system/libsystem_malloc.dylib:${CMAKE_BINARY_DIR}/src/external/libmalloc/libsystem_malloc_firstpass.dylib \
-Wl,-dylib_file,/usr/lib/libobjc.A.dylib:${CMAKE_BINARY_DIR}/src/external/objc4/runtime/libobjc.A.dylib \
-Wl,-dylib_file,/usr/lib/libicucore.A.dylib:${CMAKE_BINARY_DIR}/src/external/icu/icuSources/libicucore.A.dylib \
-Wl,-dylib_file,/usr/lib/libncurses.5.4.dylib:${CMAKE_BINARY_DIR}/src/ncurses/ncurses/libncurses.5.4.dylib \
-Wl,-dylib_file,/usr/lib/libDiagnosticMessagesClient.dylib:${CMAKE_BINARY_DIR}/src/libDiagnosticMessagesClient/libDiagnosticMessagesClient.dylib \
-Wl,-dylib_file,/usr/lib/libSystem.B.dylib:${CMAKE_BINARY_DIR}/src/external/libsystem/libSystem.B.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Python.framework/Versions/2.7/Python:${CMAKE_BINARY_DIR}/src/external/python/2.7/Python-2.7.16/Python \
-Wl,-dylib_file,/System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation:${CMAKE_BINARY_DIR}/src/external/corefoundation/CoreFoundation \
-Wl,-dylib_file,/System/Library/Frameworks/Ruby.framework/Versions/2.0/Ruby:${CMAKE_BINARY_DIR}/src/external/ruby/Ruby \
-Wl,-dylib_file,/usr/lib/libiconv.2.dylib:${CMAKE_BINARY_DIR}/src/libiconv/libiconv.2.dylib \
-Wl,-dylib_file,/usr/lib/libcharset.1.dylib:${CMAKE_BINARY_DIR}/src/libiconv/libcharset.1.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CFNetwork.framework/Versions/A/CFNetwork:${CMAKE_BINARY_DIR}/src/external/cfnetwork/src/CFNetwork \
-Wl,-dylib_file,/usr/lib/libxml2.2.dylib:${CMAKE_BINARY_DIR}/src/external/libxml2/libxml2.2.dylib \
-Wl,-dylib_file,/usr/lib/libpam.2.dylib:${CMAKE_BINARY_DIR}/src/external/openpam/libpam.2.dylib \
-Wl,-dylib_file,/usr/lib/libbsm.0.dylib:${CMAKE_BINARY_DIR}/src/external/bsm/libbsm.0.dylib \
-Wl,-dylib_file,/usr/lib/libauto.dylib:${CMAKE_BINARY_DIR}/src/external/libauto/libauto.dylib \
-Wl,-dylib_file,/usr/lib/libxar.1.dylib:${CMAKE_BINARY_DIR}/src/xar/libxar.1.dylib \
-Wl,-dylib_file,/usr/lib/libsqlite3.0.dylib:${CMAKE_BINARY_DIR}/src/external/sqlite/libsqlite3.0.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CryptoTokenKit.framework/Versions/A/CryptoTokenKit:${CMAKE_BINARY_DIR}/src/frameworks/CryptoTokenKit/CryptoTokenKit \
-Wl,-dylib_file,/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit:${CMAKE_BINARY_DIR}/src/external/IOKitUser/IOKit \
-Wl,-dylib_file,/System/Library/Frameworks/SystemConfiguration.framework/Versions/A/SystemConfiguration:${CMAKE_BINARY_DIR}/src/frameworks/SystemConfiguration/SystemConfiguration	\
-Wl,-dylib_file,/usr/lib/libbz2.1.0.dylib:${CMAKE_BINARY_DIR}/src/external/bzip2/libbz2.1.0.dylib \
-Wl,-dylib_file,/usr/lib/libssl.0.9.8.dylib:${CMAKE_BINARY_DIR}/src/external/openssl/src/libssl.0.9.8.dylib \
-Wl,-dylib_file,/usr/lib/libcrypto.0.9.8.dylib:${CMAKE_BINARY_DIR}/src/external/openssl/src/libcrypto.0.9.8.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:${CMAKE_BINARY_DIR}/src/native/libGL.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGLU.dylib:${CMAKE_BINARY_DIR}/src/native/libGLU.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CoreImage.framework/Versions/A/CoreImage:${CMAKE_BINARY_DIR}/src/frameworks/CoreImage/CoreImage \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/FSEvents.framework/Versions/A/FSEvents:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/FSEvents/FSEvents \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/LaunchServices:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/LaunchServices/LaunchServices \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/CarbonCore:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/CarbonCore/CarbonCore \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/AE.framework/Versions/A/AE:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/AE/AE \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/OSServices.framework/Versions/A/OSServices:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/OSServices/OSServices \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/DictionaryServices.framework/Versions/A/DictionaryServices:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/DictionaryServices/DictionaryServices \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/Metadata.framework/Versions/A/Metadata:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/Metadata/Metadata \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/SearchKit.framework/Versions/A/SearchKit:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/SearchKit/SearchKit \
-Wl,-dylib_file,/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/SharedFileList.framework/Versions/A/SharedFileList:${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/src/SharedFileList/SharedFileList \
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
-Wl,-dylib_file,/System/Library/Frameworks/ImageIO.framework/Versions/A/ImageIO:${CMAKE_BINARY_DIR}/src/frameworks/ImageIO/ImageIO \
-Wl,-dylib_file,/System/Library/Frameworks/LocalAuthentication.framework/Versions/A/LocalAuthentication:${CMAKE_BINARY_DIR}/src/frameworks/LocalAuthentication/LocalAuthentication \
-Wl,-dylib_file,/System/Library/Frameworks/CoreVideo.framework/Versions/A/CoreVideo:${CMAKE_BINARY_DIR}/src/frameworks/CoreVideo/CoreVideo \
-Wl,-dylib_file,/System/Library/PrivateFrameworks/FMDB.framework/Versions/A/FMDB:${CMAKE_BINARY_DIR}/src/external/fmdb/FMDB \
-Wl,-dylib_file,/usr/lib/system/libkxld.dylib:${CMAKE_BINARY_DIR}/src/external/lkm/libkern/kxld/libkxld.dylib \
-Wl,-dylib_file,/usr/lib/libcrypto.44.dylib:${CMAKE_BINARY_DIR}/src/external/libressl-2.8.3/libcrypto.44.dylib \
-Wl,-dylib_file,/usr/lib/libssl.46.dylib:${CMAKE_BINARY_DIR}/src/external/libressl-2.8.3/libssl.46.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Security.framework/Versions/A/Security:${CMAKE_BINARY_DIR}/src/external/security/OSX/Security \
-Wl,-dylib_file,/System/Library/Frameworks/Foundation.framework/Versions/C/Foundation:${CMAKE_BINARY_DIR}/src/external/foundation/Foundation \
-Wl,-dylib_file,/usr/lib/libz.1.dylib:${CMAKE_BINARY_DIR}/src/external/zlib/libz.1.dylib \
-Wl,-dylib_file,/usr/lib/libcoretls_cfhelpers.dylib:${CMAKE_BINARY_DIR}/src/external/coretls/libcoretls_cfhelpers.dylib \
-Wl,-dylib_file,/System/Library/PrivateFrameworks/AppleFSCompression.framework/Versions/A/AppleFSCompression:${CMAKE_BINARY_DIR}/src/private-frameworks/AppleFSCompression/AppleFSCompression \
-Wl,-dylib_file,/usr/lib/libcoretls.dylib:${CMAKE_BINARY_DIR}/src/external/coretls/libcoretls.dylib \
-Wl,-dylib_file,/usr/lib/libenergytrace.dylib:${CMAKE_BINARY_DIR}/src/external/energytrace/libenergytrace.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/CFOpenDirectory.framework/Versions/A/CFOpenDirectory:${CMAKE_BINARY_DIR}/src/frameworks/OpenDirectory/CFOpenDirectory/CFOpenDirectory \
-Wl,-dylib_file,/usr/lib/libnetwork.dylib:${CMAKE_BINARY_DIR}/src/external/libnetwork/libnetwork.dylib \
-Wl,-dylib_file,/usr/lib/system/libcache.dylib:${CMAKE_BINARY_DIR}/src/libcache/libcache.dylib \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/CommonPanels.framework/Versions/A/CommonPanels:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/CommonPanels/CommonPanels \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/Help.framework/Versions/A/Help:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/Help/Help \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/HIToolbox:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/HIToolbox/HIToolbox \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/ImageCapture.framework/Versions/A/ImageCapture:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/ImageCapture/ImageCapture \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/Ink.framework/Versions/A/Ink:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/Ink/Ink \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/OpenScripting.framework/Versions/A/OpenScripting:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/OpenScripting/OpenScripting \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/Print.framework/Versions/A/Print:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/Print/Print \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/SecurityHI.framework/Versions/A/SecurityHI:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/SecurityHI/SecurityHI \
-Wl,-dylib_file,/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/SpeechRecognition.framework/Versions/A/SpeechRecognition:${CMAKE_BINARY_DIR}/src/frameworks/Carbon/SpeechRecognition/SpeechRecognition \
-Wl,-dylib_file,/System/Library/Frameworks/CoreData.framework/Versions/A/CoreData:${CMAKE_BINARY_DIR}/src/external/cocotron/CoreData/CoreData \
-Wl,-dylib_file,/System/Library/Frameworks/QuartzCore.framework/Versions/A/QuartzCore:${CMAKE_BINARY_DIR}/src/external/cocotron/QuartzCore/QuartzCore \
-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL:${CMAKE_BINARY_DIR}/src/frameworks/OpenGL/OpenGL \
-Wl,-dylib_file,/System/Library/PrivateFrameworks/Onyx2D.framework/Versions/A/Onyx2D:${CMAKE_BINARY_DIR}/src/external/cocotron/Onyx2D/Onyx2D \
-Wl,-dylib_file,/usr/lib/darling/libelfloader.dylib:${CMAKE_BINARY_DIR}/src/libelfloader/libelfloader.dylib \
-Wl,-dylib_file,/usr/lib/native/libavcodec.dylib:${CMAKE_BINARY_DIR}/src/native/libavcodec.dylib \
-Wl,-dylib_file,/usr/lib/native/libavformat.dylib:${CMAKE_BINARY_DIR}/src/native/libavformat.dylib \
-Wl,-dylib_file,/usr/lib/native/libswresample.dylib:${CMAKE_BINARY_DIR}/src/native/libswresample.dylib \
-Wl,-dylib_file,/usr/lib/native/libavutil.dylib:${CMAKE_BINARY_DIR}/src/native/libavutil.dylib \
-Wl,-dylib_file,/usr/lib/native/libcairo.dylib:${CMAKE_BINARY_DIR}/src/native/libcairo.dylib \
-Wl,-dylib_file,/usr/lib/native/libdbus.dylib:${CMAKE_BINARY_DIR}/src/native/libdbus.dylib \
-Wl,-dylib_file,/usr/lib/native/libEGL.dylib:${CMAKE_BINARY_DIR}/src/native/libEGL.dylib \
-Wl,-dylib_file,/usr/lib/native/libfontconfig.dylib:${CMAKE_BINARY_DIR}/src/native/libfontconfig.dylib \
-Wl,-dylib_file,/usr/lib/native/libFreeType.dylib:${CMAKE_BINARY_DIR}/src/native/libFreeType.dylib \
-Wl,-dylib_file,/usr/lib/native/libfuse.dylib:${CMAKE_BINARY_DIR}/src/native/libfuse.dylib \
-Wl,-dylib_file,/usr/lib/native/libgif.dylib:${CMAKE_BINARY_DIR}/src/native/libgif.dylib \
-Wl,-dylib_file,/usr/lib/native/libjpeg.dylib:${CMAKE_BINARY_DIR}/src/native/libjpeg.dylib \
-Wl,-dylib_file,/usr/lib/native/libpng.dylib:${CMAKE_BINARY_DIR}/src/native/libpng.dylib \
-Wl,-dylib_file,/usr/lib/native/libpulse.dylib:${CMAKE_BINARY_DIR}/src/native/libpulse.dylib \
-Wl,-dylib_file,/usr/lib/native/libtiff.dylib:${CMAKE_BINARY_DIR}/src/native/libtiff.dylib \
-Wl,-dylib_file,/usr/lib/native/libX11.dylib:${CMAKE_BINARY_DIR}/src/native/libX11.dylib \
-Wl,-dylib_file,/usr/lib/native/libXcursor.dylib:${CMAKE_BINARY_DIR}/src/native/libXcursor.dylib \
-Wl,-dylib_file,/usr/lib/native/libXext.dylib:${CMAKE_BINARY_DIR}/src/native/libXext.dylib \
-Wl,-dylib_file,/usr/lib/native/libxkbfile.dylib:${CMAKE_BINARY_DIR}/src/native/libxkbfile.dylib \
-Wl,-dylib_file,/usr/lib/native/libXRandR.dylib:${CMAKE_BINARY_DIR}/src/native/libXRandR.dylib")

	add_dependencies(${target} ${APPLE_TARGET_TRIPLET_PRIMARY}-ld)

ENDFUNCTION(use_ld64)

function(reexport reexporter reexportee reexportee_output)
	add_dependencies(${reexporter} ${reexportee})
	set_property(TARGET ${reexporter} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-reexport_library,${reexportee_output} ")
endfunction(reexport)
