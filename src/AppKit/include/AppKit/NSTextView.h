#import <AppKit/NSText.h>
#import <AppKit/NSInputManager.h>
#import <AppKit/NSTextAttachment.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSDragging.h>
#import <AppKit/NSUserInterfaceValidation.h>
#import <AppKit/NSTextInputClient.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSTextFinder.h>
#import <AppKit/NSLayoutManager.h>
#import <Foundation/NSTextCheckingResult.h>

@class NSTextContainer;
@class NSTextStorage;
@class NSLayoutManager;
@class NSRulerView;
@class NSRulerMarker;
@class NSUndoManager;
@class NSParagraphStyle;
@class NSOrthography;
@class NSSharingServicePicker;
@protocol NSTextViewDelegate;


enum {
    NSSelectByCharacter = 0,
    NSSelectByWord = 1,
    NSSelectByParagraph = 2
};
typedef NSUInteger NSSelectionGranularity;


enum {
    NSSelectionAffinityUpstream = 0,
    NSSelectionAffinityDownstream = 1
};
typedef NSUInteger NSSelectionAffinity;




EXTERN_C NSString *NSAllRomanInputSourcesLocaleIdentifier;


@interface NSTextView : NSText <NSTextInput, NSUserInterfaceValidations, NSTextInputClient, NSTextLayoutOrientationProvider, NSDraggingSource>



- (id)initWithFrame:(NSRect)frameRect textContainer:(NSTextContainer *)container;


- (id)initWithFrame:(NSRect)frameRect;




- (NSTextContainer *)textContainer;
- (void)setTextContainer:(NSTextContainer *)container;


- (void)replaceTextContainer:(NSTextContainer *)newContainer;


- (void)setTextContainerInset:(NSSize)inset;
- (NSSize)textContainerInset;


- (NSPoint)textContainerOrigin;
- (void)invalidateTextContainerOrigin;


- (NSLayoutManager *)layoutManager;
- (NSTextStorage *)textStorage;




- (void)insertText:(id)insertString;




- (void)setConstrainedFrameSize:(NSSize)desiredSize;





- (void)setAlignment:(NSTextAlignment)alignment range:(NSRange)range;
- (void)setBaseWritingDirection:(NSWritingDirection)writingDirection range:(NSRange)range;



- (void)turnOffKerning:(id)sender;
- (void)tightenKerning:(id)sender;
- (void)loosenKerning:(id)sender;
- (void)useStandardKerning:(id)sender;
- (void)turnOffLigatures:(id)sender;
- (void)useStandardLigatures:(id)sender;
- (void)useAllLigatures:(id)sender;
- (void)raiseBaseline:(id)sender;
- (void)lowerBaseline:(id)sender;
- (void)toggleTraditionalCharacterShape:(id)sender;
- (void)outline:(id)sender;



- (void)performFindPanelAction:(id)sender;




- (void)alignJustified:(id)sender;
- (void)changeColor:(id)sender;
- (void)changeAttributes:(id)sender;
- (void)changeDocumentBackgroundColor:(id)sender;
- (void)orderFrontSpacingPanel:(id)sender;
- (void)orderFrontLinkPanel:(id)sender;
- (void)orderFrontListPanel:(id)sender;
- (void)orderFrontTablePanel:(id)sender;



- (void)rulerView:(NSRulerView *)ruler didMoveMarker:(NSRulerMarker *)marker;
- (void)rulerView:(NSRulerView *)ruler didRemoveMarker:(NSRulerMarker *)marker;
- (void)rulerView:(NSRulerView *)ruler didAddMarker:(NSRulerMarker *)marker;
- (BOOL)rulerView:(NSRulerView *)ruler shouldMoveMarker:(NSRulerMarker *)marker;
- (BOOL)rulerView:(NSRulerView *)ruler shouldAddMarker:(NSRulerMarker *)marker;
- (CGFloat)rulerView:(NSRulerView *)ruler willMoveMarker:(NSRulerMarker *)marker toLocation:(CGFloat)location;
- (BOOL)rulerView:(NSRulerView *)ruler shouldRemoveMarker:(NSRulerMarker *)marker;
- (CGFloat)rulerView:(NSRulerView *)ruler willAddMarker:(NSRulerMarker *)marker atLocation:(CGFloat)location;
- (void)rulerView:(NSRulerView *)ruler handleMouseDown:(NSEvent *)event;



- (void)setNeedsDisplayInRect:(NSRect)rect avoidAdditionalLayout:(BOOL)flag;


- (BOOL)shouldDrawInsertionPoint;
- (void)drawInsertionPointInRect:(NSRect)rect color:(NSColor *)color turnedOn:(BOOL)flag;

- (void)drawViewBackgroundInRect:(NSRect)rect;




- (void)updateRuler;
- (void)updateFontPanel;

- (void)updateDragTypeRegistration;

- (NSRange)selectionRangeForProposedRange:(NSRange)proposedCharRange granularity:(NSSelectionGranularity)granularity;



- (void)clickedOnLink:(id)link atIndex:(NSUInteger)charIndex;




- (void)startSpeaking:(id)sender;
- (void)stopSpeaking:(id)sender;


- (void)setLayoutOrientation:(NSTextLayoutOrientation)theOrientation;


- (void)changeLayoutOrientation:(id)sender;




- (NSUInteger)characterIndexForInsertionAtPoint:(NSPoint)point;


@end

@interface NSTextView (NSCompletion)



- (void)complete:(id)sender;


- (NSRange)rangeForUserCompletion;


- (NSArray *)completionsForPartialWordRange:(NSRange)charRange indexOfSelectedItem:(NSInteger *)index;


- (void)insertCompletion:(NSString *)word forPartialWordRange:(NSRange)charRange movement:(NSInteger)movement isFinal:(BOOL)flag;

@end

@interface NSTextView (NSPasteboard)
- (NSArray *)writablePasteboardTypes;


- (BOOL)writeSelectionToPasteboard:(NSPasteboard *)pboard type:(NSString *)type;


- (BOOL)writeSelectionToPasteboard:(NSPasteboard *)pboard types:(NSArray *)types;



- (NSArray *)readablePasteboardTypes;


- (NSString *)preferredPasteboardTypeFromArray:(NSArray *)availableTypes restrictedToTypesFromArray:(NSArray *)allowedTypes;


- (BOOL)readSelectionFromPasteboard:(NSPasteboard *)pboard type:(NSString *)type;


- (BOOL)readSelectionFromPasteboard:(NSPasteboard *)pboard;


+ (void)registerForServices;


- (id)validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType;


- (void)pasteAsPlainText:(id)sender;
- (void)pasteAsRichText:(id)sender;


@end

@interface NSTextView (NSDragging)

- (BOOL)dragSelectionWithEvent:(NSEvent *)event offset:(NSSize)mouseOffset slideBack:(BOOL)slideBack;


- (NSImage *)dragImageForSelectionWithEvent:(NSEvent *)event origin:(NSPointPointer)origin;


- (NSArray *)acceptableDragTypes;


- (NSDragOperation)dragOperationForDraggingInfo:(id <NSDraggingInfo>)dragInfo type:(NSString *)type;


- (void)cleanUpAfterDragOperation;


@end

@interface NSTextView (NSSharing)





- (NSArray *)selectedRanges;
- (void)setSelectedRanges:(NSArray *)ranges affinity:(NSSelectionAffinity)affinity stillSelecting:(BOOL)stillSelectingFlag;
- (void)setSelectedRanges:(NSArray *)ranges;


- (void)setSelectedRange:(NSRange)charRange affinity:(NSSelectionAffinity)affinity stillSelecting:(BOOL)stillSelectingFlag;
- (NSSelectionAffinity)selectionAffinity;
- (NSSelectionGranularity)selectionGranularity;
- (void)setSelectionGranularity:(NSSelectionGranularity)granularity;

- (void)setSelectedTextAttributes:(NSDictionary *)attributeDictionary;
- (NSDictionary *)selectedTextAttributes;


- (void)setInsertionPointColor:(NSColor *)color;
- (NSColor *)insertionPointColor;

- (void)updateInsertionPointStateAndRestartTimer:(BOOL)restartFlag;

- (void)setMarkedTextAttributes:(NSDictionary *)attributeDictionary;
- (NSDictionary *)markedTextAttributes;


- (void)setLinkTextAttributes:(NSDictionary *)attributeDictionary;
- (NSDictionary *)linkTextAttributes;


- (BOOL)displaysLinkToolTips;
- (void)setDisplaysLinkToolTips:(BOOL)flag;




- (BOOL)acceptsGlyphInfo;
- (void)setAcceptsGlyphInfo:(BOOL)flag;



- (void)setRulerVisible:(BOOL)flag;
- (BOOL)usesRuler;
- (void)setUsesRuler:(BOOL)flag;

- (BOOL)usesInspectorBar;
- (void)setUsesInspectorBar:(BOOL)flag;

- (void)setContinuousSpellCheckingEnabled:(BOOL)flag;
- (BOOL)isContinuousSpellCheckingEnabled;
- (void)toggleContinuousSpellChecking:(id)sender;

- (NSInteger)spellCheckerDocumentTag;

- (void)setGrammarCheckingEnabled:(BOOL)flag;
- (BOOL)isGrammarCheckingEnabled;
- (void)toggleGrammarChecking:(id)sender;


- (void)setSpellingState:(NSInteger)value range:(NSRange)charRange;


- (NSDictionary *)typingAttributes;
- (void)setTypingAttributes:(NSDictionary *)attrs;


- (BOOL)shouldChangeTextInRanges:(NSArray *)affectedRanges replacementStrings:(NSArray *)replacementStrings;
- (NSArray *)rangesForUserTextChange;
- (NSArray *)rangesForUserCharacterAttributeChange;
- (NSArray *)rangesForUserParagraphAttributeChange;

- (BOOL)shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString;
- (void)didChangeText;

- (NSRange)rangeForUserTextChange;
- (NSRange)rangeForUserCharacterAttributeChange;
- (NSRange)rangeForUserParagraphAttributeChange;

- (void)setAllowsDocumentBackgroundColorChange:(BOOL)flag;
- (BOOL)allowsDocumentBackgroundColorChange;

- (void)setDefaultParagraphStyle:(NSParagraphStyle *)paragraphStyle;
- (NSParagraphStyle *)defaultParagraphStyle;

- (void)setAllowsUndo:(BOOL)flag;
- (BOOL)allowsUndo;

- (void)breakUndoCoalescing;

- (BOOL)isCoalescingUndo;

- (BOOL)allowsImageEditing;
- (void)setAllowsImageEditing:(BOOL)flag;


- (void)showFindIndicatorForRange:(NSRange)charRange;




- (id <NSTextViewDelegate>)delegate;
- (void)setDelegate:(id <NSTextViewDelegate>)anObject;
- (BOOL)isEditable;
- (void)setEditable:(BOOL)flag;
- (BOOL)isSelectable;
- (void)setSelectable:(BOOL)flag;
- (BOOL)isRichText;
- (void)setRichText:(BOOL)flag;
- (BOOL)importsGraphics;
- (void)setImportsGraphics:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setDrawsBackground:(BOOL)flag;
- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;
- (BOOL)isFieldEditor;
- (void)setFieldEditor:(BOOL)flag;
- (BOOL)usesFontPanel;
- (void)setUsesFontPanel:(BOOL)flag;
- (BOOL)isRulerVisible;
- (void)setSelectedRange:(NSRange)charRange;





- (NSArray *)allowedInputSourceLocales;
- (void)setAllowedInputSourceLocales:(NSArray *)localeIdentifiers;
@end

@interface NSTextView (NSTextChecking)



- (BOOL)smartInsertDeleteEnabled;
- (void)setSmartInsertDeleteEnabled:(BOOL)flag;
- (NSRange)smartDeleteRangeForProposedRange:(NSRange)proposedCharRange;
- (void)toggleSmartInsertDelete:(id)sender;

- (void)smartInsertForString:(NSString *)pasteString replacingRange:(NSRange)charRangeToReplace beforeString:(NSString **)beforeString afterString:(NSString **)afterString;
- (NSString *)smartInsertBeforeStringForString:(NSString *)pasteString replacingRange:(NSRange)charRangeToReplace;
- (NSString *)smartInsertAfterStringForString:(NSString *)pasteString replacingRange:(NSRange)charRangeToReplace;

- (void)setAutomaticQuoteSubstitutionEnabled:(BOOL)flag;
- (BOOL)isAutomaticQuoteSubstitutionEnabled;
- (void)toggleAutomaticQuoteSubstitution:(id)sender;
- (void)setAutomaticLinkDetectionEnabled:(BOOL)flag;
- (BOOL)isAutomaticLinkDetectionEnabled;
- (void)toggleAutomaticLinkDetection:(id)sender;

- (void)setAutomaticDataDetectionEnabled:(BOOL)flag;
- (BOOL)isAutomaticDataDetectionEnabled;
- (void)toggleAutomaticDataDetection:(id)sender;
- (void)setAutomaticDashSubstitutionEnabled:(BOOL)flag;
- (BOOL)isAutomaticDashSubstitutionEnabled;
- (void)toggleAutomaticDashSubstitution:(id)sender;
- (void)setAutomaticTextReplacementEnabled:(BOOL)flag;
- (BOOL)isAutomaticTextReplacementEnabled;
- (void)toggleAutomaticTextReplacement:(id)sender;
- (void)setAutomaticSpellingCorrectionEnabled:(BOOL)flag;
- (BOOL)isAutomaticSpellingCorrectionEnabled;
- (void)toggleAutomaticSpellingCorrection:(id)sender;

- (NSTextCheckingTypes)enabledTextCheckingTypes;
- (void)setEnabledTextCheckingTypes:(NSTextCheckingTypes)checkingTypes;


- (void)checkTextInRange:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options;
- (void)handleTextCheckingResults:(NSArray *)results forRange:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger)wordCount;


- (void)orderFrontSubstitutionsPanel:(id)sender;
- (void)checkTextInSelection:(id)sender;
- (void)checkTextInDocument:(id)sender;


- (void)setUsesFindPanel:(BOOL)flag;
- (BOOL)usesFindPanel;

- (void)setUsesFindBar:(BOOL)flag;
- (BOOL)usesFindBar;

- (void)setIncrementalSearchingEnabled:(BOOL)flag;
- (BOOL)isIncrementalSearchingEnabled;

@end

@interface NSTextView (NSQuickLookPreview)

- (IBAction)toggleQuickLookPreviewPanel:(id)sender;



- (NSArray *)quickLookPreviewableItemsInRanges:(NSArray *)ranges;


- (void)updateQuickLookPreviewPanel;

@end

@interface NSTextView (NSTextView_SharingService)

- (IBAction)orderFrontSharingServicePicker:(id)sender;

@end

@interface NSTextView (NSDeprecated)
- (void)toggleBaseWritingDirection:(id)sender;


@end



@protocol NSTextViewDelegate <NSTextDelegate>
@optional

- (BOOL)textView:(NSTextView *)textView clickedOnLink:(id)link atIndex:(NSUInteger)charIndex;


- (void)textView:(NSTextView *)textView clickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)cellFrame atIndex:(NSUInteger)charIndex;


- (void)textView:(NSTextView *)textView doubleClickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)cellFrame atIndex:(NSUInteger)charIndex;


- (void)textView:(NSTextView *)view draggedCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)rect event:(NSEvent *)event atIndex:(NSUInteger)charIndex;


- (NSArray *)textView:(NSTextView *)view writablePasteboardTypesForCell:(id <NSTextAttachmentCell>)cell atIndex:(NSUInteger)charIndex;


- (BOOL)textView:(NSTextView *)view writeCell:(id <NSTextAttachmentCell>)cell atIndex:(NSUInteger)charIndex toPasteboard:(NSPasteboard *)pboard type:(NSString *)type ;


- (NSRange)textView:(NSTextView *)textView willChangeSelectionFromCharacterRange:(NSRange)oldSelectedCharRange toCharacterRange:(NSRange)newSelectedCharRange;


- (NSArray *)textView:(NSTextView *)textView willChangeSelectionFromCharacterRanges:(NSArray *)oldSelectedCharRanges toCharacterRanges:(NSArray *)newSelectedCharRanges;


- (BOOL)textView:(NSTextView *)textView shouldChangeTextInRanges:(NSArray *)affectedRanges replacementStrings:(NSArray *)replacementStrings;


- (NSDictionary *)textView:(NSTextView *)textView shouldChangeTypingAttributes:(NSDictionary *)oldTypingAttributes toAttributes:(NSDictionary *)newTypingAttributes;


- (void)textViewDidChangeSelection:(NSNotification *)notification;

- (void)textViewDidChangeTypingAttributes:(NSNotification *)notification;

- (NSString *)textView:(NSTextView *)textView willDisplayToolTip:(NSString *)tooltip forCharacterAtIndex:(NSUInteger)characterIndex;


- (NSArray *)textView:(NSTextView *)textView completions:(NSArray *)words forPartialWordRange:(NSRange)charRange indexOfSelectedItem:(NSInteger *)index;


- (BOOL)textView:(NSTextView *)textView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString;


- (BOOL)textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector;

- (NSInteger)textView:(NSTextView *)textView shouldSetSpellingState:(NSInteger)value range:(NSRange)affectedCharRange;


- (NSMenu *)textView:(NSTextView *)view menu:(NSMenu *)menu forEvent:(NSEvent *)event atIndex:(NSUInteger)charIndex;


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
- (NSDictionary *)textView:(NSTextView *)view willCheckTextInRange:(NSRange)range options:(NSDictionary *)options types:(NSTextCheckingTypes *)checkingTypes;


- (NSArray *)textView:(NSTextView *)view didCheckTextInRange:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options results:(NSArray *)results orthography:(NSOrthography *)orthography wordCount:(NSInteger)wordCount;

#endif

- (NSURL *)textView:(NSTextView *)textView URLForContentsOfTextAttachment:(NSTextAttachment *)textAttachment atIndex:(NSUInteger)charIndex;


- (NSSharingServicePicker *)textView:(NSTextView *)textView willShowSharingServicePicker:(NSSharingServicePicker *)servicePicker forItems:(NSArray *)items;



- (BOOL)textView:(NSTextView *)textView clickedOnLink:(id)link;
- (void)textView:(NSTextView *)textView clickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)cellFrame;
- (void)textView:(NSTextView *)textView doubleClickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)cellFrame;
- (void)textView:(NSTextView *)view draggedCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)rect event:(NSEvent *)event;
- (NSUndoManager *)undoManagerForTextView:(NSTextView *)view;

@end

EXTERN_C NSString *NSTextViewWillChangeNotifyingTextViewNotification;


EXTERN_C NSString *NSTextViewDidChangeSelectionNotification;


EXTERN_C NSString *NSTextViewDidChangeTypingAttributesNotification;





enum {
    NSFindPanelActionShowFindPanel = 1,
    NSFindPanelActionNext = 2,
    NSFindPanelActionPrevious = 3,
    NSFindPanelActionReplaceAll = 4,
    NSFindPanelActionReplace = 5,
    NSFindPanelActionReplaceAndFind = 6,
    NSFindPanelActionSetFindString = 7,
    NSFindPanelActionReplaceAllInSelection = 8,
    NSFindPanelActionSelectAll = 9,
    NSFindPanelActionSelectAllInSelection = 10
};
typedef NSUInteger NSFindPanelAction;



EXTERN_C NSString *NSFindPanelSearchOptionsPboardType;

EXTERN_C NSString *NSFindPanelCaseInsensitiveSearch;
EXTERN_C NSString *NSFindPanelSubstringMatch;

enum {
    NSFindPanelSubstringMatchTypeContains = 0,
    NSFindPanelSubstringMatchTypeStartsWith = 1,
    NSFindPanelSubstringMatchTypeFullWord = 2,
    NSFindPanelSubstringMatchTypeEndsWith = 3
};
typedef NSUInteger NSFindPanelSubstringMatchType;
