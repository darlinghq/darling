// Copyright (C) 2026 Lubos Dolezel
//
// Library half of objc-protorefs-data-const: defines ProtorefProto as well, so
// the protocol exists twice in the process, and returns its reference.

#include <objc/runtime.h>

@protocol ProtorefProto
- (int)protorefValue;
@end

Protocol* protoref_lib_proto(void)
{
	return @protocol(ProtorefProto);
}
