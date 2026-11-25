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

#import "PKPane.h"

static const CGFloat kPaneWidth = 640;
static const CGFloat kLabelRight = 250;   // the labels end here
static const CGFloat kControlX = 262;
static const CGFloat kControlWidth = 300;
static const CGFloat kRowGap = 10;

@interface PKFlippedView : NSView
@end
@implementation PKFlippedView
- (BOOL)isFlipped { return YES; }
@end

#pragma mark Preferences

static CFStringRef DomainRef(NSString *domain)
{
    return domain != nil ? (__bridge CFStringRef)domain : kCFPreferencesAnyApplication;
}

static BOOL ValuesEqual(id a, id b)
{
    if (a == b)
        return YES;
    if ([a isKindOfClass:[NSNumber class]] && [b isKindOfClass:[NSNumber class]])
        return [a doubleValue] == [b doubleValue];
    return [a isEqual:b];
}

#pragma mark Items

@implementation PKItem
+ (instancetype)heading:(NSString *)title
{
    PKItem *item = [PKItem new];
    item.kind = PKKindHeading;
    item.title = title;
    return item;
}
+ (instancetype)note:(NSString *)text
{
    PKItem *item = [PKItem new];
    item.kind = PKKindNote;
    item.title = text;
    return item;
}
+ (instancetype)popup:(NSString *)identifier title:(NSString *)title key:(NSString *)key values:(NSArray *)values titles:(NSArray *)titles defaultValue:(id)defaultValue
{
    PKItem *item = [PKItem new];
    item.kind = PKKindPopup;
    item.identifier = identifier;
    item.title = title;
    item.key = key;
    item.values = values;
    item.titles = titles;
    item.defaultValue = defaultValue;
    return item;
}
+ (instancetype)check:(NSString *)identifier title:(NSString *)title key:(NSString *)key defaultValue:(BOOL)defaultValue
{
    PKItem *item = [PKItem new];
    item.kind = PKKindCheck;
    item.identifier = identifier;
    item.title = title;
    item.key = key;
    item.defaultValue = @(defaultValue);
    return item;
}
+ (instancetype)slider:(NSString *)identifier title:(NSString *)title key:(NSString *)key values:(NSArray *)values labels:(NSArray *)labels defaultValue:(id)defaultValue
{
    PKItem *item = [PKItem new];
    item.kind = PKKindSlider;
    item.identifier = identifier;
    item.title = title;
    item.key = key;
    item.values = values;
    item.titles = labels;
    item.defaultValue = defaultValue;
    return item;
}
+ (instancetype)staticText:(NSString *)identifier title:(NSString *)title text:(NSString *(^)(void))text
{
    PKItem *item = [PKItem new];
    item.kind = PKKindStatic;
    item.identifier = identifier;
    item.title = title;
    item.staticText = text;
    return item;
}
- (id)storedValue
{
    if (self.readBlock != nil)
        return self.readBlock();
    return [PKPane preferenceForKey:self.key domain:self.domain currentHost:self.currentHost];
}
- (void)storeValue:(id)value
{
    if (self.writeBlock != nil)
        self.writeBlock(value);
    else
        [PKPane setPreference:value forKey:self.key domain:self.domain currentHost:self.currentHost];
}
@end

#pragma mark Round colour buttons

@interface PKSwatch : NSButton
@property (strong) NSColor *color;
@property BOOL chosen;
@end

@implementation PKSwatch
- (void)drawRect:(NSRect)dirty
{
    NSRect r = NSInsetRect([self bounds], 3, 3);
    [self.color setFill];
    [[NSBezierPath bezierPathWithOvalInRect:r] fill];
    [[NSColor colorWithCalibratedWhite:0 alpha:0.25] setStroke];
    [[NSBezierPath bezierPathWithOvalInRect:r] stroke];
    if (self.chosen) {
        NSBezierPath *ring = [NSBezierPath bezierPathWithOvalInRect:NSInsetRect([self bounds], 0.75, 0.75)];
        [ring setLineWidth:1.5];
        [[NSColor grayColor] setStroke];
        [ring stroke];
    }
}
- (BOOL)isFlipped { return YES; }
@end

#pragma mark Pane

@interface PKPane () {
    NSArray<PKItem *> *_items;
    NSMutableDictionary<NSString *, NSControl *> *_controls;
    NSMutableDictionary<NSString *, NSArray *> *_swatches;
}
@end

@implementation PKPane

+ (CGFloat)paneWidth { return kPaneWidth; }
- (NSArray<PKItem *> *)items { return @[]; }
- (void)itemDidChange:(PKItem *)item toValue:(id)value {}
- (void)refreshCustomViews {}

+ (id)preferenceForKey:(NSString *)key domain:(NSString *)domain currentHost:(BOOL)currentHost
{
    CFPropertyListRef value = CFPreferencesCopyValue((__bridge CFStringRef)key, DomainRef(domain), kCFPreferencesCurrentUser,
                                                     currentHost ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost);
    return CFBridgingRelease(value);
}

+ (void)setPreference:(id)value forKey:(NSString *)key domain:(NSString *)domain currentHost:(BOOL)currentHost
{
    CFStringRef host = currentHost ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
    CFPreferencesSetValue((__bridge CFStringRef)key, (__bridge CFPropertyListRef)value, DomainRef(domain), kCFPreferencesCurrentUser, host);
    CFPreferencesSynchronize(DomainRef(domain), kCFPreferencesCurrentUser, host);
}

+ (void)postNotification:(NSString *)name
{
    CFNotificationCenterPostNotificationWithOptions(CFNotificationCenterGetDistributedCenter(), (__bridge CFStringRef)name, NULL, NULL,
                                                    kCFNotificationDeliverImmediately | kCFNotificationPostToAllSessions);
}

- (NSControl *)controlForIdentifier:(NSString *)identifier
{
    return _controls[identifier];
}

static NSTextField *Label(NSString *text, NSRect frame, NSFont *font, NSColor *color, NSTextAlignment alignment)
{
    NSTextField *label = [[NSTextField alloc] initWithFrame:frame];
    [label setStringValue:text];
    [label setEditable:NO];
    [label setSelectable:NO];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setFont:font];
    [label setTextColor:color];
    [label setAlignment:alignment];
    [[label cell] setWraps:YES];
    return label;
}

// The height a wrapped label of the font needs for the width
static CGFloat TextHeight(NSString *text, NSFont *font, CGFloat width)
{
    NSRect r = [text boundingRectWithSize:NSMakeSize(width, 10000) options:NSStringDrawingUsesLineFragmentOrigin
                               attributes:@{NSFontAttributeName: font}];
    return ceil(r.size.height) + 2;
}

- (NSView *)loadMainView
{
    _items = [self items];
    _controls = [NSMutableDictionary dictionary];
    _swatches = [NSMutableDictionary dictionary];
    PKFlippedView *main = [[PKFlippedView alloc] initWithFrame:NSMakeRect(0, 0, kPaneWidth, 100)];
    NSFont *regular = [NSFont systemFontOfSize:[NSFont systemFontSize]];
    NSFont *small = [NSFont systemFontOfSize:[NSFont smallSystemFontSize]];
    CGFloat y = 18;

    for (PKItem *item in _items) {
        if (item.kind == PKKindHeading) {
            y += 6;
            NSTextField *heading = Label(item.title, NSMakeRect(20, y, kPaneWidth - 40, 18), [NSFont boldSystemFontOfSize:[NSFont systemFontSize]],
                                         [NSColor controlTextColor], NSTextAlignmentLeft);
            [main addSubview:heading];
            y += 24;
            continue;
        }
        if (item.kind == PKKindNote) {
            CGFloat h = TextHeight(item.title, small, kPaneWidth - 40);
            [main addSubview:Label(item.title, NSMakeRect(20, y, kPaneWidth - 40, h), small, [NSColor grayColor], NSTextAlignmentLeft)];
            y += h + kRowGap;
            continue;
        }

        if (item.kind == PKKindView) {
            [item.view setFrameOrigin:NSMakePoint(20, y)];
            [main addSubview:item.view];
            y += [item.view frame].size.height + kRowGap;
            continue;
        }
        CGFloat height = 22;
        NSControl *control = nil;
        if (item.kind == PKKindPopup) {
            NSPopUpButton *popup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(kControlX, y, kControlWidth, 22) pullsDown:NO];
            [popup addItemsWithTitles:item.titles];
            [popup setTarget:self];
            [popup setAction:@selector(controlChanged:)];
            control = popup;
        } else if (item.kind == PKKindCheck) {
            NSButton *check = [[NSButton alloc] initWithFrame:NSMakeRect(kControlX, y, kPaneWidth - kControlX - 20, 18)];
            [check setButtonType:NSButtonTypeSwitch];
            [check setTitle:item.title];
            [check setTarget:self];
            [check setAction:@selector(controlChanged:)];
            control = check;
            height = 18;
        } else if (item.kind == PKKindSlider) {
            NSSlider *slider = [[NSSlider alloc] initWithFrame:NSMakeRect(kControlX, y, kControlWidth, 26)];
            NSInteger count = [item.values count];
            [slider setMinValue:0];
            [slider setMaxValue:count - 1];
            [slider setNumberOfTickMarks:count];
            [slider setAllowsTickMarkValuesOnly:YES];
            [slider setContinuous:NO];
            [slider setTarget:self];
            [slider setAction:@selector(controlChanged:)];
            control = slider;
            height = 26;
            if ([item.titles count] == 2) {
                [main addSubview:Label(item.titles[0], NSMakeRect(kControlX, y + 28, 120, 14), small, [NSColor grayColor], NSTextAlignmentLeft)];
                [main addSubview:Label(item.titles[1], NSMakeRect(kControlX + kControlWidth - 120, y + 28, 120, 14), small, [NSColor grayColor], NSTextAlignmentRight)];
                height = 42;
            }
        } else if (item.kind == PKKindSwatches) {
            NSMutableArray *buttons = [NSMutableArray array];
            for (NSUInteger i = 0; i < [item.values count]; i++) {
                PKSwatch *swatch = [[PKSwatch alloc] initWithFrame:NSMakeRect(kControlX + i * 26, y - 1, 24, 24)];
                swatch.color = item.colors[i];
                [swatch setBordered:NO];
                [swatch setTitle:@""];
                [swatch setToolTip:[NSString stringWithFormat:@"%@-%lu", item.identifier, (unsigned long)i]];
                [swatch setTag:i];
                [swatch setTarget:self];
                [swatch setAction:@selector(swatchPressed:)];
                [main addSubview:swatch];
                [buttons addObject:swatch];
            }
            _swatches[item.identifier] = buttons;
            height = 22;
        } else if (item.kind == PKKindStatic) {
            NSTextField *text = Label(@"", NSMakeRect(kControlX, y, kPaneWidth - kControlX - 20, 18), regular, [NSColor controlTextColor], NSTextAlignmentLeft);
            control = text;
            height = 18;
        }
        if (control != nil) {
            [control setToolTip:item.identifier];
            [main addSubview:control];
            _controls[item.identifier] = control;
        }
        if (item.kind != PKKindCheck && [item.title length] > 0)
            [main addSubview:Label(item.title, NSMakeRect(10, y + 2, kLabelRight - 10, 18), regular, [NSColor controlTextColor], NSTextAlignmentRight)];
        y += height;
        if ([item.help length] > 0) {
            CGFloat h = TextHeight(item.help, small, kPaneWidth - kControlX - 20);
            [main addSubview:Label(item.help, NSMakeRect(kControlX, y + 2, kPaneWidth - kControlX - 20, h), small, [NSColor grayColor], NSTextAlignmentLeft)];
            y += h + 2;
        }
        y += kRowGap;
    }
    [main setFrameSize:NSMakeSize(kPaneWidth, y + 14)];
    [self setMainView:main];
    [self refresh];
    [self mainViewDidLoad];
    return main;
}

- (PKItem *)itemForControl:(id)control
{
    NSString *identifier = [control toolTip];
    for (PKItem *item in _items)
        if ([item.identifier isEqualToString:identifier])
            return item;
    return nil;
}

- (void)refresh
{
    for (PKItem *item in _items) {
        if (item.identifier == nil)
            continue;
        NSControl *control = _controls[item.identifier];
        id stored = (item.key != nil || item.readBlock != nil) ? [item storedValue] : nil;
        id effective = stored != nil ? stored : item.defaultValue;
        if (item.kind == PKKindPopup) {
            NSInteger index = -1;
            for (NSUInteger i = 0; i < [item.values count]; i++) {
                id v = item.values[i];
                BOOL isNull = [v isKindOfClass:[NSNull class]];
                if ((isNull && stored == nil) || (!isNull && ValuesEqual(v, stored))) {
                    index = i;
                    break;
                }
            }
            if (index < 0) {
                for (NSUInteger i = 0; i < [item.values count]; i++)
                    if (ValuesEqual(item.values[i], item.defaultValue) || ([item.values[i] isKindOfClass:[NSNull class]] && item.defaultValue == nil)) {
                        index = i;
                        break;
                    }
            }
            [(NSPopUpButton *)control selectItemAtIndex:MAX(index, 0)];
        } else if (item.kind == PKKindCheck) {
            BOOL on = [effective respondsToSelector:@selector(boolValue)] ? [effective boolValue] : NO;
            [(NSButton *)control setState:(on != item.inverted) ? NSControlStateValueOn : NSControlStateValueOff];
        } else if (item.kind == PKKindSlider) {
            NSInteger best = 0;
            double distance = HUGE_VAL;
            for (NSUInteger i = 0; i < [item.values count]; i++) {
                double d = fabs([item.values[i] doubleValue] - [effective doubleValue]);
                if (d < distance) {
                    distance = d;
                    best = i;
                }
            }
            [(NSSlider *)control setIntegerValue:best];
        } else if (item.kind == PKKindSwatches) {
            for (PKSwatch *swatch in _swatches[item.identifier]) {
                id v = item.values[[swatch tag]];
                swatch.chosen = [v isKindOfClass:[NSNull class]] ? (stored == nil) : ValuesEqual(v, stored);
                [swatch setNeedsDisplay:YES];
            }
        } else if (item.kind == PKKindStatic && item.staticText != nil) {
            NSString *text = item.staticText();
            if (![[(NSTextField *)control stringValue] isEqualToString:text])
                [(NSTextField *)control setStringValue:text];
        }
    }
    [self refreshCustomViews];
}

- (void)refreshStaticItems
{
    for (PKItem *item in _items) {
        if (item.kind != PKKindStatic || item.staticText == nil)
            continue;
        NSTextField *field = (NSTextField *)_controls[item.identifier];
        NSString *text = item.staticText();
        if (![[field stringValue] isEqualToString:text])
            [field setStringValue:text];
    }
}

- (void)willSelect
{
    [super willSelect];
    CFPreferencesSynchronize(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if (_controls != nil)
        [self refresh];
}

- (void)change:(PKItem *)item to:(id)value
{
    [item storeValue:([value isKindOfClass:[NSNull class]] ? nil : value)];
    [self itemDidChange:item toValue:([value isKindOfClass:[NSNull class]] ? nil : value)];
    if (item.notification != nil)
        [PKPane postNotification:item.notification];
    [self refresh];
}

- (void)controlChanged:(id)sender
{
    PKItem *item = [self itemForControl:sender];
    if (item == nil)
        return;
    if (item.kind == PKKindPopup) {
        [self change:item to:item.values[[(NSPopUpButton *)sender indexOfSelectedItem]]];
    } else if (item.kind == PKKindCheck) {
        BOOL on = [(NSButton *)sender state] == NSControlStateValueOn;
        [self change:item to:[NSNumber numberWithBool:on != item.inverted]];
    } else if (item.kind == PKKindSlider) {
        NSInteger index = MAX(0, MIN([(NSSlider *)sender integerValue], (NSInteger)[item.values count] - 1));
        [self change:item to:item.values[index]];
    }
}

- (void)swatchPressed:(PKSwatch *)sender
{
    PKItem *item = [self itemForControl:sender];
    for (PKItem *candidate in _items)
        if (candidate.kind == PKKindSwatches && [[sender toolTip] hasPrefix:candidate.identifier])
            item = candidate;
    if (item != nil)
        [self change:item to:item.values[[sender tag]]];
}

@end
