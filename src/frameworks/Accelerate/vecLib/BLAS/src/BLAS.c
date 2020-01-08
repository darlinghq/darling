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


#include <BLAS/BLAS.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* APL_dgemm(void)
{
    if (verbose) puts("STUB: APL_dgemm called");
    return NULL;
}

void* APL_dgemm_LU(void)
{
    if (verbose) puts("STUB: APL_dgemm_LU called");
    return NULL;
}

void* APL_dgemm_QR(void)
{
    if (verbose) puts("STUB: APL_dgemm_QR called");
    return NULL;
}

void* APL_dtrsm(void)
{
    if (verbose) puts("STUB: APL_dtrsm called");
    return NULL;
}

void* APL_sgemm(void)
{
    if (verbose) puts("STUB: APL_sgemm called");
    return NULL;
}

void* APL_sgemm_LU(void)
{
    if (verbose) puts("STUB: APL_sgemm_LU called");
    return NULL;
}

void* APL_sgemm_QR(void)
{
    if (verbose) puts("STUB: APL_sgemm_QR called");
    return NULL;
}

void* APL_strsm(void)
{
    if (verbose) puts("STUB: APL_strsm called");
    return NULL;
}

void* APPLE_NTHREADS(void)
{
    if (verbose) puts("STUB: APPLE_NTHREADS called");
    return NULL;
}

void* ATLU_DestroyThreadMemory(void)
{
    if (verbose) puts("STUB: ATLU_DestroyThreadMemory called");
    return NULL;
}

void* CAXPY(void)
{
    if (verbose) puts("STUB: CAXPY called");
    return NULL;
}

void* CAXPY_(void)
{
    if (verbose) puts("STUB: CAXPY_ called");
    return NULL;
}

void* CCOPY(void)
{
    if (verbose) puts("STUB: CCOPY called");
    return NULL;
}

void* CCOPY_(void)
{
    if (verbose) puts("STUB: CCOPY_ called");
    return NULL;
}

void* CDOTC(void)
{
    if (verbose) puts("STUB: CDOTC called");
    return NULL;
}

void* CDOTC_(void)
{
    if (verbose) puts("STUB: CDOTC_ called");
    return NULL;
}

void* CDOTU(void)
{
    if (verbose) puts("STUB: CDOTU called");
    return NULL;
}

void* CDOTU_(void)
{
    if (verbose) puts("STUB: CDOTU_ called");
    return NULL;
}

void* CGBMV(void)
{
    if (verbose) puts("STUB: CGBMV called");
    return NULL;
}

void* CGBMV_(void)
{
    if (verbose) puts("STUB: CGBMV_ called");
    return NULL;
}

void* CGEMM(void)
{
    if (verbose) puts("STUB: CGEMM called");
    return NULL;
}

void* CGEMM_(void)
{
    if (verbose) puts("STUB: CGEMM_ called");
    return NULL;
}

void* CGEMV(void)
{
    if (verbose) puts("STUB: CGEMV called");
    return NULL;
}

void* CGEMV_(void)
{
    if (verbose) puts("STUB: CGEMV_ called");
    return NULL;
}

void* CGERC(void)
{
    if (verbose) puts("STUB: CGERC called");
    return NULL;
}

void* CGERC_(void)
{
    if (verbose) puts("STUB: CGERC_ called");
    return NULL;
}

void* CGERU(void)
{
    if (verbose) puts("STUB: CGERU called");
    return NULL;
}

void* CGERU_(void)
{
    if (verbose) puts("STUB: CGERU_ called");
    return NULL;
}

void* CHBMV(void)
{
    if (verbose) puts("STUB: CHBMV called");
    return NULL;
}

void* CHBMV_(void)
{
    if (verbose) puts("STUB: CHBMV_ called");
    return NULL;
}

void* CHEMM(void)
{
    if (verbose) puts("STUB: CHEMM called");
    return NULL;
}

void* CHEMM_(void)
{
    if (verbose) puts("STUB: CHEMM_ called");
    return NULL;
}

void* CHEMV(void)
{
    if (verbose) puts("STUB: CHEMV called");
    return NULL;
}

void* CHEMV_(void)
{
    if (verbose) puts("STUB: CHEMV_ called");
    return NULL;
}

void* CHER(void)
{
    if (verbose) puts("STUB: CHER called");
    return NULL;
}

void* CHER2(void)
{
    if (verbose) puts("STUB: CHER2 called");
    return NULL;
}

void* CHER2K(void)
{
    if (verbose) puts("STUB: CHER2K called");
    return NULL;
}

void* CHER2K_(void)
{
    if (verbose) puts("STUB: CHER2K_ called");
    return NULL;
}

void* CHER2_(void)
{
    if (verbose) puts("STUB: CHER2_ called");
    return NULL;
}

void* CHERK(void)
{
    if (verbose) puts("STUB: CHERK called");
    return NULL;
}

void* CHERK_(void)
{
    if (verbose) puts("STUB: CHERK_ called");
    return NULL;
}

void* CHER_(void)
{
    if (verbose) puts("STUB: CHER_ called");
    return NULL;
}

void* CHPMV(void)
{
    if (verbose) puts("STUB: CHPMV called");
    return NULL;
}

void* CHPMV_(void)
{
    if (verbose) puts("STUB: CHPMV_ called");
    return NULL;
}

void* CHPR(void)
{
    if (verbose) puts("STUB: CHPR called");
    return NULL;
}

void* CHPR2(void)
{
    if (verbose) puts("STUB: CHPR2 called");
    return NULL;
}

void* CHPR2_(void)
{
    if (verbose) puts("STUB: CHPR2_ called");
    return NULL;
}

void* CHPR_(void)
{
    if (verbose) puts("STUB: CHPR_ called");
    return NULL;
}

void* CROTG(void)
{
    if (verbose) puts("STUB: CROTG called");
    return NULL;
}

void* CROTG_(void)
{
    if (verbose) puts("STUB: CROTG_ called");
    return NULL;
}

void* CSCAL(void)
{
    if (verbose) puts("STUB: CSCAL called");
    return NULL;
}

void* CSCAL_(void)
{
    if (verbose) puts("STUB: CSCAL_ called");
    return NULL;
}

void* CSROT(void)
{
    if (verbose) puts("STUB: CSROT called");
    return NULL;
}

void* CSROT_(void)
{
    if (verbose) puts("STUB: CSROT_ called");
    return NULL;
}

void* CSSCAL(void)
{
    if (verbose) puts("STUB: CSSCAL called");
    return NULL;
}

void* CSSCAL_(void)
{
    if (verbose) puts("STUB: CSSCAL_ called");
    return NULL;
}

void* CSWAP(void)
{
    if (verbose) puts("STUB: CSWAP called");
    return NULL;
}

void* CSWAP_(void)
{
    if (verbose) puts("STUB: CSWAP_ called");
    return NULL;
}

void* CSYMM(void)
{
    if (verbose) puts("STUB: CSYMM called");
    return NULL;
}

void* CSYMM_(void)
{
    if (verbose) puts("STUB: CSYMM_ called");
    return NULL;
}

void* CSYR2K(void)
{
    if (verbose) puts("STUB: CSYR2K called");
    return NULL;
}

void* CSYR2K_(void)
{
    if (verbose) puts("STUB: CSYR2K_ called");
    return NULL;
}

void* CSYRK(void)
{
    if (verbose) puts("STUB: CSYRK called");
    return NULL;
}

void* CSYRK_(void)
{
    if (verbose) puts("STUB: CSYRK_ called");
    return NULL;
}

void* CTBMV(void)
{
    if (verbose) puts("STUB: CTBMV called");
    return NULL;
}

void* CTBMV_(void)
{
    if (verbose) puts("STUB: CTBMV_ called");
    return NULL;
}

void* CTBSV(void)
{
    if (verbose) puts("STUB: CTBSV called");
    return NULL;
}

void* CTBSV_(void)
{
    if (verbose) puts("STUB: CTBSV_ called");
    return NULL;
}

void* CTPMV(void)
{
    if (verbose) puts("STUB: CTPMV called");
    return NULL;
}

void* CTPMV_(void)
{
    if (verbose) puts("STUB: CTPMV_ called");
    return NULL;
}

void* CTPSV(void)
{
    if (verbose) puts("STUB: CTPSV called");
    return NULL;
}

void* CTPSV_(void)
{
    if (verbose) puts("STUB: CTPSV_ called");
    return NULL;
}

void* CTRMM(void)
{
    if (verbose) puts("STUB: CTRMM called");
    return NULL;
}

void* CTRMM_(void)
{
    if (verbose) puts("STUB: CTRMM_ called");
    return NULL;
}

void* CTRMV(void)
{
    if (verbose) puts("STUB: CTRMV called");
    return NULL;
}

void* CTRMV_(void)
{
    if (verbose) puts("STUB: CTRMV_ called");
    return NULL;
}

void* CTRSM(void)
{
    if (verbose) puts("STUB: CTRSM called");
    return NULL;
}

void* CTRSM_(void)
{
    if (verbose) puts("STUB: CTRSM_ called");
    return NULL;
}

void* CTRSV(void)
{
    if (verbose) puts("STUB: CTRSV called");
    return NULL;
}

void* CTRSV_(void)
{
    if (verbose) puts("STUB: CTRSV_ called");
    return NULL;
}

void* DASUM(void)
{
    if (verbose) puts("STUB: DASUM called");
    return NULL;
}

void* DASUM_(void)
{
    if (verbose) puts("STUB: DASUM_ called");
    return NULL;
}

void* DAXPY(void)
{
    if (verbose) puts("STUB: DAXPY called");
    return NULL;
}

void* DAXPY_(void)
{
    if (verbose) puts("STUB: DAXPY_ called");
    return NULL;
}

void* DCABS1(void)
{
    if (verbose) puts("STUB: DCABS1 called");
    return NULL;
}

void* DCABS1_(void)
{
    if (verbose) puts("STUB: DCABS1_ called");
    return NULL;
}

void* DCOPY(void)
{
    if (verbose) puts("STUB: DCOPY called");
    return NULL;
}

void* DCOPY_(void)
{
    if (verbose) puts("STUB: DCOPY_ called");
    return NULL;
}

void* DDOT(void)
{
    if (verbose) puts("STUB: DDOT called");
    return NULL;
}

void* DDOT_(void)
{
    if (verbose) puts("STUB: DDOT_ called");
    return NULL;
}

void* DGBMV(void)
{
    if (verbose) puts("STUB: DGBMV called");
    return NULL;
}

void* DGBMV_(void)
{
    if (verbose) puts("STUB: DGBMV_ called");
    return NULL;
}

void* DGEMM(void)
{
    if (verbose) puts("STUB: DGEMM called");
    return NULL;
}

void* DGEMM_(void)
{
    if (verbose) puts("STUB: DGEMM_ called");
    return NULL;
}

void* DGEMV(void)
{
    if (verbose) puts("STUB: DGEMV called");
    return NULL;
}

void* DGEMV_(void)
{
    if (verbose) puts("STUB: DGEMV_ called");
    return NULL;
}

void* DGER(void)
{
    if (verbose) puts("STUB: DGER called");
    return NULL;
}

void* DGER_(void)
{
    if (verbose) puts("STUB: DGER_ called");
    return NULL;
}

void* DNRM2(void)
{
    if (verbose) puts("STUB: DNRM2 called");
    return NULL;
}

void* DNRM2_(void)
{
    if (verbose) puts("STUB: DNRM2_ called");
    return NULL;
}

void* DROT(void)
{
    if (verbose) puts("STUB: DROT called");
    return NULL;
}

void* DROTG(void)
{
    if (verbose) puts("STUB: DROTG called");
    return NULL;
}

void* DROTG_(void)
{
    if (verbose) puts("STUB: DROTG_ called");
    return NULL;
}

void* DROTM(void)
{
    if (verbose) puts("STUB: DROTM called");
    return NULL;
}

void* DROTMG(void)
{
    if (verbose) puts("STUB: DROTMG called");
    return NULL;
}

void* DROTMG_(void)
{
    if (verbose) puts("STUB: DROTMG_ called");
    return NULL;
}

void* DROTM_(void)
{
    if (verbose) puts("STUB: DROTM_ called");
    return NULL;
}

void* DROT_(void)
{
    if (verbose) puts("STUB: DROT_ called");
    return NULL;
}

void* DSBMV(void)
{
    if (verbose) puts("STUB: DSBMV called");
    return NULL;
}

void* DSBMV_(void)
{
    if (verbose) puts("STUB: DSBMV_ called");
    return NULL;
}

void* DSCAL(void)
{
    if (verbose) puts("STUB: DSCAL called");
    return NULL;
}

void* DSCAL_(void)
{
    if (verbose) puts("STUB: DSCAL_ called");
    return NULL;
}

void* DSDOT(void)
{
    if (verbose) puts("STUB: DSDOT called");
    return NULL;
}

void* DSDOT_(void)
{
    if (verbose) puts("STUB: DSDOT_ called");
    return NULL;
}

void* DSPMV(void)
{
    if (verbose) puts("STUB: DSPMV called");
    return NULL;
}

void* DSPMV_(void)
{
    if (verbose) puts("STUB: DSPMV_ called");
    return NULL;
}

void* DSPR(void)
{
    if (verbose) puts("STUB: DSPR called");
    return NULL;
}

void* DSPR2(void)
{
    if (verbose) puts("STUB: DSPR2 called");
    return NULL;
}

void* DSPR2_(void)
{
    if (verbose) puts("STUB: DSPR2_ called");
    return NULL;
}

void* DSPR_(void)
{
    if (verbose) puts("STUB: DSPR_ called");
    return NULL;
}

void* DSWAP(void)
{
    if (verbose) puts("STUB: DSWAP called");
    return NULL;
}

void* DSWAP_(void)
{
    if (verbose) puts("STUB: DSWAP_ called");
    return NULL;
}

void* DSYMM(void)
{
    if (verbose) puts("STUB: DSYMM called");
    return NULL;
}

void* DSYMM_(void)
{
    if (verbose) puts("STUB: DSYMM_ called");
    return NULL;
}

void* DSYMV(void)
{
    if (verbose) puts("STUB: DSYMV called");
    return NULL;
}

void* DSYMV_(void)
{
    if (verbose) puts("STUB: DSYMV_ called");
    return NULL;
}

void* DSYR(void)
{
    if (verbose) puts("STUB: DSYR called");
    return NULL;
}

void* DSYR2(void)
{
    if (verbose) puts("STUB: DSYR2 called");
    return NULL;
}

void* DSYR2K(void)
{
    if (verbose) puts("STUB: DSYR2K called");
    return NULL;
}

void* DSYR2K_(void)
{
    if (verbose) puts("STUB: DSYR2K_ called");
    return NULL;
}

void* DSYR2_(void)
{
    if (verbose) puts("STUB: DSYR2_ called");
    return NULL;
}

void* DSYRK(void)
{
    if (verbose) puts("STUB: DSYRK called");
    return NULL;
}

void* DSYRK_(void)
{
    if (verbose) puts("STUB: DSYRK_ called");
    return NULL;
}

void* DSYR_(void)
{
    if (verbose) puts("STUB: DSYR_ called");
    return NULL;
}

void* DTBMV(void)
{
    if (verbose) puts("STUB: DTBMV called");
    return NULL;
}

void* DTBMV_(void)
{
    if (verbose) puts("STUB: DTBMV_ called");
    return NULL;
}

void* DTBSV(void)
{
    if (verbose) puts("STUB: DTBSV called");
    return NULL;
}

void* DTBSV_(void)
{
    if (verbose) puts("STUB: DTBSV_ called");
    return NULL;
}

void* DTPMV(void)
{
    if (verbose) puts("STUB: DTPMV called");
    return NULL;
}

void* DTPMV_(void)
{
    if (verbose) puts("STUB: DTPMV_ called");
    return NULL;
}

void* DTPSV(void)
{
    if (verbose) puts("STUB: DTPSV called");
    return NULL;
}

void* DTPSV_(void)
{
    if (verbose) puts("STUB: DTPSV_ called");
    return NULL;
}

void* DTRMM(void)
{
    if (verbose) puts("STUB: DTRMM called");
    return NULL;
}

void* DTRMM_(void)
{
    if (verbose) puts("STUB: DTRMM_ called");
    return NULL;
}

void* DTRMV(void)
{
    if (verbose) puts("STUB: DTRMV called");
    return NULL;
}

void* DTRMV_(void)
{
    if (verbose) puts("STUB: DTRMV_ called");
    return NULL;
}

void* DTRSM(void)
{
    if (verbose) puts("STUB: DTRSM called");
    return NULL;
}

void* DTRSM_(void)
{
    if (verbose) puts("STUB: DTRSM_ called");
    return NULL;
}

void* DTRSV(void)
{
    if (verbose) puts("STUB: DTRSV called");
    return NULL;
}

void* DTRSV_(void)
{
    if (verbose) puts("STUB: DTRSV_ called");
    return NULL;
}

void* DZASUM(void)
{
    if (verbose) puts("STUB: DZASUM called");
    return NULL;
}

void* DZASUM_(void)
{
    if (verbose) puts("STUB: DZASUM_ called");
    return NULL;
}

void* DZNRM2(void)
{
    if (verbose) puts("STUB: DZNRM2 called");
    return NULL;
}

void* DZNRM2_(void)
{
    if (verbose) puts("STUB: DZNRM2_ called");
    return NULL;
}

void* ICAMAX(void)
{
    if (verbose) puts("STUB: ICAMAX called");
    return NULL;
}

void* ICAMAX_(void)
{
    if (verbose) puts("STUB: ICAMAX_ called");
    return NULL;
}

void* IDAMAX(void)
{
    if (verbose) puts("STUB: IDAMAX called");
    return NULL;
}

void* IDAMAX_(void)
{
    if (verbose) puts("STUB: IDAMAX_ called");
    return NULL;
}

void* ISAMAX(void)
{
    if (verbose) puts("STUB: ISAMAX called");
    return NULL;
}

void* ISAMAX_(void)
{
    if (verbose) puts("STUB: ISAMAX_ called");
    return NULL;
}

void* IZAMAX(void)
{
    if (verbose) puts("STUB: IZAMAX called");
    return NULL;
}

void* IZAMAX_(void)
{
    if (verbose) puts("STUB: IZAMAX_ called");
    return NULL;
}

void* SASUM(void)
{
    if (verbose) puts("STUB: SASUM called");
    return NULL;
}

void* SASUM_(void)
{
    if (verbose) puts("STUB: SASUM_ called");
    return NULL;
}

void* SAXPY(void)
{
    if (verbose) puts("STUB: SAXPY called");
    return NULL;
}

void* SAXPY_(void)
{
    if (verbose) puts("STUB: SAXPY_ called");
    return NULL;
}

void* SCASUM(void)
{
    if (verbose) puts("STUB: SCASUM called");
    return NULL;
}

void* SCASUM_(void)
{
    if (verbose) puts("STUB: SCASUM_ called");
    return NULL;
}

void* SCNRM2(void)
{
    if (verbose) puts("STUB: SCNRM2 called");
    return NULL;
}

void* SCNRM2_(void)
{
    if (verbose) puts("STUB: SCNRM2_ called");
    return NULL;
}

void* SCOPY(void)
{
    if (verbose) puts("STUB: SCOPY called");
    return NULL;
}

void* SCOPY_(void)
{
    if (verbose) puts("STUB: SCOPY_ called");
    return NULL;
}

void* SDOT(void)
{
    if (verbose) puts("STUB: SDOT called");
    return NULL;
}

void* SDOT_(void)
{
    if (verbose) puts("STUB: SDOT_ called");
    return NULL;
}

void* SDSDOT(void)
{
    if (verbose) puts("STUB: SDSDOT called");
    return NULL;
}

void* SDSDOT_(void)
{
    if (verbose) puts("STUB: SDSDOT_ called");
    return NULL;
}

void* SGBMV(void)
{
    if (verbose) puts("STUB: SGBMV called");
    return NULL;
}

void* SGBMV_(void)
{
    if (verbose) puts("STUB: SGBMV_ called");
    return NULL;
}

void* SGEMM(void)
{
    if (verbose) puts("STUB: SGEMM called");
    return NULL;
}

void* SGEMM_(void)
{
    if (verbose) puts("STUB: SGEMM_ called");
    return NULL;
}

void* SGEMV(void)
{
    if (verbose) puts("STUB: SGEMV called");
    return NULL;
}

void* SGEMV_(void)
{
    if (verbose) puts("STUB: SGEMV_ called");
    return NULL;
}

void* SGER(void)
{
    if (verbose) puts("STUB: SGER called");
    return NULL;
}

void* SGER_(void)
{
    if (verbose) puts("STUB: SGER_ called");
    return NULL;
}

void* SNRM2(void)
{
    if (verbose) puts("STUB: SNRM2 called");
    return NULL;
}

void* SNRM2_(void)
{
    if (verbose) puts("STUB: SNRM2_ called");
    return NULL;
}

void* SROT(void)
{
    if (verbose) puts("STUB: SROT called");
    return NULL;
}

void* SROTG(void)
{
    if (verbose) puts("STUB: SROTG called");
    return NULL;
}

void* SROTG_(void)
{
    if (verbose) puts("STUB: SROTG_ called");
    return NULL;
}

void* SROTM(void)
{
    if (verbose) puts("STUB: SROTM called");
    return NULL;
}

void* SROTMG(void)
{
    if (verbose) puts("STUB: SROTMG called");
    return NULL;
}

void* SROTMG_(void)
{
    if (verbose) puts("STUB: SROTMG_ called");
    return NULL;
}

void* SROTM_(void)
{
    if (verbose) puts("STUB: SROTM_ called");
    return NULL;
}

void* SROT_(void)
{
    if (verbose) puts("STUB: SROT_ called");
    return NULL;
}

void* SSBMV(void)
{
    if (verbose) puts("STUB: SSBMV called");
    return NULL;
}

void* SSBMV_(void)
{
    if (verbose) puts("STUB: SSBMV_ called");
    return NULL;
}

void* SSCAL(void)
{
    if (verbose) puts("STUB: SSCAL called");
    return NULL;
}

void* SSCAL_(void)
{
    if (verbose) puts("STUB: SSCAL_ called");
    return NULL;
}

void* SSPMV(void)
{
    if (verbose) puts("STUB: SSPMV called");
    return NULL;
}

void* SSPMV_(void)
{
    if (verbose) puts("STUB: SSPMV_ called");
    return NULL;
}

void* SSPR(void)
{
    if (verbose) puts("STUB: SSPR called");
    return NULL;
}

void* SSPR2(void)
{
    if (verbose) puts("STUB: SSPR2 called");
    return NULL;
}

void* SSPR2_(void)
{
    if (verbose) puts("STUB: SSPR2_ called");
    return NULL;
}

void* SSPR_(void)
{
    if (verbose) puts("STUB: SSPR_ called");
    return NULL;
}

void* SSWAP(void)
{
    if (verbose) puts("STUB: SSWAP called");
    return NULL;
}

void* SSWAP_(void)
{
    if (verbose) puts("STUB: SSWAP_ called");
    return NULL;
}

void* SSYMM(void)
{
    if (verbose) puts("STUB: SSYMM called");
    return NULL;
}

void* SSYMM_(void)
{
    if (verbose) puts("STUB: SSYMM_ called");
    return NULL;
}

void* SSYMV(void)
{
    if (verbose) puts("STUB: SSYMV called");
    return NULL;
}

void* SSYMV_(void)
{
    if (verbose) puts("STUB: SSYMV_ called");
    return NULL;
}

void* SSYR(void)
{
    if (verbose) puts("STUB: SSYR called");
    return NULL;
}

void* SSYR2(void)
{
    if (verbose) puts("STUB: SSYR2 called");
    return NULL;
}

void* SSYR2K(void)
{
    if (verbose) puts("STUB: SSYR2K called");
    return NULL;
}

void* SSYR2K_(void)
{
    if (verbose) puts("STUB: SSYR2K_ called");
    return NULL;
}

void* SSYR2_(void)
{
    if (verbose) puts("STUB: SSYR2_ called");
    return NULL;
}

void* SSYRK(void)
{
    if (verbose) puts("STUB: SSYRK called");
    return NULL;
}

void* SSYRK_(void)
{
    if (verbose) puts("STUB: SSYRK_ called");
    return NULL;
}

void* SSYR_(void)
{
    if (verbose) puts("STUB: SSYR_ called");
    return NULL;
}

void* STBMV(void)
{
    if (verbose) puts("STUB: STBMV called");
    return NULL;
}

void* STBMV_(void)
{
    if (verbose) puts("STUB: STBMV_ called");
    return NULL;
}

void* STBSV(void)
{
    if (verbose) puts("STUB: STBSV called");
    return NULL;
}

void* STBSV_(void)
{
    if (verbose) puts("STUB: STBSV_ called");
    return NULL;
}

void* STPMV(void)
{
    if (verbose) puts("STUB: STPMV called");
    return NULL;
}

void* STPMV_(void)
{
    if (verbose) puts("STUB: STPMV_ called");
    return NULL;
}

void* STPSV(void)
{
    if (verbose) puts("STUB: STPSV called");
    return NULL;
}

void* STPSV_(void)
{
    if (verbose) puts("STUB: STPSV_ called");
    return NULL;
}

void* STRMM(void)
{
    if (verbose) puts("STUB: STRMM called");
    return NULL;
}

void* STRMM_(void)
{
    if (verbose) puts("STUB: STRMM_ called");
    return NULL;
}

void* STRMV(void)
{
    if (verbose) puts("STUB: STRMV called");
    return NULL;
}

void* STRMV_(void)
{
    if (verbose) puts("STUB: STRMV_ called");
    return NULL;
}

void* STRSM(void)
{
    if (verbose) puts("STUB: STRSM called");
    return NULL;
}

void* STRSM_(void)
{
    if (verbose) puts("STUB: STRSM_ called");
    return NULL;
}

void* STRSV(void)
{
    if (verbose) puts("STUB: STRSV called");
    return NULL;
}

void* STRSV_(void)
{
    if (verbose) puts("STUB: STRSV_ called");
    return NULL;
}

void* SetBLASParamErrorProc(void)
{
    if (verbose) puts("STUB: SetBLASParamErrorProc called");
    return NULL;
}

void* XERBLA(void)
{
    if (verbose) puts("STUB: XERBLA called");
    return NULL;
}

void* XERBLA_(void)
{
    if (verbose) puts("STUB: XERBLA_ called");
    return NULL;
}

void* ZAXPY(void)
{
    if (verbose) puts("STUB: ZAXPY called");
    return NULL;
}

void* ZAXPY_(void)
{
    if (verbose) puts("STUB: ZAXPY_ called");
    return NULL;
}

void* ZCOPY(void)
{
    if (verbose) puts("STUB: ZCOPY called");
    return NULL;
}

void* ZCOPY_(void)
{
    if (verbose) puts("STUB: ZCOPY_ called");
    return NULL;
}

void* ZDOTC(void)
{
    if (verbose) puts("STUB: ZDOTC called");
    return NULL;
}

void* ZDOTC_(void)
{
    if (verbose) puts("STUB: ZDOTC_ called");
    return NULL;
}

void* ZDOTU(void)
{
    if (verbose) puts("STUB: ZDOTU called");
    return NULL;
}

void* ZDOTU_(void)
{
    if (verbose) puts("STUB: ZDOTU_ called");
    return NULL;
}

void* ZDROT(void)
{
    if (verbose) puts("STUB: ZDROT called");
    return NULL;
}

void* ZDROT_(void)
{
    if (verbose) puts("STUB: ZDROT_ called");
    return NULL;
}

void* ZDSCAL(void)
{
    if (verbose) puts("STUB: ZDSCAL called");
    return NULL;
}

void* ZDSCAL_(void)
{
    if (verbose) puts("STUB: ZDSCAL_ called");
    return NULL;
}

void* ZGBMV(void)
{
    if (verbose) puts("STUB: ZGBMV called");
    return NULL;
}

void* ZGBMV_(void)
{
    if (verbose) puts("STUB: ZGBMV_ called");
    return NULL;
}

void* ZGEMM(void)
{
    if (verbose) puts("STUB: ZGEMM called");
    return NULL;
}

void* ZGEMM_(void)
{
    if (verbose) puts("STUB: ZGEMM_ called");
    return NULL;
}

void* ZGEMV(void)
{
    if (verbose) puts("STUB: ZGEMV called");
    return NULL;
}

void* ZGEMV_(void)
{
    if (verbose) puts("STUB: ZGEMV_ called");
    return NULL;
}

void* ZGERC(void)
{
    if (verbose) puts("STUB: ZGERC called");
    return NULL;
}

void* ZGERC_(void)
{
    if (verbose) puts("STUB: ZGERC_ called");
    return NULL;
}

void* ZGERU(void)
{
    if (verbose) puts("STUB: ZGERU called");
    return NULL;
}

void* ZGERU_(void)
{
    if (verbose) puts("STUB: ZGERU_ called");
    return NULL;
}

void* ZHBMV(void)
{
    if (verbose) puts("STUB: ZHBMV called");
    return NULL;
}

void* ZHBMV_(void)
{
    if (verbose) puts("STUB: ZHBMV_ called");
    return NULL;
}

void* ZHEMM(void)
{
    if (verbose) puts("STUB: ZHEMM called");
    return NULL;
}

void* ZHEMM_(void)
{
    if (verbose) puts("STUB: ZHEMM_ called");
    return NULL;
}

void* ZHEMV(void)
{
    if (verbose) puts("STUB: ZHEMV called");
    return NULL;
}

void* ZHEMV_(void)
{
    if (verbose) puts("STUB: ZHEMV_ called");
    return NULL;
}

void* ZHER(void)
{
    if (verbose) puts("STUB: ZHER called");
    return NULL;
}

void* ZHER2(void)
{
    if (verbose) puts("STUB: ZHER2 called");
    return NULL;
}

void* ZHER2K(void)
{
    if (verbose) puts("STUB: ZHER2K called");
    return NULL;
}

void* ZHER2K_(void)
{
    if (verbose) puts("STUB: ZHER2K_ called");
    return NULL;
}

void* ZHER2_(void)
{
    if (verbose) puts("STUB: ZHER2_ called");
    return NULL;
}

void* ZHERK(void)
{
    if (verbose) puts("STUB: ZHERK called");
    return NULL;
}

void* ZHERK_(void)
{
    if (verbose) puts("STUB: ZHERK_ called");
    return NULL;
}

void* ZHER_(void)
{
    if (verbose) puts("STUB: ZHER_ called");
    return NULL;
}

void* ZHPMV(void)
{
    if (verbose) puts("STUB: ZHPMV called");
    return NULL;
}

void* ZHPMV_(void)
{
    if (verbose) puts("STUB: ZHPMV_ called");
    return NULL;
}

void* ZHPR(void)
{
    if (verbose) puts("STUB: ZHPR called");
    return NULL;
}

void* ZHPR2(void)
{
    if (verbose) puts("STUB: ZHPR2 called");
    return NULL;
}

void* ZHPR2_(void)
{
    if (verbose) puts("STUB: ZHPR2_ called");
    return NULL;
}

void* ZHPR_(void)
{
    if (verbose) puts("STUB: ZHPR_ called");
    return NULL;
}

void* ZROTG(void)
{
    if (verbose) puts("STUB: ZROTG called");
    return NULL;
}

void* ZROTG_(void)
{
    if (verbose) puts("STUB: ZROTG_ called");
    return NULL;
}

void* ZSCAL(void)
{
    if (verbose) puts("STUB: ZSCAL called");
    return NULL;
}

void* ZSCAL_(void)
{
    if (verbose) puts("STUB: ZSCAL_ called");
    return NULL;
}

void* ZSWAP(void)
{
    if (verbose) puts("STUB: ZSWAP called");
    return NULL;
}

void* ZSWAP_(void)
{
    if (verbose) puts("STUB: ZSWAP_ called");
    return NULL;
}

void* ZSYMM(void)
{
    if (verbose) puts("STUB: ZSYMM called");
    return NULL;
}

void* ZSYMM_(void)
{
    if (verbose) puts("STUB: ZSYMM_ called");
    return NULL;
}

void* ZSYR2K(void)
{
    if (verbose) puts("STUB: ZSYR2K called");
    return NULL;
}

void* ZSYR2K_(void)
{
    if (verbose) puts("STUB: ZSYR2K_ called");
    return NULL;
}

void* ZSYRK(void)
{
    if (verbose) puts("STUB: ZSYRK called");
    return NULL;
}

void* ZSYRK_(void)
{
    if (verbose) puts("STUB: ZSYRK_ called");
    return NULL;
}

void* ZTBMV(void)
{
    if (verbose) puts("STUB: ZTBMV called");
    return NULL;
}

void* ZTBMV_(void)
{
    if (verbose) puts("STUB: ZTBMV_ called");
    return NULL;
}

void* ZTBSV(void)
{
    if (verbose) puts("STUB: ZTBSV called");
    return NULL;
}

void* ZTBSV_(void)
{
    if (verbose) puts("STUB: ZTBSV_ called");
    return NULL;
}

void* ZTPMV(void)
{
    if (verbose) puts("STUB: ZTPMV called");
    return NULL;
}

void* ZTPMV_(void)
{
    if (verbose) puts("STUB: ZTPMV_ called");
    return NULL;
}

void* ZTPSV(void)
{
    if (verbose) puts("STUB: ZTPSV called");
    return NULL;
}

void* ZTPSV_(void)
{
    if (verbose) puts("STUB: ZTPSV_ called");
    return NULL;
}

void* ZTRMM(void)
{
    if (verbose) puts("STUB: ZTRMM called");
    return NULL;
}

void* ZTRMM_(void)
{
    if (verbose) puts("STUB: ZTRMM_ called");
    return NULL;
}

void* ZTRMV(void)
{
    if (verbose) puts("STUB: ZTRMV called");
    return NULL;
}

void* ZTRMV_(void)
{
    if (verbose) puts("STUB: ZTRMV_ called");
    return NULL;
}

void* ZTRSM(void)
{
    if (verbose) puts("STUB: ZTRSM called");
    return NULL;
}

void* ZTRSM_(void)
{
    if (verbose) puts("STUB: ZTRSM_ called");
    return NULL;
}

void* ZTRSV(void)
{
    if (verbose) puts("STUB: ZTRSV called");
    return NULL;
}

void* ZTRSV_(void)
{
    if (verbose) puts("STUB: ZTRSV_ called");
    return NULL;
}

void* appleblas_dgeadd(void)
{
    if (verbose) puts("STUB: appleblas_dgeadd called");
    return NULL;
}

void* appleblas_sgeadd(void)
{
    if (verbose) puts("STUB: appleblas_sgeadd called");
    return NULL;
}

void* catlas_caxpby(void)
{
    if (verbose) puts("STUB: catlas_caxpby called");
    return NULL;
}

void* catlas_cset(void)
{
    if (verbose) puts("STUB: catlas_cset called");
    return NULL;
}

void* catlas_daxpby(void)
{
    if (verbose) puts("STUB: catlas_daxpby called");
    return NULL;
}

void* catlas_dset(void)
{
    if (verbose) puts("STUB: catlas_dset called");
    return NULL;
}

void* catlas_saxpby(void)
{
    if (verbose) puts("STUB: catlas_saxpby called");
    return NULL;
}

void* catlas_sset(void)
{
    if (verbose) puts("STUB: catlas_sset called");
    return NULL;
}

void* catlas_zaxpby(void)
{
    if (verbose) puts("STUB: catlas_zaxpby called");
    return NULL;
}

void* catlas_zset(void)
{
    if (verbose) puts("STUB: catlas_zset called");
    return NULL;
}

void* caxpy(void)
{
    if (verbose) puts("STUB: caxpy called");
    return NULL;
}

void* caxpy_(void)
{
    if (verbose) puts("STUB: caxpy_ called");
    return NULL;
}

void* cblas_caxpy(void)
{
    if (verbose) puts("STUB: cblas_caxpy called");
    return NULL;
}

void* cblas_ccopy(void)
{
    if (verbose) puts("STUB: cblas_ccopy called");
    return NULL;
}

void* cblas_cdotc_sub(void)
{
    if (verbose) puts("STUB: cblas_cdotc_sub called");
    return NULL;
}

void* cblas_cdotu_sub(void)
{
    if (verbose) puts("STUB: cblas_cdotu_sub called");
    return NULL;
}

void* cblas_cgbmv(void)
{
    if (verbose) puts("STUB: cblas_cgbmv called");
    return NULL;
}

void* cblas_cgemm(void)
{
    if (verbose) puts("STUB: cblas_cgemm called");
    return NULL;
}

void* cblas_cgemv(void)
{
    if (verbose) puts("STUB: cblas_cgemv called");
    return NULL;
}

void* cblas_cgerc(void)
{
    if (verbose) puts("STUB: cblas_cgerc called");
    return NULL;
}

void* cblas_cgeru(void)
{
    if (verbose) puts("STUB: cblas_cgeru called");
    return NULL;
}

void* cblas_chbmv(void)
{
    if (verbose) puts("STUB: cblas_chbmv called");
    return NULL;
}

void* cblas_chemm(void)
{
    if (verbose) puts("STUB: cblas_chemm called");
    return NULL;
}

void* cblas_chemv(void)
{
    if (verbose) puts("STUB: cblas_chemv called");
    return NULL;
}

void* cblas_cher(void)
{
    if (verbose) puts("STUB: cblas_cher called");
    return NULL;
}

void* cblas_cher2(void)
{
    if (verbose) puts("STUB: cblas_cher2 called");
    return NULL;
}

void* cblas_cher2k(void)
{
    if (verbose) puts("STUB: cblas_cher2k called");
    return NULL;
}

void* cblas_cherk(void)
{
    if (verbose) puts("STUB: cblas_cherk called");
    return NULL;
}

void* cblas_chpmv(void)
{
    if (verbose) puts("STUB: cblas_chpmv called");
    return NULL;
}

void* cblas_chpr(void)
{
    if (verbose) puts("STUB: cblas_chpr called");
    return NULL;
}

void* cblas_chpr2(void)
{
    if (verbose) puts("STUB: cblas_chpr2 called");
    return NULL;
}

void* cblas_crotg(void)
{
    if (verbose) puts("STUB: cblas_crotg called");
    return NULL;
}

void* cblas_cscal(void)
{
    if (verbose) puts("STUB: cblas_cscal called");
    return NULL;
}

void* cblas_csrot(void)
{
    if (verbose) puts("STUB: cblas_csrot called");
    return NULL;
}

void* cblas_csscal(void)
{
    if (verbose) puts("STUB: cblas_csscal called");
    return NULL;
}

void* cblas_cswap(void)
{
    if (verbose) puts("STUB: cblas_cswap called");
    return NULL;
}

void* cblas_csymm(void)
{
    if (verbose) puts("STUB: cblas_csymm called");
    return NULL;
}

void* cblas_csyr2k(void)
{
    if (verbose) puts("STUB: cblas_csyr2k called");
    return NULL;
}

void* cblas_csyrk(void)
{
    if (verbose) puts("STUB: cblas_csyrk called");
    return NULL;
}

void* cblas_ctbmv(void)
{
    if (verbose) puts("STUB: cblas_ctbmv called");
    return NULL;
}

void* cblas_ctbsv(void)
{
    if (verbose) puts("STUB: cblas_ctbsv called");
    return NULL;
}

void* cblas_ctpmv(void)
{
    if (verbose) puts("STUB: cblas_ctpmv called");
    return NULL;
}

void* cblas_ctpsv(void)
{
    if (verbose) puts("STUB: cblas_ctpsv called");
    return NULL;
}

void* cblas_ctrmm(void)
{
    if (verbose) puts("STUB: cblas_ctrmm called");
    return NULL;
}

void* cblas_ctrmv(void)
{
    if (verbose) puts("STUB: cblas_ctrmv called");
    return NULL;
}

void* cblas_ctrsm(void)
{
    if (verbose) puts("STUB: cblas_ctrsm called");
    return NULL;
}

void* cblas_ctrsv(void)
{
    if (verbose) puts("STUB: cblas_ctrsv called");
    return NULL;
}

void* cblas_dasum(void)
{
    if (verbose) puts("STUB: cblas_dasum called");
    return NULL;
}

void* cblas_daxpy(void)
{
    if (verbose) puts("STUB: cblas_daxpy called");
    return NULL;
}

void* cblas_dcopy(void)
{
    if (verbose) puts("STUB: cblas_dcopy called");
    return NULL;
}

void* cblas_ddot(void)
{
    if (verbose) puts("STUB: cblas_ddot called");
    return NULL;
}

void* cblas_dgbmv(void)
{
    if (verbose) puts("STUB: cblas_dgbmv called");
    return NULL;
}

void* cblas_dgemm(void)
{
    if (verbose) puts("STUB: cblas_dgemm called");
    return NULL;
}

void* cblas_dgemv(void)
{
    if (verbose) puts("STUB: cblas_dgemv called");
    return NULL;
}

void* cblas_dger(void)
{
    if (verbose) puts("STUB: cblas_dger called");
    return NULL;
}

void* cblas_dnrm2(void)
{
    if (verbose) puts("STUB: cblas_dnrm2 called");
    return NULL;
}

void* cblas_drot(void)
{
    if (verbose) puts("STUB: cblas_drot called");
    return NULL;
}

void* cblas_drotg(void)
{
    if (verbose) puts("STUB: cblas_drotg called");
    return NULL;
}

void* cblas_drotm(void)
{
    if (verbose) puts("STUB: cblas_drotm called");
    return NULL;
}

void* cblas_drotmg(void)
{
    if (verbose) puts("STUB: cblas_drotmg called");
    return NULL;
}

void* cblas_dsbmv(void)
{
    if (verbose) puts("STUB: cblas_dsbmv called");
    return NULL;
}

void* cblas_dscal(void)
{
    if (verbose) puts("STUB: cblas_dscal called");
    return NULL;
}

void* cblas_dsdot(void)
{
    if (verbose) puts("STUB: cblas_dsdot called");
    return NULL;
}

void* cblas_dspmv(void)
{
    if (verbose) puts("STUB: cblas_dspmv called");
    return NULL;
}

void* cblas_dspr(void)
{
    if (verbose) puts("STUB: cblas_dspr called");
    return NULL;
}

void* cblas_dspr2(void)
{
    if (verbose) puts("STUB: cblas_dspr2 called");
    return NULL;
}

void* cblas_dswap(void)
{
    if (verbose) puts("STUB: cblas_dswap called");
    return NULL;
}

void* cblas_dsymm(void)
{
    if (verbose) puts("STUB: cblas_dsymm called");
    return NULL;
}

void* cblas_dsymv(void)
{
    if (verbose) puts("STUB: cblas_dsymv called");
    return NULL;
}

void* cblas_dsyr(void)
{
    if (verbose) puts("STUB: cblas_dsyr called");
    return NULL;
}

void* cblas_dsyr2(void)
{
    if (verbose) puts("STUB: cblas_dsyr2 called");
    return NULL;
}

void* cblas_dsyr2k(void)
{
    if (verbose) puts("STUB: cblas_dsyr2k called");
    return NULL;
}

void* cblas_dsyrk(void)
{
    if (verbose) puts("STUB: cblas_dsyrk called");
    return NULL;
}

void* cblas_dtbmv(void)
{
    if (verbose) puts("STUB: cblas_dtbmv called");
    return NULL;
}

void* cblas_dtbsv(void)
{
    if (verbose) puts("STUB: cblas_dtbsv called");
    return NULL;
}

void* cblas_dtpmv(void)
{
    if (verbose) puts("STUB: cblas_dtpmv called");
    return NULL;
}

void* cblas_dtpsv(void)
{
    if (verbose) puts("STUB: cblas_dtpsv called");
    return NULL;
}

void* cblas_dtrmm(void)
{
    if (verbose) puts("STUB: cblas_dtrmm called");
    return NULL;
}

void* cblas_dtrmv(void)
{
    if (verbose) puts("STUB: cblas_dtrmv called");
    return NULL;
}

void* cblas_dtrsm(void)
{
    if (verbose) puts("STUB: cblas_dtrsm called");
    return NULL;
}

void* cblas_dtrsv(void)
{
    if (verbose) puts("STUB: cblas_dtrsv called");
    return NULL;
}

void* cblas_dzasum(void)
{
    if (verbose) puts("STUB: cblas_dzasum called");
    return NULL;
}

void* cblas_dznrm2(void)
{
    if (verbose) puts("STUB: cblas_dznrm2 called");
    return NULL;
}

void* cblas_errprn(void)
{
    if (verbose) puts("STUB: cblas_errprn called");
    return NULL;
}

void* cblas_icamax(void)
{
    if (verbose) puts("STUB: cblas_icamax called");
    return NULL;
}

void* cblas_idamax(void)
{
    if (verbose) puts("STUB: cblas_idamax called");
    return NULL;
}

void* cblas_isamax(void)
{
    if (verbose) puts("STUB: cblas_isamax called");
    return NULL;
}

void* cblas_izamax(void)
{
    if (verbose) puts("STUB: cblas_izamax called");
    return NULL;
}

void* cblas_sasum(void)
{
    if (verbose) puts("STUB: cblas_sasum called");
    return NULL;
}

void* cblas_saxpy(void)
{
    if (verbose) puts("STUB: cblas_saxpy called");
    return NULL;
}

void* cblas_scasum(void)
{
    if (verbose) puts("STUB: cblas_scasum called");
    return NULL;
}

void* cblas_scnrm2(void)
{
    if (verbose) puts("STUB: cblas_scnrm2 called");
    return NULL;
}

void* cblas_scopy(void)
{
    if (verbose) puts("STUB: cblas_scopy called");
    return NULL;
}

void* cblas_sdot(void)
{
    if (verbose) puts("STUB: cblas_sdot called");
    return NULL;
}

void* cblas_sdsdot(void)
{
    if (verbose) puts("STUB: cblas_sdsdot called");
    return NULL;
}

void* cblas_sgbmv(void)
{
    if (verbose) puts("STUB: cblas_sgbmv called");
    return NULL;
}

void* cblas_sgemm(void)
{
    if (verbose) puts("STUB: cblas_sgemm called");
    return NULL;
}

void* cblas_sgemv(void)
{
    if (verbose) puts("STUB: cblas_sgemv called");
    return NULL;
}

void* cblas_sger(void)
{
    if (verbose) puts("STUB: cblas_sger called");
    return NULL;
}

void* cblas_snrm2(void)
{
    if (verbose) puts("STUB: cblas_snrm2 called");
    return NULL;
}

void* cblas_srot(void)
{
    if (verbose) puts("STUB: cblas_srot called");
    return NULL;
}

void* cblas_srotg(void)
{
    if (verbose) puts("STUB: cblas_srotg called");
    return NULL;
}

void* cblas_srotm(void)
{
    if (verbose) puts("STUB: cblas_srotm called");
    return NULL;
}

void* cblas_srotmg(void)
{
    if (verbose) puts("STUB: cblas_srotmg called");
    return NULL;
}

void* cblas_ssbmv(void)
{
    if (verbose) puts("STUB: cblas_ssbmv called");
    return NULL;
}

void* cblas_sscal(void)
{
    if (verbose) puts("STUB: cblas_sscal called");
    return NULL;
}

void* cblas_sspmv(void)
{
    if (verbose) puts("STUB: cblas_sspmv called");
    return NULL;
}

void* cblas_sspr(void)
{
    if (verbose) puts("STUB: cblas_sspr called");
    return NULL;
}

void* cblas_sspr2(void)
{
    if (verbose) puts("STUB: cblas_sspr2 called");
    return NULL;
}

void* cblas_sswap(void)
{
    if (verbose) puts("STUB: cblas_sswap called");
    return NULL;
}

void* cblas_ssymm(void)
{
    if (verbose) puts("STUB: cblas_ssymm called");
    return NULL;
}

void* cblas_ssymv(void)
{
    if (verbose) puts("STUB: cblas_ssymv called");
    return NULL;
}

void* cblas_ssyr(void)
{
    if (verbose) puts("STUB: cblas_ssyr called");
    return NULL;
}

void* cblas_ssyr2(void)
{
    if (verbose) puts("STUB: cblas_ssyr2 called");
    return NULL;
}

void* cblas_ssyr2k(void)
{
    if (verbose) puts("STUB: cblas_ssyr2k called");
    return NULL;
}

void* cblas_ssyrk(void)
{
    if (verbose) puts("STUB: cblas_ssyrk called");
    return NULL;
}

void* cblas_stbmv(void)
{
    if (verbose) puts("STUB: cblas_stbmv called");
    return NULL;
}

void* cblas_stbsv(void)
{
    if (verbose) puts("STUB: cblas_stbsv called");
    return NULL;
}

void* cblas_stpmv(void)
{
    if (verbose) puts("STUB: cblas_stpmv called");
    return NULL;
}

void* cblas_stpsv(void)
{
    if (verbose) puts("STUB: cblas_stpsv called");
    return NULL;
}

void* cblas_strmm(void)
{
    if (verbose) puts("STUB: cblas_strmm called");
    return NULL;
}

void* cblas_strmv(void)
{
    if (verbose) puts("STUB: cblas_strmv called");
    return NULL;
}

void* cblas_strsm(void)
{
    if (verbose) puts("STUB: cblas_strsm called");
    return NULL;
}

void* cblas_strsv(void)
{
    if (verbose) puts("STUB: cblas_strsv called");
    return NULL;
}

void* cblas_xerbla(void)
{
    if (verbose) puts("STUB: cblas_xerbla called");
    return NULL;
}

void* cblas_zaxpy(void)
{
    if (verbose) puts("STUB: cblas_zaxpy called");
    return NULL;
}

void* cblas_zcopy(void)
{
    if (verbose) puts("STUB: cblas_zcopy called");
    return NULL;
}

void* cblas_zdotc_sub(void)
{
    if (verbose) puts("STUB: cblas_zdotc_sub called");
    return NULL;
}

void* cblas_zdotu_sub(void)
{
    if (verbose) puts("STUB: cblas_zdotu_sub called");
    return NULL;
}

void* cblas_zdrot(void)
{
    if (verbose) puts("STUB: cblas_zdrot called");
    return NULL;
}

void* cblas_zdscal(void)
{
    if (verbose) puts("STUB: cblas_zdscal called");
    return NULL;
}

void* cblas_zgbmv(void)
{
    if (verbose) puts("STUB: cblas_zgbmv called");
    return NULL;
}

void* cblas_zgemm(void)
{
    if (verbose) puts("STUB: cblas_zgemm called");
    return NULL;
}

void* cblas_zgemv(void)
{
    if (verbose) puts("STUB: cblas_zgemv called");
    return NULL;
}

void* cblas_zgerc(void)
{
    if (verbose) puts("STUB: cblas_zgerc called");
    return NULL;
}

void* cblas_zgeru(void)
{
    if (verbose) puts("STUB: cblas_zgeru called");
    return NULL;
}

void* cblas_zhbmv(void)
{
    if (verbose) puts("STUB: cblas_zhbmv called");
    return NULL;
}

void* cblas_zhemm(void)
{
    if (verbose) puts("STUB: cblas_zhemm called");
    return NULL;
}

void* cblas_zhemv(void)
{
    if (verbose) puts("STUB: cblas_zhemv called");
    return NULL;
}

void* cblas_zher(void)
{
    if (verbose) puts("STUB: cblas_zher called");
    return NULL;
}

void* cblas_zher2(void)
{
    if (verbose) puts("STUB: cblas_zher2 called");
    return NULL;
}

void* cblas_zher2k(void)
{
    if (verbose) puts("STUB: cblas_zher2k called");
    return NULL;
}

void* cblas_zherk(void)
{
    if (verbose) puts("STUB: cblas_zherk called");
    return NULL;
}

void* cblas_zhpmv(void)
{
    if (verbose) puts("STUB: cblas_zhpmv called");
    return NULL;
}

void* cblas_zhpr(void)
{
    if (verbose) puts("STUB: cblas_zhpr called");
    return NULL;
}

void* cblas_zhpr2(void)
{
    if (verbose) puts("STUB: cblas_zhpr2 called");
    return NULL;
}

void* cblas_zrotg(void)
{
    if (verbose) puts("STUB: cblas_zrotg called");
    return NULL;
}

void* cblas_zscal(void)
{
    if (verbose) puts("STUB: cblas_zscal called");
    return NULL;
}

void* cblas_zswap(void)
{
    if (verbose) puts("STUB: cblas_zswap called");
    return NULL;
}

void* cblas_zsymm(void)
{
    if (verbose) puts("STUB: cblas_zsymm called");
    return NULL;
}

void* cblas_zsyr2k(void)
{
    if (verbose) puts("STUB: cblas_zsyr2k called");
    return NULL;
}

void* cblas_zsyrk(void)
{
    if (verbose) puts("STUB: cblas_zsyrk called");
    return NULL;
}

void* cblas_ztbmv(void)
{
    if (verbose) puts("STUB: cblas_ztbmv called");
    return NULL;
}

void* cblas_ztbsv(void)
{
    if (verbose) puts("STUB: cblas_ztbsv called");
    return NULL;
}

void* cblas_ztpmv(void)
{
    if (verbose) puts("STUB: cblas_ztpmv called");
    return NULL;
}

void* cblas_ztpsv(void)
{
    if (verbose) puts("STUB: cblas_ztpsv called");
    return NULL;
}

void* cblas_ztrmm(void)
{
    if (verbose) puts("STUB: cblas_ztrmm called");
    return NULL;
}

void* cblas_ztrmv(void)
{
    if (verbose) puts("STUB: cblas_ztrmv called");
    return NULL;
}

void* cblas_ztrsm(void)
{
    if (verbose) puts("STUB: cblas_ztrsm called");
    return NULL;
}

void* cblas_ztrsv(void)
{
    if (verbose) puts("STUB: cblas_ztrsv called");
    return NULL;
}

void* ccopy(void)
{
    if (verbose) puts("STUB: ccopy called");
    return NULL;
}

void* ccopy_(void)
{
    if (verbose) puts("STUB: ccopy_ called");
    return NULL;
}

void* cdotc(void)
{
    if (verbose) puts("STUB: cdotc called");
    return NULL;
}

void* cdotc_(void)
{
    if (verbose) puts("STUB: cdotc_ called");
    return NULL;
}

void* cdotu(void)
{
    if (verbose) puts("STUB: cdotu called");
    return NULL;
}

void* cdotu_(void)
{
    if (verbose) puts("STUB: cdotu_ called");
    return NULL;
}

void* cgbmv(void)
{
    if (verbose) puts("STUB: cgbmv called");
    return NULL;
}

void* cgbmv_(void)
{
    if (verbose) puts("STUB: cgbmv_ called");
    return NULL;
}

void* cgemm(void)
{
    if (verbose) puts("STUB: cgemm called");
    return NULL;
}

void* cgemm_(void)
{
    if (verbose) puts("STUB: cgemm_ called");
    return NULL;
}

void* cgemv(void)
{
    if (verbose) puts("STUB: cgemv called");
    return NULL;
}

void* cgemv_(void)
{
    if (verbose) puts("STUB: cgemv_ called");
    return NULL;
}

void* cgerc(void)
{
    if (verbose) puts("STUB: cgerc called");
    return NULL;
}

void* cgerc_(void)
{
    if (verbose) puts("STUB: cgerc_ called");
    return NULL;
}

void* cgeru(void)
{
    if (verbose) puts("STUB: cgeru called");
    return NULL;
}

void* cgeru_(void)
{
    if (verbose) puts("STUB: cgeru_ called");
    return NULL;
}

void* chbmv(void)
{
    if (verbose) puts("STUB: chbmv called");
    return NULL;
}

void* chbmv_(void)
{
    if (verbose) puts("STUB: chbmv_ called");
    return NULL;
}

void* chemm(void)
{
    if (verbose) puts("STUB: chemm called");
    return NULL;
}

void* chemm_(void)
{
    if (verbose) puts("STUB: chemm_ called");
    return NULL;
}

void* chemv(void)
{
    if (verbose) puts("STUB: chemv called");
    return NULL;
}

void* chemv_(void)
{
    if (verbose) puts("STUB: chemv_ called");
    return NULL;
}

void* cher(void)
{
    if (verbose) puts("STUB: cher called");
    return NULL;
}

void* cher2(void)
{
    if (verbose) puts("STUB: cher2 called");
    return NULL;
}

void* cher2_(void)
{
    if (verbose) puts("STUB: cher2_ called");
    return NULL;
}

void* cher2k(void)
{
    if (verbose) puts("STUB: cher2k called");
    return NULL;
}

void* cher2k_(void)
{
    if (verbose) puts("STUB: cher2k_ called");
    return NULL;
}

void* cher_(void)
{
    if (verbose) puts("STUB: cher_ called");
    return NULL;
}

void* cherk(void)
{
    if (verbose) puts("STUB: cherk called");
    return NULL;
}

void* cherk_(void)
{
    if (verbose) puts("STUB: cherk_ called");
    return NULL;
}

void* chpmv(void)
{
    if (verbose) puts("STUB: chpmv called");
    return NULL;
}

void* chpmv_(void)
{
    if (verbose) puts("STUB: chpmv_ called");
    return NULL;
}

void* chpr(void)
{
    if (verbose) puts("STUB: chpr called");
    return NULL;
}

void* chpr2(void)
{
    if (verbose) puts("STUB: chpr2 called");
    return NULL;
}

void* chpr2_(void)
{
    if (verbose) puts("STUB: chpr2_ called");
    return NULL;
}

void* chpr_(void)
{
    if (verbose) puts("STUB: chpr_ called");
    return NULL;
}

void* crotg(void)
{
    if (verbose) puts("STUB: crotg called");
    return NULL;
}

void* crotg_(void)
{
    if (verbose) puts("STUB: crotg_ called");
    return NULL;
}

void* cscal(void)
{
    if (verbose) puts("STUB: cscal called");
    return NULL;
}

void* cscal_(void)
{
    if (verbose) puts("STUB: cscal_ called");
    return NULL;
}

void* csrot(void)
{
    if (verbose) puts("STUB: csrot called");
    return NULL;
}

void* csrot_(void)
{
    if (verbose) puts("STUB: csrot_ called");
    return NULL;
}

void* csscal(void)
{
    if (verbose) puts("STUB: csscal called");
    return NULL;
}

void* csscal_(void)
{
    if (verbose) puts("STUB: csscal_ called");
    return NULL;
}

void* cswap(void)
{
    if (verbose) puts("STUB: cswap called");
    return NULL;
}

void* cswap_(void)
{
    if (verbose) puts("STUB: cswap_ called");
    return NULL;
}

void* csymm(void)
{
    if (verbose) puts("STUB: csymm called");
    return NULL;
}

void* csymm_(void)
{
    if (verbose) puts("STUB: csymm_ called");
    return NULL;
}

void* csyr2k(void)
{
    if (verbose) puts("STUB: csyr2k called");
    return NULL;
}

void* csyr2k_(void)
{
    if (verbose) puts("STUB: csyr2k_ called");
    return NULL;
}

void* csyrk(void)
{
    if (verbose) puts("STUB: csyrk called");
    return NULL;
}

void* csyrk_(void)
{
    if (verbose) puts("STUB: csyrk_ called");
    return NULL;
}

void* ctbmv(void)
{
    if (verbose) puts("STUB: ctbmv called");
    return NULL;
}

void* ctbmv_(void)
{
    if (verbose) puts("STUB: ctbmv_ called");
    return NULL;
}

void* ctbsv(void)
{
    if (verbose) puts("STUB: ctbsv called");
    return NULL;
}

void* ctbsv_(void)
{
    if (verbose) puts("STUB: ctbsv_ called");
    return NULL;
}

void* ctpmv(void)
{
    if (verbose) puts("STUB: ctpmv called");
    return NULL;
}

void* ctpmv_(void)
{
    if (verbose) puts("STUB: ctpmv_ called");
    return NULL;
}

void* ctpsv(void)
{
    if (verbose) puts("STUB: ctpsv called");
    return NULL;
}

void* ctpsv_(void)
{
    if (verbose) puts("STUB: ctpsv_ called");
    return NULL;
}

void* ctrmm(void)
{
    if (verbose) puts("STUB: ctrmm called");
    return NULL;
}

void* ctrmm_(void)
{
    if (verbose) puts("STUB: ctrmm_ called");
    return NULL;
}

void* ctrmv(void)
{
    if (verbose) puts("STUB: ctrmv called");
    return NULL;
}

void* ctrmv_(void)
{
    if (verbose) puts("STUB: ctrmv_ called");
    return NULL;
}

void* ctrsm(void)
{
    if (verbose) puts("STUB: ctrsm called");
    return NULL;
}

void* ctrsm_(void)
{
    if (verbose) puts("STUB: ctrsm_ called");
    return NULL;
}

void* ctrsv(void)
{
    if (verbose) puts("STUB: ctrsv called");
    return NULL;
}

void* ctrsv_(void)
{
    if (verbose) puts("STUB: ctrsv_ called");
    return NULL;
}

void* dasum(void)
{
    if (verbose) puts("STUB: dasum called");
    return NULL;
}

void* dasum_(void)
{
    if (verbose) puts("STUB: dasum_ called");
    return NULL;
}

void* daxpy(void)
{
    if (verbose) puts("STUB: daxpy called");
    return NULL;
}

void* daxpy_(void)
{
    if (verbose) puts("STUB: daxpy_ called");
    return NULL;
}

void* dcabs1(void)
{
    if (verbose) puts("STUB: dcabs1 called");
    return NULL;
}

void* dcabs1_(void)
{
    if (verbose) puts("STUB: dcabs1_ called");
    return NULL;
}

void* dcopy(void)
{
    if (verbose) puts("STUB: dcopy called");
    return NULL;
}

void* dcopy_(void)
{
    if (verbose) puts("STUB: dcopy_ called");
    return NULL;
}

void* ddot(void)
{
    if (verbose) puts("STUB: ddot called");
    return NULL;
}

void* ddot_(void)
{
    if (verbose) puts("STUB: ddot_ called");
    return NULL;
}

void* dgbmv(void)
{
    if (verbose) puts("STUB: dgbmv called");
    return NULL;
}

void* dgbmv_(void)
{
    if (verbose) puts("STUB: dgbmv_ called");
    return NULL;
}

void* dgeCopy(void)
{
    if (verbose) puts("STUB: dgeCopy called");
    return NULL;
}

void* dgePack_A_NoTran(void)
{
    if (verbose) puts("STUB: dgePack_A_NoTran called");
    return NULL;
}

void* dgePack_A_Tran(void)
{
    if (verbose) puts("STUB: dgePack_A_Tran called");
    return NULL;
}

void* dgePack_B_NoTran(void)
{
    if (verbose) puts("STUB: dgePack_B_NoTran called");
    return NULL;
}

void* dgePack_B_Tran(void)
{
    if (verbose) puts("STUB: dgePack_B_Tran called");
    return NULL;
}

void* dgeSetZero(void)
{
    if (verbose) puts("STUB: dgeSetZero called");
    return NULL;
}

void* dgemm(void)
{
    if (verbose) puts("STUB: dgemm called");
    return NULL;
}

void* dgemm_(void)
{
    if (verbose) puts("STUB: dgemm_ called");
    return NULL;
}

void* dgemv(void)
{
    if (verbose) puts("STUB: dgemv called");
    return NULL;
}

void* dgemv_(void)
{
    if (verbose) puts("STUB: dgemv_ called");
    return NULL;
}

void* dger(void)
{
    if (verbose) puts("STUB: dger called");
    return NULL;
}

void* dger_(void)
{
    if (verbose) puts("STUB: dger_ called");
    return NULL;
}

void* dnrm2(void)
{
    if (verbose) puts("STUB: dnrm2 called");
    return NULL;
}

void* dnrm2_(void)
{
    if (verbose) puts("STUB: dnrm2_ called");
    return NULL;
}

void* double_general_add(void)
{
    if (verbose) puts("STUB: double_general_add called");
    return NULL;
}

void* double_general_add_scalar(void)
{
    if (verbose) puts("STUB: double_general_add_scalar called");
    return NULL;
}

void* double_general_elementwise_product(void)
{
    if (verbose) puts("STUB: double_general_elementwise_product called");
    return NULL;
}

void* double_general_permute_cols(void)
{
    if (verbose) puts("STUB: double_general_permute_cols called");
    return NULL;
}

void* double_general_permute_rows(void)
{
    if (verbose) puts("STUB: double_general_permute_rows called");
    return NULL;
}

void* double_general_product_scalar(void)
{
    if (verbose) puts("STUB: double_general_product_scalar called");
    return NULL;
}

void* double_general_splat(void)
{
    if (verbose) puts("STUB: double_general_splat called");
    return NULL;
}

void* double_general_sub_scalar(void)
{
    if (verbose) puts("STUB: double_general_sub_scalar called");
    return NULL;
}

void* double_general_transpose(void)
{
    if (verbose) puts("STUB: double_general_transpose called");
    return NULL;
}

void* double_inner_product_scalar(void)
{
    if (verbose) puts("STUB: double_inner_product_scalar called");
    return NULL;
}

void* double_outer_product_scalar(void)
{
    if (verbose) puts("STUB: double_outer_product_scalar called");
    return NULL;
}

void* drot(void)
{
    if (verbose) puts("STUB: drot called");
    return NULL;
}

void* drot_(void)
{
    if (verbose) puts("STUB: drot_ called");
    return NULL;
}

void* drotg(void)
{
    if (verbose) puts("STUB: drotg called");
    return NULL;
}

void* drotg_(void)
{
    if (verbose) puts("STUB: drotg_ called");
    return NULL;
}

void* drotm(void)
{
    if (verbose) puts("STUB: drotm called");
    return NULL;
}

void* drotm_(void)
{
    if (verbose) puts("STUB: drotm_ called");
    return NULL;
}

void* drotmg(void)
{
    if (verbose) puts("STUB: drotmg called");
    return NULL;
}

void* drotmg_(void)
{
    if (verbose) puts("STUB: drotmg_ called");
    return NULL;
}

void* dsbmv(void)
{
    if (verbose) puts("STUB: dsbmv called");
    return NULL;
}

void* dsbmv_(void)
{
    if (verbose) puts("STUB: dsbmv_ called");
    return NULL;
}

void* dscal(void)
{
    if (verbose) puts("STUB: dscal called");
    return NULL;
}

void* dscal_(void)
{
    if (verbose) puts("STUB: dscal_ called");
    return NULL;
}

void* dsdot(void)
{
    if (verbose) puts("STUB: dsdot called");
    return NULL;
}

void* dsdot_(void)
{
    if (verbose) puts("STUB: dsdot_ called");
    return NULL;
}

void* dspmv(void)
{
    if (verbose) puts("STUB: dspmv called");
    return NULL;
}

void* dspmv_(void)
{
    if (verbose) puts("STUB: dspmv_ called");
    return NULL;
}

void* dspr(void)
{
    if (verbose) puts("STUB: dspr called");
    return NULL;
}

void* dspr2(void)
{
    if (verbose) puts("STUB: dspr2 called");
    return NULL;
}

void* dspr2_(void)
{
    if (verbose) puts("STUB: dspr2_ called");
    return NULL;
}

void* dspr_(void)
{
    if (verbose) puts("STUB: dspr_ called");
    return NULL;
}

void* dswap(void)
{
    if (verbose) puts("STUB: dswap called");
    return NULL;
}

void* dswap_(void)
{
    if (verbose) puts("STUB: dswap_ called");
    return NULL;
}

void* dsymm(void)
{
    if (verbose) puts("STUB: dsymm called");
    return NULL;
}

void* dsymm_(void)
{
    if (verbose) puts("STUB: dsymm_ called");
    return NULL;
}

void* dsymv(void)
{
    if (verbose) puts("STUB: dsymv called");
    return NULL;
}

void* dsymv_(void)
{
    if (verbose) puts("STUB: dsymv_ called");
    return NULL;
}

void* dsyr(void)
{
    if (verbose) puts("STUB: dsyr called");
    return NULL;
}

void* dsyr2(void)
{
    if (verbose) puts("STUB: dsyr2 called");
    return NULL;
}

void* dsyr2_(void)
{
    if (verbose) puts("STUB: dsyr2_ called");
    return NULL;
}

void* dsyr2k(void)
{
    if (verbose) puts("STUB: dsyr2k called");
    return NULL;
}

void* dsyr2k_(void)
{
    if (verbose) puts("STUB: dsyr2k_ called");
    return NULL;
}

void* dsyr_(void)
{
    if (verbose) puts("STUB: dsyr_ called");
    return NULL;
}

void* dsyrk(void)
{
    if (verbose) puts("STUB: dsyrk called");
    return NULL;
}

void* dsyrk_(void)
{
    if (verbose) puts("STUB: dsyrk_ called");
    return NULL;
}

void* dtbmv(void)
{
    if (verbose) puts("STUB: dtbmv called");
    return NULL;
}

void* dtbmv_(void)
{
    if (verbose) puts("STUB: dtbmv_ called");
    return NULL;
}

void* dtbsv(void)
{
    if (verbose) puts("STUB: dtbsv called");
    return NULL;
}

void* dtbsv_(void)
{
    if (verbose) puts("STUB: dtbsv_ called");
    return NULL;
}

void* dtpmv(void)
{
    if (verbose) puts("STUB: dtpmv called");
    return NULL;
}

void* dtpmv_(void)
{
    if (verbose) puts("STUB: dtpmv_ called");
    return NULL;
}

void* dtpsv(void)
{
    if (verbose) puts("STUB: dtpsv called");
    return NULL;
}

void* dtpsv_(void)
{
    if (verbose) puts("STUB: dtpsv_ called");
    return NULL;
}

void* dtrCopyLower(void)
{
    if (verbose) puts("STUB: dtrCopyLower called");
    return NULL;
}

void* dtrSetZeroLower(void)
{
    if (verbose) puts("STUB: dtrSetZeroLower called");
    return NULL;
}

void* dtrmm(void)
{
    if (verbose) puts("STUB: dtrmm called");
    return NULL;
}

void* dtrmm_(void)
{
    if (verbose) puts("STUB: dtrmm_ called");
    return NULL;
}

void* dtrmv(void)
{
    if (verbose) puts("STUB: dtrmv called");
    return NULL;
}

void* dtrmv_(void)
{
    if (verbose) puts("STUB: dtrmv_ called");
    return NULL;
}

void* dtrsm(void)
{
    if (verbose) puts("STUB: dtrsm called");
    return NULL;
}

void* dtrsm_(void)
{
    if (verbose) puts("STUB: dtrsm_ called");
    return NULL;
}

void* dtrsv(void)
{
    if (verbose) puts("STUB: dtrsv called");
    return NULL;
}

void* dtrsv_(void)
{
    if (verbose) puts("STUB: dtrsv_ called");
    return NULL;
}

void* dzasum(void)
{
    if (verbose) puts("STUB: dzasum called");
    return NULL;
}

void* dzasum_(void)
{
    if (verbose) puts("STUB: dzasum_ called");
    return NULL;
}

void* dznrm2(void)
{
    if (verbose) puts("STUB: dznrm2 called");
    return NULL;
}

void* dznrm2_(void)
{
    if (verbose) puts("STUB: dznrm2_ called");
    return NULL;
}

void* float_general_add(void)
{
    if (verbose) puts("STUB: float_general_add called");
    return NULL;
}

void* float_general_add_scalar(void)
{
    if (verbose) puts("STUB: float_general_add_scalar called");
    return NULL;
}

void* float_general_elementwise_product(void)
{
    if (verbose) puts("STUB: float_general_elementwise_product called");
    return NULL;
}

void* float_general_permute_cols(void)
{
    if (verbose) puts("STUB: float_general_permute_cols called");
    return NULL;
}

void* float_general_permute_rows(void)
{
    if (verbose) puts("STUB: float_general_permute_rows called");
    return NULL;
}

void* float_general_product_scalar(void)
{
    if (verbose) puts("STUB: float_general_product_scalar called");
    return NULL;
}

void* float_general_splat(void)
{
    if (verbose) puts("STUB: float_general_splat called");
    return NULL;
}

void* float_general_sub_scalar(void)
{
    if (verbose) puts("STUB: float_general_sub_scalar called");
    return NULL;
}

void* float_general_transpose(void)
{
    if (verbose) puts("STUB: float_general_transpose called");
    return NULL;
}

void* float_inner_product_scalar(void)
{
    if (verbose) puts("STUB: float_inner_product_scalar called");
    return NULL;
}

void* float_outer_product_scalar(void)
{
    if (verbose) puts("STUB: float_outer_product_scalar called");
    return NULL;
}

void* getHardwareInfo(void)
{
    if (verbose) puts("STUB: getHardwareInfo called");
    return NULL;
}

void* icamax(void)
{
    if (verbose) puts("STUB: icamax called");
    return NULL;
}

void* icamax_(void)
{
    if (verbose) puts("STUB: icamax_ called");
    return NULL;
}

void* idamax(void)
{
    if (verbose) puts("STUB: idamax called");
    return NULL;
}

void* idamax_(void)
{
    if (verbose) puts("STUB: idamax_ called");
    return NULL;
}

void* isamax(void)
{
    if (verbose) puts("STUB: isamax called");
    return NULL;
}

void* isamax_(void)
{
    if (verbose) puts("STUB: isamax_ called");
    return NULL;
}

void* izamax(void)
{
    if (verbose) puts("STUB: izamax called");
    return NULL;
}

void* izamax_(void)
{
    if (verbose) puts("STUB: izamax_ called");
    return NULL;
}

void* lsame_(void)
{
    if (verbose) puts("STUB: lsame_ called");
    return NULL;
}

void* sasum(void)
{
    if (verbose) puts("STUB: sasum called");
    return NULL;
}

void* sasum_(void)
{
    if (verbose) puts("STUB: sasum_ called");
    return NULL;
}

void* saxpy(void)
{
    if (verbose) puts("STUB: saxpy called");
    return NULL;
}

void* saxpy_(void)
{
    if (verbose) puts("STUB: saxpy_ called");
    return NULL;
}

void* scasum(void)
{
    if (verbose) puts("STUB: scasum called");
    return NULL;
}

void* scasum_(void)
{
    if (verbose) puts("STUB: scasum_ called");
    return NULL;
}

void* scnrm2(void)
{
    if (verbose) puts("STUB: scnrm2 called");
    return NULL;
}

void* scnrm2_(void)
{
    if (verbose) puts("STUB: scnrm2_ called");
    return NULL;
}

void* scopy(void)
{
    if (verbose) puts("STUB: scopy called");
    return NULL;
}

void* scopy_(void)
{
    if (verbose) puts("STUB: scopy_ called");
    return NULL;
}

void* sdot(void)
{
    if (verbose) puts("STUB: sdot called");
    return NULL;
}

void* sdot_(void)
{
    if (verbose) puts("STUB: sdot_ called");
    return NULL;
}

void* sdsdot(void)
{
    if (verbose) puts("STUB: sdsdot called");
    return NULL;
}

void* sdsdot_(void)
{
    if (verbose) puts("STUB: sdsdot_ called");
    return NULL;
}

void* sgbmv(void)
{
    if (verbose) puts("STUB: sgbmv called");
    return NULL;
}

void* sgbmv_(void)
{
    if (verbose) puts("STUB: sgbmv_ called");
    return NULL;
}

void* sgeCopy(void)
{
    if (verbose) puts("STUB: sgeCopy called");
    return NULL;
}

void* sgePack_A_NoTran(void)
{
    if (verbose) puts("STUB: sgePack_A_NoTran called");
    return NULL;
}

void* sgePack_A_Tran(void)
{
    if (verbose) puts("STUB: sgePack_A_Tran called");
    return NULL;
}

void* sgePack_B_NoTran(void)
{
    if (verbose) puts("STUB: sgePack_B_NoTran called");
    return NULL;
}

void* sgePack_B_Tran(void)
{
    if (verbose) puts("STUB: sgePack_B_Tran called");
    return NULL;
}

void* sgeSetZero(void)
{
    if (verbose) puts("STUB: sgeSetZero called");
    return NULL;
}

void* sgemm(void)
{
    if (verbose) puts("STUB: sgemm called");
    return NULL;
}

void* sgemm_(void)
{
    if (verbose) puts("STUB: sgemm_ called");
    return NULL;
}

void* sgemv(void)
{
    if (verbose) puts("STUB: sgemv called");
    return NULL;
}

void* sgemv_(void)
{
    if (verbose) puts("STUB: sgemv_ called");
    return NULL;
}

void* sger(void)
{
    if (verbose) puts("STUB: sger called");
    return NULL;
}

void* sger_(void)
{
    if (verbose) puts("STUB: sger_ called");
    return NULL;
}

void* snrm2(void)
{
    if (verbose) puts("STUB: snrm2 called");
    return NULL;
}

void* snrm2_(void)
{
    if (verbose) puts("STUB: snrm2_ called");
    return NULL;
}

void* srot(void)
{
    if (verbose) puts("STUB: srot called");
    return NULL;
}

void* srot_(void)
{
    if (verbose) puts("STUB: srot_ called");
    return NULL;
}

void* srotg(void)
{
    if (verbose) puts("STUB: srotg called");
    return NULL;
}

void* srotg_(void)
{
    if (verbose) puts("STUB: srotg_ called");
    return NULL;
}

void* srotm(void)
{
    if (verbose) puts("STUB: srotm called");
    return NULL;
}

void* srotm_(void)
{
    if (verbose) puts("STUB: srotm_ called");
    return NULL;
}

void* srotmg(void)
{
    if (verbose) puts("STUB: srotmg called");
    return NULL;
}

void* srotmg_(void)
{
    if (verbose) puts("STUB: srotmg_ called");
    return NULL;
}

void* ssbmv(void)
{
    if (verbose) puts("STUB: ssbmv called");
    return NULL;
}

void* ssbmv_(void)
{
    if (verbose) puts("STUB: ssbmv_ called");
    return NULL;
}

void* sscal(void)
{
    if (verbose) puts("STUB: sscal called");
    return NULL;
}

void* sscal_(void)
{
    if (verbose) puts("STUB: sscal_ called");
    return NULL;
}

void* sspmv(void)
{
    if (verbose) puts("STUB: sspmv called");
    return NULL;
}

void* sspmv_(void)
{
    if (verbose) puts("STUB: sspmv_ called");
    return NULL;
}

void* sspr(void)
{
    if (verbose) puts("STUB: sspr called");
    return NULL;
}

void* sspr2(void)
{
    if (verbose) puts("STUB: sspr2 called");
    return NULL;
}

void* sspr2_(void)
{
    if (verbose) puts("STUB: sspr2_ called");
    return NULL;
}

void* sspr_(void)
{
    if (verbose) puts("STUB: sspr_ called");
    return NULL;
}

void* sswap(void)
{
    if (verbose) puts("STUB: sswap called");
    return NULL;
}

void* sswap_(void)
{
    if (verbose) puts("STUB: sswap_ called");
    return NULL;
}

void* ssymm(void)
{
    if (verbose) puts("STUB: ssymm called");
    return NULL;
}

void* ssymm_(void)
{
    if (verbose) puts("STUB: ssymm_ called");
    return NULL;
}

void* ssymv(void)
{
    if (verbose) puts("STUB: ssymv called");
    return NULL;
}

void* ssymv_(void)
{
    if (verbose) puts("STUB: ssymv_ called");
    return NULL;
}

void* ssyr(void)
{
    if (verbose) puts("STUB: ssyr called");
    return NULL;
}

void* ssyr2(void)
{
    if (verbose) puts("STUB: ssyr2 called");
    return NULL;
}

void* ssyr2_(void)
{
    if (verbose) puts("STUB: ssyr2_ called");
    return NULL;
}

void* ssyr2k(void)
{
    if (verbose) puts("STUB: ssyr2k called");
    return NULL;
}

void* ssyr2k_(void)
{
    if (verbose) puts("STUB: ssyr2k_ called");
    return NULL;
}

void* ssyr_(void)
{
    if (verbose) puts("STUB: ssyr_ called");
    return NULL;
}

void* ssyrk(void)
{
    if (verbose) puts("STUB: ssyrk called");
    return NULL;
}

void* ssyrk_(void)
{
    if (verbose) puts("STUB: ssyrk_ called");
    return NULL;
}

void* stbmv(void)
{
    if (verbose) puts("STUB: stbmv called");
    return NULL;
}

void* stbmv_(void)
{
    if (verbose) puts("STUB: stbmv_ called");
    return NULL;
}

void* stbsv(void)
{
    if (verbose) puts("STUB: stbsv called");
    return NULL;
}

void* stbsv_(void)
{
    if (verbose) puts("STUB: stbsv_ called");
    return NULL;
}

void* stpmv(void)
{
    if (verbose) puts("STUB: stpmv called");
    return NULL;
}

void* stpmv_(void)
{
    if (verbose) puts("STUB: stpmv_ called");
    return NULL;
}

void* stpsv(void)
{
    if (verbose) puts("STUB: stpsv called");
    return NULL;
}

void* stpsv_(void)
{
    if (verbose) puts("STUB: stpsv_ called");
    return NULL;
}

void* strCopyLower(void)
{
    if (verbose) puts("STUB: strCopyLower called");
    return NULL;
}

void* strSetZeroLower(void)
{
    if (verbose) puts("STUB: strSetZeroLower called");
    return NULL;
}

void* strmm(void)
{
    if (verbose) puts("STUB: strmm called");
    return NULL;
}

void* strmm_(void)
{
    if (verbose) puts("STUB: strmm_ called");
    return NULL;
}

void* strmv(void)
{
    if (verbose) puts("STUB: strmv called");
    return NULL;
}

void* strmv_(void)
{
    if (verbose) puts("STUB: strmv_ called");
    return NULL;
}

void* strsm(void)
{
    if (verbose) puts("STUB: strsm called");
    return NULL;
}

void* strsm_(void)
{
    if (verbose) puts("STUB: strsm_ called");
    return NULL;
}

void* strsv(void)
{
    if (verbose) puts("STUB: strsv called");
    return NULL;
}

void* strsv_(void)
{
    if (verbose) puts("STUB: strsv_ called");
    return NULL;
}

void* xerbla(void)
{
    if (verbose) puts("STUB: xerbla called");
    return NULL;
}

void* xerbla_(void)
{
    if (verbose) puts("STUB: xerbla_ called");
    return NULL;
}

void* xerbla_array__(void)
{
    if (verbose) puts("STUB: xerbla_array__ called");
    return NULL;
}

void* zaxpy(void)
{
    if (verbose) puts("STUB: zaxpy called");
    return NULL;
}

void* zaxpy_(void)
{
    if (verbose) puts("STUB: zaxpy_ called");
    return NULL;
}

void* zcopy(void)
{
    if (verbose) puts("STUB: zcopy called");
    return NULL;
}

void* zcopy_(void)
{
    if (verbose) puts("STUB: zcopy_ called");
    return NULL;
}

void* zdotc(void)
{
    if (verbose) puts("STUB: zdotc called");
    return NULL;
}

void* zdotc_(void)
{
    if (verbose) puts("STUB: zdotc_ called");
    return NULL;
}

void* zdotu(void)
{
    if (verbose) puts("STUB: zdotu called");
    return NULL;
}

void* zdotu_(void)
{
    if (verbose) puts("STUB: zdotu_ called");
    return NULL;
}

void* zdrot(void)
{
    if (verbose) puts("STUB: zdrot called");
    return NULL;
}

void* zdrot_(void)
{
    if (verbose) puts("STUB: zdrot_ called");
    return NULL;
}

void* zdscal(void)
{
    if (verbose) puts("STUB: zdscal called");
    return NULL;
}

void* zdscal_(void)
{
    if (verbose) puts("STUB: zdscal_ called");
    return NULL;
}

void* zgbmv(void)
{
    if (verbose) puts("STUB: zgbmv called");
    return NULL;
}

void* zgbmv_(void)
{
    if (verbose) puts("STUB: zgbmv_ called");
    return NULL;
}

void* zgemm(void)
{
    if (verbose) puts("STUB: zgemm called");
    return NULL;
}

void* zgemm_(void)
{
    if (verbose) puts("STUB: zgemm_ called");
    return NULL;
}

void* zgemv(void)
{
    if (verbose) puts("STUB: zgemv called");
    return NULL;
}

void* zgemv_(void)
{
    if (verbose) puts("STUB: zgemv_ called");
    return NULL;
}

void* zgerc(void)
{
    if (verbose) puts("STUB: zgerc called");
    return NULL;
}

void* zgerc_(void)
{
    if (verbose) puts("STUB: zgerc_ called");
    return NULL;
}

void* zgeru(void)
{
    if (verbose) puts("STUB: zgeru called");
    return NULL;
}

void* zgeru_(void)
{
    if (verbose) puts("STUB: zgeru_ called");
    return NULL;
}

void* zhbmv(void)
{
    if (verbose) puts("STUB: zhbmv called");
    return NULL;
}

void* zhbmv_(void)
{
    if (verbose) puts("STUB: zhbmv_ called");
    return NULL;
}

void* zhemm(void)
{
    if (verbose) puts("STUB: zhemm called");
    return NULL;
}

void* zhemm_(void)
{
    if (verbose) puts("STUB: zhemm_ called");
    return NULL;
}

void* zhemv(void)
{
    if (verbose) puts("STUB: zhemv called");
    return NULL;
}

void* zhemv_(void)
{
    if (verbose) puts("STUB: zhemv_ called");
    return NULL;
}

void* zher(void)
{
    if (verbose) puts("STUB: zher called");
    return NULL;
}

void* zher2(void)
{
    if (verbose) puts("STUB: zher2 called");
    return NULL;
}

void* zher2_(void)
{
    if (verbose) puts("STUB: zher2_ called");
    return NULL;
}

void* zher2k(void)
{
    if (verbose) puts("STUB: zher2k called");
    return NULL;
}

void* zher2k_(void)
{
    if (verbose) puts("STUB: zher2k_ called");
    return NULL;
}

void* zher_(void)
{
    if (verbose) puts("STUB: zher_ called");
    return NULL;
}

void* zherk(void)
{
    if (verbose) puts("STUB: zherk called");
    return NULL;
}

void* zherk_(void)
{
    if (verbose) puts("STUB: zherk_ called");
    return NULL;
}

void* zhpmv(void)
{
    if (verbose) puts("STUB: zhpmv called");
    return NULL;
}

void* zhpmv_(void)
{
    if (verbose) puts("STUB: zhpmv_ called");
    return NULL;
}

void* zhpr(void)
{
    if (verbose) puts("STUB: zhpr called");
    return NULL;
}

void* zhpr2(void)
{
    if (verbose) puts("STUB: zhpr2 called");
    return NULL;
}

void* zhpr2_(void)
{
    if (verbose) puts("STUB: zhpr2_ called");
    return NULL;
}

void* zhpr_(void)
{
    if (verbose) puts("STUB: zhpr_ called");
    return NULL;
}

void* zrotg(void)
{
    if (verbose) puts("STUB: zrotg called");
    return NULL;
}

void* zrotg_(void)
{
    if (verbose) puts("STUB: zrotg_ called");
    return NULL;
}

void* zscal(void)
{
    if (verbose) puts("STUB: zscal called");
    return NULL;
}

void* zscal_(void)
{
    if (verbose) puts("STUB: zscal_ called");
    return NULL;
}

void* zswap(void)
{
    if (verbose) puts("STUB: zswap called");
    return NULL;
}

void* zswap_(void)
{
    if (verbose) puts("STUB: zswap_ called");
    return NULL;
}

void* zsymm(void)
{
    if (verbose) puts("STUB: zsymm called");
    return NULL;
}

void* zsymm_(void)
{
    if (verbose) puts("STUB: zsymm_ called");
    return NULL;
}

void* zsyr2k(void)
{
    if (verbose) puts("STUB: zsyr2k called");
    return NULL;
}

void* zsyr2k_(void)
{
    if (verbose) puts("STUB: zsyr2k_ called");
    return NULL;
}

void* zsyrk(void)
{
    if (verbose) puts("STUB: zsyrk called");
    return NULL;
}

void* zsyrk_(void)
{
    if (verbose) puts("STUB: zsyrk_ called");
    return NULL;
}

void* ztbmv(void)
{
    if (verbose) puts("STUB: ztbmv called");
    return NULL;
}

void* ztbmv_(void)
{
    if (verbose) puts("STUB: ztbmv_ called");
    return NULL;
}

void* ztbsv(void)
{
    if (verbose) puts("STUB: ztbsv called");
    return NULL;
}

void* ztbsv_(void)
{
    if (verbose) puts("STUB: ztbsv_ called");
    return NULL;
}

void* ztpmv(void)
{
    if (verbose) puts("STUB: ztpmv called");
    return NULL;
}

void* ztpmv_(void)
{
    if (verbose) puts("STUB: ztpmv_ called");
    return NULL;
}

void* ztpsv(void)
{
    if (verbose) puts("STUB: ztpsv called");
    return NULL;
}

void* ztpsv_(void)
{
    if (verbose) puts("STUB: ztpsv_ called");
    return NULL;
}

void* ztrmm(void)
{
    if (verbose) puts("STUB: ztrmm called");
    return NULL;
}

void* ztrmm_(void)
{
    if (verbose) puts("STUB: ztrmm_ called");
    return NULL;
}

void* ztrmv(void)
{
    if (verbose) puts("STUB: ztrmv called");
    return NULL;
}

void* ztrmv_(void)
{
    if (verbose) puts("STUB: ztrmv_ called");
    return NULL;
}

void* ztrsm(void)
{
    if (verbose) puts("STUB: ztrsm called");
    return NULL;
}

void* ztrsm_(void)
{
    if (verbose) puts("STUB: ztrsm_ called");
    return NULL;
}

void* ztrsv(void)
{
    if (verbose) puts("STUB: ztrsv called");
    return NULL;
}

void* ztrsv_(void)
{
    if (verbose) puts("STUB: ztrsv_ called");
    return NULL;
}
