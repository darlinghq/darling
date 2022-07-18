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


#include <SpriteKit/SpriteKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* SKGLPrintError(void)
{
    if (verbose) puts("STUB: SKGLPrintError called");
    return NULL;
}

void* SKGLPrintFramebufferStatus(void)
{
    if (verbose) puts("STUB: SKGLPrintFramebufferStatus called");
    return NULL;
}

void* SKGLPrintState(void)
{
    if (verbose) puts("STUB: SKGLPrintState called");
    return NULL;
}

void* SKGLPrintVertexAttrib(void)
{
    if (verbose) puts("STUB: SKGLPrintVertexAttrib called");
    return NULL;
}

void* SKGenerateNormalMap(void)
{
    if (verbose) puts("STUB: SKGenerateNormalMap called");
    return NULL;
}

void* SKGenerateNormalMapWithMultiSampling(void)
{
    if (verbose) puts("STUB: SKGenerateNormalMapWithMultiSampling called");
    return NULL;
}

void* SKGetCoreAnimationFPS(void)
{
    if (verbose) puts("STUB: SKGetCoreAnimationFPS called");
    return NULL;
}

void* SKGetGraphicsDeviceUtilizationPercent(void)
{
    if (verbose) puts("STUB: SKGetGraphicsDeviceUtilizationPercent called");
    return NULL;
}

void* SKGetGraphicsRendererUtilizationPercent(void)
{
    if (verbose) puts("STUB: SKGetGraphicsRendererUtilizationPercent called");
    return NULL;
}

void* SKGetGraphicsTilerUtilizationPercent(void)
{
    if (verbose) puts("STUB: SKGetGraphicsTilerUtilizationPercent called");
    return NULL;
}

void* SKGetIdleProcessorUtilizationPercent(void)
{
    if (verbose) puts("STUB: SKGetIdleProcessorUtilizationPercent called");
    return NULL;
}

void* SKGetPrefersFloatFramebuffer(void)
{
    if (verbose) puts("STUB: SKGetPrefersFloatFramebuffer called");
    return NULL;
}

void* SKGetPrefersLowPowerGPUs(void)
{
    if (verbose) puts("STUB: SKGetPrefersLowPowerGPUs called");
    return NULL;
}

void* SKGetPrefersOpenGL(void)
{
    if (verbose) puts("STUB: SKGetPrefersOpenGL called");
    return NULL;
}

void* SKGetResourceBundle(void)
{
    if (verbose) puts("STUB: SKGetResourceBundle called");
    return NULL;
}

void* SKGetSystemProcessorUtilizationPercent(void)
{
    if (verbose) puts("STUB: SKGetSystemProcessorUtilizationPercent called");
    return NULL;
}

void* SKGetUserProcessorUtilizationPercent(void)
{
    if (verbose) puts("STUB: SKGetUserProcessorUtilizationPercent called");
    return NULL;
}

void* SKGetVersionString(void)
{
    if (verbose) puts("STUB: SKGetVersionString called");
    return NULL;
}

void* SKNodeNameMatches(void)
{
    if (verbose) puts("STUB: SKNodeNameMatches called");
    return NULL;
}

void* SKSetPrefersFloatFramebuffer(void)
{
    if (verbose) puts("STUB: SKSetPrefersFloatFramebuffer called");
    return NULL;
}

void* SKSetPrefersLowPowerGPUs(void)
{
    if (verbose) puts("STUB: SKSetPrefersLowPowerGPUs called");
    return NULL;
}

void* SKSetPrefersOpenGL(void)
{
    if (verbose) puts("STUB: SKSetPrefersOpenGL called");
    return NULL;
}

void* SKSetResourceBundle(void)
{
    if (verbose) puts("STUB: SKSetResourceBundle called");
    return NULL;
}

void* _SKGetPerformanceStatisticsForPid(void)
{
    if (verbose) puts("STUB: _SKGetPerformanceStatisticsForPid called");
    return NULL;
}

void* _SKStartCollectingPerformanceStatisticsForPid(void)
{
    if (verbose) puts("STUB: _SKStartCollectingPerformanceStatisticsForPid called");
    return NULL;
}

void* _SKStopCollectingPerformanceStatisticsForPid(void)
{
    if (verbose) puts("STUB: _SKStopCollectingPerformanceStatisticsForPid called");
    return NULL;
}

void* _perfBeginActions(void)
{
    if (verbose) puts("STUB: _perfBeginActions called");
    return NULL;
}

void* _perfBeginClientUpdate(void)
{
    if (verbose) puts("STUB: _perfBeginClientUpdate called");
    return NULL;
}

void* _perfBeginConstraints(void)
{
    if (verbose) puts("STUB: _perfBeginConstraints called");
    return NULL;
}

void* _perfBeginEmitters(void)
{
    if (verbose) puts("STUB: _perfBeginEmitters called");
    return NULL;
}

void* _perfBeginPhysics(void)
{
    if (verbose) puts("STUB: _perfBeginPhysics called");
    return NULL;
}

void* _perfBeginRender(void)
{
    if (verbose) puts("STUB: _perfBeginRender called");
    return NULL;
}

void* _perfEndActions(void)
{
    if (verbose) puts("STUB: _perfEndActions called");
    return NULL;
}

void* _perfEndClientUpdate(void)
{
    if (verbose) puts("STUB: _perfEndClientUpdate called");
    return NULL;
}

void* _perfEndConstraints(void)
{
    if (verbose) puts("STUB: _perfEndConstraints called");
    return NULL;
}

void* _perfEndEmitters(void)
{
    if (verbose) puts("STUB: _perfEndEmitters called");
    return NULL;
}

void* _perfEndPhysics(void)
{
    if (verbose) puts("STUB: _perfEndPhysics called");
    return NULL;
}

void* _perfEndRender(void)
{
    if (verbose) puts("STUB: _perfEndRender called");
    return NULL;
}

void* _perfEndRequestDrawable(void)
{
    if (verbose) puts("STUB: _perfEndRequestDrawable called");
    return NULL;
}

void* _perfMarkCADisplayLinkEnd(void)
{
    if (verbose) puts("STUB: _perfMarkCADisplayLinkEnd called");
    return NULL;
}

void* _perfMarkCADisplayLinkStart(void)
{
    if (verbose) puts("STUB: _perfMarkCADisplayLinkStart called");
    return NULL;
}

void* _perfStartRequestDrawable(void)
{
    if (verbose) puts("STUB: _perfStartRequestDrawable called");
    return NULL;
}

void* _perfUpdateStatistics(void)
{
    if (verbose) puts("STUB: _perfUpdateStatistics called");
    return NULL;
}

void* skCurrentTime(void)
{
    if (verbose) puts("STUB: skCurrentTime called");
    return NULL;
}

void* skSetTimeOffset(void)
{
    if (verbose) puts("STUB: skSetTimeOffset called");
    return NULL;
}
