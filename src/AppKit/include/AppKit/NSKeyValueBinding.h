#import <AppKit/AppKitDefines.h>
#import <Foundation/NSObject.h>


@class NSArray, NSDictionary, NSString, NSError;

EXTERN_C id NSMultipleValuesMarker;
EXTERN_C id NSNoSelectionMarker;
EXTERN_C id NSNotApplicableMarker;

EXTERN_C BOOL NSIsControllerMarker(id object);


EXTERN_C NSString *NSObservedObjectKey;
EXTERN_C NSString *NSObservedKeyPathKey;
EXTERN_C NSString *NSOptionsKey;

@interface NSObject (NSKeyValueBindingCreation)

+ (void)exposeBinding:(NSString *)binding;
- (NSArray *)exposedBindings;
- (Class)valueClassForBinding:(NSString *)binding;



- (void)bind:(NSString *)binding toObject:(id)observable withKeyPath:(NSString *)keyPath options:(NSDictionary *)options;
- (void)unbind:(NSString *)binding;



- (NSDictionary *)infoForBinding:(NSString *)binding;
- (NSArray *)optionDescriptionsForBinding:(NSString *)aBinding;


@end

@interface NSObject (NSPlaceholders)

+ (void)setDefaultPlaceholder:(id)placeholder forMarker:(id)marker withBinding:(NSString *)binding;
+ (id)defaultPlaceholderForMarker:(id)marker withBinding:(NSString *)binding;

@end


@interface NSObject (NSEditorRegistration)

- (void)objectDidBeginEditing:(id)editor;
- (void)objectDidEndEditing:(id)editor;

@end


@interface NSObject (NSEditor)

- (void)discardEditing;
- (BOOL)commitEditing;
- (void)commitEditingWithDelegate:(id)delegate didCommitSelector:(SEL)didCommitSelector contextInfo:(void *)contextInfo;






- (BOOL)commitEditingAndReturnError:(NSError **)error;

@end


EXTERN_C NSString *NSAlignmentBinding;
EXTERN_C NSString *NSAlternateImageBinding;
EXTERN_C NSString *NSAlternateTitleBinding;
EXTERN_C NSString *NSAnimateBinding;
EXTERN_C NSString *NSAnimationDelayBinding;
EXTERN_C NSString *NSArgumentBinding;
EXTERN_C NSString *NSAttributedStringBinding;
EXTERN_C NSString *NSContentArrayBinding;
EXTERN_C NSString *NSContentArrayForMultipleSelectionBinding;
EXTERN_C NSString *NSContentBinding;
EXTERN_C NSString *NSContentDictionaryBinding;
EXTERN_C NSString *NSContentHeightBinding;
EXTERN_C NSString *NSContentObjectBinding;
EXTERN_C NSString *NSContentObjectsBinding;
EXTERN_C NSString *NSContentSetBinding;
EXTERN_C NSString *NSContentValuesBinding;
EXTERN_C NSString *NSContentWidthBinding;
EXTERN_C NSString *NSCriticalValueBinding;
EXTERN_C NSString *NSDataBinding;
EXTERN_C NSString *NSDisplayPatternTitleBinding;
EXTERN_C NSString *NSDisplayPatternValueBinding;
EXTERN_C NSString *NSDocumentEditedBinding;
EXTERN_C NSString *NSDoubleClickArgumentBinding;
EXTERN_C NSString *NSDoubleClickTargetBinding;
EXTERN_C NSString *NSEditableBinding;
EXTERN_C NSString *NSEnabledBinding;
EXTERN_C NSString *NSExcludedKeysBinding;
EXTERN_C NSString *NSFilterPredicateBinding;
EXTERN_C NSString *NSFontBinding;
EXTERN_C NSString *NSFontBoldBinding;
EXTERN_C NSString *NSFontFamilyNameBinding;
EXTERN_C NSString *NSFontItalicBinding;
EXTERN_C NSString *NSFontNameBinding;
EXTERN_C NSString *NSFontSizeBinding;
EXTERN_C NSString *NSHeaderTitleBinding;
EXTERN_C NSString *NSHiddenBinding;
EXTERN_C NSString *NSImageBinding;
EXTERN_C NSString *NSIncludedKeysBinding;
EXTERN_C NSString *NSInitialKeyBinding;
EXTERN_C NSString *NSInitialValueBinding;
EXTERN_C NSString *NSIsIndeterminateBinding;
EXTERN_C NSString *NSLabelBinding;
EXTERN_C NSString *NSLocalizedKeyDictionaryBinding;
EXTERN_C NSString *NSManagedObjectContextBinding;
EXTERN_C NSString *NSMaximumRecentsBinding;
EXTERN_C NSString *NSMaxValueBinding;
EXTERN_C NSString *NSMaxWidthBinding;
EXTERN_C NSString *NSMinValueBinding;
EXTERN_C NSString *NSMinWidthBinding;
EXTERN_C NSString *NSMixedStateImageBinding;
EXTERN_C NSString *NSOffStateImageBinding;
EXTERN_C NSString *NSOnStateImageBinding;
EXTERN_C NSString *NSPositioningRectBinding;
EXTERN_C NSString *NSPredicateBinding;
EXTERN_C NSString *NSRecentSearchesBinding;
EXTERN_C NSString *NSRepresentedFilenameBinding;
EXTERN_C NSString *NSRowHeightBinding;
EXTERN_C NSString *NSSelectedIdentifierBinding;
EXTERN_C NSString *NSSelectedIndexBinding;
EXTERN_C NSString *NSSelectedLabelBinding;
EXTERN_C NSString *NSSelectedObjectBinding;
EXTERN_C NSString *NSSelectedObjectsBinding;
EXTERN_C NSString *NSSelectedTagBinding;
EXTERN_C NSString *NSSelectedValueBinding;
EXTERN_C NSString *NSSelectedValuesBinding;
EXTERN_C NSString *NSSelectionIndexesBinding;
EXTERN_C NSString *NSSelectionIndexPathsBinding;
EXTERN_C NSString *NSSortDescriptorsBinding;
EXTERN_C NSString *NSTargetBinding;
EXTERN_C NSString *NSTextColorBinding;
EXTERN_C NSString *NSTitleBinding;
EXTERN_C NSString *NSToolTipBinding;
EXTERN_C NSString *NSTransparentBinding;
EXTERN_C NSString *NSValueBinding;
EXTERN_C NSString *NSValuePathBinding;
EXTERN_C NSString *NSValueURLBinding;
EXTERN_C NSString *NSVisibleBinding;
EXTERN_C NSString *NSWarningValueBinding;
EXTERN_C NSString *NSWidthBinding;


EXTERN_C NSString *NSAllowsEditingMultipleValuesSelectionBindingOption;
EXTERN_C NSString *NSAllowsNullArgumentBindingOption;
EXTERN_C NSString *NSAlwaysPresentsApplicationModalAlertsBindingOption;
EXTERN_C NSString *NSConditionallySetsEditableBindingOption;
EXTERN_C NSString *NSConditionallySetsEnabledBindingOption;
EXTERN_C NSString *NSConditionallySetsHiddenBindingOption;
EXTERN_C NSString *NSContinuouslyUpdatesValueBindingOption;
EXTERN_C NSString *NSCreatesSortDescriptorBindingOption;
EXTERN_C NSString *NSDeletesObjectsOnRemoveBindingsOption;
EXTERN_C NSString *NSDisplayNameBindingOption;
EXTERN_C NSString *NSDisplayPatternBindingOption;
EXTERN_C NSString *NSContentPlacementTagBindingOption;
EXTERN_C NSString *NSHandlesContentAsCompoundValueBindingOption;
EXTERN_C NSString *NSInsertsNullPlaceholderBindingOption;
EXTERN_C NSString *NSInvokesSeparatelyWithArrayObjectsBindingOption;
EXTERN_C NSString *NSMultipleValuesPlaceholderBindingOption;
EXTERN_C NSString *NSNoSelectionPlaceholderBindingOption;
EXTERN_C NSString *NSNotApplicablePlaceholderBindingOption;
EXTERN_C NSString *NSNullPlaceholderBindingOption;
EXTERN_C NSString *NSRaisesForNotApplicableKeysBindingOption;
EXTERN_C NSString *NSPredicateFormatBindingOption;
EXTERN_C NSString *NSSelectorNameBindingOption;
EXTERN_C NSString *NSSelectsAllWhenSettingContentBindingOption;
EXTERN_C NSString *NSValidatesImmediatelyBindingOption;
EXTERN_C NSString *NSValueTransformerNameBindingOption;
EXTERN_C NSString *NSValueTransformerBindingOption;
