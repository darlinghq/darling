// Probe 0009: what SecItemCopyMatching returns and leaves in its result pointer when nothing matches, when the
// query is malformed, and when the result pointer is NULL. Read-only: it never adds or deletes an item.
#import "probe_common.h"
#import <Security/Security.h>

static NSString *statusName(OSStatus s) {
    switch (s) {
    case errSecSuccess: return @"errSecSuccess";
    case errSecItemNotFound: return @"errSecItemNotFound";
    case errSecParam: return @"errSecParam";
    case errSecMissingEntitlement: return @"errSecMissingEntitlement";
    case errSecInteractionNotAllowed: return @"errSecInteractionNotAllowed";
    case errSecNotAvailable: return @"errSecNotAvailable";
    case errSecAuthFailed: return @"errSecAuthFailed";
    default: return @"other";
    }
}

static void run(const char *label, NSDictionary *query, BOOL passNullResult) {
    // A pointer value no real call returns: if it is still there afterwards, the call did not write the result.
    CFTypeRef sentinel = (CFTypeRef)(uintptr_t)0x10;
    CFTypeRef result = sentinel;
    OSStatus s = SecItemCopyMatching((__bridge CFDictionaryRef)query, passNullResult ? NULL : &result);
    NSString *state = passNullResult ? @"result pointer was NULL"
                    : result == sentinel ? @"result left untouched"
                    : result == NULL ? @"result set to NULL"
                    : @"result set to an object";
    probe_line("0009.SecItemCopyMatching", PF(@"%s", label), PF(@"status %d (%@) ; %@", (int)s, statusName(s), state));
    if (!passNullResult && result != sentinel && result != NULL)
        CFRelease(result);
}

int main(void) {
    @autoreleasepool {
        probe_env("0009");
        NSString *unique = [[NSUUID UUID] UUIDString];
        run("generic password, unique service, return attributes, limit one",
            @{(id)kSecClass: (id)kSecClassGenericPassword, (id)kSecAttrService: unique,
              (id)kSecReturnAttributes: @YES, (id)kSecMatchLimit: (id)kSecMatchLimitOne}, NO);
        run("generic password, unique service, return data, limit all",
            @{(id)kSecClass: (id)kSecClassGenericPassword, (id)kSecAttrService: unique,
              (id)kSecReturnData: @YES, (id)kSecMatchLimit: (id)kSecMatchLimitAll}, NO);
        run("generic password, unique service, no return type requested",
            @{(id)kSecClass: (id)kSecClassGenericPassword, (id)kSecAttrService: unique}, NO);
        run("generic password, unique service, result pointer NULL",
            @{(id)kSecClass: (id)kSecClassGenericPassword, (id)kSecAttrService: unique}, YES);
        run("identity, limit all, return refs (client certificate enumeration)",
            @{(id)kSecClass: (id)kSecClassIdentity, (id)kSecReturnRef: @YES,
              (id)kSecMatchLimit: (id)kSecMatchLimitAll}, NO);
        run("certificate, unique label, return ref, limit one",
            @{(id)kSecClass: (id)kSecClassCertificate, (id)kSecAttrLabel: unique, (id)kSecReturnRef: @YES,
              (id)kSecMatchLimit: (id)kSecMatchLimitOne}, NO);
        run("key, unique label, return ref, limit one",
            @{(id)kSecClass: (id)kSecClassKey, (id)kSecAttrLabel: unique, (id)kSecReturnRef: @YES,
              (id)kSecMatchLimit: (id)kSecMatchLimitOne}, NO);
        run("internet password, unique server, return attributes",
            @{(id)kSecClass: (id)kSecClassInternetPassword, (id)kSecAttrServer: unique, (id)kSecReturnAttributes: @YES}, NO);
#if PROBE_SDK_AT_LEAST(101500)
        run("generic password, unique service, data protection keychain (needs a signed app with an entitlement)",
            @{(id)kSecClass: (id)kSecClassGenericPassword, (id)kSecAttrService: unique,
              (id)kSecUseDataProtectionKeychain: @YES, (id)kSecReturnAttributes: @YES}, NO);
#endif
        run("malformed: empty query", @{}, NO);
        run("malformed: no class, one attribute", @{(id)kSecAttrService: unique, (id)kSecReturnData: @YES}, NO);
        run("malformed: unsupported value for the class key", @{(id)kSecClass: @"not-a-class"}, NO);
        probe_line("0009.constants", @"errSecItemNotFound", PF(@"%d", (int)errSecItemNotFound));
        probe_line("0009.constants", @"errSecParam", PF(@"%d", (int)errSecParam));
    }
    return 0;
}
