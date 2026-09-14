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
#include <CoreServices/CoreServices.h>
#import <OSAKit/OSALanguage.h>

@interface OSALanguageInstance : NSObject

+ (instancetype)languageInstanceWithLanguage:(OSALanguage *)language;
+ (ComponentInstance)defaultAppleScriptComponentInstance;

- (instancetype)initWithLanguage:(OSALanguage *)language;

@property (readonly) OSALanguage *language;
@property (readonly) ComponentInstance componentInstance;
@property (retain) NSAppleEventDescriptor *defaultTarget;

- (NSAttributedString *)richTextFromDescriptor:(NSAppleEventDescriptor *)descriptor;

@end
