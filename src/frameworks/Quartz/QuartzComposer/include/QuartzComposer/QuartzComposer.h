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

#ifndef _QUARTZCOMPOSER_H_
#define _QUARTZCOMPOSER_H_

#import <Foundation/Foundation.h>

#import <QuartzComposer/CAAction.h>
#import <QuartzComposer/CAAnimation+SceneKitAdditions.h>
#import <QuartzComposer/GFBase.h>
#import <QuartzComposer/GFConnection.h>
#import <QuartzComposer/GFGraph.h>
#import <QuartzComposer/GFGraphBrowserView.h>
#import <QuartzComposer/GFGraphEditorInspector.h>
#import <QuartzComposer/GFGraphEditorView.h>
#import <QuartzComposer/GFGraphPathView.h>
#import <QuartzComposer/GFGraphView.h>
#import <QuartzComposer/GFInspectorWindow.h>
#import <QuartzComposer/GFList.h>
#import <QuartzComposer/GFNode.h>
#import <QuartzComposer/GFNodeActor.h>
#import <QuartzComposer/GFNodeClassDescription.h>
#import <QuartzComposer/GFNodeClassDescriptionRegistry.h>
#import <QuartzComposer/GFNodeInspectorViewsController.h>
#import <QuartzComposer/GFNodeManager.h>
#import <QuartzComposer/GFNodeManagerView.h>
#import <QuartzComposer/GFPlugInRegistration.h>
#import <QuartzComposer/GFPort.h>
#import <QuartzComposer/GFSettingsColumn.h>
#import <QuartzComposer/GFSettingsView.h>
#import <QuartzComposer/GFStringEditor.h>
#import <QuartzComposer/GFStringEditorWindow.h>
#import <QuartzComposer/GFTooltipView.h>
#import <QuartzComposer/GFTooltipWindow.h>
#import <QuartzComposer/GLSLParameterInfo.h>
#import <QuartzComposer/NSColor+QCSCN_Additions.h>
#import <QuartzComposer/NSFastEnumeration.h>
#import <QuartzComposer/NSImage+QCSCN_Addition.h>
#import <QuartzComposer/NSImageRep+QCSCN_Additions.h>
#import <QuartzComposer/NSMutableDictionary+GFPrivateExtensions.h>
#import <QuartzComposer/NSObject+GFSettingsViewDelegate.h>
#import <QuartzComposer/NSOpenGLContext_QuartzComposer.h>
#import <QuartzComposer/NSString+QCTextRenderer.h>
#import <QuartzComposer/NSTextDelegate.h>
#import <QuartzComposer/NSTextView+GFLogMessage.h>
#import <QuartzComposer/NSTextViewDelegate.h>
#import <QuartzComposer/NSURLConnectionDelegate.h>
#import <QuartzComposer/NSValue+Core3DKitAdditions.h>
#import <QuartzComposer/QCAgeProfileView.h>
#import <QuartzComposer/QCAnchorPosition.h>
#import <QuartzComposer/QCAppleRemote.h>
#import <QuartzComposer/QCArray.h>
#import <QuartzComposer/QCArrayBufferObject.h>
#import <QuartzComposer/QCArrayMemObject.h>
#import <QuartzComposer/QCArrayTexture3D.h>
#import <QuartzComposer/QCBasicImageBufferConverter.h>
#import <QuartzComposer/QCBillboard.h>
#import <QuartzComposer/QCBillboardUI.h>
#import <QuartzComposer/QCBonjourServices.h>
#import <QuartzComposer/QCBonjourServicesUI.h>
#import <QuartzComposer/QCBooleanPort.h>
#import <QuartzComposer/QCCGLContext.h>
#import <QuartzComposer/QCCGLRootContext.h>
#import <QuartzComposer/QCCLContext.h>
#import <QuartzComposer/QCCLProgram.h>
#import <QuartzComposer/QCCache.h>
#import <QuartzComposer/QCCache.h>
#import <QuartzComposer/QCCamera.h>
#import <QuartzComposer/QCCirclePatchActor.h>
#import <QuartzComposer/QCCircularSlider.h>
#import <QuartzComposer/QCClear.h>
#import <QuartzComposer/QCClearUI.h>
#import <QuartzComposer/QCClock.h>
#import <QuartzComposer/QCColorCell.h>
#import <QuartzComposer/QCColorFromComponents.h>
#import <QuartzComposer/QCColorMask.h>
#import <QuartzComposer/QCColorMixer.h>
#import <QuartzComposer/QCColorPort.h>
#import <QuartzComposer/QCColorToComponents.h>
#import <QuartzComposer/QCColorTransform.h>
#import <QuartzComposer/QCComposition.h>
#import <QuartzComposer/QCCompositionCellLayer.h>
#import <QuartzComposer/QCCompositionEditor.h>
#import <QuartzComposer/QCCompositionLayer.h>
#import <QuartzComposer/QCCompositionLayer_QCViewBacking.h>
#import <QuartzComposer/QCCompositionParameterView.h>
#import <QuartzComposer/QCCompositionPickerController.h>
#import <QuartzComposer/QCCompositionPickerGridCellLayer.h>
#import <QuartzComposer/QCCompositionPickerGridLayer.h>
#import <QuartzComposer/QCCompositionPickerInfos.h>
#import <QuartzComposer/QCCompositionPickerLayer.h>
#import <QuartzComposer/QCCompositionPickerNavigationView.h>
#import <QuartzComposer/QCCompositionPickerPanel.h>
#import <QuartzComposer/QCCompositionPickerView.h>
#import <QuartzComposer/QCCompositionRenderer.h>
#import <QuartzComposer/QCCompositionRepository.h>
#import <QuartzComposer/QCCompositionSortDescriptor.h>
#import <QuartzComposer/QCConditional.h>
#import <QuartzComposer/QCContact.h>
#import <QuartzComposer/QCContext.h>
#import <QuartzComposer/QCContextProExtension.h>
#import <QuartzComposer/QCConverter_CoreGraphics.h>
#import <QuartzComposer/QCConverter_CoreImage.h>
#import <QuartzComposer/QCConverter_GLImageLib.h>
#import <QuartzComposer/QCConverter_OpenGL.h>
#import <QuartzComposer/QCConverter_XRGB.h>
#import <QuartzComposer/QCConverter_YUV422.h>
#import <QuartzComposer/QCCore3DLoader.h>
#import <QuartzComposer/QCCore3DLoaderUI.h>
#import <QuartzComposer/QCCounter.h>
#import <QuartzComposer/QCCroppedImageProvider.h>
#import <QuartzComposer/QCCroppedProvider_CoreGraphics.h>
#import <QuartzComposer/QCCroppedProvider_CoreText.h>
#import <QuartzComposer/QCCube.h>
#import <QuartzComposer/QCDashboardButton.h>
#import <QuartzComposer/QCDataImage.h>
#import <QuartzComposer/QCDatagramBroadcaster.h>
#import <QuartzComposer/QCDatagramReceiver.h>
#import <QuartzComposer/QCDatagramSocket.h>
#import <QuartzComposer/QCDateFormatter.h>
#import <QuartzComposer/QCDateFormatterUI.h>
#import <QuartzComposer/QCDemultiplexer.h>
#import <QuartzComposer/QCDemultiplexerUI.h>
#import <QuartzComposer/QCDepthBlurEffectDetector.h>
#import <QuartzComposer/QCDepthBlurEffectEditor.h>
#import <QuartzComposer/QCDepthBlurEffectImage.h>
#import <QuartzComposer/QCDepthBlurEffectParameters.h>
#import <QuartzComposer/QCDetector.h>
#import <QuartzComposer/QCDifferentialValue.h>
#import <QuartzComposer/QCDirectoryScanner.h>
#import <QuartzComposer/QCDirectoryScannerUI.h>
#import <QuartzComposer/QCDownloadPatch.h>
#import <QuartzComposer/QCEventManager.h>
#import <QuartzComposer/QCExporter_CoreAnimation.h>
#import <QuartzComposer/QCExporter_CoreGraphics.h>
#import <QuartzComposer/QCExporter_CoreImage.h>
#import <QuartzComposer/QCExporter_CoreImage_Provider.h>
#import <QuartzComposer/QCExporter_CoreVideo.h>
#import <QuartzComposer/QCExporter_NSImage.h>
#import <QuartzComposer/QCExporter_OpenCL.h>
#import <QuartzComposer/QCExpression.h>
#import <QuartzComposer/QCExpressionUI.h>
#import <QuartzComposer/QCFPSDisplay.h>
#import <QuartzComposer/QCFakePatch.h>
#import <QuartzComposer/QCFilter.h>
#import <QuartzComposer/QCFlameImage.h>
#import <QuartzComposer/QCFlameImageUI.h>
#import <QuartzComposer/QCFog.h>
#import <QuartzComposer/QCFullScreenWindow.h>
#import <QuartzComposer/QCGLSLShader.h>
#import <QuartzComposer/QCGLSLShaderUI.h>
#import <QuartzComposer/QCGradient.h>
#import <QuartzComposer/QCGraphicsContext.h>
#import <QuartzComposer/QCGraphicsRenderState.h>
#import <QuartzComposer/QCGrid.h>
#import <QuartzComposer/QCHIDInput.h>
#import <QuartzComposer/QCHIDInputUI.h>
#import <QuartzComposer/QCHostInfo.h>
#import <QuartzComposer/QCHotZone.h>
#import <QuartzComposer/QCImage.h>
#import <QuartzComposer/QCImage2DMemObject.h>
#import <QuartzComposer/QCImage3DMemObject.h>
#import <QuartzComposer/QCImageAccumulator.h>
#import <QuartzComposer/QCImageAccumulatorUI.h>
#import <QuartzComposer/QCImageBuffer.h>
#import <QuartzComposer/QCImageBufferConverter.h>
#import <QuartzComposer/QCImageBufferConverter.h>
#import <QuartzComposer/QCImageBufferProvider.h>
#import <QuartzComposer/QCImageBuffer_GCCacheResource.h>
#import <QuartzComposer/QCImageColorMatchingProperties.h>
#import <QuartzComposer/QCImageConversionSequence.h>
#import <QuartzComposer/QCImageCrop.h>
#import <QuartzComposer/QCImageDimensions.h>
#import <QuartzComposer/QCImageDownloader.h>
#import <QuartzComposer/QCImageDownloaderUI.h>
#import <QuartzComposer/QCImageExporter.h>
#import <QuartzComposer/QCImageExporter.h>
#import <QuartzComposer/QCImageFilter.h>
#import <QuartzComposer/QCImageFocusRect.h>
#import <QuartzComposer/QCImageHistogram.h>
#import <QuartzComposer/QCImageInfo.h>
#import <QuartzComposer/QCImageKernel.h>
#import <QuartzComposer/QCImageKernelUI.h>
#import <QuartzComposer/QCImageLoader.h>
#import <QuartzComposer/QCImageLoaderUI.h>
#import <QuartzComposer/QCImageLoaderView.h>
#import <QuartzComposer/QCImageManager.h>
#import <QuartzComposer/QCImageManagerAgeProfileView.h>
#import <QuartzComposer/QCImageManagerProfileView.h>
#import <QuartzComposer/QCImageMetadata.h>
#import <QuartzComposer/QCImageOrigin.h>
#import <QuartzComposer/QCImagePixel.h>
#import <QuartzComposer/QCImagePixelBuffer.h>
#import <QuartzComposer/QCImagePort.h>
#import <QuartzComposer/QCImagePort_Cache.h>
#import <QuartzComposer/QCImageProperties.h>
#import <QuartzComposer/QCImageProvider.h>
#import <QuartzComposer/QCImageProvider.h>
#import <QuartzComposer/QCImageResize.h>
#import <QuartzComposer/QCImageSurfaceBuffer.h>
#import <QuartzComposer/QCImageTextureBuffer.h>
#import <QuartzComposer/QCImageTextureBuffer_CVOpenGLTexture.h>
#import <QuartzComposer/QCImageTexturingProperties.h>
#import <QuartzComposer/QCImageTransform.h>
#import <QuartzComposer/QCImageView.h>
#import <QuartzComposer/QCIndexMenuValueTransformer.h>
#import <QuartzComposer/QCIndexPort.h>
#import <QuartzComposer/QCInfo.h>
#import <QuartzComposer/QCInspector.h>
#import <QuartzComposer/QCInteractionManager.h>
#import <QuartzComposer/QCInteractionPatch.h>
#import <QuartzComposer/QCInteractionPatchActor.h>
#import <QuartzComposer/QCInteractionPort.h>
#import <QuartzComposer/QCInterpolation.h>
#import <QuartzComposer/QCInterpolationPort.h>
#import <QuartzComposer/QCInterpolationUI.h>
#import <QuartzComposer/QCInterpolationView.h>
#import <QuartzComposer/QCIterator.h>
#import <QuartzComposer/QCIteratorVariables.h>
#import <QuartzComposer/QCKeyFrameEditor.h>
#import <QuartzComposer/QCKeyFrameEditorView.h>
#import <QuartzComposer/QCKeyFrameEditorWindow.h>
#import <QuartzComposer/QCKeyFrameList.h>
#import <QuartzComposer/QCKeyboard.h>
#import <QuartzComposer/QCKeyboardTableView.h>
#import <QuartzComposer/QCKeyboardUI.h>
#import <QuartzComposer/QCLFO.h>
#import <QuartzComposer/QCLighting.h>
#import <QuartzComposer/QCLine.h>
#import <QuartzComposer/QCLines.h>
#import <QuartzComposer/QCLink.h>
#import <QuartzComposer/QCList.h>
#import <QuartzComposer/QCLog.h>
#import <QuartzComposer/QCLogView.h>
#import <QuartzComposer/QCLogic.h>
#import <QuartzComposer/QCMath.h>
#import <QuartzComposer/QCMathematicalExpression.h>
#import <QuartzComposer/QCMemObject.h>
#import <QuartzComposer/QCMesh.h>
#import <QuartzComposer/QCMeshCreator.h>
#import <QuartzComposer/QCMeshMaterialGetter.h>
#import <QuartzComposer/QCMeshMaterialSetter.h>
#import <QuartzComposer/QCMeshPort.h>
#import <QuartzComposer/QCMeshPropertyGetter.h>
#import <QuartzComposer/QCMeshPropertySetter.h>
#import <QuartzComposer/QCMeshRenderer.h>
#import <QuartzComposer/QCMeshTransform.h>
#import <QuartzComposer/QCMiniPatchActor.h>
#import <QuartzComposer/QCMouse.h>
#import <QuartzComposer/QCMouseInteraction.h>
#import <QuartzComposer/QCMultiplexer.h>
#import <QuartzComposer/QCMultiplexerBooleanPort.h>
#import <QuartzComposer/QCMultiplexerColorPort.h>
#import <QuartzComposer/QCMultiplexerImagePort.h>
#import <QuartzComposer/QCMultiplexerIndexPort.h>
#import <QuartzComposer/QCMultiplexerMeshPort.h>
#import <QuartzComposer/QCMultiplexerNumberPort.h>
#import <QuartzComposer/QCMultiplexerStringPort.h>
#import <QuartzComposer/QCMultiplexerStructurePort.h>
#import <QuartzComposer/QCMultiplexerUI.h>
#import <QuartzComposer/QCMultiplexerVirtualPort.h>
#import <QuartzComposer/QCNSBitmapImageRep.h>
#import <QuartzComposer/QCNetworkBroadcaster.h>
#import <QuartzComposer/QCNetworkBroadcasterUI.h>
#import <QuartzComposer/QCNetworkPatch.h>
#import <QuartzComposer/QCNetworkPatchUI.h>
#import <QuartzComposer/QCNetworkReceiver.h>
#import <QuartzComposer/QCNetworkReceiverUI.h>
#import <QuartzComposer/QCNetworkSynchronizer.h>
#import <QuartzComposer/QCNetworkSynchronizerUI.h>
#import <QuartzComposer/QCNodeManager.h>
#import <QuartzComposer/QCNoise.h>
#import <QuartzComposer/QCNumberFormatter.h>
#import <QuartzComposer/QCNumberFormatterUI.h>
#import <QuartzComposer/QCNumberPort.h>
#import <QuartzComposer/QCNumericalFormatter.h>
#import <QuartzComposer/QCOSCBroadcaster.h>
#import <QuartzComposer/QCOSCBroadcasterUI.h>
#import <QuartzComposer/QCOSCPatch.h>
#import <QuartzComposer/QCOSCPatchUI.h>
#import <QuartzComposer/QCOSCReceiver.h>
#import <QuartzComposer/QCOSCReceiverUI.h>
#import <QuartzComposer/QCObject.h>
#import <QuartzComposer/QCObjectPort.h>
#import <QuartzComposer/QCOpenCL.h>
#import <QuartzComposer/QCOpenCLConverter.h>
#import <QuartzComposer/QCOpenCLInfo.h>
#import <QuartzComposer/QCOpenCLUI.h>
#import <QuartzComposer/QCOpenGLContext.h>
#import <QuartzComposer/QCOpenGLInfo.h>
#import <QuartzComposer/QCOpenGLInfoUI.h>
#import <QuartzComposer/QCOpenGLPort.h>
#import <QuartzComposer/QCOpenGLPort_Blending.h>
#import <QuartzComposer/QCOpenGLPort_Color.h>
#import <QuartzComposer/QCOpenGLPort_Culling.h>
#import <QuartzComposer/QCOpenGLPort_Image.h>
#import <QuartzComposer/QCOpenGLPort_Mesh.h>
#import <QuartzComposer/QCOpenGLPort_SceneKitBlending.h>
#import <QuartzComposer/QCOpenGLPort_SceneKitCulling.h>
#import <QuartzComposer/QCOpenGLPort_ZBuffer.h>
#import <QuartzComposer/QCOpenGLRenderState.h>
#import <QuartzComposer/QCParameterInfo.h>
#import <QuartzComposer/QCParticleSystem.h>
#import <QuartzComposer/QCParticleSystemUI.h>
#import <QuartzComposer/QCPatch.h>
#import <QuartzComposer/QCPatchActor.h>
#import <QuartzComposer/QCPatchBrowserView.h>
#import <QuartzComposer/QCPatchClassDescription.h>
#import <QuartzComposer/QCPatchController.h>
#import <QuartzComposer/QCPatchEditorView.h>
#import <QuartzComposer/QCPatchInfo.h>
#import <QuartzComposer/QCPatchManagerView.h>
#import <QuartzComposer/QCPatchParameterView.h>
#import <QuartzComposer/QCPatchParametersView.h>
#import <QuartzComposer/QCPatchPathView.h>
#import <QuartzComposer/QCPatchProExtension.h>
#import <QuartzComposer/QCPatchProfileView.h>
#import <QuartzComposer/QCPatchRenderer.h>
#import <QuartzComposer/QCPatchRendererUI.h>
#import <QuartzComposer/QCPatchRenderingInfo.h>
#import <QuartzComposer/QCPatchView.h>
#import <QuartzComposer/QCPhysics.h>
#import <QuartzComposer/QCPhysicsEffect.h>
#import <QuartzComposer/QCPickerSelectionLayer.h>
#import <QuartzComposer/QCPixelFormat.h>
#import <QuartzComposer/QCPixelFormat_422YpCbCr8_601.h>
#import <QuartzComposer/QCPixelFormat_422YpCbCr8_709.h>
#import <QuartzComposer/QCPixelFormat_ARGB8.h>
#import <QuartzComposer/QCPixelFormat_BGRA8.h>
#import <QuartzComposer/QCPixelFormat_BGRX8.h>
#import <QuartzComposer/QCPixelFormat_CMYK16.h>
#import <QuartzComposer/QCPixelFormat_CMYK8.h>
#import <QuartzComposer/QCPixelFormat_CMYKf.h>
#import <QuartzComposer/QCPixelFormat_I16.h>
#import <QuartzComposer/QCPixelFormat_I8.h>
#import <QuartzComposer/QCPixelFormat_If.h>
#import <QuartzComposer/QCPixelFormat_Ih.h>
#import <QuartzComposer/QCPixelFormat_KYMC8.h>
#import <QuartzComposer/QCPixelFormat_M_I16.h>
#import <QuartzComposer/QCPixelFormat_M_I8.h>
#import <QuartzComposer/QCPixelFormat_M_If.h>
#import <QuartzComposer/QCPixelFormat_M_Ih.h>
#import <QuartzComposer/QCPixelFormat_RGB16.h>
#import <QuartzComposer/QCPixelFormat_RGB8.h>
#import <QuartzComposer/QCPixelFormat_RGBA16.h>
#import <QuartzComposer/QCPixelFormat_RGBA8.h>
#import <QuartzComposer/QCPixelFormat_RGBAf.h>
#import <QuartzComposer/QCPixelFormat_RGBAh.h>
#import <QuartzComposer/QCPixelFormat_RGBX8.h>
#import <QuartzComposer/QCPixelFormat_RGBf.h>
#import <QuartzComposer/QCPixelFormat_RGBh.h>
#import <QuartzComposer/QCPixelFormat_XRGB8.h>
#import <QuartzComposer/QCPixelFormat_YUVS.h>
#import <QuartzComposer/QCPlasmaImage.h>
#import <QuartzComposer/QCPlasmaImageUI.h>
#import <QuartzComposer/QCPlugIn.h>
#import <QuartzComposer/QCPlugInContext.h>
#import <QuartzComposer/QCPlugInInputImage.h>
#import <QuartzComposer/QCPlugInInputImageSource.h>
#import <QuartzComposer/QCPlugInInspector.h>
#import <QuartzComposer/QCPlugInOutputImageProvider.h>
#import <QuartzComposer/QCPlugInPatch.h>
#import <QuartzComposer/QCPlugInViewController.h>
#import <QuartzComposer/QCPlugOutputImage.h>
#import <QuartzComposer/QCPort.h>
#import <QuartzComposer/QCPortGLView.h>
#import <QuartzComposer/QCPortTooltipView.h>
#import <QuartzComposer/QCPortView.h>
#import <QuartzComposer/QCProfileConfigurationView.h>
#import <QuartzComposer/QCProfileView.h>
#import <QuartzComposer/QCProgrammablePatch.h>
#import <QuartzComposer/QCProgrammablePatchView.h>
#import <QuartzComposer/QCProjection.h>
#import <QuartzComposer/QCProviderImageCache.h>
#import <QuartzComposer/QCProvider_CoreAnimation.h>
#import <QuartzComposer/QCProvider_CoreGraphics.h>
#import <QuartzComposer/QCProvider_CoreImage.h>
#import <QuartzComposer/QCProvider_CoreText.h>
#import <QuartzComposer/QCProvider_CoreVideo.h>
#import <QuartzComposer/QCProvider_OpenCL.h>
#import <QuartzComposer/QCProvider_SceneKit.h>
#import <QuartzComposer/QCProxyPort.h>
#import <QuartzComposer/QCPsychedelic.h>
#import <QuartzComposer/QCPsychedelicUI.h>
#import <QuartzComposer/QCPulse.h>
#import <QuartzComposer/QCQuadric.h>
#import <QuartzComposer/QCQuadricUI.h>
#import <QuartzComposer/QCQueue.h>
#import <QuartzComposer/QCRandom.h>
#import <QuartzComposer/QCRangeValue.h>
#import <QuartzComposer/QCRawFilter.h>
#import <QuartzComposer/QCRecursor.h>
#import <QuartzComposer/QCRegion.h>
#import <QuartzComposer/QCRenderInImage.h>
#import <QuartzComposer/QCRenderInImageUI.h>
#import <QuartzComposer/QCRenderState.h>
#import <QuartzComposer/QCRenderView.h>
#import <QuartzComposer/QCRenderer.h>
#import <QuartzComposer/QCRenderingManager.h>
#import <QuartzComposer/QCRenderingPatch.h>
#import <QuartzComposer/QCReplicator.h>
#import <QuartzComposer/QCRepositoryCompositions.h>
#import <QuartzComposer/QCResource.h>
#import <QuartzComposer/QCResourcePool.h>
#import <QuartzComposer/QCRoundValue.h>
#import <QuartzComposer/QCRunLoopPatch.h>
#import <QuartzComposer/QCSCN_Animatable.h>
#import <QuartzComposer/QCSCN_AnimationInfo.h>
#import <QuartzComposer/QCSCN_BackingLayer.h>
#import <QuartzComposer/QCSCN_BasicLayerAnimation.h>
#import <QuartzComposer/QCSCN_BoundingBox.h>
#import <QuartzComposer/QCSCN_BoundingSphere.h>
#import <QuartzComposer/QCSCN_BoundingVolume.h>
#import <QuartzComposer/QCSCN_CARendererRegistry.h>
#import <QuartzComposer/QCSCN_Camera.h>
#import <QuartzComposer/QCSCN_CameraControlEventHandler.h>
#import <QuartzComposer/QCSCN_CameraControlEventHandlerReserved.h>
#import <QuartzComposer/QCSCN_CameraReserved.h>
#import <QuartzComposer/QCSCN_DelegateSource.h>
#import <QuartzComposer/QCSCN_EdgeAntialiasingOptions.h>
#import <QuartzComposer/QCSCN_EdgeAntialiasingOptionsReserved.h>
#import <QuartzComposer/QCSCN_EventHandler.h>
#import <QuartzComposer/QCSCN_EventHandlerReserved.h>
#import <QuartzComposer/QCSCN_Geometry.h>
#import <QuartzComposer/QCSCN_GeometryElement.h>
#import <QuartzComposer/QCSCN_GeometryElementReserved.h>
#import <QuartzComposer/QCSCN_GeometryReserved.h>
#import <QuartzComposer/QCSCN_GeometrySource.h>
#import <QuartzComposer/QCSCN_GeometrySourceReserved.h>
#import <QuartzComposer/QCSCN_HitTestResult.h>
#import <QuartzComposer/QCSCN_HitTestResultReserved.h>
#import <QuartzComposer/QCSCN_ImageSource.h>
#import <QuartzComposer/QCSCN_Layer.h>
#import <QuartzComposer/QCSCN_LayerFixedPipeline.h>
#import <QuartzComposer/QCSCN_LayerReserved.h>
#import <QuartzComposer/QCSCN_LayerSource.h>
#import <QuartzComposer/QCSCN_Light.h>
#import <QuartzComposer/QCSCN_LightReserved.h>
#import <QuartzComposer/QCSCN_LoadingLayer.h>
#import <QuartzComposer/QCSCN_LoadingProgressIndicatorLayer.h>
#import <QuartzComposer/QCSCN_Material.h>
#import <QuartzComposer/QCSCN_MaterialAttachment.h>
#import <QuartzComposer/QCSCN_MaterialProperty.h>
#import <QuartzComposer/QCSCN_MaterialPropertyReserved.h>
#import <QuartzComposer/QCSCN_MaterialReserved.h>
#import <QuartzComposer/QCSCN_Morpher.h>
#import <QuartzComposer/QCSCN_MorpherReserved.h>
#import <QuartzComposer/QCSCN_Node.h>
#import <QuartzComposer/QCSCN_NodeReserved.h>
#import <QuartzComposer/QCSCN_Parameter.h>
#import <QuartzComposer/QCSCN_ParameterReserved.h>
#import <QuartzComposer/QCSCN_Program.h>
#import <QuartzComposer/QCSCN_ProgramReserved.h>
#import <QuartzComposer/QCSCN_Renderer.h>
#import <QuartzComposer/QCSCN_RendererCameraModifier.h>
#import <QuartzComposer/QCSCN_RendererReserved.h>
#import <QuartzComposer/QCSCN_Scene.h>
#import <QuartzComposer/QCSCN_SceneRenderer.h>
#import <QuartzComposer/QCSCN_SceneReserved.h>
#import <QuartzComposer/QCSCN_SceneSource.h>
#import <QuartzComposer/QCSCN_SceneSourceReserved.h>
#import <QuartzComposer/QCSCN_Skinner.h>
#import <QuartzComposer/QCSCN_TextureDelegateSource.h>
#import <QuartzComposer/QCSCN_TextureLayerSource.h>
#import <QuartzComposer/QCSCN_TextureOffscreenRenderingSource.h>
#import <QuartzComposer/QCSCN_TextureSource.h>
#import <QuartzComposer/QCSCN_Transaction.h>
#import <QuartzComposer/QCSCN_View.h>
#import <QuartzComposer/QCSCN_ViewReserved.h>
#import <QuartzComposer/QCSampleHold.h>
#import <QuartzComposer/QCScaledNumberPort.h>
#import <QuartzComposer/QCScreenInfo.h>
#import <QuartzComposer/QCScreenInfoUI.h>
#import <QuartzComposer/QCSetStructureValue.h>
#import <QuartzComposer/QCSharedPatchActor.h>
#import <QuartzComposer/QCSharedTimers.h>
#import <QuartzComposer/QCSignal.h>
#import <QuartzComposer/QCSlideShowController.h>
#import <QuartzComposer/QCSmooth.h>
#import <QuartzComposer/QCSourceCodeView.h>
#import <QuartzComposer/QCSourceRulerView.h>
#import <QuartzComposer/QCSourceTextView.h>
#import <QuartzComposer/QCSplitter.h>
#import <QuartzComposer/QCSplitterIndexPort.h>
#import <QuartzComposer/QCSplitterUI.h>
#import <QuartzComposer/QCSpotlight.h>
#import <QuartzComposer/QCSprite.h>
#import <QuartzComposer/QCStream.h>
#import <QuartzComposer/QCStreamExporter.h>
#import <QuartzComposer/QCStreamExporter.h>
#import <QuartzComposer/QCStreamExporter_QCArray.h>
#import <QuartzComposer/QCStreamManager.h>
#import <QuartzComposer/QCStreamProvider.h>
#import <QuartzComposer/QCStreamProvider.h>
#import <QuartzComposer/QCStreamProvider_Core3D.h>
#import <QuartzComposer/QCStreamProvider_QCArray.h>
#import <QuartzComposer/QCStreamProvider_QCMemObject.h>
#import <QuartzComposer/QCStringCase.h>
#import <QuartzComposer/QCStringCompare.h>
#import <QuartzComposer/QCStringComponents.h>
#import <QuartzComposer/QCStringFormat.h>
#import <QuartzComposer/QCStringLength.h>
#import <QuartzComposer/QCStringMD5.h>
#import <QuartzComposer/QCStringPort.h>
#import <QuartzComposer/QCStringRange.h>
#import <QuartzComposer/QCStringReplace.h>
#import <QuartzComposer/QCStringTimecode.h>
#import <QuartzComposer/QCStringTruncate.h>
#import <QuartzComposer/QCStructure.h>
#import <QuartzComposer/QCStructureArrayPort.h>
#import <QuartzComposer/QCStructureAsString.h>
#import <QuartzComposer/QCStructureCIVectorPort.h>
#import <QuartzComposer/QCStructureCount.h>
#import <QuartzComposer/QCStructureDictionaryPort.h>
#import <QuartzComposer/QCStructureMember.h>
#import <QuartzComposer/QCStructurePort.h>
#import <QuartzComposer/QCStructureRange.h>
#import <QuartzComposer/QCStructureReverse.h>
#import <QuartzComposer/QCStructureSort.h>
#import <QuartzComposer/QCSynchronousURLConnectionDelegate.h>
#import <QuartzComposer/QCSystemTime.h>
#import <QuartzComposer/QCTableView.h>
#import <QuartzComposer/QCTablet.h>
#import <QuartzComposer/QCTeapot.h>
#import <QuartzComposer/QCTextImage.h>
#import <QuartzComposer/QCTextImageUI.h>
#import <QuartzComposer/QCTextRenderer.h>
#import <QuartzComposer/QCTextView.h>
#import <QuartzComposer/QCThreadPatch.h>
#import <QuartzComposer/QCTime.h>
#import <QuartzComposer/QCTimeLine.h>
#import <QuartzComposer/QCTimeLineEditorView.h>
#import <QuartzComposer/QCTimeLineLegendColorWell.h>
#import <QuartzComposer/QCTimeLineLegendTextField.h>
#import <QuartzComposer/QCTimeLineUI.h>
#import <QuartzComposer/QCTimeLineView.h>
#import <QuartzComposer/QCTimeLoop.h>
#import <QuartzComposer/QCTrackBall.h>
#import <QuartzComposer/QCTrackBallUI.h>
#import <QuartzComposer/QCURLConnection.h>
#import <QuartzComposer/QCView.h>
#import <QuartzComposer/QCVirtualPort.h>
#import <QuartzComposer/QCWatcher.h>
#import <QuartzComposer/QCWatcherUI.h>
#import <QuartzComposer/QCXMLDownloader.h>
#import <QuartzComposer/QCZoomView.h>


void *GFArrayFromArgumentList(void);
void *GFBacktrace(void);
void *GFDebug(void);
void *GFDebuggingLevel(void);
void *GFException(void);
void *GFFilterStringsWithKeywords(void);
void *GFGetDebugMessageCallback(void);
void *GFGetLogMessageCallback(void);
void *GFKeywordsFromSearchString(void);
void *GFLog(void);
void *GFNameForNode(void);
void *GFNameForPort(void);
void *GFSetDebugMessageCallback(void);
void *GFSetLogMessageCallback(void);
void *GFThrowException(void);
void *QCAddCompositionTrackToMovie(void);
void *QCAddXMLElementsToStructure(void);
void *QCAppendCompositionSampleToTrack(void);
void *QCCompositionCompliesToProtocol(void);
void *QCCompositionFromData(void);
void *QCCompositionFromFile(void);
void *QCCompositionFromMovieFile(void);
void *QCCompositionFromPatch(void);
void *QCCompositionToData(void);
void *QCCompositionToFile(void);
void *QCCompositionToMovieFile(void);
void *QCCompositionsFromMovieFile(void);
void *QCComputeAspectRatioBounds(void);
void *QCComputeProportionalBounds(void);
void *QCCountCompositionTracksInMovie(void);
void *QCDataFromComposition(void);
void *QCFileIsComposition(void);
void *QCGLExtensionSupported(void);
void *QCGLMakeTransformationMatrix(void);
void *QCGetCompositionTrackAtIndex(void);
void *QCGetIndexedColorSpace(void);
void *QCHostTime(void);
void *QCInfoFromComposition(void);
void *QCMD5FromData(void);
void *QCMD5FromString(void);
void *QCMD5ToData(void);
void *QCMD5ToString(void);
void *QCMD5WithBytes(void);
void *QCMD5WithColorSpace(void);
void *QCMD5WithDoubles(void);
void *QCMD5WithIntegers(void);
void *QCMD5WithObject(void);
void *QCMD5WithPointer(void);
void *QCMD5WithString(void);
void *QCMatrix_Clear(void);
void *QCMatrix_ConcatenateWithMatrix(void);
void *QCMatrix_Copy(void);
void *QCMatrix_Determinant(void);
void *QCMatrix_GetRotationAngles(void);
void *QCMatrix_GetRotationAxisAndAngle(void);
void *QCMatrix_GetVectorW(void);
void *QCMatrix_GetVectorX(void);
void *QCMatrix_GetVectorY(void);
void *QCMatrix_GetVectorZ(void);
void *QCMatrix_IsIdentity(void);
void *QCMatrix_MultiplyByMatrix(void);
void *QCMatrix_Negate(void);
void *QCMatrix_RotateVector(void);
void *QCMatrix_SetRotationAroundAxisX(void);
void *QCMatrix_SetRotationAroundAxisY(void);
void *QCMatrix_SetRotationAroundAxisZ(void);
void *QCMatrix_SetRotationFromAngles(void);
void *QCMatrix_SetRotationFromAxisAndAngle(void);
void *QCMatrix_SetRotationFromVectors(void);
void *QCMatrix_SetScale(void);
void *QCMatrix_SetScaleUniform(void);
void *QCMatrix_SetTranslation(void);
void *QCMatrix_TransformVector(void);
void *QCPatchFromComposition(void);
void *QCPatchFromCompositionWithOptions(void);
void *QCPatchToComposition(void);
void *QCPatchToFlattenedComposition(void);
void *QCPathFromState(void);
void *QCProFX(void);
void *QCProFXRegisterAllocationCallbacks(void);
void *QCQuaternion_Add(void);
void *QCQuaternion_Clear(void);
void *QCQuaternion_Conjugate(void);
void *QCQuaternion_Copy(void);
void *QCQuaternion_Divide(void);
void *QCQuaternion_Dot(void);
void *QCQuaternion_Exp(void);
void *QCQuaternion_GetMatrix(void);
void *QCQuaternion_GetRotationAngles(void);
void *QCQuaternion_GetRotationAxisAndAngle(void);
void *QCQuaternion_Inverse(void);
void *QCQuaternion_Length(void);
void *QCQuaternion_LinearInterpolation(void);
void *QCQuaternion_Log(void);
void *QCQuaternion_LogDifference(void);
void *QCQuaternion_Multiply(void);
void *QCQuaternion_Normalize(void);
void *QCQuaternion_ScaleAngle(void);
void *QCQuaternion_SetFromMatrix(void);
void *QCQuaternion_SetFromRotationAngles(void);
void *QCQuaternion_SetFromRotationAxisAndAngle(void);
void *QCQuaternion_SetFromVectors(void);
void *QCQuaternion_SphericalLinearInterpolation(void);
void *QCQuaternion_Square(void);
void *QCQuaternion_SquareRoot(void);
void *QCQuaternion_Substract(void);
void *QCResolveAliasPath(void);
void *QCRestorePatchInputParameters(void);
void *QCSavePatchInputParameters(void);
void *QCStateFromPath(void);
void *QCVector_Add(void);
void *QCVector_CrossProduct(void);
void *QCVector_DotProduct(void);
void *QCVector_EqualsVector(void);
void *QCVector_Length(void);
void *QCVector_LinearInterpolation(void);
void *QCVector_Make(void);
void *QCVector_Normalize(void);
void *QCVector_Scale(void);
void *QCVector_Substract(void);
void *QCVisualizerIPCClient_End(void);
void *QCVisualizerIPCClient_GetAudioInformation(void);
void *QCVisualizerIPCClient_IsRunning(void);
void *QCVisualizerIPCClient_Start(void);
void *QCVisualizerIPCServer_End(void);
void *QCVisualizerIPCServer_HasClients(void);
void *QCVisualizerIPCServer_IsRunning(void);
void *QCVisualizerIPCServer_SendAudioInformation(void);
void *QCVisualizerIPCServer_SendTrackInformation(void);
void *QCVisualizerIPCServer_Start(void);


extern void* const QCCompositionAttributeApplePrivateKey;
extern void* const QCCompositionAttributeBuiltInKey;
extern void* const QCCompositionAttributeCategoryKey;
extern void* const QCCompositionAttributeCopyrightKey;
extern void* const QCCompositionAttributeDescriptionKey;
extern void* const QCCompositionAttributeExcludedHostsKey;
extern void* const QCCompositionAttributeExclusiveHostsKey;
extern void* const QCCompositionAttributeHasConsumersKey;
extern void* const QCCompositionAttributeIsTimeDependentKey;
extern void* const QCCompositionAttributeNameKey;
extern void* const QCCompositionAttributeProtocolsKey;
extern void* const QCCompositionAttributeSourceKey;
extern void* const QCCompositionAttributeThumbnailDataKey;
extern void* const QCCompositionCategoryDistortion;
extern void* const QCCompositionCategoryStylize;
extern void* const QCCompositionCategoryUtility;
extern void* const QCCompositionInputAudioPeakKey;
extern void* const QCCompositionInputAudioSpectrumKey;
extern void* const QCCompositionInputDestinationImageKey;
extern void* const QCCompositionInputImageKey;
extern void* const QCCompositionInputImageListKey;
extern void* const QCCompositionInputImageSignalKey;
extern void* const QCCompositionInputPaceKey;
extern void* const QCCompositionInputPreviewModeKey;
extern void* const QCCompositionInputPrimaryColorKey;
extern void* const QCCompositionInputScreenImageKey;
extern void* const QCCompositionInputSecondaryColorKey;
extern void* const QCCompositionInputSourceImageKey;
extern void* const QCCompositionInputTrackInfoKey;
extern void* const QCCompositionInputTrackPositionKey;
extern void* const QCCompositionInputTrackSignalKey;
extern void* const QCCompositionInputWindowImagesKey;
extern void* const QCCompositionInputXKey;
extern void* const QCCompositionInputYKey;
extern void* const QCCompositionOutputImageKey;
extern void* const QCCompositionOutputRefreshImageCountKey;
extern void* const QCCompositionOutputTotalImageCountKey;
extern void* const QCCompositionOutputWebPageURLKey;
extern void* const QCCompositionPickerPanelDidSelectCompositionNotification;
extern void* const QCCompositionPickerPanelWillSelectCompositionNotification;
extern void* const QCCompositionPickerViewDidSelectCompositionNotification;
extern void* const QCCompositionPickerViewWillSelectCompositionNotification;
extern void* const QCCompositionProtocolGraphicAnimation;
extern void* const QCCompositionProtocolGraphicTransition;
extern void* const QCCompositionProtocolImageFilter;
extern void* const QCCompositionProtocolMusicVisualizer;
extern void* const QCCompositionProtocolPhotoVisualizer;
extern void* const QCCompositionProtocolScreenSaver;
extern void* const QCCompositionRepositoryDidUpdateNotification;
extern void* const QCPixelFormatCompatibility_CoreVideo;
extern void* const QCPlugInAttributeApplePrivateKey;
extern void* const QCPlugInAttributeCategoriesKey;
extern void* const QCPlugInAttributeCopyrightKey;
extern void* const QCPlugInAttributeDescriptionKey;
extern void* const QCPlugInAttributeExamplesKey;
extern void* const QCPlugInAttributeNameKey;
extern void* const QCPlugInExecutionArgumentEventKey;
extern void* const QCPlugInExecutionArgumentMouseLocationKey;
extern void* const QCPlugInPixelFormatARGB8;
extern void* const QCPlugInPixelFormatBGRA8;
extern void* const QCPlugInPixelFormatI8;
extern void* const QCPlugInPixelFormatIf;
extern void* const QCPlugInPixelFormatRGBAf;
extern void* const QCPortAttributeDefaultValueKey;
extern void* const QCPortAttributeMaximumValueKey;
extern void* const QCPortAttributeMenuItemsKey;
extern void* const QCPortAttributeMinimumValueKey;
extern void* const QCPortAttributeNameKey;
extern void* const QCPortAttributeTypeKey;
extern void* const QCPortTypeBoolean;
extern void* const QCPortTypeColor;
extern void* const QCPortTypeImage;
extern void* const QCPortTypeIndex;
extern void* const QCPortTypeNumber;
extern void* const QCPortTypeString;
extern void* const QCPortTypeStructure;
extern void* const QCRendererEventKey;
extern void* const QCRendererMouseLocationKey;
extern void* const QCRuntimeEventKey;
extern void* const QCRuntimeMouseLocationKey;
extern void* const QCViewDidEnterFullScreenNotification;
extern void* const QCViewDidExitFullScreenNotification;
extern void* const QCViewDidSetPatchNotification;
extern void* const QCViewDidStartRenderingNotification;
extern void* const QCViewDidStopRenderingNotification;
extern void* const QuartzComposerVersionNumber;
extern void* const QuartzComposerVersionString;
extern void* const kQCNullMD5;
extern void* const kQCPixelFormatCompatibility_CGBitmapContext;
extern void* const kQCPixelFormatCompatibility_CGImage;
extern void* const kQCPixelFormatCompatibility_CIBitmapContext;
extern void* const kQCPixelFormatCompatibility_CIImage;
extern void* const kQCPixelFormatCompatibility_CoreVideo;
extern void* const kQCPixelFormatCompatibility_GLContext;
extern void* const kQCPixelFormatCompatibility_OpenGL;
extern void* const kQCPixelFormatCompatibility_QuickTime;

#endif

