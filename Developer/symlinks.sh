SDK="`pwd`/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
FRAMEWORKS="$SDK/System/Library/Frameworks"
LIBS="$SDK/usr/lib"

ln -sf "$BUILD/src/external/foundation/Foundation" "$FRAMEWORKS/Foundation.framework/Versions/C"
ln -sf "$BUILD/src/external/objc4/runtime/libobjc.A.dylib" "$LIBS"
ln -sf "$BUILD/src/libsystem/libSystem.B.dylib" "$LIBS"
