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
#import <OSAKit/OSAScript.h>
#import <OSAKit/OSAScriptView.h>

typedef NS_ENUM(NSInteger, OSAScriptState) {
    OSAScriptStopped,
    OSAScriptRunning,
    OSAScriptRecording,
};

@interface OSAScriptController : NSController

@property (assign) OSAScriptView *scriptView;
@property (assign) NSTextView *resultView;
@property (retain) OSAScript *script;
@property (retain) OSALanguage *language;
@property (readonly) OSAScriptState scriptState;
@property (readonly, getter=isCompiling) BOOL compiling;
// Not in the public header; Script Editor sets its document's undo manager here.
@property (retain) NSUndoManager *undoManager;

- (IBAction)compileScript:(id)sender;
- (IBAction)recordScript:(id)sender;
- (IBAction)runScript:(id)sender;
- (IBAction)stopScript:(id)sender;

@end
