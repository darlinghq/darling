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

#include <Foundation/Foundation.h>
#include <QTKit/QTKitDefines.h>

@protocol StandardMovieUIInterface

@end

QTKIT_EXTERN NSString * const QTUIIsFlippedKey;
QTKIT_EXTERN NSString * const QTUIIsFocusedKey;
QTKIT_EXTERN NSString * const QTUIMeasurementBoundsKey;
QTKIT_EXTERN NSString * const QTUIMeasurementContentBoundsKey;
QTKIT_EXTERN NSString * const QTUIMeasurementExtentKey;
QTKIT_EXTERN NSString * const QTUIMeasurementGlyphColorKey;
QTKIT_EXTERN NSString * const QTUIMeasurementMaximumSizeKey;
QTKIT_EXTERN NSString * const QTUIMeasurementMinimumSizeKey;
QTKIT_EXTERN NSString * const QTUIMeasurementNaturalSizeKey;
QTKIT_EXTERN NSString * const QTUIMeasurementThumbBoundsKey;
QTKIT_EXTERN NSString * const QTUIMeasurementTrackBoundsKey;
QTKIT_EXTERN NSString * const QTUISizeKey;
QTKIT_EXTERN NSString * const QTUISizeLarge;
QTKIT_EXTERN NSString * const QTUISizeMini;
QTKIT_EXTERN NSString * const QTUISizeRegular;
QTKIT_EXTERN NSString * const QTUISizeSmall;
QTKIT_EXTERN NSString * const QTUIStateActive;
QTKIT_EXTERN NSString * const QTUIStateDisabled;
QTKIT_EXTERN NSString * const QTUIStateInactive;
QTKIT_EXTERN NSString * const QTUIStateKey;
QTKIT_EXTERN NSString * const QTUIStatePressed;
QTKIT_EXTERN NSString * const QTUIValueKey;
QTKIT_EXTERN NSString * const QTUIVariantKey;
QTKIT_EXTERN NSString * const QTUIWidgetBackground;
QTKIT_EXTERN NSString * const QTUIWidgetBackgroundBorderKey;
QTKIT_EXTERN NSString * const QTUIWidgetBackgroundCornerRadiusKey;
QTKIT_EXTERN NSString * const QTUIWidgetBackgroundHighlightedFractionKey;
QTKIT_EXTERN NSString * const QTUIWidgetButton;
QTKIT_EXTERN NSString * const QTUIWidgetContainer;
QTKIT_EXTERN NSString * const QTUIWidgetContainerFillVariant;
QTKIT_EXTERN NSString * const QTUIWidgetContainerOutlineVariant;
QTKIT_EXTERN NSString * const QTUIWidgetContainerRoundLeftEdgeKey;
QTKIT_EXTERN NSString * const QTUIWidgetContainerRoundRightEdgeKey;
QTKIT_EXTERN NSString * const QTUIWidgetImage;
QTKIT_EXTERN NSString * const QTUIWidgetImageHUDVariant;
QTKIT_EXTERN NSString * const QTUIWidgetImageNormalVariant;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueAudioVolumeHighGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueAudioVolumeLowGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueAudioVolumeMediumGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueAudioVolumeMutedGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueAudioVolumeOffGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueBackwardGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueBeginningGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueEndGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueEnterFullscreenGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueExitFullscreenGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueForwardGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueNextGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValuePauseGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValuePlayGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValuePreviousGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueRetreat30Glyph;
QTKIT_EXTERN NSString * const QTUIWidgetImageValueStopGlyph;
QTKIT_EXTERN NSString * const QTUIWidgetKey;
QTKIT_EXTERN NSString * const QTUIWidgetPopUpBackground;
QTKIT_EXTERN NSString * const QTUIWidgetPopUpBackgroundCornerRadiusKey;
QTKIT_EXTERN NSString * const QTUIWidgetSlider;
QTKIT_EXTERN NSString * const QTUIWidgetSliderAvailableRangesKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderDrawOutlineKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderHighlightedRangesKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderIndicatorOnlyKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderNoIndicatorKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderOrientationHorizontal;
QTKIT_EXTERN NSString * const QTUIWidgetSliderOrientationKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderOrientationVertical;
QTKIT_EXTERN NSString * const QTUIWidgetSliderRoundLeftEdgeKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderRoundRightEdgeKey;
QTKIT_EXTERN NSString * const QTUIWidgetSliderThumb;
