/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

// Members of the OSAKit classes that are not part of the public API. They are declared here, in the
// source tree, so that they are implemented and used inside OSAKit but not installed with the
// public headers.

#import <OSAKit/OSAScript.h>
#import <OSAKit/OSAScriptController.h>
#import <OSAKit/OSAScriptView.h>

@class OSAScriptAssistant;

@interface OSAScript ()
// When the script was last compiled or loaded; clients compare it with -[OSATextStorage date].
@property (retain) NSDate *date;
- (NSAttributedString *)richTextFromDescriptorForLog:(NSAppleEventDescriptor *)descriptor;
@end

@interface OSAScriptController ()
// The public header declares the compiling state read-only.
@property (getter=isCompiling, setter=setIsCompiling:) BOOL compiling;
@property (retain) NSUndoManager *undoManager;
@property (retain) NSAppleEventDescriptor *defaultTarget;
@end

@interface OSAScriptView ()
// Always nil: there is no code completion engine.
@property (readonly) OSAScriptAssistant *scriptAssistant;
@end
