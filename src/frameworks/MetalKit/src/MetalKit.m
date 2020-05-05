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


#include <MetalKit/MetalKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* MTKMetalVertexDescriptorFromModelIO(void)
{
    if (verbose) puts("STUB: MTKMetalVertexDescriptorFromModelIO called");
    return NULL;
}

void* MTKMetalVertexDescriptorFromModelIOWithError(void)
{
    if (verbose) puts("STUB: MTKMetalVertexDescriptorFromModelIOWithError called");
    return NULL;
}

void* MTKMetalVertexFormatFromModelIO(void)
{
    if (verbose) puts("STUB: MTKMetalVertexFormatFromModelIO called");
    return NULL;
}

void* MTKModelIOVertexDescriptorFromMetal(void)
{
    if (verbose) puts("STUB: MTKModelIOVertexDescriptorFromMetal called");
    return NULL;
}

void* MTKModelIOVertexDescriptorFromMetalWithError(void)
{
    if (verbose) puts("STUB: MTKModelIOVertexDescriptorFromMetalWithError called");
    return NULL;
}

void* MTKModelIOVertexFormatFromMetal(void)
{
    if (verbose) puts("STUB: MTKModelIOVertexFormatFromMetal called");
    return NULL;
}
