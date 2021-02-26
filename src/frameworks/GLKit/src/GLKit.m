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


#include <GLKit/GLKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* GLKMathProject(void)
{
    if (verbose) puts("STUB: GLKMathProject called");
    return NULL;
}

void* GLKMathUnproject(void)
{
    if (verbose) puts("STUB: GLKMathUnproject called");
    return NULL;
}

void* GLKMatrix3Invert(void)
{
    if (verbose) puts("STUB: GLKMatrix3Invert called");
    return NULL;
}

void* GLKMatrix3InvertAndTranspose(void)
{
    if (verbose) puts("STUB: GLKMatrix3InvertAndTranspose called");
    return NULL;
}

void* GLKMatrix4Invert(void)
{
    if (verbose) puts("STUB: GLKMatrix4Invert called");
    return NULL;
}

void* GLKMatrix4InvertAndTranspose(void)
{
    if (verbose) puts("STUB: GLKMatrix4InvertAndTranspose called");
    return NULL;
}

void* GLKMatrixStackCreate(void)
{
    if (verbose) puts("STUB: GLKMatrixStackCreate called");
    return NULL;
}

void* GLKMatrixStackGetMatrix2(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix2 called");
    return NULL;
}

void* GLKMatrixStackGetMatrix3(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix3 called");
    return NULL;
}

void* GLKMatrixStackGetMatrix3Inverse(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix3Inverse called");
    return NULL;
}

void* GLKMatrixStackGetMatrix3InverseTranspose(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix3InverseTranspose called");
    return NULL;
}

void* GLKMatrixStackGetMatrix4(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix4 called");
    return NULL;
}

void* GLKMatrixStackGetMatrix4Inverse(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix4Inverse called");
    return NULL;
}

void* GLKMatrixStackGetMatrix4InverseTranspose(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetMatrix4InverseTranspose called");
    return NULL;
}

void* GLKMatrixStackGetModifyCount(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetModifyCount called");
    return NULL;
}

void* GLKMatrixStackGetTypeID(void)
{
    if (verbose) puts("STUB: GLKMatrixStackGetTypeID called");
    return NULL;
}

void* GLKMatrixStackLoadMatrix4(void)
{
    if (verbose) puts("STUB: GLKMatrixStackLoadMatrix4 called");
    return NULL;
}

void* GLKMatrixStackMultiplyMatrix4(void)
{
    if (verbose) puts("STUB: GLKMatrixStackMultiplyMatrix4 called");
    return NULL;
}

void* GLKMatrixStackMultiplyMatrixStack(void)
{
    if (verbose) puts("STUB: GLKMatrixStackMultiplyMatrixStack called");
    return NULL;
}

void* GLKMatrixStackPop(void)
{
    if (verbose) puts("STUB: GLKMatrixStackPop called");
    return NULL;
}

void* GLKMatrixStackPush(void)
{
    if (verbose) puts("STUB: GLKMatrixStackPush called");
    return NULL;
}

void* GLKMatrixStackRotate(void)
{
    if (verbose) puts("STUB: GLKMatrixStackRotate called");
    return NULL;
}

void* GLKMatrixStackRotateWithVector3(void)
{
    if (verbose) puts("STUB: GLKMatrixStackRotateWithVector3 called");
    return NULL;
}

void* GLKMatrixStackRotateWithVector4(void)
{
    if (verbose) puts("STUB: GLKMatrixStackRotateWithVector4 called");
    return NULL;
}

void* GLKMatrixStackRotateX(void)
{
    if (verbose) puts("STUB: GLKMatrixStackRotateX called");
    return NULL;
}

void* GLKMatrixStackRotateY(void)
{
    if (verbose) puts("STUB: GLKMatrixStackRotateY called");
    return NULL;
}

void* GLKMatrixStackRotateZ(void)
{
    if (verbose) puts("STUB: GLKMatrixStackRotateZ called");
    return NULL;
}

void* GLKMatrixStackScale(void)
{
    if (verbose) puts("STUB: GLKMatrixStackScale called");
    return NULL;
}

void* GLKMatrixStackScaleWithVector3(void)
{
    if (verbose) puts("STUB: GLKMatrixStackScaleWithVector3 called");
    return NULL;
}

void* GLKMatrixStackScaleWithVector4(void)
{
    if (verbose) puts("STUB: GLKMatrixStackScaleWithVector4 called");
    return NULL;
}

void* GLKMatrixStackSize(void)
{
    if (verbose) puts("STUB: GLKMatrixStackSize called");
    return NULL;
}

void* GLKMatrixStackTranslate(void)
{
    if (verbose) puts("STUB: GLKMatrixStackTranslate called");
    return NULL;
}

void* GLKMatrixStackTranslateWithVector3(void)
{
    if (verbose) puts("STUB: GLKMatrixStackTranslateWithVector3 called");
    return NULL;
}

void* GLKMatrixStackTranslateWithVector4(void)
{
    if (verbose) puts("STUB: GLKMatrixStackTranslateWithVector4 called");
    return NULL;
}

void* GLKQuaternionAngle(void)
{
    if (verbose) puts("STUB: GLKQuaternionAngle called");
    return NULL;
}

void* GLKQuaternionAxis(void)
{
    if (verbose) puts("STUB: GLKQuaternionAxis called");
    return NULL;
}

void* GLKQuaternionMakeWithMatrix3(void)
{
    if (verbose) puts("STUB: GLKQuaternionMakeWithMatrix3 called");
    return NULL;
}

void* GLKQuaternionMakeWithMatrix4(void)
{
    if (verbose) puts("STUB: GLKQuaternionMakeWithMatrix4 called");
    return NULL;
}

void* GLKQuaternionRotateVector3Array(void)
{
    if (verbose) puts("STUB: GLKQuaternionRotateVector3Array called");
    return NULL;
}

void* GLKQuaternionRotateVector4Array(void)
{
    if (verbose) puts("STUB: GLKQuaternionRotateVector4Array called");
    return NULL;
}

void* GLKQuaternionSlerp(void)
{
    if (verbose) puts("STUB: GLKQuaternionSlerp called");
    return NULL;
}

void* GLKVertexAttributeParametersFromModelIO(void)
{
    if (verbose) puts("STUB: GLKVertexAttributeParametersFromModelIO called");
    return NULL;
}

void* NSStringFromGLKMatrix2(void)
{
    if (verbose) puts("STUB: NSStringFromGLKMatrix2 called");
    return NULL;
}

void* NSStringFromGLKMatrix3(void)
{
    if (verbose) puts("STUB: NSStringFromGLKMatrix3 called");
    return NULL;
}

void* NSStringFromGLKMatrix4(void)
{
    if (verbose) puts("STUB: NSStringFromGLKMatrix4 called");
    return NULL;
}

void* NSStringFromGLKQuaternion(void)
{
    if (verbose) puts("STUB: NSStringFromGLKQuaternion called");
    return NULL;
}

void* NSStringFromGLKVector2(void)
{
    if (verbose) puts("STUB: NSStringFromGLKVector2 called");
    return NULL;
}

void* NSStringFromGLKVector3(void)
{
    if (verbose) puts("STUB: NSStringFromGLKVector3 called");
    return NULL;
}

void* NSStringFromGLKVector4(void)
{
    if (verbose) puts("STUB: NSStringFromGLKVector4 called");
    return NULL;
}

void* _GLKMatrixDebugDescription(void)
{
    if (verbose) puts("STUB: _GLKMatrixDebugDescription called");
    return NULL;
}

void* _GLKMatrixDestroy(void)
{
    if (verbose) puts("STUB: _GLKMatrixDestroy called");
    return NULL;
}

void* _GLKMatrixFormatDescription(void)
{
    if (verbose) puts("STUB: _GLKMatrixFormatDescription called");
    return NULL;
}

void* _GLKMatrixStackRegisterClass(void)
{
    if (verbose) puts("STUB: _GLKMatrixStackRegisterClass called");
    return NULL;
}

void* _GLKModelErrorWithCode(void)
{
    if (verbose) puts("STUB: _GLKModelErrorWithCode called");
    return NULL;
}

void* _GLKModelErrorWithCodeAndErrorString(void)
{
    if (verbose) puts("STUB: _GLKModelErrorWithCodeAndErrorString called");
    return NULL;
}

void* _GLKModelErrorWithCodeAndUserInfo(void)
{
    if (verbose) puts("STUB: _GLKModelErrorWithCodeAndUserInfo called");
    return NULL;
}

void* _GLKStackAlloc(void)
{
    if (verbose) puts("STUB: _GLKStackAlloc called");
    return NULL;
}

void* _GLKStackDealloc(void)
{
    if (verbose) puts("STUB: _GLKStackDealloc called");
    return NULL;
}

void* _GLKStackLoad(void)
{
    if (verbose) puts("STUB: _GLKStackLoad called");
    return NULL;
}

void* _GLKStackPop(void)
{
    if (verbose) puts("STUB: _GLKStackPop called");
    return NULL;
}

void* _GLKStackPush(void)
{
    if (verbose) puts("STUB: _GLKStackPush called");
    return NULL;
}

void* _GLKStackSize(void)
{
    if (verbose) puts("STUB: _GLKStackSize called");
    return NULL;
}

void* _GLKStackTop(void)
{
    if (verbose) puts("STUB: _GLKStackTop called");
    return NULL;
}

void* _GLKTextureErrorWithCode(void)
{
    if (verbose) puts("STUB: _GLKTextureErrorWithCode called");
    return NULL;
}

void* _GLKTextureErrorWithCodeAndErrorString(void)
{
    if (verbose) puts("STUB: _GLKTextureErrorWithCodeAndErrorString called");
    return NULL;
}

void* _GLKTextureErrorWithCodeAndUserInfo(void)
{
    if (verbose) puts("STUB: _GLKTextureErrorWithCodeAndUserInfo called");
    return NULL;
}

void* _lightStateChanged(void)
{
    if (verbose) puts("STUB: _lightStateChanged called");
    return NULL;
}

void* _modelviewMatrixMask(void)
{
    if (verbose) puts("STUB: _modelviewMatrixMask called");
    return NULL;
}

void* _normalizedNormalsMask(void)
{
    if (verbose) puts("STUB: _normalizedNormalsMask called");
    return NULL;
}

void* _reflectionMappingEnabled(void)
{
    if (verbose) puts("STUB: _reflectionMappingEnabled called");
    return NULL;
}

void* _texturingEnabledMask(void)
{
    if (verbose) puts("STUB: _texturingEnabledMask called");
    return NULL;
}

void* _useTexCoordAttribMask(void)
{
    if (verbose) puts("STUB: _useTexCoordAttribMask called");
    return NULL;
}

void* _vNormalEyeMask(void)
{
    if (verbose) puts("STUB: _vNormalEyeMask called");
    return NULL;
}

void* _vPositionEyeMask(void)
{
    if (verbose) puts("STUB: _vPositionEyeMask called");
    return NULL;
}

void* glkLinkedOSVersion(void)
{
    if (verbose) puts("STUB: glkLinkedOSVersion called");
    return NULL;
}
