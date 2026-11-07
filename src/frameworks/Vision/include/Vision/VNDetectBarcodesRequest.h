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

typedef NSString *VNBarcodeSymbology NS_TYPED_ENUM;

extern VNBarcodeSymbology const VNBarcodeSymbologyAztec;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode128;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode39;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode39Checksum;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode39FullASCII;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode39FullASCIIChecksum;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode93;
extern VNBarcodeSymbology const VNBarcodeSymbologyCode93i;
extern VNBarcodeSymbology const VNBarcodeSymbologyDataMatrix;
extern VNBarcodeSymbology const VNBarcodeSymbologyEAN13;
extern VNBarcodeSymbology const VNBarcodeSymbologyEAN8;
extern VNBarcodeSymbology const VNBarcodeSymbologyI2of5;
extern VNBarcodeSymbology const VNBarcodeSymbologyI2of5Checksum;
extern VNBarcodeSymbology const VNBarcodeSymbologyITF14;
extern VNBarcodeSymbology const VNBarcodeSymbologyPDF417;
extern VNBarcodeSymbology const VNBarcodeSymbologyQR;
extern VNBarcodeSymbology const VNBarcodeSymbologyUPCE;

@interface VNDetectBarcodesRequest : NSObject

@end
