/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

// Minimal TextKit 2 (NSTextRange, NSTextContentManager, NSTextContentStorage,
// NSTextLayoutManager), written from the public AppKit documentation. The
// content lives in an NSTextStorage and layout is done by a TextKit 1
// NSLayoutManager, so apps that create TextKit 2 objects can bind and use the
// basics; there are no text elements, text layout fragments or viewport layout.

#import <AppKit/NSAttributedString.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextContentManager.h>
#import <AppKit/NSTextLayoutManager.h>
#import <AppKit/NSTextRange.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSTextView.h>

NSString *const NSTextContentStorageUnsupportedAttributeAddedNotification =
        @"NSTextContentStorageUnsupportedAttributeAddedNotification";
NSAttributedStringDocumentReadingOptionKey const
        NSTextKit1ListMarkerFormatDocumentOption =
                @"NSTextKit1ListMarkerFormatDocumentOption";

#pragma mark - Locations

// A character offset into an NSTextContentStorage.
@interface _NSTextOffsetLocation : NSObject <NSTextLocation, NSCopying> {
    NSUInteger _offset;
}
+ (instancetype) locationWithOffset: (NSUInteger) offset;
- (NSUInteger) offset;
@end

@implementation _NSTextOffsetLocation

+ (instancetype) locationWithOffset: (NSUInteger) offset {
    _NSTextOffsetLocation *location = [[[self alloc] init] autorelease];
    location->_offset = offset;
    return location;
}

- (NSUInteger) offset {
    return _offset;
}

- (NSComparisonResult) compare: (id<NSTextLocation>) location {
    if (![(id) location isKindOfClass: [_NSTextOffsetLocation class]])
        return (NSComparisonResult) (-[location compare: self]);

    NSUInteger other = ((_NSTextOffsetLocation *) location)->_offset;
    if (_offset < other)
        return NSOrderedAscending;
    if (_offset > other)
        return NSOrderedDescending;
    return NSOrderedSame;
}

- (BOOL) isEqual: (id) object {
    return [object isKindOfClass: [_NSTextOffsetLocation class]] &&
           ((_NSTextOffsetLocation *) object)->_offset == _offset;
}

- (NSUInteger) hash {
    return _offset;
}

- (id) copyWithZone: (NSZone *) zone {
    return [self retain];
}

- (NSString *) description {
    return [NSString stringWithFormat: @"%lu", (unsigned long) _offset];
}

@end

#pragma mark - NSTextRange

@implementation NSTextRange

- (instancetype) initWithLocation: (id<NSTextLocation>) location
                      endLocation: (id<NSTextLocation>) endLocation
{
    self = [super init];
    if (self == nil)
        return nil;

    if (location == nil) {
        [self release];
        return nil;
    }
    if (endLocation == nil)
        endLocation = location;
    if ([location compare: endLocation] == NSOrderedDescending) {
        [self release];
        return nil;
    }

    _location = [(id) location retain];
    _endLocation = [(id) endLocation retain];
    return self;
}

- (instancetype) initWithLocation: (id<NSTextLocation>) location {
    return [self initWithLocation: location endLocation: nil];
}

- (void) dealloc {
    [(id) _location release];
    [(id) _endLocation release];
    [super dealloc];
}

- (id<NSTextLocation>) location {
    return _location;
}

- (id<NSTextLocation>) endLocation {
    return _endLocation;
}

- (BOOL) isEmpty {
    return [_location compare: _endLocation] == NSOrderedSame;
}

- (BOOL) isEqualToTextRange: (NSTextRange *) textRange {
    return textRange != nil &&
           [_location compare: textRange.location] == NSOrderedSame &&
           [_endLocation compare: textRange.endLocation] == NSOrderedSame;
}

- (BOOL) isEqual: (id) object {
    return [object isKindOfClass: [NSTextRange class]] &&
           [self isEqualToTextRange: object];
}

- (NSUInteger) hash {
    return [(id) _location hash] ^ ([(id) _endLocation hash] << 1);
}

- (BOOL) containsLocation: (id<NSTextLocation>) location {
    return [location compare: _location] != NSOrderedAscending &&
           [location compare: _endLocation] == NSOrderedAscending;
}

- (BOOL) containsRange: (NSTextRange *) textRange {
    return [textRange.location compare: _location] != NSOrderedAscending &&
           [textRange.endLocation compare: _endLocation] !=
                   NSOrderedDescending;
}

- (BOOL) intersectsWithTextRange: (NSTextRange *) textRange {
    return [_location compare: textRange.endLocation] == NSOrderedAscending &&
           [textRange.location compare: _endLocation] == NSOrderedAscending;
}

- (NSTextRange *) textRangeByIntersectingWithTextRange:
        (NSTextRange *) textRange
{
    id<NSTextLocation> start =
            [_location compare: textRange.location] == NSOrderedAscending
                    ? textRange.location
                    : _location;
    id<NSTextLocation> end =
            [_endLocation compare: textRange.endLocation] == NSOrderedDescending
                    ? textRange.endLocation
                    : _endLocation;

    if ([start compare: end] == NSOrderedDescending)
        return nil;
    return [[[NSTextRange alloc] initWithLocation: start endLocation: end]
            autorelease];
}

- (NSTextRange *) textRangeByFormingUnionWithTextRange:
        (NSTextRange *) textRange
{
    id<NSTextLocation> start =
            [_location compare: textRange.location] == NSOrderedDescending
                    ? textRange.location
                    : _location;
    id<NSTextLocation> end =
            [_endLocation compare: textRange.endLocation] == NSOrderedAscending
                    ? textRange.endLocation
                    : _endLocation;

    return [[[NSTextRange alloc] initWithLocation: start endLocation: end]
            autorelease];
}

- (id) copyWithZone: (NSZone *) zone {
    return [self retain];
}

- (NSString *) description {
    return [NSString stringWithFormat: @"<%@ %p> %@-%@", [self class], self,
                                       _location, _endLocation];
}

@end

#pragma mark - NSTextLayoutManager (private)

@interface NSTextLayoutManager (NSTextContentManagerPrivate)
- (void) _setTextContentManager: (NSTextContentManager *) manager;
@end

#pragma mark - NSTextContentManager

@implementation NSTextContentManager

- (instancetype) init {
    self = [super init];
    if (self != nil) {
        _textLayoutManagers = [[NSMutableArray alloc] init];
        _automaticallySynchronizesTextLayoutManagers = YES;
        _automaticallySynchronizesToBackingStore = YES;
    }
    return self;
}

- (void) dealloc {
    for (NSTextLayoutManager *manager in _textLayoutManagers)
        [manager _setTextContentManager: nil];
    [_textLayoutManagers release];
    [super dealloc];
}

- (NSArray *) textLayoutManagers {
    return [[_textLayoutManagers copy] autorelease];
}

- (NSTextLayoutManager *) primaryTextLayoutManager {
    return _primaryTextLayoutManager;
}

- (void) setPrimaryTextLayoutManager: (NSTextLayoutManager *) manager {
    if (manager == nil ||
        [_textLayoutManagers indexOfObjectIdenticalTo: manager] != NSNotFound)
        _primaryTextLayoutManager = manager;
}

- (BOOL) automaticallySynchronizesTextLayoutManagers {
    return _automaticallySynchronizesTextLayoutManagers;
}

- (void) setAutomaticallySynchronizesTextLayoutManagers: (BOOL) value {
    _automaticallySynchronizesTextLayoutManagers = value;
}

- (BOOL) automaticallySynchronizesToBackingStore {
    return _automaticallySynchronizesToBackingStore;
}

- (void) setAutomaticallySynchronizesToBackingStore: (BOOL) value {
    _automaticallySynchronizesToBackingStore = value;
}

- (BOOL) includesTextListMarkers {
    return _includesTextListMarkers;
}

- (void) setIncludesTextListMarkers: (BOOL) value {
    _includesTextListMarkers = value;
}

- (void) addTextLayoutManager: (NSTextLayoutManager *) manager {
    if (manager == nil ||
        [_textLayoutManagers indexOfObjectIdenticalTo: manager] != NSNotFound)
        return;

    // The old content manager may hold the only reference, so keep the manager
    // alive while it moves.
    [manager retain];
    [manager.textContentManager removeTextLayoutManager: manager];
    [_textLayoutManagers addObject: manager];
    if (_primaryTextLayoutManager == nil)
        _primaryTextLayoutManager = manager;
    [manager _setTextContentManager: self];
    [manager release];
}

- (void) removeTextLayoutManager: (NSTextLayoutManager *) manager {
    NSUInteger index = [_textLayoutManagers indexOfObjectIdenticalTo: manager];
    if (index == NSNotFound)
        return;

    [manager retain];
    [_textLayoutManagers removeObjectAtIndex: index];
    if (_primaryTextLayoutManager == manager)
        _primaryTextLayoutManager = [_textLayoutManagers firstObject];
    [manager _setTextContentManager: nil];
    [manager release];
}

- (NSTextRange *) documentRange {
    return [[[NSTextRange alloc]
            initWithLocation: [_NSTextOffsetLocation locationWithOffset: 0]]
            autorelease];
}

- (id<NSTextLocation>) locationFromLocation: (id<NSTextLocation>) location
                                 withOffset: (NSInteger) offset
{
    return nil;
}

- (NSInteger) offsetFromLocation: (id<NSTextLocation>) from
                      toLocation: (id<NSTextLocation>) to
{
    return 0;
}

- (BOOL) hasEditingTransaction {
    return _editingTransactionDepth > 0;
}

- (void) performEditingTransactionUsingBlock: (void (^)(void)) transaction {
    _editingTransactionDepth++;
    @try {
        if (transaction)
            transaction();
    } @finally {
        _editingTransactionDepth--;
    }
}

@end

#pragma mark - NSTextContentStorage

@implementation NSTextContentStorage

- (instancetype) init {
    self = [super init];
    if (self != nil)
        _textStorage = [[NSTextStorage alloc] init];
    return self;
}

- (void) dealloc {
    // Detach layout managers while the text storage is still alive.
    for (NSTextLayoutManager *manager in [self textLayoutManagers])
        [self removeTextLayoutManager: manager];
    [_textStorage release];
    [super dealloc];
}

- (NSTextStorage *) textStorage {
    return _textStorage;
}

- (void) setTextStorage: (NSTextStorage *) textStorage {
    if (textStorage == _textStorage)
        return;

    [textStorage retain];
    // Move each layout manager's TextKit 1 layout manager to the new storage.
    for (NSTextLayoutManager *manager in _textLayoutManagers)
        [manager _setTextContentManager: nil];
    [_textStorage release];
    _textStorage = textStorage;
    for (NSTextLayoutManager *manager in _textLayoutManagers)
        [manager _setTextContentManager: self];
}

- (NSAttributedString *) attributedString {
    return _textStorage;
}

- (void) setAttributedString: (NSAttributedString *) attributedString {
    if (_textStorage == nil)
        [self setTextStorage: [[[NSTextStorage alloc] init] autorelease]];

    NSAttributedString *value = attributedString
                                        ?: [[[NSAttributedString alloc]
                                                   initWithString: @""]
                                                   autorelease];
    [_textStorage beginEditing];
    [_textStorage
            replaceCharactersInRange: NSMakeRange(0, [_textStorage length])
                withAttributedString: value];
    [_textStorage endEditing];
}

- (NSTextRange *) documentRange {
    return [[[NSTextRange alloc]
            initWithLocation: [_NSTextOffsetLocation locationWithOffset: 0]
                 endLocation: [_NSTextOffsetLocation
                                      locationWithOffset: [_textStorage
                                                                  length]]]
            autorelease];
}

- (id<NSTextLocation>) locationFromLocation: (id<NSTextLocation>) location
                                 withOffset: (NSInteger) offset
{
    if (![(id) location isKindOfClass: [_NSTextOffsetLocation class]])
        return nil;

    NSInteger target =
            (NSInteger) ((_NSTextOffsetLocation *) location).offset + offset;
    if (target < 0 || target > (NSInteger) [_textStorage length])
        return nil;
    return [_NSTextOffsetLocation locationWithOffset: (NSUInteger) target];
}

- (NSInteger) offsetFromLocation: (id<NSTextLocation>) from
                      toLocation: (id<NSTextLocation>) to
{
    if (![(id) from isKindOfClass: [_NSTextOffsetLocation class]] ||
        ![(id) to isKindOfClass: [_NSTextOffsetLocation class]])
        return 0;
    return (NSInteger) ((_NSTextOffsetLocation *) to).offset -
           (NSInteger) ((_NSTextOffsetLocation *) from).offset;
}

@end

#pragma mark - NSTextLayoutManager

@implementation NSTextLayoutManager

- (instancetype) init {
    self = [super init];
    if (self != nil)
        _layoutManager = [[NSLayoutManager alloc] init];
    return self;
}

- (void) dealloc {
    [_textContentManager removeTextLayoutManager: self];
    [[_layoutManager textStorage] removeLayoutManager: _layoutManager];
    [_textContainer release];
    [_templateTextContainer release];
    [_layoutManager release];
    [super dealloc];
}

- (NSTextContentManager *) textContentManager {
    return _textContentManager;
}

// Called by NSTextContentManager when this layout manager is added or removed.
- (void) _setTextContentManager: (NSTextContentManager *) manager {
    NSTextStorage *oldStorage = [_layoutManager textStorage];
    NSTextStorage *newStorage = nil;

    if ([manager isKindOfClass: [NSTextContentStorage class]])
        newStorage = ((NSTextContentStorage *) manager).textStorage;

    if (oldStorage != newStorage) {
        [_layoutManager retain];
        [oldStorage removeLayoutManager: _layoutManager];
        [newStorage addLayoutManager: _layoutManager];
        [_layoutManager release];
    }
    _textContentManager = manager;
}

- (void) replaceTextContentManager: (NSTextContentManager *) manager {
    NSTextContentManager *old = _textContentManager;
    NSArray *managers = old ? [old textLayoutManagers] : @[ self ];

    for (NSTextLayoutManager *layoutManager in managers) {
        [old removeTextLayoutManager: layoutManager];
        [manager addTextLayoutManager: layoutManager];
    }
}

- (NSTextContainer *) textContainer {
    return _textContainer;
}

- (NSArray *) textContainers {
    return _textContainer ? [NSArray arrayWithObject: _textContainer]
                          : [NSArray array];
}

- (void) setTextContainer: (NSTextContainer *) container {
    if (container == _textContainer)
        return;

    [container retain];
    if (_textContainer != nil &&
        [_layoutManager respondsToSelector: @selector
                        (removeTextContainerAtIndex:)]) {
        NSUInteger index = [[_layoutManager textContainers]
                indexOfObjectIdenticalTo: _textContainer];
        if (index != NSNotFound)
            [_layoutManager removeTextContainerAtIndex: index];
    }
    [_textContainer release];
    _textContainer = container;
    if (container != nil)
        [_layoutManager addTextContainer: container];
}

- (NSRect) usageBoundsForTextContainer {
    if (_textContainer == nil)
        return NSZeroRect;
    [_layoutManager glyphRangeForTextContainer: _textContainer];
    return [_layoutManager usedRectForTextContainer: _textContainer];
}

- (NSTextRange *) documentRange {
    return [_textContentManager documentRange];
}

- (void) ensureLayoutForRange: (NSTextRange *) range {
    if (_textContainer != nil)
        [_layoutManager glyphRangeForTextContainer: _textContainer];
}

- (NSTextContainer *) templateTextContainer {
    return _templateTextContainer;
}

- (void) setTemplateTextContainer: (NSTextContainer *) container {
    [container retain];
    [_templateTextContainer release];
    _templateTextContainer = container;
}

- (BOOL) usesDefaultHyphenation {
    return _usesDefaultHyphenation;
}

- (void) setUsesDefaultHyphenation: (BOOL) flag {
    _usesDefaultHyphenation = flag;
}

- (NSTextRange *) rangeForTextContainerAtIndex: (NSUInteger) index {
    if (index != 0)
        return nil;
    if (_textContainer == nil)
        return [self documentRange];

    NSRange glyphs = [_layoutManager glyphRangeForTextContainer: _textContainer];
    NSRange characters = [_layoutManager characterRangeForGlyphRange: glyphs
                                                     actualGlyphRange: NULL];
    if (characters.length == 0)
        return [self documentRange];
    return [[[NSTextRange alloc]
            initWithLocation: [_NSTextOffsetLocation
                                      locationWithOffset: characters.location]
                 endLocation: [_NSTextOffsetLocation
                                      locationWithOffset: NSMaxRange(characters)]]
            autorelease];
}

// Locations don't map to glyphs here, so the whole document is invalidated.
- (void) invalidateLayoutForRange: (NSTextRange *) range {
    NSUInteger length = [[_layoutManager textStorage] length];
    [_layoutManager invalidateLayoutForCharacterRange: NSMakeRange(0, length)
                                               isSoft: NO
                                 actualCharacterRange: NULL];
}

@end

#pragma mark - NSTextView

@implementation NSTextView (NSTextKit2)

+ (instancetype) textViewUsingTextLayoutManager: (BOOL) usingTextLayoutManager {
    // Only TextKit 1 text views exist here.
    return [[[self alloc] initWithFrame: NSZeroRect] autorelease];
}

- (NSTextLayoutManager *) textLayoutManager {
    return nil;
}

- (NSTextContentStorage *) textContentStorage {
    return nil;
}

@end
