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


#include <QuartzComposer/QuartzComposer.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


void* const QCCompositionAttributeApplePrivateKey = (void*)0;
void* const QCCompositionAttributeBuiltInKey = (void*)0;
void* const QCCompositionAttributeCategoryKey = (void*)0;
void* const QCCompositionAttributeCopyrightKey = (void*)0;
void* const QCCompositionAttributeDescriptionKey = (void*)0;
void* const QCCompositionAttributeExcludedHostsKey = (void*)0;
void* const QCCompositionAttributeExclusiveHostsKey = (void*)0;
void* const QCCompositionAttributeHasConsumersKey = (void*)0;
void* const QCCompositionAttributeIsTimeDependentKey = (void*)0;
void* const QCCompositionAttributeNameKey = (void*)0;
void* const QCCompositionAttributeProtocolsKey = (void*)0;
void* const QCCompositionAttributeSourceKey = (void*)0;
void* const QCCompositionAttributeThumbnailDataKey = (void*)0;
void* const QCCompositionCategoryDistortion = (void*)0;
void* const QCCompositionCategoryStylize = (void*)0;
void* const QCCompositionCategoryUtility = (void*)0;
void* const QCCompositionInputAudioPeakKey = (void*)0;
void* const QCCompositionInputAudioSpectrumKey = (void*)0;
void* const QCCompositionInputDestinationImageKey = (void*)0;
void* const QCCompositionInputImageKey = (void*)0;
void* const QCCompositionInputImageListKey = (void*)0;
void* const QCCompositionInputImageSignalKey = (void*)0;
void* const QCCompositionInputPaceKey = (void*)0;
void* const QCCompositionInputPreviewModeKey = (void*)0;
void* const QCCompositionInputPrimaryColorKey = (void*)0;
void* const QCCompositionInputScreenImageKey = (void*)0;
void* const QCCompositionInputSecondaryColorKey = (void*)0;
void* const QCCompositionInputSourceImageKey = (void*)0;
void* const QCCompositionInputTrackInfoKey = (void*)0;
void* const QCCompositionInputTrackPositionKey = (void*)0;
void* const QCCompositionInputTrackSignalKey = (void*)0;
void* const QCCompositionInputWindowImagesKey = (void*)0;
void* const QCCompositionInputXKey = (void*)0;
void* const QCCompositionInputYKey = (void*)0;
void* const QCCompositionOutputImageKey = (void*)0;
void* const QCCompositionOutputRefreshImageCountKey = (void*)0;
void* const QCCompositionOutputTotalImageCountKey = (void*)0;
void* const QCCompositionOutputWebPageURLKey = (void*)0;
void* const QCCompositionPickerPanelDidSelectCompositionNotification = (void*)0;
void* const QCCompositionPickerPanelWillSelectCompositionNotification = (void*)0;
void* const QCCompositionPickerViewDidSelectCompositionNotification = (void*)0;
void* const QCCompositionPickerViewWillSelectCompositionNotification = (void*)0;
void* const QCCompositionProtocolGraphicAnimation = (void*)0;
void* const QCCompositionProtocolGraphicTransition = (void*)0;
void* const QCCompositionProtocolImageFilter = (void*)0;
void* const QCCompositionProtocolMusicVisualizer = (void*)0;
void* const QCCompositionProtocolPhotoVisualizer = (void*)0;
void* const QCCompositionProtocolScreenSaver = (void*)0;
void* const QCCompositionRepositoryDidUpdateNotification = (void*)0;
void* const QCPixelFormatCompatibility_CoreVideo = (void*)0;
void* const QCPlugInAttributeApplePrivateKey = (void*)0;
void* const QCPlugInAttributeCategoriesKey = (void*)0;
void* const QCPlugInAttributeCopyrightKey = (void*)0;
void* const QCPlugInAttributeDescriptionKey = (void*)0;
void* const QCPlugInAttributeExamplesKey = (void*)0;
void* const QCPlugInAttributeNameKey = (void*)0;
void* const QCPlugInExecutionArgumentEventKey = (void*)0;
void* const QCPlugInExecutionArgumentMouseLocationKey = (void*)0;
void* const QCPlugInPixelFormatARGB8 = (void*)0;
void* const QCPlugInPixelFormatBGRA8 = (void*)0;
void* const QCPlugInPixelFormatI8 = (void*)0;
void* const QCPlugInPixelFormatIf = (void*)0;
void* const QCPlugInPixelFormatRGBAf = (void*)0;
void* const QCPortAttributeDefaultValueKey = (void*)0;
void* const QCPortAttributeMaximumValueKey = (void*)0;
void* const QCPortAttributeMenuItemsKey = (void*)0;
void* const QCPortAttributeMinimumValueKey = (void*)0;
void* const QCPortAttributeNameKey = (void*)0;
void* const QCPortAttributeTypeKey = (void*)0;
void* const QCPortTypeBoolean = (void*)0;
void* const QCPortTypeColor = (void*)0;
void* const QCPortTypeImage = (void*)0;
void* const QCPortTypeIndex = (void*)0;
void* const QCPortTypeNumber = (void*)0;
void* const QCPortTypeString = (void*)0;
void* const QCPortTypeStructure = (void*)0;
void* const QCRendererEventKey = (void*)0;
void* const QCRendererMouseLocationKey = (void*)0;
void* const QCRuntimeEventKey = (void*)0;
void* const QCRuntimeMouseLocationKey = (void*)0;
void* const QCViewDidEnterFullScreenNotification = (void*)0;
void* const QCViewDidExitFullScreenNotification = (void*)0;
void* const QCViewDidSetPatchNotification = (void*)0;
void* const QCViewDidStartRenderingNotification = (void*)0;
void* const QCViewDidStopRenderingNotification = (void*)0;
void* const QuartzComposerVersionNumber = (void*)0;
void* const QuartzComposerVersionString = (void*)0;
void* const kQCNullMD5 = (void*)0;
void* const kQCPixelFormatCompatibility_CGBitmapContext = (void*)0;
void* const kQCPixelFormatCompatibility_CGImage = (void*)0;
void* const kQCPixelFormatCompatibility_CIBitmapContext = (void*)0;
void* const kQCPixelFormatCompatibility_CIImage = (void*)0;
void* const kQCPixelFormatCompatibility_CoreVideo = (void*)0;
void* const kQCPixelFormatCompatibility_GLContext = (void*)0;
void* const kQCPixelFormatCompatibility_OpenGL = (void*)0;
void* const kQCPixelFormatCompatibility_QuickTime = (void*)0;

void *GFArrayFromArgumentList(void) {
    if (verbose) puts("STUB: GFArrayFromArgumentList called");
    return NULL;
}

void *GFBacktrace(void) {
    if (verbose) puts("STUB: GFBacktrace called");
    return NULL;
}

void *GFDebug(void) {
    if (verbose) puts("STUB: GFDebug called");
    return NULL;
}

void *GFDebuggingLevel(void) {
    if (verbose) puts("STUB: GFDebuggingLevel called");
    return NULL;
}

void *GFException(void) {
    if (verbose) puts("STUB: GFException called");
    return NULL;
}

void *GFFilterStringsWithKeywords(void) {
    if (verbose) puts("STUB: GFFilterStringsWithKeywords called");
    return NULL;
}

void *GFGetDebugMessageCallback(void) {
    if (verbose) puts("STUB: GFGetDebugMessageCallback called");
    return NULL;
}

void *GFGetLogMessageCallback(void) {
    if (verbose) puts("STUB: GFGetLogMessageCallback called");
    return NULL;
}

void *GFKeywordsFromSearchString(void) {
    if (verbose) puts("STUB: GFKeywordsFromSearchString called");
    return NULL;
}

void *GFLog(void) {
    if (verbose) puts("STUB: GFLog called");
    return NULL;
}

void *GFNameForNode(void) {
    if (verbose) puts("STUB: GFNameForNode called");
    return NULL;
}

void *GFNameForPort(void) {
    if (verbose) puts("STUB: GFNameForPort called");
    return NULL;
}

void *GFSetDebugMessageCallback(void) {
    if (verbose) puts("STUB: GFSetDebugMessageCallback called");
    return NULL;
}

void *GFSetLogMessageCallback(void) {
    if (verbose) puts("STUB: GFSetLogMessageCallback called");
    return NULL;
}

void *GFThrowException(void) {
    if (verbose) puts("STUB: GFThrowException called");
    return NULL;
}

void *QCAddCompositionTrackToMovie(void) {
    if (verbose) puts("STUB: QCAddCompositionTrackToMovie called");
    return NULL;
}

void *QCAddXMLElementsToStructure(void) {
    if (verbose) puts("STUB: QCAddXMLElementsToStructure called");
    return NULL;
}

void *QCAppendCompositionSampleToTrack(void) {
    if (verbose) puts("STUB: QCAppendCompositionSampleToTrack called");
    return NULL;
}

void *QCCompositionCompliesToProtocol(void) {
    if (verbose) puts("STUB: QCCompositionCompliesToProtocol called");
    return NULL;
}

void *QCCompositionFromData(void) {
    if (verbose) puts("STUB: QCCompositionFromData called");
    return NULL;
}

void *QCCompositionFromFile(void) {
    if (verbose) puts("STUB: QCCompositionFromFile called");
    return NULL;
}

void *QCCompositionFromMovieFile(void) {
    if (verbose) puts("STUB: QCCompositionFromMovieFile called");
    return NULL;
}

void *QCCompositionFromPatch(void) {
    if (verbose) puts("STUB: QCCompositionFromPatch called");
    return NULL;
}

void *QCCompositionToData(void) {
    if (verbose) puts("STUB: QCCompositionToData called");
    return NULL;
}

void *QCCompositionToFile(void) {
    if (verbose) puts("STUB: QCCompositionToFile called");
    return NULL;
}

void *QCCompositionToMovieFile(void) {
    if (verbose) puts("STUB: QCCompositionToMovieFile called");
    return NULL;
}

void *QCCompositionsFromMovieFile(void) {
    if (verbose) puts("STUB: QCCompositionsFromMovieFile called");
    return NULL;
}

void *QCComputeAspectRatioBounds(void) {
    if (verbose) puts("STUB: QCComputeAspectRatioBounds called");
    return NULL;
}

void *QCComputeProportionalBounds(void) {
    if (verbose) puts("STUB: QCComputeProportionalBounds called");
    return NULL;
}

void *QCCountCompositionTracksInMovie(void) {
    if (verbose) puts("STUB: QCCountCompositionTracksInMovie called");
    return NULL;
}

void *QCDataFromComposition(void) {
    if (verbose) puts("STUB: QCDataFromComposition called");
    return NULL;
}

void *QCFileIsComposition(void) {
    if (verbose) puts("STUB: QCFileIsComposition called");
    return NULL;
}

void *QCGLExtensionSupported(void) {
    if (verbose) puts("STUB: QCGLExtensionSupported called");
    return NULL;
}

void *QCGLMakeTransformationMatrix(void) {
    if (verbose) puts("STUB: QCGLMakeTransformationMatrix called");
    return NULL;
}

void *QCGetCompositionTrackAtIndex(void) {
    if (verbose) puts("STUB: QCGetCompositionTrackAtIndex called");
    return NULL;
}

void *QCGetIndexedColorSpace(void) {
    if (verbose) puts("STUB: QCGetIndexedColorSpace called");
    return NULL;
}

void *QCHostTime(void) {
    if (verbose) puts("STUB: QCHostTime called");
    return NULL;
}

void *QCInfoFromComposition(void) {
    if (verbose) puts("STUB: QCInfoFromComposition called");
    return NULL;
}

void *QCMD5FromData(void) {
    if (verbose) puts("STUB: QCMD5FromData called");
    return NULL;
}

void *QCMD5FromString(void) {
    if (verbose) puts("STUB: QCMD5FromString called");
    return NULL;
}

void *QCMD5ToData(void) {
    if (verbose) puts("STUB: QCMD5ToData called");
    return NULL;
}

void *QCMD5ToString(void) {
    if (verbose) puts("STUB: QCMD5ToString called");
    return NULL;
}

void *QCMD5WithBytes(void) {
    if (verbose) puts("STUB: QCMD5WithBytes called");
    return NULL;
}

void *QCMD5WithColorSpace(void) {
    if (verbose) puts("STUB: QCMD5WithColorSpace called");
    return NULL;
}

void *QCMD5WithDoubles(void) {
    if (verbose) puts("STUB: QCMD5WithDoubles called");
    return NULL;
}

void *QCMD5WithIntegers(void) {
    if (verbose) puts("STUB: QCMD5WithIntegers called");
    return NULL;
}

void *QCMD5WithObject(void) {
    if (verbose) puts("STUB: QCMD5WithObject called");
    return NULL;
}

void *QCMD5WithPointer(void) {
    if (verbose) puts("STUB: QCMD5WithPointer called");
    return NULL;
}

void *QCMD5WithString(void) {
    if (verbose) puts("STUB: QCMD5WithString called");
    return NULL;
}

void *QCMatrix_Clear(void) {
    if (verbose) puts("STUB: QCMatrix_Clear called");
    return NULL;
}

void *QCMatrix_ConcatenateWithMatrix(void) {
    if (verbose) puts("STUB: QCMatrix_ConcatenateWithMatrix called");
    return NULL;
}

void *QCMatrix_Copy(void) {
    if (verbose) puts("STUB: QCMatrix_Copy called");
    return NULL;
}

void *QCMatrix_Determinant(void) {
    if (verbose) puts("STUB: QCMatrix_Determinant called");
    return NULL;
}

void *QCMatrix_GetRotationAngles(void) {
    if (verbose) puts("STUB: QCMatrix_GetRotationAngles called");
    return NULL;
}

void *QCMatrix_GetRotationAxisAndAngle(void) {
    if (verbose) puts("STUB: QCMatrix_GetRotationAxisAndAngle called");
    return NULL;
}

void *QCMatrix_GetVectorW(void) {
    if (verbose) puts("STUB: QCMatrix_GetVectorW called");
    return NULL;
}

void *QCMatrix_GetVectorX(void) {
    if (verbose) puts("STUB: QCMatrix_GetVectorX called");
    return NULL;
}

void *QCMatrix_GetVectorY(void) {
    if (verbose) puts("STUB: QCMatrix_GetVectorY called");
    return NULL;
}

void *QCMatrix_GetVectorZ(void) {
    if (verbose) puts("STUB: QCMatrix_GetVectorZ called");
    return NULL;
}

void *QCMatrix_IsIdentity(void) {
    if (verbose) puts("STUB: QCMatrix_IsIdentity called");
    return NULL;
}

void *QCMatrix_MultiplyByMatrix(void) {
    if (verbose) puts("STUB: QCMatrix_MultiplyByMatrix called");
    return NULL;
}

void *QCMatrix_Negate(void) {
    if (verbose) puts("STUB: QCMatrix_Negate called");
    return NULL;
}

void *QCMatrix_RotateVector(void) {
    if (verbose) puts("STUB: QCMatrix_RotateVector called");
    return NULL;
}

void *QCMatrix_SetRotationAroundAxisX(void) {
    if (verbose) puts("STUB: QCMatrix_SetRotationAroundAxisX called");
    return NULL;
}

void *QCMatrix_SetRotationAroundAxisY(void) {
    if (verbose) puts("STUB: QCMatrix_SetRotationAroundAxisY called");
    return NULL;
}

void *QCMatrix_SetRotationAroundAxisZ(void) {
    if (verbose) puts("STUB: QCMatrix_SetRotationAroundAxisZ called");
    return NULL;
}

void *QCMatrix_SetRotationFromAngles(void) {
    if (verbose) puts("STUB: QCMatrix_SetRotationFromAngles called");
    return NULL;
}

void *QCMatrix_SetRotationFromAxisAndAngle(void) {
    if (verbose) puts("STUB: QCMatrix_SetRotationFromAxisAndAngle called");
    return NULL;
}

void *QCMatrix_SetRotationFromVectors(void) {
    if (verbose) puts("STUB: QCMatrix_SetRotationFromVectors called");
    return NULL;
}

void *QCMatrix_SetScale(void) {
    if (verbose) puts("STUB: QCMatrix_SetScale called");
    return NULL;
}

void *QCMatrix_SetScaleUniform(void) {
    if (verbose) puts("STUB: QCMatrix_SetScaleUniform called");
    return NULL;
}

void *QCMatrix_SetTranslation(void) {
    if (verbose) puts("STUB: QCMatrix_SetTranslation called");
    return NULL;
}

void *QCMatrix_TransformVector(void) {
    if (verbose) puts("STUB: QCMatrix_TransformVector called");
    return NULL;
}

void *QCPatchFromComposition(void) {
    if (verbose) puts("STUB: QCPatchFromComposition called");
    return NULL;
}

void *QCPatchFromCompositionWithOptions(void) {
    if (verbose) puts("STUB: QCPatchFromCompositionWithOptions called");
    return NULL;
}

void *QCPatchToComposition(void) {
    if (verbose) puts("STUB: QCPatchToComposition called");
    return NULL;
}

void *QCPatchToFlattenedComposition(void) {
    if (verbose) puts("STUB: QCPatchToFlattenedComposition called");
    return NULL;
}

void *QCPathFromState(void) {
    if (verbose) puts("STUB: QCPathFromState called");
    return NULL;
}

void *QCProFX(void) {
    if (verbose) puts("STUB: QCProFX called");
    return NULL;
}

void *QCProFXRegisterAllocationCallbacks(void) {
    if (verbose) puts("STUB: QCProFXRegisterAllocationCallbacks called");
    return NULL;
}

void *QCQuaternion_Add(void) {
    if (verbose) puts("STUB: QCQuaternion_Add called");
    return NULL;
}

void *QCQuaternion_Clear(void) {
    if (verbose) puts("STUB: QCQuaternion_Clear called");
    return NULL;
}

void *QCQuaternion_Conjugate(void) {
    if (verbose) puts("STUB: QCQuaternion_Conjugate called");
    return NULL;
}

void *QCQuaternion_Copy(void) {
    if (verbose) puts("STUB: QCQuaternion_Copy called");
    return NULL;
}

void *QCQuaternion_Divide(void) {
    if (verbose) puts("STUB: QCQuaternion_Divide called");
    return NULL;
}

void *QCQuaternion_Dot(void) {
    if (verbose) puts("STUB: QCQuaternion_Dot called");
    return NULL;
}

void *QCQuaternion_Exp(void) {
    if (verbose) puts("STUB: QCQuaternion_Exp called");
    return NULL;
}

void *QCQuaternion_GetMatrix(void) {
    if (verbose) puts("STUB: QCQuaternion_GetMatrix called");
    return NULL;
}

void *QCQuaternion_GetRotationAngles(void) {
    if (verbose) puts("STUB: QCQuaternion_GetRotationAngles called");
    return NULL;
}

void *QCQuaternion_GetRotationAxisAndAngle(void) {
    if (verbose) puts("STUB: QCQuaternion_GetRotationAxisAndAngle called");
    return NULL;
}

void *QCQuaternion_Inverse(void) {
    if (verbose) puts("STUB: QCQuaternion_Inverse called");
    return NULL;
}

void *QCQuaternion_Length(void) {
    if (verbose) puts("STUB: QCQuaternion_Length called");
    return NULL;
}

void *QCQuaternion_LinearInterpolation(void) {
    if (verbose) puts("STUB: QCQuaternion_LinearInterpolation called");
    return NULL;
}

void *QCQuaternion_Log(void) {
    if (verbose) puts("STUB: QCQuaternion_Log called");
    return NULL;
}

void *QCQuaternion_LogDifference(void) {
    if (verbose) puts("STUB: QCQuaternion_LogDifference called");
    return NULL;
}

void *QCQuaternion_Multiply(void) {
    if (verbose) puts("STUB: QCQuaternion_Multiply called");
    return NULL;
}

void *QCQuaternion_Normalize(void) {
    if (verbose) puts("STUB: QCQuaternion_Normalize called");
    return NULL;
}

void *QCQuaternion_ScaleAngle(void) {
    if (verbose) puts("STUB: QCQuaternion_ScaleAngle called");
    return NULL;
}

void *QCQuaternion_SetFromMatrix(void) {
    if (verbose) puts("STUB: QCQuaternion_SetFromMatrix called");
    return NULL;
}

void *QCQuaternion_SetFromRotationAngles(void) {
    if (verbose) puts("STUB: QCQuaternion_SetFromRotationAngles called");
    return NULL;
}

void *QCQuaternion_SetFromRotationAxisAndAngle(void) {
    if (verbose) puts("STUB: QCQuaternion_SetFromRotationAxisAndAngle called");
    return NULL;
}

void *QCQuaternion_SetFromVectors(void) {
    if (verbose) puts("STUB: QCQuaternion_SetFromVectors called");
    return NULL;
}

void *QCQuaternion_SphericalLinearInterpolation(void) {
    if (verbose) puts("STUB: QCQuaternion_SphericalLinearInterpolation called");
    return NULL;
}

void *QCQuaternion_Square(void) {
    if (verbose) puts("STUB: QCQuaternion_Square called");
    return NULL;
}

void *QCQuaternion_SquareRoot(void) {
    if (verbose) puts("STUB: QCQuaternion_SquareRoot called");
    return NULL;
}

void *QCQuaternion_Substract(void) {
    if (verbose) puts("STUB: QCQuaternion_Substract called");
    return NULL;
}

void *QCResolveAliasPath(void) {
    if (verbose) puts("STUB: QCResolveAliasPath called");
    return NULL;
}

void *QCRestorePatchInputParameters(void) {
    if (verbose) puts("STUB: QCRestorePatchInputParameters called");
    return NULL;
}

void *QCSavePatchInputParameters(void) {
    if (verbose) puts("STUB: QCSavePatchInputParameters called");
    return NULL;
}

void *QCStateFromPath(void) {
    if (verbose) puts("STUB: QCStateFromPath called");
    return NULL;
}

void *QCVector_Add(void) {
    if (verbose) puts("STUB: QCVector_Add called");
    return NULL;
}

void *QCVector_CrossProduct(void) {
    if (verbose) puts("STUB: QCVector_CrossProduct called");
    return NULL;
}

void *QCVector_DotProduct(void) {
    if (verbose) puts("STUB: QCVector_DotProduct called");
    return NULL;
}

void *QCVector_EqualsVector(void) {
    if (verbose) puts("STUB: QCVector_EqualsVector called");
    return NULL;
}

void *QCVector_Length(void) {
    if (verbose) puts("STUB: QCVector_Length called");
    return NULL;
}

void *QCVector_LinearInterpolation(void) {
    if (verbose) puts("STUB: QCVector_LinearInterpolation called");
    return NULL;
}

void *QCVector_Make(void) {
    if (verbose) puts("STUB: QCVector_Make called");
    return NULL;
}

void *QCVector_Normalize(void) {
    if (verbose) puts("STUB: QCVector_Normalize called");
    return NULL;
}

void *QCVector_Scale(void) {
    if (verbose) puts("STUB: QCVector_Scale called");
    return NULL;
}

void *QCVector_Substract(void) {
    if (verbose) puts("STUB: QCVector_Substract called");
    return NULL;
}

void *QCVisualizerIPCClient_End(void) {
    if (verbose) puts("STUB: QCVisualizerIPCClient_End called");
    return NULL;
}

void *QCVisualizerIPCClient_GetAudioInformation(void) {
    if (verbose) puts("STUB: QCVisualizerIPCClient_GetAudioInformation called");
    return NULL;
}

void *QCVisualizerIPCClient_IsRunning(void) {
    if (verbose) puts("STUB: QCVisualizerIPCClient_IsRunning called");
    return NULL;
}

void *QCVisualizerIPCClient_Start(void) {
    if (verbose) puts("STUB: QCVisualizerIPCClient_Start called");
    return NULL;
}

void *QCVisualizerIPCServer_End(void) {
    if (verbose) puts("STUB: QCVisualizerIPCServer_End called");
    return NULL;
}

void *QCVisualizerIPCServer_HasClients(void) {
    if (verbose) puts("STUB: QCVisualizerIPCServer_HasClients called");
    return NULL;
}

void *QCVisualizerIPCServer_IsRunning(void) {
    if (verbose) puts("STUB: QCVisualizerIPCServer_IsRunning called");
    return NULL;
}

void *QCVisualizerIPCServer_SendAudioInformation(void) {
    if (verbose) puts("STUB: QCVisualizerIPCServer_SendAudioInformation called");
    return NULL;
}

void *QCVisualizerIPCServer_SendTrackInformation(void) {
    if (verbose) puts("STUB: QCVisualizerIPCServer_SendTrackInformation called");
    return NULL;
}

void *QCVisualizerIPCServer_Start(void) {
    if (verbose) puts("STUB: QCVisualizerIPCServer_Start called");
    return NULL;
}

