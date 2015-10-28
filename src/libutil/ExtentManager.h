/*
 * Copyright (c) 2008 Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
//
//	ExtentManager.h
//
#ifndef EXTENTMANAGER_H
#define EXTENTMANAGER_H

#include <list>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/errno.h>
#include <cstdio>
#include <cassert>
using namespace std;

struct ExtentInfo {
	off_t blockAddr;
	off_t numBlocks;
};

inline bool BeforeExtent(const ExtentInfo &a, const ExtentInfo &b)
{
		return (a.blockAddr + a.numBlocks) < b.blockAddr;
}

typedef list<ExtentInfo>::iterator ListExtIt;

class ExtentManager {
public:
	ExtentManager() : blockSize(0), totalBytes(0), totalBlocks(0) {};
	~ExtentManager() {};

	void Init(uint32_t theBlockSize, uint32_t theNativeBlockSize, off_t theTotalBytes);

	void AddBlockRangeExtent(off_t blockAddr, off_t numBlocks);
	void AddByteRangeExtent(off_t byteAddr, off_t numBytes);
	void RemoveBlockRangeExtent(off_t blockAddr, off_t numBlocks);

	void DebugPrint();

protected:
	void MergeExtent(const ExtentInfo &a, const ExtentInfo &b, ExtentInfo *c);

public:
	size_t blockSize;
	size_t nativeBlockSize;
	off_t totalBytes;
	off_t totalBlocks;
	list<ExtentInfo> extentList;
};

#endif // #ifndef EXTENTMANAGER_H
