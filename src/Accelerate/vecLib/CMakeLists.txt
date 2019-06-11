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

reexport(vecLib vMisc ${CMAKE_CURRENT_BINARY_DIR}/vMisc/libvMisc.dylib)
reexport(vecLib vDSP ${CMAKE_CURRENT_BINARY_DIR}/vDSP/libvDSP.dylib)
reexport(vecLib BLAS ${CMAKE_CURRENT_BINARY_DIR}/BLAS/libBLAS.dylib)
reexport(vecLib LAPACK ${CMAKE_CURRENT_BINARY_DIR}/LAPACK/libLAPACK.dylib)
reexport(vecLib LinearAlgebra ${CMAKE_CURRENT_BINARY_DIR}/LinearAlgebra/libLinearAlgebra.dylib)
reexport(vecLib SparseBLAS ${CMAKE_CURRENT_BINARY_DIR}/SparseBLAS/libSparseBLAS.dylib)
reexport(vecLib Quadrature ${CMAKE_CURRENT_BINARY_DIR}/Quadrature/libQuadrature.dylib)
reexport(vecLib BNNS ${CMAKE_CURRENT_BINARY_DIR}/BNNS/libBNNS.dylib)
reexport(vecLib Sparse ${CMAKE_CURRENT_BINARY_DIR}/Sparse/libSparse.dylib)

InstallSymlink(Accelerate.framework/Versions/A/Frameworks/vecLib.framework ${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/vecLib.framework)
