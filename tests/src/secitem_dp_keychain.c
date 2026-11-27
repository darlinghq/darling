// Copyright (C) 2026 Lubos Dolezel
//
// SecItem password items in the data protection keychain (securityd's database),
// one step per run, so that the host driver (secitem-dp-keychain.py) shows that the
// items persist from one process to the next:
//
//   secitem_dp_keychain step...   (add | add-again | copy | update | copy-updated | delete | gone | clean)
//
// The items are in the data protection keychain, the one securityd's database serves. The items are a generic password (service, account, text value) and an
// internet password (server, protocol, port, path, account, binary value that
// holds a NUL and 0xff). Each step prints PASS or FAIL lines and exits non-zero
// on a failure. The documented SecItem contract applies whichever store holds
// the items: a second add is errSecDuplicateItem, copy returns the stored data,
// update changes the data and the attributes, delete removes the item and a
// later copy is errSecItemNotFound.
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdio.h>
#include <string.h>

// The deployment target of the 32-bit build predates the key; Darling provides it in both slices.
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok)
        failures++;
}

static const char kService[] = "org.darlinghq.test.dpkeychain";
static const UInt8 kGenericValue[] = "s3cr3t-text-value";
static const UInt8 kGenericUpdated[] = "updated-text-value";
static const UInt8 kInternetValue[] = {'b', 'i', 'n', 0x00, 0xff, 0x7f, 'z'};

static CFStringRef str(const char *s) {
    return CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);
}

static CFMutableDictionaryRef newDict(void) {
    return CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

static void put(CFMutableDictionaryRef d, CFTypeRef key, CFTypeRef value) {
    CFDictionarySetValue(d, key, value);
    CFRelease(value);
}

static CFMutableDictionaryRef genericIdentity(void) {
    CFMutableDictionaryRef d = newDict();
    CFDictionarySetValue(d, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(d, kSecUseDataProtectionKeychain, kCFBooleanTrue);
    put(d, kSecAttrService, str(kService));
    put(d, kSecAttrAccount, str("alice"));
    return d;
}

static CFMutableDictionaryRef internetIdentity(void) {
    CFMutableDictionaryRef d = newDict();
    int port = 8443;
    CFDictionarySetValue(d, kSecClass, kSecClassInternetPassword);
    CFDictionarySetValue(d, kSecUseDataProtectionKeychain, kCFBooleanTrue);
    put(d, kSecAttrServer, str("dpkeychain.example.test"));
    put(d, kSecAttrAccount, str("bob"));
    put(d, kSecAttrPath, str("/inbox"));
    put(d, kSecAttrPort, CFNumberCreate(NULL, kCFNumberIntType, &port));
    CFDictionarySetValue(d, kSecAttrProtocol, kSecAttrProtocolHTTPS);
    return d;
}

static OSStatus add(CFMutableDictionaryRef identity, const UInt8 *value, size_t length, const char *comment) {
    CFMutableDictionaryRef d = CFDictionaryCreateMutableCopy(NULL, 0, identity);
    put(d, kSecValueData, CFDataCreate(NULL, value, (CFIndex) length));
    if (comment != NULL)
        put(d, kSecAttrComment, str(comment));
    OSStatus status = SecItemAdd(d, NULL);
    if (status != errSecSuccess)
        printf("SecItemAdd returned %d\n", (int) status);
    CFRelease(d);
    return status;
}

static OSStatus copyData(CFMutableDictionaryRef identity, CFDataRef *out) {
    CFMutableDictionaryRef q = CFDictionaryCreateMutableCopy(NULL, 0, identity);
    CFDictionarySetValue(q, kSecReturnData, kCFBooleanTrue);
    CFDictionarySetValue(q, kSecMatchLimit, kSecMatchLimitOne);
    CFTypeRef result = NULL;
    OSStatus status = SecItemCopyMatching(q, &result);
    CFRelease(q);
    if (status != errSecSuccess)
        printf("SecItemCopyMatching returned %d\n", (int) status);
    *out = status == errSecSuccess ? (CFDataRef) result : NULL;
    return status;
}

static OSStatus deleteItem(CFMutableDictionaryRef identity) {
    OSStatus status = SecItemDelete(identity);
    if (status != errSecSuccess)
        printf("SecItemDelete returned %d\n", (int) status);
    return status;
}

// SecItemCopyMatching asking for the attributes and the data of one item
static OSStatus copyAttributes(CFMutableDictionaryRef identity, CFDictionaryRef *out) {
    CFMutableDictionaryRef q = CFDictionaryCreateMutableCopy(NULL, 0, identity);
    CFDictionarySetValue(q, kSecReturnAttributes, kCFBooleanTrue);
    CFDictionarySetValue(q, kSecReturnData, kCFBooleanTrue);
    CFDictionarySetValue(q, kSecMatchLimit, kSecMatchLimitOne);
    CFTypeRef result = NULL;
    OSStatus status = SecItemCopyMatching(q, &result);
    CFRelease(q);
    if (status != errSecSuccess)
        printf("SecItemCopyMatching (attributes) returned %d\n", (int) status);
    *out = status == errSecSuccess ? (CFDictionaryRef) result : NULL;
    return status;
}

static int stringIs(CFDictionaryRef d, CFStringRef key, const char *expected) {
    CFStringRef value = d != NULL ? CFDictionaryGetValue(d, key) : NULL;
    return value != NULL && CFGetTypeID(value) == CFStringGetTypeID() &&
           CFStringGetLength(value) == (CFIndex) strlen(expected) &&
           CFStringHasPrefix(value, CFStringCreateWithCString(NULL, expected, kCFStringEncodingUTF8));
}

static int dataIs(CFDataRef data, const UInt8 *bytes, size_t length) {
    return data != NULL && CFDataGetLength(data) == (CFIndex) length && memcmp(CFDataGetBytePtr(data), bytes, length) == 0;
}

static OSStatus update(CFMutableDictionaryRef identity, const UInt8 *value, size_t length, const char *comment) {
    CFMutableDictionaryRef changes = newDict();
    put(changes, kSecValueData, CFDataCreate(NULL, value, (CFIndex) length));
    put(changes, kSecAttrComment, str(comment));
    OSStatus status = SecItemUpdate(identity, changes);
    if (status != errSecSuccess)
        printf("SecItemUpdate returned %d\n", (int) status);
    CFRelease(changes);
    return status;
}

static int runStep(const char *step) {
    CFMutableDictionaryRef generic = genericIdentity();
    CFMutableDictionaryRef internet = internetIdentity();
    CFDataRef data = NULL;
    OSStatus status;

    if (strcmp(step, "add") == 0) {
        expect(add(generic, kGenericValue, sizeof(kGenericValue) - 1, "first") == errSecSuccess,
               "adding a generic password succeeds");
        expect(add(internet, kInternetValue, sizeof(kInternetValue), NULL) == errSecSuccess,
               "adding an internet password succeeds");
    } else if (strcmp(step, "add-again") == 0) {
        expect(add(generic, kGenericValue, sizeof(kGenericValue) - 1, NULL) == errSecDuplicateItem,
               "adding the generic password again is errSecDuplicateItem");
        expect(add(internet, kInternetValue, sizeof(kInternetValue), NULL) == errSecDuplicateItem,
               "adding the internet password again is errSecDuplicateItem");
    } else if (strcmp(step, "copy") == 0 || strcmp(step, "copy-updated") == 0) {
        int updated = strcmp(step, "copy-updated") == 0;
        status = copyData(generic, &data);
        expect(status == errSecSuccess, "the generic password is found");
        expect(updated ? dataIs(data, kGenericUpdated, sizeof(kGenericUpdated) - 1)
                       : dataIs(data, kGenericValue, sizeof(kGenericValue) - 1),
               "the generic password's data is returned unchanged");
        if (data != NULL)
            CFRelease(data);
        status = copyData(internet, &data);
        expect(status == errSecSuccess, "the internet password is found");
        expect(dataIs(data, kInternetValue, sizeof(kInternetValue)), "binary data with a NUL and 0xff round trips");
        if (data != NULL)
            CFRelease(data);
        CFDictionaryRef attributes = NULL;
        status = copyAttributes(generic, &attributes);
        expect(status == errSecSuccess && stringIs(attributes, kSecAttrAccount, "alice") &&
                   stringIs(attributes, kSecAttrService, kService) &&
                   stringIs(attributes, kSecAttrComment, updated ? "second" : "first"),
               "the generic password's attributes are returned");
        expect(attributes != NULL && dataIs(CFDictionaryGetValue(attributes, kSecValueData),
                   updated ? kGenericUpdated : kGenericValue, (updated ? sizeof(kGenericUpdated) : sizeof(kGenericValue)) - 1),
               "the data comes with the attributes");
        if (attributes != NULL)
            CFRelease(attributes);
        status = copyAttributes(internet, &attributes);
        int port = 0;
        CFNumberRef number = attributes != NULL ? CFDictionaryGetValue(attributes, kSecAttrPort) : NULL;
        if (number != NULL && CFGetTypeID(number) == CFNumberGetTypeID())
            CFNumberGetValue(number, kCFNumberIntType, &port);
        expect(status == errSecSuccess && port == 8443 && stringIs(attributes, kSecAttrServer, "dpkeychain.example.test"),
               "the internet password's server and port are returned");
        if (attributes != NULL)
            CFRelease(attributes);
        CFMutableDictionaryRef wrong = genericIdentity();
        put(wrong, kSecAttrAccount, str("nobody"));
        status = copyData(wrong, &data);
        expect(status == errSecItemNotFound, "another account is errSecItemNotFound");
        CFRelease(wrong);
    } else if (strcmp(step, "update") == 0) {
        expect(update(generic, kGenericUpdated, sizeof(kGenericUpdated) - 1, "second") == errSecSuccess,
               "updating the data and an attribute succeeds");
        CFMutableDictionaryRef missing = genericIdentity();
        put(missing, kSecAttrAccount, str("nobody"));
        expect(update(missing, kGenericUpdated, sizeof(kGenericUpdated) - 1, "x") == errSecItemNotFound,
               "updating a missing item is errSecItemNotFound");
        CFRelease(missing);
    } else if (strcmp(step, "delete") == 0) {
        expect(deleteItem(generic) == errSecSuccess, "deleting the generic password succeeds");
        expect(deleteItem(internet) == errSecSuccess, "deleting the internet password succeeds");
        expect(deleteItem(generic) == errSecItemNotFound, "deleting it again is errSecItemNotFound");
    } else if (strcmp(step, "clean") == 0) {
        // leftovers of an earlier run; the results do not matter
        SecItemDelete(generic);
        SecItemDelete(internet);
    } else if (strcmp(step, "gone") == 0) {
        expect(copyData(generic, &data) == errSecItemNotFound, "the deleted generic password is not found");
        expect(copyData(internet, &data) == errSecItemNotFound, "the deleted internet password is not found");
    } else {
        fprintf(stderr, "unknown step %s\n", step);
        failures++;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: secitem_dp_keychain step...\n");
        return 2;
    }
    for (int i = 1; i < argc; i++)
        runStep(argv[i]);
    printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    return failures != 0;
}
