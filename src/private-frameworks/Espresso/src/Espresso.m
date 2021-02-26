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


#include <Espresso/Espresso.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* _Z13global_loggerv(void)
{
    if (verbose) puts("STUB: _Z13global_loggerv called");
    return NULL;
}

void* _Z23espresso_vision_computeP15EspressoNetworkP15EspressoContextPK13vImage_BufferiPfPK12VimageParamsbii(void)
{
    if (verbose) puts("STUB: _Z23espresso_vision_computeP15EspressoNetworkP15EspressoContextPK13vImage_BufferiPfPK12VimageParamsbii called");
    return NULL;
}

void* _Z25espresso_plan_add_cpp_netPvNSt3__110shared_ptrIN8Espresso3netEEE(void)
{
    if (verbose) puts("STUB: _Z25espresso_plan_add_cpp_netPvNSt3__110shared_ptrIN8Espresso3netEEE called");
    return NULL;
}

void* _Z29FaceLandmarkDetectorPointMakeRKfS0_(void)
{
    if (verbose) puts("STUB: _Z29FaceLandmarkDetectorPointMakeRKfS0_ called");
    return NULL;
}

void* _Z29espresso_vision_compute_batchP15EspressoNetworkP15EspressoContextiiPK13vImage_BufferiiPfPK12VimageParamsb(void)
{
    if (verbose) puts("STUB: _Z29espresso_vision_compute_batchP15EspressoNetworkP15EspressoContextiiPK13vImage_BufferiiPfPK12VimageParamsb called");
    return NULL;
}

void* _Z29espresso_vision_shape_networkP15EspressoNetworkiiii(void)
{
    if (verbose) puts("STUB: _Z29espresso_vision_shape_networkP15EspressoNetworkiiii called");
    return NULL;
}

void* _Z38espresso_vision_compute_pupil_positionP15EspressoNetworkP13vImage_BufferPKf(void)
{
    if (verbose) puts("STUB: _Z38espresso_vision_compute_pupil_positionP15EspressoNetworkP13vImage_BufferPKf called");
    return NULL;
}

void* _Z39espresso_vision_compute_dropout_augmentP15EspressoNetworkP15EspressoContextPfPK13vImage_BufferiS3_PK12VimageParamsb(void)
{
    if (verbose) puts("STUB: _Z39espresso_vision_compute_dropout_augmentP15EspressoNetworkP15EspressoContextPfPK13vImage_BufferiS3_PK12VimageParamsb called");
    return NULL;
}

void* _Z42espresso_vision_util_update_metal_priorityP15EspressoContextbj(void)
{
    if (verbose) puts("STUB: _Z42espresso_vision_util_update_metal_priorityP15EspressoContextbj called");
    return NULL;
}

void* _Z43espresso_vision_classify_descriptor_handlerP15EspressoNetwork18EspressoVisionBlob(void)
{
    if (verbose) puts("STUB: _Z43espresso_vision_classify_descriptor_handlerP15EspressoNetwork18EspressoVisionBlob called");
    return NULL;
}

void* _Z49espresso_vision_first_layer_number_of_input_blobsP15EspressoNetwork(void)
{
    if (verbose) puts("STUB: _Z49espresso_vision_first_layer_number_of_input_blobsP15EspressoNetwork called");
    return NULL;
}

void* _Z50espresso_vision_classify_descriptor_handler_nsdataP15EspressoNetworkP6NSData(void)
{
    if (verbose) puts("STUB: _Z50espresso_vision_classify_descriptor_handler_nsdataP15EspressoNetworkP6NSData called");
    return NULL;
}

void* _Z51espresso_vision_first_layer_first_source_blob_shapeP15EspressoNetwork(void)
{
    if (verbose) puts("STUB: _Z51espresso_vision_first_layer_first_source_blob_shapeP15EspressoNetwork called");
    return NULL;
}

void* _Z55espresso_vision_last_layer_first_destination_blob_shapeP15EspressoNetwork(void)
{
    if (verbose) puts("STUB: _Z55espresso_vision_last_layer_first_destination_blob_shapeP15EspressoNetwork called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan13input_raw_ptrEiPKc(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan13input_raw_ptrEiPKc called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan14output_raw_ptrEiPKc(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan14output_raw_ptrEiPKc called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan15bind_dyn_outputEiPKc(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan15bind_dyn_outputEiPKc called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan17set_function_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan17set_function_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan17set_input_raw_ptrEiPKcRK17espresso_buffer_t(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan17set_input_raw_ptrEiPKcRK17espresso_buffer_t called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan18set_output_raw_ptrEiPKcRK17espresso_buffer_t(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan18set_output_raw_ptrEiPKcRK17espresso_buffer_t called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan19bind_external_inputEiPKcRK17espresso_buffer_t(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan19bind_external_inputEiPKcRK17espresso_buffer_t called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan20bind_external_outputEiPKcRK17espresso_buffer_t(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan20bind_external_outputEiPKcRK17espresso_buffer_t called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan20set_breakpoint_blockEU13block_pointerFvPKc18espresso_network_t17espresso_buffer_tE(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan20set_breakpoint_blockEU13block_pointerFvPKc18espresso_network_t17espresso_buffer_tE called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan24bind_input_cvpixelbufferEiPKciP10__CVBufferP11objc_objectiP44espresso_simple_image_preprocessing_params_tP25espresso_scaling_settings(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan24bind_input_cvpixelbufferEiPKciP10__CVBufferP11objc_objectiP44espresso_simple_image_preprocessing_params_tP25espresso_scaling_settings called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan25bind_direct_cvpixelbufferEiPKcP10__CVBuffer(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan25bind_direct_cvpixelbufferEiPKcP10__CVBuffer called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan29bind_input_vimagebuffer_XYZA8EiPKciP13vImage_BufferiP44espresso_simple_image_preprocessing_params_tb(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan29bind_input_vimagebuffer_XYZA8EiPKciP13vImage_BufferiP44espresso_simple_image_preprocessing_params_tb called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan29bind_input_vimagebuffer_argb8EiPKciP13vImage_BufferiP44espresso_simple_image_preprocessing_params_t(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan29bind_input_vimagebuffer_argb8EiPKciP13vImage_BufferiP44espresso_simple_image_preprocessing_params_t called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan6submitEPU28objcproto17OS_dispatch_queue8NSObjectPvS4_U13block_pointerFvP21espresso_error_info_tE(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan6submitEPU28objcproto17OS_dispatch_queue8NSObjectPvS4_U13block_pointerFvP21espresso_error_info_tE called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_plan6unbindEiPKc24espresso_bind_mode_flags(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_plan6unbindEiPKc24espresso_bind_mode_flags called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_planC2EPNS_16espresso_contextEi(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_planC2EPNS_16espresso_contextEi called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_planD0Ev(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_planD0Ev called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_planD1Ev(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_planD1Ev called");
    return NULL;
}

void* _ZN13EspressoLight22abstract_espresso_planD2Ev(void)
{
    if (verbose) puts("STUB: _ZN13EspressoLight22abstract_espresso_planD2Ev called");
    return NULL;
}

void* _ZN8Espresso10fuse_scaleENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10fuse_scaleENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso10kern_allocEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10kern_allocEm called");
    return NULL;
}

void* _ZN8Espresso10on_deallocEPv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10on_deallocEPv called");
    return NULL;
}

void* _ZN8Espresso10solver_sgd4stepEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10solver_sgd4stepEm called");
    return NULL;
}

void* _ZN8Espresso10solver_sgdC1ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10solver_sgdC1ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso10solver_sgdC1ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsERKNS1_6vectorINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENSE_ISG_EEEESK_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10solver_sgdC1ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsERKNS1_6vectorINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENSE_ISG_EEEESK_ called");
    return NULL;
}

void* _ZN8Espresso10solver_sgdC2ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10solver_sgdC2ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso10solver_sgdC2ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsERKNS1_6vectorINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENSE_ISG_EEEESK_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso10solver_sgdC2ENSt3__110shared_ptrINS_3netEEENS2_INS_20abstract_data_sourceEEERKNS0_7optionsERKNS1_6vectorINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENSE_ISG_EEEESK_ called");
    return NULL;
}

void* _ZN8Espresso11base_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso11base_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso11base_kernel29validate_for_engine_warning_tC1ERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS0_26validate_for_engine_args_tEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel29validate_for_engine_warning_tC1ERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS0_26validate_for_engine_args_tEb called");
    return NULL;
}

void* _ZN8Espresso11base_kernel29validate_for_engine_warning_tC1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel29validate_for_engine_warning_tC1Ev called");
    return NULL;
}

void* _ZN8Espresso11base_kernel29validate_for_engine_warning_tC2ERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS0_26validate_for_engine_args_tEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel29validate_for_engine_warning_tC2ERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS0_26validate_for_engine_args_tEb called");
    return NULL;
}

void* _ZN8Espresso11base_kernel29validate_for_engine_warning_tC2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel29validate_for_engine_warning_tC2Ev called");
    return NULL;
}

void* _ZN8Espresso11base_kernel39__set_weight_raw_blob_for_serializationERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS0_18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernel39__set_weight_raw_blob_for_serializationERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS0_18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso11base_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEE called");
    return NULL;
}

void* _ZN8Espresso11base_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso11base_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso11base_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11base_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso11fold_concatENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11fold_concatENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso11fuse_biasesENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso11fuse_biasesENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel10on_resizedERNS_10layer_dataES2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel10on_resizedERNS_10layer_dataES2_ called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel15init_parametersENSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel15init_parametersENSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel15set_weight_blobERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPKvm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel15set_weight_blobERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPKvm called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel21has_dynamic_top_shapeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel21has_dynamic_top_shapeEm called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso12brick_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12brick_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12brick_paramsE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_12brick_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_12brick_paramsE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12brick_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12brick_paramsE called");
    return NULL;
}

void* _ZN8Espresso12brick_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_12brick_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12brick_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_12brick_paramsE called");
    return NULL;
}

void* _ZN8Espresso12dump_networkENSt3__110shared_ptrINS_3netEEENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES9_S9_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12dump_networkENSt3__110shared_ptrINS_3netEEENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES9_S9_ called");
    return NULL;
}

void* _ZN8Espresso12dump_networkENSt3__110shared_ptrINS_3netEEERNS0_13basic_ostreamIcNS0_11char_traitsIcEEEES8_NS0_12basic_stringIcS6_NS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12dump_networkENSt3__110shared_ptrINS_3netEEERNS0_13basic_ostreamIcNS0_11char_traitsIcEEEES8_NS0_12basic_stringIcS6_NS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso12get_cpu_blobERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12get_cpu_blobERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE called");
    return NULL;
}

void* _ZN8Espresso12get_cpu_blobERNS_10layer_dataEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12get_cpu_blobERNS_10layer_dataEi called");
    return NULL;
}

void* _ZN8Espresso12get_top_predEPfS0_ii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12get_top_predEPfS0_ii called");
    return NULL;
}

void* _ZN8Espresso12kern_deallocEPv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12kern_deallocEPv called");
    return NULL;
}

void* _ZN8Espresso12load_networkERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12load_networkERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso12net_compilerD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12net_compilerD0Ev called");
    return NULL;
}

void* _ZN8Espresso12net_compilerD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12net_compilerD1Ev called");
    return NULL;
}

void* _ZN8Espresso12net_compilerD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12net_compilerD2Ev called");
    return NULL;
}

void* _ZN8Espresso12no_op_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_23no_op_kernel_parametersENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12no_op_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_23no_op_kernel_parametersENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso12no_op_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12no_op_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso12no_op_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12no_op_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso12no_op_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12no_op_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso12no_op_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12no_op_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso12sanitize_nanEf(void)
{
    if (verbose) puts("STUB: _ZN8Espresso12sanitize_nanEf called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine11f16_to_fp32EPtPfm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine11f16_to_fp32EPtPfm called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine11f32_to_fp16EPfPtm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine11f32_to_fp16EPfPtm called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine18f16_to_fp32_1_slowEt(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine18f16_to_fp32_1_slowEt called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine18f32_to_fp16_1_slowEf(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine18f32_to_fp16_1_slowEf called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine31image2colbuffer_f32_partitionedEPfPKtiiiiiiiiiiiiiii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine31image2colbuffer_f32_partitionedEPfPKtiiiiiiiiiiiiiii called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine8_f16_addEPtS1_S1_m(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine8_f16_addEPtS1_S1_m called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine8_f16_thrEPtS1_fm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine8_f16_thrEPtS1_fm called");
    return NULL;
}

void* _ZN8Espresso13CPUFP16Engine9fp16_zeroEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13CPUFP16Engine9fp16_zeroEv called");
    return NULL;
}

void* _ZN8Espresso13auto_optimizeENSt3__110shared_ptrINS_3netEEENS_21auto_optimize_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13auto_optimizeENSt3__110shared_ptrINS_3netEEENS_21auto_optimize_optionsE called");
    return NULL;
}

void* _ZN8Espresso13axpby_closureclERKNSt3__110shared_ptrINS_14abstract_batchEEEfRNS2_INS_23abstract_blob_containerEEEfS8_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13axpby_closureclERKNSt3__110shared_ptrINS_14abstract_batchEEEfRNS2_INS_23abstract_blob_containerEEEfS8_i called");
    return NULL;
}

void* _ZN8Espresso13compute_blobsENSt3__110shared_ptrINS_3netEEERKNS0_6vectorINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS8_ISA_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13compute_blobsENSt3__110shared_ptrINS_3netEEERKNS0_6vectorINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS8_ISA_EEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_t10net_returnERKNSt3__110shared_ptrINS_19interpreter_state_tEEERKNS2_INS_15return_status_tEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_t10net_returnERKNSt3__110shared_ptrINS_19interpreter_state_tEEERKNS2_INS_15return_status_tEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_t17__launch_functionERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_t17__launch_functionERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_t32setup_fix_context_transfer_racesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_t32setup_fix_context_transfer_racesEv called");
    return NULL;
}

void* _ZN8Espresso13interpreter_t5setupENSt3__110shared_ptrINS_3netEEENS_12compute_pathENS2_INS_6SerDes21generic_serdes_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_t5setupENSt3__110shared_ptrINS_3netEEENS_12compute_pathENS2_INS_6SerDes21generic_serdes_objectEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_t8net_callERKNSt3__110shared_ptrINS_19interpreter_state_tEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_t8net_callERKNSt3__110shared_ptrINS_19interpreter_state_tEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_t8net_gotoERKNSt3__110shared_ptrINS_19interpreter_state_tEEERKm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_t8net_gotoERKNSt3__110shared_ptrINS_19interpreter_state_tEEERKm called");
    return NULL;
}

void* _ZN8Espresso13interpreter_tC1ERKNSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_tC1ERKNSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_tC2ERKNSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_tC2ERKNSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso13interpreter_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso13interpreter_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso13interpreter_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13interpreter_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso13shape_networkENSt3__110shared_ptrINS_3netEEERKNS_13network_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13shape_networkENSt3__110shared_ptrINS_3netEEERKNS_13network_shapeE called");
    return NULL;
}

void* _ZN8Espresso13shape_networkENSt3__110shared_ptrINS_3netEEERNS0_13basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS6_NS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13shape_networkENSt3__110shared_ptrINS_3netEEERNS0_13basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS6_NS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso13shape_networkEPNS_3netERKNS_13network_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13shape_networkEPNS_3netERKNS_13network_shapeE called");
    return NULL;
}

void* _ZN8Espresso13statdist_psnrEPfS0_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso13statdist_psnrEPfS0_i called");
    return NULL;
}

void* _ZN8Espresso14abstract_batchD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14abstract_batchD0Ev called");
    return NULL;
}

void* _ZN8Espresso14abstract_batchD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14abstract_batchD1Ev called");
    return NULL;
}

void* _ZN8Espresso14abstract_batchD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14abstract_batchD2Ev called");
    return NULL;
}

void* _ZN8Espresso14blob_rtti_rootD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14blob_rtti_rootD0Ev called");
    return NULL;
}

void* _ZN8Espresso14blob_rtti_rootD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14blob_rtti_rootD1Ev called");
    return NULL;
}

void* _ZN8Espresso14blob_rtti_rootD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14blob_rtti_rootD2Ev called");
    return NULL;
}

void* _ZN8Espresso14factory_objectD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14factory_objectD0Ev called");
    return NULL;
}

void* _ZN8Espresso14factory_objectD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14factory_objectD1Ev called");
    return NULL;
}

void* _ZN8Espresso14factory_objectD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14factory_objectD2Ev called");
    return NULL;
}

void* _ZN8Espresso14fold_batchnormENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14fold_batchnormENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso14fuse_batchnormENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14fuse_batchnormENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso14get_cpu_blob_8ERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14get_cpu_blob_8ERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE called");
    return NULL;
}

void* _ZN8Espresso14get_cpu_blob_hERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14get_cpu_blob_hERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE called");
    return NULL;
}

void* _ZN8Espresso14get_cpu_blob_iERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14get_cpu_blob_iERKNSt3__110shared_ptrINS_23abstract_blob_containerEEE called");
    return NULL;
}

void* _ZN8Espresso14is_ptr_alignedEPv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14is_ptr_alignedEPv called");
    return NULL;
}

void* _ZN8Espresso14optimize_mlkitENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14optimize_mlkitENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso14run_validationENSt3__110shared_ptrINS_3netEEENS1_INS_20abstract_data_sourceEEENS_18validation_optionsENS0_6vectorIS3_NS0_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14run_validationENSt3__110shared_ptrINS_3netEEENS1_INS_20abstract_data_sourceEEENS_18validation_optionsENS0_6vectorIS3_NS0_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso14wisdom_manager14get_parametersERKNS_17wisdom_kernel_keyE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14wisdom_manager14get_parametersERKNS_17wisdom_kernel_keyE called");
    return NULL;
}

void* _ZN8Espresso14wisdom_trainer5trainEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14wisdom_trainer5trainEv called");
    return NULL;
}

void* _ZN8Espresso14wisdom_trainer6recordERKNS_17wisdom_kernel_keyERKNSt3__13mapINS4_12basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEEiNS4_4lessISB_EENS9_INS4_4pairIKSB_iEEEEEEf(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14wisdom_trainer6recordERKNS_17wisdom_kernel_keyERKNSt3__13mapINS4_12basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEEiNS4_4lessISB_EENS9_INS4_4pairIKSB_iEEEEEEf called");
    return NULL;
}

void* _ZN8Espresso14wisdom_trainerC1ERKNS_17wisdom_device_keyE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14wisdom_trainerC1ERKNS_17wisdom_device_keyE called");
    return NULL;
}

void* _ZN8Espresso14wisdom_trainerC2ERKNS_17wisdom_device_keyE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso14wisdom_trainerC2ERKNS_17wisdom_device_keyE called");
    return NULL;
}

void* _ZN8Espresso15abstract_engineD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15abstract_engineD0Ev called");
    return NULL;
}

void* _ZN8Espresso15abstract_engineD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15abstract_engineD1Ev called");
    return NULL;
}

void* _ZN8Espresso15abstract_engineD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15abstract_engineD2Ev called");
    return NULL;
}

void* _ZN8Espresso15analysis_resultD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15analysis_resultD0Ev called");
    return NULL;
}

void* _ZN8Espresso15analysis_resultD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15analysis_resultD1Ev called");
    return NULL;
}

void* _ZN8Espresso15analysis_resultD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15analysis_resultD2Ev called");
    return NULL;
}

void* _ZN8Espresso15make_new_kernelERKNSt3__110shared_ptrINS_5layerEEERKNS1_INS_16abstract_contextEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15make_new_kernelERKNSt3__110shared_ptrINS_5layerEEERKNS1_INS_16abstract_contextEEE called");
    return NULL;
}

void* _ZN8Espresso15print_mem_statsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15print_mem_statsEv called");
    return NULL;
}

void* _ZN8Espresso15statdist_cosineEPfS0_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15statdist_cosineEPfS0_i called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer212tune_networkENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer212tune_networkENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer216tune_t_to_stringENSt3__16vectorIiNS1_9allocatorIiEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer216tune_t_to_stringENSt3__16vectorIiNS1_9allocatorIiEEEE called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer24loadEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer24loadEPKc called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer24saveEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer24saveEPKc called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer24tuneERKNSt3__110shared_ptrINS_11base_kernelEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer24tuneERKNSt3__110shared_ptrINS_11base_kernelEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer27to_dictEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer27to_dictEv called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer28get_tuneEPNS_11base_kernelERNS_10layer_dataES4_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer28get_tuneEPNS_11base_kernelERNS_10layer_dataES4_ called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer29from_dictEP12NSDictionary(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer29from_dictEP12NSDictionary called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer2C1ENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer2C1ENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso15wisdom_trainer2C2ENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso15wisdom_trainer2C2ENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso16abstract_context14batch_callbackERKNSt3__110shared_ptrINS_14abstract_batchEEEPU28objcproto17OS_dispatch_queue8NSObjectNS1_8functionIFvRKNS_17execution_error_tEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_context14batch_callbackERKNSt3__110shared_ptrINS_14abstract_batchEEEPU28objcproto17OS_dispatch_queue8NSObjectNS1_8functionIFvRKNS_17execution_error_tEEEE called");
    return NULL;
}

void* _ZN8Espresso16abstract_context15enable_trainingEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_context15enable_trainingEv called");
    return NULL;
}

void* _ZN8Espresso16abstract_context15set_as_child_ofENSt3__110shared_ptrIS0_EE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_context15set_as_child_ofENSt3__110shared_ptrIS0_EE called");
    return NULL;
}

void* _ZN8Espresso16abstract_context18compute_batch_syncEU13block_pointerFvRKNSt3__110shared_ptrINS_14abstract_batchEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_context18compute_batch_syncEU13block_pointerFvRKNSt3__110shared_ptrINS_14abstract_batchEEEE called");
    return NULL;
}

void* _ZN8Espresso16abstract_context27create_compiler_for_networkENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_context27create_compiler_for_networkENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso16abstract_contextC2ERKNSt3__110shared_ptrINS_15abstract_engineEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_contextC2ERKNSt3__110shared_ptrINS_15abstract_engineEEE called");
    return NULL;
}

void* _ZN8Espresso16abstract_contextD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_contextD0Ev called");
    return NULL;
}

void* _ZN8Espresso16abstract_contextD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_contextD1Ev called");
    return NULL;
}

void* _ZN8Espresso16abstract_contextD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16abstract_contextD2Ev called");
    return NULL;
}

void* _ZN8Espresso16confusion_matrix9predictedEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16confusion_matrix9predictedEii called");
    return NULL;
}

void* _ZN8Espresso16gradient_builder14build_gradientERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_6vectorIS7_NS5_IS7_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builder14build_gradientERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_6vectorIS7_NS5_IS7_EEEE called");
    return NULL;
}

void* _ZN8Espresso16gradient_builder14build_gradientEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builder14build_gradientEv called");
    return NULL;
}

void* _ZN8Espresso16gradient_builder18preprocess_networkEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builder18preprocess_networkEv called");
    return NULL;
}

void* _ZN8Espresso16gradient_builder22net_training_transformERKNSt3__113unordered_mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_26training_transform_optionsENS1_4hashIS8_EENS1_8equal_toIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builder22net_training_transformERKNSt3__113unordered_mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_26training_transform_optionsENS1_4hashIS8_EENS1_8equal_toIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso16gradient_builder22net_training_transformEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builder22net_training_transformEv called");
    return NULL;
}

void* _ZN8Espresso16gradient_builder37transform_parameter_inputs_as_globalsENSt3__16vectorINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS6_IS8_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builder37transform_parameter_inputs_as_globalsENSt3__16vectorINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS6_IS8_EEEE called");
    return NULL;
}

void* _ZN8Espresso16gradient_builderC1ENSt3__110shared_ptrINS_3netEEERKNS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builderC1ENSt3__110shared_ptrINS_3netEEERKNS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso16gradient_builderC2ENSt3__110shared_ptrINS_3netEEERKNS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16gradient_builderC2ENSt3__110shared_ptrINS_3netEEERKNS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso16keys_translation22c_api_engine_from_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation22c_api_engine_from_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso16keys_translation23cpp_platform_from_c_apiE17espresso_engine_t(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation23cpp_platform_from_c_apiE17espresso_engine_t called");
    return NULL;
}

void* _ZN8Espresso16keys_translation27cpp_compute_path_from_c_apiE23espresso_storage_type_t(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation27cpp_compute_path_from_c_apiE23espresso_storage_type_t called");
    return NULL;
}

void* _ZN8Espresso16keys_translation28c_api_storage_type_from_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation28c_api_storage_type_from_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso16keys_translation30c_api_engine_from_cpp_platformEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation30c_api_engine_from_cpp_platformEi called");
    return NULL;
}

void* _ZN8Espresso16keys_translation31cpp_quant_weight_mem_from_c_apiE26quantized_blob_in_memory_t(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation31cpp_quant_weight_mem_from_c_apiE26quantized_blob_in_memory_t called");
    return NULL;
}

void* _ZN8Espresso16keys_translation40c_api_storage_type_from_cpp_compute_pathEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16keys_translation40c_api_storage_type_from_cpp_compute_pathEi called");
    return NULL;
}

void* _ZN8Espresso16load_data_sourceERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS0_10shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16load_data_sourceERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS0_10shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso16load_subnetworksENSt3__110shared_ptrINS_3netEEENS1_INS_6SerDes21generic_serdes_objectEEENS_12compute_pathERKNS1_INS_21blob_storage_abstractEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16load_subnetworksENSt3__110shared_ptrINS_3netEEENS1_INS_6SerDes21generic_serdes_objectEEENS_12compute_pathERKNS1_INS_21blob_storage_abstractEEE called");
    return NULL;
}

void* _ZN8Espresso16log_parametersetERKNSt3__13mapINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEEiNS0_4lessIS7_EENS5_INS0_4pairIKS7_iEEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16log_parametersetERKNSt3__13mapINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEEiNS0_4lessIS7_EENS5_INS0_4pairIKS7_iEEEEEE called");
    return NULL;
}

void* _ZN8Espresso16matmul_shorthandclERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS2_INS_23abstract_blob_containerEEESA_SA_SA_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16matmul_shorthandclERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS2_INS_23abstract_blob_containerEEESA_SA_SA_ called");
    return NULL;
}

void* _ZN8Espresso16optimize_networkENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathENS1_INS_3netEEEU13block_pointerFNS0_6vectorINS1_INS_23graph_optimization_ruleEEENS0_9allocatorIS9_EEEENS1_INS_26network_optimization_graphEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16optimize_networkENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathENS1_INS_3netEEEU13block_pointerFNS0_6vectorINS1_INS_23graph_optimization_ruleEEENS0_9allocatorIS9_EEEENS1_INS_26network_optimization_graphEEEE called");
    return NULL;
}

void* _ZN8Espresso16pass_params_baseD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16pass_params_baseD0Ev called");
    return NULL;
}

void* _ZN8Espresso16pass_params_baseD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16pass_params_baseD1Ev called");
    return NULL;
}

void* _ZN8Espresso16pass_params_baseD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16pass_params_baseD2Ev called");
    return NULL;
}

void* _ZN8Espresso16reduce_shorthandclERKNSt3__110shared_ptrINS_14abstract_batchEEENS_11REDUCE_MODEERKNS2_INS_23abstract_blob_containerEEESB_SB_f(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16reduce_shorthandclERKNSt3__110shared_ptrINS_14abstract_batchEEENS_11REDUCE_MODEERKNS2_INS_23abstract_blob_containerEEESB_SB_f called");
    return NULL;
}

void* _ZN8Espresso16wrong_batch_typeD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16wrong_batch_typeD0Ev called");
    return NULL;
}

void* _ZN8Espresso16wrong_batch_typeD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16wrong_batch_typeD1Ev called");
    return NULL;
}

void* _ZN8Espresso16wrong_batch_typeD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso16wrong_batch_typeD2Ev called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helper12anec_compileERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEESA_RKNS1_6opts_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helper12anec_compileERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEESA_RKNS1_6opts_tE called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helper22temporary_plist_folderERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS1_6opts_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helper22temporary_plist_folderERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS1_6opts_tE called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helper41precompiled_filename_for_segment_and_archERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEEiSA_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helper41precompiled_filename_for_segment_and_archERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEEiSA_ called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helperC1ERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helperC1ERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helperC2ERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helperC2ERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helperD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helperD1Ev called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine19zin_compiler_helperD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine19zin_compiler_helperD2Ev called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine25precompilation_settings_t6sharedEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine25precompilation_settings_t6sharedEv called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine25precompilation_settings_tC1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine25precompilation_settings_tC1Ev called");
    return NULL;
}

void* _ZN8Espresso17ANECompilerEngine25precompilation_settings_tC2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17ANECompilerEngine25precompilation_settings_tC2Ev called");
    return NULL;
}

void* _ZN8Espresso17async_data_source19get_data_for_sampleEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17async_data_source19get_data_for_sampleEm called");
    return NULL;
}

void* _ZN8Espresso17async_data_source5readyEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17async_data_source5readyEv called");
    return NULL;
}

void* _ZN8Espresso17async_data_sourceC2ENS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17async_data_sourceC2ENS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso17async_data_sourceD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17async_data_sourceD0Ev called");
    return NULL;
}

void* _ZN8Espresso17async_data_sourceD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17async_data_sourceD1Ev called");
    return NULL;
}

void* _ZN8Espresso17async_data_sourceD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17async_data_sourceD2Ev called");
    return NULL;
}

void* _ZN8Espresso17blob_storage_rootD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17blob_storage_rootD0Ev called");
    return NULL;
}

void* _ZN8Espresso17blob_storage_rootD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17blob_storage_rootD1Ev called");
    return NULL;
}

void* _ZN8Espresso17blob_storage_rootD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17blob_storage_rootD2Ev called");
    return NULL;
}

void* _ZN8Espresso17compute_all_blobsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17compute_all_blobsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator18statistics_entry_t14compute_singleEPfi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator18statistics_entry_t14compute_singleEPfi called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator18statistics_entry_t14iterative_growERKS1_f(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator18statistics_entry_t14iterative_growERKS1_f called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator18statistics_entry_t26iterative_grow_with_scalarEfb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator18statistics_entry_t26iterative_grow_with_scalarEfb called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator23sample_stats_with_blobsERNS_21map_with_ordered_keysINSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS2_10shared_ptrINS_4blobIfLi4EEEEEEERKNS0_9options_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator23sample_stats_with_blobsERNS_21map_with_ordered_keysINSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS2_10shared_ptrINS_4blobIfLi4EEEEEEERKNS0_9options_tE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator39sample_stats_with_current_network_stateERKNS0_9options_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator39sample_stats_with_current_network_stateERKNS0_9options_tE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator8estimateENSt3__110shared_ptrINS_20abstract_data_sourceEEERKNS0_9options_tEU13block_pointerFvvE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator8estimateENSt3__110shared_ptrINS_20abstract_data_sourceEEERKNS0_9options_tEU13block_pointerFvvE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator8result_t4loadERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator8result_t4loadERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimator8result_t4saveERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimator8result_t4saveERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimatorC1ENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimatorC1ENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso17dataset_estimatorC2ENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17dataset_estimatorC2ENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_t13register_typeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEU13block_pointerFNS1_10shared_ptrINS_14factory_objectEEEvE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_t13register_typeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEU13block_pointerFNS1_10shared_ptrINS_14factory_objectEEEvE called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_t16get_objects_copyEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_t16get_objects_copyEv called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_t16register_object_ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_14factory_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_t16register_object_ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_14factory_objectEEE called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_t6sharedERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEU13block_pointerFNS1_10shared_ptrIS0_EEvE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_t6sharedERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEU13block_pointerFNS1_10shared_ptrIS0_EEvE called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_tC1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_tC1Ev called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_tC2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_tC2Ev called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso17generic_factory_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17generic_factory_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso17net_fast_reshaper7reshapeERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17net_fast_reshaper7reshapeERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso17net_fast_reshaperC1ERKNSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17net_fast_reshaperC1ERKNSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso17net_fast_reshaperC2ERKNSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17net_fast_reshaperC2ERKNSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso17serialize_networkENSt3__110shared_ptrINS_3netEEERKNS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERNS_31dump_network_layers_blobs_stateE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17serialize_networkENSt3__110shared_ptrINS_3netEEERKNS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERNS_31dump_network_layers_blobs_stateE called");
    return NULL;
}

void* _ZN8Espresso17wisdom_device_key16key_for_platformENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17wisdom_device_key16key_for_platformENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso17wisdom_device_key16key_for_platformEPNS_16abstract_contextENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17wisdom_device_key16key_for_platformEPNS_16abstract_contextENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso17wisdom_device_key9to_stringEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso17wisdom_device_key9to_stringEv called");
    return NULL;
}

void* _ZN8Espresso18cv_pixel_buffer_io17get_src_crop_rectEmmRKNS0_8settingsEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18cv_pixel_buffer_io17get_src_crop_rectEmmRKNS0_8settingsEii called");
    return NULL;
}

void* _ZN8Espresso18cv_pixel_buffer_ioD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18cv_pixel_buffer_ioD0Ev called");
    return NULL;
}

void* _ZN8Espresso18cv_pixel_buffer_ioD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18cv_pixel_buffer_ioD1Ev called");
    return NULL;
}

void* _ZN8Espresso18cv_pixel_buffer_ioD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18cv_pixel_buffer_ioD2Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_lrn_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_lrn_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso18generic_lrn_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_10lrn_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_lrn_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_10lrn_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso18generic_lrn_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_lrn_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_lrn_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_lrn_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_lrn_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_lrn_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel16cached_temp_blobENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEiiii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel16cached_temp_blobENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEiiii called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel19number_of_top_blobsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel19number_of_top_blobsEv called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel21__launch_lstm_genericENSt3__110shared_ptrINS_14abstract_batchEEENS2_INS_23abstract_blob_containerEEES6_S6_S6_S6_S6_S6_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel21__launch_lstm_genericENSt3__110shared_ptrINS_14abstract_batchEEENS2_INS_23abstract_blob_containerEEES6_S6_S6_S6_S6_S6_i called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel23weight_names_and_shapesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel23weight_names_and_shapesEv called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_rnn_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_rnn_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_sgd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_sgd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso18generic_sgd_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_sgd_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_sgd_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_sgd_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso18generic_sgd_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18generic_sgd_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso18load_network_shapeENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18load_network_shapeENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE called");
    return NULL;
}

void* _ZN8Espresso18load_network_shapeERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS3_NS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18load_network_shapeERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS3_NS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso18net_number_of_bitsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18net_number_of_bitsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso18platform_to_stringENS_8platformE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18platform_to_stringENS_8platformE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder10add_kernelENSt3__110shared_ptrINS_11base_kernelEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESC_NS1_6vectorISA_NS8_ISA_EEEESF_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder10add_kernelENSt3__110shared_ptrINS_11base_kernelEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESC_NS1_6vectorISA_NS8_ISA_EEEESF_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder10blob_shapeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder10blob_shapeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder11add_networkENSt3__110shared_ptrINS_3netEEERKNS1_6vectorINS1_4pairINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESC_EENSA_ISD_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder11add_networkENSt3__110shared_ptrINS_3netEEERKNS1_6vectorINS1_4pairINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESC_EENSA_ISD_EEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder11placeholderENS_11layer_shapeENSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder11placeholderENS_11layer_shapeENSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder13constant_likeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEfS7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder13constant_likeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEfS7_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder14by_constant_opERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEfNS_21ELEMENTWISE_OPERATIONES7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder14by_constant_opERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEfNS_21ELEMENTWISE_OPERATIONES7_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder14elementwise_opERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_NS_21ELEMENTWISE_OPERATIONES7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder14elementwise_opERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_NS_21ELEMENTWISE_OPERATIONES7_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder14init_net_shapeEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder14init_net_shapeEv called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder14load_global_opENSt3__110shared_ptrINS_23abstract_blob_containerEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESA_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder14load_global_opENSt3__110shared_ptrINS_23abstract_blob_containerEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESA_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder15store_global_opERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_S7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder15store_global_opERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_S7_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder17build_from_shapesERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder17build_from_shapesERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder17build_from_shapesERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder17build_from_shapesERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEEb called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder21add_shape_from_kernelERNS_11base_kernelERKNSt3__16vectorINS3_12basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEENS8_ISA_EEEESE_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder21add_shape_from_kernelERNS_11base_kernelERKNSt3__16vectorINS3_12basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEENS8_ISA_EEEESE_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder21build_from_data_shapeENS_11layer_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder21build_from_data_shapeENS_11layer_shapeE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder22add_layer_from_networkENSt3__110shared_ptrINS_3netEEEiRKNS1_6vectorINS1_4pairINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESC_EENSA_ISD_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder22add_layer_from_networkENSt3__110shared_ptrINS_3netEEEiRKNS1_6vectorINS1_4pairINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEESC_EENSA_ISD_EEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder22add_layer_from_networkERNS_3netEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder22add_layer_from_networkERNS_3netEm called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder22infer_shapes_and_buildEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder22infer_shapes_and_buildEm called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder24build_from_network_shapeERKNS_13network_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder24build_from_network_shapeERKNS_13network_shapeE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder27build_and_insert_in_networkERNS_3netEmRKNSt3__113unordered_mapINS3_12basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEENS_11layer_shapeENS3_4hashISA_EENS3_8equal_toISA_EENS8_INS3_4pairIKSA_SB_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder27build_and_insert_in_networkERNS_3netEmRKNSt3__113unordered_mapINS3_12basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEENS_11layer_shapeENS3_4hashISA_EENS3_8equal_toISA_EENS8_INS3_4pairIKSA_SB_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder27get_aux_elementwise_op_nameENS_21ELEMENTWISE_OPERATIONE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder27get_aux_elementwise_op_nameENS_21ELEMENTWISE_OPERATIONE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder5rsqrtERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEfS7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder5rsqrtERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEfS7_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder6reduceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11REDUCE_MODEENS_16REDUCE_AXIS_MODEES7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder6reduceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11REDUCE_MODEENS_16REDUCE_AXIS_MODEES7_ called");
    return NULL;
}

void* _ZN8Espresso18sequential_builder8constantENS_11layer_shapeEfNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builder8constantENS_11layer_shapeEfNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builderC1ENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builderC1ENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builderC1ENSt3__110shared_ptrINS_3netEEENS2_INS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builderC1ENSt3__110shared_ptrINS_3netEEENS2_INS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builderC2ENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builderC2ENSt3__110shared_ptrINS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE called");
    return NULL;
}

void* _ZN8Espresso18sequential_builderC2ENSt3__110shared_ptrINS_3netEEENS2_INS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18sequential_builderC2ENSt3__110shared_ptrINS_3netEEENS2_INS_16abstract_contextEEENS_12compute_pathENS2_INS_23abstract_engine_factoryEEE called");
    return NULL;
}

void* _ZN8Espresso18wrong_context_typeD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18wrong_context_typeD0Ev called");
    return NULL;
}

void* _ZN8Espresso18wrong_context_typeD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18wrong_context_typeD1Ev called");
    return NULL;
}

void* _ZN8Espresso18wrong_context_typeD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso18wrong_context_typeD2Ev called");
    return NULL;
}

void* _ZN8Espresso19fuse_nonlinearitiesEPNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19fuse_nonlinearitiesEPNS_3netE called");
    return NULL;
}

void* _ZN8Espresso19generic_adam_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_adam_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso19generic_adam_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_adam_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_adam_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_adam_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_adam_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_adam_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_bias_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_bias_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_bias_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_bias_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_bias_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_bias_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_cast_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_cast_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_cast_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_cast_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_cast_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_cast_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_copy_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_13copy_params_tENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_copy_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_13copy_params_tENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso19generic_copy_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_copy_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_copy_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_copy_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_copy_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_copy_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_crop_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_crop_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso19generic_crop_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_crop_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_crop_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_crop_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_crop_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_crop_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_fill_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_fill_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso19generic_fill_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_fill_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_fill_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_fill_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_fill_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_fill_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernel10on_resizedERNS_10layer_dataES2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernel10on_resizedERNS_10layer_dataES2_ called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_11pool_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_11pool_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_pool_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_pool_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_tile_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_tile_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso19generic_tile_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_tile_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_tile_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_tile_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_tile_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_tile_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_topk_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_topk_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso19generic_topk_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_topk_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_topk_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_topk_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso19generic_topk_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19generic_topk_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso19get_last_layer_blobENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19get_last_layer_blobENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso19interpreter_state_tC1ERKNSt3__110shared_ptrINS_13interpreter_tEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19interpreter_state_tC1ERKNSt3__110shared_ptrINS_13interpreter_tEEE called");
    return NULL;
}

void* _ZN8Espresso19interpreter_state_tC2ERKNSt3__110shared_ptrINS_13interpreter_tEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19interpreter_state_tC2ERKNSt3__110shared_ptrINS_13interpreter_tEEE called");
    return NULL;
}

void* _ZN8Espresso19interpreter_state_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19interpreter_state_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso19interpreter_state_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19interpreter_state_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso19load_network_layersERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19load_network_layersERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathES8_ called");
    return NULL;
}

void* _ZN8Espresso19load_network_layersERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19load_network_layersERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathES8_ called");
    return NULL;
}

void* _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEEPvmRKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEEPvmRKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEEPvmRKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEEPvmRKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS3_NS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathESB_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS3_NS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathESB_ called");
    return NULL;
}

void* _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS3_NS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathESB_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19load_network_layersERNSt3__113basic_istreamIcNS0_11char_traitsIcEEEERKNS0_12basic_stringIcS3_NS0_9allocatorIcEEEERKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathESB_ called");
    return NULL;
}

void* _ZN8Espresso19switch_engine_batch10sync_pointEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19switch_engine_batch10sync_pointEv called");
    return NULL;
}

void* _ZN8Espresso19switch_engine_batch16switch_to_engineENSt3__110shared_ptrINS_16abstract_contextEEEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19switch_engine_batch16switch_to_engineENSt3__110shared_ptrINS_16abstract_contextEEEb called");
    return NULL;
}

void* _ZN8Espresso19switch_engine_batch23sync_wait_for_scheduledEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19switch_engine_batch23sync_wait_for_scheduledEv called");
    return NULL;
}

void* _ZN8Espresso19switch_engine_batch6commitEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19switch_engine_batch6commitEv called");
    return NULL;
}

void* _ZN8Espresso19switch_engine_batch9sync_waitEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19switch_engine_batch9sync_waitEv called");
    return NULL;
}

void* _ZN8Espresso19util_bake_batchnormEiiPKfPfS1_S2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso19util_bake_batchnormEiiPKfPfS1_S2_ called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnorm14match_at_layerEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnorm14match_at_layerEi called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnorm32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnorm32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnorm8status_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnorm8status_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnorm8status_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnorm8status_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnorm8status_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnorm8status_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnormC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnormC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso20_rule_bake_batchnormC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20_rule_bake_batchnormC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso20abstract_data_source18get_data_for_batchEmm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20abstract_data_source18get_data_for_batchEmm called");
    return NULL;
}

void* _ZN8Espresso20fuse_transpose_chainENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20fuse_transpose_chainENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso20generic_alloc_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_alloc_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_alloc_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_alloc_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_alloc_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_alloc_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_axpby_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_axpby_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_axpby_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_axpby_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_axpby_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_axpby_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_cf_if_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_cf_if_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_cf_if_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_cf_if_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_cf_if_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_cf_if_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_range_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_range_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso20generic_range_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_range_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_range_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_range_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_range_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_range_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_slice_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_slice_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso20generic_slice_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_slice_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso20generic_slice_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_slice_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_slice_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_slice_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_slice_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_slice_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_split_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_split_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso20generic_split_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_12split_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_split_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_12split_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso20generic_split_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_split_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_split_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_split_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso20generic_split_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20generic_split_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso20get_internal_contextEPv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20get_internal_contextEPv called");
    return NULL;
}

void* _ZN8Espresso20get_internal_networkE18espresso_network_t(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20get_internal_networkE18espresso_network_t called");
    return NULL;
}

void* _ZN8Espresso20multikey_data_sourceC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20multikey_data_sourceC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso20multikey_data_sourceC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20multikey_data_sourceC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso20multikey_data_sourceD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20multikey_data_sourceD0Ev called");
    return NULL;
}

void* _ZN8Espresso20multikey_data_sourceD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20multikey_data_sourceD1Ev called");
    return NULL;
}

void* _ZN8Espresso20multikey_data_sourceD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20multikey_data_sourceD2Ev called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_block14match_at_layerEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_block14match_at_layerEi called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_block32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_block32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_block8status_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_block8status_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_block8status_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_block8status_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_block8status_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_block8status_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_blockC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_blockC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso20rule_inception_blockC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20rule_inception_blockC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso20shuffle_to_gpu_imageENSt3__110shared_ptrINS_4blobIfLi3EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20shuffle_to_gpu_imageENSt3__110shared_ptrINS_4blobIfLi3EEEEE called");
    return NULL;
}

void* _ZN8Espresso20statdist_cosine_topnEPfS0_ii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20statdist_cosine_topnEPfS0_ii called");
    return NULL;
}

void* _ZN8Espresso20transform_dequantizeENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso20transform_dequantizeENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso21blob_storage_abstract12setup_headerEPhmm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21blob_storage_abstract12setup_headerEPhmm called");
    return NULL;
}

void* _ZN8Espresso21blob_storage_abstract13get_blob_sizeEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21blob_storage_abstract13get_blob_sizeEi called");
    return NULL;
}

void* _ZN8Espresso21fold_transform_paramsENSt3__110shared_ptrINS_3netEEEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21fold_transform_paramsENSt3__110shared_ptrINS_3netEEEb called");
    return NULL;
}

void* _ZN8Espresso21generic_cf_end_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cf_end_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cf_end_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cf_end_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cf_end_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cf_end_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cf_for_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cf_for_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cf_for_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cf_for_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cf_for_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cf_for_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_concat_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_concat_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso21generic_concat_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_concat_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso21generic_concat_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_13concat_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_concat_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_13concat_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso21generic_concat_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_concat_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_concat_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_concat_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_concat_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_concat_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cumsum_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cumsum_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso21generic_cumsum_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cumsum_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cumsum_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cumsum_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_cumsum_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_cumsum_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_dev_op_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_dev_op_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso21generic_dev_op_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_dev_op_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_dev_op_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_dev_op_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_dev_op_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_dev_op_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_gather_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_gather_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso21generic_gather_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_gather_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_gather_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_gather_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_gather_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_gather_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernel10on_resizedERNS_10layer_dataES2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernel10on_resizedERNS_10layer_dataES2_ called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernel10top_shapesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernel10top_shapesEv called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernel13bottom_shapesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernel13bottom_shapesEv called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernel19number_of_top_blobsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernel19number_of_top_blobsEv called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_15lambda_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernelC1ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_15lambda_params_tE called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_15lambda_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernelC2ERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_12compute_pathERKNS_15lambda_params_tE called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_lambda_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_lambda_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_reduce_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_reduce_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso21generic_reduce_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_reduce_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso21generic_reduce_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_reduce_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_reduce_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_reduce_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso21generic_reduce_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21generic_reduce_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso21perf_benchmark_result4saveEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21perf_benchmark_result4saveEPKc called");
    return NULL;
}

void* _ZN8Espresso21perf_benchmark_result6uploadEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21perf_benchmark_result6uploadEPKc called");
    return NULL;
}

void* _ZN8Espresso21perf_benchmark_result9add_entryERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_S9_iS9_fff(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21perf_benchmark_result9add_entryERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_S9_iS9_fff called");
    return NULL;
}

void* _ZN8Espresso21top_names_from_serdesERNS_6SerDes21generic_serdes_objectENSt3__110shared_ptrINS_11base_kernelEEERKNS3_12basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso21top_names_from_serdesERNS_6SerDes21generic_serdes_objectENSt3__110shared_ptrINS_11base_kernelEEERKNS3_12basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso22abstract_interpreter_t17setup_subnetworksEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22abstract_interpreter_t17setup_subnetworksEv called");
    return NULL;
}

void* _ZN8Espresso22abstract_interpreter_tC2ERKNSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22abstract_interpreter_tC2ERKNSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso22abstract_interpreter_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22abstract_interpreter_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso22abstract_interpreter_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22abstract_interpreter_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso22abstract_interpreter_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22abstract_interpreter_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso22batchnorm_renormalizer17create_checkpointEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22batchnorm_renormalizer17create_checkpointEb called");
    return NULL;
}

void* _ZN8Espresso22batchnorm_renormalizer20renormalize_bn_epochENSt3__110shared_ptrINS_20abstract_data_sourceEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22batchnorm_renormalizer20renormalize_bn_epochENSt3__110shared_ptrINS_20abstract_data_sourceEEE called");
    return NULL;
}

void* _ZN8Espresso22batchnorm_renormalizer7analyzeEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22batchnorm_renormalizer7analyzeEv called");
    return NULL;
}

void* _ZN8Espresso22batchnorm_renormalizerC1ENSt3__110shared_ptrINS_3netEEENS0_9options_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22batchnorm_renormalizerC1ENSt3__110shared_ptrINS_3netEEENS0_9options_tE called");
    return NULL;
}

void* _ZN8Espresso22batchnorm_renormalizerC2ENSt3__110shared_ptrINS_3netEEENS0_9options_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22batchnorm_renormalizerC2ENSt3__110shared_ptrINS_3netEEENS0_9options_tE called");
    return NULL;
}

void* _ZN8Espresso22cast_indexes_to_n_bitsEPhmS0_mm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22cast_indexes_to_n_bitsEPhmS0_mm called");
    return NULL;
}

void* _ZN8Espresso22compute_path_to_stringENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22compute_path_to_stringENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso22dropout_augment_lowmem9__forwardERKNSt3__110shared_ptrINS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22dropout_augment_lowmem9__forwardERKNSt3__110shared_ptrINS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso22dropout_augment_lowmemC1ENSt3__110shared_ptrINS_3netEEENS_22dropout_augment_paramsENS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22dropout_augment_lowmemC1ENSt3__110shared_ptrINS_3netEEENS_22dropout_augment_paramsENS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso22dropout_augment_lowmemC2ENSt3__110shared_ptrINS_3netEEENS_22dropout_augment_paramsENS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22dropout_augment_lowmemC2ENSt3__110shared_ptrINS_3netEEENS_22dropout_augment_paramsENS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_builtin_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_builtin_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_builtin_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_builtin_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_builtin_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_builtin_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_builtin_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_builtin_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_call_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_call_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_call_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_call_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_call_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_call_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_else_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_else_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_else_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_else_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_else_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_else_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_jump_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_jump_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_jump_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_jump_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_jump_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_jump_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_loop_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_loop_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_loop_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_loop_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_cf_loop_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_cf_loop_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_dropout_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_dropout_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_dropout_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_dropout_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_dropout_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_dropout_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_flatten_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_flatten_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso22generic_flatten_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_flatten_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_flatten_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_flatten_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_flatten_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_flatten_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_flatten_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_flatten_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_nonzero_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_nonzero_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_nonzero_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_nonzero_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_nonzero_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_nonzero_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_one_hot_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_one_hot_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_one_hot_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_one_hot_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_one_hot_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_one_hot_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_one_hot_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_one_hot_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_reshape_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_reshape_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso22generic_reshape_kernel25top_shape_using_auto_axisENS_11layer_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_reshape_kernel25top_shape_using_auto_axisENS_11layer_shapeE called");
    return NULL;
}

void* _ZN8Espresso22generic_reshape_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_reshape_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_reshape_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_reshape_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_reshape_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_reshape_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_reshape_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_reshape_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_scatter_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_scatter_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_scatter_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_scatter_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_scatter_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_scatter_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_scatter_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_scatter_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_softmax_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_softmax_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso22generic_softmax_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_14softmax_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_softmax_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_14softmax_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso22generic_softmax_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_softmax_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_softmax_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_softmax_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_softmax_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_softmax_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_squeeze_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_squeeze_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso22generic_squeeze_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_squeeze_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_squeeze_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_squeeze_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso22generic_squeeze_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22generic_squeeze_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlike14match_at_layerEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlike14match_at_layerEi called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlike32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlike32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlike8status_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlike8status_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlike8status_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlike8status_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlike8status_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlike8status_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlikeC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlikeC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso22rule_1x1_inceptionlikeC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso22rule_1x1_inceptionlikeC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso23abstract_blob_container16create_host_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_blob_container16create_host_blobEv called");
    return NULL;
}

void* _ZN8Espresso23abstract_blob_container27use_external_cvpixelbuffersERKNSt3__16vectorIP10__CVBufferNS1_9allocatorIS4_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_blob_container27use_external_cvpixelbuffersERKNSt3__16vectorIP10__CVBufferNS1_9allocatorIS4_EEEE called");
    return NULL;
}

void* _ZN8Espresso23abstract_blob_container30create_uninitialized_host_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_blob_container30create_uninitialized_host_blobEv called");
    return NULL;
}

void* _ZN8Espresso23abstract_blob_container4makeERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_8platformERKNS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_blob_container4makeERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_8platformERKNS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso23abstract_engine_factory15register_engineENSt3__110shared_ptrINS_15abstract_engineEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_engine_factory15register_engineENSt3__110shared_ptrINS_15abstract_engineEEE called");
    return NULL;
}

void* _ZN8Espresso23abstract_engine_factory21register_kernel_classERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS_8platformEU13block_pointerFNS1_10shared_ptrINS_11base_kernelEEERKNSD_INS_16abstract_contextEEEPvRKNS_12compute_pathEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_engine_factory21register_kernel_classERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS_8platformEU13block_pointerFNS1_10shared_ptrINS_11base_kernelEEERKNSD_INS_16abstract_contextEEEPvRKNS_12compute_pathEE called");
    return NULL;
}

void* _ZN8Espresso23abstract_engine_factory28make_abstract_blob_containerERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_8platformERKNS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_engine_factory28make_abstract_blob_containerERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_8platformERKNS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso23abstract_engine_factory6sharedEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_engine_factory6sharedEv called");
    return NULL;
}

void* _ZN8Espresso23abstract_engine_factoryC1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_engine_factoryC1Ev called");
    return NULL;
}

void* _ZN8Espresso23abstract_engine_factoryC2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23abstract_engine_factoryC2Ev called");
    return NULL;
}

void* _ZN8Espresso23convert_weights_to_fp16ENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23convert_weights_to_fp16ENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso23generic_split_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_split_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso23generic_split_nd_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_split_nd_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_split_nd_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_split_nd_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_split_nd_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_split_nd_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_stack_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_stack_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso23generic_stack_nd_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_stack_nd_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_stack_nd_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_stack_nd_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_stack_nd_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_stack_nd_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_upsample_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_upsample_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso23generic_upsample_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_22upsample_kernel_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_upsample_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_22upsample_kernel_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso23generic_upsample_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_upsample_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_upsample_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_upsample_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso23generic_upsample_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23generic_upsample_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso23graph_optimization_ruleD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23graph_optimization_ruleD0Ev called");
    return NULL;
}

void* _ZN8Espresso23graph_optimization_ruleD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23graph_optimization_ruleD1Ev called");
    return NULL;
}

void* _ZN8Espresso23graph_optimization_ruleD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23graph_optimization_ruleD2Ev called");
    return NULL;
}

void* _ZN8Espresso23image_async_data_source15get_num_samplesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23image_async_data_source15get_num_samplesEv called");
    return NULL;
}

void* _ZN8Espresso23image_async_data_source22background_load_sampleEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23image_async_data_source22background_load_sampleEm called");
    return NULL;
}

void* _ZN8Espresso23image_async_data_source5resetEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23image_async_data_source5resetEv called");
    return NULL;
}

void* _ZN8Espresso23image_async_data_sourceC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23image_async_data_sourceC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi called");
    return NULL;
}

void* _ZN8Espresso23image_async_data_sourceC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23image_async_data_sourceC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi called");
    return NULL;
}

void* _ZN8Espresso23pass_blob_name_indexing14run_on_networkERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23pass_blob_name_indexing14run_on_networkERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso23pass_blob_name_indexing18get_analysis_usageERNS_19analysis_usage_infoE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23pass_blob_name_indexing18get_analysis_usageERNS_19analysis_usage_infoE called");
    return NULL;
}

void* _ZN8Espresso23report_benchmark_resultENS_16benchmark_resultERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKNS1_10shared_ptrINS_16abstract_contextEEES9_S9_S9_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23report_benchmark_resultENS_16benchmark_resultERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKNS1_10shared_ptrINS_16abstract_contextEEES9_S9_S9_ called");
    return NULL;
}

void* _ZN8Espresso23report_benchmark_resultENS_16benchmark_resultERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_S9_S9_S9_S9_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23report_benchmark_resultENS_16benchmark_resultERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_S9_S9_S9_S9_ called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groups14match_at_layerEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groups14match_at_layerEi called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groups32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groups32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groups8status_tD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groups8status_tD0Ev called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groups8status_tD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groups8status_tD1Ev called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groups8status_tD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groups8status_tD2Ev called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groupsC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groupsC1ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso23rule_convolution_groupsC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso23rule_convolution_groupsC2ENSt3__110shared_ptrINS_26network_optimization_graphEEE called");
    return NULL;
}

void* _ZN8Espresso24bottom_names_from_serdesERNS_6SerDes21generic_serdes_objectE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24bottom_names_from_serdesERNS_6SerDes21generic_serdes_objectE called");
    return NULL;
}

void* _ZN8Espresso24compute_all_output_blobsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24compute_all_output_blobsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso24create_tmp_net_containerERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24create_tmp_net_containerERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso24default_layer_parametersEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24default_layer_parametersEv called");
    return NULL;
}

void* _ZN8Espresso24dropout_multiply_closureclERKNSt3__110shared_ptrINS_14abstract_batchEEERNS2_INS_23abstract_blob_containerEEENS2_INS_4blobIhLi1EEEEEiS9_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24dropout_multiply_closureclERKNSt3__110shared_ptrINS_14abstract_batchEEERNS2_INS_23abstract_blob_containerEEENS2_INS_4blobIhLi1EEEEEiS9_ called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_batchnorm_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_batchnorm_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_bias_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_bias_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_bias_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_bias_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_bias_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_bias_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_broadcast_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_broadcast_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso24generic_broadcast_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_broadcast_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_broadcast_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_broadcast_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_broadcast_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_broadcast_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_cf_branch_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_cf_branch_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_cf_branch_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_cf_branch_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_cf_branch_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_cf_branch_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_cf_return_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_cf_return_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_cf_return_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_cf_return_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_cf_return_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_cf_return_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_gather_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_gather_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso24generic_gather_nd_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_gather_nd_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_gather_nd_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_gather_nd_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_gather_nd_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_gather_nd_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_get_shape_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_get_shape_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso24generic_get_shape_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_get_shape_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_get_shape_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_get_shape_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_get_shape_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_get_shape_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_pool_grad_kernel10on_resizedERNS_10layer_dataES2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_pool_grad_kernel10on_resizedERNS_10layer_dataES2_ called");
    return NULL;
}

void* _ZN8Espresso24generic_pool_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_pool_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso24generic_pool_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_pool_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_pool_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_pool_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_pool_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_pool_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_transpose_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_transpose_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso24generic_transpose_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_transpose_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_transpose_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_transpose_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso24generic_transpose_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24generic_transpose_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso24get_layer_parameter_nameERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_NS_9blob_typeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24get_layer_parameter_nameERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_NS_9blob_typeE called");
    return NULL;
}

void* _ZN8Espresso24get_max_allocated_memoryEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24get_max_allocated_memoryEv called");
    return NULL;
}

void* _ZN8Espresso24kern_alloc_uninitializedEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24kern_alloc_uninitializedEm called");
    return NULL;
}

void* _ZN8Espresso24mxnet_imagebinary_reader15get_num_samplesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24mxnet_imagebinary_reader15get_num_samplesEv called");
    return NULL;
}

void* _ZN8Espresso24mxnet_imagebinary_reader22background_load_sampleEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24mxnet_imagebinary_reader22background_load_sampleEm called");
    return NULL;
}

void* _ZN8Espresso24mxnet_imagebinary_readerC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24mxnet_imagebinary_readerC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi called");
    return NULL;
}

void* _ZN8Espresso24mxnet_imagebinary_readerC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24mxnet_imagebinary_readerC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi called");
    return NULL;
}

void* _ZN8Espresso24prepare_blob_for_networkERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS1_INS_4blobIfLi4EEEEEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24prepare_blob_for_networkERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS1_INS_4blobIfLi4EEEEEb called");
    return NULL;
}

void* _ZN8Espresso24simple_blobs_data_source19get_data_for_sampleEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24simple_blobs_data_source19get_data_for_sampleEm called");
    return NULL;
}

void* _ZN8Espresso24simple_blobs_data_sourceC1ENSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4blobIfLi4EEEEENS1_4lessIS8_EENS6_INS1_4pairIKS8_SC_EEEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24simple_blobs_data_sourceC1ENSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4blobIfLi4EEEEENS1_4lessIS8_EENS6_INS1_4pairIKS8_SC_EEEEEEi called");
    return NULL;
}

void* _ZN8Espresso24simple_blobs_data_sourceC2ENSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4blobIfLi4EEEEENS1_4lessIS8_EENS6_INS1_4pairIKS8_SC_EEEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24simple_blobs_data_sourceC2ENSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4blobIfLi4EEEEENS1_4lessIS8_EENS6_INS1_4pairIKS8_SC_EEEEEEi called");
    return NULL;
}

void* _ZN8Espresso24transformer_optimizationENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24transformer_optimizationENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso24unshuffle_from_gpu_imageENSt3__110shared_ptrINS_4blobIfLi4EEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso24unshuffle_from_gpu_imageENSt3__110shared_ptrINS_4blobIfLi4EEEEEi called");
    return NULL;
}

void* _ZN8Espresso25add_dropout_augment_layerENSt3__110shared_ptrINS_3netEEERKNS_22dropout_augment_paramsENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25add_dropout_augment_layerENSt3__110shared_ptrINS_3netEEERKNS_22dropout_augment_paramsENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso25ane_style_post_scale_biasENSt3__110shared_ptrINS_4blobIfLi1EEEEES4_f(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25ane_style_post_scale_biasENSt3__110shared_ptrINS_4blobIfLi1EEEEES4_f called");
    return NULL;
}

void* _ZN8Espresso25crop_image_to_square_8888E13vImage_BufferRS0_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25crop_image_to_square_8888E13vImage_BufferRS0_i called");
    return NULL;
}

void* _ZN8Espresso25espresso_os_log_subsystemEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25espresso_os_log_subsystemEPKc called");
    return NULL;
}

void* _ZN8Espresso25f4_blob_async_data_source10load_chunkEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25f4_blob_async_data_source10load_chunkEm called");
    return NULL;
}

void* _ZN8Espresso25f4_blob_async_data_source15get_num_samplesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25f4_blob_async_data_source15get_num_samplesEv called");
    return NULL;
}

void* _ZN8Espresso25f4_blob_async_data_source22background_load_sampleEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25f4_blob_async_data_source22background_load_sampleEm called");
    return NULL;
}

void* _ZN8Espresso25f4_blob_async_data_sourceC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25f4_blob_async_data_sourceC1ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi called");
    return NULL;
}

void* _ZN8Espresso25f4_blob_async_data_sourceC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25f4_blob_async_data_sourceC2ENS_17async_data_source7optionsENSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEi called");
    return NULL;
}

void* _ZN8Espresso25generic_activation_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_activation_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso25generic_activation_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_17activation_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_activation_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_17activation_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso25generic_activation_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_activation_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_activation_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_activation_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_activation_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_activation_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_gumbel_max_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_gumbel_max_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso25generic_gumbel_max_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_gumbel_max_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_gumbel_max_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_gumbel_max_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_gumbel_max_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_gumbel_max_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_scatter_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_scatter_nd_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso25generic_scatter_nd_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_scatter_nd_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_scatter_nd_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_scatter_nd_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_scatter_nd_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_scatter_nd_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_softmax_nd_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_softmax_nd_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_softmax_nd_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_softmax_nd_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_softmax_nd_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_softmax_nd_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernel11set_weightsENSt3__110shared_ptrINS_4blobIfLi1EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernel11set_weightsENSt3__110shared_ptrINS_4blobIfLi1EEEEE called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernel19create_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernel19create_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernel20set_batchnorm_paramsENSt3__110shared_ptrINS_4blobIfLi1EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernel20set_batchnorm_paramsENSt3__110shared_ptrINS_4blobIfLi1EEEEE called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernel23create_batchnorm_paramsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernel23create_batchnorm_paramsEv called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso25generic_transplant_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25generic_transplant_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso25pass_global_name_indexing14run_on_networkERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25pass_global_name_indexing14run_on_networkERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso25pass_global_name_indexing18get_analysis_usageERNS_19analysis_usage_infoE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25pass_global_name_indexing18get_analysis_usageERNS_19analysis_usage_infoE called");
    return NULL;
}

void* _ZN8Espresso25statdist_kullback_leiblerEPfS0_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso25statdist_kullback_leiblerEPfS0_i called");
    return NULL;
}

void* _ZN8Espresso26EspressoCreateMetalContextEPU19objcproto9MTLDevice11objc_objectP12NSDictionary(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26EspressoCreateMetalContextEPU19objcproto9MTLDevice11objc_objectP12NSDictionary called");
    return NULL;
}

void* _ZN8Espresso26add_auto_context_transfersENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26add_auto_context_transfersENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso26check_blob_storage_alignedERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26check_blob_storage_alignedERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_ called");
    return NULL;
}

void* _ZN8Espresso26dump_network_binserializerENSt3__110shared_ptrINS_3netEEENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES9_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26dump_network_binserializerENSt3__110shared_ptrINS_3netEEENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES9_ called");
    return NULL;
}

void* _ZN8Espresso26generic_concat_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_concat_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_concat_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_concat_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_concat_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_concat_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel13share_weightsERKNSt3__110shared_ptrINS_11base_kernelEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel13share_weightsERKNSt3__110shared_ptrINS_11base_kernelEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel15set_f16_weightsENSt3__110shared_ptrINS_4blobItLi4EEEEERKNS2_INS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel15set_f16_weightsENSt3__110shared_ptrINS_4blobItLi4EEEEERKNS2_INS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel18create_biases_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel18create_biases_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel18weights_blob_shapeEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel18weights_blob_shapeEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel19create_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel19create_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel24create_weights_fp16_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel24create_weights_fp16_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel28create_quantization_lut_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel28create_quantization_lut_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel28create_shuffled_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel28create_shuffled_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel29create_quantized_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel29create_quantized_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel31create_quantization_ranges_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel31create_quantization_ranges_blobEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel31dequantized_weights_from_rangesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel31dequantized_weights_from_rangesEv called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_20convolution_uniformsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_20convolution_uniformsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_convolution_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_convolution_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_elementwise_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_elementwise_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso26generic_elementwise_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_elementwise_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_elementwise_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_18elementwise_paramsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_elementwise_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_18elementwise_paramsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso26generic_elementwise_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_elementwise_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_elementwise_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_elementwise_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_elementwise_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_elementwise_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_expand_dims_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_expand_dims_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_expand_dims_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_expand_dims_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_expand_dims_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_expand_dims_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_expand_dims_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_expand_dims_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_load_global_kernel11global_nameEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_global_kernel11global_nameEv called");
    return NULL;
}

void* _ZN8Espresso26generic_load_global_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_global_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_load_global_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_global_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_load_global_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_global_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_load_random_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_random_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_load_random_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_random_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_load_random_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_random_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_load_random_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_load_random_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_multinomial_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_multinomial_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_multinomial_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_multinomial_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_multinomial_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_multinomial_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_multinomial_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_multinomial_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_phi_virtual_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_phi_virtual_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_phi_virtual_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_phi_virtual_kernel8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso26generic_phi_virtual_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_phi_virtual_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_phi_virtual_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_phi_virtual_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_phi_virtual_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_phi_virtual_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_reverse_seq_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_reverse_seq_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso26generic_reverse_seq_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_reverse_seq_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_reverse_seq_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_reverse_seq_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso26generic_reverse_seq_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26generic_reverse_seq_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso26load_image_to_vImageBufferEPKcbb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26load_image_to_vImageBufferEPKcbb called");
    return NULL;
}

void* _ZN8Espresso26load_network_binserializerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_RKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26load_network_binserializerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_RKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso26net_compiler_segment_based17identify_segmentsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26net_compiler_segment_based17identify_segmentsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso26net_compiler_segment_based5buildERKNSt3__110shared_ptrINS_14abstract_batchEEEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26net_compiler_segment_based5buildERKNSt3__110shared_ptrINS_14abstract_batchEEEii called");
    return NULL;
}

void* _ZN8Espresso26net_compiler_segment_based9__forwardERKNSt3__110shared_ptrINS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26net_compiler_segment_based9__forwardERKNSt3__110shared_ptrINS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso26network_optimization_graphC1ENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26network_optimization_graphC1ENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso26network_optimization_graphC1EPNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26network_optimization_graphC1EPNS_3netE called");
    return NULL;
}

void* _ZN8Espresso26network_optimization_graphC2ENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26network_optimization_graphC2ENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso26network_optimization_graphC2EPNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26network_optimization_graphC2EPNS_3netE called");
    return NULL;
}

void* _ZN8Espresso26run_custom_pass_on_networkERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26run_custom_pass_on_networkERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso26util_update_metal_priorityENSt3__110shared_ptrINS_16abstract_contextEEEbj(void)
{
    if (verbose) puts("STUB: _ZN8Espresso26util_update_metal_priorityENSt3__110shared_ptrINS_16abstract_contextEEEbj called");
    return NULL;
}

void* _ZN8Espresso27add_context_transfer_beforeENSt3__110shared_ptrINS_3netEEEiNS_8platformE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27add_context_transfer_beforeENSt3__110shared_ptrINS_3netEEEiNS_8platformE called");
    return NULL;
}

void* _ZN8Espresso27convert_to_explicit_paddingENSt3__110shared_ptrINS_3netEEEbb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27convert_to_explicit_paddingENSt3__110shared_ptrINS_3netEEEbb called");
    return NULL;
}

void* _ZN8Espresso27generic_array_gather_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_array_gather_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso27generic_array_gather_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_array_gather_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_array_gather_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_array_gather_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_array_gather_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_array_gather_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_batch_matmul_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_batch_matmul_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso27generic_batch_matmul_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_batch_matmul_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_batch_matmul_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_batch_matmul_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_batch_matmul_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_batch_matmul_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_dropout_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_dropout_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_dropout_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_dropout_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_dropout_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_dropout_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_l2_normalize_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_l2_normalize_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso27generic_l2_normalize_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_l2_normalize_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_l2_normalize_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_l2_normalize_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_l2_normalize_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_l2_normalize_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_store_global_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_store_global_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_store_global_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_store_global_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso27generic_store_global_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27generic_store_global_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso27kernels_validation_status_t7restoreENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27kernels_validation_status_t7restoreENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE called");
    return NULL;
}

void* _ZN8Espresso27pass_constant_if_conditions14run_on_networkERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27pass_constant_if_conditions14run_on_networkERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso27pass_constant_if_conditions18get_analysis_usageERNS_19analysis_usage_infoE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27pass_constant_if_conditions18get_analysis_usageERNS_19analysis_usage_infoE called");
    return NULL;
}

void* _ZN8Espresso27pass_constant_if_conditions37hint_fallback_constant_ifs_from_metalERNS_3netE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27pass_constant_if_conditions37hint_fallback_constant_ifs_from_metalERNS_3netE called");
    return NULL;
}

void* _ZN8Espresso27shuffle_to_gpu_conv_weightsENSt3__110shared_ptrINS_4blobIfLi4EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27shuffle_to_gpu_conv_weightsENSt3__110shared_ptrINS_4blobIfLi4EEEEE called");
    return NULL;
}

void* _ZN8Espresso27util_bake_batchnorm_genericEiiiPKfPfS1_S2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso27util_bake_batchnorm_genericEiiiPKfPfS1_S2_ called");
    return NULL;
}

void* _ZN8Espresso28crop_image_to_square_Planar8E13vImage_BufferRS0_i(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28crop_image_to_square_Planar8E13vImage_BufferRS0_i called");
    return NULL;
}

void* _ZN8Espresso28feed_data_source_to_networksENSt3__110shared_ptrINS_20abstract_data_sourceEEENS0_6vectorINS1_INS_3netEEENS0_9allocatorIS6_EEEENS_23feed_data_source_opts_tEU13block_pointerFvRNS4_INS_21map_with_ordered_keysINS0_12basic_stringIcNS0_11char_traitsIcEENS7_IcEEEENS1_INS_4blobIfLi4EEEEEEENS7_ISK_EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28feed_data_source_to_networksENSt3__110shared_ptrINS_20abstract_data_sourceEEENS0_6vectorINS1_INS_3netEEENS0_9allocatorIS6_EEEENS_23feed_data_source_opts_tEU13block_pointerFvRNS4_INS_21map_with_ordered_keysINS0_12basic_stringIcNS0_11char_traitsIcEENS7_IcEEEENS1_INS_4blobIfLi4EEEEEEENS7_ISK_EEEEE called");
    return NULL;
}

void* _ZN8Espresso28fuse_transpose_inner_productENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28fuse_transpose_inner_productENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_compute_stats_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_compute_stats_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_compute_stats_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_compute_stats_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_compute_stats_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_compute_stats_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_compute_stats_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_compute_stats_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_cross_entropy_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_cross_entropy_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_cross_entropy_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_cross_entropy_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_cross_entropy_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_cross_entropy_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_cross_entropy_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_cross_entropy_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel18weights_blob_shapeEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel18weights_blob_shapeEv called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel19create_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel19create_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel28create_shuffled_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel28create_shuffled_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel31dequantized_weights_from_rangesEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel31dequantized_weights_from_rangesEv called");
    return NULL;
}

void* _ZN8Espresso28generic_deconvolution_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_20convolution_uniformsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_deconvolution_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_20convolution_uniformsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso28generic_general_slice_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_general_slice_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_general_slice_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_general_slice_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_general_slice_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_general_slice_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_general_slice_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_general_slice_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel15set_f16_weightsENSt3__110shared_ptrINS_4blobItLi2EEEEERKNS2_INS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel15set_f16_weightsENSt3__110shared_ptrINS_4blobItLi2EEEEERKNS2_INS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel18create_biases_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel18create_biases_blobEv called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel19create_weights_blobEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel19create_weights_blobEv called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel22__set_weights_finalizeERKNSt3__110shared_ptrINS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel22__set_weights_finalizeERKNSt3__110shared_ptrINS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel22build_grads_for_inputsERNS_18sequential_builderERNS_18build_grads_paramsE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel27estimated_compute_resourcesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_22inner_product_uniformsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_22inner_product_uniformsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_inner_product_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_inner_product_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_load_constant_kernel13share_weightsERKNSt3__110shared_ptrINS_11base_kernelEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_load_constant_kernel13share_weightsERKNSt3__110shared_ptrINS_11base_kernelEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_load_constant_kernel17set_constant_blobENSt3__110shared_ptrINS_4blobIfLi4EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_load_constant_kernel17set_constant_blobENSt3__110shared_ptrINS_4blobIfLi4EEEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_load_constant_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_load_constant_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso28generic_load_constant_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_load_constant_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_load_constant_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_load_constant_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso28generic_load_constant_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28generic_load_constant_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso28load_network_layers_internalENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEERKNS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS1_INS_16abstract_contextEEERKNS_13network_shapeEPNS0_13basic_istreamIcS7_EENS_12compute_pathEbRKNS1_INS_21blob_storage_abstractEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso28load_network_layers_internalENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEERKNS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKNS1_INS_16abstract_contextEEERKNS_13network_shapeEPNS0_13basic_istreamIcS7_EENS_12compute_pathEbRKNS1_INS_21blob_storage_abstractEEE called");
    return NULL;
}

void* _ZN8Espresso29abstract_kernel_with_weights_13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29abstract_kernel_with_weights_13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso29abstract_kernel_with_weights_39__set_weight_raw_blob_for_serializationERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29abstract_kernel_with_weights_39__set_weight_raw_blob_for_serializationERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso29dispatch_error_use_cpu_scriptD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29dispatch_error_use_cpu_scriptD0Ev called");
    return NULL;
}

void* _ZN8Espresso29dispatch_error_use_cpu_scriptD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29dispatch_error_use_cpu_scriptD1Ev called");
    return NULL;
}

void* _ZN8Espresso29dispatch_error_use_cpu_scriptD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29dispatch_error_use_cpu_scriptD2Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_batchnorm_grad_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_batchnorm_grad_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso29generic_batchnorm_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_batchnorm_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso29generic_batchnorm_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_batchnorm_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_batchnorm_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_batchnorm_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_batchnorm_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_batchnorm_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_general_concat_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_general_concat_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso29generic_general_concat_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_general_concat_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_general_concat_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_general_concat_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_general_concat_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_general_concat_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_space_to_depth_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_space_to_depth_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso29generic_space_to_depth_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_space_to_depth_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_space_to_depth_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_space_to_depth_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso29generic_space_to_depth_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29generic_space_to_depth_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso29get_vImageBuffer_from_CGImageEP7CGImage(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29get_vImageBuffer_from_CGImageEP7CGImage called");
    return NULL;
}

void* _ZN8Espresso29low_precision_accumulation_fcENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29low_precision_accumulation_fcENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso29shuffle_to_gpu_conv_weights_8ENSt3__110shared_ptrINS_4blobIhLi4EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29shuffle_to_gpu_conv_weights_8ENSt3__110shared_ptrINS_4blobIhLi4EEEEE called");
    return NULL;
}

void* _ZN8Espresso29wisdom_parameterset_to_stringERKNSt3__13mapINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEEiNS0_4lessIS7_EENS5_INS0_4pairIKS7_iEEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso29wisdom_parameterset_to_stringERKNSt3__13mapINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEEiNS0_4lessIS7_EENS5_INS0_4pairIKS7_iEEEEEE called");
    return NULL;
}

void* _ZN8Espresso30add_dropout_augment_join_layerENSt3__110shared_ptrINS_3netEEERKNS_22dropout_augment_paramsENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30add_dropout_augment_join_layerENSt3__110shared_ptrINS_3netEEERKNS_22dropout_augment_paramsENS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso30dispatch_error_use_mps_buffersD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30dispatch_error_use_mps_buffersD0Ev called");
    return NULL;
}

void* _ZN8Espresso30dispatch_error_use_mps_buffersD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30dispatch_error_use_mps_buffersD1Ev called");
    return NULL;
}

void* _ZN8Espresso30dispatch_error_use_mps_buffersD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30dispatch_error_use_mps_buffersD2Ev called");
    return NULL;
}

void* _ZN8Espresso30emit_kernel_validation_warningENSt3__110shared_ptrINS_27kernels_validation_status_tEEERKNS_11base_kernel29validate_for_engine_warning_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30emit_kernel_validation_warningENSt3__110shared_ptrINS_27kernels_validation_status_tEEERKNS_11base_kernel29validate_for_engine_warning_tE called");
    return NULL;
}

void* _ZN8Espresso30generic_activation_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_activation_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_activation_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_activation_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_activation_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_activation_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_crop_and_resize_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_crop_and_resize_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso30generic_crop_and_resize_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_crop_and_resize_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_crop_and_resize_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_crop_and_resize_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_crop_and_resize_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_crop_and_resize_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_general_padding_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_general_padding_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso30generic_general_padding_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_general_padding_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_general_padding_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_general_padding_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_general_padding_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_general_padding_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_instancenorm_1d_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_instancenorm_1d_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso30generic_instancenorm_1d_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_instancenorm_1d_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_instancenorm_1d_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_instancenorm_1d_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_instancenorm_1d_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_instancenorm_1d_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_patch_extractor_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_patch_extractor_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso30generic_patch_extractor_kernel35get_homography_from_4_source_pointsEPfS1_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_patch_extractor_kernel35get_homography_from_4_source_pointsEPfS1_ called");
    return NULL;
}

void* _ZN8Espresso30generic_patch_extractor_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_patch_extractor_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_patch_extractor_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_patch_extractor_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_patch_extractor_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_patch_extractor_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_sliding_windows_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_sliding_windows_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso30generic_sliding_windows_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_sliding_windows_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_sliding_windows_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_sliding_windows_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_sliding_windows_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_sliding_windows_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_tensor_zero_pad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_tensor_zero_pad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso30generic_tensor_zero_pad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_tensor_zero_pad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_tensor_zero_pad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_tensor_zero_pad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso30generic_tensor_zero_pad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30generic_tensor_zero_pad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_simple14match_at_layerEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_simple14match_at_layerEi called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_simple32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_simple32transform_and_return_skip_layersENSt3__110shared_ptrINS_18sequential_builderEEENS2_INS_30graph_optimization_rule_statusEEE called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_simpleD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_simpleD0Ev called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_simpleD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_simpleD1Ev called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_simpleD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_simpleD2Ev called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_statusD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_statusD0Ev called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_statusD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_statusD1Ev called");
    return NULL;
}

void* _ZN8Espresso30graph_optimization_rule_statusD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30graph_optimization_rule_statusD2Ev called");
    return NULL;
}

void* _ZN8Espresso30pass_blob_name_indexing_resultD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30pass_blob_name_indexing_resultD0Ev called");
    return NULL;
}

void* _ZN8Espresso30pass_blob_name_indexing_resultD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30pass_blob_name_indexing_resultD1Ev called");
    return NULL;
}

void* _ZN8Espresso30pass_blob_name_indexing_resultD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30pass_blob_name_indexing_resultD2Ev called");
    return NULL;
}

void* _ZN8Espresso30reshape_network_graph_coloringENSt3__110shared_ptrINS_3netEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso30reshape_network_graph_coloringENSt3__110shared_ptrINS_3netEEEi called");
    return NULL;
}

void* _ZN8Espresso31abstract_blob_container_optionsD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31abstract_blob_container_optionsD0Ev called");
    return NULL;
}

void* _ZN8Espresso31abstract_blob_container_optionsD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31abstract_blob_container_optionsD1Ev called");
    return NULL;
}

void* _ZN8Espresso31abstract_blob_container_optionsD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31abstract_blob_container_optionsD2Ev called");
    return NULL;
}

void* _ZN8Espresso31cf_kernel_with_br_label_to_bindD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31cf_kernel_with_br_label_to_bindD0Ev called");
    return NULL;
}

void* _ZN8Espresso31cf_kernel_with_br_label_to_bindD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31cf_kernel_with_br_label_to_bindD1Ev called");
    return NULL;
}

void* _ZN8Espresso31cf_kernel_with_br_label_to_bindD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31cf_kernel_with_br_label_to_bindD2Ev called");
    return NULL;
}

void* _ZN8Espresso31dump_network_layers_blobs_state5writeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEP7__sFILE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31dump_network_layers_blobs_state5writeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEP7__sFILE called");
    return NULL;
}

void* _ZN8Espresso31generic_context_transfer_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_context_transfer_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_context_transfer_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_context_transfer_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_context_transfer_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_context_transfer_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_convolution_grad_kernel13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_convolution_grad_kernel13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso31generic_convolution_grad_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_convolution_grad_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso31generic_convolution_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_convolution_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso31generic_convolution_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_convolution_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_convolution_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_convolution_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_convolution_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_convolution_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_dynamic_quantize_kernel19number_of_top_blobsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_dynamic_quantize_kernel19number_of_top_blobsEv called");
    return NULL;
}

void* _ZN8Espresso31generic_dynamic_quantize_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_dynamic_quantize_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso31generic_dynamic_quantize_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_dynamic_quantize_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_dynamic_quantize_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_dynamic_quantize_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_dynamic_quantize_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_dynamic_quantize_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_elementwise_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_elementwise_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_elementwise_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_elementwise_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_elementwise_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_elementwise_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_matrix_band_part_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_matrix_band_part_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso31generic_matrix_band_part_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_matrix_band_part_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_matrix_band_part_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_matrix_band_part_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso31generic_matrix_band_part_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31generic_matrix_band_part_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso31load_image_to_vImageBuffer_stbiEPKci(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31load_image_to_vImageBuffer_stbiEPKci called");
    return NULL;
}

void* _ZN8Espresso31reshape_networks_graph_coloringENSt3__16vectorINS0_10shared_ptrINS_3netEEENS0_9allocatorIS4_EEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31reshape_networks_graph_coloringENSt3__16vectorINS0_10shared_ptrINS_3netEEENS0_9allocatorIS4_EEEEi called");
    return NULL;
}

void* _ZN8Espresso31unshuffle_from_gpu_conv_weightsENSt3__110shared_ptrINS_4blobIfLi6EEEEEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso31unshuffle_from_gpu_conv_weightsENSt3__110shared_ptrINS_4blobIfLi6EEEEEii called");
    return NULL;
}

void* _ZN8Espresso32add_context_transfer_algos_hintsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32add_context_transfer_algos_hintsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso32cast_indexes_to_n_bits_ac_formatEPhmS0_mm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32cast_indexes_to_n_bits_ac_formatEPhmS0_mm called");
    return NULL;
}

void* _ZN8Espresso32generic_l2_normalize_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32generic_l2_normalize_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso32generic_l2_normalize_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32generic_l2_normalize_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso32generic_l2_normalize_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32generic_l2_normalize_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso32generic_softmax_loss_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32generic_softmax_loss_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso32generic_softmax_loss_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32generic_softmax_loss_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso32generic_softmax_loss_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32generic_softmax_loss_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso32pass_global_name_indexing_resultD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32pass_global_name_indexing_resultD0Ev called");
    return NULL;
}

void* _ZN8Espresso32pass_global_name_indexing_resultD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32pass_global_name_indexing_resultD1Ev called");
    return NULL;
}

void* _ZN8Espresso32pass_global_name_indexing_resultD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso32pass_global_name_indexing_resultD2Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_dynamic_dequantize_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_dynamic_dequantize_kernel21__set_weight_raw_blobERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvmPNS_11base_kernel18set_weight_optionsE called");
    return NULL;
}

void* _ZN8Espresso33generic_dynamic_dequantize_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_dynamic_dequantize_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso33generic_dynamic_dequantize_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_dynamic_dequantize_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_dynamic_dequantize_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_dynamic_dequantize_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_dynamic_dequantize_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_dynamic_dequantize_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_inner_product_grad_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_inner_product_grad_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso33generic_inner_product_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_inner_product_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_inner_product_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_inner_product_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_inner_product_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_inner_product_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso33generic_load_constant_kernel_impl10on_resizedERNS_10layer_dataES2_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_load_constant_kernel_impl10on_resizedERNS_10layer_dataES2_ called");
    return NULL;
}

void* _ZN8Espresso33generic_load_constant_kernel_impl8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33generic_load_constant_kernel_impl8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso33load_network_layers_binserializerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_RKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33load_network_layers_binserializerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_RKNS0_10shared_ptrINS_16abstract_contextEEENS_12compute_pathEb called");
    return NULL;
}

void* _ZN8Espresso33load_network_layers_binserializerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_RKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso33load_network_layers_binserializerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_RKNS0_10shared_ptrINS_16abstract_contextEEERKNS_13network_shapeENS_12compute_pathEb called");
    return NULL;
}

void* _ZN8Espresso34abstract_kernel_with_weights_grad_13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34abstract_kernel_with_weights_grad_13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso34abstract_kernel_with_weights_grad_D0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34abstract_kernel_with_weights_grad_D0Ev called");
    return NULL;
}

void* _ZN8Espresso34abstract_kernel_with_weights_grad_D1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34abstract_kernel_with_weights_grad_D1Ev called");
    return NULL;
}

void* _ZN8Espresso34abstract_kernel_with_weights_grad_D2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34abstract_kernel_with_weights_grad_D2Ev called");
    return NULL;
}

void* _ZN8Espresso34dispatch_error_fallback_at_c_levelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34dispatch_error_fallback_at_c_levelD0Ev called");
    return NULL;
}

void* _ZN8Espresso34dispatch_error_fallback_at_c_levelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34dispatch_error_fallback_at_c_levelD1Ev called");
    return NULL;
}

void* _ZN8Espresso34dispatch_error_fallback_at_c_levelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34dispatch_error_fallback_at_c_levelD2Ev called");
    return NULL;
}

void* _ZN8Espresso34generic_precompiled_segment_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_precompiled_segment_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso34generic_precompiled_segment_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_precompiled_segment_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso34generic_precompiled_segment_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_precompiled_segment_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso34generic_spatial_transformer_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_spatial_transformer_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso34generic_spatial_transformer_kernel35get_homography_from_4_source_pointsEPfS1_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_spatial_transformer_kernel35get_homography_from_4_source_pointsEPfS1_ called");
    return NULL;
}

void* _ZN8Espresso34generic_spatial_transformer_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_28spatial_transformer_params_tENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_spatial_transformer_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS_28spatial_transformer_params_tENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso34generic_spatial_transformer_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_spatial_transformer_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso34generic_spatial_transformer_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_spatial_transformer_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso34generic_spatial_transformer_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34generic_spatial_transformer_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso34pass_constant_if_conditions_result11deserializeERKNSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso34pass_constant_if_conditions_result11deserializeERKNSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE called");
    return NULL;
}

void* _ZN8Espresso35abstract_kernel_with_named_weights_13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35abstract_kernel_with_named_weights_13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso35abstract_kernel_with_named_weights_D0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35abstract_kernel_with_named_weights_D0Ev called");
    return NULL;
}

void* _ZN8Espresso35abstract_kernel_with_named_weights_D1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35abstract_kernel_with_named_weights_D1Ev called");
    return NULL;
}

void* _ZN8Espresso35abstract_kernel_with_named_weights_D2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35abstract_kernel_with_named_weights_D2Ev called");
    return NULL;
}

void* _ZN8Espresso35fill_blob_with_images_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEENS1_INS_23abstract_blob_containerEEERKNS0_6vectorI13vImage_BufferNS0_9allocatorIS9_EEEEiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35fill_blob_with_images_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEENS1_INS_23abstract_blob_containerEEERKNS0_6vectorI13vImage_BufferNS0_9allocatorIS9_EEEEiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramE called");
    return NULL;
}

void* _ZN8Espresso35fuse_eltwise_mulc_max_to_leaky_reluENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35fuse_eltwise_mulc_max_to_leaky_reluENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso35generic_prediction_smoothing_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35generic_prediction_smoothing_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso35generic_prediction_smoothing_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35generic_prediction_smoothing_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso35generic_prediction_smoothing_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso35generic_prediction_smoothing_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso36enable_training_kernels_for_platformENS_8platformEPNS_23abstract_engine_factoryE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso36enable_training_kernels_for_platformENS_8platformEPNS_23abstract_engine_factoryE called");
    return NULL;
}

void* _ZN8Espresso36generic_context_transfer_kernel_impl8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso36generic_context_transfer_kernel_impl8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEERNS_10layer_dataES8_ called");
    return NULL;
}

void* _ZN8Espresso37fill_network_with_image_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEERK13vImage_BufferiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso37fill_network_with_image_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEERK13vImage_BufferiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramE called");
    return NULL;
}

void* _ZN8Espresso37swap_zero_to_first_position_ac_formatEPfPhmm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso37swap_zero_to_first_position_ac_formatEPfPhmm called");
    return NULL;
}

void* _ZN8Espresso38fill_network_with_images_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEERKNS0_6vectorI13vImage_BufferNS0_9allocatorISB_EEEEiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38fill_network_with_images_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEERKNS0_6vectorI13vImage_BufferNS0_9allocatorISB_EEEEiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramE called");
    return NULL;
}

void* _ZN8Espresso38fill_network_with_images_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEERKNS0_6vectorI13vImage_BufferNS0_9allocatorISB_EEEEiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramENS0_12basic_stringIcNS0_11char_traitsIcEENSC_IcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38fill_network_with_images_batch_safe_v2ERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS1_INS_3netEEERKNS0_6vectorI13vImage_BufferNS0_9allocatorISB_EEEEiPNS1_INS_4blobIfLi4EEEEERKNS_21vimage2espresso_paramENS0_12basic_stringIcNS0_11char_traitsIcEENSC_IcEEEE called");
    return NULL;
}

void* _ZN8Espresso38generic_inner_product_quantized_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_22inner_product_uniformsENS_8platformENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38generic_inner_product_quantized_kernel4makeERKNSt3__110shared_ptrINS_16abstract_contextEEENS_22inner_product_uniformsENS_8platformENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso38generic_non_maximum_suppression_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38generic_non_maximum_suppression_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso38generic_non_maximum_suppression_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38generic_non_maximum_suppression_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso38generic_non_maximum_suppression_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38generic_non_maximum_suppression_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso38generic_non_maximum_suppression_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38generic_non_maximum_suppression_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso38prepare_network_reshape_graph_coloringENSt3__110shared_ptrINS_3netEEERKNS_13network_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38prepare_network_reshape_graph_coloringENSt3__110shared_ptrINS_3netEEERKNS_13network_shapeE called");
    return NULL;
}

void* _ZN8Espresso38shape_network_and_keep_unchanged_blobsENSt3__110shared_ptrINS_3netEEERKNS_13network_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso38shape_network_and_keep_unchanged_blobsENSt3__110shared_ptrINS_3netEEERKNS_13network_shapeE called");
    return NULL;
}

void* _ZN8Espresso39generic_convolution_weights_grad_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso39generic_convolution_weights_grad_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso39generic_convolution_weights_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso39generic_convolution_weights_grad_kernel28top_shapes_for_bottom_shapesERKNSt3__16vectorINS_11layer_shapeENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso39generic_convolution_weights_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso39generic_convolution_weights_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso39generic_convolution_weights_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso39generic_convolution_weights_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso39generic_convolution_weights_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso39generic_convolution_weights_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso3net11_on_resizedEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net11_on_resizedEv called");
    return NULL;
}

void* _ZN8Espresso3net11swap_globalERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS1_10shared_ptrINS_23abstract_blob_containerEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net11swap_globalERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS1_10shared_ptrINS_23abstract_blob_containerEEE called");
    return NULL;
}

void* _ZN8Espresso3net12remove_layerEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net12remove_layerEi called");
    return NULL;
}

void* _ZN8Espresso3net12setup_scriptENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEENS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net12setup_scriptENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEENS_12compute_pathE called");
    return NULL;
}

void* _ZN8Espresso3net13insert_layersERKNSt3__16vectorINS0_14new_layer_infoENS1_9allocatorIS3_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net13insert_layersERKNSt3__16vectorINS0_14new_layer_infoENS1_9allocatorIS3_EEEE called");
    return NULL;
}

void* _ZN8Espresso3net13insert_layersERKiRKNSt3__16vectorINS0_14new_layer_infoENS3_9allocatorIS5_EEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net13insert_layersERKiRKNSt3__16vectorINS0_14new_layer_infoENS3_9allocatorIS5_EEEE called");
    return NULL;
}

void* _ZN8Espresso3net13shape_networkERKNS_13network_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net13shape_networkERKNS_13network_shapeE called");
    return NULL;
}

void* _ZN8Espresso3net13share_weightsERKNSt3__110shared_ptrIS0_EE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net13share_weightsERKNSt3__110shared_ptrIS0_EE called");
    return NULL;
}

void* _ZN8Espresso3net15_assign_streamsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net15_assign_streamsEv called");
    return NULL;
}

void* _ZN8Espresso3net15is_blob_dynamicEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net15is_blob_dynamicEPKc called");
    return NULL;
}

void* _ZN8Espresso3net16try_fast_reshapeERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net16try_fast_reshapeERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS_11layer_shapeENS1_4lessIS8_EENS6_INS1_4pairIKS8_S9_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso3net17_tune_concurrencyEib(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net17_tune_concurrencyEib called");
    return NULL;
}

void* _ZN8Espresso3net17needs_interpreterEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net17needs_interpreterEv called");
    return NULL;
}

void* _ZN8Espresso3net17wipe_layers_blobsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net17wipe_layers_blobsEv called");
    return NULL;
}

void* _ZN8Espresso3net18loaded_global_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net18loaded_global_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso3net18rewire_layer_inputENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES7_S7_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net18rewire_layer_inputENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES7_S7_ called");
    return NULL;
}

void* _ZN8Espresso3net20copy_parameters_fromERKS0_(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net20copy_parameters_fromERKS0_ called");
    return NULL;
}

void* _ZN8Espresso3net22__reset_temporal_stateERKNSt3__110shared_ptrINS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net22__reset_temporal_stateERKNSt3__110shared_ptrINS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso3net23_tag_inputs_and_outputsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net23_tag_inputs_and_outputsEv called");
    return NULL;
}

void* _ZN8Espresso3net23recurse_layer_producersERKNSt3__113unordered_mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEmNS1_4hashIS8_EENS1_8equal_toIS8_EENS6_INS1_4pairIKS8_mEEEEEEmRNS1_6vectorImNS6_ImEEEERNSK_IS8_NS6_IS8_EEEEU13block_pointerFbmE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net23recurse_layer_producersERKNSt3__113unordered_mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEmNS1_4hashIS8_EENS1_8equal_toIS8_EENS6_INS1_4pairIKS8_mEEEEEEmRNS1_6vectorImNS6_ImEEEERNSK_IS8_NS6_IS8_EEEEU13block_pointerFbmE called");
    return NULL;
}

void* _ZN8Espresso3net23restore_layers_if_wipedEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net23restore_layers_if_wipedEv called");
    return NULL;
}

void* _ZN8Espresso3net24get_stride_configurationENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net24get_stride_configurationENSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso3net24iterate_all_ssa_versionsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEU13block_pointerFvS9_E(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net24iterate_all_ssa_versionsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEU13block_pointerFvS9_E called");
    return NULL;
}

void* _ZN8Espresso3net25_bench_layer_and_childrenEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net25_bench_layer_and_childrenEii called");
    return NULL;
}

void* _ZN8Espresso3net26_allocate_temporary_bufferEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net26_allocate_temporary_bufferEv called");
    return NULL;
}

void* _ZN8Espresso3net27restore_precompilation_infoENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net27restore_precompilation_infoENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE called");
    return NULL;
}

void* _ZN8Espresso3net28_infer_default_configurationEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net28_infer_default_configurationEv called");
    return NULL;
}

void* _ZN8Espresso3net29transform_parameters_as_inputEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net29transform_parameters_as_inputEv called");
    return NULL;
}

void* _ZN8Espresso3net31get_estimated_compute_resourcesEmm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net31get_estimated_compute_resourcesEmm called");
    return NULL;
}

void* _ZN8Espresso3net33restore_additional_configurationsENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net33restore_additional_configurationsENSt3__110shared_ptrINS_6SerDes21generic_serdes_objectEEE called");
    return NULL;
}

void* _ZN8Espresso3net36transform_transfer_global_parametersEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net36transform_transfer_global_parametersEv called");
    return NULL;
}

void* _ZN8Espresso3net6_benchEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net6_benchEi called");
    return NULL;
}

void* _ZN8Espresso3net7__buildERKNSt3__110shared_ptrINS_14abstract_batchEEEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net7__buildERKNSt3__110shared_ptrINS_14abstract_batchEEEii called");
    return NULL;
}

void* _ZN8Espresso3net9__forwardERKNSt3__110shared_ptrINS_14abstract_batchEEEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net9__forwardERKNSt3__110shared_ptrINS_14abstract_batchEEEii called");
    return NULL;
}

void* _ZN8Espresso3net9get_gflopEii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso3net9get_gflopEii called");
    return NULL;
}

void* _ZN8Espresso40abstract_blob_container_options_aliasingD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso40abstract_blob_container_options_aliasingD0Ev called");
    return NULL;
}

void* _ZN8Espresso40abstract_blob_container_options_aliasingD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso40abstract_blob_container_options_aliasingD1Ev called");
    return NULL;
}

void* _ZN8Espresso40abstract_blob_container_options_aliasingD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso40abstract_blob_container_options_aliasingD2Ev called");
    return NULL;
}

void* _ZN8Espresso41classic_hx_engine_backwards_compatibilityENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso41classic_hx_engine_backwards_compatibilityENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso41convert_hx_engine_weights_to_peace_formatENSt3__110shared_ptrINS_3netEEEb(void)
{
    if (verbose) puts("STUB: _ZN8Espresso41convert_hx_engine_weights_to_peace_formatENSt3__110shared_ptrINS_3netEEEb called");
    return NULL;
}

void* _ZN8Espresso41generic_inner_product_weights_grad_kernel15input_blob_typeEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso41generic_inner_product_weights_grad_kernel15input_blob_typeEm called");
    return NULL;
}

void* _ZN8Espresso41generic_inner_product_weights_grad_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso41generic_inner_product_weights_grad_kernelD0Ev called");
    return NULL;
}

void* _ZN8Espresso41generic_inner_product_weights_grad_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso41generic_inner_product_weights_grad_kernelD1Ev called");
    return NULL;
}

void* _ZN8Espresso41generic_inner_product_weights_grad_kernelD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso41generic_inner_product_weights_grad_kernelD2Ev called");
    return NULL;
}

void* _ZN8Espresso42fuse_log_softmax_multinomial_to_gumbel_maxENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso42fuse_log_softmax_multinomial_to_gumbel_maxENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso46compute_network_shape_starting_from_data_shapeENSt3__110shared_ptrINS_3netEEERKNS_11layer_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso46compute_network_shape_starting_from_data_shapeENSt3__110shared_ptrINS_3netEEERKNS_11layer_shapeE called");
    return NULL;
}

void* _ZN8Espresso47compute_network_shape_starting_from_data_shapesENSt3__110shared_ptrINS_3netEEERKNS0_3mapINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS_11layer_shapeENS0_4lessISA_EENS8_INS0_4pairIKSA_SB_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso47compute_network_shape_starting_from_data_shapesENSt3__110shared_ptrINS_3netEEERKNS0_3mapINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS_11layer_shapeENS0_4lessISA_EENS8_INS0_4pairIKSA_SB_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso47compute_network_shape_starting_from_data_shapesEPNS_3netERKNSt3__13mapINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS_11layer_shapeENS2_4lessIS9_EENS7_INS2_4pairIKS9_SA_EEEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso47compute_network_shape_starting_from_data_shapesEPNS_3netERKNSt3__13mapINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS_11layer_shapeENS2_4lessIS9_EENS7_INS2_4pairIKS9_SA_EEEEEE called");
    return NULL;
}

void* _ZN8Espresso49auto_add_auto_temporal_smoothing_before_softmaxesENSt3__110shared_ptrINS_3netEEENS_27prediction_smoothing_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso49auto_add_auto_temporal_smoothing_before_softmaxesENSt3__110shared_ptrINS_3netEEENS_27prediction_smoothing_paramsE called");
    return NULL;
}

void* _ZN8Espresso4passD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso4passD0Ev called");
    return NULL;
}

void* _ZN8Espresso4passD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso4passD1Ev called");
    return NULL;
}

void* _ZN8Espresso4passD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso4passD2Ev called");
    return NULL;
}

void* _ZN8Espresso55reshape_networks_graph_coloring_raw_ptr_only_in_contextERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS0_6vectorIPNS_3netENS0_9allocatorIS8_EEEEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso55reshape_networks_graph_coloring_raw_ptr_only_in_contextERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS0_6vectorIPNS_3netENS0_9allocatorIS8_EEEEi called");
    return NULL;
}

void* _ZN8Espresso5debug18read_vImage_BufferEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5debug18read_vImage_BufferEPKc called");
    return NULL;
}

void* _ZN8Espresso5debug19write_vImage_BufferEPKc13vImage_Buffer(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5debug19write_vImage_BufferEPKc13vImage_Buffer called");
    return NULL;
}

void* _ZN8Espresso5layer13clear_weightsEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5layer13clear_weightsEv called");
    return NULL;
}

void* _ZN8Espresso5layer13share_weightsERKNSt3__110shared_ptrIS0_EE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5layer13share_weightsERKNSt3__110shared_ptrIS0_EE called");
    return NULL;
}

void* _ZN8Espresso5layer20syntetic_descriptionEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5layer20syntetic_descriptionEv called");
    return NULL;
}

void* _ZN8Espresso5layer24layer_training_transformERNS_3netEmRKNS_26training_transform_optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5layer24layer_training_transformERNS_3netEmRKNS_26training_transform_optionsE called");
    return NULL;
}

void* _ZN8Espresso5layer36transform_transfer_global_parametersERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_23abstract_blob_containerEEENS1_4lessIS8_EENS6_INS1_4pairIKS8_SB_EEEEEERNS_3netEm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5layer36transform_transfer_global_parametersERKNSt3__13mapINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_23abstract_blob_containerEEENS1_4lessIS8_EENS6_INS1_4pairIKS8_SB_EEEEEERNS_3netEm called");
    return NULL;
}

void* _ZN8Espresso5layer8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso5layer8__launchERKNSt3__110shared_ptrINS_14abstract_batchEEE called");
    return NULL;
}

void* _ZN8Espresso6SerDes13layer_io_base27factory_initialize_registryEPNS_9factory_tIS1_EE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes13layer_io_base27factory_initialize_registryEPNS_9factory_tIS1_EE called");
    return NULL;
}

void* _ZN8Espresso6SerDes13layer_io_baseD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes13layer_io_baseD0Ev called");
    return NULL;
}

void* _ZN8Espresso6SerDes13layer_io_baseD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes13layer_io_baseD1Ev called");
    return NULL;
}

void* _ZN8Espresso6SerDes13layer_io_baseD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes13layer_io_baseD2Ev called");
    return NULL;
}

void* _ZN8Espresso6SerDes21generic_serdes_object27factory_initialize_registryEPNS_9factory_tIS1_EE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes21generic_serdes_object27factory_initialize_registryEPNS_9factory_tIS1_EE called");
    return NULL;
}

void* _ZN8Espresso6SerDes21generic_serdes_objectD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes21generic_serdes_objectD0Ev called");
    return NULL;
}

void* _ZN8Espresso6SerDes21generic_serdes_objectD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes21generic_serdes_objectD1Ev called");
    return NULL;
}

void* _ZN8Espresso6SerDes21generic_serdes_objectD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes21generic_serdes_objectD2Ev called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_10lrn_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_10lrn_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_10rnn_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_10rnn_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_11bias_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_11bias_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_11crop_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_11crop_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_11layer_shapeEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_11layer_shapeEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_11pool_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_11pool_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_12axpby_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_12axpby_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_12brick_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_12brick_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_12sgd_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_12sgd_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_12split_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_12split_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13adam_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13adam_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13cast_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13cast_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13concat_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13concat_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13copy_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13copy_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13fill_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13fill_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13tile_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13tile_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_13topk_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_13topk_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14alloc_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14alloc_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14cf_if_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14cf_if_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14dropout_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14dropout_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14flatten_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14flatten_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14l2_norm_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14l2_norm_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14range_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14range_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14reshape_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14reshape_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14slice_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14slice_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_14softmax_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_14softmax_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15cf_end_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15cf_end_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15cf_for_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15cf_for_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15cumsum_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15cumsum_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15dev_op_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15dev_op_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15gather_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15gather_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15lambda_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15lambda_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15reduce_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15reduce_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_15rnn_arch_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_15rnn_arch_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16batchnorm_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16batchnorm_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16builtin_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16builtin_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16cf_call_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16cf_call_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16cf_else_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16cf_else_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16cf_jump_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16cf_jump_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16cf_loop_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16cf_loop_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16nonzero_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16nonzero_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16one_hot_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16one_hot_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16scatter_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16scatter_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_16squeeze_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_16squeeze_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_17activation_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_17activation_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_17split_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_17split_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_17stack_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_17stack_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18broadcast_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18broadcast_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18cf_branch_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18cf_branch_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18cf_return_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18cf_return_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18elementwise_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18elementwise_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18gather_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18gather_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18get_shape_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18get_shape_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_18transpose_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_18transpose_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_19gumbel_max_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_19gumbel_max_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_19scatter_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_19scatter_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_19softmax_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_19softmax_nd_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_19transplant_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_19transplant_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20convolution_uniformsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20convolution_uniformsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20expand_dims_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20expand_dims_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20load_constant_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20load_constant_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20load_global_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20load_global_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20load_random_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20load_random_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20multinomial_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20multinomial_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20phi_virtual_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20phi_virtual_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_20reverse_seq_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_20reverse_seq_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_21array_gather_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_21array_gather_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_21batch_matmul_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_21batch_matmul_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_21store_global_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_21store_global_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_21vimage2espresso_paramEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_21vimage2espresso_paramEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22compute_stats_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22compute_stats_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22cross_entropy_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22cross_entropy_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22general_padding_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22general_padding_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22general_slice_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22general_slice_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22inner_product_uniformsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22inner_product_uniformsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22sequence_concat_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22sequence_concat_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22sequence_repeat_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22sequence_repeat_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_22upsample_kernel_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_22upsample_kernel_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_23context_transfer_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_23context_transfer_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_23fused_kernel_parametersEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_23fused_kernel_parametersEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_23general_concat_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_23general_concat_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_23no_op_kernel_parametersEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_23no_op_kernel_parametersEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_23sequence_reverse_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_23sequence_reverse_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_23space_to_depth_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_23space_to_depth_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_24crop_and_resize_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_24crop_and_resize_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_24instancenorm_1d_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_24instancenorm_1d_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_24patch_extractor_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_24patch_extractor_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_24sliding_windows_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_24sliding_windows_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_24tensor_zero_pad_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_24tensor_zero_pad_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_25blob_numerical_propertiesEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_25blob_numerical_propertiesEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_25dynamic_quantize_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_25dynamic_quantize_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_25matrix_band_part_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_25matrix_band_part_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_27dynamic_dequantize_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_27dynamic_dequantize_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_27prediction_smoothing_paramsEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_27prediction_smoothing_paramsEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_28precompiled_segment_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_28precompiled_segment_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_28spatial_transformer_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_28spatial_transformer_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDes9params_ioINS_32non_maximum_suppression_params_tEEEvRNS0_21generic_serdes_objectERT_b(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDes9params_ioINS_32non_maximum_suppression_params_tEEEvRNS0_21generic_serdes_objectERT_b called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_10lrn_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_10lrn_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_10rnn_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_10rnn_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11bias_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11bias_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11crop_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11crop_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11layer_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11layer_shapeE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11pool_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_11pool_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12axpby_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12axpby_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12brick_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12brick_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12sgd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12sgd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12split_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_12split_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13adam_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13adam_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13cast_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13cast_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13concat_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13concat_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13copy_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13copy_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13fill_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13fill_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13tile_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13tile_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13topk_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_13topk_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14alloc_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14alloc_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14cf_if_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14cf_if_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14dropout_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14dropout_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14flatten_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14flatten_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14l2_norm_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14l2_norm_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14range_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14range_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14reshape_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14reshape_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14slice_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14slice_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14softmax_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_14softmax_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15cf_end_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15cf_end_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15cf_for_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15cf_for_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15cumsum_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15cumsum_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15dev_op_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15dev_op_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15gather_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15gather_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15lambda_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15lambda_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15reduce_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15reduce_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15rnn_arch_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_15rnn_arch_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16batchnorm_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16batchnorm_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16builtin_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16builtin_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_call_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_call_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_else_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_else_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_jump_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_jump_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_loop_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16cf_loop_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16nonzero_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16nonzero_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16one_hot_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16one_hot_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16scatter_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16scatter_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16squeeze_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_16squeeze_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_17activation_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_17activation_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_17split_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_17split_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_17stack_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_17stack_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18broadcast_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18broadcast_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18cf_branch_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18cf_branch_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18cf_return_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18cf_return_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18elementwise_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18elementwise_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18gather_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18gather_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18get_shape_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18get_shape_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18transpose_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_18transpose_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19gumbel_max_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19gumbel_max_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19scatter_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19scatter_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19softmax_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19softmax_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19transplant_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_19transplant_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20convolution_uniformsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20convolution_uniformsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20expand_dims_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20expand_dims_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20load_constant_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20load_constant_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20load_global_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20load_global_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20load_random_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20load_random_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20multinomial_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20multinomial_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20phi_virtual_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20phi_virtual_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20reverse_seq_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_20reverse_seq_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21array_gather_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21array_gather_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21batch_matmul_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21batch_matmul_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21store_global_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21store_global_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21vimage2espresso_paramE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_21vimage2espresso_paramE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22compute_stats_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22compute_stats_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22cross_entropy_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22cross_entropy_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22general_padding_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22general_padding_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22general_slice_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22general_slice_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22inner_product_uniformsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22inner_product_uniformsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22sequence_concat_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22sequence_concat_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22sequence_repeat_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22sequence_repeat_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22upsample_kernel_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_22upsample_kernel_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23context_transfer_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23context_transfer_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23fused_kernel_parametersE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23fused_kernel_parametersE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23general_concat_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23general_concat_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23no_op_kernel_parametersE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23no_op_kernel_parametersE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23sequence_reverse_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23sequence_reverse_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23space_to_depth_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_23space_to_depth_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24crop_and_resize_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24crop_and_resize_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24instancenorm_1d_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24instancenorm_1d_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24patch_extractor_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24patch_extractor_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24sliding_windows_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24sliding_windows_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24tensor_zero_pad_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_24tensor_zero_pad_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_25blob_numerical_propertiesE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_25blob_numerical_propertiesE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_25dynamic_quantize_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_25dynamic_quantize_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_25matrix_band_part_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_25matrix_band_part_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_27dynamic_dequantize_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_27dynamic_dequantize_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_27prediction_smoothing_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_27prediction_smoothing_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_28precompiled_segment_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_28precompiled_segment_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_28spatial_transformer_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_28spatial_transformer_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_32non_maximum_suppression_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDeslsERNS0_21generic_serdes_objectERNS_32non_maximum_suppression_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_10lrn_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_10lrn_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_10rnn_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_10rnn_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11bias_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11bias_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11crop_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11crop_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11layer_shapeE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11layer_shapeE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11pool_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_11pool_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12axpby_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12axpby_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12brick_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12brick_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12sgd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12sgd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12split_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_12split_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13adam_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13adam_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13cast_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13cast_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13concat_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13concat_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13copy_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13copy_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13fill_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13fill_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13tile_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13tile_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13topk_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_13topk_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14alloc_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14alloc_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14cf_if_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14cf_if_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14dropout_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14dropout_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14flatten_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14flatten_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14l2_norm_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14l2_norm_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14range_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14range_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14reshape_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14reshape_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14slice_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14slice_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14softmax_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_14softmax_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15cf_end_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15cf_end_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15cf_for_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15cf_for_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15cumsum_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15cumsum_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15dev_op_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15dev_op_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15gather_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15gather_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15lambda_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15lambda_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15reduce_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15reduce_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15rnn_arch_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_15rnn_arch_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16batchnorm_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16batchnorm_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16builtin_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16builtin_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_call_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_call_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_else_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_else_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_jump_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_jump_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_loop_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16cf_loop_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16nonzero_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16nonzero_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16one_hot_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16one_hot_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16scatter_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16scatter_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16squeeze_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_16squeeze_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_17activation_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_17activation_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_17split_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_17split_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_17stack_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_17stack_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18broadcast_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18broadcast_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18cf_branch_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18cf_branch_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18cf_return_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18cf_return_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18elementwise_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18elementwise_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18gather_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18gather_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18get_shape_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18get_shape_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18transpose_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_18transpose_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19gumbel_max_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19gumbel_max_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19scatter_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19scatter_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19softmax_nd_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19softmax_nd_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19transplant_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_19transplant_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20convolution_uniformsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20convolution_uniformsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20expand_dims_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20expand_dims_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20load_constant_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20load_constant_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20load_global_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20load_global_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20load_random_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20load_random_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20multinomial_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20multinomial_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20phi_virtual_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20phi_virtual_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20reverse_seq_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_20reverse_seq_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21array_gather_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21array_gather_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21batch_matmul_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21batch_matmul_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21store_global_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21store_global_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21vimage2espresso_paramE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_21vimage2espresso_paramE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22compute_stats_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22compute_stats_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22cross_entropy_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22cross_entropy_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22general_padding_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22general_padding_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22general_slice_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22general_slice_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22inner_product_uniformsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22inner_product_uniformsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22sequence_concat_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22sequence_concat_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22sequence_repeat_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22sequence_repeat_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22upsample_kernel_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_22upsample_kernel_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23context_transfer_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23context_transfer_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23fused_kernel_parametersE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23fused_kernel_parametersE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23general_concat_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23general_concat_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23no_op_kernel_parametersE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23no_op_kernel_parametersE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23sequence_reverse_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23sequence_reverse_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23space_to_depth_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_23space_to_depth_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24crop_and_resize_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24crop_and_resize_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24instancenorm_1d_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24instancenorm_1d_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24patch_extractor_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24patch_extractor_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24sliding_windows_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24sliding_windows_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24tensor_zero_pad_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_24tensor_zero_pad_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_25blob_numerical_propertiesE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_25blob_numerical_propertiesE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_25dynamic_quantize_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_25dynamic_quantize_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_25matrix_band_part_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_25matrix_band_part_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_27dynamic_dequantize_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_27dynamic_dequantize_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_27prediction_smoothing_paramsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_27prediction_smoothing_paramsE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_28precompiled_segment_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_28precompiled_segment_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_28spatial_transformer_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_28spatial_transformer_params_tE called");
    return NULL;
}

void* _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_32non_maximum_suppression_params_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6SerDesrsERNS0_21generic_serdes_objectERNS_32non_maximum_suppression_params_tE called");
    return NULL;
}

void* _ZN8Espresso6wisdom33get_optimal_parameters_for_kernelERKNS_17wisdom_kernel_keyE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6wisdom33get_optimal_parameters_for_kernelERKNS_17wisdom_kernel_keyE called");
    return NULL;
}

void* _ZN8Espresso6wisdom39get_optimal_parameters_for_kernel_exactERKNS_17wisdom_kernel_keyE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6wisdom39get_optimal_parameters_for_kernel_exactERKNS_17wisdom_kernel_keyE called");
    return NULL;
}

void* _ZN8Espresso6wisdom4loadEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6wisdom4loadEPKc called");
    return NULL;
}

void* _ZN8Espresso6wisdom4saveEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6wisdom4saveEPKc called");
    return NULL;
}

void* _ZN8Espresso6wisdom6uploadEPKc(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6wisdom6uploadEPKc called");
    return NULL;
}

void* _ZN8Espresso6wisdom9load_dictEP12NSDictionary(void)
{
    if (verbose) puts("STUB: _ZN8Espresso6wisdom9load_dictEP12NSDictionary called");
    return NULL;
}

void* _ZN8Espresso7trainer10validationEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso7trainer10validationEv called");
    return NULL;
}

void* _ZN8Espresso7trainer18initialize_weightsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso7trainer18initialize_weightsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso7trainer4stepEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso7trainer4stepEv called");
    return NULL;
}

void* _ZN8Espresso7trainer5trainEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso7trainer5trainEv called");
    return NULL;
}

void* _ZN8Espresso7trainerC1ERKNS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso7trainerC1ERKNS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso7trainerC2ERKNS0_7optionsE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso7trainerC2ERKNS0_7optionsE called");
    return NULL;
}

void* _ZN8Espresso8hardware17supports_platformENS_8platformE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso8hardware17supports_platformENS_8platformE called");
    return NULL;
}

void* _ZN8Espresso8net_passD0Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso8net_passD0Ev called");
    return NULL;
}

void* _ZN8Espresso8net_passD1Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso8net_passD1Ev called");
    return NULL;
}

void* _ZN8Espresso8net_passD2Ev(void)
{
    if (verbose) puts("STUB: _ZN8Espresso8net_passD2Ev called");
    return NULL;
}

void* _ZN8Espresso8on_allocEPvm(void)
{
    if (verbose) puts("STUB: _ZN8Espresso8on_allocEPvm called");
    return NULL;
}

void* _ZN8Espresso9quantizer10load_statsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer10load_statsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer10save_statsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer10save_statsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer14estimate_statsENSt3__110shared_ptrINS_20abstract_data_sourceEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer14estimate_statsENSt3__110shared_ptrINS_20abstract_data_sourceEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer15traits_for_blobERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer15traits_for_blobERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer18traits_for_weightsEi(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer18traits_for_weightsEi called");
    return NULL;
}

void* _ZN8Espresso9quantizer20evaluate_discrepancyENSt3__110shared_ptrINS_3netEEES4_NS2_INS_20abstract_data_sourceEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer20evaluate_discrepancyENSt3__110shared_ptrINS_3netEEES4_NS2_INS_20abstract_data_sourceEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer21quantize_conv_weightsEiNSt3__110shared_ptrINS_26generic_convolution_kernelEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer21quantize_conv_weightsEiNSt3__110shared_ptrINS_26generic_convolution_kernelEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer22_fold_transform_paramsENSt3__110shared_ptrINS_3netEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer22_fold_transform_paramsENSt3__110shared_ptrINS_3netEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer22get_scale_from_min_maxENS_15q_target_traitsEff(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer22get_scale_from_min_maxENS_15q_target_traitsEff called");
    return NULL;
}

void* _ZN8Espresso9quantizer22merge_stats_for_concatEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer22merge_stats_for_concatEv called");
    return NULL;
}

void* _ZN8Espresso9quantizer23quantize_deconv_weightsEiNSt3__110shared_ptrINS_28generic_deconvolution_kernelEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer23quantize_deconv_weightsEiNSt3__110shared_ptrINS_28generic_deconvolution_kernelEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer30quantize_inner_product_weightsEiNSt3__110shared_ptrINS_28generic_inner_product_kernelEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer30quantize_inner_product_weightsEiNSt3__110shared_ptrINS_28generic_inner_product_kernelEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer34get_per_channel_scale_from_min_maxENS_15q_target_traitsENSt3__110shared_ptrINS_4blobIfLi2EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer34get_per_channel_scale_from_min_maxENS_15q_target_traitsENSt3__110shared_ptrINS_4blobIfLi2EEEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer34get_per_channel_scale_from_min_maxENS_15q_target_traitsENSt3__110shared_ptrINS_4blobIfLi4EEEEE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer34get_per_channel_scale_from_min_maxENS_15q_target_traitsENSt3__110shared_ptrINS_4blobIfLi4EEEEE called");
    return NULL;
}

void* _ZN8Espresso9quantizer34get_per_channel_scale_from_min_maxENS_15q_target_traitsEPfii(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer34get_per_channel_scale_from_min_maxENS_15q_target_traitsEPfii called");
    return NULL;
}

void* _ZN8Espresso9quantizer8quantizeEv(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer8quantizeEv called");
    return NULL;
}

void* _ZN8Espresso9quantizer9set_statsERKNS_17dataset_estimator8result_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizer9set_statsERKNS_17dataset_estimator8result_tE called");
    return NULL;
}

void* _ZN8Espresso9quantizerC1ENSt3__110shared_ptrINS_3netEEENS0_9options_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizerC1ENSt3__110shared_ptrINS_3netEEENS0_9options_tE called");
    return NULL;
}

void* _ZN8Espresso9quantizerC2ENSt3__110shared_ptrINS_3netEEENS0_9options_tE(void)
{
    if (verbose) puts("STUB: _ZN8Espresso9quantizerC2ENSt3__110shared_ptrINS_3netEEENS0_9options_tE called");
    return NULL;
}

void* _ZNK8Espresso16abstract_context15__launch_kernelERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS2_INS_11base_kernelEEERNS_10layer_dataESC_(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso16abstract_context15__launch_kernelERKNSt3__110shared_ptrINS_14abstract_batchEEERKNS2_INS_11base_kernelEEERNS_10layer_dataESC_ called");
    return NULL;
}

void* _ZNK8Espresso16confusion_matrix16generate_summaryEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso16confusion_matrix16generate_summaryEv called");
    return NULL;
}

void* _ZNK8Espresso17ANECompilerEngine19zin_compiler_helper10precompileERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS1_6opts_tE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso17ANECompilerEngine19zin_compiler_helper10precompileERKNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEERKNS1_6opts_tE called");
    return NULL;
}

void* _ZNK8Espresso17dataset_estimator18statistics_entry_t18compute_percentileEf(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso17dataset_estimator18statistics_entry_t18compute_percentileEf called");
    return NULL;
}

void* _ZNK8Espresso17generic_factory_t4get_ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso17generic_factory_t4get_ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK8Espresso17generic_factory_t5make_ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso17generic_factory_t5make_ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK8Espresso22abstract_interpreter_t14get_subnetworkERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso22abstract_interpreter_t14get_subnetworkERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK8Espresso23abstract_engine_factory16engine_with_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso23abstract_engine_factory16engine_with_nameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK8Espresso23abstract_engine_factory16make_kernel_privERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvRKNS_8platformERKNS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso23abstract_engine_factory16make_kernel_privERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPvRKNS_8platformERKNS_12compute_pathE called");
    return NULL;
}

void* _ZNK8Espresso23abstract_engine_factory19engine_for_platformERKNS_8platformE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso23abstract_engine_factory19engine_for_platformERKNS_8platformE called");
    return NULL;
}

void* _ZNK8Espresso23abstract_engine_factory19is_kernel_supportedERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso23abstract_engine_factory19is_kernel_supportedERKNSt3__110shared_ptrINS_16abstract_contextEEERKNS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS_12compute_pathE called");
    return NULL;
}

void* _ZNK8Espresso27kernels_validation_status_t4saveEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso27kernels_validation_status_t4saveEv called");
    return NULL;
}

void* _ZNK8Espresso30generic_patch_extractor_kernel30get_coordinates_for_dense_modeERKNS_11layer_shapeE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso30generic_patch_extractor_kernel30get_coordinates_for_dense_modeERKNS_11layer_shapeE called");
    return NULL;
}

void* _ZNK8Espresso34pass_constant_if_conditions_result9serializeEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso34pass_constant_if_conditions_result9serializeEv called");
    return NULL;
}

void* _ZNK8Espresso3net15is_fallback_gpuEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net15is_fallback_gpuEv called");
    return NULL;
}

void* _ZNK8Espresso3net16need_interpreterEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net16need_interpreterEv called");
    return NULL;
}

void* _ZNK8Espresso3net16serialize_scriptEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net16serialize_scriptEv called");
    return NULL;
}

void* _ZNK8Espresso3net16sync_copy_globalERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net16sync_copy_globalERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK8Espresso3net18position_of_kernelERKNSt3__110shared_ptrINS_11base_kernelEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net18position_of_kernelERKNSt3__110shared_ptrINS_11base_kernelEEE called");
    return NULL;
}

void* _ZNK8Espresso3net20fast_reshape_enabledEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net20fast_reshape_enabledEv called");
    return NULL;
}

void* _ZNK8Espresso3net22get_consumers_of_blobsERKNSt3__113unordered_setINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_4hashIS8_EENS1_8equal_toIS8_EENS6_IS8_EEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net22get_consumers_of_blobsERKNSt3__113unordered_setINS1_12basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_4hashIS8_EENS1_8equal_toIS8_EENS6_IS8_EEEE called");
    return NULL;
}

void* _ZNK8Espresso3net22get_producers_of_blobsEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net22get_producers_of_blobsEv called");
    return NULL;
}

void* _ZNK8Espresso3net24save_precompilation_infoEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net24save_precompilation_infoEv called");
    return NULL;
}

void* _ZNK8Espresso3net30get_blob_or_global_with_prefixERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net30get_blob_or_global_with_prefixERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK8Espresso3net30save_additional_configurationsEv(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net30save_additional_configurationsEv called");
    return NULL;
}

void* _ZNK8Espresso3net33resolve_blob_numerical_propertiesERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS_8platformERKNS_12compute_pathE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net33resolve_blob_numerical_propertiesERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEERKNS_8platformERKNS_12compute_pathE called");
    return NULL;
}

void* _ZNK8Espresso3net6pprintERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_8functionIFS7_PKmPS8_EEE(void)
{
    if (verbose) puts("STUB: _ZNK8Espresso3net6pprintERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_8functionIFS7_PKmPS8_EEE called");
    return NULL;
}

void* _ZThn208_N8Espresso20generic_cf_if_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso20generic_cf_if_kernelD0Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso20generic_cf_if_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso20generic_cf_if_kernelD1Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso21generic_cf_for_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso21generic_cf_for_kernelD0Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso21generic_cf_for_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso21generic_cf_for_kernelD1Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso22generic_cf_else_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso22generic_cf_else_kernelD0Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso22generic_cf_else_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso22generic_cf_else_kernelD1Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso22generic_cf_jump_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso22generic_cf_jump_kernelD0Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso22generic_cf_jump_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso22generic_cf_jump_kernelD1Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso22generic_cf_loop_kernelD0Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso22generic_cf_loop_kernelD0Ev called");
    return NULL;
}

void* _ZThn208_N8Espresso22generic_cf_loop_kernelD1Ev(void)
{
    if (verbose) puts("STUB: _ZThn208_N8Espresso22generic_cf_loop_kernelD1Ev called");
    return NULL;
}

void* c_network_get_input_names(void)
{
    if (verbose) puts("STUB: c_network_get_input_names called");
    return NULL;
}

void* c_network_get_output_names(void)
{
    if (verbose) puts("STUB: c_network_get_output_names called");
    return NULL;
}

void* dump_test_vector(void)
{
    if (verbose) puts("STUB: dump_test_vector called");
    return NULL;
}

void* espresso_ane_cache_has_network(void)
{
    if (verbose) puts("STUB: espresso_ane_cache_has_network called");
    return NULL;
}

void* espresso_ane_cache_purge_network(void)
{
    if (verbose) puts("STUB: espresso_ane_cache_purge_network called");
    return NULL;
}

void* espresso_blob_is_dynamic(void)
{
    if (verbose) puts("STUB: espresso_blob_is_dynamic called");
    return NULL;
}

void* espresso_blob_set_int_option(void)
{
    if (verbose) puts("STUB: espresso_blob_set_int_option called");
    return NULL;
}

void* espresso_buffer_get_count(void)
{
    if (verbose) puts("STUB: espresso_buffer_get_count called");
    return NULL;
}

void* espresso_buffer_get_rank(void)
{
    if (verbose) puts("STUB: espresso_buffer_get_rank called");
    return NULL;
}

void* espresso_buffer_get_size(void)
{
    if (verbose) puts("STUB: espresso_buffer_get_size called");
    return NULL;
}

void* espresso_buffer_pack_tensor_shape(void)
{
    if (verbose) puts("STUB: espresso_buffer_pack_tensor_shape called");
    return NULL;
}

void* espresso_buffer_set_rank(void)
{
    if (verbose) puts("STUB: espresso_buffer_set_rank called");
    return NULL;
}

void* espresso_buffer_unpack_tensor_shape(void)
{
    if (verbose) puts("STUB: espresso_buffer_unpack_tensor_shape called");
    return NULL;
}

void* espresso_context_create_for_cpu_test_vectors(void)
{
    if (verbose) puts("STUB: espresso_context_create_for_cpu_test_vectors called");
    return NULL;
}

void* espresso_context_destroy(void)
{
    if (verbose) puts("STUB: espresso_context_destroy called");
    return NULL;
}

void* espresso_context_report_bench(void)
{
    if (verbose) puts("STUB: espresso_context_report_bench called");
    return NULL;
}

void* espresso_context_set_int_option(void)
{
    if (verbose) puts("STUB: espresso_context_set_int_option called");
    return NULL;
}

void* espresso_context_set_low_precision_accumulation(void)
{
    if (verbose) puts("STUB: espresso_context_set_low_precision_accumulation called");
    return NULL;
}

void* espresso_create_context(void)
{
    if (verbose) puts("STUB: espresso_create_context called");
    return NULL;
}

void* espresso_create_context_with_args(void)
{
    if (verbose) puts("STUB: espresso_create_context_with_args called");
    return NULL;
}

void* espresso_create_hx_context(void)
{
    if (verbose) puts("STUB: espresso_create_hx_context called");
    return NULL;
}

void* espresso_create_hx_cva_context(void)
{
    if (verbose) puts("STUB: espresso_create_hx_cva_context called");
    return NULL;
}

void* espresso_create_plan(void)
{
    if (verbose) puts("STUB: espresso_create_plan called");
    return NULL;
}

void* espresso_create_plan_and_load_network(void)
{
    if (verbose) puts("STUB: espresso_create_plan_and_load_network called");
    return NULL;
}

void* espresso_destroy_global_ref(void)
{
    if (verbose) puts("STUB: espresso_destroy_global_ref called");
    return NULL;
}

void* espresso_device_id_for_metal_device(void)
{
    if (verbose) puts("STUB: espresso_device_id_for_metal_device called");
    return NULL;
}

void* espresso_dump_ir(void)
{
    if (verbose) puts("STUB: espresso_dump_ir called");
    return NULL;
}

void* espresso_enable_test_vector_mode(void)
{
    if (verbose) puts("STUB: espresso_enable_test_vector_mode called");
    return NULL;
}

void* espresso_get_additional_configuration_names(void)
{
    if (verbose) puts("STUB: espresso_get_additional_configuration_names called");
    return NULL;
}

void* espresso_get_branch(void)
{
    if (verbose) puts("STUB: espresso_get_branch called");
    return NULL;
}

void* espresso_get_default_storage_type(void)
{
    if (verbose) puts("STUB: espresso_get_default_storage_type called");
    return NULL;
}

void* espresso_get_internal_queue(void)
{
    if (verbose) puts("STUB: espresso_get_internal_queue called");
    return NULL;
}

void* espresso_get_last_error(void)
{
    if (verbose) puts("STUB: espresso_get_last_error called");
    return NULL;
}

void* espresso_get_status_string(void)
{
    if (verbose) puts("STUB: espresso_get_status_string called");
    return NULL;
}

void* espresso_get_version_string(void)
{
    if (verbose) puts("STUB: espresso_get_version_string called");
    return NULL;
}

void* espresso_hx_get_processing_session(void)
{
    if (verbose) puts("STUB: espresso_hx_get_processing_session called");
    return NULL;
}

void* espresso_network_bind_buffer(void)
{
    if (verbose) puts("STUB: espresso_network_bind_buffer called");
    return NULL;
}

void* espresso_network_bind_buffer_to_global(void)
{
    if (verbose) puts("STUB: espresso_network_bind_buffer_to_global called");
    return NULL;
}

void* espresso_network_bind_cvpixelbuffer(void)
{
    if (verbose) puts("STUB: espresso_network_bind_cvpixelbuffer called");
    return NULL;
}

void* espresso_network_bind_direct_cvpixelbuffer(void)
{
    if (verbose) puts("STUB: espresso_network_bind_direct_cvpixelbuffer called");
    return NULL;
}

void* espresso_network_bind_input_cvpixelbuffer(void)
{
    if (verbose) puts("STUB: espresso_network_bind_input_cvpixelbuffer called");
    return NULL;
}

void* espresso_network_bind_input_metaltexture(void)
{
    if (verbose) puts("STUB: espresso_network_bind_input_metaltexture called");
    return NULL;
}

void* espresso_network_bind_input_vimagebuffer_argb8(void)
{
    if (verbose) puts("STUB: espresso_network_bind_input_vimagebuffer_argb8 called");
    return NULL;
}

void* espresso_network_bind_input_vimagebuffer_bgra8(void)
{
    if (verbose) puts("STUB: espresso_network_bind_input_vimagebuffer_bgra8 called");
    return NULL;
}

void* espresso_network_bind_input_vimagebuffer_planar8(void)
{
    if (verbose) puts("STUB: espresso_network_bind_input_vimagebuffer_planar8 called");
    return NULL;
}

void* espresso_network_bind_input_vimagebuffer_rgba8(void)
{
    if (verbose) puts("STUB: espresso_network_bind_input_vimagebuffer_rgba8 called");
    return NULL;
}

void* espresso_network_change_blob_shape(void)
{
    if (verbose) puts("STUB: espresso_network_change_blob_shape called");
    return NULL;
}

void* espresso_network_change_input_blob_shapes(void)
{
    if (verbose) puts("STUB: espresso_network_change_input_blob_shapes called");
    return NULL;
}

void* espresso_network_change_input_blob_shapes_seq(void)
{
    if (verbose) puts("STUB: espresso_network_change_input_blob_shapes_seq called");
    return NULL;
}

void* espresso_network_change_input_blob_shapes_seq_rank(void)
{
    if (verbose) puts("STUB: espresso_network_change_input_blob_shapes_seq_rank called");
    return NULL;
}

void* espresso_network_compiler_set_metadata_key(void)
{
    if (verbose) puts("STUB: espresso_network_compiler_set_metadata_key called");
    return NULL;
}

void* espresso_network_declare_input(void)
{
    if (verbose) puts("STUB: espresso_network_declare_input called");
    return NULL;
}

void* espresso_network_declare_output(void)
{
    if (verbose) puts("STUB: espresso_network_declare_output called");
    return NULL;
}

void* espresso_network_dump_test_vector(void)
{
    if (verbose) puts("STUB: espresso_network_dump_test_vector called");
    return NULL;
}

void* espresso_network_get_version(void)
{
    if (verbose) puts("STUB: espresso_network_get_version called");
    return NULL;
}

void* espresso_network_query_blob_dimensions(void)
{
    if (verbose) puts("STUB: espresso_network_query_blob_dimensions called");
    return NULL;
}

void* espresso_network_select_configuration(void)
{
    if (verbose) puts("STUB: espresso_network_select_configuration called");
    return NULL;
}

void* espresso_network_set_function_name(void)
{
    if (verbose) puts("STUB: espresso_network_set_function_name called");
    return NULL;
}

void* espresso_network_swap_global(void)
{
    if (verbose) puts("STUB: espresso_network_swap_global called");
    return NULL;
}

void* espresso_network_sync_copy_global(void)
{
    if (verbose) puts("STUB: espresso_network_sync_copy_global called");
    return NULL;
}

void* espresso_network_temporal_state_reset(void)
{
    if (verbose) puts("STUB: espresso_network_temporal_state_reset called");
    return NULL;
}

void* espresso_network_unbind_buffer(void)
{
    if (verbose) puts("STUB: espresso_network_unbind_buffer called");
    return NULL;
}

void* espresso_network_unbind_buffer_to_global(void)
{
    if (verbose) puts("STUB: espresso_network_unbind_buffer_to_global called");
    return NULL;
}

void* espresso_plan_activate_debug_firehose(void)
{
    if (verbose) puts("STUB: espresso_plan_activate_debug_firehose called");
    return NULL;
}

void* espresso_plan_add_network(void)
{
    if (verbose) puts("STUB: espresso_plan_add_network called");
    return NULL;
}

void* espresso_plan_add_network_from_memory(void)
{
    if (verbose) puts("STUB: espresso_plan_add_network_from_memory called");
    return NULL;
}

void* espresso_plan_build(void)
{
    if (verbose) puts("STUB: espresso_plan_build called");
    return NULL;
}

void* espresso_plan_build_clean(void)
{
    if (verbose) puts("STUB: espresso_plan_build_clean called");
    return NULL;
}

void* espresso_plan_can_use_submit(void)
{
    if (verbose) puts("STUB: espresso_plan_can_use_submit called");
    return NULL;
}

void* espresso_plan_destroy(void)
{
    if (verbose) puts("STUB: espresso_plan_destroy called");
    return NULL;
}

void* espresso_plan_execute_async(void)
{
    if (verbose) puts("STUB: espresso_plan_execute_async called");
    return NULL;
}

void* espresso_plan_execute_sync(void)
{
    if (verbose) puts("STUB: espresso_plan_execute_sync called");
    return NULL;
}

void* espresso_plan_get_error_info(void)
{
    if (verbose) puts("STUB: espresso_plan_get_error_info called");
    return NULL;
}

void* espresso_plan_get_phase(void)
{
    if (verbose) puts("STUB: espresso_plan_get_phase called");
    return NULL;
}

void* espresso_plan_perfbench(void)
{
    if (verbose) puts("STUB: espresso_plan_perfbench called");
    return NULL;
}

void* espresso_plan_set_execution_queue(void)
{
    if (verbose) puts("STUB: espresso_plan_set_execution_queue called");
    return NULL;
}

void* espresso_plan_set_priority(void)
{
    if (verbose) puts("STUB: espresso_plan_set_priority called");
    return NULL;
}

void* espresso_plan_share_intermediate_buffer(void)
{
    if (verbose) puts("STUB: espresso_plan_share_intermediate_buffer called");
    return NULL;
}

void* espresso_plan_submit(void)
{
    if (verbose) puts("STUB: espresso_plan_submit called");
    return NULL;
}

void* espresso_plan_submit_camera(void)
{
    if (verbose) puts("STUB: espresso_plan_submit_camera called");
    return NULL;
}

void* espresso_plan_submit_set_multiple_buffering(void)
{
    if (verbose) puts("STUB: espresso_plan_submit_set_multiple_buffering called");
    return NULL;
}

void* espresso_plan_submit_with_args(void)
{
    if (verbose) puts("STUB: espresso_plan_submit_with_args called");
    return NULL;
}

void* espresso_plan_wipe_tempoary(void)
{
    if (verbose) puts("STUB: espresso_plan_wipe_tempoary called");
    return NULL;
}

void* espresso_recommended_device_id(void)
{
    if (verbose) puts("STUB: espresso_recommended_device_id called");
    return NULL;
}

void* espresso_set_breakpoint(void)
{
    if (verbose) puts("STUB: espresso_set_breakpoint called");
    return NULL;
}

void* espresso_set_image_preprocessing_params(void)
{
    if (verbose) puts("STUB: espresso_set_image_preprocessing_params called");
    return NULL;
}

void* espresso_set_quantization_flags(void)
{
    if (verbose) puts("STUB: espresso_set_quantization_flags called");
    return NULL;
}

void* espresso_tmp_enable_montreal(void)
{
    if (verbose) puts("STUB: espresso_tmp_enable_montreal called");
    return NULL;
}

void* espresso_will_share_intermediate_buffer_with_existing_plan(void)
{
    if (verbose) puts("STUB: espresso_will_share_intermediate_buffer_with_existing_plan called");
    return NULL;
}

void* tmp_espresso_load_image_bgra(void)
{
    if (verbose) puts("STUB: tmp_espresso_load_image_bgra called");
    return NULL;
}

void* tmp_espresso_load_image_grayscale(void)
{
    if (verbose) puts("STUB: tmp_espresso_load_image_grayscale called");
    return NULL;
}
