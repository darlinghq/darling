// Probe item-d: the integer values of the CGPDFObjectType enumerators, by their public names, and the type
// CGPDFObjectGetType reports for objects found in a small PDF this program writes itself.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>

static void env(void) {
    struct utsname u;
    uname(&u);
    char ver[64] = "?", build[64] = "?";
    size_t n = sizeof ver;
    sysctlbyname("kern.osproductversion", ver, &n, NULL, 0);
    n = sizeof build;
    sysctlbyname("kern.osversion", build, &n, NULL, 0);
    printf("PROBE item-d env -> macOS %s (build %s) ; machine %s\n", ver, build, u.machine);
}

#define C(name) printf("PROBE item-d.constant %s -> %d\n", #name, (int)name)

static const char *typeName(CGPDFObjectType t) {
    switch (t) {
    case kCGPDFObjectTypeNull: return "kCGPDFObjectTypeNull";
    case kCGPDFObjectTypeBoolean: return "kCGPDFObjectTypeBoolean";
    case kCGPDFObjectTypeInteger: return "kCGPDFObjectTypeInteger";
    case kCGPDFObjectTypeReal: return "kCGPDFObjectTypeReal";
    case kCGPDFObjectTypeName: return "kCGPDFObjectTypeName";
    case kCGPDFObjectTypeString: return "kCGPDFObjectTypeString";
    case kCGPDFObjectTypeArray: return "kCGPDFObjectTypeArray";
    case kCGPDFObjectTypeDictionary: return "kCGPDFObjectTypeDictionary";
    case kCGPDFObjectTypeStream: return "kCGPDFObjectTypeStream";
    default: return "(unknown)";
    }
}

static void report(const char *what, CGPDFObjectRef o) {
    if (o == NULL) { printf("PROBE item-d.found %s -> (key absent)\n", what); return; }
    CGPDFObjectType t = CGPDFObjectGetType(o);
    printf("PROBE item-d.found %s -> %d %s\n", what, (int)t, typeName(t));
}

int main(void) {
    env();
    C(kCGPDFObjectTypeNull);
    C(kCGPDFObjectTypeBoolean);
    C(kCGPDFObjectTypeInteger);
    C(kCGPDFObjectTypeReal);
    C(kCGPDFObjectTypeName);
    C(kCGPDFObjectTypeString);
    C(kCGPDFObjectTypeArray);
    C(kCGPDFObjectTypeDictionary);
    C(kCGPDFObjectTypeStream);

    CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
    CGDataConsumerRef consumer = CGDataConsumerCreateWithCFData(data);
    CGRect box = CGRectMake(0, 0, 200, 100);
    CFMutableDictionaryRef aux = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(aux, kCGPDFContextTitle, CFSTR("probe title"));
    CGContextRef pdf = CGPDFContextCreate(consumer, &box, aux);
    CGPDFContextBeginPage(pdf, NULL);
    CGContextSetRGBFillColor(pdf, 1, 0, 0, 1);
    CGContextFillRect(pdf, CGRectMake(10, 10, 50, 50));
    CGPDFContextEndPage(pdf);
    CGPDFContextClose(pdf);
    CGContextRelease(pdf);
    CGDataConsumerRelease(consumer);
    CFRelease(aux);

    CGDataProviderRef dp = CGDataProviderCreateWithCFData(data);
    CGPDFDocumentRef doc = CGPDFDocumentCreateWithProvider(dp);
    if (doc == NULL) { printf("PROBE item-d.found document -> could not parse the PDF the probe wrote\n"); return 1; }
    printf("PROBE item-d.pages -> %zu\n", CGPDFDocumentGetNumberOfPages(doc));
    CGPDFDictionaryRef catalog = CGPDFDocumentGetCatalog(doc);
    CGPDFObjectRef o = NULL;
    CGPDFDictionaryGetObject(catalog, "Type", &o);   report("catalog /Type (a name)", o);
    o = NULL; CGPDFDictionaryGetObject(catalog, "Pages", &o); report("catalog /Pages (a dictionary)", o);
    CGPDFDictionaryRef pages = NULL;
    if (CGPDFDictionaryGetDictionary(catalog, "Pages", &pages)) {
        o = NULL; CGPDFDictionaryGetObject(pages, "Count", &o); report("pages /Count (an integer)", o);
        o = NULL; CGPDFDictionaryGetObject(pages, "Kids", &o);  report("pages /Kids (an array)", o);
    }
    CGPDFPageRef page = CGPDFDocumentGetPage(doc, 1);
    CGPDFDictionaryRef pd = page ? CGPDFPageGetDictionary(page) : NULL;
    if (pd) {
        o = NULL; CGPDFDictionaryGetObject(pd, "MediaBox", &o); report("page /MediaBox (an array)", o);
        o = NULL; CGPDFDictionaryGetObject(pd, "Contents", &o); report("page /Contents (a stream or an array of streams)", o);
        o = NULL; CGPDFDictionaryGetObject(pd, "Resources", &o); report("page /Resources (a dictionary)", o);
        o = NULL; CGPDFDictionaryGetObject(pd, "Nothing", &o);   report("page /Nothing (absent)", o);
        CGPDFArrayRef mb = NULL;
        if (CGPDFDictionaryGetArray(pd, "MediaBox", &mb) && CGPDFArrayGetCount(mb) > 0) {
            o = NULL; CGPDFArrayGetObject(mb, 2, &o); report("page /MediaBox[2] (a number: integer or real)", o);
        }
    }
    CGPDFDictionaryRef info = CGPDFDocumentGetInfo(doc);
    if (info) { o = NULL; CGPDFDictionaryGetObject(info, "Title", &o); report("info /Title (a string)", o); }
    CGPDFDocumentRelease(doc);
    CGDataProviderRelease(dp);
    CFRelease(data);
    return 0;
}
