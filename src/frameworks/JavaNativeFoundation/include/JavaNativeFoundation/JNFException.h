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

#include <Foundation/Foundation.h>

extern const char* kOutOfMemoryError;
extern const char* kClassNotFoundException;
extern const char* kNullPointerException;
extern const char* kIllegalAccessException;
extern const char* kIllegalArgumentException;
extern const char* kNoSuchFieldException;
extern const char* kNoSuchMethodException;
extern const char* kRuntimeException;

@interface JNFException : NSObject

@end
