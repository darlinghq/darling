/*
 * Copyright (c) 2008-2009,2011 Apple Inc. All rights reserved.
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
//	ExtentManager.cpp
//

#include "ExtentManager.h"

void
ExtentManager::Init(uint32_t theBlockSize, uint32_t theNativeBlockSize, off_t theTotalBytes)
{
	blockSize = theBlockSize;
	nativeBlockSize = theNativeBlockSize;
	totalBytes = theTotalBytes;
	totalBlocks = howmany(totalBytes, blockSize);

	// add sentry empty extents at both sides so empty partition doesn't need to be handled specially
	AddBlockRangeExtent(0, 0);
	AddBlockRangeExtent(totalBlocks, 0);
}

void
ExtentManager::MergeExtent(const ExtentInfo &a, const ExtentInfo &b, ExtentInfo *c)
{
	// merge ext into *curIt
	c->blockAddr = min(a.blockAddr, b.blockAddr);
	c->numBlocks = max(a.blockAddr + a.numBlocks, b.blockAddr + b.numBlocks) - c->blockAddr;
}

void
ExtentManager::AddBlockRangeExtent(off_t blockAddr, off_t numBlocks)
{
	struct ExtentInfo ext, newExt;
	ListExtIt curIt, newIt;
	bool merged = false;

	// make the range a valid range
	if ((blockAddr > totalBlocks) || (blockAddr + numBlocks < 0)) { // totally out of range, do nothing
		return;
	}
	if (blockAddr < 0) {
		numBlocks = blockAddr + numBlocks;
		blockAddr = 0;
	}
	if (blockAddr + numBlocks > totalBlocks) {
		numBlocks = totalBlocks - blockAddr;
	}

	ext.blockAddr = blockAddr;
	ext.numBlocks = numBlocks;

	for (curIt = extentList.begin(); curIt != extentList.end(); curIt++) {
		if (BeforeExtent(ext, *curIt))
			break;
		if (!BeforeExtent(*curIt, ext)) { // overlapped extents
			MergeExtent(ext, *curIt, &newExt);
			*curIt = newExt;
			merged = true;
			break;
		}
	}

	// insert ext before curIt
	if (!merged) {
		curIt = extentList.insert(curIt, ext); // throws bad_alloc when out of memory
	}

	// merge the extents
	newIt = curIt;
	curIt = extentList.begin();
	while (curIt != extentList.end()) {
		if (curIt == newIt || BeforeExtent(*curIt, *newIt)) { // curIt is before newIt
			curIt++;
			continue;
		}
		if (BeforeExtent(*newIt, *curIt)) { // curIt is after newIt now, we are done
			break;
		}
		// merge the two extents
		MergeExtent(*curIt, *newIt, &newExt);
		*newIt = newExt;
		curIt = extentList.erase(curIt);
	}
	// printf("After %s(%lld, %lld)\n", __func__, blockAddr, numBlocks);	 DebugPrint();
} // ExtentManager::AddBlockRangeExtent

void
ExtentManager::RemoveBlockRangeExtent(off_t blockAddr, off_t numBlocks)
{
	struct ExtentInfo ext, newExt;
	ListExtIt curIt;

	ext.blockAddr = blockAddr;
	ext.numBlocks = numBlocks;

	curIt = extentList.begin();
	while (curIt != extentList.end()) {
		if (BeforeExtent(*curIt, ext)) {
			curIt++;
			continue;
		}
		if (BeforeExtent(ext, *curIt)) // we are done
			break;
		
		//
		// If we get here, the input extent and *curIt have at least one block in common.
		// That is, they overlap in some way.  Thus *curIt needs to change, be removed,
		// or be split into two non-contiguous extents.
		//
		
		if (curIt->blockAddr >= ext.blockAddr &&
			curIt->blockAddr + curIt->numBlocks <= ext.blockAddr + ext.numBlocks) {
			//
			// The input extent totally contains *curIt, so remove *curIt.
			//
			curIt = extentList.erase(curIt);
		} else if (curIt->blockAddr < ext.blockAddr &&
			curIt->blockAddr + curIt->numBlocks > ext.blockAddr + ext.numBlocks) {
			//
			// The input extent does not include the start of *curIt, nor the end of *curIt,
			// so split *curIt into two extents.
			//
			newExt.blockAddr = ext.blockAddr + ext.numBlocks;
			newExt.numBlocks = curIt->blockAddr + curIt->numBlocks - newExt.blockAddr;
			curIt->numBlocks = ext.blockAddr - curIt->blockAddr;
			curIt++;
			extentList.insert(curIt, newExt); // throws bad_alloc when out of memory
			curIt++;	
		} else {
			//
			// The input extent contains either the start or the end of *curIt, but not both.
			// The remove will leave either the end or the start of *curIt (respectively) and
			// not change the number of extents in the list.
			//
			if (curIt->blockAddr >= ext.blockAddr) {
				//
				// Remove the start of *curIt by updating both its starting block and size.
				//
				assert(curIt->blockAddr + curIt->numBlocks > ext.blockAddr + ext.numBlocks);
				newExt.blockAddr = ext.blockAddr + ext.numBlocks;
				newExt.numBlocks = curIt->blockAddr + curIt->numBlocks - newExt.blockAddr;
				*curIt = newExt;
			} else {
				//
				// Remove the end of *curIt by updating its size.
				//
				curIt->numBlocks = ext.blockAddr - curIt->blockAddr;
			}
			curIt++;
		}
	}
	//printf("After %s(%lld, %lld)\n", __func__, blockAddr, numBlocks);	 DebugPrint();
}

void
ExtentManager::AddByteRangeExtent(off_t byteAddr, off_t numBytes)
{
	off_t blockAddr = byteAddr / blockSize;
	off_t blockAddrOfLastByte = (byteAddr + numBytes - 1) / blockSize;
	off_t numBlocks = blockAddrOfLastByte - blockAddr + 1;
	AddBlockRangeExtent(blockAddr, numBlocks);
}

void
ExtentManager::DebugPrint()
{
	ListExtIt it;

	for (it = extentList.begin(); it != extentList.end(); it++) {
		printf("[%lld, %lld] ", it->blockAddr, it->numBlocks);
	}
	printf("\n");
}


#if UNIT_TEST

/*
clang++ -arch i386 -arch x86_64 -DUNIT_TEST ExtentManager.cpp -o ExtentManager && ./ExtentManager
*/

#include <cstdio>
#include <cstdlib>

const char *DebugDescription(class ExtentManager *extMan)
{
	char *result = strdup("");
	char *temp;
	
	ListExtIt it;

	for (it = extMan->extentList.begin(); it != extMan->extentList.end(); it++) {
		temp = result;
		asprintf(&result, "%s[%lld, %lld] ", temp, it->blockAddr, it->numBlocks);
		free(temp);
	}
	
	return result;
}

int SimpleTestCase(off_t addAddr, off_t addBlocks, off_t removeAddr, off_t removeBlocks, const char *expectedResult)
{
	class ExtentManager extMan;
	const char *actualResult;
	int result = 0;
	
	extMan.Init(512, 512, 512*999);
	extMan.AddBlockRangeExtent(addAddr, addBlocks);
	extMan.RemoveBlockRangeExtent(removeAddr, removeBlocks);
	actualResult = DebugDescription(&extMan);
	if (strcmp(actualResult, expectedResult))
	{
		fprintf(stderr,
				"SimpleTestCase(%lld, %lld, %lld, %lld) failed.\n"
				"    Expected result: %s\n"
				"      Actual result: %s\n",
				addAddr, addBlocks, removeAddr, removeBlocks,
				expectedResult, actualResult);
		result = 1;
	}
	free((void *)actualResult);
	
	return result;
}

int main(void)
{
	int failed = 0;
	class ExtentManager *extMan;
	
	// Create an extent, and remove one contained inside,
	// leaving the start and end of the original extent.
	// Create: [xxxxxxxxxx]
	// Remove:   [......]
	failed |= SimpleTestCase(10, 10, 12, 6, "[0, 0] [10, 2] [18, 2] [999, 0] ");

	// Create an extent, and remove the whole extent.
	// Create: [xxxxxxxxxx]
	// Remove: [..........]
	failed |= SimpleTestCase(10, 10, 10, 10, "[0, 0] [999, 0] ");
	
	// Create an extent, and remove the first part of the extent.
	// Create: [xxxxxxxxxx]
	// Remove: [......]
	failed |= SimpleTestCase(10, 10, 10, 6, "[0, 0] [16, 4] [999, 0] ");

	// Create an extent, and remove the last part of the extent.
	// Create: [xxxxxxxxxx]
	// Remove:     [......]
	failed |= SimpleTestCase(10, 10, 14, 6, "[0, 0] [10, 4] [999, 0] ");
	
	// Create an extent and remove before the start, through the middle.
	// Create:     [xxxxxxxxxx]
	// Remove: [..........]
	failed |= SimpleTestCase(10, 10, 6, 10, "[0, 0] [16, 4] [999, 0] ");

	// Create an extent and remove from middle to past the end.
	// Create: [xxxxxxxxxx]
	// Remove:     [..........]
	failed |= SimpleTestCase(10, 10, 14, 10, "[0, 0] [10, 4] [999, 0] ");
	
	// Create an extent and remove from before through past end.
	// Create:   [xxxxxxxxxx]
	// Remove: [..............]
	failed |= SimpleTestCase(10, 10, 6, 18, "[0, 0] [999, 0] ");
	
	// Create an extent and remove purely before the extent.
	// Create:      [xxxxxxxxxx]
	// Remove: [...]
	failed |= SimpleTestCase(10, 10, 2, 5, "[0, 0] [10, 10] [999, 0] ");
	
	// Create an extent and remove purely after the extent.
	// Create: [xxxxxxxxxx]
	// Remove:              [...]
	failed |= SimpleTestCase(10, 10, 22, 5, "[0, 0] [10, 10] [999, 0] ");
	
	if (failed)
		printf("FAIL!\n");
	else
		printf("Success.\n");
	
	return failed;
}

#endif /* UNIT_TEST */
