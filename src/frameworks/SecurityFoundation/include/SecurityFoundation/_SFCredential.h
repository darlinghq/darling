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

#import <SecurityFoundation/_SFServiceIdentifier.h>

#define SFCredential _SFCredential

@interface SFCredential : NSObject

@property (retain, nonatomic) SFServiceIdentifier* primaryServiceIdentifier;
@property (retain, nonatomic) NSArray<SFServiceIdentifier*>* supplementaryServiceIdentifiers;
@property (copy, nonatomic) NSString* localizedLabel;
@property (copy, nonatomic) NSString* localizedDescription;
@property (copy, nonatomic) NSDictionary* customAttributes;
@property (copy, nonatomic) NSDate* creationDate;
@property (copy, nonatomic) NSDate* modificationDate;
@property (copy, nonatomic) NSString* persistentIdentifier;

@end
