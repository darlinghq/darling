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


#ifndef _SpriteKit_H_
#define _SpriteKit_H_

#import <Foundation/Foundation.h>

#import <SpriteKit/NSKeyedArchiverDelegate.h>
#import <SpriteKit/NSKeyedUnarchiverDelegate.h>
#import <SpriteKit/SKWarpable.h>
#import <SpriteKit/SKReferencedAction.h>
#import <SpriteKit/SKTransition.h>
#import <SpriteKit/SKBitmapFont.h>
#import <SpriteKit/SKLabelNode.h>
#import <SpriteKit/SKTileMapNode.h>
#import <SpriteKit/SKAudioNode.h>
#import <SpriteKit/SKSCNRenderer.h>
#import <SpriteKit/SKThreadSafeMapTable.h>
#import <SpriteKit/SKThreadSafeMutableArray.h>
#import <SpriteKit/SKThreadSafeMutableDictionary.h>
#import <SpriteKit/SKView.h>
#import <SpriteKit/SKPlaySound.h>
#import <SpriteKit/SKCustomAction.h>
#import <SpriteKit/SKRunBlock.h>
#import <SpriteKit/SKParticleSystemNode.h>
#import <SpriteKit/SKAction.h>
#import <SpriteKit/SKActionTable.h>
#import <SpriteKit/SKAnimate.h>
#import <SpriteKit/SKAnimateWarp.h>
#import <SpriteKit/SKReferenceNode.h>
#import <SpriteKit/SKColorize.h>
#import <SpriteKit/SKFade.h>
#import <SpriteKit/SKGroup.h>
#import <SpriteKit/SK3DNode.h>
#import <SpriteKit/SKFieldNode.h>
#import <SpriteKit/SKDragFieldNode.h>
#import <SpriteKit/SKVortexFieldNode.h>
#import <SpriteKit/SKLinearGravityFieldNode.h>
#import <SpriteKit/SKRadialGravityFieldNode.h>
#import <SpriteKit/SKCustomFieldNode.h>
#import <SpriteKit/SKVelocityFieldNode.h>
#import <SpriteKit/SKNoiseFieldNode.h>
#import <SpriteKit/SKTurbulenceFieldNode.h>
#import <SpriteKit/SKSpringFieldNode.h>
#import <SpriteKit/SKMagneticFieldNode.h>
#import <SpriteKit/SKElectricFieldNode.h>
#import <SpriteKit/SKMove.h>
#import <SpriteKit/SKCameraNode.h>
#import <SpriteKit/SKRemove.h>
#import <SpriteKit/SKRepeat.h>
#import <SpriteKit/SKSpeed.h>
#import <SpriteKit/SKFrameCaptureDelegate.h>
#import <SpriteKit/SKRotate.h>
#import <SpriteKit/SKRunAction.h>
#import <SpriteKit/SKScale.h>
#import <SpriteKit/SKSequence.h>
#import <SpriteKit/SKWarpGeometry.h>
#import <SpriteKit/SKWarpGeometryGrid.h>
#import <SpriteKit/SKWait.h>
#import <SpriteKit/SKFollowPath.h>
#import <SpriteKit/SKGlobalSharedContextRegistry.h>
#import <SpriteKit/SKTransformNode.h>
#import <SpriteKit/SKScene.h>
#import <SpriteKit/SKShapeNode.h>
#import <SpriteKit/SKSpriteNode.h>
#import <SpriteKit/SKEffectNode.h>
#import <SpriteKit/SKDisplayLink.h>
#import <SpriteKit/SKRange.h>
#import <SpriteKit/SKConstraint.h>
#import <SpriteKit/SKSizeConstraint.h>
#import <SpriteKit/SKScaleConstraint.h>
#import <SpriteKit/SKRotationConstraint.h>
#import <SpriteKit/SKOrientationConstraint.h>
#import <SpriteKit/SKDistanceConstraint.h>
#import <SpriteKit/SKPositionConstraint.h>
#import <SpriteKit/SKCaptureUnarchiverDelegate.h>
#import <SpriteKit/SKNodeVisualRepresentationUnarchiver.h>
#import <SpriteKit/SKNode.h>
#import <SpriteKit/SKNodeCustomClassUnarchiver.h>
#import <SpriteKit/SKNodeVisualRepresentationArchiver.h>
#import <SpriteKit/SKUniformShaderReference.h>
#import <SpriteKit/SKUniform.h>
#import <SpriteKit/SKTileDefinition.h>
#import <SpriteKit/SKButtonNode.h>
#import <SpriteKit/SKButtonNodeSprite.h>
#import <SpriteKit/SKEmitterNode.h>
#import <SpriteKit/SKKeyframeSequence.h>
#import <SpriteKit/SKVideoNode.h>
#import <SpriteKit/SKTextureCache.h>
#import <SpriteKit/SKTexture.h>
#import <SpriteKit/SKTextureAtlas.h>
#import <SpriteKit/SKRegion.h>
#import <SpriteKit/SKOpenGLLayer.h>
#import <SpriteKit/SKKeyedUnarchiver.h>
#import <SpriteKit/SKCropNode.h>
#import <SpriteKit/SKHide.h>
#import <SpriteKit/SKPerformSelector.h>
#import <SpriteKit/SKJavaScriptAction.h>
#import <SpriteKit/SKSoundBuffer.h>
#import <SpriteKit/SKTextureAtlasPacker.h>
#import <SpriteKit/SKSoundContext.h>
#import <SpriteKit/SKSoundSource.h>
#import <SpriteKit/SKSceneUnarchiveDummy.h>
#import <SpriteKit/SKMutableTexture.h>
#import <SpriteKit/IKLink.h>
#import <SpriteKit/SKReach.h>
#import <SpriteKit/SKTileGroupRule.h>
#import <SpriteKit/SKTileGroup.h>
#import <SpriteKit/SKTileSet.h>
#import <SpriteKit/SKTileStamp.h>
#import <SpriteKit/SKRenderer.h>
#import <SpriteKit/SKStrength.h>
#import <SpriteKit/SKResize.h>
#import <SpriteKit/SKPhysicsBody.h>
#import <SpriteKit/SKPhysicsContact.h>
#import <SpriteKit/SKPhysicsJoint.h>
#import <SpriteKit/SKPhysicsJointPin.h>
#import <SpriteKit/SKPhysicsJointSpring.h>
#import <SpriteKit/SKPhysicsJointFixed.h>
#import <SpriteKit/SKPhysicsJointSliding.h>
#import <SpriteKit/SKPhysicsJointLimit.h>
#import <SpriteKit/SKPhysicsWorld.h>
#import <SpriteKit/SKLightNode.h>
#import <SpriteKit/SKUniformInfo.h>
#import <SpriteKit/SKShader.h>
#import <SpriteKit/SKReachConstraints.h>
#import <SpriteKit/SKOpenGLNode.h>
#import <SpriteKit/SKPhysicsGrid.h>
#import <SpriteKit/SKFalloff.h>
#import <SpriteKit/SKAttribute.h>
#import <SpriteKit/SKAttributeValue.h>

void* SKGLPrintError(void);
void* SKGLPrintFramebufferStatus(void);
void* SKGLPrintState(void);
void* SKGLPrintVertexAttrib(void);
void* SKGenerateNormalMap(void);
void* SKGenerateNormalMapWithMultiSampling(void);
void* SKGetCoreAnimationFPS(void);
void* SKGetGraphicsDeviceUtilizationPercent(void);
void* SKGetGraphicsRendererUtilizationPercent(void);
void* SKGetGraphicsTilerUtilizationPercent(void);
void* SKGetIdleProcessorUtilizationPercent(void);
void* SKGetPrefersFloatFramebuffer(void);
void* SKGetPrefersLowPowerGPUs(void);
void* SKGetPrefersOpenGL(void);
void* SKGetResourceBundle(void);
void* SKGetSystemProcessorUtilizationPercent(void);
void* SKGetUserProcessorUtilizationPercent(void);
void* SKGetVersionString(void);
void* SKNodeNameMatches(void);
void* SKSetPrefersFloatFramebuffer(void);
void* SKSetPrefersLowPowerGPUs(void);
void* SKSetPrefersOpenGL(void);
void* SKSetResourceBundle(void);
void* _SKGetPerformanceStatisticsForPid(void);
void* _SKStartCollectingPerformanceStatisticsForPid(void);
void* _SKStopCollectingPerformanceStatisticsForPid(void);
void* _perfBeginActions(void);
void* _perfBeginClientUpdate(void);
void* _perfBeginConstraints(void);
void* _perfBeginEmitters(void);
void* _perfBeginPhysics(void);
void* _perfBeginRender(void);
void* _perfEndActions(void);
void* _perfEndClientUpdate(void);
void* _perfEndConstraints(void);
void* _perfEndEmitters(void);
void* _perfEndPhysics(void);
void* _perfEndRender(void);
void* _perfEndRequestDrawable(void);
void* _perfMarkCADisplayLinkEnd(void);
void* _perfMarkCADisplayLinkStart(void);
void* _perfStartRequestDrawable(void);
void* _perfUpdateStatistics(void);
void* skCurrentTime(void);
void* skSetTimeOffset(void);

#endif
