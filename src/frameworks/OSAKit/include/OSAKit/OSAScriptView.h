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

#import <AppKit/AppKit.h>

@class OSAScriptAssistant;

@interface OSAScriptView : NSTextView
{
    BOOL _usesScriptAssistant;
    BOOL _usesTabs;
    NSUInteger _tabWidth;
    BOOL _wrapsLines;
    BOOL _indentsWrappedLines;
    NSUInteger _indentWidth;
}

@property (copy) NSString *source;
@property BOOL usesScriptAssistant;
@property BOOL usesTabs;
@property NSUInteger tabWidth;
@property BOOL wrapsLines;
@property BOOL indentsWrappedLines;
@property NSUInteger indentWidth;
// Always nil: there is no code completion engine.
@property (readonly) OSAScriptAssistant *scriptAssistant;

@end
