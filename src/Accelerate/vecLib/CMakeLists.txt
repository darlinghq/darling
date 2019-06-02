project(vecLib)

# TODO: Move to /src/CMakeLists.txt when done
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vMisc/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vDSP/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/BLAS/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/LAPACK/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/LinearAlgebra/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SparseBLAS/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Quadrature/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/BNNS/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Sparse/include)

add_subdirectory(vMisc)
add_subdirectory(vDSP)
add_subdirectory(BLAS)
add_subdirectory(LAPACK)
add_subdirectory(LinearAlgebra)
add_subdirectory(SparseBLAS)
add_subdirectory(Quadrature)
add_subdirectory(BNNS)
add_subdirectory(Sparse)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "671.250.4")

add_framework(vecLib
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT "Accelerate"

    SOURCES
        empty.c

    DEPENDENCIES
    	vMisc
	vDSP
	BLAS
	LAPACK
	LinearAlgebra
	SparseBLAS
	Quadrature
	BNNS
	Sparse
        system
)
reexport(vecLib vMisc)
reexport(vecLib vDSP)
reexport(vecLib BLAS)
reexport(vecLib LAPACK)
reexport(vecLib LinearAlgebra)
reexport(vecLib SparseBLAS)
reexport(vecLib Quadrature)
reexport(vecLib BNNS)
reexport(vecLib Sparse)

InstallSymlink(Accelerate.framework/Versions/A/Frameworks/vecLib.framework ${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/vecLib.framework)
