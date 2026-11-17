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

#ifndef _HITOOLBOX_APPEARANCE_H_
#define _HITOOLBOX_APPEARANCE_H_

#include <CoreServices/CoreServices.h>
#include <CoreGraphics/CoreGraphics.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)

enum {
  kAppearanceEventClass         = 'appr',
  kAEAppearanceChanged          = 'thme',
  kAESystemFontChanged          = 'sysf',
  kAESmallSystemFontChanged     = 'ssfn',
  kAEViewsFontChanged           = 'vfnt'
};
enum {
  kThemeDataFileType            = 'thme',
  kThemePlatinumFileType        = 'pltn',
  kThemeCustomThemesFileType    = 'scen',
  kThemeSoundTrackFileType      = 'tsnd'
};
#define kThemeAppearancePlatinum       CFSTR( "com.apple.theme.appearance.platinum" )
#define kThemeAppearanceAqua         CFSTR( "com.apple.theme.appearance.aqua" )
#define kThemeAppearanceAquaBlue     CFSTR( "com.apple.theme.appearance.aqua.blue" )
#define kThemeAppearanceAquaGraphite    CFSTR( "com.apple.theme.appearance.aqua.graphite" )
typedef SInt16 AppearancePartCode;
enum {
  kAppearancePartMetaNone       = 0,
  kAppearancePartMetaDisabled   = 254,
  kAppearancePartMetaInactive   = 255,
  kAppearancePartIndicator      = 129,
  kAppearancePartUpButton       = 20,
  kAppearancePartDownButton     = 21,
  kAppearancePartLeftButton     = kAppearancePartUpButton,
  kAppearancePartRightButton    = kAppearancePartDownButton,
  kAppearancePartPageUpArea     = 22,
  kAppearancePartPageDownArea   = 23,
  kAppearancePartPageLeftArea   = kAppearancePartPageUpArea,
  kAppearancePartPageRightArea  = kAppearancePartPageDownArea
};
typedef UInt16 AppearanceRegionCode;
enum {
  kAppearanceRegionTitleBar     = 0,
  kAppearanceRegionTitleText    = 1,
  kAppearanceRegionCloseBox     = 2,
  kAppearanceRegionZoomBox      = 3,
  kAppearanceRegionDrag         = 5,
  kAppearanceRegionGrow         = 6,
  kAppearanceRegionCollapseBox  = 7,
  kAppearanceRegionTitleProxyIcon = 8,
  kAppearanceRegionStructure    = 32,
  kAppearanceRegionContent      = 33,
  kAppearanceRegionToolbarButton = 41
};
enum {
  kThemeBrushDialogBackgroundActive = 1,
  kThemeBrushDialogBackgroundInactive = 2,
  kThemeBrushAlertBackgroundActive = 3,
  kThemeBrushAlertBackgroundInactive = 4,
  kThemeBrushModelessDialogBackgroundActive = 5,
  kThemeBrushModelessDialogBackgroundInactive = 6,
  kThemeBrushUtilityWindowBackgroundActive = 7,
  kThemeBrushUtilityWindowBackgroundInactive = 8,
  kThemeBrushListViewSortColumnBackground = 9,
  kThemeBrushListViewBackground = 10,
  kThemeBrushIconLabelBackground = 11,
  kThemeBrushListViewSeparator  = 12,
  kThemeBrushChasingArrows      = 13,
  kThemeBrushDragHilite         = 14,
  kThemeBrushDocumentWindowBackground = 15,
  kThemeBrushFinderWindowBackground = 16,
  kThemeBrushScrollBarDelimiterActive = 17,
  kThemeBrushScrollBarDelimiterInactive = 18,
  kThemeBrushFocusHighlight     = 19,
  kThemeBrushPopupArrowActive   = 20,
  kThemeBrushPopupArrowPressed  = 21,
  kThemeBrushPopupArrowInactive = 22,
  kThemeBrushAppleGuideCoachmark = 23,
  kThemeBrushIconLabelBackgroundSelected = 24,
  kThemeBrushStaticAreaFill     = 25,
  kThemeBrushActiveAreaFill     = 26,
  kThemeBrushButtonFrameActive  = 27,
  kThemeBrushButtonFrameInactive = 28,
  kThemeBrushButtonFaceActive   = 29,
  kThemeBrushButtonFaceInactive = 30,
  kThemeBrushButtonFacePressed  = 31,
  kThemeBrushButtonActiveDarkShadow = 32,
  kThemeBrushButtonActiveDarkHighlight = 33,
  kThemeBrushButtonActiveLightShadow = 34,
  kThemeBrushButtonActiveLightHighlight = 35,
  kThemeBrushButtonInactiveDarkShadow = 36,
  kThemeBrushButtonInactiveDarkHighlight = 37,
  kThemeBrushButtonInactiveLightShadow = 38,
  kThemeBrushButtonInactiveLightHighlight = 39,
  kThemeBrushButtonPressedDarkShadow = 40,
  kThemeBrushButtonPressedDarkHighlight = 41,
  kThemeBrushButtonPressedLightShadow = 42,
  kThemeBrushButtonPressedLightHighlight = 43,
  kThemeBrushBevelActiveLight   = 44,
  kThemeBrushBevelActiveDark    = 45,
  kThemeBrushBevelInactiveLight = 46,
  kThemeBrushBevelInactiveDark  = 47,
  kThemeBrushNotificationWindowBackground = 48,
  kThemeBrushMovableModalBackground = 49,
  kThemeBrushSheetBackgroundOpaque = 50,
  kThemeBrushDrawerBackground   = 51,
  kThemeBrushToolbarBackground  = 52,
  kThemeBrushSheetBackgroundTransparent = 53,
  kThemeBrushMenuBackground     = 54,
  kThemeBrushMenuBackgroundSelected = 55,
  kThemeBrushListViewOddRowBackground = 56,
  kThemeBrushListViewEvenRowBackground = 57,
  kThemeBrushListViewColumnDivider = 58,
  kThemeBrushSheetBackground    = kThemeBrushSheetBackgroundOpaque,
  kThemeBrushBlack              = -1,
  kThemeBrushWhite              = -2,
  kThemeBrushPrimaryHighlightColor = -3,
  kThemeBrushSecondaryHighlightColor = -4,
  kThemeBrushAlternatePrimaryHighlightColor = -5
};
typedef SInt16                          ThemeBrush;
enum {
  kThemeTextColorDialogActive   = 1,
  kThemeTextColorDialogInactive = 2,
  kThemeTextColorAlertActive    = 3,
  kThemeTextColorAlertInactive  = 4,
  kThemeTextColorModelessDialogActive = 5,
  kThemeTextColorModelessDialogInactive = 6,
  kThemeTextColorWindowHeaderActive = 7,
  kThemeTextColorWindowHeaderInactive = 8,
  kThemeTextColorPlacardActive  = 9,
  kThemeTextColorPlacardInactive = 10,
  kThemeTextColorPlacardPressed = 11,
  kThemeTextColorPushButtonActive = 12,
  kThemeTextColorPushButtonInactive = 13,
  kThemeTextColorPushButtonPressed = 14,
  kThemeTextColorBevelButtonActive = 15,
  kThemeTextColorBevelButtonInactive = 16,
  kThemeTextColorBevelButtonPressed = 17,
  kThemeTextColorPopupButtonActive = 18,
  kThemeTextColorPopupButtonInactive = 19,
  kThemeTextColorPopupButtonPressed = 20,
  kThemeTextColorIconLabel      = 21,
  kThemeTextColorListView       = 22
};
enum {
  kThemeTextColorDocumentWindowTitleActive = 23,
  kThemeTextColorDocumentWindowTitleInactive = 24,
  kThemeTextColorMovableModalWindowTitleActive = 25,
  kThemeTextColorMovableModalWindowTitleInactive = 26,
  kThemeTextColorUtilityWindowTitleActive = 27,
  kThemeTextColorUtilityWindowTitleInactive = 28,
  kThemeTextColorPopupWindowTitleActive = 29,
  kThemeTextColorPopupWindowTitleInactive = 30,
  kThemeTextColorRootMenuActive = 31,
  kThemeTextColorRootMenuSelected = 32,
  kThemeTextColorRootMenuDisabled = 33,
  kThemeTextColorMenuItemActive = 34,
  kThemeTextColorMenuItemSelected = 35,
  kThemeTextColorMenuItemDisabled = 36,
  kThemeTextColorPopupLabelActive = 37,
  kThemeTextColorPopupLabelInactive = 38
};
enum {
  kThemeTextColorTabFrontActive = 39,
  kThemeTextColorTabNonFrontActive = 40,
  kThemeTextColorTabNonFrontPressed = 41,
  kThemeTextColorTabFrontInactive = 42,
  kThemeTextColorTabNonFrontInactive = 43,
  kThemeTextColorIconLabelSelected = 44,
  kThemeTextColorBevelButtonStickyActive = 45,
  kThemeTextColorBevelButtonStickyInactive = 46
};
enum {
  kThemeTextColorNotification   = 47
};
enum {
  kThemeTextColorSystemDetail   = 48
};
enum {
  kThemeTextColorBlack          = -1,
  kThemeTextColorWhite          = -2
};
typedef SInt16                          ThemeTextColor;
enum {
  kThemeStateInactive           = 0,
  kThemeStateActive             = 1,
  kThemeStatePressed            = 2,
  kThemeStateRollover           = 6,
  kThemeStateUnavailable        = 7,
  kThemeStateUnavailableInactive = 8
};
enum {
  kThemeStateDisabled           = 0
};
enum {
  kThemeStatePressedUp          = 2,
  kThemeStatePressedDown        = 3
};
typedef UInt32                          ThemeDrawState;
enum {
  kThemeArrowCursor             = 0,
  kThemeCopyArrowCursor         = 1,
  kThemeAliasArrowCursor        = 2,
  kThemeContextualMenuArrowCursor = 3,
  kThemeIBeamCursor             = 4,
  kThemeCrossCursor             = 5,
  kThemePlusCursor              = 6,
  kThemeWatchCursor             = 7,
  kThemeClosedHandCursor        = 8,
  kThemeOpenHandCursor          = 9,
  kThemePointingHandCursor      = 10,
  kThemeCountingUpHandCursor    = 11,
  kThemeCountingDownHandCursor  = 12,
  kThemeCountingUpAndDownHandCursor = 13,
  kThemeSpinningCursor          = 14,
  kThemeResizeLeftCursor        = 15,
  kThemeResizeRightCursor       = 16,
  kThemeResizeLeftRightCursor   = 17,
  kThemeNotAllowedCursor        = 18,
  kThemeResizeUpCursor          = 19,
  kThemeResizeDownCursor        = 20,
  kThemeResizeUpDownCursor      = 21,
  kThemePoofCursor              = 22
};
typedef UInt32                          ThemeCursor;
enum {
  kThemeMenuBarNormal           = 0,
  kThemeMenuBarSelected         = 1,
  kThemeMenuBarInactive			= 2
};
typedef UInt16                          ThemeMenuBarState;
enum {
  kThemeMenuSquareMenuBar       = (1 << 0)
};
enum {
  kThemeMenuActive              = 0,
  kThemeMenuSelected            = 1,
  kThemeMenuDisabled            = 3
};
typedef UInt16                          ThemeMenuState;
enum {
  kThemeMenuTypePullDown        = 0,
  kThemeMenuTypePopUp           = 1,
  kThemeMenuTypeHierarchical    = 2,
  kThemeMenuTypeInactive        = 0x0100
};
typedef UInt16                          ThemeMenuType;
enum {
  kThemeMenuItemPlain           = 0,
  kThemeMenuItemHierarchical    = 1,
  kThemeMenuItemScrollUpArrow   = 2,
  kThemeMenuItemScrollDownArrow = 3,
  kThemeMenuItemAtTop           = 0x0100,
  kThemeMenuItemAtBottom        = 0x0200,
  kThemeMenuItemHierBackground  = 0x0400,
  kThemeMenuItemPopUpBackground = 0x0800,
  kThemeMenuItemHasIcon         = 0x8000,
  kThemeMenuItemNoBackground    = 0x4000,
  kThemeMenuItemAlignRight      = 0x2000
};
typedef UInt16                          ThemeMenuItemType;
enum {
  kThemeBackgroundTabPane       = 1,
  kThemeBackgroundPlacard       = 2,
  kThemeBackgroundWindowHeader  = 3,
  kThemeBackgroundListViewWindowHeader = 4,
  kThemeBackgroundSecondaryGroupBox = 5,
  kThemeBackgroundMetal         = 6
};
typedef UInt32                          ThemeBackgroundKind;
enum {
  kThemeNameTag                 = 'name',
  kThemeVariantNameTag          = 'varn',
  kThemeVariantBaseTintTag      = 'tint',
  kThemeHighlightColorTag       = 'hcol',
  kThemeScrollBarArrowStyleTag  = 'sbar',
  kThemeScrollBarThumbStyleTag  = 'sbth',
  kThemeSoundsEnabledTag        = 'snds',
  kThemeDblClickCollapseTag     = 'coll'
};
enum {
  kThemeAppearanceFileNameTag   = 'thme',
  kThemeSystemFontTag           = 'lgsf',
  kThemeSmallSystemFontTag      = 'smsf',
  kThemeViewsFontTag            = 'vfnt',
  kThemeViewsFontSizeTag        = 'vfsz',
  kThemeDesktopPatternNameTag   = 'patn',
  kThemeDesktopPatternTag       = 'patt',
  kThemeDesktopPictureNameTag   = 'dpnm',
  kThemeDesktopPictureAliasTag  = 'dpal',
  kThemeDesktopPictureAlignmentTag = 'dpan',
  kThemeHighlightColorNameTag   = 'hcnm',
  kThemeExamplePictureIDTag     = 'epic',
  kThemeSoundTrackNameTag       = 'sndt',
  kThemeSoundMaskTag            = 'smsk',
  kThemeUserDefinedTag          = 'user',
  kThemeSmoothFontEnabledTag    = 'smoo',
  kThemeSmoothFontMinSizeTag    = 'smos'
};
enum {
  kTiledOnScreen                = 1,
  kCenterOnScreen               = 2,
  kFitToScreen                  = 3,
  kFillScreen                   = 4,
  kUseBestGuess                 = 5
};
enum {
  kThemeCheckBoxClassicX        = 0,
  kThemeCheckBoxCheckMark       = 1
};
typedef UInt16                          ThemeCheckBoxStyle;
enum {
  kThemeScrollBarArrowsSingle   = 0,
  kThemeScrollBarArrowsLowerRight = 1
};
typedef UInt16                          ThemeScrollBarArrowStyle;
enum {
  kThemeScrollBarThumbNormal    = 0,
  kThemeScrollBarThumbProportional = 1
};
typedef UInt16                          ThemeScrollBarThumbStyle;
enum {
  kThemeSystemFont              = 0,
  kThemeSmallSystemFont         = 1,
  kThemeSmallEmphasizedSystemFont = 2,
  kThemeViewsFont               = 3,
  kThemeEmphasizedSystemFont    = 4,
  kThemeApplicationFont         = 5,
  kThemeLabelFont               = 6,
  kThemeSystemFontDetail        = 7,
  kThemeSystemFontDetailEmphasized = 8,
  kThemeMenuTitleFont           = 100,
  kThemeMenuItemFont            = 101,
  kThemeMenuItemMarkFont        = 102,
  kThemeMenuItemCmdKeyFont      = 103,
  kThemeWindowTitleFont         = 104,
  kThemePushButtonFont          = 105,
  kThemeUtilityWindowTitleFont  = 106,
  kThemeAlertHeaderFont         = 107,
  kThemeToolbarFont             = 108,
  kThemeMiniSystemFont          = 109,
  kThemeCurrentPortFont         = 200,
  kThemeSpecifiedFont           = 201
};
enum {
  kPublicThemeFontCount         = 21
};
typedef UInt16                          ThemeFontID;
enum {
  kThemeTabNonFront             = 0,
  kThemeTabNonFrontPressed      = 1,
  kThemeTabNonFrontInactive     = 2,
  kThemeTabFront                = 3,
  kThemeTabFrontInactive        = 4,
  kThemeTabNonFrontUnavailable  = 5,
  kThemeTabFrontUnavailable     = 6
};
typedef UInt16                          ThemeTabStyle;
enum {
  kThemeTabNorth                = 0,
  kThemeTabSouth                = 1,
  kThemeTabEast                 = 2,
  kThemeTabWest                 = 3
};
typedef UInt16                          ThemeTabDirection;
enum {
  kThemeSmallTabHeight          = 16,
  kThemeLargeTabHeight          = 21,
  kThemeTabPaneOverlap          = 3,
  kThemeSmallTabHeightMax       = 19,
  kThemeLargeTabHeightMax       = 24
};
enum {
  kThemeScrollBarMedium         = 0,
  kThemeScrollBarSmall          = 1,
  kThemeSliderMedium            = 2,
  kThemeProgressBarMedium       = 3,
  kThemeIndeterminateBarMedium  = 4,
  kThemeRelevanceBar            = 5,
  kThemeSliderSmall             = 6,
  kThemeProgressBarLarge        = 7,
  kThemeIndeterminateBarLarge   = 8
};
enum {
  kThemeScrollBarMini           = 9,
  kThemeSliderMini              = 10,
  kThemeProgressBarMini         = 11,
  kThemeIndeterminateBarMini    = 12
};
enum {
  kThemeMediumScrollBar         = kThemeScrollBarMedium,
  kThemeSmallScrollBar          = kThemeScrollBarSmall,
  kThemeMediumSlider            = kThemeSliderMedium,
  kThemeMediumProgressBar       = kThemeProgressBarMedium,
  kThemeMediumIndeterminateBar  = kThemeIndeterminateBarMedium,
  kThemeSmallSlider             = kThemeSliderSmall,
  kThemeLargeProgressBar        = kThemeProgressBarLarge,
  kThemeLargeIndeterminateBar   = kThemeIndeterminateBarLarge,
  kThemeMiniScrollBar           = kThemeScrollBarMini,
  kThemeMiniSlider              = kThemeSliderMini,
  kThemeMiniProgressBar         = kThemeProgressBarMini,
  kThemeMiniIndeterminateBar    = kThemeIndeterminateBarMini
};
typedef UInt16                          ThemeTrackKind;
enum {
  kThemeTrackActive             = 0,
  kThemeTrackDisabled           = 1,
  kThemeTrackNothingToScroll    = 2,
  kThemeTrackInactive           = 3
};
typedef UInt8                           ThemeTrackEnableState;
enum {
  kThemeLeftOutsideArrowPressed = 0x01,
  kThemeLeftInsideArrowPressed  = 0x02,
  kThemeLeftTrackPressed        = 0x04,
  kThemeThumbPressed            = 0x08,
  kThemeRightTrackPressed       = 0x10,
  kThemeRightInsideArrowPressed = 0x20,
  kThemeRightOutsideArrowPressed = 0x40,
  kThemeTopOutsideArrowPressed  = kThemeLeftOutsideArrowPressed,
  kThemeTopInsideArrowPressed   = kThemeLeftInsideArrowPressed,
  kThemeTopTrackPressed         = kThemeLeftTrackPressed,
  kThemeBottomTrackPressed      = kThemeRightTrackPressed,
  kThemeBottomInsideArrowPressed = kThemeRightInsideArrowPressed,
  kThemeBottomOutsideArrowPressed = kThemeRightOutsideArrowPressed
};
typedef UInt8                           ThemeTrackPressState;
enum {
  kThemeThumbPlain              = 0,
  kThemeThumbUpward             = 1,
  kThemeThumbDownward           = 2
};
typedef UInt8                           ThemeThumbDirection;
enum {
  kThemeTrackHorizontal         = (1 << 0),
  kThemeTrackRightToLeft        = (1 << 1),
  kThemeTrackShowThumb          = (1 << 2),
  kThemeTrackThumbRgnIsNotGhost = (1 << 3),
  kThemeTrackNoScrollBarArrows  = (1 << 4),
  kThemeTrackHasFocus           = (1 << 5),
  kThemeTrackHideTrack          = (1 << 6)
};
typedef UInt16                          ThemeTrackAttributes;
struct ScrollBarTrackInfo {
  SInt32              viewsize;
  ThemeTrackPressState  pressState;
};
typedef struct ScrollBarTrackInfo       ScrollBarTrackInfo;
struct SliderTrackInfo {
  ThemeThumbDirection  thumbDir;
  ThemeTrackPressState  pressState;
};
typedef struct SliderTrackInfo          SliderTrackInfo;
struct ProgressTrackInfo {
  UInt8               phase;
};
typedef struct ProgressTrackInfo        ProgressTrackInfo;
struct ThemeTrackDrawInfo {
  ThemeTrackKind      kind;
  Rect                bounds;
  SInt32              min;
  SInt32              max;
  SInt32              value;
  UInt32              reserved;
  ThemeTrackAttributes  attributes;
  ThemeTrackEnableState  enableState;
  UInt8               filler1;
  union {
    ScrollBarTrackInfo  scrollbar;
    SliderTrackInfo     slider;
    ProgressTrackInfo   progress;
  }                       trackInfo;
};
typedef struct ThemeTrackDrawInfo       ThemeTrackDrawInfo;
enum {
  kThemeWindowHasGrow           = (1 << 0),
  kThemeWindowHasHorizontalZoom = (1 << 3),
  kThemeWindowHasVerticalZoom   = (1 << 4),
  kThemeWindowHasFullZoom       = kThemeWindowHasHorizontalZoom + kThemeWindowHasVerticalZoom,
  kThemeWindowHasCloseBox       = (1 << 5),
  kThemeWindowHasCollapseBox    = (1 << 6),
  kThemeWindowHasTitleText      = (1 << 7),
  kThemeWindowIsCollapsed       = (1 << 8),
  kThemeWindowHasDirty          = (1 << 9),
  kThemeWindowHasToolbarButton  = (1 << 11)
};
typedef UInt32                          ThemeWindowAttributes;
enum {
  kThemeDocumentWindow          = 0,
  kThemeDialogWindow            = 1,
  kThemeMovableDialogWindow     = 2,
  kThemeAlertWindow             = 3,
  kThemeMovableAlertWindow      = 4,
  kThemePlainDialogWindow       = 5,
  kThemeShadowDialogWindow      = 6,
  kThemePopupWindow             = 7,
  kThemeUtilityWindow           = 8,
  kThemeUtilitySideWindow       = 9,
  kThemeSheetWindow             = 10,
  kThemeDrawerWindow            = 11
};
typedef UInt16                          ThemeWindowType;
enum {
  kThemeWidgetCloseBox          = 0,
  kThemeWidgetZoomBox           = 1,
  kThemeWidgetCollapseBox       = 2,
  kThemeWidgetDirtyCloseBox     = 6,
  kThemeWidgetToolbarButton     = 7
};
typedef UInt16                          ThemeTitleBarWidget;
enum {
  kThemeArrowLeft               = 0,
  kThemeArrowDown               = 1,
  kThemeArrowRight              = 2,
  kThemeArrowUp                 = 3
};
typedef UInt16                          ThemeArrowOrientation;
enum {
  kThemeArrow3pt                = 0,
  kThemeArrow5pt                = 1,
  kThemeArrow7pt                = 2,
  kThemeArrow9pt                = 3
};
typedef UInt16                          ThemePopupArrowSize;
enum {
  kThemeGrowLeft                = (1 << 0),
  kThemeGrowRight               = (1 << 1),
  kThemeGrowUp                  = (1 << 2),
  kThemeGrowDown                = (1 << 3)
};
typedef UInt16                          ThemeGrowDirection;
enum {
  kThemePushButton              = 0,
  kThemeCheckBox                = 1,
  kThemeRadioButton             = 2,
  kThemeBevelButton             = 3,
  kThemeArrowButton             = 4,
  kThemePopupButton             = 5,
  kThemeDisclosureTriangle      = 6,
  kThemeIncDecButton            = 7,
  kThemeBevelButtonSmall        = 8,
  kThemeBevelButtonMedium       = 3,
  kThemeBevelButtonLarge        = 9,
  kThemeListHeaderButton        = 10,
  kThemeRoundButton             = 11,
  kThemeRoundButtonLarge        = 12,
  kThemeCheckBoxSmall           = 13,
  kThemeRadioButtonSmall        = 14,
  kThemeRoundedBevelButton      = 15,
  kThemeComboBox                = 16,
  kThemeComboBoxSmall           = 17
};
enum {
  kThemeComboBoxMini            = 18,
  kThemeCheckBoxMini            = 19,
  kThemeRadioButtonMini         = 20,
  kThemeIncDecButtonSmall       = 21,
  kThemeIncDecButtonMini        = 22,
  kThemeArrowButtonSmall        = 23,
  kThemeArrowButtonMini         = 24,
  kThemePushButtonNormal        = 25,
  kThemePushButtonSmall         = 26,
  kThemePushButtonMini          = 27,
  kThemePopupButtonNormal       = 28,
  kThemePopupButtonSmall        = 29,
  kThemePopupButtonMini         = 30
};
enum {
  kThemeBevelButtonInset        = 31,
  kThemePushButtonTextured      = 32,
  kThemePushButtonTexturedSmall = 33,
  kThemeRoundButtonHelp         = 34,
  kThemePushButtonInset         = kThemePushButtonTextured,
  kThemePushButtonInsetSmall    = kThemePushButtonTexturedSmall
};
enum {
  kThemeNormalCheckBox          = kThemeCheckBox,
  kThemeNormalRadioButton       = kThemeRadioButton,
  kThemeLargeBevelButton        = kThemeBevelButtonLarge,
  kThemeMediumBevelButton       = kThemeBevelButtonMedium,
  kThemeMiniCheckBox            = kThemeCheckBoxMini,
  kThemeMiniRadioButton         = kThemeRadioButtonMini,
  kThemeSmallBevelButton        = kThemeBevelButtonSmall,
  kThemeSmallCheckBox           = kThemeCheckBoxSmall,
  kThemeSmallRadioButton        = kThemeRadioButtonSmall,
  kThemeLargeRoundButton        = kThemeRoundButtonLarge,
  kThemeDisclosureButton        = kThemeDisclosureTriangle
};
typedef UInt16                          ThemeButtonKind;
enum {
  kThemeButtonOff               = 0,
  kThemeButtonOn                = 1,
  kThemeButtonMixed             = 2,
  kThemeDisclosureRight         = 0,
  kThemeDisclosureDown          = 1,
  kThemeDisclosureLeft          = 2
};
typedef UInt16                          ThemeButtonValue;
enum {
  kThemeAdornmentNone           = 0,
  kThemeAdornmentDefault        = (1 << 0),
  kThemeAdornmentFocus          = (1 << 2),
  kThemeAdornmentRightToLeft    = (1 << 4),
  kThemeAdornmentDrawIndicatorOnly = (1 << 5),
  kThemeAdornmentHeaderButtonLeftNeighborSelected = (1 << 6),
  kThemeAdornmentHeaderButtonRightNeighborSelected = (1 << 7),
  kThemeAdornmentHeaderButtonSortUp = (1 << 8),
  kThemeAdornmentHeaderMenuButton = (1 << 9),
  kThemeAdornmentHeaderButtonNoShadow = (1 << 10),
  kThemeAdornmentHeaderButtonShadowOnly = (1 << 11),
  kThemeAdornmentHeaderButtonNoSortArrow = (1 << 12),
  kThemeAdornmentArrowLeftArrow = (1 << 6),
  kThemeAdornmentArrowDownArrow = (1 << 7),
  kThemeAdornmentArrowDoubleArrow = (1 << 8),
  kThemeAdornmentArrowUpArrow   = (1 << 9),
  kThemeAdornmentArrowRightArrow = (1 << 10)
};
enum {
  kThemeAdornmentNoShadow       = kThemeAdornmentHeaderButtonNoShadow,
  kThemeAdornmentShadowOnly     = kThemeAdornmentHeaderButtonShadowOnly
};
typedef UInt16                          ThemeButtonAdornment;
struct ThemeButtonDrawInfo {
  ThemeDrawState      state;
  ThemeButtonValue    value;
  ThemeButtonAdornment  adornment;
};
typedef struct ThemeButtonDrawInfo      ThemeButtonDrawInfo;
typedef ThemeButtonDrawInfo *           ThemeButtonDrawInfoPtr;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
