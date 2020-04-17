
#import <Foundation/Foundation.h>

@protocol DyldProtocol
@end

@protocol DyldLinkedProtocol
@end

Protocol* getLinked2DyldProtocol() {
  return @protocol(DyldProtocol);
}

Protocol* getLinked2DyldLinkedProtocol() {
  return @protocol(DyldLinkedProtocol);
}
