#import <AppKit/AppKit.h>

// Sidebar icons Console shows next to its report folders and files. Darling has no system icon
// catalog of its own, so these come from the generic workspace icons AppKit provides.
@implementation NSImage (CSKSidebarIcons)

+ (NSImage *)csk_sidebarGenericFolderIcon
{
	return [NSImage imageNamed:NSImageNameFolder];
}

+ (NSImage *)csk_sidebarGenericFileIcon
{
	return [[NSWorkspace sharedWorkspace] iconForFileType:@"public.data"];
}

+ (NSImage *)csk_sidebarCrashReportIcon
{
	return [[NSWorkspace sharedWorkspace] iconForFileType:@"crash"];
}

+ (NSImage *)csk_sidebarSpinReportIcon
{
	return [[NSWorkspace sharedWorkspace] iconForFileType:@"spin"];
}

+ (NSImage *)csk_sidebarLogReportIcon
{
	return [[NSWorkspace sharedWorkspace] iconForFileType:@"log"];
}

+ (NSImage *)csk_sidebarMiscReportIcon
{
	return [[NSWorkspace sharedWorkspace] iconForFileType:@"diag"];
}

+ (NSImage *)csk_sidebarMacAnalyticsReportIcon
{
	return [[NSWorkspace sharedWorkspace] iconForFileType:@"ips"];
}

@end

@implementation NSView (CSKLayout)

// Pins the view to its superview's edges. Cocotron does not solve Auto Layout constraints, so the
// same result is expressed with autoresizing, which it does honor.
- (void)csk_activateTiedConstraintsToSuperview
{
	NSView *superview = self.superview;
	if (superview == nil)
		return;
	self.translatesAutoresizingMaskIntoConstraints = YES;
	self.frame = superview.bounds;
	self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
}

@end
