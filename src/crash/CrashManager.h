#include <AppKit/AppKit.h>

@interface CrashManager : NSObject
{
	id descriptionLabel;
	NSString* executablePath;
	NSString* commandLine;
	NSString* shortDescription;
	NSString* fullLog;
}
- (void) close: (id)sender;
- (void) debugInfo: (id)sender;
- (void) reopen: (id)sender;
- (id)   init;
@end
