/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _Ink_H_
#define _Ink_H_

void* InkAddStrokeToCurrentPhrase(void);
void* InkIsPhraseInProgress(void);
void* InkSetApplicationRecognitionMode(void);
void* InkSetApplicationWritingMode(void);
void* InkSetDrawingMode(void);
void* InkSetPhraseTerminationMode(void);
void* InkStrokeGetPointCount(void);
void* InkStrokeGetPoints(void);
void* InkStrokeGetTypeID(void);
void* InkTerminateCurrentPhrase(void);
void* InkTextAlternatesCount(void);
void* InkTextBounds(void);
void* InkTextCopy(void);
void* InkTextCreateCFString(void);
void* InkTextCreateFromCFData(void);
void* InkTextDraw(void);
void* InkTextFlatten(void);
void* InkTextGetStroke(void);
void* InkTextGetStrokeCount(void);
void* InkTextGetTypeID(void);
void* InkTextKeyModifiers(void);
void* InkUserWritingMode(void);

#endif
