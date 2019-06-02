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


#include </libvMisc.dylib//libvMisc.dylib.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* VVACOS(void)
{
    if (verbose) puts("STUB: VVACOS called");
    return NULL;
}

void* VVACOSF(void)
{
    if (verbose) puts("STUB: VVACOSF called");
    return NULL;
}

void* VVACOSF_(void)
{
    if (verbose) puts("STUB: VVACOSF_ called");
    return NULL;
}

void* VVACOSH(void)
{
    if (verbose) puts("STUB: VVACOSH called");
    return NULL;
}

void* VVACOSHF(void)
{
    if (verbose) puts("STUB: VVACOSHF called");
    return NULL;
}

void* VVACOSHF_(void)
{
    if (verbose) puts("STUB: VVACOSHF_ called");
    return NULL;
}

void* VVACOSH_(void)
{
    if (verbose) puts("STUB: VVACOSH_ called");
    return NULL;
}

void* VVACOS_(void)
{
    if (verbose) puts("STUB: VVACOS_ called");
    return NULL;
}

void* VVASIN(void)
{
    if (verbose) puts("STUB: VVASIN called");
    return NULL;
}

void* VVASINF(void)
{
    if (verbose) puts("STUB: VVASINF called");
    return NULL;
}

void* VVASINF_(void)
{
    if (verbose) puts("STUB: VVASINF_ called");
    return NULL;
}

void* VVASINH(void)
{
    if (verbose) puts("STUB: VVASINH called");
    return NULL;
}

void* VVASINHF(void)
{
    if (verbose) puts("STUB: VVASINHF called");
    return NULL;
}

void* VVASINHF_(void)
{
    if (verbose) puts("STUB: VVASINHF_ called");
    return NULL;
}

void* VVASINH_(void)
{
    if (verbose) puts("STUB: VVASINH_ called");
    return NULL;
}

void* VVASIN_(void)
{
    if (verbose) puts("STUB: VVASIN_ called");
    return NULL;
}

void* VVATAN(void)
{
    if (verbose) puts("STUB: VVATAN called");
    return NULL;
}

void* VVATAN2(void)
{
    if (verbose) puts("STUB: VVATAN2 called");
    return NULL;
}

void* VVATAN2F(void)
{
    if (verbose) puts("STUB: VVATAN2F called");
    return NULL;
}

void* VVATAN2F_(void)
{
    if (verbose) puts("STUB: VVATAN2F_ called");
    return NULL;
}

void* VVATAN2_(void)
{
    if (verbose) puts("STUB: VVATAN2_ called");
    return NULL;
}

void* VVATANF(void)
{
    if (verbose) puts("STUB: VVATANF called");
    return NULL;
}

void* VVATANF_(void)
{
    if (verbose) puts("STUB: VVATANF_ called");
    return NULL;
}

void* VVATANH(void)
{
    if (verbose) puts("STUB: VVATANH called");
    return NULL;
}

void* VVATANHF(void)
{
    if (verbose) puts("STUB: VVATANHF called");
    return NULL;
}

void* VVATANHF_(void)
{
    if (verbose) puts("STUB: VVATANHF_ called");
    return NULL;
}

void* VVATANH_(void)
{
    if (verbose) puts("STUB: VVATANH_ called");
    return NULL;
}

void* VVATAN_(void)
{
    if (verbose) puts("STUB: VVATAN_ called");
    return NULL;
}

void* VVCBRT(void)
{
    if (verbose) puts("STUB: VVCBRT called");
    return NULL;
}

void* VVCBRTF(void)
{
    if (verbose) puts("STUB: VVCBRTF called");
    return NULL;
}

void* VVCBRTF_(void)
{
    if (verbose) puts("STUB: VVCBRTF_ called");
    return NULL;
}

void* VVCBRT_(void)
{
    if (verbose) puts("STUB: VVCBRT_ called");
    return NULL;
}

void* VVCEIL(void)
{
    if (verbose) puts("STUB: VVCEIL called");
    return NULL;
}

void* VVCEILF(void)
{
    if (verbose) puts("STUB: VVCEILF called");
    return NULL;
}

void* VVCEILF_(void)
{
    if (verbose) puts("STUB: VVCEILF_ called");
    return NULL;
}

void* VVCEIL_(void)
{
    if (verbose) puts("STUB: VVCEIL_ called");
    return NULL;
}

void* VVCOPYSIGN(void)
{
    if (verbose) puts("STUB: VVCOPYSIGN called");
    return NULL;
}

void* VVCOPYSIGNF(void)
{
    if (verbose) puts("STUB: VVCOPYSIGNF called");
    return NULL;
}

void* VVCOPYSIGNF_(void)
{
    if (verbose) puts("STUB: VVCOPYSIGNF_ called");
    return NULL;
}

void* VVCOPYSIGN_(void)
{
    if (verbose) puts("STUB: VVCOPYSIGN_ called");
    return NULL;
}

void* VVCOS(void)
{
    if (verbose) puts("STUB: VVCOS called");
    return NULL;
}

void* VVCOSF(void)
{
    if (verbose) puts("STUB: VVCOSF called");
    return NULL;
}

void* VVCOSF_(void)
{
    if (verbose) puts("STUB: VVCOSF_ called");
    return NULL;
}

void* VVCOSH(void)
{
    if (verbose) puts("STUB: VVCOSH called");
    return NULL;
}

void* VVCOSHF(void)
{
    if (verbose) puts("STUB: VVCOSHF called");
    return NULL;
}

void* VVCOSHF_(void)
{
    if (verbose) puts("STUB: VVCOSHF_ called");
    return NULL;
}

void* VVCOSH_(void)
{
    if (verbose) puts("STUB: VVCOSH_ called");
    return NULL;
}

void* VVCOSISIN(void)
{
    if (verbose) puts("STUB: VVCOSISIN called");
    return NULL;
}

void* VVCOSISINF(void)
{
    if (verbose) puts("STUB: VVCOSISINF called");
    return NULL;
}

void* VVCOSISINF_(void)
{
    if (verbose) puts("STUB: VVCOSISINF_ called");
    return NULL;
}

void* VVCOSISIN_(void)
{
    if (verbose) puts("STUB: VVCOSISIN_ called");
    return NULL;
}

void* VVCOSPI(void)
{
    if (verbose) puts("STUB: VVCOSPI called");
    return NULL;
}

void* VVCOSPIF(void)
{
    if (verbose) puts("STUB: VVCOSPIF called");
    return NULL;
}

void* VVCOSPIF_(void)
{
    if (verbose) puts("STUB: VVCOSPIF_ called");
    return NULL;
}

void* VVCOSPI_(void)
{
    if (verbose) puts("STUB: VVCOSPI_ called");
    return NULL;
}

void* VVCOS_(void)
{
    if (verbose) puts("STUB: VVCOS_ called");
    return NULL;
}

void* VVDIV(void)
{
    if (verbose) puts("STUB: VVDIV called");
    return NULL;
}

void* VVDIVF(void)
{
    if (verbose) puts("STUB: VVDIVF called");
    return NULL;
}

void* VVDIVF_(void)
{
    if (verbose) puts("STUB: VVDIVF_ called");
    return NULL;
}

void* VVDIV_(void)
{
    if (verbose) puts("STUB: VVDIV_ called");
    return NULL;
}

void* VVEXP(void)
{
    if (verbose) puts("STUB: VVEXP called");
    return NULL;
}

void* VVEXP2(void)
{
    if (verbose) puts("STUB: VVEXP2 called");
    return NULL;
}

void* VVEXP2F(void)
{
    if (verbose) puts("STUB: VVEXP2F called");
    return NULL;
}

void* VVEXP2F_(void)
{
    if (verbose) puts("STUB: VVEXP2F_ called");
    return NULL;
}

void* VVEXP2_(void)
{
    if (verbose) puts("STUB: VVEXP2_ called");
    return NULL;
}

void* VVEXPF(void)
{
    if (verbose) puts("STUB: VVEXPF called");
    return NULL;
}

void* VVEXPF_(void)
{
    if (verbose) puts("STUB: VVEXPF_ called");
    return NULL;
}

void* VVEXPM1(void)
{
    if (verbose) puts("STUB: VVEXPM1 called");
    return NULL;
}

void* VVEXPM1F(void)
{
    if (verbose) puts("STUB: VVEXPM1F called");
    return NULL;
}

void* VVEXPM1F_(void)
{
    if (verbose) puts("STUB: VVEXPM1F_ called");
    return NULL;
}

void* VVEXPM1_(void)
{
    if (verbose) puts("STUB: VVEXPM1_ called");
    return NULL;
}

void* VVEXP_(void)
{
    if (verbose) puts("STUB: VVEXP_ called");
    return NULL;
}

void* VVFABF(void)
{
    if (verbose) puts("STUB: VVFABF called");
    return NULL;
}

void* VVFABF_(void)
{
    if (verbose) puts("STUB: VVFABF_ called");
    return NULL;
}

void* VVFABS(void)
{
    if (verbose) puts("STUB: VVFABS called");
    return NULL;
}

void* VVFABSF(void)
{
    if (verbose) puts("STUB: VVFABSF called");
    return NULL;
}

void* VVFABSF_(void)
{
    if (verbose) puts("STUB: VVFABSF_ called");
    return NULL;
}

void* VVFABS_(void)
{
    if (verbose) puts("STUB: VVFABS_ called");
    return NULL;
}

void* VVFLOOR(void)
{
    if (verbose) puts("STUB: VVFLOOR called");
    return NULL;
}

void* VVFLOORF(void)
{
    if (verbose) puts("STUB: VVFLOORF called");
    return NULL;
}

void* VVFLOORF_(void)
{
    if (verbose) puts("STUB: VVFLOORF_ called");
    return NULL;
}

void* VVFLOOR_(void)
{
    if (verbose) puts("STUB: VVFLOOR_ called");
    return NULL;
}

void* VVFMOD(void)
{
    if (verbose) puts("STUB: VVFMOD called");
    return NULL;
}

void* VVFMODF(void)
{
    if (verbose) puts("STUB: VVFMODF called");
    return NULL;
}

void* VVFMODF_(void)
{
    if (verbose) puts("STUB: VVFMODF_ called");
    return NULL;
}

void* VVFMOD_(void)
{
    if (verbose) puts("STUB: VVFMOD_ called");
    return NULL;
}

void* VVINT(void)
{
    if (verbose) puts("STUB: VVINT called");
    return NULL;
}

void* VVINTF(void)
{
    if (verbose) puts("STUB: VVINTF called");
    return NULL;
}

void* VVINTF_(void)
{
    if (verbose) puts("STUB: VVINTF_ called");
    return NULL;
}

void* VVINT_(void)
{
    if (verbose) puts("STUB: VVINT_ called");
    return NULL;
}

void* VVLOG(void)
{
    if (verbose) puts("STUB: VVLOG called");
    return NULL;
}

void* VVLOG10(void)
{
    if (verbose) puts("STUB: VVLOG10 called");
    return NULL;
}

void* VVLOG10F(void)
{
    if (verbose) puts("STUB: VVLOG10F called");
    return NULL;
}

void* VVLOG10F_(void)
{
    if (verbose) puts("STUB: VVLOG10F_ called");
    return NULL;
}

void* VVLOG10_(void)
{
    if (verbose) puts("STUB: VVLOG10_ called");
    return NULL;
}

void* VVLOG1P(void)
{
    if (verbose) puts("STUB: VVLOG1P called");
    return NULL;
}

void* VVLOG1PF(void)
{
    if (verbose) puts("STUB: VVLOG1PF called");
    return NULL;
}

void* VVLOG1PF_(void)
{
    if (verbose) puts("STUB: VVLOG1PF_ called");
    return NULL;
}

void* VVLOG1P_(void)
{
    if (verbose) puts("STUB: VVLOG1P_ called");
    return NULL;
}

void* VVLOG2(void)
{
    if (verbose) puts("STUB: VVLOG2 called");
    return NULL;
}

void* VVLOG2F(void)
{
    if (verbose) puts("STUB: VVLOG2F called");
    return NULL;
}

void* VVLOG2F_(void)
{
    if (verbose) puts("STUB: VVLOG2F_ called");
    return NULL;
}

void* VVLOG2_(void)
{
    if (verbose) puts("STUB: VVLOG2_ called");
    return NULL;
}

void* VVLOGB(void)
{
    if (verbose) puts("STUB: VVLOGB called");
    return NULL;
}

void* VVLOGBF(void)
{
    if (verbose) puts("STUB: VVLOGBF called");
    return NULL;
}

void* VVLOGBF_(void)
{
    if (verbose) puts("STUB: VVLOGBF_ called");
    return NULL;
}

void* VVLOGB_(void)
{
    if (verbose) puts("STUB: VVLOGB_ called");
    return NULL;
}

void* VVLOGF(void)
{
    if (verbose) puts("STUB: VVLOGF called");
    return NULL;
}

void* VVLOGF_(void)
{
    if (verbose) puts("STUB: VVLOGF_ called");
    return NULL;
}

void* VVLOG_(void)
{
    if (verbose) puts("STUB: VVLOG_ called");
    return NULL;
}

void* VVNEXTAFTER(void)
{
    if (verbose) puts("STUB: VVNEXTAFTER called");
    return NULL;
}

void* VVNEXTAFTERF(void)
{
    if (verbose) puts("STUB: VVNEXTAFTERF called");
    return NULL;
}

void* VVNEXTAFTERF_(void)
{
    if (verbose) puts("STUB: VVNEXTAFTERF_ called");
    return NULL;
}

void* VVNEXTAFTER_(void)
{
    if (verbose) puts("STUB: VVNEXTAFTER_ called");
    return NULL;
}

void* VVNINT(void)
{
    if (verbose) puts("STUB: VVNINT called");
    return NULL;
}

void* VVNINTF(void)
{
    if (verbose) puts("STUB: VVNINTF called");
    return NULL;
}

void* VVNINTF_(void)
{
    if (verbose) puts("STUB: VVNINTF_ called");
    return NULL;
}

void* VVNINT_(void)
{
    if (verbose) puts("STUB: VVNINT_ called");
    return NULL;
}

void* VVPOW(void)
{
    if (verbose) puts("STUB: VVPOW called");
    return NULL;
}

void* VVPOWF(void)
{
    if (verbose) puts("STUB: VVPOWF called");
    return NULL;
}

void* VVPOWF_(void)
{
    if (verbose) puts("STUB: VVPOWF_ called");
    return NULL;
}

void* VVPOWS(void)
{
    if (verbose) puts("STUB: VVPOWS called");
    return NULL;
}

void* VVPOWSF(void)
{
    if (verbose) puts("STUB: VVPOWSF called");
    return NULL;
}

void* VVPOWSF_(void)
{
    if (verbose) puts("STUB: VVPOWSF_ called");
    return NULL;
}

void* VVPOWS_(void)
{
    if (verbose) puts("STUB: VVPOWS_ called");
    return NULL;
}

void* VVPOW_(void)
{
    if (verbose) puts("STUB: VVPOW_ called");
    return NULL;
}

void* VVREC(void)
{
    if (verbose) puts("STUB: VVREC called");
    return NULL;
}

void* VVRECF(void)
{
    if (verbose) puts("STUB: VVRECF called");
    return NULL;
}

void* VVRECF_(void)
{
    if (verbose) puts("STUB: VVRECF_ called");
    return NULL;
}

void* VVREC_(void)
{
    if (verbose) puts("STUB: VVREC_ called");
    return NULL;
}

void* VVREMAINDER(void)
{
    if (verbose) puts("STUB: VVREMAINDER called");
    return NULL;
}

void* VVREMAINDERF(void)
{
    if (verbose) puts("STUB: VVREMAINDERF called");
    return NULL;
}

void* VVREMAINDERF_(void)
{
    if (verbose) puts("STUB: VVREMAINDERF_ called");
    return NULL;
}

void* VVREMAINDER_(void)
{
    if (verbose) puts("STUB: VVREMAINDER_ called");
    return NULL;
}

void* VVRSQRT(void)
{
    if (verbose) puts("STUB: VVRSQRT called");
    return NULL;
}

void* VVRSQRTF(void)
{
    if (verbose) puts("STUB: VVRSQRTF called");
    return NULL;
}

void* VVRSQRTF_(void)
{
    if (verbose) puts("STUB: VVRSQRTF_ called");
    return NULL;
}

void* VVRSQRT_(void)
{
    if (verbose) puts("STUB: VVRSQRT_ called");
    return NULL;
}

void* VVSIN(void)
{
    if (verbose) puts("STUB: VVSIN called");
    return NULL;
}

void* VVSINCOS(void)
{
    if (verbose) puts("STUB: VVSINCOS called");
    return NULL;
}

void* VVSINCOSF(void)
{
    if (verbose) puts("STUB: VVSINCOSF called");
    return NULL;
}

void* VVSINCOSF_(void)
{
    if (verbose) puts("STUB: VVSINCOSF_ called");
    return NULL;
}

void* VVSINCOS_(void)
{
    if (verbose) puts("STUB: VVSINCOS_ called");
    return NULL;
}

void* VVSINF(void)
{
    if (verbose) puts("STUB: VVSINF called");
    return NULL;
}

void* VVSINF_(void)
{
    if (verbose) puts("STUB: VVSINF_ called");
    return NULL;
}

void* VVSINH(void)
{
    if (verbose) puts("STUB: VVSINH called");
    return NULL;
}

void* VVSINHF(void)
{
    if (verbose) puts("STUB: VVSINHF called");
    return NULL;
}

void* VVSINHF_(void)
{
    if (verbose) puts("STUB: VVSINHF_ called");
    return NULL;
}

void* VVSINH_(void)
{
    if (verbose) puts("STUB: VVSINH_ called");
    return NULL;
}

void* VVSINPI(void)
{
    if (verbose) puts("STUB: VVSINPI called");
    return NULL;
}

void* VVSINPIF(void)
{
    if (verbose) puts("STUB: VVSINPIF called");
    return NULL;
}

void* VVSINPIF_(void)
{
    if (verbose) puts("STUB: VVSINPIF_ called");
    return NULL;
}

void* VVSINPI_(void)
{
    if (verbose) puts("STUB: VVSINPI_ called");
    return NULL;
}

void* VVSIN_(void)
{
    if (verbose) puts("STUB: VVSIN_ called");
    return NULL;
}

void* VVSQRT(void)
{
    if (verbose) puts("STUB: VVSQRT called");
    return NULL;
}

void* VVSQRTF(void)
{
    if (verbose) puts("STUB: VVSQRTF called");
    return NULL;
}

void* VVSQRTF_(void)
{
    if (verbose) puts("STUB: VVSQRTF_ called");
    return NULL;
}

void* VVSQRT_(void)
{
    if (verbose) puts("STUB: VVSQRT_ called");
    return NULL;
}

void* VVTAN(void)
{
    if (verbose) puts("STUB: VVTAN called");
    return NULL;
}

void* VVTANF(void)
{
    if (verbose) puts("STUB: VVTANF called");
    return NULL;
}

void* VVTANF_(void)
{
    if (verbose) puts("STUB: VVTANF_ called");
    return NULL;
}

void* VVTANH(void)
{
    if (verbose) puts("STUB: VVTANH called");
    return NULL;
}

void* VVTANHF(void)
{
    if (verbose) puts("STUB: VVTANHF called");
    return NULL;
}

void* VVTANHF_(void)
{
    if (verbose) puts("STUB: VVTANHF_ called");
    return NULL;
}

void* VVTANH_(void)
{
    if (verbose) puts("STUB: VVTANH_ called");
    return NULL;
}

void* VVTANPI(void)
{
    if (verbose) puts("STUB: VVTANPI called");
    return NULL;
}

void* VVTANPIF(void)
{
    if (verbose) puts("STUB: VVTANPIF called");
    return NULL;
}

void* VVTANPIF_(void)
{
    if (verbose) puts("STUB: VVTANPIF_ called");
    return NULL;
}

void* VVTANPI_(void)
{
    if (verbose) puts("STUB: VVTANPI_ called");
    return NULL;
}

void* VVTAN_(void)
{
    if (verbose) puts("STUB: VVTAN_ called");
    return NULL;
}

void* __cblas_isamax(void)
{
    if (verbose) puts("STUB: __cblas_isamax called");
    return NULL;
}

void* __cblas_sasum(void)
{
    if (verbose) puts("STUB: __cblas_sasum called");
    return NULL;
}

void* __cblas_saxpy(void)
{
    if (verbose) puts("STUB: __cblas_saxpy called");
    return NULL;
}

void* __cblas_sgemm(void)
{
    if (verbose) puts("STUB: __cblas_sgemm called");
    return NULL;
}

void* __cblas_sgemv(void)
{
    if (verbose) puts("STUB: __cblas_sgemv called");
    return NULL;
}

void* __cblas_srot(void)
{
    if (verbose) puts("STUB: __cblas_srot called");
    return NULL;
}

void* __cblas_sscal(void)
{
    if (verbose) puts("STUB: __cblas_sscal called");
    return NULL;
}

void* __cblas_sswap(void)
{
    if (verbose) puts("STUB: __cblas_sswap called");
    return NULL;
}

void* dMultMatMat_16x16(void)
{
    if (verbose) puts("STUB: dMultMatMat_16x16 called");
    return NULL;
}

void* dMultMatMat_32x32(void)
{
    if (verbose) puts("STUB: dMultMatMat_32x32 called");
    return NULL;
}

void* dMultMatMat_4x4(void)
{
    if (verbose) puts("STUB: dMultMatMat_4x4 called");
    return NULL;
}

void* dMultMatMat_8x8(void)
{
    if (verbose) puts("STUB: dMultMatMat_8x8 called");
    return NULL;
}

void* dMultMatVec_16x16(void)
{
    if (verbose) puts("STUB: dMultMatVec_16x16 called");
    return NULL;
}

void* dMultMatVec_32x32(void)
{
    if (verbose) puts("STUB: dMultMatVec_32x32 called");
    return NULL;
}

void* dMultMatVec_4x4(void)
{
    if (verbose) puts("STUB: dMultMatVec_4x4 called");
    return NULL;
}

void* dMultMatVec_8x8(void)
{
    if (verbose) puts("STUB: dMultMatVec_8x8 called");
    return NULL;
}

void* dMultVecMat_16x16(void)
{
    if (verbose) puts("STUB: dMultVecMat_16x16 called");
    return NULL;
}

void* dMultVecMat_32x32(void)
{
    if (verbose) puts("STUB: dMultVecMat_32x32 called");
    return NULL;
}

void* dMultVecMat_4x4(void)
{
    if (verbose) puts("STUB: dMultVecMat_4x4 called");
    return NULL;
}

void* dMultVecMat_8x8(void)
{
    if (verbose) puts("STUB: dMultVecMat_8x8 called");
    return NULL;
}

void* sMultMatMat_16x16(void)
{
    if (verbose) puts("STUB: sMultMatMat_16x16 called");
    return NULL;
}

void* sMultMatMat_32x32(void)
{
    if (verbose) puts("STUB: sMultMatMat_32x32 called");
    return NULL;
}

void* sMultMatMat_4x4(void)
{
    if (verbose) puts("STUB: sMultMatMat_4x4 called");
    return NULL;
}

void* sMultMatMat_8x8(void)
{
    if (verbose) puts("STUB: sMultMatMat_8x8 called");
    return NULL;
}

void* sMultMatVec_16x16(void)
{
    if (verbose) puts("STUB: sMultMatVec_16x16 called");
    return NULL;
}

void* sMultMatVec_32x32(void)
{
    if (verbose) puts("STUB: sMultMatVec_32x32 called");
    return NULL;
}

void* sMultMatVec_4x4(void)
{
    if (verbose) puts("STUB: sMultMatVec_4x4 called");
    return NULL;
}

void* sMultMatVec_8x8(void)
{
    if (verbose) puts("STUB: sMultMatVec_8x8 called");
    return NULL;
}

void* sMultVecMat_16x16(void)
{
    if (verbose) puts("STUB: sMultVecMat_16x16 called");
    return NULL;
}

void* sMultVecMat_32x32(void)
{
    if (verbose) puts("STUB: sMultVecMat_32x32 called");
    return NULL;
}

void* sMultVecMat_4x4(void)
{
    if (verbose) puts("STUB: sMultVecMat_4x4 called");
    return NULL;
}

void* sMultVecMat_8x8(void)
{
    if (verbose) puts("STUB: sMultVecMat_8x8 called");
    return NULL;
}

void* vA1024Shift(void)
{
    if (verbose) puts("STUB: vA1024Shift called");
    return NULL;
}

void* vA128Shift(void)
{
    if (verbose) puts("STUB: vA128Shift called");
    return NULL;
}

void* vA256Shift(void)
{
    if (verbose) puts("STUB: vA256Shift called");
    return NULL;
}

void* vA512Shift(void)
{
    if (verbose) puts("STUB: vA512Shift called");
    return NULL;
}

void* vA64Shift(void)
{
    if (verbose) puts("STUB: vA64Shift called");
    return NULL;
}

void* vA64Shift2(void)
{
    if (verbose) puts("STUB: vA64Shift2 called");
    return NULL;
}

void* vIsamax(void)
{
    if (verbose) puts("STUB: vIsamax called");
    return NULL;
}

void* vIsamin(void)
{
    if (verbose) puts("STUB: vIsamin called");
    return NULL;
}

void* vIsmax(void)
{
    if (verbose) puts("STUB: vIsmax called");
    return NULL;
}

void* vIsmin(void)
{
    if (verbose) puts("STUB: vIsmin called");
    return NULL;
}

void* vL1024Rotate(void)
{
    if (verbose) puts("STUB: vL1024Rotate called");
    return NULL;
}

void* vL128Rotate(void)
{
    if (verbose) puts("STUB: vL128Rotate called");
    return NULL;
}

void* vL256Rotate(void)
{
    if (verbose) puts("STUB: vL256Rotate called");
    return NULL;
}

void* vL512Rotate(void)
{
    if (verbose) puts("STUB: vL512Rotate called");
    return NULL;
}

void* vL64Rotate(void)
{
    if (verbose) puts("STUB: vL64Rotate called");
    return NULL;
}

void* vL64Rotate2(void)
{
    if (verbose) puts("STUB: vL64Rotate2 called");
    return NULL;
}

void* vLL1024Shift(void)
{
    if (verbose) puts("STUB: vLL1024Shift called");
    return NULL;
}

void* vLL128Shift(void)
{
    if (verbose) puts("STUB: vLL128Shift called");
    return NULL;
}

void* vLL256Shift(void)
{
    if (verbose) puts("STUB: vLL256Shift called");
    return NULL;
}

void* vLL512Shift(void)
{
    if (verbose) puts("STUB: vLL512Shift called");
    return NULL;
}

void* vLL64Shift(void)
{
    if (verbose) puts("STUB: vLL64Shift called");
    return NULL;
}

void* vLL64Shift2(void)
{
    if (verbose) puts("STUB: vLL64Shift2 called");
    return NULL;
}

void* vLR1024Shift(void)
{
    if (verbose) puts("STUB: vLR1024Shift called");
    return NULL;
}

void* vLR128Shift(void)
{
    if (verbose) puts("STUB: vLR128Shift called");
    return NULL;
}

void* vLR256Shift(void)
{
    if (verbose) puts("STUB: vLR256Shift called");
    return NULL;
}

void* vLR512Shift(void)
{
    if (verbose) puts("STUB: vLR512Shift called");
    return NULL;
}

void* vLR64Shift(void)
{
    if (verbose) puts("STUB: vLR64Shift called");
    return NULL;
}

void* vLR64Shift2(void)
{
    if (verbose) puts("STUB: vLR64Shift2 called");
    return NULL;
}

void* vMultMatMat_16x16(void)
{
    if (verbose) puts("STUB: vMultMatMat_16x16 called");
    return NULL;
}

void* vMultMatMat_32x32(void)
{
    if (verbose) puts("STUB: vMultMatMat_32x32 called");
    return NULL;
}

void* vMultMatMat_4x4(void)
{
    if (verbose) puts("STUB: vMultMatMat_4x4 called");
    return NULL;
}

void* vMultMatMat_8x8(void)
{
    if (verbose) puts("STUB: vMultMatMat_8x8 called");
    return NULL;
}

void* vMultMatVec_16x16(void)
{
    if (verbose) puts("STUB: vMultMatVec_16x16 called");
    return NULL;
}

void* vMultMatVec_32x32(void)
{
    if (verbose) puts("STUB: vMultMatVec_32x32 called");
    return NULL;
}

void* vMultMatVec_4x4(void)
{
    if (verbose) puts("STUB: vMultMatVec_4x4 called");
    return NULL;
}

void* vMultMatVec_8x8(void)
{
    if (verbose) puts("STUB: vMultMatVec_8x8 called");
    return NULL;
}

void* vMultVecMat_16x16(void)
{
    if (verbose) puts("STUB: vMultVecMat_16x16 called");
    return NULL;
}

void* vMultVecMat_32x32(void)
{
    if (verbose) puts("STUB: vMultVecMat_32x32 called");
    return NULL;
}

void* vMultVecMat_4x4(void)
{
    if (verbose) puts("STUB: vMultVecMat_4x4 called");
    return NULL;
}

void* vMultVecMat_8x8(void)
{
    if (verbose) puts("STUB: vMultVecMat_8x8 called");
    return NULL;
}

void* vR1024Rotate(void)
{
    if (verbose) puts("STUB: vR1024Rotate called");
    return NULL;
}

void* vR128Rotate(void)
{
    if (verbose) puts("STUB: vR128Rotate called");
    return NULL;
}

void* vR256Rotate(void)
{
    if (verbose) puts("STUB: vR256Rotate called");
    return NULL;
}

void* vR512Rotate(void)
{
    if (verbose) puts("STUB: vR512Rotate called");
    return NULL;
}

void* vR64Rotate(void)
{
    if (verbose) puts("STUB: vR64Rotate called");
    return NULL;
}

void* vR64Rotate2(void)
{
    if (verbose) puts("STUB: vR64Rotate2 called");
    return NULL;
}

void* vS1024Add(void)
{
    if (verbose) puts("STUB: vS1024Add called");
    return NULL;
}

void* vS1024AddS(void)
{
    if (verbose) puts("STUB: vS1024AddS called");
    return NULL;
}

void* vS1024Divide(void)
{
    if (verbose) puts("STUB: vS1024Divide called");
    return NULL;
}

void* vS1024HalfMultiply(void)
{
    if (verbose) puts("STUB: vS1024HalfMultiply called");
    return NULL;
}

void* vS1024Mod(void)
{
    if (verbose) puts("STUB: vS1024Mod called");
    return NULL;
}

void* vS1024Neg(void)
{
    if (verbose) puts("STUB: vS1024Neg called");
    return NULL;
}

void* vS1024Sub(void)
{
    if (verbose) puts("STUB: vS1024Sub called");
    return NULL;
}

void* vS1024SubS(void)
{
    if (verbose) puts("STUB: vS1024SubS called");
    return NULL;
}

void* vS128Add(void)
{
    if (verbose) puts("STUB: vS128Add called");
    return NULL;
}

void* vS128AddS(void)
{
    if (verbose) puts("STUB: vS128AddS called");
    return NULL;
}

void* vS128Divide(void)
{
    if (verbose) puts("STUB: vS128Divide called");
    return NULL;
}

void* vS128FullMultiply(void)
{
    if (verbose) puts("STUB: vS128FullMultiply called");
    return NULL;
}

void* vS128HalfMultiply(void)
{
    if (verbose) puts("STUB: vS128HalfMultiply called");
    return NULL;
}

void* vS128Neg(void)
{
    if (verbose) puts("STUB: vS128Neg called");
    return NULL;
}

void* vS128Sub(void)
{
    if (verbose) puts("STUB: vS128Sub called");
    return NULL;
}

void* vS128SubS(void)
{
    if (verbose) puts("STUB: vS128SubS called");
    return NULL;
}

void* vS16Divide(void)
{
    if (verbose) puts("STUB: vS16Divide called");
    return NULL;
}

void* vS16HalfMultiply(void)
{
    if (verbose) puts("STUB: vS16HalfMultiply called");
    return NULL;
}

void* vS256Add(void)
{
    if (verbose) puts("STUB: vS256Add called");
    return NULL;
}

void* vS256AddS(void)
{
    if (verbose) puts("STUB: vS256AddS called");
    return NULL;
}

void* vS256Divide(void)
{
    if (verbose) puts("STUB: vS256Divide called");
    return NULL;
}

void* vS256FullMultiply(void)
{
    if (verbose) puts("STUB: vS256FullMultiply called");
    return NULL;
}

void* vS256HalfMultiply(void)
{
    if (verbose) puts("STUB: vS256HalfMultiply called");
    return NULL;
}

void* vS256Mod(void)
{
    if (verbose) puts("STUB: vS256Mod called");
    return NULL;
}

void* vS256Neg(void)
{
    if (verbose) puts("STUB: vS256Neg called");
    return NULL;
}

void* vS256Sub(void)
{
    if (verbose) puts("STUB: vS256Sub called");
    return NULL;
}

void* vS256SubS(void)
{
    if (verbose) puts("STUB: vS256SubS called");
    return NULL;
}

void* vS32Divide(void)
{
    if (verbose) puts("STUB: vS32Divide called");
    return NULL;
}

void* vS32FullMulEven(void)
{
    if (verbose) puts("STUB: vS32FullMulEven called");
    return NULL;
}

void* vS32FullMulOdd(void)
{
    if (verbose) puts("STUB: vS32FullMulOdd called");
    return NULL;
}

void* vS32HalfMultiply(void)
{
    if (verbose) puts("STUB: vS32HalfMultiply called");
    return NULL;
}

void* vS512Add(void)
{
    if (verbose) puts("STUB: vS512Add called");
    return NULL;
}

void* vS512AddS(void)
{
    if (verbose) puts("STUB: vS512AddS called");
    return NULL;
}

void* vS512Divide(void)
{
    if (verbose) puts("STUB: vS512Divide called");
    return NULL;
}

void* vS512FullMultiply(void)
{
    if (verbose) puts("STUB: vS512FullMultiply called");
    return NULL;
}

void* vS512HalfMultiply(void)
{
    if (verbose) puts("STUB: vS512HalfMultiply called");
    return NULL;
}

void* vS512Mod(void)
{
    if (verbose) puts("STUB: vS512Mod called");
    return NULL;
}

void* vS512Neg(void)
{
    if (verbose) puts("STUB: vS512Neg called");
    return NULL;
}

void* vS512Sub(void)
{
    if (verbose) puts("STUB: vS512Sub called");
    return NULL;
}

void* vS512SubS(void)
{
    if (verbose) puts("STUB: vS512SubS called");
    return NULL;
}

void* vS64Add(void)
{
    if (verbose) puts("STUB: vS64Add called");
    return NULL;
}

void* vS64AddS(void)
{
    if (verbose) puts("STUB: vS64AddS called");
    return NULL;
}

void* vS64Divide(void)
{
    if (verbose) puts("STUB: vS64Divide called");
    return NULL;
}

void* vS64FullMulEven(void)
{
    if (verbose) puts("STUB: vS64FullMulEven called");
    return NULL;
}

void* vS64FullMulOdd(void)
{
    if (verbose) puts("STUB: vS64FullMulOdd called");
    return NULL;
}

void* vS64HalfMultiply(void)
{
    if (verbose) puts("STUB: vS64HalfMultiply called");
    return NULL;
}

void* vS64Neg(void)
{
    if (verbose) puts("STUB: vS64Neg called");
    return NULL;
}

void* vS64Sub(void)
{
    if (verbose) puts("STUB: vS64Sub called");
    return NULL;
}

void* vS64SubS(void)
{
    if (verbose) puts("STUB: vS64SubS called");
    return NULL;
}

void* vS8Divide(void)
{
    if (verbose) puts("STUB: vS8Divide called");
    return NULL;
}

void* vS8HalfMultiply(void)
{
    if (verbose) puts("STUB: vS8HalfMultiply called");
    return NULL;
}

void* vSasum(void)
{
    if (verbose) puts("STUB: vSasum called");
    return NULL;
}

void* vSaxpy(void)
{
    if (verbose) puts("STUB: vSaxpy called");
    return NULL;
}

void* vScopy(void)
{
    if (verbose) puts("STUB: vScopy called");
    return NULL;
}

void* vSdot(void)
{
    if (verbose) puts("STUB: vSdot called");
    return NULL;
}

void* vSgeadd(void)
{
    if (verbose) puts("STUB: vSgeadd called");
    return NULL;
}

void* vSgemm(void)
{
    if (verbose) puts("STUB: vSgemm called");
    return NULL;
}

void* vSgemtx(void)
{
    if (verbose) puts("STUB: vSgemtx called");
    return NULL;
}

void* vSgemul(void)
{
    if (verbose) puts("STUB: vSgemul called");
    return NULL;
}

void* vSgemv(void)
{
    if (verbose) puts("STUB: vSgemv called");
    return NULL;
}

void* vSgemx(void)
{
    if (verbose) puts("STUB: vSgemx called");
    return NULL;
}

void* vSgesub(void)
{
    if (verbose) puts("STUB: vSgesub called");
    return NULL;
}

void* vSgetmi(void)
{
    if (verbose) puts("STUB: vSgetmi called");
    return NULL;
}

void* vSgetmo(void)
{
    if (verbose) puts("STUB: vSgetmo called");
    return NULL;
}

void* vSgevv(void)
{
    if (verbose) puts("STUB: vSgevv called");
    return NULL;
}

void* vSnaxpy(void)
{
    if (verbose) puts("STUB: vSnaxpy called");
    return NULL;
}

void* vSndot(void)
{
    if (verbose) puts("STUB: vSndot called");
    return NULL;
}

void* vSnorm2(void)
{
    if (verbose) puts("STUB: vSnorm2 called");
    return NULL;
}

void* vSnrm2(void)
{
    if (verbose) puts("STUB: vSnrm2 called");
    return NULL;
}

void* vSrot(void)
{
    if (verbose) puts("STUB: vSrot called");
    return NULL;
}

void* vSscal(void)
{
    if (verbose) puts("STUB: vSscal called");
    return NULL;
}

void* vSsum(void)
{
    if (verbose) puts("STUB: vSsum called");
    return NULL;
}

void* vSswap(void)
{
    if (verbose) puts("STUB: vSswap called");
    return NULL;
}

void* vSyax(void)
{
    if (verbose) puts("STUB: vSyax called");
    return NULL;
}

void* vSzaxpy(void)
{
    if (verbose) puts("STUB: vSzaxpy called");
    return NULL;
}

void* vU1024Add(void)
{
    if (verbose) puts("STUB: vU1024Add called");
    return NULL;
}

void* vU1024AddS(void)
{
    if (verbose) puts("STUB: vU1024AddS called");
    return NULL;
}

void* vU1024Divide(void)
{
    if (verbose) puts("STUB: vU1024Divide called");
    return NULL;
}

void* vU1024HalfMultiply(void)
{
    if (verbose) puts("STUB: vU1024HalfMultiply called");
    return NULL;
}

void* vU1024Mod(void)
{
    if (verbose) puts("STUB: vU1024Mod called");
    return NULL;
}

void* vU1024Neg(void)
{
    if (verbose) puts("STUB: vU1024Neg called");
    return NULL;
}

void* vU1024Sub(void)
{
    if (verbose) puts("STUB: vU1024Sub called");
    return NULL;
}

void* vU1024SubS(void)
{
    if (verbose) puts("STUB: vU1024SubS called");
    return NULL;
}

void* vU128Add(void)
{
    if (verbose) puts("STUB: vU128Add called");
    return NULL;
}

void* vU128AddS(void)
{
    if (verbose) puts("STUB: vU128AddS called");
    return NULL;
}

void* vU128Divide(void)
{
    if (verbose) puts("STUB: vU128Divide called");
    return NULL;
}

void* vU128FullMultiply(void)
{
    if (verbose) puts("STUB: vU128FullMultiply called");
    return NULL;
}

void* vU128HalfMultiply(void)
{
    if (verbose) puts("STUB: vU128HalfMultiply called");
    return NULL;
}

void* vU128Neg(void)
{
    if (verbose) puts("STUB: vU128Neg called");
    return NULL;
}

void* vU128Sub(void)
{
    if (verbose) puts("STUB: vU128Sub called");
    return NULL;
}

void* vU128SubS(void)
{
    if (verbose) puts("STUB: vU128SubS called");
    return NULL;
}

void* vU16Divide(void)
{
    if (verbose) puts("STUB: vU16Divide called");
    return NULL;
}

void* vU16HalfMultiply(void)
{
    if (verbose) puts("STUB: vU16HalfMultiply called");
    return NULL;
}

void* vU256Add(void)
{
    if (verbose) puts("STUB: vU256Add called");
    return NULL;
}

void* vU256AddS(void)
{
    if (verbose) puts("STUB: vU256AddS called");
    return NULL;
}

void* vU256Divide(void)
{
    if (verbose) puts("STUB: vU256Divide called");
    return NULL;
}

void* vU256FullMultiply(void)
{
    if (verbose) puts("STUB: vU256FullMultiply called");
    return NULL;
}

void* vU256HalfMultiply(void)
{
    if (verbose) puts("STUB: vU256HalfMultiply called");
    return NULL;
}

void* vU256Mod(void)
{
    if (verbose) puts("STUB: vU256Mod called");
    return NULL;
}

void* vU256Neg(void)
{
    if (verbose) puts("STUB: vU256Neg called");
    return NULL;
}

void* vU256Sub(void)
{
    if (verbose) puts("STUB: vU256Sub called");
    return NULL;
}

void* vU256SubS(void)
{
    if (verbose) puts("STUB: vU256SubS called");
    return NULL;
}

void* vU32Divide(void)
{
    if (verbose) puts("STUB: vU32Divide called");
    return NULL;
}

void* vU32FullMulEven(void)
{
    if (verbose) puts("STUB: vU32FullMulEven called");
    return NULL;
}

void* vU32FullMulOdd(void)
{
    if (verbose) puts("STUB: vU32FullMulOdd called");
    return NULL;
}

void* vU32HalfMultiply(void)
{
    if (verbose) puts("STUB: vU32HalfMultiply called");
    return NULL;
}

void* vU512Add(void)
{
    if (verbose) puts("STUB: vU512Add called");
    return NULL;
}

void* vU512AddS(void)
{
    if (verbose) puts("STUB: vU512AddS called");
    return NULL;
}

void* vU512Divide(void)
{
    if (verbose) puts("STUB: vU512Divide called");
    return NULL;
}

void* vU512FullMultiply(void)
{
    if (verbose) puts("STUB: vU512FullMultiply called");
    return NULL;
}

void* vU512HalfMultiply(void)
{
    if (verbose) puts("STUB: vU512HalfMultiply called");
    return NULL;
}

void* vU512Mod(void)
{
    if (verbose) puts("STUB: vU512Mod called");
    return NULL;
}

void* vU512Neg(void)
{
    if (verbose) puts("STUB: vU512Neg called");
    return NULL;
}

void* vU512Sub(void)
{
    if (verbose) puts("STUB: vU512Sub called");
    return NULL;
}

void* vU512SubS(void)
{
    if (verbose) puts("STUB: vU512SubS called");
    return NULL;
}

void* vU64Add(void)
{
    if (verbose) puts("STUB: vU64Add called");
    return NULL;
}

void* vU64AddS(void)
{
    if (verbose) puts("STUB: vU64AddS called");
    return NULL;
}

void* vU64Divide(void)
{
    if (verbose) puts("STUB: vU64Divide called");
    return NULL;
}

void* vU64FullMulEven(void)
{
    if (verbose) puts("STUB: vU64FullMulEven called");
    return NULL;
}

void* vU64FullMulOdd(void)
{
    if (verbose) puts("STUB: vU64FullMulOdd called");
    return NULL;
}

void* vU64HalfMultiply(void)
{
    if (verbose) puts("STUB: vU64HalfMultiply called");
    return NULL;
}

void* vU64Neg(void)
{
    if (verbose) puts("STUB: vU64Neg called");
    return NULL;
}

void* vU64Sub(void)
{
    if (verbose) puts("STUB: vU64Sub called");
    return NULL;
}

void* vU64SubS(void)
{
    if (verbose) puts("STUB: vU64SubS called");
    return NULL;
}

void* vU8Divide(void)
{
    if (verbose) puts("STUB: vU8Divide called");
    return NULL;
}

void* vU8HalfMultiply(void)
{
    if (verbose) puts("STUB: vU8HalfMultiply called");
    return NULL;
}

void* vacosf(void)
{
    if (verbose) puts("STUB: vacosf called");
    return NULL;
}

void* vacoshf(void)
{
    if (verbose) puts("STUB: vacoshf called");
    return NULL;
}

void* vasinf(void)
{
    if (verbose) puts("STUB: vasinf called");
    return NULL;
}

void* vasinhf(void)
{
    if (verbose) puts("STUB: vasinhf called");
    return NULL;
}

void* vatan2f(void)
{
    if (verbose) puts("STUB: vatan2f called");
    return NULL;
}

void* vatanf(void)
{
    if (verbose) puts("STUB: vatanf called");
    return NULL;
}

void* vatanhf(void)
{
    if (verbose) puts("STUB: vatanhf called");
    return NULL;
}

void* vceilf(void)
{
    if (verbose) puts("STUB: vceilf called");
    return NULL;
}

void* vclassifyf(void)
{
    if (verbose) puts("STUB: vclassifyf called");
    return NULL;
}

void* vcopysignf(void)
{
    if (verbose) puts("STUB: vcopysignf called");
    return NULL;
}

void* vcosf(void)
{
    if (verbose) puts("STUB: vcosf called");
    return NULL;
}

void* vcoshf(void)
{
    if (verbose) puts("STUB: vcoshf called");
    return NULL;
}

void* vcospif(void)
{
    if (verbose) puts("STUB: vcospif called");
    return NULL;
}

void* vdivf(void)
{
    if (verbose) puts("STUB: vdivf called");
    return NULL;
}

void* vec_sdot(void)
{
    if (verbose) puts("STUB: vec_sdot called");
    return NULL;
}

void* vexp2f(void)
{
    if (verbose) puts("STUB: vexp2f called");
    return NULL;
}

void* vexpf(void)
{
    if (verbose) puts("STUB: vexpf called");
    return NULL;
}

void* vexpm1f(void)
{
    if (verbose) puts("STUB: vexpm1f called");
    return NULL;
}

void* vfabf(void)
{
    if (verbose) puts("STUB: vfabf called");
    return NULL;
}

void* vfabsf(void)
{
    if (verbose) puts("STUB: vfabsf called");
    return NULL;
}

void* vfloorf(void)
{
    if (verbose) puts("STUB: vfloorf called");
    return NULL;
}

void* vfmodf(void)
{
    if (verbose) puts("STUB: vfmodf called");
    return NULL;
}

void* vintf(void)
{
    if (verbose) puts("STUB: vintf called");
    return NULL;
}

void* vipowf(void)
{
    if (verbose) puts("STUB: vipowf called");
    return NULL;
}

void* vlog10f(void)
{
    if (verbose) puts("STUB: vlog10f called");
    return NULL;
}

void* vlog1pf(void)
{
    if (verbose) puts("STUB: vlog1pf called");
    return NULL;
}

void* vlog2f(void)
{
    if (verbose) puts("STUB: vlog2f called");
    return NULL;
}

void* vlogbf(void)
{
    if (verbose) puts("STUB: vlogbf called");
    return NULL;
}

void* vlogf(void)
{
    if (verbose) puts("STUB: vlogf called");
    return NULL;
}

void* vnextafterf(void)
{
    if (verbose) puts("STUB: vnextafterf called");
    return NULL;
}

void* vnintf(void)
{
    if (verbose) puts("STUB: vnintf called");
    return NULL;
}

void* vpowf(void)
{
    if (verbose) puts("STUB: vpowf called");
    return NULL;
}

void* vrecf(void)
{
    if (verbose) puts("STUB: vrecf called");
    return NULL;
}

void* vremainderf(void)
{
    if (verbose) puts("STUB: vremainderf called");
    return NULL;
}

void* vremquof(void)
{
    if (verbose) puts("STUB: vremquof called");
    return NULL;
}

void* vrsqrtf(void)
{
    if (verbose) puts("STUB: vrsqrtf called");
    return NULL;
}

void* vscalbf(void)
{
    if (verbose) puts("STUB: vscalbf called");
    return NULL;
}

void* vsignbitf(void)
{
    if (verbose) puts("STUB: vsignbitf called");
    return NULL;
}

void* vsincosf(void)
{
    if (verbose) puts("STUB: vsincosf called");
    return NULL;
}

void* vsinf(void)
{
    if (verbose) puts("STUB: vsinf called");
    return NULL;
}

void* vsinhf(void)
{
    if (verbose) puts("STUB: vsinhf called");
    return NULL;
}

void* vsinpif(void)
{
    if (verbose) puts("STUB: vsinpif called");
    return NULL;
}

void* vsqrtf(void)
{
    if (verbose) puts("STUB: vsqrtf called");
    return NULL;
}

void* vtablelookup(void)
{
    if (verbose) puts("STUB: vtablelookup called");
    return NULL;
}

void* vtanf(void)
{
    if (verbose) puts("STUB: vtanf called");
    return NULL;
}

void* vtanhf(void)
{
    if (verbose) puts("STUB: vtanhf called");
    return NULL;
}

void* vtanpif(void)
{
    if (verbose) puts("STUB: vtanpif called");
    return NULL;
}

void* vtruncf(void)
{
    if (verbose) puts("STUB: vtruncf called");
    return NULL;
}

void* vvacos(void)
{
    if (verbose) puts("STUB: vvacos called");
    return NULL;
}

void* vvacos_(void)
{
    if (verbose) puts("STUB: vvacos_ called");
    return NULL;
}

void* vvacosf(void)
{
    if (verbose) puts("STUB: vvacosf called");
    return NULL;
}

void* vvacosf_(void)
{
    if (verbose) puts("STUB: vvacosf_ called");
    return NULL;
}

void* vvacosh(void)
{
    if (verbose) puts("STUB: vvacosh called");
    return NULL;
}

void* vvacosh_(void)
{
    if (verbose) puts("STUB: vvacosh_ called");
    return NULL;
}

void* vvacoshf(void)
{
    if (verbose) puts("STUB: vvacoshf called");
    return NULL;
}

void* vvacoshf_(void)
{
    if (verbose) puts("STUB: vvacoshf_ called");
    return NULL;
}

void* vvasin(void)
{
    if (verbose) puts("STUB: vvasin called");
    return NULL;
}

void* vvasin_(void)
{
    if (verbose) puts("STUB: vvasin_ called");
    return NULL;
}

void* vvasinf(void)
{
    if (verbose) puts("STUB: vvasinf called");
    return NULL;
}

void* vvasinf_(void)
{
    if (verbose) puts("STUB: vvasinf_ called");
    return NULL;
}

void* vvasinh(void)
{
    if (verbose) puts("STUB: vvasinh called");
    return NULL;
}

void* vvasinh_(void)
{
    if (verbose) puts("STUB: vvasinh_ called");
    return NULL;
}

void* vvasinhf(void)
{
    if (verbose) puts("STUB: vvasinhf called");
    return NULL;
}

void* vvasinhf_(void)
{
    if (verbose) puts("STUB: vvasinhf_ called");
    return NULL;
}

void* vvatan(void)
{
    if (verbose) puts("STUB: vvatan called");
    return NULL;
}

void* vvatan2(void)
{
    if (verbose) puts("STUB: vvatan2 called");
    return NULL;
}

void* vvatan2_(void)
{
    if (verbose) puts("STUB: vvatan2_ called");
    return NULL;
}

void* vvatan2f(void)
{
    if (verbose) puts("STUB: vvatan2f called");
    return NULL;
}

void* vvatan2f_(void)
{
    if (verbose) puts("STUB: vvatan2f_ called");
    return NULL;
}

void* vvatan_(void)
{
    if (verbose) puts("STUB: vvatan_ called");
    return NULL;
}

void* vvatanf(void)
{
    if (verbose) puts("STUB: vvatanf called");
    return NULL;
}

void* vvatanf_(void)
{
    if (verbose) puts("STUB: vvatanf_ called");
    return NULL;
}

void* vvatanh(void)
{
    if (verbose) puts("STUB: vvatanh called");
    return NULL;
}

void* vvatanh_(void)
{
    if (verbose) puts("STUB: vvatanh_ called");
    return NULL;
}

void* vvatanhf(void)
{
    if (verbose) puts("STUB: vvatanhf called");
    return NULL;
}

void* vvatanhf_(void)
{
    if (verbose) puts("STUB: vvatanhf_ called");
    return NULL;
}

void* vvcbrt(void)
{
    if (verbose) puts("STUB: vvcbrt called");
    return NULL;
}

void* vvcbrt_(void)
{
    if (verbose) puts("STUB: vvcbrt_ called");
    return NULL;
}

void* vvcbrtf(void)
{
    if (verbose) puts("STUB: vvcbrtf called");
    return NULL;
}

void* vvcbrtf_(void)
{
    if (verbose) puts("STUB: vvcbrtf_ called");
    return NULL;
}

void* vvceil(void)
{
    if (verbose) puts("STUB: vvceil called");
    return NULL;
}

void* vvceil_(void)
{
    if (verbose) puts("STUB: vvceil_ called");
    return NULL;
}

void* vvceilf(void)
{
    if (verbose) puts("STUB: vvceilf called");
    return NULL;
}

void* vvceilf_(void)
{
    if (verbose) puts("STUB: vvceilf_ called");
    return NULL;
}

void* vvcopysign(void)
{
    if (verbose) puts("STUB: vvcopysign called");
    return NULL;
}

void* vvcopysign_(void)
{
    if (verbose) puts("STUB: vvcopysign_ called");
    return NULL;
}

void* vvcopysignf(void)
{
    if (verbose) puts("STUB: vvcopysignf called");
    return NULL;
}

void* vvcopysignf_(void)
{
    if (verbose) puts("STUB: vvcopysignf_ called");
    return NULL;
}

void* vvcos(void)
{
    if (verbose) puts("STUB: vvcos called");
    return NULL;
}

void* vvcos_(void)
{
    if (verbose) puts("STUB: vvcos_ called");
    return NULL;
}

void* vvcosf(void)
{
    if (verbose) puts("STUB: vvcosf called");
    return NULL;
}

void* vvcosf_(void)
{
    if (verbose) puts("STUB: vvcosf_ called");
    return NULL;
}

void* vvcosh(void)
{
    if (verbose) puts("STUB: vvcosh called");
    return NULL;
}

void* vvcosh_(void)
{
    if (verbose) puts("STUB: vvcosh_ called");
    return NULL;
}

void* vvcoshf(void)
{
    if (verbose) puts("STUB: vvcoshf called");
    return NULL;
}

void* vvcoshf_(void)
{
    if (verbose) puts("STUB: vvcoshf_ called");
    return NULL;
}

void* vvcosisin(void)
{
    if (verbose) puts("STUB: vvcosisin called");
    return NULL;
}

void* vvcosisin_(void)
{
    if (verbose) puts("STUB: vvcosisin_ called");
    return NULL;
}

void* vvcosisinf(void)
{
    if (verbose) puts("STUB: vvcosisinf called");
    return NULL;
}

void* vvcosisinf_(void)
{
    if (verbose) puts("STUB: vvcosisinf_ called");
    return NULL;
}

void* vvcospi(void)
{
    if (verbose) puts("STUB: vvcospi called");
    return NULL;
}

void* vvcospi_(void)
{
    if (verbose) puts("STUB: vvcospi_ called");
    return NULL;
}

void* vvcospif(void)
{
    if (verbose) puts("STUB: vvcospif called");
    return NULL;
}

void* vvcospif_(void)
{
    if (verbose) puts("STUB: vvcospif_ called");
    return NULL;
}

void* vvdiv(void)
{
    if (verbose) puts("STUB: vvdiv called");
    return NULL;
}

void* vvdiv_(void)
{
    if (verbose) puts("STUB: vvdiv_ called");
    return NULL;
}

void* vvdivf(void)
{
    if (verbose) puts("STUB: vvdivf called");
    return NULL;
}

void* vvdivf_(void)
{
    if (verbose) puts("STUB: vvdivf_ called");
    return NULL;
}

void* vvexp(void)
{
    if (verbose) puts("STUB: vvexp called");
    return NULL;
}

void* vvexp2(void)
{
    if (verbose) puts("STUB: vvexp2 called");
    return NULL;
}

void* vvexp2_(void)
{
    if (verbose) puts("STUB: vvexp2_ called");
    return NULL;
}

void* vvexp2f(void)
{
    if (verbose) puts("STUB: vvexp2f called");
    return NULL;
}

void* vvexp2f_(void)
{
    if (verbose) puts("STUB: vvexp2f_ called");
    return NULL;
}

void* vvexp_(void)
{
    if (verbose) puts("STUB: vvexp_ called");
    return NULL;
}

void* vvexpf(void)
{
    if (verbose) puts("STUB: vvexpf called");
    return NULL;
}

void* vvexpf_(void)
{
    if (verbose) puts("STUB: vvexpf_ called");
    return NULL;
}

void* vvexpm1(void)
{
    if (verbose) puts("STUB: vvexpm1 called");
    return NULL;
}

void* vvexpm1_(void)
{
    if (verbose) puts("STUB: vvexpm1_ called");
    return NULL;
}

void* vvexpm1f(void)
{
    if (verbose) puts("STUB: vvexpm1f called");
    return NULL;
}

void* vvexpm1f_(void)
{
    if (verbose) puts("STUB: vvexpm1f_ called");
    return NULL;
}

void* vvfabf(void)
{
    if (verbose) puts("STUB: vvfabf called");
    return NULL;
}

void* vvfabf_(void)
{
    if (verbose) puts("STUB: vvfabf_ called");
    return NULL;
}

void* vvfabs(void)
{
    if (verbose) puts("STUB: vvfabs called");
    return NULL;
}

void* vvfabs_(void)
{
    if (verbose) puts("STUB: vvfabs_ called");
    return NULL;
}

void* vvfabsf(void)
{
    if (verbose) puts("STUB: vvfabsf called");
    return NULL;
}

void* vvfabsf_(void)
{
    if (verbose) puts("STUB: vvfabsf_ called");
    return NULL;
}

void* vvfloor(void)
{
    if (verbose) puts("STUB: vvfloor called");
    return NULL;
}

void* vvfloor_(void)
{
    if (verbose) puts("STUB: vvfloor_ called");
    return NULL;
}

void* vvfloorf(void)
{
    if (verbose) puts("STUB: vvfloorf called");
    return NULL;
}

void* vvfloorf_(void)
{
    if (verbose) puts("STUB: vvfloorf_ called");
    return NULL;
}

void* vvfmod(void)
{
    if (verbose) puts("STUB: vvfmod called");
    return NULL;
}

void* vvfmod_(void)
{
    if (verbose) puts("STUB: vvfmod_ called");
    return NULL;
}

void* vvfmodf(void)
{
    if (verbose) puts("STUB: vvfmodf called");
    return NULL;
}

void* vvfmodf_(void)
{
    if (verbose) puts("STUB: vvfmodf_ called");
    return NULL;
}

void* vvint(void)
{
    if (verbose) puts("STUB: vvint called");
    return NULL;
}

void* vvint_(void)
{
    if (verbose) puts("STUB: vvint_ called");
    return NULL;
}

void* vvintf(void)
{
    if (verbose) puts("STUB: vvintf called");
    return NULL;
}

void* vvintf_(void)
{
    if (verbose) puts("STUB: vvintf_ called");
    return NULL;
}

void* vvlog(void)
{
    if (verbose) puts("STUB: vvlog called");
    return NULL;
}

void* vvlog10(void)
{
    if (verbose) puts("STUB: vvlog10 called");
    return NULL;
}

void* vvlog10_(void)
{
    if (verbose) puts("STUB: vvlog10_ called");
    return NULL;
}

void* vvlog10f(void)
{
    if (verbose) puts("STUB: vvlog10f called");
    return NULL;
}

void* vvlog10f_(void)
{
    if (verbose) puts("STUB: vvlog10f_ called");
    return NULL;
}

void* vvlog1p(void)
{
    if (verbose) puts("STUB: vvlog1p called");
    return NULL;
}

void* vvlog1p_(void)
{
    if (verbose) puts("STUB: vvlog1p_ called");
    return NULL;
}

void* vvlog1pf(void)
{
    if (verbose) puts("STUB: vvlog1pf called");
    return NULL;
}

void* vvlog1pf_(void)
{
    if (verbose) puts("STUB: vvlog1pf_ called");
    return NULL;
}

void* vvlog2(void)
{
    if (verbose) puts("STUB: vvlog2 called");
    return NULL;
}

void* vvlog2_(void)
{
    if (verbose) puts("STUB: vvlog2_ called");
    return NULL;
}

void* vvlog2f(void)
{
    if (verbose) puts("STUB: vvlog2f called");
    return NULL;
}

void* vvlog2f_(void)
{
    if (verbose) puts("STUB: vvlog2f_ called");
    return NULL;
}

void* vvlog_(void)
{
    if (verbose) puts("STUB: vvlog_ called");
    return NULL;
}

void* vvlogb(void)
{
    if (verbose) puts("STUB: vvlogb called");
    return NULL;
}

void* vvlogb_(void)
{
    if (verbose) puts("STUB: vvlogb_ called");
    return NULL;
}

void* vvlogbf(void)
{
    if (verbose) puts("STUB: vvlogbf called");
    return NULL;
}

void* vvlogbf_(void)
{
    if (verbose) puts("STUB: vvlogbf_ called");
    return NULL;
}

void* vvlogf(void)
{
    if (verbose) puts("STUB: vvlogf called");
    return NULL;
}

void* vvlogf_(void)
{
    if (verbose) puts("STUB: vvlogf_ called");
    return NULL;
}

void* vvnextafter(void)
{
    if (verbose) puts("STUB: vvnextafter called");
    return NULL;
}

void* vvnextafter_(void)
{
    if (verbose) puts("STUB: vvnextafter_ called");
    return NULL;
}

void* vvnextafterf(void)
{
    if (verbose) puts("STUB: vvnextafterf called");
    return NULL;
}

void* vvnextafterf_(void)
{
    if (verbose) puts("STUB: vvnextafterf_ called");
    return NULL;
}

void* vvnint(void)
{
    if (verbose) puts("STUB: vvnint called");
    return NULL;
}

void* vvnint_(void)
{
    if (verbose) puts("STUB: vvnint_ called");
    return NULL;
}

void* vvnintf(void)
{
    if (verbose) puts("STUB: vvnintf called");
    return NULL;
}

void* vvnintf_(void)
{
    if (verbose) puts("STUB: vvnintf_ called");
    return NULL;
}

void* vvpow(void)
{
    if (verbose) puts("STUB: vvpow called");
    return NULL;
}

void* vvpow_(void)
{
    if (verbose) puts("STUB: vvpow_ called");
    return NULL;
}

void* vvpowf(void)
{
    if (verbose) puts("STUB: vvpowf called");
    return NULL;
}

void* vvpowf_(void)
{
    if (verbose) puts("STUB: vvpowf_ called");
    return NULL;
}

void* vvpows(void)
{
    if (verbose) puts("STUB: vvpows called");
    return NULL;
}

void* vvpows_(void)
{
    if (verbose) puts("STUB: vvpows_ called");
    return NULL;
}

void* vvpowsf(void)
{
    if (verbose) puts("STUB: vvpowsf called");
    return NULL;
}

void* vvpowsf_(void)
{
    if (verbose) puts("STUB: vvpowsf_ called");
    return NULL;
}

void* vvrec(void)
{
    if (verbose) puts("STUB: vvrec called");
    return NULL;
}

void* vvrec_(void)
{
    if (verbose) puts("STUB: vvrec_ called");
    return NULL;
}

void* vvrecf(void)
{
    if (verbose) puts("STUB: vvrecf called");
    return NULL;
}

void* vvrecf_(void)
{
    if (verbose) puts("STUB: vvrecf_ called");
    return NULL;
}

void* vvremainder(void)
{
    if (verbose) puts("STUB: vvremainder called");
    return NULL;
}

void* vvremainder_(void)
{
    if (verbose) puts("STUB: vvremainder_ called");
    return NULL;
}

void* vvremainderf(void)
{
    if (verbose) puts("STUB: vvremainderf called");
    return NULL;
}

void* vvremainderf_(void)
{
    if (verbose) puts("STUB: vvremainderf_ called");
    return NULL;
}

void* vvrsqrt(void)
{
    if (verbose) puts("STUB: vvrsqrt called");
    return NULL;
}

void* vvrsqrt_(void)
{
    if (verbose) puts("STUB: vvrsqrt_ called");
    return NULL;
}

void* vvrsqrtf(void)
{
    if (verbose) puts("STUB: vvrsqrtf called");
    return NULL;
}

void* vvrsqrtf_(void)
{
    if (verbose) puts("STUB: vvrsqrtf_ called");
    return NULL;
}

void* vvsin(void)
{
    if (verbose) puts("STUB: vvsin called");
    return NULL;
}

void* vvsin_(void)
{
    if (verbose) puts("STUB: vvsin_ called");
    return NULL;
}

void* vvsincos(void)
{
    if (verbose) puts("STUB: vvsincos called");
    return NULL;
}

void* vvsincos_(void)
{
    if (verbose) puts("STUB: vvsincos_ called");
    return NULL;
}

void* vvsincosf(void)
{
    if (verbose) puts("STUB: vvsincosf called");
    return NULL;
}

void* vvsincosf_(void)
{
    if (verbose) puts("STUB: vvsincosf_ called");
    return NULL;
}

void* vvsinf(void)
{
    if (verbose) puts("STUB: vvsinf called");
    return NULL;
}

void* vvsinf_(void)
{
    if (verbose) puts("STUB: vvsinf_ called");
    return NULL;
}

void* vvsinh(void)
{
    if (verbose) puts("STUB: vvsinh called");
    return NULL;
}

void* vvsinh_(void)
{
    if (verbose) puts("STUB: vvsinh_ called");
    return NULL;
}

void* vvsinhf(void)
{
    if (verbose) puts("STUB: vvsinhf called");
    return NULL;
}

void* vvsinhf_(void)
{
    if (verbose) puts("STUB: vvsinhf_ called");
    return NULL;
}

void* vvsinpi(void)
{
    if (verbose) puts("STUB: vvsinpi called");
    return NULL;
}

void* vvsinpi_(void)
{
    if (verbose) puts("STUB: vvsinpi_ called");
    return NULL;
}

void* vvsinpif(void)
{
    if (verbose) puts("STUB: vvsinpif called");
    return NULL;
}

void* vvsinpif_(void)
{
    if (verbose) puts("STUB: vvsinpif_ called");
    return NULL;
}

void* vvsqrt(void)
{
    if (verbose) puts("STUB: vvsqrt called");
    return NULL;
}

void* vvsqrt_(void)
{
    if (verbose) puts("STUB: vvsqrt_ called");
    return NULL;
}

void* vvsqrtf(void)
{
    if (verbose) puts("STUB: vvsqrtf called");
    return NULL;
}

void* vvsqrtf_(void)
{
    if (verbose) puts("STUB: vvsqrtf_ called");
    return NULL;
}

void* vvtan(void)
{
    if (verbose) puts("STUB: vvtan called");
    return NULL;
}

void* vvtan_(void)
{
    if (verbose) puts("STUB: vvtan_ called");
    return NULL;
}

void* vvtanf(void)
{
    if (verbose) puts("STUB: vvtanf called");
    return NULL;
}

void* vvtanf_(void)
{
    if (verbose) puts("STUB: vvtanf_ called");
    return NULL;
}

void* vvtanh(void)
{
    if (verbose) puts("STUB: vvtanh called");
    return NULL;
}

void* vvtanh_(void)
{
    if (verbose) puts("STUB: vvtanh_ called");
    return NULL;
}

void* vvtanhf(void)
{
    if (verbose) puts("STUB: vvtanhf called");
    return NULL;
}

void* vvtanhf_(void)
{
    if (verbose) puts("STUB: vvtanhf_ called");
    return NULL;
}

void* vvtanpi(void)
{
    if (verbose) puts("STUB: vvtanpi called");
    return NULL;
}

void* vvtanpi_(void)
{
    if (verbose) puts("STUB: vvtanpi_ called");
    return NULL;
}

void* vvtanpif(void)
{
    if (verbose) puts("STUB: vvtanpif called");
    return NULL;
}

void* vvtanpif_(void)
{
    if (verbose) puts("STUB: vvtanpif_ called");
    return NULL;
}
