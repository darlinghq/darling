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

#import "DPSetting.h"

static NSString * const kSystemDirectory = @"/Library/Preferences";
@implementation DPSetting

+ (instancetype)settingWithKey:(NSString *)key domain:(NSString *)domain kind:(DPKind)kind scope:(DPScope)scope
                         title:(NSString *)title help:(NSString *)help defaultText:(NSString *)defaultText
{
    DPSetting *setting = [[DPSetting alloc] init];
    setting.key = key;
    setting.domain = domain;
    setting.kind = kind;
    setting.scope = scope;
    setting.title = title;
    setting.help = help;
    setting.defaultText = defaultText;
    return setting;
}

#pragma mark Reading

- (id)storedValueInDomain:(NSString *)domain
{
    if (domain == nil)
        domain = self.domain;
    CFPropertyListRef value;
    if (self.scope == DPScopeSystem) {
        // the system-wide file may have been changed by another process since it was read
        CFPreferencesSynchronize((__bridge CFStringRef)domain, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        value = CFPreferencesCopyValue((__bridge CFStringRef)self.key, (__bridge CFStringRef)domain,
                                       kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    } else {
        CFPreferencesAppSynchronize((__bridge CFStringRef)domain);
        value = CFPreferencesCopyAppValue((__bridge CFStringRef)self.key, (__bridge CFStringRef)domain);
    }
    return CFBridgingRelease(value);
}

- (double)effectiveScale
{
    return self.scale != 0 ? self.scale : 1;
}

static NSString *HexOfData(NSData *data)
{
    NSMutableString *text = [NSMutableString string];
    const uint8_t *bytes = [data bytes];
    for (NSUInteger i = 0; i < [data length]; i++)
        [text appendFormat:@"%02X", bytes[i]];
    return text;
}

- (NSString *)textForValue:(id)value
{
    if (value == nil)
        return @"";
    switch (self.kind) {
    case DPKindInteger:
    case DPKindReal:
        if ([value isKindOfClass:[NSNumber class]])
            return [NSString stringWithFormat:@"%g", [value doubleValue] / [self effectiveScale]];
        return @"";
    case DPKindHex:
        return [value isKindOfClass:[NSData class]] ? HexOfData(value) : @"";
    default:
        return [value isKindOfClass:[NSString class]] ? value : @"";
    }
}

#pragma mark Typed text

static BOOL IsHexDigit(unichar c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

- (id)valueForText:(NSString *)text error:(NSString **)error
{
    if (error != NULL)
        *error = nil;
    NSString *trimmed = [text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if ([trimmed length] == 0)
        return nil;

    NSString *problem = nil;
    id result = nil;
    switch (self.kind) {
    case DPKindText:
    case DPKindPath: {
        NSData *utf8 = [trimmed dataUsingEncoding:NSUTF8StringEncoding];
        if (self.asciiOnly) {
            for (NSUInteger i = 0; i < [trimmed length]; i++) {
                unichar c = [trimmed characterAtIndex:i];
                if (c < 0x20 || c > 0x7e)
                    problem = NSLocalizedString(@"Use printable ASCII characters only.", nil);
            }
        }
        if (problem == nil && self.stringLimit != 0 && [utf8 length] >= self.stringLimit)
            problem = [NSString stringWithFormat:NSLocalizedString(@"Use fewer than %lu characters.", nil), (unsigned long)self.stringLimit];
        if (problem == nil && self.kind == DPKindPath && ![trimmed hasPrefix:@"/"])
            problem = NSLocalizedString(@"Give an absolute path, starting with a slash.", nil);
        result = trimmed;
        break;
    }
    case DPKindInteger:
    case DPKindReal: {
        NSScanner *scanner = [NSScanner scannerWithString:trimmed];
        double number = 0;
        if (![scanner scanDouble:&number] || ![scanner isAtEnd] || isnan(number) || isinf(number)) {
            problem = NSLocalizedString(@"Type a number.", nil);
        } else if (number < self.minimum || (self.kind == DPKindInteger && number <= 0)) {
            problem = [NSString stringWithFormat:NSLocalizedString(@"The number must be at least %g.", nil), self.minimum];
        } else if (self.kind == DPKindInteger) {
            double stored = round(number * [self effectiveScale]);
            if (stored < 1 || stored > 9007199254740992.0)
                problem = NSLocalizedString(@"The number is out of range.", nil);
            else
                result = [NSNumber numberWithLongLong:(long long)stored];
        } else {
            result = [NSNumber numberWithDouble:number * [self effectiveScale]];
        }
        break;
    }
    case DPKindUUID: {
        NSUUID *uuid = [[NSUUID alloc] initWithUUIDString:trimmed];
        if (uuid == nil || [[uuid UUIDString] isEqualToString:@"00000000-0000-0000-0000-000000000000"])
            problem = NSLocalizedString(@"Type a UUID such as 5A1E3C7B-0D4F-4B8A-9C2E-6F7081928374, not the nil UUID.", nil);
        result = [uuid UUIDString];
        break;
    }
    case DPKindHex: {
        NSMutableData *data = [NSMutableData data];
        NSMutableString *digits = [NSMutableString string];
        for (NSUInteger i = 0; i < [trimmed length]; i++) {
            unichar c = [trimmed characterAtIndex:i];
            if (c == ':' || c == '-' || c == ' ')
                continue;
            if (!IsHexDigit(c)) {
                problem = NSLocalizedString(@"Use hexadecimal digits only.", nil);
                break;
            }
            [digits appendFormat:@"%C", c];
        }
        if (problem == nil && [digits length] != self.byteLength * 2)
            problem = [NSString stringWithFormat:NSLocalizedString(@"Type exactly %lu hexadecimal digits (%lu bytes).", nil),
                       (unsigned long)(self.byteLength * 2), (unsigned long)self.byteLength];
        BOOL allZero = YES;
        for (NSUInteger i = 0; problem == nil && i + 1 < [digits length]; i += 2) {
            unsigned value = 0;
            [[NSScanner scannerWithString:[digits substringWithRange:NSMakeRange(i, 2)]] scanHexInt:&value];
            uint8_t byte = (uint8_t)value;
            [data appendBytes:&byte length:1];
            if (byte != 0)
                allZero = NO;
        }
        if (problem == nil && allZero)
            problem = NSLocalizedString(@"The value must not be all zero.", nil);
        result = data;
        break;
    }
    default:
        break;
    }
    if (problem != nil) {
        if (error != NULL)
            *error = problem;
        return nil;
    }
    return result;
}

#pragma mark Popups

static BOOL TruthOfValue(id value, BOOL *known)
{
    *known = YES;
    if ([value isKindOfClass:[NSNumber class]])
        return [value boolValue];
    if ([value isKindOfClass:[NSString class]]) {
        NSString *s = [value lowercaseString];
        if ([s isEqualToString:@"yes"] || [s isEqualToString:@"true"] || [s isEqualToString:@"1"])
            return YES;
        if ([s isEqualToString:@"no"] || [s isEqualToString:@"false"] || [s isEqualToString:@"0"])
            return NO;
    }
    *known = NO;
    return NO;
}

- (NSInteger)popupIndexForValue:(id)value
{
    if (value == nil)
        return 0;
    if (self.kind == DPKindBool) {
        BOOL known;
        BOOL on = TruthOfValue(value, &known);
        return known ? (on ? 1 : 2) : 0;
    }
    if ([value isKindOfClass:[NSString class]]) {
        NSUInteger index = 0;
        for (NSString *choice in self.choices) {
            index++;
            if ([choice caseInsensitiveCompare:value] == NSOrderedSame)
                return (NSInteger)index;
        }
    }
    return 0;
}

- (id)valueForPopupIndex:(NSInteger)index
{
    if (index <= 0)
        return nil;
    if (self.kind == DPKindBool)
        return (__bridge id)(index == 1 ? kCFBooleanTrue : kCFBooleanFalse);
    if ((NSUInteger)index <= [self.choices count])
        return self.choices[index - 1];
    return nil;
}

- (NSArray<NSString *> *)popupTitles
{
    NSMutableArray *titles = [NSMutableArray arrayWithObject:
        [NSString stringWithFormat:NSLocalizedString(@"Default (%@)", nil), self.defaultText ?: @""]];
    if (self.kind == DPKindBool)
        [titles addObjectsFromArray:@[ NSLocalizedString(@"On", nil), NSLocalizedString(@"Off", nil) ]];
    else
        [titles addObjectsFromArray:self.choiceTitles ?: self.choices];
    return titles;
}

#pragma mark Writing

- (BOOL)writeUserValue:(id)value toDomain:(NSString *)domain
{
    if (domain == nil)
        domain = self.domain;
    CFPreferencesSetAppValue((__bridge CFStringRef)self.key, (__bridge CFPropertyListRef)value, (__bridge CFStringRef)domain);
    return CFPreferencesAppSynchronize((__bridge CFStringRef)domain);
}

+ (BOOL)applySystemChanges:(NSArray<NSArray *> *)changes error:(NSString **)error
{
    for (NSArray *change in changes) {
        DPSetting *setting = change[0];
        id value = change[1] == [NSNull null] ? nil : change[1];
        CFStringRef domain = (__bridge CFStringRef)setting.domain;
        CFPreferencesSetValue((__bridge CFStringRef)setting.key, (__bridge CFPropertyListRef)value, domain,
                              kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        if (!CFPreferencesSynchronize(domain, kCFPreferencesAnyUser, kCFPreferencesCurrentHost)) {
            if (error != NULL)
                *error = [NSString stringWithFormat:NSLocalizedString(@"%@ could not be written to %@/%@.plist.", nil),
                          setting.key, kSystemDirectory, setting.domain];
            return NO;
        }
    }
    return YES;
}

@end
