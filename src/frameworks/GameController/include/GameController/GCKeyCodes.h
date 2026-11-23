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

#ifndef _GCKeyCodes_H_
#define _GCKeyCodes_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/* Key codes of a keyboard; each equals the usage of the key on the HID Keyboard/Keypad page. */
typedef CFIndex GCKeyCode NS_TYPED_ENUM;

extern GCKeyCode const GCKeyCodeKeyA;
extern GCKeyCode const GCKeyCodeKeyB;
extern GCKeyCode const GCKeyCodeKeyC;
extern GCKeyCode const GCKeyCodeKeyD;
extern GCKeyCode const GCKeyCodeKeyE;
extern GCKeyCode const GCKeyCodeKeyF;
extern GCKeyCode const GCKeyCodeKeyG;
extern GCKeyCode const GCKeyCodeKeyH;
extern GCKeyCode const GCKeyCodeKeyI;
extern GCKeyCode const GCKeyCodeKeyJ;
extern GCKeyCode const GCKeyCodeKeyK;
extern GCKeyCode const GCKeyCodeKeyL;
extern GCKeyCode const GCKeyCodeKeyM;
extern GCKeyCode const GCKeyCodeKeyN;
extern GCKeyCode const GCKeyCodeKeyO;
extern GCKeyCode const GCKeyCodeKeyP;
extern GCKeyCode const GCKeyCodeKeyQ;
extern GCKeyCode const GCKeyCodeKeyR;
extern GCKeyCode const GCKeyCodeKeyS;
extern GCKeyCode const GCKeyCodeKeyT;
extern GCKeyCode const GCKeyCodeKeyU;
extern GCKeyCode const GCKeyCodeKeyV;
extern GCKeyCode const GCKeyCodeKeyW;
extern GCKeyCode const GCKeyCodeKeyX;
extern GCKeyCode const GCKeyCodeKeyY;
extern GCKeyCode const GCKeyCodeKeyZ;
extern GCKeyCode const GCKeyCodeOne;
extern GCKeyCode const GCKeyCodeTwo;
extern GCKeyCode const GCKeyCodeThree;
extern GCKeyCode const GCKeyCodeFour;
extern GCKeyCode const GCKeyCodeFive;
extern GCKeyCode const GCKeyCodeSix;
extern GCKeyCode const GCKeyCodeSeven;
extern GCKeyCode const GCKeyCodeEight;
extern GCKeyCode const GCKeyCodeNine;
extern GCKeyCode const GCKeyCodeZero;
extern GCKeyCode const GCKeyCodeReturnOrEnter;
extern GCKeyCode const GCKeyCodeEscape;
extern GCKeyCode const GCKeyCodeDeleteOrBackspace;
extern GCKeyCode const GCKeyCodeTab;
extern GCKeyCode const GCKeyCodeSpacebar;
extern GCKeyCode const GCKeyCodeHyphen;
extern GCKeyCode const GCKeyCodeEqualSign;
extern GCKeyCode const GCKeyCodeOpenBracket;
extern GCKeyCode const GCKeyCodeCloseBracket;
extern GCKeyCode const GCKeyCodeBackslash;
extern GCKeyCode const GCKeyCodeNonUSPound;
extern GCKeyCode const GCKeyCodeSemicolon;
extern GCKeyCode const GCKeyCodeQuote;
extern GCKeyCode const GCKeyCodeGraveAccentAndTilde;
extern GCKeyCode const GCKeyCodeComma;
extern GCKeyCode const GCKeyCodePeriod;
extern GCKeyCode const GCKeyCodeSlash;
extern GCKeyCode const GCKeyCodeCapsLock;
extern GCKeyCode const GCKeyCodeF1;
extern GCKeyCode const GCKeyCodeF2;
extern GCKeyCode const GCKeyCodeF3;
extern GCKeyCode const GCKeyCodeF4;
extern GCKeyCode const GCKeyCodeF5;
extern GCKeyCode const GCKeyCodeF6;
extern GCKeyCode const GCKeyCodeF7;
extern GCKeyCode const GCKeyCodeF8;
extern GCKeyCode const GCKeyCodeF9;
extern GCKeyCode const GCKeyCodeF10;
extern GCKeyCode const GCKeyCodeF11;
extern GCKeyCode const GCKeyCodeF12;
extern GCKeyCode const GCKeyCodeF13;
extern GCKeyCode const GCKeyCodeF14;
extern GCKeyCode const GCKeyCodeF15;
extern GCKeyCode const GCKeyCodeF16;
extern GCKeyCode const GCKeyCodeF17;
extern GCKeyCode const GCKeyCodeF18;
extern GCKeyCode const GCKeyCodeF19;
extern GCKeyCode const GCKeyCodeF20;
extern GCKeyCode const GCKeyCodePrintScreen;
extern GCKeyCode const GCKeyCodeScrollLock;
extern GCKeyCode const GCKeyCodePause;
extern GCKeyCode const GCKeyCodeInsert;
extern GCKeyCode const GCKeyCodeHome;
extern GCKeyCode const GCKeyCodePageUp;
extern GCKeyCode const GCKeyCodeDeleteForward;
extern GCKeyCode const GCKeyCodeEnd;
extern GCKeyCode const GCKeyCodePageDown;
extern GCKeyCode const GCKeyCodeRightArrow;
extern GCKeyCode const GCKeyCodeLeftArrow;
extern GCKeyCode const GCKeyCodeDownArrow;
extern GCKeyCode const GCKeyCodeUpArrow;
extern GCKeyCode const GCKeyCodeKeypadNumLock;
extern GCKeyCode const GCKeyCodeKeypadSlash;
extern GCKeyCode const GCKeyCodeKeypadAsterisk;
extern GCKeyCode const GCKeyCodeKeypadHyphen;
extern GCKeyCode const GCKeyCodeKeypadPlus;
extern GCKeyCode const GCKeyCodeKeypadEnter;
extern GCKeyCode const GCKeyCodeKeypad1;
extern GCKeyCode const GCKeyCodeKeypad2;
extern GCKeyCode const GCKeyCodeKeypad3;
extern GCKeyCode const GCKeyCodeKeypad4;
extern GCKeyCode const GCKeyCodeKeypad5;
extern GCKeyCode const GCKeyCodeKeypad6;
extern GCKeyCode const GCKeyCodeKeypad7;
extern GCKeyCode const GCKeyCodeKeypad8;
extern GCKeyCode const GCKeyCodeKeypad9;
extern GCKeyCode const GCKeyCodeKeypad0;
extern GCKeyCode const GCKeyCodeKeypadPeriod;
extern GCKeyCode const GCKeyCodeKeypadEqualSign;
extern GCKeyCode const GCKeyCodeNonUSBackslash;
extern GCKeyCode const GCKeyCodeApplication;
extern GCKeyCode const GCKeyCodePower;
extern GCKeyCode const GCKeyCodeInternational1;
extern GCKeyCode const GCKeyCodeInternational2;
extern GCKeyCode const GCKeyCodeInternational3;
extern GCKeyCode const GCKeyCodeInternational4;
extern GCKeyCode const GCKeyCodeInternational5;
extern GCKeyCode const GCKeyCodeInternational6;
extern GCKeyCode const GCKeyCodeInternational7;
extern GCKeyCode const GCKeyCodeInternational8;
extern GCKeyCode const GCKeyCodeInternational9;
extern GCKeyCode const GCKeyCodeLANG1;
extern GCKeyCode const GCKeyCodeLANG2;
extern GCKeyCode const GCKeyCodeLANG3;
extern GCKeyCode const GCKeyCodeLANG4;
extern GCKeyCode const GCKeyCodeLANG5;
extern GCKeyCode const GCKeyCodeLANG6;
extern GCKeyCode const GCKeyCodeLANG7;
extern GCKeyCode const GCKeyCodeLANG8;
extern GCKeyCode const GCKeyCodeLANG9;
extern GCKeyCode const GCKeyCodeLeftControl;
extern GCKeyCode const GCKeyCodeLeftShift;
extern GCKeyCode const GCKeyCodeLeftAlt;
extern GCKeyCode const GCKeyCodeLeftGUI;
extern GCKeyCode const GCKeyCodeRightControl;
extern GCKeyCode const GCKeyCodeRightShift;
extern GCKeyCode const GCKeyCodeRightAlt;
extern GCKeyCode const GCKeyCodeRightGUI;

NS_ASSUME_NONNULL_END

#endif
