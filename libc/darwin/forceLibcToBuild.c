//  XCode will not build a library unless it contains at least one module.
//  Absent this requirement, libm.dylib would be composed entirely by linking
//  the component static libraries together, but to satisfy it, we must have
//  a C file.

typedef int theCStandardDoesNotAllowAnEmptyModule;
