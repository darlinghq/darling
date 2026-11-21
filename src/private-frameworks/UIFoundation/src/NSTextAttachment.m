/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import "UIFoundation-ExtraIvars.h"
#import <Foundation/NSFileWrapper.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSTextAttachment.h>
#import <UIFoundation/UIFoundationGraphicsContext.h>

// NSTextAttachmentViewProvider's initializer; this AppKit has no provider base
// class, so registered provider classes supply it themselves.
@protocol NSTextAttachmentViewProviderInit
- (instancetype) initWithTextAttachment: (NSTextAttachment *) textAttachment
                             parentView: (NSView *) parentView
                      textLayoutManager: (NSTextLayoutManager *) textLayoutManager
                               location: (id<NSTextLocation>) location;
@end

static NSMutableDictionary *viewProviderClasses;

@implementation NSTextAttachment
#if __OBJC2__
{
    _NSTEXTATTACHMENT_IVARS
}
#endif

+ (BOOL) supportsSecureCoding {
    return YES;
}

+ (Class) textAttachmentViewProviderClassForFileType: (NSString *) fileType {
    @synchronized([NSTextAttachment class]) {
        return [viewProviderClasses objectForKey: fileType];
    }
}

+ (void) registerTextAttachmentViewProviderClass: (Class) providerClass
                                     forFileType: (NSString *) fileType
{
    if (![providerClass instancesRespondToSelector: @selector(initWithTextAttachment:parentView:textLayoutManager:location:)])
        [NSException raise: NSInvalidArgumentException
                    format: @"%@ does not implement the text attachment view provider initializer", providerClass];
    @synchronized([NSTextAttachment class]) {
        if (viewProviderClasses == nil)
            viewProviderClasses = [[NSMutableDictionary alloc] init];
        [viewProviderClasses setObject: providerClass forKey: fileType];
    }
}

- (instancetype) initWithData: (NSData *) contentData ofType: (NSString *) uti {
    if ((self = [super init])) {
        UIF_EXTRA_NSTextAttachment(self)->_contents = [contentData copy];
        UIF_EXTRA_NSTextAttachment(self)->_fileType = [uti copy];
        UIF_EXTRA_NSTextAttachment(self)->_bounds = CGRectZero;
        UIF_EXTRA_NSTextAttachment(self)->_allowsTextAttachmentView = YES;
        /* Every other initializer here gives the attachment a cell, and -[NSLayoutManager
         * _drawAttachment:] only ever reaches the cell through -[NSTextAttachment attachmentCell].
         * Without this, an attachment created from data had a nil cell, so the layout manager drew
         * nothing for it at all -- silently, because messaging nil is a no-op. */
        // NSTextAttachmentCell is an AppKit class; AppKit makes the cell.
        UIF_EXTRA_NSTextAttachment(self)->_cell = _UIFoundationNewTextAttachmentCell();
        [UIF_EXTRA_NSTextAttachment(self)->_cell setAttachment: self];
    }
    return self;
}

- (instancetype) init {
    return [self initWithData: nil ofType: nil];
}

- (instancetype) initWithFileWrapper: (NSFileWrapper *) fileWrapper {
    if ((self = [self initWithData: nil ofType: nil])) {
        UIF_EXTRA_NSTextAttachment(self)->_fileWrapper = [fileWrapper retain];
    }
    return self;
}

- (instancetype) initWithCoder: (NSCoder *) coder {
    if (![coder allowsKeyedCoding])
        [NSException raise: NSInvalidArgumentException
                    format: @"-[%@ %s]: only keyed archiving is supported", [self class], sel_getName(_cmd)];
    self = [self initWithData: [coder decodeObjectOfClass: [NSData class] forKey: @"Contents"]
                       ofType: [coder decodeObjectOfClass: [NSString class] forKey: @"FileType"]];
    if (self) {
        UIF_EXTRA_NSTextAttachment(self)->_bounds = NSRectToCGRect([coder decodeRectForKey: @"Bounds"]);
        UIF_EXTRA_NSTextAttachment(self)->_lineLayoutPadding = [coder decodeDoubleForKey: @"LineLayoutPadding"];
        if ([coder containsValueForKey: @"AllowsTextAttachmentView"])
            UIF_EXTRA_NSTextAttachment(self)->_allowsTextAttachmentView = [coder decodeBoolForKey: @"AllowsTextAttachmentView"];
    }
    return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
    if (![coder allowsKeyedCoding])
        [NSException raise: NSInvalidArgumentException
                    format: @"-[%@ %s]: only keyed archiving is supported", [self class], sel_getName(_cmd)];
    [coder encodeObject: [self contents] forKey: @"Contents"];
    [coder encodeObject: UIF_EXTRA_NSTextAttachment(self)->_fileType forKey: @"FileType"];
    [coder encodeRect: NSRectFromCGRect(UIF_EXTRA_NSTextAttachment(self)->_bounds) forKey: @"Bounds"];
    [coder encodeDouble: UIF_EXTRA_NSTextAttachment(self)->_lineLayoutPadding forKey: @"LineLayoutPadding"];
    [coder encodeBool: UIF_EXTRA_NSTextAttachment(self)->_allowsTextAttachmentView forKey: @"AllowsTextAttachmentView"];
}

- (void) dealloc {
    [UIF_EXTRA_NSTextAttachment(self)->_contents release];
    [UIF_EXTRA_NSTextAttachment(self)->_fileType release];
    [UIF_EXTRA_NSTextAttachment(self)->_image release];
    [UIF_EXTRA_NSTextAttachment(self)->_fileWrapper release];
    [UIF_EXTRA_NSTextAttachment(self)->_cell release];
    UIF_EXTRA_FREE(NSTextAttachment, self);
    [super dealloc];
}

// Without explicit contents, a regular-file wrapper supplies them.
- (NSData *) contents {
    if (UIF_EXTRA_NSTextAttachment(self)->_contents == nil && [UIF_EXTRA_NSTextAttachment(self)->_fileWrapper isRegularFile])
        return [UIF_EXTRA_NSTextAttachment(self)->_fileWrapper regularFileContents];
    return UIF_EXTRA_NSTextAttachment(self)->_contents;
}

- (void) setContents: (NSData *) contents {
    contents = [contents copy];
    [UIF_EXTRA_NSTextAttachment(self)->_contents release];
    UIF_EXTRA_NSTextAttachment(self)->_contents = contents;
}

- (NSString *) fileType {
    return UIF_EXTRA_NSTextAttachment(self)->_fileType;
}

- (void) setFileType: (NSString *) fileType {
    fileType = [fileType copy];
    [UIF_EXTRA_NSTextAttachment(self)->_fileType release];
    UIF_EXTRA_NSTextAttachment(self)->_fileType = fileType;
}

- (NSImage *) image {
    return UIF_EXTRA_NSTextAttachment(self)->_image;
}

- (void) setImage: (NSImage *) image {
    [image retain];
    [UIF_EXTRA_NSTextAttachment(self)->_image release];
    UIF_EXTRA_NSTextAttachment(self)->_image = image;
}

- (CGRect) bounds {
    return UIF_EXTRA_NSTextAttachment(self)->_bounds;
}

- (void) setBounds: (CGRect) bounds {
    UIF_EXTRA_NSTextAttachment(self)->_bounds = bounds;
}

- (NSFileWrapper *) fileWrapper {
    return UIF_EXTRA_NSTextAttachment(self)->_fileWrapper;
}

- (void) setFileWrapper: (NSFileWrapper *) fileWrapper {
    fileWrapper = [fileWrapper retain];
    [UIF_EXTRA_NSTextAttachment(self)->_fileWrapper release];
    UIF_EXTRA_NSTextAttachment(self)->_fileWrapper = fileWrapper;
}

- (id<NSTextAttachmentCell>) attachmentCell {
    return UIF_EXTRA_NSTextAttachment(self)->_cell;
}

- (void) setAttachmentCell: (id<NSTextAttachmentCell>) cell {
    cell = [cell retain];
    [UIF_EXTRA_NSTextAttachment(self)->_cell release];
    UIF_EXTRA_NSTextAttachment(self)->_cell = cell;
}

- (CGFloat) lineLayoutPadding {
    return UIF_EXTRA_NSTextAttachment(self)->_lineLayoutPadding;
}

- (void) setLineLayoutPadding: (CGFloat) padding {
    UIF_EXTRA_NSTextAttachment(self)->_lineLayoutPadding = padding;
}

- (BOOL) allowsTextAttachmentView {
    return UIF_EXTRA_NSTextAttachment(self)->_allowsTextAttachmentView;
}

- (void) setAllowsTextAttachmentView: (BOOL) allows {
    UIF_EXTRA_NSTextAttachment(self)->_allowsTextAttachmentView = allows;
}

- (BOOL) usesTextAttachmentView {
    return UIF_EXTRA_NSTextAttachment(self)->_allowsTextAttachmentView && UIF_EXTRA_NSTextAttachment(self)->_fileType != nil &&
           [[self class] textAttachmentViewProviderClassForFileType: UIF_EXTRA_NSTextAttachment(self)->_fileType] != Nil;
}

- (NSImage *) imageForBounds: (CGRect) bounds
                  attributes: (NSDictionary<NSAttributedStringKey, id> *) attributes
                    location: (id<NSTextLocation>) location
               textContainer: (NSTextContainer *) textContainer
{
    if (UIF_EXTRA_NSTextAttachment(self)->_image != nil)
        return UIF_EXTRA_NSTextAttachment(self)->_image;
    NSData *contents = [self contents];
    return _UIFoundationImageWithData(contents);
}

- (CGRect) attachmentBoundsForAttributes: (NSDictionary<NSAttributedStringKey, id> *) attributes
                                location: (id<NSTextLocation>) location
                           textContainer: (NSTextContainer *) textContainer
                    proposedLineFragment: (CGRect) proposedLineFragment
                                position: (CGPoint) position
{
    if (!CGRectIsEmpty(UIF_EXTRA_NSTextAttachment(self)->_bounds))
        return UIF_EXTRA_NSTextAttachment(self)->_bounds;
    NSImage *image = [self imageForBounds: UIF_EXTRA_NSTextAttachment(self)->_bounds
                               attributes: attributes
                                 location: location
                            textContainer: textContainer];
    if (image == nil)
        return UIF_EXTRA_NSTextAttachment(self)->_bounds;
    NSSize size = [image size];
    return CGRectMake(0, 0, size.width, size.height);
}

- (NSTextAttachmentViewProvider *) viewProviderForParentView: (NSView *) parentView
                                                    location: (id<NSTextLocation>) location
                                               textContainer: (NSTextContainer *) textContainer
{
    if (![self usesTextAttachmentView])
        return nil;
    Class providerClass = [[self class] textAttachmentViewProviderClassForFileType: UIF_EXTRA_NSTextAttachment(self)->_fileType];
    id provider = [[providerClass alloc] initWithTextAttachment: self
                                                     parentView: parentView
                                              textLayoutManager: nil
                                                       location: location];
    return [provider autorelease];
}

@end
