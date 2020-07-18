#import <Foundation/Foundation.h>

#import <Metal/MTLCommandBuffer.h>
#import <Metal/MTLDevice.h>

@protocol MTLDevice;

@protocol MTLCommandQueue<NSObject>

@property(readonly) id<MTLDevice> device;
@property(copy, atomic) NSString *label;

- (id<MTLCommandBuffer>)commandBuffer;
- (id<MTLCommandBuffer>)commandBufferWithUnretainedReferences;

@end
