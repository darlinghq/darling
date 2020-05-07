#import <ScriptingBridge/SBObject.h>

@implementation SBObject

- (instancetype) init {
    return nil;
}
- (instancetype) initWithProperties:(NSDictionary *)properties {
    return nil;
}

- (instancetype) initWithData:(id)data {
    return nil;
}

- (nullable id) get {
    return nil;
}

- (nullable NSError *) lastError {
    return nil;
}

- (instancetype) initWithElementCode:(DescType)code properties:(nullable NSDictionary<NSString *, id> *)properties data:(nullable id)data {
    return nil;
}

- (SBObject *) propertyWithCode:(AEKeyword)code {
    return nil;
}

- (SBObject *) propertyWithClass:(Class)cls code:(AEKeyword)code {
    return nil;
}

- (SBElementArray *) elementArrayWithCode:(DescType)code {
    return nil;
}

- (id) sendEvent:(AEEventClass)eventClass id:(AEEventID)eventID parameters:(DescType)firstParamCode, ... {
    return nil;
}

- (void) setTo:(nullable id)value {

}

@end
