#!/bin/sh
# Builds the dlsym test (arm64 Darling SDK and ld64 on PATH, or pass CC/SDK).
# Run in the guest: ./arm64e-dlsym-run ./arm64e-dlsym-main (from this directory).
set -eu
cc=${CC:-clang}
sdk=${SDK:?set SDK to the arm64 Darling SDK}
c() { "$cc" -isysroot "$sdk" -mmacosx-version-min=11.0 "$@"; }
c -arch arm64 -arch arm64e -dynamiclib -install_name @rpath/libdlsymslices.dylib -o libdlsymslices.dylib arm64e-dlsym-lib.c
c -arch arm64 -arch arm64e -dynamiclib -install_name @rpath/libdlsymdl.dylib -o libdlsymdl.dylib arm64e-dlsym-lib.c
c -arch arm64 -dynamiclib -install_name @rpath/libdlsymplain.dylib -o libdlsymplain.dylib arm64e-dlsym-plain.c
c -arch arm64e -o arm64e-dlsym-main arm64e-dlsym-main.c -L. -ldlsymslices -Wl,-rpath,@executable_path
c -arch arm64 -o arm64e-dlsym-run arm64e-dlsym-run.c
