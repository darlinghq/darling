/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <LaunchServices/IconsCore.h>

OSErr AcquireIconRef(IconRef theIconRef)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

OSErr GetIconRef(SInt16 vRefNum, OSType creator, OSType iconType, IconRef *theIconRef)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

// GUESS: the public page gives only the declaration and the deprecation, not the results. Unlike its
// neighbours in this file, which return 0 without writing their outputs, this one fails loudly: it
// returns unimpErr and clears the outputs, so a caller never uses an IconRef that does not exist.
OSStatus GetIconRefFromFileInfo(const FSRef *inRef, UniCharCount inFileNameLength, const UniChar *inFileName, FSCatalogInfoBitmap inWhichInfo, const FSCatalogInfo *inCatalogInfo, IconServicesUsageFlags inUsageFlags, IconRef *outIconRef, SInt16 *outLabel)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    if (outIconRef != NULL)
        *outIconRef = NULL;
    if (outLabel != NULL)
        *outLabel = 0;

    return unimpErr;
}

OSStatus GetIconRefFromIconFamilyPtr(const IconFamilyResource *inIconFamilyPtr, Size inSize, IconRef *outIconRef)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

OSStatus ReadIconFromFSRef(const FSRef *ref, IconFamilyHandle *iconFamily)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

OSErr ReleaseIconRef(IconRef theIconRef)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}
