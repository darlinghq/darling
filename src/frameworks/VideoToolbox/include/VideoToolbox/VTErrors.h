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

#ifndef _VTERRORS_H_
#define _VTERRORS_H_

#include <CoreFoundation/CoreFoundation.h>

enum {
	kVTPropertyNotSupportedErr = -12900,
	kVTPropertyReadOnlyErr = -12901,
	kVTParameterErr = -12902,
	kVTInvalidSessionErr = -12903,
	kVTAllocationFailedErr = -12904,
	kVTPixelTransferNotSupportedErr = -12905,
	kVTCouldNotFindVideoDecoderErr = -12906,
	kVTCouldNotCreateInstanceErr = -12907,
	kVTCouldNotFindVideoEncoderErr = -12908,
	kVTVideoDecoderBadDataErr = -12909,
	kVTVideoDecoderUnsupportedDataFormatErr = -12910,
	kVTVideoDecoderMalfunctionErr = -12911,
	kVTVideoEncoderMalfunctionErr = -12912,
	kVTVideoDecoderNotAvailableNowErr = -12913,
	kVTImageRotationNotSupportedErr = -12914,
	kVTPixelRotationNotSupportedErr = -12914,
	kVTVideoEncoderNotAvailableNowErr = -12915,
	kVTFormatDescriptionChangeNotSupportedErr = -12916,
	kVTInsufficientSourceColorDataErr = -12917,
	kVTCouldNotCreateColorCorrectionDataErr = -12918,
	kVTColorSyncTransformConvertFailedErr = -12919,
	kVTVideoDecoderAuthorizationErr = -12210,
	kVTVideoEncoderAuthorizationErr = -12211,
	kVTColorCorrectionPixelTransferFailedErr = -12212,
	kVTMultiPassStorageIdentifierMismatchErr = -12213,
	kVTMultiPassStorageInvalidErr = -12214,
	kVTFrameSiloInvalidTimeStampErr = -12215,
	kVTFrameSiloInvalidTimeRangeErr = -12216,
	kVTCouldNotFindTemporalFilterErr = -12217,
	kVTPixelTransferNotPermittedErr = -12218,
	kVTColorCorrectionImageRotationFailedErr = -12219,
	kVTVideoDecoderRemovedErr = -17690,
	kVTSessionMalfunctionErr = -17691,
	kVTVideoDecoderNeedsRosettaErr = -17692,
	kVTVideoEncoderNeedsRosettaErr = -17693,
	kVTVideoDecoderReferenceMissingErr = -17694,
	kVTVideoDecoderCallbackMessagingErr = -17695,
	kVTVideoDecoderUnknownErr = -17696,
	kVTExtensionDisabledErr = -17697,
	kVTVideoEncoderMVHEVCVideoLayerIDsMismatchErr = -17698,
	kVTCouldNotOutputTaggedBufferGroupErr = -17699,
	kVTCouldNotFindExtensionErr = -19510,
	kVTExtensionConflictErr = -19511,
	kVTVideoEncoderAutoWhiteBalanceNotLockedErr = -19512,
	kVTLogTransferFunctionMismatchErr = -19513
};

#endif
