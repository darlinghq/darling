#import <Foundation/NSObject.h>

@interface NSNibConnector : NSObject <NSCoding>
{
    @private
    id _source;
    id _destination;
    NSString *_label;
}
- (id)source;
- (void)setSource:(id)source;
- (id)destination;
- (void)setDestination:(id)destination;
- (NSString *)label;
- (void)setLabel:(NSString *)label;
- (void)replaceObject:(id)oldObject withObject:(id)newObject;
- (void)establishConnection;
@end
