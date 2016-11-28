
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>


class IOKitDiagnosticsClient : public IOUserClient
{
    OSDeclareDefaultStructors(IOKitDiagnosticsClient)

public:
    static  IOUserClient * withTask(task_t owningTask);
    virtual IOReturn       clientClose(void) APPLE_KEXT_OVERRIDE;
    virtual IOReturn       setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;
    virtual IOReturn       externalMethod(uint32_t selector, IOExternalMethodArguments * args,
                                          IOExternalMethodDispatch * dispatch, OSObject * target, void * reference) APPLE_KEXT_OVERRIDE;
};

