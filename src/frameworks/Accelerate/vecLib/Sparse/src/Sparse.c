/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <Sparse/Sparse.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* _SparseCGIterate_Double(void)
{
    if (verbose) puts("STUB: _SparseCGIterate_Double called");
    return NULL;
}

void* _SparseCGIterate_Float(void)
{
    if (verbose) puts("STUB: _SparseCGIterate_Float called");
    return NULL;
}

void* _SparseCGSolve_Double(void)
{
    if (verbose) puts("STUB: _SparseCGSolve_Double called");
    return NULL;
}

void* _SparseCGSolve_Float(void)
{
    if (verbose) puts("STUB: _SparseCGSolve_Float called");
    return NULL;
}

void* _SparseConvertFromCoordinate_Double(void)
{
    if (verbose) puts("STUB: _SparseConvertFromCoordinate_Double called");
    return NULL;
}

void* _SparseConvertFromCoordinate_Float(void)
{
    if (verbose) puts("STUB: _SparseConvertFromCoordinate_Float called");
    return NULL;
}

void* _SparseConvertFromOpaque_Double(void)
{
    if (verbose) puts("STUB: _SparseConvertFromOpaque_Double called");
    return NULL;
}

void* _SparseConvertFromOpaque_Float(void)
{
    if (verbose) puts("STUB: _SparseConvertFromOpaque_Float called");
    return NULL;
}

void* _SparseCreatePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _SparseCreatePreconditioner_Double called");
    return NULL;
}

void* _SparseCreatePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _SparseCreatePreconditioner_Float called");
    return NULL;
}

void* _SparseDestroyOpaqueNumeric_Double(void)
{
    if (verbose) puts("STUB: _SparseDestroyOpaqueNumeric_Double called");
    return NULL;
}

void* _SparseDestroyOpaqueNumeric_Float(void)
{
    if (verbose) puts("STUB: _SparseDestroyOpaqueNumeric_Float called");
    return NULL;
}

void* _SparseDestroyOpaqueSymbolic(void)
{
    if (verbose) puts("STUB: _SparseDestroyOpaqueSymbolic called");
    return NULL;
}

void* _SparseFactorQR_Double(void)
{
    if (verbose) puts("STUB: _SparseFactorQR_Double called");
    return NULL;
}

void* _SparseFactorQR_Float(void)
{
    if (verbose) puts("STUB: _SparseFactorQR_Float called");
    return NULL;
}

void* _SparseFactorSymmetric_Double(void)
{
    if (verbose) puts("STUB: _SparseFactorSymmetric_Double called");
    return NULL;
}

void* _SparseFactorSymmetric_Float(void)
{
    if (verbose) puts("STUB: _SparseFactorSymmetric_Float called");
    return NULL;
}

void* _SparseGMRESIterate_Double(void)
{
    if (verbose) puts("STUB: _SparseGMRESIterate_Double called");
    return NULL;
}

void* _SparseGMRESIterate_Float(void)
{
    if (verbose) puts("STUB: _SparseGMRESIterate_Float called");
    return NULL;
}

void* _SparseGMRESSolve_Double(void)
{
    if (verbose) puts("STUB: _SparseGMRESSolve_Double called");
    return NULL;
}

void* _SparseGMRESSolve_Float(void)
{
    if (verbose) puts("STUB: _SparseGMRESSolve_Float called");
    return NULL;
}

void* _SparseGetIterativeStateSize_Double(void)
{
    if (verbose) puts("STUB: _SparseGetIterativeStateSize_Double called");
    return NULL;
}

void* _SparseGetIterativeStateSize_Float(void)
{
    if (verbose) puts("STUB: _SparseGetIterativeStateSize_Float called");
    return NULL;
}

void* _SparseGetOptionsFromNumericFactor_Double(void)
{
    if (verbose) puts("STUB: _SparseGetOptionsFromNumericFactor_Double called");
    return NULL;
}

void* _SparseGetOptionsFromNumericFactor_Float(void)
{
    if (verbose) puts("STUB: _SparseGetOptionsFromNumericFactor_Float called");
    return NULL;
}

void* _SparseGetOptionsFromSymbolicFactor(void)
{
    if (verbose) puts("STUB: _SparseGetOptionsFromSymbolicFactor called");
    return NULL;
}

void* _SparseGetWorkspaceRequired_Double(void)
{
    if (verbose) puts("STUB: _SparseGetWorkspaceRequired_Double called");
    return NULL;
}

void* _SparseGetWorkspaceRequired_Float(void)
{
    if (verbose) puts("STUB: _SparseGetWorkspaceRequired_Float called");
    return NULL;
}

void* _SparseLSMRIterate_Double(void)
{
    if (verbose) puts("STUB: _SparseLSMRIterate_Double called");
    return NULL;
}

void* _SparseLSMRIterate_Float(void)
{
    if (verbose) puts("STUB: _SparseLSMRIterate_Float called");
    return NULL;
}

void* _SparseLSMRSolve_Double(void)
{
    if (verbose) puts("STUB: _SparseLSMRSolve_Double called");
    return NULL;
}

void* _SparseLSMRSolve_Float(void)
{
    if (verbose) puts("STUB: _SparseLSMRSolve_Float called");
    return NULL;
}

void* _SparseMultiplySubfactor_Double(void)
{
    if (verbose) puts("STUB: _SparseMultiplySubfactor_Double called");
    return NULL;
}

void* _SparseMultiplySubfactor_Float(void)
{
    if (verbose) puts("STUB: _SparseMultiplySubfactor_Float called");
    return NULL;
}

void* _SparseNumericFactorQR_Double(void)
{
    if (verbose) puts("STUB: _SparseNumericFactorQR_Double called");
    return NULL;
}

void* _SparseNumericFactorQR_Float(void)
{
    if (verbose) puts("STUB: _SparseNumericFactorQR_Float called");
    return NULL;
}

void* _SparseNumericFactorSymmetric_Double(void)
{
    if (verbose) puts("STUB: _SparseNumericFactorSymmetric_Double called");
    return NULL;
}

void* _SparseNumericFactorSymmetric_Float(void)
{
    if (verbose) puts("STUB: _SparseNumericFactorSymmetric_Float called");
    return NULL;
}

void* _SparseRefactorQR_Double(void)
{
    if (verbose) puts("STUB: _SparseRefactorQR_Double called");
    return NULL;
}

void* _SparseRefactorQR_Float(void)
{
    if (verbose) puts("STUB: _SparseRefactorQR_Float called");
    return NULL;
}

void* _SparseRefactorSymmetric_Double(void)
{
    if (verbose) puts("STUB: _SparseRefactorSymmetric_Double called");
    return NULL;
}

void* _SparseRefactorSymmetric_Float(void)
{
    if (verbose) puts("STUB: _SparseRefactorSymmetric_Float called");
    return NULL;
}

void* _SparseReleaseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _SparseReleaseOpaquePreconditioner_Double called");
    return NULL;
}

void* _SparseReleaseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _SparseReleaseOpaquePreconditioner_Float called");
    return NULL;
}

void* _SparseRetainNumeric_Double(void)
{
    if (verbose) puts("STUB: _SparseRetainNumeric_Double called");
    return NULL;
}

void* _SparseRetainNumeric_Float(void)
{
    if (verbose) puts("STUB: _SparseRetainNumeric_Float called");
    return NULL;
}

void* _SparseRetainSymbolic(void)
{
    if (verbose) puts("STUB: _SparseRetainSymbolic called");
    return NULL;
}

void* _SparseSolveOpaque_Double(void)
{
    if (verbose) puts("STUB: _SparseSolveOpaque_Double called");
    return NULL;
}

void* _SparseSolveOpaque_Float(void)
{
    if (verbose) puts("STUB: _SparseSolveOpaque_Float called");
    return NULL;
}

void* _SparseSolveSubfactor_Double(void)
{
    if (verbose) puts("STUB: _SparseSolveSubfactor_Double called");
    return NULL;
}

void* _SparseSolveSubfactor_Float(void)
{
    if (verbose) puts("STUB: _SparseSolveSubfactor_Float called");
    return NULL;
}

void* _SparseSpMV_Double(void)
{
    if (verbose) puts("STUB: _SparseSpMV_Double called");
    return NULL;
}

void* _SparseSpMV_Float(void)
{
    if (verbose) puts("STUB: _SparseSpMV_Float called");
    return NULL;
}

void* _SparseSymbolicFactorQR(void)
{
    if (verbose) puts("STUB: _SparseSymbolicFactorQR called");
    return NULL;
}

void* _SparseSymbolicFactorSymmetric(void)
{
    if (verbose) puts("STUB: _SparseSymbolicFactorSymmetric called");
    return NULL;
}

void* _SparseTrap(void)
{
    if (verbose) puts("STUB: _SparseTrap called");
    return NULL;
}

void* _Z10SparseLSMR17SparseLSMROptions(void)
{
    if (verbose) puts("STUB: _Z10SparseLSMR17SparseLSMROptions called");
    return NULL;
}

void* _Z10SparseLSMRv(void)
{
    if (verbose) puts("STUB: _Z10SparseLSMRv called");
    return NULL;
}

void* _Z11SparseGMRES18SparseGMRESOptions(void)
{
    if (verbose) puts("STUB: _Z11SparseGMRES18SparseGMRESOptions called");
    return NULL;
}

void* _Z11SparseGMRESv(void)
{
    if (verbose) puts("STUB: _Z11SparseGMRESv called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseMatrix_FloatS1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseMatrix_FloatS1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseMatrix_FloatS1_32SparseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseMatrix_FloatS1_32SparseOpaquePreconditioner_Float called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseMatrix_FloatS1_i(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseMatrix_FloatS1_i called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseVector_FloatS1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseVector_FloatS1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseVector_FloatS1_32SparseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseVector_FloatS1_32SparseOpaquePreconditioner_Float called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseVector_FloatS1_i(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod18SparseMatrix_Float17DenseVector_FloatS1_i called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseMatrix_DoubleS1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseMatrix_DoubleS1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseMatrix_DoubleS1_33SparseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseMatrix_DoubleS1_33SparseOpaquePreconditioner_Double called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseMatrix_DoubleS1_i(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseMatrix_DoubleS1_i called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseVector_DoubleS1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseVector_DoubleS1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseVector_DoubleS1_33SparseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseVector_DoubleS1_33SparseOpaquePreconditioner_Double called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseVector_DoubleS1_i(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethod19SparseMatrix_Double18DenseVector_DoubleS1_i called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS1_ES1_S1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS1_ES1_S1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS1_ES1_S1_32SparseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS1_ES1_S1_32SparseOpaquePreconditioner_Float called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseVector_FloatS1_ES1_S1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseVector_FloatS1_ES1_S1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseVector_FloatS1_ES1_S1_32SparseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE17DenseVector_FloatS1_ES1_S1_32SparseOpaquePreconditioner_Float called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS1_ES1_S1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS1_ES1_S1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS1_ES1_S1_33SparseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS1_ES1_S1_33SparseOpaquePreconditioner_Double called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseVector_DoubleS1_ES1_S1_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseVector_DoubleS1_ES1_S1_ called");
    return NULL;
}

void* _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseVector_DoubleS1_ES1_S1_33SparseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve21SparseIterativeMethodU13block_pointerFvb15CBLAS_TRANSPOSE18DenseVector_DoubleS1_ES1_S1_33SparseOpaquePreconditioner_Double called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_Float called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_FloatPv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_FloatPv called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_Float called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_FloatPv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_FloatPv called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_Double called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_DoublePv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_DoublePv called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_Double called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_DoublePv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_DoublePv called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_Float called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_FloatPv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_FloatPv called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_FloatS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseMatrix_FloatS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_Float(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_Float called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_FloatPv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_FloatPv called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_FloatS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve31SparseOpaqueFactorization_Float17DenseVector_FloatS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_Double called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_DoublePv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_DoublePv called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_DoubleS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseMatrix_DoubleS0_Pv called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_Double(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_Double called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_DoublePv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_DoublePv called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_DoubleS0_Pv(void)
{
    if (verbose) puts("STUB: _Z11SparseSolve32SparseOpaqueFactorization_Double18DenseVector_DoubleS0_Pv called");
    return NULL;
}

void* _Z12SparseFactor33SparseOpaqueSymbolicFactorization18SparseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z12SparseFactor33SparseOpaqueSymbolicFactorization18SparseMatrix_Float called");
    return NULL;
}

void* _Z12SparseFactor33SparseOpaqueSymbolicFactorization18SparseMatrix_Float26SparseNumericFactorOptions(void)
{
    if (verbose) puts("STUB: _Z12SparseFactor33SparseOpaqueSymbolicFactorization18SparseMatrix_Float26SparseNumericFactorOptions called");
    return NULL;
}

void* _Z12SparseFactor33SparseOpaqueSymbolicFactorization18SparseMatrix_Float26SparseNumericFactorOptionsPvS2_(void)
{
    if (verbose) puts("STUB: _Z12SparseFactor33SparseOpaqueSymbolicFactorization18SparseMatrix_Float26SparseNumericFactorOptionsPvS2_ called");
    return NULL;
}

void* _Z12SparseFactor33SparseOpaqueSymbolicFactorization19SparseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z12SparseFactor33SparseOpaqueSymbolicFactorization19SparseMatrix_Double called");
    return NULL;
}

void* _Z12SparseFactor33SparseOpaqueSymbolicFactorization19SparseMatrix_Double26SparseNumericFactorOptions(void)
{
    if (verbose) puts("STUB: _Z12SparseFactor33SparseOpaqueSymbolicFactorization19SparseMatrix_Double26SparseNumericFactorOptions called");
    return NULL;
}

void* _Z12SparseFactor33SparseOpaqueSymbolicFactorization19SparseMatrix_Double26SparseNumericFactorOptionsPvS2_(void)
{
    if (verbose) puts("STUB: _Z12SparseFactor33SparseOpaqueSymbolicFactorization19SparseMatrix_Double26SparseNumericFactorOptionsPvS2_ called");
    return NULL;
}

void* _Z12SparseFactorh18SparseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z12SparseFactorh18SparseMatrix_Float called");
    return NULL;
}

void* _Z12SparseFactorh18SparseMatrix_Float27SparseSymbolicFactorOptions26SparseNumericFactorOptions(void)
{
    if (verbose) puts("STUB: _Z12SparseFactorh18SparseMatrix_Float27SparseSymbolicFactorOptions26SparseNumericFactorOptions called");
    return NULL;
}

void* _Z12SparseFactorh19SparseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z12SparseFactorh19SparseMatrix_Double called");
    return NULL;
}

void* _Z12SparseFactorh19SparseMatrix_Double27SparseSymbolicFactorOptions26SparseNumericFactorOptions(void)
{
    if (verbose) puts("STUB: _Z12SparseFactorh19SparseMatrix_Double27SparseSymbolicFactorOptions26SparseNumericFactorOptions called");
    return NULL;
}

void* _Z12SparseFactorh21SparseMatrixStructure(void)
{
    if (verbose) puts("STUB: _Z12SparseFactorh21SparseMatrixStructure called");
    return NULL;
}

void* _Z12SparseFactorh21SparseMatrixStructure27SparseSymbolicFactorOptions(void)
{
    if (verbose) puts("STUB: _Z12SparseFactorh21SparseMatrixStructure27SparseSymbolicFactorOptions called");
    return NULL;
}

void* _Z12SparseRetain27SparseOpaqueSubfactor_Float(void)
{
    if (verbose) puts("STUB: _Z12SparseRetain27SparseOpaqueSubfactor_Float called");
    return NULL;
}

void* _Z12SparseRetain28SparseOpaqueSubfactor_Double(void)
{
    if (verbose) puts("STUB: _Z12SparseRetain28SparseOpaqueSubfactor_Double called");
    return NULL;
}

void* _Z12SparseRetain31SparseOpaqueFactorization_Float(void)
{
    if (verbose) puts("STUB: _Z12SparseRetain31SparseOpaqueFactorization_Float called");
    return NULL;
}

void* _Z12SparseRetain32SparseOpaqueFactorization_Double(void)
{
    if (verbose) puts("STUB: _Z12SparseRetain32SparseOpaqueFactorization_Double called");
    return NULL;
}

void* _Z12SparseRetain33SparseOpaqueSymbolicFactorization(void)
{
    if (verbose) puts("STUB: _Z12SparseRetain33SparseOpaqueSymbolicFactorization called");
    return NULL;
}

void* _Z13SparseCleanup18SparseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup18SparseMatrix_Float called");
    return NULL;
}

void* _Z13SparseCleanup19SparseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup19SparseMatrix_Double called");
    return NULL;
}

void* _Z13SparseCleanup27SparseOpaqueSubfactor_Float(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup27SparseOpaqueSubfactor_Float called");
    return NULL;
}

void* _Z13SparseCleanup28SparseOpaqueSubfactor_Double(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup28SparseOpaqueSubfactor_Double called");
    return NULL;
}

void* _Z13SparseCleanup31SparseOpaqueFactorization_Float(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup31SparseOpaqueFactorization_Float called");
    return NULL;
}

void* _Z13SparseCleanup32SparseOpaqueFactorization_Double(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup32SparseOpaqueFactorization_Double called");
    return NULL;
}

void* _Z13SparseCleanup32SparseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup32SparseOpaquePreconditioner_Float called");
    return NULL;
}

void* _Z13SparseCleanup33SparseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup33SparseOpaquePreconditioner_Double called");
    return NULL;
}

void* _Z13SparseCleanup33SparseOpaqueSymbolicFactorization(void)
{
    if (verbose) puts("STUB: _Z13SparseCleanup33SparseOpaqueSymbolicFactorization called");
    return NULL;
}

void* _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS4_ES4_S4_S4_(void)
{
    if (verbose) puts("STUB: _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS4_ES4_S4_S4_ called");
    return NULL;
}

void* _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS4_ES4_S4_S4_32SparseOpaquePreconditioner_Float(void)
{
    if (verbose) puts("STUB: _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE17DenseMatrix_FloatS4_ES4_S4_S4_32SparseOpaquePreconditioner_Float called");
    return NULL;
}

void* _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS4_ES4_S4_S4_(void)
{
    if (verbose) puts("STUB: _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS4_ES4_S4_S4_ called");
    return NULL;
}

void* _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS4_ES4_S4_S4_33SparseOpaquePreconditioner_Double(void)
{
    if (verbose) puts("STUB: _Z13SparseIterate21SparseIterativeMethodiPKbPvU13block_pointerFvb15CBLAS_TRANSPOSE18DenseMatrix_DoubleS4_ES4_S4_S4_33SparseOpaquePreconditioner_Double called");
    return NULL;
}

void* _Z14SparseMultiply18SparseMatrix_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply18SparseMatrix_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z14SparseMultiply18SparseMatrix_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply18SparseMatrix_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z14SparseMultiply19SparseMatrix_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply19SparseMatrix_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z14SparseMultiply19SparseMatrix_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply19SparseMatrix_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_Float called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_FloatPv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_FloatPv called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_Pv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseMatrix_FloatS0_Pv called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_Float(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_Float called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_FloatPv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_FloatPv called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_Pv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply27SparseOpaqueSubfactor_Float17DenseVector_FloatS0_Pv called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_Double called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_DoublePv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_DoublePv called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_Pv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseMatrix_DoubleS0_Pv called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_Double(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_Double called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_DoublePv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_DoublePv called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_Pv(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiply28SparseOpaqueSubfactor_Double18DenseVector_DoubleS0_Pv called");
    return NULL;
}

void* _Z14SparseMultiplyd19SparseMatrix_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiplyd19SparseMatrix_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z14SparseMultiplyd19SparseMatrix_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiplyd19SparseMatrix_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z14SparseMultiplyf18SparseMatrix_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiplyf18SparseMatrix_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z14SparseMultiplyf18SparseMatrix_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z14SparseMultiplyf18SparseMatrix_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_Float(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_Float called");
    return NULL;
}

void* _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_Float26SparseNumericFactorOptions(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_Float26SparseNumericFactorOptions called");
    return NULL;
}

void* _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_Float26SparseNumericFactorOptionsPv(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_Float26SparseNumericFactorOptionsPv called");
    return NULL;
}

void* _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_FloatPv(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor18SparseMatrix_FloatP31SparseOpaqueFactorization_FloatPv called");
    return NULL;
}

void* _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_Double(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_Double called");
    return NULL;
}

void* _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_Double26SparseNumericFactorOptions(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_Double26SparseNumericFactorOptions called");
    return NULL;
}

void* _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_Double26SparseNumericFactorOptionsPv(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_Double26SparseNumericFactorOptionsPv called");
    return NULL;
}

void* _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_DoublePv(void)
{
    if (verbose) puts("STUB: _Z14SparseRefactor19SparseMatrix_DoubleP32SparseOpaqueFactorization_DoublePv called");
    return NULL;
}

void* _Z17SparseMultiplyAdd18SparseMatrix_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAdd18SparseMatrix_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAdd18SparseMatrix_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAdd18SparseMatrix_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAdd19SparseMatrix_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAdd19SparseMatrix_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAdd19SparseMatrix_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAdd19SparseMatrix_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAddd19SparseMatrix_Double18DenseMatrix_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAddd19SparseMatrix_Double18DenseMatrix_DoubleS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAddd19SparseMatrix_Double18DenseVector_DoubleS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAddd19SparseMatrix_Double18DenseVector_DoubleS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAddf18SparseMatrix_Float17DenseMatrix_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAddf18SparseMatrix_Float17DenseMatrix_FloatS0_ called");
    return NULL;
}

void* _Z17SparseMultiplyAddf18SparseMatrix_Float17DenseVector_FloatS0_(void)
{
    if (verbose) puts("STUB: _Z17SparseMultiplyAddf18SparseMatrix_Float17DenseVector_FloatS0_ called");
    return NULL;
}

void* _Z18SparseGetTranspose18SparseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z18SparseGetTranspose18SparseMatrix_Float called");
    return NULL;
}

void* _Z18SparseGetTranspose19SparseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z18SparseGetTranspose19SparseMatrix_Double called");
    return NULL;
}

void* _Z18SparseGetTranspose27SparseOpaqueSubfactor_Float(void)
{
    if (verbose) puts("STUB: _Z18SparseGetTranspose27SparseOpaqueSubfactor_Float called");
    return NULL;
}

void* _Z18SparseGetTranspose28SparseOpaqueSubfactor_Double(void)
{
    if (verbose) puts("STUB: _Z18SparseGetTranspose28SparseOpaqueSubfactor_Double called");
    return NULL;
}

void* _Z18SparseGetTranspose31SparseOpaqueFactorization_Float(void)
{
    if (verbose) puts("STUB: _Z18SparseGetTranspose31SparseOpaqueFactorization_Float called");
    return NULL;
}

void* _Z18SparseGetTranspose32SparseOpaqueFactorization_Double(void)
{
    if (verbose) puts("STUB: _Z18SparseGetTranspose32SparseOpaqueFactorization_Double called");
    return NULL;
}

void* _Z21SparseCreateSubfactorh31SparseOpaqueFactorization_Float(void)
{
    if (verbose) puts("STUB: _Z21SparseCreateSubfactorh31SparseOpaqueFactorization_Float called");
    return NULL;
}

void* _Z21SparseCreateSubfactorh32SparseOpaqueFactorization_Double(void)
{
    if (verbose) puts("STUB: _Z21SparseCreateSubfactorh32SparseOpaqueFactorization_Double called");
    return NULL;
}

void* _Z23SparseConjugateGradient15SparseCGOptions(void)
{
    if (verbose) puts("STUB: _Z23SparseConjugateGradient15SparseCGOptions called");
    return NULL;
}

void* _Z23SparseConjugateGradientv(void)
{
    if (verbose) puts("STUB: _Z23SparseConjugateGradientv called");
    return NULL;
}

void* _Z23SparseConvertFromOpaqueP14sparse_m_float(void)
{
    if (verbose) puts("STUB: _Z23SparseConvertFromOpaqueP14sparse_m_float called");
    return NULL;
}

void* _Z23SparseConvertFromOpaqueP15sparse_m_double(void)
{
    if (verbose) puts("STUB: _Z23SparseConvertFromOpaqueP15sparse_m_double called");
    return NULL;
}

void* _Z24SparseGetStateSize_Float21SparseIterativeMethodbiii(void)
{
    if (verbose) puts("STUB: _Z24SparseGetStateSize_Float21SparseIterativeMethodbiii called");
    return NULL;
}

void* _Z25SparseGetStateSize_Double21SparseIterativeMethodbiii(void)
{
    if (verbose) puts("STUB: _Z25SparseGetStateSize_Double21SparseIterativeMethodbiii called");
    return NULL;
}

void* _Z26SparseCreatePreconditioneri18SparseMatrix_Float(void)
{
    if (verbose) puts("STUB: _Z26SparseCreatePreconditioneri18SparseMatrix_Float called");
    return NULL;
}

void* _Z26SparseCreatePreconditioneri19SparseMatrix_Double(void)
{
    if (verbose) puts("STUB: _Z26SparseCreatePreconditioneri19SparseMatrix_Double called");
    return NULL;
}

void* _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKd(void)
{
    if (verbose) puts("STUB: _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKd called");
    return NULL;
}

void* _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKdPvS4_(void)
{
    if (verbose) puts("STUB: _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKdPvS4_ called");
    return NULL;
}

void* _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKf(void)
{
    if (verbose) puts("STUB: _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKf called");
    return NULL;
}

void* _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKfPvS4_(void)
{
    if (verbose) puts("STUB: _Z27SparseConvertFromCoordinateiilh18SparseAttributes_tPKiS1_PKfPvS4_ called");
    return NULL;
}
