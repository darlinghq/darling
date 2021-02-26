/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _GLKit_H_
#define _GLKit_H_

#import <Foundation/Foundation.h>

#import <GLKit/GLKNamedEffect.h>
#import <GLKit/MDLMeshBuffer.h>
#import <GLKit/MDLMeshBufferAllocator.h>
#import <GLKit/MDLMeshBufferZone.h>
#import <GLKit/NSXMLParserDelegate.h>
#import <GLKit/GLKBaseEffect.h>
#import <GLKit/GLKSkyboxEffect.h>
#import <GLKit/GLKMeshBufferHolder.h>
#import <GLKit/GLKMeshBufferZone.h>
#import <GLKit/GLKMeshBuffer.h>
#import <GLKit/GLKMeshBufferAllocator.h>
#import <GLKit/GLKSubmesh.h>
#import <GLKit/GLKMesh.h>
#import <GLKit/GLKReflectionMapEffect.h>
#import <GLKit/GLKEffect.h>
#import <GLKit/GLKShaderBlockNode.h>
#import <GLKit/GLKHashableBigInt.h>
#import <GLKit/GLKShadingHash.h>
#import <GLKit/GLKEffectProperty.h>
#import <GLKit/GLKEffectPropertyConstantColor.h>
#import <GLKit/GLKEffectPropertyFog.h>
#import <GLKit/GLKEffectPropertyLight.h>
#import <GLKit/GLKEffectPropertyMaterial.h>
#import <GLKit/GLKEffectPropertyTexGen.h>
#import <GLKit/GLKEffectPropertyTexture.h>
#import <GLKit/GLKEffectPropertyTransform.h>
#import <GLKit/GLKTextureTXR.h>
#import <GLKit/GLKTexture.h>
#import <GLKit/GLKTextureInfo.h>
#import <GLKit/GLKTextureLoader.h>

void* GLKMathProject(void);
void* GLKMathUnproject(void);
void* GLKMatrix3Invert(void);
void* GLKMatrix3InvertAndTranspose(void);
void* GLKMatrix4Invert(void);
void* GLKMatrix4InvertAndTranspose(void);
void* GLKMatrixStackCreate(void);
void* GLKMatrixStackGetMatrix2(void);
void* GLKMatrixStackGetMatrix3(void);
void* GLKMatrixStackGetMatrix3Inverse(void);
void* GLKMatrixStackGetMatrix3InverseTranspose(void);
void* GLKMatrixStackGetMatrix4(void);
void* GLKMatrixStackGetMatrix4Inverse(void);
void* GLKMatrixStackGetMatrix4InverseTranspose(void);
void* GLKMatrixStackGetModifyCount(void);
void* GLKMatrixStackGetTypeID(void);
void* GLKMatrixStackLoadMatrix4(void);
void* GLKMatrixStackMultiplyMatrix4(void);
void* GLKMatrixStackMultiplyMatrixStack(void);
void* GLKMatrixStackPop(void);
void* GLKMatrixStackPush(void);
void* GLKMatrixStackRotate(void);
void* GLKMatrixStackRotateWithVector3(void);
void* GLKMatrixStackRotateWithVector4(void);
void* GLKMatrixStackRotateX(void);
void* GLKMatrixStackRotateY(void);
void* GLKMatrixStackRotateZ(void);
void* GLKMatrixStackScale(void);
void* GLKMatrixStackScaleWithVector3(void);
void* GLKMatrixStackScaleWithVector4(void);
void* GLKMatrixStackSize(void);
void* GLKMatrixStackTranslate(void);
void* GLKMatrixStackTranslateWithVector3(void);
void* GLKMatrixStackTranslateWithVector4(void);
void* GLKQuaternionAngle(void);
void* GLKQuaternionAxis(void);
void* GLKQuaternionMakeWithMatrix3(void);
void* GLKQuaternionMakeWithMatrix4(void);
void* GLKQuaternionRotateVector3Array(void);
void* GLKQuaternionRotateVector4Array(void);
void* GLKQuaternionSlerp(void);
void* GLKVertexAttributeParametersFromModelIO(void);
void* NSStringFromGLKMatrix2(void);
void* NSStringFromGLKMatrix3(void);
void* NSStringFromGLKMatrix4(void);
void* NSStringFromGLKQuaternion(void);
void* NSStringFromGLKVector2(void);
void* NSStringFromGLKVector3(void);
void* NSStringFromGLKVector4(void);
void* _GLKMatrixDebugDescription(void);
void* _GLKMatrixDestroy(void);
void* _GLKMatrixFormatDescription(void);
void* _GLKMatrixStackRegisterClass(void);
void* _GLKModelErrorWithCode(void);
void* _GLKModelErrorWithCodeAndErrorString(void);
void* _GLKModelErrorWithCodeAndUserInfo(void);
void* _GLKStackAlloc(void);
void* _GLKStackDealloc(void);
void* _GLKStackLoad(void);
void* _GLKStackPop(void);
void* _GLKStackPush(void);
void* _GLKStackSize(void);
void* _GLKStackTop(void);
void* _GLKTextureErrorWithCode(void);
void* _GLKTextureErrorWithCodeAndErrorString(void);
void* _GLKTextureErrorWithCodeAndUserInfo(void);
void* _lightStateChanged(void);
void* _modelviewMatrixMask(void);
void* _normalizedNormalsMask(void);
void* _reflectionMappingEnabled(void);
void* _texturingEnabledMask(void);
void* _useTexCoordAttribMask(void);
void* _vNormalEyeMask(void);
void* _vPositionEyeMask(void);
void* glkLinkedOSVersion(void);

#endif
