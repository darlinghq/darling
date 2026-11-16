/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// Access to the instance variables of the AppKit text classes that UIFoundation
// implements. On the 32-bit (fragile) Objective-C ABI AppKit's headers keep
// the variables of NSGlyphInfo, NSParagraphStyle, NSTextAttachment,
// NSTextBlock, NSTextTable, NSTextTableBlock and NSTypesetter in an extra-state
// structure (NS_EXTRA, see AppKit/AppKitIvarLayout.h) and say so by defining
// _NS_HAS_EXTRA_IVARS_<class>. An AppKit that does not define it declares the
// variables in the class itself, and they are read directly. UIF_EXTRA_<class>(obj)
// is the object whose members are the variables, UIF_EXTRA_FREE the release of
// the structure (right before [super dealloc]) and UIF_EXTRA_DEALLOC a -dealloc
// for a class that has none.

#import <AppKit/AppKitIvarLayout.h>
#import <AppKit/NSGlyphInfo.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSTextAttachment.h>
#import <AppKit/NSTextBlock.h>
#import <AppKit/NSTextTable.h>
#import <AppKit/NSTextTableBlock.h>
#import <AppKit/NSTypesetter.h>

#ifdef _NS_HAS_EXTRA_IVARS_NSGlyphInfo
#define UIF_EXTRA_NSGlyphInfo(obj) NS_EXTRA(NSGlyphInfo, obj)
#define UIF_EXTRA_FREE_NSGlyphInfo(obj) NS_EXTRA_FREE(NSGlyphInfo, obj)
#define UIF_EXTRA_DEALLOC_NSGlyphInfo NS_EXTRA_DEALLOC(NSGlyphInfo)
#else
#define UIF_EXTRA_NSGlyphInfo(obj) ((NSGlyphInfo *) (obj))
#define UIF_EXTRA_FREE_NSGlyphInfo(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSGlyphInfo
#endif

#ifdef _NS_HAS_EXTRA_IVARS_NSParagraphStyle
#define UIF_EXTRA_NSParagraphStyle(obj) NS_EXTRA(NSParagraphStyle, obj)
#define UIF_EXTRA_FREE_NSParagraphStyle(obj) NS_EXTRA_FREE(NSParagraphStyle, obj)
#define UIF_EXTRA_DEALLOC_NSParagraphStyle NS_EXTRA_DEALLOC(NSParagraphStyle)
#else
#define UIF_EXTRA_NSParagraphStyle(obj) ((NSParagraphStyle *) (obj))
#define UIF_EXTRA_FREE_NSParagraphStyle(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSParagraphStyle
#endif

#ifdef _NS_HAS_EXTRA_IVARS_NSTextAttachment
#define UIF_EXTRA_NSTextAttachment(obj) NS_EXTRA(NSTextAttachment, obj)
#define UIF_EXTRA_FREE_NSTextAttachment(obj) NS_EXTRA_FREE(NSTextAttachment, obj)
#define UIF_EXTRA_DEALLOC_NSTextAttachment NS_EXTRA_DEALLOC(NSTextAttachment)
#else
#define UIF_EXTRA_NSTextAttachment(obj) ((NSTextAttachment *) (obj))
#define UIF_EXTRA_FREE_NSTextAttachment(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSTextAttachment
#endif

#ifdef _NS_HAS_EXTRA_IVARS_NSTextBlock
#define UIF_EXTRA_NSTextBlock(obj) NS_EXTRA(NSTextBlock, obj)
#define UIF_EXTRA_FREE_NSTextBlock(obj) NS_EXTRA_FREE(NSTextBlock, obj)
#define UIF_EXTRA_DEALLOC_NSTextBlock NS_EXTRA_DEALLOC(NSTextBlock)
#else
#define UIF_EXTRA_NSTextBlock(obj) ((NSTextBlock *) (obj))
#define UIF_EXTRA_FREE_NSTextBlock(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSTextBlock
#endif

#ifdef _NS_HAS_EXTRA_IVARS_NSTextTable
#define UIF_EXTRA_NSTextTable(obj) NS_EXTRA(NSTextTable, obj)
#define UIF_EXTRA_FREE_NSTextTable(obj) NS_EXTRA_FREE(NSTextTable, obj)
#define UIF_EXTRA_DEALLOC_NSTextTable NS_EXTRA_DEALLOC(NSTextTable)
#else
#define UIF_EXTRA_NSTextTable(obj) ((NSTextTable *) (obj))
#define UIF_EXTRA_FREE_NSTextTable(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSTextTable
#endif

#ifdef _NS_HAS_EXTRA_IVARS_NSTextTableBlock
#define UIF_EXTRA_NSTextTableBlock(obj) NS_EXTRA(NSTextTableBlock, obj)
#define UIF_EXTRA_FREE_NSTextTableBlock(obj) NS_EXTRA_FREE(NSTextTableBlock, obj)
#define UIF_EXTRA_DEALLOC_NSTextTableBlock NS_EXTRA_DEALLOC(NSTextTableBlock)
#else
#define UIF_EXTRA_NSTextTableBlock(obj) ((NSTextTableBlock *) (obj))
#define UIF_EXTRA_FREE_NSTextTableBlock(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSTextTableBlock
#endif

#ifdef _NS_HAS_EXTRA_IVARS_NSTypesetter
#define UIF_EXTRA_NSTypesetter(obj) NS_EXTRA(NSTypesetter, obj)
#define UIF_EXTRA_FREE_NSTypesetter(obj) NS_EXTRA_FREE(NSTypesetter, obj)
#define UIF_EXTRA_DEALLOC_NSTypesetter NS_EXTRA_DEALLOC(NSTypesetter)
#else
#define UIF_EXTRA_NSTypesetter(obj) ((NSTypesetter *) (obj))
#define UIF_EXTRA_FREE_NSTypesetter(obj) do { } while (0)
#define UIF_EXTRA_DEALLOC_NSTypesetter
#endif

#define UIF_EXTRA_FREE(cls, obj) UIF_EXTRA_FREE_##cls(obj)
#define UIF_EXTRA_DEALLOC(cls) UIF_EXTRA_DEALLOC_##cls
