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


#include <PDFKit/PDFKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* PDFPointEqualToPoint(void)
{
    if (verbose) puts("STUB: PDFPointEqualToPoint called");
    return NULL;
}

void* PDFPointFromCGPoint(void)
{
    if (verbose) puts("STUB: PDFPointFromCGPoint called");
    return NULL;
}

void* PDFPointMake(void)
{
    if (verbose) puts("STUB: PDFPointMake called");
    return NULL;
}

void* PDFPointToCGPoint(void)
{
    if (verbose) puts("STUB: PDFPointToCGPoint called");
    return NULL;
}

void* PDFRectContainsPoint(void)
{
    if (verbose) puts("STUB: PDFRectContainsPoint called");
    return NULL;
}

void* PDFRectContainsRect(void)
{
    if (verbose) puts("STUB: PDFRectContainsRect called");
    return NULL;
}

void* PDFRectEqualToRect(void)
{
    if (verbose) puts("STUB: PDFRectEqualToRect called");
    return NULL;
}

void* PDFRectFromCGRect(void)
{
    if (verbose) puts("STUB: PDFRectFromCGRect called");
    return NULL;
}

void* PDFRectGetMaxX(void)
{
    if (verbose) puts("STUB: PDFRectGetMaxX called");
    return NULL;
}

void* PDFRectGetMaxY(void)
{
    if (verbose) puts("STUB: PDFRectGetMaxY called");
    return NULL;
}

void* PDFRectGetMidX(void)
{
    if (verbose) puts("STUB: PDFRectGetMidX called");
    return NULL;
}

void* PDFRectGetMidY(void)
{
    if (verbose) puts("STUB: PDFRectGetMidY called");
    return NULL;
}

void* PDFRectGetMinX(void)
{
    if (verbose) puts("STUB: PDFRectGetMinX called");
    return NULL;
}

void* PDFRectGetMinY(void)
{
    if (verbose) puts("STUB: PDFRectGetMinY called");
    return NULL;
}

void* PDFRectInset(void)
{
    if (verbose) puts("STUB: PDFRectInset called");
    return NULL;
}

void* PDFRectIntegral(void)
{
    if (verbose) puts("STUB: PDFRectIntegral called");
    return NULL;
}

void* PDFRectIntersection(void)
{
    if (verbose) puts("STUB: PDFRectIntersection called");
    return NULL;
}

void* PDFRectIntersectsRect(void)
{
    if (verbose) puts("STUB: PDFRectIntersectsRect called");
    return NULL;
}

void* PDFRectIsEmpty(void)
{
    if (verbose) puts("STUB: PDFRectIsEmpty called");
    return NULL;
}

void* PDFRectMake(void)
{
    if (verbose) puts("STUB: PDFRectMake called");
    return NULL;
}

void* PDFRectOffset(void)
{
    if (verbose) puts("STUB: PDFRectOffset called");
    return NULL;
}

void* PDFRectToCGRect(void)
{
    if (verbose) puts("STUB: PDFRectToCGRect called");
    return NULL;
}

void* PDFRectUnion(void)
{
    if (verbose) puts("STUB: PDFRectUnion called");
    return NULL;
}

void* PDFSizeFromCGSize(void)
{
    if (verbose) puts("STUB: PDFSizeFromCGSize called");
    return NULL;
}

void* PDFSizeMake(void)
{
    if (verbose) puts("STUB: PDFSizeMake called");
    return NULL;
}

void* PDFSizeToCGSize(void)
{
    if (verbose) puts("STUB: PDFSizeToCGSize called");
    return NULL;
}
