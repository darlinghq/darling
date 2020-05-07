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


#ifndef _MetalKit_H_
#define _MetalKit_H_

#import <Foundation/Foundation.h>

#import <MetalKit/CALayerDelegate.h>
#import <MetalKit/CAMetalDrawable.h>
#import <MetalKit/MDLMeshBuffer.h>
#import <MetalKit/MDLMeshBufferAllocator.h>
#import <MetalKit/MDLMeshBufferZone.h>
#import <MetalKit/MDLNamed.h>
#import <MetalKit/MTLDrawable.h>
#import <MetalKit/TXRBuffer.h>
#import <MetalKit/TXRBufferAllocator.h>
#import <MetalKit/TXRBufferMap.h>
#import <MetalKit/MTKTextureLoaderPVR3.h>
#import <MetalKit/MTKTextureUploader.h>
#import <MetalKit/MTKTextureLoaderMDL.h>
#import <MetalKit/MTKTextureIOBufferMap.h>
#import <MetalKit/MTKTextureIOBuffer.h>
#import <MetalKit/MTKTextureIOBufferAllocator.h>
#import <MetalKit/MTKTextureLoader.h>
#import <MetalKit/MTKTextureLoaderData.h>
#import <MetalKit/MTKTextureLoaderImageIO.h>
#import <MetalKit/MTKTextureLoaderKTX.h>
#import <MetalKit/MTKTextureLoaderPVR.h>
#import <MetalKit/MTKOffscreenDrawable.h>
#import <MetalKit/MTKMeshBufferAllocator.h>
#import <MetalKit/MTKSubmesh.h>
#import <MetalKit/MTKMesh.h>
#import <MetalKit/MTKMeshBufferHolder.h>
#import <MetalKit/MTKMeshBufferZone.h>
#import <MetalKit/MTKMeshBuffer.h>
#import <MetalKit/MTKView.h>

void* MTKMetalVertexDescriptorFromModelIO(void);
void* MTKMetalVertexDescriptorFromModelIOWithError(void);
void* MTKMetalVertexFormatFromModelIO(void);
void* MTKModelIOVertexDescriptorFromMetal(void);
void* MTKModelIOVertexDescriptorFromMetalWithError(void);
void* MTKModelIOVertexFormatFromMetal(void);

#endif
