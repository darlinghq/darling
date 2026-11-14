// Round trips between NSData and dispatch data with the Network framework's conversion helpers that the Network
// Swift overlay uses: nil in gives nil/NULL out, the bytes are copied, and dispatch data made of several regions
// becomes one contiguous NSData. Prints PASS when everything holds; the exit code is the number of failed checks.
#import <Foundation/Foundation.h>
#import <Network/Network.h>
#include <string.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

int main(void)
{
    @autoreleasepool {
        CHECK(NWCreateNSDataFromDispatchData(NULL) == nil);
        CHECK(NWCreateDispatchDataFromNSData(nil) == NULL);

        NSData *empty = NWCreateNSDataFromDispatchData(dispatch_data_empty);
        CHECK(empty != nil && [empty length] == 0);

        uint8_t bytes[300];
        for (int i = 0; i < 300; i++)
            bytes[i] = (uint8_t)i;
        NSMutableData *source = [NSMutableData dataWithBytes:bytes length:sizeof(bytes)];
        dispatch_data_t converted = NWCreateDispatchDataFromNSData(source);
        CHECK(converted != NULL && dispatch_data_get_size(converted) == sizeof(bytes));

        // The conversion copies: changing the source afterwards does not change the dispatch data.
        ((uint8_t *)[source mutableBytes])[0] = 0xff;
        const void *mapped = NULL;
        size_t mappedSize = 0;
        dispatch_data_t map = dispatch_data_create_map(converted, &mapped, &mappedSize);
        CHECK(mappedSize == sizeof(bytes) && memcmp(mapped, bytes, sizeof(bytes)) == 0);
        (void)map;

        // Two regions concatenated come back as one NSData in order.
        dispatch_data_t first = dispatch_data_create(bytes, 100, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
        dispatch_data_t second = dispatch_data_create(bytes + 100, 200, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
        dispatch_data_t both = dispatch_data_create_concat(first, second);
        NSData *joined = NWCreateNSDataFromDispatchData(both);
        CHECK([joined length] == sizeof(bytes) && memcmp([joined bytes], bytes, sizeof(bytes)) == 0);
    }
    if (!failures)
        printf("PASS\n");
    return failures;
}
