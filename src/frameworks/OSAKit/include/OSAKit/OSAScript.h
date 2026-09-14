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
#import <OSAKit/OSALanguage.h>
#import <OSAKit/OSALanguageInstance.h>

extern NSString *const OSAScriptErrorMessageKey;
extern NSString *const OSAScriptErrorBriefMessageKey;
extern NSString *const OSAScriptErrorNumberKey;
extern NSString *const OSAScriptErrorPartialResultKey;
extern NSString *const OSAScriptErrorOffendingObjectKey;
extern NSString *const OSAScriptErrorExpectedTypeKey;
extern NSString *const OSAScriptErrorAppAddressKey;
extern NSString *const OSAScriptErrorAppNameKey;
extern NSString *const OSAScriptErrorRangeKey;

extern NSString *const OSAScriptErrorMessage;
extern NSString *const OSAScriptErrorBriefMessage;
extern NSString *const OSAScriptErrorNumber;
extern NSString *const OSAScriptErrorAppName;
extern NSString *const OSAScriptErrorRange;

extern NSString *const OSAStorageScriptType;
extern NSString *const OSAStorageScriptBundleType;
extern NSString *const OSAStorageApplicationType;
extern NSString *const OSAStorageApplicationBundleType;
extern NSString *const OSAStorageTextType;

typedef NS_OPTIONS(NSUInteger, OSAStorageOptions) {
    OSANull                  = 0x00000000,
    OSAPreventGetSource      = 0x00000001,
    OSACompileIntoContext    = 0x00000002,
    OSADontSetScriptLocation = 0x01000000,
    OSAStayOpenApplet        = 0x10000000,
    OSAShowStartupScreen     = 0x20000000,
};

@interface OSAScript : NSObject

+ (instancetype)scriptWithSource:(NSString *)source language:(OSALanguage *)language;
+ (instancetype)scriptWithSource:(NSString *)source fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions;
+ (instancetype)scriptWithDataDescriptor:(NSAppleEventDescriptor *)data fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo;

- (instancetype)initWithSource:(NSString *)source;
- (instancetype)initWithSource:(NSString *)source language:(OSALanguage *)language;
- (instancetype)initWithSource:(NSString *)source fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions;
- (instancetype)initWithContentsOfURL:(NSURL *)url error:(NSDictionary **)errorInfo;
- (instancetype)initWithContentsOfURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo;
- (instancetype)initWithScriptDataDescriptor:(NSAppleEventDescriptor *)data fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo;

@property (readonly, copy) NSString *source;
@property (readonly, copy) NSURL *url;
@property (retain) OSALanguage *language;
@property (retain) OSALanguageInstance *languageInstance;
@property (readonly, getter=isCompiled) BOOL compiled;
@property (readonly, copy) NSAttributedString *richTextSource;
@property (readonly) BOOL hasOpenHandler;

- (NSString *)sourceAndReturnError:(NSDictionary **)errorInfo;
- (BOOL)compileAndReturnError:(NSDictionary **)errorInfo;
- (NSAppleEventDescriptor *)executeAndReturnError:(NSDictionary **)errorInfo;
- (NSAppleEventDescriptor *)executeAppleEvent:(NSAppleEventDescriptor *)event error:(NSDictionary **)errorInfo;
- (NSAppleEventDescriptor *)executeAndReturnDisplayValue:(NSAttributedString **)displayValue error:(NSDictionary **)errorInfo;
- (NSAttributedString *)richTextFromDescriptor:(NSAppleEventDescriptor *)descriptor;
- (NSData *)compiledDataForType:(NSString *)type usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo;
- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type error:(NSDictionary **)errorInfo;
- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo;

@end
