/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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


#include <ShazamKit/ShazamKit.h>
#include <stdlib.h>
#include <stdio.h>

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern const void* ShazamKitVersionString = (void*)0;
extern const void* ShazamKitVersionNumber = (void*)0;
extern const void* SHSignatureMinimumSeconds = (void*)0;
extern const void* SHCustomCatalogMinimumQuerySignatureDuration = (void*)0;
extern const void* _ZL6_chanw = (void*)0;
extern const void* _ZL19_penalty_weightings = (void*)0;
extern const void* _ZTS7OSTimer = (void*)0;
extern const void* _ZTS5Timer = (void*)0;
extern const void* _ZN6shazamL21thresholdCurve_4bandsE = (void*)0;
extern const void* _ZN6shazamL14thresholdCurveE = (void*)0;
extern const void* _ZZN6shazam4algoIL8AlgoType1EE10matrix_forE12QueryDensityE5query = (void*)0;
extern const void* _ZZN6shazam4algoIL8AlgoType1EE10matrix_forE9DBDensityE2db = (void*)0;
extern const void* _ZTSN6shazam24StaticCPUSearcherFactoryE = (void*)0;
extern const void* _ZTSN6shazam15SearcherFactoryE = (void*)0;
extern const void* _ZN12_GLOBAL__N_16srkeysE = (void*)0;
extern const void* _ZTSN12_GLOBAL__N_123signature_category_implE = (void*)0;
extern const void* _ZL19HANNING_WINDOW_2048 = (void*)0;
extern const void* _ZTSNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEE = (void*)0;
extern const void* _ZTSZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC1ERKNS_21packed_signature_viewEEUlNS_16signature_headerEE_ = (void*)0;
extern const void* _ZTSNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEE = (void*)0;
extern const void* _ZTSZN6shazam15basic_signatureILNS_17signature_densityE24EEC1ERKNS_21packed_signature_viewEEUlNS_16signature_headerEE_ = (void*)0;
extern const void* _ZTSNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEE = (void*)0;
extern const void* _ZTSZN6shazam15basic_signatureILNS_17signature_densityE16EEC1ERKNS_21packed_signature_viewEEUlNS_16signature_headerEE_ = (void*)0;
extern const void* decimation_3_1_tight_table = (void*)0;
extern const void* decimation_2_1_loose_table = (void*)0;
extern const void* decimation_2_1_tight_table = (void*)0;
extern const void* polyphase_coeffs_441_320 = (void*)0;
extern const void* _ZTSNSt3__110__function6__baseIFvN6shazam16signature_headerEEEE = (void*)0;
extern const void* _ZTSN6shazam17StaticCPUSearcherE = (void*)0;
extern const void* _ZTS8Searcher = (void*)0;
extern const void* _ZTSNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEEE = (void*)0;
extern const void* _ZTSNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEEE = (void*)0;
extern const void* _ZTSNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEEE = (void*)0;
extern const void* _ZTS18sig_extractor_pipeI15spectral_peak_tE = (void*)0;
extern const void* _ZTS14sigx_pipe_base = (void*)0;
extern const void* _ZL15FREQ_BAND_EDGES = (void*)0;
extern const void* _ZN12_GLOBAL__N_16srkeysE = (void*)0;
extern const void* _ZTS24ConstQueryConfigProvider = (void*)0;
extern const void* _ZTS19QueryConfigProvider = (void*)0;
extern const void* _ZTSNSt3__110__function6__baseIFvRK14SearchResponseEEE = (void*)0;
extern const void* _ZTS24FFT_implementation_apple = (void*)0;
extern const void* _ZTS18FFT_implementation = (void*)0;

void *_ZN6shazam10fit_pointsERKNS_16CPUOnePtDatabaseERKNSt3__16vectorI7peak_tfNS3_9allocatorIS5_EEEERK9SkewRangeyfRfRNS3_6chrono8durationIdNS3_5ratioILl1ELl125EEEEERNS4_INS_7tfmatchENS6_ISL_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam10fit_pointsERKNS_16CPUOnePtDatabaseERKNSt3__16vectorI7peak_tfNS3_9allocatorIS5_EEEERK9SkewRangeyfRfRNS3_6chrono8durationIdNS3_5ratioILl1ELl125EEEEERNS4_INS_7tfmatchENS6_ISL_EEEE called");
    return NULL;
)

void *_ZN6shazam7regressIKNS_7tfmatchENS_8extractXIS2_EENS_8extractYIS2_EEEENSt3__18optionalINS_12regress_lineEEERKN3gsl4spanIT_Lln1EEET0_T1_(void) {
    if (verbose) puts("STUB: _ZN6shazam7regressIKNS_7tfmatchENS_8extractXIS2_EENS_8extractYIS2_EEEENSt3__18optionalINS_12regress_lineEEERKN3gsl4spanIT_Lln1EEET0_T1_ called");
    return NULL;
)

void *_ZN3gsl4spanIKN6shazam7tfmatchELln1EEC1INSt3__16vectorIS2_NS6_9allocatorIS2_EEEEvEERKT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIKN6shazam7tfmatchELln1EEC1INSt3__16vectorIS2_NS6_9allocatorIS2_EEEEvEERKT_ called");
    return NULL;
)

void *_ZN3gsl4spanIKN6shazam7tfmatchELln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS3_T_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIKN6shazam7tfmatchELln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS3_T_ called");
    return NULL;
)

void *_Z11crc32_checkjPKcm(void) {
    if (verbose) puts("STUB: _Z11crc32_checkjPKcm called");
    return NULL;
)

void *_Z8checksumPKvm(void) {
    if (verbose) puts("STUB: _Z8checksumPKvm called");
    return NULL;
)

void *_ZN6shazam24get_spect_peak_log_scaleEv(void) {
    if (verbose) puts("STUB: _ZN6shazam24get_spect_peak_log_scaleEv called");
    return NULL;
)

void *_ZN6shazam25get_spect_peak_log_offsetEv(void) {
    if (verbose) puts("STUB: _ZN6shazam25get_spect_peak_log_offsetEv called");
    return NULL;
)

void *_Z11InterpolateR15spectral_peak_tjPKf(void) {
    if (verbose) puts("STUB: _Z11InterpolateR15spectral_peak_tjPKf called");
    return NULL;
)

void *_ZN15spectral_peak_tC1ENSt3__16chrono8durationIjNS0_5ratioILl1ELl125EEEEEjPKf(void) {
    if (verbose) puts("STUB: _ZN15spectral_peak_tC1ENSt3__16chrono8durationIjNS0_5ratioILl1ELl125EEEEEjPKf called");
    return NULL;
)

void *_ZN6shazam23ShazamKitHeaderIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam23ShazamKitHeaderIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZN6shazam11score_matchERKNSt3__16vectorINS_7tfmatchENS0_9allocatorIS2_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam11score_matchERKNSt3__16vectorINS_7tfmatchENS0_9allocatorIS2_EEEE called");
    return NULL;
)

void *_ZNK7OSTimer3nowEv(void) {
    if (verbose) puts("STUB: _ZNK7OSTimer3nowEv called");
    return NULL;
)

void *_ZNK7OSTimer8durationENSt3__16chrono10time_pointINS1_12steady_clockENS1_8durationIxNS0_5ratioILl1ELl1000000000EEEEEEES8_(void) {
    if (verbose) puts("STUB: _ZNK7OSTimer8durationENSt3__16chrono10time_pointINS1_12steady_clockENS1_8durationIxNS0_5ratioILl1ELl1000000000EEEEEEES8_ called");
    return NULL;
)

void *_ZN6shazam12get_os_timerEv(void) {
    if (verbose) puts("STUB: _ZN6shazam12get_os_timerEv called");
    return NULL;
)

void *_ZN7OSTimerD1Ev(void) {
    if (verbose) puts("STUB: _ZN7OSTimerD1Ev called");
    return NULL;
)

void *_ZN7OSTimerD0Ev(void) {
    if (verbose) puts("STUB: _ZN7OSTimerD0Ev called");
    return NULL;
)

void *_ZN6shazam23spectral_peak_compact_t10set_deltaTENSt3__16chrono8durationIjNS1_5ratioILl1ELl125EEEEES6_(void) {
    if (verbose) puts("STUB: _ZN6shazam23spectral_peak_compact_t10set_deltaTENSt3__16chrono8durationIjNS1_5ratioILl1ELl125EEEEES6_ called");
    return NULL;
)

void *_ZN6shazam23spectral_peak_compact_t13convert_arrayEPK15spectral_peak_tm(void) {
    if (verbose) puts("STUB: _ZN6shazam23spectral_peak_compact_t13convert_arrayEPK15spectral_peak_tm called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam23spectral_peak_compact_tENS_9allocatorIS2_EEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam23spectral_peak_compact_tENS_9allocatorIS2_EEE7reserveEm called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam23spectral_peak_compact_tERNS_9allocatorIS2_EEEC2EmmS5_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam23spectral_peak_compact_tERNS_9allocatorIS2_EEEC2EmmS5_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam23spectral_peak_compact_tENS_9allocatorIS2_EEE21__push_back_slow_pathIRKS2_EEvOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam23spectral_peak_compact_tENS_9allocatorIS2_EEE21__push_back_slow_pathIRKS2_EEvOT_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam23spectral_peak_compact_tENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam23spectral_peak_compact_tENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN6shazam14signature_infoC2EPK15legacy_sig_info(void) {
    if (verbose) puts("STUB: _ZN6shazam14signature_infoC2EPK15legacy_sig_info called");
    return NULL;
)

void *_ZN6shazam14signature_infoC1EPK15legacy_sig_info(void) {
    if (verbose) puts("STUB: _ZN6shazam14signature_infoC1EPK15legacy_sig_info called");
    return NULL;
)

void *_ZN6shazam14signature_infoC2EPK16LegacyFatSigInfo(void) {
    if (verbose) puts("STUB: _ZN6shazam14signature_infoC2EPK16LegacyFatSigInfo called");
    return NULL;
)

void *_ZN6shazam14signature_infoC1EPK16LegacyFatSigInfo(void) {
    if (verbose) puts("STUB: _ZN6shazam14signature_infoC1EPK16LegacyFatSigInfo called");
    return NULL;
)

void *_ZN6shazam14signature_infoC2EPK11SumoSigInfo(void) {
    if (verbose) puts("STUB: _ZN6shazam14signature_infoC2EPK11SumoSigInfo called");
    return NULL;
)

void *_ZN6shazam14signature_infoC1EPK11SumoSigInfo(void) {
    if (verbose) puts("STUB: _ZN6shazam14signature_infoC1EPK11SumoSigInfo called");
    return NULL;
)

void *_ZN15resample_streamC2Ej(void) {
    if (verbose) puts("STUB: _ZN15resample_streamC2Ej called");
    return NULL;
)

void *_ZN15resample_stream14push_3_1_tightEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE(void) {
    if (verbose) puts("STUB: _ZN15resample_stream14push_3_1_tightEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE called");
    return NULL;
)

void *_ZN15resample_stream19push_44100_to_16000EPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE(void) {
    if (verbose) puts("STUB: _ZN15resample_stream19push_44100_to_16000EPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE called");
    return NULL;
)

void *_ZN15resample_stream14push_2_1_tightEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE(void) {
    if (verbose) puts("STUB: _ZN15resample_stream14push_2_1_tightEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE called");
    return NULL;
)

void *_ZN15resample_stream14push_1_1_tightEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE(void) {
    if (verbose) puts("STUB: _ZN15resample_stream14push_1_1_tightEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE called");
    return NULL;
)

void *_ZN15resample_streamC1Ej(void) {
    if (verbose) puts("STUB: _ZN15resample_streamC1Ej called");
    return NULL;
)

void *_ZN15resample_stream5resetEv(void) {
    if (verbose) puts("STUB: _ZN15resample_stream5resetEv called");
    return NULL;
)

void *_ZN15resample_stream11stream_flowEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE(void) {
    if (verbose) puts("STUB: _ZN15resample_stream11stream_flowEPKfiRNSt3__16vectorIfNS2_9allocatorIfEEEE called");
    return NULL;
)

void *_ZN15resample_stream17sample_rate_checkEj(void) {
    if (verbose) puts("STUB: _ZN15resample_stream17sample_rate_checkEj called");
    return NULL;
)

void *_ZN15resample_stream21push_441_to_320_tightEfRNSt3__16vectorIfNS0_9allocatorIfEEEE(void) {
    if (verbose) puts("STUB: _ZN15resample_stream21push_441_to_320_tightEfRNSt3__16vectorIfNS0_9allocatorIfEEEE called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEE6insertIPKfEENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIfNS_15iterator_traitsIS8_E9referenceEEE5valueENS_11__wrap_iterIPfEEE4typeENSC_IS6_EES8_S8_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEE6insertIPKfEENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIfNS_15iterator_traitsIS8_E9referenceEEE5valueENS_11__wrap_iterIPfEEE4typeENSC_IS6_EES8_S8_ called");
    return NULL;
)

void *_ZNK15resample_stream24get_samples_for_fft_stepEv(void) {
    if (verbose) puts("STUB: _ZNK15resample_stream24get_samples_for_fft_stepEv called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEE26__swap_out_circular_bufferERNS_14__split_bufferIfRS2_EEPf(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEE26__swap_out_circular_bufferERNS_14__split_bufferIfRS2_EEPf called");
    return NULL;
)

void *_ZN6shazam11CPUSearcherC2ERK5Timermmb(void) {
    if (verbose) puts("STUB: _ZN6shazam11CPUSearcherC2ERK5Timermmb called");
    return NULL;
)

void *_ZN6shazam13virtual_arrayIcE6resizeEm(void) {
    if (verbose) puts("STUB: _ZN6shazam13virtual_arrayIcE6resizeEm called");
    return NULL;
)

void *_ZN6shazam11CPUSearcherC1ERK5Timermmb(void) {
    if (verbose) puts("STUB: _ZN6shazam11CPUSearcherC1ERK5Timermmb called");
    return NULL;
)

void *_ZN6shazam11CPUSearcher6searchERKNS_12CPUDatabasesERKNSt3__16vectorINS_15basic_signatureILNS_17signature_densityE24EEENS4_9allocatorIS8_EEEEmmRK19QueryConfigProviderRKNS4_8functionIFvRK14SearchResponseEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam11CPUSearcher6searchERKNS_12CPUDatabasesERKNSt3__16vectorINS_15basic_signatureILNS_17signature_densityE24EEENS4_9allocatorIS8_EEEEmmRK19QueryConfigProviderRKNS4_8functionIFvRK14SearchResponseEEE called");
    return NULL;
)

void *_Z5visitIRN6shazam11CPUSearcherEJRNS0_12SongDataBaseERNS0_16CPUOnePtDatabaseERNS0_15CPULMFPDataBaseERKNS0_15basic_signatureILNS0_17signature_densityE24EEERK11QueryConfigRmRKNSt3__16chrono10time_pointINSJ_12steady_clockENSJ_8durationIxNSI_5ratioILl1ELl1000000000EEEEEEERKNSI_8functionIFvRK14SearchResponseEEEEEDa8AlgoTypeOT_DpOT0_(void) {
    if (verbose) puts("STUB: _Z5visitIRN6shazam11CPUSearcherEJRNS0_12SongDataBaseERNS0_16CPUOnePtDatabaseERNS0_15CPULMFPDataBaseERKNS0_15basic_signatureILNS0_17signature_densityE24EEERK11QueryConfigRmRKNSt3__16chrono10time_pointINSJ_12steady_clockENSJ_8durationIxNSI_5ratioILl1ELl1000000000EEEEEEERKNSI_8functionIFvRK14SearchResponseEEEEEDa8AlgoTypeOT_DpOT0_ called");
    return NULL;
)

void *_ZNSt3__110shared_ptrINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110shared_ptrINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEEED2Ev called");
    return NULL;
)

void *_ZNSt3__110shared_ptrIN6shazam16CPUOnePtDatabaseEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110shared_ptrIN6shazam16CPUOnePtDatabaseEED2Ev called");
    return NULL;
)

void *_ZNSt3__110shared_ptrIN6shazam12SongDataBaseEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110shared_ptrIN6shazam12SongDataBaseEED2Ev called");
    return NULL;
)

void *_ZN6shazam11CPUSearcherclIL8AlgoType1EEEvRKNS_12SongDataBaseERKNS_16CPUOnePtDatabaseERKNS_15CPULMFPDataBaseERKNS_15basic_signatureILNS_17signature_densityE24EEERK11QueryConfigmNSt3__16chrono10time_pointINSL_12steady_clockENSL_8durationIxNSK_5ratioILl1ELl1000000000EEEEEEERKNSK_8functionIFvRK14SearchResponseEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam11CPUSearcherclIL8AlgoType1EEEvRKNS_12SongDataBaseERKNS_16CPUOnePtDatabaseERKNS_15CPULMFPDataBaseERKNS_15basic_signatureILNS_17signature_densityE24EEERK11QueryConfigmNSt3__16chrono10time_pointINSL_12steady_clockENSL_8durationIxNSK_5ratioILl1ELl1000000000EEEEEEERKNSK_8functionIFvRK14SearchResponseEEE called");
    return NULL;
)

void *_ZN6shazam10make_queryIL8AlgoType1EEENS_5QueryIXT_ENSt3__16vectorINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEENS3_9allocatorIS6_EEEEEERKNS_6linkerIXT_EEERKNS_15basic_signatureILNS_17signature_densityE24EEERK11QueryConfigm(void) {
    if (verbose) puts("STUB: _ZN6shazam10make_queryIL8AlgoType1EEENS_5QueryIXT_ENSt3__16vectorINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEENS3_9allocatorIS6_EEEEEERKNS_6linkerIXT_EEERKNS_15basic_signatureILNS_17signature_densityE24EEERK11QueryConfigm called");
    return NULL;
)

void *_ZN6shazam11CPUSearcher6searchIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEEEEvRKNS_12SongDataBaseERKNS_16CPUOnePtDatabaseERKNS_15CPULMFPDataBaseERKNS_5QueryIXT_ENSt3__16vectorINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEENSG_9allocatorISJ_EEEEEERK11QueryConfigmNSG_6chrono10time_pointINST_12steady_clockENST_8durationIxNSG_5ratioILl1ELl1000000000EEEEEEERKNSG_8functionIFvRK14SearchResponseEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam11CPUSearcher6searchIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEEEEvRKNS_12SongDataBaseERKNS_16CPUOnePtDatabaseERKNS_15CPULMFPDataBaseERKNS_5QueryIXT_ENSt3__16vectorINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEENSG_9allocatorISJ_EEEEEERK11QueryConfigmNSG_6chrono10time_pointINST_12steady_clockENST_8durationIxNSG_5ratioILl1ELl1000000000EEEEEEERKNSG_8functionIFvRK14SearchResponseEEE called");
    return NULL;
)

void *_ZN6shazam10matrix_forIL8AlgoType1EEENS_12query_matrixINS_4algoIXT_EE11matrix_typeEEENSt3__16chrono8durationIfNS7_5ratioILl1ELl1EEEEERK11QueryConfig(void) {
    if (verbose) puts("STUB: _ZN6shazam10matrix_forIL8AlgoType1EEENS_12query_matrixINS_4algoIXT_EE11matrix_typeEEENSt3__16chrono8durationIfNS7_5ratioILl1ELl1EEEEERK11QueryConfig called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEE7reserveEm called");
    return NULL;
)

void *_ZN6shazam6kvsortINSt3__16vectorINS_4lmfpILNS_7fp_typeE0EEENS1_9allocatorIS5_EEEEEEvRT_SA_(void) {
    if (verbose) puts("STUB: _ZN6shazam6kvsortINSt3__16vectorINS_4lmfpILNS_7fp_typeE0EEENS1_9allocatorIS5_EEEEEEvRT_SA_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEEC2Em called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEE11__vallocateEm called");
    return NULL;
)

void *_ZN6shazam5QueryIL8AlgoType1ENSt3__16vectorINS_4lmfpILNS_7fp_typeE0EEENS2_9allocatorIS6_EEEEEC2ERKNS_15basic_signatureILNS_17signature_densityE24EEERK9SkewRangeOS9_(void) {
    if (verbose) puts("STUB: _ZN6shazam5QueryIL8AlgoType1ENSt3__16vectorINS_4lmfpILNS_7fp_typeE0EEENS2_9allocatorIS6_EEEEEC2ERKNS_15basic_signatureILNS_17signature_densityE24EEERK9SkewRangeOS9_ called");
    return NULL;
)

void *_ZN6shazam19search_fingerprintsIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEEEEN3gsl4spanIT0_Lln1EEENS6_IKNS_9fp_suffixIXT_EEELln1EEERKNSt3__16vectorIjNSD_9allocatorIjEEEENS6_IKNS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEESJ_ffS8_(void) {
    if (verbose) puts("STUB: _ZN6shazam19search_fingerprintsIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEEEEN3gsl4spanIT0_Lln1EEENS6_IKNS_9fp_suffixIXT_EEELln1EEERKNSt3__16vectorIjNSD_9allocatorIjEEEENS6_IKNS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEESJ_ffS8_ called");
    return NULL;
)

void *_Z25recast_varray_as_gsl_spanI11xpair_groupIJ12xpair_fieldsEEcEN3gsl4spanIT_Lln1EEERN6shazam13virtual_arrayIT0_EE(void) {
    if (verbose) puts("STUB: _Z25recast_varray_as_gsl_spanI11xpair_groupIJ12xpair_fieldsEEcEN3gsl4spanIT_Lln1EEERN6shazam13virtual_arrayIT0_EE called");
    return NULL;
)

void *_ZN6shazam9xpairsortI11xpair_groupIJ12xpair_fieldsEEEEvRN3gsl4spanIT_Lln1EEES8_(void) {
    if (verbose) puts("STUB: _ZN6shazam9xpairsortI11xpair_groupIJ12xpair_fieldsEEEEvRN3gsl4spanIT_Lln1EEES8_ called");
    return NULL;
)

void *_ZN6shazam10bin_tracksI11xpair_groupIJ12xpair_fieldsEEEEvRKN3gsl4spanIKjLln1EEERNS5_IT_Lln1EEERNSt3__16vectorINS_10SongOffsetENSD_9allocatorISF_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam10bin_tracksI11xpair_groupIJ12xpair_fieldsEEEEvRKN3gsl4spanIKjLln1EEERNS5_IT_Lln1EEERNSt3__16vectorINS_10SongOffsetENSD_9allocatorISF_EEEE called");
    return NULL;
)

void *_ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE6resizeEm called");
    return NULL;
)

void *_ZNK6shazam10QueryRangeILNS_7fp_typeE0EE25ConstructFingerprintRangeERKNS_4lmfpILS1_0EEE(void) {
    if (verbose) puts("STUB: _ZNK6shazam10QueryRangeILNS_7fp_typeE0EE25ConstructFingerprintRangeERKNS_4lmfpILS1_0EEE called");
    return NULL;
)

void *_ZN6shazam25binary_fingerprint_searchIL8AlgoType1EEElN3gsl4spanIKNS_9fp_suffixIXT_EEELln1EEEllmRS6_(void) {
    if (verbose) puts("STUB: _ZN6shazam25binary_fingerprint_searchIL8AlgoType1EEElN3gsl4spanIKNS_9fp_suffixIXT_EEELln1EEEllmRS6_ called");
    return NULL;
)

void *_ZNK3gsl4spanI11xpair_groupIJ12xpair_fieldsEELln1EE12make_subspanEllNS4_16subspan_selectorILln1EEE(void) {
    if (verbose) puts("STUB: _ZNK3gsl4spanI11xpair_groupIJ12xpair_fieldsEELln1EE12make_subspanEllNS4_16subspan_selectorILln1EEE called");
    return NULL;
)

void *_ZN3gsl4spanIKN6shazam4lmfpILNS1_7fp_typeE0EEELln1EEC1INSt3__16vectorIS4_NS8_9allocatorIS4_EEEEvEERKT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIKN6shazam4lmfpILNS1_7fp_typeE0EEELln1EEC1INSt3__16vectorIS4_NS8_9allocatorIS4_EEEEvEERKT_ called");
    return NULL;
)

void *_ZN3gsl4spanIKN6shazam4lmfpILNS1_7fp_typeE0EEELln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS5_T_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIKN6shazam4lmfpILNS1_7fp_typeE0EEELln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS5_T_ called");
    return NULL;
)

void *_ZN3gsl4spanIKjLln1EEC1INSt3__16vectorIjNS4_9allocatorIjEEEEvEERKT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIKjLln1EEC1INSt3__16vectorIjNS4_9allocatorIjEEEEvEERKT_ called");
    return NULL;
)

void *_ZN3gsl4spanIKjLln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS1_T_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIKjLln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS1_T_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam10SongOffsetENS_9allocatorIS2_EEE24__emplace_back_slow_pathIJmmEEEvDpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam10SongOffsetENS_9allocatorIS2_EEE24__emplace_back_slow_pathIJmmEEEvDpOT_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam10SongOffsetENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam10SongOffsetENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam10SongOffsetERNS_9allocatorIS2_EEEC2EmmS5_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam10SongOffsetERNS_9allocatorIS2_EEEC2EmmS5_ called");
    return NULL;
)

void *_ZN6shazam10cull_phaseIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELm2ELPv0EEEvRNSt3__16vectorI6ResultNS6_9allocatorIS8_EEEES8_RNS7_INS_11regresspairENS9_ISD_EEEERKN3gsl4spanIT0_Lln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam10cull_phaseIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELm2ELPv0EEEvRNSt3__16vectorI6ResultNS6_9allocatorIS8_EEEES8_RNS7_INS_11regresspairENS9_ISD_EEEERKN3gsl4spanIT0_Lln1EEE called");
    return NULL;
)

void *_Z14fdelta_compareI11xpair_groupIJ12xpair_fieldsEEEbRKT_S5_(void) {
    if (verbose) puts("STUB: _Z14fdelta_compareI11xpair_groupIJ12xpair_fieldsEEEbRKT_S5_ called");
    return NULL;
)

void *_ZNSt3__16__sortIRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEvT0_SF_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__16__sortIRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEvT0_SF_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort3IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEjT0_SF_SF_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort3IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEjT0_SF_SF_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort4IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEjT0_SF_SF_SF_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort4IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEjT0_SF_SF_SF_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort5IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEjT0_SF_SF_SF_SF_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort5IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEjT0_SF_SF_SF_SF_T_ called");
    return NULL;
)

void *_ZNSt3__118__insertion_sort_3IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEvT0_SF_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__118__insertion_sort_3IRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEvT0_SF_T_ called");
    return NULL;
)

void *_ZNSt3__127__insertion_sort_incompleteIRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEbT0_SF_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__127__insertion_sort_incompleteIRPDoFbRK11xpair_groupIJ12xpair_fieldsEES5_EN3gsl7details13span_iteratorINS9_4spanIS3_Lln1EEELb0EEEEEbT0_SF_T_ called");
    return NULL;
)

void *_ZZN6shazam10cull_phaseIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELm2ELPv0EEEvRNSt3__16vectorI6ResultNS6_9allocatorIS8_EEEES8_RNS7_INS_11regresspairENS9_ISD_EEEERKN3gsl4spanIT0_Lln1EEEENKUlT_E_clINSI_IS4_Lln1EEEEEDaSN_(void) {
    if (verbose) puts("STUB: _ZZN6shazam10cull_phaseIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELm2ELPv0EEEvRNSt3__16vectorI6ResultNS6_9allocatorIS8_EEEES8_RNS7_INS_11regresspairENS9_ISD_EEEERKN3gsl4spanIT0_Lln1EEEENKUlT_E_clINSI_IS4_Lln1EEEEEDaSN_ called");
    return NULL;
)

void *_ZN6shazam11shearfactorIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELPv0EEEvRKN3gsl4spanIT0_Lln1EEERfSC_(void) {
    if (verbose) puts("STUB: _ZN6shazam11shearfactorIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELPv0EEEvRKN3gsl4spanIT0_Lln1EEERfSC_ called");
    return NULL;
)

void *_ZN6shazam22xpairs_to_regresspairsI11xpair_groupIJ12xpair_fieldsEEEEvN3gsl4spanIT_Lln1EEEfRNSt3__16vectorINS_11regresspairENS8_9allocatorISA_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam22xpairs_to_regresspairsI11xpair_groupIJ12xpair_fieldsEEEEvN3gsl4spanIT_Lln1EEEfRNSt3__16vectorINS_11regresspairENS8_9allocatorISA_EEEE called");
    return NULL;
)

void *_ZN6shazam5focusILl2ENS_11regresspairEPDoFfRKS1_EZZNS_10cull_phaseIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELm2ELPv0EEEvRNSt3__16vectorI6ResultNSC_9allocatorISE_EEEESE_RNSD_IS1_NSF_IS1_EEEERKN3gsl4spanIT0_Lln1EEEENKUlT_E_clINSN_ISA_Lln1EEEEEDaSS_EUlSS_E_EEvSP_xT1_T2_(void) {
    if (verbose) puts("STUB: _ZN6shazam5focusILl2ENS_11regresspairEPDoFfRKS1_EZZNS_10cull_phaseIL8AlgoType1E11xpair_groupIJ12xpair_fieldsEELm2ELPv0EEEvRNSt3__16vectorI6ResultNSC_9allocatorISE_EEEESE_RNSD_IS1_NSF_IS1_EEEERKN3gsl4spanIT0_Lln1EEEENKUlT_E_clINSN_ISA_Lln1EEEEEDaSS_EUlSS_E_EEvSP_xT1_T2_ called");
    return NULL;
)

void *_ZN6shazam13regresspair_yERKNS_11regresspairE(void) {
    if (verbose) puts("STUB: _ZN6shazam13regresspair_yERKNS_11regresspairE called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam11regresspairENS_9allocatorIS2_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam11regresspairENS_9allocatorIS2_EEE6resizeEm called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam11regresspairENS_9allocatorIS2_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam11regresspairENS_9allocatorIS2_EEE8__appendEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam11regresspairENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam11regresspairENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_Z13insertionsortIN6shazam11regresspairEmNSt3__14lessIS1_EEEvPT_T0_S7_T1_(void) {
    if (verbose) puts("STUB: _Z13insertionsortIN6shazam11regresspairEmNSt3__14lessIS1_EEEvPT_T0_S7_T1_ called");
    return NULL;
)

void *_ZNSt3__16__sortIRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEvT0_S7_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__16__sortIRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEvT0_S7_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort3IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEjT0_S7_S7_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort3IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEjT0_S7_S7_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort4IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEjT0_S7_S7_S7_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort4IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEjT0_S7_S7_S7_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort5IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEjT0_S7_S7_S7_S7_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort5IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEjT0_S7_S7_S7_S7_T_ called");
    return NULL;
)

void *_ZNSt3__118__insertion_sort_3IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEvT0_S7_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__118__insertion_sort_3IRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEvT0_S7_T_ called");
    return NULL;
)

void *_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEbT0_S7_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIN6shazam11regresspairES3_EEPS3_EEbT0_S7_T_ called");
    return NULL;
)

void *_ZN6shazam7regressINS_11regresspairENS_8extractXIS1_EENS_8extractYIS1_EEEENSt3__18optionalINS_12regress_lineEEERKN3gsl4spanIT_Lln1EEET0_T1_(void) {
    if (verbose) puts("STUB: _ZN6shazam7regressINS_11regresspairENS_8extractXIS1_EENS_8extractYIS1_EEEENSt3__18optionalINS_12regress_lineEEERKN3gsl4spanIT_Lln1EEET0_T1_ called");
    return NULL;
)

void *_ZNK3gsl4spanIN6shazam11regresspairELln1EE12make_subspanEllNS3_16subspan_selectorILln1EEE(void) {
    if (verbose) puts("STUB: _ZNK3gsl4spanIN6shazam11regresspairELln1EE12make_subspanEllNS3_16subspan_selectorILln1EEE called");
    return NULL;
)

void *_ZN3gsl4spanIN6shazam11regresspairELln1EEC1INSt3__16vectorIS2_NS5_9allocatorIS2_EEEEvEERT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIN6shazam11regresspairELln1EEC1INSt3__16vectorIS2_NS5_9allocatorIS2_EEEEvEERT_ called");
    return NULL;
)

void *_ZN3gsl4spanIN6shazam11regresspairELln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS2_T_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIN6shazam11regresspairELln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS2_T_ called");
    return NULL;
)

void *_ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE21__push_back_slow_pathIRKS1_EEvOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE21__push_back_slow_pathIRKS1_EEvOT_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseI6ResultNS_9allocatorIS1_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseI6ResultNS_9allocatorIS1_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__114__split_bufferI6ResultRNS_9allocatorIS1_EEEC2EmmS4_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferI6ResultRNS_9allocatorIS1_EEEC2EmmS4_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferI6ResultRNS_9allocatorIS1_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferI6ResultRNS_9allocatorIS1_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__114__split_bufferI6ResultRNS_9allocatorIS1_EEE17__destruct_at_endEPS1_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferI6ResultRNS_9allocatorIS1_EEE17__destruct_at_endEPS1_ called");
    return NULL;
)

void *_ZN6shazam14thresholdScoreIL8AlgoType1EEEfffhh(void) {
    if (verbose) puts("STUB: _ZN6shazam14thresholdScoreIL8AlgoType1EEEfffhh called");
    return NULL;
)

void *_ZN6shazam14peaksToTFPeaksINSt3__16vectorINS2_I15spectral_peak_tNS1_9allocatorIS3_EEEENS4_IS6_EEEEEENS2_I7peak_tfNS4_IS9_EEEERKT_RK14one_pt_options(void) {
    if (verbose) puts("STUB: _ZN6shazam14peaksToTFPeaksINSt3__16vectorINS2_I15spectral_peak_tNS1_9allocatorIS3_EEEENS4_IS6_EEEEEENS2_I7peak_tfNS4_IS9_EEEERKT_RK14one_pt_options called");
    return NULL;
)

void *_ZNSt3__16__sortIRNS_7greaterIvEEP6ResultEEvT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__16__sortIRNS_7greaterIvEEP6ResultEEvT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort3IRNS_7greaterIvEEP6ResultEEjT0_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort3IRNS_7greaterIvEEP6ResultEEjT0_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort4IRNS_7greaterIvEEP6ResultEEjT0_S6_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort4IRNS_7greaterIvEEP6ResultEEjT0_S6_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort5IRNS_7greaterIvEEP6ResultEEjT0_S6_S6_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort5IRNS_7greaterIvEEP6ResultEEjT0_S6_S6_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__118__insertion_sort_3IRNS_7greaterIvEEP6ResultEEvT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__118__insertion_sort_3IRNS_7greaterIvEEP6ResultEEvT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__127__insertion_sort_incompleteIRNS_7greaterIvEEP6ResultEEbT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__127__insertion_sort_incompleteIRNS_7greaterIvEEP6ResultEEbT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE8__appendEm called");
    return NULL;
)

void *_ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE17__destruct_at_endEPS1_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE17__destruct_at_endEPS1_ called");
    return NULL;
)

void *_ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEEC2ERKS4_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEEC2ERKS4_ called");
    return NULL;
)

void *_ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI6ResultNS_9allocatorIS1_EEE11__vallocateEm called");
    return NULL;
)

void *_ZN2tl6detail21expected_storage_baseINSt3__16vectorI6ResultNS2_9allocatorIS4_EEEENS2_12system_errorELb0ELb0EED2Ev(void) {
    if (verbose) puts("STUB: _ZN2tl6detail21expected_storage_baseINSt3__16vectorI6ResultNS2_9allocatorIS4_EEEENS2_12system_errorELb0ELb0EED2Ev called");
    return NULL;
)

void *_ZNSt3__125__throw_bad_function_callEv(void) {
    if (verbose) puts("STUB: _ZNSt3__125__throw_bad_function_callEv called");
    return NULL;
)

void *_ZN6shazam13virtual_arrayIcE7reserveEm(void) {
    if (verbose) puts("STUB: _ZN6shazam13virtual_arrayIcE7reserveEm called");
    return NULL;
)

void *_ZNK10SearchPlan11assertValidEv(void) {
    if (verbose) puts("STUB: _ZNK10SearchPlan11assertValidEv called");
    return NULL;
)

void *_ZN10SearchPlanC1E8AlgoType9DBDensity(void) {
    if (verbose) puts("STUB: _ZN10SearchPlanC1E8AlgoType9DBDensity called");
    return NULL;
)

void *_ZN6shazam17FatHeaderIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17FatHeaderIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZN6shazam15iterate_headersINS_25fp_data_header_extern_newEEEvN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam15iterate_headersINS_25fp_data_header_extern_newEEEvN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZNSt3__117bad_function_callD1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__117bad_function_callD1Ev called");
    return NULL;
)

void *_ZNSt3__117bad_function_callD0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__117bad_function_callD0Ev called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPKNS_25fp_data_header_extern_newELi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPKNS_25fp_data_header_extern_newELi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam4algoIL8AlgoType1EE10matrix_forE12QueryDensity(void) {
    if (verbose) puts("STUB: _ZN6shazam4algoIL8AlgoType1EE10matrix_forE12QueryDensity called");
    return NULL;
)

void *_ZN6shazam4algoIL8AlgoType1EE10matrix_forE9DBDensity(void) {
    if (verbose) puts("STUB: _ZN6shazam4algoIL8AlgoType1EE10matrix_forE9DBDensity called");
    return NULL;
)

void *_ZNSt3__120__throw_out_of_rangeEPKc(void) {
    if (verbose) puts("STUB: _ZNSt3__120__throw_out_of_rangeEPKc called");
    return NULL;
)

void *_ZN6shazam24StaticCPUSearcherFactoryC1ENS_12CPUDatabasesEmRK5Timer(void) {
    if (verbose) puts("STUB: _ZN6shazam24StaticCPUSearcherFactoryC1ENS_12CPUDatabasesEmRK5Timer called");
    return NULL;
)

void *_ZNK6shazam24StaticCPUSearcherFactory13make_searcherEv(void) {
    if (verbose) puts("STUB: _ZNK6shazam24StaticCPUSearcherFactory13make_searcherEv called");
    return NULL;
)

void *_ZNSt3__111make_uniqueIN6shazam17StaticCPUSearcherEJRKNS1_12CPUDatabasesERK5TimerRKmSA_EEENS_11__unique_ifIT_E15__unique_singleEDpOT0_(void) {
    if (verbose) puts("STUB: _ZNSt3__111make_uniqueIN6shazam17StaticCPUSearcherEJRKNS1_12CPUDatabasesERK5TimerRKmSA_EEENS_11__unique_ifIT_E15__unique_singleEDpOT0_ called");
    return NULL;
)

void *_ZN6shazam24StaticCPUSearcherFactoryD1Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam24StaticCPUSearcherFactoryD1Ev called");
    return NULL;
)

void *_ZN6shazam24StaticCPUSearcherFactoryD0Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam24StaticCPUSearcherFactoryD0Ev called");
    return NULL;
)

void *_ZN6shazam2io4packERKNS_17basic_signature_tE(void) {
    if (verbose) puts("STUB: _ZN6shazam2io4packERKNS_17basic_signature_tE called");
    return NULL;
)

void *_ZN6shazam2io18addShazamKitHeaderERNSt3__16vectorIcNS1_9allocatorIcEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam2io18addShazamKitHeaderERNSt3__16vectorIcNS1_9allocatorIcEEEE called");
    return NULL;
)

void *_ZN6shazam2io4packERNSt3__16vectorIcNS1_9allocatorIcEEEERKNS_17basic_signature_tE(void) {
    if (verbose) puts("STUB: _ZN6shazam2io4packERNSt3__16vectorIcNS1_9allocatorIcEEEERKNS_17basic_signature_tE called");
    return NULL;
)

void *_ZNSt3__16vectorIcNS_9allocatorIcEEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIcNS_9allocatorIcEEE7reserveEm called");
    return NULL;
)

void *_ZN6shazam2io15write_unalignedINS_23spectral_peak_compact_tEEEvRNSt3__16vectorIcNS3_9allocatorIcEEEERKNS4_IT_NS5_IS9_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam2io15write_unalignedINS_23spectral_peak_compact_tEEEvRNSt3__16vectorIcNS3_9allocatorIcEEEERKNS4_IT_NS5_IS9_EEEE called");
    return NULL;
)

void *_ZNSt3__112__rotate_gcdINS_16reverse_iteratorINS_11__wrap_iterIPcEEEEEET_S6_S6_S6_(void) {
    if (verbose) puts("STUB: _ZNSt3__112__rotate_gcdINS_16reverse_iteratorINS_11__wrap_iterIPcEEEEEET_S6_S6_S6_ called");
    return NULL;
)

void *_ZNSt3__113__vector_baseIN6shazam2io12_GLOBAL__N_115compacted_peaksENS_9allocatorIS4_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseIN6shazam2io12_GLOBAL__N_115compacted_peaksENS_9allocatorIS4_EEED2Ev called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam2io12_GLOBAL__N_115compacted_peaksENS_9allocatorIS4_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam2io12_GLOBAL__N_115compacted_peaksENS_9allocatorIS4_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNK11SumoSigInfo19get_raw_sample_rateEv(void) {
    if (verbose) puts("STUB: _ZNK11SumoSigInfo19get_raw_sample_rateEv called");
    return NULL;
)

void *_ZNK11SumoSigInfo25get_sample_length_samplesEv(void) {
    if (verbose) puts("STUB: _ZNK11SumoSigInfo25get_sample_length_samplesEv called");
    return NULL;
)

void *_ZNK11SumoSigInfo19get_stream_positionEv(void) {
    if (verbose) puts("STUB: _ZNK11SumoSigInfo19get_stream_positionEv called");
    return NULL;
)

void *_ZN6shazam4MMapD2Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam4MMapD2Ev called");
    return NULL;
)

void *_ZN6shazam4MMapD1Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam4MMapD1Ev called");
    return NULL;
)

void *_ZN6shazam7windows14blackmanHarrisEm(void) {
    if (verbose) puts("STUB: _ZN6shazam7windows14blackmanHarrisEm called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEEC2EmRKf(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEEC2EmRKf called");
    return NULL;
)

void *_ZN6shazam8targetFPERKNS_14link_matrix_2dEm(void) {
    if (verbose) puts("STUB: _ZN6shazam8targetFPERKNS_14link_matrix_2dEm called");
    return NULL;
)

void *_ZN6shazam14reframe_matrixERKNS_14link_matrix_2dEjNSt3__16chrono8durationIjNS3_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam14reframe_matrixERKNS_14link_matrix_2dEjNSt3__16chrono8durationIjNS3_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZN6shazam16intensify_matrixENS_14link_matrix_2dENSt3__16chrono8durationIfNS1_5ratioILl1ELl1EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam16intensify_matrixENS_14link_matrix_2dENSt3__16chrono8durationIfNS1_5ratioILl1ELl1EEEEE called");
    return NULL;
)

void *_ZN12FreqLogTable2atEt(void) {
    if (verbose) puts("STUB: _ZN12FreqLogTable2atEt called");
    return NULL;
)

void *_ZN6shazam18signature_categoryEv(void) {
    if (verbose) puts("STUB: _ZN6shazam18signature_categoryEv called");
    return NULL;
)

void *_ZN12_GLOBAL__N_123signature_category_implD1Ev(void) {
    if (verbose) puts("STUB: _ZN12_GLOBAL__N_123signature_category_implD1Ev called");
    return NULL;
)

void *_ZN6shazam15make_error_codeENS_9sig_errorE(void) {
    if (verbose) puts("STUB: _ZN6shazam15make_error_codeENS_9sig_errorE called");
    return NULL;
)

void *_ZN12_GLOBAL__N_123signature_category_implD0Ev(void) {
    if (verbose) puts("STUB: _ZN12_GLOBAL__N_123signature_category_implD0Ev called");
    return NULL;
)

void *_ZNK12_GLOBAL__N_123signature_category_impl4nameEv(void) {
    if (verbose) puts("STUB: _ZNK12_GLOBAL__N_123signature_category_impl4nameEv called");
    return NULL;
)

void *_ZNK12_GLOBAL__N_123signature_category_impl7messageEi(void) {
    if (verbose) puts("STUB: _ZNK12_GLOBAL__N_123signature_category_impl7messageEi called");
    return NULL;
)

void *_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEC1IDnEEPKc(void) {
    if (verbose) puts("STUB: _ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEC1IDnEEPKc called");
    return NULL;
)

void *_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN7SHZ_FFTC2Em(void) {
    if (verbose) puts("STUB: _ZN7SHZ_FFTC2Em called");
    return NULL;
)

void *_ZN7SHZ_FFTC1Em(void) {
    if (verbose) puts("STUB: _ZN7SHZ_FFTC1Em called");
    return NULL;
)

void *_ZN7SHZ_FFT11complexformEPKfPfPNSt3__17complexIfEE(void) {
    if (verbose) puts("STUB: _ZN7SHZ_FFT11complexformEPKfPfPNSt3__17complexIfEE called");
    return NULL;
)

void *_ZN10sig_streamC2Ev(void) {
    if (verbose) puts("STUB: _ZN10sig_streamC2Ev called");
    return NULL;
)

void *_ZN10sig_stream5resetEv(void) {
    if (verbose) puts("STUB: _ZN10sig_stream5resetEv called");
    return NULL;
)

void *_ZN10sig_streamC1Ev(void) {
    if (verbose) puts("STUB: _ZN10sig_streamC1Ev called");
    return NULL;
)

void *_ZNK10sig_stream3nowEv(void) {
    if (verbose) puts("STUB: _ZNK10sig_stream3nowEv called");
    return NULL;
)

void *_ZN10sig_stream10checkpointEv(void) {
    if (verbose) puts("STUB: _ZN10sig_stream10checkpointEv called");
    return NULL;
)

void *_ZN10sig_stream6rewindEv(void) {
    if (verbose) puts("STUB: _ZN10sig_stream6rewindEv called");
    return NULL;
)

void *_ZNK17AlignedRingBufferIfE2atEi(void) {
    if (verbose) puts("STUB: _ZNK17AlignedRingBufferIfE2atEi called");
    return NULL;
)

void *_ZN10sig_stream14push_frame_maxEPKfi(void) {
    if (verbose) puts("STUB: _ZN10sig_stream14push_frame_maxEPKfi called");
    return NULL;
)

void *_ZNK10sig_stream32get_writable_fft_frame_magnitudeEv(void) {
    if (verbose) puts("STUB: _ZNK10sig_stream32get_writable_fft_frame_magnitudeEv called");
    return NULL;
)

void *_ZNK10sig_stream30get_writable_fft_frame_complexEv(void) {
    if (verbose) puts("STUB: _ZNK10sig_stream30get_writable_fft_frame_complexEv called");
    return NULL;
)

void *_ZN10sig_stream10find_peaksEPKfRm(void) {
    if (verbose) puts("STUB: _ZN10sig_stream10find_peaksEPKfRm called");
    return NULL;
)

void *_ZN10sig_stream11stream_flowERNSt3__15dequeI15spectral_peak_tNS0_9allocatorIS2_EEEE(void) {
    if (verbose) puts("STUB: _ZN10sig_stream11stream_flowERNSt3__15dequeI15spectral_peak_tNS0_9allocatorIS2_EEEE called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE12emplace_backIJRKNS_6chrono8durationIxNS_5ratioILl1ELl125EEEEERKjRPKfEEERS1_DpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE12emplace_backIJRKNS_6chrono8durationIxNS_5ratioILl1ELl125EEEEERKjRPKfEEERS1_DpOT_ called");
    return NULL;
)

void *_ZN17AlignedRingBufferIfEC2Eii(void) {
    if (verbose) puts("STUB: _ZN17AlignedRingBufferIfEC2Eii called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEEC2Em called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIfNS_9allocatorIfEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIfNS_9allocatorIfEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN17AlignedRingBufferINSt3__17complexIfEEEC2Eii(void) {
    if (verbose) puts("STUB: _ZN17AlignedRingBufferINSt3__17complexIfEEEC2Eii called");
    return NULL;
)

void *_ZNSt3__16vectorINS_7complexIfEENS_9allocatorIS2_EEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS_7complexIfEENS_9allocatorIS2_EEEC2Em called");
    return NULL;
)

void *_ZNSt3__16vectorINS_7complexIfEENS_9allocatorIS2_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS_7complexIfEENS_9allocatorIS2_EEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseINS_7complexIfEENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseINS_7complexIfEENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNK17AlignedRingBufferIfE9get_frameEi(void) {
    if (verbose) puts("STUB: _ZNK17AlignedRingBufferIfE9get_frameEi called");
    return NULL;
)

void *_ZNK17AlignedRingBufferINSt3__17complexIfEEE9get_frameEi(void) {
    if (verbose) puts("STUB: _ZNK17AlignedRingBufferINSt3__17complexIfEEE9get_frameEi called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE19__add_back_capacityEv(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE19__add_back_capacityEv called");
    return NULL;
)

void *_Z16flatten_and_sortRKNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEEE(void) {
    if (verbose) puts("STUB: _Z16flatten_and_sortRKNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEEE called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE6insertINS_11__wrap_iterIPKS1_EEEENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIS1_NS_15iterator_traitsISB_E9referenceEEE5valueENS6_IPS1_EEE4typeES9_SB_SB_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE6insertINS_11__wrap_iterIPKS1_EEEENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIS1_NS_15iterator_traitsISB_E9referenceEEE5valueENS6_IPS1_EEE4typeES9_SB_SB_ called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS1_RS3_EEPS1_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS1_RS3_EEPS1_ called");
    return NULL;
)

void *_Z11unobfuscatePymyS_(void) {
    if (verbose) puts("STUB: _Z11unobfuscatePymyS_ called");
    return NULL;
)

void *_Z11UnobfuscatePcm(void) {
    if (verbose) puts("STUB: _Z11UnobfuscatePcm called");
    return NULL;
)

void *_ZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC2ERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC2ERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE6resizeEm called");
    return NULL;
)

void *_ZN6shazam19find_last_peak_tickERKNSt3__16vectorINS1_I15spectral_peak_tNS0_9allocatorIS2_EEEENS3_IS5_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam19find_last_peak_tickERKNSt3__16vectorINS1_I15spectral_peak_tNS0_9allocatorIS2_EEEENS3_IS5_EEEE called");
    return NULL;
)

void *_ZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC1ERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC1ERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZN6shazam15basic_signatureILNS_17signature_densityE24EEC2ERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam15basic_signatureILNS_17signature_densityE24EEC2ERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZN6shazam15basic_signatureILNS_17signature_densityE24EEC1ERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam15basic_signatureILNS_17signature_densityE24EEC1ERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZN6shazam15basic_signatureILNS_17signature_densityE16EEC2ERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam15basic_signatureILNS_17signature_densityE16EEC2ERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZN6shazam15basic_signatureILNS_17signature_densityE16EEC1ERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam15basic_signatureILNS_17signature_densityE16EEC1ERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE8__appendEm called");
    return NULL;
)

void *_ZNSt3__17__sort3IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEjT0_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort3IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEjT0_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort4IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEjT0_S6_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort4IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEjT0_S6_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort5IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEjT0_S6_S6_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort5IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEjT0_S6_S6_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__118__insertion_sort_3IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEvT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__118__insertion_sort_3IRNS_6__lessI15spectral_peak_tS2_EEPS2_EEvT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessI15spectral_peak_tS2_EEPS2_EEbT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__127__insertion_sort_incompleteIRNS_6__lessI15spectral_peak_tS2_EEPS2_EEbT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE8__appendEm called");
    return NULL;
)

void *_ZNSt3__114__split_bufferINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEERNS3_IS5_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEERNS3_IS5_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__114__split_bufferINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEERNS3_IS5_EEE17__destruct_at_endEPS5_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEERNS3_IS5_EEE17__destruct_at_endEPS5_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam21signature_header_typeENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam21signature_header_typeENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__113__vector_baseINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEENS4_IS6_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEENS4_IS6_EEED2Ev called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEENS4_IS6_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEENS4_IS6_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__113__vector_baseINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEENS4_IS6_EEE5clearEv(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEENS4_IS6_EEE5clearEv called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE13__vdeallocateEv(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE13__vdeallocateEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED1Ev called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED0Ev called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEv called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEPNS0_6__baseISD_EE(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEPNS0_6__baseISD_EE called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7destroyEv(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7destroyEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE18destroy_deallocateEv(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE18destroy_deallocateEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEclEOS9_(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEclEOS9_ called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE6targetERKSt9type_info(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE6targetERKSt9type_info called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE11target_typeEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE2147483647EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE11target_typeEv called");
    return NULL;
)

void *_ZZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC1ERKNS_21packed_signature_viewEENKUlNS_16signature_headerEE_clES6_(void) {
    if (verbose) puts("STUB: _ZZN6shazam15basic_signatureILNS_17signature_densityE2147483647EEC1ERKNS_21packed_signature_viewEENKUlNS_16signature_headerEE_clES6_ called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPK15legacy_sig_infoLi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPK15legacy_sig_infoLi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPK16LegacyFatSigInfoLi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPK16LegacyFatSigInfoLi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPK11SumoSigInfoLi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPK11SumoSigInfoLi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPKNS_23spectral_peak_compact_tELi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPKNS_23spectral_peak_compact_tELi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN12_GLOBAL__N_112decode_peaksEPKN6shazam23spectral_peak_compact_tEmNS0_17signature_densityENSt3__16chrono8durationIhNS5_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN12_GLOBAL__N_112decode_peaksEPKN6shazam23spectral_peak_compact_tEmNS0_17signature_densityENSt3__16chrono8durationIhNS5_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPK15spectral_peak_tLi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPK15spectral_peak_tLi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE6assignIPKS1_EENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIS1_NS_15iterator_traitsIS9_E9referenceEEE5valueEvE4typeES9_S9_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE6assignIPKS1_EENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIS1_NS_15iterator_traitsIS9_E9referenceEEE5valueEvE4typeES9_S9_ called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_IN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEEENS3_IS5_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_IN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEEENS3_IS5_EEE6resizeEm called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam21signature_header_typeENS_9allocatorIS2_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam21signature_header_typeENS_9allocatorIS2_EEE6resizeEm called");
    return NULL;
)

void *_ZN3gsl9make_spanIKN6shazam23spectral_peak_compact_tEEENS_4spanIT_Lln1EEEPS5_NS6_10index_typeE(void) {
    if (verbose) puts("STUB: _ZN3gsl9make_spanIKN6shazam23spectral_peak_compact_tEEENS_4spanIT_Lln1EEEPS5_NS6_10index_typeE called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE11__vallocateEm called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_IN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEEENS3_IS5_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_IN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEEENS3_IS5_EEE8__appendEm called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_IN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEEENS3_IS5_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS5_RS6_EE(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_IN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEEENS3_IS5_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS5_RS6_EE called");
    return NULL;
)

void *_ZNSt3__114__split_bufferINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEERNS4_IS6_EEEC2EmmS8_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEERNS4_IS6_EEEC2EmmS8_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEERNS4_IS6_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEERNS4_IS6_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__114__split_bufferINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEERNS4_IS6_EEE17__destruct_at_endEPS6_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferINS_6vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS3_EEEERNS4_IS6_EEE17__destruct_at_endEPS6_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam21signature_header_typeENS_9allocatorIS2_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam21signature_header_typeENS_9allocatorIS2_EEE8__appendEm called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPKNS_24ancient_dead_sig_point_tELi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPKNS_24ancient_dead_sig_point_tELi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEE6assignIPKS2_EENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIS2_NS_15iterator_traitsISA_E9referenceEEE5valueEvE4typeESA_SA_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEE6assignIPKS2_EENS_9enable_ifIXaasr27__is_cpp17_forward_iteratorIT_EE5valuesr16is_constructibleIS2_NS_15iterator_traitsISA_E9referenceEEE5valueEvE4typeESA_SA_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam24ancient_dead_sig_point_tENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED1Ev called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED0Ev called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEv called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEPNS0_6__baseISD_EE(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEPNS0_6__baseISD_EE called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7destroyEv(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7destroyEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE18destroy_deallocateEv(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE18destroy_deallocateEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEclEOS9_(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEclEOS9_ called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE6targetERKSt9type_info(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE6targetERKSt9type_info called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE11target_typeEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE24EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE11target_typeEv called");
    return NULL;
)

void *_ZZN6shazam15basic_signatureILNS_17signature_densityE24EEC1ERKNS_21packed_signature_viewEENKUlNS_16signature_headerEE_clES6_(void) {
    if (verbose) puts("STUB: _ZZN6shazam15basic_signatureILNS_17signature_densityE24EEC1ERKNS_21packed_signature_viewEENKUlNS_16signature_headerEE_clES6_ called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED1Ev called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EED0Ev called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEv called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEPNS0_6__baseISD_EE(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7__cloneEPNS0_6__baseISD_EE called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7destroyEv(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE7destroyEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE18destroy_deallocateEv(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE18destroy_deallocateEv called");
    return NULL;
)

void *_ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEclEOS9_(void) {
    if (verbose) puts("STUB: _ZNSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EEclEOS9_ called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE6targetERKSt9type_info(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE6targetERKSt9type_info called");
    return NULL;
)

void *_ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE11target_typeEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__110__function6__funcIZN6shazam15basic_signatureILNS2_17signature_densityE16EEC1ERKNS2_21packed_signature_viewEEUlNS2_16signature_headerEE_NS_9allocatorISA_EEFvS9_EE11target_typeEv called");
    return NULL;
)

void *_ZZN6shazam15basic_signatureILNS_17signature_densityE16EEC1ERKNS_21packed_signature_viewEENKUlNS_16signature_headerEE_clES6_(void) {
    if (verbose) puts("STUB: _ZZN6shazam15basic_signatureILNS_17signature_densityE16EEC1ERKNS_21packed_signature_viewEENKUlNS_16signature_headerEE_clES6_ called");
    return NULL;
)

void *_ZN6shazam11match_peaksEN3gsl4spanIK7peak_tfLln1EEERKNSt3__16vectorIS2_NS5_9allocatorIS2_EEEEffiRNS6_INS_7tfmatchENS7_ISC_EEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam11match_peaksEN3gsl4spanIK7peak_tfLln1EEERKNSt3__16vectorIS2_NS5_9allocatorIS2_EEEEffiRNS6_INS_7tfmatchENS7_ISC_EEEE called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam7tfmatchENS_9allocatorIS2_EEE24__emplace_back_slow_pathIJiRKjRKfRKiEEEvDpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam7tfmatchENS_9allocatorIS2_EEE24__emplace_back_slow_pathIJiRKjRKfRKiEEEvDpOT_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam7tfmatchENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam7tfmatchENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam7tfmatchERNS_9allocatorIS2_EEEC2EmmS5_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam7tfmatchERNS_9allocatorIS2_EEEC2EmmS5_ called");
    return NULL;
)

void *_ZN6shazam13thresholdLerpERKNSt3__15arrayIfLm16EEEf(void) {
    if (verbose) puts("STUB: _ZN6shazam13thresholdLerpERKNSt3__15arrayIfLm16EEEf called");
    return NULL;
)

void *_ZN6shazam15make_error_codeENS_10sigx_errorE(void) {
    if (verbose) puts("STUB: _ZN6shazam15make_error_codeENS_10sigx_errorE called");
    return NULL;
)

void *_ZN6shazam21packed_signature_viewC2EN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam21packed_signature_viewC2EN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam21packed_signature_viewC1EN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam21packed_signature_viewC1EN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZNK6shazam21packed_signature_view7headersERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZNK6shazam21packed_signature_view7headersERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPKjLi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPKjLi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam11get_siginfoERKNS_21packed_signature_viewE(void) {
    if (verbose) puts("STUB: _ZN6shazam11get_siginfoERKNS_21packed_signature_viewE called");
    return NULL;
)

void *_ZN12_GLOBAL__N_119type_from_magic_keyEN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN12_GLOBAL__N_119type_from_magic_keyEN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam4cropILNS_17signature_densityE2147483647EEENS_15basic_signatureIXT_EEERKS3_NSt3__16chrono8durationIjNS6_5ratioILl1ELl125EEEEESB_(void) {
    if (verbose) puts("STUB: _ZN6shazam4cropILNS_17signature_densityE2147483647EEENS_15basic_signatureIXT_EEERKS3_NSt3__16chrono8durationIjNS6_5ratioILl1ELl125EEEEESB_ called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS4_RS5_EE(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS4_RS5_EE called");
    return NULL;
)

void *_ZNSt3__114__split_bufferINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEERNS3_IS5_EEEC2EmmS7_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEERNS3_IS5_EEEC2EmmS7_ called");
    return NULL;
)

void *_ZNSt12length_errorC1EPKc(void) {
    if (verbose) puts("STUB: _ZNSt12length_errorC1EPKc called");
    return NULL;
)

void *_ZNSt3__113__lower_boundIRZN6shazam9crop_bandI15spectral_peak_tEEvN3gsl4spanIKT_Lln1EEERNS_6vectorIS6_NS_9allocatorIS6_EEEENS_6chrono8durationIjNS_5ratioILl1ELl125EEEEESI_EUlRS7_RKT0_E_NS4_7details13span_iteratorINS5_IKS3_Lln1EEELb0EEESI_EESK_SK_SK_RKT1_S6_(void) {
    if (verbose) puts("STUB: _ZNSt3__113__lower_boundIRZN6shazam9crop_bandI15spectral_peak_tEEvN3gsl4spanIKT_Lln1EEERNS_6vectorIS6_NS_9allocatorIS6_EEEENS_6chrono8durationIjNS_5ratioILl1ELl125EEEEESI_EUlRS7_RKT0_E_NS4_7details13span_iteratorINS5_IKS3_Lln1EEELb0EEESI_EESK_SK_SK_RKT1_S6_ called");
    return NULL;
)

void *_ZNSt3__113__upper_boundIRZN6shazam9crop_bandI15spectral_peak_tEEvN3gsl4spanIKT_Lln1EEERNS_6vectorIS6_NS_9allocatorIS6_EEEENS_6chrono8durationIjNS_5ratioILl1ELl125EEEEESI_EUlRS7_RKT0_E0_NS4_7details13span_iteratorINS5_IKS3_Lln1EEELb0EEESI_EESK_SK_SK_RKT1_S6_(void) {
    if (verbose) puts("STUB: _ZNSt3__113__upper_boundIRZN6shazam9crop_bandI15spectral_peak_tEEvN3gsl4spanIKT_Lln1EEERNS_6vectorIS6_NS_9allocatorIS6_EEEENS_6chrono8durationIjNS_5ratioILl1ELl125EEEEESI_EUlRS7_RKT0_E0_NS4_7details13span_iteratorINS5_IKS3_Lln1EEELb0EEESI_EESK_SK_SK_RKT1_S6_ called");
    return NULL;
)

void *_ZN3gsl4spanIK15spectral_peak_tLln1EEC1INSt3__16vectorIS1_NS5_9allocatorIS1_EEEEvEERKT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIK15spectral_peak_tLln1EEC1INSt3__16vectorIS1_NS5_9allocatorIS1_EEEEvEERKT_ called");
    return NULL;
)

void *_ZN3gsl4spanIK15spectral_peak_tLln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS2_T_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIK15spectral_peak_tLln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPS2_T_ called");
    return NULL;
)

void *_ZN6shazam17StaticCPUSearcherC2ENS_12CPUDatabasesERK5Timermm(void) {
    if (verbose) puts("STUB: _ZN6shazam17StaticCPUSearcherC2ENS_12CPUDatabasesERK5Timermm called");
    return NULL;
)

void *_ZN6shazam17StaticCPUSearcherC1ENS_12CPUDatabasesERK5Timermm(void) {
    if (verbose) puts("STUB: _ZN6shazam17StaticCPUSearcherC1ENS_12CPUDatabasesERK5Timermm called");
    return NULL;
)

void *_ZN6shazam17StaticCPUSearcher6searchERKNSt3__16vectorINS_15basic_signatureILNS_17signature_densityE24EEENS1_9allocatorIS5_EEEEmmRK19QueryConfigProviderNS1_8functionIFvRK14SearchResponseEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17StaticCPUSearcher6searchERKNSt3__16vectorINS_15basic_signatureILNS_17signature_densityE24EEENS1_9allocatorIS5_EEEEmmRK19QueryConfigProviderNS1_8functionIFvRK14SearchResponseEEE called");
    return NULL;
)

void *_ZN6shazam17StaticCPUSearcherD1Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam17StaticCPUSearcherD1Ev called");
    return NULL;
)

void *_ZN6shazam17StaticCPUSearcherD0Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam17StaticCPUSearcherD0Ev called");
    return NULL;
)

void *_ZN6shazam17StaticCPUSearcherD2Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam17StaticCPUSearcherD2Ev called");
    return NULL;
)

void *_ZN6shazam11CPUSearcherD2Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam11CPUSearcherD2Ev called");
    return NULL;
)

void *_ZNSt3__113__vector_baseI6ResultNS_9allocatorIS1_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseI6ResultNS_9allocatorIS1_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__113__vector_baseI6ResultNS_9allocatorIS1_EEE5clearEv(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseI6ResultNS_9allocatorIS1_EEE5clearEv called");
    return NULL;
)

void *_ZN6shazam13virtual_arrayIcED2Ev(void) {
    if (verbose) puts("STUB: _ZN6shazam13virtual_arrayIcED2Ev called");
    return NULL;
)

void *_ZN6shazam19parse_ancient_peaksERKNSt3__16vectorINS_21signature_header_typeENS0_9allocatorIS2_EEEERKNS1_INS1_INS_24ancient_dead_sig_point_tENS3_IS8_EEEENS3_ISA_EEEENS_17signature_densityENS0_6chrono8durationIhNS0_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam19parse_ancient_peaksERKNSt3__16vectorINS_21signature_header_typeENS0_9allocatorIS2_EEEERKNS1_INS1_INS_24ancient_dead_sig_point_tENS3_IS8_EEEENS3_ISA_EEEENS_17signature_densityENS0_6chrono8durationIhNS0_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEENS3_IS5_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEENS3_IS5_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseI15spectral_peak_tNS_9allocatorIS1_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseI15spectral_peak_tNS_9allocatorIS1_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN6shazam14density_filterI15spectral_peak_tE5flushEv(void) {
    if (verbose) puts("STUB: _ZN6shazam14density_filterI15spectral_peak_tE5flushEv called");
    return NULL;
)

void *_ZN6shazam12cropped_infoERKNS_14signature_infoENSt3__16chrono8durationIjNS3_5ratioILl1ELl125EEEEES8_(void) {
    if (verbose) puts("STUB: _ZN6shazam12cropped_infoERKNS_14signature_infoENSt3__16chrono8durationIjNS3_5ratioILl1ELl125EEEEES8_ called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilderC2ESt16initializer_listI10SearchPlanE(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilderC2ESt16initializer_listI10SearchPlanE called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilderC1ESt16initializer_listI10SearchPlanE(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilderC1ESt16initializer_listI10SearchPlanE called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilder3addERKNS_15basic_signatureILNS_17signature_densityE16EEEyy(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilder3addERKNS_15basic_signatureILNS_17signature_densityE16EEEyy called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilder13addSigSegmentERKNS_15basic_signatureILNS_17signature_densityE16EEEyNSt3__16chrono8durationIfNS6_5ratioILl1ELl1EEEEENS8_IxNS9_ILl1ELl1000EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilder13addSigSegmentERKNS_15basic_signatureILNS_17signature_densityE16EEEyNSt3__16chrono8durationIfNS6_5ratioILl1ELl1EEEEENS8_IxNS9_ILl1ELl1000EEEEE called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilder7addDataEyNSt3__16chrono8durationIfNS1_5ratioILl1ELl1EEEEENS3_IhNS4_ILl1ELl125EEEEEhNS3_IjS7_EERKNS1_6vectorI7peak_tfNS1_9allocatorISB_EEEENS3_IxNS4_ILl1ELl1000EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilder7addDataEyNSt3__16chrono8durationIfNS1_5ratioILl1ELl1EEEEENS3_IhNS4_ILl1ELl125EEEEEhNS3_IjS7_EERKNS1_6vectorI7peak_tfNS1_9allocatorISB_EEEENS3_IxNS4_ILl1ELl1000EEEEE called");
    return NULL;
)

void *_ZNSt3__16vectorI7peak_tfNS_9allocatorIS1_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI7peak_tfNS_9allocatorIS1_EEE6resizeEm called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilder5buildEv(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilder5buildEv called");
    return NULL;
)

void *_ZN6shazam18CPUDatabaseBuilder5resetEv(void) {
    if (verbose) puts("STUB: _ZN6shazam18CPUDatabaseBuilder5resetEv called");
    return NULL;
)

void *_ZN6shazam6detail13writeLMFPDataIL8AlgoType1EEEvRKNSt3__16vectorINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEENS3_9allocatorIS6_EEEERNS4_IcNS7_IcEEEERKNS3_6chrono8durationIyNS3_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam6detail13writeLMFPDataIL8AlgoType1EEEvRKNSt3__16vectorINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEENS3_9allocatorIS6_EEEERNS4_IcNS7_IcEEEERKNS3_6chrono8durationIyNS3_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZNSt3__16vectorIcNS_9allocatorIcEEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIcNS_9allocatorIcEEE8__appendEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIcNS_9allocatorIcEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIcNS_9allocatorIcEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN6shazam10link_bandsIL8AlgoType1EEEDaRKNS_6linkerIXT_EEERKNSt3__16vectorINS8_I15spectral_peak_tNS7_9allocatorIS9_EEEENSA_ISC_EEEENS7_6chrono8durationIjNS7_5ratioILl1ELl125EEEEERK19link_matrix_optionsm(void) {
    if (verbose) puts("STUB: _ZN6shazam10link_bandsIL8AlgoType1EEEDaRKNS_6linkerIXT_EEERKNSt3__16vectorINS8_I15spectral_peak_tNS7_9allocatorIS9_EEEENSA_ISC_EEEENS7_6chrono8durationIjNS7_5ratioILl1ELl125EEEEERK19link_matrix_optionsm called");
    return NULL;
)

void *_ZN6shazam10matrix_forIL8AlgoType1ELPv0EEENS_14link_matrix_2dERK19link_matrix_optionsNSt3__16chrono8durationIjNS7_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam10matrix_forIL8AlgoType1ELPv0EEENS_14link_matrix_2dERK19link_matrix_optionsNSt3__16chrono8durationIjNS7_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam4lmfpILNS1_7fp_typeE0EEENS_9allocatorIS4_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN6shazam6kvsortIN3gsl4spanINS_4lmfpILNS_7fp_typeE0EEELln1EEEEEvRT_S8_(void) {
    if (verbose) puts("STUB: _ZN6shazam6kvsortIN3gsl4spanINS_4lmfpILNS_7fp_typeE0EEELln1EEEEEvRT_S8_ called");
    return NULL;
)

void *_ZNSt3__16vectorIcNS_9allocatorIcEEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIcNS_9allocatorIcEEEC2Em called");
    return NULL;
)

void *_ZNSt3__16vectorIcNS_9allocatorIcEEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIcNS_9allocatorIcEEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIjNS_9allocatorIjEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIjNS_9allocatorIjEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__115allocate_sharedIN6shazam12SongDataBaseENS_9allocatorIS2_EEJEvEENS_10shared_ptrIT_EERKT0_DpOT1_(void) {
    if (verbose) puts("STUB: _ZNSt3__115allocate_sharedIN6shazam12SongDataBaseENS_9allocatorIS2_EEJEvEENS_10shared_ptrIT_EERKT0_DpOT1_ called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEEC2IJEEES4_DpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEEC2IJEEES4_DpOT_ called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEED1Ev called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEED0Ev called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEE16__on_zero_sharedEv(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEE16__on_zero_sharedEv called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEE21__on_zero_shared_weakEv(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam12SongDataBaseENS_9allocatorIS2_EEE21__on_zero_shared_weakEv called");
    return NULL;
)

void *_ZNSt3__115allocate_sharedIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEJEvEENS_10shared_ptrIT_EERKT0_DpOT1_(void) {
    if (verbose) puts("STUB: _ZNSt3__115allocate_sharedIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEJEvEENS_10shared_ptrIT_EERKT0_DpOT1_ called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEEC2IJEEES4_DpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEEC2IJEEES4_DpOT_ called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEED1Ev called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEED0Ev called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEE16__on_zero_sharedEv(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEE16__on_zero_sharedEv called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEE21__on_zero_shared_weakEv(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceIN6shazam16CPUOnePtDatabaseENS_9allocatorIS2_EEE21__on_zero_shared_weakEv called");
    return NULL;
)

void *_ZNSt3__16vectorI10SearchPlanNS_9allocatorIS1_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI10SearchPlanNS_9allocatorIS1_EEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseI10SearchPlanNS_9allocatorIS1_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseI10SearchPlanNS_9allocatorIS1_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_IcNS_9allocatorIcEEEENS1_IS3_EEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_IcNS_9allocatorIcEEEENS1_IS3_EEEC2Em called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_IcNS_9allocatorIcEEEENS1_IS3_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_IcNS_9allocatorIcEEEENS1_IS3_EEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseINS_6vectorIcNS_9allocatorIcEEEENS2_IS4_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseINS_6vectorIcNS_9allocatorIcEEEENS2_IS4_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZN6shazam4cropILNS_17signature_densityE16EEENS_15basic_signatureIXT_EEERKS3_NSt3__16chrono8durationIjNS6_5ratioILl1ELl125EEEEESB_(void) {
    if (verbose) puts("STUB: _ZN6shazam4cropILNS_17signature_densityE16EEENS_15basic_signatureIXT_EEERKS3_NSt3__16chrono8durationIjNS6_5ratioILl1ELl125EEEEESB_ called");
    return NULL;
)

void *_ZN6shazam9crop_bandI15spectral_peak_tEEvN3gsl4spanIKT_Lln1EEERNSt3__16vectorIS4_NS7_9allocatorIS4_EEEENS7_6chrono8durationIjNS7_5ratioILl1ELl125EEEEESH_(void) {
    if (verbose) puts("STUB: _ZN6shazam9crop_bandI15spectral_peak_tEEvN3gsl4spanIKT_Lln1EEERNSt3__16vectorIS4_NS7_9allocatorIS4_EEEENS7_6chrono8durationIjNS7_5ratioILl1ELl125EEEEESH_ called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEE11__vallocateEm called");
    return NULL;
)

void *_ZNSt3__113__vector_baseINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEENS3_IS5_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEENS3_IS5_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__16vectorI7peak_tfNS_9allocatorIS1_EEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI7peak_tfNS_9allocatorIS1_EEE7reserveEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseI7peak_tfNS_9allocatorIS1_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseI7peak_tfNS_9allocatorIS1_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__16__sortIRNS_6__lessI7peak_tfS2_EEPS2_EEvT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__16__sortIRNS_6__lessI7peak_tfS2_EEPS2_EEvT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort3IRNS_6__lessI7peak_tfS2_EEPS2_EEjT0_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort3IRNS_6__lessI7peak_tfS2_EEPS2_EEjT0_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort4IRNS_6__lessI7peak_tfS2_EEPS2_EEjT0_S6_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort4IRNS_6__lessI7peak_tfS2_EEPS2_EEjT0_S6_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__17__sort5IRNS_6__lessI7peak_tfS2_EEPS2_EEjT0_S6_S6_S6_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__17__sort5IRNS_6__lessI7peak_tfS2_EEPS2_EEjT0_S6_S6_S6_S6_T_ called");
    return NULL;
)

void *_ZNSt3__118__insertion_sort_3IRNS_6__lessI7peak_tfS2_EEPS2_EEvT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__118__insertion_sort_3IRNS_6__lessI7peak_tfS2_EEPS2_EEvT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessI7peak_tfS2_EEPS2_EEbT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__127__insertion_sort_incompleteIRNS_6__lessI7peak_tfS2_EEPS2_EEbT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__16vectorI7peak_tfNS_9allocatorIS1_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI7peak_tfNS_9allocatorIS1_EEE8__appendEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIyNS_9allocatorIyEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIyNS_9allocatorIyEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseINS_6chrono8durationIfNS_5ratioILl1ELl1EEEEENS_9allocatorIS5_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseINS_6chrono8durationIfNS_5ratioILl1ELl1EEEEENS_9allocatorIS5_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIhNS_9allocatorIhEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIhNS_9allocatorIhEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseINS_6chrono8durationIxNS_5ratioILl1ELl1000EEEEENS_9allocatorIS5_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseINS_6chrono8durationIxNS_5ratioILl1ELl1000EEEEENS_9allocatorIS5_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEED1Ev called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEED0Ev called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEE16__on_zero_sharedEv(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEE16__on_zero_sharedEv called");
    return NULL;
)

void *_ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEE21__on_zero_shared_weakEv(void) {
    if (verbose) puts("STUB: _ZNSt3__120__shared_ptr_emplaceINS_6vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS3_EEEENS4_IS6_EEE21__on_zero_shared_weakEv called");
    return NULL;
)

void *_ZNSt3__113__vector_baseIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE22__construct_one_at_endIJR10SearchPlanRNS0_IcNS3_IcEEEEEEEvDpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE22__construct_one_at_endIJR10SearchPlanRNS0_IcNS3_IcEEEEEEEvDpOT_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE24__emplace_back_slow_pathIJR10SearchPlanRNS0_IcNS3_IcEEEEEEEvDpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE24__emplace_back_slow_pathIJR10SearchPlanRNS0_IcNS3_IcEEEEEEEvDpOT_ called");
    return NULL;
)

void *_ZN3gsl4spanIcLln1EEC1INSt3__16vectorIcNS3_9allocatorIcEEEEvEERT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIcLln1EEC1INSt3__16vectorIcNS3_9allocatorIcEEEEvEERT_ called");
    return NULL;
)

void *_ZN3gsl4spanIcLln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPcT_(void) {
    if (verbose) puts("STUB: _ZN3gsl4spanIcLln1EE12storage_typeINS_7details11extent_typeILln1EEEEC2IlEEPcT_ called");
    return NULL;
)

void *_ZN6shazam15CPULMFPDataBaseC2ERK10SearchPlanN3gsl4spanIcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam15CPULMFPDataBaseC2ERK10SearchPlanN3gsl4spanIcLln1EEE called");
    return NULL;
)

void *_Z5visitIN6shazam15CPULMFPDataBase12prefix_stripEJRN3gsl4spanIcLln1EEEEEDa8AlgoTypeOT_DpOT0_(void) {
    if (verbose) puts("STUB: _Z5visitIN6shazam15CPULMFPDataBase12prefix_stripEJRN3gsl4spanIcLln1EEEEEDa8AlgoTypeOT_DpOT0_ called");
    return NULL;
)

void *_Z5visitIN6shazam15CPULMFPDataBase14landmarks_initEJRN3gsl4spanIcLln1EEEEEDa8AlgoTypeOT_DpOT0_(void) {
    if (verbose) puts("STUB: _Z5visitIN6shazam15CPULMFPDataBase14landmarks_initEJRN3gsl4spanIcLln1EEEEEDa8AlgoTypeOT_DpOT0_ called");
    return NULL;
)

void *_Z5visitIN6shazam15CPULMFPDataBase11ptable_initEJRN3gsl4spanIcLln1EEEEEDa8AlgoTypeOT_DpOT0_(void) {
    if (verbose) puts("STUB: _Z5visitIN6shazam15CPULMFPDataBase11ptable_initEJRN3gsl4spanIcLln1EEEEEDa8AlgoTypeOT_DpOT0_ called");
    return NULL;
)

void *_ZNK6shazam15CPULMFPDataBase12prefix_stripclIL8AlgoType1EEEDaN3gsl4spanIcLln1EEE(void) {
    if (verbose) puts("STUB: _ZNK6shazam15CPULMFPDataBase12prefix_stripclIL8AlgoType1EEEDaN3gsl4spanIcLln1EEE called");
    return NULL;
)

void *_ZN6shazam15getLMFPSuffixesIL8AlgoType1EEENSt3__16vectorIcNS2_9allocatorIcEEEERKN3gsl4spanINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam15getLMFPSuffixesIL8AlgoType1EEENSt3__16vectorIcNS2_9allocatorIcEEEERKN3gsl4spanINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEE called");
    return NULL;
)

void *_ZN6shazam10lmfpAsAlgoIL8AlgoType1EEEDaRN3gsl4spanIcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam10lmfpAsAlgoIL8AlgoType1EEEDaRN3gsl4spanIcLln1EEE called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam9fp_suffixIL8AlgoType1EEENS_9allocatorIS4_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam9fp_suffixIL8AlgoType1EEENS_9allocatorIS4_EEE6resizeEm called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam9fp_suffixIL8AlgoType1EEENS_9allocatorIS4_EEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam9fp_suffixIL8AlgoType1EEENS_9allocatorIS4_EEE8__appendEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam9fp_suffixIL8AlgoType1EEENS_9allocatorIS4_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam9fp_suffixIL8AlgoType1EEENS_9allocatorIS4_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNK6shazam15CPULMFPDataBase14landmarks_initclIL8AlgoType1EEEDaN3gsl4spanIcLln1EEE(void) {
    if (verbose) puts("STUB: _ZNK6shazam15CPULMFPDataBase14landmarks_initclIL8AlgoType1EEEDaN3gsl4spanIcLln1EEE called");
    return NULL;
)

void *_ZN6shazam12getLandmarksIL8AlgoType1EEENSt3__16vectorIjNS2_9allocatorIjEEEERKN3gsl4spanINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam12getLandmarksIL8AlgoType1EEENSt3__16vectorIjNS2_9allocatorIjEEEERKN3gsl4spanINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEE called");
    return NULL;
)

void *_ZNSt3__16vectorIjNS_9allocatorIjEEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIjNS_9allocatorIjEEE6resizeEm called");
    return NULL;
)

void *_ZNSt3__16vectorIjNS_9allocatorIjEEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIjNS_9allocatorIjEEE8__appendEm called");
    return NULL;
)

void *_ZNK6shazam15CPULMFPDataBase11ptable_initclIL8AlgoType1EEEDaN3gsl4spanIcLln1EEE(void) {
    if (verbose) puts("STUB: _ZNK6shazam15CPULMFPDataBase11ptable_initclIL8AlgoType1EEEDaN3gsl4spanIcLln1EEE called");
    return NULL;
)

void *_ZN6shazam14genPrefixTableIL8AlgoType1EEENSt3__16vectorIjNS2_9allocatorIjEEEERKN3gsl4spanINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam14genPrefixTableIL8AlgoType1EEENSt3__16vectorIjNS2_9allocatorIjEEEERKN3gsl4spanINS_4lmfpIXsr4algoIXT_EEE6fpTypeEEELln1EEE called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS2_RS4_EE(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS2_RS4_EE called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam15CPULMFPDataBaseENS_9allocatorIS2_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam15CPULMFPDataBaseERNS_9allocatorIS2_EEEC2EmmS5_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam15CPULMFPDataBaseERNS_9allocatorIS2_EEEC2EmmS5_ called");
    return NULL;
)

void *_ZNSt3__146__construct_backward_with_exception_guaranteesINS_9allocatorIN6shazam15CPULMFPDataBaseEEEPS3_EEvRT_T0_S8_RS8_(void) {
    if (verbose) puts("STUB: _ZNSt3__146__construct_backward_with_exception_guaranteesINS_9allocatorIN6shazam15CPULMFPDataBaseEEEPS3_EEvRT_T0_S8_RS8_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam15CPULMFPDataBaseERNS_9allocatorIS2_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam15CPULMFPDataBaseERNS_9allocatorIS2_EEED2Ev called");
    return NULL;
)

void *__chkstk_darwin(void) {
    if (verbose) puts("STUB: __chkstk_darwin called");
    return NULL;
)

void *__chkstk_darwin_llvm_probe(void) {
    if (verbose) puts("STUB: __chkstk_darwin_llvm_probe called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tEC2E14signature_typei17signature_options(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tEC2E14signature_typei17signature_options called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE15getStreamingSigEN6shazam17signature_densityE(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE15getStreamingSigEN6shazam17signature_densityE called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE22set_max_rolling_bufferEf(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE22set_max_rolling_bufferEf called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE14getWindowedSigEN6shazam17signature_densityE(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE14getWindowedSigEN6shazam17signature_densityE called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE17getProgressiveSigEN6shazam17signature_densityE(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE17getProgressiveSigEN6shazam17signature_densityE called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tEC1E14signature_typei17signature_options(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tEC1E14signature_typei17signature_options called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE20stream_flow_internalEPKfi(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE20stream_flow_internalEPKfi called");
    return NULL;
)

void *_ZN3gsl9make_spanIKfEENS_4spanIT_Lln1EEEPS3_NS4_10index_typeE(void) {
    if (verbose) puts("STUB: _ZN3gsl9make_spanIKfEENS_4spanIT_Lln1EEEPS3_NS4_10index_typeE called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE17flow_single_frameEPKf(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE17flow_single_frameEPKf called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEE6resizeEm called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE11stream_flowEPKsi(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE11stream_flowEPKsi called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE11stream_flowEPKfi(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE11stream_flowEPKfi called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE16get_sample_countEv(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE16get_sample_countEv called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE19get_samples_secondsEv(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE19get_samples_secondsEv called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE13get_signatureERNSt3__16vectorIcNS2_9allocatorIcEEEE(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE13get_signatureERNSt3__16vectorIcNS2_9allocatorIcEEEE called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE5resetEv(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE5resetEv called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE19get_spectral_outputEv(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE19get_spectral_outputEv called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tE9flush_fftEv(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tE9flush_fftEv called");
    return NULL;
)

void *_ZN12_GLOBAL__N_17get_sigERKNSt3__15dequeI15spectral_peak_tNS0_9allocatorIS2_EEEERKN6shazam14signature_infoENS8_17signature_densityEbm(void) {
    if (verbose) puts("STUB: _ZN12_GLOBAL__N_17get_sigERKNSt3__15dequeI15spectral_peak_tNS0_9allocatorIS2_EEEERKN6shazam14signature_infoENS8_17signature_densityEbm called");
    return NULL;
)

void *_Z12window_peaksI15spectral_peak_tENSt3__15dequeIT_NS1_9allocatorIS3_EEEERS6_f(void) {
    if (verbose) puts("STUB: _Z12window_peaksI15spectral_peak_tENSt3__15dequeIT_NS1_9allocatorIS3_EEEERS6_f called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tED1Ev(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tED1Ev called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tED0Ev(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tED0Ev called");
    return NULL;
)

void *_ZN10fft_streamC2Ev(void) {
    if (verbose) puts("STUB: _ZN10fft_streamC2Ev called");
    return NULL;
)

void *_ZNSt3__112__deque_baseI15spectral_peak_tNS_9allocatorIS1_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__112__deque_baseI15spectral_peak_tNS_9allocatorIS1_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__112__deque_baseI15spectral_peak_tNS_9allocatorIS1_EEE5clearEv(void) {
    if (verbose) puts("STUB: _ZNSt3__112__deque_baseI15spectral_peak_tNS_9allocatorIS1_EEE5clearEv called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIP15spectral_peak_tNS_9allocatorIS2_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIP15spectral_peak_tNS_9allocatorIS2_EEED2Ev called");
    return NULL;
)

void *_ZNK3gsl4spanIKfLln1EE12make_subspanEllNS2_16subspan_selectorILln1EEE(void) {
    if (verbose) puts("STUB: _ZNK3gsl4spanIKfLln1EE12make_subspanEllNS2_16subspan_selectorILln1EEE called");
    return NULL;
)

void *_Z17BandSpectralPeaksINSt3__116__deque_iteratorI15spectral_peak_tPKS2_RS3_PKS4_lLl512EEENS0_6vectorINS9_IS2_NS0_9allocatorIS2_EEEENSA_ISC_EEEEET0_T_SG_N6shazam17signature_densityEmb(void) {
    if (verbose) puts("STUB: _Z17BandSpectralPeaksINSt3__116__deque_iteratorI15spectral_peak_tPKS2_RS3_PKS4_lLl512EEENS0_6vectorINS9_IS2_NS0_9allocatorIS2_EEEENSA_ISC_EEEEET0_T_SG_N6shazam17signature_densityEmb called");
    return NULL;
)

void *_Z14addPeakToBandsI15spectral_peak_tNSt3__16vectorINS2_IS0_NS1_9allocatorIS0_EEEENS3_IS5_EEEEEvRKT_RT0_(void) {
    if (verbose) puts("STUB: _Z14addPeakToBandsI15spectral_peak_tNSt3__16vectorINS2_IS0_NS1_9allocatorIS0_EEEENS3_IS5_EEEEEvRKT_RT0_ called");
    return NULL;
)

void *_ZNSt3__115is_sorted_untilINS_16__deque_iteratorI15spectral_peak_tPKS2_RS3_PKS4_lLl512EEENS_6__lessIS2_S2_EEEET_SB_SB_T0_(void) {
    if (verbose) puts("STUB: _ZNSt3__115is_sorted_untilINS_16__deque_iteratorI15spectral_peak_tPKS2_RS3_PKS4_lLl512EEENS_6__lessIS2_S2_EEEET_SB_SB_T0_ called");
    return NULL;
)

void *_ZNSt3__16vectorImNS_9allocatorImEEEC2EmRKm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorImNS_9allocatorImEEEC2EmRKm called");
    return NULL;
)

void *_ZNSt3__16vectorImNS_9allocatorImEEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorImNS_9allocatorImEEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseImNS_9allocatorImEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseImNS_9allocatorImEEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEEC2Em called");
    return NULL;
)

void *_ZNSt3__16__sortIRNS_6__lessI15spectral_peak_tS2_EEPS2_EEvT0_S6_T_(void) {
    if (verbose) puts("STUB: _ZNSt3__16__sortIRNS_6__lessI15spectral_peak_tS2_EEPS2_EEvT0_S6_T_ called");
    return NULL;
)

void *_ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEEC2ERKS6_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorINS0_I15spectral_peak_tNS_9allocatorIS1_EEEENS2_IS4_EEEC2ERKS6_ called");
    return NULL;
)

void *_ZN18sig_extractor_pipeI15spectral_peak_tED2Ev(void) {
    if (verbose) puts("STUB: _ZN18sig_extractor_pipeI15spectral_peak_tED2Ev called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE26__maybe_remove_front_spareEb(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE26__maybe_remove_front_spareEb called");
    return NULL;
)

void *_ZNSt3__16vectorIfNS_9allocatorIfEEE8__appendEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIfNS_9allocatorIfEEE8__appendEm called");
    return NULL;
)

void *_ZN3gsl12final_actionIZN18sig_extractor_pipeI15spectral_peak_tE17getProgressiveSigEN6shazam17signature_densityEEUlvE_ED2Ev(void) {
    if (verbose) puts("STUB: _ZN3gsl12final_actionIZN18sig_extractor_pipeI15spectral_peak_tE17getProgressiveSigEN6shazam17signature_densityEEUlvE_ED2Ev called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE5eraseENS_16__deque_iteratorIS1_PKS1_RS6_PKS7_lLl512EEESB_(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE5eraseENS_16__deque_iteratorIS1_PKS1_RS6_PKS7_lLl512EEESB_ called");
    return NULL;
)

void *_ZNKSt3__116__deque_iteratorI15spectral_peak_tPS1_RS1_PS2_lLl512EEplEl(void) {
    if (verbose) puts("STUB: _ZNKSt3__116__deque_iteratorI15spectral_peak_tPS1_RS1_PS2_lLl512EEplEl called");
    return NULL;
)

void *_ZNSt3__113move_backwardI15spectral_peak_tPS1_RS1_PS2_lLl512ES1_S2_S3_S4_lLl512EEENS_16__deque_iteratorIT5_T6_T7_T8_T9_XT10_EEENS5_IT_T0_T1_T2_T3_XT4_EEESH_SB_(void) {
    if (verbose) puts("STUB: _ZNSt3__113move_backwardI15spectral_peak_tPS1_RS1_PS2_lLl512ES1_S2_S3_S4_lLl512EEENS_16__deque_iteratorIT5_T6_T7_T8_T9_XT10_EEENS5_IT_T0_T1_T2_T3_XT4_EEESH_SB_ called");
    return NULL;
)

void *_ZNSt3__14moveI15spectral_peak_tPS1_RS1_PS2_lLl512ES1_S2_S3_S4_lLl512EEENS_16__deque_iteratorIT5_T6_T7_T8_T9_XT10_EEENS5_IT_T0_T1_T2_T3_XT4_EEESH_SB_(void) {
    if (verbose) puts("STUB: _ZNSt3__14moveI15spectral_peak_tPS1_RS1_PS2_lLl512ES1_S2_S3_S4_lLl512EEENS_16__deque_iteratorIT5_T6_T7_T8_T9_XT10_EEENS5_IT_T0_T1_T2_T3_XT4_EEESH_SB_ called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE25__maybe_remove_back_spareEb(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE25__maybe_remove_back_spareEb called");
    return NULL;
)

void *_ZNSt3__113move_backwardIP15spectral_peak_tS1_S2_RS1_PS2_lLl512EEENS_16__deque_iteratorIT0_T1_T2_T3_T4_XT5_EEET_SC_SB_PNS_9enable_ifIXsr33__is_cpp17_random_access_iteratorISC_EE5valueEvE4typeE(void) {
    if (verbose) puts("STUB: _ZNSt3__113move_backwardIP15spectral_peak_tS1_S2_RS1_PS2_lLl512EEENS_16__deque_iteratorIT0_T1_T2_T3_T4_XT5_EEET_SC_SB_PNS_9enable_ifIXsr33__is_cpp17_random_access_iteratorISC_EE5valueEvE4typeE called");
    return NULL;
)

void *_ZNSt3__14moveIP15spectral_peak_tS1_S2_RS1_PS2_lLl512EEENS_16__deque_iteratorIT0_T1_T2_T3_T4_XT5_EEET_SC_SB_PNS_9enable_ifIXsr33__is_cpp17_random_access_iteratorISC_EE5valueEvE4typeE(void) {
    if (verbose) puts("STUB: _ZNSt3__14moveIP15spectral_peak_tS1_S2_RS1_PS2_lLl512EEENS_16__deque_iteratorIT0_T1_T2_T3_T4_XT5_EEET_SC_SB_PNS_9enable_ifIXsr33__is_cpp17_random_access_iteratorISC_EE5valueEvE4typeE called");
    return NULL;
)

void *_ZN3gsl12final_actionIZN18sig_extractor_pipeI15spectral_peak_tE14getWindowedSigEN6shazam17signature_densityEEUlvE_ED2Ev(void) {
    if (verbose) puts("STUB: _ZN3gsl12final_actionIZN18sig_extractor_pipeI15spectral_peak_tE14getWindowedSigEN6shazam17signature_densityEEUlvE_ED2Ev called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEEC2ERKS4_(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEEC2ERKS4_ called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE8__appendINS_16__deque_iteratorIS1_PKS1_RS7_PKS8_lLl512EEEEEvT_SD_PNS_9enable_ifIXsr27__is_cpp17_forward_iteratorISD_EE5valueEvE4typeE(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE8__appendINS_16__deque_iteratorIS1_PKS1_RS7_PKS8_lLl512EEEEEvT_SD_PNS_9enable_ifIXsr27__is_cpp17_forward_iteratorISD_EE5valueEvE4typeE called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE19__add_back_capacityEm(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE19__add_back_capacityEm called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIP15spectral_peak_tNS_9allocatorIS2_EEE9push_backEOS2_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIP15spectral_peak_tNS_9allocatorIS2_EEE9push_backEOS2_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIP15spectral_peak_tNS_9allocatorIS2_EEE10push_frontEOS2_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIP15spectral_peak_tNS_9allocatorIS2_EEE10push_frontEOS2_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIP15spectral_peak_tRNS_9allocatorIS2_EEE9push_backEOS2_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIP15spectral_peak_tRNS_9allocatorIS2_EEE9push_backEOS2_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIP15spectral_peak_tRNS_9allocatorIS2_EEE10push_frontERKS2_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIP15spectral_peak_tRNS_9allocatorIS2_EEE10push_frontERKS2_ called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEEC2INS_16__deque_iteratorIS1_PS1_RS1_PS7_lLl512EEEEET_SB_PNS_9enable_ifIXsr25__is_cpp17_input_iteratorISB_EE5valueEvE4typeE(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEEC2INS_16__deque_iteratorIS1_PS1_RS1_PS7_lLl512EEEEET_SB_PNS_9enable_ifIXsr25__is_cpp17_input_iteratorISB_EE5valueEvE4typeE called");
    return NULL;
)

void *_ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE8__appendINS_16__deque_iteratorIS1_PS1_RS1_PS7_lLl512EEEEEvT_SB_PNS_9enable_ifIXsr27__is_cpp17_forward_iteratorISB_EE5valueEvE4typeE(void) {
    if (verbose) puts("STUB: _ZNSt3__15dequeI15spectral_peak_tNS_9allocatorIS1_EEE8__appendINS_16__deque_iteratorIS1_PS1_RS1_PS7_lLl512EEEEEvT_SB_PNS_9enable_ifIXsr27__is_cpp17_forward_iteratorISB_EE5valueEvE4typeE called");
    return NULL;
)

void *_ZN3gsl9make_spanIfEENS_4spanIT_Lln1EEEPS2_NS3_10index_typeE(void) {
    if (verbose) puts("STUB: _ZN3gsl9make_spanIfEENS_4spanIT_Lln1EEEPS2_NS3_10index_typeE called");
    return NULL;
)

void *_Z13get_peak_bandf(void) {
    if (verbose) puts("STUB: _Z13get_peak_bandf called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE7reserveEm called");
    return NULL;
)

void *_Z13chooseAnchorsRKNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEEEjRKN6shazam10frame_infoERS4_(void) {
    if (verbose) puts("STUB: _Z13chooseAnchorsRKNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEEEjRKN6shazam10frame_infoERS4_ called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEEC2ERKS4_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEEC2ERKS4_ called");
    return NULL;
)

void *_ZN6shazam18SumoHeaderIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam18SumoHeaderIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZN10fft_stream5resetEv(void) {
    if (verbose) puts("STUB: _ZN10fft_stream5resetEv called");
    return NULL;
)

void *_ZN10fft_stream10checkpointEv(void) {
    if (verbose) puts("STUB: _ZN10fft_stream10checkpointEv called");
    return NULL;
)

void *_ZN10fft_stream6rewindEv(void) {
    if (verbose) puts("STUB: _ZN10fft_stream6rewindEv called");
    return NULL;
)

void *_ZN10fft_stream11stream_flowEPKfPfPNSt3__17complexIfEE(void) {
    if (verbose) puts("STUB: _ZN10fft_stream11stream_flowEPKfPfPNSt3__17complexIfEE called");
    return NULL;
)

void *_ZNSt3__113basic_fstreamIcNS_11char_traitsIcEEED1Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113basic_fstreamIcNS_11char_traitsIcEEED1Ev called");
    return NULL;
)

void *_ZThn16_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED1Ev(void) {
    if (verbose) puts("STUB: _ZThn16_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED1Ev called");
    return NULL;
)

void *_ZTv0_n24_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED1Ev(void) {
    if (verbose) puts("STUB: _ZTv0_n24_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED1Ev called");
    return NULL;
)

void *_ZNSt3__113basic_fstreamIcNS_11char_traitsIcEEED0Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113basic_fstreamIcNS_11char_traitsIcEEED0Ev called");
    return NULL;
)

void *_ZThn16_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED0Ev(void) {
    if (verbose) puts("STUB: _ZThn16_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED0Ev called");
    return NULL;
)

void *_ZTv0_n24_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED0Ev(void) {
    if (verbose) puts("STUB: _ZTv0_n24_NSt3__113basic_fstreamIcNS_11char_traitsIcEEED0Ev called");
    return NULL;
)

void *_ZN6shazam3ccp26validateFirstClusterCmdPktERKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam3ccp26validateFirstClusterCmdPktERKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPK13ClusterCmdPktLi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPK13ClusterCmdPktLi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam3ccp25validateNextClusterCmdPktERKN3gsl4spanIKcLln1EEEPK13ClusterCmdPktmm(void) {
    if (verbose) puts("STUB: _ZN6shazam3ccp25validateNextClusterCmdPktERKN3gsl4spanIKcLln1EEEPK13ClusterCmdPktmm called");
    return NULL;
)

void *_ZN6shazam3ccp16validateChecksumEPK13ClusterCmdPktPKcS5_(void) {
    if (verbose) puts("STUB: _ZN6shazam3ccp16validateChecksumEPK13ClusterCmdPktPKcS5_ called");
    return NULL;
)

void *_ZN6shazam21ClusterCmdPktIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam21ClusterCmdPktIteratorEN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZN3gsl9make_spanIcEENS_4spanIT_Lln1EEEPS2_NS3_10index_typeE(void) {
    if (verbose) puts("STUB: _ZN3gsl9make_spanIcEENS_4spanIT_Lln1EEEPS2_NS3_10index_typeE called");
    return NULL;
)

void *_ZN6shazam15iterate_headersINS_28fp_data_header_extern_legacyEEEvN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam15iterate_headersINS_28fp_data_header_extern_legacyEEEvN3gsl4spanIKcLln1EEERKNSt3__18functionIFvNS_16signature_headerEEEE called");
    return NULL;
)

void *_ZNK3gsl4spanIKcLln1EE12make_subspanEllNS2_16subspan_selectorILln1EEE(void) {
    if (verbose) puts("STUB: _ZNK3gsl4spanIKcLln1EE12make_subspanEllNS2_16subspan_selectorILln1EEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPKNS_28fp_data_header_extern_legacyELi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPKNS_28fp_data_header_extern_legacyELi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN16LegacyFatSigInfoC1Ej(void) {
    if (verbose) puts("STUB: _ZN16LegacyFatSigInfoC1Ej called");
    return NULL;
)

void *_ZN16LegacyFatSigInfo23set_audio_sample_lengthEy(void) {
    if (verbose) puts("STUB: _ZN16LegacyFatSigInfo23set_audio_sample_lengthEy called");
    return NULL;
)

void *_ZNK16LegacyFatSigInfo25get_sample_length_samplesEv(void) {
    if (verbose) puts("STUB: _ZNK16LegacyFatSigInfo25get_sample_length_samplesEv called");
    return NULL;
)

void *_ZNK16LegacyFatSigInfo19get_raw_sample_rateEv(void) {
    if (verbose) puts("STUB: _ZNK16LegacyFatSigInfo19get_raw_sample_rateEv called");
    return NULL;
)

void *_ZN16LegacyFatSigInfo14set_stream_posENSt3__16chrono8durationIyNS0_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN16LegacyFatSigInfo14set_stream_posENSt3__16chrono8durationIyNS0_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZNK16LegacyFatSigInfo14get_stream_posEv(void) {
    if (verbose) puts("STUB: _ZNK16LegacyFatSigInfo14get_stream_posEv called");
    return NULL;
)

void *_ZN11SHAZAM_SIGXC2E14signature_typej17signature_options(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGXC2E14signature_typej17signature_options called");
    return NULL;
)

void *_ZN11SHAZAM_SIGXC1E14signature_typej17signature_options(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGXC1E14signature_typej17signature_options called");
    return NULL;
)

void *_ZN11SHAZAM_SIGXD1Ev(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGXD1Ev called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX4flowEPKsi(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX4flowEPKsi called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX4flowEPsii(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX4flowEPsii called");
    return NULL;
)

void *_ZN6shazam5audio7downMixIsEEvPT_mm(void) {
    if (verbose) puts("STUB: _ZN6shazam5audio7downMixIsEEvPT_mm called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX4flowEPKfi(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX4flowEPKfi called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX4flowEPfii(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX4flowEPfii called");
    return NULL;
)

void *_ZN6shazam5audio7downMixIfEEvPT_mm(void) {
    if (verbose) puts("STUB: _ZN6shazam5audio7downMixIfEEvPT_mm called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX13get_signatureERNSt3__16vectorIcNS0_9allocatorIcEEEE(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX13get_signatureERNSt3__16vectorIcNS0_9allocatorIcEEEE called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX22set_max_rolling_bufferEf(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX22set_max_rolling_bufferEf called");
    return NULL;
)

void *_ZN11SHAZAM_SIGX5resetEv(void) {
    if (verbose) puts("STUB: _ZN11SHAZAM_SIGX5resetEv called");
    return NULL;
)

void *_ZNSt3__113__vector_baseINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEENS3_IS5_EEE5clearEv(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseINS_6vectorI15spectral_peak_tNS_9allocatorIS2_EEEENS3_IS5_EEE5clearEv called");
    return NULL;
)

void *_ZN6shazam15ShazamKitHeader8validateEN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam15ShazamKitHeader8validateEN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam17reinterpret_saferIPKNS_15ShazamKitHeaderELi0EEET_RKN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam17reinterpret_saferIPKNS_15ShazamKitHeaderELi0EEET_RKN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam15ShazamKitHeader3popEN3gsl4spanIKcLln1EEE(void) {
    if (verbose) puts("STUB: _ZN6shazam15ShazamKitHeader3popEN3gsl4spanIKcLln1EEE called");
    return NULL;
)

void *_ZN6shazam11unpack_timeERNS_2io6readerE(void) {
    if (verbose) puts("STUB: _ZN6shazam11unpack_timeERNS_2io6readerE called");
    return NULL;
)

void *_ZN6shazam2io14read_unalignedItLi0EEET_RNS0_6readerE(void) {
    if (verbose) puts("STUB: _ZN6shazam2io14read_unalignedItLi0EEET_RNS0_6readerE called");
    return NULL;
)

void *_ZN6shazam28inflate_basic_sumo_peak_dataEN3gsl4spanIKcLln1EEENS_17signature_densityENSt3__16chrono8durationIhNS5_5ratioILl1ELl125EEEEE(void) {
    if (verbose) puts("STUB: _ZN6shazam28inflate_basic_sumo_peak_dataEN3gsl4spanIKcLln1EEENS_17signature_densityENSt3__16chrono8durationIhNS5_5ratioILl1ELl125EEEEE called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEE7reserveEm called");
    return NULL;
)

void *_ZN6shazam14density_filterI15spectral_peak_tElsERKS1_(void) {
    if (verbose) puts("STUB: _ZN6shazam14density_filterI15spectral_peak_tElsERKS1_ called");
    return NULL;
)

void *_ZNSt12out_of_rangeC1EPKc(void) {
    if (verbose) puts("STUB: _ZNSt12out_of_rangeC1EPKc called");
    return NULL;
)

void *_ZNSt3__113__vector_baseIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__113__vector_baseIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEE5clearEv(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEE5clearEv called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam14density_filterI15spectral_peak_tEERNS_9allocatorIS4_EEEC2EmmS7_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam14density_filterI15spectral_peak_tEERNS_9allocatorIS4_EEEC2EmmS7_ called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam14density_filterI15spectral_peak_tEERNS_9allocatorIS4_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam14density_filterI15spectral_peak_tEERNS_9allocatorIS4_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam14density_filterI15spectral_peak_tEERNS_9allocatorIS4_EEE17__destruct_at_endEPS4_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam14density_filterI15spectral_peak_tEERNS_9allocatorIS4_EEE17__destruct_at_endEPS4_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam14density_filterI15spectral_peak_tEENS_9allocatorIS4_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE6resizeEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorI15spectral_peak_tNS_9allocatorIS1_EEE6resizeEm called");
    return NULL;
)

void *_ZN6shazam9get_grainERKNSt3__16vectorIfNS0_9allocatorIfEEEES6_f(void) {
    if (verbose) puts("STUB: _ZN6shazam9get_grainERKNSt3__16vectorIfNS0_9allocatorIfEEEES6_f called");
    return NULL;
)

void *_ZN6shazam15blips_and_blopsERKNSt3__16vectorI15spectral_peak_tNS0_9allocatorIS2_EEEEi(void) {
    if (verbose) puts("STUB: _ZN6shazam15blips_and_blopsERKNSt3__16vectorI15spectral_peak_tNS0_9allocatorIS2_EEEEi called");
    return NULL;
)

void *_ZN6shazam16sonify_signatureERKNS_17basic_signature_tEj(void) {
    if (verbose) puts("STUB: _ZN6shazam16sonify_signatureERKNS_17basic_signature_tEj called");
    return NULL;
)

void *_ZNSt3__16vectorIsNS_9allocatorIsEEEC2Em(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIsNS_9allocatorIsEEEC2Em called");
    return NULL;
)

void *_ZNSt3__16vectorIsNS_9allocatorIsEEE11__vallocateEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIsNS_9allocatorIsEEE11__vallocateEm called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIsNS_9allocatorIsEEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIsNS_9allocatorIsEEE20__throw_length_errorEv called");
    return NULL;
)

void *__clang_call_terminate(void) {
    if (verbose) puts("STUB: __clang_call_terminate called");
    return NULL;
)

void *_ZN3gsl9make_spanIKcEENS_4spanIT_Lln1EEEPS3_NS4_10index_typeE(void) {
    if (verbose) puts("STUB: _ZN3gsl9make_spanIKcEENS_4spanIT_Lln1EEEPS3_NS4_10index_typeE called");
    return NULL;
)

void *_ZN6shazam11make_vectorINS_15basic_signatureILNS_17signature_densityE24EEEJRKN3gsl4spanIKcLln1EEEEEEDaDpOT0_(void) {
    if (verbose) puts("STUB: _ZN6shazam11make_vectorINS_15basic_signatureILNS_17signature_densityE24EEEJRKN3gsl4spanIKcLln1EEEEEEDaDpOT0_ called");
    return NULL;
)

void *_ZN24ConstQueryConfigProviderD1Ev(void) {
    if (verbose) puts("STUB: _ZN24ConstQueryConfigProviderD1Ev called");
    return NULL;
)

void *_ZNSt3__110unique_ptrIN6shazam4MMapENS_14default_deleteIS2_EEE5resetEPS2_(void) {
    if (verbose) puts("STUB: _ZNSt3__110unique_ptrIN6shazam4MMapENS_14default_deleteIS2_EEE5resetEPS2_ called");
    return NULL;
)

void *_ZN3gsl7details9terminateEv(void) {
    if (verbose) puts("STUB: _ZN3gsl7details9terminateEv called");
    return NULL;
)

void *_ZNSt3__113__vector_baseINS_6vectorIcNS_9allocatorIcEEEENS2_IS4_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseINS_6vectorIcNS_9allocatorIcEEEENS2_IS4_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__113__vector_baseINS_6vectorIcNS_9allocatorIcEEEENS2_IS4_EEE5clearEv(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseINS_6vectorIcNS_9allocatorIcEEEENS2_IS4_EEE5clearEv called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE7reserveEm(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE7reserveEm called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS4_RS6_EE(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS4_RS6_EE called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam15basic_signatureILNS1_17signature_densityE24EEERNS_9allocatorIS4_EEEC2EmmS7_(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam15basic_signatureILNS1_17signature_densityE24EEERNS_9allocatorIS4_EEEC2EmmS7_ called");
    return NULL;
)

void *_ZNSt3__120__throw_length_errorEPKc(void) {
    if (verbose) puts("STUB: _ZNSt3__120__throw_length_errorEPKc called");
    return NULL;
)

void *_ZNSt3__114__split_bufferIN6shazam15basic_signatureILNS1_17signature_densityE24EEERNS_9allocatorIS4_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__114__split_bufferIN6shazam15basic_signatureILNS1_17signature_densityE24EEERNS_9allocatorIS4_EEED2Ev called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE22__construct_one_at_endIJRKN3gsl4spanIKcLln1EEEEEEvDpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE22__construct_one_at_endIJRKN3gsl4spanIKcLln1EEEEEEvDpOT_ called");
    return NULL;
)

void *_ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE24__emplace_back_slow_pathIJRKN3gsl4spanIKcLln1EEEEEEvDpOT_(void) {
    if (verbose) puts("STUB: _ZNSt3__16vectorIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE24__emplace_back_slow_pathIJRKN3gsl4spanIKcLln1EEEEEEvDpOT_ called");
    return NULL;
)

void *_ZNKSt3__113__vector_baseIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE20__throw_length_errorEv(void) {
    if (verbose) puts("STUB: _ZNKSt3__113__vector_baseIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEE20__throw_length_errorEv called");
    return NULL;
)

void *_ZNK24ConstQueryConfigProvider9getConfigEm(void) {
    if (verbose) puts("STUB: _ZNK24ConstQueryConfigProvider9getConfigEm called");
    return NULL;
)

void *_ZN24ConstQueryConfigProviderD0Ev(void) {
    if (verbose) puts("STUB: _ZN24ConstQueryConfigProviderD0Ev called");
    return NULL;
)

void *_ZNKSt3__114default_deleteI11MREInstanceEclEPS1_(void) {
    if (verbose) puts("STUB: _ZNKSt3__114default_deleteI11MREInstanceEclEPS1_ called");
    return NULL;
)

void *_ZNSt3__113__vector_baseIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEED2Ev(void) {
    if (verbose) puts("STUB: _ZNSt3__113__vector_baseIN6shazam15basic_signatureILNS1_17signature_densityE24EEENS_9allocatorIS4_EEED2Ev called");
    return NULL;
)

void *_ZN24FFT_implementation_appleC2Em(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_appleC2Em called");
    return NULL;
)

void *_ZN24FFT_implementation_appleC1Em(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_appleC1Em called");
    return NULL;
)

void *_ZN24FFT_implementation_appleD2Ev(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_appleD2Ev called");
    return NULL;
)

void *_ZN24FFT_implementation_appleD1Ev(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_appleD1Ev called");
    return NULL;
)

void *_ZN24FFT_implementation_appleD0Ev(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_appleD0Ev called");
    return NULL;
)

void *_ZN24FFT_implementation_apple8magxformEPfS0_(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_apple8magxformEPfS0_ called");
    return NULL;
)

void *_ZN24FFT_implementation_apple18toSHZFFT_MagnitudeEPf(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_apple18toSHZFFT_MagnitudeEPf called");
    return NULL;
)

void *_ZN24FFT_implementation_apple14magcomplexformEPfS0_PNSt3__17complexIfEE(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_apple14magcomplexformEPfS0_PNSt3__17complexIfEE called");
    return NULL;
)

void *_ZN24FFT_implementation_apple4fftrEPKfPNSt3__17complexIfEE(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_apple4fftrEPKfPNSt3__17complexIfEE called");
    return NULL;
)

void *_ZN24FFT_implementation_apple5fftriERKN3gsl4spanINSt3__17complexIfEELln1EEENS1_IfLln1EEE(void) {
    if (verbose) puts("STUB: _ZN24FFT_implementation_apple5fftriERKN3gsl4spanINSt3__17complexIfEELln1EEENS1_IfLln1EEE called");
    return NULL;
)

