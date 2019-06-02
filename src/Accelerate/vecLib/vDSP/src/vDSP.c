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


#include <vDSP/vDSP.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* vDSP_DCT_CreateSetup(void)
{
    if (verbose) puts("STUB: vDSP_DCT_CreateSetup called");
    return NULL;
}

void* vDSP_DCT_Execute(void)
{
    if (verbose) puts("STUB: vDSP_DCT_Execute called");
    return NULL;
}

void* vDSP_DFT_CreateSetup(void)
{
    if (verbose) puts("STUB: vDSP_DFT_CreateSetup called");
    return NULL;
}

void* vDSP_DFT_DestroySetup(void)
{
    if (verbose) puts("STUB: vDSP_DFT_DestroySetup called");
    return NULL;
}

void* vDSP_DFT_DestroySetupD(void)
{
    if (verbose) puts("STUB: vDSP_DFT_DestroySetupD called");
    return NULL;
}

void* vDSP_DFT_Execute(void)
{
    if (verbose) puts("STUB: vDSP_DFT_Execute called");
    return NULL;
}

void* vDSP_DFT_ExecuteD(void)
{
    if (verbose) puts("STUB: vDSP_DFT_ExecuteD called");
    return NULL;
}

void* vDSP_DFT_zop(void)
{
    if (verbose) puts("STUB: vDSP_DFT_zop called");
    return NULL;
}

void* vDSP_DFT_zop_CreateSetup(void)
{
    if (verbose) puts("STUB: vDSP_DFT_zop_CreateSetup called");
    return NULL;
}

void* vDSP_DFT_zop_CreateSetupD(void)
{
    if (verbose) puts("STUB: vDSP_DFT_zop_CreateSetupD called");
    return NULL;
}

void* vDSP_DFT_zrop_CreateSetup(void)
{
    if (verbose) puts("STUB: vDSP_DFT_zrop_CreateSetup called");
    return NULL;
}

void* vDSP_DFT_zrop_CreateSetupD(void)
{
    if (verbose) puts("STUB: vDSP_DFT_zrop_CreateSetupD called");
    return NULL;
}

void* vDSP_FFT16_copv(void)
{
    if (verbose) puts("STUB: vDSP_FFT16_copv called");
    return NULL;
}

void* vDSP_FFT16_zopv(void)
{
    if (verbose) puts("STUB: vDSP_FFT16_zopv called");
    return NULL;
}

void* vDSP_FFT32_copv(void)
{
    if (verbose) puts("STUB: vDSP_FFT32_copv called");
    return NULL;
}

void* vDSP_FFT32_zopv(void)
{
    if (verbose) puts("STUB: vDSP_FFT32_zopv called");
    return NULL;
}

void* vDSP_biquad(void)
{
    if (verbose) puts("STUB: vDSP_biquad called");
    return NULL;
}

void* vDSP_biquadD(void)
{
    if (verbose) puts("STUB: vDSP_biquadD called");
    return NULL;
}

void* vDSP_biquad_CreateSetup(void)
{
    if (verbose) puts("STUB: vDSP_biquad_CreateSetup called");
    return NULL;
}

void* vDSP_biquad_CreateSetupD(void)
{
    if (verbose) puts("STUB: vDSP_biquad_CreateSetupD called");
    return NULL;
}

void* vDSP_biquad_DestroySetup(void)
{
    if (verbose) puts("STUB: vDSP_biquad_DestroySetup called");
    return NULL;
}

void* vDSP_biquad_DestroySetupD(void)
{
    if (verbose) puts("STUB: vDSP_biquad_DestroySetupD called");
    return NULL;
}

void* vDSP_biquadm(void)
{
    if (verbose) puts("STUB: vDSP_biquadm called");
    return NULL;
}

void* vDSP_biquadmD(void)
{
    if (verbose) puts("STUB: vDSP_biquadmD called");
    return NULL;
}

void* vDSP_biquadm_CopyState(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_CopyState called");
    return NULL;
}

void* vDSP_biquadm_CopyStateD(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_CopyStateD called");
    return NULL;
}

void* vDSP_biquadm_CreateSetup(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_CreateSetup called");
    return NULL;
}

void* vDSP_biquadm_CreateSetupD(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_CreateSetupD called");
    return NULL;
}

void* vDSP_biquadm_DestroySetup(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_DestroySetup called");
    return NULL;
}

void* vDSP_biquadm_DestroySetupD(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_DestroySetupD called");
    return NULL;
}

void* vDSP_biquadm_ResetState(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_ResetState called");
    return NULL;
}

void* vDSP_biquadm_ResetStateD(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_ResetStateD called");
    return NULL;
}

void* vDSP_biquadm_SetActiveFilters(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_SetActiveFilters called");
    return NULL;
}

void* vDSP_biquadm_SetCoefficientsDouble(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_SetCoefficientsDouble called");
    return NULL;
}

void* vDSP_biquadm_SetCoefficientsSingle(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_SetCoefficientsSingle called");
    return NULL;
}

void* vDSP_biquadm_SetTargetsDouble(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_SetTargetsDouble called");
    return NULL;
}

void* vDSP_biquadm_SetTargetsSingle(void)
{
    if (verbose) puts("STUB: vDSP_biquadm_SetTargetsSingle called");
    return NULL;
}

void* vDSP_blkman_window(void)
{
    if (verbose) puts("STUB: vDSP_blkman_window called");
    return NULL;
}

void* vDSP_blkman_windowD(void)
{
    if (verbose) puts("STUB: vDSP_blkman_windowD called");
    return NULL;
}

void* vDSP_conv(void)
{
    if (verbose) puts("STUB: vDSP_conv called");
    return NULL;
}

void* vDSP_convD(void)
{
    if (verbose) puts("STUB: vDSP_convD called");
    return NULL;
}

void* vDSP_create_fftsetup(void)
{
    if (verbose) puts("STUB: vDSP_create_fftsetup called");
    return NULL;
}

void* vDSP_create_fftsetupD(void)
{
    if (verbose) puts("STUB: vDSP_create_fftsetupD called");
    return NULL;
}

void* vDSP_ctoz(void)
{
    if (verbose) puts("STUB: vDSP_ctoz called");
    return NULL;
}

void* vDSP_ctozD(void)
{
    if (verbose) puts("STUB: vDSP_ctozD called");
    return NULL;
}

void* vDSP_deq22(void)
{
    if (verbose) puts("STUB: vDSP_deq22 called");
    return NULL;
}

void* vDSP_deq22D(void)
{
    if (verbose) puts("STUB: vDSP_deq22D called");
    return NULL;
}

void* vDSP_desamp(void)
{
    if (verbose) puts("STUB: vDSP_desamp called");
    return NULL;
}

void* vDSP_desampD(void)
{
    if (verbose) puts("STUB: vDSP_desampD called");
    return NULL;
}

void* vDSP_destroy_fftsetup(void)
{
    if (verbose) puts("STUB: vDSP_destroy_fftsetup called");
    return NULL;
}

void* vDSP_destroy_fftsetupD(void)
{
    if (verbose) puts("STUB: vDSP_destroy_fftsetupD called");
    return NULL;
}

void* vDSP_distancesq(void)
{
    if (verbose) puts("STUB: vDSP_distancesq called");
    return NULL;
}

void* vDSP_distancesqD(void)
{
    if (verbose) puts("STUB: vDSP_distancesqD called");
    return NULL;
}

void* vDSP_dotpr(void)
{
    if (verbose) puts("STUB: vDSP_dotpr called");
    return NULL;
}

void* vDSP_dotpr2(void)
{
    if (verbose) puts("STUB: vDSP_dotpr2 called");
    return NULL;
}

void* vDSP_dotpr2D(void)
{
    if (verbose) puts("STUB: vDSP_dotpr2D called");
    return NULL;
}

void* vDSP_dotpr2_s1_15(void)
{
    if (verbose) puts("STUB: vDSP_dotpr2_s1_15 called");
    return NULL;
}

void* vDSP_dotpr2_s8_24(void)
{
    if (verbose) puts("STUB: vDSP_dotpr2_s8_24 called");
    return NULL;
}

void* vDSP_dotprD(void)
{
    if (verbose) puts("STUB: vDSP_dotprD called");
    return NULL;
}

void* vDSP_dotpr_s1_15(void)
{
    if (verbose) puts("STUB: vDSP_dotpr_s1_15 called");
    return NULL;
}

void* vDSP_dotpr_s8_24(void)
{
    if (verbose) puts("STUB: vDSP_dotpr_s8_24 called");
    return NULL;
}

void* vDSP_f3x3(void)
{
    if (verbose) puts("STUB: vDSP_f3x3 called");
    return NULL;
}

void* vDSP_f3x3D(void)
{
    if (verbose) puts("STUB: vDSP_f3x3D called");
    return NULL;
}

void* vDSP_f5x5(void)
{
    if (verbose) puts("STUB: vDSP_f5x5 called");
    return NULL;
}

void* vDSP_f5x5D(void)
{
    if (verbose) puts("STUB: vDSP_f5x5D called");
    return NULL;
}

void* vDSP_fft2d_zip(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zip called");
    return NULL;
}

void* vDSP_fft2d_zipD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zipD called");
    return NULL;
}

void* vDSP_fft2d_zipt(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zipt called");
    return NULL;
}

void* vDSP_fft2d_ziptD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_ziptD called");
    return NULL;
}

void* vDSP_fft2d_zop(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zop called");
    return NULL;
}

void* vDSP_fft2d_zopD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zopD called");
    return NULL;
}

void* vDSP_fft2d_zopt(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zopt called");
    return NULL;
}

void* vDSP_fft2d_zoptD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zoptD called");
    return NULL;
}

void* vDSP_fft2d_zrip(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zrip called");
    return NULL;
}

void* vDSP_fft2d_zripD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zripD called");
    return NULL;
}

void* vDSP_fft2d_zript(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zript called");
    return NULL;
}

void* vDSP_fft2d_zriptD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zriptD called");
    return NULL;
}

void* vDSP_fft2d_zrop(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zrop called");
    return NULL;
}

void* vDSP_fft2d_zropD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zropD called");
    return NULL;
}

void* vDSP_fft2d_zropt(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zropt called");
    return NULL;
}

void* vDSP_fft2d_zroptD(void)
{
    if (verbose) puts("STUB: vDSP_fft2d_zroptD called");
    return NULL;
}

void* vDSP_fft3_zop(void)
{
    if (verbose) puts("STUB: vDSP_fft3_zop called");
    return NULL;
}

void* vDSP_fft3_zopD(void)
{
    if (verbose) puts("STUB: vDSP_fft3_zopD called");
    return NULL;
}

void* vDSP_fft5_zop(void)
{
    if (verbose) puts("STUB: vDSP_fft5_zop called");
    return NULL;
}

void* vDSP_fft5_zopD(void)
{
    if (verbose) puts("STUB: vDSP_fft5_zopD called");
    return NULL;
}

void* vDSP_fft_zip(void)
{
    if (verbose) puts("STUB: vDSP_fft_zip called");
    return NULL;
}

void* vDSP_fft_zipD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zipD called");
    return NULL;
}

void* vDSP_fft_zipt(void)
{
    if (verbose) puts("STUB: vDSP_fft_zipt called");
    return NULL;
}

void* vDSP_fft_ziptD(void)
{
    if (verbose) puts("STUB: vDSP_fft_ziptD called");
    return NULL;
}

void* vDSP_fft_zop(void)
{
    if (verbose) puts("STUB: vDSP_fft_zop called");
    return NULL;
}

void* vDSP_fft_zopD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zopD called");
    return NULL;
}

void* vDSP_fft_zopt(void)
{
    if (verbose) puts("STUB: vDSP_fft_zopt called");
    return NULL;
}

void* vDSP_fft_zoptD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zoptD called");
    return NULL;
}

void* vDSP_fft_zrip(void)
{
    if (verbose) puts("STUB: vDSP_fft_zrip called");
    return NULL;
}

void* vDSP_fft_zripD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zripD called");
    return NULL;
}

void* vDSP_fft_zript(void)
{
    if (verbose) puts("STUB: vDSP_fft_zript called");
    return NULL;
}

void* vDSP_fft_zriptD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zriptD called");
    return NULL;
}

void* vDSP_fft_zrop(void)
{
    if (verbose) puts("STUB: vDSP_fft_zrop called");
    return NULL;
}

void* vDSP_fft_zropD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zropD called");
    return NULL;
}

void* vDSP_fft_zropt(void)
{
    if (verbose) puts("STUB: vDSP_fft_zropt called");
    return NULL;
}

void* vDSP_fft_zroptD(void)
{
    if (verbose) puts("STUB: vDSP_fft_zroptD called");
    return NULL;
}

void* vDSP_fftm_zip(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zip called");
    return NULL;
}

void* vDSP_fftm_zipD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zipD called");
    return NULL;
}

void* vDSP_fftm_zipt(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zipt called");
    return NULL;
}

void* vDSP_fftm_ziptD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_ziptD called");
    return NULL;
}

void* vDSP_fftm_zop(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zop called");
    return NULL;
}

void* vDSP_fftm_zopD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zopD called");
    return NULL;
}

void* vDSP_fftm_zopt(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zopt called");
    return NULL;
}

void* vDSP_fftm_zoptD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zoptD called");
    return NULL;
}

void* vDSP_fftm_zrip(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zrip called");
    return NULL;
}

void* vDSP_fftm_zripD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zripD called");
    return NULL;
}

void* vDSP_fftm_zript(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zript called");
    return NULL;
}

void* vDSP_fftm_zriptD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zriptD called");
    return NULL;
}

void* vDSP_fftm_zrop(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zrop called");
    return NULL;
}

void* vDSP_fftm_zropD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zropD called");
    return NULL;
}

void* vDSP_fftm_zropt(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zropt called");
    return NULL;
}

void* vDSP_fftm_zroptD(void)
{
    if (verbose) puts("STUB: vDSP_fftm_zroptD called");
    return NULL;
}

void* vDSP_hamm_window(void)
{
    if (verbose) puts("STUB: vDSP_hamm_window called");
    return NULL;
}

void* vDSP_hamm_windowD(void)
{
    if (verbose) puts("STUB: vDSP_hamm_windowD called");
    return NULL;
}

void* vDSP_hann_window(void)
{
    if (verbose) puts("STUB: vDSP_hann_window called");
    return NULL;
}

void* vDSP_hann_windowD(void)
{
    if (verbose) puts("STUB: vDSP_hann_windowD called");
    return NULL;
}

void* vDSP_imgfir(void)
{
    if (verbose) puts("STUB: vDSP_imgfir called");
    return NULL;
}

void* vDSP_imgfirD(void)
{
    if (verbose) puts("STUB: vDSP_imgfirD called");
    return NULL;
}

void* vDSP_maxmgv(void)
{
    if (verbose) puts("STUB: vDSP_maxmgv called");
    return NULL;
}

void* vDSP_maxmgvD(void)
{
    if (verbose) puts("STUB: vDSP_maxmgvD called");
    return NULL;
}

void* vDSP_maxmgvi(void)
{
    if (verbose) puts("STUB: vDSP_maxmgvi called");
    return NULL;
}

void* vDSP_maxmgviD(void)
{
    if (verbose) puts("STUB: vDSP_maxmgviD called");
    return NULL;
}

void* vDSP_maxv(void)
{
    if (verbose) puts("STUB: vDSP_maxv called");
    return NULL;
}

void* vDSP_maxvD(void)
{
    if (verbose) puts("STUB: vDSP_maxvD called");
    return NULL;
}

void* vDSP_maxvi(void)
{
    if (verbose) puts("STUB: vDSP_maxvi called");
    return NULL;
}

void* vDSP_maxviD(void)
{
    if (verbose) puts("STUB: vDSP_maxviD called");
    return NULL;
}

void* vDSP_meamgv(void)
{
    if (verbose) puts("STUB: vDSP_meamgv called");
    return NULL;
}

void* vDSP_meamgvD(void)
{
    if (verbose) puts("STUB: vDSP_meamgvD called");
    return NULL;
}

void* vDSP_meanv(void)
{
    if (verbose) puts("STUB: vDSP_meanv called");
    return NULL;
}

void* vDSP_meanvD(void)
{
    if (verbose) puts("STUB: vDSP_meanvD called");
    return NULL;
}

void* vDSP_measqv(void)
{
    if (verbose) puts("STUB: vDSP_measqv called");
    return NULL;
}

void* vDSP_measqvD(void)
{
    if (verbose) puts("STUB: vDSP_measqvD called");
    return NULL;
}

void* vDSP_minmgv(void)
{
    if (verbose) puts("STUB: vDSP_minmgv called");
    return NULL;
}

void* vDSP_minmgvD(void)
{
    if (verbose) puts("STUB: vDSP_minmgvD called");
    return NULL;
}

void* vDSP_minmgvi(void)
{
    if (verbose) puts("STUB: vDSP_minmgvi called");
    return NULL;
}

void* vDSP_minmgviD(void)
{
    if (verbose) puts("STUB: vDSP_minmgviD called");
    return NULL;
}

void* vDSP_minv(void)
{
    if (verbose) puts("STUB: vDSP_minv called");
    return NULL;
}

void* vDSP_minvD(void)
{
    if (verbose) puts("STUB: vDSP_minvD called");
    return NULL;
}

void* vDSP_minvi(void)
{
    if (verbose) puts("STUB: vDSP_minvi called");
    return NULL;
}

void* vDSP_minviD(void)
{
    if (verbose) puts("STUB: vDSP_minviD called");
    return NULL;
}

void* vDSP_mmov(void)
{
    if (verbose) puts("STUB: vDSP_mmov called");
    return NULL;
}

void* vDSP_mmovD(void)
{
    if (verbose) puts("STUB: vDSP_mmovD called");
    return NULL;
}

void* vDSP_mmul(void)
{
    if (verbose) puts("STUB: vDSP_mmul called");
    return NULL;
}

void* vDSP_mmulD(void)
{
    if (verbose) puts("STUB: vDSP_mmulD called");
    return NULL;
}

void* vDSP_mtrans(void)
{
    if (verbose) puts("STUB: vDSP_mtrans called");
    return NULL;
}

void* vDSP_mtransD(void)
{
    if (verbose) puts("STUB: vDSP_mtransD called");
    return NULL;
}

void* vDSP_mvessq(void)
{
    if (verbose) puts("STUB: vDSP_mvessq called");
    return NULL;
}

void* vDSP_mvessqD(void)
{
    if (verbose) puts("STUB: vDSP_mvessqD called");
    return NULL;
}

void* vDSP_normalize(void)
{
    if (verbose) puts("STUB: vDSP_normalize called");
    return NULL;
}

void* vDSP_normalizeD(void)
{
    if (verbose) puts("STUB: vDSP_normalizeD called");
    return NULL;
}

void* vDSP_nzcros(void)
{
    if (verbose) puts("STUB: vDSP_nzcros called");
    return NULL;
}

void* vDSP_nzcrosD(void)
{
    if (verbose) puts("STUB: vDSP_nzcrosD called");
    return NULL;
}

void* vDSP_polar(void)
{
    if (verbose) puts("STUB: vDSP_polar called");
    return NULL;
}

void* vDSP_polarD(void)
{
    if (verbose) puts("STUB: vDSP_polarD called");
    return NULL;
}

void* vDSP_rect(void)
{
    if (verbose) puts("STUB: vDSP_rect called");
    return NULL;
}

void* vDSP_rectD(void)
{
    if (verbose) puts("STUB: vDSP_rectD called");
    return NULL;
}

void* vDSP_rmsqv(void)
{
    if (verbose) puts("STUB: vDSP_rmsqv called");
    return NULL;
}

void* vDSP_rmsqvD(void)
{
    if (verbose) puts("STUB: vDSP_rmsqvD called");
    return NULL;
}

void* vDSP_svdiv(void)
{
    if (verbose) puts("STUB: vDSP_svdiv called");
    return NULL;
}

void* vDSP_svdivD(void)
{
    if (verbose) puts("STUB: vDSP_svdivD called");
    return NULL;
}

void* vDSP_sve(void)
{
    if (verbose) puts("STUB: vDSP_sve called");
    return NULL;
}

void* vDSP_sveD(void)
{
    if (verbose) puts("STUB: vDSP_sveD called");
    return NULL;
}

void* vDSP_sve_svesq(void)
{
    if (verbose) puts("STUB: vDSP_sve_svesq called");
    return NULL;
}

void* vDSP_sve_svesqD(void)
{
    if (verbose) puts("STUB: vDSP_sve_svesqD called");
    return NULL;
}

void* vDSP_svemg(void)
{
    if (verbose) puts("STUB: vDSP_svemg called");
    return NULL;
}

void* vDSP_svemgD(void)
{
    if (verbose) puts("STUB: vDSP_svemgD called");
    return NULL;
}

void* vDSP_svesq(void)
{
    if (verbose) puts("STUB: vDSP_svesq called");
    return NULL;
}

void* vDSP_svesqD(void)
{
    if (verbose) puts("STUB: vDSP_svesqD called");
    return NULL;
}

void* vDSP_svs(void)
{
    if (verbose) puts("STUB: vDSP_svs called");
    return NULL;
}

void* vDSP_svsD(void)
{
    if (verbose) puts("STUB: vDSP_svsD called");
    return NULL;
}

void* vDSP_vaam(void)
{
    if (verbose) puts("STUB: vDSP_vaam called");
    return NULL;
}

void* vDSP_vaamD(void)
{
    if (verbose) puts("STUB: vDSP_vaamD called");
    return NULL;
}

void* vDSP_vabs(void)
{
    if (verbose) puts("STUB: vDSP_vabs called");
    return NULL;
}

void* vDSP_vabsD(void)
{
    if (verbose) puts("STUB: vDSP_vabsD called");
    return NULL;
}

void* vDSP_vabsi(void)
{
    if (verbose) puts("STUB: vDSP_vabsi called");
    return NULL;
}

void* vDSP_vadd(void)
{
    if (verbose) puts("STUB: vDSP_vadd called");
    return NULL;
}

void* vDSP_vaddD(void)
{
    if (verbose) puts("STUB: vDSP_vaddD called");
    return NULL;
}

void* vDSP_vaddi(void)
{
    if (verbose) puts("STUB: vDSP_vaddi called");
    return NULL;
}

void* vDSP_vaddsub(void)
{
    if (verbose) puts("STUB: vDSP_vaddsub called");
    return NULL;
}

void* vDSP_vaddsubD(void)
{
    if (verbose) puts("STUB: vDSP_vaddsubD called");
    return NULL;
}

void* vDSP_vam(void)
{
    if (verbose) puts("STUB: vDSP_vam called");
    return NULL;
}

void* vDSP_vamD(void)
{
    if (verbose) puts("STUB: vDSP_vamD called");
    return NULL;
}

void* vDSP_vasbm(void)
{
    if (verbose) puts("STUB: vDSP_vasbm called");
    return NULL;
}

void* vDSP_vasbmD(void)
{
    if (verbose) puts("STUB: vDSP_vasbmD called");
    return NULL;
}

void* vDSP_vasm(void)
{
    if (verbose) puts("STUB: vDSP_vasm called");
    return NULL;
}

void* vDSP_vasmD(void)
{
    if (verbose) puts("STUB: vDSP_vasmD called");
    return NULL;
}

void* vDSP_vavlin(void)
{
    if (verbose) puts("STUB: vDSP_vavlin called");
    return NULL;
}

void* vDSP_vavlinD(void)
{
    if (verbose) puts("STUB: vDSP_vavlinD called");
    return NULL;
}

void* vDSP_vclip(void)
{
    if (verbose) puts("STUB: vDSP_vclip called");
    return NULL;
}

void* vDSP_vclipD(void)
{
    if (verbose) puts("STUB: vDSP_vclipD called");
    return NULL;
}

void* vDSP_vclipc(void)
{
    if (verbose) puts("STUB: vDSP_vclipc called");
    return NULL;
}

void* vDSP_vclipcD(void)
{
    if (verbose) puts("STUB: vDSP_vclipcD called");
    return NULL;
}

void* vDSP_vclr(void)
{
    if (verbose) puts("STUB: vDSP_vclr called");
    return NULL;
}

void* vDSP_vclrD(void)
{
    if (verbose) puts("STUB: vDSP_vclrD called");
    return NULL;
}

void* vDSP_vcmprs(void)
{
    if (verbose) puts("STUB: vDSP_vcmprs called");
    return NULL;
}

void* vDSP_vcmprsD(void)
{
    if (verbose) puts("STUB: vDSP_vcmprsD called");
    return NULL;
}

void* vDSP_vdbcon(void)
{
    if (verbose) puts("STUB: vDSP_vdbcon called");
    return NULL;
}

void* vDSP_vdbconD(void)
{
    if (verbose) puts("STUB: vDSP_vdbconD called");
    return NULL;
}

void* vDSP_vdist(void)
{
    if (verbose) puts("STUB: vDSP_vdist called");
    return NULL;
}

void* vDSP_vdistD(void)
{
    if (verbose) puts("STUB: vDSP_vdistD called");
    return NULL;
}

void* vDSP_vdiv(void)
{
    if (verbose) puts("STUB: vDSP_vdiv called");
    return NULL;
}

void* vDSP_vdivD(void)
{
    if (verbose) puts("STUB: vDSP_vdivD called");
    return NULL;
}

void* vDSP_vdivi(void)
{
    if (verbose) puts("STUB: vDSP_vdivi called");
    return NULL;
}

void* vDSP_vdpsp(void)
{
    if (verbose) puts("STUB: vDSP_vdpsp called");
    return NULL;
}

void* vDSP_venvlp(void)
{
    if (verbose) puts("STUB: vDSP_venvlp called");
    return NULL;
}

void* vDSP_venvlpD(void)
{
    if (verbose) puts("STUB: vDSP_venvlpD called");
    return NULL;
}

void* vDSP_veqvi(void)
{
    if (verbose) puts("STUB: vDSP_veqvi called");
    return NULL;
}

void* vDSP_vfill(void)
{
    if (verbose) puts("STUB: vDSP_vfill called");
    return NULL;
}

void* vDSP_vfillD(void)
{
    if (verbose) puts("STUB: vDSP_vfillD called");
    return NULL;
}

void* vDSP_vfilli(void)
{
    if (verbose) puts("STUB: vDSP_vfilli called");
    return NULL;
}

void* vDSP_vfix16(void)
{
    if (verbose) puts("STUB: vDSP_vfix16 called");
    return NULL;
}

void* vDSP_vfix16D(void)
{
    if (verbose) puts("STUB: vDSP_vfix16D called");
    return NULL;
}

void* vDSP_vfix32(void)
{
    if (verbose) puts("STUB: vDSP_vfix32 called");
    return NULL;
}

void* vDSP_vfix32D(void)
{
    if (verbose) puts("STUB: vDSP_vfix32D called");
    return NULL;
}

void* vDSP_vfix8(void)
{
    if (verbose) puts("STUB: vDSP_vfix8 called");
    return NULL;
}

void* vDSP_vfix8D(void)
{
    if (verbose) puts("STUB: vDSP_vfix8D called");
    return NULL;
}

void* vDSP_vfixr16(void)
{
    if (verbose) puts("STUB: vDSP_vfixr16 called");
    return NULL;
}

void* vDSP_vfixr16D(void)
{
    if (verbose) puts("STUB: vDSP_vfixr16D called");
    return NULL;
}

void* vDSP_vfixr32(void)
{
    if (verbose) puts("STUB: vDSP_vfixr32 called");
    return NULL;
}

void* vDSP_vfixr32D(void)
{
    if (verbose) puts("STUB: vDSP_vfixr32D called");
    return NULL;
}

void* vDSP_vfixr8(void)
{
    if (verbose) puts("STUB: vDSP_vfixr8 called");
    return NULL;
}

void* vDSP_vfixr8D(void)
{
    if (verbose) puts("STUB: vDSP_vfixr8D called");
    return NULL;
}

void* vDSP_vfixru16(void)
{
    if (verbose) puts("STUB: vDSP_vfixru16 called");
    return NULL;
}

void* vDSP_vfixru16D(void)
{
    if (verbose) puts("STUB: vDSP_vfixru16D called");
    return NULL;
}

void* vDSP_vfixru32(void)
{
    if (verbose) puts("STUB: vDSP_vfixru32 called");
    return NULL;
}

void* vDSP_vfixru32D(void)
{
    if (verbose) puts("STUB: vDSP_vfixru32D called");
    return NULL;
}

void* vDSP_vfixru8(void)
{
    if (verbose) puts("STUB: vDSP_vfixru8 called");
    return NULL;
}

void* vDSP_vfixru8D(void)
{
    if (verbose) puts("STUB: vDSP_vfixru8D called");
    return NULL;
}

void* vDSP_vfixu16(void)
{
    if (verbose) puts("STUB: vDSP_vfixu16 called");
    return NULL;
}

void* vDSP_vfixu16D(void)
{
    if (verbose) puts("STUB: vDSP_vfixu16D called");
    return NULL;
}

void* vDSP_vfixu32(void)
{
    if (verbose) puts("STUB: vDSP_vfixu32 called");
    return NULL;
}

void* vDSP_vfixu32D(void)
{
    if (verbose) puts("STUB: vDSP_vfixu32D called");
    return NULL;
}

void* vDSP_vfixu8(void)
{
    if (verbose) puts("STUB: vDSP_vfixu8 called");
    return NULL;
}

void* vDSP_vfixu8D(void)
{
    if (verbose) puts("STUB: vDSP_vfixu8D called");
    return NULL;
}

void* vDSP_vflt16(void)
{
    if (verbose) puts("STUB: vDSP_vflt16 called");
    return NULL;
}

void* vDSP_vflt16D(void)
{
    if (verbose) puts("STUB: vDSP_vflt16D called");
    return NULL;
}

void* vDSP_vflt24(void)
{
    if (verbose) puts("STUB: vDSP_vflt24 called");
    return NULL;
}

void* vDSP_vflt32(void)
{
    if (verbose) puts("STUB: vDSP_vflt32 called");
    return NULL;
}

void* vDSP_vflt32D(void)
{
    if (verbose) puts("STUB: vDSP_vflt32D called");
    return NULL;
}

void* vDSP_vflt8(void)
{
    if (verbose) puts("STUB: vDSP_vflt8 called");
    return NULL;
}

void* vDSP_vflt8D(void)
{
    if (verbose) puts("STUB: vDSP_vflt8D called");
    return NULL;
}

void* vDSP_vfltsm24(void)
{
    if (verbose) puts("STUB: vDSP_vfltsm24 called");
    return NULL;
}

void* vDSP_vfltsmu24(void)
{
    if (verbose) puts("STUB: vDSP_vfltsmu24 called");
    return NULL;
}

void* vDSP_vfltu16(void)
{
    if (verbose) puts("STUB: vDSP_vfltu16 called");
    return NULL;
}

void* vDSP_vfltu16D(void)
{
    if (verbose) puts("STUB: vDSP_vfltu16D called");
    return NULL;
}

void* vDSP_vfltu24(void)
{
    if (verbose) puts("STUB: vDSP_vfltu24 called");
    return NULL;
}

void* vDSP_vfltu32(void)
{
    if (verbose) puts("STUB: vDSP_vfltu32 called");
    return NULL;
}

void* vDSP_vfltu32D(void)
{
    if (verbose) puts("STUB: vDSP_vfltu32D called");
    return NULL;
}

void* vDSP_vfltu8(void)
{
    if (verbose) puts("STUB: vDSP_vfltu8 called");
    return NULL;
}

void* vDSP_vfltu8D(void)
{
    if (verbose) puts("STUB: vDSP_vfltu8D called");
    return NULL;
}

void* vDSP_vfrac(void)
{
    if (verbose) puts("STUB: vDSP_vfrac called");
    return NULL;
}

void* vDSP_vfracD(void)
{
    if (verbose) puts("STUB: vDSP_vfracD called");
    return NULL;
}

void* vDSP_vgathr(void)
{
    if (verbose) puts("STUB: vDSP_vgathr called");
    return NULL;
}

void* vDSP_vgathrD(void)
{
    if (verbose) puts("STUB: vDSP_vgathrD called");
    return NULL;
}

void* vDSP_vgathra(void)
{
    if (verbose) puts("STUB: vDSP_vgathra called");
    return NULL;
}

void* vDSP_vgathraD(void)
{
    if (verbose) puts("STUB: vDSP_vgathraD called");
    return NULL;
}

void* vDSP_vgen(void)
{
    if (verbose) puts("STUB: vDSP_vgen called");
    return NULL;
}

void* vDSP_vgenD(void)
{
    if (verbose) puts("STUB: vDSP_vgenD called");
    return NULL;
}

void* vDSP_vgenp(void)
{
    if (verbose) puts("STUB: vDSP_vgenp called");
    return NULL;
}

void* vDSP_vgenpD(void)
{
    if (verbose) puts("STUB: vDSP_vgenpD called");
    return NULL;
}

void* vDSP_viclip(void)
{
    if (verbose) puts("STUB: vDSP_viclip called");
    return NULL;
}

void* vDSP_viclipD(void)
{
    if (verbose) puts("STUB: vDSP_viclipD called");
    return NULL;
}

void* vDSP_vindex(void)
{
    if (verbose) puts("STUB: vDSP_vindex called");
    return NULL;
}

void* vDSP_vindexD(void)
{
    if (verbose) puts("STUB: vDSP_vindexD called");
    return NULL;
}

void* vDSP_vintb(void)
{
    if (verbose) puts("STUB: vDSP_vintb called");
    return NULL;
}

void* vDSP_vintbD(void)
{
    if (verbose) puts("STUB: vDSP_vintbD called");
    return NULL;
}

void* vDSP_vlim(void)
{
    if (verbose) puts("STUB: vDSP_vlim called");
    return NULL;
}

void* vDSP_vlimD(void)
{
    if (verbose) puts("STUB: vDSP_vlimD called");
    return NULL;
}

void* vDSP_vlint(void)
{
    if (verbose) puts("STUB: vDSP_vlint called");
    return NULL;
}

void* vDSP_vlintD(void)
{
    if (verbose) puts("STUB: vDSP_vlintD called");
    return NULL;
}

void* vDSP_vma(void)
{
    if (verbose) puts("STUB: vDSP_vma called");
    return NULL;
}

void* vDSP_vmaD(void)
{
    if (verbose) puts("STUB: vDSP_vmaD called");
    return NULL;
}

void* vDSP_vmax(void)
{
    if (verbose) puts("STUB: vDSP_vmax called");
    return NULL;
}

void* vDSP_vmaxD(void)
{
    if (verbose) puts("STUB: vDSP_vmaxD called");
    return NULL;
}

void* vDSP_vmaxmg(void)
{
    if (verbose) puts("STUB: vDSP_vmaxmg called");
    return NULL;
}

void* vDSP_vmaxmgD(void)
{
    if (verbose) puts("STUB: vDSP_vmaxmgD called");
    return NULL;
}

void* vDSP_vmin(void)
{
    if (verbose) puts("STUB: vDSP_vmin called");
    return NULL;
}

void* vDSP_vminD(void)
{
    if (verbose) puts("STUB: vDSP_vminD called");
    return NULL;
}

void* vDSP_vminmg(void)
{
    if (verbose) puts("STUB: vDSP_vminmg called");
    return NULL;
}

void* vDSP_vminmgD(void)
{
    if (verbose) puts("STUB: vDSP_vminmgD called");
    return NULL;
}

void* vDSP_vmma(void)
{
    if (verbose) puts("STUB: vDSP_vmma called");
    return NULL;
}

void* vDSP_vmmaD(void)
{
    if (verbose) puts("STUB: vDSP_vmmaD called");
    return NULL;
}

void* vDSP_vmmsb(void)
{
    if (verbose) puts("STUB: vDSP_vmmsb called");
    return NULL;
}

void* vDSP_vmmsbD(void)
{
    if (verbose) puts("STUB: vDSP_vmmsbD called");
    return NULL;
}

void* vDSP_vmsa(void)
{
    if (verbose) puts("STUB: vDSP_vmsa called");
    return NULL;
}

void* vDSP_vmsaD(void)
{
    if (verbose) puts("STUB: vDSP_vmsaD called");
    return NULL;
}

void* vDSP_vmsb(void)
{
    if (verbose) puts("STUB: vDSP_vmsb called");
    return NULL;
}

void* vDSP_vmsbD(void)
{
    if (verbose) puts("STUB: vDSP_vmsbD called");
    return NULL;
}

void* vDSP_vmul(void)
{
    if (verbose) puts("STUB: vDSP_vmul called");
    return NULL;
}

void* vDSP_vmulD(void)
{
    if (verbose) puts("STUB: vDSP_vmulD called");
    return NULL;
}

void* vDSP_vnabs(void)
{
    if (verbose) puts("STUB: vDSP_vnabs called");
    return NULL;
}

void* vDSP_vnabsD(void)
{
    if (verbose) puts("STUB: vDSP_vnabsD called");
    return NULL;
}

void* vDSP_vneg(void)
{
    if (verbose) puts("STUB: vDSP_vneg called");
    return NULL;
}

void* vDSP_vnegD(void)
{
    if (verbose) puts("STUB: vDSP_vnegD called");
    return NULL;
}

void* vDSP_vpoly(void)
{
    if (verbose) puts("STUB: vDSP_vpoly called");
    return NULL;
}

void* vDSP_vpolyD(void)
{
    if (verbose) puts("STUB: vDSP_vpolyD called");
    return NULL;
}

void* vDSP_vpythg(void)
{
    if (verbose) puts("STUB: vDSP_vpythg called");
    return NULL;
}

void* vDSP_vpythgD(void)
{
    if (verbose) puts("STUB: vDSP_vpythgD called");
    return NULL;
}

void* vDSP_vqint(void)
{
    if (verbose) puts("STUB: vDSP_vqint called");
    return NULL;
}

void* vDSP_vqintD(void)
{
    if (verbose) puts("STUB: vDSP_vqintD called");
    return NULL;
}

void* vDSP_vramp(void)
{
    if (verbose) puts("STUB: vDSP_vramp called");
    return NULL;
}

void* vDSP_vrampD(void)
{
    if (verbose) puts("STUB: vDSP_vrampD called");
    return NULL;
}

void* vDSP_vrampmul(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul called");
    return NULL;
}

void* vDSP_vrampmul2(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul2 called");
    return NULL;
}

void* vDSP_vrampmul2D(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul2D called");
    return NULL;
}

void* vDSP_vrampmul2_s1_15(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul2_s1_15 called");
    return NULL;
}

void* vDSP_vrampmul2_s8_24(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul2_s8_24 called");
    return NULL;
}

void* vDSP_vrampmulD(void)
{
    if (verbose) puts("STUB: vDSP_vrampmulD called");
    return NULL;
}

void* vDSP_vrampmul_s1_15(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul_s1_15 called");
    return NULL;
}

void* vDSP_vrampmul_s8_24(void)
{
    if (verbose) puts("STUB: vDSP_vrampmul_s8_24 called");
    return NULL;
}

void* vDSP_vrampmuladd(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd called");
    return NULL;
}

void* vDSP_vrampmuladd2(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd2 called");
    return NULL;
}

void* vDSP_vrampmuladd2D(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd2D called");
    return NULL;
}

void* vDSP_vrampmuladd2_s1_15(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd2_s1_15 called");
    return NULL;
}

void* vDSP_vrampmuladd2_s8_24(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd2_s8_24 called");
    return NULL;
}

void* vDSP_vrampmuladdD(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladdD called");
    return NULL;
}

void* vDSP_vrampmuladd_s1_15(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd_s1_15 called");
    return NULL;
}

void* vDSP_vrampmuladd_s8_24(void)
{
    if (verbose) puts("STUB: vDSP_vrampmuladd_s8_24 called");
    return NULL;
}

void* vDSP_vrsum(void)
{
    if (verbose) puts("STUB: vDSP_vrsum called");
    return NULL;
}

void* vDSP_vrsumD(void)
{
    if (verbose) puts("STUB: vDSP_vrsumD called");
    return NULL;
}

void* vDSP_vrvrs(void)
{
    if (verbose) puts("STUB: vDSP_vrvrs called");
    return NULL;
}

void* vDSP_vrvrsD(void)
{
    if (verbose) puts("STUB: vDSP_vrvrsD called");
    return NULL;
}

void* vDSP_vsadd(void)
{
    if (verbose) puts("STUB: vDSP_vsadd called");
    return NULL;
}

void* vDSP_vsaddD(void)
{
    if (verbose) puts("STUB: vDSP_vsaddD called");
    return NULL;
}

void* vDSP_vsaddi(void)
{
    if (verbose) puts("STUB: vDSP_vsaddi called");
    return NULL;
}

void* vDSP_vsbm(void)
{
    if (verbose) puts("STUB: vDSP_vsbm called");
    return NULL;
}

void* vDSP_vsbmD(void)
{
    if (verbose) puts("STUB: vDSP_vsbmD called");
    return NULL;
}

void* vDSP_vsbsbm(void)
{
    if (verbose) puts("STUB: vDSP_vsbsbm called");
    return NULL;
}

void* vDSP_vsbsbmD(void)
{
    if (verbose) puts("STUB: vDSP_vsbsbmD called");
    return NULL;
}

void* vDSP_vsbsm(void)
{
    if (verbose) puts("STUB: vDSP_vsbsm called");
    return NULL;
}

void* vDSP_vsbsmD(void)
{
    if (verbose) puts("STUB: vDSP_vsbsmD called");
    return NULL;
}

void* vDSP_vsdiv(void)
{
    if (verbose) puts("STUB: vDSP_vsdiv called");
    return NULL;
}

void* vDSP_vsdivD(void)
{
    if (verbose) puts("STUB: vDSP_vsdivD called");
    return NULL;
}

void* vDSP_vsdivi(void)
{
    if (verbose) puts("STUB: vDSP_vsdivi called");
    return NULL;
}

void* vDSP_vsimps(void)
{
    if (verbose) puts("STUB: vDSP_vsimps called");
    return NULL;
}

void* vDSP_vsimpsD(void)
{
    if (verbose) puts("STUB: vDSP_vsimpsD called");
    return NULL;
}

void* vDSP_vsma(void)
{
    if (verbose) puts("STUB: vDSP_vsma called");
    return NULL;
}

void* vDSP_vsmaD(void)
{
    if (verbose) puts("STUB: vDSP_vsmaD called");
    return NULL;
}

void* vDSP_vsmfix24(void)
{
    if (verbose) puts("STUB: vDSP_vsmfix24 called");
    return NULL;
}

void* vDSP_vsmfixu24(void)
{
    if (verbose) puts("STUB: vDSP_vsmfixu24 called");
    return NULL;
}

void* vDSP_vsmsa(void)
{
    if (verbose) puts("STUB: vDSP_vsmsa called");
    return NULL;
}

void* vDSP_vsmsaD(void)
{
    if (verbose) puts("STUB: vDSP_vsmsaD called");
    return NULL;
}

void* vDSP_vsmsb(void)
{
    if (verbose) puts("STUB: vDSP_vsmsb called");
    return NULL;
}

void* vDSP_vsmsbD(void)
{
    if (verbose) puts("STUB: vDSP_vsmsbD called");
    return NULL;
}

void* vDSP_vsmsma(void)
{
    if (verbose) puts("STUB: vDSP_vsmsma called");
    return NULL;
}

void* vDSP_vsmsmaD(void)
{
    if (verbose) puts("STUB: vDSP_vsmsmaD called");
    return NULL;
}

void* vDSP_vsmul(void)
{
    if (verbose) puts("STUB: vDSP_vsmul called");
    return NULL;
}

void* vDSP_vsmulD(void)
{
    if (verbose) puts("STUB: vDSP_vsmulD called");
    return NULL;
}

void* vDSP_vsort(void)
{
    if (verbose) puts("STUB: vDSP_vsort called");
    return NULL;
}

void* vDSP_vsortD(void)
{
    if (verbose) puts("STUB: vDSP_vsortD called");
    return NULL;
}

void* vDSP_vsorti(void)
{
    if (verbose) puts("STUB: vDSP_vsorti called");
    return NULL;
}

void* vDSP_vsortiD(void)
{
    if (verbose) puts("STUB: vDSP_vsortiD called");
    return NULL;
}

void* vDSP_vspdp(void)
{
    if (verbose) puts("STUB: vDSP_vspdp called");
    return NULL;
}

void* vDSP_vsq(void)
{
    if (verbose) puts("STUB: vDSP_vsq called");
    return NULL;
}

void* vDSP_vsqD(void)
{
    if (verbose) puts("STUB: vDSP_vsqD called");
    return NULL;
}

void* vDSP_vssq(void)
{
    if (verbose) puts("STUB: vDSP_vssq called");
    return NULL;
}

void* vDSP_vssqD(void)
{
    if (verbose) puts("STUB: vDSP_vssqD called");
    return NULL;
}

void* vDSP_vsub(void)
{
    if (verbose) puts("STUB: vDSP_vsub called");
    return NULL;
}

void* vDSP_vsubD(void)
{
    if (verbose) puts("STUB: vDSP_vsubD called");
    return NULL;
}

void* vDSP_vswap(void)
{
    if (verbose) puts("STUB: vDSP_vswap called");
    return NULL;
}

void* vDSP_vswapD(void)
{
    if (verbose) puts("STUB: vDSP_vswapD called");
    return NULL;
}

void* vDSP_vswmax(void)
{
    if (verbose) puts("STUB: vDSP_vswmax called");
    return NULL;
}

void* vDSP_vswmaxD(void)
{
    if (verbose) puts("STUB: vDSP_vswmaxD called");
    return NULL;
}

void* vDSP_vswsum(void)
{
    if (verbose) puts("STUB: vDSP_vswsum called");
    return NULL;
}

void* vDSP_vswsumD(void)
{
    if (verbose) puts("STUB: vDSP_vswsumD called");
    return NULL;
}

void* vDSP_vtabi(void)
{
    if (verbose) puts("STUB: vDSP_vtabi called");
    return NULL;
}

void* vDSP_vtabiD(void)
{
    if (verbose) puts("STUB: vDSP_vtabiD called");
    return NULL;
}

void* vDSP_vthr(void)
{
    if (verbose) puts("STUB: vDSP_vthr called");
    return NULL;
}

void* vDSP_vthrD(void)
{
    if (verbose) puts("STUB: vDSP_vthrD called");
    return NULL;
}

void* vDSP_vthres(void)
{
    if (verbose) puts("STUB: vDSP_vthres called");
    return NULL;
}

void* vDSP_vthresD(void)
{
    if (verbose) puts("STUB: vDSP_vthresD called");
    return NULL;
}

void* vDSP_vthrsc(void)
{
    if (verbose) puts("STUB: vDSP_vthrsc called");
    return NULL;
}

void* vDSP_vthrscD(void)
{
    if (verbose) puts("STUB: vDSP_vthrscD called");
    return NULL;
}

void* vDSP_vtmerg(void)
{
    if (verbose) puts("STUB: vDSP_vtmerg called");
    return NULL;
}

void* vDSP_vtmergD(void)
{
    if (verbose) puts("STUB: vDSP_vtmergD called");
    return NULL;
}

void* vDSP_vtrapz(void)
{
    if (verbose) puts("STUB: vDSP_vtrapz called");
    return NULL;
}

void* vDSP_vtrapzD(void)
{
    if (verbose) puts("STUB: vDSP_vtrapzD called");
    return NULL;
}

void* vDSP_wiener(void)
{
    if (verbose) puts("STUB: vDSP_wiener called");
    return NULL;
}

void* vDSP_wienerD(void)
{
    if (verbose) puts("STUB: vDSP_wienerD called");
    return NULL;
}

void* vDSP_zaspec(void)
{
    if (verbose) puts("STUB: vDSP_zaspec called");
    return NULL;
}

void* vDSP_zaspecD(void)
{
    if (verbose) puts("STUB: vDSP_zaspecD called");
    return NULL;
}

void* vDSP_zcoher(void)
{
    if (verbose) puts("STUB: vDSP_zcoher called");
    return NULL;
}

void* vDSP_zcoherD(void)
{
    if (verbose) puts("STUB: vDSP_zcoherD called");
    return NULL;
}

void* vDSP_zconv(void)
{
    if (verbose) puts("STUB: vDSP_zconv called");
    return NULL;
}

void* vDSP_zconvD(void)
{
    if (verbose) puts("STUB: vDSP_zconvD called");
    return NULL;
}

void* vDSP_zcspec(void)
{
    if (verbose) puts("STUB: vDSP_zcspec called");
    return NULL;
}

void* vDSP_zcspecD(void)
{
    if (verbose) puts("STUB: vDSP_zcspecD called");
    return NULL;
}

void* vDSP_zdotpr(void)
{
    if (verbose) puts("STUB: vDSP_zdotpr called");
    return NULL;
}

void* vDSP_zdotprD(void)
{
    if (verbose) puts("STUB: vDSP_zdotprD called");
    return NULL;
}

void* vDSP_zidotpr(void)
{
    if (verbose) puts("STUB: vDSP_zidotpr called");
    return NULL;
}

void* vDSP_zidotprD(void)
{
    if (verbose) puts("STUB: vDSP_zidotprD called");
    return NULL;
}

void* vDSP_zmma(void)
{
    if (verbose) puts("STUB: vDSP_zmma called");
    return NULL;
}

void* vDSP_zmmaD(void)
{
    if (verbose) puts("STUB: vDSP_zmmaD called");
    return NULL;
}

void* vDSP_zmms(void)
{
    if (verbose) puts("STUB: vDSP_zmms called");
    return NULL;
}

void* vDSP_zmmsD(void)
{
    if (verbose) puts("STUB: vDSP_zmmsD called");
    return NULL;
}

void* vDSP_zmmul(void)
{
    if (verbose) puts("STUB: vDSP_zmmul called");
    return NULL;
}

void* vDSP_zmmulD(void)
{
    if (verbose) puts("STUB: vDSP_zmmulD called");
    return NULL;
}

void* vDSP_zmsm(void)
{
    if (verbose) puts("STUB: vDSP_zmsm called");
    return NULL;
}

void* vDSP_zmsmD(void)
{
    if (verbose) puts("STUB: vDSP_zmsmD called");
    return NULL;
}

void* vDSP_zrdesamp(void)
{
    if (verbose) puts("STUB: vDSP_zrdesamp called");
    return NULL;
}

void* vDSP_zrdesampD(void)
{
    if (verbose) puts("STUB: vDSP_zrdesampD called");
    return NULL;
}

void* vDSP_zrdotpr(void)
{
    if (verbose) puts("STUB: vDSP_zrdotpr called");
    return NULL;
}

void* vDSP_zrdotprD(void)
{
    if (verbose) puts("STUB: vDSP_zrdotprD called");
    return NULL;
}

void* vDSP_zrvadd(void)
{
    if (verbose) puts("STUB: vDSP_zrvadd called");
    return NULL;
}

void* vDSP_zrvaddD(void)
{
    if (verbose) puts("STUB: vDSP_zrvaddD called");
    return NULL;
}

void* vDSP_zrvdiv(void)
{
    if (verbose) puts("STUB: vDSP_zrvdiv called");
    return NULL;
}

void* vDSP_zrvdivD(void)
{
    if (verbose) puts("STUB: vDSP_zrvdivD called");
    return NULL;
}

void* vDSP_zrvmul(void)
{
    if (verbose) puts("STUB: vDSP_zrvmul called");
    return NULL;
}

void* vDSP_zrvmulD(void)
{
    if (verbose) puts("STUB: vDSP_zrvmulD called");
    return NULL;
}

void* vDSP_zrvsub(void)
{
    if (verbose) puts("STUB: vDSP_zrvsub called");
    return NULL;
}

void* vDSP_zrvsubD(void)
{
    if (verbose) puts("STUB: vDSP_zrvsubD called");
    return NULL;
}

void* vDSP_ztoc(void)
{
    if (verbose) puts("STUB: vDSP_ztoc called");
    return NULL;
}

void* vDSP_ztocD(void)
{
    if (verbose) puts("STUB: vDSP_ztocD called");
    return NULL;
}

void* vDSP_ztrans(void)
{
    if (verbose) puts("STUB: vDSP_ztrans called");
    return NULL;
}

void* vDSP_ztransD(void)
{
    if (verbose) puts("STUB: vDSP_ztransD called");
    return NULL;
}

void* vDSP_zvabs(void)
{
    if (verbose) puts("STUB: vDSP_zvabs called");
    return NULL;
}

void* vDSP_zvabsD(void)
{
    if (verbose) puts("STUB: vDSP_zvabsD called");
    return NULL;
}

void* vDSP_zvadd(void)
{
    if (verbose) puts("STUB: vDSP_zvadd called");
    return NULL;
}

void* vDSP_zvaddD(void)
{
    if (verbose) puts("STUB: vDSP_zvaddD called");
    return NULL;
}

void* vDSP_zvcma(void)
{
    if (verbose) puts("STUB: vDSP_zvcma called");
    return NULL;
}

void* vDSP_zvcmaD(void)
{
    if (verbose) puts("STUB: vDSP_zvcmaD called");
    return NULL;
}

void* vDSP_zvcmul(void)
{
    if (verbose) puts("STUB: vDSP_zvcmul called");
    return NULL;
}

void* vDSP_zvcmulD(void)
{
    if (verbose) puts("STUB: vDSP_zvcmulD called");
    return NULL;
}

void* vDSP_zvconj(void)
{
    if (verbose) puts("STUB: vDSP_zvconj called");
    return NULL;
}

void* vDSP_zvconjD(void)
{
    if (verbose) puts("STUB: vDSP_zvconjD called");
    return NULL;
}

void* vDSP_zvdiv(void)
{
    if (verbose) puts("STUB: vDSP_zvdiv called");
    return NULL;
}

void* vDSP_zvdivD(void)
{
    if (verbose) puts("STUB: vDSP_zvdivD called");
    return NULL;
}

void* vDSP_zvfill(void)
{
    if (verbose) puts("STUB: vDSP_zvfill called");
    return NULL;
}

void* vDSP_zvfillD(void)
{
    if (verbose) puts("STUB: vDSP_zvfillD called");
    return NULL;
}

void* vDSP_zvma(void)
{
    if (verbose) puts("STUB: vDSP_zvma called");
    return NULL;
}

void* vDSP_zvmaD(void)
{
    if (verbose) puts("STUB: vDSP_zvmaD called");
    return NULL;
}

void* vDSP_zvmags(void)
{
    if (verbose) puts("STUB: vDSP_zvmags called");
    return NULL;
}

void* vDSP_zvmagsD(void)
{
    if (verbose) puts("STUB: vDSP_zvmagsD called");
    return NULL;
}

void* vDSP_zvmgsa(void)
{
    if (verbose) puts("STUB: vDSP_zvmgsa called");
    return NULL;
}

void* vDSP_zvmgsaD(void)
{
    if (verbose) puts("STUB: vDSP_zvmgsaD called");
    return NULL;
}

void* vDSP_zvmmaa(void)
{
    if (verbose) puts("STUB: vDSP_zvmmaa called");
    return NULL;
}

void* vDSP_zvmmaaD(void)
{
    if (verbose) puts("STUB: vDSP_zvmmaaD called");
    return NULL;
}

void* vDSP_zvmov(void)
{
    if (verbose) puts("STUB: vDSP_zvmov called");
    return NULL;
}

void* vDSP_zvmovD(void)
{
    if (verbose) puts("STUB: vDSP_zvmovD called");
    return NULL;
}

void* vDSP_zvmul(void)
{
    if (verbose) puts("STUB: vDSP_zvmul called");
    return NULL;
}

void* vDSP_zvmulD(void)
{
    if (verbose) puts("STUB: vDSP_zvmulD called");
    return NULL;
}

void* vDSP_zvneg(void)
{
    if (verbose) puts("STUB: vDSP_zvneg called");
    return NULL;
}

void* vDSP_zvnegD(void)
{
    if (verbose) puts("STUB: vDSP_zvnegD called");
    return NULL;
}

void* vDSP_zvphas(void)
{
    if (verbose) puts("STUB: vDSP_zvphas called");
    return NULL;
}

void* vDSP_zvphasD(void)
{
    if (verbose) puts("STUB: vDSP_zvphasD called");
    return NULL;
}

void* vDSP_zvsma(void)
{
    if (verbose) puts("STUB: vDSP_zvsma called");
    return NULL;
}

void* vDSP_zvsmaD(void)
{
    if (verbose) puts("STUB: vDSP_zvsmaD called");
    return NULL;
}

void* vDSP_zvsub(void)
{
    if (verbose) puts("STUB: vDSP_zvsub called");
    return NULL;
}

void* vDSP_zvsubD(void)
{
    if (verbose) puts("STUB: vDSP_zvsubD called");
    return NULL;
}

void* vDSP_zvzsml(void)
{
    if (verbose) puts("STUB: vDSP_zvzsml called");
    return NULL;
}

void* vDSP_zvzsmlD(void)
{
    if (verbose) puts("STUB: vDSP_zvzsmlD called");
    return NULL;
}
