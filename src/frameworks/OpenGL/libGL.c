// Copyright (C) 2026 Lubos Dolezel
//
// Libraries/libGL.dylib: what an application that opens OpenGL.framework's libGL.dylib by path and
// looks GL functions up there gets. On macOS that library holds the GL entry points that
// OpenGL.framework re-exports; here the entry points that follow the thread's context binding
// are in the framework, so this library only re-exports the framework (see CMakeLists.txt) and
// has no code of its own.
