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

typedef NS_OPTIONS(NSUInteger, OSALanguageFeatures) {
    OSASupportsCompiling = 0x0002,
    OSASupportsGetSource = 0x0004,
    OSASupportsAECoercion = 0x0008,
    OSASupportsAESending = 0x0010,
    OSASupportsRecording = 0x0020,
    OSASupportsConvenience = 0x0040,
    OSASupportsDialects = 0x0080,
    OSASupportsEventHandling = 0x0100,
};

@interface OSALanguage : NSObject

+ (NSArray *)availableLanguages;
+ (OSALanguage *)languageForName:(NSString *)name;
+ (OSALanguage *)languageForScriptDataDescriptor:(NSAppleEventDescriptor *)descriptor;
+ (OSALanguage *)defaultLanguage;
+ (void)setDefaultLanguage:(OSALanguage *)defaultLanguage;

// Private class methods
+ (OSALanguage *)languageForSubType:(OSType)subType;
+ (OSALanguage *)languageForScript:(id)script;

@property (readonly, copy) NSString *name;
@property (readonly, copy) NSString *info;
@property (readonly, copy) NSString *version;
@property (readonly) OSType type;
@property (readonly) OSType subType;
@property (readonly) OSType manufacturer;
@property (readonly) OSALanguageFeatures features;
@property (getter=isThreadSafe, readonly) BOOL threadSafe;

@end
