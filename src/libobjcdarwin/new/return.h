#ifndef OBJC_RETURN_H
#define OBJC_RETURN_H

// Does nothing, but given it's signature, it will actually return
// the last return value (contents of eax/rax on x86)
extern "C" void* returnReturn();

#endif

