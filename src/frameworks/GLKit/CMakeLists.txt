project(GLKit)

remove_sdk_framework(GLKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "118.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(GLKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/GLKit"
)

add_framework(GLKit
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/GLKit.m
		src/GLKBaseEffect.m
		src/GLKSkyboxEffect.m
		src/GLKMatrix.m
		src/GLKMeshBufferHolder.m
		src/GLKMeshBufferZone.m
		src/GLKMeshBuffer.m
		src/GLKMeshBufferAllocator.m
		src/GLKSubmesh.m
		src/GLKMesh.m
		src/GLKReflectionMapEffect.m
		src/GLKEffect.m
		src/GLKShaderBlockNode.m
		src/GLKHashableBigInt.m
		src/GLKShadingHash.m
		src/GLKEffectProperty.m
		src/GLKEffectPropertyConstantColor.m
		src/GLKEffectPropertyFog.m
		src/GLKEffectPropertyLight.m
		src/GLKEffectPropertyMaterial.m
		src/GLKEffectPropertyTexGen.m
		src/GLKEffectPropertyTexture.m
		src/GLKEffectPropertyTransform.m
		src/GLKTextureTXR.m
		src/GLKTexture.m
		src/GLKTextureInfo.m
		src/GLKTextureLoader.m

	DEPENDENCIES
		system
		objc
		Foundation
)
