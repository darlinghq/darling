#import <Foundation/NSAffineTransform.h>

@class NSBezierPath;

@interface NSAffineTransform (NSAppKitAdditons)

- (NSBezierPath *)transformBezierPath:(NSBezierPath *)aPath;


- (void)set;
- (void)concat;
@end
