#import <AppKit/AppKitDefines.h>
#import <AppKit/NSErrors.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSString, NSArray;





@interface NSObject (NSAccessibility)



- (NSArray *)accessibilityAttributeNames;
- (id)accessibilityAttributeValue:(NSString *)attribute;
- (BOOL)accessibilityIsAttributeSettable:(NSString *)attribute;
- (void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute;



- (NSArray *)accessibilityParameterizedAttributeNames;
- (id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter;



- (NSArray *)accessibilityActionNames;
- (NSString *)accessibilityActionDescription:(NSString *)action;
- (void)accessibilityPerformAction:(NSString *)action;



- (BOOL)accessibilityIsIgnored;



- (id)accessibilityHitTest:(NSPoint)point;



- (id)accessibilityFocusedUIElement;







- (NSUInteger)accessibilityIndexOfChild:(id)child;



- (NSUInteger)accessibilityArrayAttributeCount:(NSString *)attribute;



- (NSArray *)accessibilityArrayAttributeValues:(NSString *)attribute index:(NSUInteger)index maxCount:(NSUInteger)maxCount;

@end




@interface NSObject (NSAccessibilityAdditions)



- (BOOL)accessibilitySetOverrideValue:(id)value forAttribute:(NSString *)attribute;

@end




EXTERN_C NSString *NSAccessibilityRoleDescription(NSString *role, NSString *subrole);
EXTERN_C NSString *NSAccessibilityRoleDescriptionForUIElement(id element);
EXTERN_C NSString *NSAccessibilityActionDescription(NSString *action);




EXTERN_C void NSAccessibilityRaiseBadArgumentException(id element, NSString *attribute, id value);




EXTERN_C id NSAccessibilityUnignoredAncestor(id element);
EXTERN_C id NSAccessibilityUnignoredDescendant(id element);
EXTERN_C NSArray *NSAccessibilityUnignoredChildren(NSArray *originalChildren);
EXTERN_C NSArray *NSAccessibilityUnignoredChildrenForOnlyChild(id originalChild);
EXTERN_C void NSAccessibilityPostNotification(id element, NSString *notification);
EXTERN_C NSString *const NSAccessibilityErrorCodeExceptionInfo;






EXTERN_C NSString *const NSAccessibilityRoleAttribute;
EXTERN_C NSString *const NSAccessibilityRoleDescriptionAttribute;
EXTERN_C NSString *const NSAccessibilitySubroleAttribute;
EXTERN_C NSString *const NSAccessibilityHelpAttribute;
EXTERN_C NSString *const NSAccessibilityValueAttribute;
EXTERN_C NSString *const NSAccessibilityMinValueAttribute;
EXTERN_C NSString *const NSAccessibilityMaxValueAttribute;
EXTERN_C NSString *const NSAccessibilityEnabledAttribute;
EXTERN_C NSString *const NSAccessibilityFocusedAttribute;
EXTERN_C NSString *const NSAccessibilityParentAttribute;
EXTERN_C NSString *const NSAccessibilityChildrenAttribute;
EXTERN_C NSString *const NSAccessibilityWindowAttribute;
EXTERN_C NSString *const NSAccessibilityTopLevelUIElementAttribute;
EXTERN_C NSString *const NSAccessibilitySelectedChildrenAttribute;
EXTERN_C NSString *const NSAccessibilityVisibleChildrenAttribute;
EXTERN_C NSString *const NSAccessibilityPositionAttribute;
EXTERN_C NSString *const NSAccessibilitySizeAttribute;
EXTERN_C NSString *const NSAccessibilityContentsAttribute;
EXTERN_C NSString *const NSAccessibilityTitleAttribute;
EXTERN_C NSString *const NSAccessibilityDescriptionAttribute;
EXTERN_C NSString *const NSAccessibilityShownMenuAttribute;
EXTERN_C NSString *const NSAccessibilityValueDescriptionAttribute
   ;




EXTERN_C NSString *const NSAccessibilityPreviousContentsAttribute;
EXTERN_C NSString *const NSAccessibilityNextContentsAttribute;
EXTERN_C NSString *const NSAccessibilityHeaderAttribute;
EXTERN_C NSString *const NSAccessibilityEditedAttribute;
EXTERN_C NSString *const NSAccessibilityTabsAttribute;
EXTERN_C NSString *const NSAccessibilityHorizontalScrollBarAttribute;
EXTERN_C NSString *const NSAccessibilityVerticalScrollBarAttribute;
EXTERN_C NSString *const NSAccessibilityOverflowButtonAttribute;
EXTERN_C NSString *const NSAccessibilityIncrementButtonAttribute;
EXTERN_C NSString *const NSAccessibilityDecrementButtonAttribute;
EXTERN_C NSString *const NSAccessibilityFilenameAttribute;
EXTERN_C NSString *const NSAccessibilityExpandedAttribute;
EXTERN_C NSString *const NSAccessibilitySelectedAttribute;
EXTERN_C NSString *const NSAccessibilitySplittersAttribute;
EXTERN_C NSString *const NSAccessibilityDocumentAttribute;
EXTERN_C NSString *const NSAccessibilityURLAttribute;
EXTERN_C NSString *const NSAccessibilityIndexAttribute;

EXTERN_C NSString *const NSAccessibilityRowCountAttribute
   ;
EXTERN_C NSString *const NSAccessibilityColumnCountAttribute
   ;
EXTERN_C NSString *const NSAccessibilityOrderedByRowAttribute
   ;

EXTERN_C NSString *const NSAccessibilityWarningValueAttribute
   ;
EXTERN_C NSString *const NSAccessibilityCriticalValueAttribute
   ;
EXTERN_C NSString *const NSAccessibilityPlaceholderValueAttribute
   ;



EXTERN_C NSString *const NSAccessibilityTitleUIElementAttribute;
EXTERN_C NSString *const NSAccessibilityServesAsTitleForUIElementsAttribute;
EXTERN_C NSString *const NSAccessibilityLinkedUIElementsAttribute;




EXTERN_C NSString *const NSAccessibilitySelectedTextAttribute;
EXTERN_C NSString *const NSAccessibilitySelectedTextRangeAttribute;
EXTERN_C NSString *const NSAccessibilityNumberOfCharactersAttribute;
EXTERN_C NSString *const NSAccessibilityVisibleCharacterRangeAttribute;
EXTERN_C NSString *const NSAccessibilitySharedTextUIElementsAttribute;
EXTERN_C NSString *const NSAccessibilitySharedCharacterRangeAttribute;
EXTERN_C NSString *const NSAccessibilityInsertionPointLineNumberAttribute;
EXTERN_C NSString *const NSAccessibilitySelectedTextRangesAttribute
   ;




EXTERN_C NSString *const NSAccessibilityLineForIndexParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityRangeForLineParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityStringForRangeParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityRangeForPositionParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityRangeForIndexParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityBoundsForRangeParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityRTFForRangeParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityStyleRangeForIndexParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityAttributedStringForRangeParameterizedAttribute;




EXTERN_C NSString *const NSAccessibilityFontTextAttribute;
EXTERN_C NSString *const NSAccessibilityForegroundColorTextAttribute;
EXTERN_C NSString *const NSAccessibilityBackgroundColorTextAttribute;
EXTERN_C NSString *const NSAccessibilityUnderlineColorTextAttribute;
EXTERN_C NSString *const NSAccessibilityStrikethroughColorTextAttribute;
EXTERN_C NSString *const NSAccessibilityUnderlineTextAttribute;
EXTERN_C NSString *const NSAccessibilitySuperscriptTextAttribute;
EXTERN_C NSString *const NSAccessibilityStrikethroughTextAttribute;
EXTERN_C NSString *const NSAccessibilityShadowTextAttribute;
EXTERN_C NSString *const NSAccessibilityAttachmentTextAttribute;
EXTERN_C NSString *const NSAccessibilityLinkTextAttribute;
EXTERN_C NSString *const NSAccessibilityMisspelledTextAttribute;
EXTERN_C NSString *const NSAccessibilityAutocorrectedTextAttribute;

EXTERN_C NSString *const NSAccessibilityFontNameKey;
EXTERN_C NSString *const NSAccessibilityFontFamilyKey;
EXTERN_C NSString *const NSAccessibilityVisibleNameKey;
EXTERN_C NSString *const NSAccessibilityFontSizeKey;




EXTERN_C NSString *const NSAccessibilityMainAttribute;
EXTERN_C NSString *const NSAccessibilityMinimizedAttribute;
EXTERN_C NSString *const NSAccessibilityCloseButtonAttribute;
EXTERN_C NSString *const NSAccessibilityZoomButtonAttribute;
EXTERN_C NSString *const NSAccessibilityMinimizeButtonAttribute;
EXTERN_C NSString *const NSAccessibilityToolbarButtonAttribute;
EXTERN_C NSString *const NSAccessibilityProxyAttribute;
EXTERN_C NSString *const NSAccessibilityGrowAreaAttribute;
EXTERN_C NSString *const NSAccessibilityModalAttribute;
EXTERN_C NSString *const NSAccessibilityDefaultButtonAttribute;
EXTERN_C NSString *const NSAccessibilityCancelButtonAttribute;
EXTERN_C NSString *const NSAccessibilityFullScreenButtonAttribute;



EXTERN_C NSString *const NSAccessibilityMenuBarAttribute;
EXTERN_C NSString *const NSAccessibilityWindowsAttribute;
EXTERN_C NSString *const NSAccessibilityFrontmostAttribute;
EXTERN_C NSString *const NSAccessibilityHiddenAttribute;
EXTERN_C NSString *const NSAccessibilityMainWindowAttribute;
EXTERN_C NSString *const NSAccessibilityFocusedWindowAttribute;
EXTERN_C NSString *const NSAccessibilityFocusedUIElementAttribute;
EXTERN_C NSString *const NSAccessibilityExtrasMenuBarAttribute;

EXTERN_C NSString *const NSAccessibilityOrientationAttribute;
EXTERN_C NSString *const NSAccessibilityVerticalOrientationValue;
EXTERN_C NSString *const NSAccessibilityHorizontalOrientationValue;
EXTERN_C NSString *const NSAccessibilityUnknownOrientationValue;

EXTERN_C NSString *const NSAccessibilityColumnTitlesAttribute;

EXTERN_C NSString *const NSAccessibilitySearchButtonAttribute;
EXTERN_C NSString *const NSAccessibilitySearchMenuAttribute;
EXTERN_C NSString *const NSAccessibilityClearButtonAttribute;




EXTERN_C NSString *const NSAccessibilityRowsAttribute;
EXTERN_C NSString *const NSAccessibilityVisibleRowsAttribute;
EXTERN_C NSString *const NSAccessibilitySelectedRowsAttribute;
EXTERN_C NSString *const NSAccessibilityColumnsAttribute;
EXTERN_C NSString *const NSAccessibilityVisibleColumnsAttribute;
EXTERN_C NSString *const NSAccessibilitySelectedColumnsAttribute;
EXTERN_C NSString *const NSAccessibilitySortDirectionAttribute;



EXTERN_C NSString *const NSAccessibilitySelectedCellsAttribute;
EXTERN_C NSString *const NSAccessibilityVisibleCellsAttribute;
EXTERN_C NSString *const NSAccessibilityRowHeaderUIElementsAttribute;
EXTERN_C NSString *const NSAccessibilityColumnHeaderUIElementsAttribute;



EXTERN_C NSString *const NSAccessibilityCellForColumnAndRowParameterizedAttribute;



EXTERN_C NSString *const NSAccessibilityRowIndexRangeAttribute;
EXTERN_C NSString *const NSAccessibilityColumnIndexRangeAttribute;



EXTERN_C NSString *const NSAccessibilityHorizontalUnitsAttribute;
EXTERN_C NSString *const NSAccessibilityVerticalUnitsAttribute;
EXTERN_C NSString *const NSAccessibilityHorizontalUnitDescriptionAttribute;
EXTERN_C NSString *const NSAccessibilityVerticalUnitDescriptionAttribute;



EXTERN_C NSString *const NSAccessibilityLayoutPointForScreenPointParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityLayoutSizeForScreenSizeParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityScreenPointForLayoutPointParameterizedAttribute;
EXTERN_C NSString *const NSAccessibilityScreenSizeForLayoutSizeParameterizedAttribute;



EXTERN_C NSString *const NSAccessibilityHandlesAttribute;



EXTERN_C NSString *const NSAccessibilityAscendingSortDirectionValue;
EXTERN_C NSString *const NSAccessibilityDescendingSortDirectionValue;
EXTERN_C NSString *const NSAccessibilityUnknownSortDirectionValue;



EXTERN_C NSString *const NSAccessibilityDisclosingAttribute;
EXTERN_C NSString *const NSAccessibilityDisclosedRowsAttribute;
EXTERN_C NSString *const NSAccessibilityDisclosedByRowAttribute;
EXTERN_C NSString *const NSAccessibilityDisclosureLevelAttribute;



EXTERN_C NSString *const NSAccessibilityAllowedValuesAttribute;
EXTERN_C NSString *const NSAccessibilityLabelUIElementsAttribute;
EXTERN_C NSString *const NSAccessibilityLabelValueAttribute;



EXTERN_C NSString *const NSAccessibilityMatteHoleAttribute;
EXTERN_C NSString *const NSAccessibilityMatteContentUIElementAttribute;



EXTERN_C NSString *const NSAccessibilityMarkerUIElementsAttribute;
EXTERN_C NSString *const NSAccessibilityMarkerValuesAttribute;
EXTERN_C NSString *const NSAccessibilityMarkerGroupUIElementAttribute;
EXTERN_C NSString *const NSAccessibilityUnitsAttribute;
EXTERN_C NSString *const NSAccessibilityUnitDescriptionAttribute;
EXTERN_C NSString *const NSAccessibilityMarkerTypeAttribute;
EXTERN_C NSString *const NSAccessibilityMarkerTypeDescriptionAttribute;



EXTERN_C NSString *const NSAccessibilityIdentifierAttribute
   ;



EXTERN_C NSString *const NSAccessibilityLeftTabStopMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityRightTabStopMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityCenterTabStopMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityDecimalTabStopMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityHeadIndentMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityTailIndentMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityFirstLineIndentMarkerTypeValue;
EXTERN_C NSString *const NSAccessibilityUnknownMarkerTypeValue;



EXTERN_C NSString *const NSAccessibilityInchesUnitValue;
EXTERN_C NSString *const NSAccessibilityCentimetersUnitValue;
EXTERN_C NSString *const NSAccessibilityPointsUnitValue;
EXTERN_C NSString *const NSAccessibilityPicasUnitValue;
EXTERN_C NSString *const NSAccessibilityUnknownUnitValue;



EXTERN_C NSString *const NSAccessibilityPressAction;
EXTERN_C NSString *const NSAccessibilityIncrementAction;
EXTERN_C NSString *const NSAccessibilityDecrementAction;
EXTERN_C NSString *const NSAccessibilityConfirmAction;
EXTERN_C NSString *const NSAccessibilityPickAction;
EXTERN_C NSString *const NSAccessibilityCancelAction;
EXTERN_C NSString *const NSAccessibilityRaiseAction;
EXTERN_C NSString *const NSAccessibilityShowMenuAction;
EXTERN_C NSString *const NSAccessibilityDeleteAction;



EXTERN_C NSString *const NSAccessibilityMainWindowChangedNotification;
EXTERN_C NSString *const NSAccessibilityFocusedWindowChangedNotification;
EXTERN_C NSString *const NSAccessibilityFocusedUIElementChangedNotification;



EXTERN_C NSString *const NSAccessibilityApplicationActivatedNotification;
EXTERN_C NSString *const NSAccessibilityApplicationDeactivatedNotification;
EXTERN_C NSString *const NSAccessibilityApplicationHiddenNotification;
EXTERN_C NSString *const NSAccessibilityApplicationShownNotification;



EXTERN_C NSString *const NSAccessibilityWindowCreatedNotification;
EXTERN_C NSString *const NSAccessibilityWindowMovedNotification;
EXTERN_C NSString *const NSAccessibilityWindowResizedNotification;
EXTERN_C NSString *const NSAccessibilityWindowMiniaturizedNotification;
EXTERN_C NSString *const NSAccessibilityWindowDeminiaturizedNotification;



EXTERN_C NSString *const NSAccessibilityDrawerCreatedNotification;
EXTERN_C NSString *const NSAccessibilitySheetCreatedNotification;



EXTERN_C NSString *const NSAccessibilityUIElementDestroyedNotification;
EXTERN_C NSString *const NSAccessibilityValueChangedNotification;
EXTERN_C NSString *const NSAccessibilityTitleChangedNotification;
EXTERN_C NSString *const NSAccessibilityResizedNotification;
EXTERN_C NSString *const NSAccessibilityMovedNotification;
EXTERN_C NSString *const NSAccessibilityCreatedNotification;



EXTERN_C NSString *const NSAccessibilityHelpTagCreatedNotification;
EXTERN_C NSString *const NSAccessibilitySelectedTextChangedNotification;
EXTERN_C NSString *const NSAccessibilityRowCountChangedNotification;
EXTERN_C NSString *const NSAccessibilitySelectedChildrenChangedNotification;
EXTERN_C NSString *const NSAccessibilitySelectedRowsChangedNotification;
EXTERN_C NSString *const NSAccessibilitySelectedColumnsChangedNotification;

EXTERN_C NSString *const NSAccessibilityRowExpandedNotification;
EXTERN_C NSString *const NSAccessibilityRowCollapsedNotification;



EXTERN_C NSString *const NSAccessibilitySelectedCellsChangedNotification;



EXTERN_C NSString *const NSAccessibilityUnitsChangedNotification;
EXTERN_C NSString *const NSAccessibilitySelectedChildrenMovedNotification;




EXTERN_C NSString *const NSAccessibilityUnknownRole;
EXTERN_C NSString *const NSAccessibilityButtonRole;
EXTERN_C NSString *const NSAccessibilityRadioButtonRole;
EXTERN_C NSString *const NSAccessibilityCheckBoxRole;
EXTERN_C NSString *const NSAccessibilitySliderRole;
EXTERN_C NSString *const NSAccessibilityTabGroupRole;
EXTERN_C NSString *const NSAccessibilityTextFieldRole;
EXTERN_C NSString *const NSAccessibilityStaticTextRole;
EXTERN_C NSString *const NSAccessibilityTextAreaRole;
EXTERN_C NSString *const NSAccessibilityScrollAreaRole;
EXTERN_C NSString *const NSAccessibilityPopUpButtonRole;
EXTERN_C NSString *const NSAccessibilityMenuButtonRole;
EXTERN_C NSString *const NSAccessibilityTableRole;
EXTERN_C NSString *const NSAccessibilityApplicationRole;
EXTERN_C NSString *const NSAccessibilityGroupRole;
EXTERN_C NSString *const NSAccessibilityRadioGroupRole;
EXTERN_C NSString *const NSAccessibilityListRole;
EXTERN_C NSString *const NSAccessibilityScrollBarRole;
EXTERN_C NSString *const NSAccessibilityValueIndicatorRole;
EXTERN_C NSString *const NSAccessibilityImageRole;
EXTERN_C NSString *const NSAccessibilityMenuBarRole;
EXTERN_C NSString *const NSAccessibilityMenuRole;
EXTERN_C NSString *const NSAccessibilityMenuItemRole;
EXTERN_C NSString *const NSAccessibilityColumnRole;
EXTERN_C NSString *const NSAccessibilityRowRole;
EXTERN_C NSString *const NSAccessibilityToolbarRole;
EXTERN_C NSString *const NSAccessibilityBusyIndicatorRole;
EXTERN_C NSString *const NSAccessibilityProgressIndicatorRole;
EXTERN_C NSString *const NSAccessibilityWindowRole;
EXTERN_C NSString *const NSAccessibilityDrawerRole;
EXTERN_C NSString *const NSAccessibilitySystemWideRole;
EXTERN_C NSString *const NSAccessibilityOutlineRole;
EXTERN_C NSString *const NSAccessibilityIncrementorRole;
EXTERN_C NSString *const NSAccessibilityBrowserRole;
EXTERN_C NSString *const NSAccessibilityComboBoxRole;
EXTERN_C NSString *const NSAccessibilitySplitGroupRole;
EXTERN_C NSString *const NSAccessibilitySplitterRole;
EXTERN_C NSString *const NSAccessibilityColorWellRole;
EXTERN_C NSString *const NSAccessibilityGrowAreaRole;
EXTERN_C NSString *const NSAccessibilitySheetRole;
EXTERN_C NSString *const NSAccessibilityHelpTagRole;
EXTERN_C NSString *const NSAccessibilityMatteRole;
EXTERN_C NSString *const NSAccessibilityRulerRole;
EXTERN_C NSString *const NSAccessibilityRulerMarkerRole;
EXTERN_C NSString *const NSAccessibilityLinkRole;
EXTERN_C NSString *const NSAccessibilityDisclosureTriangleRole;
EXTERN_C NSString *const NSAccessibilityGridRole;
EXTERN_C NSString *const NSAccessibilityRelevanceIndicatorRole;
EXTERN_C NSString *const NSAccessibilityLevelIndicatorRole;
EXTERN_C NSString *const NSAccessibilityCellRole;
EXTERN_C NSString *const NSAccessibilityPopoverRole;



EXTERN_C NSString *const NSAccessibilityLayoutAreaRole;
EXTERN_C NSString *const NSAccessibilityLayoutItemRole;
EXTERN_C NSString *const NSAccessibilityHandleRole;




EXTERN_C NSString *const NSAccessibilityUnknownSubrole;
EXTERN_C NSString *const NSAccessibilityCloseButtonSubrole;
EXTERN_C NSString *const NSAccessibilityZoomButtonSubrole;
EXTERN_C NSString *const NSAccessibilityMinimizeButtonSubrole;
EXTERN_C NSString *const NSAccessibilityToolbarButtonSubrole;
EXTERN_C NSString *const NSAccessibilityTableRowSubrole;
EXTERN_C NSString *const NSAccessibilityOutlineRowSubrole;
EXTERN_C NSString *const NSAccessibilitySecureTextFieldSubrole;
EXTERN_C NSString *const NSAccessibilityStandardWindowSubrole;
EXTERN_C NSString *const NSAccessibilityDialogSubrole;
EXTERN_C NSString *const NSAccessibilitySystemDialogSubrole;
EXTERN_C NSString *const NSAccessibilityFloatingWindowSubrole;
EXTERN_C NSString *const NSAccessibilitySystemFloatingWindowSubrole;
EXTERN_C NSString *const NSAccessibilityIncrementArrowSubrole;
EXTERN_C NSString *const NSAccessibilityDecrementArrowSubrole;
EXTERN_C NSString *const NSAccessibilityIncrementPageSubrole;
EXTERN_C NSString *const NSAccessibilityDecrementPageSubrole;
EXTERN_C NSString *const NSAccessibilitySearchFieldSubrole;
EXTERN_C NSString *const NSAccessibilityTextAttachmentSubrole;
EXTERN_C NSString *const NSAccessibilityTextLinkSubrole;
EXTERN_C NSString *const NSAccessibilityTimelineSubrole;
EXTERN_C NSString *const NSAccessibilitySortButtonSubrole;
EXTERN_C NSString *const NSAccessibilityRatingIndicatorSubrole;
EXTERN_C NSString *const NSAccessibilityContentListSubrole;
EXTERN_C NSString *const NSAccessibilityDefinitionListSubrole;
EXTERN_C NSString *const NSAccessibilityFullScreenButtonSubrole;



EXTERN_C NSString *const NSAccessibilitySortButtonRole;
