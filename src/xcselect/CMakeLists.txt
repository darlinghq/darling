project(xcselect)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

add_definitions(-nostdinc)

set(DYLIB_INSTALL_NAME "/usr/lib/libxcselect.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(xcselect xcselect.c)
target_link_libraries(xcselect system CoreFoundation)

add_darling_executable(xcrun xcrun.c)
target_link_libraries(xcrun system xcselect)

add_darling_executable(xcode-select xcode-select.c)
target_link_libraries(xcode-select system xcselect)

install(TARGETS xcselect DESTINATION libexec/darling/usr/lib)
install(TARGETS xcrun xcode-select DESTINATION libexec/darling/usr/bin)
install(PROGRAMS clt_install.py DESTINATION libexec/darling/usr/libexec/darling)

function(add_shim name require_xc)
	add_darling_executable("${name}_shim" xcrun-shim.c)

	set_target_properties("${name}_shim"
		PROPERTIES
			OUTPUT_NAME "${name}"
			COMPILE_FLAGS "-DTOOL_NAME=\\\"${name}\\\" -DREQUIRE_XCODE=${require_xc}"
	)

	target_link_libraries("${name}_shim" system xcselect)
	install(TARGETS "${name}_shim" DESTINATION libexec/darling/usr/bin)
endfunction(add_shim)

function(add_shims require_xc) 
	foreach(tool ${ARGN})
		add_shim(${tool} ${require_xc})
	endforeach()
endfunction(add_shims)

# TODO: c++, cc, llvm-g++, llvm-gcc -- symlinks to clang

add_shims(0
	BuildStrings CpMac DeRez GetFileInfo MergePef MvMac
	ResMerger Rez RezDet RezWack SetFile SplitForks UnRezWack
	ar as asa bison clang clang++ cmpdylib codesign_allocate
	cpp ctags ctf_insert dsymutil dwarfdump flex flex++ g++
	gatherheaderdoc gcc gcov git git-cvsserver git-receive-pack
	git-shell git-upload-archive git-upload-pack gm4 gnumake
	gperf hdxml2manxml headerdoc2html indent install_name_tool
	ld lex libtool lipo lldb llvm-g++ llvm-gcc lorder m4 make
	mig mkdep nasm ndisasm nm nmedit objdump otool pagestuff
	ranlib rebase redo_prebinding resolveLinks rpcgen segedit
	size strings strip svn svnadmin svndumpfilter svnlook
	svnserve svnsync svnversion swift swiftc unifdef unifdefall
	xml2man yacc)

add_shims(1
	agvtool desdp genstrings ibtool ictool instruments
	iprofiler opendiff sdef sdp xcodebuild xcscontrol
	xcsdiagnose xed)

InstallSymlink(clang ${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/cc)
InstallSymlink(clang++ ${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/bin/c++)
