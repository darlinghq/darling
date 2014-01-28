#if !__LP64__
#import <AppKit/NSView.h>

@interface NSQuickDrawView : NSView
{
@private
    void* _qdPort;
    void* _savePort;
    BOOL _synchToView;
}

- (void*) qdPort ;

@end
#endif
