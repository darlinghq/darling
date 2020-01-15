#ifndef _Carbon_Scrap_H_
#define _Carbon_Scrap_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ScrapRef;
typedef uint32_t ScrapFlavorType;
typedef uint32_t ScrapFlavorFlags;
typedef uint32_t ScrapFlavorType;

OSStatus PutScrapFlavor(ScrapRef a, ScrapFlavorType b, ScrapFlavorFlags c, Size d, const void * e);

OSStatus GetScrapByName(CFStringRef a, OptionBits b, ScrapRef * c);
OSStatus GetScrapFlavorData(ScrapRef a, ScrapFlavorType b, Size * c, void * d);
OSStatus GetScrapFlavorSize(ScrapRef a, ScrapFlavorType b, Size * c);


#ifdef __cplusplus
}
#endif

#endif
