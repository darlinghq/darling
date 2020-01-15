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


#include <LAPACK/LAPACK.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CBDSQR(void)
{
    if (verbose) puts("STUB: CBDSQR called");
    return NULL;
}

void* CBDSQR_(void)
{
    if (verbose) puts("STUB: CBDSQR_ called");
    return NULL;
}

void* CGBBRD(void)
{
    if (verbose) puts("STUB: CGBBRD called");
    return NULL;
}

void* CGBBRD_(void)
{
    if (verbose) puts("STUB: CGBBRD_ called");
    return NULL;
}

void* CGBCON(void)
{
    if (verbose) puts("STUB: CGBCON called");
    return NULL;
}

void* CGBCON_(void)
{
    if (verbose) puts("STUB: CGBCON_ called");
    return NULL;
}

void* CGBEQU(void)
{
    if (verbose) puts("STUB: CGBEQU called");
    return NULL;
}

void* CGBEQUB(void)
{
    if (verbose) puts("STUB: CGBEQUB called");
    return NULL;
}

void* CGBEQUB_(void)
{
    if (verbose) puts("STUB: CGBEQUB_ called");
    return NULL;
}

void* CGBEQU_(void)
{
    if (verbose) puts("STUB: CGBEQU_ called");
    return NULL;
}

void* CGBRFS(void)
{
    if (verbose) puts("STUB: CGBRFS called");
    return NULL;
}

void* CGBRFS_(void)
{
    if (verbose) puts("STUB: CGBRFS_ called");
    return NULL;
}

void* CGBSV(void)
{
    if (verbose) puts("STUB: CGBSV called");
    return NULL;
}

void* CGBSVX(void)
{
    if (verbose) puts("STUB: CGBSVX called");
    return NULL;
}

void* CGBSVX_(void)
{
    if (verbose) puts("STUB: CGBSVX_ called");
    return NULL;
}

void* CGBSV_(void)
{
    if (verbose) puts("STUB: CGBSV_ called");
    return NULL;
}

void* CGBTF2(void)
{
    if (verbose) puts("STUB: CGBTF2 called");
    return NULL;
}

void* CGBTF2_(void)
{
    if (verbose) puts("STUB: CGBTF2_ called");
    return NULL;
}

void* CGBTRF(void)
{
    if (verbose) puts("STUB: CGBTRF called");
    return NULL;
}

void* CGBTRF_(void)
{
    if (verbose) puts("STUB: CGBTRF_ called");
    return NULL;
}

void* CGBTRS(void)
{
    if (verbose) puts("STUB: CGBTRS called");
    return NULL;
}

void* CGBTRS_(void)
{
    if (verbose) puts("STUB: CGBTRS_ called");
    return NULL;
}

void* CGEBAK(void)
{
    if (verbose) puts("STUB: CGEBAK called");
    return NULL;
}

void* CGEBAK_(void)
{
    if (verbose) puts("STUB: CGEBAK_ called");
    return NULL;
}

void* CGEBAL(void)
{
    if (verbose) puts("STUB: CGEBAL called");
    return NULL;
}

void* CGEBAL_(void)
{
    if (verbose) puts("STUB: CGEBAL_ called");
    return NULL;
}

void* CGEBD2(void)
{
    if (verbose) puts("STUB: CGEBD2 called");
    return NULL;
}

void* CGEBD2_(void)
{
    if (verbose) puts("STUB: CGEBD2_ called");
    return NULL;
}

void* CGEBRD(void)
{
    if (verbose) puts("STUB: CGEBRD called");
    return NULL;
}

void* CGEBRD_(void)
{
    if (verbose) puts("STUB: CGEBRD_ called");
    return NULL;
}

void* CGECON(void)
{
    if (verbose) puts("STUB: CGECON called");
    return NULL;
}

void* CGECON_(void)
{
    if (verbose) puts("STUB: CGECON_ called");
    return NULL;
}

void* CGEEQU(void)
{
    if (verbose) puts("STUB: CGEEQU called");
    return NULL;
}

void* CGEEQUB(void)
{
    if (verbose) puts("STUB: CGEEQUB called");
    return NULL;
}

void* CGEEQUB_(void)
{
    if (verbose) puts("STUB: CGEEQUB_ called");
    return NULL;
}

void* CGEEQU_(void)
{
    if (verbose) puts("STUB: CGEEQU_ called");
    return NULL;
}

void* CGEES(void)
{
    if (verbose) puts("STUB: CGEES called");
    return NULL;
}

void* CGEESX(void)
{
    if (verbose) puts("STUB: CGEESX called");
    return NULL;
}

void* CGEESX_(void)
{
    if (verbose) puts("STUB: CGEESX_ called");
    return NULL;
}

void* CGEES_(void)
{
    if (verbose) puts("STUB: CGEES_ called");
    return NULL;
}

void* CGEEV(void)
{
    if (verbose) puts("STUB: CGEEV called");
    return NULL;
}

void* CGEEVX(void)
{
    if (verbose) puts("STUB: CGEEVX called");
    return NULL;
}

void* CGEEVX_(void)
{
    if (verbose) puts("STUB: CGEEVX_ called");
    return NULL;
}

void* CGEEV_(void)
{
    if (verbose) puts("STUB: CGEEV_ called");
    return NULL;
}

void* CGEGS(void)
{
    if (verbose) puts("STUB: CGEGS called");
    return NULL;
}

void* CGEGS_(void)
{
    if (verbose) puts("STUB: CGEGS_ called");
    return NULL;
}

void* CGEGV(void)
{
    if (verbose) puts("STUB: CGEGV called");
    return NULL;
}

void* CGEGV_(void)
{
    if (verbose) puts("STUB: CGEGV_ called");
    return NULL;
}

void* CGEHD2(void)
{
    if (verbose) puts("STUB: CGEHD2 called");
    return NULL;
}

void* CGEHD2_(void)
{
    if (verbose) puts("STUB: CGEHD2_ called");
    return NULL;
}

void* CGEHRD(void)
{
    if (verbose) puts("STUB: CGEHRD called");
    return NULL;
}

void* CGEHRD_(void)
{
    if (verbose) puts("STUB: CGEHRD_ called");
    return NULL;
}

void* CGELQ2(void)
{
    if (verbose) puts("STUB: CGELQ2 called");
    return NULL;
}

void* CGELQ2_(void)
{
    if (verbose) puts("STUB: CGELQ2_ called");
    return NULL;
}

void* CGELQF(void)
{
    if (verbose) puts("STUB: CGELQF called");
    return NULL;
}

void* CGELQF_(void)
{
    if (verbose) puts("STUB: CGELQF_ called");
    return NULL;
}

void* CGELS(void)
{
    if (verbose) puts("STUB: CGELS called");
    return NULL;
}

void* CGELSD(void)
{
    if (verbose) puts("STUB: CGELSD called");
    return NULL;
}

void* CGELSD_(void)
{
    if (verbose) puts("STUB: CGELSD_ called");
    return NULL;
}

void* CGELSS(void)
{
    if (verbose) puts("STUB: CGELSS called");
    return NULL;
}

void* CGELSS_(void)
{
    if (verbose) puts("STUB: CGELSS_ called");
    return NULL;
}

void* CGELSX(void)
{
    if (verbose) puts("STUB: CGELSX called");
    return NULL;
}

void* CGELSX_(void)
{
    if (verbose) puts("STUB: CGELSX_ called");
    return NULL;
}

void* CGELSY(void)
{
    if (verbose) puts("STUB: CGELSY called");
    return NULL;
}

void* CGELSY_(void)
{
    if (verbose) puts("STUB: CGELSY_ called");
    return NULL;
}

void* CGELS_(void)
{
    if (verbose) puts("STUB: CGELS_ called");
    return NULL;
}

void* CGEQL2(void)
{
    if (verbose) puts("STUB: CGEQL2 called");
    return NULL;
}

void* CGEQL2_(void)
{
    if (verbose) puts("STUB: CGEQL2_ called");
    return NULL;
}

void* CGEQLF(void)
{
    if (verbose) puts("STUB: CGEQLF called");
    return NULL;
}

void* CGEQLF_(void)
{
    if (verbose) puts("STUB: CGEQLF_ called");
    return NULL;
}

void* CGEQP3(void)
{
    if (verbose) puts("STUB: CGEQP3 called");
    return NULL;
}

void* CGEQP3_(void)
{
    if (verbose) puts("STUB: CGEQP3_ called");
    return NULL;
}

void* CGEQPF(void)
{
    if (verbose) puts("STUB: CGEQPF called");
    return NULL;
}

void* CGEQPF_(void)
{
    if (verbose) puts("STUB: CGEQPF_ called");
    return NULL;
}

void* CGEQR2(void)
{
    if (verbose) puts("STUB: CGEQR2 called");
    return NULL;
}

void* CGEQR2_(void)
{
    if (verbose) puts("STUB: CGEQR2_ called");
    return NULL;
}

void* CGEQRF(void)
{
    if (verbose) puts("STUB: CGEQRF called");
    return NULL;
}

void* CGEQRF_(void)
{
    if (verbose) puts("STUB: CGEQRF_ called");
    return NULL;
}

void* CGERFS(void)
{
    if (verbose) puts("STUB: CGERFS called");
    return NULL;
}

void* CGERFS_(void)
{
    if (verbose) puts("STUB: CGERFS_ called");
    return NULL;
}

void* CGERQ2(void)
{
    if (verbose) puts("STUB: CGERQ2 called");
    return NULL;
}

void* CGERQ2_(void)
{
    if (verbose) puts("STUB: CGERQ2_ called");
    return NULL;
}

void* CGERQF(void)
{
    if (verbose) puts("STUB: CGERQF called");
    return NULL;
}

void* CGERQF_(void)
{
    if (verbose) puts("STUB: CGERQF_ called");
    return NULL;
}

void* CGESC2(void)
{
    if (verbose) puts("STUB: CGESC2 called");
    return NULL;
}

void* CGESC2_(void)
{
    if (verbose) puts("STUB: CGESC2_ called");
    return NULL;
}

void* CGESDD(void)
{
    if (verbose) puts("STUB: CGESDD called");
    return NULL;
}

void* CGESDD_(void)
{
    if (verbose) puts("STUB: CGESDD_ called");
    return NULL;
}

void* CGESV(void)
{
    if (verbose) puts("STUB: CGESV called");
    return NULL;
}

void* CGESVD(void)
{
    if (verbose) puts("STUB: CGESVD called");
    return NULL;
}

void* CGESVD_(void)
{
    if (verbose) puts("STUB: CGESVD_ called");
    return NULL;
}

void* CGESVX(void)
{
    if (verbose) puts("STUB: CGESVX called");
    return NULL;
}

void* CGESVX_(void)
{
    if (verbose) puts("STUB: CGESVX_ called");
    return NULL;
}

void* CGESV_(void)
{
    if (verbose) puts("STUB: CGESV_ called");
    return NULL;
}

void* CGETC2(void)
{
    if (verbose) puts("STUB: CGETC2 called");
    return NULL;
}

void* CGETC2_(void)
{
    if (verbose) puts("STUB: CGETC2_ called");
    return NULL;
}

void* CGETF2(void)
{
    if (verbose) puts("STUB: CGETF2 called");
    return NULL;
}

void* CGETF2_(void)
{
    if (verbose) puts("STUB: CGETF2_ called");
    return NULL;
}

void* CGETRF(void)
{
    if (verbose) puts("STUB: CGETRF called");
    return NULL;
}

void* CGETRF_(void)
{
    if (verbose) puts("STUB: CGETRF_ called");
    return NULL;
}

void* CGETRI(void)
{
    if (verbose) puts("STUB: CGETRI called");
    return NULL;
}

void* CGETRI_(void)
{
    if (verbose) puts("STUB: CGETRI_ called");
    return NULL;
}

void* CGETRS(void)
{
    if (verbose) puts("STUB: CGETRS called");
    return NULL;
}

void* CGETRS_(void)
{
    if (verbose) puts("STUB: CGETRS_ called");
    return NULL;
}

void* CGGBAK(void)
{
    if (verbose) puts("STUB: CGGBAK called");
    return NULL;
}

void* CGGBAK_(void)
{
    if (verbose) puts("STUB: CGGBAK_ called");
    return NULL;
}

void* CGGBAL(void)
{
    if (verbose) puts("STUB: CGGBAL called");
    return NULL;
}

void* CGGBAL_(void)
{
    if (verbose) puts("STUB: CGGBAL_ called");
    return NULL;
}

void* CGGES(void)
{
    if (verbose) puts("STUB: CGGES called");
    return NULL;
}

void* CGGESX(void)
{
    if (verbose) puts("STUB: CGGESX called");
    return NULL;
}

void* CGGESX_(void)
{
    if (verbose) puts("STUB: CGGESX_ called");
    return NULL;
}

void* CGGES_(void)
{
    if (verbose) puts("STUB: CGGES_ called");
    return NULL;
}

void* CGGEV(void)
{
    if (verbose) puts("STUB: CGGEV called");
    return NULL;
}

void* CGGEVX(void)
{
    if (verbose) puts("STUB: CGGEVX called");
    return NULL;
}

void* CGGEVX_(void)
{
    if (verbose) puts("STUB: CGGEVX_ called");
    return NULL;
}

void* CGGEV_(void)
{
    if (verbose) puts("STUB: CGGEV_ called");
    return NULL;
}

void* CGGGLM(void)
{
    if (verbose) puts("STUB: CGGGLM called");
    return NULL;
}

void* CGGGLM_(void)
{
    if (verbose) puts("STUB: CGGGLM_ called");
    return NULL;
}

void* CGGHRD(void)
{
    if (verbose) puts("STUB: CGGHRD called");
    return NULL;
}

void* CGGHRD_(void)
{
    if (verbose) puts("STUB: CGGHRD_ called");
    return NULL;
}

void* CGGLSE(void)
{
    if (verbose) puts("STUB: CGGLSE called");
    return NULL;
}

void* CGGLSE_(void)
{
    if (verbose) puts("STUB: CGGLSE_ called");
    return NULL;
}

void* CGGQRF(void)
{
    if (verbose) puts("STUB: CGGQRF called");
    return NULL;
}

void* CGGQRF_(void)
{
    if (verbose) puts("STUB: CGGQRF_ called");
    return NULL;
}

void* CGGRQF(void)
{
    if (verbose) puts("STUB: CGGRQF called");
    return NULL;
}

void* CGGRQF_(void)
{
    if (verbose) puts("STUB: CGGRQF_ called");
    return NULL;
}

void* CGGSVD(void)
{
    if (verbose) puts("STUB: CGGSVD called");
    return NULL;
}

void* CGGSVD_(void)
{
    if (verbose) puts("STUB: CGGSVD_ called");
    return NULL;
}

void* CGGSVP(void)
{
    if (verbose) puts("STUB: CGGSVP called");
    return NULL;
}

void* CGGSVP_(void)
{
    if (verbose) puts("STUB: CGGSVP_ called");
    return NULL;
}

void* CGTCON(void)
{
    if (verbose) puts("STUB: CGTCON called");
    return NULL;
}

void* CGTCON_(void)
{
    if (verbose) puts("STUB: CGTCON_ called");
    return NULL;
}

void* CGTRFS(void)
{
    if (verbose) puts("STUB: CGTRFS called");
    return NULL;
}

void* CGTRFS_(void)
{
    if (verbose) puts("STUB: CGTRFS_ called");
    return NULL;
}

void* CGTSV(void)
{
    if (verbose) puts("STUB: CGTSV called");
    return NULL;
}

void* CGTSVX(void)
{
    if (verbose) puts("STUB: CGTSVX called");
    return NULL;
}

void* CGTSVX_(void)
{
    if (verbose) puts("STUB: CGTSVX_ called");
    return NULL;
}

void* CGTSV_(void)
{
    if (verbose) puts("STUB: CGTSV_ called");
    return NULL;
}

void* CGTTRF(void)
{
    if (verbose) puts("STUB: CGTTRF called");
    return NULL;
}

void* CGTTRF_(void)
{
    if (verbose) puts("STUB: CGTTRF_ called");
    return NULL;
}

void* CGTTRS(void)
{
    if (verbose) puts("STUB: CGTTRS called");
    return NULL;
}

void* CGTTRS_(void)
{
    if (verbose) puts("STUB: CGTTRS_ called");
    return NULL;
}

void* CGTTS2(void)
{
    if (verbose) puts("STUB: CGTTS2 called");
    return NULL;
}

void* CGTTS2_(void)
{
    if (verbose) puts("STUB: CGTTS2_ called");
    return NULL;
}

void* CHBEV(void)
{
    if (verbose) puts("STUB: CHBEV called");
    return NULL;
}

void* CHBEVD(void)
{
    if (verbose) puts("STUB: CHBEVD called");
    return NULL;
}

void* CHBEVD_(void)
{
    if (verbose) puts("STUB: CHBEVD_ called");
    return NULL;
}

void* CHBEVX(void)
{
    if (verbose) puts("STUB: CHBEVX called");
    return NULL;
}

void* CHBEVX_(void)
{
    if (verbose) puts("STUB: CHBEVX_ called");
    return NULL;
}

void* CHBEV_(void)
{
    if (verbose) puts("STUB: CHBEV_ called");
    return NULL;
}

void* CHBGST(void)
{
    if (verbose) puts("STUB: CHBGST called");
    return NULL;
}

void* CHBGST_(void)
{
    if (verbose) puts("STUB: CHBGST_ called");
    return NULL;
}

void* CHBGV(void)
{
    if (verbose) puts("STUB: CHBGV called");
    return NULL;
}

void* CHBGVD(void)
{
    if (verbose) puts("STUB: CHBGVD called");
    return NULL;
}

void* CHBGVD_(void)
{
    if (verbose) puts("STUB: CHBGVD_ called");
    return NULL;
}

void* CHBGVX(void)
{
    if (verbose) puts("STUB: CHBGVX called");
    return NULL;
}

void* CHBGVX_(void)
{
    if (verbose) puts("STUB: CHBGVX_ called");
    return NULL;
}

void* CHBGV_(void)
{
    if (verbose) puts("STUB: CHBGV_ called");
    return NULL;
}

void* CHBTRD(void)
{
    if (verbose) puts("STUB: CHBTRD called");
    return NULL;
}

void* CHBTRD_(void)
{
    if (verbose) puts("STUB: CHBTRD_ called");
    return NULL;
}

void* CHECON(void)
{
    if (verbose) puts("STUB: CHECON called");
    return NULL;
}

void* CHECON_(void)
{
    if (verbose) puts("STUB: CHECON_ called");
    return NULL;
}

void* CHEEQUB(void)
{
    if (verbose) puts("STUB: CHEEQUB called");
    return NULL;
}

void* CHEEQUB_(void)
{
    if (verbose) puts("STUB: CHEEQUB_ called");
    return NULL;
}

void* CHEEV(void)
{
    if (verbose) puts("STUB: CHEEV called");
    return NULL;
}

void* CHEEVD(void)
{
    if (verbose) puts("STUB: CHEEVD called");
    return NULL;
}

void* CHEEVD_(void)
{
    if (verbose) puts("STUB: CHEEVD_ called");
    return NULL;
}

void* CHEEVR(void)
{
    if (verbose) puts("STUB: CHEEVR called");
    return NULL;
}

void* CHEEVR_(void)
{
    if (verbose) puts("STUB: CHEEVR_ called");
    return NULL;
}

void* CHEEVX(void)
{
    if (verbose) puts("STUB: CHEEVX called");
    return NULL;
}

void* CHEEVX_(void)
{
    if (verbose) puts("STUB: CHEEVX_ called");
    return NULL;
}

void* CHEEV_(void)
{
    if (verbose) puts("STUB: CHEEV_ called");
    return NULL;
}

void* CHEGS2(void)
{
    if (verbose) puts("STUB: CHEGS2 called");
    return NULL;
}

void* CHEGS2_(void)
{
    if (verbose) puts("STUB: CHEGS2_ called");
    return NULL;
}

void* CHEGST(void)
{
    if (verbose) puts("STUB: CHEGST called");
    return NULL;
}

void* CHEGST_(void)
{
    if (verbose) puts("STUB: CHEGST_ called");
    return NULL;
}

void* CHEGV(void)
{
    if (verbose) puts("STUB: CHEGV called");
    return NULL;
}

void* CHEGVD(void)
{
    if (verbose) puts("STUB: CHEGVD called");
    return NULL;
}

void* CHEGVD_(void)
{
    if (verbose) puts("STUB: CHEGVD_ called");
    return NULL;
}

void* CHEGVX(void)
{
    if (verbose) puts("STUB: CHEGVX called");
    return NULL;
}

void* CHEGVX_(void)
{
    if (verbose) puts("STUB: CHEGVX_ called");
    return NULL;
}

void* CHEGV_(void)
{
    if (verbose) puts("STUB: CHEGV_ called");
    return NULL;
}

void* CHERFS(void)
{
    if (verbose) puts("STUB: CHERFS called");
    return NULL;
}

void* CHERFS_(void)
{
    if (verbose) puts("STUB: CHERFS_ called");
    return NULL;
}

void* CHESV(void)
{
    if (verbose) puts("STUB: CHESV called");
    return NULL;
}

void* CHESVX(void)
{
    if (verbose) puts("STUB: CHESVX called");
    return NULL;
}

void* CHESVX_(void)
{
    if (verbose) puts("STUB: CHESVX_ called");
    return NULL;
}

void* CHESV_(void)
{
    if (verbose) puts("STUB: CHESV_ called");
    return NULL;
}

void* CHETD2(void)
{
    if (verbose) puts("STUB: CHETD2 called");
    return NULL;
}

void* CHETD2_(void)
{
    if (verbose) puts("STUB: CHETD2_ called");
    return NULL;
}

void* CHETF2(void)
{
    if (verbose) puts("STUB: CHETF2 called");
    return NULL;
}

void* CHETF2_(void)
{
    if (verbose) puts("STUB: CHETF2_ called");
    return NULL;
}

void* CHETRD(void)
{
    if (verbose) puts("STUB: CHETRD called");
    return NULL;
}

void* CHETRD_(void)
{
    if (verbose) puts("STUB: CHETRD_ called");
    return NULL;
}

void* CHETRF(void)
{
    if (verbose) puts("STUB: CHETRF called");
    return NULL;
}

void* CHETRF_(void)
{
    if (verbose) puts("STUB: CHETRF_ called");
    return NULL;
}

void* CHETRI(void)
{
    if (verbose) puts("STUB: CHETRI called");
    return NULL;
}

void* CHETRI_(void)
{
    if (verbose) puts("STUB: CHETRI_ called");
    return NULL;
}

void* CHETRS(void)
{
    if (verbose) puts("STUB: CHETRS called");
    return NULL;
}

void* CHETRS_(void)
{
    if (verbose) puts("STUB: CHETRS_ called");
    return NULL;
}

void* CHFRK(void)
{
    if (verbose) puts("STUB: CHFRK called");
    return NULL;
}

void* CHFRK_(void)
{
    if (verbose) puts("STUB: CHFRK_ called");
    return NULL;
}

void* CHGEQZ(void)
{
    if (verbose) puts("STUB: CHGEQZ called");
    return NULL;
}

void* CHGEQZ_(void)
{
    if (verbose) puts("STUB: CHGEQZ_ called");
    return NULL;
}

void* CHLA_TRANSTYPE__(void)
{
    if (verbose) puts("STUB: CHLA_TRANSTYPE__ called");
    return NULL;
}

void* CHPCON(void)
{
    if (verbose) puts("STUB: CHPCON called");
    return NULL;
}

void* CHPCON_(void)
{
    if (verbose) puts("STUB: CHPCON_ called");
    return NULL;
}

void* CHPEV(void)
{
    if (verbose) puts("STUB: CHPEV called");
    return NULL;
}

void* CHPEVD(void)
{
    if (verbose) puts("STUB: CHPEVD called");
    return NULL;
}

void* CHPEVD_(void)
{
    if (verbose) puts("STUB: CHPEVD_ called");
    return NULL;
}

void* CHPEVX(void)
{
    if (verbose) puts("STUB: CHPEVX called");
    return NULL;
}

void* CHPEVX_(void)
{
    if (verbose) puts("STUB: CHPEVX_ called");
    return NULL;
}

void* CHPEV_(void)
{
    if (verbose) puts("STUB: CHPEV_ called");
    return NULL;
}

void* CHPGST(void)
{
    if (verbose) puts("STUB: CHPGST called");
    return NULL;
}

void* CHPGST_(void)
{
    if (verbose) puts("STUB: CHPGST_ called");
    return NULL;
}

void* CHPGV(void)
{
    if (verbose) puts("STUB: CHPGV called");
    return NULL;
}

void* CHPGVD(void)
{
    if (verbose) puts("STUB: CHPGVD called");
    return NULL;
}

void* CHPGVD_(void)
{
    if (verbose) puts("STUB: CHPGVD_ called");
    return NULL;
}

void* CHPGVX(void)
{
    if (verbose) puts("STUB: CHPGVX called");
    return NULL;
}

void* CHPGVX_(void)
{
    if (verbose) puts("STUB: CHPGVX_ called");
    return NULL;
}

void* CHPGV_(void)
{
    if (verbose) puts("STUB: CHPGV_ called");
    return NULL;
}

void* CHPRFS(void)
{
    if (verbose) puts("STUB: CHPRFS called");
    return NULL;
}

void* CHPRFS_(void)
{
    if (verbose) puts("STUB: CHPRFS_ called");
    return NULL;
}

void* CHPSV(void)
{
    if (verbose) puts("STUB: CHPSV called");
    return NULL;
}

void* CHPSVX(void)
{
    if (verbose) puts("STUB: CHPSVX called");
    return NULL;
}

void* CHPSVX_(void)
{
    if (verbose) puts("STUB: CHPSVX_ called");
    return NULL;
}

void* CHPSV_(void)
{
    if (verbose) puts("STUB: CHPSV_ called");
    return NULL;
}

void* CHPTRD(void)
{
    if (verbose) puts("STUB: CHPTRD called");
    return NULL;
}

void* CHPTRD_(void)
{
    if (verbose) puts("STUB: CHPTRD_ called");
    return NULL;
}

void* CHPTRF(void)
{
    if (verbose) puts("STUB: CHPTRF called");
    return NULL;
}

void* CHPTRF_(void)
{
    if (verbose) puts("STUB: CHPTRF_ called");
    return NULL;
}

void* CHPTRI(void)
{
    if (verbose) puts("STUB: CHPTRI called");
    return NULL;
}

void* CHPTRI_(void)
{
    if (verbose) puts("STUB: CHPTRI_ called");
    return NULL;
}

void* CHPTRS(void)
{
    if (verbose) puts("STUB: CHPTRS called");
    return NULL;
}

void* CHPTRS_(void)
{
    if (verbose) puts("STUB: CHPTRS_ called");
    return NULL;
}

void* CHSEIN(void)
{
    if (verbose) puts("STUB: CHSEIN called");
    return NULL;
}

void* CHSEIN_(void)
{
    if (verbose) puts("STUB: CHSEIN_ called");
    return NULL;
}

void* CHSEQR(void)
{
    if (verbose) puts("STUB: CHSEQR called");
    return NULL;
}

void* CHSEQR_(void)
{
    if (verbose) puts("STUB: CHSEQR_ called");
    return NULL;
}

void* CLABRD(void)
{
    if (verbose) puts("STUB: CLABRD called");
    return NULL;
}

void* CLABRD_(void)
{
    if (verbose) puts("STUB: CLABRD_ called");
    return NULL;
}

void* CLACGV(void)
{
    if (verbose) puts("STUB: CLACGV called");
    return NULL;
}

void* CLACGV_(void)
{
    if (verbose) puts("STUB: CLACGV_ called");
    return NULL;
}

void* CLACN2(void)
{
    if (verbose) puts("STUB: CLACN2 called");
    return NULL;
}

void* CLACN2_(void)
{
    if (verbose) puts("STUB: CLACN2_ called");
    return NULL;
}

void* CLACON(void)
{
    if (verbose) puts("STUB: CLACON called");
    return NULL;
}

void* CLACON_(void)
{
    if (verbose) puts("STUB: CLACON_ called");
    return NULL;
}

void* CLACP2(void)
{
    if (verbose) puts("STUB: CLACP2 called");
    return NULL;
}

void* CLACP2_(void)
{
    if (verbose) puts("STUB: CLACP2_ called");
    return NULL;
}

void* CLACPY(void)
{
    if (verbose) puts("STUB: CLACPY called");
    return NULL;
}

void* CLACPY_(void)
{
    if (verbose) puts("STUB: CLACPY_ called");
    return NULL;
}

void* CLACRM(void)
{
    if (verbose) puts("STUB: CLACRM called");
    return NULL;
}

void* CLACRM_(void)
{
    if (verbose) puts("STUB: CLACRM_ called");
    return NULL;
}

void* CLACRT(void)
{
    if (verbose) puts("STUB: CLACRT called");
    return NULL;
}

void* CLACRT_(void)
{
    if (verbose) puts("STUB: CLACRT_ called");
    return NULL;
}

void* CLADIV(void)
{
    if (verbose) puts("STUB: CLADIV called");
    return NULL;
}

void* CLADIV_(void)
{
    if (verbose) puts("STUB: CLADIV_ called");
    return NULL;
}

void* CLAED0(void)
{
    if (verbose) puts("STUB: CLAED0 called");
    return NULL;
}

void* CLAED0_(void)
{
    if (verbose) puts("STUB: CLAED0_ called");
    return NULL;
}

void* CLAED7(void)
{
    if (verbose) puts("STUB: CLAED7 called");
    return NULL;
}

void* CLAED7_(void)
{
    if (verbose) puts("STUB: CLAED7_ called");
    return NULL;
}

void* CLAED8(void)
{
    if (verbose) puts("STUB: CLAED8 called");
    return NULL;
}

void* CLAED8_(void)
{
    if (verbose) puts("STUB: CLAED8_ called");
    return NULL;
}

void* CLAEIN(void)
{
    if (verbose) puts("STUB: CLAEIN called");
    return NULL;
}

void* CLAEIN_(void)
{
    if (verbose) puts("STUB: CLAEIN_ called");
    return NULL;
}

void* CLAESY(void)
{
    if (verbose) puts("STUB: CLAESY called");
    return NULL;
}

void* CLAESY_(void)
{
    if (verbose) puts("STUB: CLAESY_ called");
    return NULL;
}

void* CLAEV2(void)
{
    if (verbose) puts("STUB: CLAEV2 called");
    return NULL;
}

void* CLAEV2_(void)
{
    if (verbose) puts("STUB: CLAEV2_ called");
    return NULL;
}

void* CLAG2Z(void)
{
    if (verbose) puts("STUB: CLAG2Z called");
    return NULL;
}

void* CLAG2Z_(void)
{
    if (verbose) puts("STUB: CLAG2Z_ called");
    return NULL;
}

void* CLAGS2(void)
{
    if (verbose) puts("STUB: CLAGS2 called");
    return NULL;
}

void* CLAGS2_(void)
{
    if (verbose) puts("STUB: CLAGS2_ called");
    return NULL;
}

void* CLAGTM(void)
{
    if (verbose) puts("STUB: CLAGTM called");
    return NULL;
}

void* CLAGTM_(void)
{
    if (verbose) puts("STUB: CLAGTM_ called");
    return NULL;
}

void* CLAHEF(void)
{
    if (verbose) puts("STUB: CLAHEF called");
    return NULL;
}

void* CLAHEF_(void)
{
    if (verbose) puts("STUB: CLAHEF_ called");
    return NULL;
}

void* CLAHQR(void)
{
    if (verbose) puts("STUB: CLAHQR called");
    return NULL;
}

void* CLAHQR_(void)
{
    if (verbose) puts("STUB: CLAHQR_ called");
    return NULL;
}

void* CLAHR2(void)
{
    if (verbose) puts("STUB: CLAHR2 called");
    return NULL;
}

void* CLAHR2_(void)
{
    if (verbose) puts("STUB: CLAHR2_ called");
    return NULL;
}

void* CLAHRD(void)
{
    if (verbose) puts("STUB: CLAHRD called");
    return NULL;
}

void* CLAHRD_(void)
{
    if (verbose) puts("STUB: CLAHRD_ called");
    return NULL;
}

void* CLAIC1(void)
{
    if (verbose) puts("STUB: CLAIC1 called");
    return NULL;
}

void* CLAIC1_(void)
{
    if (verbose) puts("STUB: CLAIC1_ called");
    return NULL;
}

void* CLALS0(void)
{
    if (verbose) puts("STUB: CLALS0 called");
    return NULL;
}

void* CLALS0_(void)
{
    if (verbose) puts("STUB: CLALS0_ called");
    return NULL;
}

void* CLALSA(void)
{
    if (verbose) puts("STUB: CLALSA called");
    return NULL;
}

void* CLALSA_(void)
{
    if (verbose) puts("STUB: CLALSA_ called");
    return NULL;
}

void* CLALSD(void)
{
    if (verbose) puts("STUB: CLALSD called");
    return NULL;
}

void* CLALSD_(void)
{
    if (verbose) puts("STUB: CLALSD_ called");
    return NULL;
}

void* CLANGB(void)
{
    if (verbose) puts("STUB: CLANGB called");
    return NULL;
}

void* CLANGB_(void)
{
    if (verbose) puts("STUB: CLANGB_ called");
    return NULL;
}

void* CLANGE(void)
{
    if (verbose) puts("STUB: CLANGE called");
    return NULL;
}

void* CLANGE_(void)
{
    if (verbose) puts("STUB: CLANGE_ called");
    return NULL;
}

void* CLANGT(void)
{
    if (verbose) puts("STUB: CLANGT called");
    return NULL;
}

void* CLANGT_(void)
{
    if (verbose) puts("STUB: CLANGT_ called");
    return NULL;
}

void* CLANHB(void)
{
    if (verbose) puts("STUB: CLANHB called");
    return NULL;
}

void* CLANHB_(void)
{
    if (verbose) puts("STUB: CLANHB_ called");
    return NULL;
}

void* CLANHE(void)
{
    if (verbose) puts("STUB: CLANHE called");
    return NULL;
}

void* CLANHE_(void)
{
    if (verbose) puts("STUB: CLANHE_ called");
    return NULL;
}

void* CLANHF(void)
{
    if (verbose) puts("STUB: CLANHF called");
    return NULL;
}

void* CLANHF_(void)
{
    if (verbose) puts("STUB: CLANHF_ called");
    return NULL;
}

void* CLANHP(void)
{
    if (verbose) puts("STUB: CLANHP called");
    return NULL;
}

void* CLANHP_(void)
{
    if (verbose) puts("STUB: CLANHP_ called");
    return NULL;
}

void* CLANHS(void)
{
    if (verbose) puts("STUB: CLANHS called");
    return NULL;
}

void* CLANHS_(void)
{
    if (verbose) puts("STUB: CLANHS_ called");
    return NULL;
}

void* CLANHT(void)
{
    if (verbose) puts("STUB: CLANHT called");
    return NULL;
}

void* CLANHT_(void)
{
    if (verbose) puts("STUB: CLANHT_ called");
    return NULL;
}

void* CLANSB(void)
{
    if (verbose) puts("STUB: CLANSB called");
    return NULL;
}

void* CLANSB_(void)
{
    if (verbose) puts("STUB: CLANSB_ called");
    return NULL;
}

void* CLANSP(void)
{
    if (verbose) puts("STUB: CLANSP called");
    return NULL;
}

void* CLANSP_(void)
{
    if (verbose) puts("STUB: CLANSP_ called");
    return NULL;
}

void* CLANSY(void)
{
    if (verbose) puts("STUB: CLANSY called");
    return NULL;
}

void* CLANSY_(void)
{
    if (verbose) puts("STUB: CLANSY_ called");
    return NULL;
}

void* CLANTB(void)
{
    if (verbose) puts("STUB: CLANTB called");
    return NULL;
}

void* CLANTB_(void)
{
    if (verbose) puts("STUB: CLANTB_ called");
    return NULL;
}

void* CLANTP(void)
{
    if (verbose) puts("STUB: CLANTP called");
    return NULL;
}

void* CLANTP_(void)
{
    if (verbose) puts("STUB: CLANTP_ called");
    return NULL;
}

void* CLANTR(void)
{
    if (verbose) puts("STUB: CLANTR called");
    return NULL;
}

void* CLANTR_(void)
{
    if (verbose) puts("STUB: CLANTR_ called");
    return NULL;
}

void* CLAPLL(void)
{
    if (verbose) puts("STUB: CLAPLL called");
    return NULL;
}

void* CLAPLL_(void)
{
    if (verbose) puts("STUB: CLAPLL_ called");
    return NULL;
}

void* CLAPMT(void)
{
    if (verbose) puts("STUB: CLAPMT called");
    return NULL;
}

void* CLAPMT_(void)
{
    if (verbose) puts("STUB: CLAPMT_ called");
    return NULL;
}

void* CLAQGB(void)
{
    if (verbose) puts("STUB: CLAQGB called");
    return NULL;
}

void* CLAQGB_(void)
{
    if (verbose) puts("STUB: CLAQGB_ called");
    return NULL;
}

void* CLAQGE(void)
{
    if (verbose) puts("STUB: CLAQGE called");
    return NULL;
}

void* CLAQGE_(void)
{
    if (verbose) puts("STUB: CLAQGE_ called");
    return NULL;
}

void* CLAQHB(void)
{
    if (verbose) puts("STUB: CLAQHB called");
    return NULL;
}

void* CLAQHB_(void)
{
    if (verbose) puts("STUB: CLAQHB_ called");
    return NULL;
}

void* CLAQHE(void)
{
    if (verbose) puts("STUB: CLAQHE called");
    return NULL;
}

void* CLAQHE_(void)
{
    if (verbose) puts("STUB: CLAQHE_ called");
    return NULL;
}

void* CLAQHP(void)
{
    if (verbose) puts("STUB: CLAQHP called");
    return NULL;
}

void* CLAQHP_(void)
{
    if (verbose) puts("STUB: CLAQHP_ called");
    return NULL;
}

void* CLAQP2(void)
{
    if (verbose) puts("STUB: CLAQP2 called");
    return NULL;
}

void* CLAQP2_(void)
{
    if (verbose) puts("STUB: CLAQP2_ called");
    return NULL;
}

void* CLAQPS(void)
{
    if (verbose) puts("STUB: CLAQPS called");
    return NULL;
}

void* CLAQPS_(void)
{
    if (verbose) puts("STUB: CLAQPS_ called");
    return NULL;
}

void* CLAQR0(void)
{
    if (verbose) puts("STUB: CLAQR0 called");
    return NULL;
}

void* CLAQR0_(void)
{
    if (verbose) puts("STUB: CLAQR0_ called");
    return NULL;
}

void* CLAQR1(void)
{
    if (verbose) puts("STUB: CLAQR1 called");
    return NULL;
}

void* CLAQR1_(void)
{
    if (verbose) puts("STUB: CLAQR1_ called");
    return NULL;
}

void* CLAQR2(void)
{
    if (verbose) puts("STUB: CLAQR2 called");
    return NULL;
}

void* CLAQR2_(void)
{
    if (verbose) puts("STUB: CLAQR2_ called");
    return NULL;
}

void* CLAQR3(void)
{
    if (verbose) puts("STUB: CLAQR3 called");
    return NULL;
}

void* CLAQR3_(void)
{
    if (verbose) puts("STUB: CLAQR3_ called");
    return NULL;
}

void* CLAQR4(void)
{
    if (verbose) puts("STUB: CLAQR4 called");
    return NULL;
}

void* CLAQR4_(void)
{
    if (verbose) puts("STUB: CLAQR4_ called");
    return NULL;
}

void* CLAQR5(void)
{
    if (verbose) puts("STUB: CLAQR5 called");
    return NULL;
}

void* CLAQR5_(void)
{
    if (verbose) puts("STUB: CLAQR5_ called");
    return NULL;
}

void* CLAQSB(void)
{
    if (verbose) puts("STUB: CLAQSB called");
    return NULL;
}

void* CLAQSB_(void)
{
    if (verbose) puts("STUB: CLAQSB_ called");
    return NULL;
}

void* CLAQSP(void)
{
    if (verbose) puts("STUB: CLAQSP called");
    return NULL;
}

void* CLAQSP_(void)
{
    if (verbose) puts("STUB: CLAQSP_ called");
    return NULL;
}

void* CLAQSY(void)
{
    if (verbose) puts("STUB: CLAQSY called");
    return NULL;
}

void* CLAQSY_(void)
{
    if (verbose) puts("STUB: CLAQSY_ called");
    return NULL;
}

void* CLAR1V(void)
{
    if (verbose) puts("STUB: CLAR1V called");
    return NULL;
}

void* CLAR1V_(void)
{
    if (verbose) puts("STUB: CLAR1V_ called");
    return NULL;
}

void* CLAR2V(void)
{
    if (verbose) puts("STUB: CLAR2V called");
    return NULL;
}

void* CLAR2V_(void)
{
    if (verbose) puts("STUB: CLAR2V_ called");
    return NULL;
}

void* CLARCM(void)
{
    if (verbose) puts("STUB: CLARCM called");
    return NULL;
}

void* CLARCM_(void)
{
    if (verbose) puts("STUB: CLARCM_ called");
    return NULL;
}

void* CLARF(void)
{
    if (verbose) puts("STUB: CLARF called");
    return NULL;
}

void* CLARFB(void)
{
    if (verbose) puts("STUB: CLARFB called");
    return NULL;
}

void* CLARFB_(void)
{
    if (verbose) puts("STUB: CLARFB_ called");
    return NULL;
}

void* CLARFG(void)
{
    if (verbose) puts("STUB: CLARFG called");
    return NULL;
}

void* CLARFG_(void)
{
    if (verbose) puts("STUB: CLARFG_ called");
    return NULL;
}

void* CLARFP(void)
{
    if (verbose) puts("STUB: CLARFP called");
    return NULL;
}

void* CLARFP_(void)
{
    if (verbose) puts("STUB: CLARFP_ called");
    return NULL;
}

void* CLARFT(void)
{
    if (verbose) puts("STUB: CLARFT called");
    return NULL;
}

void* CLARFT_(void)
{
    if (verbose) puts("STUB: CLARFT_ called");
    return NULL;
}

void* CLARFX(void)
{
    if (verbose) puts("STUB: CLARFX called");
    return NULL;
}

void* CLARFX_(void)
{
    if (verbose) puts("STUB: CLARFX_ called");
    return NULL;
}

void* CLARF_(void)
{
    if (verbose) puts("STUB: CLARF_ called");
    return NULL;
}

void* CLARGV(void)
{
    if (verbose) puts("STUB: CLARGV called");
    return NULL;
}

void* CLARGV_(void)
{
    if (verbose) puts("STUB: CLARGV_ called");
    return NULL;
}

void* CLARNV(void)
{
    if (verbose) puts("STUB: CLARNV called");
    return NULL;
}

void* CLARNV_(void)
{
    if (verbose) puts("STUB: CLARNV_ called");
    return NULL;
}

void* CLARRV(void)
{
    if (verbose) puts("STUB: CLARRV called");
    return NULL;
}

void* CLARRV_(void)
{
    if (verbose) puts("STUB: CLARRV_ called");
    return NULL;
}

void* CLARSCL2(void)
{
    if (verbose) puts("STUB: CLARSCL2 called");
    return NULL;
}

void* CLARSCL2_(void)
{
    if (verbose) puts("STUB: CLARSCL2_ called");
    return NULL;
}

void* CLARTG(void)
{
    if (verbose) puts("STUB: CLARTG called");
    return NULL;
}

void* CLARTG_(void)
{
    if (verbose) puts("STUB: CLARTG_ called");
    return NULL;
}

void* CLARTV(void)
{
    if (verbose) puts("STUB: CLARTV called");
    return NULL;
}

void* CLARTV_(void)
{
    if (verbose) puts("STUB: CLARTV_ called");
    return NULL;
}

void* CLARZ(void)
{
    if (verbose) puts("STUB: CLARZ called");
    return NULL;
}

void* CLARZB(void)
{
    if (verbose) puts("STUB: CLARZB called");
    return NULL;
}

void* CLARZB_(void)
{
    if (verbose) puts("STUB: CLARZB_ called");
    return NULL;
}

void* CLARZT(void)
{
    if (verbose) puts("STUB: CLARZT called");
    return NULL;
}

void* CLARZT_(void)
{
    if (verbose) puts("STUB: CLARZT_ called");
    return NULL;
}

void* CLARZ_(void)
{
    if (verbose) puts("STUB: CLARZ_ called");
    return NULL;
}

void* CLASCL(void)
{
    if (verbose) puts("STUB: CLASCL called");
    return NULL;
}

void* CLASCL2(void)
{
    if (verbose) puts("STUB: CLASCL2 called");
    return NULL;
}

void* CLASCL2_(void)
{
    if (verbose) puts("STUB: CLASCL2_ called");
    return NULL;
}

void* CLASCL_(void)
{
    if (verbose) puts("STUB: CLASCL_ called");
    return NULL;
}

void* CLASET(void)
{
    if (verbose) puts("STUB: CLASET called");
    return NULL;
}

void* CLASET_(void)
{
    if (verbose) puts("STUB: CLASET_ called");
    return NULL;
}

void* CLASR(void)
{
    if (verbose) puts("STUB: CLASR called");
    return NULL;
}

void* CLASR_(void)
{
    if (verbose) puts("STUB: CLASR_ called");
    return NULL;
}

void* CLASSQ(void)
{
    if (verbose) puts("STUB: CLASSQ called");
    return NULL;
}

void* CLASSQ_(void)
{
    if (verbose) puts("STUB: CLASSQ_ called");
    return NULL;
}

void* CLASWP(void)
{
    if (verbose) puts("STUB: CLASWP called");
    return NULL;
}

void* CLASWP_(void)
{
    if (verbose) puts("STUB: CLASWP_ called");
    return NULL;
}

void* CLASYF(void)
{
    if (verbose) puts("STUB: CLASYF called");
    return NULL;
}

void* CLASYF_(void)
{
    if (verbose) puts("STUB: CLASYF_ called");
    return NULL;
}

void* CLATBS(void)
{
    if (verbose) puts("STUB: CLATBS called");
    return NULL;
}

void* CLATBS_(void)
{
    if (verbose) puts("STUB: CLATBS_ called");
    return NULL;
}

void* CLATDF(void)
{
    if (verbose) puts("STUB: CLATDF called");
    return NULL;
}

void* CLATDF_(void)
{
    if (verbose) puts("STUB: CLATDF_ called");
    return NULL;
}

void* CLATPS(void)
{
    if (verbose) puts("STUB: CLATPS called");
    return NULL;
}

void* CLATPS_(void)
{
    if (verbose) puts("STUB: CLATPS_ called");
    return NULL;
}

void* CLATRD(void)
{
    if (verbose) puts("STUB: CLATRD called");
    return NULL;
}

void* CLATRD_(void)
{
    if (verbose) puts("STUB: CLATRD_ called");
    return NULL;
}

void* CLATRS(void)
{
    if (verbose) puts("STUB: CLATRS called");
    return NULL;
}

void* CLATRS_(void)
{
    if (verbose) puts("STUB: CLATRS_ called");
    return NULL;
}

void* CLATRZ(void)
{
    if (verbose) puts("STUB: CLATRZ called");
    return NULL;
}

void* CLATRZ_(void)
{
    if (verbose) puts("STUB: CLATRZ_ called");
    return NULL;
}

void* CLATZM(void)
{
    if (verbose) puts("STUB: CLATZM called");
    return NULL;
}

void* CLATZM_(void)
{
    if (verbose) puts("STUB: CLATZM_ called");
    return NULL;
}

void* CLAUU2(void)
{
    if (verbose) puts("STUB: CLAUU2 called");
    return NULL;
}

void* CLAUU2_(void)
{
    if (verbose) puts("STUB: CLAUU2_ called");
    return NULL;
}

void* CLAUUM(void)
{
    if (verbose) puts("STUB: CLAUUM called");
    return NULL;
}

void* CLAUUM_(void)
{
    if (verbose) puts("STUB: CLAUUM_ called");
    return NULL;
}

void* CPBCON(void)
{
    if (verbose) puts("STUB: CPBCON called");
    return NULL;
}

void* CPBCON_(void)
{
    if (verbose) puts("STUB: CPBCON_ called");
    return NULL;
}

void* CPBEQU(void)
{
    if (verbose) puts("STUB: CPBEQU called");
    return NULL;
}

void* CPBEQU_(void)
{
    if (verbose) puts("STUB: CPBEQU_ called");
    return NULL;
}

void* CPBRFS(void)
{
    if (verbose) puts("STUB: CPBRFS called");
    return NULL;
}

void* CPBRFS_(void)
{
    if (verbose) puts("STUB: CPBRFS_ called");
    return NULL;
}

void* CPBSTF(void)
{
    if (verbose) puts("STUB: CPBSTF called");
    return NULL;
}

void* CPBSTF_(void)
{
    if (verbose) puts("STUB: CPBSTF_ called");
    return NULL;
}

void* CPBSV(void)
{
    if (verbose) puts("STUB: CPBSV called");
    return NULL;
}

void* CPBSVX(void)
{
    if (verbose) puts("STUB: CPBSVX called");
    return NULL;
}

void* CPBSVX_(void)
{
    if (verbose) puts("STUB: CPBSVX_ called");
    return NULL;
}

void* CPBSV_(void)
{
    if (verbose) puts("STUB: CPBSV_ called");
    return NULL;
}

void* CPBTF2(void)
{
    if (verbose) puts("STUB: CPBTF2 called");
    return NULL;
}

void* CPBTF2_(void)
{
    if (verbose) puts("STUB: CPBTF2_ called");
    return NULL;
}

void* CPBTRF(void)
{
    if (verbose) puts("STUB: CPBTRF called");
    return NULL;
}

void* CPBTRF_(void)
{
    if (verbose) puts("STUB: CPBTRF_ called");
    return NULL;
}

void* CPBTRS(void)
{
    if (verbose) puts("STUB: CPBTRS called");
    return NULL;
}

void* CPBTRS_(void)
{
    if (verbose) puts("STUB: CPBTRS_ called");
    return NULL;
}

void* CPFTRF(void)
{
    if (verbose) puts("STUB: CPFTRF called");
    return NULL;
}

void* CPFTRF_(void)
{
    if (verbose) puts("STUB: CPFTRF_ called");
    return NULL;
}

void* CPFTRI(void)
{
    if (verbose) puts("STUB: CPFTRI called");
    return NULL;
}

void* CPFTRI_(void)
{
    if (verbose) puts("STUB: CPFTRI_ called");
    return NULL;
}

void* CPFTRS(void)
{
    if (verbose) puts("STUB: CPFTRS called");
    return NULL;
}

void* CPFTRS_(void)
{
    if (verbose) puts("STUB: CPFTRS_ called");
    return NULL;
}

void* CPOCON(void)
{
    if (verbose) puts("STUB: CPOCON called");
    return NULL;
}

void* CPOCON_(void)
{
    if (verbose) puts("STUB: CPOCON_ called");
    return NULL;
}

void* CPOEQU(void)
{
    if (verbose) puts("STUB: CPOEQU called");
    return NULL;
}

void* CPOEQUB(void)
{
    if (verbose) puts("STUB: CPOEQUB called");
    return NULL;
}

void* CPOEQUB_(void)
{
    if (verbose) puts("STUB: CPOEQUB_ called");
    return NULL;
}

void* CPOEQU_(void)
{
    if (verbose) puts("STUB: CPOEQU_ called");
    return NULL;
}

void* CPORFS(void)
{
    if (verbose) puts("STUB: CPORFS called");
    return NULL;
}

void* CPORFS_(void)
{
    if (verbose) puts("STUB: CPORFS_ called");
    return NULL;
}

void* CPOSV(void)
{
    if (verbose) puts("STUB: CPOSV called");
    return NULL;
}

void* CPOSVX(void)
{
    if (verbose) puts("STUB: CPOSVX called");
    return NULL;
}

void* CPOSVX_(void)
{
    if (verbose) puts("STUB: CPOSVX_ called");
    return NULL;
}

void* CPOSV_(void)
{
    if (verbose) puts("STUB: CPOSV_ called");
    return NULL;
}

void* CPOTF2(void)
{
    if (verbose) puts("STUB: CPOTF2 called");
    return NULL;
}

void* CPOTF2_(void)
{
    if (verbose) puts("STUB: CPOTF2_ called");
    return NULL;
}

void* CPOTRF(void)
{
    if (verbose) puts("STUB: CPOTRF called");
    return NULL;
}

void* CPOTRF_(void)
{
    if (verbose) puts("STUB: CPOTRF_ called");
    return NULL;
}

void* CPOTRI(void)
{
    if (verbose) puts("STUB: CPOTRI called");
    return NULL;
}

void* CPOTRI_(void)
{
    if (verbose) puts("STUB: CPOTRI_ called");
    return NULL;
}

void* CPOTRS(void)
{
    if (verbose) puts("STUB: CPOTRS called");
    return NULL;
}

void* CPOTRS_(void)
{
    if (verbose) puts("STUB: CPOTRS_ called");
    return NULL;
}

void* CPPCON(void)
{
    if (verbose) puts("STUB: CPPCON called");
    return NULL;
}

void* CPPCON_(void)
{
    if (verbose) puts("STUB: CPPCON_ called");
    return NULL;
}

void* CPPEQU(void)
{
    if (verbose) puts("STUB: CPPEQU called");
    return NULL;
}

void* CPPEQU_(void)
{
    if (verbose) puts("STUB: CPPEQU_ called");
    return NULL;
}

void* CPPRFS(void)
{
    if (verbose) puts("STUB: CPPRFS called");
    return NULL;
}

void* CPPRFS_(void)
{
    if (verbose) puts("STUB: CPPRFS_ called");
    return NULL;
}

void* CPPSV(void)
{
    if (verbose) puts("STUB: CPPSV called");
    return NULL;
}

void* CPPSVX(void)
{
    if (verbose) puts("STUB: CPPSVX called");
    return NULL;
}

void* CPPSVX_(void)
{
    if (verbose) puts("STUB: CPPSVX_ called");
    return NULL;
}

void* CPPSV_(void)
{
    if (verbose) puts("STUB: CPPSV_ called");
    return NULL;
}

void* CPPTRF(void)
{
    if (verbose) puts("STUB: CPPTRF called");
    return NULL;
}

void* CPPTRF_(void)
{
    if (verbose) puts("STUB: CPPTRF_ called");
    return NULL;
}

void* CPPTRI(void)
{
    if (verbose) puts("STUB: CPPTRI called");
    return NULL;
}

void* CPPTRI_(void)
{
    if (verbose) puts("STUB: CPPTRI_ called");
    return NULL;
}

void* CPPTRS(void)
{
    if (verbose) puts("STUB: CPPTRS called");
    return NULL;
}

void* CPPTRS_(void)
{
    if (verbose) puts("STUB: CPPTRS_ called");
    return NULL;
}

void* CPSTF2(void)
{
    if (verbose) puts("STUB: CPSTF2 called");
    return NULL;
}

void* CPSTF2_(void)
{
    if (verbose) puts("STUB: CPSTF2_ called");
    return NULL;
}

void* CPSTRF(void)
{
    if (verbose) puts("STUB: CPSTRF called");
    return NULL;
}

void* CPSTRF_(void)
{
    if (verbose) puts("STUB: CPSTRF_ called");
    return NULL;
}

void* CPTCON(void)
{
    if (verbose) puts("STUB: CPTCON called");
    return NULL;
}

void* CPTCON_(void)
{
    if (verbose) puts("STUB: CPTCON_ called");
    return NULL;
}

void* CPTEQR(void)
{
    if (verbose) puts("STUB: CPTEQR called");
    return NULL;
}

void* CPTEQR_(void)
{
    if (verbose) puts("STUB: CPTEQR_ called");
    return NULL;
}

void* CPTRFS(void)
{
    if (verbose) puts("STUB: CPTRFS called");
    return NULL;
}

void* CPTRFS_(void)
{
    if (verbose) puts("STUB: CPTRFS_ called");
    return NULL;
}

void* CPTSV(void)
{
    if (verbose) puts("STUB: CPTSV called");
    return NULL;
}

void* CPTSVX(void)
{
    if (verbose) puts("STUB: CPTSVX called");
    return NULL;
}

void* CPTSVX_(void)
{
    if (verbose) puts("STUB: CPTSVX_ called");
    return NULL;
}

void* CPTSV_(void)
{
    if (verbose) puts("STUB: CPTSV_ called");
    return NULL;
}

void* CPTTRF(void)
{
    if (verbose) puts("STUB: CPTTRF called");
    return NULL;
}

void* CPTTRF_(void)
{
    if (verbose) puts("STUB: CPTTRF_ called");
    return NULL;
}

void* CPTTRS(void)
{
    if (verbose) puts("STUB: CPTTRS called");
    return NULL;
}

void* CPTTRS_(void)
{
    if (verbose) puts("STUB: CPTTRS_ called");
    return NULL;
}

void* CPTTS2(void)
{
    if (verbose) puts("STUB: CPTTS2 called");
    return NULL;
}

void* CPTTS2_(void)
{
    if (verbose) puts("STUB: CPTTS2_ called");
    return NULL;
}

void* CROT(void)
{
    if (verbose) puts("STUB: CROT called");
    return NULL;
}

void* CROT_(void)
{
    if (verbose) puts("STUB: CROT_ called");
    return NULL;
}

void* CSPCON(void)
{
    if (verbose) puts("STUB: CSPCON called");
    return NULL;
}

void* CSPCON_(void)
{
    if (verbose) puts("STUB: CSPCON_ called");
    return NULL;
}

void* CSPMV(void)
{
    if (verbose) puts("STUB: CSPMV called");
    return NULL;
}

void* CSPMV_(void)
{
    if (verbose) puts("STUB: CSPMV_ called");
    return NULL;
}

void* CSPR(void)
{
    if (verbose) puts("STUB: CSPR called");
    return NULL;
}

void* CSPRFS(void)
{
    if (verbose) puts("STUB: CSPRFS called");
    return NULL;
}

void* CSPRFS_(void)
{
    if (verbose) puts("STUB: CSPRFS_ called");
    return NULL;
}

void* CSPR_(void)
{
    if (verbose) puts("STUB: CSPR_ called");
    return NULL;
}

void* CSPSV(void)
{
    if (verbose) puts("STUB: CSPSV called");
    return NULL;
}

void* CSPSVX(void)
{
    if (verbose) puts("STUB: CSPSVX called");
    return NULL;
}

void* CSPSVX_(void)
{
    if (verbose) puts("STUB: CSPSVX_ called");
    return NULL;
}

void* CSPSV_(void)
{
    if (verbose) puts("STUB: CSPSV_ called");
    return NULL;
}

void* CSPTRF(void)
{
    if (verbose) puts("STUB: CSPTRF called");
    return NULL;
}

void* CSPTRF_(void)
{
    if (verbose) puts("STUB: CSPTRF_ called");
    return NULL;
}

void* CSPTRI(void)
{
    if (verbose) puts("STUB: CSPTRI called");
    return NULL;
}

void* CSPTRI_(void)
{
    if (verbose) puts("STUB: CSPTRI_ called");
    return NULL;
}

void* CSPTRS(void)
{
    if (verbose) puts("STUB: CSPTRS called");
    return NULL;
}

void* CSPTRS_(void)
{
    if (verbose) puts("STUB: CSPTRS_ called");
    return NULL;
}

void* CSRSCL(void)
{
    if (verbose) puts("STUB: CSRSCL called");
    return NULL;
}

void* CSRSCL_(void)
{
    if (verbose) puts("STUB: CSRSCL_ called");
    return NULL;
}

void* CSTEDC(void)
{
    if (verbose) puts("STUB: CSTEDC called");
    return NULL;
}

void* CSTEDC_(void)
{
    if (verbose) puts("STUB: CSTEDC_ called");
    return NULL;
}

void* CSTEGR(void)
{
    if (verbose) puts("STUB: CSTEGR called");
    return NULL;
}

void* CSTEGR_(void)
{
    if (verbose) puts("STUB: CSTEGR_ called");
    return NULL;
}

void* CSTEIN(void)
{
    if (verbose) puts("STUB: CSTEIN called");
    return NULL;
}

void* CSTEIN_(void)
{
    if (verbose) puts("STUB: CSTEIN_ called");
    return NULL;
}

void* CSTEMR(void)
{
    if (verbose) puts("STUB: CSTEMR called");
    return NULL;
}

void* CSTEMR_(void)
{
    if (verbose) puts("STUB: CSTEMR_ called");
    return NULL;
}

void* CSTEQR(void)
{
    if (verbose) puts("STUB: CSTEQR called");
    return NULL;
}

void* CSTEQR_(void)
{
    if (verbose) puts("STUB: CSTEQR_ called");
    return NULL;
}

void* CSYCON(void)
{
    if (verbose) puts("STUB: CSYCON called");
    return NULL;
}

void* CSYCON_(void)
{
    if (verbose) puts("STUB: CSYCON_ called");
    return NULL;
}

void* CSYEQUB(void)
{
    if (verbose) puts("STUB: CSYEQUB called");
    return NULL;
}

void* CSYEQUB_(void)
{
    if (verbose) puts("STUB: CSYEQUB_ called");
    return NULL;
}

void* CSYMV(void)
{
    if (verbose) puts("STUB: CSYMV called");
    return NULL;
}

void* CSYMV_(void)
{
    if (verbose) puts("STUB: CSYMV_ called");
    return NULL;
}

void* CSYR(void)
{
    if (verbose) puts("STUB: CSYR called");
    return NULL;
}

void* CSYRFS(void)
{
    if (verbose) puts("STUB: CSYRFS called");
    return NULL;
}

void* CSYRFS_(void)
{
    if (verbose) puts("STUB: CSYRFS_ called");
    return NULL;
}

void* CSYR_(void)
{
    if (verbose) puts("STUB: CSYR_ called");
    return NULL;
}

void* CSYSV(void)
{
    if (verbose) puts("STUB: CSYSV called");
    return NULL;
}

void* CSYSVX(void)
{
    if (verbose) puts("STUB: CSYSVX called");
    return NULL;
}

void* CSYSVX_(void)
{
    if (verbose) puts("STUB: CSYSVX_ called");
    return NULL;
}

void* CSYSV_(void)
{
    if (verbose) puts("STUB: CSYSV_ called");
    return NULL;
}

void* CSYTF2(void)
{
    if (verbose) puts("STUB: CSYTF2 called");
    return NULL;
}

void* CSYTF2_(void)
{
    if (verbose) puts("STUB: CSYTF2_ called");
    return NULL;
}

void* CSYTRF(void)
{
    if (verbose) puts("STUB: CSYTRF called");
    return NULL;
}

void* CSYTRF_(void)
{
    if (verbose) puts("STUB: CSYTRF_ called");
    return NULL;
}

void* CSYTRI(void)
{
    if (verbose) puts("STUB: CSYTRI called");
    return NULL;
}

void* CSYTRI_(void)
{
    if (verbose) puts("STUB: CSYTRI_ called");
    return NULL;
}

void* CSYTRS(void)
{
    if (verbose) puts("STUB: CSYTRS called");
    return NULL;
}

void* CSYTRS_(void)
{
    if (verbose) puts("STUB: CSYTRS_ called");
    return NULL;
}

void* CTBCON(void)
{
    if (verbose) puts("STUB: CTBCON called");
    return NULL;
}

void* CTBCON_(void)
{
    if (verbose) puts("STUB: CTBCON_ called");
    return NULL;
}

void* CTBRFS(void)
{
    if (verbose) puts("STUB: CTBRFS called");
    return NULL;
}

void* CTBRFS_(void)
{
    if (verbose) puts("STUB: CTBRFS_ called");
    return NULL;
}

void* CTBTRS(void)
{
    if (verbose) puts("STUB: CTBTRS called");
    return NULL;
}

void* CTBTRS_(void)
{
    if (verbose) puts("STUB: CTBTRS_ called");
    return NULL;
}

void* CTFSM(void)
{
    if (verbose) puts("STUB: CTFSM called");
    return NULL;
}

void* CTFSM_(void)
{
    if (verbose) puts("STUB: CTFSM_ called");
    return NULL;
}

void* CTFTRI(void)
{
    if (verbose) puts("STUB: CTFTRI called");
    return NULL;
}

void* CTFTRI_(void)
{
    if (verbose) puts("STUB: CTFTRI_ called");
    return NULL;
}

void* CTFTTP(void)
{
    if (verbose) puts("STUB: CTFTTP called");
    return NULL;
}

void* CTFTTP_(void)
{
    if (verbose) puts("STUB: CTFTTP_ called");
    return NULL;
}

void* CTFTTR(void)
{
    if (verbose) puts("STUB: CTFTTR called");
    return NULL;
}

void* CTFTTR_(void)
{
    if (verbose) puts("STUB: CTFTTR_ called");
    return NULL;
}

void* CTGEVC(void)
{
    if (verbose) puts("STUB: CTGEVC called");
    return NULL;
}

void* CTGEVC_(void)
{
    if (verbose) puts("STUB: CTGEVC_ called");
    return NULL;
}

void* CTGEX2(void)
{
    if (verbose) puts("STUB: CTGEX2 called");
    return NULL;
}

void* CTGEX2_(void)
{
    if (verbose) puts("STUB: CTGEX2_ called");
    return NULL;
}

void* CTGEXC(void)
{
    if (verbose) puts("STUB: CTGEXC called");
    return NULL;
}

void* CTGEXC_(void)
{
    if (verbose) puts("STUB: CTGEXC_ called");
    return NULL;
}

void* CTGSEN(void)
{
    if (verbose) puts("STUB: CTGSEN called");
    return NULL;
}

void* CTGSEN_(void)
{
    if (verbose) puts("STUB: CTGSEN_ called");
    return NULL;
}

void* CTGSJA(void)
{
    if (verbose) puts("STUB: CTGSJA called");
    return NULL;
}

void* CTGSJA_(void)
{
    if (verbose) puts("STUB: CTGSJA_ called");
    return NULL;
}

void* CTGSNA(void)
{
    if (verbose) puts("STUB: CTGSNA called");
    return NULL;
}

void* CTGSNA_(void)
{
    if (verbose) puts("STUB: CTGSNA_ called");
    return NULL;
}

void* CTGSY2(void)
{
    if (verbose) puts("STUB: CTGSY2 called");
    return NULL;
}

void* CTGSY2_(void)
{
    if (verbose) puts("STUB: CTGSY2_ called");
    return NULL;
}

void* CTGSYL(void)
{
    if (verbose) puts("STUB: CTGSYL called");
    return NULL;
}

void* CTGSYL_(void)
{
    if (verbose) puts("STUB: CTGSYL_ called");
    return NULL;
}

void* CTPCON(void)
{
    if (verbose) puts("STUB: CTPCON called");
    return NULL;
}

void* CTPCON_(void)
{
    if (verbose) puts("STUB: CTPCON_ called");
    return NULL;
}

void* CTPRFS(void)
{
    if (verbose) puts("STUB: CTPRFS called");
    return NULL;
}

void* CTPRFS_(void)
{
    if (verbose) puts("STUB: CTPRFS_ called");
    return NULL;
}

void* CTPTRI(void)
{
    if (verbose) puts("STUB: CTPTRI called");
    return NULL;
}

void* CTPTRI_(void)
{
    if (verbose) puts("STUB: CTPTRI_ called");
    return NULL;
}

void* CTPTRS(void)
{
    if (verbose) puts("STUB: CTPTRS called");
    return NULL;
}

void* CTPTRS_(void)
{
    if (verbose) puts("STUB: CTPTRS_ called");
    return NULL;
}

void* CTPTTF(void)
{
    if (verbose) puts("STUB: CTPTTF called");
    return NULL;
}

void* CTPTTF_(void)
{
    if (verbose) puts("STUB: CTPTTF_ called");
    return NULL;
}

void* CTPTTR(void)
{
    if (verbose) puts("STUB: CTPTTR called");
    return NULL;
}

void* CTPTTR_(void)
{
    if (verbose) puts("STUB: CTPTTR_ called");
    return NULL;
}

void* CTRCON(void)
{
    if (verbose) puts("STUB: CTRCON called");
    return NULL;
}

void* CTRCON_(void)
{
    if (verbose) puts("STUB: CTRCON_ called");
    return NULL;
}

void* CTREVC(void)
{
    if (verbose) puts("STUB: CTREVC called");
    return NULL;
}

void* CTREVC_(void)
{
    if (verbose) puts("STUB: CTREVC_ called");
    return NULL;
}

void* CTREXC(void)
{
    if (verbose) puts("STUB: CTREXC called");
    return NULL;
}

void* CTREXC_(void)
{
    if (verbose) puts("STUB: CTREXC_ called");
    return NULL;
}

void* CTRRFS(void)
{
    if (verbose) puts("STUB: CTRRFS called");
    return NULL;
}

void* CTRRFS_(void)
{
    if (verbose) puts("STUB: CTRRFS_ called");
    return NULL;
}

void* CTRSEN(void)
{
    if (verbose) puts("STUB: CTRSEN called");
    return NULL;
}

void* CTRSEN_(void)
{
    if (verbose) puts("STUB: CTRSEN_ called");
    return NULL;
}

void* CTRSNA(void)
{
    if (verbose) puts("STUB: CTRSNA called");
    return NULL;
}

void* CTRSNA_(void)
{
    if (verbose) puts("STUB: CTRSNA_ called");
    return NULL;
}

void* CTRSYL(void)
{
    if (verbose) puts("STUB: CTRSYL called");
    return NULL;
}

void* CTRSYL_(void)
{
    if (verbose) puts("STUB: CTRSYL_ called");
    return NULL;
}

void* CTRTI2(void)
{
    if (verbose) puts("STUB: CTRTI2 called");
    return NULL;
}

void* CTRTI2_(void)
{
    if (verbose) puts("STUB: CTRTI2_ called");
    return NULL;
}

void* CTRTRI(void)
{
    if (verbose) puts("STUB: CTRTRI called");
    return NULL;
}

void* CTRTRI_(void)
{
    if (verbose) puts("STUB: CTRTRI_ called");
    return NULL;
}

void* CTRTRS(void)
{
    if (verbose) puts("STUB: CTRTRS called");
    return NULL;
}

void* CTRTRS_(void)
{
    if (verbose) puts("STUB: CTRTRS_ called");
    return NULL;
}

void* CTRTTF(void)
{
    if (verbose) puts("STUB: CTRTTF called");
    return NULL;
}

void* CTRTTF_(void)
{
    if (verbose) puts("STUB: CTRTTF_ called");
    return NULL;
}

void* CTRTTP(void)
{
    if (verbose) puts("STUB: CTRTTP called");
    return NULL;
}

void* CTRTTP_(void)
{
    if (verbose) puts("STUB: CTRTTP_ called");
    return NULL;
}

void* CTZRQF(void)
{
    if (verbose) puts("STUB: CTZRQF called");
    return NULL;
}

void* CTZRQF_(void)
{
    if (verbose) puts("STUB: CTZRQF_ called");
    return NULL;
}

void* CTZRZF(void)
{
    if (verbose) puts("STUB: CTZRZF called");
    return NULL;
}

void* CTZRZF_(void)
{
    if (verbose) puts("STUB: CTZRZF_ called");
    return NULL;
}

void* CUNG2L(void)
{
    if (verbose) puts("STUB: CUNG2L called");
    return NULL;
}

void* CUNG2L_(void)
{
    if (verbose) puts("STUB: CUNG2L_ called");
    return NULL;
}

void* CUNG2R(void)
{
    if (verbose) puts("STUB: CUNG2R called");
    return NULL;
}

void* CUNG2R_(void)
{
    if (verbose) puts("STUB: CUNG2R_ called");
    return NULL;
}

void* CUNGBR(void)
{
    if (verbose) puts("STUB: CUNGBR called");
    return NULL;
}

void* CUNGBR_(void)
{
    if (verbose) puts("STUB: CUNGBR_ called");
    return NULL;
}

void* CUNGHR(void)
{
    if (verbose) puts("STUB: CUNGHR called");
    return NULL;
}

void* CUNGHR_(void)
{
    if (verbose) puts("STUB: CUNGHR_ called");
    return NULL;
}

void* CUNGL2(void)
{
    if (verbose) puts("STUB: CUNGL2 called");
    return NULL;
}

void* CUNGL2_(void)
{
    if (verbose) puts("STUB: CUNGL2_ called");
    return NULL;
}

void* CUNGLQ(void)
{
    if (verbose) puts("STUB: CUNGLQ called");
    return NULL;
}

void* CUNGLQ_(void)
{
    if (verbose) puts("STUB: CUNGLQ_ called");
    return NULL;
}

void* CUNGQL(void)
{
    if (verbose) puts("STUB: CUNGQL called");
    return NULL;
}

void* CUNGQL_(void)
{
    if (verbose) puts("STUB: CUNGQL_ called");
    return NULL;
}

void* CUNGQR(void)
{
    if (verbose) puts("STUB: CUNGQR called");
    return NULL;
}

void* CUNGQR_(void)
{
    if (verbose) puts("STUB: CUNGQR_ called");
    return NULL;
}

void* CUNGR2(void)
{
    if (verbose) puts("STUB: CUNGR2 called");
    return NULL;
}

void* CUNGR2_(void)
{
    if (verbose) puts("STUB: CUNGR2_ called");
    return NULL;
}

void* CUNGRQ(void)
{
    if (verbose) puts("STUB: CUNGRQ called");
    return NULL;
}

void* CUNGRQ_(void)
{
    if (verbose) puts("STUB: CUNGRQ_ called");
    return NULL;
}

void* CUNGTR(void)
{
    if (verbose) puts("STUB: CUNGTR called");
    return NULL;
}

void* CUNGTR_(void)
{
    if (verbose) puts("STUB: CUNGTR_ called");
    return NULL;
}

void* CUNM2L(void)
{
    if (verbose) puts("STUB: CUNM2L called");
    return NULL;
}

void* CUNM2L_(void)
{
    if (verbose) puts("STUB: CUNM2L_ called");
    return NULL;
}

void* CUNM2R(void)
{
    if (verbose) puts("STUB: CUNM2R called");
    return NULL;
}

void* CUNM2R_(void)
{
    if (verbose) puts("STUB: CUNM2R_ called");
    return NULL;
}

void* CUNMBR(void)
{
    if (verbose) puts("STUB: CUNMBR called");
    return NULL;
}

void* CUNMBR_(void)
{
    if (verbose) puts("STUB: CUNMBR_ called");
    return NULL;
}

void* CUNMHR(void)
{
    if (verbose) puts("STUB: CUNMHR called");
    return NULL;
}

void* CUNMHR_(void)
{
    if (verbose) puts("STUB: CUNMHR_ called");
    return NULL;
}

void* CUNML2(void)
{
    if (verbose) puts("STUB: CUNML2 called");
    return NULL;
}

void* CUNML2_(void)
{
    if (verbose) puts("STUB: CUNML2_ called");
    return NULL;
}

void* CUNMLQ(void)
{
    if (verbose) puts("STUB: CUNMLQ called");
    return NULL;
}

void* CUNMLQ_(void)
{
    if (verbose) puts("STUB: CUNMLQ_ called");
    return NULL;
}

void* CUNMQL(void)
{
    if (verbose) puts("STUB: CUNMQL called");
    return NULL;
}

void* CUNMQL_(void)
{
    if (verbose) puts("STUB: CUNMQL_ called");
    return NULL;
}

void* CUNMQR(void)
{
    if (verbose) puts("STUB: CUNMQR called");
    return NULL;
}

void* CUNMQR_(void)
{
    if (verbose) puts("STUB: CUNMQR_ called");
    return NULL;
}

void* CUNMR2(void)
{
    if (verbose) puts("STUB: CUNMR2 called");
    return NULL;
}

void* CUNMR2_(void)
{
    if (verbose) puts("STUB: CUNMR2_ called");
    return NULL;
}

void* CUNMR3(void)
{
    if (verbose) puts("STUB: CUNMR3 called");
    return NULL;
}

void* CUNMR3_(void)
{
    if (verbose) puts("STUB: CUNMR3_ called");
    return NULL;
}

void* CUNMRQ(void)
{
    if (verbose) puts("STUB: CUNMRQ called");
    return NULL;
}

void* CUNMRQ_(void)
{
    if (verbose) puts("STUB: CUNMRQ_ called");
    return NULL;
}

void* CUNMRZ(void)
{
    if (verbose) puts("STUB: CUNMRZ called");
    return NULL;
}

void* CUNMRZ_(void)
{
    if (verbose) puts("STUB: CUNMRZ_ called");
    return NULL;
}

void* CUNMTR(void)
{
    if (verbose) puts("STUB: CUNMTR called");
    return NULL;
}

void* CUNMTR_(void)
{
    if (verbose) puts("STUB: CUNMTR_ called");
    return NULL;
}

void* CUPGTR(void)
{
    if (verbose) puts("STUB: CUPGTR called");
    return NULL;
}

void* CUPGTR_(void)
{
    if (verbose) puts("STUB: CUPGTR_ called");
    return NULL;
}

void* CUPMTR(void)
{
    if (verbose) puts("STUB: CUPMTR called");
    return NULL;
}

void* CUPMTR_(void)
{
    if (verbose) puts("STUB: CUPMTR_ called");
    return NULL;
}

void* DBDSDC(void)
{
    if (verbose) puts("STUB: DBDSDC called");
    return NULL;
}

void* DBDSDC_(void)
{
    if (verbose) puts("STUB: DBDSDC_ called");
    return NULL;
}

void* DBDSQR(void)
{
    if (verbose) puts("STUB: DBDSQR called");
    return NULL;
}

void* DBDSQR_(void)
{
    if (verbose) puts("STUB: DBDSQR_ called");
    return NULL;
}

void* DDISNA(void)
{
    if (verbose) puts("STUB: DDISNA called");
    return NULL;
}

void* DDISNA_(void)
{
    if (verbose) puts("STUB: DDISNA_ called");
    return NULL;
}

void* DGBBRD(void)
{
    if (verbose) puts("STUB: DGBBRD called");
    return NULL;
}

void* DGBBRD_(void)
{
    if (verbose) puts("STUB: DGBBRD_ called");
    return NULL;
}

void* DGBCON(void)
{
    if (verbose) puts("STUB: DGBCON called");
    return NULL;
}

void* DGBCON_(void)
{
    if (verbose) puts("STUB: DGBCON_ called");
    return NULL;
}

void* DGBEQU(void)
{
    if (verbose) puts("STUB: DGBEQU called");
    return NULL;
}

void* DGBEQUB(void)
{
    if (verbose) puts("STUB: DGBEQUB called");
    return NULL;
}

void* DGBEQUB_(void)
{
    if (verbose) puts("STUB: DGBEQUB_ called");
    return NULL;
}

void* DGBEQU_(void)
{
    if (verbose) puts("STUB: DGBEQU_ called");
    return NULL;
}

void* DGBRFS(void)
{
    if (verbose) puts("STUB: DGBRFS called");
    return NULL;
}

void* DGBRFS_(void)
{
    if (verbose) puts("STUB: DGBRFS_ called");
    return NULL;
}

void* DGBSV(void)
{
    if (verbose) puts("STUB: DGBSV called");
    return NULL;
}

void* DGBSVX(void)
{
    if (verbose) puts("STUB: DGBSVX called");
    return NULL;
}

void* DGBSVX_(void)
{
    if (verbose) puts("STUB: DGBSVX_ called");
    return NULL;
}

void* DGBSV_(void)
{
    if (verbose) puts("STUB: DGBSV_ called");
    return NULL;
}

void* DGBTF2(void)
{
    if (verbose) puts("STUB: DGBTF2 called");
    return NULL;
}

void* DGBTF2_(void)
{
    if (verbose) puts("STUB: DGBTF2_ called");
    return NULL;
}

void* DGBTRF(void)
{
    if (verbose) puts("STUB: DGBTRF called");
    return NULL;
}

void* DGBTRF_(void)
{
    if (verbose) puts("STUB: DGBTRF_ called");
    return NULL;
}

void* DGBTRS(void)
{
    if (verbose) puts("STUB: DGBTRS called");
    return NULL;
}

void* DGBTRS_(void)
{
    if (verbose) puts("STUB: DGBTRS_ called");
    return NULL;
}

void* DGEBAK(void)
{
    if (verbose) puts("STUB: DGEBAK called");
    return NULL;
}

void* DGEBAK_(void)
{
    if (verbose) puts("STUB: DGEBAK_ called");
    return NULL;
}

void* DGEBAL(void)
{
    if (verbose) puts("STUB: DGEBAL called");
    return NULL;
}

void* DGEBAL_(void)
{
    if (verbose) puts("STUB: DGEBAL_ called");
    return NULL;
}

void* DGEBD2(void)
{
    if (verbose) puts("STUB: DGEBD2 called");
    return NULL;
}

void* DGEBD2_(void)
{
    if (verbose) puts("STUB: DGEBD2_ called");
    return NULL;
}

void* DGEBRD(void)
{
    if (verbose) puts("STUB: DGEBRD called");
    return NULL;
}

void* DGEBRD_(void)
{
    if (verbose) puts("STUB: DGEBRD_ called");
    return NULL;
}

void* DGECON(void)
{
    if (verbose) puts("STUB: DGECON called");
    return NULL;
}

void* DGECON_(void)
{
    if (verbose) puts("STUB: DGECON_ called");
    return NULL;
}

void* DGEEQU(void)
{
    if (verbose) puts("STUB: DGEEQU called");
    return NULL;
}

void* DGEEQUB(void)
{
    if (verbose) puts("STUB: DGEEQUB called");
    return NULL;
}

void* DGEEQUB_(void)
{
    if (verbose) puts("STUB: DGEEQUB_ called");
    return NULL;
}

void* DGEEQU_(void)
{
    if (verbose) puts("STUB: DGEEQU_ called");
    return NULL;
}

void* DGEES(void)
{
    if (verbose) puts("STUB: DGEES called");
    return NULL;
}

void* DGEESX(void)
{
    if (verbose) puts("STUB: DGEESX called");
    return NULL;
}

void* DGEESX_(void)
{
    if (verbose) puts("STUB: DGEESX_ called");
    return NULL;
}

void* DGEES_(void)
{
    if (verbose) puts("STUB: DGEES_ called");
    return NULL;
}

void* DGEEV(void)
{
    if (verbose) puts("STUB: DGEEV called");
    return NULL;
}

void* DGEEVX(void)
{
    if (verbose) puts("STUB: DGEEVX called");
    return NULL;
}

void* DGEEVX_(void)
{
    if (verbose) puts("STUB: DGEEVX_ called");
    return NULL;
}

void* DGEEV_(void)
{
    if (verbose) puts("STUB: DGEEV_ called");
    return NULL;
}

void* DGEGS(void)
{
    if (verbose) puts("STUB: DGEGS called");
    return NULL;
}

void* DGEGS_(void)
{
    if (verbose) puts("STUB: DGEGS_ called");
    return NULL;
}

void* DGEGV(void)
{
    if (verbose) puts("STUB: DGEGV called");
    return NULL;
}

void* DGEGV_(void)
{
    if (verbose) puts("STUB: DGEGV_ called");
    return NULL;
}

void* DGEHD2(void)
{
    if (verbose) puts("STUB: DGEHD2 called");
    return NULL;
}

void* DGEHD2_(void)
{
    if (verbose) puts("STUB: DGEHD2_ called");
    return NULL;
}

void* DGEHRD(void)
{
    if (verbose) puts("STUB: DGEHRD called");
    return NULL;
}

void* DGEHRD_(void)
{
    if (verbose) puts("STUB: DGEHRD_ called");
    return NULL;
}

void* DGEJSV(void)
{
    if (verbose) puts("STUB: DGEJSV called");
    return NULL;
}

void* DGEJSV_(void)
{
    if (verbose) puts("STUB: DGEJSV_ called");
    return NULL;
}

void* DGELQ2(void)
{
    if (verbose) puts("STUB: DGELQ2 called");
    return NULL;
}

void* DGELQ2_(void)
{
    if (verbose) puts("STUB: DGELQ2_ called");
    return NULL;
}

void* DGELQF(void)
{
    if (verbose) puts("STUB: DGELQF called");
    return NULL;
}

void* DGELQF_(void)
{
    if (verbose) puts("STUB: DGELQF_ called");
    return NULL;
}

void* DGELS(void)
{
    if (verbose) puts("STUB: DGELS called");
    return NULL;
}

void* DGELSD(void)
{
    if (verbose) puts("STUB: DGELSD called");
    return NULL;
}

void* DGELSD_(void)
{
    if (verbose) puts("STUB: DGELSD_ called");
    return NULL;
}

void* DGELSS(void)
{
    if (verbose) puts("STUB: DGELSS called");
    return NULL;
}

void* DGELSS_(void)
{
    if (verbose) puts("STUB: DGELSS_ called");
    return NULL;
}

void* DGELSX(void)
{
    if (verbose) puts("STUB: DGELSX called");
    return NULL;
}

void* DGELSX_(void)
{
    if (verbose) puts("STUB: DGELSX_ called");
    return NULL;
}

void* DGELSY(void)
{
    if (verbose) puts("STUB: DGELSY called");
    return NULL;
}

void* DGELSY_(void)
{
    if (verbose) puts("STUB: DGELSY_ called");
    return NULL;
}

void* DGELS_(void)
{
    if (verbose) puts("STUB: DGELS_ called");
    return NULL;
}

void* DGEQL2(void)
{
    if (verbose) puts("STUB: DGEQL2 called");
    return NULL;
}

void* DGEQL2_(void)
{
    if (verbose) puts("STUB: DGEQL2_ called");
    return NULL;
}

void* DGEQLF(void)
{
    if (verbose) puts("STUB: DGEQLF called");
    return NULL;
}

void* DGEQLF_(void)
{
    if (verbose) puts("STUB: DGEQLF_ called");
    return NULL;
}

void* DGEQP3(void)
{
    if (verbose) puts("STUB: DGEQP3 called");
    return NULL;
}

void* DGEQP3_(void)
{
    if (verbose) puts("STUB: DGEQP3_ called");
    return NULL;
}

void* DGEQPF(void)
{
    if (verbose) puts("STUB: DGEQPF called");
    return NULL;
}

void* DGEQPF_(void)
{
    if (verbose) puts("STUB: DGEQPF_ called");
    return NULL;
}

void* DGEQR2(void)
{
    if (verbose) puts("STUB: DGEQR2 called");
    return NULL;
}

void* DGEQR2_(void)
{
    if (verbose) puts("STUB: DGEQR2_ called");
    return NULL;
}

void* DGEQRF(void)
{
    if (verbose) puts("STUB: DGEQRF called");
    return NULL;
}

void* DGEQRF_(void)
{
    if (verbose) puts("STUB: DGEQRF_ called");
    return NULL;
}

void* DGERFS(void)
{
    if (verbose) puts("STUB: DGERFS called");
    return NULL;
}

void* DGERFS_(void)
{
    if (verbose) puts("STUB: DGERFS_ called");
    return NULL;
}

void* DGERQ2(void)
{
    if (verbose) puts("STUB: DGERQ2 called");
    return NULL;
}

void* DGERQ2_(void)
{
    if (verbose) puts("STUB: DGERQ2_ called");
    return NULL;
}

void* DGERQF(void)
{
    if (verbose) puts("STUB: DGERQF called");
    return NULL;
}

void* DGERQF_(void)
{
    if (verbose) puts("STUB: DGERQF_ called");
    return NULL;
}

void* DGESC2(void)
{
    if (verbose) puts("STUB: DGESC2 called");
    return NULL;
}

void* DGESC2_(void)
{
    if (verbose) puts("STUB: DGESC2_ called");
    return NULL;
}

void* DGESDD(void)
{
    if (verbose) puts("STUB: DGESDD called");
    return NULL;
}

void* DGESDD_(void)
{
    if (verbose) puts("STUB: DGESDD_ called");
    return NULL;
}

void* DGESV(void)
{
    if (verbose) puts("STUB: DGESV called");
    return NULL;
}

void* DGESVD(void)
{
    if (verbose) puts("STUB: DGESVD called");
    return NULL;
}

void* DGESVD_(void)
{
    if (verbose) puts("STUB: DGESVD_ called");
    return NULL;
}

void* DGESVJ(void)
{
    if (verbose) puts("STUB: DGESVJ called");
    return NULL;
}

void* DGESVJ_(void)
{
    if (verbose) puts("STUB: DGESVJ_ called");
    return NULL;
}

void* DGESVX(void)
{
    if (verbose) puts("STUB: DGESVX called");
    return NULL;
}

void* DGESVX_(void)
{
    if (verbose) puts("STUB: DGESVX_ called");
    return NULL;
}

void* DGESV_(void)
{
    if (verbose) puts("STUB: DGESV_ called");
    return NULL;
}

void* DGETC2(void)
{
    if (verbose) puts("STUB: DGETC2 called");
    return NULL;
}

void* DGETC2_(void)
{
    if (verbose) puts("STUB: DGETC2_ called");
    return NULL;
}

void* DGETF2(void)
{
    if (verbose) puts("STUB: DGETF2 called");
    return NULL;
}

void* DGETF2_(void)
{
    if (verbose) puts("STUB: DGETF2_ called");
    return NULL;
}

void* DGETRF(void)
{
    if (verbose) puts("STUB: DGETRF called");
    return NULL;
}

void* DGETRF_(void)
{
    if (verbose) puts("STUB: DGETRF_ called");
    return NULL;
}

void* DGETRI(void)
{
    if (verbose) puts("STUB: DGETRI called");
    return NULL;
}

void* DGETRI_(void)
{
    if (verbose) puts("STUB: DGETRI_ called");
    return NULL;
}

void* DGETRS(void)
{
    if (verbose) puts("STUB: DGETRS called");
    return NULL;
}

void* DGETRS_(void)
{
    if (verbose) puts("STUB: DGETRS_ called");
    return NULL;
}

void* DGGBAK(void)
{
    if (verbose) puts("STUB: DGGBAK called");
    return NULL;
}

void* DGGBAK_(void)
{
    if (verbose) puts("STUB: DGGBAK_ called");
    return NULL;
}

void* DGGBAL(void)
{
    if (verbose) puts("STUB: DGGBAL called");
    return NULL;
}

void* DGGBAL_(void)
{
    if (verbose) puts("STUB: DGGBAL_ called");
    return NULL;
}

void* DGGES(void)
{
    if (verbose) puts("STUB: DGGES called");
    return NULL;
}

void* DGGESX(void)
{
    if (verbose) puts("STUB: DGGESX called");
    return NULL;
}

void* DGGESX_(void)
{
    if (verbose) puts("STUB: DGGESX_ called");
    return NULL;
}

void* DGGES_(void)
{
    if (verbose) puts("STUB: DGGES_ called");
    return NULL;
}

void* DGGEV(void)
{
    if (verbose) puts("STUB: DGGEV called");
    return NULL;
}

void* DGGEVX(void)
{
    if (verbose) puts("STUB: DGGEVX called");
    return NULL;
}

void* DGGEVX_(void)
{
    if (verbose) puts("STUB: DGGEVX_ called");
    return NULL;
}

void* DGGEV_(void)
{
    if (verbose) puts("STUB: DGGEV_ called");
    return NULL;
}

void* DGGGLM(void)
{
    if (verbose) puts("STUB: DGGGLM called");
    return NULL;
}

void* DGGGLM_(void)
{
    if (verbose) puts("STUB: DGGGLM_ called");
    return NULL;
}

void* DGGHRD(void)
{
    if (verbose) puts("STUB: DGGHRD called");
    return NULL;
}

void* DGGHRD_(void)
{
    if (verbose) puts("STUB: DGGHRD_ called");
    return NULL;
}

void* DGGLSE(void)
{
    if (verbose) puts("STUB: DGGLSE called");
    return NULL;
}

void* DGGLSE_(void)
{
    if (verbose) puts("STUB: DGGLSE_ called");
    return NULL;
}

void* DGGQRF(void)
{
    if (verbose) puts("STUB: DGGQRF called");
    return NULL;
}

void* DGGQRF_(void)
{
    if (verbose) puts("STUB: DGGQRF_ called");
    return NULL;
}

void* DGGRQF(void)
{
    if (verbose) puts("STUB: DGGRQF called");
    return NULL;
}

void* DGGRQF_(void)
{
    if (verbose) puts("STUB: DGGRQF_ called");
    return NULL;
}

void* DGGSVD(void)
{
    if (verbose) puts("STUB: DGGSVD called");
    return NULL;
}

void* DGGSVD_(void)
{
    if (verbose) puts("STUB: DGGSVD_ called");
    return NULL;
}

void* DGGSVP(void)
{
    if (verbose) puts("STUB: DGGSVP called");
    return NULL;
}

void* DGGSVP_(void)
{
    if (verbose) puts("STUB: DGGSVP_ called");
    return NULL;
}

void* DGSVJ0(void)
{
    if (verbose) puts("STUB: DGSVJ0 called");
    return NULL;
}

void* DGSVJ0_(void)
{
    if (verbose) puts("STUB: DGSVJ0_ called");
    return NULL;
}

void* DGSVJ1(void)
{
    if (verbose) puts("STUB: DGSVJ1 called");
    return NULL;
}

void* DGSVJ1_(void)
{
    if (verbose) puts("STUB: DGSVJ1_ called");
    return NULL;
}

void* DGTCON(void)
{
    if (verbose) puts("STUB: DGTCON called");
    return NULL;
}

void* DGTCON_(void)
{
    if (verbose) puts("STUB: DGTCON_ called");
    return NULL;
}

void* DGTRFS(void)
{
    if (verbose) puts("STUB: DGTRFS called");
    return NULL;
}

void* DGTRFS_(void)
{
    if (verbose) puts("STUB: DGTRFS_ called");
    return NULL;
}

void* DGTSV(void)
{
    if (verbose) puts("STUB: DGTSV called");
    return NULL;
}

void* DGTSVX(void)
{
    if (verbose) puts("STUB: DGTSVX called");
    return NULL;
}

void* DGTSVX_(void)
{
    if (verbose) puts("STUB: DGTSVX_ called");
    return NULL;
}

void* DGTSV_(void)
{
    if (verbose) puts("STUB: DGTSV_ called");
    return NULL;
}

void* DGTTRF(void)
{
    if (verbose) puts("STUB: DGTTRF called");
    return NULL;
}

void* DGTTRF_(void)
{
    if (verbose) puts("STUB: DGTTRF_ called");
    return NULL;
}

void* DGTTRS(void)
{
    if (verbose) puts("STUB: DGTTRS called");
    return NULL;
}

void* DGTTRS_(void)
{
    if (verbose) puts("STUB: DGTTRS_ called");
    return NULL;
}

void* DGTTS2(void)
{
    if (verbose) puts("STUB: DGTTS2 called");
    return NULL;
}

void* DGTTS2_(void)
{
    if (verbose) puts("STUB: DGTTS2_ called");
    return NULL;
}

void* DHGEQZ(void)
{
    if (verbose) puts("STUB: DHGEQZ called");
    return NULL;
}

void* DHGEQZ_(void)
{
    if (verbose) puts("STUB: DHGEQZ_ called");
    return NULL;
}

void* DHSEIN(void)
{
    if (verbose) puts("STUB: DHSEIN called");
    return NULL;
}

void* DHSEIN_(void)
{
    if (verbose) puts("STUB: DHSEIN_ called");
    return NULL;
}

void* DHSEQR(void)
{
    if (verbose) puts("STUB: DHSEQR called");
    return NULL;
}

void* DHSEQR_(void)
{
    if (verbose) puts("STUB: DHSEQR_ called");
    return NULL;
}

void* DISNAN(void)
{
    if (verbose) puts("STUB: DISNAN called");
    return NULL;
}

void* DISNAN_(void)
{
    if (verbose) puts("STUB: DISNAN_ called");
    return NULL;
}

void* DLABAD(void)
{
    if (verbose) puts("STUB: DLABAD called");
    return NULL;
}

void* DLABAD_(void)
{
    if (verbose) puts("STUB: DLABAD_ called");
    return NULL;
}

void* DLABRD(void)
{
    if (verbose) puts("STUB: DLABRD called");
    return NULL;
}

void* DLABRD_(void)
{
    if (verbose) puts("STUB: DLABRD_ called");
    return NULL;
}

void* DLACN2(void)
{
    if (verbose) puts("STUB: DLACN2 called");
    return NULL;
}

void* DLACN2_(void)
{
    if (verbose) puts("STUB: DLACN2_ called");
    return NULL;
}

void* DLACON(void)
{
    if (verbose) puts("STUB: DLACON called");
    return NULL;
}

void* DLACON_(void)
{
    if (verbose) puts("STUB: DLACON_ called");
    return NULL;
}

void* DLACPY(void)
{
    if (verbose) puts("STUB: DLACPY called");
    return NULL;
}

void* DLACPY_(void)
{
    if (verbose) puts("STUB: DLACPY_ called");
    return NULL;
}

void* DLADIV(void)
{
    if (verbose) puts("STUB: DLADIV called");
    return NULL;
}

void* DLADIV_(void)
{
    if (verbose) puts("STUB: DLADIV_ called");
    return NULL;
}

void* DLAE2(void)
{
    if (verbose) puts("STUB: DLAE2 called");
    return NULL;
}

void* DLAE2_(void)
{
    if (verbose) puts("STUB: DLAE2_ called");
    return NULL;
}

void* DLAEBZ(void)
{
    if (verbose) puts("STUB: DLAEBZ called");
    return NULL;
}

void* DLAEBZ_(void)
{
    if (verbose) puts("STUB: DLAEBZ_ called");
    return NULL;
}

void* DLAED0(void)
{
    if (verbose) puts("STUB: DLAED0 called");
    return NULL;
}

void* DLAED0_(void)
{
    if (verbose) puts("STUB: DLAED0_ called");
    return NULL;
}

void* DLAED1(void)
{
    if (verbose) puts("STUB: DLAED1 called");
    return NULL;
}

void* DLAED1_(void)
{
    if (verbose) puts("STUB: DLAED1_ called");
    return NULL;
}

void* DLAED2(void)
{
    if (verbose) puts("STUB: DLAED2 called");
    return NULL;
}

void* DLAED2_(void)
{
    if (verbose) puts("STUB: DLAED2_ called");
    return NULL;
}

void* DLAED3(void)
{
    if (verbose) puts("STUB: DLAED3 called");
    return NULL;
}

void* DLAED3_(void)
{
    if (verbose) puts("STUB: DLAED3_ called");
    return NULL;
}

void* DLAED4(void)
{
    if (verbose) puts("STUB: DLAED4 called");
    return NULL;
}

void* DLAED4_(void)
{
    if (verbose) puts("STUB: DLAED4_ called");
    return NULL;
}

void* DLAED5(void)
{
    if (verbose) puts("STUB: DLAED5 called");
    return NULL;
}

void* DLAED5_(void)
{
    if (verbose) puts("STUB: DLAED5_ called");
    return NULL;
}

void* DLAED6(void)
{
    if (verbose) puts("STUB: DLAED6 called");
    return NULL;
}

void* DLAED6_(void)
{
    if (verbose) puts("STUB: DLAED6_ called");
    return NULL;
}

void* DLAED7(void)
{
    if (verbose) puts("STUB: DLAED7 called");
    return NULL;
}

void* DLAED7_(void)
{
    if (verbose) puts("STUB: DLAED7_ called");
    return NULL;
}

void* DLAED8(void)
{
    if (verbose) puts("STUB: DLAED8 called");
    return NULL;
}

void* DLAED8_(void)
{
    if (verbose) puts("STUB: DLAED8_ called");
    return NULL;
}

void* DLAED9(void)
{
    if (verbose) puts("STUB: DLAED9 called");
    return NULL;
}

void* DLAED9_(void)
{
    if (verbose) puts("STUB: DLAED9_ called");
    return NULL;
}

void* DLAEDA(void)
{
    if (verbose) puts("STUB: DLAEDA called");
    return NULL;
}

void* DLAEDA_(void)
{
    if (verbose) puts("STUB: DLAEDA_ called");
    return NULL;
}

void* DLAEIN(void)
{
    if (verbose) puts("STUB: DLAEIN called");
    return NULL;
}

void* DLAEIN_(void)
{
    if (verbose) puts("STUB: DLAEIN_ called");
    return NULL;
}

void* DLAEV2(void)
{
    if (verbose) puts("STUB: DLAEV2 called");
    return NULL;
}

void* DLAEV2_(void)
{
    if (verbose) puts("STUB: DLAEV2_ called");
    return NULL;
}

void* DLAEXC(void)
{
    if (verbose) puts("STUB: DLAEXC called");
    return NULL;
}

void* DLAEXC_(void)
{
    if (verbose) puts("STUB: DLAEXC_ called");
    return NULL;
}

void* DLAG2(void)
{
    if (verbose) puts("STUB: DLAG2 called");
    return NULL;
}

void* DLAG2S(void)
{
    if (verbose) puts("STUB: DLAG2S called");
    return NULL;
}

void* DLAG2S_(void)
{
    if (verbose) puts("STUB: DLAG2S_ called");
    return NULL;
}

void* DLAG2_(void)
{
    if (verbose) puts("STUB: DLAG2_ called");
    return NULL;
}

void* DLAGS2(void)
{
    if (verbose) puts("STUB: DLAGS2 called");
    return NULL;
}

void* DLAGS2_(void)
{
    if (verbose) puts("STUB: DLAGS2_ called");
    return NULL;
}

void* DLAGTF(void)
{
    if (verbose) puts("STUB: DLAGTF called");
    return NULL;
}

void* DLAGTF_(void)
{
    if (verbose) puts("STUB: DLAGTF_ called");
    return NULL;
}

void* DLAGTM(void)
{
    if (verbose) puts("STUB: DLAGTM called");
    return NULL;
}

void* DLAGTM_(void)
{
    if (verbose) puts("STUB: DLAGTM_ called");
    return NULL;
}

void* DLAGTS(void)
{
    if (verbose) puts("STUB: DLAGTS called");
    return NULL;
}

void* DLAGTS_(void)
{
    if (verbose) puts("STUB: DLAGTS_ called");
    return NULL;
}

void* DLAGV2(void)
{
    if (verbose) puts("STUB: DLAGV2 called");
    return NULL;
}

void* DLAGV2_(void)
{
    if (verbose) puts("STUB: DLAGV2_ called");
    return NULL;
}

void* DLAHQR(void)
{
    if (verbose) puts("STUB: DLAHQR called");
    return NULL;
}

void* DLAHQR_(void)
{
    if (verbose) puts("STUB: DLAHQR_ called");
    return NULL;
}

void* DLAHR2(void)
{
    if (verbose) puts("STUB: DLAHR2 called");
    return NULL;
}

void* DLAHR2_(void)
{
    if (verbose) puts("STUB: DLAHR2_ called");
    return NULL;
}

void* DLAHRD(void)
{
    if (verbose) puts("STUB: DLAHRD called");
    return NULL;
}

void* DLAHRD_(void)
{
    if (verbose) puts("STUB: DLAHRD_ called");
    return NULL;
}

void* DLAIC1(void)
{
    if (verbose) puts("STUB: DLAIC1 called");
    return NULL;
}

void* DLAIC1_(void)
{
    if (verbose) puts("STUB: DLAIC1_ called");
    return NULL;
}

void* DLAISNAN(void)
{
    if (verbose) puts("STUB: DLAISNAN called");
    return NULL;
}

void* DLAISNAN_(void)
{
    if (verbose) puts("STUB: DLAISNAN_ called");
    return NULL;
}

void* DLALN2(void)
{
    if (verbose) puts("STUB: DLALN2 called");
    return NULL;
}

void* DLALN2_(void)
{
    if (verbose) puts("STUB: DLALN2_ called");
    return NULL;
}

void* DLALS0(void)
{
    if (verbose) puts("STUB: DLALS0 called");
    return NULL;
}

void* DLALS0_(void)
{
    if (verbose) puts("STUB: DLALS0_ called");
    return NULL;
}

void* DLALSA(void)
{
    if (verbose) puts("STUB: DLALSA called");
    return NULL;
}

void* DLALSA_(void)
{
    if (verbose) puts("STUB: DLALSA_ called");
    return NULL;
}

void* DLALSD(void)
{
    if (verbose) puts("STUB: DLALSD called");
    return NULL;
}

void* DLALSD_(void)
{
    if (verbose) puts("STUB: DLALSD_ called");
    return NULL;
}

void* DLAMC1(void)
{
    if (verbose) puts("STUB: DLAMC1 called");
    return NULL;
}

void* DLAMC1_(void)
{
    if (verbose) puts("STUB: DLAMC1_ called");
    return NULL;
}

void* DLAMC2(void)
{
    if (verbose) puts("STUB: DLAMC2 called");
    return NULL;
}

void* DLAMC2_(void)
{
    if (verbose) puts("STUB: DLAMC2_ called");
    return NULL;
}

void* DLAMC3(void)
{
    if (verbose) puts("STUB: DLAMC3 called");
    return NULL;
}

void* DLAMC3_(void)
{
    if (verbose) puts("STUB: DLAMC3_ called");
    return NULL;
}

void* DLAMC4(void)
{
    if (verbose) puts("STUB: DLAMC4 called");
    return NULL;
}

void* DLAMC4_(void)
{
    if (verbose) puts("STUB: DLAMC4_ called");
    return NULL;
}

void* DLAMC5(void)
{
    if (verbose) puts("STUB: DLAMC5 called");
    return NULL;
}

void* DLAMC5_(void)
{
    if (verbose) puts("STUB: DLAMC5_ called");
    return NULL;
}

void* DLAMCH(void)
{
    if (verbose) puts("STUB: DLAMCH called");
    return NULL;
}

void* DLAMCH_(void)
{
    if (verbose) puts("STUB: DLAMCH_ called");
    return NULL;
}

void* DLAMRG(void)
{
    if (verbose) puts("STUB: DLAMRG called");
    return NULL;
}

void* DLAMRG_(void)
{
    if (verbose) puts("STUB: DLAMRG_ called");
    return NULL;
}

void* DLANEG(void)
{
    if (verbose) puts("STUB: DLANEG called");
    return NULL;
}

void* DLANEG_(void)
{
    if (verbose) puts("STUB: DLANEG_ called");
    return NULL;
}

void* DLANGB(void)
{
    if (verbose) puts("STUB: DLANGB called");
    return NULL;
}

void* DLANGB_(void)
{
    if (verbose) puts("STUB: DLANGB_ called");
    return NULL;
}

void* DLANGE(void)
{
    if (verbose) puts("STUB: DLANGE called");
    return NULL;
}

void* DLANGE_(void)
{
    if (verbose) puts("STUB: DLANGE_ called");
    return NULL;
}

void* DLANGT(void)
{
    if (verbose) puts("STUB: DLANGT called");
    return NULL;
}

void* DLANGT_(void)
{
    if (verbose) puts("STUB: DLANGT_ called");
    return NULL;
}

void* DLANHS(void)
{
    if (verbose) puts("STUB: DLANHS called");
    return NULL;
}

void* DLANHS_(void)
{
    if (verbose) puts("STUB: DLANHS_ called");
    return NULL;
}

void* DLANSB(void)
{
    if (verbose) puts("STUB: DLANSB called");
    return NULL;
}

void* DLANSB_(void)
{
    if (verbose) puts("STUB: DLANSB_ called");
    return NULL;
}

void* DLANSF(void)
{
    if (verbose) puts("STUB: DLANSF called");
    return NULL;
}

void* DLANSF_(void)
{
    if (verbose) puts("STUB: DLANSF_ called");
    return NULL;
}

void* DLANSP(void)
{
    if (verbose) puts("STUB: DLANSP called");
    return NULL;
}

void* DLANSP_(void)
{
    if (verbose) puts("STUB: DLANSP_ called");
    return NULL;
}

void* DLANST(void)
{
    if (verbose) puts("STUB: DLANST called");
    return NULL;
}

void* DLANST_(void)
{
    if (verbose) puts("STUB: DLANST_ called");
    return NULL;
}

void* DLANSY(void)
{
    if (verbose) puts("STUB: DLANSY called");
    return NULL;
}

void* DLANSY_(void)
{
    if (verbose) puts("STUB: DLANSY_ called");
    return NULL;
}

void* DLANTB(void)
{
    if (verbose) puts("STUB: DLANTB called");
    return NULL;
}

void* DLANTB_(void)
{
    if (verbose) puts("STUB: DLANTB_ called");
    return NULL;
}

void* DLANTP(void)
{
    if (verbose) puts("STUB: DLANTP called");
    return NULL;
}

void* DLANTP_(void)
{
    if (verbose) puts("STUB: DLANTP_ called");
    return NULL;
}

void* DLANTR(void)
{
    if (verbose) puts("STUB: DLANTR called");
    return NULL;
}

void* DLANTR_(void)
{
    if (verbose) puts("STUB: DLANTR_ called");
    return NULL;
}

void* DLANV2(void)
{
    if (verbose) puts("STUB: DLANV2 called");
    return NULL;
}

void* DLANV2_(void)
{
    if (verbose) puts("STUB: DLANV2_ called");
    return NULL;
}

void* DLAPLL(void)
{
    if (verbose) puts("STUB: DLAPLL called");
    return NULL;
}

void* DLAPLL_(void)
{
    if (verbose) puts("STUB: DLAPLL_ called");
    return NULL;
}

void* DLAPMT(void)
{
    if (verbose) puts("STUB: DLAPMT called");
    return NULL;
}

void* DLAPMT_(void)
{
    if (verbose) puts("STUB: DLAPMT_ called");
    return NULL;
}

void* DLAPY2(void)
{
    if (verbose) puts("STUB: DLAPY2 called");
    return NULL;
}

void* DLAPY2_(void)
{
    if (verbose) puts("STUB: DLAPY2_ called");
    return NULL;
}

void* DLAPY3(void)
{
    if (verbose) puts("STUB: DLAPY3 called");
    return NULL;
}

void* DLAPY3_(void)
{
    if (verbose) puts("STUB: DLAPY3_ called");
    return NULL;
}

void* DLAQGB(void)
{
    if (verbose) puts("STUB: DLAQGB called");
    return NULL;
}

void* DLAQGB_(void)
{
    if (verbose) puts("STUB: DLAQGB_ called");
    return NULL;
}

void* DLAQGE(void)
{
    if (verbose) puts("STUB: DLAQGE called");
    return NULL;
}

void* DLAQGE_(void)
{
    if (verbose) puts("STUB: DLAQGE_ called");
    return NULL;
}

void* DLAQP2(void)
{
    if (verbose) puts("STUB: DLAQP2 called");
    return NULL;
}

void* DLAQP2_(void)
{
    if (verbose) puts("STUB: DLAQP2_ called");
    return NULL;
}

void* DLAQPS(void)
{
    if (verbose) puts("STUB: DLAQPS called");
    return NULL;
}

void* DLAQPS_(void)
{
    if (verbose) puts("STUB: DLAQPS_ called");
    return NULL;
}

void* DLAQR0(void)
{
    if (verbose) puts("STUB: DLAQR0 called");
    return NULL;
}

void* DLAQR0_(void)
{
    if (verbose) puts("STUB: DLAQR0_ called");
    return NULL;
}

void* DLAQR1(void)
{
    if (verbose) puts("STUB: DLAQR1 called");
    return NULL;
}

void* DLAQR1_(void)
{
    if (verbose) puts("STUB: DLAQR1_ called");
    return NULL;
}

void* DLAQR2(void)
{
    if (verbose) puts("STUB: DLAQR2 called");
    return NULL;
}

void* DLAQR2_(void)
{
    if (verbose) puts("STUB: DLAQR2_ called");
    return NULL;
}

void* DLAQR3(void)
{
    if (verbose) puts("STUB: DLAQR3 called");
    return NULL;
}

void* DLAQR3_(void)
{
    if (verbose) puts("STUB: DLAQR3_ called");
    return NULL;
}

void* DLAQR4(void)
{
    if (verbose) puts("STUB: DLAQR4 called");
    return NULL;
}

void* DLAQR4_(void)
{
    if (verbose) puts("STUB: DLAQR4_ called");
    return NULL;
}

void* DLAQR5(void)
{
    if (verbose) puts("STUB: DLAQR5 called");
    return NULL;
}

void* DLAQR5_(void)
{
    if (verbose) puts("STUB: DLAQR5_ called");
    return NULL;
}

void* DLAQSB(void)
{
    if (verbose) puts("STUB: DLAQSB called");
    return NULL;
}

void* DLAQSB_(void)
{
    if (verbose) puts("STUB: DLAQSB_ called");
    return NULL;
}

void* DLAQSP(void)
{
    if (verbose) puts("STUB: DLAQSP called");
    return NULL;
}

void* DLAQSP_(void)
{
    if (verbose) puts("STUB: DLAQSP_ called");
    return NULL;
}

void* DLAQSY(void)
{
    if (verbose) puts("STUB: DLAQSY called");
    return NULL;
}

void* DLAQSY_(void)
{
    if (verbose) puts("STUB: DLAQSY_ called");
    return NULL;
}

void* DLAQTR(void)
{
    if (verbose) puts("STUB: DLAQTR called");
    return NULL;
}

void* DLAQTR_(void)
{
    if (verbose) puts("STUB: DLAQTR_ called");
    return NULL;
}

void* DLAR1V(void)
{
    if (verbose) puts("STUB: DLAR1V called");
    return NULL;
}

void* DLAR1V_(void)
{
    if (verbose) puts("STUB: DLAR1V_ called");
    return NULL;
}

void* DLAR2V(void)
{
    if (verbose) puts("STUB: DLAR2V called");
    return NULL;
}

void* DLAR2V_(void)
{
    if (verbose) puts("STUB: DLAR2V_ called");
    return NULL;
}

void* DLARF(void)
{
    if (verbose) puts("STUB: DLARF called");
    return NULL;
}

void* DLARFB(void)
{
    if (verbose) puts("STUB: DLARFB called");
    return NULL;
}

void* DLARFB_(void)
{
    if (verbose) puts("STUB: DLARFB_ called");
    return NULL;
}

void* DLARFG(void)
{
    if (verbose) puts("STUB: DLARFG called");
    return NULL;
}

void* DLARFG_(void)
{
    if (verbose) puts("STUB: DLARFG_ called");
    return NULL;
}

void* DLARFP(void)
{
    if (verbose) puts("STUB: DLARFP called");
    return NULL;
}

void* DLARFP_(void)
{
    if (verbose) puts("STUB: DLARFP_ called");
    return NULL;
}

void* DLARFT(void)
{
    if (verbose) puts("STUB: DLARFT called");
    return NULL;
}

void* DLARFT_(void)
{
    if (verbose) puts("STUB: DLARFT_ called");
    return NULL;
}

void* DLARFX(void)
{
    if (verbose) puts("STUB: DLARFX called");
    return NULL;
}

void* DLARFX_(void)
{
    if (verbose) puts("STUB: DLARFX_ called");
    return NULL;
}

void* DLARF_(void)
{
    if (verbose) puts("STUB: DLARF_ called");
    return NULL;
}

void* DLARGV(void)
{
    if (verbose) puts("STUB: DLARGV called");
    return NULL;
}

void* DLARGV_(void)
{
    if (verbose) puts("STUB: DLARGV_ called");
    return NULL;
}

void* DLARNV(void)
{
    if (verbose) puts("STUB: DLARNV called");
    return NULL;
}

void* DLARNV_(void)
{
    if (verbose) puts("STUB: DLARNV_ called");
    return NULL;
}

void* DLARRA(void)
{
    if (verbose) puts("STUB: DLARRA called");
    return NULL;
}

void* DLARRA_(void)
{
    if (verbose) puts("STUB: DLARRA_ called");
    return NULL;
}

void* DLARRB(void)
{
    if (verbose) puts("STUB: DLARRB called");
    return NULL;
}

void* DLARRB_(void)
{
    if (verbose) puts("STUB: DLARRB_ called");
    return NULL;
}

void* DLARRC(void)
{
    if (verbose) puts("STUB: DLARRC called");
    return NULL;
}

void* DLARRC_(void)
{
    if (verbose) puts("STUB: DLARRC_ called");
    return NULL;
}

void* DLARRD(void)
{
    if (verbose) puts("STUB: DLARRD called");
    return NULL;
}

void* DLARRD_(void)
{
    if (verbose) puts("STUB: DLARRD_ called");
    return NULL;
}

void* DLARRE(void)
{
    if (verbose) puts("STUB: DLARRE called");
    return NULL;
}

void* DLARRE_(void)
{
    if (verbose) puts("STUB: DLARRE_ called");
    return NULL;
}

void* DLARRF(void)
{
    if (verbose) puts("STUB: DLARRF called");
    return NULL;
}

void* DLARRF_(void)
{
    if (verbose) puts("STUB: DLARRF_ called");
    return NULL;
}

void* DLARRJ(void)
{
    if (verbose) puts("STUB: DLARRJ called");
    return NULL;
}

void* DLARRJ_(void)
{
    if (verbose) puts("STUB: DLARRJ_ called");
    return NULL;
}

void* DLARRK(void)
{
    if (verbose) puts("STUB: DLARRK called");
    return NULL;
}

void* DLARRK_(void)
{
    if (verbose) puts("STUB: DLARRK_ called");
    return NULL;
}

void* DLARRR(void)
{
    if (verbose) puts("STUB: DLARRR called");
    return NULL;
}

void* DLARRR_(void)
{
    if (verbose) puts("STUB: DLARRR_ called");
    return NULL;
}

void* DLARRV(void)
{
    if (verbose) puts("STUB: DLARRV called");
    return NULL;
}

void* DLARRV_(void)
{
    if (verbose) puts("STUB: DLARRV_ called");
    return NULL;
}

void* DLARSCL2(void)
{
    if (verbose) puts("STUB: DLARSCL2 called");
    return NULL;
}

void* DLARSCL2_(void)
{
    if (verbose) puts("STUB: DLARSCL2_ called");
    return NULL;
}

void* DLARTG(void)
{
    if (verbose) puts("STUB: DLARTG called");
    return NULL;
}

void* DLARTG_(void)
{
    if (verbose) puts("STUB: DLARTG_ called");
    return NULL;
}

void* DLARTV(void)
{
    if (verbose) puts("STUB: DLARTV called");
    return NULL;
}

void* DLARTV_(void)
{
    if (verbose) puts("STUB: DLARTV_ called");
    return NULL;
}

void* DLARUV(void)
{
    if (verbose) puts("STUB: DLARUV called");
    return NULL;
}

void* DLARUV_(void)
{
    if (verbose) puts("STUB: DLARUV_ called");
    return NULL;
}

void* DLARZ(void)
{
    if (verbose) puts("STUB: DLARZ called");
    return NULL;
}

void* DLARZB(void)
{
    if (verbose) puts("STUB: DLARZB called");
    return NULL;
}

void* DLARZB_(void)
{
    if (verbose) puts("STUB: DLARZB_ called");
    return NULL;
}

void* DLARZT(void)
{
    if (verbose) puts("STUB: DLARZT called");
    return NULL;
}

void* DLARZT_(void)
{
    if (verbose) puts("STUB: DLARZT_ called");
    return NULL;
}

void* DLARZ_(void)
{
    if (verbose) puts("STUB: DLARZ_ called");
    return NULL;
}

void* DLAS2(void)
{
    if (verbose) puts("STUB: DLAS2 called");
    return NULL;
}

void* DLAS2_(void)
{
    if (verbose) puts("STUB: DLAS2_ called");
    return NULL;
}

void* DLASCL(void)
{
    if (verbose) puts("STUB: DLASCL called");
    return NULL;
}

void* DLASCL2(void)
{
    if (verbose) puts("STUB: DLASCL2 called");
    return NULL;
}

void* DLASCL2_(void)
{
    if (verbose) puts("STUB: DLASCL2_ called");
    return NULL;
}

void* DLASCL_(void)
{
    if (verbose) puts("STUB: DLASCL_ called");
    return NULL;
}

void* DLASD0(void)
{
    if (verbose) puts("STUB: DLASD0 called");
    return NULL;
}

void* DLASD0_(void)
{
    if (verbose) puts("STUB: DLASD0_ called");
    return NULL;
}

void* DLASD1(void)
{
    if (verbose) puts("STUB: DLASD1 called");
    return NULL;
}

void* DLASD1_(void)
{
    if (verbose) puts("STUB: DLASD1_ called");
    return NULL;
}

void* DLASD2(void)
{
    if (verbose) puts("STUB: DLASD2 called");
    return NULL;
}

void* DLASD2_(void)
{
    if (verbose) puts("STUB: DLASD2_ called");
    return NULL;
}

void* DLASD3(void)
{
    if (verbose) puts("STUB: DLASD3 called");
    return NULL;
}

void* DLASD3_(void)
{
    if (verbose) puts("STUB: DLASD3_ called");
    return NULL;
}

void* DLASD4(void)
{
    if (verbose) puts("STUB: DLASD4 called");
    return NULL;
}

void* DLASD4_(void)
{
    if (verbose) puts("STUB: DLASD4_ called");
    return NULL;
}

void* DLASD5(void)
{
    if (verbose) puts("STUB: DLASD5 called");
    return NULL;
}

void* DLASD5_(void)
{
    if (verbose) puts("STUB: DLASD5_ called");
    return NULL;
}

void* DLASD6(void)
{
    if (verbose) puts("STUB: DLASD6 called");
    return NULL;
}

void* DLASD6_(void)
{
    if (verbose) puts("STUB: DLASD6_ called");
    return NULL;
}

void* DLASD7(void)
{
    if (verbose) puts("STUB: DLASD7 called");
    return NULL;
}

void* DLASD7_(void)
{
    if (verbose) puts("STUB: DLASD7_ called");
    return NULL;
}

void* DLASD8(void)
{
    if (verbose) puts("STUB: DLASD8 called");
    return NULL;
}

void* DLASD8_(void)
{
    if (verbose) puts("STUB: DLASD8_ called");
    return NULL;
}

void* DLASD9(void)
{
    if (verbose) puts("STUB: DLASD9 called");
    return NULL;
}

void* DLASD9_(void)
{
    if (verbose) puts("STUB: DLASD9_ called");
    return NULL;
}

void* DLASDA(void)
{
    if (verbose) puts("STUB: DLASDA called");
    return NULL;
}

void* DLASDA_(void)
{
    if (verbose) puts("STUB: DLASDA_ called");
    return NULL;
}

void* DLASDQ(void)
{
    if (verbose) puts("STUB: DLASDQ called");
    return NULL;
}

void* DLASDQ_(void)
{
    if (verbose) puts("STUB: DLASDQ_ called");
    return NULL;
}

void* DLASDT(void)
{
    if (verbose) puts("STUB: DLASDT called");
    return NULL;
}

void* DLASDT_(void)
{
    if (verbose) puts("STUB: DLASDT_ called");
    return NULL;
}

void* DLASET(void)
{
    if (verbose) puts("STUB: DLASET called");
    return NULL;
}

void* DLASET_(void)
{
    if (verbose) puts("STUB: DLASET_ called");
    return NULL;
}

void* DLASQ1(void)
{
    if (verbose) puts("STUB: DLASQ1 called");
    return NULL;
}

void* DLASQ1_(void)
{
    if (verbose) puts("STUB: DLASQ1_ called");
    return NULL;
}

void* DLASQ2(void)
{
    if (verbose) puts("STUB: DLASQ2 called");
    return NULL;
}

void* DLASQ2_(void)
{
    if (verbose) puts("STUB: DLASQ2_ called");
    return NULL;
}

void* DLASQ3(void)
{
    if (verbose) puts("STUB: DLASQ3 called");
    return NULL;
}

void* DLASQ3_(void)
{
    if (verbose) puts("STUB: DLASQ3_ called");
    return NULL;
}

void* DLASQ4(void)
{
    if (verbose) puts("STUB: DLASQ4 called");
    return NULL;
}

void* DLASQ4_(void)
{
    if (verbose) puts("STUB: DLASQ4_ called");
    return NULL;
}

void* DLASQ5(void)
{
    if (verbose) puts("STUB: DLASQ5 called");
    return NULL;
}

void* DLASQ5_(void)
{
    if (verbose) puts("STUB: DLASQ5_ called");
    return NULL;
}

void* DLASQ6(void)
{
    if (verbose) puts("STUB: DLASQ6 called");
    return NULL;
}

void* DLASQ6_(void)
{
    if (verbose) puts("STUB: DLASQ6_ called");
    return NULL;
}

void* DLASR(void)
{
    if (verbose) puts("STUB: DLASR called");
    return NULL;
}

void* DLASRT(void)
{
    if (verbose) puts("STUB: DLASRT called");
    return NULL;
}

void* DLASRT_(void)
{
    if (verbose) puts("STUB: DLASRT_ called");
    return NULL;
}

void* DLASR_(void)
{
    if (verbose) puts("STUB: DLASR_ called");
    return NULL;
}

void* DLASSQ(void)
{
    if (verbose) puts("STUB: DLASSQ called");
    return NULL;
}

void* DLASSQ_(void)
{
    if (verbose) puts("STUB: DLASSQ_ called");
    return NULL;
}

void* DLASV2(void)
{
    if (verbose) puts("STUB: DLASV2 called");
    return NULL;
}

void* DLASV2_(void)
{
    if (verbose) puts("STUB: DLASV2_ called");
    return NULL;
}

void* DLASWP(void)
{
    if (verbose) puts("STUB: DLASWP called");
    return NULL;
}

void* DLASWP_(void)
{
    if (verbose) puts("STUB: DLASWP_ called");
    return NULL;
}

void* DLASY2(void)
{
    if (verbose) puts("STUB: DLASY2 called");
    return NULL;
}

void* DLASY2_(void)
{
    if (verbose) puts("STUB: DLASY2_ called");
    return NULL;
}

void* DLASYF(void)
{
    if (verbose) puts("STUB: DLASYF called");
    return NULL;
}

void* DLASYF_(void)
{
    if (verbose) puts("STUB: DLASYF_ called");
    return NULL;
}

void* DLAT2S(void)
{
    if (verbose) puts("STUB: DLAT2S called");
    return NULL;
}

void* DLAT2S_(void)
{
    if (verbose) puts("STUB: DLAT2S_ called");
    return NULL;
}

void* DLATBS(void)
{
    if (verbose) puts("STUB: DLATBS called");
    return NULL;
}

void* DLATBS_(void)
{
    if (verbose) puts("STUB: DLATBS_ called");
    return NULL;
}

void* DLATDF(void)
{
    if (verbose) puts("STUB: DLATDF called");
    return NULL;
}

void* DLATDF_(void)
{
    if (verbose) puts("STUB: DLATDF_ called");
    return NULL;
}

void* DLATPS(void)
{
    if (verbose) puts("STUB: DLATPS called");
    return NULL;
}

void* DLATPS_(void)
{
    if (verbose) puts("STUB: DLATPS_ called");
    return NULL;
}

void* DLATRD(void)
{
    if (verbose) puts("STUB: DLATRD called");
    return NULL;
}

void* DLATRD_(void)
{
    if (verbose) puts("STUB: DLATRD_ called");
    return NULL;
}

void* DLATRS(void)
{
    if (verbose) puts("STUB: DLATRS called");
    return NULL;
}

void* DLATRS_(void)
{
    if (verbose) puts("STUB: DLATRS_ called");
    return NULL;
}

void* DLATRZ(void)
{
    if (verbose) puts("STUB: DLATRZ called");
    return NULL;
}

void* DLATRZ_(void)
{
    if (verbose) puts("STUB: DLATRZ_ called");
    return NULL;
}

void* DLATZM(void)
{
    if (verbose) puts("STUB: DLATZM called");
    return NULL;
}

void* DLATZM_(void)
{
    if (verbose) puts("STUB: DLATZM_ called");
    return NULL;
}

void* DLAUU2(void)
{
    if (verbose) puts("STUB: DLAUU2 called");
    return NULL;
}

void* DLAUU2_(void)
{
    if (verbose) puts("STUB: DLAUU2_ called");
    return NULL;
}

void* DLAUUM(void)
{
    if (verbose) puts("STUB: DLAUUM called");
    return NULL;
}

void* DLAUUM_(void)
{
    if (verbose) puts("STUB: DLAUUM_ called");
    return NULL;
}

void* DOPGTR(void)
{
    if (verbose) puts("STUB: DOPGTR called");
    return NULL;
}

void* DOPGTR_(void)
{
    if (verbose) puts("STUB: DOPGTR_ called");
    return NULL;
}

void* DOPMTR(void)
{
    if (verbose) puts("STUB: DOPMTR called");
    return NULL;
}

void* DOPMTR_(void)
{
    if (verbose) puts("STUB: DOPMTR_ called");
    return NULL;
}

void* DORG2L(void)
{
    if (verbose) puts("STUB: DORG2L called");
    return NULL;
}

void* DORG2L_(void)
{
    if (verbose) puts("STUB: DORG2L_ called");
    return NULL;
}

void* DORG2R(void)
{
    if (verbose) puts("STUB: DORG2R called");
    return NULL;
}

void* DORG2R_(void)
{
    if (verbose) puts("STUB: DORG2R_ called");
    return NULL;
}

void* DORGBR(void)
{
    if (verbose) puts("STUB: DORGBR called");
    return NULL;
}

void* DORGBR_(void)
{
    if (verbose) puts("STUB: DORGBR_ called");
    return NULL;
}

void* DORGHR(void)
{
    if (verbose) puts("STUB: DORGHR called");
    return NULL;
}

void* DORGHR_(void)
{
    if (verbose) puts("STUB: DORGHR_ called");
    return NULL;
}

void* DORGL2(void)
{
    if (verbose) puts("STUB: DORGL2 called");
    return NULL;
}

void* DORGL2_(void)
{
    if (verbose) puts("STUB: DORGL2_ called");
    return NULL;
}

void* DORGLQ(void)
{
    if (verbose) puts("STUB: DORGLQ called");
    return NULL;
}

void* DORGLQ_(void)
{
    if (verbose) puts("STUB: DORGLQ_ called");
    return NULL;
}

void* DORGQL(void)
{
    if (verbose) puts("STUB: DORGQL called");
    return NULL;
}

void* DORGQL_(void)
{
    if (verbose) puts("STUB: DORGQL_ called");
    return NULL;
}

void* DORGQR(void)
{
    if (verbose) puts("STUB: DORGQR called");
    return NULL;
}

void* DORGQR_(void)
{
    if (verbose) puts("STUB: DORGQR_ called");
    return NULL;
}

void* DORGR2(void)
{
    if (verbose) puts("STUB: DORGR2 called");
    return NULL;
}

void* DORGR2_(void)
{
    if (verbose) puts("STUB: DORGR2_ called");
    return NULL;
}

void* DORGRQ(void)
{
    if (verbose) puts("STUB: DORGRQ called");
    return NULL;
}

void* DORGRQ_(void)
{
    if (verbose) puts("STUB: DORGRQ_ called");
    return NULL;
}

void* DORGTR(void)
{
    if (verbose) puts("STUB: DORGTR called");
    return NULL;
}

void* DORGTR_(void)
{
    if (verbose) puts("STUB: DORGTR_ called");
    return NULL;
}

void* DORM2L(void)
{
    if (verbose) puts("STUB: DORM2L called");
    return NULL;
}

void* DORM2L_(void)
{
    if (verbose) puts("STUB: DORM2L_ called");
    return NULL;
}

void* DORM2R(void)
{
    if (verbose) puts("STUB: DORM2R called");
    return NULL;
}

void* DORM2R_(void)
{
    if (verbose) puts("STUB: DORM2R_ called");
    return NULL;
}

void* DORMBR(void)
{
    if (verbose) puts("STUB: DORMBR called");
    return NULL;
}

void* DORMBR_(void)
{
    if (verbose) puts("STUB: DORMBR_ called");
    return NULL;
}

void* DORMHR(void)
{
    if (verbose) puts("STUB: DORMHR called");
    return NULL;
}

void* DORMHR_(void)
{
    if (verbose) puts("STUB: DORMHR_ called");
    return NULL;
}

void* DORML2(void)
{
    if (verbose) puts("STUB: DORML2 called");
    return NULL;
}

void* DORML2_(void)
{
    if (verbose) puts("STUB: DORML2_ called");
    return NULL;
}

void* DORMLQ(void)
{
    if (verbose) puts("STUB: DORMLQ called");
    return NULL;
}

void* DORMLQ_(void)
{
    if (verbose) puts("STUB: DORMLQ_ called");
    return NULL;
}

void* DORMQL(void)
{
    if (verbose) puts("STUB: DORMQL called");
    return NULL;
}

void* DORMQL_(void)
{
    if (verbose) puts("STUB: DORMQL_ called");
    return NULL;
}

void* DORMQR(void)
{
    if (verbose) puts("STUB: DORMQR called");
    return NULL;
}

void* DORMQR_(void)
{
    if (verbose) puts("STUB: DORMQR_ called");
    return NULL;
}

void* DORMR2(void)
{
    if (verbose) puts("STUB: DORMR2 called");
    return NULL;
}

void* DORMR2_(void)
{
    if (verbose) puts("STUB: DORMR2_ called");
    return NULL;
}

void* DORMR3(void)
{
    if (verbose) puts("STUB: DORMR3 called");
    return NULL;
}

void* DORMR3_(void)
{
    if (verbose) puts("STUB: DORMR3_ called");
    return NULL;
}

void* DORMRQ(void)
{
    if (verbose) puts("STUB: DORMRQ called");
    return NULL;
}

void* DORMRQ_(void)
{
    if (verbose) puts("STUB: DORMRQ_ called");
    return NULL;
}

void* DORMRZ(void)
{
    if (verbose) puts("STUB: DORMRZ called");
    return NULL;
}

void* DORMRZ_(void)
{
    if (verbose) puts("STUB: DORMRZ_ called");
    return NULL;
}

void* DORMTR(void)
{
    if (verbose) puts("STUB: DORMTR called");
    return NULL;
}

void* DORMTR_(void)
{
    if (verbose) puts("STUB: DORMTR_ called");
    return NULL;
}

void* DPBCON(void)
{
    if (verbose) puts("STUB: DPBCON called");
    return NULL;
}

void* DPBCON_(void)
{
    if (verbose) puts("STUB: DPBCON_ called");
    return NULL;
}

void* DPBEQU(void)
{
    if (verbose) puts("STUB: DPBEQU called");
    return NULL;
}

void* DPBEQU_(void)
{
    if (verbose) puts("STUB: DPBEQU_ called");
    return NULL;
}

void* DPBRFS(void)
{
    if (verbose) puts("STUB: DPBRFS called");
    return NULL;
}

void* DPBRFS_(void)
{
    if (verbose) puts("STUB: DPBRFS_ called");
    return NULL;
}

void* DPBSTF(void)
{
    if (verbose) puts("STUB: DPBSTF called");
    return NULL;
}

void* DPBSTF_(void)
{
    if (verbose) puts("STUB: DPBSTF_ called");
    return NULL;
}

void* DPBSV(void)
{
    if (verbose) puts("STUB: DPBSV called");
    return NULL;
}

void* DPBSVX(void)
{
    if (verbose) puts("STUB: DPBSVX called");
    return NULL;
}

void* DPBSVX_(void)
{
    if (verbose) puts("STUB: DPBSVX_ called");
    return NULL;
}

void* DPBSV_(void)
{
    if (verbose) puts("STUB: DPBSV_ called");
    return NULL;
}

void* DPBTF2(void)
{
    if (verbose) puts("STUB: DPBTF2 called");
    return NULL;
}

void* DPBTF2_(void)
{
    if (verbose) puts("STUB: DPBTF2_ called");
    return NULL;
}

void* DPBTRF(void)
{
    if (verbose) puts("STUB: DPBTRF called");
    return NULL;
}

void* DPBTRF_(void)
{
    if (verbose) puts("STUB: DPBTRF_ called");
    return NULL;
}

void* DPBTRS(void)
{
    if (verbose) puts("STUB: DPBTRS called");
    return NULL;
}

void* DPBTRS_(void)
{
    if (verbose) puts("STUB: DPBTRS_ called");
    return NULL;
}

void* DPFTRF(void)
{
    if (verbose) puts("STUB: DPFTRF called");
    return NULL;
}

void* DPFTRF_(void)
{
    if (verbose) puts("STUB: DPFTRF_ called");
    return NULL;
}

void* DPFTRI(void)
{
    if (verbose) puts("STUB: DPFTRI called");
    return NULL;
}

void* DPFTRI_(void)
{
    if (verbose) puts("STUB: DPFTRI_ called");
    return NULL;
}

void* DPFTRS(void)
{
    if (verbose) puts("STUB: DPFTRS called");
    return NULL;
}

void* DPFTRS_(void)
{
    if (verbose) puts("STUB: DPFTRS_ called");
    return NULL;
}

void* DPOCON(void)
{
    if (verbose) puts("STUB: DPOCON called");
    return NULL;
}

void* DPOCON_(void)
{
    if (verbose) puts("STUB: DPOCON_ called");
    return NULL;
}

void* DPOEQU(void)
{
    if (verbose) puts("STUB: DPOEQU called");
    return NULL;
}

void* DPOEQUB(void)
{
    if (verbose) puts("STUB: DPOEQUB called");
    return NULL;
}

void* DPOEQUB_(void)
{
    if (verbose) puts("STUB: DPOEQUB_ called");
    return NULL;
}

void* DPOEQU_(void)
{
    if (verbose) puts("STUB: DPOEQU_ called");
    return NULL;
}

void* DPORFS(void)
{
    if (verbose) puts("STUB: DPORFS called");
    return NULL;
}

void* DPORFS_(void)
{
    if (verbose) puts("STUB: DPORFS_ called");
    return NULL;
}

void* DPOSV(void)
{
    if (verbose) puts("STUB: DPOSV called");
    return NULL;
}

void* DPOSVX(void)
{
    if (verbose) puts("STUB: DPOSVX called");
    return NULL;
}

void* DPOSVX_(void)
{
    if (verbose) puts("STUB: DPOSVX_ called");
    return NULL;
}

void* DPOSV_(void)
{
    if (verbose) puts("STUB: DPOSV_ called");
    return NULL;
}

void* DPOTF2(void)
{
    if (verbose) puts("STUB: DPOTF2 called");
    return NULL;
}

void* DPOTF2_(void)
{
    if (verbose) puts("STUB: DPOTF2_ called");
    return NULL;
}

void* DPOTRF(void)
{
    if (verbose) puts("STUB: DPOTRF called");
    return NULL;
}

void* DPOTRF_(void)
{
    if (verbose) puts("STUB: DPOTRF_ called");
    return NULL;
}

void* DPOTRI(void)
{
    if (verbose) puts("STUB: DPOTRI called");
    return NULL;
}

void* DPOTRI_(void)
{
    if (verbose) puts("STUB: DPOTRI_ called");
    return NULL;
}

void* DPOTRS(void)
{
    if (verbose) puts("STUB: DPOTRS called");
    return NULL;
}

void* DPOTRS_(void)
{
    if (verbose) puts("STUB: DPOTRS_ called");
    return NULL;
}

void* DPPCON(void)
{
    if (verbose) puts("STUB: DPPCON called");
    return NULL;
}

void* DPPCON_(void)
{
    if (verbose) puts("STUB: DPPCON_ called");
    return NULL;
}

void* DPPEQU(void)
{
    if (verbose) puts("STUB: DPPEQU called");
    return NULL;
}

void* DPPEQU_(void)
{
    if (verbose) puts("STUB: DPPEQU_ called");
    return NULL;
}

void* DPPRFS(void)
{
    if (verbose) puts("STUB: DPPRFS called");
    return NULL;
}

void* DPPRFS_(void)
{
    if (verbose) puts("STUB: DPPRFS_ called");
    return NULL;
}

void* DPPSV(void)
{
    if (verbose) puts("STUB: DPPSV called");
    return NULL;
}

void* DPPSVX(void)
{
    if (verbose) puts("STUB: DPPSVX called");
    return NULL;
}

void* DPPSVX_(void)
{
    if (verbose) puts("STUB: DPPSVX_ called");
    return NULL;
}

void* DPPSV_(void)
{
    if (verbose) puts("STUB: DPPSV_ called");
    return NULL;
}

void* DPPTRF(void)
{
    if (verbose) puts("STUB: DPPTRF called");
    return NULL;
}

void* DPPTRF_(void)
{
    if (verbose) puts("STUB: DPPTRF_ called");
    return NULL;
}

void* DPPTRI(void)
{
    if (verbose) puts("STUB: DPPTRI called");
    return NULL;
}

void* DPPTRI_(void)
{
    if (verbose) puts("STUB: DPPTRI_ called");
    return NULL;
}

void* DPPTRS(void)
{
    if (verbose) puts("STUB: DPPTRS called");
    return NULL;
}

void* DPPTRS_(void)
{
    if (verbose) puts("STUB: DPPTRS_ called");
    return NULL;
}

void* DPSTF2(void)
{
    if (verbose) puts("STUB: DPSTF2 called");
    return NULL;
}

void* DPSTF2_(void)
{
    if (verbose) puts("STUB: DPSTF2_ called");
    return NULL;
}

void* DPSTRF(void)
{
    if (verbose) puts("STUB: DPSTRF called");
    return NULL;
}

void* DPSTRF_(void)
{
    if (verbose) puts("STUB: DPSTRF_ called");
    return NULL;
}

void* DPTCON(void)
{
    if (verbose) puts("STUB: DPTCON called");
    return NULL;
}

void* DPTCON_(void)
{
    if (verbose) puts("STUB: DPTCON_ called");
    return NULL;
}

void* DPTEQR(void)
{
    if (verbose) puts("STUB: DPTEQR called");
    return NULL;
}

void* DPTEQR_(void)
{
    if (verbose) puts("STUB: DPTEQR_ called");
    return NULL;
}

void* DPTRFS(void)
{
    if (verbose) puts("STUB: DPTRFS called");
    return NULL;
}

void* DPTRFS_(void)
{
    if (verbose) puts("STUB: DPTRFS_ called");
    return NULL;
}

void* DPTSV(void)
{
    if (verbose) puts("STUB: DPTSV called");
    return NULL;
}

void* DPTSVX(void)
{
    if (verbose) puts("STUB: DPTSVX called");
    return NULL;
}

void* DPTSVX_(void)
{
    if (verbose) puts("STUB: DPTSVX_ called");
    return NULL;
}

void* DPTSV_(void)
{
    if (verbose) puts("STUB: DPTSV_ called");
    return NULL;
}

void* DPTTRF(void)
{
    if (verbose) puts("STUB: DPTTRF called");
    return NULL;
}

void* DPTTRF_(void)
{
    if (verbose) puts("STUB: DPTTRF_ called");
    return NULL;
}

void* DPTTRS(void)
{
    if (verbose) puts("STUB: DPTTRS called");
    return NULL;
}

void* DPTTRS_(void)
{
    if (verbose) puts("STUB: DPTTRS_ called");
    return NULL;
}

void* DPTTS2(void)
{
    if (verbose) puts("STUB: DPTTS2 called");
    return NULL;
}

void* DPTTS2_(void)
{
    if (verbose) puts("STUB: DPTTS2_ called");
    return NULL;
}

void* DRSCL(void)
{
    if (verbose) puts("STUB: DRSCL called");
    return NULL;
}

void* DRSCL_(void)
{
    if (verbose) puts("STUB: DRSCL_ called");
    return NULL;
}

void* DSBEV(void)
{
    if (verbose) puts("STUB: DSBEV called");
    return NULL;
}

void* DSBEVD(void)
{
    if (verbose) puts("STUB: DSBEVD called");
    return NULL;
}

void* DSBEVD_(void)
{
    if (verbose) puts("STUB: DSBEVD_ called");
    return NULL;
}

void* DSBEVX(void)
{
    if (verbose) puts("STUB: DSBEVX called");
    return NULL;
}

void* DSBEVX_(void)
{
    if (verbose) puts("STUB: DSBEVX_ called");
    return NULL;
}

void* DSBEV_(void)
{
    if (verbose) puts("STUB: DSBEV_ called");
    return NULL;
}

void* DSBGST(void)
{
    if (verbose) puts("STUB: DSBGST called");
    return NULL;
}

void* DSBGST_(void)
{
    if (verbose) puts("STUB: DSBGST_ called");
    return NULL;
}

void* DSBGV(void)
{
    if (verbose) puts("STUB: DSBGV called");
    return NULL;
}

void* DSBGVD(void)
{
    if (verbose) puts("STUB: DSBGVD called");
    return NULL;
}

void* DSBGVD_(void)
{
    if (verbose) puts("STUB: DSBGVD_ called");
    return NULL;
}

void* DSBGVX(void)
{
    if (verbose) puts("STUB: DSBGVX called");
    return NULL;
}

void* DSBGVX_(void)
{
    if (verbose) puts("STUB: DSBGVX_ called");
    return NULL;
}

void* DSBGV_(void)
{
    if (verbose) puts("STUB: DSBGV_ called");
    return NULL;
}

void* DSBTRD(void)
{
    if (verbose) puts("STUB: DSBTRD called");
    return NULL;
}

void* DSBTRD_(void)
{
    if (verbose) puts("STUB: DSBTRD_ called");
    return NULL;
}

void* DSFRK(void)
{
    if (verbose) puts("STUB: DSFRK called");
    return NULL;
}

void* DSFRK_(void)
{
    if (verbose) puts("STUB: DSFRK_ called");
    return NULL;
}

void* DSGESV(void)
{
    if (verbose) puts("STUB: DSGESV called");
    return NULL;
}

void* DSGESV_(void)
{
    if (verbose) puts("STUB: DSGESV_ called");
    return NULL;
}

void* DSPCON(void)
{
    if (verbose) puts("STUB: DSPCON called");
    return NULL;
}

void* DSPCON_(void)
{
    if (verbose) puts("STUB: DSPCON_ called");
    return NULL;
}

void* DSPEV(void)
{
    if (verbose) puts("STUB: DSPEV called");
    return NULL;
}

void* DSPEVD(void)
{
    if (verbose) puts("STUB: DSPEVD called");
    return NULL;
}

void* DSPEVD_(void)
{
    if (verbose) puts("STUB: DSPEVD_ called");
    return NULL;
}

void* DSPEVX(void)
{
    if (verbose) puts("STUB: DSPEVX called");
    return NULL;
}

void* DSPEVX_(void)
{
    if (verbose) puts("STUB: DSPEVX_ called");
    return NULL;
}

void* DSPEV_(void)
{
    if (verbose) puts("STUB: DSPEV_ called");
    return NULL;
}

void* DSPGST(void)
{
    if (verbose) puts("STUB: DSPGST called");
    return NULL;
}

void* DSPGST_(void)
{
    if (verbose) puts("STUB: DSPGST_ called");
    return NULL;
}

void* DSPGV(void)
{
    if (verbose) puts("STUB: DSPGV called");
    return NULL;
}

void* DSPGVD(void)
{
    if (verbose) puts("STUB: DSPGVD called");
    return NULL;
}

void* DSPGVD_(void)
{
    if (verbose) puts("STUB: DSPGVD_ called");
    return NULL;
}

void* DSPGVX(void)
{
    if (verbose) puts("STUB: DSPGVX called");
    return NULL;
}

void* DSPGVX_(void)
{
    if (verbose) puts("STUB: DSPGVX_ called");
    return NULL;
}

void* DSPGV_(void)
{
    if (verbose) puts("STUB: DSPGV_ called");
    return NULL;
}

void* DSPOSV(void)
{
    if (verbose) puts("STUB: DSPOSV called");
    return NULL;
}

void* DSPOSV_(void)
{
    if (verbose) puts("STUB: DSPOSV_ called");
    return NULL;
}

void* DSPRFS(void)
{
    if (verbose) puts("STUB: DSPRFS called");
    return NULL;
}

void* DSPRFS_(void)
{
    if (verbose) puts("STUB: DSPRFS_ called");
    return NULL;
}

void* DSPSV(void)
{
    if (verbose) puts("STUB: DSPSV called");
    return NULL;
}

void* DSPSVX(void)
{
    if (verbose) puts("STUB: DSPSVX called");
    return NULL;
}

void* DSPSVX_(void)
{
    if (verbose) puts("STUB: DSPSVX_ called");
    return NULL;
}

void* DSPSV_(void)
{
    if (verbose) puts("STUB: DSPSV_ called");
    return NULL;
}

void* DSPTRD(void)
{
    if (verbose) puts("STUB: DSPTRD called");
    return NULL;
}

void* DSPTRD_(void)
{
    if (verbose) puts("STUB: DSPTRD_ called");
    return NULL;
}

void* DSPTRF(void)
{
    if (verbose) puts("STUB: DSPTRF called");
    return NULL;
}

void* DSPTRF_(void)
{
    if (verbose) puts("STUB: DSPTRF_ called");
    return NULL;
}

void* DSPTRI(void)
{
    if (verbose) puts("STUB: DSPTRI called");
    return NULL;
}

void* DSPTRI_(void)
{
    if (verbose) puts("STUB: DSPTRI_ called");
    return NULL;
}

void* DSPTRS(void)
{
    if (verbose) puts("STUB: DSPTRS called");
    return NULL;
}

void* DSPTRS_(void)
{
    if (verbose) puts("STUB: DSPTRS_ called");
    return NULL;
}

void* DSTEBZ(void)
{
    if (verbose) puts("STUB: DSTEBZ called");
    return NULL;
}

void* DSTEBZ_(void)
{
    if (verbose) puts("STUB: DSTEBZ_ called");
    return NULL;
}

void* DSTEDC(void)
{
    if (verbose) puts("STUB: DSTEDC called");
    return NULL;
}

void* DSTEDC_(void)
{
    if (verbose) puts("STUB: DSTEDC_ called");
    return NULL;
}

void* DSTEGR(void)
{
    if (verbose) puts("STUB: DSTEGR called");
    return NULL;
}

void* DSTEGR_(void)
{
    if (verbose) puts("STUB: DSTEGR_ called");
    return NULL;
}

void* DSTEIN(void)
{
    if (verbose) puts("STUB: DSTEIN called");
    return NULL;
}

void* DSTEIN_(void)
{
    if (verbose) puts("STUB: DSTEIN_ called");
    return NULL;
}

void* DSTEMR(void)
{
    if (verbose) puts("STUB: DSTEMR called");
    return NULL;
}

void* DSTEMR_(void)
{
    if (verbose) puts("STUB: DSTEMR_ called");
    return NULL;
}

void* DSTEQR(void)
{
    if (verbose) puts("STUB: DSTEQR called");
    return NULL;
}

void* DSTEQR_(void)
{
    if (verbose) puts("STUB: DSTEQR_ called");
    return NULL;
}

void* DSTERF(void)
{
    if (verbose) puts("STUB: DSTERF called");
    return NULL;
}

void* DSTERF_(void)
{
    if (verbose) puts("STUB: DSTERF_ called");
    return NULL;
}

void* DSTEV(void)
{
    if (verbose) puts("STUB: DSTEV called");
    return NULL;
}

void* DSTEVD(void)
{
    if (verbose) puts("STUB: DSTEVD called");
    return NULL;
}

void* DSTEVD_(void)
{
    if (verbose) puts("STUB: DSTEVD_ called");
    return NULL;
}

void* DSTEVR(void)
{
    if (verbose) puts("STUB: DSTEVR called");
    return NULL;
}

void* DSTEVR_(void)
{
    if (verbose) puts("STUB: DSTEVR_ called");
    return NULL;
}

void* DSTEVX(void)
{
    if (verbose) puts("STUB: DSTEVX called");
    return NULL;
}

void* DSTEVX_(void)
{
    if (verbose) puts("STUB: DSTEVX_ called");
    return NULL;
}

void* DSTEV_(void)
{
    if (verbose) puts("STUB: DSTEV_ called");
    return NULL;
}

void* DSYCON(void)
{
    if (verbose) puts("STUB: DSYCON called");
    return NULL;
}

void* DSYCON_(void)
{
    if (verbose) puts("STUB: DSYCON_ called");
    return NULL;
}

void* DSYEQUB(void)
{
    if (verbose) puts("STUB: DSYEQUB called");
    return NULL;
}

void* DSYEQUB_(void)
{
    if (verbose) puts("STUB: DSYEQUB_ called");
    return NULL;
}

void* DSYEV(void)
{
    if (verbose) puts("STUB: DSYEV called");
    return NULL;
}

void* DSYEVD(void)
{
    if (verbose) puts("STUB: DSYEVD called");
    return NULL;
}

void* DSYEVD_(void)
{
    if (verbose) puts("STUB: DSYEVD_ called");
    return NULL;
}

void* DSYEVR(void)
{
    if (verbose) puts("STUB: DSYEVR called");
    return NULL;
}

void* DSYEVR_(void)
{
    if (verbose) puts("STUB: DSYEVR_ called");
    return NULL;
}

void* DSYEVX(void)
{
    if (verbose) puts("STUB: DSYEVX called");
    return NULL;
}

void* DSYEVX_(void)
{
    if (verbose) puts("STUB: DSYEVX_ called");
    return NULL;
}

void* DSYEV_(void)
{
    if (verbose) puts("STUB: DSYEV_ called");
    return NULL;
}

void* DSYGS2(void)
{
    if (verbose) puts("STUB: DSYGS2 called");
    return NULL;
}

void* DSYGS2_(void)
{
    if (verbose) puts("STUB: DSYGS2_ called");
    return NULL;
}

void* DSYGST(void)
{
    if (verbose) puts("STUB: DSYGST called");
    return NULL;
}

void* DSYGST_(void)
{
    if (verbose) puts("STUB: DSYGST_ called");
    return NULL;
}

void* DSYGV(void)
{
    if (verbose) puts("STUB: DSYGV called");
    return NULL;
}

void* DSYGVD(void)
{
    if (verbose) puts("STUB: DSYGVD called");
    return NULL;
}

void* DSYGVD_(void)
{
    if (verbose) puts("STUB: DSYGVD_ called");
    return NULL;
}

void* DSYGVX(void)
{
    if (verbose) puts("STUB: DSYGVX called");
    return NULL;
}

void* DSYGVX_(void)
{
    if (verbose) puts("STUB: DSYGVX_ called");
    return NULL;
}

void* DSYGV_(void)
{
    if (verbose) puts("STUB: DSYGV_ called");
    return NULL;
}

void* DSYRFS(void)
{
    if (verbose) puts("STUB: DSYRFS called");
    return NULL;
}

void* DSYRFS_(void)
{
    if (verbose) puts("STUB: DSYRFS_ called");
    return NULL;
}

void* DSYSV(void)
{
    if (verbose) puts("STUB: DSYSV called");
    return NULL;
}

void* DSYSVX(void)
{
    if (verbose) puts("STUB: DSYSVX called");
    return NULL;
}

void* DSYSVX_(void)
{
    if (verbose) puts("STUB: DSYSVX_ called");
    return NULL;
}

void* DSYSV_(void)
{
    if (verbose) puts("STUB: DSYSV_ called");
    return NULL;
}

void* DSYTD2(void)
{
    if (verbose) puts("STUB: DSYTD2 called");
    return NULL;
}

void* DSYTD2_(void)
{
    if (verbose) puts("STUB: DSYTD2_ called");
    return NULL;
}

void* DSYTF2(void)
{
    if (verbose) puts("STUB: DSYTF2 called");
    return NULL;
}

void* DSYTF2_(void)
{
    if (verbose) puts("STUB: DSYTF2_ called");
    return NULL;
}

void* DSYTRD(void)
{
    if (verbose) puts("STUB: DSYTRD called");
    return NULL;
}

void* DSYTRD_(void)
{
    if (verbose) puts("STUB: DSYTRD_ called");
    return NULL;
}

void* DSYTRF(void)
{
    if (verbose) puts("STUB: DSYTRF called");
    return NULL;
}

void* DSYTRF_(void)
{
    if (verbose) puts("STUB: DSYTRF_ called");
    return NULL;
}

void* DSYTRI(void)
{
    if (verbose) puts("STUB: DSYTRI called");
    return NULL;
}

void* DSYTRI_(void)
{
    if (verbose) puts("STUB: DSYTRI_ called");
    return NULL;
}

void* DSYTRS(void)
{
    if (verbose) puts("STUB: DSYTRS called");
    return NULL;
}

void* DSYTRS_(void)
{
    if (verbose) puts("STUB: DSYTRS_ called");
    return NULL;
}

void* DTBCON(void)
{
    if (verbose) puts("STUB: DTBCON called");
    return NULL;
}

void* DTBCON_(void)
{
    if (verbose) puts("STUB: DTBCON_ called");
    return NULL;
}

void* DTBRFS(void)
{
    if (verbose) puts("STUB: DTBRFS called");
    return NULL;
}

void* DTBRFS_(void)
{
    if (verbose) puts("STUB: DTBRFS_ called");
    return NULL;
}

void* DTBTRS(void)
{
    if (verbose) puts("STUB: DTBTRS called");
    return NULL;
}

void* DTBTRS_(void)
{
    if (verbose) puts("STUB: DTBTRS_ called");
    return NULL;
}

void* DTFSM(void)
{
    if (verbose) puts("STUB: DTFSM called");
    return NULL;
}

void* DTFSM_(void)
{
    if (verbose) puts("STUB: DTFSM_ called");
    return NULL;
}

void* DTFTRI(void)
{
    if (verbose) puts("STUB: DTFTRI called");
    return NULL;
}

void* DTFTRI_(void)
{
    if (verbose) puts("STUB: DTFTRI_ called");
    return NULL;
}

void* DTFTTP(void)
{
    if (verbose) puts("STUB: DTFTTP called");
    return NULL;
}

void* DTFTTP_(void)
{
    if (verbose) puts("STUB: DTFTTP_ called");
    return NULL;
}

void* DTFTTR(void)
{
    if (verbose) puts("STUB: DTFTTR called");
    return NULL;
}

void* DTFTTR_(void)
{
    if (verbose) puts("STUB: DTFTTR_ called");
    return NULL;
}

void* DTGEVC(void)
{
    if (verbose) puts("STUB: DTGEVC called");
    return NULL;
}

void* DTGEVC_(void)
{
    if (verbose) puts("STUB: DTGEVC_ called");
    return NULL;
}

void* DTGEX2(void)
{
    if (verbose) puts("STUB: DTGEX2 called");
    return NULL;
}

void* DTGEX2_(void)
{
    if (verbose) puts("STUB: DTGEX2_ called");
    return NULL;
}

void* DTGEXC(void)
{
    if (verbose) puts("STUB: DTGEXC called");
    return NULL;
}

void* DTGEXC_(void)
{
    if (verbose) puts("STUB: DTGEXC_ called");
    return NULL;
}

void* DTGSEN(void)
{
    if (verbose) puts("STUB: DTGSEN called");
    return NULL;
}

void* DTGSEN_(void)
{
    if (verbose) puts("STUB: DTGSEN_ called");
    return NULL;
}

void* DTGSJA(void)
{
    if (verbose) puts("STUB: DTGSJA called");
    return NULL;
}

void* DTGSJA_(void)
{
    if (verbose) puts("STUB: DTGSJA_ called");
    return NULL;
}

void* DTGSNA(void)
{
    if (verbose) puts("STUB: DTGSNA called");
    return NULL;
}

void* DTGSNA_(void)
{
    if (verbose) puts("STUB: DTGSNA_ called");
    return NULL;
}

void* DTGSY2(void)
{
    if (verbose) puts("STUB: DTGSY2 called");
    return NULL;
}

void* DTGSY2_(void)
{
    if (verbose) puts("STUB: DTGSY2_ called");
    return NULL;
}

void* DTGSYL(void)
{
    if (verbose) puts("STUB: DTGSYL called");
    return NULL;
}

void* DTGSYL_(void)
{
    if (verbose) puts("STUB: DTGSYL_ called");
    return NULL;
}

void* DTPCON(void)
{
    if (verbose) puts("STUB: DTPCON called");
    return NULL;
}

void* DTPCON_(void)
{
    if (verbose) puts("STUB: DTPCON_ called");
    return NULL;
}

void* DTPRFS(void)
{
    if (verbose) puts("STUB: DTPRFS called");
    return NULL;
}

void* DTPRFS_(void)
{
    if (verbose) puts("STUB: DTPRFS_ called");
    return NULL;
}

void* DTPTRI(void)
{
    if (verbose) puts("STUB: DTPTRI called");
    return NULL;
}

void* DTPTRI_(void)
{
    if (verbose) puts("STUB: DTPTRI_ called");
    return NULL;
}

void* DTPTRS(void)
{
    if (verbose) puts("STUB: DTPTRS called");
    return NULL;
}

void* DTPTRS_(void)
{
    if (verbose) puts("STUB: DTPTRS_ called");
    return NULL;
}

void* DTPTTF(void)
{
    if (verbose) puts("STUB: DTPTTF called");
    return NULL;
}

void* DTPTTF_(void)
{
    if (verbose) puts("STUB: DTPTTF_ called");
    return NULL;
}

void* DTPTTR(void)
{
    if (verbose) puts("STUB: DTPTTR called");
    return NULL;
}

void* DTPTTR_(void)
{
    if (verbose) puts("STUB: DTPTTR_ called");
    return NULL;
}

void* DTRCON(void)
{
    if (verbose) puts("STUB: DTRCON called");
    return NULL;
}

void* DTRCON_(void)
{
    if (verbose) puts("STUB: DTRCON_ called");
    return NULL;
}

void* DTREVC(void)
{
    if (verbose) puts("STUB: DTREVC called");
    return NULL;
}

void* DTREVC_(void)
{
    if (verbose) puts("STUB: DTREVC_ called");
    return NULL;
}

void* DTREXC(void)
{
    if (verbose) puts("STUB: DTREXC called");
    return NULL;
}

void* DTREXC_(void)
{
    if (verbose) puts("STUB: DTREXC_ called");
    return NULL;
}

void* DTRRFS(void)
{
    if (verbose) puts("STUB: DTRRFS called");
    return NULL;
}

void* DTRRFS_(void)
{
    if (verbose) puts("STUB: DTRRFS_ called");
    return NULL;
}

void* DTRSEN(void)
{
    if (verbose) puts("STUB: DTRSEN called");
    return NULL;
}

void* DTRSEN_(void)
{
    if (verbose) puts("STUB: DTRSEN_ called");
    return NULL;
}

void* DTRSNA(void)
{
    if (verbose) puts("STUB: DTRSNA called");
    return NULL;
}

void* DTRSNA_(void)
{
    if (verbose) puts("STUB: DTRSNA_ called");
    return NULL;
}

void* DTRSYL(void)
{
    if (verbose) puts("STUB: DTRSYL called");
    return NULL;
}

void* DTRSYL_(void)
{
    if (verbose) puts("STUB: DTRSYL_ called");
    return NULL;
}

void* DTRTI2(void)
{
    if (verbose) puts("STUB: DTRTI2 called");
    return NULL;
}

void* DTRTI2_(void)
{
    if (verbose) puts("STUB: DTRTI2_ called");
    return NULL;
}

void* DTRTRI(void)
{
    if (verbose) puts("STUB: DTRTRI called");
    return NULL;
}

void* DTRTRI_(void)
{
    if (verbose) puts("STUB: DTRTRI_ called");
    return NULL;
}

void* DTRTRS(void)
{
    if (verbose) puts("STUB: DTRTRS called");
    return NULL;
}

void* DTRTRS_(void)
{
    if (verbose) puts("STUB: DTRTRS_ called");
    return NULL;
}

void* DTRTTF(void)
{
    if (verbose) puts("STUB: DTRTTF called");
    return NULL;
}

void* DTRTTF_(void)
{
    if (verbose) puts("STUB: DTRTTF_ called");
    return NULL;
}

void* DTRTTP(void)
{
    if (verbose) puts("STUB: DTRTTP called");
    return NULL;
}

void* DTRTTP_(void)
{
    if (verbose) puts("STUB: DTRTTP_ called");
    return NULL;
}

void* DTZRQF(void)
{
    if (verbose) puts("STUB: DTZRQF called");
    return NULL;
}

void* DTZRQF_(void)
{
    if (verbose) puts("STUB: DTZRQF_ called");
    return NULL;
}

void* DTZRZF(void)
{
    if (verbose) puts("STUB: DTZRZF called");
    return NULL;
}

void* DTZRZF_(void)
{
    if (verbose) puts("STUB: DTZRZF_ called");
    return NULL;
}

void* DZSUM1(void)
{
    if (verbose) puts("STUB: DZSUM1 called");
    return NULL;
}

void* DZSUM1_(void)
{
    if (verbose) puts("STUB: DZSUM1_ called");
    return NULL;
}

void* ICMAX1(void)
{
    if (verbose) puts("STUB: ICMAX1 called");
    return NULL;
}

void* ICMAX1_(void)
{
    if (verbose) puts("STUB: ICMAX1_ called");
    return NULL;
}

void* IEEECK(void)
{
    if (verbose) puts("STUB: IEEECK called");
    return NULL;
}

void* IEEECK_(void)
{
    if (verbose) puts("STUB: IEEECK_ called");
    return NULL;
}

void* ILACLC(void)
{
    if (verbose) puts("STUB: ILACLC called");
    return NULL;
}

void* ILACLC_(void)
{
    if (verbose) puts("STUB: ILACLC_ called");
    return NULL;
}

void* ILACLR(void)
{
    if (verbose) puts("STUB: ILACLR called");
    return NULL;
}

void* ILACLR_(void)
{
    if (verbose) puts("STUB: ILACLR_ called");
    return NULL;
}

void* ILADIAG(void)
{
    if (verbose) puts("STUB: ILADIAG called");
    return NULL;
}

void* ILADIAG_(void)
{
    if (verbose) puts("STUB: ILADIAG_ called");
    return NULL;
}

void* ILADLC(void)
{
    if (verbose) puts("STUB: ILADLC called");
    return NULL;
}

void* ILADLC_(void)
{
    if (verbose) puts("STUB: ILADLC_ called");
    return NULL;
}

void* ILADLR(void)
{
    if (verbose) puts("STUB: ILADLR called");
    return NULL;
}

void* ILADLR_(void)
{
    if (verbose) puts("STUB: ILADLR_ called");
    return NULL;
}

void* ILAENV(void)
{
    if (verbose) puts("STUB: ILAENV called");
    return NULL;
}

void* ILAENV_(void)
{
    if (verbose) puts("STUB: ILAENV_ called");
    return NULL;
}

void* ILAPREC(void)
{
    if (verbose) puts("STUB: ILAPREC called");
    return NULL;
}

void* ILAPREC_(void)
{
    if (verbose) puts("STUB: ILAPREC_ called");
    return NULL;
}

void* ILASLC(void)
{
    if (verbose) puts("STUB: ILASLC called");
    return NULL;
}

void* ILASLC_(void)
{
    if (verbose) puts("STUB: ILASLC_ called");
    return NULL;
}

void* ILASLR(void)
{
    if (verbose) puts("STUB: ILASLR called");
    return NULL;
}

void* ILASLR_(void)
{
    if (verbose) puts("STUB: ILASLR_ called");
    return NULL;
}

void* ILATRANS(void)
{
    if (verbose) puts("STUB: ILATRANS called");
    return NULL;
}

void* ILATRANS_(void)
{
    if (verbose) puts("STUB: ILATRANS_ called");
    return NULL;
}

void* ILAUPLO(void)
{
    if (verbose) puts("STUB: ILAUPLO called");
    return NULL;
}

void* ILAUPLO_(void)
{
    if (verbose) puts("STUB: ILAUPLO_ called");
    return NULL;
}

void* ILAVER(void)
{
    if (verbose) puts("STUB: ILAVER called");
    return NULL;
}

void* ILAVER_(void)
{
    if (verbose) puts("STUB: ILAVER_ called");
    return NULL;
}

void* ILAZLC(void)
{
    if (verbose) puts("STUB: ILAZLC called");
    return NULL;
}

void* ILAZLC_(void)
{
    if (verbose) puts("STUB: ILAZLC_ called");
    return NULL;
}

void* ILAZLR(void)
{
    if (verbose) puts("STUB: ILAZLR called");
    return NULL;
}

void* ILAZLR_(void)
{
    if (verbose) puts("STUB: ILAZLR_ called");
    return NULL;
}

void* IPARMQ(void)
{
    if (verbose) puts("STUB: IPARMQ called");
    return NULL;
}

void* IPARMQ_(void)
{
    if (verbose) puts("STUB: IPARMQ_ called");
    return NULL;
}

void* IZMAX1(void)
{
    if (verbose) puts("STUB: IZMAX1 called");
    return NULL;
}

void* IZMAX1_(void)
{
    if (verbose) puts("STUB: IZMAX1_ called");
    return NULL;
}

void* LSAMEN(void)
{
    if (verbose) puts("STUB: LSAMEN called");
    return NULL;
}

void* LSAMEN_(void)
{
    if (verbose) puts("STUB: LSAMEN_ called");
    return NULL;
}

void* SBDSDC(void)
{
    if (verbose) puts("STUB: SBDSDC called");
    return NULL;
}

void* SBDSDC_(void)
{
    if (verbose) puts("STUB: SBDSDC_ called");
    return NULL;
}

void* SBDSQR(void)
{
    if (verbose) puts("STUB: SBDSQR called");
    return NULL;
}

void* SBDSQR_(void)
{
    if (verbose) puts("STUB: SBDSQR_ called");
    return NULL;
}

void* SCSUM1(void)
{
    if (verbose) puts("STUB: SCSUM1 called");
    return NULL;
}

void* SCSUM1_(void)
{
    if (verbose) puts("STUB: SCSUM1_ called");
    return NULL;
}

void* SDISNA(void)
{
    if (verbose) puts("STUB: SDISNA called");
    return NULL;
}

void* SDISNA_(void)
{
    if (verbose) puts("STUB: SDISNA_ called");
    return NULL;
}

void* SGBBRD(void)
{
    if (verbose) puts("STUB: SGBBRD called");
    return NULL;
}

void* SGBBRD_(void)
{
    if (verbose) puts("STUB: SGBBRD_ called");
    return NULL;
}

void* SGBCON(void)
{
    if (verbose) puts("STUB: SGBCON called");
    return NULL;
}

void* SGBCON_(void)
{
    if (verbose) puts("STUB: SGBCON_ called");
    return NULL;
}

void* SGBEQU(void)
{
    if (verbose) puts("STUB: SGBEQU called");
    return NULL;
}

void* SGBEQUB(void)
{
    if (verbose) puts("STUB: SGBEQUB called");
    return NULL;
}

void* SGBEQUB_(void)
{
    if (verbose) puts("STUB: SGBEQUB_ called");
    return NULL;
}

void* SGBEQU_(void)
{
    if (verbose) puts("STUB: SGBEQU_ called");
    return NULL;
}

void* SGBRFS(void)
{
    if (verbose) puts("STUB: SGBRFS called");
    return NULL;
}

void* SGBRFS_(void)
{
    if (verbose) puts("STUB: SGBRFS_ called");
    return NULL;
}

void* SGBSV(void)
{
    if (verbose) puts("STUB: SGBSV called");
    return NULL;
}

void* SGBSVX(void)
{
    if (verbose) puts("STUB: SGBSVX called");
    return NULL;
}

void* SGBSVX_(void)
{
    if (verbose) puts("STUB: SGBSVX_ called");
    return NULL;
}

void* SGBSV_(void)
{
    if (verbose) puts("STUB: SGBSV_ called");
    return NULL;
}

void* SGBTF2(void)
{
    if (verbose) puts("STUB: SGBTF2 called");
    return NULL;
}

void* SGBTF2_(void)
{
    if (verbose) puts("STUB: SGBTF2_ called");
    return NULL;
}

void* SGBTRF(void)
{
    if (verbose) puts("STUB: SGBTRF called");
    return NULL;
}

void* SGBTRF_(void)
{
    if (verbose) puts("STUB: SGBTRF_ called");
    return NULL;
}

void* SGBTRS(void)
{
    if (verbose) puts("STUB: SGBTRS called");
    return NULL;
}

void* SGBTRS_(void)
{
    if (verbose) puts("STUB: SGBTRS_ called");
    return NULL;
}

void* SGEBAK(void)
{
    if (verbose) puts("STUB: SGEBAK called");
    return NULL;
}

void* SGEBAK_(void)
{
    if (verbose) puts("STUB: SGEBAK_ called");
    return NULL;
}

void* SGEBAL(void)
{
    if (verbose) puts("STUB: SGEBAL called");
    return NULL;
}

void* SGEBAL_(void)
{
    if (verbose) puts("STUB: SGEBAL_ called");
    return NULL;
}

void* SGEBD2(void)
{
    if (verbose) puts("STUB: SGEBD2 called");
    return NULL;
}

void* SGEBD2_(void)
{
    if (verbose) puts("STUB: SGEBD2_ called");
    return NULL;
}

void* SGEBRD(void)
{
    if (verbose) puts("STUB: SGEBRD called");
    return NULL;
}

void* SGEBRD_(void)
{
    if (verbose) puts("STUB: SGEBRD_ called");
    return NULL;
}

void* SGECON(void)
{
    if (verbose) puts("STUB: SGECON called");
    return NULL;
}

void* SGECON_(void)
{
    if (verbose) puts("STUB: SGECON_ called");
    return NULL;
}

void* SGEEQU(void)
{
    if (verbose) puts("STUB: SGEEQU called");
    return NULL;
}

void* SGEEQUB(void)
{
    if (verbose) puts("STUB: SGEEQUB called");
    return NULL;
}

void* SGEEQUB_(void)
{
    if (verbose) puts("STUB: SGEEQUB_ called");
    return NULL;
}

void* SGEEQU_(void)
{
    if (verbose) puts("STUB: SGEEQU_ called");
    return NULL;
}

void* SGEES(void)
{
    if (verbose) puts("STUB: SGEES called");
    return NULL;
}

void* SGEESX(void)
{
    if (verbose) puts("STUB: SGEESX called");
    return NULL;
}

void* SGEESX_(void)
{
    if (verbose) puts("STUB: SGEESX_ called");
    return NULL;
}

void* SGEES_(void)
{
    if (verbose) puts("STUB: SGEES_ called");
    return NULL;
}

void* SGEEV(void)
{
    if (verbose) puts("STUB: SGEEV called");
    return NULL;
}

void* SGEEVX(void)
{
    if (verbose) puts("STUB: SGEEVX called");
    return NULL;
}

void* SGEEVX_(void)
{
    if (verbose) puts("STUB: SGEEVX_ called");
    return NULL;
}

void* SGEEV_(void)
{
    if (verbose) puts("STUB: SGEEV_ called");
    return NULL;
}

void* SGEGS(void)
{
    if (verbose) puts("STUB: SGEGS called");
    return NULL;
}

void* SGEGS_(void)
{
    if (verbose) puts("STUB: SGEGS_ called");
    return NULL;
}

void* SGEGV(void)
{
    if (verbose) puts("STUB: SGEGV called");
    return NULL;
}

void* SGEGV_(void)
{
    if (verbose) puts("STUB: SGEGV_ called");
    return NULL;
}

void* SGEHD2(void)
{
    if (verbose) puts("STUB: SGEHD2 called");
    return NULL;
}

void* SGEHD2_(void)
{
    if (verbose) puts("STUB: SGEHD2_ called");
    return NULL;
}

void* SGEHRD(void)
{
    if (verbose) puts("STUB: SGEHRD called");
    return NULL;
}

void* SGEHRD_(void)
{
    if (verbose) puts("STUB: SGEHRD_ called");
    return NULL;
}

void* SGEJSV(void)
{
    if (verbose) puts("STUB: SGEJSV called");
    return NULL;
}

void* SGEJSV_(void)
{
    if (verbose) puts("STUB: SGEJSV_ called");
    return NULL;
}

void* SGELQ2(void)
{
    if (verbose) puts("STUB: SGELQ2 called");
    return NULL;
}

void* SGELQ2_(void)
{
    if (verbose) puts("STUB: SGELQ2_ called");
    return NULL;
}

void* SGELQF(void)
{
    if (verbose) puts("STUB: SGELQF called");
    return NULL;
}

void* SGELQF_(void)
{
    if (verbose) puts("STUB: SGELQF_ called");
    return NULL;
}

void* SGELS(void)
{
    if (verbose) puts("STUB: SGELS called");
    return NULL;
}

void* SGELSD(void)
{
    if (verbose) puts("STUB: SGELSD called");
    return NULL;
}

void* SGELSD_(void)
{
    if (verbose) puts("STUB: SGELSD_ called");
    return NULL;
}

void* SGELSS(void)
{
    if (verbose) puts("STUB: SGELSS called");
    return NULL;
}

void* SGELSS_(void)
{
    if (verbose) puts("STUB: SGELSS_ called");
    return NULL;
}

void* SGELSX(void)
{
    if (verbose) puts("STUB: SGELSX called");
    return NULL;
}

void* SGELSX_(void)
{
    if (verbose) puts("STUB: SGELSX_ called");
    return NULL;
}

void* SGELSY(void)
{
    if (verbose) puts("STUB: SGELSY called");
    return NULL;
}

void* SGELSY_(void)
{
    if (verbose) puts("STUB: SGELSY_ called");
    return NULL;
}

void* SGELS_(void)
{
    if (verbose) puts("STUB: SGELS_ called");
    return NULL;
}

void* SGEQL2(void)
{
    if (verbose) puts("STUB: SGEQL2 called");
    return NULL;
}

void* SGEQL2_(void)
{
    if (verbose) puts("STUB: SGEQL2_ called");
    return NULL;
}

void* SGEQLF(void)
{
    if (verbose) puts("STUB: SGEQLF called");
    return NULL;
}

void* SGEQLF_(void)
{
    if (verbose) puts("STUB: SGEQLF_ called");
    return NULL;
}

void* SGEQP3(void)
{
    if (verbose) puts("STUB: SGEQP3 called");
    return NULL;
}

void* SGEQP3_(void)
{
    if (verbose) puts("STUB: SGEQP3_ called");
    return NULL;
}

void* SGEQPF(void)
{
    if (verbose) puts("STUB: SGEQPF called");
    return NULL;
}

void* SGEQPF_(void)
{
    if (verbose) puts("STUB: SGEQPF_ called");
    return NULL;
}

void* SGEQR2(void)
{
    if (verbose) puts("STUB: SGEQR2 called");
    return NULL;
}

void* SGEQR2_(void)
{
    if (verbose) puts("STUB: SGEQR2_ called");
    return NULL;
}

void* SGEQRF(void)
{
    if (verbose) puts("STUB: SGEQRF called");
    return NULL;
}

void* SGEQRF_(void)
{
    if (verbose) puts("STUB: SGEQRF_ called");
    return NULL;
}

void* SGERFS(void)
{
    if (verbose) puts("STUB: SGERFS called");
    return NULL;
}

void* SGERFS_(void)
{
    if (verbose) puts("STUB: SGERFS_ called");
    return NULL;
}

void* SGERQ2(void)
{
    if (verbose) puts("STUB: SGERQ2 called");
    return NULL;
}

void* SGERQ2_(void)
{
    if (verbose) puts("STUB: SGERQ2_ called");
    return NULL;
}

void* SGERQF(void)
{
    if (verbose) puts("STUB: SGERQF called");
    return NULL;
}

void* SGERQF_(void)
{
    if (verbose) puts("STUB: SGERQF_ called");
    return NULL;
}

void* SGESC2(void)
{
    if (verbose) puts("STUB: SGESC2 called");
    return NULL;
}

void* SGESC2_(void)
{
    if (verbose) puts("STUB: SGESC2_ called");
    return NULL;
}

void* SGESDD(void)
{
    if (verbose) puts("STUB: SGESDD called");
    return NULL;
}

void* SGESDD_(void)
{
    if (verbose) puts("STUB: SGESDD_ called");
    return NULL;
}

void* SGESV(void)
{
    if (verbose) puts("STUB: SGESV called");
    return NULL;
}

void* SGESVD(void)
{
    if (verbose) puts("STUB: SGESVD called");
    return NULL;
}

void* SGESVD_(void)
{
    if (verbose) puts("STUB: SGESVD_ called");
    return NULL;
}

void* SGESVJ(void)
{
    if (verbose) puts("STUB: SGESVJ called");
    return NULL;
}

void* SGESVJ_(void)
{
    if (verbose) puts("STUB: SGESVJ_ called");
    return NULL;
}

void* SGESVX(void)
{
    if (verbose) puts("STUB: SGESVX called");
    return NULL;
}

void* SGESVX_(void)
{
    if (verbose) puts("STUB: SGESVX_ called");
    return NULL;
}

void* SGESV_(void)
{
    if (verbose) puts("STUB: SGESV_ called");
    return NULL;
}

void* SGETC2(void)
{
    if (verbose) puts("STUB: SGETC2 called");
    return NULL;
}

void* SGETC2_(void)
{
    if (verbose) puts("STUB: SGETC2_ called");
    return NULL;
}

void* SGETF2(void)
{
    if (verbose) puts("STUB: SGETF2 called");
    return NULL;
}

void* SGETF2_(void)
{
    if (verbose) puts("STUB: SGETF2_ called");
    return NULL;
}

void* SGETRF(void)
{
    if (verbose) puts("STUB: SGETRF called");
    return NULL;
}

void* SGETRF_(void)
{
    if (verbose) puts("STUB: SGETRF_ called");
    return NULL;
}

void* SGETRI(void)
{
    if (verbose) puts("STUB: SGETRI called");
    return NULL;
}

void* SGETRI_(void)
{
    if (verbose) puts("STUB: SGETRI_ called");
    return NULL;
}

void* SGETRS(void)
{
    if (verbose) puts("STUB: SGETRS called");
    return NULL;
}

void* SGETRS_(void)
{
    if (verbose) puts("STUB: SGETRS_ called");
    return NULL;
}

void* SGGBAK(void)
{
    if (verbose) puts("STUB: SGGBAK called");
    return NULL;
}

void* SGGBAK_(void)
{
    if (verbose) puts("STUB: SGGBAK_ called");
    return NULL;
}

void* SGGBAL(void)
{
    if (verbose) puts("STUB: SGGBAL called");
    return NULL;
}

void* SGGBAL_(void)
{
    if (verbose) puts("STUB: SGGBAL_ called");
    return NULL;
}

void* SGGES(void)
{
    if (verbose) puts("STUB: SGGES called");
    return NULL;
}

void* SGGESX(void)
{
    if (verbose) puts("STUB: SGGESX called");
    return NULL;
}

void* SGGESX_(void)
{
    if (verbose) puts("STUB: SGGESX_ called");
    return NULL;
}

void* SGGES_(void)
{
    if (verbose) puts("STUB: SGGES_ called");
    return NULL;
}

void* SGGEV(void)
{
    if (verbose) puts("STUB: SGGEV called");
    return NULL;
}

void* SGGEVX(void)
{
    if (verbose) puts("STUB: SGGEVX called");
    return NULL;
}

void* SGGEVX_(void)
{
    if (verbose) puts("STUB: SGGEVX_ called");
    return NULL;
}

void* SGGEV_(void)
{
    if (verbose) puts("STUB: SGGEV_ called");
    return NULL;
}

void* SGGGLM(void)
{
    if (verbose) puts("STUB: SGGGLM called");
    return NULL;
}

void* SGGGLM_(void)
{
    if (verbose) puts("STUB: SGGGLM_ called");
    return NULL;
}

void* SGGHRD(void)
{
    if (verbose) puts("STUB: SGGHRD called");
    return NULL;
}

void* SGGHRD_(void)
{
    if (verbose) puts("STUB: SGGHRD_ called");
    return NULL;
}

void* SGGLSE(void)
{
    if (verbose) puts("STUB: SGGLSE called");
    return NULL;
}

void* SGGLSE_(void)
{
    if (verbose) puts("STUB: SGGLSE_ called");
    return NULL;
}

void* SGGQRF(void)
{
    if (verbose) puts("STUB: SGGQRF called");
    return NULL;
}

void* SGGQRF_(void)
{
    if (verbose) puts("STUB: SGGQRF_ called");
    return NULL;
}

void* SGGRQF(void)
{
    if (verbose) puts("STUB: SGGRQF called");
    return NULL;
}

void* SGGRQF_(void)
{
    if (verbose) puts("STUB: SGGRQF_ called");
    return NULL;
}

void* SGGSVD(void)
{
    if (verbose) puts("STUB: SGGSVD called");
    return NULL;
}

void* SGGSVD_(void)
{
    if (verbose) puts("STUB: SGGSVD_ called");
    return NULL;
}

void* SGGSVP(void)
{
    if (verbose) puts("STUB: SGGSVP called");
    return NULL;
}

void* SGGSVP_(void)
{
    if (verbose) puts("STUB: SGGSVP_ called");
    return NULL;
}

void* SGSVJ0(void)
{
    if (verbose) puts("STUB: SGSVJ0 called");
    return NULL;
}

void* SGSVJ0_(void)
{
    if (verbose) puts("STUB: SGSVJ0_ called");
    return NULL;
}

void* SGSVJ1(void)
{
    if (verbose) puts("STUB: SGSVJ1 called");
    return NULL;
}

void* SGSVJ1_(void)
{
    if (verbose) puts("STUB: SGSVJ1_ called");
    return NULL;
}

void* SGTCON(void)
{
    if (verbose) puts("STUB: SGTCON called");
    return NULL;
}

void* SGTCON_(void)
{
    if (verbose) puts("STUB: SGTCON_ called");
    return NULL;
}

void* SGTRFS(void)
{
    if (verbose) puts("STUB: SGTRFS called");
    return NULL;
}

void* SGTRFS_(void)
{
    if (verbose) puts("STUB: SGTRFS_ called");
    return NULL;
}

void* SGTSV(void)
{
    if (verbose) puts("STUB: SGTSV called");
    return NULL;
}

void* SGTSVX(void)
{
    if (verbose) puts("STUB: SGTSVX called");
    return NULL;
}

void* SGTSVX_(void)
{
    if (verbose) puts("STUB: SGTSVX_ called");
    return NULL;
}

void* SGTSV_(void)
{
    if (verbose) puts("STUB: SGTSV_ called");
    return NULL;
}

void* SGTTRF(void)
{
    if (verbose) puts("STUB: SGTTRF called");
    return NULL;
}

void* SGTTRF_(void)
{
    if (verbose) puts("STUB: SGTTRF_ called");
    return NULL;
}

void* SGTTRS(void)
{
    if (verbose) puts("STUB: SGTTRS called");
    return NULL;
}

void* SGTTRS_(void)
{
    if (verbose) puts("STUB: SGTTRS_ called");
    return NULL;
}

void* SGTTS2(void)
{
    if (verbose) puts("STUB: SGTTS2 called");
    return NULL;
}

void* SGTTS2_(void)
{
    if (verbose) puts("STUB: SGTTS2_ called");
    return NULL;
}

void* SHGEQZ(void)
{
    if (verbose) puts("STUB: SHGEQZ called");
    return NULL;
}

void* SHGEQZ_(void)
{
    if (verbose) puts("STUB: SHGEQZ_ called");
    return NULL;
}

void* SHSEIN(void)
{
    if (verbose) puts("STUB: SHSEIN called");
    return NULL;
}

void* SHSEIN_(void)
{
    if (verbose) puts("STUB: SHSEIN_ called");
    return NULL;
}

void* SHSEQR(void)
{
    if (verbose) puts("STUB: SHSEQR called");
    return NULL;
}

void* SHSEQR_(void)
{
    if (verbose) puts("STUB: SHSEQR_ called");
    return NULL;
}

void* SISNAN(void)
{
    if (verbose) puts("STUB: SISNAN called");
    return NULL;
}

void* SISNAN_(void)
{
    if (verbose) puts("STUB: SISNAN_ called");
    return NULL;
}

void* SLABAD(void)
{
    if (verbose) puts("STUB: SLABAD called");
    return NULL;
}

void* SLABAD_(void)
{
    if (verbose) puts("STUB: SLABAD_ called");
    return NULL;
}

void* SLABRD(void)
{
    if (verbose) puts("STUB: SLABRD called");
    return NULL;
}

void* SLABRD_(void)
{
    if (verbose) puts("STUB: SLABRD_ called");
    return NULL;
}

void* SLACN2(void)
{
    if (verbose) puts("STUB: SLACN2 called");
    return NULL;
}

void* SLACN2_(void)
{
    if (verbose) puts("STUB: SLACN2_ called");
    return NULL;
}

void* SLACON(void)
{
    if (verbose) puts("STUB: SLACON called");
    return NULL;
}

void* SLACON_(void)
{
    if (verbose) puts("STUB: SLACON_ called");
    return NULL;
}

void* SLACPY(void)
{
    if (verbose) puts("STUB: SLACPY called");
    return NULL;
}

void* SLACPY_(void)
{
    if (verbose) puts("STUB: SLACPY_ called");
    return NULL;
}

void* SLADIV(void)
{
    if (verbose) puts("STUB: SLADIV called");
    return NULL;
}

void* SLADIV_(void)
{
    if (verbose) puts("STUB: SLADIV_ called");
    return NULL;
}

void* SLAE2(void)
{
    if (verbose) puts("STUB: SLAE2 called");
    return NULL;
}

void* SLAE2_(void)
{
    if (verbose) puts("STUB: SLAE2_ called");
    return NULL;
}

void* SLAEBZ(void)
{
    if (verbose) puts("STUB: SLAEBZ called");
    return NULL;
}

void* SLAEBZ_(void)
{
    if (verbose) puts("STUB: SLAEBZ_ called");
    return NULL;
}

void* SLAED0(void)
{
    if (verbose) puts("STUB: SLAED0 called");
    return NULL;
}

void* SLAED0_(void)
{
    if (verbose) puts("STUB: SLAED0_ called");
    return NULL;
}

void* SLAED1(void)
{
    if (verbose) puts("STUB: SLAED1 called");
    return NULL;
}

void* SLAED1_(void)
{
    if (verbose) puts("STUB: SLAED1_ called");
    return NULL;
}

void* SLAED2(void)
{
    if (verbose) puts("STUB: SLAED2 called");
    return NULL;
}

void* SLAED2_(void)
{
    if (verbose) puts("STUB: SLAED2_ called");
    return NULL;
}

void* SLAED3(void)
{
    if (verbose) puts("STUB: SLAED3 called");
    return NULL;
}

void* SLAED3_(void)
{
    if (verbose) puts("STUB: SLAED3_ called");
    return NULL;
}

void* SLAED4(void)
{
    if (verbose) puts("STUB: SLAED4 called");
    return NULL;
}

void* SLAED4_(void)
{
    if (verbose) puts("STUB: SLAED4_ called");
    return NULL;
}

void* SLAED5(void)
{
    if (verbose) puts("STUB: SLAED5 called");
    return NULL;
}

void* SLAED5_(void)
{
    if (verbose) puts("STUB: SLAED5_ called");
    return NULL;
}

void* SLAED6(void)
{
    if (verbose) puts("STUB: SLAED6 called");
    return NULL;
}

void* SLAED6_(void)
{
    if (verbose) puts("STUB: SLAED6_ called");
    return NULL;
}

void* SLAED7(void)
{
    if (verbose) puts("STUB: SLAED7 called");
    return NULL;
}

void* SLAED7_(void)
{
    if (verbose) puts("STUB: SLAED7_ called");
    return NULL;
}

void* SLAED8(void)
{
    if (verbose) puts("STUB: SLAED8 called");
    return NULL;
}

void* SLAED8_(void)
{
    if (verbose) puts("STUB: SLAED8_ called");
    return NULL;
}

void* SLAED9(void)
{
    if (verbose) puts("STUB: SLAED9 called");
    return NULL;
}

void* SLAED9_(void)
{
    if (verbose) puts("STUB: SLAED9_ called");
    return NULL;
}

void* SLAEDA(void)
{
    if (verbose) puts("STUB: SLAEDA called");
    return NULL;
}

void* SLAEDA_(void)
{
    if (verbose) puts("STUB: SLAEDA_ called");
    return NULL;
}

void* SLAEIN(void)
{
    if (verbose) puts("STUB: SLAEIN called");
    return NULL;
}

void* SLAEIN_(void)
{
    if (verbose) puts("STUB: SLAEIN_ called");
    return NULL;
}

void* SLAEV2(void)
{
    if (verbose) puts("STUB: SLAEV2 called");
    return NULL;
}

void* SLAEV2_(void)
{
    if (verbose) puts("STUB: SLAEV2_ called");
    return NULL;
}

void* SLAEXC(void)
{
    if (verbose) puts("STUB: SLAEXC called");
    return NULL;
}

void* SLAEXC_(void)
{
    if (verbose) puts("STUB: SLAEXC_ called");
    return NULL;
}

void* SLAG2(void)
{
    if (verbose) puts("STUB: SLAG2 called");
    return NULL;
}

void* SLAG2D(void)
{
    if (verbose) puts("STUB: SLAG2D called");
    return NULL;
}

void* SLAG2D_(void)
{
    if (verbose) puts("STUB: SLAG2D_ called");
    return NULL;
}

void* SLAG2_(void)
{
    if (verbose) puts("STUB: SLAG2_ called");
    return NULL;
}

void* SLAGS2(void)
{
    if (verbose) puts("STUB: SLAGS2 called");
    return NULL;
}

void* SLAGS2_(void)
{
    if (verbose) puts("STUB: SLAGS2_ called");
    return NULL;
}

void* SLAGTF(void)
{
    if (verbose) puts("STUB: SLAGTF called");
    return NULL;
}

void* SLAGTF_(void)
{
    if (verbose) puts("STUB: SLAGTF_ called");
    return NULL;
}

void* SLAGTM(void)
{
    if (verbose) puts("STUB: SLAGTM called");
    return NULL;
}

void* SLAGTM_(void)
{
    if (verbose) puts("STUB: SLAGTM_ called");
    return NULL;
}

void* SLAGTS(void)
{
    if (verbose) puts("STUB: SLAGTS called");
    return NULL;
}

void* SLAGTS_(void)
{
    if (verbose) puts("STUB: SLAGTS_ called");
    return NULL;
}

void* SLAGV2(void)
{
    if (verbose) puts("STUB: SLAGV2 called");
    return NULL;
}

void* SLAGV2_(void)
{
    if (verbose) puts("STUB: SLAGV2_ called");
    return NULL;
}

void* SLAHQR(void)
{
    if (verbose) puts("STUB: SLAHQR called");
    return NULL;
}

void* SLAHQR_(void)
{
    if (verbose) puts("STUB: SLAHQR_ called");
    return NULL;
}

void* SLAHR2(void)
{
    if (verbose) puts("STUB: SLAHR2 called");
    return NULL;
}

void* SLAHR2_(void)
{
    if (verbose) puts("STUB: SLAHR2_ called");
    return NULL;
}

void* SLAHRD(void)
{
    if (verbose) puts("STUB: SLAHRD called");
    return NULL;
}

void* SLAHRD_(void)
{
    if (verbose) puts("STUB: SLAHRD_ called");
    return NULL;
}

void* SLAIC1(void)
{
    if (verbose) puts("STUB: SLAIC1 called");
    return NULL;
}

void* SLAIC1_(void)
{
    if (verbose) puts("STUB: SLAIC1_ called");
    return NULL;
}

void* SLAISNAN(void)
{
    if (verbose) puts("STUB: SLAISNAN called");
    return NULL;
}

void* SLAISNAN_(void)
{
    if (verbose) puts("STUB: SLAISNAN_ called");
    return NULL;
}

void* SLALN2(void)
{
    if (verbose) puts("STUB: SLALN2 called");
    return NULL;
}

void* SLALN2_(void)
{
    if (verbose) puts("STUB: SLALN2_ called");
    return NULL;
}

void* SLALS0(void)
{
    if (verbose) puts("STUB: SLALS0 called");
    return NULL;
}

void* SLALS0_(void)
{
    if (verbose) puts("STUB: SLALS0_ called");
    return NULL;
}

void* SLALSA(void)
{
    if (verbose) puts("STUB: SLALSA called");
    return NULL;
}

void* SLALSA_(void)
{
    if (verbose) puts("STUB: SLALSA_ called");
    return NULL;
}

void* SLALSD(void)
{
    if (verbose) puts("STUB: SLALSD called");
    return NULL;
}

void* SLALSD_(void)
{
    if (verbose) puts("STUB: SLALSD_ called");
    return NULL;
}

void* SLAMC1(void)
{
    if (verbose) puts("STUB: SLAMC1 called");
    return NULL;
}

void* SLAMC1_(void)
{
    if (verbose) puts("STUB: SLAMC1_ called");
    return NULL;
}

void* SLAMC2(void)
{
    if (verbose) puts("STUB: SLAMC2 called");
    return NULL;
}

void* SLAMC2_(void)
{
    if (verbose) puts("STUB: SLAMC2_ called");
    return NULL;
}

void* SLAMC3(void)
{
    if (verbose) puts("STUB: SLAMC3 called");
    return NULL;
}

void* SLAMC3_(void)
{
    if (verbose) puts("STUB: SLAMC3_ called");
    return NULL;
}

void* SLAMC4(void)
{
    if (verbose) puts("STUB: SLAMC4 called");
    return NULL;
}

void* SLAMC4_(void)
{
    if (verbose) puts("STUB: SLAMC4_ called");
    return NULL;
}

void* SLAMC5(void)
{
    if (verbose) puts("STUB: SLAMC5 called");
    return NULL;
}

void* SLAMC5_(void)
{
    if (verbose) puts("STUB: SLAMC5_ called");
    return NULL;
}

void* SLAMCH(void)
{
    if (verbose) puts("STUB: SLAMCH called");
    return NULL;
}

void* SLAMCH_(void)
{
    if (verbose) puts("STUB: SLAMCH_ called");
    return NULL;
}

void* SLAMRG(void)
{
    if (verbose) puts("STUB: SLAMRG called");
    return NULL;
}

void* SLAMRG_(void)
{
    if (verbose) puts("STUB: SLAMRG_ called");
    return NULL;
}

void* SLANEG(void)
{
    if (verbose) puts("STUB: SLANEG called");
    return NULL;
}

void* SLANEG_(void)
{
    if (verbose) puts("STUB: SLANEG_ called");
    return NULL;
}

void* SLANGB(void)
{
    if (verbose) puts("STUB: SLANGB called");
    return NULL;
}

void* SLANGB_(void)
{
    if (verbose) puts("STUB: SLANGB_ called");
    return NULL;
}

void* SLANGE(void)
{
    if (verbose) puts("STUB: SLANGE called");
    return NULL;
}

void* SLANGE_(void)
{
    if (verbose) puts("STUB: SLANGE_ called");
    return NULL;
}

void* SLANGT(void)
{
    if (verbose) puts("STUB: SLANGT called");
    return NULL;
}

void* SLANGT_(void)
{
    if (verbose) puts("STUB: SLANGT_ called");
    return NULL;
}

void* SLANHS(void)
{
    if (verbose) puts("STUB: SLANHS called");
    return NULL;
}

void* SLANHS_(void)
{
    if (verbose) puts("STUB: SLANHS_ called");
    return NULL;
}

void* SLANSB(void)
{
    if (verbose) puts("STUB: SLANSB called");
    return NULL;
}

void* SLANSB_(void)
{
    if (verbose) puts("STUB: SLANSB_ called");
    return NULL;
}

void* SLANSF(void)
{
    if (verbose) puts("STUB: SLANSF called");
    return NULL;
}

void* SLANSF_(void)
{
    if (verbose) puts("STUB: SLANSF_ called");
    return NULL;
}

void* SLANSP(void)
{
    if (verbose) puts("STUB: SLANSP called");
    return NULL;
}

void* SLANSP_(void)
{
    if (verbose) puts("STUB: SLANSP_ called");
    return NULL;
}

void* SLANST(void)
{
    if (verbose) puts("STUB: SLANST called");
    return NULL;
}

void* SLANST_(void)
{
    if (verbose) puts("STUB: SLANST_ called");
    return NULL;
}

void* SLANSY(void)
{
    if (verbose) puts("STUB: SLANSY called");
    return NULL;
}

void* SLANSY_(void)
{
    if (verbose) puts("STUB: SLANSY_ called");
    return NULL;
}

void* SLANTB(void)
{
    if (verbose) puts("STUB: SLANTB called");
    return NULL;
}

void* SLANTB_(void)
{
    if (verbose) puts("STUB: SLANTB_ called");
    return NULL;
}

void* SLANTP(void)
{
    if (verbose) puts("STUB: SLANTP called");
    return NULL;
}

void* SLANTP_(void)
{
    if (verbose) puts("STUB: SLANTP_ called");
    return NULL;
}

void* SLANTR(void)
{
    if (verbose) puts("STUB: SLANTR called");
    return NULL;
}

void* SLANTR_(void)
{
    if (verbose) puts("STUB: SLANTR_ called");
    return NULL;
}

void* SLANV2(void)
{
    if (verbose) puts("STUB: SLANV2 called");
    return NULL;
}

void* SLANV2_(void)
{
    if (verbose) puts("STUB: SLANV2_ called");
    return NULL;
}

void* SLAPLL(void)
{
    if (verbose) puts("STUB: SLAPLL called");
    return NULL;
}

void* SLAPLL_(void)
{
    if (verbose) puts("STUB: SLAPLL_ called");
    return NULL;
}

void* SLAPMT(void)
{
    if (verbose) puts("STUB: SLAPMT called");
    return NULL;
}

void* SLAPMT_(void)
{
    if (verbose) puts("STUB: SLAPMT_ called");
    return NULL;
}

void* SLAPY2(void)
{
    if (verbose) puts("STUB: SLAPY2 called");
    return NULL;
}

void* SLAPY2_(void)
{
    if (verbose) puts("STUB: SLAPY2_ called");
    return NULL;
}

void* SLAPY3(void)
{
    if (verbose) puts("STUB: SLAPY3 called");
    return NULL;
}

void* SLAPY3_(void)
{
    if (verbose) puts("STUB: SLAPY3_ called");
    return NULL;
}

void* SLAQGB(void)
{
    if (verbose) puts("STUB: SLAQGB called");
    return NULL;
}

void* SLAQGB_(void)
{
    if (verbose) puts("STUB: SLAQGB_ called");
    return NULL;
}

void* SLAQGE(void)
{
    if (verbose) puts("STUB: SLAQGE called");
    return NULL;
}

void* SLAQGE_(void)
{
    if (verbose) puts("STUB: SLAQGE_ called");
    return NULL;
}

void* SLAQP2(void)
{
    if (verbose) puts("STUB: SLAQP2 called");
    return NULL;
}

void* SLAQP2_(void)
{
    if (verbose) puts("STUB: SLAQP2_ called");
    return NULL;
}

void* SLAQPS(void)
{
    if (verbose) puts("STUB: SLAQPS called");
    return NULL;
}

void* SLAQPS_(void)
{
    if (verbose) puts("STUB: SLAQPS_ called");
    return NULL;
}

void* SLAQR0(void)
{
    if (verbose) puts("STUB: SLAQR0 called");
    return NULL;
}

void* SLAQR0_(void)
{
    if (verbose) puts("STUB: SLAQR0_ called");
    return NULL;
}

void* SLAQR1(void)
{
    if (verbose) puts("STUB: SLAQR1 called");
    return NULL;
}

void* SLAQR1_(void)
{
    if (verbose) puts("STUB: SLAQR1_ called");
    return NULL;
}

void* SLAQR2(void)
{
    if (verbose) puts("STUB: SLAQR2 called");
    return NULL;
}

void* SLAQR2_(void)
{
    if (verbose) puts("STUB: SLAQR2_ called");
    return NULL;
}

void* SLAQR3(void)
{
    if (verbose) puts("STUB: SLAQR3 called");
    return NULL;
}

void* SLAQR3_(void)
{
    if (verbose) puts("STUB: SLAQR3_ called");
    return NULL;
}

void* SLAQR4(void)
{
    if (verbose) puts("STUB: SLAQR4 called");
    return NULL;
}

void* SLAQR4_(void)
{
    if (verbose) puts("STUB: SLAQR4_ called");
    return NULL;
}

void* SLAQR5(void)
{
    if (verbose) puts("STUB: SLAQR5 called");
    return NULL;
}

void* SLAQR5_(void)
{
    if (verbose) puts("STUB: SLAQR5_ called");
    return NULL;
}

void* SLAQSB(void)
{
    if (verbose) puts("STUB: SLAQSB called");
    return NULL;
}

void* SLAQSB_(void)
{
    if (verbose) puts("STUB: SLAQSB_ called");
    return NULL;
}

void* SLAQSP(void)
{
    if (verbose) puts("STUB: SLAQSP called");
    return NULL;
}

void* SLAQSP_(void)
{
    if (verbose) puts("STUB: SLAQSP_ called");
    return NULL;
}

void* SLAQSY(void)
{
    if (verbose) puts("STUB: SLAQSY called");
    return NULL;
}

void* SLAQSY_(void)
{
    if (verbose) puts("STUB: SLAQSY_ called");
    return NULL;
}

void* SLAQTR(void)
{
    if (verbose) puts("STUB: SLAQTR called");
    return NULL;
}

void* SLAQTR_(void)
{
    if (verbose) puts("STUB: SLAQTR_ called");
    return NULL;
}

void* SLAR1V(void)
{
    if (verbose) puts("STUB: SLAR1V called");
    return NULL;
}

void* SLAR1V_(void)
{
    if (verbose) puts("STUB: SLAR1V_ called");
    return NULL;
}

void* SLAR2V(void)
{
    if (verbose) puts("STUB: SLAR2V called");
    return NULL;
}

void* SLAR2V_(void)
{
    if (verbose) puts("STUB: SLAR2V_ called");
    return NULL;
}

void* SLARF(void)
{
    if (verbose) puts("STUB: SLARF called");
    return NULL;
}

void* SLARFB(void)
{
    if (verbose) puts("STUB: SLARFB called");
    return NULL;
}

void* SLARFB_(void)
{
    if (verbose) puts("STUB: SLARFB_ called");
    return NULL;
}

void* SLARFG(void)
{
    if (verbose) puts("STUB: SLARFG called");
    return NULL;
}

void* SLARFG_(void)
{
    if (verbose) puts("STUB: SLARFG_ called");
    return NULL;
}

void* SLARFP(void)
{
    if (verbose) puts("STUB: SLARFP called");
    return NULL;
}

void* SLARFP_(void)
{
    if (verbose) puts("STUB: SLARFP_ called");
    return NULL;
}

void* SLARFT(void)
{
    if (verbose) puts("STUB: SLARFT called");
    return NULL;
}

void* SLARFT_(void)
{
    if (verbose) puts("STUB: SLARFT_ called");
    return NULL;
}

void* SLARFX(void)
{
    if (verbose) puts("STUB: SLARFX called");
    return NULL;
}

void* SLARFX_(void)
{
    if (verbose) puts("STUB: SLARFX_ called");
    return NULL;
}

void* SLARF_(void)
{
    if (verbose) puts("STUB: SLARF_ called");
    return NULL;
}

void* SLARGV(void)
{
    if (verbose) puts("STUB: SLARGV called");
    return NULL;
}

void* SLARGV_(void)
{
    if (verbose) puts("STUB: SLARGV_ called");
    return NULL;
}

void* SLARNV(void)
{
    if (verbose) puts("STUB: SLARNV called");
    return NULL;
}

void* SLARNV_(void)
{
    if (verbose) puts("STUB: SLARNV_ called");
    return NULL;
}

void* SLARRA(void)
{
    if (verbose) puts("STUB: SLARRA called");
    return NULL;
}

void* SLARRA_(void)
{
    if (verbose) puts("STUB: SLARRA_ called");
    return NULL;
}

void* SLARRB(void)
{
    if (verbose) puts("STUB: SLARRB called");
    return NULL;
}

void* SLARRB_(void)
{
    if (verbose) puts("STUB: SLARRB_ called");
    return NULL;
}

void* SLARRC(void)
{
    if (verbose) puts("STUB: SLARRC called");
    return NULL;
}

void* SLARRC_(void)
{
    if (verbose) puts("STUB: SLARRC_ called");
    return NULL;
}

void* SLARRD(void)
{
    if (verbose) puts("STUB: SLARRD called");
    return NULL;
}

void* SLARRD_(void)
{
    if (verbose) puts("STUB: SLARRD_ called");
    return NULL;
}

void* SLARRE(void)
{
    if (verbose) puts("STUB: SLARRE called");
    return NULL;
}

void* SLARRE_(void)
{
    if (verbose) puts("STUB: SLARRE_ called");
    return NULL;
}

void* SLARRF(void)
{
    if (verbose) puts("STUB: SLARRF called");
    return NULL;
}

void* SLARRF_(void)
{
    if (verbose) puts("STUB: SLARRF_ called");
    return NULL;
}

void* SLARRJ(void)
{
    if (verbose) puts("STUB: SLARRJ called");
    return NULL;
}

void* SLARRJ_(void)
{
    if (verbose) puts("STUB: SLARRJ_ called");
    return NULL;
}

void* SLARRK(void)
{
    if (verbose) puts("STUB: SLARRK called");
    return NULL;
}

void* SLARRK_(void)
{
    if (verbose) puts("STUB: SLARRK_ called");
    return NULL;
}

void* SLARRR(void)
{
    if (verbose) puts("STUB: SLARRR called");
    return NULL;
}

void* SLARRR_(void)
{
    if (verbose) puts("STUB: SLARRR_ called");
    return NULL;
}

void* SLARRV(void)
{
    if (verbose) puts("STUB: SLARRV called");
    return NULL;
}

void* SLARRV_(void)
{
    if (verbose) puts("STUB: SLARRV_ called");
    return NULL;
}

void* SLARSCL2(void)
{
    if (verbose) puts("STUB: SLARSCL2 called");
    return NULL;
}

void* SLARSCL2_(void)
{
    if (verbose) puts("STUB: SLARSCL2_ called");
    return NULL;
}

void* SLARTG(void)
{
    if (verbose) puts("STUB: SLARTG called");
    return NULL;
}

void* SLARTG_(void)
{
    if (verbose) puts("STUB: SLARTG_ called");
    return NULL;
}

void* SLARTV(void)
{
    if (verbose) puts("STUB: SLARTV called");
    return NULL;
}

void* SLARTV_(void)
{
    if (verbose) puts("STUB: SLARTV_ called");
    return NULL;
}

void* SLARUV(void)
{
    if (verbose) puts("STUB: SLARUV called");
    return NULL;
}

void* SLARUV_(void)
{
    if (verbose) puts("STUB: SLARUV_ called");
    return NULL;
}

void* SLARZ(void)
{
    if (verbose) puts("STUB: SLARZ called");
    return NULL;
}

void* SLARZB(void)
{
    if (verbose) puts("STUB: SLARZB called");
    return NULL;
}

void* SLARZB_(void)
{
    if (verbose) puts("STUB: SLARZB_ called");
    return NULL;
}

void* SLARZT(void)
{
    if (verbose) puts("STUB: SLARZT called");
    return NULL;
}

void* SLARZT_(void)
{
    if (verbose) puts("STUB: SLARZT_ called");
    return NULL;
}

void* SLARZ_(void)
{
    if (verbose) puts("STUB: SLARZ_ called");
    return NULL;
}

void* SLAS2(void)
{
    if (verbose) puts("STUB: SLAS2 called");
    return NULL;
}

void* SLAS2_(void)
{
    if (verbose) puts("STUB: SLAS2_ called");
    return NULL;
}

void* SLASCL(void)
{
    if (verbose) puts("STUB: SLASCL called");
    return NULL;
}

void* SLASCL2(void)
{
    if (verbose) puts("STUB: SLASCL2 called");
    return NULL;
}

void* SLASCL2_(void)
{
    if (verbose) puts("STUB: SLASCL2_ called");
    return NULL;
}

void* SLASCL_(void)
{
    if (verbose) puts("STUB: SLASCL_ called");
    return NULL;
}

void* SLASD0(void)
{
    if (verbose) puts("STUB: SLASD0 called");
    return NULL;
}

void* SLASD0_(void)
{
    if (verbose) puts("STUB: SLASD0_ called");
    return NULL;
}

void* SLASD1(void)
{
    if (verbose) puts("STUB: SLASD1 called");
    return NULL;
}

void* SLASD1_(void)
{
    if (verbose) puts("STUB: SLASD1_ called");
    return NULL;
}

void* SLASD2(void)
{
    if (verbose) puts("STUB: SLASD2 called");
    return NULL;
}

void* SLASD2_(void)
{
    if (verbose) puts("STUB: SLASD2_ called");
    return NULL;
}

void* SLASD3(void)
{
    if (verbose) puts("STUB: SLASD3 called");
    return NULL;
}

void* SLASD3_(void)
{
    if (verbose) puts("STUB: SLASD3_ called");
    return NULL;
}

void* SLASD4(void)
{
    if (verbose) puts("STUB: SLASD4 called");
    return NULL;
}

void* SLASD4_(void)
{
    if (verbose) puts("STUB: SLASD4_ called");
    return NULL;
}

void* SLASD5(void)
{
    if (verbose) puts("STUB: SLASD5 called");
    return NULL;
}

void* SLASD5_(void)
{
    if (verbose) puts("STUB: SLASD5_ called");
    return NULL;
}

void* SLASD6(void)
{
    if (verbose) puts("STUB: SLASD6 called");
    return NULL;
}

void* SLASD6_(void)
{
    if (verbose) puts("STUB: SLASD6_ called");
    return NULL;
}

void* SLASD7(void)
{
    if (verbose) puts("STUB: SLASD7 called");
    return NULL;
}

void* SLASD7_(void)
{
    if (verbose) puts("STUB: SLASD7_ called");
    return NULL;
}

void* SLASD8(void)
{
    if (verbose) puts("STUB: SLASD8 called");
    return NULL;
}

void* SLASD8_(void)
{
    if (verbose) puts("STUB: SLASD8_ called");
    return NULL;
}

void* SLASD9(void)
{
    if (verbose) puts("STUB: SLASD9 called");
    return NULL;
}

void* SLASD9_(void)
{
    if (verbose) puts("STUB: SLASD9_ called");
    return NULL;
}

void* SLASDA(void)
{
    if (verbose) puts("STUB: SLASDA called");
    return NULL;
}

void* SLASDA_(void)
{
    if (verbose) puts("STUB: SLASDA_ called");
    return NULL;
}

void* SLASDQ(void)
{
    if (verbose) puts("STUB: SLASDQ called");
    return NULL;
}

void* SLASDQ_(void)
{
    if (verbose) puts("STUB: SLASDQ_ called");
    return NULL;
}

void* SLASDT(void)
{
    if (verbose) puts("STUB: SLASDT called");
    return NULL;
}

void* SLASDT_(void)
{
    if (verbose) puts("STUB: SLASDT_ called");
    return NULL;
}

void* SLASET(void)
{
    if (verbose) puts("STUB: SLASET called");
    return NULL;
}

void* SLASET_(void)
{
    if (verbose) puts("STUB: SLASET_ called");
    return NULL;
}

void* SLASQ1(void)
{
    if (verbose) puts("STUB: SLASQ1 called");
    return NULL;
}

void* SLASQ1_(void)
{
    if (verbose) puts("STUB: SLASQ1_ called");
    return NULL;
}

void* SLASQ2(void)
{
    if (verbose) puts("STUB: SLASQ2 called");
    return NULL;
}

void* SLASQ2_(void)
{
    if (verbose) puts("STUB: SLASQ2_ called");
    return NULL;
}

void* SLASQ3(void)
{
    if (verbose) puts("STUB: SLASQ3 called");
    return NULL;
}

void* SLASQ3_(void)
{
    if (verbose) puts("STUB: SLASQ3_ called");
    return NULL;
}

void* SLASQ4(void)
{
    if (verbose) puts("STUB: SLASQ4 called");
    return NULL;
}

void* SLASQ4_(void)
{
    if (verbose) puts("STUB: SLASQ4_ called");
    return NULL;
}

void* SLASQ5(void)
{
    if (verbose) puts("STUB: SLASQ5 called");
    return NULL;
}

void* SLASQ5_(void)
{
    if (verbose) puts("STUB: SLASQ5_ called");
    return NULL;
}

void* SLASQ6(void)
{
    if (verbose) puts("STUB: SLASQ6 called");
    return NULL;
}

void* SLASQ6_(void)
{
    if (verbose) puts("STUB: SLASQ6_ called");
    return NULL;
}

void* SLASR(void)
{
    if (verbose) puts("STUB: SLASR called");
    return NULL;
}

void* SLASRT(void)
{
    if (verbose) puts("STUB: SLASRT called");
    return NULL;
}

void* SLASRT_(void)
{
    if (verbose) puts("STUB: SLASRT_ called");
    return NULL;
}

void* SLASR_(void)
{
    if (verbose) puts("STUB: SLASR_ called");
    return NULL;
}

void* SLASSQ(void)
{
    if (verbose) puts("STUB: SLASSQ called");
    return NULL;
}

void* SLASSQ_(void)
{
    if (verbose) puts("STUB: SLASSQ_ called");
    return NULL;
}

void* SLASV2(void)
{
    if (verbose) puts("STUB: SLASV2 called");
    return NULL;
}

void* SLASV2_(void)
{
    if (verbose) puts("STUB: SLASV2_ called");
    return NULL;
}

void* SLASWP(void)
{
    if (verbose) puts("STUB: SLASWP called");
    return NULL;
}

void* SLASWP_(void)
{
    if (verbose) puts("STUB: SLASWP_ called");
    return NULL;
}

void* SLASY2(void)
{
    if (verbose) puts("STUB: SLASY2 called");
    return NULL;
}

void* SLASY2_(void)
{
    if (verbose) puts("STUB: SLASY2_ called");
    return NULL;
}

void* SLASYF(void)
{
    if (verbose) puts("STUB: SLASYF called");
    return NULL;
}

void* SLASYF_(void)
{
    if (verbose) puts("STUB: SLASYF_ called");
    return NULL;
}

void* SLATBS(void)
{
    if (verbose) puts("STUB: SLATBS called");
    return NULL;
}

void* SLATBS_(void)
{
    if (verbose) puts("STUB: SLATBS_ called");
    return NULL;
}

void* SLATDF(void)
{
    if (verbose) puts("STUB: SLATDF called");
    return NULL;
}

void* SLATDF_(void)
{
    if (verbose) puts("STUB: SLATDF_ called");
    return NULL;
}

void* SLATPS(void)
{
    if (verbose) puts("STUB: SLATPS called");
    return NULL;
}

void* SLATPS_(void)
{
    if (verbose) puts("STUB: SLATPS_ called");
    return NULL;
}

void* SLATRD(void)
{
    if (verbose) puts("STUB: SLATRD called");
    return NULL;
}

void* SLATRD_(void)
{
    if (verbose) puts("STUB: SLATRD_ called");
    return NULL;
}

void* SLATRS(void)
{
    if (verbose) puts("STUB: SLATRS called");
    return NULL;
}

void* SLATRS_(void)
{
    if (verbose) puts("STUB: SLATRS_ called");
    return NULL;
}

void* SLATRZ(void)
{
    if (verbose) puts("STUB: SLATRZ called");
    return NULL;
}

void* SLATRZ_(void)
{
    if (verbose) puts("STUB: SLATRZ_ called");
    return NULL;
}

void* SLATZM(void)
{
    if (verbose) puts("STUB: SLATZM called");
    return NULL;
}

void* SLATZM_(void)
{
    if (verbose) puts("STUB: SLATZM_ called");
    return NULL;
}

void* SLAUU2(void)
{
    if (verbose) puts("STUB: SLAUU2 called");
    return NULL;
}

void* SLAUU2_(void)
{
    if (verbose) puts("STUB: SLAUU2_ called");
    return NULL;
}

void* SLAUUM(void)
{
    if (verbose) puts("STUB: SLAUUM called");
    return NULL;
}

void* SLAUUM_(void)
{
    if (verbose) puts("STUB: SLAUUM_ called");
    return NULL;
}

void* SMAXLOC(void)
{
    if (verbose) puts("STUB: SMAXLOC called");
    return NULL;
}

void* SMAXLOC_(void)
{
    if (verbose) puts("STUB: SMAXLOC_ called");
    return NULL;
}

void* SOPGTR(void)
{
    if (verbose) puts("STUB: SOPGTR called");
    return NULL;
}

void* SOPGTR_(void)
{
    if (verbose) puts("STUB: SOPGTR_ called");
    return NULL;
}

void* SOPMTR(void)
{
    if (verbose) puts("STUB: SOPMTR called");
    return NULL;
}

void* SOPMTR_(void)
{
    if (verbose) puts("STUB: SOPMTR_ called");
    return NULL;
}

void* SORG2L(void)
{
    if (verbose) puts("STUB: SORG2L called");
    return NULL;
}

void* SORG2L_(void)
{
    if (verbose) puts("STUB: SORG2L_ called");
    return NULL;
}

void* SORG2R(void)
{
    if (verbose) puts("STUB: SORG2R called");
    return NULL;
}

void* SORG2R_(void)
{
    if (verbose) puts("STUB: SORG2R_ called");
    return NULL;
}

void* SORGBR(void)
{
    if (verbose) puts("STUB: SORGBR called");
    return NULL;
}

void* SORGBR_(void)
{
    if (verbose) puts("STUB: SORGBR_ called");
    return NULL;
}

void* SORGHR(void)
{
    if (verbose) puts("STUB: SORGHR called");
    return NULL;
}

void* SORGHR_(void)
{
    if (verbose) puts("STUB: SORGHR_ called");
    return NULL;
}

void* SORGL2(void)
{
    if (verbose) puts("STUB: SORGL2 called");
    return NULL;
}

void* SORGL2_(void)
{
    if (verbose) puts("STUB: SORGL2_ called");
    return NULL;
}

void* SORGLQ(void)
{
    if (verbose) puts("STUB: SORGLQ called");
    return NULL;
}

void* SORGLQ_(void)
{
    if (verbose) puts("STUB: SORGLQ_ called");
    return NULL;
}

void* SORGQL(void)
{
    if (verbose) puts("STUB: SORGQL called");
    return NULL;
}

void* SORGQL_(void)
{
    if (verbose) puts("STUB: SORGQL_ called");
    return NULL;
}

void* SORGQR(void)
{
    if (verbose) puts("STUB: SORGQR called");
    return NULL;
}

void* SORGQR_(void)
{
    if (verbose) puts("STUB: SORGQR_ called");
    return NULL;
}

void* SORGR2(void)
{
    if (verbose) puts("STUB: SORGR2 called");
    return NULL;
}

void* SORGR2_(void)
{
    if (verbose) puts("STUB: SORGR2_ called");
    return NULL;
}

void* SORGRQ(void)
{
    if (verbose) puts("STUB: SORGRQ called");
    return NULL;
}

void* SORGRQ_(void)
{
    if (verbose) puts("STUB: SORGRQ_ called");
    return NULL;
}

void* SORGTR(void)
{
    if (verbose) puts("STUB: SORGTR called");
    return NULL;
}

void* SORGTR_(void)
{
    if (verbose) puts("STUB: SORGTR_ called");
    return NULL;
}

void* SORM2L(void)
{
    if (verbose) puts("STUB: SORM2L called");
    return NULL;
}

void* SORM2L_(void)
{
    if (verbose) puts("STUB: SORM2L_ called");
    return NULL;
}

void* SORM2R(void)
{
    if (verbose) puts("STUB: SORM2R called");
    return NULL;
}

void* SORM2R_(void)
{
    if (verbose) puts("STUB: SORM2R_ called");
    return NULL;
}

void* SORMBR(void)
{
    if (verbose) puts("STUB: SORMBR called");
    return NULL;
}

void* SORMBR_(void)
{
    if (verbose) puts("STUB: SORMBR_ called");
    return NULL;
}

void* SORMHR(void)
{
    if (verbose) puts("STUB: SORMHR called");
    return NULL;
}

void* SORMHR_(void)
{
    if (verbose) puts("STUB: SORMHR_ called");
    return NULL;
}

void* SORML2(void)
{
    if (verbose) puts("STUB: SORML2 called");
    return NULL;
}

void* SORML2_(void)
{
    if (verbose) puts("STUB: SORML2_ called");
    return NULL;
}

void* SORMLQ(void)
{
    if (verbose) puts("STUB: SORMLQ called");
    return NULL;
}

void* SORMLQ_(void)
{
    if (verbose) puts("STUB: SORMLQ_ called");
    return NULL;
}

void* SORMQL(void)
{
    if (verbose) puts("STUB: SORMQL called");
    return NULL;
}

void* SORMQL_(void)
{
    if (verbose) puts("STUB: SORMQL_ called");
    return NULL;
}

void* SORMQR(void)
{
    if (verbose) puts("STUB: SORMQR called");
    return NULL;
}

void* SORMQR_(void)
{
    if (verbose) puts("STUB: SORMQR_ called");
    return NULL;
}

void* SORMR2(void)
{
    if (verbose) puts("STUB: SORMR2 called");
    return NULL;
}

void* SORMR2_(void)
{
    if (verbose) puts("STUB: SORMR2_ called");
    return NULL;
}

void* SORMR3(void)
{
    if (verbose) puts("STUB: SORMR3 called");
    return NULL;
}

void* SORMR3_(void)
{
    if (verbose) puts("STUB: SORMR3_ called");
    return NULL;
}

void* SORMRQ(void)
{
    if (verbose) puts("STUB: SORMRQ called");
    return NULL;
}

void* SORMRQ_(void)
{
    if (verbose) puts("STUB: SORMRQ_ called");
    return NULL;
}

void* SORMRZ(void)
{
    if (verbose) puts("STUB: SORMRZ called");
    return NULL;
}

void* SORMRZ_(void)
{
    if (verbose) puts("STUB: SORMRZ_ called");
    return NULL;
}

void* SORMTR(void)
{
    if (verbose) puts("STUB: SORMTR called");
    return NULL;
}

void* SORMTR_(void)
{
    if (verbose) puts("STUB: SORMTR_ called");
    return NULL;
}

void* SPBCON(void)
{
    if (verbose) puts("STUB: SPBCON called");
    return NULL;
}

void* SPBCON_(void)
{
    if (verbose) puts("STUB: SPBCON_ called");
    return NULL;
}

void* SPBEQU(void)
{
    if (verbose) puts("STUB: SPBEQU called");
    return NULL;
}

void* SPBEQU_(void)
{
    if (verbose) puts("STUB: SPBEQU_ called");
    return NULL;
}

void* SPBRFS(void)
{
    if (verbose) puts("STUB: SPBRFS called");
    return NULL;
}

void* SPBRFS_(void)
{
    if (verbose) puts("STUB: SPBRFS_ called");
    return NULL;
}

void* SPBSTF(void)
{
    if (verbose) puts("STUB: SPBSTF called");
    return NULL;
}

void* SPBSTF_(void)
{
    if (verbose) puts("STUB: SPBSTF_ called");
    return NULL;
}

void* SPBSV(void)
{
    if (verbose) puts("STUB: SPBSV called");
    return NULL;
}

void* SPBSVX(void)
{
    if (verbose) puts("STUB: SPBSVX called");
    return NULL;
}

void* SPBSVX_(void)
{
    if (verbose) puts("STUB: SPBSVX_ called");
    return NULL;
}

void* SPBSV_(void)
{
    if (verbose) puts("STUB: SPBSV_ called");
    return NULL;
}

void* SPBTF2(void)
{
    if (verbose) puts("STUB: SPBTF2 called");
    return NULL;
}

void* SPBTF2_(void)
{
    if (verbose) puts("STUB: SPBTF2_ called");
    return NULL;
}

void* SPBTRF(void)
{
    if (verbose) puts("STUB: SPBTRF called");
    return NULL;
}

void* SPBTRF_(void)
{
    if (verbose) puts("STUB: SPBTRF_ called");
    return NULL;
}

void* SPBTRS(void)
{
    if (verbose) puts("STUB: SPBTRS called");
    return NULL;
}

void* SPBTRS_(void)
{
    if (verbose) puts("STUB: SPBTRS_ called");
    return NULL;
}

void* SPFTRF(void)
{
    if (verbose) puts("STUB: SPFTRF called");
    return NULL;
}

void* SPFTRF_(void)
{
    if (verbose) puts("STUB: SPFTRF_ called");
    return NULL;
}

void* SPFTRI(void)
{
    if (verbose) puts("STUB: SPFTRI called");
    return NULL;
}

void* SPFTRI_(void)
{
    if (verbose) puts("STUB: SPFTRI_ called");
    return NULL;
}

void* SPFTRS(void)
{
    if (verbose) puts("STUB: SPFTRS called");
    return NULL;
}

void* SPFTRS_(void)
{
    if (verbose) puts("STUB: SPFTRS_ called");
    return NULL;
}

void* SPOCON(void)
{
    if (verbose) puts("STUB: SPOCON called");
    return NULL;
}

void* SPOCON_(void)
{
    if (verbose) puts("STUB: SPOCON_ called");
    return NULL;
}

void* SPOEQU(void)
{
    if (verbose) puts("STUB: SPOEQU called");
    return NULL;
}

void* SPOEQUB(void)
{
    if (verbose) puts("STUB: SPOEQUB called");
    return NULL;
}

void* SPOEQUB_(void)
{
    if (verbose) puts("STUB: SPOEQUB_ called");
    return NULL;
}

void* SPOEQU_(void)
{
    if (verbose) puts("STUB: SPOEQU_ called");
    return NULL;
}

void* SPORFS(void)
{
    if (verbose) puts("STUB: SPORFS called");
    return NULL;
}

void* SPORFS_(void)
{
    if (verbose) puts("STUB: SPORFS_ called");
    return NULL;
}

void* SPOSV(void)
{
    if (verbose) puts("STUB: SPOSV called");
    return NULL;
}

void* SPOSVX(void)
{
    if (verbose) puts("STUB: SPOSVX called");
    return NULL;
}

void* SPOSVX_(void)
{
    if (verbose) puts("STUB: SPOSVX_ called");
    return NULL;
}

void* SPOSV_(void)
{
    if (verbose) puts("STUB: SPOSV_ called");
    return NULL;
}

void* SPOTF2(void)
{
    if (verbose) puts("STUB: SPOTF2 called");
    return NULL;
}

void* SPOTF2_(void)
{
    if (verbose) puts("STUB: SPOTF2_ called");
    return NULL;
}

void* SPOTRF(void)
{
    if (verbose) puts("STUB: SPOTRF called");
    return NULL;
}

void* SPOTRF_(void)
{
    if (verbose) puts("STUB: SPOTRF_ called");
    return NULL;
}

void* SPOTRI(void)
{
    if (verbose) puts("STUB: SPOTRI called");
    return NULL;
}

void* SPOTRI_(void)
{
    if (verbose) puts("STUB: SPOTRI_ called");
    return NULL;
}

void* SPOTRS(void)
{
    if (verbose) puts("STUB: SPOTRS called");
    return NULL;
}

void* SPOTRS_(void)
{
    if (verbose) puts("STUB: SPOTRS_ called");
    return NULL;
}

void* SPPCON(void)
{
    if (verbose) puts("STUB: SPPCON called");
    return NULL;
}

void* SPPCON_(void)
{
    if (verbose) puts("STUB: SPPCON_ called");
    return NULL;
}

void* SPPEQU(void)
{
    if (verbose) puts("STUB: SPPEQU called");
    return NULL;
}

void* SPPEQU_(void)
{
    if (verbose) puts("STUB: SPPEQU_ called");
    return NULL;
}

void* SPPRFS(void)
{
    if (verbose) puts("STUB: SPPRFS called");
    return NULL;
}

void* SPPRFS_(void)
{
    if (verbose) puts("STUB: SPPRFS_ called");
    return NULL;
}

void* SPPSV(void)
{
    if (verbose) puts("STUB: SPPSV called");
    return NULL;
}

void* SPPSVX(void)
{
    if (verbose) puts("STUB: SPPSVX called");
    return NULL;
}

void* SPPSVX_(void)
{
    if (verbose) puts("STUB: SPPSVX_ called");
    return NULL;
}

void* SPPSV_(void)
{
    if (verbose) puts("STUB: SPPSV_ called");
    return NULL;
}

void* SPPTRF(void)
{
    if (verbose) puts("STUB: SPPTRF called");
    return NULL;
}

void* SPPTRF_(void)
{
    if (verbose) puts("STUB: SPPTRF_ called");
    return NULL;
}

void* SPPTRI(void)
{
    if (verbose) puts("STUB: SPPTRI called");
    return NULL;
}

void* SPPTRI_(void)
{
    if (verbose) puts("STUB: SPPTRI_ called");
    return NULL;
}

void* SPPTRS(void)
{
    if (verbose) puts("STUB: SPPTRS called");
    return NULL;
}

void* SPPTRS_(void)
{
    if (verbose) puts("STUB: SPPTRS_ called");
    return NULL;
}

void* SPSTF2(void)
{
    if (verbose) puts("STUB: SPSTF2 called");
    return NULL;
}

void* SPSTF2_(void)
{
    if (verbose) puts("STUB: SPSTF2_ called");
    return NULL;
}

void* SPSTRF(void)
{
    if (verbose) puts("STUB: SPSTRF called");
    return NULL;
}

void* SPSTRF_(void)
{
    if (verbose) puts("STUB: SPSTRF_ called");
    return NULL;
}

void* SPTCON(void)
{
    if (verbose) puts("STUB: SPTCON called");
    return NULL;
}

void* SPTCON_(void)
{
    if (verbose) puts("STUB: SPTCON_ called");
    return NULL;
}

void* SPTEQR(void)
{
    if (verbose) puts("STUB: SPTEQR called");
    return NULL;
}

void* SPTEQR_(void)
{
    if (verbose) puts("STUB: SPTEQR_ called");
    return NULL;
}

void* SPTRFS(void)
{
    if (verbose) puts("STUB: SPTRFS called");
    return NULL;
}

void* SPTRFS_(void)
{
    if (verbose) puts("STUB: SPTRFS_ called");
    return NULL;
}

void* SPTSV(void)
{
    if (verbose) puts("STUB: SPTSV called");
    return NULL;
}

void* SPTSVX(void)
{
    if (verbose) puts("STUB: SPTSVX called");
    return NULL;
}

void* SPTSVX_(void)
{
    if (verbose) puts("STUB: SPTSVX_ called");
    return NULL;
}

void* SPTSV_(void)
{
    if (verbose) puts("STUB: SPTSV_ called");
    return NULL;
}

void* SPTTRF(void)
{
    if (verbose) puts("STUB: SPTTRF called");
    return NULL;
}

void* SPTTRF_(void)
{
    if (verbose) puts("STUB: SPTTRF_ called");
    return NULL;
}

void* SPTTRS(void)
{
    if (verbose) puts("STUB: SPTTRS called");
    return NULL;
}

void* SPTTRS_(void)
{
    if (verbose) puts("STUB: SPTTRS_ called");
    return NULL;
}

void* SPTTS2(void)
{
    if (verbose) puts("STUB: SPTTS2 called");
    return NULL;
}

void* SPTTS2_(void)
{
    if (verbose) puts("STUB: SPTTS2_ called");
    return NULL;
}

void* SRSCL(void)
{
    if (verbose) puts("STUB: SRSCL called");
    return NULL;
}

void* SRSCL_(void)
{
    if (verbose) puts("STUB: SRSCL_ called");
    return NULL;
}

void* SSBEV(void)
{
    if (verbose) puts("STUB: SSBEV called");
    return NULL;
}

void* SSBEVD(void)
{
    if (verbose) puts("STUB: SSBEVD called");
    return NULL;
}

void* SSBEVD_(void)
{
    if (verbose) puts("STUB: SSBEVD_ called");
    return NULL;
}

void* SSBEVX(void)
{
    if (verbose) puts("STUB: SSBEVX called");
    return NULL;
}

void* SSBEVX_(void)
{
    if (verbose) puts("STUB: SSBEVX_ called");
    return NULL;
}

void* SSBEV_(void)
{
    if (verbose) puts("STUB: SSBEV_ called");
    return NULL;
}

void* SSBGST(void)
{
    if (verbose) puts("STUB: SSBGST called");
    return NULL;
}

void* SSBGST_(void)
{
    if (verbose) puts("STUB: SSBGST_ called");
    return NULL;
}

void* SSBGV(void)
{
    if (verbose) puts("STUB: SSBGV called");
    return NULL;
}

void* SSBGVD(void)
{
    if (verbose) puts("STUB: SSBGVD called");
    return NULL;
}

void* SSBGVD_(void)
{
    if (verbose) puts("STUB: SSBGVD_ called");
    return NULL;
}

void* SSBGVX(void)
{
    if (verbose) puts("STUB: SSBGVX called");
    return NULL;
}

void* SSBGVX_(void)
{
    if (verbose) puts("STUB: SSBGVX_ called");
    return NULL;
}

void* SSBGV_(void)
{
    if (verbose) puts("STUB: SSBGV_ called");
    return NULL;
}

void* SSBTRD(void)
{
    if (verbose) puts("STUB: SSBTRD called");
    return NULL;
}

void* SSBTRD_(void)
{
    if (verbose) puts("STUB: SSBTRD_ called");
    return NULL;
}

void* SSFRK(void)
{
    if (verbose) puts("STUB: SSFRK called");
    return NULL;
}

void* SSFRK_(void)
{
    if (verbose) puts("STUB: SSFRK_ called");
    return NULL;
}

void* SSPCON(void)
{
    if (verbose) puts("STUB: SSPCON called");
    return NULL;
}

void* SSPCON_(void)
{
    if (verbose) puts("STUB: SSPCON_ called");
    return NULL;
}

void* SSPEV(void)
{
    if (verbose) puts("STUB: SSPEV called");
    return NULL;
}

void* SSPEVD(void)
{
    if (verbose) puts("STUB: SSPEVD called");
    return NULL;
}

void* SSPEVD_(void)
{
    if (verbose) puts("STUB: SSPEVD_ called");
    return NULL;
}

void* SSPEVX(void)
{
    if (verbose) puts("STUB: SSPEVX called");
    return NULL;
}

void* SSPEVX_(void)
{
    if (verbose) puts("STUB: SSPEVX_ called");
    return NULL;
}

void* SSPEV_(void)
{
    if (verbose) puts("STUB: SSPEV_ called");
    return NULL;
}

void* SSPGST(void)
{
    if (verbose) puts("STUB: SSPGST called");
    return NULL;
}

void* SSPGST_(void)
{
    if (verbose) puts("STUB: SSPGST_ called");
    return NULL;
}

void* SSPGV(void)
{
    if (verbose) puts("STUB: SSPGV called");
    return NULL;
}

void* SSPGVD(void)
{
    if (verbose) puts("STUB: SSPGVD called");
    return NULL;
}

void* SSPGVD_(void)
{
    if (verbose) puts("STUB: SSPGVD_ called");
    return NULL;
}

void* SSPGVX(void)
{
    if (verbose) puts("STUB: SSPGVX called");
    return NULL;
}

void* SSPGVX_(void)
{
    if (verbose) puts("STUB: SSPGVX_ called");
    return NULL;
}

void* SSPGV_(void)
{
    if (verbose) puts("STUB: SSPGV_ called");
    return NULL;
}

void* SSPRFS(void)
{
    if (verbose) puts("STUB: SSPRFS called");
    return NULL;
}

void* SSPRFS_(void)
{
    if (verbose) puts("STUB: SSPRFS_ called");
    return NULL;
}

void* SSPSV(void)
{
    if (verbose) puts("STUB: SSPSV called");
    return NULL;
}

void* SSPSVX(void)
{
    if (verbose) puts("STUB: SSPSVX called");
    return NULL;
}

void* SSPSVX_(void)
{
    if (verbose) puts("STUB: SSPSVX_ called");
    return NULL;
}

void* SSPSV_(void)
{
    if (verbose) puts("STUB: SSPSV_ called");
    return NULL;
}

void* SSPTRD(void)
{
    if (verbose) puts("STUB: SSPTRD called");
    return NULL;
}

void* SSPTRD_(void)
{
    if (verbose) puts("STUB: SSPTRD_ called");
    return NULL;
}

void* SSPTRF(void)
{
    if (verbose) puts("STUB: SSPTRF called");
    return NULL;
}

void* SSPTRF_(void)
{
    if (verbose) puts("STUB: SSPTRF_ called");
    return NULL;
}

void* SSPTRI(void)
{
    if (verbose) puts("STUB: SSPTRI called");
    return NULL;
}

void* SSPTRI_(void)
{
    if (verbose) puts("STUB: SSPTRI_ called");
    return NULL;
}

void* SSPTRS(void)
{
    if (verbose) puts("STUB: SSPTRS called");
    return NULL;
}

void* SSPTRS_(void)
{
    if (verbose) puts("STUB: SSPTRS_ called");
    return NULL;
}

void* SSTEBZ(void)
{
    if (verbose) puts("STUB: SSTEBZ called");
    return NULL;
}

void* SSTEBZ_(void)
{
    if (verbose) puts("STUB: SSTEBZ_ called");
    return NULL;
}

void* SSTEDC(void)
{
    if (verbose) puts("STUB: SSTEDC called");
    return NULL;
}

void* SSTEDC_(void)
{
    if (verbose) puts("STUB: SSTEDC_ called");
    return NULL;
}

void* SSTEGR(void)
{
    if (verbose) puts("STUB: SSTEGR called");
    return NULL;
}

void* SSTEGR_(void)
{
    if (verbose) puts("STUB: SSTEGR_ called");
    return NULL;
}

void* SSTEIN(void)
{
    if (verbose) puts("STUB: SSTEIN called");
    return NULL;
}

void* SSTEIN_(void)
{
    if (verbose) puts("STUB: SSTEIN_ called");
    return NULL;
}

void* SSTEMR(void)
{
    if (verbose) puts("STUB: SSTEMR called");
    return NULL;
}

void* SSTEMR_(void)
{
    if (verbose) puts("STUB: SSTEMR_ called");
    return NULL;
}

void* SSTEQR(void)
{
    if (verbose) puts("STUB: SSTEQR called");
    return NULL;
}

void* SSTEQR_(void)
{
    if (verbose) puts("STUB: SSTEQR_ called");
    return NULL;
}

void* SSTERF(void)
{
    if (verbose) puts("STUB: SSTERF called");
    return NULL;
}

void* SSTERF_(void)
{
    if (verbose) puts("STUB: SSTERF_ called");
    return NULL;
}

void* SSTEV(void)
{
    if (verbose) puts("STUB: SSTEV called");
    return NULL;
}

void* SSTEVD(void)
{
    if (verbose) puts("STUB: SSTEVD called");
    return NULL;
}

void* SSTEVD_(void)
{
    if (verbose) puts("STUB: SSTEVD_ called");
    return NULL;
}

void* SSTEVR(void)
{
    if (verbose) puts("STUB: SSTEVR called");
    return NULL;
}

void* SSTEVR_(void)
{
    if (verbose) puts("STUB: SSTEVR_ called");
    return NULL;
}

void* SSTEVX(void)
{
    if (verbose) puts("STUB: SSTEVX called");
    return NULL;
}

void* SSTEVX_(void)
{
    if (verbose) puts("STUB: SSTEVX_ called");
    return NULL;
}

void* SSTEV_(void)
{
    if (verbose) puts("STUB: SSTEV_ called");
    return NULL;
}

void* SSYCON(void)
{
    if (verbose) puts("STUB: SSYCON called");
    return NULL;
}

void* SSYCON_(void)
{
    if (verbose) puts("STUB: SSYCON_ called");
    return NULL;
}

void* SSYEQUB(void)
{
    if (verbose) puts("STUB: SSYEQUB called");
    return NULL;
}

void* SSYEQUB_(void)
{
    if (verbose) puts("STUB: SSYEQUB_ called");
    return NULL;
}

void* SSYEV(void)
{
    if (verbose) puts("STUB: SSYEV called");
    return NULL;
}

void* SSYEVD(void)
{
    if (verbose) puts("STUB: SSYEVD called");
    return NULL;
}

void* SSYEVD_(void)
{
    if (verbose) puts("STUB: SSYEVD_ called");
    return NULL;
}

void* SSYEVR(void)
{
    if (verbose) puts("STUB: SSYEVR called");
    return NULL;
}

void* SSYEVR_(void)
{
    if (verbose) puts("STUB: SSYEVR_ called");
    return NULL;
}

void* SSYEVX(void)
{
    if (verbose) puts("STUB: SSYEVX called");
    return NULL;
}

void* SSYEVX_(void)
{
    if (verbose) puts("STUB: SSYEVX_ called");
    return NULL;
}

void* SSYEV_(void)
{
    if (verbose) puts("STUB: SSYEV_ called");
    return NULL;
}

void* SSYGS2(void)
{
    if (verbose) puts("STUB: SSYGS2 called");
    return NULL;
}

void* SSYGS2_(void)
{
    if (verbose) puts("STUB: SSYGS2_ called");
    return NULL;
}

void* SSYGST(void)
{
    if (verbose) puts("STUB: SSYGST called");
    return NULL;
}

void* SSYGST_(void)
{
    if (verbose) puts("STUB: SSYGST_ called");
    return NULL;
}

void* SSYGV(void)
{
    if (verbose) puts("STUB: SSYGV called");
    return NULL;
}

void* SSYGVD(void)
{
    if (verbose) puts("STUB: SSYGVD called");
    return NULL;
}

void* SSYGVD_(void)
{
    if (verbose) puts("STUB: SSYGVD_ called");
    return NULL;
}

void* SSYGVX(void)
{
    if (verbose) puts("STUB: SSYGVX called");
    return NULL;
}

void* SSYGVX_(void)
{
    if (verbose) puts("STUB: SSYGVX_ called");
    return NULL;
}

void* SSYGV_(void)
{
    if (verbose) puts("STUB: SSYGV_ called");
    return NULL;
}

void* SSYRFS(void)
{
    if (verbose) puts("STUB: SSYRFS called");
    return NULL;
}

void* SSYRFS_(void)
{
    if (verbose) puts("STUB: SSYRFS_ called");
    return NULL;
}

void* SSYSV(void)
{
    if (verbose) puts("STUB: SSYSV called");
    return NULL;
}

void* SSYSVX(void)
{
    if (verbose) puts("STUB: SSYSVX called");
    return NULL;
}

void* SSYSVX_(void)
{
    if (verbose) puts("STUB: SSYSVX_ called");
    return NULL;
}

void* SSYSV_(void)
{
    if (verbose) puts("STUB: SSYSV_ called");
    return NULL;
}

void* SSYTD2(void)
{
    if (verbose) puts("STUB: SSYTD2 called");
    return NULL;
}

void* SSYTD2_(void)
{
    if (verbose) puts("STUB: SSYTD2_ called");
    return NULL;
}

void* SSYTF2(void)
{
    if (verbose) puts("STUB: SSYTF2 called");
    return NULL;
}

void* SSYTF2_(void)
{
    if (verbose) puts("STUB: SSYTF2_ called");
    return NULL;
}

void* SSYTRD(void)
{
    if (verbose) puts("STUB: SSYTRD called");
    return NULL;
}

void* SSYTRD_(void)
{
    if (verbose) puts("STUB: SSYTRD_ called");
    return NULL;
}

void* SSYTRF(void)
{
    if (verbose) puts("STUB: SSYTRF called");
    return NULL;
}

void* SSYTRF_(void)
{
    if (verbose) puts("STUB: SSYTRF_ called");
    return NULL;
}

void* SSYTRI(void)
{
    if (verbose) puts("STUB: SSYTRI called");
    return NULL;
}

void* SSYTRI_(void)
{
    if (verbose) puts("STUB: SSYTRI_ called");
    return NULL;
}

void* SSYTRS(void)
{
    if (verbose) puts("STUB: SSYTRS called");
    return NULL;
}

void* SSYTRS_(void)
{
    if (verbose) puts("STUB: SSYTRS_ called");
    return NULL;
}

void* STBCON(void)
{
    if (verbose) puts("STUB: STBCON called");
    return NULL;
}

void* STBCON_(void)
{
    if (verbose) puts("STUB: STBCON_ called");
    return NULL;
}

void* STBRFS(void)
{
    if (verbose) puts("STUB: STBRFS called");
    return NULL;
}

void* STBRFS_(void)
{
    if (verbose) puts("STUB: STBRFS_ called");
    return NULL;
}

void* STBTRS(void)
{
    if (verbose) puts("STUB: STBTRS called");
    return NULL;
}

void* STBTRS_(void)
{
    if (verbose) puts("STUB: STBTRS_ called");
    return NULL;
}

void* STFSM(void)
{
    if (verbose) puts("STUB: STFSM called");
    return NULL;
}

void* STFSM_(void)
{
    if (verbose) puts("STUB: STFSM_ called");
    return NULL;
}

void* STFTRI(void)
{
    if (verbose) puts("STUB: STFTRI called");
    return NULL;
}

void* STFTRI_(void)
{
    if (verbose) puts("STUB: STFTRI_ called");
    return NULL;
}

void* STFTTP(void)
{
    if (verbose) puts("STUB: STFTTP called");
    return NULL;
}

void* STFTTP_(void)
{
    if (verbose) puts("STUB: STFTTP_ called");
    return NULL;
}

void* STFTTR(void)
{
    if (verbose) puts("STUB: STFTTR called");
    return NULL;
}

void* STFTTR_(void)
{
    if (verbose) puts("STUB: STFTTR_ called");
    return NULL;
}

void* STGEVC(void)
{
    if (verbose) puts("STUB: STGEVC called");
    return NULL;
}

void* STGEVC_(void)
{
    if (verbose) puts("STUB: STGEVC_ called");
    return NULL;
}

void* STGEX2(void)
{
    if (verbose) puts("STUB: STGEX2 called");
    return NULL;
}

void* STGEX2_(void)
{
    if (verbose) puts("STUB: STGEX2_ called");
    return NULL;
}

void* STGEXC(void)
{
    if (verbose) puts("STUB: STGEXC called");
    return NULL;
}

void* STGEXC_(void)
{
    if (verbose) puts("STUB: STGEXC_ called");
    return NULL;
}

void* STGSEN(void)
{
    if (verbose) puts("STUB: STGSEN called");
    return NULL;
}

void* STGSEN_(void)
{
    if (verbose) puts("STUB: STGSEN_ called");
    return NULL;
}

void* STGSJA(void)
{
    if (verbose) puts("STUB: STGSJA called");
    return NULL;
}

void* STGSJA_(void)
{
    if (verbose) puts("STUB: STGSJA_ called");
    return NULL;
}

void* STGSNA(void)
{
    if (verbose) puts("STUB: STGSNA called");
    return NULL;
}

void* STGSNA_(void)
{
    if (verbose) puts("STUB: STGSNA_ called");
    return NULL;
}

void* STGSY2(void)
{
    if (verbose) puts("STUB: STGSY2 called");
    return NULL;
}

void* STGSY2_(void)
{
    if (verbose) puts("STUB: STGSY2_ called");
    return NULL;
}

void* STGSYL(void)
{
    if (verbose) puts("STUB: STGSYL called");
    return NULL;
}

void* STGSYL_(void)
{
    if (verbose) puts("STUB: STGSYL_ called");
    return NULL;
}

void* STPCON(void)
{
    if (verbose) puts("STUB: STPCON called");
    return NULL;
}

void* STPCON_(void)
{
    if (verbose) puts("STUB: STPCON_ called");
    return NULL;
}

void* STPRFS(void)
{
    if (verbose) puts("STUB: STPRFS called");
    return NULL;
}

void* STPRFS_(void)
{
    if (verbose) puts("STUB: STPRFS_ called");
    return NULL;
}

void* STPTRI(void)
{
    if (verbose) puts("STUB: STPTRI called");
    return NULL;
}

void* STPTRI_(void)
{
    if (verbose) puts("STUB: STPTRI_ called");
    return NULL;
}

void* STPTRS(void)
{
    if (verbose) puts("STUB: STPTRS called");
    return NULL;
}

void* STPTRS_(void)
{
    if (verbose) puts("STUB: STPTRS_ called");
    return NULL;
}

void* STPTTF(void)
{
    if (verbose) puts("STUB: STPTTF called");
    return NULL;
}

void* STPTTF_(void)
{
    if (verbose) puts("STUB: STPTTF_ called");
    return NULL;
}

void* STPTTR(void)
{
    if (verbose) puts("STUB: STPTTR called");
    return NULL;
}

void* STPTTR_(void)
{
    if (verbose) puts("STUB: STPTTR_ called");
    return NULL;
}

void* STRCON(void)
{
    if (verbose) puts("STUB: STRCON called");
    return NULL;
}

void* STRCON_(void)
{
    if (verbose) puts("STUB: STRCON_ called");
    return NULL;
}

void* STREVC(void)
{
    if (verbose) puts("STUB: STREVC called");
    return NULL;
}

void* STREVC_(void)
{
    if (verbose) puts("STUB: STREVC_ called");
    return NULL;
}

void* STREXC(void)
{
    if (verbose) puts("STUB: STREXC called");
    return NULL;
}

void* STREXC_(void)
{
    if (verbose) puts("STUB: STREXC_ called");
    return NULL;
}

void* STRRFS(void)
{
    if (verbose) puts("STUB: STRRFS called");
    return NULL;
}

void* STRRFS_(void)
{
    if (verbose) puts("STUB: STRRFS_ called");
    return NULL;
}

void* STRSEN(void)
{
    if (verbose) puts("STUB: STRSEN called");
    return NULL;
}

void* STRSEN_(void)
{
    if (verbose) puts("STUB: STRSEN_ called");
    return NULL;
}

void* STRSNA(void)
{
    if (verbose) puts("STUB: STRSNA called");
    return NULL;
}

void* STRSNA_(void)
{
    if (verbose) puts("STUB: STRSNA_ called");
    return NULL;
}

void* STRSYL(void)
{
    if (verbose) puts("STUB: STRSYL called");
    return NULL;
}

void* STRSYL_(void)
{
    if (verbose) puts("STUB: STRSYL_ called");
    return NULL;
}

void* STRTI2(void)
{
    if (verbose) puts("STUB: STRTI2 called");
    return NULL;
}

void* STRTI2_(void)
{
    if (verbose) puts("STUB: STRTI2_ called");
    return NULL;
}

void* STRTRI(void)
{
    if (verbose) puts("STUB: STRTRI called");
    return NULL;
}

void* STRTRI_(void)
{
    if (verbose) puts("STUB: STRTRI_ called");
    return NULL;
}

void* STRTRS(void)
{
    if (verbose) puts("STUB: STRTRS called");
    return NULL;
}

void* STRTRS_(void)
{
    if (verbose) puts("STUB: STRTRS_ called");
    return NULL;
}

void* STRTTF(void)
{
    if (verbose) puts("STUB: STRTTF called");
    return NULL;
}

void* STRTTF_(void)
{
    if (verbose) puts("STUB: STRTTF_ called");
    return NULL;
}

void* STRTTP(void)
{
    if (verbose) puts("STUB: STRTTP called");
    return NULL;
}

void* STRTTP_(void)
{
    if (verbose) puts("STUB: STRTTP_ called");
    return NULL;
}

void* STZRQF(void)
{
    if (verbose) puts("STUB: STZRQF called");
    return NULL;
}

void* STZRQF_(void)
{
    if (verbose) puts("STUB: STZRQF_ called");
    return NULL;
}

void* STZRZF(void)
{
    if (verbose) puts("STUB: STZRZF called");
    return NULL;
}

void* STZRZF_(void)
{
    if (verbose) puts("STUB: STZRZF_ called");
    return NULL;
}

void* ZBDSQR(void)
{
    if (verbose) puts("STUB: ZBDSQR called");
    return NULL;
}

void* ZBDSQR_(void)
{
    if (verbose) puts("STUB: ZBDSQR_ called");
    return NULL;
}

void* ZCGESV(void)
{
    if (verbose) puts("STUB: ZCGESV called");
    return NULL;
}

void* ZCGESV_(void)
{
    if (verbose) puts("STUB: ZCGESV_ called");
    return NULL;
}

void* ZCPOSV(void)
{
    if (verbose) puts("STUB: ZCPOSV called");
    return NULL;
}

void* ZCPOSV_(void)
{
    if (verbose) puts("STUB: ZCPOSV_ called");
    return NULL;
}

void* ZDRSCL(void)
{
    if (verbose) puts("STUB: ZDRSCL called");
    return NULL;
}

void* ZDRSCL_(void)
{
    if (verbose) puts("STUB: ZDRSCL_ called");
    return NULL;
}

void* ZGBBRD(void)
{
    if (verbose) puts("STUB: ZGBBRD called");
    return NULL;
}

void* ZGBBRD_(void)
{
    if (verbose) puts("STUB: ZGBBRD_ called");
    return NULL;
}

void* ZGBCON(void)
{
    if (verbose) puts("STUB: ZGBCON called");
    return NULL;
}

void* ZGBCON_(void)
{
    if (verbose) puts("STUB: ZGBCON_ called");
    return NULL;
}

void* ZGBEQU(void)
{
    if (verbose) puts("STUB: ZGBEQU called");
    return NULL;
}

void* ZGBEQUB(void)
{
    if (verbose) puts("STUB: ZGBEQUB called");
    return NULL;
}

void* ZGBEQUB_(void)
{
    if (verbose) puts("STUB: ZGBEQUB_ called");
    return NULL;
}

void* ZGBEQU_(void)
{
    if (verbose) puts("STUB: ZGBEQU_ called");
    return NULL;
}

void* ZGBRFS(void)
{
    if (verbose) puts("STUB: ZGBRFS called");
    return NULL;
}

void* ZGBRFS_(void)
{
    if (verbose) puts("STUB: ZGBRFS_ called");
    return NULL;
}

void* ZGBSV(void)
{
    if (verbose) puts("STUB: ZGBSV called");
    return NULL;
}

void* ZGBSVX(void)
{
    if (verbose) puts("STUB: ZGBSVX called");
    return NULL;
}

void* ZGBSVX_(void)
{
    if (verbose) puts("STUB: ZGBSVX_ called");
    return NULL;
}

void* ZGBSV_(void)
{
    if (verbose) puts("STUB: ZGBSV_ called");
    return NULL;
}

void* ZGBTF2(void)
{
    if (verbose) puts("STUB: ZGBTF2 called");
    return NULL;
}

void* ZGBTF2_(void)
{
    if (verbose) puts("STUB: ZGBTF2_ called");
    return NULL;
}

void* ZGBTRF(void)
{
    if (verbose) puts("STUB: ZGBTRF called");
    return NULL;
}

void* ZGBTRF_(void)
{
    if (verbose) puts("STUB: ZGBTRF_ called");
    return NULL;
}

void* ZGBTRS(void)
{
    if (verbose) puts("STUB: ZGBTRS called");
    return NULL;
}

void* ZGBTRS_(void)
{
    if (verbose) puts("STUB: ZGBTRS_ called");
    return NULL;
}

void* ZGEBAK(void)
{
    if (verbose) puts("STUB: ZGEBAK called");
    return NULL;
}

void* ZGEBAK_(void)
{
    if (verbose) puts("STUB: ZGEBAK_ called");
    return NULL;
}

void* ZGEBAL(void)
{
    if (verbose) puts("STUB: ZGEBAL called");
    return NULL;
}

void* ZGEBAL_(void)
{
    if (verbose) puts("STUB: ZGEBAL_ called");
    return NULL;
}

void* ZGEBD2(void)
{
    if (verbose) puts("STUB: ZGEBD2 called");
    return NULL;
}

void* ZGEBD2_(void)
{
    if (verbose) puts("STUB: ZGEBD2_ called");
    return NULL;
}

void* ZGEBRD(void)
{
    if (verbose) puts("STUB: ZGEBRD called");
    return NULL;
}

void* ZGEBRD_(void)
{
    if (verbose) puts("STUB: ZGEBRD_ called");
    return NULL;
}

void* ZGECON(void)
{
    if (verbose) puts("STUB: ZGECON called");
    return NULL;
}

void* ZGECON_(void)
{
    if (verbose) puts("STUB: ZGECON_ called");
    return NULL;
}

void* ZGEEQU(void)
{
    if (verbose) puts("STUB: ZGEEQU called");
    return NULL;
}

void* ZGEEQUB(void)
{
    if (verbose) puts("STUB: ZGEEQUB called");
    return NULL;
}

void* ZGEEQUB_(void)
{
    if (verbose) puts("STUB: ZGEEQUB_ called");
    return NULL;
}

void* ZGEEQU_(void)
{
    if (verbose) puts("STUB: ZGEEQU_ called");
    return NULL;
}

void* ZGEES(void)
{
    if (verbose) puts("STUB: ZGEES called");
    return NULL;
}

void* ZGEESX(void)
{
    if (verbose) puts("STUB: ZGEESX called");
    return NULL;
}

void* ZGEESX_(void)
{
    if (verbose) puts("STUB: ZGEESX_ called");
    return NULL;
}

void* ZGEES_(void)
{
    if (verbose) puts("STUB: ZGEES_ called");
    return NULL;
}

void* ZGEEV(void)
{
    if (verbose) puts("STUB: ZGEEV called");
    return NULL;
}

void* ZGEEVX(void)
{
    if (verbose) puts("STUB: ZGEEVX called");
    return NULL;
}

void* ZGEEVX_(void)
{
    if (verbose) puts("STUB: ZGEEVX_ called");
    return NULL;
}

void* ZGEEV_(void)
{
    if (verbose) puts("STUB: ZGEEV_ called");
    return NULL;
}

void* ZGEGS(void)
{
    if (verbose) puts("STUB: ZGEGS called");
    return NULL;
}

void* ZGEGS_(void)
{
    if (verbose) puts("STUB: ZGEGS_ called");
    return NULL;
}

void* ZGEGV(void)
{
    if (verbose) puts("STUB: ZGEGV called");
    return NULL;
}

void* ZGEGV_(void)
{
    if (verbose) puts("STUB: ZGEGV_ called");
    return NULL;
}

void* ZGEHD2(void)
{
    if (verbose) puts("STUB: ZGEHD2 called");
    return NULL;
}

void* ZGEHD2_(void)
{
    if (verbose) puts("STUB: ZGEHD2_ called");
    return NULL;
}

void* ZGEHRD(void)
{
    if (verbose) puts("STUB: ZGEHRD called");
    return NULL;
}

void* ZGEHRD_(void)
{
    if (verbose) puts("STUB: ZGEHRD_ called");
    return NULL;
}

void* ZGELQ2(void)
{
    if (verbose) puts("STUB: ZGELQ2 called");
    return NULL;
}

void* ZGELQ2_(void)
{
    if (verbose) puts("STUB: ZGELQ2_ called");
    return NULL;
}

void* ZGELQF(void)
{
    if (verbose) puts("STUB: ZGELQF called");
    return NULL;
}

void* ZGELQF_(void)
{
    if (verbose) puts("STUB: ZGELQF_ called");
    return NULL;
}

void* ZGELS(void)
{
    if (verbose) puts("STUB: ZGELS called");
    return NULL;
}

void* ZGELSD(void)
{
    if (verbose) puts("STUB: ZGELSD called");
    return NULL;
}

void* ZGELSD_(void)
{
    if (verbose) puts("STUB: ZGELSD_ called");
    return NULL;
}

void* ZGELSS(void)
{
    if (verbose) puts("STUB: ZGELSS called");
    return NULL;
}

void* ZGELSS_(void)
{
    if (verbose) puts("STUB: ZGELSS_ called");
    return NULL;
}

void* ZGELSX(void)
{
    if (verbose) puts("STUB: ZGELSX called");
    return NULL;
}

void* ZGELSX_(void)
{
    if (verbose) puts("STUB: ZGELSX_ called");
    return NULL;
}

void* ZGELSY(void)
{
    if (verbose) puts("STUB: ZGELSY called");
    return NULL;
}

void* ZGELSY_(void)
{
    if (verbose) puts("STUB: ZGELSY_ called");
    return NULL;
}

void* ZGELS_(void)
{
    if (verbose) puts("STUB: ZGELS_ called");
    return NULL;
}

void* ZGEQL2(void)
{
    if (verbose) puts("STUB: ZGEQL2 called");
    return NULL;
}

void* ZGEQL2_(void)
{
    if (verbose) puts("STUB: ZGEQL2_ called");
    return NULL;
}

void* ZGEQLF(void)
{
    if (verbose) puts("STUB: ZGEQLF called");
    return NULL;
}

void* ZGEQLF_(void)
{
    if (verbose) puts("STUB: ZGEQLF_ called");
    return NULL;
}

void* ZGEQP3(void)
{
    if (verbose) puts("STUB: ZGEQP3 called");
    return NULL;
}

void* ZGEQP3_(void)
{
    if (verbose) puts("STUB: ZGEQP3_ called");
    return NULL;
}

void* ZGEQPF(void)
{
    if (verbose) puts("STUB: ZGEQPF called");
    return NULL;
}

void* ZGEQPF_(void)
{
    if (verbose) puts("STUB: ZGEQPF_ called");
    return NULL;
}

void* ZGEQR2(void)
{
    if (verbose) puts("STUB: ZGEQR2 called");
    return NULL;
}

void* ZGEQR2_(void)
{
    if (verbose) puts("STUB: ZGEQR2_ called");
    return NULL;
}

void* ZGEQRF(void)
{
    if (verbose) puts("STUB: ZGEQRF called");
    return NULL;
}

void* ZGEQRF_(void)
{
    if (verbose) puts("STUB: ZGEQRF_ called");
    return NULL;
}

void* ZGERFS(void)
{
    if (verbose) puts("STUB: ZGERFS called");
    return NULL;
}

void* ZGERFS_(void)
{
    if (verbose) puts("STUB: ZGERFS_ called");
    return NULL;
}

void* ZGERQ2(void)
{
    if (verbose) puts("STUB: ZGERQ2 called");
    return NULL;
}

void* ZGERQ2_(void)
{
    if (verbose) puts("STUB: ZGERQ2_ called");
    return NULL;
}

void* ZGERQF(void)
{
    if (verbose) puts("STUB: ZGERQF called");
    return NULL;
}

void* ZGERQF_(void)
{
    if (verbose) puts("STUB: ZGERQF_ called");
    return NULL;
}

void* ZGESC2(void)
{
    if (verbose) puts("STUB: ZGESC2 called");
    return NULL;
}

void* ZGESC2_(void)
{
    if (verbose) puts("STUB: ZGESC2_ called");
    return NULL;
}

void* ZGESDD(void)
{
    if (verbose) puts("STUB: ZGESDD called");
    return NULL;
}

void* ZGESDD_(void)
{
    if (verbose) puts("STUB: ZGESDD_ called");
    return NULL;
}

void* ZGESV(void)
{
    if (verbose) puts("STUB: ZGESV called");
    return NULL;
}

void* ZGESVD(void)
{
    if (verbose) puts("STUB: ZGESVD called");
    return NULL;
}

void* ZGESVD_(void)
{
    if (verbose) puts("STUB: ZGESVD_ called");
    return NULL;
}

void* ZGESVX(void)
{
    if (verbose) puts("STUB: ZGESVX called");
    return NULL;
}

void* ZGESVX_(void)
{
    if (verbose) puts("STUB: ZGESVX_ called");
    return NULL;
}

void* ZGESV_(void)
{
    if (verbose) puts("STUB: ZGESV_ called");
    return NULL;
}

void* ZGETC2(void)
{
    if (verbose) puts("STUB: ZGETC2 called");
    return NULL;
}

void* ZGETC2_(void)
{
    if (verbose) puts("STUB: ZGETC2_ called");
    return NULL;
}

void* ZGETF2(void)
{
    if (verbose) puts("STUB: ZGETF2 called");
    return NULL;
}

void* ZGETF2_(void)
{
    if (verbose) puts("STUB: ZGETF2_ called");
    return NULL;
}

void* ZGETRF(void)
{
    if (verbose) puts("STUB: ZGETRF called");
    return NULL;
}

void* ZGETRF_(void)
{
    if (verbose) puts("STUB: ZGETRF_ called");
    return NULL;
}

void* ZGETRI(void)
{
    if (verbose) puts("STUB: ZGETRI called");
    return NULL;
}

void* ZGETRI_(void)
{
    if (verbose) puts("STUB: ZGETRI_ called");
    return NULL;
}

void* ZGETRS(void)
{
    if (verbose) puts("STUB: ZGETRS called");
    return NULL;
}

void* ZGETRS_(void)
{
    if (verbose) puts("STUB: ZGETRS_ called");
    return NULL;
}

void* ZGGBAK(void)
{
    if (verbose) puts("STUB: ZGGBAK called");
    return NULL;
}

void* ZGGBAK_(void)
{
    if (verbose) puts("STUB: ZGGBAK_ called");
    return NULL;
}

void* ZGGBAL(void)
{
    if (verbose) puts("STUB: ZGGBAL called");
    return NULL;
}

void* ZGGBAL_(void)
{
    if (verbose) puts("STUB: ZGGBAL_ called");
    return NULL;
}

void* ZGGES(void)
{
    if (verbose) puts("STUB: ZGGES called");
    return NULL;
}

void* ZGGESX(void)
{
    if (verbose) puts("STUB: ZGGESX called");
    return NULL;
}

void* ZGGESX_(void)
{
    if (verbose) puts("STUB: ZGGESX_ called");
    return NULL;
}

void* ZGGES_(void)
{
    if (verbose) puts("STUB: ZGGES_ called");
    return NULL;
}

void* ZGGEV(void)
{
    if (verbose) puts("STUB: ZGGEV called");
    return NULL;
}

void* ZGGEVX(void)
{
    if (verbose) puts("STUB: ZGGEVX called");
    return NULL;
}

void* ZGGEVX_(void)
{
    if (verbose) puts("STUB: ZGGEVX_ called");
    return NULL;
}

void* ZGGEV_(void)
{
    if (verbose) puts("STUB: ZGGEV_ called");
    return NULL;
}

void* ZGGGLM(void)
{
    if (verbose) puts("STUB: ZGGGLM called");
    return NULL;
}

void* ZGGGLM_(void)
{
    if (verbose) puts("STUB: ZGGGLM_ called");
    return NULL;
}

void* ZGGHRD(void)
{
    if (verbose) puts("STUB: ZGGHRD called");
    return NULL;
}

void* ZGGHRD_(void)
{
    if (verbose) puts("STUB: ZGGHRD_ called");
    return NULL;
}

void* ZGGLSE(void)
{
    if (verbose) puts("STUB: ZGGLSE called");
    return NULL;
}

void* ZGGLSE_(void)
{
    if (verbose) puts("STUB: ZGGLSE_ called");
    return NULL;
}

void* ZGGQRF(void)
{
    if (verbose) puts("STUB: ZGGQRF called");
    return NULL;
}

void* ZGGQRF_(void)
{
    if (verbose) puts("STUB: ZGGQRF_ called");
    return NULL;
}

void* ZGGRQF(void)
{
    if (verbose) puts("STUB: ZGGRQF called");
    return NULL;
}

void* ZGGRQF_(void)
{
    if (verbose) puts("STUB: ZGGRQF_ called");
    return NULL;
}

void* ZGGSVD(void)
{
    if (verbose) puts("STUB: ZGGSVD called");
    return NULL;
}

void* ZGGSVD_(void)
{
    if (verbose) puts("STUB: ZGGSVD_ called");
    return NULL;
}

void* ZGGSVP(void)
{
    if (verbose) puts("STUB: ZGGSVP called");
    return NULL;
}

void* ZGGSVP_(void)
{
    if (verbose) puts("STUB: ZGGSVP_ called");
    return NULL;
}

void* ZGTCON(void)
{
    if (verbose) puts("STUB: ZGTCON called");
    return NULL;
}

void* ZGTCON_(void)
{
    if (verbose) puts("STUB: ZGTCON_ called");
    return NULL;
}

void* ZGTRFS(void)
{
    if (verbose) puts("STUB: ZGTRFS called");
    return NULL;
}

void* ZGTRFS_(void)
{
    if (verbose) puts("STUB: ZGTRFS_ called");
    return NULL;
}

void* ZGTSV(void)
{
    if (verbose) puts("STUB: ZGTSV called");
    return NULL;
}

void* ZGTSVX(void)
{
    if (verbose) puts("STUB: ZGTSVX called");
    return NULL;
}

void* ZGTSVX_(void)
{
    if (verbose) puts("STUB: ZGTSVX_ called");
    return NULL;
}

void* ZGTSV_(void)
{
    if (verbose) puts("STUB: ZGTSV_ called");
    return NULL;
}

void* ZGTTRF(void)
{
    if (verbose) puts("STUB: ZGTTRF called");
    return NULL;
}

void* ZGTTRF_(void)
{
    if (verbose) puts("STUB: ZGTTRF_ called");
    return NULL;
}

void* ZGTTRS(void)
{
    if (verbose) puts("STUB: ZGTTRS called");
    return NULL;
}

void* ZGTTRS_(void)
{
    if (verbose) puts("STUB: ZGTTRS_ called");
    return NULL;
}

void* ZGTTS2(void)
{
    if (verbose) puts("STUB: ZGTTS2 called");
    return NULL;
}

void* ZGTTS2_(void)
{
    if (verbose) puts("STUB: ZGTTS2_ called");
    return NULL;
}

void* ZHBEV(void)
{
    if (verbose) puts("STUB: ZHBEV called");
    return NULL;
}

void* ZHBEVD(void)
{
    if (verbose) puts("STUB: ZHBEVD called");
    return NULL;
}

void* ZHBEVD_(void)
{
    if (verbose) puts("STUB: ZHBEVD_ called");
    return NULL;
}

void* ZHBEVX(void)
{
    if (verbose) puts("STUB: ZHBEVX called");
    return NULL;
}

void* ZHBEVX_(void)
{
    if (verbose) puts("STUB: ZHBEVX_ called");
    return NULL;
}

void* ZHBEV_(void)
{
    if (verbose) puts("STUB: ZHBEV_ called");
    return NULL;
}

void* ZHBGST(void)
{
    if (verbose) puts("STUB: ZHBGST called");
    return NULL;
}

void* ZHBGST_(void)
{
    if (verbose) puts("STUB: ZHBGST_ called");
    return NULL;
}

void* ZHBGV(void)
{
    if (verbose) puts("STUB: ZHBGV called");
    return NULL;
}

void* ZHBGVD(void)
{
    if (verbose) puts("STUB: ZHBGVD called");
    return NULL;
}

void* ZHBGVD_(void)
{
    if (verbose) puts("STUB: ZHBGVD_ called");
    return NULL;
}

void* ZHBGVX(void)
{
    if (verbose) puts("STUB: ZHBGVX called");
    return NULL;
}

void* ZHBGVX_(void)
{
    if (verbose) puts("STUB: ZHBGVX_ called");
    return NULL;
}

void* ZHBGV_(void)
{
    if (verbose) puts("STUB: ZHBGV_ called");
    return NULL;
}

void* ZHBTRD(void)
{
    if (verbose) puts("STUB: ZHBTRD called");
    return NULL;
}

void* ZHBTRD_(void)
{
    if (verbose) puts("STUB: ZHBTRD_ called");
    return NULL;
}

void* ZHECON(void)
{
    if (verbose) puts("STUB: ZHECON called");
    return NULL;
}

void* ZHECON_(void)
{
    if (verbose) puts("STUB: ZHECON_ called");
    return NULL;
}

void* ZHEEQUB(void)
{
    if (verbose) puts("STUB: ZHEEQUB called");
    return NULL;
}

void* ZHEEQUB_(void)
{
    if (verbose) puts("STUB: ZHEEQUB_ called");
    return NULL;
}

void* ZHEEV(void)
{
    if (verbose) puts("STUB: ZHEEV called");
    return NULL;
}

void* ZHEEVD(void)
{
    if (verbose) puts("STUB: ZHEEVD called");
    return NULL;
}

void* ZHEEVD_(void)
{
    if (verbose) puts("STUB: ZHEEVD_ called");
    return NULL;
}

void* ZHEEVR(void)
{
    if (verbose) puts("STUB: ZHEEVR called");
    return NULL;
}

void* ZHEEVR_(void)
{
    if (verbose) puts("STUB: ZHEEVR_ called");
    return NULL;
}

void* ZHEEVX(void)
{
    if (verbose) puts("STUB: ZHEEVX called");
    return NULL;
}

void* ZHEEVX_(void)
{
    if (verbose) puts("STUB: ZHEEVX_ called");
    return NULL;
}

void* ZHEEV_(void)
{
    if (verbose) puts("STUB: ZHEEV_ called");
    return NULL;
}

void* ZHEGS2(void)
{
    if (verbose) puts("STUB: ZHEGS2 called");
    return NULL;
}

void* ZHEGS2_(void)
{
    if (verbose) puts("STUB: ZHEGS2_ called");
    return NULL;
}

void* ZHEGST(void)
{
    if (verbose) puts("STUB: ZHEGST called");
    return NULL;
}

void* ZHEGST_(void)
{
    if (verbose) puts("STUB: ZHEGST_ called");
    return NULL;
}

void* ZHEGV(void)
{
    if (verbose) puts("STUB: ZHEGV called");
    return NULL;
}

void* ZHEGVD(void)
{
    if (verbose) puts("STUB: ZHEGVD called");
    return NULL;
}

void* ZHEGVD_(void)
{
    if (verbose) puts("STUB: ZHEGVD_ called");
    return NULL;
}

void* ZHEGVX(void)
{
    if (verbose) puts("STUB: ZHEGVX called");
    return NULL;
}

void* ZHEGVX_(void)
{
    if (verbose) puts("STUB: ZHEGVX_ called");
    return NULL;
}

void* ZHEGV_(void)
{
    if (verbose) puts("STUB: ZHEGV_ called");
    return NULL;
}

void* ZHERFS(void)
{
    if (verbose) puts("STUB: ZHERFS called");
    return NULL;
}

void* ZHERFS_(void)
{
    if (verbose) puts("STUB: ZHERFS_ called");
    return NULL;
}

void* ZHESV(void)
{
    if (verbose) puts("STUB: ZHESV called");
    return NULL;
}

void* ZHESVX(void)
{
    if (verbose) puts("STUB: ZHESVX called");
    return NULL;
}

void* ZHESVX_(void)
{
    if (verbose) puts("STUB: ZHESVX_ called");
    return NULL;
}

void* ZHESV_(void)
{
    if (verbose) puts("STUB: ZHESV_ called");
    return NULL;
}

void* ZHETD2(void)
{
    if (verbose) puts("STUB: ZHETD2 called");
    return NULL;
}

void* ZHETD2_(void)
{
    if (verbose) puts("STUB: ZHETD2_ called");
    return NULL;
}

void* ZHETF2(void)
{
    if (verbose) puts("STUB: ZHETF2 called");
    return NULL;
}

void* ZHETF2_(void)
{
    if (verbose) puts("STUB: ZHETF2_ called");
    return NULL;
}

void* ZHETRD(void)
{
    if (verbose) puts("STUB: ZHETRD called");
    return NULL;
}

void* ZHETRD_(void)
{
    if (verbose) puts("STUB: ZHETRD_ called");
    return NULL;
}

void* ZHETRF(void)
{
    if (verbose) puts("STUB: ZHETRF called");
    return NULL;
}

void* ZHETRF_(void)
{
    if (verbose) puts("STUB: ZHETRF_ called");
    return NULL;
}

void* ZHETRI(void)
{
    if (verbose) puts("STUB: ZHETRI called");
    return NULL;
}

void* ZHETRI_(void)
{
    if (verbose) puts("STUB: ZHETRI_ called");
    return NULL;
}

void* ZHETRS(void)
{
    if (verbose) puts("STUB: ZHETRS called");
    return NULL;
}

void* ZHETRS_(void)
{
    if (verbose) puts("STUB: ZHETRS_ called");
    return NULL;
}

void* ZHFRK(void)
{
    if (verbose) puts("STUB: ZHFRK called");
    return NULL;
}

void* ZHFRK_(void)
{
    if (verbose) puts("STUB: ZHFRK_ called");
    return NULL;
}

void* ZHGEQZ(void)
{
    if (verbose) puts("STUB: ZHGEQZ called");
    return NULL;
}

void* ZHGEQZ_(void)
{
    if (verbose) puts("STUB: ZHGEQZ_ called");
    return NULL;
}

void* ZHPCON(void)
{
    if (verbose) puts("STUB: ZHPCON called");
    return NULL;
}

void* ZHPCON_(void)
{
    if (verbose) puts("STUB: ZHPCON_ called");
    return NULL;
}

void* ZHPEV(void)
{
    if (verbose) puts("STUB: ZHPEV called");
    return NULL;
}

void* ZHPEVD(void)
{
    if (verbose) puts("STUB: ZHPEVD called");
    return NULL;
}

void* ZHPEVD_(void)
{
    if (verbose) puts("STUB: ZHPEVD_ called");
    return NULL;
}

void* ZHPEVX(void)
{
    if (verbose) puts("STUB: ZHPEVX called");
    return NULL;
}

void* ZHPEVX_(void)
{
    if (verbose) puts("STUB: ZHPEVX_ called");
    return NULL;
}

void* ZHPEV_(void)
{
    if (verbose) puts("STUB: ZHPEV_ called");
    return NULL;
}

void* ZHPGST(void)
{
    if (verbose) puts("STUB: ZHPGST called");
    return NULL;
}

void* ZHPGST_(void)
{
    if (verbose) puts("STUB: ZHPGST_ called");
    return NULL;
}

void* ZHPGV(void)
{
    if (verbose) puts("STUB: ZHPGV called");
    return NULL;
}

void* ZHPGVD(void)
{
    if (verbose) puts("STUB: ZHPGVD called");
    return NULL;
}

void* ZHPGVD_(void)
{
    if (verbose) puts("STUB: ZHPGVD_ called");
    return NULL;
}

void* ZHPGVX(void)
{
    if (verbose) puts("STUB: ZHPGVX called");
    return NULL;
}

void* ZHPGVX_(void)
{
    if (verbose) puts("STUB: ZHPGVX_ called");
    return NULL;
}

void* ZHPGV_(void)
{
    if (verbose) puts("STUB: ZHPGV_ called");
    return NULL;
}

void* ZHPRFS(void)
{
    if (verbose) puts("STUB: ZHPRFS called");
    return NULL;
}

void* ZHPRFS_(void)
{
    if (verbose) puts("STUB: ZHPRFS_ called");
    return NULL;
}

void* ZHPSV(void)
{
    if (verbose) puts("STUB: ZHPSV called");
    return NULL;
}

void* ZHPSVX(void)
{
    if (verbose) puts("STUB: ZHPSVX called");
    return NULL;
}

void* ZHPSVX_(void)
{
    if (verbose) puts("STUB: ZHPSVX_ called");
    return NULL;
}

void* ZHPSV_(void)
{
    if (verbose) puts("STUB: ZHPSV_ called");
    return NULL;
}

void* ZHPTRD(void)
{
    if (verbose) puts("STUB: ZHPTRD called");
    return NULL;
}

void* ZHPTRD_(void)
{
    if (verbose) puts("STUB: ZHPTRD_ called");
    return NULL;
}

void* ZHPTRF(void)
{
    if (verbose) puts("STUB: ZHPTRF called");
    return NULL;
}

void* ZHPTRF_(void)
{
    if (verbose) puts("STUB: ZHPTRF_ called");
    return NULL;
}

void* ZHPTRI(void)
{
    if (verbose) puts("STUB: ZHPTRI called");
    return NULL;
}

void* ZHPTRI_(void)
{
    if (verbose) puts("STUB: ZHPTRI_ called");
    return NULL;
}

void* ZHPTRS(void)
{
    if (verbose) puts("STUB: ZHPTRS called");
    return NULL;
}

void* ZHPTRS_(void)
{
    if (verbose) puts("STUB: ZHPTRS_ called");
    return NULL;
}

void* ZHSEIN(void)
{
    if (verbose) puts("STUB: ZHSEIN called");
    return NULL;
}

void* ZHSEIN_(void)
{
    if (verbose) puts("STUB: ZHSEIN_ called");
    return NULL;
}

void* ZHSEQR(void)
{
    if (verbose) puts("STUB: ZHSEQR called");
    return NULL;
}

void* ZHSEQR_(void)
{
    if (verbose) puts("STUB: ZHSEQR_ called");
    return NULL;
}

void* ZLABRD(void)
{
    if (verbose) puts("STUB: ZLABRD called");
    return NULL;
}

void* ZLABRD_(void)
{
    if (verbose) puts("STUB: ZLABRD_ called");
    return NULL;
}

void* ZLACGV(void)
{
    if (verbose) puts("STUB: ZLACGV called");
    return NULL;
}

void* ZLACGV_(void)
{
    if (verbose) puts("STUB: ZLACGV_ called");
    return NULL;
}

void* ZLACN2(void)
{
    if (verbose) puts("STUB: ZLACN2 called");
    return NULL;
}

void* ZLACN2_(void)
{
    if (verbose) puts("STUB: ZLACN2_ called");
    return NULL;
}

void* ZLACON(void)
{
    if (verbose) puts("STUB: ZLACON called");
    return NULL;
}

void* ZLACON_(void)
{
    if (verbose) puts("STUB: ZLACON_ called");
    return NULL;
}

void* ZLACP2(void)
{
    if (verbose) puts("STUB: ZLACP2 called");
    return NULL;
}

void* ZLACP2_(void)
{
    if (verbose) puts("STUB: ZLACP2_ called");
    return NULL;
}

void* ZLACPY(void)
{
    if (verbose) puts("STUB: ZLACPY called");
    return NULL;
}

void* ZLACPY_(void)
{
    if (verbose) puts("STUB: ZLACPY_ called");
    return NULL;
}

void* ZLACRM(void)
{
    if (verbose) puts("STUB: ZLACRM called");
    return NULL;
}

void* ZLACRM_(void)
{
    if (verbose) puts("STUB: ZLACRM_ called");
    return NULL;
}

void* ZLACRT(void)
{
    if (verbose) puts("STUB: ZLACRT called");
    return NULL;
}

void* ZLACRT_(void)
{
    if (verbose) puts("STUB: ZLACRT_ called");
    return NULL;
}

void* ZLADIV(void)
{
    if (verbose) puts("STUB: ZLADIV called");
    return NULL;
}

void* ZLADIV_(void)
{
    if (verbose) puts("STUB: ZLADIV_ called");
    return NULL;
}

void* ZLAED0(void)
{
    if (verbose) puts("STUB: ZLAED0 called");
    return NULL;
}

void* ZLAED0_(void)
{
    if (verbose) puts("STUB: ZLAED0_ called");
    return NULL;
}

void* ZLAED7(void)
{
    if (verbose) puts("STUB: ZLAED7 called");
    return NULL;
}

void* ZLAED7_(void)
{
    if (verbose) puts("STUB: ZLAED7_ called");
    return NULL;
}

void* ZLAED8(void)
{
    if (verbose) puts("STUB: ZLAED8 called");
    return NULL;
}

void* ZLAED8_(void)
{
    if (verbose) puts("STUB: ZLAED8_ called");
    return NULL;
}

void* ZLAEIN(void)
{
    if (verbose) puts("STUB: ZLAEIN called");
    return NULL;
}

void* ZLAEIN_(void)
{
    if (verbose) puts("STUB: ZLAEIN_ called");
    return NULL;
}

void* ZLAESY(void)
{
    if (verbose) puts("STUB: ZLAESY called");
    return NULL;
}

void* ZLAESY_(void)
{
    if (verbose) puts("STUB: ZLAESY_ called");
    return NULL;
}

void* ZLAEV2(void)
{
    if (verbose) puts("STUB: ZLAEV2 called");
    return NULL;
}

void* ZLAEV2_(void)
{
    if (verbose) puts("STUB: ZLAEV2_ called");
    return NULL;
}

void* ZLAG2C(void)
{
    if (verbose) puts("STUB: ZLAG2C called");
    return NULL;
}

void* ZLAG2C_(void)
{
    if (verbose) puts("STUB: ZLAG2C_ called");
    return NULL;
}

void* ZLAGS2(void)
{
    if (verbose) puts("STUB: ZLAGS2 called");
    return NULL;
}

void* ZLAGS2_(void)
{
    if (verbose) puts("STUB: ZLAGS2_ called");
    return NULL;
}

void* ZLAGTM(void)
{
    if (verbose) puts("STUB: ZLAGTM called");
    return NULL;
}

void* ZLAGTM_(void)
{
    if (verbose) puts("STUB: ZLAGTM_ called");
    return NULL;
}

void* ZLAHEF(void)
{
    if (verbose) puts("STUB: ZLAHEF called");
    return NULL;
}

void* ZLAHEF_(void)
{
    if (verbose) puts("STUB: ZLAHEF_ called");
    return NULL;
}

void* ZLAHQR(void)
{
    if (verbose) puts("STUB: ZLAHQR called");
    return NULL;
}

void* ZLAHQR_(void)
{
    if (verbose) puts("STUB: ZLAHQR_ called");
    return NULL;
}

void* ZLAHR2(void)
{
    if (verbose) puts("STUB: ZLAHR2 called");
    return NULL;
}

void* ZLAHR2_(void)
{
    if (verbose) puts("STUB: ZLAHR2_ called");
    return NULL;
}

void* ZLAHRD(void)
{
    if (verbose) puts("STUB: ZLAHRD called");
    return NULL;
}

void* ZLAHRD_(void)
{
    if (verbose) puts("STUB: ZLAHRD_ called");
    return NULL;
}

void* ZLAIC1(void)
{
    if (verbose) puts("STUB: ZLAIC1 called");
    return NULL;
}

void* ZLAIC1_(void)
{
    if (verbose) puts("STUB: ZLAIC1_ called");
    return NULL;
}

void* ZLALS0(void)
{
    if (verbose) puts("STUB: ZLALS0 called");
    return NULL;
}

void* ZLALS0_(void)
{
    if (verbose) puts("STUB: ZLALS0_ called");
    return NULL;
}

void* ZLALSA(void)
{
    if (verbose) puts("STUB: ZLALSA called");
    return NULL;
}

void* ZLALSA_(void)
{
    if (verbose) puts("STUB: ZLALSA_ called");
    return NULL;
}

void* ZLALSD(void)
{
    if (verbose) puts("STUB: ZLALSD called");
    return NULL;
}

void* ZLALSD_(void)
{
    if (verbose) puts("STUB: ZLALSD_ called");
    return NULL;
}

void* ZLANGB(void)
{
    if (verbose) puts("STUB: ZLANGB called");
    return NULL;
}

void* ZLANGB_(void)
{
    if (verbose) puts("STUB: ZLANGB_ called");
    return NULL;
}

void* ZLANGE(void)
{
    if (verbose) puts("STUB: ZLANGE called");
    return NULL;
}

void* ZLANGE_(void)
{
    if (verbose) puts("STUB: ZLANGE_ called");
    return NULL;
}

void* ZLANGT(void)
{
    if (verbose) puts("STUB: ZLANGT called");
    return NULL;
}

void* ZLANGT_(void)
{
    if (verbose) puts("STUB: ZLANGT_ called");
    return NULL;
}

void* ZLANHB(void)
{
    if (verbose) puts("STUB: ZLANHB called");
    return NULL;
}

void* ZLANHB_(void)
{
    if (verbose) puts("STUB: ZLANHB_ called");
    return NULL;
}

void* ZLANHE(void)
{
    if (verbose) puts("STUB: ZLANHE called");
    return NULL;
}

void* ZLANHE_(void)
{
    if (verbose) puts("STUB: ZLANHE_ called");
    return NULL;
}

void* ZLANHF(void)
{
    if (verbose) puts("STUB: ZLANHF called");
    return NULL;
}

void* ZLANHF_(void)
{
    if (verbose) puts("STUB: ZLANHF_ called");
    return NULL;
}

void* ZLANHP(void)
{
    if (verbose) puts("STUB: ZLANHP called");
    return NULL;
}

void* ZLANHP_(void)
{
    if (verbose) puts("STUB: ZLANHP_ called");
    return NULL;
}

void* ZLANHS(void)
{
    if (verbose) puts("STUB: ZLANHS called");
    return NULL;
}

void* ZLANHS_(void)
{
    if (verbose) puts("STUB: ZLANHS_ called");
    return NULL;
}

void* ZLANHT(void)
{
    if (verbose) puts("STUB: ZLANHT called");
    return NULL;
}

void* ZLANHT_(void)
{
    if (verbose) puts("STUB: ZLANHT_ called");
    return NULL;
}

void* ZLANSB(void)
{
    if (verbose) puts("STUB: ZLANSB called");
    return NULL;
}

void* ZLANSB_(void)
{
    if (verbose) puts("STUB: ZLANSB_ called");
    return NULL;
}

void* ZLANSP(void)
{
    if (verbose) puts("STUB: ZLANSP called");
    return NULL;
}

void* ZLANSP_(void)
{
    if (verbose) puts("STUB: ZLANSP_ called");
    return NULL;
}

void* ZLANSY(void)
{
    if (verbose) puts("STUB: ZLANSY called");
    return NULL;
}

void* ZLANSY_(void)
{
    if (verbose) puts("STUB: ZLANSY_ called");
    return NULL;
}

void* ZLANTB(void)
{
    if (verbose) puts("STUB: ZLANTB called");
    return NULL;
}

void* ZLANTB_(void)
{
    if (verbose) puts("STUB: ZLANTB_ called");
    return NULL;
}

void* ZLANTP(void)
{
    if (verbose) puts("STUB: ZLANTP called");
    return NULL;
}

void* ZLANTP_(void)
{
    if (verbose) puts("STUB: ZLANTP_ called");
    return NULL;
}

void* ZLANTR(void)
{
    if (verbose) puts("STUB: ZLANTR called");
    return NULL;
}

void* ZLANTR_(void)
{
    if (verbose) puts("STUB: ZLANTR_ called");
    return NULL;
}

void* ZLAPLL(void)
{
    if (verbose) puts("STUB: ZLAPLL called");
    return NULL;
}

void* ZLAPLL_(void)
{
    if (verbose) puts("STUB: ZLAPLL_ called");
    return NULL;
}

void* ZLAPMT(void)
{
    if (verbose) puts("STUB: ZLAPMT called");
    return NULL;
}

void* ZLAPMT_(void)
{
    if (verbose) puts("STUB: ZLAPMT_ called");
    return NULL;
}

void* ZLAQGB(void)
{
    if (verbose) puts("STUB: ZLAQGB called");
    return NULL;
}

void* ZLAQGB_(void)
{
    if (verbose) puts("STUB: ZLAQGB_ called");
    return NULL;
}

void* ZLAQGE(void)
{
    if (verbose) puts("STUB: ZLAQGE called");
    return NULL;
}

void* ZLAQGE_(void)
{
    if (verbose) puts("STUB: ZLAQGE_ called");
    return NULL;
}

void* ZLAQHB(void)
{
    if (verbose) puts("STUB: ZLAQHB called");
    return NULL;
}

void* ZLAQHB_(void)
{
    if (verbose) puts("STUB: ZLAQHB_ called");
    return NULL;
}

void* ZLAQHE(void)
{
    if (verbose) puts("STUB: ZLAQHE called");
    return NULL;
}

void* ZLAQHE_(void)
{
    if (verbose) puts("STUB: ZLAQHE_ called");
    return NULL;
}

void* ZLAQHP(void)
{
    if (verbose) puts("STUB: ZLAQHP called");
    return NULL;
}

void* ZLAQHP_(void)
{
    if (verbose) puts("STUB: ZLAQHP_ called");
    return NULL;
}

void* ZLAQP2(void)
{
    if (verbose) puts("STUB: ZLAQP2 called");
    return NULL;
}

void* ZLAQP2_(void)
{
    if (verbose) puts("STUB: ZLAQP2_ called");
    return NULL;
}

void* ZLAQPS(void)
{
    if (verbose) puts("STUB: ZLAQPS called");
    return NULL;
}

void* ZLAQPS_(void)
{
    if (verbose) puts("STUB: ZLAQPS_ called");
    return NULL;
}

void* ZLAQR0(void)
{
    if (verbose) puts("STUB: ZLAQR0 called");
    return NULL;
}

void* ZLAQR0_(void)
{
    if (verbose) puts("STUB: ZLAQR0_ called");
    return NULL;
}

void* ZLAQR1(void)
{
    if (verbose) puts("STUB: ZLAQR1 called");
    return NULL;
}

void* ZLAQR1_(void)
{
    if (verbose) puts("STUB: ZLAQR1_ called");
    return NULL;
}

void* ZLAQR2(void)
{
    if (verbose) puts("STUB: ZLAQR2 called");
    return NULL;
}

void* ZLAQR2_(void)
{
    if (verbose) puts("STUB: ZLAQR2_ called");
    return NULL;
}

void* ZLAQR3(void)
{
    if (verbose) puts("STUB: ZLAQR3 called");
    return NULL;
}

void* ZLAQR3_(void)
{
    if (verbose) puts("STUB: ZLAQR3_ called");
    return NULL;
}

void* ZLAQR4(void)
{
    if (verbose) puts("STUB: ZLAQR4 called");
    return NULL;
}

void* ZLAQR4_(void)
{
    if (verbose) puts("STUB: ZLAQR4_ called");
    return NULL;
}

void* ZLAQR5(void)
{
    if (verbose) puts("STUB: ZLAQR5 called");
    return NULL;
}

void* ZLAQR5_(void)
{
    if (verbose) puts("STUB: ZLAQR5_ called");
    return NULL;
}

void* ZLAQSB(void)
{
    if (verbose) puts("STUB: ZLAQSB called");
    return NULL;
}

void* ZLAQSB_(void)
{
    if (verbose) puts("STUB: ZLAQSB_ called");
    return NULL;
}

void* ZLAQSP(void)
{
    if (verbose) puts("STUB: ZLAQSP called");
    return NULL;
}

void* ZLAQSP_(void)
{
    if (verbose) puts("STUB: ZLAQSP_ called");
    return NULL;
}

void* ZLAQSY(void)
{
    if (verbose) puts("STUB: ZLAQSY called");
    return NULL;
}

void* ZLAQSY_(void)
{
    if (verbose) puts("STUB: ZLAQSY_ called");
    return NULL;
}

void* ZLAR1V(void)
{
    if (verbose) puts("STUB: ZLAR1V called");
    return NULL;
}

void* ZLAR1V_(void)
{
    if (verbose) puts("STUB: ZLAR1V_ called");
    return NULL;
}

void* ZLAR2V(void)
{
    if (verbose) puts("STUB: ZLAR2V called");
    return NULL;
}

void* ZLAR2V_(void)
{
    if (verbose) puts("STUB: ZLAR2V_ called");
    return NULL;
}

void* ZLARCM(void)
{
    if (verbose) puts("STUB: ZLARCM called");
    return NULL;
}

void* ZLARCM_(void)
{
    if (verbose) puts("STUB: ZLARCM_ called");
    return NULL;
}

void* ZLARF(void)
{
    if (verbose) puts("STUB: ZLARF called");
    return NULL;
}

void* ZLARFB(void)
{
    if (verbose) puts("STUB: ZLARFB called");
    return NULL;
}

void* ZLARFB_(void)
{
    if (verbose) puts("STUB: ZLARFB_ called");
    return NULL;
}

void* ZLARFG(void)
{
    if (verbose) puts("STUB: ZLARFG called");
    return NULL;
}

void* ZLARFG_(void)
{
    if (verbose) puts("STUB: ZLARFG_ called");
    return NULL;
}

void* ZLARFP(void)
{
    if (verbose) puts("STUB: ZLARFP called");
    return NULL;
}

void* ZLARFP_(void)
{
    if (verbose) puts("STUB: ZLARFP_ called");
    return NULL;
}

void* ZLARFT(void)
{
    if (verbose) puts("STUB: ZLARFT called");
    return NULL;
}

void* ZLARFT_(void)
{
    if (verbose) puts("STUB: ZLARFT_ called");
    return NULL;
}

void* ZLARFX(void)
{
    if (verbose) puts("STUB: ZLARFX called");
    return NULL;
}

void* ZLARFX_(void)
{
    if (verbose) puts("STUB: ZLARFX_ called");
    return NULL;
}

void* ZLARF_(void)
{
    if (verbose) puts("STUB: ZLARF_ called");
    return NULL;
}

void* ZLARGV(void)
{
    if (verbose) puts("STUB: ZLARGV called");
    return NULL;
}

void* ZLARGV_(void)
{
    if (verbose) puts("STUB: ZLARGV_ called");
    return NULL;
}

void* ZLARNV(void)
{
    if (verbose) puts("STUB: ZLARNV called");
    return NULL;
}

void* ZLARNV_(void)
{
    if (verbose) puts("STUB: ZLARNV_ called");
    return NULL;
}

void* ZLARRV(void)
{
    if (verbose) puts("STUB: ZLARRV called");
    return NULL;
}

void* ZLARRV_(void)
{
    if (verbose) puts("STUB: ZLARRV_ called");
    return NULL;
}

void* ZLARSCL2(void)
{
    if (verbose) puts("STUB: ZLARSCL2 called");
    return NULL;
}

void* ZLARSCL2_(void)
{
    if (verbose) puts("STUB: ZLARSCL2_ called");
    return NULL;
}

void* ZLARTG(void)
{
    if (verbose) puts("STUB: ZLARTG called");
    return NULL;
}

void* ZLARTG_(void)
{
    if (verbose) puts("STUB: ZLARTG_ called");
    return NULL;
}

void* ZLARTV(void)
{
    if (verbose) puts("STUB: ZLARTV called");
    return NULL;
}

void* ZLARTV_(void)
{
    if (verbose) puts("STUB: ZLARTV_ called");
    return NULL;
}

void* ZLARZ(void)
{
    if (verbose) puts("STUB: ZLARZ called");
    return NULL;
}

void* ZLARZB(void)
{
    if (verbose) puts("STUB: ZLARZB called");
    return NULL;
}

void* ZLARZB_(void)
{
    if (verbose) puts("STUB: ZLARZB_ called");
    return NULL;
}

void* ZLARZT(void)
{
    if (verbose) puts("STUB: ZLARZT called");
    return NULL;
}

void* ZLARZT_(void)
{
    if (verbose) puts("STUB: ZLARZT_ called");
    return NULL;
}

void* ZLARZ_(void)
{
    if (verbose) puts("STUB: ZLARZ_ called");
    return NULL;
}

void* ZLASCL(void)
{
    if (verbose) puts("STUB: ZLASCL called");
    return NULL;
}

void* ZLASCL2(void)
{
    if (verbose) puts("STUB: ZLASCL2 called");
    return NULL;
}

void* ZLASCL2_(void)
{
    if (verbose) puts("STUB: ZLASCL2_ called");
    return NULL;
}

void* ZLASCL_(void)
{
    if (verbose) puts("STUB: ZLASCL_ called");
    return NULL;
}

void* ZLASET(void)
{
    if (verbose) puts("STUB: ZLASET called");
    return NULL;
}

void* ZLASET_(void)
{
    if (verbose) puts("STUB: ZLASET_ called");
    return NULL;
}

void* ZLASR(void)
{
    if (verbose) puts("STUB: ZLASR called");
    return NULL;
}

void* ZLASR_(void)
{
    if (verbose) puts("STUB: ZLASR_ called");
    return NULL;
}

void* ZLASSQ(void)
{
    if (verbose) puts("STUB: ZLASSQ called");
    return NULL;
}

void* ZLASSQ_(void)
{
    if (verbose) puts("STUB: ZLASSQ_ called");
    return NULL;
}

void* ZLASWP(void)
{
    if (verbose) puts("STUB: ZLASWP called");
    return NULL;
}

void* ZLASWP_(void)
{
    if (verbose) puts("STUB: ZLASWP_ called");
    return NULL;
}

void* ZLASYF(void)
{
    if (verbose) puts("STUB: ZLASYF called");
    return NULL;
}

void* ZLASYF_(void)
{
    if (verbose) puts("STUB: ZLASYF_ called");
    return NULL;
}

void* ZLAT2C(void)
{
    if (verbose) puts("STUB: ZLAT2C called");
    return NULL;
}

void* ZLAT2C_(void)
{
    if (verbose) puts("STUB: ZLAT2C_ called");
    return NULL;
}

void* ZLATBS(void)
{
    if (verbose) puts("STUB: ZLATBS called");
    return NULL;
}

void* ZLATBS_(void)
{
    if (verbose) puts("STUB: ZLATBS_ called");
    return NULL;
}

void* ZLATDF(void)
{
    if (verbose) puts("STUB: ZLATDF called");
    return NULL;
}

void* ZLATDF_(void)
{
    if (verbose) puts("STUB: ZLATDF_ called");
    return NULL;
}

void* ZLATPS(void)
{
    if (verbose) puts("STUB: ZLATPS called");
    return NULL;
}

void* ZLATPS_(void)
{
    if (verbose) puts("STUB: ZLATPS_ called");
    return NULL;
}

void* ZLATRD(void)
{
    if (verbose) puts("STUB: ZLATRD called");
    return NULL;
}

void* ZLATRD_(void)
{
    if (verbose) puts("STUB: ZLATRD_ called");
    return NULL;
}

void* ZLATRS(void)
{
    if (verbose) puts("STUB: ZLATRS called");
    return NULL;
}

void* ZLATRS_(void)
{
    if (verbose) puts("STUB: ZLATRS_ called");
    return NULL;
}

void* ZLATRZ(void)
{
    if (verbose) puts("STUB: ZLATRZ called");
    return NULL;
}

void* ZLATRZ_(void)
{
    if (verbose) puts("STUB: ZLATRZ_ called");
    return NULL;
}

void* ZLATZM(void)
{
    if (verbose) puts("STUB: ZLATZM called");
    return NULL;
}

void* ZLATZM_(void)
{
    if (verbose) puts("STUB: ZLATZM_ called");
    return NULL;
}

void* ZLAUU2(void)
{
    if (verbose) puts("STUB: ZLAUU2 called");
    return NULL;
}

void* ZLAUU2_(void)
{
    if (verbose) puts("STUB: ZLAUU2_ called");
    return NULL;
}

void* ZLAUUM(void)
{
    if (verbose) puts("STUB: ZLAUUM called");
    return NULL;
}

void* ZLAUUM_(void)
{
    if (verbose) puts("STUB: ZLAUUM_ called");
    return NULL;
}

void* ZPBCON(void)
{
    if (verbose) puts("STUB: ZPBCON called");
    return NULL;
}

void* ZPBCON_(void)
{
    if (verbose) puts("STUB: ZPBCON_ called");
    return NULL;
}

void* ZPBEQU(void)
{
    if (verbose) puts("STUB: ZPBEQU called");
    return NULL;
}

void* ZPBEQU_(void)
{
    if (verbose) puts("STUB: ZPBEQU_ called");
    return NULL;
}

void* ZPBRFS(void)
{
    if (verbose) puts("STUB: ZPBRFS called");
    return NULL;
}

void* ZPBRFS_(void)
{
    if (verbose) puts("STUB: ZPBRFS_ called");
    return NULL;
}

void* ZPBSTF(void)
{
    if (verbose) puts("STUB: ZPBSTF called");
    return NULL;
}

void* ZPBSTF_(void)
{
    if (verbose) puts("STUB: ZPBSTF_ called");
    return NULL;
}

void* ZPBSV(void)
{
    if (verbose) puts("STUB: ZPBSV called");
    return NULL;
}

void* ZPBSVX(void)
{
    if (verbose) puts("STUB: ZPBSVX called");
    return NULL;
}

void* ZPBSVX_(void)
{
    if (verbose) puts("STUB: ZPBSVX_ called");
    return NULL;
}

void* ZPBSV_(void)
{
    if (verbose) puts("STUB: ZPBSV_ called");
    return NULL;
}

void* ZPBTF2(void)
{
    if (verbose) puts("STUB: ZPBTF2 called");
    return NULL;
}

void* ZPBTF2_(void)
{
    if (verbose) puts("STUB: ZPBTF2_ called");
    return NULL;
}

void* ZPBTRF(void)
{
    if (verbose) puts("STUB: ZPBTRF called");
    return NULL;
}

void* ZPBTRF_(void)
{
    if (verbose) puts("STUB: ZPBTRF_ called");
    return NULL;
}

void* ZPBTRS(void)
{
    if (verbose) puts("STUB: ZPBTRS called");
    return NULL;
}

void* ZPBTRS_(void)
{
    if (verbose) puts("STUB: ZPBTRS_ called");
    return NULL;
}

void* ZPFTRF(void)
{
    if (verbose) puts("STUB: ZPFTRF called");
    return NULL;
}

void* ZPFTRF_(void)
{
    if (verbose) puts("STUB: ZPFTRF_ called");
    return NULL;
}

void* ZPFTRI(void)
{
    if (verbose) puts("STUB: ZPFTRI called");
    return NULL;
}

void* ZPFTRI_(void)
{
    if (verbose) puts("STUB: ZPFTRI_ called");
    return NULL;
}

void* ZPFTRS(void)
{
    if (verbose) puts("STUB: ZPFTRS called");
    return NULL;
}

void* ZPFTRS_(void)
{
    if (verbose) puts("STUB: ZPFTRS_ called");
    return NULL;
}

void* ZPOCON(void)
{
    if (verbose) puts("STUB: ZPOCON called");
    return NULL;
}

void* ZPOCON_(void)
{
    if (verbose) puts("STUB: ZPOCON_ called");
    return NULL;
}

void* ZPOEQU(void)
{
    if (verbose) puts("STUB: ZPOEQU called");
    return NULL;
}

void* ZPOEQUB(void)
{
    if (verbose) puts("STUB: ZPOEQUB called");
    return NULL;
}

void* ZPOEQUB_(void)
{
    if (verbose) puts("STUB: ZPOEQUB_ called");
    return NULL;
}

void* ZPOEQU_(void)
{
    if (verbose) puts("STUB: ZPOEQU_ called");
    return NULL;
}

void* ZPORFS(void)
{
    if (verbose) puts("STUB: ZPORFS called");
    return NULL;
}

void* ZPORFS_(void)
{
    if (verbose) puts("STUB: ZPORFS_ called");
    return NULL;
}

void* ZPOSV(void)
{
    if (verbose) puts("STUB: ZPOSV called");
    return NULL;
}

void* ZPOSVX(void)
{
    if (verbose) puts("STUB: ZPOSVX called");
    return NULL;
}

void* ZPOSVX_(void)
{
    if (verbose) puts("STUB: ZPOSVX_ called");
    return NULL;
}

void* ZPOSV_(void)
{
    if (verbose) puts("STUB: ZPOSV_ called");
    return NULL;
}

void* ZPOTF2(void)
{
    if (verbose) puts("STUB: ZPOTF2 called");
    return NULL;
}

void* ZPOTF2_(void)
{
    if (verbose) puts("STUB: ZPOTF2_ called");
    return NULL;
}

void* ZPOTRF(void)
{
    if (verbose) puts("STUB: ZPOTRF called");
    return NULL;
}

void* ZPOTRF_(void)
{
    if (verbose) puts("STUB: ZPOTRF_ called");
    return NULL;
}

void* ZPOTRI(void)
{
    if (verbose) puts("STUB: ZPOTRI called");
    return NULL;
}

void* ZPOTRI_(void)
{
    if (verbose) puts("STUB: ZPOTRI_ called");
    return NULL;
}

void* ZPOTRS(void)
{
    if (verbose) puts("STUB: ZPOTRS called");
    return NULL;
}

void* ZPOTRS_(void)
{
    if (verbose) puts("STUB: ZPOTRS_ called");
    return NULL;
}

void* ZPPCON(void)
{
    if (verbose) puts("STUB: ZPPCON called");
    return NULL;
}

void* ZPPCON_(void)
{
    if (verbose) puts("STUB: ZPPCON_ called");
    return NULL;
}

void* ZPPEQU(void)
{
    if (verbose) puts("STUB: ZPPEQU called");
    return NULL;
}

void* ZPPEQU_(void)
{
    if (verbose) puts("STUB: ZPPEQU_ called");
    return NULL;
}

void* ZPPRFS(void)
{
    if (verbose) puts("STUB: ZPPRFS called");
    return NULL;
}

void* ZPPRFS_(void)
{
    if (verbose) puts("STUB: ZPPRFS_ called");
    return NULL;
}

void* ZPPSV(void)
{
    if (verbose) puts("STUB: ZPPSV called");
    return NULL;
}

void* ZPPSVX(void)
{
    if (verbose) puts("STUB: ZPPSVX called");
    return NULL;
}

void* ZPPSVX_(void)
{
    if (verbose) puts("STUB: ZPPSVX_ called");
    return NULL;
}

void* ZPPSV_(void)
{
    if (verbose) puts("STUB: ZPPSV_ called");
    return NULL;
}

void* ZPPTRF(void)
{
    if (verbose) puts("STUB: ZPPTRF called");
    return NULL;
}

void* ZPPTRF_(void)
{
    if (verbose) puts("STUB: ZPPTRF_ called");
    return NULL;
}

void* ZPPTRI(void)
{
    if (verbose) puts("STUB: ZPPTRI called");
    return NULL;
}

void* ZPPTRI_(void)
{
    if (verbose) puts("STUB: ZPPTRI_ called");
    return NULL;
}

void* ZPPTRS(void)
{
    if (verbose) puts("STUB: ZPPTRS called");
    return NULL;
}

void* ZPPTRS_(void)
{
    if (verbose) puts("STUB: ZPPTRS_ called");
    return NULL;
}

void* ZPSTF2(void)
{
    if (verbose) puts("STUB: ZPSTF2 called");
    return NULL;
}

void* ZPSTF2_(void)
{
    if (verbose) puts("STUB: ZPSTF2_ called");
    return NULL;
}

void* ZPSTRF(void)
{
    if (verbose) puts("STUB: ZPSTRF called");
    return NULL;
}

void* ZPSTRF_(void)
{
    if (verbose) puts("STUB: ZPSTRF_ called");
    return NULL;
}

void* ZPTCON(void)
{
    if (verbose) puts("STUB: ZPTCON called");
    return NULL;
}

void* ZPTCON_(void)
{
    if (verbose) puts("STUB: ZPTCON_ called");
    return NULL;
}

void* ZPTEQR(void)
{
    if (verbose) puts("STUB: ZPTEQR called");
    return NULL;
}

void* ZPTEQR_(void)
{
    if (verbose) puts("STUB: ZPTEQR_ called");
    return NULL;
}

void* ZPTRFS(void)
{
    if (verbose) puts("STUB: ZPTRFS called");
    return NULL;
}

void* ZPTRFS_(void)
{
    if (verbose) puts("STUB: ZPTRFS_ called");
    return NULL;
}

void* ZPTSV(void)
{
    if (verbose) puts("STUB: ZPTSV called");
    return NULL;
}

void* ZPTSVX(void)
{
    if (verbose) puts("STUB: ZPTSVX called");
    return NULL;
}

void* ZPTSVX_(void)
{
    if (verbose) puts("STUB: ZPTSVX_ called");
    return NULL;
}

void* ZPTSV_(void)
{
    if (verbose) puts("STUB: ZPTSV_ called");
    return NULL;
}

void* ZPTTRF(void)
{
    if (verbose) puts("STUB: ZPTTRF called");
    return NULL;
}

void* ZPTTRF_(void)
{
    if (verbose) puts("STUB: ZPTTRF_ called");
    return NULL;
}

void* ZPTTRS(void)
{
    if (verbose) puts("STUB: ZPTTRS called");
    return NULL;
}

void* ZPTTRS_(void)
{
    if (verbose) puts("STUB: ZPTTRS_ called");
    return NULL;
}

void* ZPTTS2(void)
{
    if (verbose) puts("STUB: ZPTTS2 called");
    return NULL;
}

void* ZPTTS2_(void)
{
    if (verbose) puts("STUB: ZPTTS2_ called");
    return NULL;
}

void* ZROT(void)
{
    if (verbose) puts("STUB: ZROT called");
    return NULL;
}

void* ZROT_(void)
{
    if (verbose) puts("STUB: ZROT_ called");
    return NULL;
}

void* ZSPCON(void)
{
    if (verbose) puts("STUB: ZSPCON called");
    return NULL;
}

void* ZSPCON_(void)
{
    if (verbose) puts("STUB: ZSPCON_ called");
    return NULL;
}

void* ZSPMV(void)
{
    if (verbose) puts("STUB: ZSPMV called");
    return NULL;
}

void* ZSPMV_(void)
{
    if (verbose) puts("STUB: ZSPMV_ called");
    return NULL;
}

void* ZSPR(void)
{
    if (verbose) puts("STUB: ZSPR called");
    return NULL;
}

void* ZSPRFS(void)
{
    if (verbose) puts("STUB: ZSPRFS called");
    return NULL;
}

void* ZSPRFS_(void)
{
    if (verbose) puts("STUB: ZSPRFS_ called");
    return NULL;
}

void* ZSPR_(void)
{
    if (verbose) puts("STUB: ZSPR_ called");
    return NULL;
}

void* ZSPSV(void)
{
    if (verbose) puts("STUB: ZSPSV called");
    return NULL;
}

void* ZSPSVX(void)
{
    if (verbose) puts("STUB: ZSPSVX called");
    return NULL;
}

void* ZSPSVX_(void)
{
    if (verbose) puts("STUB: ZSPSVX_ called");
    return NULL;
}

void* ZSPSV_(void)
{
    if (verbose) puts("STUB: ZSPSV_ called");
    return NULL;
}

void* ZSPTRF(void)
{
    if (verbose) puts("STUB: ZSPTRF called");
    return NULL;
}

void* ZSPTRF_(void)
{
    if (verbose) puts("STUB: ZSPTRF_ called");
    return NULL;
}

void* ZSPTRI(void)
{
    if (verbose) puts("STUB: ZSPTRI called");
    return NULL;
}

void* ZSPTRI_(void)
{
    if (verbose) puts("STUB: ZSPTRI_ called");
    return NULL;
}

void* ZSPTRS(void)
{
    if (verbose) puts("STUB: ZSPTRS called");
    return NULL;
}

void* ZSPTRS_(void)
{
    if (verbose) puts("STUB: ZSPTRS_ called");
    return NULL;
}

void* ZSTEDC(void)
{
    if (verbose) puts("STUB: ZSTEDC called");
    return NULL;
}

void* ZSTEDC_(void)
{
    if (verbose) puts("STUB: ZSTEDC_ called");
    return NULL;
}

void* ZSTEGR(void)
{
    if (verbose) puts("STUB: ZSTEGR called");
    return NULL;
}

void* ZSTEGR_(void)
{
    if (verbose) puts("STUB: ZSTEGR_ called");
    return NULL;
}

void* ZSTEIN(void)
{
    if (verbose) puts("STUB: ZSTEIN called");
    return NULL;
}

void* ZSTEIN_(void)
{
    if (verbose) puts("STUB: ZSTEIN_ called");
    return NULL;
}

void* ZSTEMR(void)
{
    if (verbose) puts("STUB: ZSTEMR called");
    return NULL;
}

void* ZSTEMR_(void)
{
    if (verbose) puts("STUB: ZSTEMR_ called");
    return NULL;
}

void* ZSTEQR(void)
{
    if (verbose) puts("STUB: ZSTEQR called");
    return NULL;
}

void* ZSTEQR_(void)
{
    if (verbose) puts("STUB: ZSTEQR_ called");
    return NULL;
}

void* ZSYCON(void)
{
    if (verbose) puts("STUB: ZSYCON called");
    return NULL;
}

void* ZSYCON_(void)
{
    if (verbose) puts("STUB: ZSYCON_ called");
    return NULL;
}

void* ZSYEQUB(void)
{
    if (verbose) puts("STUB: ZSYEQUB called");
    return NULL;
}

void* ZSYEQUB_(void)
{
    if (verbose) puts("STUB: ZSYEQUB_ called");
    return NULL;
}

void* ZSYMV(void)
{
    if (verbose) puts("STUB: ZSYMV called");
    return NULL;
}

void* ZSYMV_(void)
{
    if (verbose) puts("STUB: ZSYMV_ called");
    return NULL;
}

void* ZSYR(void)
{
    if (verbose) puts("STUB: ZSYR called");
    return NULL;
}

void* ZSYRFS(void)
{
    if (verbose) puts("STUB: ZSYRFS called");
    return NULL;
}

void* ZSYRFS_(void)
{
    if (verbose) puts("STUB: ZSYRFS_ called");
    return NULL;
}

void* ZSYR_(void)
{
    if (verbose) puts("STUB: ZSYR_ called");
    return NULL;
}

void* ZSYSV(void)
{
    if (verbose) puts("STUB: ZSYSV called");
    return NULL;
}

void* ZSYSVX(void)
{
    if (verbose) puts("STUB: ZSYSVX called");
    return NULL;
}

void* ZSYSVX_(void)
{
    if (verbose) puts("STUB: ZSYSVX_ called");
    return NULL;
}

void* ZSYSV_(void)
{
    if (verbose) puts("STUB: ZSYSV_ called");
    return NULL;
}

void* ZSYTF2(void)
{
    if (verbose) puts("STUB: ZSYTF2 called");
    return NULL;
}

void* ZSYTF2_(void)
{
    if (verbose) puts("STUB: ZSYTF2_ called");
    return NULL;
}

void* ZSYTRF(void)
{
    if (verbose) puts("STUB: ZSYTRF called");
    return NULL;
}

void* ZSYTRF_(void)
{
    if (verbose) puts("STUB: ZSYTRF_ called");
    return NULL;
}

void* ZSYTRI(void)
{
    if (verbose) puts("STUB: ZSYTRI called");
    return NULL;
}

void* ZSYTRI_(void)
{
    if (verbose) puts("STUB: ZSYTRI_ called");
    return NULL;
}

void* ZSYTRS(void)
{
    if (verbose) puts("STUB: ZSYTRS called");
    return NULL;
}

void* ZSYTRS_(void)
{
    if (verbose) puts("STUB: ZSYTRS_ called");
    return NULL;
}

void* ZTBCON(void)
{
    if (verbose) puts("STUB: ZTBCON called");
    return NULL;
}

void* ZTBCON_(void)
{
    if (verbose) puts("STUB: ZTBCON_ called");
    return NULL;
}

void* ZTBRFS(void)
{
    if (verbose) puts("STUB: ZTBRFS called");
    return NULL;
}

void* ZTBRFS_(void)
{
    if (verbose) puts("STUB: ZTBRFS_ called");
    return NULL;
}

void* ZTBTRS(void)
{
    if (verbose) puts("STUB: ZTBTRS called");
    return NULL;
}

void* ZTBTRS_(void)
{
    if (verbose) puts("STUB: ZTBTRS_ called");
    return NULL;
}

void* ZTFSM(void)
{
    if (verbose) puts("STUB: ZTFSM called");
    return NULL;
}

void* ZTFSM_(void)
{
    if (verbose) puts("STUB: ZTFSM_ called");
    return NULL;
}

void* ZTFTRI(void)
{
    if (verbose) puts("STUB: ZTFTRI called");
    return NULL;
}

void* ZTFTRI_(void)
{
    if (verbose) puts("STUB: ZTFTRI_ called");
    return NULL;
}

void* ZTFTTP(void)
{
    if (verbose) puts("STUB: ZTFTTP called");
    return NULL;
}

void* ZTFTTP_(void)
{
    if (verbose) puts("STUB: ZTFTTP_ called");
    return NULL;
}

void* ZTFTTR(void)
{
    if (verbose) puts("STUB: ZTFTTR called");
    return NULL;
}

void* ZTFTTR_(void)
{
    if (verbose) puts("STUB: ZTFTTR_ called");
    return NULL;
}

void* ZTGEVC(void)
{
    if (verbose) puts("STUB: ZTGEVC called");
    return NULL;
}

void* ZTGEVC_(void)
{
    if (verbose) puts("STUB: ZTGEVC_ called");
    return NULL;
}

void* ZTGEX2(void)
{
    if (verbose) puts("STUB: ZTGEX2 called");
    return NULL;
}

void* ZTGEX2_(void)
{
    if (verbose) puts("STUB: ZTGEX2_ called");
    return NULL;
}

void* ZTGEXC(void)
{
    if (verbose) puts("STUB: ZTGEXC called");
    return NULL;
}

void* ZTGEXC_(void)
{
    if (verbose) puts("STUB: ZTGEXC_ called");
    return NULL;
}

void* ZTGSEN(void)
{
    if (verbose) puts("STUB: ZTGSEN called");
    return NULL;
}

void* ZTGSEN_(void)
{
    if (verbose) puts("STUB: ZTGSEN_ called");
    return NULL;
}

void* ZTGSJA(void)
{
    if (verbose) puts("STUB: ZTGSJA called");
    return NULL;
}

void* ZTGSJA_(void)
{
    if (verbose) puts("STUB: ZTGSJA_ called");
    return NULL;
}

void* ZTGSNA(void)
{
    if (verbose) puts("STUB: ZTGSNA called");
    return NULL;
}

void* ZTGSNA_(void)
{
    if (verbose) puts("STUB: ZTGSNA_ called");
    return NULL;
}

void* ZTGSY2(void)
{
    if (verbose) puts("STUB: ZTGSY2 called");
    return NULL;
}

void* ZTGSY2_(void)
{
    if (verbose) puts("STUB: ZTGSY2_ called");
    return NULL;
}

void* ZTGSYL(void)
{
    if (verbose) puts("STUB: ZTGSYL called");
    return NULL;
}

void* ZTGSYL_(void)
{
    if (verbose) puts("STUB: ZTGSYL_ called");
    return NULL;
}

void* ZTPCON(void)
{
    if (verbose) puts("STUB: ZTPCON called");
    return NULL;
}

void* ZTPCON_(void)
{
    if (verbose) puts("STUB: ZTPCON_ called");
    return NULL;
}

void* ZTPRFS(void)
{
    if (verbose) puts("STUB: ZTPRFS called");
    return NULL;
}

void* ZTPRFS_(void)
{
    if (verbose) puts("STUB: ZTPRFS_ called");
    return NULL;
}

void* ZTPTRI(void)
{
    if (verbose) puts("STUB: ZTPTRI called");
    return NULL;
}

void* ZTPTRI_(void)
{
    if (verbose) puts("STUB: ZTPTRI_ called");
    return NULL;
}

void* ZTPTRS(void)
{
    if (verbose) puts("STUB: ZTPTRS called");
    return NULL;
}

void* ZTPTRS_(void)
{
    if (verbose) puts("STUB: ZTPTRS_ called");
    return NULL;
}

void* ZTPTTF(void)
{
    if (verbose) puts("STUB: ZTPTTF called");
    return NULL;
}

void* ZTPTTF_(void)
{
    if (verbose) puts("STUB: ZTPTTF_ called");
    return NULL;
}

void* ZTPTTR(void)
{
    if (verbose) puts("STUB: ZTPTTR called");
    return NULL;
}

void* ZTPTTR_(void)
{
    if (verbose) puts("STUB: ZTPTTR_ called");
    return NULL;
}

void* ZTRCON(void)
{
    if (verbose) puts("STUB: ZTRCON called");
    return NULL;
}

void* ZTRCON_(void)
{
    if (verbose) puts("STUB: ZTRCON_ called");
    return NULL;
}

void* ZTREVC(void)
{
    if (verbose) puts("STUB: ZTREVC called");
    return NULL;
}

void* ZTREVC_(void)
{
    if (verbose) puts("STUB: ZTREVC_ called");
    return NULL;
}

void* ZTREXC(void)
{
    if (verbose) puts("STUB: ZTREXC called");
    return NULL;
}

void* ZTREXC_(void)
{
    if (verbose) puts("STUB: ZTREXC_ called");
    return NULL;
}

void* ZTRRFS(void)
{
    if (verbose) puts("STUB: ZTRRFS called");
    return NULL;
}

void* ZTRRFS_(void)
{
    if (verbose) puts("STUB: ZTRRFS_ called");
    return NULL;
}

void* ZTRSEN(void)
{
    if (verbose) puts("STUB: ZTRSEN called");
    return NULL;
}

void* ZTRSEN_(void)
{
    if (verbose) puts("STUB: ZTRSEN_ called");
    return NULL;
}

void* ZTRSNA(void)
{
    if (verbose) puts("STUB: ZTRSNA called");
    return NULL;
}

void* ZTRSNA_(void)
{
    if (verbose) puts("STUB: ZTRSNA_ called");
    return NULL;
}

void* ZTRSYL(void)
{
    if (verbose) puts("STUB: ZTRSYL called");
    return NULL;
}

void* ZTRSYL_(void)
{
    if (verbose) puts("STUB: ZTRSYL_ called");
    return NULL;
}

void* ZTRTI2(void)
{
    if (verbose) puts("STUB: ZTRTI2 called");
    return NULL;
}

void* ZTRTI2_(void)
{
    if (verbose) puts("STUB: ZTRTI2_ called");
    return NULL;
}

void* ZTRTRI(void)
{
    if (verbose) puts("STUB: ZTRTRI called");
    return NULL;
}

void* ZTRTRI_(void)
{
    if (verbose) puts("STUB: ZTRTRI_ called");
    return NULL;
}

void* ZTRTRS(void)
{
    if (verbose) puts("STUB: ZTRTRS called");
    return NULL;
}

void* ZTRTRS_(void)
{
    if (verbose) puts("STUB: ZTRTRS_ called");
    return NULL;
}

void* ZTRTTF(void)
{
    if (verbose) puts("STUB: ZTRTTF called");
    return NULL;
}

void* ZTRTTF_(void)
{
    if (verbose) puts("STUB: ZTRTTF_ called");
    return NULL;
}

void* ZTRTTP(void)
{
    if (verbose) puts("STUB: ZTRTTP called");
    return NULL;
}

void* ZTRTTP_(void)
{
    if (verbose) puts("STUB: ZTRTTP_ called");
    return NULL;
}

void* ZTZRQF(void)
{
    if (verbose) puts("STUB: ZTZRQF called");
    return NULL;
}

void* ZTZRQF_(void)
{
    if (verbose) puts("STUB: ZTZRQF_ called");
    return NULL;
}

void* ZTZRZF(void)
{
    if (verbose) puts("STUB: ZTZRZF called");
    return NULL;
}

void* ZTZRZF_(void)
{
    if (verbose) puts("STUB: ZTZRZF_ called");
    return NULL;
}

void* ZUNG2L(void)
{
    if (verbose) puts("STUB: ZUNG2L called");
    return NULL;
}

void* ZUNG2L_(void)
{
    if (verbose) puts("STUB: ZUNG2L_ called");
    return NULL;
}

void* ZUNG2R(void)
{
    if (verbose) puts("STUB: ZUNG2R called");
    return NULL;
}

void* ZUNG2R_(void)
{
    if (verbose) puts("STUB: ZUNG2R_ called");
    return NULL;
}

void* ZUNGBR(void)
{
    if (verbose) puts("STUB: ZUNGBR called");
    return NULL;
}

void* ZUNGBR_(void)
{
    if (verbose) puts("STUB: ZUNGBR_ called");
    return NULL;
}

void* ZUNGHR(void)
{
    if (verbose) puts("STUB: ZUNGHR called");
    return NULL;
}

void* ZUNGHR_(void)
{
    if (verbose) puts("STUB: ZUNGHR_ called");
    return NULL;
}

void* ZUNGL2(void)
{
    if (verbose) puts("STUB: ZUNGL2 called");
    return NULL;
}

void* ZUNGL2_(void)
{
    if (verbose) puts("STUB: ZUNGL2_ called");
    return NULL;
}

void* ZUNGLQ(void)
{
    if (verbose) puts("STUB: ZUNGLQ called");
    return NULL;
}

void* ZUNGLQ_(void)
{
    if (verbose) puts("STUB: ZUNGLQ_ called");
    return NULL;
}

void* ZUNGQL(void)
{
    if (verbose) puts("STUB: ZUNGQL called");
    return NULL;
}

void* ZUNGQL_(void)
{
    if (verbose) puts("STUB: ZUNGQL_ called");
    return NULL;
}

void* ZUNGQR(void)
{
    if (verbose) puts("STUB: ZUNGQR called");
    return NULL;
}

void* ZUNGQR_(void)
{
    if (verbose) puts("STUB: ZUNGQR_ called");
    return NULL;
}

void* ZUNGR2(void)
{
    if (verbose) puts("STUB: ZUNGR2 called");
    return NULL;
}

void* ZUNGR2_(void)
{
    if (verbose) puts("STUB: ZUNGR2_ called");
    return NULL;
}

void* ZUNGRQ(void)
{
    if (verbose) puts("STUB: ZUNGRQ called");
    return NULL;
}

void* ZUNGRQ_(void)
{
    if (verbose) puts("STUB: ZUNGRQ_ called");
    return NULL;
}

void* ZUNGTR(void)
{
    if (verbose) puts("STUB: ZUNGTR called");
    return NULL;
}

void* ZUNGTR_(void)
{
    if (verbose) puts("STUB: ZUNGTR_ called");
    return NULL;
}

void* ZUNM2L(void)
{
    if (verbose) puts("STUB: ZUNM2L called");
    return NULL;
}

void* ZUNM2L_(void)
{
    if (verbose) puts("STUB: ZUNM2L_ called");
    return NULL;
}

void* ZUNM2R(void)
{
    if (verbose) puts("STUB: ZUNM2R called");
    return NULL;
}

void* ZUNM2R_(void)
{
    if (verbose) puts("STUB: ZUNM2R_ called");
    return NULL;
}

void* ZUNMBR(void)
{
    if (verbose) puts("STUB: ZUNMBR called");
    return NULL;
}

void* ZUNMBR_(void)
{
    if (verbose) puts("STUB: ZUNMBR_ called");
    return NULL;
}

void* ZUNMHR(void)
{
    if (verbose) puts("STUB: ZUNMHR called");
    return NULL;
}

void* ZUNMHR_(void)
{
    if (verbose) puts("STUB: ZUNMHR_ called");
    return NULL;
}

void* ZUNML2(void)
{
    if (verbose) puts("STUB: ZUNML2 called");
    return NULL;
}

void* ZUNML2_(void)
{
    if (verbose) puts("STUB: ZUNML2_ called");
    return NULL;
}

void* ZUNMLQ(void)
{
    if (verbose) puts("STUB: ZUNMLQ called");
    return NULL;
}

void* ZUNMLQ_(void)
{
    if (verbose) puts("STUB: ZUNMLQ_ called");
    return NULL;
}

void* ZUNMQL(void)
{
    if (verbose) puts("STUB: ZUNMQL called");
    return NULL;
}

void* ZUNMQL_(void)
{
    if (verbose) puts("STUB: ZUNMQL_ called");
    return NULL;
}

void* ZUNMQR(void)
{
    if (verbose) puts("STUB: ZUNMQR called");
    return NULL;
}

void* ZUNMQR_(void)
{
    if (verbose) puts("STUB: ZUNMQR_ called");
    return NULL;
}

void* ZUNMR2(void)
{
    if (verbose) puts("STUB: ZUNMR2 called");
    return NULL;
}

void* ZUNMR2_(void)
{
    if (verbose) puts("STUB: ZUNMR2_ called");
    return NULL;
}

void* ZUNMR3(void)
{
    if (verbose) puts("STUB: ZUNMR3 called");
    return NULL;
}

void* ZUNMR3_(void)
{
    if (verbose) puts("STUB: ZUNMR3_ called");
    return NULL;
}

void* ZUNMRQ(void)
{
    if (verbose) puts("STUB: ZUNMRQ called");
    return NULL;
}

void* ZUNMRQ_(void)
{
    if (verbose) puts("STUB: ZUNMRQ_ called");
    return NULL;
}

void* ZUNMRZ(void)
{
    if (verbose) puts("STUB: ZUNMRZ called");
    return NULL;
}

void* ZUNMRZ_(void)
{
    if (verbose) puts("STUB: ZUNMRZ_ called");
    return NULL;
}

void* ZUNMTR(void)
{
    if (verbose) puts("STUB: ZUNMTR called");
    return NULL;
}

void* ZUNMTR_(void)
{
    if (verbose) puts("STUB: ZUNMTR_ called");
    return NULL;
}

void* ZUPGTR(void)
{
    if (verbose) puts("STUB: ZUPGTR called");
    return NULL;
}

void* ZUPGTR_(void)
{
    if (verbose) puts("STUB: ZUPGTR_ called");
    return NULL;
}

void* ZUPMTR(void)
{
    if (verbose) puts("STUB: ZUPMTR called");
    return NULL;
}

void* ZUPMTR_(void)
{
    if (verbose) puts("STUB: ZUPMTR_ called");
    return NULL;
}

void* cbdsqr(void)
{
    if (verbose) puts("STUB: cbdsqr called");
    return NULL;
}

void* cbdsqr_(void)
{
    if (verbose) puts("STUB: cbdsqr_ called");
    return NULL;
}

void* cgbbrd(void)
{
    if (verbose) puts("STUB: cgbbrd called");
    return NULL;
}

void* cgbbrd_(void)
{
    if (verbose) puts("STUB: cgbbrd_ called");
    return NULL;
}

void* cgbcon(void)
{
    if (verbose) puts("STUB: cgbcon called");
    return NULL;
}

void* cgbcon_(void)
{
    if (verbose) puts("STUB: cgbcon_ called");
    return NULL;
}

void* cgbequ(void)
{
    if (verbose) puts("STUB: cgbequ called");
    return NULL;
}

void* cgbequ_(void)
{
    if (verbose) puts("STUB: cgbequ_ called");
    return NULL;
}

void* cgbequb(void)
{
    if (verbose) puts("STUB: cgbequb called");
    return NULL;
}

void* cgbequb_(void)
{
    if (verbose) puts("STUB: cgbequb_ called");
    return NULL;
}

void* cgbrfs(void)
{
    if (verbose) puts("STUB: cgbrfs called");
    return NULL;
}

void* cgbrfs_(void)
{
    if (verbose) puts("STUB: cgbrfs_ called");
    return NULL;
}

void* cgbsv(void)
{
    if (verbose) puts("STUB: cgbsv called");
    return NULL;
}

void* cgbsv_(void)
{
    if (verbose) puts("STUB: cgbsv_ called");
    return NULL;
}

void* cgbsvx(void)
{
    if (verbose) puts("STUB: cgbsvx called");
    return NULL;
}

void* cgbsvx_(void)
{
    if (verbose) puts("STUB: cgbsvx_ called");
    return NULL;
}

void* cgbtf2(void)
{
    if (verbose) puts("STUB: cgbtf2 called");
    return NULL;
}

void* cgbtf2_(void)
{
    if (verbose) puts("STUB: cgbtf2_ called");
    return NULL;
}

void* cgbtrf(void)
{
    if (verbose) puts("STUB: cgbtrf called");
    return NULL;
}

void* cgbtrf_(void)
{
    if (verbose) puts("STUB: cgbtrf_ called");
    return NULL;
}

void* cgbtrs(void)
{
    if (verbose) puts("STUB: cgbtrs called");
    return NULL;
}

void* cgbtrs_(void)
{
    if (verbose) puts("STUB: cgbtrs_ called");
    return NULL;
}

void* cgebak(void)
{
    if (verbose) puts("STUB: cgebak called");
    return NULL;
}

void* cgebak_(void)
{
    if (verbose) puts("STUB: cgebak_ called");
    return NULL;
}

void* cgebal(void)
{
    if (verbose) puts("STUB: cgebal called");
    return NULL;
}

void* cgebal_(void)
{
    if (verbose) puts("STUB: cgebal_ called");
    return NULL;
}

void* cgebd2(void)
{
    if (verbose) puts("STUB: cgebd2 called");
    return NULL;
}

void* cgebd2_(void)
{
    if (verbose) puts("STUB: cgebd2_ called");
    return NULL;
}

void* cgebrd(void)
{
    if (verbose) puts("STUB: cgebrd called");
    return NULL;
}

void* cgebrd_(void)
{
    if (verbose) puts("STUB: cgebrd_ called");
    return NULL;
}

void* cgecon(void)
{
    if (verbose) puts("STUB: cgecon called");
    return NULL;
}

void* cgecon_(void)
{
    if (verbose) puts("STUB: cgecon_ called");
    return NULL;
}

void* cgeequ(void)
{
    if (verbose) puts("STUB: cgeequ called");
    return NULL;
}

void* cgeequ_(void)
{
    if (verbose) puts("STUB: cgeequ_ called");
    return NULL;
}

void* cgeequb(void)
{
    if (verbose) puts("STUB: cgeequb called");
    return NULL;
}

void* cgeequb_(void)
{
    if (verbose) puts("STUB: cgeequb_ called");
    return NULL;
}

void* cgees(void)
{
    if (verbose) puts("STUB: cgees called");
    return NULL;
}

void* cgees_(void)
{
    if (verbose) puts("STUB: cgees_ called");
    return NULL;
}

void* cgeesx(void)
{
    if (verbose) puts("STUB: cgeesx called");
    return NULL;
}

void* cgeesx_(void)
{
    if (verbose) puts("STUB: cgeesx_ called");
    return NULL;
}

void* cgeev(void)
{
    if (verbose) puts("STUB: cgeev called");
    return NULL;
}

void* cgeev_(void)
{
    if (verbose) puts("STUB: cgeev_ called");
    return NULL;
}

void* cgeevx(void)
{
    if (verbose) puts("STUB: cgeevx called");
    return NULL;
}

void* cgeevx_(void)
{
    if (verbose) puts("STUB: cgeevx_ called");
    return NULL;
}

void* cgegs(void)
{
    if (verbose) puts("STUB: cgegs called");
    return NULL;
}

void* cgegs_(void)
{
    if (verbose) puts("STUB: cgegs_ called");
    return NULL;
}

void* cgegv(void)
{
    if (verbose) puts("STUB: cgegv called");
    return NULL;
}

void* cgegv_(void)
{
    if (verbose) puts("STUB: cgegv_ called");
    return NULL;
}

void* cgehd2(void)
{
    if (verbose) puts("STUB: cgehd2 called");
    return NULL;
}

void* cgehd2_(void)
{
    if (verbose) puts("STUB: cgehd2_ called");
    return NULL;
}

void* cgehrd(void)
{
    if (verbose) puts("STUB: cgehrd called");
    return NULL;
}

void* cgehrd_(void)
{
    if (verbose) puts("STUB: cgehrd_ called");
    return NULL;
}

void* cgelq2(void)
{
    if (verbose) puts("STUB: cgelq2 called");
    return NULL;
}

void* cgelq2_(void)
{
    if (verbose) puts("STUB: cgelq2_ called");
    return NULL;
}

void* cgelqf(void)
{
    if (verbose) puts("STUB: cgelqf called");
    return NULL;
}

void* cgelqf_(void)
{
    if (verbose) puts("STUB: cgelqf_ called");
    return NULL;
}

void* cgels(void)
{
    if (verbose) puts("STUB: cgels called");
    return NULL;
}

void* cgels_(void)
{
    if (verbose) puts("STUB: cgels_ called");
    return NULL;
}

void* cgelsd(void)
{
    if (verbose) puts("STUB: cgelsd called");
    return NULL;
}

void* cgelsd_(void)
{
    if (verbose) puts("STUB: cgelsd_ called");
    return NULL;
}

void* cgelss(void)
{
    if (verbose) puts("STUB: cgelss called");
    return NULL;
}

void* cgelss_(void)
{
    if (verbose) puts("STUB: cgelss_ called");
    return NULL;
}

void* cgelsx(void)
{
    if (verbose) puts("STUB: cgelsx called");
    return NULL;
}

void* cgelsx_(void)
{
    if (verbose) puts("STUB: cgelsx_ called");
    return NULL;
}

void* cgelsy(void)
{
    if (verbose) puts("STUB: cgelsy called");
    return NULL;
}

void* cgelsy_(void)
{
    if (verbose) puts("STUB: cgelsy_ called");
    return NULL;
}

void* cgeql2(void)
{
    if (verbose) puts("STUB: cgeql2 called");
    return NULL;
}

void* cgeql2_(void)
{
    if (verbose) puts("STUB: cgeql2_ called");
    return NULL;
}

void* cgeqlf(void)
{
    if (verbose) puts("STUB: cgeqlf called");
    return NULL;
}

void* cgeqlf_(void)
{
    if (verbose) puts("STUB: cgeqlf_ called");
    return NULL;
}

void* cgeqp3(void)
{
    if (verbose) puts("STUB: cgeqp3 called");
    return NULL;
}

void* cgeqp3_(void)
{
    if (verbose) puts("STUB: cgeqp3_ called");
    return NULL;
}

void* cgeqpf(void)
{
    if (verbose) puts("STUB: cgeqpf called");
    return NULL;
}

void* cgeqpf_(void)
{
    if (verbose) puts("STUB: cgeqpf_ called");
    return NULL;
}

void* cgeqr2(void)
{
    if (verbose) puts("STUB: cgeqr2 called");
    return NULL;
}

void* cgeqr2_(void)
{
    if (verbose) puts("STUB: cgeqr2_ called");
    return NULL;
}

void* cgeqrf(void)
{
    if (verbose) puts("STUB: cgeqrf called");
    return NULL;
}

void* cgeqrf_(void)
{
    if (verbose) puts("STUB: cgeqrf_ called");
    return NULL;
}

void* cgerfs(void)
{
    if (verbose) puts("STUB: cgerfs called");
    return NULL;
}

void* cgerfs_(void)
{
    if (verbose) puts("STUB: cgerfs_ called");
    return NULL;
}

void* cgerq2(void)
{
    if (verbose) puts("STUB: cgerq2 called");
    return NULL;
}

void* cgerq2_(void)
{
    if (verbose) puts("STUB: cgerq2_ called");
    return NULL;
}

void* cgerqf(void)
{
    if (verbose) puts("STUB: cgerqf called");
    return NULL;
}

void* cgerqf_(void)
{
    if (verbose) puts("STUB: cgerqf_ called");
    return NULL;
}

void* cgesc2(void)
{
    if (verbose) puts("STUB: cgesc2 called");
    return NULL;
}

void* cgesc2_(void)
{
    if (verbose) puts("STUB: cgesc2_ called");
    return NULL;
}

void* cgesdd(void)
{
    if (verbose) puts("STUB: cgesdd called");
    return NULL;
}

void* cgesdd_(void)
{
    if (verbose) puts("STUB: cgesdd_ called");
    return NULL;
}

void* cgesv(void)
{
    if (verbose) puts("STUB: cgesv called");
    return NULL;
}

void* cgesv_(void)
{
    if (verbose) puts("STUB: cgesv_ called");
    return NULL;
}

void* cgesvd(void)
{
    if (verbose) puts("STUB: cgesvd called");
    return NULL;
}

void* cgesvd_(void)
{
    if (verbose) puts("STUB: cgesvd_ called");
    return NULL;
}

void* cgesvx(void)
{
    if (verbose) puts("STUB: cgesvx called");
    return NULL;
}

void* cgesvx_(void)
{
    if (verbose) puts("STUB: cgesvx_ called");
    return NULL;
}

void* cgetc2(void)
{
    if (verbose) puts("STUB: cgetc2 called");
    return NULL;
}

void* cgetc2_(void)
{
    if (verbose) puts("STUB: cgetc2_ called");
    return NULL;
}

void* cgetf2(void)
{
    if (verbose) puts("STUB: cgetf2 called");
    return NULL;
}

void* cgetf2_(void)
{
    if (verbose) puts("STUB: cgetf2_ called");
    return NULL;
}

void* cgetrf(void)
{
    if (verbose) puts("STUB: cgetrf called");
    return NULL;
}

void* cgetrf_(void)
{
    if (verbose) puts("STUB: cgetrf_ called");
    return NULL;
}

void* cgetri(void)
{
    if (verbose) puts("STUB: cgetri called");
    return NULL;
}

void* cgetri_(void)
{
    if (verbose) puts("STUB: cgetri_ called");
    return NULL;
}

void* cgetrs(void)
{
    if (verbose) puts("STUB: cgetrs called");
    return NULL;
}

void* cgetrs_(void)
{
    if (verbose) puts("STUB: cgetrs_ called");
    return NULL;
}

void* cggbak(void)
{
    if (verbose) puts("STUB: cggbak called");
    return NULL;
}

void* cggbak_(void)
{
    if (verbose) puts("STUB: cggbak_ called");
    return NULL;
}

void* cggbal(void)
{
    if (verbose) puts("STUB: cggbal called");
    return NULL;
}

void* cggbal_(void)
{
    if (verbose) puts("STUB: cggbal_ called");
    return NULL;
}

void* cgges(void)
{
    if (verbose) puts("STUB: cgges called");
    return NULL;
}

void* cgges_(void)
{
    if (verbose) puts("STUB: cgges_ called");
    return NULL;
}

void* cggesx(void)
{
    if (verbose) puts("STUB: cggesx called");
    return NULL;
}

void* cggesx_(void)
{
    if (verbose) puts("STUB: cggesx_ called");
    return NULL;
}

void* cggev(void)
{
    if (verbose) puts("STUB: cggev called");
    return NULL;
}

void* cggev_(void)
{
    if (verbose) puts("STUB: cggev_ called");
    return NULL;
}

void* cggevx(void)
{
    if (verbose) puts("STUB: cggevx called");
    return NULL;
}

void* cggevx_(void)
{
    if (verbose) puts("STUB: cggevx_ called");
    return NULL;
}

void* cggglm(void)
{
    if (verbose) puts("STUB: cggglm called");
    return NULL;
}

void* cggglm_(void)
{
    if (verbose) puts("STUB: cggglm_ called");
    return NULL;
}

void* cgghrd(void)
{
    if (verbose) puts("STUB: cgghrd called");
    return NULL;
}

void* cgghrd_(void)
{
    if (verbose) puts("STUB: cgghrd_ called");
    return NULL;
}

void* cgglse(void)
{
    if (verbose) puts("STUB: cgglse called");
    return NULL;
}

void* cgglse_(void)
{
    if (verbose) puts("STUB: cgglse_ called");
    return NULL;
}

void* cggqrf(void)
{
    if (verbose) puts("STUB: cggqrf called");
    return NULL;
}

void* cggqrf_(void)
{
    if (verbose) puts("STUB: cggqrf_ called");
    return NULL;
}

void* cggrqf(void)
{
    if (verbose) puts("STUB: cggrqf called");
    return NULL;
}

void* cggrqf_(void)
{
    if (verbose) puts("STUB: cggrqf_ called");
    return NULL;
}

void* cggsvd(void)
{
    if (verbose) puts("STUB: cggsvd called");
    return NULL;
}

void* cggsvd_(void)
{
    if (verbose) puts("STUB: cggsvd_ called");
    return NULL;
}

void* cggsvp(void)
{
    if (verbose) puts("STUB: cggsvp called");
    return NULL;
}

void* cggsvp_(void)
{
    if (verbose) puts("STUB: cggsvp_ called");
    return NULL;
}

void* cgtcon(void)
{
    if (verbose) puts("STUB: cgtcon called");
    return NULL;
}

void* cgtcon_(void)
{
    if (verbose) puts("STUB: cgtcon_ called");
    return NULL;
}

void* cgtrfs(void)
{
    if (verbose) puts("STUB: cgtrfs called");
    return NULL;
}

void* cgtrfs_(void)
{
    if (verbose) puts("STUB: cgtrfs_ called");
    return NULL;
}

void* cgtsv(void)
{
    if (verbose) puts("STUB: cgtsv called");
    return NULL;
}

void* cgtsv_(void)
{
    if (verbose) puts("STUB: cgtsv_ called");
    return NULL;
}

void* cgtsvx(void)
{
    if (verbose) puts("STUB: cgtsvx called");
    return NULL;
}

void* cgtsvx_(void)
{
    if (verbose) puts("STUB: cgtsvx_ called");
    return NULL;
}

void* cgttrf(void)
{
    if (verbose) puts("STUB: cgttrf called");
    return NULL;
}

void* cgttrf_(void)
{
    if (verbose) puts("STUB: cgttrf_ called");
    return NULL;
}

void* cgttrs(void)
{
    if (verbose) puts("STUB: cgttrs called");
    return NULL;
}

void* cgttrs_(void)
{
    if (verbose) puts("STUB: cgttrs_ called");
    return NULL;
}

void* cgtts2(void)
{
    if (verbose) puts("STUB: cgtts2 called");
    return NULL;
}

void* cgtts2_(void)
{
    if (verbose) puts("STUB: cgtts2_ called");
    return NULL;
}

void* chbev(void)
{
    if (verbose) puts("STUB: chbev called");
    return NULL;
}

void* chbev_(void)
{
    if (verbose) puts("STUB: chbev_ called");
    return NULL;
}

void* chbevd(void)
{
    if (verbose) puts("STUB: chbevd called");
    return NULL;
}

void* chbevd_(void)
{
    if (verbose) puts("STUB: chbevd_ called");
    return NULL;
}

void* chbevx(void)
{
    if (verbose) puts("STUB: chbevx called");
    return NULL;
}

void* chbevx_(void)
{
    if (verbose) puts("STUB: chbevx_ called");
    return NULL;
}

void* chbgst(void)
{
    if (verbose) puts("STUB: chbgst called");
    return NULL;
}

void* chbgst_(void)
{
    if (verbose) puts("STUB: chbgst_ called");
    return NULL;
}

void* chbgv(void)
{
    if (verbose) puts("STUB: chbgv called");
    return NULL;
}

void* chbgv_(void)
{
    if (verbose) puts("STUB: chbgv_ called");
    return NULL;
}

void* chbgvd(void)
{
    if (verbose) puts("STUB: chbgvd called");
    return NULL;
}

void* chbgvd_(void)
{
    if (verbose) puts("STUB: chbgvd_ called");
    return NULL;
}

void* chbgvx(void)
{
    if (verbose) puts("STUB: chbgvx called");
    return NULL;
}

void* chbgvx_(void)
{
    if (verbose) puts("STUB: chbgvx_ called");
    return NULL;
}

void* chbtrd(void)
{
    if (verbose) puts("STUB: chbtrd called");
    return NULL;
}

void* chbtrd_(void)
{
    if (verbose) puts("STUB: chbtrd_ called");
    return NULL;
}

void* checon(void)
{
    if (verbose) puts("STUB: checon called");
    return NULL;
}

void* checon_(void)
{
    if (verbose) puts("STUB: checon_ called");
    return NULL;
}

void* cheequb(void)
{
    if (verbose) puts("STUB: cheequb called");
    return NULL;
}

void* cheequb_(void)
{
    if (verbose) puts("STUB: cheequb_ called");
    return NULL;
}

void* cheev(void)
{
    if (verbose) puts("STUB: cheev called");
    return NULL;
}

void* cheev_(void)
{
    if (verbose) puts("STUB: cheev_ called");
    return NULL;
}

void* cheevd(void)
{
    if (verbose) puts("STUB: cheevd called");
    return NULL;
}

void* cheevd_(void)
{
    if (verbose) puts("STUB: cheevd_ called");
    return NULL;
}

void* cheevr(void)
{
    if (verbose) puts("STUB: cheevr called");
    return NULL;
}

void* cheevr_(void)
{
    if (verbose) puts("STUB: cheevr_ called");
    return NULL;
}

void* cheevx(void)
{
    if (verbose) puts("STUB: cheevx called");
    return NULL;
}

void* cheevx_(void)
{
    if (verbose) puts("STUB: cheevx_ called");
    return NULL;
}

void* chegs2(void)
{
    if (verbose) puts("STUB: chegs2 called");
    return NULL;
}

void* chegs2_(void)
{
    if (verbose) puts("STUB: chegs2_ called");
    return NULL;
}

void* chegst(void)
{
    if (verbose) puts("STUB: chegst called");
    return NULL;
}

void* chegst_(void)
{
    if (verbose) puts("STUB: chegst_ called");
    return NULL;
}

void* chegv(void)
{
    if (verbose) puts("STUB: chegv called");
    return NULL;
}

void* chegv_(void)
{
    if (verbose) puts("STUB: chegv_ called");
    return NULL;
}

void* chegvd(void)
{
    if (verbose) puts("STUB: chegvd called");
    return NULL;
}

void* chegvd_(void)
{
    if (verbose) puts("STUB: chegvd_ called");
    return NULL;
}

void* chegvx(void)
{
    if (verbose) puts("STUB: chegvx called");
    return NULL;
}

void* chegvx_(void)
{
    if (verbose) puts("STUB: chegvx_ called");
    return NULL;
}

void* cherfs(void)
{
    if (verbose) puts("STUB: cherfs called");
    return NULL;
}

void* cherfs_(void)
{
    if (verbose) puts("STUB: cherfs_ called");
    return NULL;
}

void* chesv(void)
{
    if (verbose) puts("STUB: chesv called");
    return NULL;
}

void* chesv_(void)
{
    if (verbose) puts("STUB: chesv_ called");
    return NULL;
}

void* chesvx(void)
{
    if (verbose) puts("STUB: chesvx called");
    return NULL;
}

void* chesvx_(void)
{
    if (verbose) puts("STUB: chesvx_ called");
    return NULL;
}

void* chetd2(void)
{
    if (verbose) puts("STUB: chetd2 called");
    return NULL;
}

void* chetd2_(void)
{
    if (verbose) puts("STUB: chetd2_ called");
    return NULL;
}

void* chetf2(void)
{
    if (verbose) puts("STUB: chetf2 called");
    return NULL;
}

void* chetf2_(void)
{
    if (verbose) puts("STUB: chetf2_ called");
    return NULL;
}

void* chetrd(void)
{
    if (verbose) puts("STUB: chetrd called");
    return NULL;
}

void* chetrd_(void)
{
    if (verbose) puts("STUB: chetrd_ called");
    return NULL;
}

void* chetrf(void)
{
    if (verbose) puts("STUB: chetrf called");
    return NULL;
}

void* chetrf_(void)
{
    if (verbose) puts("STUB: chetrf_ called");
    return NULL;
}

void* chetri(void)
{
    if (verbose) puts("STUB: chetri called");
    return NULL;
}

void* chetri_(void)
{
    if (verbose) puts("STUB: chetri_ called");
    return NULL;
}

void* chetrs(void)
{
    if (verbose) puts("STUB: chetrs called");
    return NULL;
}

void* chetrs_(void)
{
    if (verbose) puts("STUB: chetrs_ called");
    return NULL;
}

void* chfrk(void)
{
    if (verbose) puts("STUB: chfrk called");
    return NULL;
}

void* chfrk_(void)
{
    if (verbose) puts("STUB: chfrk_ called");
    return NULL;
}

void* chgeqz(void)
{
    if (verbose) puts("STUB: chgeqz called");
    return NULL;
}

void* chgeqz_(void)
{
    if (verbose) puts("STUB: chgeqz_ called");
    return NULL;
}

void* chla_transtype__(void)
{
    if (verbose) puts("STUB: chla_transtype__ called");
    return NULL;
}

void* chpcon(void)
{
    if (verbose) puts("STUB: chpcon called");
    return NULL;
}

void* chpcon_(void)
{
    if (verbose) puts("STUB: chpcon_ called");
    return NULL;
}

void* chpev(void)
{
    if (verbose) puts("STUB: chpev called");
    return NULL;
}

void* chpev_(void)
{
    if (verbose) puts("STUB: chpev_ called");
    return NULL;
}

void* chpevd(void)
{
    if (verbose) puts("STUB: chpevd called");
    return NULL;
}

void* chpevd_(void)
{
    if (verbose) puts("STUB: chpevd_ called");
    return NULL;
}

void* chpevx(void)
{
    if (verbose) puts("STUB: chpevx called");
    return NULL;
}

void* chpevx_(void)
{
    if (verbose) puts("STUB: chpevx_ called");
    return NULL;
}

void* chpgst(void)
{
    if (verbose) puts("STUB: chpgst called");
    return NULL;
}

void* chpgst_(void)
{
    if (verbose) puts("STUB: chpgst_ called");
    return NULL;
}

void* chpgv(void)
{
    if (verbose) puts("STUB: chpgv called");
    return NULL;
}

void* chpgv_(void)
{
    if (verbose) puts("STUB: chpgv_ called");
    return NULL;
}

void* chpgvd(void)
{
    if (verbose) puts("STUB: chpgvd called");
    return NULL;
}

void* chpgvd_(void)
{
    if (verbose) puts("STUB: chpgvd_ called");
    return NULL;
}

void* chpgvx(void)
{
    if (verbose) puts("STUB: chpgvx called");
    return NULL;
}

void* chpgvx_(void)
{
    if (verbose) puts("STUB: chpgvx_ called");
    return NULL;
}

void* chprfs(void)
{
    if (verbose) puts("STUB: chprfs called");
    return NULL;
}

void* chprfs_(void)
{
    if (verbose) puts("STUB: chprfs_ called");
    return NULL;
}

void* chpsv(void)
{
    if (verbose) puts("STUB: chpsv called");
    return NULL;
}

void* chpsv_(void)
{
    if (verbose) puts("STUB: chpsv_ called");
    return NULL;
}

void* chpsvx(void)
{
    if (verbose) puts("STUB: chpsvx called");
    return NULL;
}

void* chpsvx_(void)
{
    if (verbose) puts("STUB: chpsvx_ called");
    return NULL;
}

void* chptrd(void)
{
    if (verbose) puts("STUB: chptrd called");
    return NULL;
}

void* chptrd_(void)
{
    if (verbose) puts("STUB: chptrd_ called");
    return NULL;
}

void* chptrf(void)
{
    if (verbose) puts("STUB: chptrf called");
    return NULL;
}

void* chptrf_(void)
{
    if (verbose) puts("STUB: chptrf_ called");
    return NULL;
}

void* chptri(void)
{
    if (verbose) puts("STUB: chptri called");
    return NULL;
}

void* chptri_(void)
{
    if (verbose) puts("STUB: chptri_ called");
    return NULL;
}

void* chptrs(void)
{
    if (verbose) puts("STUB: chptrs called");
    return NULL;
}

void* chptrs_(void)
{
    if (verbose) puts("STUB: chptrs_ called");
    return NULL;
}

void* chsein(void)
{
    if (verbose) puts("STUB: chsein called");
    return NULL;
}

void* chsein_(void)
{
    if (verbose) puts("STUB: chsein_ called");
    return NULL;
}

void* chseqr(void)
{
    if (verbose) puts("STUB: chseqr called");
    return NULL;
}

void* chseqr_(void)
{
    if (verbose) puts("STUB: chseqr_ called");
    return NULL;
}

void* clabrd(void)
{
    if (verbose) puts("STUB: clabrd called");
    return NULL;
}

void* clabrd_(void)
{
    if (verbose) puts("STUB: clabrd_ called");
    return NULL;
}

void* clacgv(void)
{
    if (verbose) puts("STUB: clacgv called");
    return NULL;
}

void* clacgv_(void)
{
    if (verbose) puts("STUB: clacgv_ called");
    return NULL;
}

void* clacn2(void)
{
    if (verbose) puts("STUB: clacn2 called");
    return NULL;
}

void* clacn2_(void)
{
    if (verbose) puts("STUB: clacn2_ called");
    return NULL;
}

void* clacon(void)
{
    if (verbose) puts("STUB: clacon called");
    return NULL;
}

void* clacon_(void)
{
    if (verbose) puts("STUB: clacon_ called");
    return NULL;
}

void* clacp2(void)
{
    if (verbose) puts("STUB: clacp2 called");
    return NULL;
}

void* clacp2_(void)
{
    if (verbose) puts("STUB: clacp2_ called");
    return NULL;
}

void* clacpy(void)
{
    if (verbose) puts("STUB: clacpy called");
    return NULL;
}

void* clacpy_(void)
{
    if (verbose) puts("STUB: clacpy_ called");
    return NULL;
}

void* clacrm(void)
{
    if (verbose) puts("STUB: clacrm called");
    return NULL;
}

void* clacrm_(void)
{
    if (verbose) puts("STUB: clacrm_ called");
    return NULL;
}

void* clacrt(void)
{
    if (verbose) puts("STUB: clacrt called");
    return NULL;
}

void* clacrt_(void)
{
    if (verbose) puts("STUB: clacrt_ called");
    return NULL;
}

void* cladiv(void)
{
    if (verbose) puts("STUB: cladiv called");
    return NULL;
}

void* cladiv_(void)
{
    if (verbose) puts("STUB: cladiv_ called");
    return NULL;
}

void* claed0(void)
{
    if (verbose) puts("STUB: claed0 called");
    return NULL;
}

void* claed0_(void)
{
    if (verbose) puts("STUB: claed0_ called");
    return NULL;
}

void* claed7(void)
{
    if (verbose) puts("STUB: claed7 called");
    return NULL;
}

void* claed7_(void)
{
    if (verbose) puts("STUB: claed7_ called");
    return NULL;
}

void* claed8(void)
{
    if (verbose) puts("STUB: claed8 called");
    return NULL;
}

void* claed8_(void)
{
    if (verbose) puts("STUB: claed8_ called");
    return NULL;
}

void* claein(void)
{
    if (verbose) puts("STUB: claein called");
    return NULL;
}

void* claein_(void)
{
    if (verbose) puts("STUB: claein_ called");
    return NULL;
}

void* claesy(void)
{
    if (verbose) puts("STUB: claesy called");
    return NULL;
}

void* claesy_(void)
{
    if (verbose) puts("STUB: claesy_ called");
    return NULL;
}

void* claev2(void)
{
    if (verbose) puts("STUB: claev2 called");
    return NULL;
}

void* claev2_(void)
{
    if (verbose) puts("STUB: claev2_ called");
    return NULL;
}

void* clag2z(void)
{
    if (verbose) puts("STUB: clag2z called");
    return NULL;
}

void* clag2z_(void)
{
    if (verbose) puts("STUB: clag2z_ called");
    return NULL;
}

void* clags2(void)
{
    if (verbose) puts("STUB: clags2 called");
    return NULL;
}

void* clags2_(void)
{
    if (verbose) puts("STUB: clags2_ called");
    return NULL;
}

void* clagtm(void)
{
    if (verbose) puts("STUB: clagtm called");
    return NULL;
}

void* clagtm_(void)
{
    if (verbose) puts("STUB: clagtm_ called");
    return NULL;
}

void* clahef(void)
{
    if (verbose) puts("STUB: clahef called");
    return NULL;
}

void* clahef_(void)
{
    if (verbose) puts("STUB: clahef_ called");
    return NULL;
}

void* clahqr(void)
{
    if (verbose) puts("STUB: clahqr called");
    return NULL;
}

void* clahqr_(void)
{
    if (verbose) puts("STUB: clahqr_ called");
    return NULL;
}

void* clahr2(void)
{
    if (verbose) puts("STUB: clahr2 called");
    return NULL;
}

void* clahr2_(void)
{
    if (verbose) puts("STUB: clahr2_ called");
    return NULL;
}

void* clahrd(void)
{
    if (verbose) puts("STUB: clahrd called");
    return NULL;
}

void* clahrd_(void)
{
    if (verbose) puts("STUB: clahrd_ called");
    return NULL;
}

void* claic1(void)
{
    if (verbose) puts("STUB: claic1 called");
    return NULL;
}

void* claic1_(void)
{
    if (verbose) puts("STUB: claic1_ called");
    return NULL;
}

void* clals0(void)
{
    if (verbose) puts("STUB: clals0 called");
    return NULL;
}

void* clals0_(void)
{
    if (verbose) puts("STUB: clals0_ called");
    return NULL;
}

void* clalsa(void)
{
    if (verbose) puts("STUB: clalsa called");
    return NULL;
}

void* clalsa_(void)
{
    if (verbose) puts("STUB: clalsa_ called");
    return NULL;
}

void* clalsd(void)
{
    if (verbose) puts("STUB: clalsd called");
    return NULL;
}

void* clalsd_(void)
{
    if (verbose) puts("STUB: clalsd_ called");
    return NULL;
}

void* clangb(void)
{
    if (verbose) puts("STUB: clangb called");
    return NULL;
}

void* clangb_(void)
{
    if (verbose) puts("STUB: clangb_ called");
    return NULL;
}

void* clange(void)
{
    if (verbose) puts("STUB: clange called");
    return NULL;
}

void* clange_(void)
{
    if (verbose) puts("STUB: clange_ called");
    return NULL;
}

void* clangt(void)
{
    if (verbose) puts("STUB: clangt called");
    return NULL;
}

void* clangt_(void)
{
    if (verbose) puts("STUB: clangt_ called");
    return NULL;
}

void* clanhb(void)
{
    if (verbose) puts("STUB: clanhb called");
    return NULL;
}

void* clanhb_(void)
{
    if (verbose) puts("STUB: clanhb_ called");
    return NULL;
}

void* clanhe(void)
{
    if (verbose) puts("STUB: clanhe called");
    return NULL;
}

void* clanhe_(void)
{
    if (verbose) puts("STUB: clanhe_ called");
    return NULL;
}

void* clanhf(void)
{
    if (verbose) puts("STUB: clanhf called");
    return NULL;
}

void* clanhf_(void)
{
    if (verbose) puts("STUB: clanhf_ called");
    return NULL;
}

void* clanhp(void)
{
    if (verbose) puts("STUB: clanhp called");
    return NULL;
}

void* clanhp_(void)
{
    if (verbose) puts("STUB: clanhp_ called");
    return NULL;
}

void* clanhs(void)
{
    if (verbose) puts("STUB: clanhs called");
    return NULL;
}

void* clanhs_(void)
{
    if (verbose) puts("STUB: clanhs_ called");
    return NULL;
}

void* clanht(void)
{
    if (verbose) puts("STUB: clanht called");
    return NULL;
}

void* clanht_(void)
{
    if (verbose) puts("STUB: clanht_ called");
    return NULL;
}

void* clansb(void)
{
    if (verbose) puts("STUB: clansb called");
    return NULL;
}

void* clansb_(void)
{
    if (verbose) puts("STUB: clansb_ called");
    return NULL;
}

void* clansp(void)
{
    if (verbose) puts("STUB: clansp called");
    return NULL;
}

void* clansp_(void)
{
    if (verbose) puts("STUB: clansp_ called");
    return NULL;
}

void* clansy(void)
{
    if (verbose) puts("STUB: clansy called");
    return NULL;
}

void* clansy_(void)
{
    if (verbose) puts("STUB: clansy_ called");
    return NULL;
}

void* clantb(void)
{
    if (verbose) puts("STUB: clantb called");
    return NULL;
}

void* clantb_(void)
{
    if (verbose) puts("STUB: clantb_ called");
    return NULL;
}

void* clantp(void)
{
    if (verbose) puts("STUB: clantp called");
    return NULL;
}

void* clantp_(void)
{
    if (verbose) puts("STUB: clantp_ called");
    return NULL;
}

void* clantr(void)
{
    if (verbose) puts("STUB: clantr called");
    return NULL;
}

void* clantr_(void)
{
    if (verbose) puts("STUB: clantr_ called");
    return NULL;
}

void* clapll(void)
{
    if (verbose) puts("STUB: clapll called");
    return NULL;
}

void* clapll_(void)
{
    if (verbose) puts("STUB: clapll_ called");
    return NULL;
}

void* clapmt(void)
{
    if (verbose) puts("STUB: clapmt called");
    return NULL;
}

void* clapmt_(void)
{
    if (verbose) puts("STUB: clapmt_ called");
    return NULL;
}

void* claqgb(void)
{
    if (verbose) puts("STUB: claqgb called");
    return NULL;
}

void* claqgb_(void)
{
    if (verbose) puts("STUB: claqgb_ called");
    return NULL;
}

void* claqge(void)
{
    if (verbose) puts("STUB: claqge called");
    return NULL;
}

void* claqge_(void)
{
    if (verbose) puts("STUB: claqge_ called");
    return NULL;
}

void* claqhb(void)
{
    if (verbose) puts("STUB: claqhb called");
    return NULL;
}

void* claqhb_(void)
{
    if (verbose) puts("STUB: claqhb_ called");
    return NULL;
}

void* claqhe(void)
{
    if (verbose) puts("STUB: claqhe called");
    return NULL;
}

void* claqhe_(void)
{
    if (verbose) puts("STUB: claqhe_ called");
    return NULL;
}

void* claqhp(void)
{
    if (verbose) puts("STUB: claqhp called");
    return NULL;
}

void* claqhp_(void)
{
    if (verbose) puts("STUB: claqhp_ called");
    return NULL;
}

void* claqp2(void)
{
    if (verbose) puts("STUB: claqp2 called");
    return NULL;
}

void* claqp2_(void)
{
    if (verbose) puts("STUB: claqp2_ called");
    return NULL;
}

void* claqps(void)
{
    if (verbose) puts("STUB: claqps called");
    return NULL;
}

void* claqps_(void)
{
    if (verbose) puts("STUB: claqps_ called");
    return NULL;
}

void* claqr0(void)
{
    if (verbose) puts("STUB: claqr0 called");
    return NULL;
}

void* claqr0_(void)
{
    if (verbose) puts("STUB: claqr0_ called");
    return NULL;
}

void* claqr1(void)
{
    if (verbose) puts("STUB: claqr1 called");
    return NULL;
}

void* claqr1_(void)
{
    if (verbose) puts("STUB: claqr1_ called");
    return NULL;
}

void* claqr2(void)
{
    if (verbose) puts("STUB: claqr2 called");
    return NULL;
}

void* claqr2_(void)
{
    if (verbose) puts("STUB: claqr2_ called");
    return NULL;
}

void* claqr3(void)
{
    if (verbose) puts("STUB: claqr3 called");
    return NULL;
}

void* claqr3_(void)
{
    if (verbose) puts("STUB: claqr3_ called");
    return NULL;
}

void* claqr4(void)
{
    if (verbose) puts("STUB: claqr4 called");
    return NULL;
}

void* claqr4_(void)
{
    if (verbose) puts("STUB: claqr4_ called");
    return NULL;
}

void* claqr5(void)
{
    if (verbose) puts("STUB: claqr5 called");
    return NULL;
}

void* claqr5_(void)
{
    if (verbose) puts("STUB: claqr5_ called");
    return NULL;
}

void* claqsb(void)
{
    if (verbose) puts("STUB: claqsb called");
    return NULL;
}

void* claqsb_(void)
{
    if (verbose) puts("STUB: claqsb_ called");
    return NULL;
}

void* claqsp(void)
{
    if (verbose) puts("STUB: claqsp called");
    return NULL;
}

void* claqsp_(void)
{
    if (verbose) puts("STUB: claqsp_ called");
    return NULL;
}

void* claqsy(void)
{
    if (verbose) puts("STUB: claqsy called");
    return NULL;
}

void* claqsy_(void)
{
    if (verbose) puts("STUB: claqsy_ called");
    return NULL;
}

void* clar1v(void)
{
    if (verbose) puts("STUB: clar1v called");
    return NULL;
}

void* clar1v_(void)
{
    if (verbose) puts("STUB: clar1v_ called");
    return NULL;
}

void* clar2v(void)
{
    if (verbose) puts("STUB: clar2v called");
    return NULL;
}

void* clar2v_(void)
{
    if (verbose) puts("STUB: clar2v_ called");
    return NULL;
}

void* clarcm(void)
{
    if (verbose) puts("STUB: clarcm called");
    return NULL;
}

void* clarcm_(void)
{
    if (verbose) puts("STUB: clarcm_ called");
    return NULL;
}

void* clarf(void)
{
    if (verbose) puts("STUB: clarf called");
    return NULL;
}

void* clarf_(void)
{
    if (verbose) puts("STUB: clarf_ called");
    return NULL;
}

void* clarfb(void)
{
    if (verbose) puts("STUB: clarfb called");
    return NULL;
}

void* clarfb_(void)
{
    if (verbose) puts("STUB: clarfb_ called");
    return NULL;
}

void* clarfg(void)
{
    if (verbose) puts("STUB: clarfg called");
    return NULL;
}

void* clarfg_(void)
{
    if (verbose) puts("STUB: clarfg_ called");
    return NULL;
}

void* clarfp(void)
{
    if (verbose) puts("STUB: clarfp called");
    return NULL;
}

void* clarfp_(void)
{
    if (verbose) puts("STUB: clarfp_ called");
    return NULL;
}

void* clarft(void)
{
    if (verbose) puts("STUB: clarft called");
    return NULL;
}

void* clarft_(void)
{
    if (verbose) puts("STUB: clarft_ called");
    return NULL;
}

void* clarfx(void)
{
    if (verbose) puts("STUB: clarfx called");
    return NULL;
}

void* clarfx_(void)
{
    if (verbose) puts("STUB: clarfx_ called");
    return NULL;
}

void* clargv(void)
{
    if (verbose) puts("STUB: clargv called");
    return NULL;
}

void* clargv_(void)
{
    if (verbose) puts("STUB: clargv_ called");
    return NULL;
}

void* clarnv(void)
{
    if (verbose) puts("STUB: clarnv called");
    return NULL;
}

void* clarnv_(void)
{
    if (verbose) puts("STUB: clarnv_ called");
    return NULL;
}

void* clarrv(void)
{
    if (verbose) puts("STUB: clarrv called");
    return NULL;
}

void* clarrv_(void)
{
    if (verbose) puts("STUB: clarrv_ called");
    return NULL;
}

void* clarscl2(void)
{
    if (verbose) puts("STUB: clarscl2 called");
    return NULL;
}

void* clarscl2_(void)
{
    if (verbose) puts("STUB: clarscl2_ called");
    return NULL;
}

void* clartg(void)
{
    if (verbose) puts("STUB: clartg called");
    return NULL;
}

void* clartg_(void)
{
    if (verbose) puts("STUB: clartg_ called");
    return NULL;
}

void* clartv(void)
{
    if (verbose) puts("STUB: clartv called");
    return NULL;
}

void* clartv_(void)
{
    if (verbose) puts("STUB: clartv_ called");
    return NULL;
}

void* clarz(void)
{
    if (verbose) puts("STUB: clarz called");
    return NULL;
}

void* clarz_(void)
{
    if (verbose) puts("STUB: clarz_ called");
    return NULL;
}

void* clarzb(void)
{
    if (verbose) puts("STUB: clarzb called");
    return NULL;
}

void* clarzb_(void)
{
    if (verbose) puts("STUB: clarzb_ called");
    return NULL;
}

void* clarzt(void)
{
    if (verbose) puts("STUB: clarzt called");
    return NULL;
}

void* clarzt_(void)
{
    if (verbose) puts("STUB: clarzt_ called");
    return NULL;
}

void* clascl(void)
{
    if (verbose) puts("STUB: clascl called");
    return NULL;
}

void* clascl2(void)
{
    if (verbose) puts("STUB: clascl2 called");
    return NULL;
}

void* clascl2_(void)
{
    if (verbose) puts("STUB: clascl2_ called");
    return NULL;
}

void* clascl_(void)
{
    if (verbose) puts("STUB: clascl_ called");
    return NULL;
}

void* claset(void)
{
    if (verbose) puts("STUB: claset called");
    return NULL;
}

void* claset_(void)
{
    if (verbose) puts("STUB: claset_ called");
    return NULL;
}

void* clasr(void)
{
    if (verbose) puts("STUB: clasr called");
    return NULL;
}

void* clasr_(void)
{
    if (verbose) puts("STUB: clasr_ called");
    return NULL;
}

void* classq(void)
{
    if (verbose) puts("STUB: classq called");
    return NULL;
}

void* classq_(void)
{
    if (verbose) puts("STUB: classq_ called");
    return NULL;
}

void* claswp(void)
{
    if (verbose) puts("STUB: claswp called");
    return NULL;
}

void* claswp_(void)
{
    if (verbose) puts("STUB: claswp_ called");
    return NULL;
}

void* clasyf(void)
{
    if (verbose) puts("STUB: clasyf called");
    return NULL;
}

void* clasyf_(void)
{
    if (verbose) puts("STUB: clasyf_ called");
    return NULL;
}

void* clatbs(void)
{
    if (verbose) puts("STUB: clatbs called");
    return NULL;
}

void* clatbs_(void)
{
    if (verbose) puts("STUB: clatbs_ called");
    return NULL;
}

void* clatdf(void)
{
    if (verbose) puts("STUB: clatdf called");
    return NULL;
}

void* clatdf_(void)
{
    if (verbose) puts("STUB: clatdf_ called");
    return NULL;
}

void* clatps(void)
{
    if (verbose) puts("STUB: clatps called");
    return NULL;
}

void* clatps_(void)
{
    if (verbose) puts("STUB: clatps_ called");
    return NULL;
}

void* clatrd(void)
{
    if (verbose) puts("STUB: clatrd called");
    return NULL;
}

void* clatrd_(void)
{
    if (verbose) puts("STUB: clatrd_ called");
    return NULL;
}

void* clatrs(void)
{
    if (verbose) puts("STUB: clatrs called");
    return NULL;
}

void* clatrs_(void)
{
    if (verbose) puts("STUB: clatrs_ called");
    return NULL;
}

void* clatrz(void)
{
    if (verbose) puts("STUB: clatrz called");
    return NULL;
}

void* clatrz_(void)
{
    if (verbose) puts("STUB: clatrz_ called");
    return NULL;
}

void* clatzm(void)
{
    if (verbose) puts("STUB: clatzm called");
    return NULL;
}

void* clatzm_(void)
{
    if (verbose) puts("STUB: clatzm_ called");
    return NULL;
}

void* clauu2(void)
{
    if (verbose) puts("STUB: clauu2 called");
    return NULL;
}

void* clauu2_(void)
{
    if (verbose) puts("STUB: clauu2_ called");
    return NULL;
}

void* clauum(void)
{
    if (verbose) puts("STUB: clauum called");
    return NULL;
}

void* clauum_(void)
{
    if (verbose) puts("STUB: clauum_ called");
    return NULL;
}

void* cpbcon(void)
{
    if (verbose) puts("STUB: cpbcon called");
    return NULL;
}

void* cpbcon_(void)
{
    if (verbose) puts("STUB: cpbcon_ called");
    return NULL;
}

void* cpbequ(void)
{
    if (verbose) puts("STUB: cpbequ called");
    return NULL;
}

void* cpbequ_(void)
{
    if (verbose) puts("STUB: cpbequ_ called");
    return NULL;
}

void* cpbrfs(void)
{
    if (verbose) puts("STUB: cpbrfs called");
    return NULL;
}

void* cpbrfs_(void)
{
    if (verbose) puts("STUB: cpbrfs_ called");
    return NULL;
}

void* cpbstf(void)
{
    if (verbose) puts("STUB: cpbstf called");
    return NULL;
}

void* cpbstf_(void)
{
    if (verbose) puts("STUB: cpbstf_ called");
    return NULL;
}

void* cpbsv(void)
{
    if (verbose) puts("STUB: cpbsv called");
    return NULL;
}

void* cpbsv_(void)
{
    if (verbose) puts("STUB: cpbsv_ called");
    return NULL;
}

void* cpbsvx(void)
{
    if (verbose) puts("STUB: cpbsvx called");
    return NULL;
}

void* cpbsvx_(void)
{
    if (verbose) puts("STUB: cpbsvx_ called");
    return NULL;
}

void* cpbtf2(void)
{
    if (verbose) puts("STUB: cpbtf2 called");
    return NULL;
}

void* cpbtf2_(void)
{
    if (verbose) puts("STUB: cpbtf2_ called");
    return NULL;
}

void* cpbtrf(void)
{
    if (verbose) puts("STUB: cpbtrf called");
    return NULL;
}

void* cpbtrf_(void)
{
    if (verbose) puts("STUB: cpbtrf_ called");
    return NULL;
}

void* cpbtrs(void)
{
    if (verbose) puts("STUB: cpbtrs called");
    return NULL;
}

void* cpbtrs_(void)
{
    if (verbose) puts("STUB: cpbtrs_ called");
    return NULL;
}

void* cpftrf(void)
{
    if (verbose) puts("STUB: cpftrf called");
    return NULL;
}

void* cpftrf_(void)
{
    if (verbose) puts("STUB: cpftrf_ called");
    return NULL;
}

void* cpftri(void)
{
    if (verbose) puts("STUB: cpftri called");
    return NULL;
}

void* cpftri_(void)
{
    if (verbose) puts("STUB: cpftri_ called");
    return NULL;
}

void* cpftrs(void)
{
    if (verbose) puts("STUB: cpftrs called");
    return NULL;
}

void* cpftrs_(void)
{
    if (verbose) puts("STUB: cpftrs_ called");
    return NULL;
}

void* cpocon(void)
{
    if (verbose) puts("STUB: cpocon called");
    return NULL;
}

void* cpocon_(void)
{
    if (verbose) puts("STUB: cpocon_ called");
    return NULL;
}

void* cpoequ(void)
{
    if (verbose) puts("STUB: cpoequ called");
    return NULL;
}

void* cpoequ_(void)
{
    if (verbose) puts("STUB: cpoequ_ called");
    return NULL;
}

void* cpoequb(void)
{
    if (verbose) puts("STUB: cpoequb called");
    return NULL;
}

void* cpoequb_(void)
{
    if (verbose) puts("STUB: cpoequb_ called");
    return NULL;
}

void* cporfs(void)
{
    if (verbose) puts("STUB: cporfs called");
    return NULL;
}

void* cporfs_(void)
{
    if (verbose) puts("STUB: cporfs_ called");
    return NULL;
}

void* cposv(void)
{
    if (verbose) puts("STUB: cposv called");
    return NULL;
}

void* cposv_(void)
{
    if (verbose) puts("STUB: cposv_ called");
    return NULL;
}

void* cposvx(void)
{
    if (verbose) puts("STUB: cposvx called");
    return NULL;
}

void* cposvx_(void)
{
    if (verbose) puts("STUB: cposvx_ called");
    return NULL;
}

void* cpotf2(void)
{
    if (verbose) puts("STUB: cpotf2 called");
    return NULL;
}

void* cpotf2_(void)
{
    if (verbose) puts("STUB: cpotf2_ called");
    return NULL;
}

void* cpotrf(void)
{
    if (verbose) puts("STUB: cpotrf called");
    return NULL;
}

void* cpotrf_(void)
{
    if (verbose) puts("STUB: cpotrf_ called");
    return NULL;
}

void* cpotri(void)
{
    if (verbose) puts("STUB: cpotri called");
    return NULL;
}

void* cpotri_(void)
{
    if (verbose) puts("STUB: cpotri_ called");
    return NULL;
}

void* cpotrs(void)
{
    if (verbose) puts("STUB: cpotrs called");
    return NULL;
}

void* cpotrs_(void)
{
    if (verbose) puts("STUB: cpotrs_ called");
    return NULL;
}

void* cppcon(void)
{
    if (verbose) puts("STUB: cppcon called");
    return NULL;
}

void* cppcon_(void)
{
    if (verbose) puts("STUB: cppcon_ called");
    return NULL;
}

void* cppequ(void)
{
    if (verbose) puts("STUB: cppequ called");
    return NULL;
}

void* cppequ_(void)
{
    if (verbose) puts("STUB: cppequ_ called");
    return NULL;
}

void* cpprfs(void)
{
    if (verbose) puts("STUB: cpprfs called");
    return NULL;
}

void* cpprfs_(void)
{
    if (verbose) puts("STUB: cpprfs_ called");
    return NULL;
}

void* cppsv(void)
{
    if (verbose) puts("STUB: cppsv called");
    return NULL;
}

void* cppsv_(void)
{
    if (verbose) puts("STUB: cppsv_ called");
    return NULL;
}

void* cppsvx(void)
{
    if (verbose) puts("STUB: cppsvx called");
    return NULL;
}

void* cppsvx_(void)
{
    if (verbose) puts("STUB: cppsvx_ called");
    return NULL;
}

void* cpptrf(void)
{
    if (verbose) puts("STUB: cpptrf called");
    return NULL;
}

void* cpptrf_(void)
{
    if (verbose) puts("STUB: cpptrf_ called");
    return NULL;
}

void* cpptri(void)
{
    if (verbose) puts("STUB: cpptri called");
    return NULL;
}

void* cpptri_(void)
{
    if (verbose) puts("STUB: cpptri_ called");
    return NULL;
}

void* cpptrs(void)
{
    if (verbose) puts("STUB: cpptrs called");
    return NULL;
}

void* cpptrs_(void)
{
    if (verbose) puts("STUB: cpptrs_ called");
    return NULL;
}

void* cpstf2(void)
{
    if (verbose) puts("STUB: cpstf2 called");
    return NULL;
}

void* cpstf2_(void)
{
    if (verbose) puts("STUB: cpstf2_ called");
    return NULL;
}

void* cpstrf(void)
{
    if (verbose) puts("STUB: cpstrf called");
    return NULL;
}

void* cpstrf_(void)
{
    if (verbose) puts("STUB: cpstrf_ called");
    return NULL;
}

void* cptcon(void)
{
    if (verbose) puts("STUB: cptcon called");
    return NULL;
}

void* cptcon_(void)
{
    if (verbose) puts("STUB: cptcon_ called");
    return NULL;
}

void* cpteqr(void)
{
    if (verbose) puts("STUB: cpteqr called");
    return NULL;
}

void* cpteqr_(void)
{
    if (verbose) puts("STUB: cpteqr_ called");
    return NULL;
}

void* cptrfs(void)
{
    if (verbose) puts("STUB: cptrfs called");
    return NULL;
}

void* cptrfs_(void)
{
    if (verbose) puts("STUB: cptrfs_ called");
    return NULL;
}

void* cptsv(void)
{
    if (verbose) puts("STUB: cptsv called");
    return NULL;
}

void* cptsv_(void)
{
    if (verbose) puts("STUB: cptsv_ called");
    return NULL;
}

void* cptsvx(void)
{
    if (verbose) puts("STUB: cptsvx called");
    return NULL;
}

void* cptsvx_(void)
{
    if (verbose) puts("STUB: cptsvx_ called");
    return NULL;
}

void* cpttrf(void)
{
    if (verbose) puts("STUB: cpttrf called");
    return NULL;
}

void* cpttrf_(void)
{
    if (verbose) puts("STUB: cpttrf_ called");
    return NULL;
}

void* cpttrs(void)
{
    if (verbose) puts("STUB: cpttrs called");
    return NULL;
}

void* cpttrs_(void)
{
    if (verbose) puts("STUB: cpttrs_ called");
    return NULL;
}

void* cptts2(void)
{
    if (verbose) puts("STUB: cptts2 called");
    return NULL;
}

void* cptts2_(void)
{
    if (verbose) puts("STUB: cptts2_ called");
    return NULL;
}

void* crot(void)
{
    if (verbose) puts("STUB: crot called");
    return NULL;
}

void* crot_(void)
{
    if (verbose) puts("STUB: crot_ called");
    return NULL;
}

void* cspcon(void)
{
    if (verbose) puts("STUB: cspcon called");
    return NULL;
}

void* cspcon_(void)
{
    if (verbose) puts("STUB: cspcon_ called");
    return NULL;
}

void* cspmv(void)
{
    if (verbose) puts("STUB: cspmv called");
    return NULL;
}

void* cspmv_(void)
{
    if (verbose) puts("STUB: cspmv_ called");
    return NULL;
}

void* cspr(void)
{
    if (verbose) puts("STUB: cspr called");
    return NULL;
}

void* cspr_(void)
{
    if (verbose) puts("STUB: cspr_ called");
    return NULL;
}

void* csprfs(void)
{
    if (verbose) puts("STUB: csprfs called");
    return NULL;
}

void* csprfs_(void)
{
    if (verbose) puts("STUB: csprfs_ called");
    return NULL;
}

void* cspsv(void)
{
    if (verbose) puts("STUB: cspsv called");
    return NULL;
}

void* cspsv_(void)
{
    if (verbose) puts("STUB: cspsv_ called");
    return NULL;
}

void* cspsvx(void)
{
    if (verbose) puts("STUB: cspsvx called");
    return NULL;
}

void* cspsvx_(void)
{
    if (verbose) puts("STUB: cspsvx_ called");
    return NULL;
}

void* csptrf(void)
{
    if (verbose) puts("STUB: csptrf called");
    return NULL;
}

void* csptrf_(void)
{
    if (verbose) puts("STUB: csptrf_ called");
    return NULL;
}

void* csptri(void)
{
    if (verbose) puts("STUB: csptri called");
    return NULL;
}

void* csptri_(void)
{
    if (verbose) puts("STUB: csptri_ called");
    return NULL;
}

void* csptrs(void)
{
    if (verbose) puts("STUB: csptrs called");
    return NULL;
}

void* csptrs_(void)
{
    if (verbose) puts("STUB: csptrs_ called");
    return NULL;
}

void* csrscl(void)
{
    if (verbose) puts("STUB: csrscl called");
    return NULL;
}

void* csrscl_(void)
{
    if (verbose) puts("STUB: csrscl_ called");
    return NULL;
}

void* cstedc(void)
{
    if (verbose) puts("STUB: cstedc called");
    return NULL;
}

void* cstedc_(void)
{
    if (verbose) puts("STUB: cstedc_ called");
    return NULL;
}

void* cstegr(void)
{
    if (verbose) puts("STUB: cstegr called");
    return NULL;
}

void* cstegr_(void)
{
    if (verbose) puts("STUB: cstegr_ called");
    return NULL;
}

void* cstein(void)
{
    if (verbose) puts("STUB: cstein called");
    return NULL;
}

void* cstein_(void)
{
    if (verbose) puts("STUB: cstein_ called");
    return NULL;
}

void* cstemr(void)
{
    if (verbose) puts("STUB: cstemr called");
    return NULL;
}

void* cstemr_(void)
{
    if (verbose) puts("STUB: cstemr_ called");
    return NULL;
}

void* csteqr(void)
{
    if (verbose) puts("STUB: csteqr called");
    return NULL;
}

void* csteqr_(void)
{
    if (verbose) puts("STUB: csteqr_ called");
    return NULL;
}

void* csycon(void)
{
    if (verbose) puts("STUB: csycon called");
    return NULL;
}

void* csycon_(void)
{
    if (verbose) puts("STUB: csycon_ called");
    return NULL;
}

void* csyequb(void)
{
    if (verbose) puts("STUB: csyequb called");
    return NULL;
}

void* csyequb_(void)
{
    if (verbose) puts("STUB: csyequb_ called");
    return NULL;
}

void* csymv(void)
{
    if (verbose) puts("STUB: csymv called");
    return NULL;
}

void* csymv_(void)
{
    if (verbose) puts("STUB: csymv_ called");
    return NULL;
}

void* csyr(void)
{
    if (verbose) puts("STUB: csyr called");
    return NULL;
}

void* csyr_(void)
{
    if (verbose) puts("STUB: csyr_ called");
    return NULL;
}

void* csyrfs(void)
{
    if (verbose) puts("STUB: csyrfs called");
    return NULL;
}

void* csyrfs_(void)
{
    if (verbose) puts("STUB: csyrfs_ called");
    return NULL;
}

void* csysv(void)
{
    if (verbose) puts("STUB: csysv called");
    return NULL;
}

void* csysv_(void)
{
    if (verbose) puts("STUB: csysv_ called");
    return NULL;
}

void* csysvx(void)
{
    if (verbose) puts("STUB: csysvx called");
    return NULL;
}

void* csysvx_(void)
{
    if (verbose) puts("STUB: csysvx_ called");
    return NULL;
}

void* csytf2(void)
{
    if (verbose) puts("STUB: csytf2 called");
    return NULL;
}

void* csytf2_(void)
{
    if (verbose) puts("STUB: csytf2_ called");
    return NULL;
}

void* csytrf(void)
{
    if (verbose) puts("STUB: csytrf called");
    return NULL;
}

void* csytrf_(void)
{
    if (verbose) puts("STUB: csytrf_ called");
    return NULL;
}

void* csytri(void)
{
    if (verbose) puts("STUB: csytri called");
    return NULL;
}

void* csytri_(void)
{
    if (verbose) puts("STUB: csytri_ called");
    return NULL;
}

void* csytrs(void)
{
    if (verbose) puts("STUB: csytrs called");
    return NULL;
}

void* csytrs_(void)
{
    if (verbose) puts("STUB: csytrs_ called");
    return NULL;
}

void* ctbcon(void)
{
    if (verbose) puts("STUB: ctbcon called");
    return NULL;
}

void* ctbcon_(void)
{
    if (verbose) puts("STUB: ctbcon_ called");
    return NULL;
}

void* ctbrfs(void)
{
    if (verbose) puts("STUB: ctbrfs called");
    return NULL;
}

void* ctbrfs_(void)
{
    if (verbose) puts("STUB: ctbrfs_ called");
    return NULL;
}

void* ctbtrs(void)
{
    if (verbose) puts("STUB: ctbtrs called");
    return NULL;
}

void* ctbtrs_(void)
{
    if (verbose) puts("STUB: ctbtrs_ called");
    return NULL;
}

void* ctfsm(void)
{
    if (verbose) puts("STUB: ctfsm called");
    return NULL;
}

void* ctfsm_(void)
{
    if (verbose) puts("STUB: ctfsm_ called");
    return NULL;
}

void* ctftri(void)
{
    if (verbose) puts("STUB: ctftri called");
    return NULL;
}

void* ctftri_(void)
{
    if (verbose) puts("STUB: ctftri_ called");
    return NULL;
}

void* ctfttp(void)
{
    if (verbose) puts("STUB: ctfttp called");
    return NULL;
}

void* ctfttp_(void)
{
    if (verbose) puts("STUB: ctfttp_ called");
    return NULL;
}

void* ctfttr(void)
{
    if (verbose) puts("STUB: ctfttr called");
    return NULL;
}

void* ctfttr_(void)
{
    if (verbose) puts("STUB: ctfttr_ called");
    return NULL;
}

void* ctgevc(void)
{
    if (verbose) puts("STUB: ctgevc called");
    return NULL;
}

void* ctgevc_(void)
{
    if (verbose) puts("STUB: ctgevc_ called");
    return NULL;
}

void* ctgex2(void)
{
    if (verbose) puts("STUB: ctgex2 called");
    return NULL;
}

void* ctgex2_(void)
{
    if (verbose) puts("STUB: ctgex2_ called");
    return NULL;
}

void* ctgexc(void)
{
    if (verbose) puts("STUB: ctgexc called");
    return NULL;
}

void* ctgexc_(void)
{
    if (verbose) puts("STUB: ctgexc_ called");
    return NULL;
}

void* ctgsen(void)
{
    if (verbose) puts("STUB: ctgsen called");
    return NULL;
}

void* ctgsen_(void)
{
    if (verbose) puts("STUB: ctgsen_ called");
    return NULL;
}

void* ctgsja(void)
{
    if (verbose) puts("STUB: ctgsja called");
    return NULL;
}

void* ctgsja_(void)
{
    if (verbose) puts("STUB: ctgsja_ called");
    return NULL;
}

void* ctgsna(void)
{
    if (verbose) puts("STUB: ctgsna called");
    return NULL;
}

void* ctgsna_(void)
{
    if (verbose) puts("STUB: ctgsna_ called");
    return NULL;
}

void* ctgsy2(void)
{
    if (verbose) puts("STUB: ctgsy2 called");
    return NULL;
}

void* ctgsy2_(void)
{
    if (verbose) puts("STUB: ctgsy2_ called");
    return NULL;
}

void* ctgsyl(void)
{
    if (verbose) puts("STUB: ctgsyl called");
    return NULL;
}

void* ctgsyl_(void)
{
    if (verbose) puts("STUB: ctgsyl_ called");
    return NULL;
}

void* ctpcon(void)
{
    if (verbose) puts("STUB: ctpcon called");
    return NULL;
}

void* ctpcon_(void)
{
    if (verbose) puts("STUB: ctpcon_ called");
    return NULL;
}

void* ctprfs(void)
{
    if (verbose) puts("STUB: ctprfs called");
    return NULL;
}

void* ctprfs_(void)
{
    if (verbose) puts("STUB: ctprfs_ called");
    return NULL;
}

void* ctptri(void)
{
    if (verbose) puts("STUB: ctptri called");
    return NULL;
}

void* ctptri_(void)
{
    if (verbose) puts("STUB: ctptri_ called");
    return NULL;
}

void* ctptrs(void)
{
    if (verbose) puts("STUB: ctptrs called");
    return NULL;
}

void* ctptrs_(void)
{
    if (verbose) puts("STUB: ctptrs_ called");
    return NULL;
}

void* ctpttf(void)
{
    if (verbose) puts("STUB: ctpttf called");
    return NULL;
}

void* ctpttf_(void)
{
    if (verbose) puts("STUB: ctpttf_ called");
    return NULL;
}

void* ctpttr(void)
{
    if (verbose) puts("STUB: ctpttr called");
    return NULL;
}

void* ctpttr_(void)
{
    if (verbose) puts("STUB: ctpttr_ called");
    return NULL;
}

void* ctrcon(void)
{
    if (verbose) puts("STUB: ctrcon called");
    return NULL;
}

void* ctrcon_(void)
{
    if (verbose) puts("STUB: ctrcon_ called");
    return NULL;
}

void* ctrevc(void)
{
    if (verbose) puts("STUB: ctrevc called");
    return NULL;
}

void* ctrevc_(void)
{
    if (verbose) puts("STUB: ctrevc_ called");
    return NULL;
}

void* ctrexc(void)
{
    if (verbose) puts("STUB: ctrexc called");
    return NULL;
}

void* ctrexc_(void)
{
    if (verbose) puts("STUB: ctrexc_ called");
    return NULL;
}

void* ctrrfs(void)
{
    if (verbose) puts("STUB: ctrrfs called");
    return NULL;
}

void* ctrrfs_(void)
{
    if (verbose) puts("STUB: ctrrfs_ called");
    return NULL;
}

void* ctrsen(void)
{
    if (verbose) puts("STUB: ctrsen called");
    return NULL;
}

void* ctrsen_(void)
{
    if (verbose) puts("STUB: ctrsen_ called");
    return NULL;
}

void* ctrsna(void)
{
    if (verbose) puts("STUB: ctrsna called");
    return NULL;
}

void* ctrsna_(void)
{
    if (verbose) puts("STUB: ctrsna_ called");
    return NULL;
}

void* ctrsyl(void)
{
    if (verbose) puts("STUB: ctrsyl called");
    return NULL;
}

void* ctrsyl_(void)
{
    if (verbose) puts("STUB: ctrsyl_ called");
    return NULL;
}

void* ctrti2(void)
{
    if (verbose) puts("STUB: ctrti2 called");
    return NULL;
}

void* ctrti2_(void)
{
    if (verbose) puts("STUB: ctrti2_ called");
    return NULL;
}

void* ctrtri(void)
{
    if (verbose) puts("STUB: ctrtri called");
    return NULL;
}

void* ctrtri_(void)
{
    if (verbose) puts("STUB: ctrtri_ called");
    return NULL;
}

void* ctrtrs(void)
{
    if (verbose) puts("STUB: ctrtrs called");
    return NULL;
}

void* ctrtrs_(void)
{
    if (verbose) puts("STUB: ctrtrs_ called");
    return NULL;
}

void* ctrttf(void)
{
    if (verbose) puts("STUB: ctrttf called");
    return NULL;
}

void* ctrttf_(void)
{
    if (verbose) puts("STUB: ctrttf_ called");
    return NULL;
}

void* ctrttp(void)
{
    if (verbose) puts("STUB: ctrttp called");
    return NULL;
}

void* ctrttp_(void)
{
    if (verbose) puts("STUB: ctrttp_ called");
    return NULL;
}

void* ctzrqf(void)
{
    if (verbose) puts("STUB: ctzrqf called");
    return NULL;
}

void* ctzrqf_(void)
{
    if (verbose) puts("STUB: ctzrqf_ called");
    return NULL;
}

void* ctzrzf(void)
{
    if (verbose) puts("STUB: ctzrzf called");
    return NULL;
}

void* ctzrzf_(void)
{
    if (verbose) puts("STUB: ctzrzf_ called");
    return NULL;
}

void* cung2l(void)
{
    if (verbose) puts("STUB: cung2l called");
    return NULL;
}

void* cung2l_(void)
{
    if (verbose) puts("STUB: cung2l_ called");
    return NULL;
}

void* cung2r(void)
{
    if (verbose) puts("STUB: cung2r called");
    return NULL;
}

void* cung2r_(void)
{
    if (verbose) puts("STUB: cung2r_ called");
    return NULL;
}

void* cungbr(void)
{
    if (verbose) puts("STUB: cungbr called");
    return NULL;
}

void* cungbr_(void)
{
    if (verbose) puts("STUB: cungbr_ called");
    return NULL;
}

void* cunghr(void)
{
    if (verbose) puts("STUB: cunghr called");
    return NULL;
}

void* cunghr_(void)
{
    if (verbose) puts("STUB: cunghr_ called");
    return NULL;
}

void* cungl2(void)
{
    if (verbose) puts("STUB: cungl2 called");
    return NULL;
}

void* cungl2_(void)
{
    if (verbose) puts("STUB: cungl2_ called");
    return NULL;
}

void* cunglq(void)
{
    if (verbose) puts("STUB: cunglq called");
    return NULL;
}

void* cunglq_(void)
{
    if (verbose) puts("STUB: cunglq_ called");
    return NULL;
}

void* cungql(void)
{
    if (verbose) puts("STUB: cungql called");
    return NULL;
}

void* cungql_(void)
{
    if (verbose) puts("STUB: cungql_ called");
    return NULL;
}

void* cungqr(void)
{
    if (verbose) puts("STUB: cungqr called");
    return NULL;
}

void* cungqr_(void)
{
    if (verbose) puts("STUB: cungqr_ called");
    return NULL;
}

void* cungr2(void)
{
    if (verbose) puts("STUB: cungr2 called");
    return NULL;
}

void* cungr2_(void)
{
    if (verbose) puts("STUB: cungr2_ called");
    return NULL;
}

void* cungrq(void)
{
    if (verbose) puts("STUB: cungrq called");
    return NULL;
}

void* cungrq_(void)
{
    if (verbose) puts("STUB: cungrq_ called");
    return NULL;
}

void* cungtr(void)
{
    if (verbose) puts("STUB: cungtr called");
    return NULL;
}

void* cungtr_(void)
{
    if (verbose) puts("STUB: cungtr_ called");
    return NULL;
}

void* cunm2l(void)
{
    if (verbose) puts("STUB: cunm2l called");
    return NULL;
}

void* cunm2l_(void)
{
    if (verbose) puts("STUB: cunm2l_ called");
    return NULL;
}

void* cunm2r(void)
{
    if (verbose) puts("STUB: cunm2r called");
    return NULL;
}

void* cunm2r_(void)
{
    if (verbose) puts("STUB: cunm2r_ called");
    return NULL;
}

void* cunmbr(void)
{
    if (verbose) puts("STUB: cunmbr called");
    return NULL;
}

void* cunmbr_(void)
{
    if (verbose) puts("STUB: cunmbr_ called");
    return NULL;
}

void* cunmhr(void)
{
    if (verbose) puts("STUB: cunmhr called");
    return NULL;
}

void* cunmhr_(void)
{
    if (verbose) puts("STUB: cunmhr_ called");
    return NULL;
}

void* cunml2(void)
{
    if (verbose) puts("STUB: cunml2 called");
    return NULL;
}

void* cunml2_(void)
{
    if (verbose) puts("STUB: cunml2_ called");
    return NULL;
}

void* cunmlq(void)
{
    if (verbose) puts("STUB: cunmlq called");
    return NULL;
}

void* cunmlq_(void)
{
    if (verbose) puts("STUB: cunmlq_ called");
    return NULL;
}

void* cunmql(void)
{
    if (verbose) puts("STUB: cunmql called");
    return NULL;
}

void* cunmql_(void)
{
    if (verbose) puts("STUB: cunmql_ called");
    return NULL;
}

void* cunmqr(void)
{
    if (verbose) puts("STUB: cunmqr called");
    return NULL;
}

void* cunmqr_(void)
{
    if (verbose) puts("STUB: cunmqr_ called");
    return NULL;
}

void* cunmr2(void)
{
    if (verbose) puts("STUB: cunmr2 called");
    return NULL;
}

void* cunmr2_(void)
{
    if (verbose) puts("STUB: cunmr2_ called");
    return NULL;
}

void* cunmr3(void)
{
    if (verbose) puts("STUB: cunmr3 called");
    return NULL;
}

void* cunmr3_(void)
{
    if (verbose) puts("STUB: cunmr3_ called");
    return NULL;
}

void* cunmrq(void)
{
    if (verbose) puts("STUB: cunmrq called");
    return NULL;
}

void* cunmrq_(void)
{
    if (verbose) puts("STUB: cunmrq_ called");
    return NULL;
}

void* cunmrz(void)
{
    if (verbose) puts("STUB: cunmrz called");
    return NULL;
}

void* cunmrz_(void)
{
    if (verbose) puts("STUB: cunmrz_ called");
    return NULL;
}

void* cunmtr(void)
{
    if (verbose) puts("STUB: cunmtr called");
    return NULL;
}

void* cunmtr_(void)
{
    if (verbose) puts("STUB: cunmtr_ called");
    return NULL;
}

void* cupgtr(void)
{
    if (verbose) puts("STUB: cupgtr called");
    return NULL;
}

void* cupgtr_(void)
{
    if (verbose) puts("STUB: cupgtr_ called");
    return NULL;
}

void* cupmtr(void)
{
    if (verbose) puts("STUB: cupmtr called");
    return NULL;
}

void* cupmtr_(void)
{
    if (verbose) puts("STUB: cupmtr_ called");
    return NULL;
}

void* dbdsdc(void)
{
    if (verbose) puts("STUB: dbdsdc called");
    return NULL;
}

void* dbdsdc_(void)
{
    if (verbose) puts("STUB: dbdsdc_ called");
    return NULL;
}

void* dbdsqr(void)
{
    if (verbose) puts("STUB: dbdsqr called");
    return NULL;
}

void* dbdsqr_(void)
{
    if (verbose) puts("STUB: dbdsqr_ called");
    return NULL;
}

void* ddisna(void)
{
    if (verbose) puts("STUB: ddisna called");
    return NULL;
}

void* ddisna_(void)
{
    if (verbose) puts("STUB: ddisna_ called");
    return NULL;
}

void* dgbbrd(void)
{
    if (verbose) puts("STUB: dgbbrd called");
    return NULL;
}

void* dgbbrd_(void)
{
    if (verbose) puts("STUB: dgbbrd_ called");
    return NULL;
}

void* dgbcon(void)
{
    if (verbose) puts("STUB: dgbcon called");
    return NULL;
}

void* dgbcon_(void)
{
    if (verbose) puts("STUB: dgbcon_ called");
    return NULL;
}

void* dgbequ(void)
{
    if (verbose) puts("STUB: dgbequ called");
    return NULL;
}

void* dgbequ_(void)
{
    if (verbose) puts("STUB: dgbequ_ called");
    return NULL;
}

void* dgbequb(void)
{
    if (verbose) puts("STUB: dgbequb called");
    return NULL;
}

void* dgbequb_(void)
{
    if (verbose) puts("STUB: dgbequb_ called");
    return NULL;
}

void* dgbrfs(void)
{
    if (verbose) puts("STUB: dgbrfs called");
    return NULL;
}

void* dgbrfs_(void)
{
    if (verbose) puts("STUB: dgbrfs_ called");
    return NULL;
}

void* dgbsv(void)
{
    if (verbose) puts("STUB: dgbsv called");
    return NULL;
}

void* dgbsv_(void)
{
    if (verbose) puts("STUB: dgbsv_ called");
    return NULL;
}

void* dgbsvx(void)
{
    if (verbose) puts("STUB: dgbsvx called");
    return NULL;
}

void* dgbsvx_(void)
{
    if (verbose) puts("STUB: dgbsvx_ called");
    return NULL;
}

void* dgbtf2(void)
{
    if (verbose) puts("STUB: dgbtf2 called");
    return NULL;
}

void* dgbtf2_(void)
{
    if (verbose) puts("STUB: dgbtf2_ called");
    return NULL;
}

void* dgbtrf(void)
{
    if (verbose) puts("STUB: dgbtrf called");
    return NULL;
}

void* dgbtrf_(void)
{
    if (verbose) puts("STUB: dgbtrf_ called");
    return NULL;
}

void* dgbtrs(void)
{
    if (verbose) puts("STUB: dgbtrs called");
    return NULL;
}

void* dgbtrs_(void)
{
    if (verbose) puts("STUB: dgbtrs_ called");
    return NULL;
}

void* dgebak(void)
{
    if (verbose) puts("STUB: dgebak called");
    return NULL;
}

void* dgebak_(void)
{
    if (verbose) puts("STUB: dgebak_ called");
    return NULL;
}

void* dgebal(void)
{
    if (verbose) puts("STUB: dgebal called");
    return NULL;
}

void* dgebal_(void)
{
    if (verbose) puts("STUB: dgebal_ called");
    return NULL;
}

void* dgebd2(void)
{
    if (verbose) puts("STUB: dgebd2 called");
    return NULL;
}

void* dgebd2_(void)
{
    if (verbose) puts("STUB: dgebd2_ called");
    return NULL;
}

void* dgebrd(void)
{
    if (verbose) puts("STUB: dgebrd called");
    return NULL;
}

void* dgebrd_(void)
{
    if (verbose) puts("STUB: dgebrd_ called");
    return NULL;
}

void* dgecon(void)
{
    if (verbose) puts("STUB: dgecon called");
    return NULL;
}

void* dgecon_(void)
{
    if (verbose) puts("STUB: dgecon_ called");
    return NULL;
}

void* dgeequ(void)
{
    if (verbose) puts("STUB: dgeequ called");
    return NULL;
}

void* dgeequ_(void)
{
    if (verbose) puts("STUB: dgeequ_ called");
    return NULL;
}

void* dgeequb(void)
{
    if (verbose) puts("STUB: dgeequb called");
    return NULL;
}

void* dgeequb_(void)
{
    if (verbose) puts("STUB: dgeequb_ called");
    return NULL;
}

void* dgees(void)
{
    if (verbose) puts("STUB: dgees called");
    return NULL;
}

void* dgees_(void)
{
    if (verbose) puts("STUB: dgees_ called");
    return NULL;
}

void* dgeesx(void)
{
    if (verbose) puts("STUB: dgeesx called");
    return NULL;
}

void* dgeesx_(void)
{
    if (verbose) puts("STUB: dgeesx_ called");
    return NULL;
}

void* dgeev(void)
{
    if (verbose) puts("STUB: dgeev called");
    return NULL;
}

void* dgeev_(void)
{
    if (verbose) puts("STUB: dgeev_ called");
    return NULL;
}

void* dgeevx(void)
{
    if (verbose) puts("STUB: dgeevx called");
    return NULL;
}

void* dgeevx_(void)
{
    if (verbose) puts("STUB: dgeevx_ called");
    return NULL;
}

void* dgegs(void)
{
    if (verbose) puts("STUB: dgegs called");
    return NULL;
}

void* dgegs_(void)
{
    if (verbose) puts("STUB: dgegs_ called");
    return NULL;
}

void* dgegv(void)
{
    if (verbose) puts("STUB: dgegv called");
    return NULL;
}

void* dgegv_(void)
{
    if (verbose) puts("STUB: dgegv_ called");
    return NULL;
}

void* dgehd2(void)
{
    if (verbose) puts("STUB: dgehd2 called");
    return NULL;
}

void* dgehd2_(void)
{
    if (verbose) puts("STUB: dgehd2_ called");
    return NULL;
}

void* dgehrd(void)
{
    if (verbose) puts("STUB: dgehrd called");
    return NULL;
}

void* dgehrd_(void)
{
    if (verbose) puts("STUB: dgehrd_ called");
    return NULL;
}

void* dgejsv(void)
{
    if (verbose) puts("STUB: dgejsv called");
    return NULL;
}

void* dgejsv_(void)
{
    if (verbose) puts("STUB: dgejsv_ called");
    return NULL;
}

void* dgelq2(void)
{
    if (verbose) puts("STUB: dgelq2 called");
    return NULL;
}

void* dgelq2_(void)
{
    if (verbose) puts("STUB: dgelq2_ called");
    return NULL;
}

void* dgelqf(void)
{
    if (verbose) puts("STUB: dgelqf called");
    return NULL;
}

void* dgelqf_(void)
{
    if (verbose) puts("STUB: dgelqf_ called");
    return NULL;
}

void* dgels(void)
{
    if (verbose) puts("STUB: dgels called");
    return NULL;
}

void* dgels_(void)
{
    if (verbose) puts("STUB: dgels_ called");
    return NULL;
}

void* dgelsd(void)
{
    if (verbose) puts("STUB: dgelsd called");
    return NULL;
}

void* dgelsd_(void)
{
    if (verbose) puts("STUB: dgelsd_ called");
    return NULL;
}

void* dgelss(void)
{
    if (verbose) puts("STUB: dgelss called");
    return NULL;
}

void* dgelss_(void)
{
    if (verbose) puts("STUB: dgelss_ called");
    return NULL;
}

void* dgelsx(void)
{
    if (verbose) puts("STUB: dgelsx called");
    return NULL;
}

void* dgelsx_(void)
{
    if (verbose) puts("STUB: dgelsx_ called");
    return NULL;
}

void* dgelsy(void)
{
    if (verbose) puts("STUB: dgelsy called");
    return NULL;
}

void* dgelsy_(void)
{
    if (verbose) puts("STUB: dgelsy_ called");
    return NULL;
}

void* dgeql2(void)
{
    if (verbose) puts("STUB: dgeql2 called");
    return NULL;
}

void* dgeql2_(void)
{
    if (verbose) puts("STUB: dgeql2_ called");
    return NULL;
}

void* dgeqlf(void)
{
    if (verbose) puts("STUB: dgeqlf called");
    return NULL;
}

void* dgeqlf_(void)
{
    if (verbose) puts("STUB: dgeqlf_ called");
    return NULL;
}

void* dgeqp3(void)
{
    if (verbose) puts("STUB: dgeqp3 called");
    return NULL;
}

void* dgeqp3_(void)
{
    if (verbose) puts("STUB: dgeqp3_ called");
    return NULL;
}

void* dgeqpf(void)
{
    if (verbose) puts("STUB: dgeqpf called");
    return NULL;
}

void* dgeqpf_(void)
{
    if (verbose) puts("STUB: dgeqpf_ called");
    return NULL;
}

void* dgeqr2(void)
{
    if (verbose) puts("STUB: dgeqr2 called");
    return NULL;
}

void* dgeqr2_(void)
{
    if (verbose) puts("STUB: dgeqr2_ called");
    return NULL;
}

void* dgeqrf(void)
{
    if (verbose) puts("STUB: dgeqrf called");
    return NULL;
}

void* dgeqrf_(void)
{
    if (verbose) puts("STUB: dgeqrf_ called");
    return NULL;
}

void* dgerfs(void)
{
    if (verbose) puts("STUB: dgerfs called");
    return NULL;
}

void* dgerfs_(void)
{
    if (verbose) puts("STUB: dgerfs_ called");
    return NULL;
}

void* dgerq2(void)
{
    if (verbose) puts("STUB: dgerq2 called");
    return NULL;
}

void* dgerq2_(void)
{
    if (verbose) puts("STUB: dgerq2_ called");
    return NULL;
}

void* dgerqf(void)
{
    if (verbose) puts("STUB: dgerqf called");
    return NULL;
}

void* dgerqf_(void)
{
    if (verbose) puts("STUB: dgerqf_ called");
    return NULL;
}

void* dgesc2(void)
{
    if (verbose) puts("STUB: dgesc2 called");
    return NULL;
}

void* dgesc2_(void)
{
    if (verbose) puts("STUB: dgesc2_ called");
    return NULL;
}

void* dgesdd(void)
{
    if (verbose) puts("STUB: dgesdd called");
    return NULL;
}

void* dgesdd_(void)
{
    if (verbose) puts("STUB: dgesdd_ called");
    return NULL;
}

void* dgesv(void)
{
    if (verbose) puts("STUB: dgesv called");
    return NULL;
}

void* dgesv_(void)
{
    if (verbose) puts("STUB: dgesv_ called");
    return NULL;
}

void* dgesvd(void)
{
    if (verbose) puts("STUB: dgesvd called");
    return NULL;
}

void* dgesvd_(void)
{
    if (verbose) puts("STUB: dgesvd_ called");
    return NULL;
}

void* dgesvj(void)
{
    if (verbose) puts("STUB: dgesvj called");
    return NULL;
}

void* dgesvj_(void)
{
    if (verbose) puts("STUB: dgesvj_ called");
    return NULL;
}

void* dgesvx(void)
{
    if (verbose) puts("STUB: dgesvx called");
    return NULL;
}

void* dgesvx_(void)
{
    if (verbose) puts("STUB: dgesvx_ called");
    return NULL;
}

void* dgetc2(void)
{
    if (verbose) puts("STUB: dgetc2 called");
    return NULL;
}

void* dgetc2_(void)
{
    if (verbose) puts("STUB: dgetc2_ called");
    return NULL;
}

void* dgetf2(void)
{
    if (verbose) puts("STUB: dgetf2 called");
    return NULL;
}

void* dgetf2_(void)
{
    if (verbose) puts("STUB: dgetf2_ called");
    return NULL;
}

void* dgetrf(void)
{
    if (verbose) puts("STUB: dgetrf called");
    return NULL;
}

void* dgetrf_(void)
{
    if (verbose) puts("STUB: dgetrf_ called");
    return NULL;
}

void* dgetri(void)
{
    if (verbose) puts("STUB: dgetri called");
    return NULL;
}

void* dgetri_(void)
{
    if (verbose) puts("STUB: dgetri_ called");
    return NULL;
}

void* dgetrs(void)
{
    if (verbose) puts("STUB: dgetrs called");
    return NULL;
}

void* dgetrs_(void)
{
    if (verbose) puts("STUB: dgetrs_ called");
    return NULL;
}

void* dggbak(void)
{
    if (verbose) puts("STUB: dggbak called");
    return NULL;
}

void* dggbak_(void)
{
    if (verbose) puts("STUB: dggbak_ called");
    return NULL;
}

void* dggbal(void)
{
    if (verbose) puts("STUB: dggbal called");
    return NULL;
}

void* dggbal_(void)
{
    if (verbose) puts("STUB: dggbal_ called");
    return NULL;
}

void* dgges(void)
{
    if (verbose) puts("STUB: dgges called");
    return NULL;
}

void* dgges_(void)
{
    if (verbose) puts("STUB: dgges_ called");
    return NULL;
}

void* dggesx(void)
{
    if (verbose) puts("STUB: dggesx called");
    return NULL;
}

void* dggesx_(void)
{
    if (verbose) puts("STUB: dggesx_ called");
    return NULL;
}

void* dggev(void)
{
    if (verbose) puts("STUB: dggev called");
    return NULL;
}

void* dggev_(void)
{
    if (verbose) puts("STUB: dggev_ called");
    return NULL;
}

void* dggevx(void)
{
    if (verbose) puts("STUB: dggevx called");
    return NULL;
}

void* dggevx_(void)
{
    if (verbose) puts("STUB: dggevx_ called");
    return NULL;
}

void* dggglm(void)
{
    if (verbose) puts("STUB: dggglm called");
    return NULL;
}

void* dggglm_(void)
{
    if (verbose) puts("STUB: dggglm_ called");
    return NULL;
}

void* dgghrd(void)
{
    if (verbose) puts("STUB: dgghrd called");
    return NULL;
}

void* dgghrd_(void)
{
    if (verbose) puts("STUB: dgghrd_ called");
    return NULL;
}

void* dgglse(void)
{
    if (verbose) puts("STUB: dgglse called");
    return NULL;
}

void* dgglse_(void)
{
    if (verbose) puts("STUB: dgglse_ called");
    return NULL;
}

void* dggqrf(void)
{
    if (verbose) puts("STUB: dggqrf called");
    return NULL;
}

void* dggqrf_(void)
{
    if (verbose) puts("STUB: dggqrf_ called");
    return NULL;
}

void* dggrqf(void)
{
    if (verbose) puts("STUB: dggrqf called");
    return NULL;
}

void* dggrqf_(void)
{
    if (verbose) puts("STUB: dggrqf_ called");
    return NULL;
}

void* dggsvd(void)
{
    if (verbose) puts("STUB: dggsvd called");
    return NULL;
}

void* dggsvd_(void)
{
    if (verbose) puts("STUB: dggsvd_ called");
    return NULL;
}

void* dggsvp(void)
{
    if (verbose) puts("STUB: dggsvp called");
    return NULL;
}

void* dggsvp_(void)
{
    if (verbose) puts("STUB: dggsvp_ called");
    return NULL;
}

void* dgsvj0(void)
{
    if (verbose) puts("STUB: dgsvj0 called");
    return NULL;
}

void* dgsvj0_(void)
{
    if (verbose) puts("STUB: dgsvj0_ called");
    return NULL;
}

void* dgsvj1(void)
{
    if (verbose) puts("STUB: dgsvj1 called");
    return NULL;
}

void* dgsvj1_(void)
{
    if (verbose) puts("STUB: dgsvj1_ called");
    return NULL;
}

void* dgtcon(void)
{
    if (verbose) puts("STUB: dgtcon called");
    return NULL;
}

void* dgtcon_(void)
{
    if (verbose) puts("STUB: dgtcon_ called");
    return NULL;
}

void* dgtrfs(void)
{
    if (verbose) puts("STUB: dgtrfs called");
    return NULL;
}

void* dgtrfs_(void)
{
    if (verbose) puts("STUB: dgtrfs_ called");
    return NULL;
}

void* dgtsv(void)
{
    if (verbose) puts("STUB: dgtsv called");
    return NULL;
}

void* dgtsv_(void)
{
    if (verbose) puts("STUB: dgtsv_ called");
    return NULL;
}

void* dgtsvx(void)
{
    if (verbose) puts("STUB: dgtsvx called");
    return NULL;
}

void* dgtsvx_(void)
{
    if (verbose) puts("STUB: dgtsvx_ called");
    return NULL;
}

void* dgttrf(void)
{
    if (verbose) puts("STUB: dgttrf called");
    return NULL;
}

void* dgttrf_(void)
{
    if (verbose) puts("STUB: dgttrf_ called");
    return NULL;
}

void* dgttrs(void)
{
    if (verbose) puts("STUB: dgttrs called");
    return NULL;
}

void* dgttrs_(void)
{
    if (verbose) puts("STUB: dgttrs_ called");
    return NULL;
}

void* dgtts2(void)
{
    if (verbose) puts("STUB: dgtts2 called");
    return NULL;
}

void* dgtts2_(void)
{
    if (verbose) puts("STUB: dgtts2_ called");
    return NULL;
}

void* dhgeqz(void)
{
    if (verbose) puts("STUB: dhgeqz called");
    return NULL;
}

void* dhgeqz_(void)
{
    if (verbose) puts("STUB: dhgeqz_ called");
    return NULL;
}

void* dhsein(void)
{
    if (verbose) puts("STUB: dhsein called");
    return NULL;
}

void* dhsein_(void)
{
    if (verbose) puts("STUB: dhsein_ called");
    return NULL;
}

void* dhseqr(void)
{
    if (verbose) puts("STUB: dhseqr called");
    return NULL;
}

void* dhseqr_(void)
{
    if (verbose) puts("STUB: dhseqr_ called");
    return NULL;
}

void* disnan(void)
{
    if (verbose) puts("STUB: disnan called");
    return NULL;
}

void* disnan_(void)
{
    if (verbose) puts("STUB: disnan_ called");
    return NULL;
}

void* dlabad(void)
{
    if (verbose) puts("STUB: dlabad called");
    return NULL;
}

void* dlabad_(void)
{
    if (verbose) puts("STUB: dlabad_ called");
    return NULL;
}

void* dlabrd(void)
{
    if (verbose) puts("STUB: dlabrd called");
    return NULL;
}

void* dlabrd_(void)
{
    if (verbose) puts("STUB: dlabrd_ called");
    return NULL;
}

void* dlacn2(void)
{
    if (verbose) puts("STUB: dlacn2 called");
    return NULL;
}

void* dlacn2_(void)
{
    if (verbose) puts("STUB: dlacn2_ called");
    return NULL;
}

void* dlacon(void)
{
    if (verbose) puts("STUB: dlacon called");
    return NULL;
}

void* dlacon_(void)
{
    if (verbose) puts("STUB: dlacon_ called");
    return NULL;
}

void* dlacpy(void)
{
    if (verbose) puts("STUB: dlacpy called");
    return NULL;
}

void* dlacpy_(void)
{
    if (verbose) puts("STUB: dlacpy_ called");
    return NULL;
}

void* dladiv(void)
{
    if (verbose) puts("STUB: dladiv called");
    return NULL;
}

void* dladiv_(void)
{
    if (verbose) puts("STUB: dladiv_ called");
    return NULL;
}

void* dlae2(void)
{
    if (verbose) puts("STUB: dlae2 called");
    return NULL;
}

void* dlae2_(void)
{
    if (verbose) puts("STUB: dlae2_ called");
    return NULL;
}

void* dlaebz(void)
{
    if (verbose) puts("STUB: dlaebz called");
    return NULL;
}

void* dlaebz_(void)
{
    if (verbose) puts("STUB: dlaebz_ called");
    return NULL;
}

void* dlaed0(void)
{
    if (verbose) puts("STUB: dlaed0 called");
    return NULL;
}

void* dlaed0_(void)
{
    if (verbose) puts("STUB: dlaed0_ called");
    return NULL;
}

void* dlaed1(void)
{
    if (verbose) puts("STUB: dlaed1 called");
    return NULL;
}

void* dlaed1_(void)
{
    if (verbose) puts("STUB: dlaed1_ called");
    return NULL;
}

void* dlaed2(void)
{
    if (verbose) puts("STUB: dlaed2 called");
    return NULL;
}

void* dlaed2_(void)
{
    if (verbose) puts("STUB: dlaed2_ called");
    return NULL;
}

void* dlaed3(void)
{
    if (verbose) puts("STUB: dlaed3 called");
    return NULL;
}

void* dlaed3_(void)
{
    if (verbose) puts("STUB: dlaed3_ called");
    return NULL;
}

void* dlaed4(void)
{
    if (verbose) puts("STUB: dlaed4 called");
    return NULL;
}

void* dlaed4_(void)
{
    if (verbose) puts("STUB: dlaed4_ called");
    return NULL;
}

void* dlaed5(void)
{
    if (verbose) puts("STUB: dlaed5 called");
    return NULL;
}

void* dlaed5_(void)
{
    if (verbose) puts("STUB: dlaed5_ called");
    return NULL;
}

void* dlaed6(void)
{
    if (verbose) puts("STUB: dlaed6 called");
    return NULL;
}

void* dlaed6_(void)
{
    if (verbose) puts("STUB: dlaed6_ called");
    return NULL;
}

void* dlaed7(void)
{
    if (verbose) puts("STUB: dlaed7 called");
    return NULL;
}

void* dlaed7_(void)
{
    if (verbose) puts("STUB: dlaed7_ called");
    return NULL;
}

void* dlaed8(void)
{
    if (verbose) puts("STUB: dlaed8 called");
    return NULL;
}

void* dlaed8_(void)
{
    if (verbose) puts("STUB: dlaed8_ called");
    return NULL;
}

void* dlaed9(void)
{
    if (verbose) puts("STUB: dlaed9 called");
    return NULL;
}

void* dlaed9_(void)
{
    if (verbose) puts("STUB: dlaed9_ called");
    return NULL;
}

void* dlaeda(void)
{
    if (verbose) puts("STUB: dlaeda called");
    return NULL;
}

void* dlaeda_(void)
{
    if (verbose) puts("STUB: dlaeda_ called");
    return NULL;
}

void* dlaein(void)
{
    if (verbose) puts("STUB: dlaein called");
    return NULL;
}

void* dlaein_(void)
{
    if (verbose) puts("STUB: dlaein_ called");
    return NULL;
}

void* dlaev2(void)
{
    if (verbose) puts("STUB: dlaev2 called");
    return NULL;
}

void* dlaev2_(void)
{
    if (verbose) puts("STUB: dlaev2_ called");
    return NULL;
}

void* dlaexc(void)
{
    if (verbose) puts("STUB: dlaexc called");
    return NULL;
}

void* dlaexc_(void)
{
    if (verbose) puts("STUB: dlaexc_ called");
    return NULL;
}

void* dlag2(void)
{
    if (verbose) puts("STUB: dlag2 called");
    return NULL;
}

void* dlag2_(void)
{
    if (verbose) puts("STUB: dlag2_ called");
    return NULL;
}

void* dlag2s(void)
{
    if (verbose) puts("STUB: dlag2s called");
    return NULL;
}

void* dlag2s_(void)
{
    if (verbose) puts("STUB: dlag2s_ called");
    return NULL;
}

void* dlags2(void)
{
    if (verbose) puts("STUB: dlags2 called");
    return NULL;
}

void* dlags2_(void)
{
    if (verbose) puts("STUB: dlags2_ called");
    return NULL;
}

void* dlagtf(void)
{
    if (verbose) puts("STUB: dlagtf called");
    return NULL;
}

void* dlagtf_(void)
{
    if (verbose) puts("STUB: dlagtf_ called");
    return NULL;
}

void* dlagtm(void)
{
    if (verbose) puts("STUB: dlagtm called");
    return NULL;
}

void* dlagtm_(void)
{
    if (verbose) puts("STUB: dlagtm_ called");
    return NULL;
}

void* dlagts(void)
{
    if (verbose) puts("STUB: dlagts called");
    return NULL;
}

void* dlagts_(void)
{
    if (verbose) puts("STUB: dlagts_ called");
    return NULL;
}

void* dlagv2(void)
{
    if (verbose) puts("STUB: dlagv2 called");
    return NULL;
}

void* dlagv2_(void)
{
    if (verbose) puts("STUB: dlagv2_ called");
    return NULL;
}

void* dlahqr(void)
{
    if (verbose) puts("STUB: dlahqr called");
    return NULL;
}

void* dlahqr_(void)
{
    if (verbose) puts("STUB: dlahqr_ called");
    return NULL;
}

void* dlahr2(void)
{
    if (verbose) puts("STUB: dlahr2 called");
    return NULL;
}

void* dlahr2_(void)
{
    if (verbose) puts("STUB: dlahr2_ called");
    return NULL;
}

void* dlahrd(void)
{
    if (verbose) puts("STUB: dlahrd called");
    return NULL;
}

void* dlahrd_(void)
{
    if (verbose) puts("STUB: dlahrd_ called");
    return NULL;
}

void* dlaic1(void)
{
    if (verbose) puts("STUB: dlaic1 called");
    return NULL;
}

void* dlaic1_(void)
{
    if (verbose) puts("STUB: dlaic1_ called");
    return NULL;
}

void* dlaisnan(void)
{
    if (verbose) puts("STUB: dlaisnan called");
    return NULL;
}

void* dlaisnan_(void)
{
    if (verbose) puts("STUB: dlaisnan_ called");
    return NULL;
}

void* dlaln2(void)
{
    if (verbose) puts("STUB: dlaln2 called");
    return NULL;
}

void* dlaln2_(void)
{
    if (verbose) puts("STUB: dlaln2_ called");
    return NULL;
}

void* dlals0(void)
{
    if (verbose) puts("STUB: dlals0 called");
    return NULL;
}

void* dlals0_(void)
{
    if (verbose) puts("STUB: dlals0_ called");
    return NULL;
}

void* dlalsa(void)
{
    if (verbose) puts("STUB: dlalsa called");
    return NULL;
}

void* dlalsa_(void)
{
    if (verbose) puts("STUB: dlalsa_ called");
    return NULL;
}

void* dlalsd(void)
{
    if (verbose) puts("STUB: dlalsd called");
    return NULL;
}

void* dlalsd_(void)
{
    if (verbose) puts("STUB: dlalsd_ called");
    return NULL;
}

void* dlamc1(void)
{
    if (verbose) puts("STUB: dlamc1 called");
    return NULL;
}

void* dlamc1_(void)
{
    if (verbose) puts("STUB: dlamc1_ called");
    return NULL;
}

void* dlamc2(void)
{
    if (verbose) puts("STUB: dlamc2 called");
    return NULL;
}

void* dlamc2_(void)
{
    if (verbose) puts("STUB: dlamc2_ called");
    return NULL;
}

void* dlamc3(void)
{
    if (verbose) puts("STUB: dlamc3 called");
    return NULL;
}

void* dlamc3_(void)
{
    if (verbose) puts("STUB: dlamc3_ called");
    return NULL;
}

void* dlamc4(void)
{
    if (verbose) puts("STUB: dlamc4 called");
    return NULL;
}

void* dlamc4_(void)
{
    if (verbose) puts("STUB: dlamc4_ called");
    return NULL;
}

void* dlamc5(void)
{
    if (verbose) puts("STUB: dlamc5 called");
    return NULL;
}

void* dlamc5_(void)
{
    if (verbose) puts("STUB: dlamc5_ called");
    return NULL;
}

void* dlamch(void)
{
    if (verbose) puts("STUB: dlamch called");
    return NULL;
}

void* dlamch_(void)
{
    if (verbose) puts("STUB: dlamch_ called");
    return NULL;
}

void* dlamrg(void)
{
    if (verbose) puts("STUB: dlamrg called");
    return NULL;
}

void* dlamrg_(void)
{
    if (verbose) puts("STUB: dlamrg_ called");
    return NULL;
}

void* dlaneg(void)
{
    if (verbose) puts("STUB: dlaneg called");
    return NULL;
}

void* dlaneg_(void)
{
    if (verbose) puts("STUB: dlaneg_ called");
    return NULL;
}

void* dlangb(void)
{
    if (verbose) puts("STUB: dlangb called");
    return NULL;
}

void* dlangb_(void)
{
    if (verbose) puts("STUB: dlangb_ called");
    return NULL;
}

void* dlange(void)
{
    if (verbose) puts("STUB: dlange called");
    return NULL;
}

void* dlange_(void)
{
    if (verbose) puts("STUB: dlange_ called");
    return NULL;
}

void* dlangt(void)
{
    if (verbose) puts("STUB: dlangt called");
    return NULL;
}

void* dlangt_(void)
{
    if (verbose) puts("STUB: dlangt_ called");
    return NULL;
}

void* dlanhs(void)
{
    if (verbose) puts("STUB: dlanhs called");
    return NULL;
}

void* dlanhs_(void)
{
    if (verbose) puts("STUB: dlanhs_ called");
    return NULL;
}

void* dlansb(void)
{
    if (verbose) puts("STUB: dlansb called");
    return NULL;
}

void* dlansb_(void)
{
    if (verbose) puts("STUB: dlansb_ called");
    return NULL;
}

void* dlansf(void)
{
    if (verbose) puts("STUB: dlansf called");
    return NULL;
}

void* dlansf_(void)
{
    if (verbose) puts("STUB: dlansf_ called");
    return NULL;
}

void* dlansp(void)
{
    if (verbose) puts("STUB: dlansp called");
    return NULL;
}

void* dlansp_(void)
{
    if (verbose) puts("STUB: dlansp_ called");
    return NULL;
}

void* dlanst(void)
{
    if (verbose) puts("STUB: dlanst called");
    return NULL;
}

void* dlanst_(void)
{
    if (verbose) puts("STUB: dlanst_ called");
    return NULL;
}

void* dlansy(void)
{
    if (verbose) puts("STUB: dlansy called");
    return NULL;
}

void* dlansy_(void)
{
    if (verbose) puts("STUB: dlansy_ called");
    return NULL;
}

void* dlantb(void)
{
    if (verbose) puts("STUB: dlantb called");
    return NULL;
}

void* dlantb_(void)
{
    if (verbose) puts("STUB: dlantb_ called");
    return NULL;
}

void* dlantp(void)
{
    if (verbose) puts("STUB: dlantp called");
    return NULL;
}

void* dlantp_(void)
{
    if (verbose) puts("STUB: dlantp_ called");
    return NULL;
}

void* dlantr(void)
{
    if (verbose) puts("STUB: dlantr called");
    return NULL;
}

void* dlantr_(void)
{
    if (verbose) puts("STUB: dlantr_ called");
    return NULL;
}

void* dlanv2(void)
{
    if (verbose) puts("STUB: dlanv2 called");
    return NULL;
}

void* dlanv2_(void)
{
    if (verbose) puts("STUB: dlanv2_ called");
    return NULL;
}

void* dlapll(void)
{
    if (verbose) puts("STUB: dlapll called");
    return NULL;
}

void* dlapll_(void)
{
    if (verbose) puts("STUB: dlapll_ called");
    return NULL;
}

void* dlapmt(void)
{
    if (verbose) puts("STUB: dlapmt called");
    return NULL;
}

void* dlapmt_(void)
{
    if (verbose) puts("STUB: dlapmt_ called");
    return NULL;
}

void* dlapy2(void)
{
    if (verbose) puts("STUB: dlapy2 called");
    return NULL;
}

void* dlapy2_(void)
{
    if (verbose) puts("STUB: dlapy2_ called");
    return NULL;
}

void* dlapy3(void)
{
    if (verbose) puts("STUB: dlapy3 called");
    return NULL;
}

void* dlapy3_(void)
{
    if (verbose) puts("STUB: dlapy3_ called");
    return NULL;
}

void* dlaqgb(void)
{
    if (verbose) puts("STUB: dlaqgb called");
    return NULL;
}

void* dlaqgb_(void)
{
    if (verbose) puts("STUB: dlaqgb_ called");
    return NULL;
}

void* dlaqge(void)
{
    if (verbose) puts("STUB: dlaqge called");
    return NULL;
}

void* dlaqge_(void)
{
    if (verbose) puts("STUB: dlaqge_ called");
    return NULL;
}

void* dlaqp2(void)
{
    if (verbose) puts("STUB: dlaqp2 called");
    return NULL;
}

void* dlaqp2_(void)
{
    if (verbose) puts("STUB: dlaqp2_ called");
    return NULL;
}

void* dlaqps(void)
{
    if (verbose) puts("STUB: dlaqps called");
    return NULL;
}

void* dlaqps_(void)
{
    if (verbose) puts("STUB: dlaqps_ called");
    return NULL;
}

void* dlaqr0(void)
{
    if (verbose) puts("STUB: dlaqr0 called");
    return NULL;
}

void* dlaqr0_(void)
{
    if (verbose) puts("STUB: dlaqr0_ called");
    return NULL;
}

void* dlaqr1(void)
{
    if (verbose) puts("STUB: dlaqr1 called");
    return NULL;
}

void* dlaqr1_(void)
{
    if (verbose) puts("STUB: dlaqr1_ called");
    return NULL;
}

void* dlaqr2(void)
{
    if (verbose) puts("STUB: dlaqr2 called");
    return NULL;
}

void* dlaqr2_(void)
{
    if (verbose) puts("STUB: dlaqr2_ called");
    return NULL;
}

void* dlaqr3(void)
{
    if (verbose) puts("STUB: dlaqr3 called");
    return NULL;
}

void* dlaqr3_(void)
{
    if (verbose) puts("STUB: dlaqr3_ called");
    return NULL;
}

void* dlaqr4(void)
{
    if (verbose) puts("STUB: dlaqr4 called");
    return NULL;
}

void* dlaqr4_(void)
{
    if (verbose) puts("STUB: dlaqr4_ called");
    return NULL;
}

void* dlaqr5(void)
{
    if (verbose) puts("STUB: dlaqr5 called");
    return NULL;
}

void* dlaqr5_(void)
{
    if (verbose) puts("STUB: dlaqr5_ called");
    return NULL;
}

void* dlaqsb(void)
{
    if (verbose) puts("STUB: dlaqsb called");
    return NULL;
}

void* dlaqsb_(void)
{
    if (verbose) puts("STUB: dlaqsb_ called");
    return NULL;
}

void* dlaqsp(void)
{
    if (verbose) puts("STUB: dlaqsp called");
    return NULL;
}

void* dlaqsp_(void)
{
    if (verbose) puts("STUB: dlaqsp_ called");
    return NULL;
}

void* dlaqsy(void)
{
    if (verbose) puts("STUB: dlaqsy called");
    return NULL;
}

void* dlaqsy_(void)
{
    if (verbose) puts("STUB: dlaqsy_ called");
    return NULL;
}

void* dlaqtr(void)
{
    if (verbose) puts("STUB: dlaqtr called");
    return NULL;
}

void* dlaqtr_(void)
{
    if (verbose) puts("STUB: dlaqtr_ called");
    return NULL;
}

void* dlar1v(void)
{
    if (verbose) puts("STUB: dlar1v called");
    return NULL;
}

void* dlar1v_(void)
{
    if (verbose) puts("STUB: dlar1v_ called");
    return NULL;
}

void* dlar2v(void)
{
    if (verbose) puts("STUB: dlar2v called");
    return NULL;
}

void* dlar2v_(void)
{
    if (verbose) puts("STUB: dlar2v_ called");
    return NULL;
}

void* dlarf(void)
{
    if (verbose) puts("STUB: dlarf called");
    return NULL;
}

void* dlarf_(void)
{
    if (verbose) puts("STUB: dlarf_ called");
    return NULL;
}

void* dlarfb(void)
{
    if (verbose) puts("STUB: dlarfb called");
    return NULL;
}

void* dlarfb_(void)
{
    if (verbose) puts("STUB: dlarfb_ called");
    return NULL;
}

void* dlarfg(void)
{
    if (verbose) puts("STUB: dlarfg called");
    return NULL;
}

void* dlarfg_(void)
{
    if (verbose) puts("STUB: dlarfg_ called");
    return NULL;
}

void* dlarfp(void)
{
    if (verbose) puts("STUB: dlarfp called");
    return NULL;
}

void* dlarfp_(void)
{
    if (verbose) puts("STUB: dlarfp_ called");
    return NULL;
}

void* dlarft(void)
{
    if (verbose) puts("STUB: dlarft called");
    return NULL;
}

void* dlarft_(void)
{
    if (verbose) puts("STUB: dlarft_ called");
    return NULL;
}

void* dlarfx(void)
{
    if (verbose) puts("STUB: dlarfx called");
    return NULL;
}

void* dlarfx_(void)
{
    if (verbose) puts("STUB: dlarfx_ called");
    return NULL;
}

void* dlargv(void)
{
    if (verbose) puts("STUB: dlargv called");
    return NULL;
}

void* dlargv_(void)
{
    if (verbose) puts("STUB: dlargv_ called");
    return NULL;
}

void* dlarnv(void)
{
    if (verbose) puts("STUB: dlarnv called");
    return NULL;
}

void* dlarnv_(void)
{
    if (verbose) puts("STUB: dlarnv_ called");
    return NULL;
}

void* dlarra(void)
{
    if (verbose) puts("STUB: dlarra called");
    return NULL;
}

void* dlarra_(void)
{
    if (verbose) puts("STUB: dlarra_ called");
    return NULL;
}

void* dlarrb(void)
{
    if (verbose) puts("STUB: dlarrb called");
    return NULL;
}

void* dlarrb_(void)
{
    if (verbose) puts("STUB: dlarrb_ called");
    return NULL;
}

void* dlarrc(void)
{
    if (verbose) puts("STUB: dlarrc called");
    return NULL;
}

void* dlarrc_(void)
{
    if (verbose) puts("STUB: dlarrc_ called");
    return NULL;
}

void* dlarrd(void)
{
    if (verbose) puts("STUB: dlarrd called");
    return NULL;
}

void* dlarrd_(void)
{
    if (verbose) puts("STUB: dlarrd_ called");
    return NULL;
}

void* dlarre(void)
{
    if (verbose) puts("STUB: dlarre called");
    return NULL;
}

void* dlarre_(void)
{
    if (verbose) puts("STUB: dlarre_ called");
    return NULL;
}

void* dlarrf(void)
{
    if (verbose) puts("STUB: dlarrf called");
    return NULL;
}

void* dlarrf_(void)
{
    if (verbose) puts("STUB: dlarrf_ called");
    return NULL;
}

void* dlarrj(void)
{
    if (verbose) puts("STUB: dlarrj called");
    return NULL;
}

void* dlarrj_(void)
{
    if (verbose) puts("STUB: dlarrj_ called");
    return NULL;
}

void* dlarrk(void)
{
    if (verbose) puts("STUB: dlarrk called");
    return NULL;
}

void* dlarrk_(void)
{
    if (verbose) puts("STUB: dlarrk_ called");
    return NULL;
}

void* dlarrr(void)
{
    if (verbose) puts("STUB: dlarrr called");
    return NULL;
}

void* dlarrr_(void)
{
    if (verbose) puts("STUB: dlarrr_ called");
    return NULL;
}

void* dlarrv(void)
{
    if (verbose) puts("STUB: dlarrv called");
    return NULL;
}

void* dlarrv_(void)
{
    if (verbose) puts("STUB: dlarrv_ called");
    return NULL;
}

void* dlarscl2(void)
{
    if (verbose) puts("STUB: dlarscl2 called");
    return NULL;
}

void* dlarscl2_(void)
{
    if (verbose) puts("STUB: dlarscl2_ called");
    return NULL;
}

void* dlartg(void)
{
    if (verbose) puts("STUB: dlartg called");
    return NULL;
}

void* dlartg_(void)
{
    if (verbose) puts("STUB: dlartg_ called");
    return NULL;
}

void* dlartv(void)
{
    if (verbose) puts("STUB: dlartv called");
    return NULL;
}

void* dlartv_(void)
{
    if (verbose) puts("STUB: dlartv_ called");
    return NULL;
}

void* dlaruv(void)
{
    if (verbose) puts("STUB: dlaruv called");
    return NULL;
}

void* dlaruv_(void)
{
    if (verbose) puts("STUB: dlaruv_ called");
    return NULL;
}

void* dlarz(void)
{
    if (verbose) puts("STUB: dlarz called");
    return NULL;
}

void* dlarz_(void)
{
    if (verbose) puts("STUB: dlarz_ called");
    return NULL;
}

void* dlarzb(void)
{
    if (verbose) puts("STUB: dlarzb called");
    return NULL;
}

void* dlarzb_(void)
{
    if (verbose) puts("STUB: dlarzb_ called");
    return NULL;
}

void* dlarzt(void)
{
    if (verbose) puts("STUB: dlarzt called");
    return NULL;
}

void* dlarzt_(void)
{
    if (verbose) puts("STUB: dlarzt_ called");
    return NULL;
}

void* dlas2(void)
{
    if (verbose) puts("STUB: dlas2 called");
    return NULL;
}

void* dlas2_(void)
{
    if (verbose) puts("STUB: dlas2_ called");
    return NULL;
}

void* dlascl(void)
{
    if (verbose) puts("STUB: dlascl called");
    return NULL;
}

void* dlascl2(void)
{
    if (verbose) puts("STUB: dlascl2 called");
    return NULL;
}

void* dlascl2_(void)
{
    if (verbose) puts("STUB: dlascl2_ called");
    return NULL;
}

void* dlascl_(void)
{
    if (verbose) puts("STUB: dlascl_ called");
    return NULL;
}

void* dlasd0(void)
{
    if (verbose) puts("STUB: dlasd0 called");
    return NULL;
}

void* dlasd0_(void)
{
    if (verbose) puts("STUB: dlasd0_ called");
    return NULL;
}

void* dlasd1(void)
{
    if (verbose) puts("STUB: dlasd1 called");
    return NULL;
}

void* dlasd1_(void)
{
    if (verbose) puts("STUB: dlasd1_ called");
    return NULL;
}

void* dlasd2(void)
{
    if (verbose) puts("STUB: dlasd2 called");
    return NULL;
}

void* dlasd2_(void)
{
    if (verbose) puts("STUB: dlasd2_ called");
    return NULL;
}

void* dlasd3(void)
{
    if (verbose) puts("STUB: dlasd3 called");
    return NULL;
}

void* dlasd3_(void)
{
    if (verbose) puts("STUB: dlasd3_ called");
    return NULL;
}

void* dlasd4(void)
{
    if (verbose) puts("STUB: dlasd4 called");
    return NULL;
}

void* dlasd4_(void)
{
    if (verbose) puts("STUB: dlasd4_ called");
    return NULL;
}

void* dlasd5(void)
{
    if (verbose) puts("STUB: dlasd5 called");
    return NULL;
}

void* dlasd5_(void)
{
    if (verbose) puts("STUB: dlasd5_ called");
    return NULL;
}

void* dlasd6(void)
{
    if (verbose) puts("STUB: dlasd6 called");
    return NULL;
}

void* dlasd6_(void)
{
    if (verbose) puts("STUB: dlasd6_ called");
    return NULL;
}

void* dlasd7(void)
{
    if (verbose) puts("STUB: dlasd7 called");
    return NULL;
}

void* dlasd7_(void)
{
    if (verbose) puts("STUB: dlasd7_ called");
    return NULL;
}

void* dlasd8(void)
{
    if (verbose) puts("STUB: dlasd8 called");
    return NULL;
}

void* dlasd8_(void)
{
    if (verbose) puts("STUB: dlasd8_ called");
    return NULL;
}

void* dlasd9(void)
{
    if (verbose) puts("STUB: dlasd9 called");
    return NULL;
}

void* dlasd9_(void)
{
    if (verbose) puts("STUB: dlasd9_ called");
    return NULL;
}

void* dlasda(void)
{
    if (verbose) puts("STUB: dlasda called");
    return NULL;
}

void* dlasda_(void)
{
    if (verbose) puts("STUB: dlasda_ called");
    return NULL;
}

void* dlasdq(void)
{
    if (verbose) puts("STUB: dlasdq called");
    return NULL;
}

void* dlasdq_(void)
{
    if (verbose) puts("STUB: dlasdq_ called");
    return NULL;
}

void* dlasdt(void)
{
    if (verbose) puts("STUB: dlasdt called");
    return NULL;
}

void* dlasdt_(void)
{
    if (verbose) puts("STUB: dlasdt_ called");
    return NULL;
}

void* dlaset(void)
{
    if (verbose) puts("STUB: dlaset called");
    return NULL;
}

void* dlaset_(void)
{
    if (verbose) puts("STUB: dlaset_ called");
    return NULL;
}

void* dlasq1(void)
{
    if (verbose) puts("STUB: dlasq1 called");
    return NULL;
}

void* dlasq1_(void)
{
    if (verbose) puts("STUB: dlasq1_ called");
    return NULL;
}

void* dlasq2(void)
{
    if (verbose) puts("STUB: dlasq2 called");
    return NULL;
}

void* dlasq2_(void)
{
    if (verbose) puts("STUB: dlasq2_ called");
    return NULL;
}

void* dlasq3(void)
{
    if (verbose) puts("STUB: dlasq3 called");
    return NULL;
}

void* dlasq3_(void)
{
    if (verbose) puts("STUB: dlasq3_ called");
    return NULL;
}

void* dlasq4(void)
{
    if (verbose) puts("STUB: dlasq4 called");
    return NULL;
}

void* dlasq4_(void)
{
    if (verbose) puts("STUB: dlasq4_ called");
    return NULL;
}

void* dlasq5(void)
{
    if (verbose) puts("STUB: dlasq5 called");
    return NULL;
}

void* dlasq5_(void)
{
    if (verbose) puts("STUB: dlasq5_ called");
    return NULL;
}

void* dlasq6(void)
{
    if (verbose) puts("STUB: dlasq6 called");
    return NULL;
}

void* dlasq6_(void)
{
    if (verbose) puts("STUB: dlasq6_ called");
    return NULL;
}

void* dlasr(void)
{
    if (verbose) puts("STUB: dlasr called");
    return NULL;
}

void* dlasr_(void)
{
    if (verbose) puts("STUB: dlasr_ called");
    return NULL;
}

void* dlasrt(void)
{
    if (verbose) puts("STUB: dlasrt called");
    return NULL;
}

void* dlasrt_(void)
{
    if (verbose) puts("STUB: dlasrt_ called");
    return NULL;
}

void* dlassq(void)
{
    if (verbose) puts("STUB: dlassq called");
    return NULL;
}

void* dlassq_(void)
{
    if (verbose) puts("STUB: dlassq_ called");
    return NULL;
}

void* dlasv2(void)
{
    if (verbose) puts("STUB: dlasv2 called");
    return NULL;
}

void* dlasv2_(void)
{
    if (verbose) puts("STUB: dlasv2_ called");
    return NULL;
}

void* dlaswp(void)
{
    if (verbose) puts("STUB: dlaswp called");
    return NULL;
}

void* dlaswp_(void)
{
    if (verbose) puts("STUB: dlaswp_ called");
    return NULL;
}

void* dlasy2(void)
{
    if (verbose) puts("STUB: dlasy2 called");
    return NULL;
}

void* dlasy2_(void)
{
    if (verbose) puts("STUB: dlasy2_ called");
    return NULL;
}

void* dlasyf(void)
{
    if (verbose) puts("STUB: dlasyf called");
    return NULL;
}

void* dlasyf_(void)
{
    if (verbose) puts("STUB: dlasyf_ called");
    return NULL;
}

void* dlat2s(void)
{
    if (verbose) puts("STUB: dlat2s called");
    return NULL;
}

void* dlat2s_(void)
{
    if (verbose) puts("STUB: dlat2s_ called");
    return NULL;
}

void* dlatbs(void)
{
    if (verbose) puts("STUB: dlatbs called");
    return NULL;
}

void* dlatbs_(void)
{
    if (verbose) puts("STUB: dlatbs_ called");
    return NULL;
}

void* dlatdf(void)
{
    if (verbose) puts("STUB: dlatdf called");
    return NULL;
}

void* dlatdf_(void)
{
    if (verbose) puts("STUB: dlatdf_ called");
    return NULL;
}

void* dlatps(void)
{
    if (verbose) puts("STUB: dlatps called");
    return NULL;
}

void* dlatps_(void)
{
    if (verbose) puts("STUB: dlatps_ called");
    return NULL;
}

void* dlatrd(void)
{
    if (verbose) puts("STUB: dlatrd called");
    return NULL;
}

void* dlatrd_(void)
{
    if (verbose) puts("STUB: dlatrd_ called");
    return NULL;
}

void* dlatrs(void)
{
    if (verbose) puts("STUB: dlatrs called");
    return NULL;
}

void* dlatrs_(void)
{
    if (verbose) puts("STUB: dlatrs_ called");
    return NULL;
}

void* dlatrz(void)
{
    if (verbose) puts("STUB: dlatrz called");
    return NULL;
}

void* dlatrz_(void)
{
    if (verbose) puts("STUB: dlatrz_ called");
    return NULL;
}

void* dlatzm(void)
{
    if (verbose) puts("STUB: dlatzm called");
    return NULL;
}

void* dlatzm_(void)
{
    if (verbose) puts("STUB: dlatzm_ called");
    return NULL;
}

void* dlauu2(void)
{
    if (verbose) puts("STUB: dlauu2 called");
    return NULL;
}

void* dlauu2_(void)
{
    if (verbose) puts("STUB: dlauu2_ called");
    return NULL;
}

void* dlauum(void)
{
    if (verbose) puts("STUB: dlauum called");
    return NULL;
}

void* dlauum_(void)
{
    if (verbose) puts("STUB: dlauum_ called");
    return NULL;
}

void* dopgtr(void)
{
    if (verbose) puts("STUB: dopgtr called");
    return NULL;
}

void* dopgtr_(void)
{
    if (verbose) puts("STUB: dopgtr_ called");
    return NULL;
}

void* dopmtr(void)
{
    if (verbose) puts("STUB: dopmtr called");
    return NULL;
}

void* dopmtr_(void)
{
    if (verbose) puts("STUB: dopmtr_ called");
    return NULL;
}

void* dorg2l(void)
{
    if (verbose) puts("STUB: dorg2l called");
    return NULL;
}

void* dorg2l_(void)
{
    if (verbose) puts("STUB: dorg2l_ called");
    return NULL;
}

void* dorg2r(void)
{
    if (verbose) puts("STUB: dorg2r called");
    return NULL;
}

void* dorg2r_(void)
{
    if (verbose) puts("STUB: dorg2r_ called");
    return NULL;
}

void* dorgbr(void)
{
    if (verbose) puts("STUB: dorgbr called");
    return NULL;
}

void* dorgbr_(void)
{
    if (verbose) puts("STUB: dorgbr_ called");
    return NULL;
}

void* dorghr(void)
{
    if (verbose) puts("STUB: dorghr called");
    return NULL;
}

void* dorghr_(void)
{
    if (verbose) puts("STUB: dorghr_ called");
    return NULL;
}

void* dorgl2(void)
{
    if (verbose) puts("STUB: dorgl2 called");
    return NULL;
}

void* dorgl2_(void)
{
    if (verbose) puts("STUB: dorgl2_ called");
    return NULL;
}

void* dorglq(void)
{
    if (verbose) puts("STUB: dorglq called");
    return NULL;
}

void* dorglq_(void)
{
    if (verbose) puts("STUB: dorglq_ called");
    return NULL;
}

void* dorgql(void)
{
    if (verbose) puts("STUB: dorgql called");
    return NULL;
}

void* dorgql_(void)
{
    if (verbose) puts("STUB: dorgql_ called");
    return NULL;
}

void* dorgqr(void)
{
    if (verbose) puts("STUB: dorgqr called");
    return NULL;
}

void* dorgqr_(void)
{
    if (verbose) puts("STUB: dorgqr_ called");
    return NULL;
}

void* dorgr2(void)
{
    if (verbose) puts("STUB: dorgr2 called");
    return NULL;
}

void* dorgr2_(void)
{
    if (verbose) puts("STUB: dorgr2_ called");
    return NULL;
}

void* dorgrq(void)
{
    if (verbose) puts("STUB: dorgrq called");
    return NULL;
}

void* dorgrq_(void)
{
    if (verbose) puts("STUB: dorgrq_ called");
    return NULL;
}

void* dorgtr(void)
{
    if (verbose) puts("STUB: dorgtr called");
    return NULL;
}

void* dorgtr_(void)
{
    if (verbose) puts("STUB: dorgtr_ called");
    return NULL;
}

void* dorm2l(void)
{
    if (verbose) puts("STUB: dorm2l called");
    return NULL;
}

void* dorm2l_(void)
{
    if (verbose) puts("STUB: dorm2l_ called");
    return NULL;
}

void* dorm2r(void)
{
    if (verbose) puts("STUB: dorm2r called");
    return NULL;
}

void* dorm2r_(void)
{
    if (verbose) puts("STUB: dorm2r_ called");
    return NULL;
}

void* dormbr(void)
{
    if (verbose) puts("STUB: dormbr called");
    return NULL;
}

void* dormbr_(void)
{
    if (verbose) puts("STUB: dormbr_ called");
    return NULL;
}

void* dormhr(void)
{
    if (verbose) puts("STUB: dormhr called");
    return NULL;
}

void* dormhr_(void)
{
    if (verbose) puts("STUB: dormhr_ called");
    return NULL;
}

void* dorml2(void)
{
    if (verbose) puts("STUB: dorml2 called");
    return NULL;
}

void* dorml2_(void)
{
    if (verbose) puts("STUB: dorml2_ called");
    return NULL;
}

void* dormlq(void)
{
    if (verbose) puts("STUB: dormlq called");
    return NULL;
}

void* dormlq_(void)
{
    if (verbose) puts("STUB: dormlq_ called");
    return NULL;
}

void* dormql(void)
{
    if (verbose) puts("STUB: dormql called");
    return NULL;
}

void* dormql_(void)
{
    if (verbose) puts("STUB: dormql_ called");
    return NULL;
}

void* dormqr(void)
{
    if (verbose) puts("STUB: dormqr called");
    return NULL;
}

void* dormqr_(void)
{
    if (verbose) puts("STUB: dormqr_ called");
    return NULL;
}

void* dormr2(void)
{
    if (verbose) puts("STUB: dormr2 called");
    return NULL;
}

void* dormr2_(void)
{
    if (verbose) puts("STUB: dormr2_ called");
    return NULL;
}

void* dormr3(void)
{
    if (verbose) puts("STUB: dormr3 called");
    return NULL;
}

void* dormr3_(void)
{
    if (verbose) puts("STUB: dormr3_ called");
    return NULL;
}

void* dormrq(void)
{
    if (verbose) puts("STUB: dormrq called");
    return NULL;
}

void* dormrq_(void)
{
    if (verbose) puts("STUB: dormrq_ called");
    return NULL;
}

void* dormrz(void)
{
    if (verbose) puts("STUB: dormrz called");
    return NULL;
}

void* dormrz_(void)
{
    if (verbose) puts("STUB: dormrz_ called");
    return NULL;
}

void* dormtr(void)
{
    if (verbose) puts("STUB: dormtr called");
    return NULL;
}

void* dormtr_(void)
{
    if (verbose) puts("STUB: dormtr_ called");
    return NULL;
}

void* dpbcon(void)
{
    if (verbose) puts("STUB: dpbcon called");
    return NULL;
}

void* dpbcon_(void)
{
    if (verbose) puts("STUB: dpbcon_ called");
    return NULL;
}

void* dpbequ(void)
{
    if (verbose) puts("STUB: dpbequ called");
    return NULL;
}

void* dpbequ_(void)
{
    if (verbose) puts("STUB: dpbequ_ called");
    return NULL;
}

void* dpbrfs(void)
{
    if (verbose) puts("STUB: dpbrfs called");
    return NULL;
}

void* dpbrfs_(void)
{
    if (verbose) puts("STUB: dpbrfs_ called");
    return NULL;
}

void* dpbstf(void)
{
    if (verbose) puts("STUB: dpbstf called");
    return NULL;
}

void* dpbstf_(void)
{
    if (verbose) puts("STUB: dpbstf_ called");
    return NULL;
}

void* dpbsv(void)
{
    if (verbose) puts("STUB: dpbsv called");
    return NULL;
}

void* dpbsv_(void)
{
    if (verbose) puts("STUB: dpbsv_ called");
    return NULL;
}

void* dpbsvx(void)
{
    if (verbose) puts("STUB: dpbsvx called");
    return NULL;
}

void* dpbsvx_(void)
{
    if (verbose) puts("STUB: dpbsvx_ called");
    return NULL;
}

void* dpbtf2(void)
{
    if (verbose) puts("STUB: dpbtf2 called");
    return NULL;
}

void* dpbtf2_(void)
{
    if (verbose) puts("STUB: dpbtf2_ called");
    return NULL;
}

void* dpbtrf(void)
{
    if (verbose) puts("STUB: dpbtrf called");
    return NULL;
}

void* dpbtrf_(void)
{
    if (verbose) puts("STUB: dpbtrf_ called");
    return NULL;
}

void* dpbtrs(void)
{
    if (verbose) puts("STUB: dpbtrs called");
    return NULL;
}

void* dpbtrs_(void)
{
    if (verbose) puts("STUB: dpbtrs_ called");
    return NULL;
}

void* dpftrf(void)
{
    if (verbose) puts("STUB: dpftrf called");
    return NULL;
}

void* dpftrf_(void)
{
    if (verbose) puts("STUB: dpftrf_ called");
    return NULL;
}

void* dpftri(void)
{
    if (verbose) puts("STUB: dpftri called");
    return NULL;
}

void* dpftri_(void)
{
    if (verbose) puts("STUB: dpftri_ called");
    return NULL;
}

void* dpftrs(void)
{
    if (verbose) puts("STUB: dpftrs called");
    return NULL;
}

void* dpftrs_(void)
{
    if (verbose) puts("STUB: dpftrs_ called");
    return NULL;
}

void* dpocon(void)
{
    if (verbose) puts("STUB: dpocon called");
    return NULL;
}

void* dpocon_(void)
{
    if (verbose) puts("STUB: dpocon_ called");
    return NULL;
}

void* dpoequ(void)
{
    if (verbose) puts("STUB: dpoequ called");
    return NULL;
}

void* dpoequ_(void)
{
    if (verbose) puts("STUB: dpoequ_ called");
    return NULL;
}

void* dpoequb(void)
{
    if (verbose) puts("STUB: dpoequb called");
    return NULL;
}

void* dpoequb_(void)
{
    if (verbose) puts("STUB: dpoequb_ called");
    return NULL;
}

void* dporfs(void)
{
    if (verbose) puts("STUB: dporfs called");
    return NULL;
}

void* dporfs_(void)
{
    if (verbose) puts("STUB: dporfs_ called");
    return NULL;
}

void* dposv(void)
{
    if (verbose) puts("STUB: dposv called");
    return NULL;
}

void* dposv_(void)
{
    if (verbose) puts("STUB: dposv_ called");
    return NULL;
}

void* dposvx(void)
{
    if (verbose) puts("STUB: dposvx called");
    return NULL;
}

void* dposvx_(void)
{
    if (verbose) puts("STUB: dposvx_ called");
    return NULL;
}

void* dpotf2(void)
{
    if (verbose) puts("STUB: dpotf2 called");
    return NULL;
}

void* dpotf2_(void)
{
    if (verbose) puts("STUB: dpotf2_ called");
    return NULL;
}

void* dpotrf(void)
{
    if (verbose) puts("STUB: dpotrf called");
    return NULL;
}

void* dpotrf_(void)
{
    if (verbose) puts("STUB: dpotrf_ called");
    return NULL;
}

void* dpotri(void)
{
    if (verbose) puts("STUB: dpotri called");
    return NULL;
}

void* dpotri_(void)
{
    if (verbose) puts("STUB: dpotri_ called");
    return NULL;
}

void* dpotrs(void)
{
    if (verbose) puts("STUB: dpotrs called");
    return NULL;
}

void* dpotrs_(void)
{
    if (verbose) puts("STUB: dpotrs_ called");
    return NULL;
}

void* dppcon(void)
{
    if (verbose) puts("STUB: dppcon called");
    return NULL;
}

void* dppcon_(void)
{
    if (verbose) puts("STUB: dppcon_ called");
    return NULL;
}

void* dppequ(void)
{
    if (verbose) puts("STUB: dppequ called");
    return NULL;
}

void* dppequ_(void)
{
    if (verbose) puts("STUB: dppequ_ called");
    return NULL;
}

void* dpprfs(void)
{
    if (verbose) puts("STUB: dpprfs called");
    return NULL;
}

void* dpprfs_(void)
{
    if (verbose) puts("STUB: dpprfs_ called");
    return NULL;
}

void* dppsv(void)
{
    if (verbose) puts("STUB: dppsv called");
    return NULL;
}

void* dppsv_(void)
{
    if (verbose) puts("STUB: dppsv_ called");
    return NULL;
}

void* dppsvx(void)
{
    if (verbose) puts("STUB: dppsvx called");
    return NULL;
}

void* dppsvx_(void)
{
    if (verbose) puts("STUB: dppsvx_ called");
    return NULL;
}

void* dpptrf(void)
{
    if (verbose) puts("STUB: dpptrf called");
    return NULL;
}

void* dpptrf_(void)
{
    if (verbose) puts("STUB: dpptrf_ called");
    return NULL;
}

void* dpptri(void)
{
    if (verbose) puts("STUB: dpptri called");
    return NULL;
}

void* dpptri_(void)
{
    if (verbose) puts("STUB: dpptri_ called");
    return NULL;
}

void* dpptrs(void)
{
    if (verbose) puts("STUB: dpptrs called");
    return NULL;
}

void* dpptrs_(void)
{
    if (verbose) puts("STUB: dpptrs_ called");
    return NULL;
}

void* dpstf2(void)
{
    if (verbose) puts("STUB: dpstf2 called");
    return NULL;
}

void* dpstf2_(void)
{
    if (verbose) puts("STUB: dpstf2_ called");
    return NULL;
}

void* dpstrf(void)
{
    if (verbose) puts("STUB: dpstrf called");
    return NULL;
}

void* dpstrf_(void)
{
    if (verbose) puts("STUB: dpstrf_ called");
    return NULL;
}

void* dptcon(void)
{
    if (verbose) puts("STUB: dptcon called");
    return NULL;
}

void* dptcon_(void)
{
    if (verbose) puts("STUB: dptcon_ called");
    return NULL;
}

void* dpteqr(void)
{
    if (verbose) puts("STUB: dpteqr called");
    return NULL;
}

void* dpteqr_(void)
{
    if (verbose) puts("STUB: dpteqr_ called");
    return NULL;
}

void* dptrfs(void)
{
    if (verbose) puts("STUB: dptrfs called");
    return NULL;
}

void* dptrfs_(void)
{
    if (verbose) puts("STUB: dptrfs_ called");
    return NULL;
}

void* dptsv(void)
{
    if (verbose) puts("STUB: dptsv called");
    return NULL;
}

void* dptsv_(void)
{
    if (verbose) puts("STUB: dptsv_ called");
    return NULL;
}

void* dptsvx(void)
{
    if (verbose) puts("STUB: dptsvx called");
    return NULL;
}

void* dptsvx_(void)
{
    if (verbose) puts("STUB: dptsvx_ called");
    return NULL;
}

void* dpttrf(void)
{
    if (verbose) puts("STUB: dpttrf called");
    return NULL;
}

void* dpttrf_(void)
{
    if (verbose) puts("STUB: dpttrf_ called");
    return NULL;
}

void* dpttrs(void)
{
    if (verbose) puts("STUB: dpttrs called");
    return NULL;
}

void* dpttrs_(void)
{
    if (verbose) puts("STUB: dpttrs_ called");
    return NULL;
}

void* dptts2(void)
{
    if (verbose) puts("STUB: dptts2 called");
    return NULL;
}

void* dptts2_(void)
{
    if (verbose) puts("STUB: dptts2_ called");
    return NULL;
}

void* drscl(void)
{
    if (verbose) puts("STUB: drscl called");
    return NULL;
}

void* drscl_(void)
{
    if (verbose) puts("STUB: drscl_ called");
    return NULL;
}

void* dsbev(void)
{
    if (verbose) puts("STUB: dsbev called");
    return NULL;
}

void* dsbev_(void)
{
    if (verbose) puts("STUB: dsbev_ called");
    return NULL;
}

void* dsbevd(void)
{
    if (verbose) puts("STUB: dsbevd called");
    return NULL;
}

void* dsbevd_(void)
{
    if (verbose) puts("STUB: dsbevd_ called");
    return NULL;
}

void* dsbevx(void)
{
    if (verbose) puts("STUB: dsbevx called");
    return NULL;
}

void* dsbevx_(void)
{
    if (verbose) puts("STUB: dsbevx_ called");
    return NULL;
}

void* dsbgst(void)
{
    if (verbose) puts("STUB: dsbgst called");
    return NULL;
}

void* dsbgst_(void)
{
    if (verbose) puts("STUB: dsbgst_ called");
    return NULL;
}

void* dsbgv(void)
{
    if (verbose) puts("STUB: dsbgv called");
    return NULL;
}

void* dsbgv_(void)
{
    if (verbose) puts("STUB: dsbgv_ called");
    return NULL;
}

void* dsbgvd(void)
{
    if (verbose) puts("STUB: dsbgvd called");
    return NULL;
}

void* dsbgvd_(void)
{
    if (verbose) puts("STUB: dsbgvd_ called");
    return NULL;
}

void* dsbgvx(void)
{
    if (verbose) puts("STUB: dsbgvx called");
    return NULL;
}

void* dsbgvx_(void)
{
    if (verbose) puts("STUB: dsbgvx_ called");
    return NULL;
}

void* dsbtrd(void)
{
    if (verbose) puts("STUB: dsbtrd called");
    return NULL;
}

void* dsbtrd_(void)
{
    if (verbose) puts("STUB: dsbtrd_ called");
    return NULL;
}

void* dsfrk(void)
{
    if (verbose) puts("STUB: dsfrk called");
    return NULL;
}

void* dsfrk_(void)
{
    if (verbose) puts("STUB: dsfrk_ called");
    return NULL;
}

void* dsgesv(void)
{
    if (verbose) puts("STUB: dsgesv called");
    return NULL;
}

void* dsgesv_(void)
{
    if (verbose) puts("STUB: dsgesv_ called");
    return NULL;
}

void* dspcon(void)
{
    if (verbose) puts("STUB: dspcon called");
    return NULL;
}

void* dspcon_(void)
{
    if (verbose) puts("STUB: dspcon_ called");
    return NULL;
}

void* dspev(void)
{
    if (verbose) puts("STUB: dspev called");
    return NULL;
}

void* dspev_(void)
{
    if (verbose) puts("STUB: dspev_ called");
    return NULL;
}

void* dspevd(void)
{
    if (verbose) puts("STUB: dspevd called");
    return NULL;
}

void* dspevd_(void)
{
    if (verbose) puts("STUB: dspevd_ called");
    return NULL;
}

void* dspevx(void)
{
    if (verbose) puts("STUB: dspevx called");
    return NULL;
}

void* dspevx_(void)
{
    if (verbose) puts("STUB: dspevx_ called");
    return NULL;
}

void* dspgst(void)
{
    if (verbose) puts("STUB: dspgst called");
    return NULL;
}

void* dspgst_(void)
{
    if (verbose) puts("STUB: dspgst_ called");
    return NULL;
}

void* dspgv(void)
{
    if (verbose) puts("STUB: dspgv called");
    return NULL;
}

void* dspgv_(void)
{
    if (verbose) puts("STUB: dspgv_ called");
    return NULL;
}

void* dspgvd(void)
{
    if (verbose) puts("STUB: dspgvd called");
    return NULL;
}

void* dspgvd_(void)
{
    if (verbose) puts("STUB: dspgvd_ called");
    return NULL;
}

void* dspgvx(void)
{
    if (verbose) puts("STUB: dspgvx called");
    return NULL;
}

void* dspgvx_(void)
{
    if (verbose) puts("STUB: dspgvx_ called");
    return NULL;
}

void* dsposv(void)
{
    if (verbose) puts("STUB: dsposv called");
    return NULL;
}

void* dsposv_(void)
{
    if (verbose) puts("STUB: dsposv_ called");
    return NULL;
}

void* dsprfs(void)
{
    if (verbose) puts("STUB: dsprfs called");
    return NULL;
}

void* dsprfs_(void)
{
    if (verbose) puts("STUB: dsprfs_ called");
    return NULL;
}

void* dspsv(void)
{
    if (verbose) puts("STUB: dspsv called");
    return NULL;
}

void* dspsv_(void)
{
    if (verbose) puts("STUB: dspsv_ called");
    return NULL;
}

void* dspsvx(void)
{
    if (verbose) puts("STUB: dspsvx called");
    return NULL;
}

void* dspsvx_(void)
{
    if (verbose) puts("STUB: dspsvx_ called");
    return NULL;
}

void* dsptrd(void)
{
    if (verbose) puts("STUB: dsptrd called");
    return NULL;
}

void* dsptrd_(void)
{
    if (verbose) puts("STUB: dsptrd_ called");
    return NULL;
}

void* dsptrf(void)
{
    if (verbose) puts("STUB: dsptrf called");
    return NULL;
}

void* dsptrf_(void)
{
    if (verbose) puts("STUB: dsptrf_ called");
    return NULL;
}

void* dsptri(void)
{
    if (verbose) puts("STUB: dsptri called");
    return NULL;
}

void* dsptri_(void)
{
    if (verbose) puts("STUB: dsptri_ called");
    return NULL;
}

void* dsptrs(void)
{
    if (verbose) puts("STUB: dsptrs called");
    return NULL;
}

void* dsptrs_(void)
{
    if (verbose) puts("STUB: dsptrs_ called");
    return NULL;
}

void* dstebz(void)
{
    if (verbose) puts("STUB: dstebz called");
    return NULL;
}

void* dstebz_(void)
{
    if (verbose) puts("STUB: dstebz_ called");
    return NULL;
}

void* dstedc(void)
{
    if (verbose) puts("STUB: dstedc called");
    return NULL;
}

void* dstedc_(void)
{
    if (verbose) puts("STUB: dstedc_ called");
    return NULL;
}

void* dstegr(void)
{
    if (verbose) puts("STUB: dstegr called");
    return NULL;
}

void* dstegr_(void)
{
    if (verbose) puts("STUB: dstegr_ called");
    return NULL;
}

void* dstein(void)
{
    if (verbose) puts("STUB: dstein called");
    return NULL;
}

void* dstein_(void)
{
    if (verbose) puts("STUB: dstein_ called");
    return NULL;
}

void* dstemr(void)
{
    if (verbose) puts("STUB: dstemr called");
    return NULL;
}

void* dstemr_(void)
{
    if (verbose) puts("STUB: dstemr_ called");
    return NULL;
}

void* dsteqr(void)
{
    if (verbose) puts("STUB: dsteqr called");
    return NULL;
}

void* dsteqr_(void)
{
    if (verbose) puts("STUB: dsteqr_ called");
    return NULL;
}

void* dsterf(void)
{
    if (verbose) puts("STUB: dsterf called");
    return NULL;
}

void* dsterf_(void)
{
    if (verbose) puts("STUB: dsterf_ called");
    return NULL;
}

void* dstev(void)
{
    if (verbose) puts("STUB: dstev called");
    return NULL;
}

void* dstev_(void)
{
    if (verbose) puts("STUB: dstev_ called");
    return NULL;
}

void* dstevd(void)
{
    if (verbose) puts("STUB: dstevd called");
    return NULL;
}

void* dstevd_(void)
{
    if (verbose) puts("STUB: dstevd_ called");
    return NULL;
}

void* dstevr(void)
{
    if (verbose) puts("STUB: dstevr called");
    return NULL;
}

void* dstevr_(void)
{
    if (verbose) puts("STUB: dstevr_ called");
    return NULL;
}

void* dstevx(void)
{
    if (verbose) puts("STUB: dstevx called");
    return NULL;
}

void* dstevx_(void)
{
    if (verbose) puts("STUB: dstevx_ called");
    return NULL;
}

void* dsycon(void)
{
    if (verbose) puts("STUB: dsycon called");
    return NULL;
}

void* dsycon_(void)
{
    if (verbose) puts("STUB: dsycon_ called");
    return NULL;
}

void* dsyequb(void)
{
    if (verbose) puts("STUB: dsyequb called");
    return NULL;
}

void* dsyequb_(void)
{
    if (verbose) puts("STUB: dsyequb_ called");
    return NULL;
}

void* dsyev(void)
{
    if (verbose) puts("STUB: dsyev called");
    return NULL;
}

void* dsyev_(void)
{
    if (verbose) puts("STUB: dsyev_ called");
    return NULL;
}

void* dsyevd(void)
{
    if (verbose) puts("STUB: dsyevd called");
    return NULL;
}

void* dsyevd_(void)
{
    if (verbose) puts("STUB: dsyevd_ called");
    return NULL;
}

void* dsyevr(void)
{
    if (verbose) puts("STUB: dsyevr called");
    return NULL;
}

void* dsyevr_(void)
{
    if (verbose) puts("STUB: dsyevr_ called");
    return NULL;
}

void* dsyevx(void)
{
    if (verbose) puts("STUB: dsyevx called");
    return NULL;
}

void* dsyevx_(void)
{
    if (verbose) puts("STUB: dsyevx_ called");
    return NULL;
}

void* dsygs2(void)
{
    if (verbose) puts("STUB: dsygs2 called");
    return NULL;
}

void* dsygs2_(void)
{
    if (verbose) puts("STUB: dsygs2_ called");
    return NULL;
}

void* dsygst(void)
{
    if (verbose) puts("STUB: dsygst called");
    return NULL;
}

void* dsygst_(void)
{
    if (verbose) puts("STUB: dsygst_ called");
    return NULL;
}

void* dsygv(void)
{
    if (verbose) puts("STUB: dsygv called");
    return NULL;
}

void* dsygv_(void)
{
    if (verbose) puts("STUB: dsygv_ called");
    return NULL;
}

void* dsygvd(void)
{
    if (verbose) puts("STUB: dsygvd called");
    return NULL;
}

void* dsygvd_(void)
{
    if (verbose) puts("STUB: dsygvd_ called");
    return NULL;
}

void* dsygvx(void)
{
    if (verbose) puts("STUB: dsygvx called");
    return NULL;
}

void* dsygvx_(void)
{
    if (verbose) puts("STUB: dsygvx_ called");
    return NULL;
}

void* dsyrfs(void)
{
    if (verbose) puts("STUB: dsyrfs called");
    return NULL;
}

void* dsyrfs_(void)
{
    if (verbose) puts("STUB: dsyrfs_ called");
    return NULL;
}

void* dsysv(void)
{
    if (verbose) puts("STUB: dsysv called");
    return NULL;
}

void* dsysv_(void)
{
    if (verbose) puts("STUB: dsysv_ called");
    return NULL;
}

void* dsysvx(void)
{
    if (verbose) puts("STUB: dsysvx called");
    return NULL;
}

void* dsysvx_(void)
{
    if (verbose) puts("STUB: dsysvx_ called");
    return NULL;
}

void* dsytd2(void)
{
    if (verbose) puts("STUB: dsytd2 called");
    return NULL;
}

void* dsytd2_(void)
{
    if (verbose) puts("STUB: dsytd2_ called");
    return NULL;
}

void* dsytf2(void)
{
    if (verbose) puts("STUB: dsytf2 called");
    return NULL;
}

void* dsytf2_(void)
{
    if (verbose) puts("STUB: dsytf2_ called");
    return NULL;
}

void* dsytrd(void)
{
    if (verbose) puts("STUB: dsytrd called");
    return NULL;
}

void* dsytrd_(void)
{
    if (verbose) puts("STUB: dsytrd_ called");
    return NULL;
}

void* dsytrf(void)
{
    if (verbose) puts("STUB: dsytrf called");
    return NULL;
}

void* dsytrf_(void)
{
    if (verbose) puts("STUB: dsytrf_ called");
    return NULL;
}

void* dsytri(void)
{
    if (verbose) puts("STUB: dsytri called");
    return NULL;
}

void* dsytri_(void)
{
    if (verbose) puts("STUB: dsytri_ called");
    return NULL;
}

void* dsytrs(void)
{
    if (verbose) puts("STUB: dsytrs called");
    return NULL;
}

void* dsytrs_(void)
{
    if (verbose) puts("STUB: dsytrs_ called");
    return NULL;
}

void* dtbcon(void)
{
    if (verbose) puts("STUB: dtbcon called");
    return NULL;
}

void* dtbcon_(void)
{
    if (verbose) puts("STUB: dtbcon_ called");
    return NULL;
}

void* dtbrfs(void)
{
    if (verbose) puts("STUB: dtbrfs called");
    return NULL;
}

void* dtbrfs_(void)
{
    if (verbose) puts("STUB: dtbrfs_ called");
    return NULL;
}

void* dtbtrs(void)
{
    if (verbose) puts("STUB: dtbtrs called");
    return NULL;
}

void* dtbtrs_(void)
{
    if (verbose) puts("STUB: dtbtrs_ called");
    return NULL;
}

void* dtfsm(void)
{
    if (verbose) puts("STUB: dtfsm called");
    return NULL;
}

void* dtfsm_(void)
{
    if (verbose) puts("STUB: dtfsm_ called");
    return NULL;
}

void* dtftri(void)
{
    if (verbose) puts("STUB: dtftri called");
    return NULL;
}

void* dtftri_(void)
{
    if (verbose) puts("STUB: dtftri_ called");
    return NULL;
}

void* dtfttp(void)
{
    if (verbose) puts("STUB: dtfttp called");
    return NULL;
}

void* dtfttp_(void)
{
    if (verbose) puts("STUB: dtfttp_ called");
    return NULL;
}

void* dtfttr(void)
{
    if (verbose) puts("STUB: dtfttr called");
    return NULL;
}

void* dtfttr_(void)
{
    if (verbose) puts("STUB: dtfttr_ called");
    return NULL;
}

void* dtgevc(void)
{
    if (verbose) puts("STUB: dtgevc called");
    return NULL;
}

void* dtgevc_(void)
{
    if (verbose) puts("STUB: dtgevc_ called");
    return NULL;
}

void* dtgex2(void)
{
    if (verbose) puts("STUB: dtgex2 called");
    return NULL;
}

void* dtgex2_(void)
{
    if (verbose) puts("STUB: dtgex2_ called");
    return NULL;
}

void* dtgexc(void)
{
    if (verbose) puts("STUB: dtgexc called");
    return NULL;
}

void* dtgexc_(void)
{
    if (verbose) puts("STUB: dtgexc_ called");
    return NULL;
}

void* dtgsen(void)
{
    if (verbose) puts("STUB: dtgsen called");
    return NULL;
}

void* dtgsen_(void)
{
    if (verbose) puts("STUB: dtgsen_ called");
    return NULL;
}

void* dtgsja(void)
{
    if (verbose) puts("STUB: dtgsja called");
    return NULL;
}

void* dtgsja_(void)
{
    if (verbose) puts("STUB: dtgsja_ called");
    return NULL;
}

void* dtgsna(void)
{
    if (verbose) puts("STUB: dtgsna called");
    return NULL;
}

void* dtgsna_(void)
{
    if (verbose) puts("STUB: dtgsna_ called");
    return NULL;
}

void* dtgsy2(void)
{
    if (verbose) puts("STUB: dtgsy2 called");
    return NULL;
}

void* dtgsy2_(void)
{
    if (verbose) puts("STUB: dtgsy2_ called");
    return NULL;
}

void* dtgsyl(void)
{
    if (verbose) puts("STUB: dtgsyl called");
    return NULL;
}

void* dtgsyl_(void)
{
    if (verbose) puts("STUB: dtgsyl_ called");
    return NULL;
}

void* dtpcon(void)
{
    if (verbose) puts("STUB: dtpcon called");
    return NULL;
}

void* dtpcon_(void)
{
    if (verbose) puts("STUB: dtpcon_ called");
    return NULL;
}

void* dtprfs(void)
{
    if (verbose) puts("STUB: dtprfs called");
    return NULL;
}

void* dtprfs_(void)
{
    if (verbose) puts("STUB: dtprfs_ called");
    return NULL;
}

void* dtptri(void)
{
    if (verbose) puts("STUB: dtptri called");
    return NULL;
}

void* dtptri_(void)
{
    if (verbose) puts("STUB: dtptri_ called");
    return NULL;
}

void* dtptrs(void)
{
    if (verbose) puts("STUB: dtptrs called");
    return NULL;
}

void* dtptrs_(void)
{
    if (verbose) puts("STUB: dtptrs_ called");
    return NULL;
}

void* dtpttf(void)
{
    if (verbose) puts("STUB: dtpttf called");
    return NULL;
}

void* dtpttf_(void)
{
    if (verbose) puts("STUB: dtpttf_ called");
    return NULL;
}

void* dtpttr(void)
{
    if (verbose) puts("STUB: dtpttr called");
    return NULL;
}

void* dtpttr_(void)
{
    if (verbose) puts("STUB: dtpttr_ called");
    return NULL;
}

void* dtrcon(void)
{
    if (verbose) puts("STUB: dtrcon called");
    return NULL;
}

void* dtrcon_(void)
{
    if (verbose) puts("STUB: dtrcon_ called");
    return NULL;
}

void* dtrevc(void)
{
    if (verbose) puts("STUB: dtrevc called");
    return NULL;
}

void* dtrevc_(void)
{
    if (verbose) puts("STUB: dtrevc_ called");
    return NULL;
}

void* dtrexc(void)
{
    if (verbose) puts("STUB: dtrexc called");
    return NULL;
}

void* dtrexc_(void)
{
    if (verbose) puts("STUB: dtrexc_ called");
    return NULL;
}

void* dtrrfs(void)
{
    if (verbose) puts("STUB: dtrrfs called");
    return NULL;
}

void* dtrrfs_(void)
{
    if (verbose) puts("STUB: dtrrfs_ called");
    return NULL;
}

void* dtrsen(void)
{
    if (verbose) puts("STUB: dtrsen called");
    return NULL;
}

void* dtrsen_(void)
{
    if (verbose) puts("STUB: dtrsen_ called");
    return NULL;
}

void* dtrsna(void)
{
    if (verbose) puts("STUB: dtrsna called");
    return NULL;
}

void* dtrsna_(void)
{
    if (verbose) puts("STUB: dtrsna_ called");
    return NULL;
}

void* dtrsyl(void)
{
    if (verbose) puts("STUB: dtrsyl called");
    return NULL;
}

void* dtrsyl_(void)
{
    if (verbose) puts("STUB: dtrsyl_ called");
    return NULL;
}

void* dtrti2(void)
{
    if (verbose) puts("STUB: dtrti2 called");
    return NULL;
}

void* dtrti2_(void)
{
    if (verbose) puts("STUB: dtrti2_ called");
    return NULL;
}

void* dtrtri(void)
{
    if (verbose) puts("STUB: dtrtri called");
    return NULL;
}

void* dtrtri_(void)
{
    if (verbose) puts("STUB: dtrtri_ called");
    return NULL;
}

void* dtrtrs(void)
{
    if (verbose) puts("STUB: dtrtrs called");
    return NULL;
}

void* dtrtrs_(void)
{
    if (verbose) puts("STUB: dtrtrs_ called");
    return NULL;
}

void* dtrttf(void)
{
    if (verbose) puts("STUB: dtrttf called");
    return NULL;
}

void* dtrttf_(void)
{
    if (verbose) puts("STUB: dtrttf_ called");
    return NULL;
}

void* dtrttp(void)
{
    if (verbose) puts("STUB: dtrttp called");
    return NULL;
}

void* dtrttp_(void)
{
    if (verbose) puts("STUB: dtrttp_ called");
    return NULL;
}

void* dtzrqf(void)
{
    if (verbose) puts("STUB: dtzrqf called");
    return NULL;
}

void* dtzrqf_(void)
{
    if (verbose) puts("STUB: dtzrqf_ called");
    return NULL;
}

void* dtzrzf(void)
{
    if (verbose) puts("STUB: dtzrzf called");
    return NULL;
}

void* dtzrzf_(void)
{
    if (verbose) puts("STUB: dtzrzf_ called");
    return NULL;
}

void* dzsum1(void)
{
    if (verbose) puts("STUB: dzsum1 called");
    return NULL;
}

void* dzsum1_(void)
{
    if (verbose) puts("STUB: dzsum1_ called");
    return NULL;
}

void* icmax1(void)
{
    if (verbose) puts("STUB: icmax1 called");
    return NULL;
}

void* icmax1_(void)
{
    if (verbose) puts("STUB: icmax1_ called");
    return NULL;
}

void* ieeeck(void)
{
    if (verbose) puts("STUB: ieeeck called");
    return NULL;
}

void* ieeeck_(void)
{
    if (verbose) puts("STUB: ieeeck_ called");
    return NULL;
}

void* ilaclc(void)
{
    if (verbose) puts("STUB: ilaclc called");
    return NULL;
}

void* ilaclc_(void)
{
    if (verbose) puts("STUB: ilaclc_ called");
    return NULL;
}

void* ilaclr(void)
{
    if (verbose) puts("STUB: ilaclr called");
    return NULL;
}

void* ilaclr_(void)
{
    if (verbose) puts("STUB: ilaclr_ called");
    return NULL;
}

void* iladiag(void)
{
    if (verbose) puts("STUB: iladiag called");
    return NULL;
}

void* iladiag_(void)
{
    if (verbose) puts("STUB: iladiag_ called");
    return NULL;
}

void* iladlc(void)
{
    if (verbose) puts("STUB: iladlc called");
    return NULL;
}

void* iladlc_(void)
{
    if (verbose) puts("STUB: iladlc_ called");
    return NULL;
}

void* iladlr(void)
{
    if (verbose) puts("STUB: iladlr called");
    return NULL;
}

void* iladlr_(void)
{
    if (verbose) puts("STUB: iladlr_ called");
    return NULL;
}

void* ilaenv(void)
{
    if (verbose) puts("STUB: ilaenv called");
    return NULL;
}

void* ilaenv_(void)
{
    if (verbose) puts("STUB: ilaenv_ called");
    return NULL;
}

void* ilaprec(void)
{
    if (verbose) puts("STUB: ilaprec called");
    return NULL;
}

void* ilaprec_(void)
{
    if (verbose) puts("STUB: ilaprec_ called");
    return NULL;
}

void* ilaslc(void)
{
    if (verbose) puts("STUB: ilaslc called");
    return NULL;
}

void* ilaslc_(void)
{
    if (verbose) puts("STUB: ilaslc_ called");
    return NULL;
}

void* ilaslr(void)
{
    if (verbose) puts("STUB: ilaslr called");
    return NULL;
}

void* ilaslr_(void)
{
    if (verbose) puts("STUB: ilaslr_ called");
    return NULL;
}

void* ilatrans(void)
{
    if (verbose) puts("STUB: ilatrans called");
    return NULL;
}

void* ilatrans_(void)
{
    if (verbose) puts("STUB: ilatrans_ called");
    return NULL;
}

void* ilauplo(void)
{
    if (verbose) puts("STUB: ilauplo called");
    return NULL;
}

void* ilauplo_(void)
{
    if (verbose) puts("STUB: ilauplo_ called");
    return NULL;
}

void* ilaver(void)
{
    if (verbose) puts("STUB: ilaver called");
    return NULL;
}

void* ilaver_(void)
{
    if (verbose) puts("STUB: ilaver_ called");
    return NULL;
}

void* ilazlc(void)
{
    if (verbose) puts("STUB: ilazlc called");
    return NULL;
}

void* ilazlc_(void)
{
    if (verbose) puts("STUB: ilazlc_ called");
    return NULL;
}

void* ilazlr(void)
{
    if (verbose) puts("STUB: ilazlr called");
    return NULL;
}

void* ilazlr_(void)
{
    if (verbose) puts("STUB: ilazlr_ called");
    return NULL;
}

void* iparmq(void)
{
    if (verbose) puts("STUB: iparmq called");
    return NULL;
}

void* iparmq_(void)
{
    if (verbose) puts("STUB: iparmq_ called");
    return NULL;
}

void* izmax1(void)
{
    if (verbose) puts("STUB: izmax1 called");
    return NULL;
}

void* izmax1_(void)
{
    if (verbose) puts("STUB: izmax1_ called");
    return NULL;
}

void* lsamen(void)
{
    if (verbose) puts("STUB: lsamen called");
    return NULL;
}

void* lsamen_(void)
{
    if (verbose) puts("STUB: lsamen_ called");
    return NULL;
}

void* sbdsdc(void)
{
    if (verbose) puts("STUB: sbdsdc called");
    return NULL;
}

void* sbdsdc_(void)
{
    if (verbose) puts("STUB: sbdsdc_ called");
    return NULL;
}

void* sbdsqr(void)
{
    if (verbose) puts("STUB: sbdsqr called");
    return NULL;
}

void* sbdsqr_(void)
{
    if (verbose) puts("STUB: sbdsqr_ called");
    return NULL;
}

void* scsum1(void)
{
    if (verbose) puts("STUB: scsum1 called");
    return NULL;
}

void* scsum1_(void)
{
    if (verbose) puts("STUB: scsum1_ called");
    return NULL;
}

void* sdisna(void)
{
    if (verbose) puts("STUB: sdisna called");
    return NULL;
}

void* sdisna_(void)
{
    if (verbose) puts("STUB: sdisna_ called");
    return NULL;
}

void* sgbbrd(void)
{
    if (verbose) puts("STUB: sgbbrd called");
    return NULL;
}

void* sgbbrd_(void)
{
    if (verbose) puts("STUB: sgbbrd_ called");
    return NULL;
}

void* sgbcon(void)
{
    if (verbose) puts("STUB: sgbcon called");
    return NULL;
}

void* sgbcon_(void)
{
    if (verbose) puts("STUB: sgbcon_ called");
    return NULL;
}

void* sgbequ(void)
{
    if (verbose) puts("STUB: sgbequ called");
    return NULL;
}

void* sgbequ_(void)
{
    if (verbose) puts("STUB: sgbequ_ called");
    return NULL;
}

void* sgbequb(void)
{
    if (verbose) puts("STUB: sgbequb called");
    return NULL;
}

void* sgbequb_(void)
{
    if (verbose) puts("STUB: sgbequb_ called");
    return NULL;
}

void* sgbrfs(void)
{
    if (verbose) puts("STUB: sgbrfs called");
    return NULL;
}

void* sgbrfs_(void)
{
    if (verbose) puts("STUB: sgbrfs_ called");
    return NULL;
}

void* sgbsv(void)
{
    if (verbose) puts("STUB: sgbsv called");
    return NULL;
}

void* sgbsv_(void)
{
    if (verbose) puts("STUB: sgbsv_ called");
    return NULL;
}

void* sgbsvx(void)
{
    if (verbose) puts("STUB: sgbsvx called");
    return NULL;
}

void* sgbsvx_(void)
{
    if (verbose) puts("STUB: sgbsvx_ called");
    return NULL;
}

void* sgbtf2(void)
{
    if (verbose) puts("STUB: sgbtf2 called");
    return NULL;
}

void* sgbtf2_(void)
{
    if (verbose) puts("STUB: sgbtf2_ called");
    return NULL;
}

void* sgbtrf(void)
{
    if (verbose) puts("STUB: sgbtrf called");
    return NULL;
}

void* sgbtrf_(void)
{
    if (verbose) puts("STUB: sgbtrf_ called");
    return NULL;
}

void* sgbtrs(void)
{
    if (verbose) puts("STUB: sgbtrs called");
    return NULL;
}

void* sgbtrs_(void)
{
    if (verbose) puts("STUB: sgbtrs_ called");
    return NULL;
}

void* sgebak(void)
{
    if (verbose) puts("STUB: sgebak called");
    return NULL;
}

void* sgebak_(void)
{
    if (verbose) puts("STUB: sgebak_ called");
    return NULL;
}

void* sgebal(void)
{
    if (verbose) puts("STUB: sgebal called");
    return NULL;
}

void* sgebal_(void)
{
    if (verbose) puts("STUB: sgebal_ called");
    return NULL;
}

void* sgebd2(void)
{
    if (verbose) puts("STUB: sgebd2 called");
    return NULL;
}

void* sgebd2_(void)
{
    if (verbose) puts("STUB: sgebd2_ called");
    return NULL;
}

void* sgebrd(void)
{
    if (verbose) puts("STUB: sgebrd called");
    return NULL;
}

void* sgebrd_(void)
{
    if (verbose) puts("STUB: sgebrd_ called");
    return NULL;
}

void* sgecon(void)
{
    if (verbose) puts("STUB: sgecon called");
    return NULL;
}

void* sgecon_(void)
{
    if (verbose) puts("STUB: sgecon_ called");
    return NULL;
}

void* sgeequ(void)
{
    if (verbose) puts("STUB: sgeequ called");
    return NULL;
}

void* sgeequ_(void)
{
    if (verbose) puts("STUB: sgeequ_ called");
    return NULL;
}

void* sgeequb(void)
{
    if (verbose) puts("STUB: sgeequb called");
    return NULL;
}

void* sgeequb_(void)
{
    if (verbose) puts("STUB: sgeequb_ called");
    return NULL;
}

void* sgees(void)
{
    if (verbose) puts("STUB: sgees called");
    return NULL;
}

void* sgees_(void)
{
    if (verbose) puts("STUB: sgees_ called");
    return NULL;
}

void* sgeesx(void)
{
    if (verbose) puts("STUB: sgeesx called");
    return NULL;
}

void* sgeesx_(void)
{
    if (verbose) puts("STUB: sgeesx_ called");
    return NULL;
}

void* sgeev(void)
{
    if (verbose) puts("STUB: sgeev called");
    return NULL;
}

void* sgeev_(void)
{
    if (verbose) puts("STUB: sgeev_ called");
    return NULL;
}

void* sgeevx(void)
{
    if (verbose) puts("STUB: sgeevx called");
    return NULL;
}

void* sgeevx_(void)
{
    if (verbose) puts("STUB: sgeevx_ called");
    return NULL;
}

void* sgegs(void)
{
    if (verbose) puts("STUB: sgegs called");
    return NULL;
}

void* sgegs_(void)
{
    if (verbose) puts("STUB: sgegs_ called");
    return NULL;
}

void* sgegv(void)
{
    if (verbose) puts("STUB: sgegv called");
    return NULL;
}

void* sgegv_(void)
{
    if (verbose) puts("STUB: sgegv_ called");
    return NULL;
}

void* sgehd2(void)
{
    if (verbose) puts("STUB: sgehd2 called");
    return NULL;
}

void* sgehd2_(void)
{
    if (verbose) puts("STUB: sgehd2_ called");
    return NULL;
}

void* sgehrd(void)
{
    if (verbose) puts("STUB: sgehrd called");
    return NULL;
}

void* sgehrd_(void)
{
    if (verbose) puts("STUB: sgehrd_ called");
    return NULL;
}

void* sgejsv(void)
{
    if (verbose) puts("STUB: sgejsv called");
    return NULL;
}

void* sgejsv_(void)
{
    if (verbose) puts("STUB: sgejsv_ called");
    return NULL;
}

void* sgelq2(void)
{
    if (verbose) puts("STUB: sgelq2 called");
    return NULL;
}

void* sgelq2_(void)
{
    if (verbose) puts("STUB: sgelq2_ called");
    return NULL;
}

void* sgelqf(void)
{
    if (verbose) puts("STUB: sgelqf called");
    return NULL;
}

void* sgelqf_(void)
{
    if (verbose) puts("STUB: sgelqf_ called");
    return NULL;
}

void* sgels(void)
{
    if (verbose) puts("STUB: sgels called");
    return NULL;
}

void* sgels_(void)
{
    if (verbose) puts("STUB: sgels_ called");
    return NULL;
}

void* sgelsd(void)
{
    if (verbose) puts("STUB: sgelsd called");
    return NULL;
}

void* sgelsd_(void)
{
    if (verbose) puts("STUB: sgelsd_ called");
    return NULL;
}

void* sgelss(void)
{
    if (verbose) puts("STUB: sgelss called");
    return NULL;
}

void* sgelss_(void)
{
    if (verbose) puts("STUB: sgelss_ called");
    return NULL;
}

void* sgelsx(void)
{
    if (verbose) puts("STUB: sgelsx called");
    return NULL;
}

void* sgelsx_(void)
{
    if (verbose) puts("STUB: sgelsx_ called");
    return NULL;
}

void* sgelsy(void)
{
    if (verbose) puts("STUB: sgelsy called");
    return NULL;
}

void* sgelsy_(void)
{
    if (verbose) puts("STUB: sgelsy_ called");
    return NULL;
}

void* sgeql2(void)
{
    if (verbose) puts("STUB: sgeql2 called");
    return NULL;
}

void* sgeql2_(void)
{
    if (verbose) puts("STUB: sgeql2_ called");
    return NULL;
}

void* sgeqlf(void)
{
    if (verbose) puts("STUB: sgeqlf called");
    return NULL;
}

void* sgeqlf_(void)
{
    if (verbose) puts("STUB: sgeqlf_ called");
    return NULL;
}

void* sgeqp3(void)
{
    if (verbose) puts("STUB: sgeqp3 called");
    return NULL;
}

void* sgeqp3_(void)
{
    if (verbose) puts("STUB: sgeqp3_ called");
    return NULL;
}

void* sgeqpf(void)
{
    if (verbose) puts("STUB: sgeqpf called");
    return NULL;
}

void* sgeqpf_(void)
{
    if (verbose) puts("STUB: sgeqpf_ called");
    return NULL;
}

void* sgeqr2(void)
{
    if (verbose) puts("STUB: sgeqr2 called");
    return NULL;
}

void* sgeqr2_(void)
{
    if (verbose) puts("STUB: sgeqr2_ called");
    return NULL;
}

void* sgeqrf(void)
{
    if (verbose) puts("STUB: sgeqrf called");
    return NULL;
}

void* sgeqrf_(void)
{
    if (verbose) puts("STUB: sgeqrf_ called");
    return NULL;
}

void* sgerfs(void)
{
    if (verbose) puts("STUB: sgerfs called");
    return NULL;
}

void* sgerfs_(void)
{
    if (verbose) puts("STUB: sgerfs_ called");
    return NULL;
}

void* sgerq2(void)
{
    if (verbose) puts("STUB: sgerq2 called");
    return NULL;
}

void* sgerq2_(void)
{
    if (verbose) puts("STUB: sgerq2_ called");
    return NULL;
}

void* sgerqf(void)
{
    if (verbose) puts("STUB: sgerqf called");
    return NULL;
}

void* sgerqf_(void)
{
    if (verbose) puts("STUB: sgerqf_ called");
    return NULL;
}

void* sgesc2(void)
{
    if (verbose) puts("STUB: sgesc2 called");
    return NULL;
}

void* sgesc2_(void)
{
    if (verbose) puts("STUB: sgesc2_ called");
    return NULL;
}

void* sgesdd(void)
{
    if (verbose) puts("STUB: sgesdd called");
    return NULL;
}

void* sgesdd_(void)
{
    if (verbose) puts("STUB: sgesdd_ called");
    return NULL;
}

void* sgesv(void)
{
    if (verbose) puts("STUB: sgesv called");
    return NULL;
}

void* sgesv_(void)
{
    if (verbose) puts("STUB: sgesv_ called");
    return NULL;
}

void* sgesvd(void)
{
    if (verbose) puts("STUB: sgesvd called");
    return NULL;
}

void* sgesvd_(void)
{
    if (verbose) puts("STUB: sgesvd_ called");
    return NULL;
}

void* sgesvj(void)
{
    if (verbose) puts("STUB: sgesvj called");
    return NULL;
}

void* sgesvj_(void)
{
    if (verbose) puts("STUB: sgesvj_ called");
    return NULL;
}

void* sgesvx(void)
{
    if (verbose) puts("STUB: sgesvx called");
    return NULL;
}

void* sgesvx_(void)
{
    if (verbose) puts("STUB: sgesvx_ called");
    return NULL;
}

void* sgetc2(void)
{
    if (verbose) puts("STUB: sgetc2 called");
    return NULL;
}

void* sgetc2_(void)
{
    if (verbose) puts("STUB: sgetc2_ called");
    return NULL;
}

void* sgetf2(void)
{
    if (verbose) puts("STUB: sgetf2 called");
    return NULL;
}

void* sgetf2_(void)
{
    if (verbose) puts("STUB: sgetf2_ called");
    return NULL;
}

void* sgetrf(void)
{
    if (verbose) puts("STUB: sgetrf called");
    return NULL;
}

void* sgetrf_(void)
{
    if (verbose) puts("STUB: sgetrf_ called");
    return NULL;
}

void* sgetri(void)
{
    if (verbose) puts("STUB: sgetri called");
    return NULL;
}

void* sgetri_(void)
{
    if (verbose) puts("STUB: sgetri_ called");
    return NULL;
}

void* sgetrs(void)
{
    if (verbose) puts("STUB: sgetrs called");
    return NULL;
}

void* sgetrs_(void)
{
    if (verbose) puts("STUB: sgetrs_ called");
    return NULL;
}

void* sggbak(void)
{
    if (verbose) puts("STUB: sggbak called");
    return NULL;
}

void* sggbak_(void)
{
    if (verbose) puts("STUB: sggbak_ called");
    return NULL;
}

void* sggbal(void)
{
    if (verbose) puts("STUB: sggbal called");
    return NULL;
}

void* sggbal_(void)
{
    if (verbose) puts("STUB: sggbal_ called");
    return NULL;
}

void* sgges(void)
{
    if (verbose) puts("STUB: sgges called");
    return NULL;
}

void* sgges_(void)
{
    if (verbose) puts("STUB: sgges_ called");
    return NULL;
}

void* sggesx(void)
{
    if (verbose) puts("STUB: sggesx called");
    return NULL;
}

void* sggesx_(void)
{
    if (verbose) puts("STUB: sggesx_ called");
    return NULL;
}

void* sggev(void)
{
    if (verbose) puts("STUB: sggev called");
    return NULL;
}

void* sggev_(void)
{
    if (verbose) puts("STUB: sggev_ called");
    return NULL;
}

void* sggevx(void)
{
    if (verbose) puts("STUB: sggevx called");
    return NULL;
}

void* sggevx_(void)
{
    if (verbose) puts("STUB: sggevx_ called");
    return NULL;
}

void* sggglm(void)
{
    if (verbose) puts("STUB: sggglm called");
    return NULL;
}

void* sggglm_(void)
{
    if (verbose) puts("STUB: sggglm_ called");
    return NULL;
}

void* sgghrd(void)
{
    if (verbose) puts("STUB: sgghrd called");
    return NULL;
}

void* sgghrd_(void)
{
    if (verbose) puts("STUB: sgghrd_ called");
    return NULL;
}

void* sgglse(void)
{
    if (verbose) puts("STUB: sgglse called");
    return NULL;
}

void* sgglse_(void)
{
    if (verbose) puts("STUB: sgglse_ called");
    return NULL;
}

void* sggqrf(void)
{
    if (verbose) puts("STUB: sggqrf called");
    return NULL;
}

void* sggqrf_(void)
{
    if (verbose) puts("STUB: sggqrf_ called");
    return NULL;
}

void* sggrqf(void)
{
    if (verbose) puts("STUB: sggrqf called");
    return NULL;
}

void* sggrqf_(void)
{
    if (verbose) puts("STUB: sggrqf_ called");
    return NULL;
}

void* sggsvd(void)
{
    if (verbose) puts("STUB: sggsvd called");
    return NULL;
}

void* sggsvd_(void)
{
    if (verbose) puts("STUB: sggsvd_ called");
    return NULL;
}

void* sggsvp(void)
{
    if (verbose) puts("STUB: sggsvp called");
    return NULL;
}

void* sggsvp_(void)
{
    if (verbose) puts("STUB: sggsvp_ called");
    return NULL;
}

void* sgsvj0(void)
{
    if (verbose) puts("STUB: sgsvj0 called");
    return NULL;
}

void* sgsvj0_(void)
{
    if (verbose) puts("STUB: sgsvj0_ called");
    return NULL;
}

void* sgsvj1(void)
{
    if (verbose) puts("STUB: sgsvj1 called");
    return NULL;
}

void* sgsvj1_(void)
{
    if (verbose) puts("STUB: sgsvj1_ called");
    return NULL;
}

void* sgtcon(void)
{
    if (verbose) puts("STUB: sgtcon called");
    return NULL;
}

void* sgtcon_(void)
{
    if (verbose) puts("STUB: sgtcon_ called");
    return NULL;
}

void* sgtrfs(void)
{
    if (verbose) puts("STUB: sgtrfs called");
    return NULL;
}

void* sgtrfs_(void)
{
    if (verbose) puts("STUB: sgtrfs_ called");
    return NULL;
}

void* sgtsv(void)
{
    if (verbose) puts("STUB: sgtsv called");
    return NULL;
}

void* sgtsv_(void)
{
    if (verbose) puts("STUB: sgtsv_ called");
    return NULL;
}

void* sgtsvx(void)
{
    if (verbose) puts("STUB: sgtsvx called");
    return NULL;
}

void* sgtsvx_(void)
{
    if (verbose) puts("STUB: sgtsvx_ called");
    return NULL;
}

void* sgttrf(void)
{
    if (verbose) puts("STUB: sgttrf called");
    return NULL;
}

void* sgttrf_(void)
{
    if (verbose) puts("STUB: sgttrf_ called");
    return NULL;
}

void* sgttrs(void)
{
    if (verbose) puts("STUB: sgttrs called");
    return NULL;
}

void* sgttrs_(void)
{
    if (verbose) puts("STUB: sgttrs_ called");
    return NULL;
}

void* sgtts2(void)
{
    if (verbose) puts("STUB: sgtts2 called");
    return NULL;
}

void* sgtts2_(void)
{
    if (verbose) puts("STUB: sgtts2_ called");
    return NULL;
}

void* shgeqz(void)
{
    if (verbose) puts("STUB: shgeqz called");
    return NULL;
}

void* shgeqz_(void)
{
    if (verbose) puts("STUB: shgeqz_ called");
    return NULL;
}

void* shsein(void)
{
    if (verbose) puts("STUB: shsein called");
    return NULL;
}

void* shsein_(void)
{
    if (verbose) puts("STUB: shsein_ called");
    return NULL;
}

void* shseqr(void)
{
    if (verbose) puts("STUB: shseqr called");
    return NULL;
}

void* shseqr_(void)
{
    if (verbose) puts("STUB: shseqr_ called");
    return NULL;
}

void* sisnan(void)
{
    if (verbose) puts("STUB: sisnan called");
    return NULL;
}

void* sisnan_(void)
{
    if (verbose) puts("STUB: sisnan_ called");
    return NULL;
}

void* slabad(void)
{
    if (verbose) puts("STUB: slabad called");
    return NULL;
}

void* slabad_(void)
{
    if (verbose) puts("STUB: slabad_ called");
    return NULL;
}

void* slabrd(void)
{
    if (verbose) puts("STUB: slabrd called");
    return NULL;
}

void* slabrd_(void)
{
    if (verbose) puts("STUB: slabrd_ called");
    return NULL;
}

void* slacn2(void)
{
    if (verbose) puts("STUB: slacn2 called");
    return NULL;
}

void* slacn2_(void)
{
    if (verbose) puts("STUB: slacn2_ called");
    return NULL;
}

void* slacon(void)
{
    if (verbose) puts("STUB: slacon called");
    return NULL;
}

void* slacon_(void)
{
    if (verbose) puts("STUB: slacon_ called");
    return NULL;
}

void* slacpy(void)
{
    if (verbose) puts("STUB: slacpy called");
    return NULL;
}

void* slacpy_(void)
{
    if (verbose) puts("STUB: slacpy_ called");
    return NULL;
}

void* sladiv(void)
{
    if (verbose) puts("STUB: sladiv called");
    return NULL;
}

void* sladiv_(void)
{
    if (verbose) puts("STUB: sladiv_ called");
    return NULL;
}

void* slae2(void)
{
    if (verbose) puts("STUB: slae2 called");
    return NULL;
}

void* slae2_(void)
{
    if (verbose) puts("STUB: slae2_ called");
    return NULL;
}

void* slaebz(void)
{
    if (verbose) puts("STUB: slaebz called");
    return NULL;
}

void* slaebz_(void)
{
    if (verbose) puts("STUB: slaebz_ called");
    return NULL;
}

void* slaed0(void)
{
    if (verbose) puts("STUB: slaed0 called");
    return NULL;
}

void* slaed0_(void)
{
    if (verbose) puts("STUB: slaed0_ called");
    return NULL;
}

void* slaed1(void)
{
    if (verbose) puts("STUB: slaed1 called");
    return NULL;
}

void* slaed1_(void)
{
    if (verbose) puts("STUB: slaed1_ called");
    return NULL;
}

void* slaed2(void)
{
    if (verbose) puts("STUB: slaed2 called");
    return NULL;
}

void* slaed2_(void)
{
    if (verbose) puts("STUB: slaed2_ called");
    return NULL;
}

void* slaed3(void)
{
    if (verbose) puts("STUB: slaed3 called");
    return NULL;
}

void* slaed3_(void)
{
    if (verbose) puts("STUB: slaed3_ called");
    return NULL;
}

void* slaed4(void)
{
    if (verbose) puts("STUB: slaed4 called");
    return NULL;
}

void* slaed4_(void)
{
    if (verbose) puts("STUB: slaed4_ called");
    return NULL;
}

void* slaed5(void)
{
    if (verbose) puts("STUB: slaed5 called");
    return NULL;
}

void* slaed5_(void)
{
    if (verbose) puts("STUB: slaed5_ called");
    return NULL;
}

void* slaed6(void)
{
    if (verbose) puts("STUB: slaed6 called");
    return NULL;
}

void* slaed6_(void)
{
    if (verbose) puts("STUB: slaed6_ called");
    return NULL;
}

void* slaed7(void)
{
    if (verbose) puts("STUB: slaed7 called");
    return NULL;
}

void* slaed7_(void)
{
    if (verbose) puts("STUB: slaed7_ called");
    return NULL;
}

void* slaed8(void)
{
    if (verbose) puts("STUB: slaed8 called");
    return NULL;
}

void* slaed8_(void)
{
    if (verbose) puts("STUB: slaed8_ called");
    return NULL;
}

void* slaed9(void)
{
    if (verbose) puts("STUB: slaed9 called");
    return NULL;
}

void* slaed9_(void)
{
    if (verbose) puts("STUB: slaed9_ called");
    return NULL;
}

void* slaeda(void)
{
    if (verbose) puts("STUB: slaeda called");
    return NULL;
}

void* slaeda_(void)
{
    if (verbose) puts("STUB: slaeda_ called");
    return NULL;
}

void* slaein(void)
{
    if (verbose) puts("STUB: slaein called");
    return NULL;
}

void* slaein_(void)
{
    if (verbose) puts("STUB: slaein_ called");
    return NULL;
}

void* slaev2(void)
{
    if (verbose) puts("STUB: slaev2 called");
    return NULL;
}

void* slaev2_(void)
{
    if (verbose) puts("STUB: slaev2_ called");
    return NULL;
}

void* slaexc(void)
{
    if (verbose) puts("STUB: slaexc called");
    return NULL;
}

void* slaexc_(void)
{
    if (verbose) puts("STUB: slaexc_ called");
    return NULL;
}

void* slag2(void)
{
    if (verbose) puts("STUB: slag2 called");
    return NULL;
}

void* slag2_(void)
{
    if (verbose) puts("STUB: slag2_ called");
    return NULL;
}

void* slag2d(void)
{
    if (verbose) puts("STUB: slag2d called");
    return NULL;
}

void* slag2d_(void)
{
    if (verbose) puts("STUB: slag2d_ called");
    return NULL;
}

void* slags2(void)
{
    if (verbose) puts("STUB: slags2 called");
    return NULL;
}

void* slags2_(void)
{
    if (verbose) puts("STUB: slags2_ called");
    return NULL;
}

void* slagtf(void)
{
    if (verbose) puts("STUB: slagtf called");
    return NULL;
}

void* slagtf_(void)
{
    if (verbose) puts("STUB: slagtf_ called");
    return NULL;
}

void* slagtm(void)
{
    if (verbose) puts("STUB: slagtm called");
    return NULL;
}

void* slagtm_(void)
{
    if (verbose) puts("STUB: slagtm_ called");
    return NULL;
}

void* slagts(void)
{
    if (verbose) puts("STUB: slagts called");
    return NULL;
}

void* slagts_(void)
{
    if (verbose) puts("STUB: slagts_ called");
    return NULL;
}

void* slagv2(void)
{
    if (verbose) puts("STUB: slagv2 called");
    return NULL;
}

void* slagv2_(void)
{
    if (verbose) puts("STUB: slagv2_ called");
    return NULL;
}

void* slahqr(void)
{
    if (verbose) puts("STUB: slahqr called");
    return NULL;
}

void* slahqr_(void)
{
    if (verbose) puts("STUB: slahqr_ called");
    return NULL;
}

void* slahr2(void)
{
    if (verbose) puts("STUB: slahr2 called");
    return NULL;
}

void* slahr2_(void)
{
    if (verbose) puts("STUB: slahr2_ called");
    return NULL;
}

void* slahrd(void)
{
    if (verbose) puts("STUB: slahrd called");
    return NULL;
}

void* slahrd_(void)
{
    if (verbose) puts("STUB: slahrd_ called");
    return NULL;
}

void* slaic1(void)
{
    if (verbose) puts("STUB: slaic1 called");
    return NULL;
}

void* slaic1_(void)
{
    if (verbose) puts("STUB: slaic1_ called");
    return NULL;
}

void* slaisnan(void)
{
    if (verbose) puts("STUB: slaisnan called");
    return NULL;
}

void* slaisnan_(void)
{
    if (verbose) puts("STUB: slaisnan_ called");
    return NULL;
}

void* slaln2(void)
{
    if (verbose) puts("STUB: slaln2 called");
    return NULL;
}

void* slaln2_(void)
{
    if (verbose) puts("STUB: slaln2_ called");
    return NULL;
}

void* slals0(void)
{
    if (verbose) puts("STUB: slals0 called");
    return NULL;
}

void* slals0_(void)
{
    if (verbose) puts("STUB: slals0_ called");
    return NULL;
}

void* slalsa(void)
{
    if (verbose) puts("STUB: slalsa called");
    return NULL;
}

void* slalsa_(void)
{
    if (verbose) puts("STUB: slalsa_ called");
    return NULL;
}

void* slalsd(void)
{
    if (verbose) puts("STUB: slalsd called");
    return NULL;
}

void* slalsd_(void)
{
    if (verbose) puts("STUB: slalsd_ called");
    return NULL;
}

void* slamc1(void)
{
    if (verbose) puts("STUB: slamc1 called");
    return NULL;
}

void* slamc1_(void)
{
    if (verbose) puts("STUB: slamc1_ called");
    return NULL;
}

void* slamc2(void)
{
    if (verbose) puts("STUB: slamc2 called");
    return NULL;
}

void* slamc2_(void)
{
    if (verbose) puts("STUB: slamc2_ called");
    return NULL;
}

void* slamc3(void)
{
    if (verbose) puts("STUB: slamc3 called");
    return NULL;
}

void* slamc3_(void)
{
    if (verbose) puts("STUB: slamc3_ called");
    return NULL;
}

void* slamc4(void)
{
    if (verbose) puts("STUB: slamc4 called");
    return NULL;
}

void* slamc4_(void)
{
    if (verbose) puts("STUB: slamc4_ called");
    return NULL;
}

void* slamc5(void)
{
    if (verbose) puts("STUB: slamc5 called");
    return NULL;
}

void* slamc5_(void)
{
    if (verbose) puts("STUB: slamc5_ called");
    return NULL;
}

void* slamch(void)
{
    if (verbose) puts("STUB: slamch called");
    return NULL;
}

void* slamch_(void)
{
    if (verbose) puts("STUB: slamch_ called");
    return NULL;
}

void* slamrg(void)
{
    if (verbose) puts("STUB: slamrg called");
    return NULL;
}

void* slamrg_(void)
{
    if (verbose) puts("STUB: slamrg_ called");
    return NULL;
}

void* slaneg(void)
{
    if (verbose) puts("STUB: slaneg called");
    return NULL;
}

void* slaneg_(void)
{
    if (verbose) puts("STUB: slaneg_ called");
    return NULL;
}

void* slangb(void)
{
    if (verbose) puts("STUB: slangb called");
    return NULL;
}

void* slangb_(void)
{
    if (verbose) puts("STUB: slangb_ called");
    return NULL;
}

void* slange(void)
{
    if (verbose) puts("STUB: slange called");
    return NULL;
}

void* slange_(void)
{
    if (verbose) puts("STUB: slange_ called");
    return NULL;
}

void* slangt(void)
{
    if (verbose) puts("STUB: slangt called");
    return NULL;
}

void* slangt_(void)
{
    if (verbose) puts("STUB: slangt_ called");
    return NULL;
}

void* slanhs(void)
{
    if (verbose) puts("STUB: slanhs called");
    return NULL;
}

void* slanhs_(void)
{
    if (verbose) puts("STUB: slanhs_ called");
    return NULL;
}

void* slansb(void)
{
    if (verbose) puts("STUB: slansb called");
    return NULL;
}

void* slansb_(void)
{
    if (verbose) puts("STUB: slansb_ called");
    return NULL;
}

void* slansf(void)
{
    if (verbose) puts("STUB: slansf called");
    return NULL;
}

void* slansf_(void)
{
    if (verbose) puts("STUB: slansf_ called");
    return NULL;
}

void* slansp(void)
{
    if (verbose) puts("STUB: slansp called");
    return NULL;
}

void* slansp_(void)
{
    if (verbose) puts("STUB: slansp_ called");
    return NULL;
}

void* slanst(void)
{
    if (verbose) puts("STUB: slanst called");
    return NULL;
}

void* slanst_(void)
{
    if (verbose) puts("STUB: slanst_ called");
    return NULL;
}

void* slansy(void)
{
    if (verbose) puts("STUB: slansy called");
    return NULL;
}

void* slansy_(void)
{
    if (verbose) puts("STUB: slansy_ called");
    return NULL;
}

void* slantb(void)
{
    if (verbose) puts("STUB: slantb called");
    return NULL;
}

void* slantb_(void)
{
    if (verbose) puts("STUB: slantb_ called");
    return NULL;
}

void* slantp(void)
{
    if (verbose) puts("STUB: slantp called");
    return NULL;
}

void* slantp_(void)
{
    if (verbose) puts("STUB: slantp_ called");
    return NULL;
}

void* slantr(void)
{
    if (verbose) puts("STUB: slantr called");
    return NULL;
}

void* slantr_(void)
{
    if (verbose) puts("STUB: slantr_ called");
    return NULL;
}

void* slanv2(void)
{
    if (verbose) puts("STUB: slanv2 called");
    return NULL;
}

void* slanv2_(void)
{
    if (verbose) puts("STUB: slanv2_ called");
    return NULL;
}

void* slapll(void)
{
    if (verbose) puts("STUB: slapll called");
    return NULL;
}

void* slapll_(void)
{
    if (verbose) puts("STUB: slapll_ called");
    return NULL;
}

void* slapmt(void)
{
    if (verbose) puts("STUB: slapmt called");
    return NULL;
}

void* slapmt_(void)
{
    if (verbose) puts("STUB: slapmt_ called");
    return NULL;
}

void* slapy2(void)
{
    if (verbose) puts("STUB: slapy2 called");
    return NULL;
}

void* slapy2_(void)
{
    if (verbose) puts("STUB: slapy2_ called");
    return NULL;
}

void* slapy3(void)
{
    if (verbose) puts("STUB: slapy3 called");
    return NULL;
}

void* slapy3_(void)
{
    if (verbose) puts("STUB: slapy3_ called");
    return NULL;
}

void* slaqgb(void)
{
    if (verbose) puts("STUB: slaqgb called");
    return NULL;
}

void* slaqgb_(void)
{
    if (verbose) puts("STUB: slaqgb_ called");
    return NULL;
}

void* slaqge(void)
{
    if (verbose) puts("STUB: slaqge called");
    return NULL;
}

void* slaqge_(void)
{
    if (verbose) puts("STUB: slaqge_ called");
    return NULL;
}

void* slaqp2(void)
{
    if (verbose) puts("STUB: slaqp2 called");
    return NULL;
}

void* slaqp2_(void)
{
    if (verbose) puts("STUB: slaqp2_ called");
    return NULL;
}

void* slaqps(void)
{
    if (verbose) puts("STUB: slaqps called");
    return NULL;
}

void* slaqps_(void)
{
    if (verbose) puts("STUB: slaqps_ called");
    return NULL;
}

void* slaqr0(void)
{
    if (verbose) puts("STUB: slaqr0 called");
    return NULL;
}

void* slaqr0_(void)
{
    if (verbose) puts("STUB: slaqr0_ called");
    return NULL;
}

void* slaqr1(void)
{
    if (verbose) puts("STUB: slaqr1 called");
    return NULL;
}

void* slaqr1_(void)
{
    if (verbose) puts("STUB: slaqr1_ called");
    return NULL;
}

void* slaqr2(void)
{
    if (verbose) puts("STUB: slaqr2 called");
    return NULL;
}

void* slaqr2_(void)
{
    if (verbose) puts("STUB: slaqr2_ called");
    return NULL;
}

void* slaqr3(void)
{
    if (verbose) puts("STUB: slaqr3 called");
    return NULL;
}

void* slaqr3_(void)
{
    if (verbose) puts("STUB: slaqr3_ called");
    return NULL;
}

void* slaqr4(void)
{
    if (verbose) puts("STUB: slaqr4 called");
    return NULL;
}

void* slaqr4_(void)
{
    if (verbose) puts("STUB: slaqr4_ called");
    return NULL;
}

void* slaqr5(void)
{
    if (verbose) puts("STUB: slaqr5 called");
    return NULL;
}

void* slaqr5_(void)
{
    if (verbose) puts("STUB: slaqr5_ called");
    return NULL;
}

void* slaqsb(void)
{
    if (verbose) puts("STUB: slaqsb called");
    return NULL;
}

void* slaqsb_(void)
{
    if (verbose) puts("STUB: slaqsb_ called");
    return NULL;
}

void* slaqsp(void)
{
    if (verbose) puts("STUB: slaqsp called");
    return NULL;
}

void* slaqsp_(void)
{
    if (verbose) puts("STUB: slaqsp_ called");
    return NULL;
}

void* slaqsy(void)
{
    if (verbose) puts("STUB: slaqsy called");
    return NULL;
}

void* slaqsy_(void)
{
    if (verbose) puts("STUB: slaqsy_ called");
    return NULL;
}

void* slaqtr(void)
{
    if (verbose) puts("STUB: slaqtr called");
    return NULL;
}

void* slaqtr_(void)
{
    if (verbose) puts("STUB: slaqtr_ called");
    return NULL;
}

void* slar1v(void)
{
    if (verbose) puts("STUB: slar1v called");
    return NULL;
}

void* slar1v_(void)
{
    if (verbose) puts("STUB: slar1v_ called");
    return NULL;
}

void* slar2v(void)
{
    if (verbose) puts("STUB: slar2v called");
    return NULL;
}

void* slar2v_(void)
{
    if (verbose) puts("STUB: slar2v_ called");
    return NULL;
}

void* slarf(void)
{
    if (verbose) puts("STUB: slarf called");
    return NULL;
}

void* slarf_(void)
{
    if (verbose) puts("STUB: slarf_ called");
    return NULL;
}

void* slarfb(void)
{
    if (verbose) puts("STUB: slarfb called");
    return NULL;
}

void* slarfb_(void)
{
    if (verbose) puts("STUB: slarfb_ called");
    return NULL;
}

void* slarfg(void)
{
    if (verbose) puts("STUB: slarfg called");
    return NULL;
}

void* slarfg_(void)
{
    if (verbose) puts("STUB: slarfg_ called");
    return NULL;
}

void* slarfp(void)
{
    if (verbose) puts("STUB: slarfp called");
    return NULL;
}

void* slarfp_(void)
{
    if (verbose) puts("STUB: slarfp_ called");
    return NULL;
}

void* slarft(void)
{
    if (verbose) puts("STUB: slarft called");
    return NULL;
}

void* slarft_(void)
{
    if (verbose) puts("STUB: slarft_ called");
    return NULL;
}

void* slarfx(void)
{
    if (verbose) puts("STUB: slarfx called");
    return NULL;
}

void* slarfx_(void)
{
    if (verbose) puts("STUB: slarfx_ called");
    return NULL;
}

void* slargv(void)
{
    if (verbose) puts("STUB: slargv called");
    return NULL;
}

void* slargv_(void)
{
    if (verbose) puts("STUB: slargv_ called");
    return NULL;
}

void* slarnv(void)
{
    if (verbose) puts("STUB: slarnv called");
    return NULL;
}

void* slarnv_(void)
{
    if (verbose) puts("STUB: slarnv_ called");
    return NULL;
}

void* slarra(void)
{
    if (verbose) puts("STUB: slarra called");
    return NULL;
}

void* slarra_(void)
{
    if (verbose) puts("STUB: slarra_ called");
    return NULL;
}

void* slarrb(void)
{
    if (verbose) puts("STUB: slarrb called");
    return NULL;
}

void* slarrb_(void)
{
    if (verbose) puts("STUB: slarrb_ called");
    return NULL;
}

void* slarrc(void)
{
    if (verbose) puts("STUB: slarrc called");
    return NULL;
}

void* slarrc_(void)
{
    if (verbose) puts("STUB: slarrc_ called");
    return NULL;
}

void* slarrd(void)
{
    if (verbose) puts("STUB: slarrd called");
    return NULL;
}

void* slarrd_(void)
{
    if (verbose) puts("STUB: slarrd_ called");
    return NULL;
}

void* slarre(void)
{
    if (verbose) puts("STUB: slarre called");
    return NULL;
}

void* slarre_(void)
{
    if (verbose) puts("STUB: slarre_ called");
    return NULL;
}

void* slarrf(void)
{
    if (verbose) puts("STUB: slarrf called");
    return NULL;
}

void* slarrf_(void)
{
    if (verbose) puts("STUB: slarrf_ called");
    return NULL;
}

void* slarrj(void)
{
    if (verbose) puts("STUB: slarrj called");
    return NULL;
}

void* slarrj_(void)
{
    if (verbose) puts("STUB: slarrj_ called");
    return NULL;
}

void* slarrk(void)
{
    if (verbose) puts("STUB: slarrk called");
    return NULL;
}

void* slarrk_(void)
{
    if (verbose) puts("STUB: slarrk_ called");
    return NULL;
}

void* slarrr(void)
{
    if (verbose) puts("STUB: slarrr called");
    return NULL;
}

void* slarrr_(void)
{
    if (verbose) puts("STUB: slarrr_ called");
    return NULL;
}

void* slarrv(void)
{
    if (verbose) puts("STUB: slarrv called");
    return NULL;
}

void* slarrv_(void)
{
    if (verbose) puts("STUB: slarrv_ called");
    return NULL;
}

void* slarscl2(void)
{
    if (verbose) puts("STUB: slarscl2 called");
    return NULL;
}

void* slarscl2_(void)
{
    if (verbose) puts("STUB: slarscl2_ called");
    return NULL;
}

void* slartg(void)
{
    if (verbose) puts("STUB: slartg called");
    return NULL;
}

void* slartg_(void)
{
    if (verbose) puts("STUB: slartg_ called");
    return NULL;
}

void* slartv(void)
{
    if (verbose) puts("STUB: slartv called");
    return NULL;
}

void* slartv_(void)
{
    if (verbose) puts("STUB: slartv_ called");
    return NULL;
}

void* slaruv(void)
{
    if (verbose) puts("STUB: slaruv called");
    return NULL;
}

void* slaruv_(void)
{
    if (verbose) puts("STUB: slaruv_ called");
    return NULL;
}

void* slarz(void)
{
    if (verbose) puts("STUB: slarz called");
    return NULL;
}

void* slarz_(void)
{
    if (verbose) puts("STUB: slarz_ called");
    return NULL;
}

void* slarzb(void)
{
    if (verbose) puts("STUB: slarzb called");
    return NULL;
}

void* slarzb_(void)
{
    if (verbose) puts("STUB: slarzb_ called");
    return NULL;
}

void* slarzt(void)
{
    if (verbose) puts("STUB: slarzt called");
    return NULL;
}

void* slarzt_(void)
{
    if (verbose) puts("STUB: slarzt_ called");
    return NULL;
}

void* slas2(void)
{
    if (verbose) puts("STUB: slas2 called");
    return NULL;
}

void* slas2_(void)
{
    if (verbose) puts("STUB: slas2_ called");
    return NULL;
}

void* slascl(void)
{
    if (verbose) puts("STUB: slascl called");
    return NULL;
}

void* slascl2(void)
{
    if (verbose) puts("STUB: slascl2 called");
    return NULL;
}

void* slascl2_(void)
{
    if (verbose) puts("STUB: slascl2_ called");
    return NULL;
}

void* slascl_(void)
{
    if (verbose) puts("STUB: slascl_ called");
    return NULL;
}

void* slasd0(void)
{
    if (verbose) puts("STUB: slasd0 called");
    return NULL;
}

void* slasd0_(void)
{
    if (verbose) puts("STUB: slasd0_ called");
    return NULL;
}

void* slasd1(void)
{
    if (verbose) puts("STUB: slasd1 called");
    return NULL;
}

void* slasd1_(void)
{
    if (verbose) puts("STUB: slasd1_ called");
    return NULL;
}

void* slasd2(void)
{
    if (verbose) puts("STUB: slasd2 called");
    return NULL;
}

void* slasd2_(void)
{
    if (verbose) puts("STUB: slasd2_ called");
    return NULL;
}

void* slasd3(void)
{
    if (verbose) puts("STUB: slasd3 called");
    return NULL;
}

void* slasd3_(void)
{
    if (verbose) puts("STUB: slasd3_ called");
    return NULL;
}

void* slasd4(void)
{
    if (verbose) puts("STUB: slasd4 called");
    return NULL;
}

void* slasd4_(void)
{
    if (verbose) puts("STUB: slasd4_ called");
    return NULL;
}

void* slasd5(void)
{
    if (verbose) puts("STUB: slasd5 called");
    return NULL;
}

void* slasd5_(void)
{
    if (verbose) puts("STUB: slasd5_ called");
    return NULL;
}

void* slasd6(void)
{
    if (verbose) puts("STUB: slasd6 called");
    return NULL;
}

void* slasd6_(void)
{
    if (verbose) puts("STUB: slasd6_ called");
    return NULL;
}

void* slasd7(void)
{
    if (verbose) puts("STUB: slasd7 called");
    return NULL;
}

void* slasd7_(void)
{
    if (verbose) puts("STUB: slasd7_ called");
    return NULL;
}

void* slasd8(void)
{
    if (verbose) puts("STUB: slasd8 called");
    return NULL;
}

void* slasd8_(void)
{
    if (verbose) puts("STUB: slasd8_ called");
    return NULL;
}

void* slasd9(void)
{
    if (verbose) puts("STUB: slasd9 called");
    return NULL;
}

void* slasd9_(void)
{
    if (verbose) puts("STUB: slasd9_ called");
    return NULL;
}

void* slasda(void)
{
    if (verbose) puts("STUB: slasda called");
    return NULL;
}

void* slasda_(void)
{
    if (verbose) puts("STUB: slasda_ called");
    return NULL;
}

void* slasdq(void)
{
    if (verbose) puts("STUB: slasdq called");
    return NULL;
}

void* slasdq_(void)
{
    if (verbose) puts("STUB: slasdq_ called");
    return NULL;
}

void* slasdt(void)
{
    if (verbose) puts("STUB: slasdt called");
    return NULL;
}

void* slasdt_(void)
{
    if (verbose) puts("STUB: slasdt_ called");
    return NULL;
}

void* slaset(void)
{
    if (verbose) puts("STUB: slaset called");
    return NULL;
}

void* slaset_(void)
{
    if (verbose) puts("STUB: slaset_ called");
    return NULL;
}

void* slasq1(void)
{
    if (verbose) puts("STUB: slasq1 called");
    return NULL;
}

void* slasq1_(void)
{
    if (verbose) puts("STUB: slasq1_ called");
    return NULL;
}

void* slasq2(void)
{
    if (verbose) puts("STUB: slasq2 called");
    return NULL;
}

void* slasq2_(void)
{
    if (verbose) puts("STUB: slasq2_ called");
    return NULL;
}

void* slasq3(void)
{
    if (verbose) puts("STUB: slasq3 called");
    return NULL;
}

void* slasq3_(void)
{
    if (verbose) puts("STUB: slasq3_ called");
    return NULL;
}

void* slasq4(void)
{
    if (verbose) puts("STUB: slasq4 called");
    return NULL;
}

void* slasq4_(void)
{
    if (verbose) puts("STUB: slasq4_ called");
    return NULL;
}

void* slasq5(void)
{
    if (verbose) puts("STUB: slasq5 called");
    return NULL;
}

void* slasq5_(void)
{
    if (verbose) puts("STUB: slasq5_ called");
    return NULL;
}

void* slasq6(void)
{
    if (verbose) puts("STUB: slasq6 called");
    return NULL;
}

void* slasq6_(void)
{
    if (verbose) puts("STUB: slasq6_ called");
    return NULL;
}

void* slasr(void)
{
    if (verbose) puts("STUB: slasr called");
    return NULL;
}

void* slasr_(void)
{
    if (verbose) puts("STUB: slasr_ called");
    return NULL;
}

void* slasrt(void)
{
    if (verbose) puts("STUB: slasrt called");
    return NULL;
}

void* slasrt_(void)
{
    if (verbose) puts("STUB: slasrt_ called");
    return NULL;
}

void* slassq(void)
{
    if (verbose) puts("STUB: slassq called");
    return NULL;
}

void* slassq_(void)
{
    if (verbose) puts("STUB: slassq_ called");
    return NULL;
}

void* slasv2(void)
{
    if (verbose) puts("STUB: slasv2 called");
    return NULL;
}

void* slasv2_(void)
{
    if (verbose) puts("STUB: slasv2_ called");
    return NULL;
}

void* slaswp(void)
{
    if (verbose) puts("STUB: slaswp called");
    return NULL;
}

void* slaswp_(void)
{
    if (verbose) puts("STUB: slaswp_ called");
    return NULL;
}

void* slasy2(void)
{
    if (verbose) puts("STUB: slasy2 called");
    return NULL;
}

void* slasy2_(void)
{
    if (verbose) puts("STUB: slasy2_ called");
    return NULL;
}

void* slasyf(void)
{
    if (verbose) puts("STUB: slasyf called");
    return NULL;
}

void* slasyf_(void)
{
    if (verbose) puts("STUB: slasyf_ called");
    return NULL;
}

void* slatbs(void)
{
    if (verbose) puts("STUB: slatbs called");
    return NULL;
}

void* slatbs_(void)
{
    if (verbose) puts("STUB: slatbs_ called");
    return NULL;
}

void* slatdf(void)
{
    if (verbose) puts("STUB: slatdf called");
    return NULL;
}

void* slatdf_(void)
{
    if (verbose) puts("STUB: slatdf_ called");
    return NULL;
}

void* slatps(void)
{
    if (verbose) puts("STUB: slatps called");
    return NULL;
}

void* slatps_(void)
{
    if (verbose) puts("STUB: slatps_ called");
    return NULL;
}

void* slatrd(void)
{
    if (verbose) puts("STUB: slatrd called");
    return NULL;
}

void* slatrd_(void)
{
    if (verbose) puts("STUB: slatrd_ called");
    return NULL;
}

void* slatrs(void)
{
    if (verbose) puts("STUB: slatrs called");
    return NULL;
}

void* slatrs_(void)
{
    if (verbose) puts("STUB: slatrs_ called");
    return NULL;
}

void* slatrz(void)
{
    if (verbose) puts("STUB: slatrz called");
    return NULL;
}

void* slatrz_(void)
{
    if (verbose) puts("STUB: slatrz_ called");
    return NULL;
}

void* slatzm(void)
{
    if (verbose) puts("STUB: slatzm called");
    return NULL;
}

void* slatzm_(void)
{
    if (verbose) puts("STUB: slatzm_ called");
    return NULL;
}

void* slauu2(void)
{
    if (verbose) puts("STUB: slauu2 called");
    return NULL;
}

void* slauu2_(void)
{
    if (verbose) puts("STUB: slauu2_ called");
    return NULL;
}

void* slauum(void)
{
    if (verbose) puts("STUB: slauum called");
    return NULL;
}

void* slauum_(void)
{
    if (verbose) puts("STUB: slauum_ called");
    return NULL;
}

void* smaxloc(void)
{
    if (verbose) puts("STUB: smaxloc called");
    return NULL;
}

void* smaxloc_(void)
{
    if (verbose) puts("STUB: smaxloc_ called");
    return NULL;
}

void* sopgtr(void)
{
    if (verbose) puts("STUB: sopgtr called");
    return NULL;
}

void* sopgtr_(void)
{
    if (verbose) puts("STUB: sopgtr_ called");
    return NULL;
}

void* sopmtr(void)
{
    if (verbose) puts("STUB: sopmtr called");
    return NULL;
}

void* sopmtr_(void)
{
    if (verbose) puts("STUB: sopmtr_ called");
    return NULL;
}

void* sorg2l(void)
{
    if (verbose) puts("STUB: sorg2l called");
    return NULL;
}

void* sorg2l_(void)
{
    if (verbose) puts("STUB: sorg2l_ called");
    return NULL;
}

void* sorg2r(void)
{
    if (verbose) puts("STUB: sorg2r called");
    return NULL;
}

void* sorg2r_(void)
{
    if (verbose) puts("STUB: sorg2r_ called");
    return NULL;
}

void* sorgbr(void)
{
    if (verbose) puts("STUB: sorgbr called");
    return NULL;
}

void* sorgbr_(void)
{
    if (verbose) puts("STUB: sorgbr_ called");
    return NULL;
}

void* sorghr(void)
{
    if (verbose) puts("STUB: sorghr called");
    return NULL;
}

void* sorghr_(void)
{
    if (verbose) puts("STUB: sorghr_ called");
    return NULL;
}

void* sorgl2(void)
{
    if (verbose) puts("STUB: sorgl2 called");
    return NULL;
}

void* sorgl2_(void)
{
    if (verbose) puts("STUB: sorgl2_ called");
    return NULL;
}

void* sorglq(void)
{
    if (verbose) puts("STUB: sorglq called");
    return NULL;
}

void* sorglq_(void)
{
    if (verbose) puts("STUB: sorglq_ called");
    return NULL;
}

void* sorgql(void)
{
    if (verbose) puts("STUB: sorgql called");
    return NULL;
}

void* sorgql_(void)
{
    if (verbose) puts("STUB: sorgql_ called");
    return NULL;
}

void* sorgqr(void)
{
    if (verbose) puts("STUB: sorgqr called");
    return NULL;
}

void* sorgqr_(void)
{
    if (verbose) puts("STUB: sorgqr_ called");
    return NULL;
}

void* sorgr2(void)
{
    if (verbose) puts("STUB: sorgr2 called");
    return NULL;
}

void* sorgr2_(void)
{
    if (verbose) puts("STUB: sorgr2_ called");
    return NULL;
}

void* sorgrq(void)
{
    if (verbose) puts("STUB: sorgrq called");
    return NULL;
}

void* sorgrq_(void)
{
    if (verbose) puts("STUB: sorgrq_ called");
    return NULL;
}

void* sorgtr(void)
{
    if (verbose) puts("STUB: sorgtr called");
    return NULL;
}

void* sorgtr_(void)
{
    if (verbose) puts("STUB: sorgtr_ called");
    return NULL;
}

void* sorm2l(void)
{
    if (verbose) puts("STUB: sorm2l called");
    return NULL;
}

void* sorm2l_(void)
{
    if (verbose) puts("STUB: sorm2l_ called");
    return NULL;
}

void* sorm2r(void)
{
    if (verbose) puts("STUB: sorm2r called");
    return NULL;
}

void* sorm2r_(void)
{
    if (verbose) puts("STUB: sorm2r_ called");
    return NULL;
}

void* sormbr(void)
{
    if (verbose) puts("STUB: sormbr called");
    return NULL;
}

void* sormbr_(void)
{
    if (verbose) puts("STUB: sormbr_ called");
    return NULL;
}

void* sormhr(void)
{
    if (verbose) puts("STUB: sormhr called");
    return NULL;
}

void* sormhr_(void)
{
    if (verbose) puts("STUB: sormhr_ called");
    return NULL;
}

void* sorml2(void)
{
    if (verbose) puts("STUB: sorml2 called");
    return NULL;
}

void* sorml2_(void)
{
    if (verbose) puts("STUB: sorml2_ called");
    return NULL;
}

void* sormlq(void)
{
    if (verbose) puts("STUB: sormlq called");
    return NULL;
}

void* sormlq_(void)
{
    if (verbose) puts("STUB: sormlq_ called");
    return NULL;
}

void* sormql(void)
{
    if (verbose) puts("STUB: sormql called");
    return NULL;
}

void* sormql_(void)
{
    if (verbose) puts("STUB: sormql_ called");
    return NULL;
}

void* sormqr(void)
{
    if (verbose) puts("STUB: sormqr called");
    return NULL;
}

void* sormqr_(void)
{
    if (verbose) puts("STUB: sormqr_ called");
    return NULL;
}

void* sormr2(void)
{
    if (verbose) puts("STUB: sormr2 called");
    return NULL;
}

void* sormr2_(void)
{
    if (verbose) puts("STUB: sormr2_ called");
    return NULL;
}

void* sormr3(void)
{
    if (verbose) puts("STUB: sormr3 called");
    return NULL;
}

void* sormr3_(void)
{
    if (verbose) puts("STUB: sormr3_ called");
    return NULL;
}

void* sormrq(void)
{
    if (verbose) puts("STUB: sormrq called");
    return NULL;
}

void* sormrq_(void)
{
    if (verbose) puts("STUB: sormrq_ called");
    return NULL;
}

void* sormrz(void)
{
    if (verbose) puts("STUB: sormrz called");
    return NULL;
}

void* sormrz_(void)
{
    if (verbose) puts("STUB: sormrz_ called");
    return NULL;
}

void* sormtr(void)
{
    if (verbose) puts("STUB: sormtr called");
    return NULL;
}

void* sormtr_(void)
{
    if (verbose) puts("STUB: sormtr_ called");
    return NULL;
}

void* spbcon(void)
{
    if (verbose) puts("STUB: spbcon called");
    return NULL;
}

void* spbcon_(void)
{
    if (verbose) puts("STUB: spbcon_ called");
    return NULL;
}

void* spbequ(void)
{
    if (verbose) puts("STUB: spbequ called");
    return NULL;
}

void* spbequ_(void)
{
    if (verbose) puts("STUB: spbequ_ called");
    return NULL;
}

void* spbrfs(void)
{
    if (verbose) puts("STUB: spbrfs called");
    return NULL;
}

void* spbrfs_(void)
{
    if (verbose) puts("STUB: spbrfs_ called");
    return NULL;
}

void* spbstf(void)
{
    if (verbose) puts("STUB: spbstf called");
    return NULL;
}

void* spbstf_(void)
{
    if (verbose) puts("STUB: spbstf_ called");
    return NULL;
}

void* spbsv(void)
{
    if (verbose) puts("STUB: spbsv called");
    return NULL;
}

void* spbsv_(void)
{
    if (verbose) puts("STUB: spbsv_ called");
    return NULL;
}

void* spbsvx(void)
{
    if (verbose) puts("STUB: spbsvx called");
    return NULL;
}

void* spbsvx_(void)
{
    if (verbose) puts("STUB: spbsvx_ called");
    return NULL;
}

void* spbtf2(void)
{
    if (verbose) puts("STUB: spbtf2 called");
    return NULL;
}

void* spbtf2_(void)
{
    if (verbose) puts("STUB: spbtf2_ called");
    return NULL;
}

void* spbtrf(void)
{
    if (verbose) puts("STUB: spbtrf called");
    return NULL;
}

void* spbtrf_(void)
{
    if (verbose) puts("STUB: spbtrf_ called");
    return NULL;
}

void* spbtrs(void)
{
    if (verbose) puts("STUB: spbtrs called");
    return NULL;
}

void* spbtrs_(void)
{
    if (verbose) puts("STUB: spbtrs_ called");
    return NULL;
}

void* spftrf(void)
{
    if (verbose) puts("STUB: spftrf called");
    return NULL;
}

void* spftrf_(void)
{
    if (verbose) puts("STUB: spftrf_ called");
    return NULL;
}

void* spftri(void)
{
    if (verbose) puts("STUB: spftri called");
    return NULL;
}

void* spftri_(void)
{
    if (verbose) puts("STUB: spftri_ called");
    return NULL;
}

void* spftrs(void)
{
    if (verbose) puts("STUB: spftrs called");
    return NULL;
}

void* spftrs_(void)
{
    if (verbose) puts("STUB: spftrs_ called");
    return NULL;
}

void* spocon(void)
{
    if (verbose) puts("STUB: spocon called");
    return NULL;
}

void* spocon_(void)
{
    if (verbose) puts("STUB: spocon_ called");
    return NULL;
}

void* spoequ(void)
{
    if (verbose) puts("STUB: spoequ called");
    return NULL;
}

void* spoequ_(void)
{
    if (verbose) puts("STUB: spoequ_ called");
    return NULL;
}

void* spoequb(void)
{
    if (verbose) puts("STUB: spoequb called");
    return NULL;
}

void* spoequb_(void)
{
    if (verbose) puts("STUB: spoequb_ called");
    return NULL;
}

void* sporfs(void)
{
    if (verbose) puts("STUB: sporfs called");
    return NULL;
}

void* sporfs_(void)
{
    if (verbose) puts("STUB: sporfs_ called");
    return NULL;
}

void* sposv(void)
{
    if (verbose) puts("STUB: sposv called");
    return NULL;
}

void* sposv_(void)
{
    if (verbose) puts("STUB: sposv_ called");
    return NULL;
}

void* sposvx(void)
{
    if (verbose) puts("STUB: sposvx called");
    return NULL;
}

void* sposvx_(void)
{
    if (verbose) puts("STUB: sposvx_ called");
    return NULL;
}

void* spotf2(void)
{
    if (verbose) puts("STUB: spotf2 called");
    return NULL;
}

void* spotf2_(void)
{
    if (verbose) puts("STUB: spotf2_ called");
    return NULL;
}

void* spotrf(void)
{
    if (verbose) puts("STUB: spotrf called");
    return NULL;
}

void* spotrf_(void)
{
    if (verbose) puts("STUB: spotrf_ called");
    return NULL;
}

void* spotri(void)
{
    if (verbose) puts("STUB: spotri called");
    return NULL;
}

void* spotri_(void)
{
    if (verbose) puts("STUB: spotri_ called");
    return NULL;
}

void* spotrs(void)
{
    if (verbose) puts("STUB: spotrs called");
    return NULL;
}

void* spotrs_(void)
{
    if (verbose) puts("STUB: spotrs_ called");
    return NULL;
}

void* sppcon(void)
{
    if (verbose) puts("STUB: sppcon called");
    return NULL;
}

void* sppcon_(void)
{
    if (verbose) puts("STUB: sppcon_ called");
    return NULL;
}

void* sppequ(void)
{
    if (verbose) puts("STUB: sppequ called");
    return NULL;
}

void* sppequ_(void)
{
    if (verbose) puts("STUB: sppequ_ called");
    return NULL;
}

void* spprfs(void)
{
    if (verbose) puts("STUB: spprfs called");
    return NULL;
}

void* spprfs_(void)
{
    if (verbose) puts("STUB: spprfs_ called");
    return NULL;
}

void* sppsv(void)
{
    if (verbose) puts("STUB: sppsv called");
    return NULL;
}

void* sppsv_(void)
{
    if (verbose) puts("STUB: sppsv_ called");
    return NULL;
}

void* sppsvx(void)
{
    if (verbose) puts("STUB: sppsvx called");
    return NULL;
}

void* sppsvx_(void)
{
    if (verbose) puts("STUB: sppsvx_ called");
    return NULL;
}

void* spptrf(void)
{
    if (verbose) puts("STUB: spptrf called");
    return NULL;
}

void* spptrf_(void)
{
    if (verbose) puts("STUB: spptrf_ called");
    return NULL;
}

void* spptri(void)
{
    if (verbose) puts("STUB: spptri called");
    return NULL;
}

void* spptri_(void)
{
    if (verbose) puts("STUB: spptri_ called");
    return NULL;
}

void* spptrs(void)
{
    if (verbose) puts("STUB: spptrs called");
    return NULL;
}

void* spptrs_(void)
{
    if (verbose) puts("STUB: spptrs_ called");
    return NULL;
}

void* spstf2(void)
{
    if (verbose) puts("STUB: spstf2 called");
    return NULL;
}

void* spstf2_(void)
{
    if (verbose) puts("STUB: spstf2_ called");
    return NULL;
}

void* spstrf(void)
{
    if (verbose) puts("STUB: spstrf called");
    return NULL;
}

void* spstrf_(void)
{
    if (verbose) puts("STUB: spstrf_ called");
    return NULL;
}

void* sptcon(void)
{
    if (verbose) puts("STUB: sptcon called");
    return NULL;
}

void* sptcon_(void)
{
    if (verbose) puts("STUB: sptcon_ called");
    return NULL;
}

void* spteqr(void)
{
    if (verbose) puts("STUB: spteqr called");
    return NULL;
}

void* spteqr_(void)
{
    if (verbose) puts("STUB: spteqr_ called");
    return NULL;
}

void* sptrfs(void)
{
    if (verbose) puts("STUB: sptrfs called");
    return NULL;
}

void* sptrfs_(void)
{
    if (verbose) puts("STUB: sptrfs_ called");
    return NULL;
}

void* sptsv(void)
{
    if (verbose) puts("STUB: sptsv called");
    return NULL;
}

void* sptsv_(void)
{
    if (verbose) puts("STUB: sptsv_ called");
    return NULL;
}

void* sptsvx(void)
{
    if (verbose) puts("STUB: sptsvx called");
    return NULL;
}

void* sptsvx_(void)
{
    if (verbose) puts("STUB: sptsvx_ called");
    return NULL;
}

void* spttrf(void)
{
    if (verbose) puts("STUB: spttrf called");
    return NULL;
}

void* spttrf_(void)
{
    if (verbose) puts("STUB: spttrf_ called");
    return NULL;
}

void* spttrs(void)
{
    if (verbose) puts("STUB: spttrs called");
    return NULL;
}

void* spttrs_(void)
{
    if (verbose) puts("STUB: spttrs_ called");
    return NULL;
}

void* sptts2(void)
{
    if (verbose) puts("STUB: sptts2 called");
    return NULL;
}

void* sptts2_(void)
{
    if (verbose) puts("STUB: sptts2_ called");
    return NULL;
}

void* srscl(void)
{
    if (verbose) puts("STUB: srscl called");
    return NULL;
}

void* srscl_(void)
{
    if (verbose) puts("STUB: srscl_ called");
    return NULL;
}

void* ssbev(void)
{
    if (verbose) puts("STUB: ssbev called");
    return NULL;
}

void* ssbev_(void)
{
    if (verbose) puts("STUB: ssbev_ called");
    return NULL;
}

void* ssbevd(void)
{
    if (verbose) puts("STUB: ssbevd called");
    return NULL;
}

void* ssbevd_(void)
{
    if (verbose) puts("STUB: ssbevd_ called");
    return NULL;
}

void* ssbevx(void)
{
    if (verbose) puts("STUB: ssbevx called");
    return NULL;
}

void* ssbevx_(void)
{
    if (verbose) puts("STUB: ssbevx_ called");
    return NULL;
}

void* ssbgst(void)
{
    if (verbose) puts("STUB: ssbgst called");
    return NULL;
}

void* ssbgst_(void)
{
    if (verbose) puts("STUB: ssbgst_ called");
    return NULL;
}

void* ssbgv(void)
{
    if (verbose) puts("STUB: ssbgv called");
    return NULL;
}

void* ssbgv_(void)
{
    if (verbose) puts("STUB: ssbgv_ called");
    return NULL;
}

void* ssbgvd(void)
{
    if (verbose) puts("STUB: ssbgvd called");
    return NULL;
}

void* ssbgvd_(void)
{
    if (verbose) puts("STUB: ssbgvd_ called");
    return NULL;
}

void* ssbgvx(void)
{
    if (verbose) puts("STUB: ssbgvx called");
    return NULL;
}

void* ssbgvx_(void)
{
    if (verbose) puts("STUB: ssbgvx_ called");
    return NULL;
}

void* ssbtrd(void)
{
    if (verbose) puts("STUB: ssbtrd called");
    return NULL;
}

void* ssbtrd_(void)
{
    if (verbose) puts("STUB: ssbtrd_ called");
    return NULL;
}

void* ssfrk(void)
{
    if (verbose) puts("STUB: ssfrk called");
    return NULL;
}

void* ssfrk_(void)
{
    if (verbose) puts("STUB: ssfrk_ called");
    return NULL;
}

void* sspcon(void)
{
    if (verbose) puts("STUB: sspcon called");
    return NULL;
}

void* sspcon_(void)
{
    if (verbose) puts("STUB: sspcon_ called");
    return NULL;
}

void* sspev(void)
{
    if (verbose) puts("STUB: sspev called");
    return NULL;
}

void* sspev_(void)
{
    if (verbose) puts("STUB: sspev_ called");
    return NULL;
}

void* sspevd(void)
{
    if (verbose) puts("STUB: sspevd called");
    return NULL;
}

void* sspevd_(void)
{
    if (verbose) puts("STUB: sspevd_ called");
    return NULL;
}

void* sspevx(void)
{
    if (verbose) puts("STUB: sspevx called");
    return NULL;
}

void* sspevx_(void)
{
    if (verbose) puts("STUB: sspevx_ called");
    return NULL;
}

void* sspgst(void)
{
    if (verbose) puts("STUB: sspgst called");
    return NULL;
}

void* sspgst_(void)
{
    if (verbose) puts("STUB: sspgst_ called");
    return NULL;
}

void* sspgv(void)
{
    if (verbose) puts("STUB: sspgv called");
    return NULL;
}

void* sspgv_(void)
{
    if (verbose) puts("STUB: sspgv_ called");
    return NULL;
}

void* sspgvd(void)
{
    if (verbose) puts("STUB: sspgvd called");
    return NULL;
}

void* sspgvd_(void)
{
    if (verbose) puts("STUB: sspgvd_ called");
    return NULL;
}

void* sspgvx(void)
{
    if (verbose) puts("STUB: sspgvx called");
    return NULL;
}

void* sspgvx_(void)
{
    if (verbose) puts("STUB: sspgvx_ called");
    return NULL;
}

void* ssprfs(void)
{
    if (verbose) puts("STUB: ssprfs called");
    return NULL;
}

void* ssprfs_(void)
{
    if (verbose) puts("STUB: ssprfs_ called");
    return NULL;
}

void* sspsv(void)
{
    if (verbose) puts("STUB: sspsv called");
    return NULL;
}

void* sspsv_(void)
{
    if (verbose) puts("STUB: sspsv_ called");
    return NULL;
}

void* sspsvx(void)
{
    if (verbose) puts("STUB: sspsvx called");
    return NULL;
}

void* sspsvx_(void)
{
    if (verbose) puts("STUB: sspsvx_ called");
    return NULL;
}

void* ssptrd(void)
{
    if (verbose) puts("STUB: ssptrd called");
    return NULL;
}

void* ssptrd_(void)
{
    if (verbose) puts("STUB: ssptrd_ called");
    return NULL;
}

void* ssptrf(void)
{
    if (verbose) puts("STUB: ssptrf called");
    return NULL;
}

void* ssptrf_(void)
{
    if (verbose) puts("STUB: ssptrf_ called");
    return NULL;
}

void* ssptri(void)
{
    if (verbose) puts("STUB: ssptri called");
    return NULL;
}

void* ssptri_(void)
{
    if (verbose) puts("STUB: ssptri_ called");
    return NULL;
}

void* ssptrs(void)
{
    if (verbose) puts("STUB: ssptrs called");
    return NULL;
}

void* ssptrs_(void)
{
    if (verbose) puts("STUB: ssptrs_ called");
    return NULL;
}

void* sstebz(void)
{
    if (verbose) puts("STUB: sstebz called");
    return NULL;
}

void* sstebz_(void)
{
    if (verbose) puts("STUB: sstebz_ called");
    return NULL;
}

void* sstedc(void)
{
    if (verbose) puts("STUB: sstedc called");
    return NULL;
}

void* sstedc_(void)
{
    if (verbose) puts("STUB: sstedc_ called");
    return NULL;
}

void* sstegr(void)
{
    if (verbose) puts("STUB: sstegr called");
    return NULL;
}

void* sstegr_(void)
{
    if (verbose) puts("STUB: sstegr_ called");
    return NULL;
}

void* sstein(void)
{
    if (verbose) puts("STUB: sstein called");
    return NULL;
}

void* sstein_(void)
{
    if (verbose) puts("STUB: sstein_ called");
    return NULL;
}

void* sstemr(void)
{
    if (verbose) puts("STUB: sstemr called");
    return NULL;
}

void* sstemr_(void)
{
    if (verbose) puts("STUB: sstemr_ called");
    return NULL;
}

void* ssteqr(void)
{
    if (verbose) puts("STUB: ssteqr called");
    return NULL;
}

void* ssteqr_(void)
{
    if (verbose) puts("STUB: ssteqr_ called");
    return NULL;
}

void* ssterf(void)
{
    if (verbose) puts("STUB: ssterf called");
    return NULL;
}

void* ssterf_(void)
{
    if (verbose) puts("STUB: ssterf_ called");
    return NULL;
}

void* sstev(void)
{
    if (verbose) puts("STUB: sstev called");
    return NULL;
}

void* sstev_(void)
{
    if (verbose) puts("STUB: sstev_ called");
    return NULL;
}

void* sstevd(void)
{
    if (verbose) puts("STUB: sstevd called");
    return NULL;
}

void* sstevd_(void)
{
    if (verbose) puts("STUB: sstevd_ called");
    return NULL;
}

void* sstevr(void)
{
    if (verbose) puts("STUB: sstevr called");
    return NULL;
}

void* sstevr_(void)
{
    if (verbose) puts("STUB: sstevr_ called");
    return NULL;
}

void* sstevx(void)
{
    if (verbose) puts("STUB: sstevx called");
    return NULL;
}

void* sstevx_(void)
{
    if (verbose) puts("STUB: sstevx_ called");
    return NULL;
}

void* ssycon(void)
{
    if (verbose) puts("STUB: ssycon called");
    return NULL;
}

void* ssycon_(void)
{
    if (verbose) puts("STUB: ssycon_ called");
    return NULL;
}

void* ssyequb(void)
{
    if (verbose) puts("STUB: ssyequb called");
    return NULL;
}

void* ssyequb_(void)
{
    if (verbose) puts("STUB: ssyequb_ called");
    return NULL;
}

void* ssyev(void)
{
    if (verbose) puts("STUB: ssyev called");
    return NULL;
}

void* ssyev_(void)
{
    if (verbose) puts("STUB: ssyev_ called");
    return NULL;
}

void* ssyevd(void)
{
    if (verbose) puts("STUB: ssyevd called");
    return NULL;
}

void* ssyevd_(void)
{
    if (verbose) puts("STUB: ssyevd_ called");
    return NULL;
}

void* ssyevr(void)
{
    if (verbose) puts("STUB: ssyevr called");
    return NULL;
}

void* ssyevr_(void)
{
    if (verbose) puts("STUB: ssyevr_ called");
    return NULL;
}

void* ssyevx(void)
{
    if (verbose) puts("STUB: ssyevx called");
    return NULL;
}

void* ssyevx_(void)
{
    if (verbose) puts("STUB: ssyevx_ called");
    return NULL;
}

void* ssygs2(void)
{
    if (verbose) puts("STUB: ssygs2 called");
    return NULL;
}

void* ssygs2_(void)
{
    if (verbose) puts("STUB: ssygs2_ called");
    return NULL;
}

void* ssygst(void)
{
    if (verbose) puts("STUB: ssygst called");
    return NULL;
}

void* ssygst_(void)
{
    if (verbose) puts("STUB: ssygst_ called");
    return NULL;
}

void* ssygv(void)
{
    if (verbose) puts("STUB: ssygv called");
    return NULL;
}

void* ssygv_(void)
{
    if (verbose) puts("STUB: ssygv_ called");
    return NULL;
}

void* ssygvd(void)
{
    if (verbose) puts("STUB: ssygvd called");
    return NULL;
}

void* ssygvd_(void)
{
    if (verbose) puts("STUB: ssygvd_ called");
    return NULL;
}

void* ssygvx(void)
{
    if (verbose) puts("STUB: ssygvx called");
    return NULL;
}

void* ssygvx_(void)
{
    if (verbose) puts("STUB: ssygvx_ called");
    return NULL;
}

void* ssyrfs(void)
{
    if (verbose) puts("STUB: ssyrfs called");
    return NULL;
}

void* ssyrfs_(void)
{
    if (verbose) puts("STUB: ssyrfs_ called");
    return NULL;
}

void* ssysv(void)
{
    if (verbose) puts("STUB: ssysv called");
    return NULL;
}

void* ssysv_(void)
{
    if (verbose) puts("STUB: ssysv_ called");
    return NULL;
}

void* ssysvx(void)
{
    if (verbose) puts("STUB: ssysvx called");
    return NULL;
}

void* ssysvx_(void)
{
    if (verbose) puts("STUB: ssysvx_ called");
    return NULL;
}

void* ssytd2(void)
{
    if (verbose) puts("STUB: ssytd2 called");
    return NULL;
}

void* ssytd2_(void)
{
    if (verbose) puts("STUB: ssytd2_ called");
    return NULL;
}

void* ssytf2(void)
{
    if (verbose) puts("STUB: ssytf2 called");
    return NULL;
}

void* ssytf2_(void)
{
    if (verbose) puts("STUB: ssytf2_ called");
    return NULL;
}

void* ssytrd(void)
{
    if (verbose) puts("STUB: ssytrd called");
    return NULL;
}

void* ssytrd_(void)
{
    if (verbose) puts("STUB: ssytrd_ called");
    return NULL;
}

void* ssytrf(void)
{
    if (verbose) puts("STUB: ssytrf called");
    return NULL;
}

void* ssytrf_(void)
{
    if (verbose) puts("STUB: ssytrf_ called");
    return NULL;
}

void* ssytri(void)
{
    if (verbose) puts("STUB: ssytri called");
    return NULL;
}

void* ssytri_(void)
{
    if (verbose) puts("STUB: ssytri_ called");
    return NULL;
}

void* ssytrs(void)
{
    if (verbose) puts("STUB: ssytrs called");
    return NULL;
}

void* ssytrs_(void)
{
    if (verbose) puts("STUB: ssytrs_ called");
    return NULL;
}

void* stbcon(void)
{
    if (verbose) puts("STUB: stbcon called");
    return NULL;
}

void* stbcon_(void)
{
    if (verbose) puts("STUB: stbcon_ called");
    return NULL;
}

void* stbrfs(void)
{
    if (verbose) puts("STUB: stbrfs called");
    return NULL;
}

void* stbrfs_(void)
{
    if (verbose) puts("STUB: stbrfs_ called");
    return NULL;
}

void* stbtrs(void)
{
    if (verbose) puts("STUB: stbtrs called");
    return NULL;
}

void* stbtrs_(void)
{
    if (verbose) puts("STUB: stbtrs_ called");
    return NULL;
}

void* stfsm(void)
{
    if (verbose) puts("STUB: stfsm called");
    return NULL;
}

void* stfsm_(void)
{
    if (verbose) puts("STUB: stfsm_ called");
    return NULL;
}

void* stftri(void)
{
    if (verbose) puts("STUB: stftri called");
    return NULL;
}

void* stftri_(void)
{
    if (verbose) puts("STUB: stftri_ called");
    return NULL;
}

void* stfttp(void)
{
    if (verbose) puts("STUB: stfttp called");
    return NULL;
}

void* stfttp_(void)
{
    if (verbose) puts("STUB: stfttp_ called");
    return NULL;
}

void* stfttr(void)
{
    if (verbose) puts("STUB: stfttr called");
    return NULL;
}

void* stfttr_(void)
{
    if (verbose) puts("STUB: stfttr_ called");
    return NULL;
}

void* stgevc(void)
{
    if (verbose) puts("STUB: stgevc called");
    return NULL;
}

void* stgevc_(void)
{
    if (verbose) puts("STUB: stgevc_ called");
    return NULL;
}

void* stgex2(void)
{
    if (verbose) puts("STUB: stgex2 called");
    return NULL;
}

void* stgex2_(void)
{
    if (verbose) puts("STUB: stgex2_ called");
    return NULL;
}

void* stgexc(void)
{
    if (verbose) puts("STUB: stgexc called");
    return NULL;
}

void* stgexc_(void)
{
    if (verbose) puts("STUB: stgexc_ called");
    return NULL;
}

void* stgsen(void)
{
    if (verbose) puts("STUB: stgsen called");
    return NULL;
}

void* stgsen_(void)
{
    if (verbose) puts("STUB: stgsen_ called");
    return NULL;
}

void* stgsja(void)
{
    if (verbose) puts("STUB: stgsja called");
    return NULL;
}

void* stgsja_(void)
{
    if (verbose) puts("STUB: stgsja_ called");
    return NULL;
}

void* stgsna(void)
{
    if (verbose) puts("STUB: stgsna called");
    return NULL;
}

void* stgsna_(void)
{
    if (verbose) puts("STUB: stgsna_ called");
    return NULL;
}

void* stgsy2(void)
{
    if (verbose) puts("STUB: stgsy2 called");
    return NULL;
}

void* stgsy2_(void)
{
    if (verbose) puts("STUB: stgsy2_ called");
    return NULL;
}

void* stgsyl(void)
{
    if (verbose) puts("STUB: stgsyl called");
    return NULL;
}

void* stgsyl_(void)
{
    if (verbose) puts("STUB: stgsyl_ called");
    return NULL;
}

void* stpcon(void)
{
    if (verbose) puts("STUB: stpcon called");
    return NULL;
}

void* stpcon_(void)
{
    if (verbose) puts("STUB: stpcon_ called");
    return NULL;
}

void* stprfs(void)
{
    if (verbose) puts("STUB: stprfs called");
    return NULL;
}

void* stprfs_(void)
{
    if (verbose) puts("STUB: stprfs_ called");
    return NULL;
}

void* stptri(void)
{
    if (verbose) puts("STUB: stptri called");
    return NULL;
}

void* stptri_(void)
{
    if (verbose) puts("STUB: stptri_ called");
    return NULL;
}

void* stptrs(void)
{
    if (verbose) puts("STUB: stptrs called");
    return NULL;
}

void* stptrs_(void)
{
    if (verbose) puts("STUB: stptrs_ called");
    return NULL;
}

void* stpttf(void)
{
    if (verbose) puts("STUB: stpttf called");
    return NULL;
}

void* stpttf_(void)
{
    if (verbose) puts("STUB: stpttf_ called");
    return NULL;
}

void* stpttr(void)
{
    if (verbose) puts("STUB: stpttr called");
    return NULL;
}

void* stpttr_(void)
{
    if (verbose) puts("STUB: stpttr_ called");
    return NULL;
}

void* strcon(void)
{
    if (verbose) puts("STUB: strcon called");
    return NULL;
}

void* strcon_(void)
{
    if (verbose) puts("STUB: strcon_ called");
    return NULL;
}

void* strevc(void)
{
    if (verbose) puts("STUB: strevc called");
    return NULL;
}

void* strevc_(void)
{
    if (verbose) puts("STUB: strevc_ called");
    return NULL;
}

void* strexc(void)
{
    if (verbose) puts("STUB: strexc called");
    return NULL;
}

void* strexc_(void)
{
    if (verbose) puts("STUB: strexc_ called");
    return NULL;
}

void* strrfs(void)
{
    if (verbose) puts("STUB: strrfs called");
    return NULL;
}

void* strrfs_(void)
{
    if (verbose) puts("STUB: strrfs_ called");
    return NULL;
}

void* strsen(void)
{
    if (verbose) puts("STUB: strsen called");
    return NULL;
}

void* strsen_(void)
{
    if (verbose) puts("STUB: strsen_ called");
    return NULL;
}

void* strsna(void)
{
    if (verbose) puts("STUB: strsna called");
    return NULL;
}

void* strsna_(void)
{
    if (verbose) puts("STUB: strsna_ called");
    return NULL;
}

void* strsyl(void)
{
    if (verbose) puts("STUB: strsyl called");
    return NULL;
}

void* strsyl_(void)
{
    if (verbose) puts("STUB: strsyl_ called");
    return NULL;
}

void* strti2(void)
{
    if (verbose) puts("STUB: strti2 called");
    return NULL;
}

void* strti2_(void)
{
    if (verbose) puts("STUB: strti2_ called");
    return NULL;
}

void* strtri(void)
{
    if (verbose) puts("STUB: strtri called");
    return NULL;
}

void* strtri_(void)
{
    if (verbose) puts("STUB: strtri_ called");
    return NULL;
}

void* strtrs(void)
{
    if (verbose) puts("STUB: strtrs called");
    return NULL;
}

void* strtrs_(void)
{
    if (verbose) puts("STUB: strtrs_ called");
    return NULL;
}

void* strttf(void)
{
    if (verbose) puts("STUB: strttf called");
    return NULL;
}

void* strttf_(void)
{
    if (verbose) puts("STUB: strttf_ called");
    return NULL;
}

void* strttp(void)
{
    if (verbose) puts("STUB: strttp called");
    return NULL;
}

void* strttp_(void)
{
    if (verbose) puts("STUB: strttp_ called");
    return NULL;
}

void* stzrqf(void)
{
    if (verbose) puts("STUB: stzrqf called");
    return NULL;
}

void* stzrqf_(void)
{
    if (verbose) puts("STUB: stzrqf_ called");
    return NULL;
}

void* stzrzf(void)
{
    if (verbose) puts("STUB: stzrzf called");
    return NULL;
}

void* stzrzf_(void)
{
    if (verbose) puts("STUB: stzrzf_ called");
    return NULL;
}

void* zbdsqr(void)
{
    if (verbose) puts("STUB: zbdsqr called");
    return NULL;
}

void* zbdsqr_(void)
{
    if (verbose) puts("STUB: zbdsqr_ called");
    return NULL;
}

void* zcgesv(void)
{
    if (verbose) puts("STUB: zcgesv called");
    return NULL;
}

void* zcgesv_(void)
{
    if (verbose) puts("STUB: zcgesv_ called");
    return NULL;
}

void* zcposv(void)
{
    if (verbose) puts("STUB: zcposv called");
    return NULL;
}

void* zcposv_(void)
{
    if (verbose) puts("STUB: zcposv_ called");
    return NULL;
}

void* zdrscl(void)
{
    if (verbose) puts("STUB: zdrscl called");
    return NULL;
}

void* zdrscl_(void)
{
    if (verbose) puts("STUB: zdrscl_ called");
    return NULL;
}

void* zgbbrd(void)
{
    if (verbose) puts("STUB: zgbbrd called");
    return NULL;
}

void* zgbbrd_(void)
{
    if (verbose) puts("STUB: zgbbrd_ called");
    return NULL;
}

void* zgbcon(void)
{
    if (verbose) puts("STUB: zgbcon called");
    return NULL;
}

void* zgbcon_(void)
{
    if (verbose) puts("STUB: zgbcon_ called");
    return NULL;
}

void* zgbequ(void)
{
    if (verbose) puts("STUB: zgbequ called");
    return NULL;
}

void* zgbequ_(void)
{
    if (verbose) puts("STUB: zgbequ_ called");
    return NULL;
}

void* zgbequb(void)
{
    if (verbose) puts("STUB: zgbequb called");
    return NULL;
}

void* zgbequb_(void)
{
    if (verbose) puts("STUB: zgbequb_ called");
    return NULL;
}

void* zgbrfs(void)
{
    if (verbose) puts("STUB: zgbrfs called");
    return NULL;
}

void* zgbrfs_(void)
{
    if (verbose) puts("STUB: zgbrfs_ called");
    return NULL;
}

void* zgbsv(void)
{
    if (verbose) puts("STUB: zgbsv called");
    return NULL;
}

void* zgbsv_(void)
{
    if (verbose) puts("STUB: zgbsv_ called");
    return NULL;
}

void* zgbsvx(void)
{
    if (verbose) puts("STUB: zgbsvx called");
    return NULL;
}

void* zgbsvx_(void)
{
    if (verbose) puts("STUB: zgbsvx_ called");
    return NULL;
}

void* zgbtf2(void)
{
    if (verbose) puts("STUB: zgbtf2 called");
    return NULL;
}

void* zgbtf2_(void)
{
    if (verbose) puts("STUB: zgbtf2_ called");
    return NULL;
}

void* zgbtrf(void)
{
    if (verbose) puts("STUB: zgbtrf called");
    return NULL;
}

void* zgbtrf_(void)
{
    if (verbose) puts("STUB: zgbtrf_ called");
    return NULL;
}

void* zgbtrs(void)
{
    if (verbose) puts("STUB: zgbtrs called");
    return NULL;
}

void* zgbtrs_(void)
{
    if (verbose) puts("STUB: zgbtrs_ called");
    return NULL;
}

void* zgebak(void)
{
    if (verbose) puts("STUB: zgebak called");
    return NULL;
}

void* zgebak_(void)
{
    if (verbose) puts("STUB: zgebak_ called");
    return NULL;
}

void* zgebal(void)
{
    if (verbose) puts("STUB: zgebal called");
    return NULL;
}

void* zgebal_(void)
{
    if (verbose) puts("STUB: zgebal_ called");
    return NULL;
}

void* zgebd2(void)
{
    if (verbose) puts("STUB: zgebd2 called");
    return NULL;
}

void* zgebd2_(void)
{
    if (verbose) puts("STUB: zgebd2_ called");
    return NULL;
}

void* zgebrd(void)
{
    if (verbose) puts("STUB: zgebrd called");
    return NULL;
}

void* zgebrd_(void)
{
    if (verbose) puts("STUB: zgebrd_ called");
    return NULL;
}

void* zgecon(void)
{
    if (verbose) puts("STUB: zgecon called");
    return NULL;
}

void* zgecon_(void)
{
    if (verbose) puts("STUB: zgecon_ called");
    return NULL;
}

void* zgeequ(void)
{
    if (verbose) puts("STUB: zgeequ called");
    return NULL;
}

void* zgeequ_(void)
{
    if (verbose) puts("STUB: zgeequ_ called");
    return NULL;
}

void* zgeequb(void)
{
    if (verbose) puts("STUB: zgeequb called");
    return NULL;
}

void* zgeequb_(void)
{
    if (verbose) puts("STUB: zgeequb_ called");
    return NULL;
}

void* zgees(void)
{
    if (verbose) puts("STUB: zgees called");
    return NULL;
}

void* zgees_(void)
{
    if (verbose) puts("STUB: zgees_ called");
    return NULL;
}

void* zgeesx(void)
{
    if (verbose) puts("STUB: zgeesx called");
    return NULL;
}

void* zgeesx_(void)
{
    if (verbose) puts("STUB: zgeesx_ called");
    return NULL;
}

void* zgeev(void)
{
    if (verbose) puts("STUB: zgeev called");
    return NULL;
}

void* zgeev_(void)
{
    if (verbose) puts("STUB: zgeev_ called");
    return NULL;
}

void* zgeevx(void)
{
    if (verbose) puts("STUB: zgeevx called");
    return NULL;
}

void* zgeevx_(void)
{
    if (verbose) puts("STUB: zgeevx_ called");
    return NULL;
}

void* zgegs(void)
{
    if (verbose) puts("STUB: zgegs called");
    return NULL;
}

void* zgegs_(void)
{
    if (verbose) puts("STUB: zgegs_ called");
    return NULL;
}

void* zgegv(void)
{
    if (verbose) puts("STUB: zgegv called");
    return NULL;
}

void* zgegv_(void)
{
    if (verbose) puts("STUB: zgegv_ called");
    return NULL;
}

void* zgehd2(void)
{
    if (verbose) puts("STUB: zgehd2 called");
    return NULL;
}

void* zgehd2_(void)
{
    if (verbose) puts("STUB: zgehd2_ called");
    return NULL;
}

void* zgehrd(void)
{
    if (verbose) puts("STUB: zgehrd called");
    return NULL;
}

void* zgehrd_(void)
{
    if (verbose) puts("STUB: zgehrd_ called");
    return NULL;
}

void* zgelq2(void)
{
    if (verbose) puts("STUB: zgelq2 called");
    return NULL;
}

void* zgelq2_(void)
{
    if (verbose) puts("STUB: zgelq2_ called");
    return NULL;
}

void* zgelqf(void)
{
    if (verbose) puts("STUB: zgelqf called");
    return NULL;
}

void* zgelqf_(void)
{
    if (verbose) puts("STUB: zgelqf_ called");
    return NULL;
}

void* zgels(void)
{
    if (verbose) puts("STUB: zgels called");
    return NULL;
}

void* zgels_(void)
{
    if (verbose) puts("STUB: zgels_ called");
    return NULL;
}

void* zgelsd(void)
{
    if (verbose) puts("STUB: zgelsd called");
    return NULL;
}

void* zgelsd_(void)
{
    if (verbose) puts("STUB: zgelsd_ called");
    return NULL;
}

void* zgelss(void)
{
    if (verbose) puts("STUB: zgelss called");
    return NULL;
}

void* zgelss_(void)
{
    if (verbose) puts("STUB: zgelss_ called");
    return NULL;
}

void* zgelsx(void)
{
    if (verbose) puts("STUB: zgelsx called");
    return NULL;
}

void* zgelsx_(void)
{
    if (verbose) puts("STUB: zgelsx_ called");
    return NULL;
}

void* zgelsy(void)
{
    if (verbose) puts("STUB: zgelsy called");
    return NULL;
}

void* zgelsy_(void)
{
    if (verbose) puts("STUB: zgelsy_ called");
    return NULL;
}

void* zgeql2(void)
{
    if (verbose) puts("STUB: zgeql2 called");
    return NULL;
}

void* zgeql2_(void)
{
    if (verbose) puts("STUB: zgeql2_ called");
    return NULL;
}

void* zgeqlf(void)
{
    if (verbose) puts("STUB: zgeqlf called");
    return NULL;
}

void* zgeqlf_(void)
{
    if (verbose) puts("STUB: zgeqlf_ called");
    return NULL;
}

void* zgeqp3(void)
{
    if (verbose) puts("STUB: zgeqp3 called");
    return NULL;
}

void* zgeqp3_(void)
{
    if (verbose) puts("STUB: zgeqp3_ called");
    return NULL;
}

void* zgeqpf(void)
{
    if (verbose) puts("STUB: zgeqpf called");
    return NULL;
}

void* zgeqpf_(void)
{
    if (verbose) puts("STUB: zgeqpf_ called");
    return NULL;
}

void* zgeqr2(void)
{
    if (verbose) puts("STUB: zgeqr2 called");
    return NULL;
}

void* zgeqr2_(void)
{
    if (verbose) puts("STUB: zgeqr2_ called");
    return NULL;
}

void* zgeqrf(void)
{
    if (verbose) puts("STUB: zgeqrf called");
    return NULL;
}

void* zgeqrf_(void)
{
    if (verbose) puts("STUB: zgeqrf_ called");
    return NULL;
}

void* zgerfs(void)
{
    if (verbose) puts("STUB: zgerfs called");
    return NULL;
}

void* zgerfs_(void)
{
    if (verbose) puts("STUB: zgerfs_ called");
    return NULL;
}

void* zgerq2(void)
{
    if (verbose) puts("STUB: zgerq2 called");
    return NULL;
}

void* zgerq2_(void)
{
    if (verbose) puts("STUB: zgerq2_ called");
    return NULL;
}

void* zgerqf(void)
{
    if (verbose) puts("STUB: zgerqf called");
    return NULL;
}

void* zgerqf_(void)
{
    if (verbose) puts("STUB: zgerqf_ called");
    return NULL;
}

void* zgesc2(void)
{
    if (verbose) puts("STUB: zgesc2 called");
    return NULL;
}

void* zgesc2_(void)
{
    if (verbose) puts("STUB: zgesc2_ called");
    return NULL;
}

void* zgesdd(void)
{
    if (verbose) puts("STUB: zgesdd called");
    return NULL;
}

void* zgesdd_(void)
{
    if (verbose) puts("STUB: zgesdd_ called");
    return NULL;
}

void* zgesv(void)
{
    if (verbose) puts("STUB: zgesv called");
    return NULL;
}

void* zgesv_(void)
{
    if (verbose) puts("STUB: zgesv_ called");
    return NULL;
}

void* zgesvd(void)
{
    if (verbose) puts("STUB: zgesvd called");
    return NULL;
}

void* zgesvd_(void)
{
    if (verbose) puts("STUB: zgesvd_ called");
    return NULL;
}

void* zgesvx(void)
{
    if (verbose) puts("STUB: zgesvx called");
    return NULL;
}

void* zgesvx_(void)
{
    if (verbose) puts("STUB: zgesvx_ called");
    return NULL;
}

void* zgetc2(void)
{
    if (verbose) puts("STUB: zgetc2 called");
    return NULL;
}

void* zgetc2_(void)
{
    if (verbose) puts("STUB: zgetc2_ called");
    return NULL;
}

void* zgetf2(void)
{
    if (verbose) puts("STUB: zgetf2 called");
    return NULL;
}

void* zgetf2_(void)
{
    if (verbose) puts("STUB: zgetf2_ called");
    return NULL;
}

void* zgetrf(void)
{
    if (verbose) puts("STUB: zgetrf called");
    return NULL;
}

void* zgetrf_(void)
{
    if (verbose) puts("STUB: zgetrf_ called");
    return NULL;
}

void* zgetri(void)
{
    if (verbose) puts("STUB: zgetri called");
    return NULL;
}

void* zgetri_(void)
{
    if (verbose) puts("STUB: zgetri_ called");
    return NULL;
}

void* zgetrs(void)
{
    if (verbose) puts("STUB: zgetrs called");
    return NULL;
}

void* zgetrs_(void)
{
    if (verbose) puts("STUB: zgetrs_ called");
    return NULL;
}

void* zggbak(void)
{
    if (verbose) puts("STUB: zggbak called");
    return NULL;
}

void* zggbak_(void)
{
    if (verbose) puts("STUB: zggbak_ called");
    return NULL;
}

void* zggbal(void)
{
    if (verbose) puts("STUB: zggbal called");
    return NULL;
}

void* zggbal_(void)
{
    if (verbose) puts("STUB: zggbal_ called");
    return NULL;
}

void* zgges(void)
{
    if (verbose) puts("STUB: zgges called");
    return NULL;
}

void* zgges_(void)
{
    if (verbose) puts("STUB: zgges_ called");
    return NULL;
}

void* zggesx(void)
{
    if (verbose) puts("STUB: zggesx called");
    return NULL;
}

void* zggesx_(void)
{
    if (verbose) puts("STUB: zggesx_ called");
    return NULL;
}

void* zggev(void)
{
    if (verbose) puts("STUB: zggev called");
    return NULL;
}

void* zggev_(void)
{
    if (verbose) puts("STUB: zggev_ called");
    return NULL;
}

void* zggevx(void)
{
    if (verbose) puts("STUB: zggevx called");
    return NULL;
}

void* zggevx_(void)
{
    if (verbose) puts("STUB: zggevx_ called");
    return NULL;
}

void* zggglm(void)
{
    if (verbose) puts("STUB: zggglm called");
    return NULL;
}

void* zggglm_(void)
{
    if (verbose) puts("STUB: zggglm_ called");
    return NULL;
}

void* zgghrd(void)
{
    if (verbose) puts("STUB: zgghrd called");
    return NULL;
}

void* zgghrd_(void)
{
    if (verbose) puts("STUB: zgghrd_ called");
    return NULL;
}

void* zgglse(void)
{
    if (verbose) puts("STUB: zgglse called");
    return NULL;
}

void* zgglse_(void)
{
    if (verbose) puts("STUB: zgglse_ called");
    return NULL;
}

void* zggqrf(void)
{
    if (verbose) puts("STUB: zggqrf called");
    return NULL;
}

void* zggqrf_(void)
{
    if (verbose) puts("STUB: zggqrf_ called");
    return NULL;
}

void* zggrqf(void)
{
    if (verbose) puts("STUB: zggrqf called");
    return NULL;
}

void* zggrqf_(void)
{
    if (verbose) puts("STUB: zggrqf_ called");
    return NULL;
}

void* zggsvd(void)
{
    if (verbose) puts("STUB: zggsvd called");
    return NULL;
}

void* zggsvd_(void)
{
    if (verbose) puts("STUB: zggsvd_ called");
    return NULL;
}

void* zggsvp(void)
{
    if (verbose) puts("STUB: zggsvp called");
    return NULL;
}

void* zggsvp_(void)
{
    if (verbose) puts("STUB: zggsvp_ called");
    return NULL;
}

void* zgtcon(void)
{
    if (verbose) puts("STUB: zgtcon called");
    return NULL;
}

void* zgtcon_(void)
{
    if (verbose) puts("STUB: zgtcon_ called");
    return NULL;
}

void* zgtrfs(void)
{
    if (verbose) puts("STUB: zgtrfs called");
    return NULL;
}

void* zgtrfs_(void)
{
    if (verbose) puts("STUB: zgtrfs_ called");
    return NULL;
}

void* zgtsv(void)
{
    if (verbose) puts("STUB: zgtsv called");
    return NULL;
}

void* zgtsv_(void)
{
    if (verbose) puts("STUB: zgtsv_ called");
    return NULL;
}

void* zgtsvx(void)
{
    if (verbose) puts("STUB: zgtsvx called");
    return NULL;
}

void* zgtsvx_(void)
{
    if (verbose) puts("STUB: zgtsvx_ called");
    return NULL;
}

void* zgttrf(void)
{
    if (verbose) puts("STUB: zgttrf called");
    return NULL;
}

void* zgttrf_(void)
{
    if (verbose) puts("STUB: zgttrf_ called");
    return NULL;
}

void* zgttrs(void)
{
    if (verbose) puts("STUB: zgttrs called");
    return NULL;
}

void* zgttrs_(void)
{
    if (verbose) puts("STUB: zgttrs_ called");
    return NULL;
}

void* zgtts2(void)
{
    if (verbose) puts("STUB: zgtts2 called");
    return NULL;
}

void* zgtts2_(void)
{
    if (verbose) puts("STUB: zgtts2_ called");
    return NULL;
}

void* zhbev(void)
{
    if (verbose) puts("STUB: zhbev called");
    return NULL;
}

void* zhbev_(void)
{
    if (verbose) puts("STUB: zhbev_ called");
    return NULL;
}

void* zhbevd(void)
{
    if (verbose) puts("STUB: zhbevd called");
    return NULL;
}

void* zhbevd_(void)
{
    if (verbose) puts("STUB: zhbevd_ called");
    return NULL;
}

void* zhbevx(void)
{
    if (verbose) puts("STUB: zhbevx called");
    return NULL;
}

void* zhbevx_(void)
{
    if (verbose) puts("STUB: zhbevx_ called");
    return NULL;
}

void* zhbgst(void)
{
    if (verbose) puts("STUB: zhbgst called");
    return NULL;
}

void* zhbgst_(void)
{
    if (verbose) puts("STUB: zhbgst_ called");
    return NULL;
}

void* zhbgv(void)
{
    if (verbose) puts("STUB: zhbgv called");
    return NULL;
}

void* zhbgv_(void)
{
    if (verbose) puts("STUB: zhbgv_ called");
    return NULL;
}

void* zhbgvd(void)
{
    if (verbose) puts("STUB: zhbgvd called");
    return NULL;
}

void* zhbgvd_(void)
{
    if (verbose) puts("STUB: zhbgvd_ called");
    return NULL;
}

void* zhbgvx(void)
{
    if (verbose) puts("STUB: zhbgvx called");
    return NULL;
}

void* zhbgvx_(void)
{
    if (verbose) puts("STUB: zhbgvx_ called");
    return NULL;
}

void* zhbtrd(void)
{
    if (verbose) puts("STUB: zhbtrd called");
    return NULL;
}

void* zhbtrd_(void)
{
    if (verbose) puts("STUB: zhbtrd_ called");
    return NULL;
}

void* zhecon(void)
{
    if (verbose) puts("STUB: zhecon called");
    return NULL;
}

void* zhecon_(void)
{
    if (verbose) puts("STUB: zhecon_ called");
    return NULL;
}

void* zheequb(void)
{
    if (verbose) puts("STUB: zheequb called");
    return NULL;
}

void* zheequb_(void)
{
    if (verbose) puts("STUB: zheequb_ called");
    return NULL;
}

void* zheev(void)
{
    if (verbose) puts("STUB: zheev called");
    return NULL;
}

void* zheev_(void)
{
    if (verbose) puts("STUB: zheev_ called");
    return NULL;
}

void* zheevd(void)
{
    if (verbose) puts("STUB: zheevd called");
    return NULL;
}

void* zheevd_(void)
{
    if (verbose) puts("STUB: zheevd_ called");
    return NULL;
}

void* zheevr(void)
{
    if (verbose) puts("STUB: zheevr called");
    return NULL;
}

void* zheevr_(void)
{
    if (verbose) puts("STUB: zheevr_ called");
    return NULL;
}

void* zheevx(void)
{
    if (verbose) puts("STUB: zheevx called");
    return NULL;
}

void* zheevx_(void)
{
    if (verbose) puts("STUB: zheevx_ called");
    return NULL;
}

void* zhegs2(void)
{
    if (verbose) puts("STUB: zhegs2 called");
    return NULL;
}

void* zhegs2_(void)
{
    if (verbose) puts("STUB: zhegs2_ called");
    return NULL;
}

void* zhegst(void)
{
    if (verbose) puts("STUB: zhegst called");
    return NULL;
}

void* zhegst_(void)
{
    if (verbose) puts("STUB: zhegst_ called");
    return NULL;
}

void* zhegv(void)
{
    if (verbose) puts("STUB: zhegv called");
    return NULL;
}

void* zhegv_(void)
{
    if (verbose) puts("STUB: zhegv_ called");
    return NULL;
}

void* zhegvd(void)
{
    if (verbose) puts("STUB: zhegvd called");
    return NULL;
}

void* zhegvd_(void)
{
    if (verbose) puts("STUB: zhegvd_ called");
    return NULL;
}

void* zhegvx(void)
{
    if (verbose) puts("STUB: zhegvx called");
    return NULL;
}

void* zhegvx_(void)
{
    if (verbose) puts("STUB: zhegvx_ called");
    return NULL;
}

void* zherfs(void)
{
    if (verbose) puts("STUB: zherfs called");
    return NULL;
}

void* zherfs_(void)
{
    if (verbose) puts("STUB: zherfs_ called");
    return NULL;
}

void* zhesv(void)
{
    if (verbose) puts("STUB: zhesv called");
    return NULL;
}

void* zhesv_(void)
{
    if (verbose) puts("STUB: zhesv_ called");
    return NULL;
}

void* zhesvx(void)
{
    if (verbose) puts("STUB: zhesvx called");
    return NULL;
}

void* zhesvx_(void)
{
    if (verbose) puts("STUB: zhesvx_ called");
    return NULL;
}

void* zhetd2(void)
{
    if (verbose) puts("STUB: zhetd2 called");
    return NULL;
}

void* zhetd2_(void)
{
    if (verbose) puts("STUB: zhetd2_ called");
    return NULL;
}

void* zhetf2(void)
{
    if (verbose) puts("STUB: zhetf2 called");
    return NULL;
}

void* zhetf2_(void)
{
    if (verbose) puts("STUB: zhetf2_ called");
    return NULL;
}

void* zhetrd(void)
{
    if (verbose) puts("STUB: zhetrd called");
    return NULL;
}

void* zhetrd_(void)
{
    if (verbose) puts("STUB: zhetrd_ called");
    return NULL;
}

void* zhetrf(void)
{
    if (verbose) puts("STUB: zhetrf called");
    return NULL;
}

void* zhetrf_(void)
{
    if (verbose) puts("STUB: zhetrf_ called");
    return NULL;
}

void* zhetri(void)
{
    if (verbose) puts("STUB: zhetri called");
    return NULL;
}

void* zhetri_(void)
{
    if (verbose) puts("STUB: zhetri_ called");
    return NULL;
}

void* zhetrs(void)
{
    if (verbose) puts("STUB: zhetrs called");
    return NULL;
}

void* zhetrs_(void)
{
    if (verbose) puts("STUB: zhetrs_ called");
    return NULL;
}

void* zhfrk(void)
{
    if (verbose) puts("STUB: zhfrk called");
    return NULL;
}

void* zhfrk_(void)
{
    if (verbose) puts("STUB: zhfrk_ called");
    return NULL;
}

void* zhgeqz(void)
{
    if (verbose) puts("STUB: zhgeqz called");
    return NULL;
}

void* zhgeqz_(void)
{
    if (verbose) puts("STUB: zhgeqz_ called");
    return NULL;
}

void* zhpcon(void)
{
    if (verbose) puts("STUB: zhpcon called");
    return NULL;
}

void* zhpcon_(void)
{
    if (verbose) puts("STUB: zhpcon_ called");
    return NULL;
}

void* zhpev(void)
{
    if (verbose) puts("STUB: zhpev called");
    return NULL;
}

void* zhpev_(void)
{
    if (verbose) puts("STUB: zhpev_ called");
    return NULL;
}

void* zhpevd(void)
{
    if (verbose) puts("STUB: zhpevd called");
    return NULL;
}

void* zhpevd_(void)
{
    if (verbose) puts("STUB: zhpevd_ called");
    return NULL;
}

void* zhpevx(void)
{
    if (verbose) puts("STUB: zhpevx called");
    return NULL;
}

void* zhpevx_(void)
{
    if (verbose) puts("STUB: zhpevx_ called");
    return NULL;
}

void* zhpgst(void)
{
    if (verbose) puts("STUB: zhpgst called");
    return NULL;
}

void* zhpgst_(void)
{
    if (verbose) puts("STUB: zhpgst_ called");
    return NULL;
}

void* zhpgv(void)
{
    if (verbose) puts("STUB: zhpgv called");
    return NULL;
}

void* zhpgv_(void)
{
    if (verbose) puts("STUB: zhpgv_ called");
    return NULL;
}

void* zhpgvd(void)
{
    if (verbose) puts("STUB: zhpgvd called");
    return NULL;
}

void* zhpgvd_(void)
{
    if (verbose) puts("STUB: zhpgvd_ called");
    return NULL;
}

void* zhpgvx(void)
{
    if (verbose) puts("STUB: zhpgvx called");
    return NULL;
}

void* zhpgvx_(void)
{
    if (verbose) puts("STUB: zhpgvx_ called");
    return NULL;
}

void* zhprfs(void)
{
    if (verbose) puts("STUB: zhprfs called");
    return NULL;
}

void* zhprfs_(void)
{
    if (verbose) puts("STUB: zhprfs_ called");
    return NULL;
}

void* zhpsv(void)
{
    if (verbose) puts("STUB: zhpsv called");
    return NULL;
}

void* zhpsv_(void)
{
    if (verbose) puts("STUB: zhpsv_ called");
    return NULL;
}

void* zhpsvx(void)
{
    if (verbose) puts("STUB: zhpsvx called");
    return NULL;
}

void* zhpsvx_(void)
{
    if (verbose) puts("STUB: zhpsvx_ called");
    return NULL;
}

void* zhptrd(void)
{
    if (verbose) puts("STUB: zhptrd called");
    return NULL;
}

void* zhptrd_(void)
{
    if (verbose) puts("STUB: zhptrd_ called");
    return NULL;
}

void* zhptrf(void)
{
    if (verbose) puts("STUB: zhptrf called");
    return NULL;
}

void* zhptrf_(void)
{
    if (verbose) puts("STUB: zhptrf_ called");
    return NULL;
}

void* zhptri(void)
{
    if (verbose) puts("STUB: zhptri called");
    return NULL;
}

void* zhptri_(void)
{
    if (verbose) puts("STUB: zhptri_ called");
    return NULL;
}

void* zhptrs(void)
{
    if (verbose) puts("STUB: zhptrs called");
    return NULL;
}

void* zhptrs_(void)
{
    if (verbose) puts("STUB: zhptrs_ called");
    return NULL;
}

void* zhsein(void)
{
    if (verbose) puts("STUB: zhsein called");
    return NULL;
}

void* zhsein_(void)
{
    if (verbose) puts("STUB: zhsein_ called");
    return NULL;
}

void* zhseqr(void)
{
    if (verbose) puts("STUB: zhseqr called");
    return NULL;
}

void* zhseqr_(void)
{
    if (verbose) puts("STUB: zhseqr_ called");
    return NULL;
}

void* zlabrd(void)
{
    if (verbose) puts("STUB: zlabrd called");
    return NULL;
}

void* zlabrd_(void)
{
    if (verbose) puts("STUB: zlabrd_ called");
    return NULL;
}

void* zlacgv(void)
{
    if (verbose) puts("STUB: zlacgv called");
    return NULL;
}

void* zlacgv_(void)
{
    if (verbose) puts("STUB: zlacgv_ called");
    return NULL;
}

void* zlacn2(void)
{
    if (verbose) puts("STUB: zlacn2 called");
    return NULL;
}

void* zlacn2_(void)
{
    if (verbose) puts("STUB: zlacn2_ called");
    return NULL;
}

void* zlacon(void)
{
    if (verbose) puts("STUB: zlacon called");
    return NULL;
}

void* zlacon_(void)
{
    if (verbose) puts("STUB: zlacon_ called");
    return NULL;
}

void* zlacp2(void)
{
    if (verbose) puts("STUB: zlacp2 called");
    return NULL;
}

void* zlacp2_(void)
{
    if (verbose) puts("STUB: zlacp2_ called");
    return NULL;
}

void* zlacpy(void)
{
    if (verbose) puts("STUB: zlacpy called");
    return NULL;
}

void* zlacpy_(void)
{
    if (verbose) puts("STUB: zlacpy_ called");
    return NULL;
}

void* zlacrm(void)
{
    if (verbose) puts("STUB: zlacrm called");
    return NULL;
}

void* zlacrm_(void)
{
    if (verbose) puts("STUB: zlacrm_ called");
    return NULL;
}

void* zlacrt(void)
{
    if (verbose) puts("STUB: zlacrt called");
    return NULL;
}

void* zlacrt_(void)
{
    if (verbose) puts("STUB: zlacrt_ called");
    return NULL;
}

void* zladiv(void)
{
    if (verbose) puts("STUB: zladiv called");
    return NULL;
}

void* zladiv_(void)
{
    if (verbose) puts("STUB: zladiv_ called");
    return NULL;
}

void* zlaed0(void)
{
    if (verbose) puts("STUB: zlaed0 called");
    return NULL;
}

void* zlaed0_(void)
{
    if (verbose) puts("STUB: zlaed0_ called");
    return NULL;
}

void* zlaed7(void)
{
    if (verbose) puts("STUB: zlaed7 called");
    return NULL;
}

void* zlaed7_(void)
{
    if (verbose) puts("STUB: zlaed7_ called");
    return NULL;
}

void* zlaed8(void)
{
    if (verbose) puts("STUB: zlaed8 called");
    return NULL;
}

void* zlaed8_(void)
{
    if (verbose) puts("STUB: zlaed8_ called");
    return NULL;
}

void* zlaein(void)
{
    if (verbose) puts("STUB: zlaein called");
    return NULL;
}

void* zlaein_(void)
{
    if (verbose) puts("STUB: zlaein_ called");
    return NULL;
}

void* zlaesy(void)
{
    if (verbose) puts("STUB: zlaesy called");
    return NULL;
}

void* zlaesy_(void)
{
    if (verbose) puts("STUB: zlaesy_ called");
    return NULL;
}

void* zlaev2(void)
{
    if (verbose) puts("STUB: zlaev2 called");
    return NULL;
}

void* zlaev2_(void)
{
    if (verbose) puts("STUB: zlaev2_ called");
    return NULL;
}

void* zlag2c(void)
{
    if (verbose) puts("STUB: zlag2c called");
    return NULL;
}

void* zlag2c_(void)
{
    if (verbose) puts("STUB: zlag2c_ called");
    return NULL;
}

void* zlags2(void)
{
    if (verbose) puts("STUB: zlags2 called");
    return NULL;
}

void* zlags2_(void)
{
    if (verbose) puts("STUB: zlags2_ called");
    return NULL;
}

void* zlagtm(void)
{
    if (verbose) puts("STUB: zlagtm called");
    return NULL;
}

void* zlagtm_(void)
{
    if (verbose) puts("STUB: zlagtm_ called");
    return NULL;
}

void* zlahef(void)
{
    if (verbose) puts("STUB: zlahef called");
    return NULL;
}

void* zlahef_(void)
{
    if (verbose) puts("STUB: zlahef_ called");
    return NULL;
}

void* zlahqr(void)
{
    if (verbose) puts("STUB: zlahqr called");
    return NULL;
}

void* zlahqr_(void)
{
    if (verbose) puts("STUB: zlahqr_ called");
    return NULL;
}

void* zlahr2(void)
{
    if (verbose) puts("STUB: zlahr2 called");
    return NULL;
}

void* zlahr2_(void)
{
    if (verbose) puts("STUB: zlahr2_ called");
    return NULL;
}

void* zlahrd(void)
{
    if (verbose) puts("STUB: zlahrd called");
    return NULL;
}

void* zlahrd_(void)
{
    if (verbose) puts("STUB: zlahrd_ called");
    return NULL;
}

void* zlaic1(void)
{
    if (verbose) puts("STUB: zlaic1 called");
    return NULL;
}

void* zlaic1_(void)
{
    if (verbose) puts("STUB: zlaic1_ called");
    return NULL;
}

void* zlals0(void)
{
    if (verbose) puts("STUB: zlals0 called");
    return NULL;
}

void* zlals0_(void)
{
    if (verbose) puts("STUB: zlals0_ called");
    return NULL;
}

void* zlalsa(void)
{
    if (verbose) puts("STUB: zlalsa called");
    return NULL;
}

void* zlalsa_(void)
{
    if (verbose) puts("STUB: zlalsa_ called");
    return NULL;
}

void* zlalsd(void)
{
    if (verbose) puts("STUB: zlalsd called");
    return NULL;
}

void* zlalsd_(void)
{
    if (verbose) puts("STUB: zlalsd_ called");
    return NULL;
}

void* zlangb(void)
{
    if (verbose) puts("STUB: zlangb called");
    return NULL;
}

void* zlangb_(void)
{
    if (verbose) puts("STUB: zlangb_ called");
    return NULL;
}

void* zlange(void)
{
    if (verbose) puts("STUB: zlange called");
    return NULL;
}

void* zlange_(void)
{
    if (verbose) puts("STUB: zlange_ called");
    return NULL;
}

void* zlangt(void)
{
    if (verbose) puts("STUB: zlangt called");
    return NULL;
}

void* zlangt_(void)
{
    if (verbose) puts("STUB: zlangt_ called");
    return NULL;
}

void* zlanhb(void)
{
    if (verbose) puts("STUB: zlanhb called");
    return NULL;
}

void* zlanhb_(void)
{
    if (verbose) puts("STUB: zlanhb_ called");
    return NULL;
}

void* zlanhe(void)
{
    if (verbose) puts("STUB: zlanhe called");
    return NULL;
}

void* zlanhe_(void)
{
    if (verbose) puts("STUB: zlanhe_ called");
    return NULL;
}

void* zlanhf(void)
{
    if (verbose) puts("STUB: zlanhf called");
    return NULL;
}

void* zlanhf_(void)
{
    if (verbose) puts("STUB: zlanhf_ called");
    return NULL;
}

void* zlanhp(void)
{
    if (verbose) puts("STUB: zlanhp called");
    return NULL;
}

void* zlanhp_(void)
{
    if (verbose) puts("STUB: zlanhp_ called");
    return NULL;
}

void* zlanhs(void)
{
    if (verbose) puts("STUB: zlanhs called");
    return NULL;
}

void* zlanhs_(void)
{
    if (verbose) puts("STUB: zlanhs_ called");
    return NULL;
}

void* zlanht(void)
{
    if (verbose) puts("STUB: zlanht called");
    return NULL;
}

void* zlanht_(void)
{
    if (verbose) puts("STUB: zlanht_ called");
    return NULL;
}

void* zlansb(void)
{
    if (verbose) puts("STUB: zlansb called");
    return NULL;
}

void* zlansb_(void)
{
    if (verbose) puts("STUB: zlansb_ called");
    return NULL;
}

void* zlansp(void)
{
    if (verbose) puts("STUB: zlansp called");
    return NULL;
}

void* zlansp_(void)
{
    if (verbose) puts("STUB: zlansp_ called");
    return NULL;
}

void* zlansy(void)
{
    if (verbose) puts("STUB: zlansy called");
    return NULL;
}

void* zlansy_(void)
{
    if (verbose) puts("STUB: zlansy_ called");
    return NULL;
}

void* zlantb(void)
{
    if (verbose) puts("STUB: zlantb called");
    return NULL;
}

void* zlantb_(void)
{
    if (verbose) puts("STUB: zlantb_ called");
    return NULL;
}

void* zlantp(void)
{
    if (verbose) puts("STUB: zlantp called");
    return NULL;
}

void* zlantp_(void)
{
    if (verbose) puts("STUB: zlantp_ called");
    return NULL;
}

void* zlantr(void)
{
    if (verbose) puts("STUB: zlantr called");
    return NULL;
}

void* zlantr_(void)
{
    if (verbose) puts("STUB: zlantr_ called");
    return NULL;
}

void* zlapll(void)
{
    if (verbose) puts("STUB: zlapll called");
    return NULL;
}

void* zlapll_(void)
{
    if (verbose) puts("STUB: zlapll_ called");
    return NULL;
}

void* zlapmt(void)
{
    if (verbose) puts("STUB: zlapmt called");
    return NULL;
}

void* zlapmt_(void)
{
    if (verbose) puts("STUB: zlapmt_ called");
    return NULL;
}

void* zlaqgb(void)
{
    if (verbose) puts("STUB: zlaqgb called");
    return NULL;
}

void* zlaqgb_(void)
{
    if (verbose) puts("STUB: zlaqgb_ called");
    return NULL;
}

void* zlaqge(void)
{
    if (verbose) puts("STUB: zlaqge called");
    return NULL;
}

void* zlaqge_(void)
{
    if (verbose) puts("STUB: zlaqge_ called");
    return NULL;
}

void* zlaqhb(void)
{
    if (verbose) puts("STUB: zlaqhb called");
    return NULL;
}

void* zlaqhb_(void)
{
    if (verbose) puts("STUB: zlaqhb_ called");
    return NULL;
}

void* zlaqhe(void)
{
    if (verbose) puts("STUB: zlaqhe called");
    return NULL;
}

void* zlaqhe_(void)
{
    if (verbose) puts("STUB: zlaqhe_ called");
    return NULL;
}

void* zlaqhp(void)
{
    if (verbose) puts("STUB: zlaqhp called");
    return NULL;
}

void* zlaqhp_(void)
{
    if (verbose) puts("STUB: zlaqhp_ called");
    return NULL;
}

void* zlaqp2(void)
{
    if (verbose) puts("STUB: zlaqp2 called");
    return NULL;
}

void* zlaqp2_(void)
{
    if (verbose) puts("STUB: zlaqp2_ called");
    return NULL;
}

void* zlaqps(void)
{
    if (verbose) puts("STUB: zlaqps called");
    return NULL;
}

void* zlaqps_(void)
{
    if (verbose) puts("STUB: zlaqps_ called");
    return NULL;
}

void* zlaqr0(void)
{
    if (verbose) puts("STUB: zlaqr0 called");
    return NULL;
}

void* zlaqr0_(void)
{
    if (verbose) puts("STUB: zlaqr0_ called");
    return NULL;
}

void* zlaqr1(void)
{
    if (verbose) puts("STUB: zlaqr1 called");
    return NULL;
}

void* zlaqr1_(void)
{
    if (verbose) puts("STUB: zlaqr1_ called");
    return NULL;
}

void* zlaqr2(void)
{
    if (verbose) puts("STUB: zlaqr2 called");
    return NULL;
}

void* zlaqr2_(void)
{
    if (verbose) puts("STUB: zlaqr2_ called");
    return NULL;
}

void* zlaqr3(void)
{
    if (verbose) puts("STUB: zlaqr3 called");
    return NULL;
}

void* zlaqr3_(void)
{
    if (verbose) puts("STUB: zlaqr3_ called");
    return NULL;
}

void* zlaqr4(void)
{
    if (verbose) puts("STUB: zlaqr4 called");
    return NULL;
}

void* zlaqr4_(void)
{
    if (verbose) puts("STUB: zlaqr4_ called");
    return NULL;
}

void* zlaqr5(void)
{
    if (verbose) puts("STUB: zlaqr5 called");
    return NULL;
}

void* zlaqr5_(void)
{
    if (verbose) puts("STUB: zlaqr5_ called");
    return NULL;
}

void* zlaqsb(void)
{
    if (verbose) puts("STUB: zlaqsb called");
    return NULL;
}

void* zlaqsb_(void)
{
    if (verbose) puts("STUB: zlaqsb_ called");
    return NULL;
}

void* zlaqsp(void)
{
    if (verbose) puts("STUB: zlaqsp called");
    return NULL;
}

void* zlaqsp_(void)
{
    if (verbose) puts("STUB: zlaqsp_ called");
    return NULL;
}

void* zlaqsy(void)
{
    if (verbose) puts("STUB: zlaqsy called");
    return NULL;
}

void* zlaqsy_(void)
{
    if (verbose) puts("STUB: zlaqsy_ called");
    return NULL;
}

void* zlar1v(void)
{
    if (verbose) puts("STUB: zlar1v called");
    return NULL;
}

void* zlar1v_(void)
{
    if (verbose) puts("STUB: zlar1v_ called");
    return NULL;
}

void* zlar2v(void)
{
    if (verbose) puts("STUB: zlar2v called");
    return NULL;
}

void* zlar2v_(void)
{
    if (verbose) puts("STUB: zlar2v_ called");
    return NULL;
}

void* zlarcm(void)
{
    if (verbose) puts("STUB: zlarcm called");
    return NULL;
}

void* zlarcm_(void)
{
    if (verbose) puts("STUB: zlarcm_ called");
    return NULL;
}

void* zlarf(void)
{
    if (verbose) puts("STUB: zlarf called");
    return NULL;
}

void* zlarf_(void)
{
    if (verbose) puts("STUB: zlarf_ called");
    return NULL;
}

void* zlarfb(void)
{
    if (verbose) puts("STUB: zlarfb called");
    return NULL;
}

void* zlarfb_(void)
{
    if (verbose) puts("STUB: zlarfb_ called");
    return NULL;
}

void* zlarfg(void)
{
    if (verbose) puts("STUB: zlarfg called");
    return NULL;
}

void* zlarfg_(void)
{
    if (verbose) puts("STUB: zlarfg_ called");
    return NULL;
}

void* zlarfp(void)
{
    if (verbose) puts("STUB: zlarfp called");
    return NULL;
}

void* zlarfp_(void)
{
    if (verbose) puts("STUB: zlarfp_ called");
    return NULL;
}

void* zlarft(void)
{
    if (verbose) puts("STUB: zlarft called");
    return NULL;
}

void* zlarft_(void)
{
    if (verbose) puts("STUB: zlarft_ called");
    return NULL;
}

void* zlarfx(void)
{
    if (verbose) puts("STUB: zlarfx called");
    return NULL;
}

void* zlarfx_(void)
{
    if (verbose) puts("STUB: zlarfx_ called");
    return NULL;
}

void* zlargv(void)
{
    if (verbose) puts("STUB: zlargv called");
    return NULL;
}

void* zlargv_(void)
{
    if (verbose) puts("STUB: zlargv_ called");
    return NULL;
}

void* zlarnv(void)
{
    if (verbose) puts("STUB: zlarnv called");
    return NULL;
}

void* zlarnv_(void)
{
    if (verbose) puts("STUB: zlarnv_ called");
    return NULL;
}

void* zlarrv(void)
{
    if (verbose) puts("STUB: zlarrv called");
    return NULL;
}

void* zlarrv_(void)
{
    if (verbose) puts("STUB: zlarrv_ called");
    return NULL;
}

void* zlarscl2(void)
{
    if (verbose) puts("STUB: zlarscl2 called");
    return NULL;
}

void* zlarscl2_(void)
{
    if (verbose) puts("STUB: zlarscl2_ called");
    return NULL;
}

void* zlartg(void)
{
    if (verbose) puts("STUB: zlartg called");
    return NULL;
}

void* zlartg_(void)
{
    if (verbose) puts("STUB: zlartg_ called");
    return NULL;
}

void* zlartv(void)
{
    if (verbose) puts("STUB: zlartv called");
    return NULL;
}

void* zlartv_(void)
{
    if (verbose) puts("STUB: zlartv_ called");
    return NULL;
}

void* zlarz(void)
{
    if (verbose) puts("STUB: zlarz called");
    return NULL;
}

void* zlarz_(void)
{
    if (verbose) puts("STUB: zlarz_ called");
    return NULL;
}

void* zlarzb(void)
{
    if (verbose) puts("STUB: zlarzb called");
    return NULL;
}

void* zlarzb_(void)
{
    if (verbose) puts("STUB: zlarzb_ called");
    return NULL;
}

void* zlarzt(void)
{
    if (verbose) puts("STUB: zlarzt called");
    return NULL;
}

void* zlarzt_(void)
{
    if (verbose) puts("STUB: zlarzt_ called");
    return NULL;
}

void* zlascl(void)
{
    if (verbose) puts("STUB: zlascl called");
    return NULL;
}

void* zlascl2(void)
{
    if (verbose) puts("STUB: zlascl2 called");
    return NULL;
}

void* zlascl2_(void)
{
    if (verbose) puts("STUB: zlascl2_ called");
    return NULL;
}

void* zlascl_(void)
{
    if (verbose) puts("STUB: zlascl_ called");
    return NULL;
}

void* zlaset(void)
{
    if (verbose) puts("STUB: zlaset called");
    return NULL;
}

void* zlaset_(void)
{
    if (verbose) puts("STUB: zlaset_ called");
    return NULL;
}

void* zlasr(void)
{
    if (verbose) puts("STUB: zlasr called");
    return NULL;
}

void* zlasr_(void)
{
    if (verbose) puts("STUB: zlasr_ called");
    return NULL;
}

void* zlassq(void)
{
    if (verbose) puts("STUB: zlassq called");
    return NULL;
}

void* zlassq_(void)
{
    if (verbose) puts("STUB: zlassq_ called");
    return NULL;
}

void* zlaswp(void)
{
    if (verbose) puts("STUB: zlaswp called");
    return NULL;
}

void* zlaswp_(void)
{
    if (verbose) puts("STUB: zlaswp_ called");
    return NULL;
}

void* zlasyf(void)
{
    if (verbose) puts("STUB: zlasyf called");
    return NULL;
}

void* zlasyf_(void)
{
    if (verbose) puts("STUB: zlasyf_ called");
    return NULL;
}

void* zlat2c(void)
{
    if (verbose) puts("STUB: zlat2c called");
    return NULL;
}

void* zlat2c_(void)
{
    if (verbose) puts("STUB: zlat2c_ called");
    return NULL;
}

void* zlatbs(void)
{
    if (verbose) puts("STUB: zlatbs called");
    return NULL;
}

void* zlatbs_(void)
{
    if (verbose) puts("STUB: zlatbs_ called");
    return NULL;
}

void* zlatdf(void)
{
    if (verbose) puts("STUB: zlatdf called");
    return NULL;
}

void* zlatdf_(void)
{
    if (verbose) puts("STUB: zlatdf_ called");
    return NULL;
}

void* zlatps(void)
{
    if (verbose) puts("STUB: zlatps called");
    return NULL;
}

void* zlatps_(void)
{
    if (verbose) puts("STUB: zlatps_ called");
    return NULL;
}

void* zlatrd(void)
{
    if (verbose) puts("STUB: zlatrd called");
    return NULL;
}

void* zlatrd_(void)
{
    if (verbose) puts("STUB: zlatrd_ called");
    return NULL;
}

void* zlatrs(void)
{
    if (verbose) puts("STUB: zlatrs called");
    return NULL;
}

void* zlatrs_(void)
{
    if (verbose) puts("STUB: zlatrs_ called");
    return NULL;
}

void* zlatrz(void)
{
    if (verbose) puts("STUB: zlatrz called");
    return NULL;
}

void* zlatrz_(void)
{
    if (verbose) puts("STUB: zlatrz_ called");
    return NULL;
}

void* zlatzm(void)
{
    if (verbose) puts("STUB: zlatzm called");
    return NULL;
}

void* zlatzm_(void)
{
    if (verbose) puts("STUB: zlatzm_ called");
    return NULL;
}

void* zlauu2(void)
{
    if (verbose) puts("STUB: zlauu2 called");
    return NULL;
}

void* zlauu2_(void)
{
    if (verbose) puts("STUB: zlauu2_ called");
    return NULL;
}

void* zlauum(void)
{
    if (verbose) puts("STUB: zlauum called");
    return NULL;
}

void* zlauum_(void)
{
    if (verbose) puts("STUB: zlauum_ called");
    return NULL;
}

void* zpbcon(void)
{
    if (verbose) puts("STUB: zpbcon called");
    return NULL;
}

void* zpbcon_(void)
{
    if (verbose) puts("STUB: zpbcon_ called");
    return NULL;
}

void* zpbequ(void)
{
    if (verbose) puts("STUB: zpbequ called");
    return NULL;
}

void* zpbequ_(void)
{
    if (verbose) puts("STUB: zpbequ_ called");
    return NULL;
}

void* zpbrfs(void)
{
    if (verbose) puts("STUB: zpbrfs called");
    return NULL;
}

void* zpbrfs_(void)
{
    if (verbose) puts("STUB: zpbrfs_ called");
    return NULL;
}

void* zpbstf(void)
{
    if (verbose) puts("STUB: zpbstf called");
    return NULL;
}

void* zpbstf_(void)
{
    if (verbose) puts("STUB: zpbstf_ called");
    return NULL;
}

void* zpbsv(void)
{
    if (verbose) puts("STUB: zpbsv called");
    return NULL;
}

void* zpbsv_(void)
{
    if (verbose) puts("STUB: zpbsv_ called");
    return NULL;
}

void* zpbsvx(void)
{
    if (verbose) puts("STUB: zpbsvx called");
    return NULL;
}

void* zpbsvx_(void)
{
    if (verbose) puts("STUB: zpbsvx_ called");
    return NULL;
}

void* zpbtf2(void)
{
    if (verbose) puts("STUB: zpbtf2 called");
    return NULL;
}

void* zpbtf2_(void)
{
    if (verbose) puts("STUB: zpbtf2_ called");
    return NULL;
}

void* zpbtrf(void)
{
    if (verbose) puts("STUB: zpbtrf called");
    return NULL;
}

void* zpbtrf_(void)
{
    if (verbose) puts("STUB: zpbtrf_ called");
    return NULL;
}

void* zpbtrs(void)
{
    if (verbose) puts("STUB: zpbtrs called");
    return NULL;
}

void* zpbtrs_(void)
{
    if (verbose) puts("STUB: zpbtrs_ called");
    return NULL;
}

void* zpftrf(void)
{
    if (verbose) puts("STUB: zpftrf called");
    return NULL;
}

void* zpftrf_(void)
{
    if (verbose) puts("STUB: zpftrf_ called");
    return NULL;
}

void* zpftri(void)
{
    if (verbose) puts("STUB: zpftri called");
    return NULL;
}

void* zpftri_(void)
{
    if (verbose) puts("STUB: zpftri_ called");
    return NULL;
}

void* zpftrs(void)
{
    if (verbose) puts("STUB: zpftrs called");
    return NULL;
}

void* zpftrs_(void)
{
    if (verbose) puts("STUB: zpftrs_ called");
    return NULL;
}

void* zpocon(void)
{
    if (verbose) puts("STUB: zpocon called");
    return NULL;
}

void* zpocon_(void)
{
    if (verbose) puts("STUB: zpocon_ called");
    return NULL;
}

void* zpoequ(void)
{
    if (verbose) puts("STUB: zpoequ called");
    return NULL;
}

void* zpoequ_(void)
{
    if (verbose) puts("STUB: zpoequ_ called");
    return NULL;
}

void* zpoequb(void)
{
    if (verbose) puts("STUB: zpoequb called");
    return NULL;
}

void* zpoequb_(void)
{
    if (verbose) puts("STUB: zpoequb_ called");
    return NULL;
}

void* zporfs(void)
{
    if (verbose) puts("STUB: zporfs called");
    return NULL;
}

void* zporfs_(void)
{
    if (verbose) puts("STUB: zporfs_ called");
    return NULL;
}

void* zposv(void)
{
    if (verbose) puts("STUB: zposv called");
    return NULL;
}

void* zposv_(void)
{
    if (verbose) puts("STUB: zposv_ called");
    return NULL;
}

void* zposvx(void)
{
    if (verbose) puts("STUB: zposvx called");
    return NULL;
}

void* zposvx_(void)
{
    if (verbose) puts("STUB: zposvx_ called");
    return NULL;
}

void* zpotf2(void)
{
    if (verbose) puts("STUB: zpotf2 called");
    return NULL;
}

void* zpotf2_(void)
{
    if (verbose) puts("STUB: zpotf2_ called");
    return NULL;
}

void* zpotrf(void)
{
    if (verbose) puts("STUB: zpotrf called");
    return NULL;
}

void* zpotrf_(void)
{
    if (verbose) puts("STUB: zpotrf_ called");
    return NULL;
}

void* zpotri(void)
{
    if (verbose) puts("STUB: zpotri called");
    return NULL;
}

void* zpotri_(void)
{
    if (verbose) puts("STUB: zpotri_ called");
    return NULL;
}

void* zpotrs(void)
{
    if (verbose) puts("STUB: zpotrs called");
    return NULL;
}

void* zpotrs_(void)
{
    if (verbose) puts("STUB: zpotrs_ called");
    return NULL;
}

void* zppcon(void)
{
    if (verbose) puts("STUB: zppcon called");
    return NULL;
}

void* zppcon_(void)
{
    if (verbose) puts("STUB: zppcon_ called");
    return NULL;
}

void* zppequ(void)
{
    if (verbose) puts("STUB: zppequ called");
    return NULL;
}

void* zppequ_(void)
{
    if (verbose) puts("STUB: zppequ_ called");
    return NULL;
}

void* zpprfs(void)
{
    if (verbose) puts("STUB: zpprfs called");
    return NULL;
}

void* zpprfs_(void)
{
    if (verbose) puts("STUB: zpprfs_ called");
    return NULL;
}

void* zppsv(void)
{
    if (verbose) puts("STUB: zppsv called");
    return NULL;
}

void* zppsv_(void)
{
    if (verbose) puts("STUB: zppsv_ called");
    return NULL;
}

void* zppsvx(void)
{
    if (verbose) puts("STUB: zppsvx called");
    return NULL;
}

void* zppsvx_(void)
{
    if (verbose) puts("STUB: zppsvx_ called");
    return NULL;
}

void* zpptrf(void)
{
    if (verbose) puts("STUB: zpptrf called");
    return NULL;
}

void* zpptrf_(void)
{
    if (verbose) puts("STUB: zpptrf_ called");
    return NULL;
}

void* zpptri(void)
{
    if (verbose) puts("STUB: zpptri called");
    return NULL;
}

void* zpptri_(void)
{
    if (verbose) puts("STUB: zpptri_ called");
    return NULL;
}

void* zpptrs(void)
{
    if (verbose) puts("STUB: zpptrs called");
    return NULL;
}

void* zpptrs_(void)
{
    if (verbose) puts("STUB: zpptrs_ called");
    return NULL;
}

void* zpstf2(void)
{
    if (verbose) puts("STUB: zpstf2 called");
    return NULL;
}

void* zpstf2_(void)
{
    if (verbose) puts("STUB: zpstf2_ called");
    return NULL;
}

void* zpstrf(void)
{
    if (verbose) puts("STUB: zpstrf called");
    return NULL;
}

void* zpstrf_(void)
{
    if (verbose) puts("STUB: zpstrf_ called");
    return NULL;
}

void* zptcon(void)
{
    if (verbose) puts("STUB: zptcon called");
    return NULL;
}

void* zptcon_(void)
{
    if (verbose) puts("STUB: zptcon_ called");
    return NULL;
}

void* zpteqr(void)
{
    if (verbose) puts("STUB: zpteqr called");
    return NULL;
}

void* zpteqr_(void)
{
    if (verbose) puts("STUB: zpteqr_ called");
    return NULL;
}

void* zptrfs(void)
{
    if (verbose) puts("STUB: zptrfs called");
    return NULL;
}

void* zptrfs_(void)
{
    if (verbose) puts("STUB: zptrfs_ called");
    return NULL;
}

void* zptsv(void)
{
    if (verbose) puts("STUB: zptsv called");
    return NULL;
}

void* zptsv_(void)
{
    if (verbose) puts("STUB: zptsv_ called");
    return NULL;
}

void* zptsvx(void)
{
    if (verbose) puts("STUB: zptsvx called");
    return NULL;
}

void* zptsvx_(void)
{
    if (verbose) puts("STUB: zptsvx_ called");
    return NULL;
}

void* zpttrf(void)
{
    if (verbose) puts("STUB: zpttrf called");
    return NULL;
}

void* zpttrf_(void)
{
    if (verbose) puts("STUB: zpttrf_ called");
    return NULL;
}

void* zpttrs(void)
{
    if (verbose) puts("STUB: zpttrs called");
    return NULL;
}

void* zpttrs_(void)
{
    if (verbose) puts("STUB: zpttrs_ called");
    return NULL;
}

void* zptts2(void)
{
    if (verbose) puts("STUB: zptts2 called");
    return NULL;
}

void* zptts2_(void)
{
    if (verbose) puts("STUB: zptts2_ called");
    return NULL;
}

void* zrot(void)
{
    if (verbose) puts("STUB: zrot called");
    return NULL;
}

void* zrot_(void)
{
    if (verbose) puts("STUB: zrot_ called");
    return NULL;
}

void* zspcon(void)
{
    if (verbose) puts("STUB: zspcon called");
    return NULL;
}

void* zspcon_(void)
{
    if (verbose) puts("STUB: zspcon_ called");
    return NULL;
}

void* zspmv(void)
{
    if (verbose) puts("STUB: zspmv called");
    return NULL;
}

void* zspmv_(void)
{
    if (verbose) puts("STUB: zspmv_ called");
    return NULL;
}

void* zspr(void)
{
    if (verbose) puts("STUB: zspr called");
    return NULL;
}

void* zspr_(void)
{
    if (verbose) puts("STUB: zspr_ called");
    return NULL;
}

void* zsprfs(void)
{
    if (verbose) puts("STUB: zsprfs called");
    return NULL;
}

void* zsprfs_(void)
{
    if (verbose) puts("STUB: zsprfs_ called");
    return NULL;
}

void* zspsv(void)
{
    if (verbose) puts("STUB: zspsv called");
    return NULL;
}

void* zspsv_(void)
{
    if (verbose) puts("STUB: zspsv_ called");
    return NULL;
}

void* zspsvx(void)
{
    if (verbose) puts("STUB: zspsvx called");
    return NULL;
}

void* zspsvx_(void)
{
    if (verbose) puts("STUB: zspsvx_ called");
    return NULL;
}

void* zsptrf(void)
{
    if (verbose) puts("STUB: zsptrf called");
    return NULL;
}

void* zsptrf_(void)
{
    if (verbose) puts("STUB: zsptrf_ called");
    return NULL;
}

void* zsptri(void)
{
    if (verbose) puts("STUB: zsptri called");
    return NULL;
}

void* zsptri_(void)
{
    if (verbose) puts("STUB: zsptri_ called");
    return NULL;
}

void* zsptrs(void)
{
    if (verbose) puts("STUB: zsptrs called");
    return NULL;
}

void* zsptrs_(void)
{
    if (verbose) puts("STUB: zsptrs_ called");
    return NULL;
}

void* zstedc(void)
{
    if (verbose) puts("STUB: zstedc called");
    return NULL;
}

void* zstedc_(void)
{
    if (verbose) puts("STUB: zstedc_ called");
    return NULL;
}

void* zstegr(void)
{
    if (verbose) puts("STUB: zstegr called");
    return NULL;
}

void* zstegr_(void)
{
    if (verbose) puts("STUB: zstegr_ called");
    return NULL;
}

void* zstein(void)
{
    if (verbose) puts("STUB: zstein called");
    return NULL;
}

void* zstein_(void)
{
    if (verbose) puts("STUB: zstein_ called");
    return NULL;
}

void* zstemr(void)
{
    if (verbose) puts("STUB: zstemr called");
    return NULL;
}

void* zstemr_(void)
{
    if (verbose) puts("STUB: zstemr_ called");
    return NULL;
}

void* zsteqr(void)
{
    if (verbose) puts("STUB: zsteqr called");
    return NULL;
}

void* zsteqr_(void)
{
    if (verbose) puts("STUB: zsteqr_ called");
    return NULL;
}

void* zsycon(void)
{
    if (verbose) puts("STUB: zsycon called");
    return NULL;
}

void* zsycon_(void)
{
    if (verbose) puts("STUB: zsycon_ called");
    return NULL;
}

void* zsyequb(void)
{
    if (verbose) puts("STUB: zsyequb called");
    return NULL;
}

void* zsyequb_(void)
{
    if (verbose) puts("STUB: zsyequb_ called");
    return NULL;
}

void* zsymv(void)
{
    if (verbose) puts("STUB: zsymv called");
    return NULL;
}

void* zsymv_(void)
{
    if (verbose) puts("STUB: zsymv_ called");
    return NULL;
}

void* zsyr(void)
{
    if (verbose) puts("STUB: zsyr called");
    return NULL;
}

void* zsyr_(void)
{
    if (verbose) puts("STUB: zsyr_ called");
    return NULL;
}

void* zsyrfs(void)
{
    if (verbose) puts("STUB: zsyrfs called");
    return NULL;
}

void* zsyrfs_(void)
{
    if (verbose) puts("STUB: zsyrfs_ called");
    return NULL;
}

void* zsysv(void)
{
    if (verbose) puts("STUB: zsysv called");
    return NULL;
}

void* zsysv_(void)
{
    if (verbose) puts("STUB: zsysv_ called");
    return NULL;
}

void* zsysvx(void)
{
    if (verbose) puts("STUB: zsysvx called");
    return NULL;
}

void* zsysvx_(void)
{
    if (verbose) puts("STUB: zsysvx_ called");
    return NULL;
}

void* zsytf2(void)
{
    if (verbose) puts("STUB: zsytf2 called");
    return NULL;
}

void* zsytf2_(void)
{
    if (verbose) puts("STUB: zsytf2_ called");
    return NULL;
}

void* zsytrf(void)
{
    if (verbose) puts("STUB: zsytrf called");
    return NULL;
}

void* zsytrf_(void)
{
    if (verbose) puts("STUB: zsytrf_ called");
    return NULL;
}

void* zsytri(void)
{
    if (verbose) puts("STUB: zsytri called");
    return NULL;
}

void* zsytri_(void)
{
    if (verbose) puts("STUB: zsytri_ called");
    return NULL;
}

void* zsytrs(void)
{
    if (verbose) puts("STUB: zsytrs called");
    return NULL;
}

void* zsytrs_(void)
{
    if (verbose) puts("STUB: zsytrs_ called");
    return NULL;
}

void* ztbcon(void)
{
    if (verbose) puts("STUB: ztbcon called");
    return NULL;
}

void* ztbcon_(void)
{
    if (verbose) puts("STUB: ztbcon_ called");
    return NULL;
}

void* ztbrfs(void)
{
    if (verbose) puts("STUB: ztbrfs called");
    return NULL;
}

void* ztbrfs_(void)
{
    if (verbose) puts("STUB: ztbrfs_ called");
    return NULL;
}

void* ztbtrs(void)
{
    if (verbose) puts("STUB: ztbtrs called");
    return NULL;
}

void* ztbtrs_(void)
{
    if (verbose) puts("STUB: ztbtrs_ called");
    return NULL;
}

void* ztfsm(void)
{
    if (verbose) puts("STUB: ztfsm called");
    return NULL;
}

void* ztfsm_(void)
{
    if (verbose) puts("STUB: ztfsm_ called");
    return NULL;
}

void* ztftri(void)
{
    if (verbose) puts("STUB: ztftri called");
    return NULL;
}

void* ztftri_(void)
{
    if (verbose) puts("STUB: ztftri_ called");
    return NULL;
}

void* ztfttp(void)
{
    if (verbose) puts("STUB: ztfttp called");
    return NULL;
}

void* ztfttp_(void)
{
    if (verbose) puts("STUB: ztfttp_ called");
    return NULL;
}

void* ztfttr(void)
{
    if (verbose) puts("STUB: ztfttr called");
    return NULL;
}

void* ztfttr_(void)
{
    if (verbose) puts("STUB: ztfttr_ called");
    return NULL;
}

void* ztgevc(void)
{
    if (verbose) puts("STUB: ztgevc called");
    return NULL;
}

void* ztgevc_(void)
{
    if (verbose) puts("STUB: ztgevc_ called");
    return NULL;
}

void* ztgex2(void)
{
    if (verbose) puts("STUB: ztgex2 called");
    return NULL;
}

void* ztgex2_(void)
{
    if (verbose) puts("STUB: ztgex2_ called");
    return NULL;
}

void* ztgexc(void)
{
    if (verbose) puts("STUB: ztgexc called");
    return NULL;
}

void* ztgexc_(void)
{
    if (verbose) puts("STUB: ztgexc_ called");
    return NULL;
}

void* ztgsen(void)
{
    if (verbose) puts("STUB: ztgsen called");
    return NULL;
}

void* ztgsen_(void)
{
    if (verbose) puts("STUB: ztgsen_ called");
    return NULL;
}

void* ztgsja(void)
{
    if (verbose) puts("STUB: ztgsja called");
    return NULL;
}

void* ztgsja_(void)
{
    if (verbose) puts("STUB: ztgsja_ called");
    return NULL;
}

void* ztgsna(void)
{
    if (verbose) puts("STUB: ztgsna called");
    return NULL;
}

void* ztgsna_(void)
{
    if (verbose) puts("STUB: ztgsna_ called");
    return NULL;
}

void* ztgsy2(void)
{
    if (verbose) puts("STUB: ztgsy2 called");
    return NULL;
}

void* ztgsy2_(void)
{
    if (verbose) puts("STUB: ztgsy2_ called");
    return NULL;
}

void* ztgsyl(void)
{
    if (verbose) puts("STUB: ztgsyl called");
    return NULL;
}

void* ztgsyl_(void)
{
    if (verbose) puts("STUB: ztgsyl_ called");
    return NULL;
}

void* ztpcon(void)
{
    if (verbose) puts("STUB: ztpcon called");
    return NULL;
}

void* ztpcon_(void)
{
    if (verbose) puts("STUB: ztpcon_ called");
    return NULL;
}

void* ztprfs(void)
{
    if (verbose) puts("STUB: ztprfs called");
    return NULL;
}

void* ztprfs_(void)
{
    if (verbose) puts("STUB: ztprfs_ called");
    return NULL;
}

void* ztptri(void)
{
    if (verbose) puts("STUB: ztptri called");
    return NULL;
}

void* ztptri_(void)
{
    if (verbose) puts("STUB: ztptri_ called");
    return NULL;
}

void* ztptrs(void)
{
    if (verbose) puts("STUB: ztptrs called");
    return NULL;
}

void* ztptrs_(void)
{
    if (verbose) puts("STUB: ztptrs_ called");
    return NULL;
}

void* ztpttf(void)
{
    if (verbose) puts("STUB: ztpttf called");
    return NULL;
}

void* ztpttf_(void)
{
    if (verbose) puts("STUB: ztpttf_ called");
    return NULL;
}

void* ztpttr(void)
{
    if (verbose) puts("STUB: ztpttr called");
    return NULL;
}

void* ztpttr_(void)
{
    if (verbose) puts("STUB: ztpttr_ called");
    return NULL;
}

void* ztrcon(void)
{
    if (verbose) puts("STUB: ztrcon called");
    return NULL;
}

void* ztrcon_(void)
{
    if (verbose) puts("STUB: ztrcon_ called");
    return NULL;
}

void* ztrevc(void)
{
    if (verbose) puts("STUB: ztrevc called");
    return NULL;
}

void* ztrevc_(void)
{
    if (verbose) puts("STUB: ztrevc_ called");
    return NULL;
}

void* ztrexc(void)
{
    if (verbose) puts("STUB: ztrexc called");
    return NULL;
}

void* ztrexc_(void)
{
    if (verbose) puts("STUB: ztrexc_ called");
    return NULL;
}

void* ztrrfs(void)
{
    if (verbose) puts("STUB: ztrrfs called");
    return NULL;
}

void* ztrrfs_(void)
{
    if (verbose) puts("STUB: ztrrfs_ called");
    return NULL;
}

void* ztrsen(void)
{
    if (verbose) puts("STUB: ztrsen called");
    return NULL;
}

void* ztrsen_(void)
{
    if (verbose) puts("STUB: ztrsen_ called");
    return NULL;
}

void* ztrsna(void)
{
    if (verbose) puts("STUB: ztrsna called");
    return NULL;
}

void* ztrsna_(void)
{
    if (verbose) puts("STUB: ztrsna_ called");
    return NULL;
}

void* ztrsyl(void)
{
    if (verbose) puts("STUB: ztrsyl called");
    return NULL;
}

void* ztrsyl_(void)
{
    if (verbose) puts("STUB: ztrsyl_ called");
    return NULL;
}

void* ztrti2(void)
{
    if (verbose) puts("STUB: ztrti2 called");
    return NULL;
}

void* ztrti2_(void)
{
    if (verbose) puts("STUB: ztrti2_ called");
    return NULL;
}

void* ztrtri(void)
{
    if (verbose) puts("STUB: ztrtri called");
    return NULL;
}

void* ztrtri_(void)
{
    if (verbose) puts("STUB: ztrtri_ called");
    return NULL;
}

void* ztrtrs(void)
{
    if (verbose) puts("STUB: ztrtrs called");
    return NULL;
}

void* ztrtrs_(void)
{
    if (verbose) puts("STUB: ztrtrs_ called");
    return NULL;
}

void* ztrttf(void)
{
    if (verbose) puts("STUB: ztrttf called");
    return NULL;
}

void* ztrttf_(void)
{
    if (verbose) puts("STUB: ztrttf_ called");
    return NULL;
}

void* ztrttp(void)
{
    if (verbose) puts("STUB: ztrttp called");
    return NULL;
}

void* ztrttp_(void)
{
    if (verbose) puts("STUB: ztrttp_ called");
    return NULL;
}

void* ztzrqf(void)
{
    if (verbose) puts("STUB: ztzrqf called");
    return NULL;
}

void* ztzrqf_(void)
{
    if (verbose) puts("STUB: ztzrqf_ called");
    return NULL;
}

void* ztzrzf(void)
{
    if (verbose) puts("STUB: ztzrzf called");
    return NULL;
}

void* ztzrzf_(void)
{
    if (verbose) puts("STUB: ztzrzf_ called");
    return NULL;
}

void* zung2l(void)
{
    if (verbose) puts("STUB: zung2l called");
    return NULL;
}

void* zung2l_(void)
{
    if (verbose) puts("STUB: zung2l_ called");
    return NULL;
}

void* zung2r(void)
{
    if (verbose) puts("STUB: zung2r called");
    return NULL;
}

void* zung2r_(void)
{
    if (verbose) puts("STUB: zung2r_ called");
    return NULL;
}

void* zungbr(void)
{
    if (verbose) puts("STUB: zungbr called");
    return NULL;
}

void* zungbr_(void)
{
    if (verbose) puts("STUB: zungbr_ called");
    return NULL;
}

void* zunghr(void)
{
    if (verbose) puts("STUB: zunghr called");
    return NULL;
}

void* zunghr_(void)
{
    if (verbose) puts("STUB: zunghr_ called");
    return NULL;
}

void* zungl2(void)
{
    if (verbose) puts("STUB: zungl2 called");
    return NULL;
}

void* zungl2_(void)
{
    if (verbose) puts("STUB: zungl2_ called");
    return NULL;
}

void* zunglq(void)
{
    if (verbose) puts("STUB: zunglq called");
    return NULL;
}

void* zunglq_(void)
{
    if (verbose) puts("STUB: zunglq_ called");
    return NULL;
}

void* zungql(void)
{
    if (verbose) puts("STUB: zungql called");
    return NULL;
}

void* zungql_(void)
{
    if (verbose) puts("STUB: zungql_ called");
    return NULL;
}

void* zungqr(void)
{
    if (verbose) puts("STUB: zungqr called");
    return NULL;
}

void* zungqr_(void)
{
    if (verbose) puts("STUB: zungqr_ called");
    return NULL;
}

void* zungr2(void)
{
    if (verbose) puts("STUB: zungr2 called");
    return NULL;
}

void* zungr2_(void)
{
    if (verbose) puts("STUB: zungr2_ called");
    return NULL;
}

void* zungrq(void)
{
    if (verbose) puts("STUB: zungrq called");
    return NULL;
}

void* zungrq_(void)
{
    if (verbose) puts("STUB: zungrq_ called");
    return NULL;
}

void* zungtr(void)
{
    if (verbose) puts("STUB: zungtr called");
    return NULL;
}

void* zungtr_(void)
{
    if (verbose) puts("STUB: zungtr_ called");
    return NULL;
}

void* zunm2l(void)
{
    if (verbose) puts("STUB: zunm2l called");
    return NULL;
}

void* zunm2l_(void)
{
    if (verbose) puts("STUB: zunm2l_ called");
    return NULL;
}

void* zunm2r(void)
{
    if (verbose) puts("STUB: zunm2r called");
    return NULL;
}

void* zunm2r_(void)
{
    if (verbose) puts("STUB: zunm2r_ called");
    return NULL;
}

void* zunmbr(void)
{
    if (verbose) puts("STUB: zunmbr called");
    return NULL;
}

void* zunmbr_(void)
{
    if (verbose) puts("STUB: zunmbr_ called");
    return NULL;
}

void* zunmhr(void)
{
    if (verbose) puts("STUB: zunmhr called");
    return NULL;
}

void* zunmhr_(void)
{
    if (verbose) puts("STUB: zunmhr_ called");
    return NULL;
}

void* zunml2(void)
{
    if (verbose) puts("STUB: zunml2 called");
    return NULL;
}

void* zunml2_(void)
{
    if (verbose) puts("STUB: zunml2_ called");
    return NULL;
}

void* zunmlq(void)
{
    if (verbose) puts("STUB: zunmlq called");
    return NULL;
}

void* zunmlq_(void)
{
    if (verbose) puts("STUB: zunmlq_ called");
    return NULL;
}

void* zunmql(void)
{
    if (verbose) puts("STUB: zunmql called");
    return NULL;
}

void* zunmql_(void)
{
    if (verbose) puts("STUB: zunmql_ called");
    return NULL;
}

void* zunmqr(void)
{
    if (verbose) puts("STUB: zunmqr called");
    return NULL;
}

void* zunmqr_(void)
{
    if (verbose) puts("STUB: zunmqr_ called");
    return NULL;
}

void* zunmr2(void)
{
    if (verbose) puts("STUB: zunmr2 called");
    return NULL;
}

void* zunmr2_(void)
{
    if (verbose) puts("STUB: zunmr2_ called");
    return NULL;
}

void* zunmr3(void)
{
    if (verbose) puts("STUB: zunmr3 called");
    return NULL;
}

void* zunmr3_(void)
{
    if (verbose) puts("STUB: zunmr3_ called");
    return NULL;
}

void* zunmrq(void)
{
    if (verbose) puts("STUB: zunmrq called");
    return NULL;
}

void* zunmrq_(void)
{
    if (verbose) puts("STUB: zunmrq_ called");
    return NULL;
}

void* zunmrz(void)
{
    if (verbose) puts("STUB: zunmrz called");
    return NULL;
}

void* zunmrz_(void)
{
    if (verbose) puts("STUB: zunmrz_ called");
    return NULL;
}

void* zunmtr(void)
{
    if (verbose) puts("STUB: zunmtr called");
    return NULL;
}

void* zunmtr_(void)
{
    if (verbose) puts("STUB: zunmtr_ called");
    return NULL;
}

void* zupgtr(void)
{
    if (verbose) puts("STUB: zupgtr called");
    return NULL;
}

void* zupgtr_(void)
{
    if (verbose) puts("STUB: zupgtr_ called");
    return NULL;
}

void* zupmtr(void)
{
    if (verbose) puts("STUB: zupmtr called");
    return NULL;
}

void* zupmtr_(void)
{
    if (verbose) puts("STUB: zupmtr_ called");
    return NULL;
}
