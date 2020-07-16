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


#ifndef _ModelIO_H_
#define _ModelIO_H_

#import <Foundation/Foundation.h>

#import <ModelIO/MDLAssetResolver.h>
#import <ModelIO/MDLComponent.h>
#import <ModelIO/MDLJointAnimation.h>
#import <ModelIO/MDLMeshBufferAllocator.h>
#import <ModelIO/MDLMeshBufferZone.h>
#import <ModelIO/MDLMorphDeformerComponent.h>
#import <ModelIO/MDLNamed.h>
#import <ModelIO/MDLObjectContainerComponent.h>
#import <ModelIO/MDLSkinDeformerComponent.h>
#import <ModelIO/MDLTransformComponent.h>
#import <ModelIO/MDLTransformOp.h>
#import <ModelIO/MDLTextureFilter.h>
#import <ModelIO/MDLTextureSampler.h>
#import <ModelIO/MDLMaterialProperty.h>
#import <ModelIO/MDLScatteringFunction.h>
#import <ModelIO/MDLPhysicallyPlausibleScatteringFunction.h>
#import <ModelIO/MDLMaterial.h>
#import <ModelIO/MDLMaterialPropertyConnection.h>
#import <ModelIO/MDLMaterialPropertyNode.h>
#import <ModelIO/MDLMaterialPropertyGraph.h>
#import <ModelIO/MDLMorphDeformer.h>
#import <ModelIO/MDLRelativeAssetResolver.h>
#import <ModelIO/MDLPathAssetResolver.h>
#import <ModelIO/MDLBundleAssetResolver.h>
#import <ModelIO/MDLArchiveAssetResolver.h>
#import <ModelIO/_MDLProbeCluster.h>
#import <ModelIO/MDLAsset.h>
#import <ModelIO/MDLLight.h>
#import <ModelIO/MDLPhysicallyPlausibleLight.h>
#import <ModelIO/MDLAreaLight.h>
#import <ModelIO/MDLPhotometricLight.h>
#import <ModelIO/MDLTransformRotateXOp.h>
#import <ModelIO/MDLTransformRotateYOp.h>
#import <ModelIO/MDLTransformRotateZOp.h>
#import <ModelIO/MDLTransformTranslateOp.h>
#import <ModelIO/MDLTransformRotateOp.h>
#import <ModelIO/MDLTransformScaleOp.h>
#import <ModelIO/MDLTransformOrientOp.h>
#import <ModelIO/MDLTransformMatrixOp.h>
#import <ModelIO/MDLTransformStack.h>
#import <ModelIO/MDLLightProbe.h>
#import <ModelIO/MDLVolumeGrid.h>
#import <ModelIO/MDLSubmeshTopology.h>
#import <ModelIO/MDLSubmesh.h>
#import <ModelIO/MDLCamera.h>
#import <ModelIO/MDLStereoscopicCamera.h>
#import <ModelIO/MDLInteractiveCameraController.h>
#import <ModelIO/MDLVoxelArray.h>
#import <ModelIO/MDLVertexBufferLayout.h>
#import <ModelIO/MDLVertexAttribute.h>
#import <ModelIO/MDLVertexDescriptor.h>
#import <ModelIO/MDLAnimatedValue.h>
#import <ModelIO/MDLAnimatedScalarArray.h>
#import <ModelIO/MDLAnimatedVector3Array.h>
#import <ModelIO/MDLAnimatedQuaternionArray.h>
#import <ModelIO/MDLAnimatedScalar.h>
#import <ModelIO/MDLAnimatedVector2.h>
#import <ModelIO/MDLAnimatedVector3.h>
#import <ModelIO/MDLAnimatedVector4.h>
#import <ModelIO/MDLAnimatedQuaternion.h>
#import <ModelIO/MDLAnimatedMatrix4x4.h>
#import <ModelIO/MDLMatrix4x4Array.h>
#import <ModelIO/MDLVertexAttributeData.h>
#import <ModelIO/MDLMesh.h>
#import <ModelIO/MDLSkeleton.h>
#import <ModelIO/MDLPackedJointAnimation.h>
#import <ModelIO/MDLAnimationBindComponent.h>
#import <ModelIO/MDLMemoryMappedData.h>
#import <ModelIO/MDLScene.h>
#import <ModelIO/MDLTransform.h>
#import <ModelIO/_MDL_DarwinHelper__internal.h>
#import <ModelIO/MDLSkinDeformer.h>
#import <ModelIO/MDLMeshBufferMap.h>
#import <ModelIO/MDLMeshBufferZoneDefault.h>
#import <ModelIO/MDLMeshBufferData.h>
#import <ModelIO/MDLMeshBufferDataAllocator.h>
#import <ModelIO/MDLMeshBufferAllocatorDefault.h>
#import <ModelIO/MDLObject.h>
#import <ModelIO/MDLObjectContainer.h>
#import <ModelIO/MDLTexture.h>
#import <ModelIO/MDLNoiseTexture.h>
#import <ModelIO/MDLURLTexture.h>
#import <ModelIO/MDLCheckerboardTexture.h>
#import <ModelIO/MDLColorSwatchTexture.h>
#import <ModelIO/MDLSkyCubeTexture.h>
#import <ModelIO/MDLNormalMapTexture.h>

void* _ZNK7MDLAABB9intersectERKN7ModelIO12RTRaySegmentERDv4_fS5_RfS6_Rh(void);
void* _ZNK7MDLAABB9intersectERKN7ModelIO12RTRaySegmentERfS4_Rh(void);
void* directionForPixel(void);
void* equirectangularCoordinateForDirection(void);

#endif
