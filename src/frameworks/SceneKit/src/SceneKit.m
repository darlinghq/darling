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


#include <SceneKit/SceneKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern void* const SCNAnimationCommitOnCompletion = (void*)0;
extern void* const SCNBindingValueTransformerKey = (void*)0;
extern void* const SCNCaptureDeviceOutputConsumerOptionContainsAlpha = (void*)0;
extern void* const SCNConsistencyElementIDErrorKey = (void*)0;
extern void* const SCNConsistencyElementTypeErrorKey = (void*)0;
extern void* const SCNConsistencyLineNumberErrorKey = (void*)0;
extern void* const SCNDataProviderAnimationTime = (void*)0;
extern void* const SCNDataProviderCAWaitTime = (void*)0;
extern void* const SCNDataProviderCPUIdleTime = (void*)0;
extern void* const SCNDataProviderClientUpdateTime = (void*)0;
extern void* const SCNDataProviderConstraintsTime = (void*)0;
extern void* const SCNDataProviderFPS = (void*)0;
extern void* const SCNDataProviderFrameTime = (void*)0;
extern void* const SCNDataProviderGPUDeviceUtil = (void*)0;
extern void* const SCNDataProviderGPURendererUtil = (void*)0;
extern void* const SCNDataProviderGPUTilerUtil = (void*)0;
extern void* const SCNDataProviderParticleTime = (void*)0;
extern void* const SCNDataProviderPhysicsTime = (void*)0;
extern void* const SCNDataProviderPrimitivesProcessed = (void*)0;
extern void* const SCNDataProviderRenderTime = (void*)0;
extern void* const SCNDetailedErrorsKey = (void*)0;
extern void* const SCNErrorDomain = (void*)0;
extern void* const SCNExportPointOfViewAttribute = (void*)0;
extern void* const SCNExportSceneIdentifier = (void*)0;
extern void* const SCNFreeViewCameraName = (void*)0;
extern void* const SCNGeometrySourceSemanticBoneIndices = (void*)0;
extern void* const SCNGeometrySourceSemanticBoneWeights = (void*)0;
extern void* const SCNGeometrySourceSemanticColor = (void*)0;
extern void* const SCNGeometrySourceSemanticEdgeCrease = (void*)0;
extern void* const SCNGeometrySourceSemanticNormal = (void*)0;
extern void* const SCNGeometrySourceSemanticTangent = (void*)0;
extern void* const SCNGeometrySourceSemanticTexcoord = (void*)0;
extern void* const SCNGeometrySourceSemanticVertex = (void*)0;
extern void* const SCNGeometrySourceSemanticVertexCrease = (void*)0;
extern void* const SCNHitTestAllLayers = (void*)0;
extern void* const SCNHitTestBackFaceCullingKey = (void*)0;
extern void* const SCNHitTestBoundingBoxOnlyKey = (void*)0;
extern void* const SCNHitTestCategoryBitMaskKey = (void*)0;
extern void* const SCNHitTestClipToZRangeKey = (void*)0;
extern void* const SCNHitTestFirstFoundOnlyKey = (void*)0;
extern void* const SCNHitTestIgnoreChildNodesKey = (void*)0;
extern void* const SCNHitTestIgnoreHiddenNodesKey = (void*)0;
extern void* const SCNHitTestIgnoreTransparentNodesKey = (void*)0;
extern void* const SCNHitTestLayerMaskKey = (void*)0;
extern void* const SCNHitTestOptionCategoryBitMask = (void*)0;
extern void* const SCNHitTestOptionIgnoreLightArea = (void*)0;
extern void* const SCNHitTestOptionSearchMode = (void*)0;
extern void* const SCNHitTestOptionShowDebugInfo = (void*)0;
extern void* const SCNHitTestRootNodeKey = (void*)0;
extern void* const SCNHitTestSearchModeKey = (void*)0;
extern void* const SCNHitTestSortResultsKey = (void*)0;
extern void* const SCNLightAttenuationEndKey = (void*)0;
extern void* const SCNLightAttenuationFalloffExponentKey = (void*)0;
extern void* const SCNLightAttenuationStartKey = (void*)0;
extern void* const SCNLightConstantAttenuationKey = (void*)0;
extern void* const SCNLightFallOffAngleKey = (void*)0;
extern void* const SCNLightFallOffExponentKey = (void*)0;
extern void* const SCNLightGoboProjectShadows = (void*)0;
extern void* const SCNLightHotspotBeamKey = (void*)0;
extern void* const SCNLightLinearAttenuationKey = (void*)0;
extern void* const SCNLightQuadraticAttenuationKey = (void*)0;
extern void* const SCNLightShadowFarClippingKey = (void*)0;
extern void* const SCNLightShadowNearClippingKey = (void*)0;
extern void* const SCNLightSpotFalloffExponentKey = (void*)0;
extern void* const SCNLightSpotInnerAngleKey = (void*)0;
extern void* const SCNLightSpotOuterAngleKey = (void*)0;
extern void* const SCNLightTypeAmbient = (void*)0;
extern void* const SCNLightTypeArea = (void*)0;
extern void* const SCNLightTypeDirectional = (void*)0;
extern void* const SCNLightTypeIES = (void*)0;
extern void* const SCNLightTypeOmni = (void*)0;
extern void* const SCNLightTypeProbe = (void*)0;
extern void* const SCNLightTypeSpot = (void*)0;
extern void* const SCNLightingModelBlinn = (void*)0;
extern void* const SCNLightingModelConstant = (void*)0;
extern void* const SCNLightingModelLambert = (void*)0;
extern void* const SCNLightingModelNone = (void*)0;
extern void* const SCNLightingModelPhong = (void*)0;
extern void* const SCNLightingModelPhysicallyBased = (void*)0;
extern void* const SCNLightingModelShadowOnly = (void*)0;
extern void* const SCNMatrix4Identity = (void*)0;
extern void* const SCNModelTransform = (void*)0;
extern void* const SCNModelViewProjectionTransform = (void*)0;
extern void* const SCNModelViewTransform = (void*)0;
extern void* const SCNNormalTransform = (void*)0;
extern void* const SCNParticlePropertyAngle = (void*)0;
extern void* const SCNParticlePropertyAngularVelocity = (void*)0;
extern void* const SCNParticlePropertyBounce = (void*)0;
extern void* const SCNParticlePropertyCharge = (void*)0;
extern void* const SCNParticlePropertyColor = (void*)0;
extern void* const SCNParticlePropertyContactNormal = (void*)0;
extern void* const SCNParticlePropertyContactPoint = (void*)0;
extern void* const SCNParticlePropertyFrame = (void*)0;
extern void* const SCNParticlePropertyFrameRate = (void*)0;
extern void* const SCNParticlePropertyFriction = (void*)0;
extern void* const SCNParticlePropertyLife = (void*)0;
extern void* const SCNParticlePropertyOpacity = (void*)0;
extern void* const SCNParticlePropertyPosition = (void*)0;
extern void* const SCNParticlePropertyRotationAxis = (void*)0;
extern void* const SCNParticlePropertySize = (void*)0;
extern void* const SCNParticlePropertyVelocity = (void*)0;
extern void* const SCNPhysicsShapeCollisionMarginKey = (void*)0;
extern void* const SCNPhysicsShapeKeepAsCompoundKey = (void*)0;
extern void* const SCNPhysicsShapeOptionCollisionMargin = (void*)0;
extern void* const SCNPhysicsShapeScaleKey = (void*)0;
extern void* const SCNPhysicsShapeTypeBoundingBox = (void*)0;
extern void* const SCNPhysicsShapeTypeConcavePolyhedron = (void*)0;
extern void* const SCNPhysicsShapeTypeConvexHull = (void*)0;
extern void* const SCNPhysicsShapeTypeKey = (void*)0;
extern void* const SCNPhysicsTestBackfaceCullingKey = (void*)0;
extern void* const SCNPhysicsTestCollisionBitMaskKey = (void*)0;
extern void* const SCNPhysicsTestSearchModeAll = (void*)0;
extern void* const SCNPhysicsTestSearchModeAny = (void*)0;
extern void* const SCNPhysicsTestSearchModeClosest = (void*)0;
extern void* const SCNPhysicsTestSearchModeKey = (void*)0;
extern void* const SCNPreferLowPowerDeviceKey = (void*)0;
extern void* const SCNPreferredDeviceKey = (void*)0;
extern void* const SCNPreferredRenderingAPIKey = (void*)0;
extern void* const SCNPrimitiveCapSegments = (void*)0;
extern void* const SCNPrimitiveChamferSegments = (void*)0;
extern void* const SCNPrimitiveGenerateMappingCoordinates = (void*)0;
extern void* const SCNPrimitiveHeightSegments = (void*)0;
extern void* const SCNPrimitiveLengthSegments = (void*)0;
extern void* const SCNPrimitivePipeSegments = (void*)0;
extern void* const SCNPrimitiveRingSegments = (void*)0;
extern void* const SCNPrimitiveRotationSegments = (void*)0;
extern void* const SCNPrimitiveSphereSegments = (void*)0;
extern void* const SCNPrimitiveSphereType = (void*)0;
extern void* const SCNPrimitiveTypeGeosphere = (void*)0;
extern void* const SCNPrimitiveTypeSphere = (void*)0;
extern void* const SCNPrimitiveVertexTransformation = (void*)0;
extern void* const SCNPrimitiveWidthSegments = (void*)0;
extern void* const SCNProgramMappingChannelKey = (void*)0;
extern void* const SCNProjectionTransform = (void*)0;
extern void* const SCNRenderPassName = (void*)0;
extern void* const SCNRendererOffscreenSampleCount = (void*)0;
extern void* const SCNSceneEndTimeAttributeKey = (void*)0;
extern void* const SCNSceneExportCompressGeometryElements = (void*)0;
extern void* const SCNSceneExportCompressMorphTargets = (void*)0;
extern void* const SCNSceneExportDestinationURL = (void*)0;
extern void* const SCNSceneExportEmbedImages = (void*)0;
extern void* const SCNSceneExportEmbedReferences = (void*)0;
extern void* const SCNSceneExportPresentationTree = (void*)0;
extern void* const SCNSceneExportUTIKey = (void*)0;
extern void* const SCNSceneFrameRateAttributeKey = (void*)0;
extern void* SCNSceneKitVersionNumber = (void*)0;
extern void* const SCNSceneSourceAdjustInvalidCameraClippingPlanesKey = (void*)0;
extern void* const SCNSceneSourceAnimationImportPolicyDoNotPlay = (void*)0;
extern void* const SCNSceneSourceAnimationImportPolicyKey = (void*)0;
extern void* const SCNSceneSourceAnimationImportPolicyPlay = (void*)0;
extern void* const SCNSceneSourceAnimationImportPolicyPlayRepeatedly = (void*)0;
extern void* const SCNSceneSourceAnimationImportPolicyPlayUsingSceneTimeBase = (void*)0;
extern void* const SCNSceneSourceAnimationLoadingModeKey = (void*)0;
extern void* const SCNSceneSourceAnimationLoadingModePlayUsingSceneTime = (void*)0;
extern void* const SCNSceneSourceAssetAuthorKey = (void*)0;
extern void* const SCNSceneSourceAssetAuthoringToolKey = (void*)0;
extern void* const SCNSceneSourceAssetContributorsKey = (void*)0;
extern void* const SCNSceneSourceAssetCreatedDateKey = (void*)0;
extern void* const SCNSceneSourceAssetDirectoryURLsKey = (void*)0;
extern void* const SCNSceneSourceAssetModifiedDateKey = (void*)0;
extern void* const SCNSceneSourceAssetUnitKey = (void*)0;
extern void* const SCNSceneSourceAssetUnitMeterKey = (void*)0;
extern void* const SCNSceneSourceAssetUnitNameKey = (void*)0;
extern void* const SCNSceneSourceAssetUpAxisKey = (void*)0;
extern void* const SCNSceneSourceCheckConsistencyKey = (void*)0;
extern void* const SCNSceneSourceConvertToYUpIfNeededKey = (void*)0;
extern void* const SCNSceneSourceConvertToYUpKey = (void*)0;
extern void* const SCNSceneSourceConvertUnitsToMetersKey = (void*)0;
extern void* const SCNSceneSourceCreateCameraIfAbsentKey = (void*)0;
extern void* const SCNSceneSourceCreateLightIfAbsentKey = (void*)0;
extern void* const SCNSceneSourceCreateNormalsIfAbsentKey = (void*)0;
extern void* const SCNSceneSourceDocumentDestinationURL = (void*)0;
extern void* const SCNSceneSourceFlattenSceneKey = (void*)0;
extern void* const SCNSceneSourceForceInProcess = (void*)0;
extern void* const SCNSceneSourceForceMorphNormals = (void*)0;
extern void* const SCNSceneSourceFormat = (void*)0;
extern void* const SCNSceneSourceFormatC3D = (void*)0;
extern void* const SCNSceneSourceFormatCOLLADA = (void*)0;
extern void* const SCNSceneSourceFormatHint = (void*)0;
extern void* const SCNSceneSourceFormatOBJ = (void*)0;
extern void* const SCNSceneSourceFormatSCN = (void*)0;
extern void* const SCNSceneSourceGeometryMemoryUsageKey = (void*)0;
extern void* const SCNSceneSourceInputSource = (void*)0;
extern void* const SCNSceneSourceLoadTexturesInCPUMemory = (void*)0;
extern void* const SCNSceneSourceLoadingOptionPreserveOriginalTopology = (void*)0;
extern void* const SCNSceneSourceOverrideAssetURLsKey = (void*)0;
extern void* const SCNSceneSourceSceneIndexKey = (void*)0;
extern void* const SCNSceneSourceShouldCacheKey = (void*)0;
extern void* const SCNSceneSourceSkipConsistencyCheckKey = (void*)0;
extern void* const SCNSceneSourceStrictConformanceKey = (void*)0;
extern void* const SCNSceneSourceTextureMemoryUsageKey = (void*)0;
extern void* const SCNSceneSourceUseModelIOToLoadDAEFiles = (void*)0;
extern void* const SCNSceneSourceUseSafeModeKey = (void*)0;
extern void* const SCNSceneSourceVerboseMode = (void*)0;
extern void* const SCNSceneStartTimeAttributeKey = (void*)0;
extern void* const SCNSceneUpAxisAttributeKey = (void*)0;
extern void* const SCNSceneWriteToURLShouldKeepReferencesToFilesOption = (void*)0;
extern void* const SCNSceneWriteToURLShouldZipOption = (void*)0;
extern void* const SCNShaderModifierEntryPointFragment = (void*)0;
extern void* const SCNShaderModifierEntryPointGeometry = (void*)0;
extern void* const SCNShaderModifierEntryPointLightingModel = (void*)0;
extern void* const SCNShaderModifierEntryPointSurface = (void*)0;
extern void* const SCNTextureOffsetXKey = (void*)0;
extern void* const SCNTextureOffsetYKey = (void*)0;
extern void* const SCNVector3Zero = (void*)0;
extern void* const SCNVector4Zero = (void*)0;
extern void* const SCNViewTransform = (void*)0;
extern void* const _SCNShadableDidCompileNotificationName = (void*)0;
extern void* const _SCNShadableIssuesKey = (void*)0;
extern void* const kC3DGeometryKey = (void*)0;
extern void* const kC3DIOSceneSourceAutoAdjustInvalidZRange = (void*)0;
extern void* const kC3DIOSceneSourceAutoLimitMemoryForImages = (void*)0;
extern void* const kC3DIOSceneSourceCheckConsistency = (void*)0;
extern void* const kC3DIOSceneSourceCleanupMeshes = (void*)0;
extern void* const kC3DIOSceneSourceFlattenScene = (void*)0;
extern void* const kC3DIOSceneSourceInterleaveSources = (void*)0;
extern void* const kC3DIOSceneSourceMakeSourcesCheaper = (void*)0;
extern void* const kC3DIOSceneSourceProcessIsSandboxed = (void*)0;
extern void* const kC3DIOSceneSourceRemoveAllLights = (void*)0;
extern void* const kC3DIOSceneSourceRemoveColorArrays = (void*)0;
extern void* const kC3DIOSceneSourceRemoveConstantAnimations = (void*)0;
extern void* const kC3DIOSceneSourceSkipImagePathResolution = (void*)0;
extern void* const kC3DIOSceneSourceSplitMeshesForGLES = (void*)0;
extern void* const kC3DIOSceneSourceTriggerOptionsForRealtimeViewer = (void*)0;
extern void* const kC3DIOSceneSourceUseSafeMode = (void*)0;
extern void* const kC3DImageInitFromPath = (void*)0;
extern void* const kC3DImageLocation = (void*)0;
extern void* const kC3DImageRelativePath = (void*)0;
extern void* const kC3DSceneDidUpdateNotification = (void*)0;
extern void* const kC3DTriggerOptionsForRealtimeViewer = (void*)0;
extern void* const kSCNAuthoringLayerIndex = (void*)0;
extern void* const kSCNAuthoringLayerMask = (void*)0;
extern void* const kSCNAuthoringOverlayLayerIndex = (void*)0;
extern void* const kSCNAuthoringOverlayLayerMask = (void*)0;

void *C3DGetFileDecryptionCallbacks(void) {
    if (verbose) puts("STUB: C3DGetFileDecryptionCallbacks called");
    return NULL;
}

void *C3DIOCreateCStringFromCFString(void) {
    if (verbose) puts("STUB: C3DIOCreateCStringFromCFString called");
    return NULL;
}

void *C3DIOCreateOptionsForRealtimeViewer(void) {
    if (verbose) puts("STUB: C3DIOCreateOptionsForRealtimeViewer called");
    return NULL;
}

void *C3DImageGetAttribute(void) {
    if (verbose) puts("STUB: C3DImageGetAttribute called");
    return NULL;
}

void *C3DImageGetURL(void) {
    if (verbose) puts("STUB: C3DImageGetURL called");
    return NULL;
}

void *C3DImageSetAttribute(void) {
    if (verbose) puts("STUB: C3DImageSetAttribute called");
    return NULL;
}

void *C3DSetFileDecryptionCallbacks(void) {
    if (verbose) puts("STUB: C3DSetFileDecryptionCallbacks called");
    return NULL;
}

void *SCNDeleteMetalShaderCache(void) {
    if (verbose) puts("STUB: SCNDeleteMetalShaderCache called");
    return NULL;
}

void *SCNExportJavaScriptModule(void) {
    if (verbose) puts("STUB: SCNExportJavaScriptModule called");
    return NULL;
}

void *SCNGetExternalTextureSource(void) {
    if (verbose) puts("STUB: SCNGetExternalTextureSource called");
    return NULL;
}

void *SCNGetPerformanceStatistics(void) {
    if (verbose) puts("STUB: SCNGetPerformanceStatistics called");
    return NULL;
}

void *SCNGetResourceBundle(void) {
    if (verbose) puts("STUB: SCNGetResourceBundle called");
    return NULL;
}

void *SCNGetShaderCollectionOutputURL(void) {
    if (verbose) puts("STUB: SCNGetShaderCollectionOutputURL called");
    return NULL;
}

void *SCNHasSpecialResourceBundle(void) {
    if (verbose) puts("STUB: SCNHasSpecialResourceBundle called");
    return NULL;
}

void *SCNMatrix4EqualToMatrix4(void) {
    if (verbose) puts("STUB: SCNMatrix4EqualToMatrix4 called");
    return NULL;
}

void *SCNMatrix4FromGLKMatrix4(void) {
    if (verbose) puts("STUB: SCNMatrix4FromGLKMatrix4 called");
    return NULL;
}

void *SCNMatrix4Invert(void) {
    if (verbose) puts("STUB: SCNMatrix4Invert called");
    return NULL;
}

void *SCNMatrix4IsIdentity(void) {
    if (verbose) puts("STUB: SCNMatrix4IsIdentity called");
    return NULL;
}

void *SCNMatrix4MakeRotation(void) {
    if (verbose) puts("STUB: SCNMatrix4MakeRotation called");
    return NULL;
}

void *SCNMatrix4Mult(void) {
    if (verbose) puts("STUB: SCNMatrix4Mult called");
    return NULL;
}

void *SCNMatrix4Rotate(void) {
    if (verbose) puts("STUB: SCNMatrix4Rotate called");
    return NULL;
}

void *SCNMatrix4Scale(void) {
    if (verbose) puts("STUB: SCNMatrix4Scale called");
    return NULL;
}

void *SCNMatrix4ToGLKMatrix4(void) {
    if (verbose) puts("STUB: SCNMatrix4ToGLKMatrix4 called");
    return NULL;
}

void *SCNNodeGetBoundingSphere(void) {
    if (verbose) puts("STUB: SCNNodeGetBoundingSphere called");
    return NULL;
}

void *SCNNodesToMDLAsset(void) {
    if (verbose) puts("STUB: SCNNodesToMDLAsset called");
    return NULL;
}

void *SCNResolveImageContents(void) {
    if (verbose) puts("STUB: SCNResolveImageContents called");
    return NULL;
}

void *SCNSetExternalTextureSource(void) {
    if (verbose) puts("STUB: SCNSetExternalTextureSource called");
    return NULL;
}

void *SCNSetPerformanceStatisticsEnabled(void) {
    if (verbose) puts("STUB: SCNSetPerformanceStatisticsEnabled called");
    return NULL;
}

void *SCNSetResourceBundle(void) {
    if (verbose) puts("STUB: SCNSetResourceBundle called");
    return NULL;
}

void *SCNSetShaderCollectionEnabled(void) {
    if (verbose) puts("STUB: SCNSetShaderCollectionEnabled called");
    return NULL;
}

void *SCNTextureContentWithGLInfo(void) {
    if (verbose) puts("STUB: SCNTextureContentWithGLInfo called");
    return NULL;
}

void *SCNVector3EqualToVector3(void) {
    if (verbose) puts("STUB: SCNVector3EqualToVector3 called");
    return NULL;
}

void *SCNVector4EqualToVector4(void) {
    if (verbose) puts("STUB: SCNVector4EqualToVector4 called");
    return NULL;
}

void *SCNWriteMTLTextureToFile(void) {
    if (verbose) puts("STUB: SCNWriteMTLTextureToFile called");
    return NULL;
}

void *_SCNGetCollectedShadersForPid(void) {
    if (verbose) puts("STUB: _SCNGetCollectedShadersForPid called");
    return NULL;
}

void *_SCNGetPerformanceStatisticsForPid(void) {
    if (verbose) puts("STUB: _SCNGetPerformanceStatisticsForPid called");
    return NULL;
}

void *_SCNSetLinearRenderingEnabled(void) {
    if (verbose) puts("STUB: _SCNSetLinearRenderingEnabled called");
    return NULL;
}

void *_SCNSetShaderCacheURL(void) {
    if (verbose) puts("STUB: _SCNSetShaderCacheURL called");
    return NULL;
}

void *_SCNSetShouldPostShadableCompilationNotifications(void) {
    if (verbose) puts("STUB: _SCNSetShouldPostShadableCompilationNotifications called");
    return NULL;
}

void *_SCNShadableConvertShaderToMetalIfNeeded(void) {
    if (verbose) puts("STUB: _SCNShadableConvertShaderToMetalIfNeeded called");
    return NULL;
}

void *_SCNStartCollectingPerformanceStatisticsForPid(void) {
    if (verbose) puts("STUB: _SCNStartCollectingPerformanceStatisticsForPid called");
    return NULL;
}

void *_SCNStopCollectingPerformanceStatisticsForPid(void) {
    if (verbose) puts("STUB: _SCNStopCollectingPerformanceStatisticsForPid called");
    return NULL;
}

void *scn_frustum_classify_aabb(void) {
    if (verbose) puts("STUB: scn_frustum_classify_aabb called");
    return NULL;
}

void *scn_matrix_compute_normal(void) {
    if (verbose) puts("STUB: scn_matrix_compute_normal called");
    return NULL;
}

void *scn_matrix_fast_invert(void) {
    if (verbose) puts("STUB: scn_matrix_fast_invert called");
    return NULL;
}

void *scn_plane_intersect_3_planes(void) {
    if (verbose) puts("STUB: scn_plane_intersect_3_planes called");
    return NULL;
}

void *scn_triangle_barycentric_coordinates(void) {
    if (verbose) puts("STUB: scn_triangle_barycentric_coordinates called");
    return NULL;
}

void *scn_triangle_point_distance_squared(void) {
    if (verbose) puts("STUB: scn_triangle_point_distance_squared called");
    return NULL;
}

