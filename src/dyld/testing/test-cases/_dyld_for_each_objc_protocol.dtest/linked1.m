
#import <Foundation/Foundation.h>

@protocol DyldProtocol
@end

@protocol DyldLinkedProtocol
@end

Protocol* getLinked1DyldProtocol() {
  return @protocol(DyldProtocol);
}

Protocol* getLinked1DyldLinkedProtocol() {
  return @protocol(DyldLinkedProtocol);
}
