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

#ifndef _DCERROR_H_
#define _DCERROR_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSErrorDomain const DCErrorDomain;

typedef NS_ERROR_ENUM(DCErrorDomain, DCError) {
    DCErrorUnknownSystemFailure = 0,
    DCErrorFeatureUnsupported = 1,
    DCErrorInvalidInput = 2,
    DCErrorInvalidKey = 3,
    DCErrorServerUnavailable = 4,
};

NS_ASSUME_NONNULL_END

#endif
