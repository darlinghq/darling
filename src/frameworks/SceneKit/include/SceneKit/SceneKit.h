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

#ifndef _SCENEKIT_H_
#define _SCENEKIT_H_

#import <Foundation/Foundation.h>

#import <SceneKit/AVAudio3DMixing.h>
#import <SceneKit/AVCaptureVideoDataOutputSampleBufferDelegate.h>
#import <SceneKit/C3DAnimationBinding.h>
#import <SceneKit/C3DBinding.h>
#import <SceneKit/C3DEngineNotificationQueueTransientWrapper.h>
#import <SceneKit/C3DIONSZipFileArchive.h>
#import <SceneKit/CAAnimation+SceneKitAdditions.h>
#import <SceneKit/CAKeyframeAnimation+SCNAddition.h>
#import <SceneKit/CALayer+SCN_JSSupport.h>
#import <SceneKit/JSExport.h>
#import <SceneKit/MDLAsset+SCNModelIO.h>
#import <SceneKit/MDLCamera+SCNModelIO.h>
#import <SceneKit/MDLComponent.h>
#import <SceneKit/MDLLight+SCNModelIO.h>
#import <SceneKit/MDLLightProbe+SCNModelIO.h>
#import <SceneKit/MDLMaterial+SCNModelIO.h>
#import <SceneKit/MDLMesh+SCNModelIO.h>
#import <SceneKit/MDLMorphDeformerComponent.h>
#import <SceneKit/MDLObject+SCNModelIO.h>
#import <SceneKit/MDLSkinDeformerComponent.h>
#import <SceneKit/MDLSubmesh+SCNModelIO.h>
#import <SceneKit/MTLBuffer.h>
#import <SceneKit/MTLDevice.h>
#import <SceneKit/MTLResource.h>
#import <SceneKit/MTLTexture.h>
#import <SceneKit/NSArray+SCNAdditions.h>
#import <SceneKit/NSCoder+SCNExtensions.h>
#import <SceneKit/NSColor+SCNAdditions.h>
#import <SceneKit/NSControlTextEditingDelegate.h>
#import <SceneKit/NSData+SCNExtensions.h>
#import <SceneKit/NSDictionary+SCNAdditions.h>
#import <SceneKit/NSImage+SCNAddition.h>
#import <SceneKit/NSImageRep+SCNAdditions.h>
#import <SceneKit/NSKeyedUnarchiverDelegate.h>
#import <SceneKit/NSObject+SCN_DisplayLinkExtensions.h>
#import <SceneKit/NSOutlineViewDataSource.h>
#import <SceneKit/NSOutlineViewDelegate.h>
#import <SceneKit/NSString+SCNExtensions.h>
#import <SceneKit/NSValue+Core3DKitAdditions.h>
#import <SceneKit/SCN3DConnexionDevice.h>
#import <SceneKit/SCNAVPlayerSource.h>
#import <SceneKit/SCNAccelerationConstraint.h>
#import <SceneKit/SCNAction.h>
#import <SceneKit/SCNActionCustom.h>
#import <SceneKit/SCNActionFade.h>
#import <SceneKit/SCNActionGroup.h>
#import <SceneKit/SCNActionHide.h>
#import <SceneKit/SCNActionJSExport.h>
#import <SceneKit/SCNActionJavaScript.h>
#import <SceneKit/SCNActionMove.h>
#import <SceneKit/SCNActionPerformSelector.h>
#import <SceneKit/SCNActionPlaySound.h>
#import <SceneKit/SCNActionReference.h>
#import <SceneKit/SCNActionRemove.h>
#import <SceneKit/SCNActionRepeat.h>
#import <SceneKit/SCNActionRotate.h>
#import <SceneKit/SCNActionRunAction.h>
#import <SceneKit/SCNActionRunBlock.h>
#import <SceneKit/SCNActionScale.h>
#import <SceneKit/SCNActionSequence.h>
#import <SceneKit/SCNActionTargetState.h>
#import <SceneKit/SCNActionWait.h>
#import <SceneKit/SCNActionable.h>
#import <SceneKit/SCNAnimatable.h>
#import <SceneKit/SCNAnimation.h>
#import <SceneKit/SCNAnimation.h>
#import <SceneKit/SCNAnimationEvent.h>
#import <SceneKit/SCNAnimationEventJSExport.h>
#import <SceneKit/SCNAnimationPlayer.h>
#import <SceneKit/SCNAnimationReference.h>
#import <SceneKit/SCNAssetCatalog.h>
#import <SceneKit/SCNAssetCatalogCacheEntry.h>
#import <SceneKit/SCNAudioPlayer.h>
#import <SceneKit/SCNAudioSource.h>
#import <SceneKit/SCNAuthoringEnvironment.h>
#import <SceneKit/SCNAuthoringEnvironment2.h>
#import <SceneKit/SCNAvoidOccluderConstraint.h>
#import <SceneKit/SCNBackingLayer.h>
#import <SceneKit/SCNBillboardConstraint.h>
#import <SceneKit/SCNBoundingBox.h>
#import <SceneKit/SCNBoundingBoxJSExport.h>
#import <SceneKit/SCNBoundingSphere.h>
#import <SceneKit/SCNBoundingSphereJSExport.h>
#import <SceneKit/SCNBoundingVolume.h>
#import <SceneKit/SCNBox.h>
#import <SceneKit/SCNBoxJSExport.h>
#import <SceneKit/SCNBufferBinding.h>
#import <SceneKit/SCNBufferStream.h>
#import <SceneKit/SCNCOLLADAExportOperation.h>
#import <SceneKit/SCNCamera.h>
#import <SceneKit/SCNCameraControlConfiguration.h>
#import <SceneKit/SCNCameraControlEventHandler.h>
#import <SceneKit/SCNCameraController.h>
#import <SceneKit/SCNCameraControllerDelegate.h>
#import <SceneKit/SCNCameraJSExport.h>
#import <SceneKit/SCNCameraNavigationController.h>
#import <SceneKit/SCNCapsule.h>
#import <SceneKit/SCNCapsuleJSExport.h>
#import <SceneKit/SCNCaptureDeviceOutputConsumer.h>
#import <SceneKit/SCNCaptureDeviceOutputConsumerSource.h>
#import <SceneKit/SCNCaptureDeviceSource.h>
#import <SceneKit/SCNColorJSExport.h>
#import <SceneKit/SCNCommonProfileProgramCache.h>
#import <SceneKit/SCNCommonProfileProgramGenerator.h>
#import <SceneKit/SCNCommonProfileProgramGeneratorGL.h>
#import <SceneKit/SCNCommonProfileProgramGeneratorMetal.h>
#import <SceneKit/SCNCone.h>
#import <SceneKit/SCNConeJSExport.h>
#import <SceneKit/SCNConstantScaleConstraint.h>
#import <SceneKit/SCNConstraint.h>
#import <SceneKit/SCNConstraintJSExport.h>
#import <SceneKit/SCNCoreAnimationSource.h>
#import <SceneKit/SCNCylinder.h>
#import <SceneKit/SCNCylinderJSExport.h>
#import <SceneKit/SCNDeferredProgramGeneratorMetal.h>
#import <SceneKit/SCNDelegateSource.h>
#import <SceneKit/SCNDisplayLink.h>
#import <SceneKit/SCNDistanceConstraint.h>
#import <SceneKit/SCNEffect.h>
#import <SceneKit/SCNEventHandler.h>
#import <SceneKit/SCNEventHandler.h>
#import <SceneKit/SCNExportOperation.h>
#import <SceneKit/SCNFixedSizePage.h>
#import <SceneKit/SCNFloor.h>
#import <SceneKit/SCNFloorJSExport.h>
#import <SceneKit/SCNGeometry.h>
#import <SceneKit/SCNGeometryDeformer.h>
#import <SceneKit/SCNGeometryElement.h>
#import <SceneKit/SCNGeometryElementJSExport.h>
#import <SceneKit/SCNGeometryJSExport.h>
#import <SceneKit/SCNGeometrySource.h>
#import <SceneKit/SCNGeometrySourceJSExport.h>
#import <SceneKit/SCNGeometryTessellator.h>
#import <SceneKit/SCNHitTestResult.h>
#import <SceneKit/SCNHitTestResultJSExport.h>
#import <SceneKit/SCNIKConstraint.h>
#import <SceneKit/SCNIKConstraintJSExport.h>
#import <SceneKit/SCNIKJoint.h>
#import <SceneKit/SCNImageSource.h>
#import <SceneKit/SCNJSAnimationGroupJSExport.h>
#import <SceneKit/SCNJSBasicAnimationJSExport.h>
#import <SceneKit/SCNJSImage.h>
#import <SceneKit/SCNJSImageJSExport.h>
#import <SceneKit/SCNJSKeyframeAnimationJSExport.h>
#import <SceneKit/SCNJSLayerJSExport.h>
#import <SceneKit/SCNJSMediaTimingFunctionJSExport.h>
#import <SceneKit/SCNJSValue.h>
#import <SceneKit/SCNJSValueSExport.h>
#import <SceneKit/SCNJSValueTmpImp.h>
#import <SceneKit/SCNJitterer.h>
#import <SceneKit/SCNKeyedArchiver.h>
#import <SceneKit/SCNKeyedUnarchiver.h>
#import <SceneKit/SCNLayer.h>
#import <SceneKit/SCNLevelOfDetail.h>
#import <SceneKit/SCNLevelOfDetailJSExport.h>
#import <SceneKit/SCNLight.h>
#import <SceneKit/SCNLightJSExport.h>
#import <SceneKit/SCNLookAtConstraint.h>
#import <SceneKit/SCNLookAtConstraintJSExport.h>
#import <SceneKit/SCNLowLatencyMetalLayer.h>
#import <SceneKit/SCNMTLArgumentBinder.h>
#import <SceneKit/SCNMTLBuffer.h>
#import <SceneKit/SCNMTLBufferAllocator.h>
#import <SceneKit/SCNMTLBufferAllocatorSubBuffer.h>
#import <SceneKit/SCNMTLComputePipeline.h>
#import <SceneKit/SCNMTLLibrary.h>
#import <SceneKit/SCNMTLLibraryManager.h>
#import <SceneKit/SCNMTLMesh.h>
#import <SceneKit/SCNMTLMeshElement.h>
#import <SceneKit/SCNMTLMorph.h>
#import <SceneKit/SCNMTLPassResourceBinding.h>
#import <SceneKit/SCNMTLRenderContext.h>
#import <SceneKit/SCNMTLRenderContextCommandBufferStatusMonitor.h>
#import <SceneKit/SCNMTLRenderContextResourceManagerMonitor.h>
#import <SceneKit/SCNMTLRenderPipeline.h>
#import <SceneKit/SCNMTLResourceBinding.h>
#import <SceneKit/SCNMTLResourceManager.h>
#import <SceneKit/SCNMTLSemanticResourceBinding.h>
#import <SceneKit/SCNMTLShadable.h>
#import <SceneKit/SCNMTLShadableKey.h>
#import <SceneKit/SCNMTLShaderBindingsGenerator.h>
#import <SceneKit/SCNMTLSkin.h>
#import <SceneKit/SCNMTLTessellator.h>
#import <SceneKit/SCNManipulableItem.h>
#import <SceneKit/SCNManipulator.h>
#import <SceneKit/SCNMaterial.h>
#import <SceneKit/SCNMaterialAttachment.h>
#import <SceneKit/SCNMaterialJSExport.h>
#import <SceneKit/SCNMaterialProperty.h>
#import <SceneKit/SCNMaterialPropertyJSExport.h>
#import <SceneKit/SCNMaterialPropertyTextureProvider.h>
#import <SceneKit/SCNMaterialPropertyTextureProviderHelper.h>
#import <SceneKit/SCNMaterialPropertyTextureProviderSource.h>
#import <SceneKit/SCNMetalBackingLayer.h>
#import <SceneKit/SCNMetalLayer.h>
#import <SceneKit/SCNMetalLayerJSExport.h>
#import <SceneKit/SCNMetalWireframeResource.h>
#import <SceneKit/SCNMonitor.h>
#import <SceneKit/SCNMorpher.h>
#import <SceneKit/SCNMorpherJSExport.h>
#import <SceneKit/SCNMovieExportOperation.h>
#import <SceneKit/SCNMutableGeometry.h>
#import <SceneKit/SCNMutableGeometrySource.h>
#import <SceneKit/SCNNode.h>
#import <SceneKit/SCNNodeComponent.h>
#import <SceneKit/SCNNodeJSExport.h>
#import <SceneKit/SCNNodeManipulableItem.h>
#import <SceneKit/SCNNodeReference.h>
#import <SceneKit/SCNNodeWeakComponent.h>
#import <SceneKit/SCNOffscreenRenderer.h>
#import <SceneKit/SCNOrderedDictionary.h>
#import <SceneKit/SCNParticlePropertyController.h>
#import <SceneKit/SCNParticlePropertyControllerJSExport.h>
#import <SceneKit/SCNParticleSystem.h>
#import <SceneKit/SCNParticleSystemJSExport.h>
#import <SceneKit/SCNPass.h>
#import <SceneKit/SCNPassContext.h>
#import <SceneKit/SCNPhysicsBallSocketJoint.h>
#import <SceneKit/SCNPhysicsBallSocketJointJSExport.h>
#import <SceneKit/SCNPhysicsBehavior.h>
#import <SceneKit/SCNPhysicsBehaviorJSExport.h>
#import <SceneKit/SCNPhysicsBody.h>
#import <SceneKit/SCNPhysicsBodyJSExport.h>
#import <SceneKit/SCNPhysicsCharacter.h>
#import <SceneKit/SCNPhysicsConeTwistJoint.h>
#import <SceneKit/SCNPhysicsContact.h>
#import <SceneKit/SCNPhysicsContactJSExport.h>
#import <SceneKit/SCNPhysicsCustomField.h>
#import <SceneKit/SCNPhysicsDragField.h>
#import <SceneKit/SCNPhysicsElectricField.h>
#import <SceneKit/SCNPhysicsField.h>
#import <SceneKit/SCNPhysicsFieldJSExport.h>
#import <SceneKit/SCNPhysicsHingeJoint.h>
#import <SceneKit/SCNPhysicsHingeJointJSExport.h>
#import <SceneKit/SCNPhysicsLinearGravityField.h>
#import <SceneKit/SCNPhysicsMagneticField.h>
#import <SceneKit/SCNPhysicsNoiseField.h>
#import <SceneKit/SCNPhysicsRadialGravityField.h>
#import <SceneKit/SCNPhysicsShape.h>
#import <SceneKit/SCNPhysicsShapeJSExport.h>
#import <SceneKit/SCNPhysicsSliderJoint.h>
#import <SceneKit/SCNPhysicsSliderJointJSExport.h>
#import <SceneKit/SCNPhysicsSpringField.h>
#import <SceneKit/SCNPhysicsTurbulenceField.h>
#import <SceneKit/SCNPhysicsVehicle.h>
#import <SceneKit/SCNPhysicsVehicleJSExport.h>
#import <SceneKit/SCNPhysicsVehicleWheel.h>
#import <SceneKit/SCNPhysicsVehicleWheelJSExport.h>
#import <SceneKit/SCNPhysicsVortexField.h>
#import <SceneKit/SCNPhysicsWorld.h>
#import <SceneKit/SCNPhysicsWorldJSExport.h>
#import <SceneKit/SCNPlane.h>
#import <SceneKit/SCNPlaneJSExport.h>
#import <SceneKit/SCNPrecomputedLightingEnvironment.h>
#import <SceneKit/SCNProgram.h>
#import <SceneKit/SCNProgramJSExport.h>
#import <SceneKit/SCNProgramSemanticInfo.h>
#import <SceneKit/SCNPyramid.h>
#import <SceneKit/SCNPyramidJSExport.h>
#import <SceneKit/SCNRecursiveLock.h>
#import <SceneKit/SCNReferenceNode.h>
#import <SceneKit/SCNRenderTarget.h>
#import <SceneKit/SCNRenderTargetView.h>
#import <SceneKit/SCNRenderer.h>
#import <SceneKit/SCNRendererEvents.h>
#import <SceneKit/SCNRendererJSExport.h>
#import <SceneKit/SCNRendererOptionsPanel.h>
#import <SceneKit/SCNRendererOptionsPanelController.h>
#import <SceneKit/SCNRendererTransitionContext.h>
#import <SceneKit/SCNRendererViewPoint.h>
#import <SceneKit/SCNReplicatorConstraint.h>
#import <SceneKit/SCNScene.h>
#import <SceneKit/SCNSceneDatabase.h>
#import <SceneKit/SCNSceneJSExport.h>
#import <SceneKit/SCNSceneLoaderProtocol.h>
#import <SceneKit/SCNSceneLookUpUnarchiver.h>
#import <SceneKit/SCNSceneRenderer.h>
#import <SceneKit/SCNSceneSource.h>
#import <SceneKit/SCNSceneSourceJSExport.h>
#import <SceneKit/SCNShadable.h>
#import <SceneKit/SCNShadableHelper.h>
#import <SceneKit/SCNShape.h>
#import <SceneKit/SCNShapeJSExport.h>
#import <SceneKit/SCNSkinner.h>
#import <SceneKit/SCNSliderConstraint.h>
#import <SceneKit/SCNSourceRendererRegistry.h>
#import <SceneKit/SCNSphere.h>
#import <SceneKit/SCNSphereJSExport.h>
#import <SceneKit/SCNSpriteKitEventHandler.h>
#import <SceneKit/SCNSpriteKitSource.h>
#import <SceneKit/SCNStatisticsProvider.h>
#import <SceneKit/SCNTechnique.h>
#import <SceneKit/SCNTechniqueJSExport.h>
#import <SceneKit/SCNTechniquePanel.h>
#import <SceneKit/SCNTechniquePanelController.h>
#import <SceneKit/SCNTechniqueSupport.h>
#import <SceneKit/SCNText.h>
#import <SceneKit/SCNTextJSExport.h>
#import <SceneKit/SCNTextureCoreAnimationSource.h>
#import <SceneKit/SCNTextureDelegateSource.h>
#import <SceneKit/SCNTextureOffscreenRenderingSource.h>
#import <SceneKit/SCNTextureSource.h>
#import <SceneKit/SCNTextureSpriteKitSource.h>
#import <SceneKit/SCNTimingFunction.h>
#import <SceneKit/SCNTorus.h>
#import <SceneKit/SCNTorusJSExport.h>
#import <SceneKit/SCNTransaction.h>
#import <SceneKit/SCNTransactionCommandObject.h>
#import <SceneKit/SCNTransactionJSExport.h>
#import <SceneKit/SCNTransformConstraint.h>
#import <SceneKit/SCNTransformConstraintJSExport.h>
#import <SceneKit/SCNTube.h>
#import <SceneKit/SCNTubeJSExport.h>
#import <SceneKit/SCNTypeInspector.h>
#import <SceneKit/SCNUIArrayEditor.h>
#import <SceneKit/SCNUIArrayEditorController.h>
#import <SceneKit/SCNUIBindableView.h>
#import <SceneKit/SCNUIColorWell.h>
#import <SceneKit/SCNUID_ConstantTransformer.h>
#import <SceneKit/SCNUID_NumberTransformer.h>
#import <SceneKit/SCNUIDynamicInspector.h>
#import <SceneKit/SCNUIDynamicInspectorConfiguration.h>
#import <SceneKit/SCNUIHandleButton.h>
#import <SceneKit/SCNUIImagePicker.h>
#import <SceneKit/SCNUIMatrix4TextField.h>
#import <SceneKit/SCNUIPopUpButton.h>
#import <SceneKit/SCNUIProperty.h>
#import <SceneKit/SCNUISceneGraphView.h>
#import <SceneKit/SCNUIScrubber.h>
#import <SceneKit/SCNUIScrubberCell.h>
#import <SceneKit/SCNUIVector2TextField.h>
#import <SceneKit/SCNUIVector3TextField.h>
#import <SceneKit/SCNUIVector4TextField.h>
#import <SceneKit/SCNView.h>
#import <SceneKit/SCNViewJSExport.h>
#import <SceneKit/SCNWeakPointer.h>
#import <SceneKit/SCN_CAKeyframeAnimation.h>
#import <SceneKit/SCN_NSImageFromImageValue.h>
#import <SceneKit/SCN_NSImageFromSlotContents.h>
#import <SceneKit/USKData_helper.h>
#import <SceneKit/USKHelper.h>
#import <SceneKit/USKObjectPath_helper.h>
#import <SceneKit/USKToken_helper.h>
#import <SceneKit/_C3DProgressDebugger.h>
#import <SceneKit/_SCNExportOperation.h>
#import <SceneKit/_SCNShadableCompilationIssue.h>


void *C3DGetFileDecryptionCallbacks(void);
void *C3DIOCreateCStringFromCFString(void);
void *C3DIOCreateOptionsForRealtimeViewer(void);
void *C3DImageGetAttribute(void);
void *C3DImageGetURL(void);
void *C3DImageSetAttribute(void);
void *C3DSetFileDecryptionCallbacks(void);
void *SCNDeleteMetalShaderCache(void);
void *SCNExportJavaScriptModule(void);
void *SCNGetExternalTextureSource(void);
void *SCNGetPerformanceStatistics(void);
void *SCNGetResourceBundle(void);
void *SCNGetShaderCollectionOutputURL(void);
void *SCNHasSpecialResourceBundle(void);
void *SCNMatrix4EqualToMatrix4(void);
void *SCNMatrix4FromGLKMatrix4(void);
void *SCNMatrix4Invert(void);
void *SCNMatrix4IsIdentity(void);
void *SCNMatrix4MakeRotation(void);
void *SCNMatrix4Mult(void);
void *SCNMatrix4Rotate(void);
void *SCNMatrix4Scale(void);
void *SCNMatrix4ToGLKMatrix4(void);
void *SCNNodeGetBoundingSphere(void);
void *SCNNodesToMDLAsset(void);
void *SCNResolveImageContents(void);
void *SCNSetExternalTextureSource(void);
void *SCNSetPerformanceStatisticsEnabled(void);
void *SCNSetResourceBundle(void);
void *SCNSetShaderCollectionEnabled(void);
void *SCNTextureContentWithGLInfo(void);
void *SCNVector3EqualToVector3(void);
void *SCNVector4EqualToVector4(void);
void *SCNWriteMTLTextureToFile(void);
void *_SCNGetCollectedShadersForPid(void);
void *_SCNGetPerformanceStatisticsForPid(void);
void *_SCNSetLinearRenderingEnabled(void);
void *_SCNSetShaderCacheURL(void);
void *_SCNSetShouldPostShadableCompilationNotifications(void);
void *_SCNShadableConvertShaderToMetalIfNeeded(void);
void *_SCNStartCollectingPerformanceStatisticsForPid(void);
void *_SCNStopCollectingPerformanceStatisticsForPid(void);
void *scn_frustum_classify_aabb(void);
void *scn_matrix_compute_normal(void);
void *scn_matrix_fast_invert(void);
void *scn_plane_intersect_3_planes(void);
void *scn_triangle_barycentric_coordinates(void);
void *scn_triangle_point_distance_squared(void);


extern void* const SCNAnimationCommitOnCompletion;
extern void* const SCNBindingValueTransformerKey;
extern void* const SCNCaptureDeviceOutputConsumerOptionContainsAlpha;
extern void* const SCNConsistencyElementIDErrorKey;
extern void* const SCNConsistencyElementTypeErrorKey;
extern void* const SCNConsistencyLineNumberErrorKey;
extern void* const SCNDataProviderAnimationTime;
extern void* const SCNDataProviderCAWaitTime;
extern void* const SCNDataProviderCPUIdleTime;
extern void* const SCNDataProviderClientUpdateTime;
extern void* const SCNDataProviderConstraintsTime;
extern void* const SCNDataProviderFPS;
extern void* const SCNDataProviderFrameTime;
extern void* const SCNDataProviderGPUDeviceUtil;
extern void* const SCNDataProviderGPURendererUtil;
extern void* const SCNDataProviderGPUTilerUtil;
extern void* const SCNDataProviderParticleTime;
extern void* const SCNDataProviderPhysicsTime;
extern void* const SCNDataProviderPrimitivesProcessed;
extern void* const SCNDataProviderRenderTime;
extern void* const SCNDetailedErrorsKey;
extern void* const SCNErrorDomain;
extern void* const SCNExportPointOfViewAttribute;
extern void* const SCNExportSceneIdentifier;
extern void* const SCNFreeViewCameraName;
extern void* const SCNGeometrySourceSemanticBoneIndices;
extern void* const SCNGeometrySourceSemanticBoneWeights;
extern void* const SCNGeometrySourceSemanticColor;
extern void* const SCNGeometrySourceSemanticEdgeCrease;
extern void* const SCNGeometrySourceSemanticNormal;
extern void* const SCNGeometrySourceSemanticTangent;
extern void* const SCNGeometrySourceSemanticTexcoord;
extern void* const SCNGeometrySourceSemanticVertex;
extern void* const SCNGeometrySourceSemanticVertexCrease;
extern void* const SCNHitTestAllLayers;
extern void* const SCNHitTestBackFaceCullingKey;
extern void* const SCNHitTestBoundingBoxOnlyKey;
extern void* const SCNHitTestCategoryBitMaskKey;
extern void* const SCNHitTestClipToZRangeKey;
extern void* const SCNHitTestFirstFoundOnlyKey;
extern void* const SCNHitTestIgnoreChildNodesKey;
extern void* const SCNHitTestIgnoreHiddenNodesKey;
extern void* const SCNHitTestIgnoreTransparentNodesKey;
extern void* const SCNHitTestLayerMaskKey;
extern void* const SCNHitTestOptionCategoryBitMask;
extern void* const SCNHitTestOptionIgnoreLightArea;
extern void* const SCNHitTestOptionSearchMode;
extern void* const SCNHitTestOptionShowDebugInfo;
extern void* const SCNHitTestRootNodeKey;
extern void* const SCNHitTestSearchModeKey;
extern void* const SCNHitTestSortResultsKey;
extern void* const SCNLightAttenuationEndKey;
extern void* const SCNLightAttenuationFalloffExponentKey;
extern void* const SCNLightAttenuationStartKey;
extern void* const SCNLightConstantAttenuationKey;
extern void* const SCNLightFallOffAngleKey;
extern void* const SCNLightFallOffExponentKey;
extern void* const SCNLightGoboProjectShadows;
extern void* const SCNLightHotspotBeamKey;
extern void* const SCNLightLinearAttenuationKey;
extern void* const SCNLightQuadraticAttenuationKey;
extern void* const SCNLightShadowFarClippingKey;
extern void* const SCNLightShadowNearClippingKey;
extern void* const SCNLightSpotFalloffExponentKey;
extern void* const SCNLightSpotInnerAngleKey;
extern void* const SCNLightSpotOuterAngleKey;
extern void* const SCNLightTypeAmbient;
extern void* const SCNLightTypeArea;
extern void* const SCNLightTypeDirectional;
extern void* const SCNLightTypeIES;
extern void* const SCNLightTypeOmni;
extern void* const SCNLightTypeProbe;
extern void* const SCNLightTypeSpot;
extern void* const SCNLightingModelBlinn;
extern void* const SCNLightingModelConstant;
extern void* const SCNLightingModelLambert;
extern void* const SCNLightingModelNone;
extern void* const SCNLightingModelPhong;
extern void* const SCNLightingModelPhysicallyBased;
extern void* const SCNLightingModelShadowOnly;
extern void* const SCNMatrix4Identity;
extern void* const SCNModelTransform;
extern void* const SCNModelViewProjectionTransform;
extern void* const SCNModelViewTransform;
extern void* const SCNNormalTransform;
extern void* const SCNParticlePropertyAngle;
extern void* const SCNParticlePropertyAngularVelocity;
extern void* const SCNParticlePropertyBounce;
extern void* const SCNParticlePropertyCharge;
extern void* const SCNParticlePropertyColor;
extern void* const SCNParticlePropertyContactNormal;
extern void* const SCNParticlePropertyContactPoint;
extern void* const SCNParticlePropertyFrame;
extern void* const SCNParticlePropertyFrameRate;
extern void* const SCNParticlePropertyFriction;
extern void* const SCNParticlePropertyLife;
extern void* const SCNParticlePropertyOpacity;
extern void* const SCNParticlePropertyPosition;
extern void* const SCNParticlePropertyRotationAxis;
extern void* const SCNParticlePropertySize;
extern void* const SCNParticlePropertyVelocity;
extern void* const SCNPhysicsShapeCollisionMarginKey;
extern void* const SCNPhysicsShapeKeepAsCompoundKey;
extern void* const SCNPhysicsShapeOptionCollisionMargin;
extern void* const SCNPhysicsShapeScaleKey;
extern void* const SCNPhysicsShapeTypeBoundingBox;
extern void* const SCNPhysicsShapeTypeConcavePolyhedron;
extern void* const SCNPhysicsShapeTypeConvexHull;
extern void* const SCNPhysicsShapeTypeKey;
extern void* const SCNPhysicsTestBackfaceCullingKey;
extern void* const SCNPhysicsTestCollisionBitMaskKey;
extern void* const SCNPhysicsTestSearchModeAll;
extern void* const SCNPhysicsTestSearchModeAny;
extern void* const SCNPhysicsTestSearchModeClosest;
extern void* const SCNPhysicsTestSearchModeKey;
extern void* const SCNPreferLowPowerDeviceKey;
extern void* const SCNPreferredDeviceKey;
extern void* const SCNPreferredRenderingAPIKey;
extern void* const SCNPrimitiveCapSegments;
extern void* const SCNPrimitiveChamferSegments;
extern void* const SCNPrimitiveGenerateMappingCoordinates;
extern void* const SCNPrimitiveHeightSegments;
extern void* const SCNPrimitiveLengthSegments;
extern void* const SCNPrimitivePipeSegments;
extern void* const SCNPrimitiveRingSegments;
extern void* const SCNPrimitiveRotationSegments;
extern void* const SCNPrimitiveSphereSegments;
extern void* const SCNPrimitiveSphereType;
extern void* const SCNPrimitiveTypeGeosphere;
extern void* const SCNPrimitiveTypeSphere;
extern void* const SCNPrimitiveVertexTransformation;
extern void* const SCNPrimitiveWidthSegments;
extern void* const SCNProgramMappingChannelKey;
extern void* const SCNProjectionTransform;
extern void* const SCNRenderPassName;
extern void* const SCNRendererOffscreenSampleCount;
extern void* const SCNSceneEndTimeAttributeKey;
extern void* const SCNSceneExportCompressGeometryElements;
extern void* const SCNSceneExportCompressMorphTargets;
extern void* const SCNSceneExportDestinationURL;
extern void* const SCNSceneExportEmbedImages;
extern void* const SCNSceneExportEmbedReferences;
extern void* const SCNSceneExportPresentationTree;
extern void* const SCNSceneExportUTIKey;
extern void* const SCNSceneFrameRateAttributeKey;
extern void* SCNSceneKitVersionNumber;
extern void* const SCNSceneSourceAdjustInvalidCameraClippingPlanesKey;
extern void* const SCNSceneSourceAnimationImportPolicyDoNotPlay;
extern void* const SCNSceneSourceAnimationImportPolicyKey;
extern void* const SCNSceneSourceAnimationImportPolicyPlay;
extern void* const SCNSceneSourceAnimationImportPolicyPlayRepeatedly;
extern void* const SCNSceneSourceAnimationImportPolicyPlayUsingSceneTimeBase;
extern void* const SCNSceneSourceAnimationLoadingModeKey;
extern void* const SCNSceneSourceAnimationLoadingModePlayUsingSceneTime;
extern void* const SCNSceneSourceAssetAuthorKey;
extern void* const SCNSceneSourceAssetAuthoringToolKey;
extern void* const SCNSceneSourceAssetContributorsKey;
extern void* const SCNSceneSourceAssetCreatedDateKey;
extern void* const SCNSceneSourceAssetDirectoryURLsKey;
extern void* const SCNSceneSourceAssetModifiedDateKey;
extern void* const SCNSceneSourceAssetUnitKey;
extern void* const SCNSceneSourceAssetUnitMeterKey;
extern void* const SCNSceneSourceAssetUnitNameKey;
extern void* const SCNSceneSourceAssetUpAxisKey;
extern void* const SCNSceneSourceCheckConsistencyKey;
extern void* const SCNSceneSourceConvertToYUpIfNeededKey;
extern void* const SCNSceneSourceConvertToYUpKey;
extern void* const SCNSceneSourceConvertUnitsToMetersKey;
extern void* const SCNSceneSourceCreateCameraIfAbsentKey;
extern void* const SCNSceneSourceCreateLightIfAbsentKey;
extern void* const SCNSceneSourceCreateNormalsIfAbsentKey;
extern void* const SCNSceneSourceDocumentDestinationURL;
extern void* const SCNSceneSourceFlattenSceneKey;
extern void* const SCNSceneSourceForceInProcess;
extern void* const SCNSceneSourceForceMorphNormals;
extern void* const SCNSceneSourceFormat;
extern void* const SCNSceneSourceFormatC3D;
extern void* const SCNSceneSourceFormatCOLLADA;
extern void* const SCNSceneSourceFormatHint;
extern void* const SCNSceneSourceFormatOBJ;
extern void* const SCNSceneSourceFormatSCN;
extern void* const SCNSceneSourceGeometryMemoryUsageKey;
extern void* const SCNSceneSourceInputSource;
extern void* const SCNSceneSourceLoadTexturesInCPUMemory;
extern void* const SCNSceneSourceLoadingOptionPreserveOriginalTopology;
extern void* const SCNSceneSourceOverrideAssetURLsKey;
extern void* const SCNSceneSourceSceneIndexKey;
extern void* const SCNSceneSourceShouldCacheKey;
extern void* const SCNSceneSourceSkipConsistencyCheckKey;
extern void* const SCNSceneSourceStrictConformanceKey;
extern void* const SCNSceneSourceTextureMemoryUsageKey;
extern void* const SCNSceneSourceUseModelIOToLoadDAEFiles;
extern void* const SCNSceneSourceUseSafeModeKey;
extern void* const SCNSceneSourceVerboseMode;
extern void* const SCNSceneStartTimeAttributeKey;
extern void* const SCNSceneUpAxisAttributeKey;
extern void* const SCNSceneWriteToURLShouldKeepReferencesToFilesOption;
extern void* const SCNSceneWriteToURLShouldZipOption;
extern void* const SCNShaderModifierEntryPointFragment;
extern void* const SCNShaderModifierEntryPointGeometry;
extern void* const SCNShaderModifierEntryPointLightingModel;
extern void* const SCNShaderModifierEntryPointSurface;
extern void* const SCNTextureOffsetXKey;
extern void* const SCNTextureOffsetYKey;
extern void* const SCNVector3Zero;
extern void* const SCNVector4Zero;
extern void* const SCNViewTransform;
extern void* const _SCNShadableDidCompileNotificationName;
extern void* const _SCNShadableIssuesKey;
extern void* const kC3DGeometryKey;
extern void* const kC3DIOSceneSourceAutoAdjustInvalidZRange;
extern void* const kC3DIOSceneSourceAutoLimitMemoryForImages;
extern void* const kC3DIOSceneSourceCheckConsistency;
extern void* const kC3DIOSceneSourceCleanupMeshes;
extern void* const kC3DIOSceneSourceFlattenScene;
extern void* const kC3DIOSceneSourceInterleaveSources;
extern void* const kC3DIOSceneSourceMakeSourcesCheaper;
extern void* const kC3DIOSceneSourceProcessIsSandboxed;
extern void* const kC3DIOSceneSourceRemoveAllLights;
extern void* const kC3DIOSceneSourceRemoveColorArrays;
extern void* const kC3DIOSceneSourceRemoveConstantAnimations;
extern void* const kC3DIOSceneSourceSkipImagePathResolution;
extern void* const kC3DIOSceneSourceSplitMeshesForGLES;
extern void* const kC3DIOSceneSourceTriggerOptionsForRealtimeViewer;
extern void* const kC3DIOSceneSourceUseSafeMode;
extern void* const kC3DImageInitFromPath;
extern void* const kC3DImageLocation;
extern void* const kC3DImageRelativePath;
extern void* const kC3DSceneDidUpdateNotification;
extern void* const kC3DTriggerOptionsForRealtimeViewer;
extern void* const kSCNAuthoringLayerIndex;
extern void* const kSCNAuthoringLayerMask;
extern void* const kSCNAuthoringOverlayLayerIndex;
extern void* const kSCNAuthoringOverlayLayerMask;

#endif

