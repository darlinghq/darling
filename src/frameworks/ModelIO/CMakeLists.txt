project(ModelIO)

remove_sdk_framework(ModelIO)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "217.43.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ModelIO
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ModelIO"
)

add_framework(ModelIO
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ModelIO.m
        src/MDLTextureFilter.m
        src/MDLTextureSampler.m
        src/MDLMaterialProperty.m
        src/MDLScatteringFunction.m
        src/MDLPhysicallyPlausibleScatteringFunction.m
        src/MDLMaterial.m
        src/MDLMaterialPropertyConnection.m
        src/MDLMaterialPropertyNode.m
        src/MDLMaterialPropertyGraph.m
        src/MDLMorphDeformer.m
        src/MDLRelativeAssetResolver.m
        src/MDLPathAssetResolver.m
        src/MDLBundleAssetResolver.m
        src/MDLArchiveAssetResolver.m
        src/_MDLProbeCluster.m
        src/MDLAsset.m
        src/MDLLight.m
        src/MDLPhysicallyPlausibleLight.m
        src/MDLAreaLight.m
        src/MDLPhotometricLight.m
        src/MDLTransformRotateXOp.m
        src/MDLTransformRotateYOp.m
        src/MDLTransformRotateZOp.m
        src/MDLTransformTranslateOp.m
        src/MDLTransformRotateOp.m
        src/MDLTransformScaleOp.m
        src/MDLTransformOrientOp.m
        src/MDLTransformMatrixOp.m
        src/MDLTransformStack.m
        src/MDLLightProbe.m
        src/MDLVolumeGrid.m
        src/MDLSubmeshTopology.m
        src/MDLSubmesh.m
        src/MDLCamera.m
        src/MDLStereoscopicCamera.m
        src/MDLInteractiveCameraController.m
        src/MDLVoxelArray.m
        src/MDLVertexBufferLayout.m
        src/MDLVertexAttribute.m
        src/MDLVertexDescriptor.m
        src/MDLAnimatedValue.m
        src/MDLAnimatedScalarArray.m
        src/MDLAnimatedVector3Array.m
        src/MDLAnimatedQuaternionArray.m
        src/MDLAnimatedScalar.m
        src/MDLAnimatedVector2.m
        src/MDLAnimatedVector3.m
        src/MDLAnimatedVector4.m
        src/MDLAnimatedQuaternion.m
        src/MDLAnimatedMatrix4x4.m
        src/MDLMatrix4x4Array.m
        src/MDLVertexAttributeData.m
        src/MDLMesh.m
        src/MDLSkeleton.m
        src/MDLPackedJointAnimation.m
        src/MDLAnimationBindComponent.m
        src/MDLMemoryMappedData.m
        src/MDLScene.m
        src/MDLTransform.m
        src/_MDL_DarwinHelper__internal.m
        src/MDLSkinDeformer.m
        src/MDLMeshBufferMap.m
        src/MDLMeshBufferZoneDefault.m
        src/MDLMeshBufferData.m
        src/MDLMeshBufferDataAllocator.m
        src/MDLMeshBufferAllocatorDefault.m
        src/MDLObject.m
        src/MDLObjectContainer.m
        src/MDLTexture.m
        src/MDLNoiseTexture.m
        src/MDLURLTexture.m
        src/MDLCheckerboardTexture.m
        src/MDLColorSwatchTexture.m
        src/MDLSkyCubeTexture.m
        src/MDLNormalMapTexture.m

    DEPENDENCIES
        system
        objc
        Foundation
)
