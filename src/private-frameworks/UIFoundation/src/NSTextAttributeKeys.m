/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

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
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSTextView.h>
#import <UIFoundation/UIFoundationGraphicsContext.h>

// The attribute keys carry the string values macOS gives them, which is also what archived attributed strings
// and CoreText's kCTFontAttributeName and kCTParagraphStyleAttributeName use.
NSAttributedStringKey NSFontAttributeName = @"NSFont";
NSAttributedStringKey NSParagraphStyleAttributeName = @"NSParagraphStyle";
NSAttributedStringKey NSForegroundColorAttributeName = @"NSColor";
NSAttributedStringKey NSBackgroundColorAttributeName = @"NSBackgroundColor";
NSAttributedStringKey NSUnderlineStyleAttributeName = @"NSUnderline";
NSAttributedStringKey NSUnderlineColorAttributeName = @"NSUnderlineColor";
NSAttributedStringKey NSAttachmentAttributeName = @"NSAttachment";
NSAttributedStringKey NSKernAttributeName = @"NSKern";
NSAttributedStringKey NSLigatureAttributeName = @"NSLigature";
NSAttributedStringKey NSStrikethroughStyleAttributeName = @"NSStrikethrough";
NSAttributedStringKey NSStrikethroughColorAttributeName = @"NSStrikethroughColor";
NSAttributedStringKey NSObliquenessAttributeName = @"NSObliqueness";
NSAttributedStringKey NSStrokeWidthAttributeName = @"NSStrokeWidth";
NSAttributedStringKey NSStrokeColorAttributeName = @"NSStrokeColor";
NSAttributedStringKey NSBaselineOffsetAttributeName = @"NSBaselineOffset";
NSAttributedStringKey NSSuperscriptAttributeName = @"NSSuperScript";
NSAttributedStringKey NSLinkAttributeName = @"NSLink";
NSAttributedStringKey NSShadowAttributeName = @"NSShadow";
NSAttributedStringKey NSExpansionAttributeName = @"NSExpansion";
NSAttributedStringKey NSCursorAttributeName = @"NSCursor";
NSAttributedStringKey NSMarkedClauseSegmentAttributeName = @"NSMarkedClauseSegment";
NSAttributedStringKey NSToolTipAttributeName = @"NSToolTip";
NSAttributedStringKey NSGlyphInfoAttributeName = @"NSGlyphInfo";
// The string the attribute is stored under: macOS 26.7.3 and macOS 10.14 both hold "CTVerticalForms" (black-box observation of the
// exported constant; the earlier "NSVerticalGlyphForm" was a guess).
NSAttributedStringKey NSVerticalGlyphFormAttributeName = @"CTVerticalForms";
NSAttributedStringKey NSTextAlternativesAttributeName = @"NSTextAlternatives";
NSAttributedStringKey NSSpellingStateAttributeName = @"NSSpellingState"; // temporary attribute

// Not declared in the public SDK headers, but exported by macOS and bound by applications. The string was read from the exported
// constant on macOS 26.7.3 (black-box observation; absent on macOS 10.14, so applications that bind it need a newer system).
NSAttributedStringKey NSParagraphAttachmentAttributeName = @"NSTextParagraphAnchoredAttachment";
NSAttributedStringKey NSTextEncapsulationAttributeName = @"NSTextEncapsulation";

NSAttributedStringDocumentAttributeKey NSDocumentTypeDocumentAttribute = @"DocumentType";
NSAttributedStringDocumentAttributeKey NSConvertedDocumentAttribute = @"Converted";
NSAttributedStringDocumentAttributeKey NSFileTypeDocumentAttribute = @"UTI";
NSAttributedStringDocumentAttributeKey NSTitleDocumentAttribute = @"NSTitleDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSCompanyDocumentAttribute = @"NSCompanyDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSCopyrightDocumentAttribute = @"NSCopyrightDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSSubjectDocumentAttribute = @"NSSubjectDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSAuthorDocumentAttribute = @"NSAuthorDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSKeywordsDocumentAttribute = @"NSKeywordsDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSCommentDocumentAttribute = @"NSCommentDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSEditorDocumentAttribute = @"NSEditorDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSCreationTimeDocumentAttribute = @"NSCreationTimeDocumentAttribute";
NSAttributedStringKey NSModificationTimeDocumentAttribute = @"NSModificationTimeDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSManagerDocumentAttribute = @"NSManagerDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSCategoryDocumentAttribute = @"NSCategoryDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSAppearanceDocumentAttribute = @"NSAppearanceDocumentAttribute";
NSAttributedStringDocumentAttributeKey NSCharacterEncodingDocumentAttribute = @"CharacterEncoding";
NSAttributedStringDocumentAttributeKey NSDefaultAttributesDocumentAttribute = @"DefaultAttributes";
NSAttributedStringDocumentAttributeKey NSPaperSizeDocumentAttribute = @"PaperSize";
NSAttributedStringDocumentAttributeKey NSLeftMarginDocumentAttribute = @"LeftMargin";
NSAttributedStringDocumentAttributeKey NSRightMarginDocumentAttribute = @"RightMargin";
NSAttributedStringDocumentAttributeKey NSTopMarginDocumentAttribute = @"TopMargin";
NSAttributedStringDocumentAttributeKey NSBottomMarginDocumentAttribute = @"BottomMargin";
NSAttributedStringDocumentAttributeKey NSViewSizeDocumentAttribute = @"ViewSize";
NSAttributedStringDocumentAttributeKey NSViewZoomDocumentAttribute = @"ViewZoom";
NSAttributedStringDocumentAttributeKey NSViewModeDocumentAttribute = @"ViewMode";
NSAttributedStringDocumentAttributeKey NSReadOnlyDocumentAttribute = @"ReadOnly";
NSAttributedStringDocumentAttributeKey NSBackgroundColorDocumentAttribute = @"BackgroundColor";
NSAttributedStringDocumentAttributeKey NSHyphenationFactorDocumentAttribute = @"HyphenationFactor";
NSAttributedStringDocumentAttributeKey NSDefaultTabIntervalDocumentAttribute = @"DefaultTabInterval";
NSAttributedStringDocumentAttributeKey NSTextLayoutSectionsAttribute = @"NSTextLayoutSectionsAttribute";
NSAttributedStringDocumentAttributeKey NSExcludedElementsDocumentAttribute = @"ExcludedElements";
NSAttributedStringDocumentAttributeKey NSTextEncodingNameDocumentAttribute = @"TextEncodingName";
NSAttributedStringDocumentAttributeKey NSPrefixSpacesDocumentAttribute = @"PrefixSpaces";
NSAttributedStringDocumentAttributeKey NSCocoaVersionDocumentAttribute = @"CocoaRTFVersion";

NSAttributedStringDocumentReadingOptionKey NSDocumentTypeDocumentOption = @"DocumentType";
NSAttributedStringDocumentReadingOptionKey NSDefaultAttributesDocumentOption = @"DefaultAttributes";
NSAttributedStringDocumentReadingOptionKey NSCharacterEncodingDocumentOption = @"CharacterEncoding";
NSAttributedStringDocumentReadingOptionKey NSTextEncodingNameDocumentOption = @"TextEncodingName";
NSAttributedStringDocumentReadingOptionKey NSBaseURLDocumentOption = @"BaseURL";
NSAttributedStringDocumentReadingOptionKey NSTimeoutDocumentOption = @"Timeout";
NSAttributedStringDocumentReadingOptionKey NSWebPreferencesDocumentOption = @"WebPreferences";
NSAttributedStringDocumentReadingOptionKey NSWebResourceLoadDelegateDocumentOption = @"WebResourceLoadDelegate";
NSAttributedStringDocumentReadingOptionKey NSTextSizeMultiplierDocumentOption = @"TextSizeMultiplier";
NSAttributedStringDocumentReadingOptionKey NSFileTypeDocumentOption = @"UTI";

NSAttributedStringDocumentType NSPlainTextDocumentType = @"NSPlainText";
NSAttributedStringDocumentType NSRTFTextDocumentType = @"NSRTF";
NSAttributedStringDocumentType NSRTFDTextDocumentType = @"NSRTFD";
NSAttributedStringDocumentType NSHTMLTextDocumentType = @"NSHTML";
NSAttributedStringDocumentType NSMacSimpleTextDocumentType = @"NSMacSimpleText";
NSAttributedStringDocumentType NSDocFormatTextDocumentType = @"NSDocFormat";
NSAttributedStringDocumentType NSWordMLTextDocumentType = @"NSWordML";
NSAttributedStringDocumentType NSWebArchiveTextDocumentType = @"NSWebArchive";
NSAttributedStringDocumentType NSOfficeOpenXMLTextDocumentType = @"NSOfficeOpenXML";
NSAttributedStringDocumentType NSOpenDocumentTextDocumentType = @"NSOpenDocument";

NSTextLayoutSectionKey NSTextLayoutSectionOrientation = @"NSTextLayoutSectionOrientation";
NSTextLayoutSectionKey NSTextLayoutSectionRange = @"NSTextLayoutSectionRange";

NSAttributedStringKey NSCharacterShapeAttributeName = @"NSCharacterShape";

const NSAttributedStringKey NSTextEffectAttributeName = @"NSTextEffect";
NSAttributedStringKey NSWritingDirectionAttributeName = @"NSWritingDirection";

NSUInteger NSUnderlineStrikethroughMask = 0x4000;
NSUInteger NSUnderlineByWordMask = 0x8000;

NSString *const NSTextViewDidChangeSelectionNotification =
        @"NSTextViewDidChangeSelectionNotification";
NSString *const NSTextViewDidChangeTypingAttributesNotification =
        @"NSTextViewDidChangeTypingAttributesNotification";
NSString *const NSAllRomanInputSourcesLocaleIdentifier =
        @"NSAllRomanInputSourcesLocaleIdentifier";

#pragma mark -
#pragma mark Private

NSFont *NSFontAttributeInDictionary(NSDictionary *dictionary) {
    NSFont *font = [dictionary objectForKey: NSFontAttributeName];

    if (font == nil)
        font = _UIFoundationDefaultTextFont();

    return font;
}

NSColor *NSForegroundColorAttributeInDictionary(NSDictionary *dictionary) {
    NSColor *color = [dictionary objectForKey: NSForegroundColorAttributeName];

    if (color == nil)
        color = _UIFoundationTextColorForRole(_UIFoundationTextColorRoleText);

    return color;
}

NSParagraphStyle *
NSParagraphStyleAttributeInDictionary(NSDictionary *dictionary)
{
    NSParagraphStyle *style =
            [dictionary objectForKey: NSParagraphStyleAttributeName];

    if (style == nil)
        style = [NSParagraphStyle defaultParagraphStyle];

    return style;
}
