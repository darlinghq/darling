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

#import <Vision/VNDetectBarcodesRequest.h>

// Values observed on macOS 27.0.1 by printing the public constants: each
// symbology string is the constant's own name.
VNBarcodeSymbology const VNBarcodeSymbologyAztec = @"VNBarcodeSymbologyAztec";
VNBarcodeSymbology const VNBarcodeSymbologyCode128 = @"VNBarcodeSymbologyCode128";
VNBarcodeSymbology const VNBarcodeSymbologyCode39 = @"VNBarcodeSymbologyCode39";
VNBarcodeSymbology const VNBarcodeSymbologyCode39Checksum = @"VNBarcodeSymbologyCode39Checksum";
VNBarcodeSymbology const VNBarcodeSymbologyCode39FullASCII = @"VNBarcodeSymbologyCode39FullASCII";
VNBarcodeSymbology const VNBarcodeSymbologyCode39FullASCIIChecksum = @"VNBarcodeSymbologyCode39FullASCIIChecksum";
VNBarcodeSymbology const VNBarcodeSymbologyCode93 = @"VNBarcodeSymbologyCode93";
VNBarcodeSymbology const VNBarcodeSymbologyCode93i = @"VNBarcodeSymbologyCode93i";
VNBarcodeSymbology const VNBarcodeSymbologyDataMatrix = @"VNBarcodeSymbologyDataMatrix";
VNBarcodeSymbology const VNBarcodeSymbologyEAN13 = @"VNBarcodeSymbologyEAN13";
VNBarcodeSymbology const VNBarcodeSymbologyEAN8 = @"VNBarcodeSymbologyEAN8";
VNBarcodeSymbology const VNBarcodeSymbologyI2of5 = @"VNBarcodeSymbologyI2of5";
VNBarcodeSymbology const VNBarcodeSymbologyI2of5Checksum = @"VNBarcodeSymbologyI2of5Checksum";
VNBarcodeSymbology const VNBarcodeSymbologyITF14 = @"VNBarcodeSymbologyITF14";
VNBarcodeSymbology const VNBarcodeSymbologyPDF417 = @"VNBarcodeSymbologyPDF417";
VNBarcodeSymbology const VNBarcodeSymbologyQR = @"VNBarcodeSymbologyQR";
VNBarcodeSymbology const VNBarcodeSymbologyUPCE = @"VNBarcodeSymbologyUPCE";

@implementation VNDetectBarcodesRequest

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
