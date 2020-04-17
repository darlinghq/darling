/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <string.h>

#include <string>
#include <map>
#include <vector>

#include "ClosurePrinter.h"
#include "JSONWriter.h"

#include "objc-shared-cache.h"

using namespace dyld3::json;

namespace dyld3 {
namespace closure {

static std::string printTarget(const Array<const ImageArray*>& imagesArrays, Image::ResolvedSymbolTarget target)
{
	const Image* targetImage;
	uint64_t value;
    switch ( target.image.kind ) {
        case Image::ResolvedSymbolTarget::kindImage:
            targetImage = ImageArray::findImage(imagesArrays, target.image.imageNum);
            if ( target.image.offset & 0x8000000000ULL ) {
                uint64_t signExtend = target.image.offset | 0xFFFFFF0000000000ULL;
                return std::string("bind to ") + targetImage->leafName() + " - " + hex8(-signExtend);
            }
            else
                return std::string("bind to ") + targetImage->leafName() + " + " + hex8(target.image.offset);
            break;
        case Image::ResolvedSymbolTarget::kindSharedCache:
            return std::string("bind to dyld cache + ") + hex8(target.sharedCache.offset);
            break;
        case Image::ResolvedSymbolTarget::kindAbsolute:
            value = target.absolute.value;
            if ( value & 0x2000000000000000LL )
                value |= 0xC000000000000000LL;
            return std::string("bind to absolute ") + hex(value);
            break;
    }
    return "???";
}

static const char* nameForType(TypedBytes::Type type) {
    switch (type) {
    // containers
    case TypedBytes::Type::launchClosure:
        return "launchClosure";
    case TypedBytes::Type::imageArray:
        return "imageArray";
    case TypedBytes::Type::image:
        return "image";
    case TypedBytes::Type::dlopenClosure:
        return "dlopenClosure";
    // attributes for Images
    case TypedBytes::Type::imageFlags:
        return "imageFlags";
    case TypedBytes::Type::pathWithHash:
        return "pathWithHash";
    case TypedBytes::Type::fileInodeAndTime:
        return "fileInodeAndTime";
    case TypedBytes::Type::cdHash:
        return "cdHash";
    case TypedBytes::Type::uuid:
        return "uuid";
    case TypedBytes::Type::mappingInfo:
        return "mappingInfo";
    case TypedBytes::Type::diskSegment:
        return "diskSegment";
    case TypedBytes::Type::cacheSegment:
        return "cacheSegment";
    case TypedBytes::Type::dependents:
        return "dependents";
    case TypedBytes::Type::initOffsets:
        return "initOffsets";
    case TypedBytes::Type::dofOffsets:
        return "dofOffsets";
    case TypedBytes::Type::codeSignLoc:
        return "codeSignLoc";
    case TypedBytes::Type::fairPlayLoc:
        return "fairPlayLoc";
    case TypedBytes::Type::rebaseFixups:
        return "rebaseFixups";
    case TypedBytes::Type::bindFixups:
        return "bindFixups";
    case TypedBytes::Type::cachePatchInfo:
        return "cachePatchInfo";
    case TypedBytes::Type::textFixups:
        return "textFixups";
    case TypedBytes::Type::imageOverride:
        return "imageOverride";
    case TypedBytes::Type::initBefores:
        return "initBefores";
    case TypedBytes::Type::initsSection:
        return "initSection";
    case TypedBytes::Type::chainedFixupsTargets:
        return "chainedFixupsTargets";
    case TypedBytes::Type::termOffsets:
        return "termOffsets";
    case TypedBytes::Type::chainedStartsOffset:
        return "chainedStartsOffset";
    case TypedBytes::Type::objcFixups:
        return "objcFixups";
    // attributes for Closures (launch or dlopen)
    case TypedBytes::Type::closureFlags:
        return "closureFlags";
    case TypedBytes::Type::dyldCacheUUID:
        return "dyldCacheUUID";
    case TypedBytes::Type::missingFiles:
        return "missingFiles";
    case TypedBytes::Type::envVar:
        return "envVar";
    case TypedBytes::Type::topImage:
        return "topImage";
    case TypedBytes::Type::libDyldEntry:
        return "libDyldEntry";
    case TypedBytes::Type::libSystemNum:
        return "libSystemNum";
    case TypedBytes::Type::bootUUID:
        return "bootUUID";
    case TypedBytes::Type::mainEntry:
        return "mainEntry";
    case TypedBytes::Type::startEntry:
        return "startEntry";
    case TypedBytes::Type::cacheOverrides:
        return "cacheOverrides";
    case TypedBytes::Type::interposeTuples:
        return "interposeTuples";
    case TypedBytes::Type::existingFiles:
        return "existingFiles";
    case TypedBytes::Type::selectorTable:
        return "selectorTable";
    case TypedBytes::Type::classTable:
            return "classTable";
    case TypedBytes::Type::warning:
        return "warning";
    case TypedBytes::Type::duplicateClassesTable:
        return "duplicateClassesTable";
    }
}

static Node buildImageNode(const Image* image, const Array<const ImageArray*>& imagesArrays, bool printFixups,
                           bool printDependentsDetails, bool printRaw,
                           const DyldSharedCache* dyldCache, const closure::ObjCSelectorOpt* selOpt,
                           const Array<closure::Image::ObjCSelectorImage>& selImages)
{
    __block Node imageNode;

    if ( image->isInvalid() )
        return imageNode;

    imageNode.map["image-num"].value = hex4(image->imageNum());
    imageNode.map["path"].value = image->path();

    if (printRaw) {
        __block Node attributes;
        image->forEachAttribute(^(const TypedBytes *typedBytes, bool &stop) {
            Node anAttribute;
            anAttribute.map["type"].value = decimal((uint32_t)typedBytes->type);
            anAttribute.map["type-name"].value = nameForType((TypedBytes::Type)typedBytes->type);
            anAttribute.map["length"].value = decimal(typedBytes->payloadLength);
            attributes.array.push_back(anAttribute);
        });
        imageNode.map["attributes"] = attributes;
        return imageNode;
    }

    __block Node imageAliases;
    image->forEachAlias(^(const char* aliasPath, bool& stop) {
        Node anAlias;
        anAlias.value = aliasPath;
        imageAliases.array.push_back(anAlias);
    });
    if ( !imageAliases.array.empty() )
        imageNode.map["aliases"] = imageAliases;
    uuid_t uuid;
    if ( image->getUuid(uuid) ) {
        uuid_string_t uuidStr;
        uuid_unparse(uuid, uuidStr);
        imageNode.map["uuid"].value = uuidStr;
    }
    imageNode.map["has-objc"].value = (image->hasObjC() ? "true" : "false");
    imageNode.map["has-weak-defs"].value = (image->hasWeakDefs() ? "true" : "false");
    imageNode.map["has-plus-loads"].value = (image->mayHavePlusLoads() ? "true" : "false");
    imageNode.map["never-unload"].value = (image->neverUnload() ? "true" : "false");
    imageNode.map["has-precomputed-objc"].value = (image->hasPrecomputedObjC() ? "true" : "false");
//    imageNode.map["platform-binary"].value = (image->isPlatformBinary() ? "true" : "false");
//    if ( image->cwdMustBeThisDir() )
//        imageNode.map["cwd-must-be-this-dir"].value = "true";
    if ( !image->inDyldCache() ) {
        uint32_t csFileOffset;
        uint32_t csSize;
        if ( image->hasCodeSignature(csFileOffset, csSize) ) {
            imageNode.map["code-sign-location"].map["offset"].value = hex(csFileOffset);
            imageNode.map["code-sign-location"].map["size"].value = hex(csSize);
        }
//        uint32_t fpTextOffset;
//        uint32_t fpSize;
//        if ( image->isFairPlayEncrypted(fpTextOffset, fpSize) ) {
//            imageNode.map["fairplay-encryption-location"].map["offset"].value = hex(fpTextOffset);
//            imageNode.map["fairplay-encryption-location"].map["size"].value = hex(fpSize);
//        }
        uint64_t inode;
        uint64_t mTime;
        if ( image->hasFileModTimeAndInode(inode, mTime) ) {
            imageNode.map["file-mod-time"].value = hex(inode);
            imageNode.map["file-inode"].value = hex(mTime);
        }
        image->forEachCDHash(^(const uint8_t *cdHash, bool& stop) {
            std::string cdHashStr;
            cdHashStr.reserve(24);
            for (int i=0; i < 20; ++i) {
                uint8_t byte = cdHash[i];
                uint8_t nibbleL = byte & 0x0F;
                uint8_t nibbleH = byte >> 4;
                if ( nibbleH < 10 )
                    cdHashStr += '0' + nibbleH;
                else
                    cdHashStr += 'a' + (nibbleH-10);
                if ( nibbleL < 10 )
                    cdHashStr += '0' + nibbleL;
                else
                    cdHashStr += 'a' + (nibbleL-10);
            }
            if ( cdHashStr != "0000000000000000000000000000000000000000" ) {
                Node hashNode;
                hashNode.value = cdHashStr;
                imageNode.map["cd-hashes"].array.push_back(hashNode);
            }
        });
        imageNode.map["total-vm-size"].value = hex(image->vmSizeToMap());
        uint64_t sliceOffset = image->sliceOffsetInFile();
        if ( sliceOffset != 0 )
            imageNode.map["file-offset-of-slice"].value = hex(sliceOffset);
        //if ( image->hasTextRelocs() )
        //    imageNode.map["has-text-relocs"].value = "true";
        image->forEachDiskSegment(^(uint32_t segIndex, uint32_t fileOffset, uint32_t fileSize, int64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool laterReadOnly, bool& stop) {
            Node segInfoNode;
            segInfoNode.map["file-offset"].value = hex(fileOffset);
            segInfoNode.map["file-size"].value = hex(fileSize);
            segInfoNode.map["vm-size"].value = hex(vmSize);
            imageNode.map["mappings"].array.push_back(segInfoNode);
            switch ( permissions ) {
                case 0:
                    segInfoNode.map["permissions"].value = "--";
                    break;
                case 1:
                    segInfoNode.map["permissions"].value = "r";
                    break;
                case 2:
                    segInfoNode.map["permissions"].value = "ro";  // r/w then r/o
                    break;
                case 3:
                    segInfoNode.map["permissions"].value = "rw";
                    break;
                case 4:
                    segInfoNode.map["permissions"].value = "rx";
                    break;
                default:
                    segInfoNode.map["permissions"].value = "??";
            }
        });


        if ( printFixups ) {
            image->forEachFixup(^(uint64_t imageOffsetToRebase, bool &stop) {
                // rebase
                imageNode.map["fixups"].map[hex8(imageOffsetToRebase)].value = "rebase";
            }, ^(uint64_t imageOffsetToBind, Image::ResolvedSymbolTarget target, bool &stop) {
                // bind
                imageNode.map["fixups"].map[hex8(imageOffsetToBind)].value = printTarget(imagesArrays, target);
            }, ^(uint64_t startsStructImageOffset, const Array<Image::ResolvedSymbolTarget>& targets, bool& stop) {
                // chain
                imageNode.map["fixups-chain-starts-offset"].value = hex8(startsStructImageOffset);
                for (const Image::ResolvedSymbolTarget& target: targets) {
                    Node targetNode;
                    targetNode.value = printTarget(imagesArrays, target);
                    imageNode.map["fixups-targets"].array.push_back(targetNode);
                }

            }, ^(uint64_t imageOffsetToFixup) {
                // fixupObjCImageInfo
                imageNode.map["fixups"].map[hex8(imageOffsetToFixup)].value = "objc image info pre-optimized by dyld flag";
            }, ^(uint64_t imageOffsetToBind, Image::ResolvedSymbolTarget target, bool &stop) {
                // fixupObjCProtocol
                imageNode.map["fixups"].map[hex8(imageOffsetToBind)].value = printTarget(imagesArrays, target);
            }, ^(uint64_t imageOffsetToFixup, uint32_t selectorIndex, bool inSharedCache, bool &stop) {
                // fixupObjCSelRefs
                Image::ResolvedSymbolTarget target;
                if ( inSharedCache ) {
                    const char* selectorString = dyldCache->objcOpt()->selopt()->getEntryForIndex(selectorIndex);
                    target.sharedCache.kind     = Image::ResolvedSymbolTarget::kindSharedCache;
                    target.sharedCache.offset   = (uint64_t)selectorString - (uint64_t)dyldCache;
                } else {
                    ImageNum imageNum;
                    uint64_t vmOffset;
                    bool gotLocation = selOpt->getStringLocation(selectorIndex, selImages, imageNum, vmOffset);
                    assert(gotLocation);
                    target.image.kind = Image::ResolvedSymbolTarget::kindImage;
                    target.image.imageNum = imageNum;
                    target.image.offset = vmOffset;
                }
                imageNode.map["fixups"].map[hex8(imageOffsetToFixup)].value = printTarget(imagesArrays, target);
            }, ^(uint64_t imageOffsetToFixup, bool &stop) {
                // fixupObjCStableSwift
                imageNode.map["fixups"].map[hex8(imageOffsetToFixup)].value = "objc set stable Swift";
            }, ^(uint64_t imageOffsetToFixup, bool &stop) {
                // fixupObjCMethodList
                imageNode.map["fixups"].map[hex8(imageOffsetToFixup)].value = "objc set fixed up method list";
            });
            image->forEachTextReloc(^(uint32_t imageOffsetToRebase, bool &stop) {
                // rebase
                imageNode.map["fixups"].map[hex8(imageOffsetToRebase)].value = "text rebase";
            }, ^(uint32_t imageOffsetToBind, Image::ResolvedSymbolTarget target, bool &stop) {
                imageNode.map["fixups"].map[hex8(imageOffsetToBind)].value = "text " + printTarget(imagesArrays, target);
            });
        }
    }
    else {
        if ( printFixups ) {
            if ( dyldCache != nullptr ) {
                uint32_t imageIndex = image->imageNum() - (uint32_t)dyldCache->cachedDylibsImageArray()->startImageNum();
                dyldCache->forEachPatchableExport(imageIndex, ^(uint32_t cacheOffsetOfImpl, const char* name) {
                    __block Node implNode;
                    implNode.map["name"].value = name;
                    implNode.map["impl-cache-offset"].value = hex8(cacheOffsetOfImpl);
                    dyldCache->forEachPatchableUseOfExport(imageIndex, cacheOffsetOfImpl, ^(dyld_cache_patchable_location patchLocation) {
                        Node siteNode;
                        siteNode.map["cache-offset"].value = hex8(patchLocation.cacheOffset);
                        if ( patchLocation.addend != 0 )
                            siteNode.map["addend"].value = hex(patchLocation.addend);
                        if ( patchLocation.authenticated != 0 ) {
                            siteNode.map["key"].value = DyldSharedCache::keyName(patchLocation);
                            siteNode.map["address-diversity"].value = patchLocation.usesAddressDiversity ? "true" : "false";
                            siteNode.map["discriminator"].value = hex4(patchLocation.discriminator);
                        }
                        implNode.map["usage-sites"].array.push_back(siteNode);
                    });
                    imageNode.map["patches"].array.push_back(implNode);
                });
            }
        }
    }

    // add dependents
    image->forEachDependentImage(^(uint32_t depIndex, Image::LinkKind kind, ImageNum imageNum, bool& stop) {
        Node depMapNode;
        const Image* depImage = ImageArray::findImage(imagesArrays, imageNum);
        depMapNode.map["image-num"].value = hex4(imageNum);
        if ( depImage != nullptr )
            depMapNode.map["path"].value      = depImage->path();
        switch ( kind ) {
            case Image::LinkKind::regular:
                depMapNode.map["link"].value = "regular";
                break;
            case Image::LinkKind::reExport:
                depMapNode.map["link"].value = "re-export";
                break;
            case Image::LinkKind::upward:
                depMapNode.map["link"].value = "upward";
                break;
            case Image::LinkKind::weak:
                depMapNode.map["link"].value = "weak";
                break;
        }
        imageNode.map["dependents"].array.push_back(depMapNode);
    });
    
    // add initializers
    bool usesInitsSection = image->forEachInitializerSection(^(uint32_t sectionOffset, uint32_t sectionSize) {
        Node initSectNode;
        initSectNode.map["offset"].value = hex(sectionOffset);
        initSectNode.map["size"].value = hex(sectionSize);
        imageNode.map["initializers-section"].array.push_back(initSectNode);
    });
    if ( !usesInitsSection ) {
        image->forEachInitializer(nullptr, ^(const void* initializer) {
            Node initNode;
            initNode.value = hex((long)initializer);
            imageNode.map["initializer-offsets"].array.push_back(initNode);
        });
    }
	__block Node initBeforeNode;
    image->forEachImageToInitBefore(^(ImageNum imageToInit, bool& stop) {
        Node beforeNode;
        const Image* initImage = ImageArray::findImage(imagesArrays, imageToInit);
        assert(initImage != nullptr);
        beforeNode.value = initImage->path();
        imageNode.map["initializer-order"].array.push_back(beforeNode);
    });

   // add static terminators
    image->forEachTerminator(nullptr, ^(const void* terminator) {
        Node termNode;
        termNode.value = hex8((long)terminator);
        imageNode.map["terminator-offsets"].array.push_back(termNode);
    });

    ImageNum cacheImageNum;
	if ( image->isOverrideOfDyldCacheImage(cacheImageNum) ) {
        imageNode.map["override-of-dyld-cache-image"].value = ImageArray::findImage(imagesArrays, cacheImageNum)->path();
	}


#if 0
    // add things to init before this image
    __block Node initBeforeNode;
    image->forEachInitBefore(groupList, ^(Image beforeImage) {
        Node beforeNode;
        beforeNode.value = beforeimage->path();
        imageNode.map["initializer-order"].array.push_back(beforeNode);
    });

     // add override info if relevant
    group.forEachImageRefOverride(groupList, ^(Image standardDylib, Image overrideDylib, bool& stop) {
        if ( overrideDylib.binaryData() == image->binaryData() ) {
            imageNode.map["override-of-cached-dylib"].value = standardDylib.path();
        }
    });
    // add dtrace info
    image->forEachDOF(nullptr, ^(const void* section) {
        Node initNode;
        initNode.value = hex((long)section);
        imageNode.map["dof-offsets"].array.push_back(initNode);
    });
#endif

    return imageNode;
}


static Node buildImageArrayNode(const ImageArray* imageArray, const Array<const ImageArray*>& imagesArrays,
                                bool printFixups, bool printDependentsDetails,  bool printRaw,
                                const DyldSharedCache* dyldCache, const closure::ObjCSelectorOpt* selOpt,
                                const Array<closure::Image::ObjCSelectorImage>& selImages)
{
    __block Node images;
    imageArray->forEachImage(^(const Image* image, bool& stop) {
         images.array.push_back(buildImageNode(image, imagesArrays, printFixups, printDependentsDetails, printRaw, dyldCache, selOpt, selImages));
    });
     return images;
}


static Node buildClosureNode(const DlopenClosure* closure, const Array<const ImageArray*>& imagesArrays,
                             bool printFixups, bool printRaw, bool printDependentsDetails)
{
    __block Node root;
    root.map["images"] = buildImageArrayNode(closure->images(), imagesArrays,
                                             printFixups, printDependentsDetails, printRaw,
                                             nullptr, nullptr, {});

    closure->forEachPatchEntry(^(const Closure::PatchEntry& patchEntry) {
        Node patchNode;
        patchNode.map["func-dyld-cache-offset"].value = hex8(patchEntry.exportCacheOffset);
        patchNode.map["func-image-num"].value         = hex8(patchEntry.overriddenDylibInCache);
        patchNode.map["replacement"].value            = printTarget(imagesArrays, patchEntry.replacement);
        root.map["dyld-cache-fixups"].array.push_back(patchNode);
    });

    return root;
}

static Node buildClosureNode(const LaunchClosure* closure, const Array<const ImageArray*>& imagesArrays,
                             bool printFixups, bool printDependentsDetails,  bool printRaw,
                             const DyldSharedCache* dyldCache)
{
    __block Node root;

    Array<Image::ObjCSelectorImage> selectorImages;
    const closure::ObjCSelectorOpt* selectorHashTable = nullptr;
    bool hasPreoptimizedObjCSelectors = closure->selectorHashTable(selectorImages, selectorHashTable);

    root.map["images"] = buildImageArrayNode(closure->images(), imagesArrays, printFixups,
                                             printDependentsDetails, printRaw,
                                             dyldCache, selectorHashTable, selectorImages);

    if ( printRaw ) {
        __block Node attributes;
        closure->forEachAttribute(^(const TypedBytes *typedBytes, bool &stop) {
            Node anAttribute;
            anAttribute.map["type"].value = decimal((uint32_t)typedBytes->type);
            anAttribute.map["type-name"].value = nameForType((TypedBytes::Type)typedBytes->type);
            anAttribute.map["length"].value = decimal(typedBytes->payloadLength);
            attributes.array.push_back(anAttribute);
        });
        root.map["attributes"] = attributes;
        return root;
    }

    closure->forEachPatchEntry(^(const Closure::PatchEntry& patchEntry) {
        Node patchNode;
        patchNode.map["func-dyld-cache-offset"].value = hex8(patchEntry.exportCacheOffset);
        patchNode.map["func-image-num"].value         = hex8(patchEntry.overriddenDylibInCache);
        patchNode.map["replacement"].value            = printTarget(imagesArrays, patchEntry.replacement);
        root.map["dyld-cache-fixups"].array.push_back(patchNode);
    });

     Image::ResolvedSymbolTarget entry;
    if ( closure->mainEntry(entry) )
        root.map["main"].value = printTarget(imagesArrays, entry);
    else if ( closure->startEntry(entry) )
        root.map["start"].value = printTarget(imagesArrays, entry);

    Image::ResolvedSymbolTarget libdyldEntry;
    closure->libDyldEntry(libdyldEntry);
    root.map["libdyld-entry"].value = printTarget(imagesArrays, libdyldEntry);

    root.map["uses-@paths"].value = (closure->usedAtPaths() ? "true" : "false");
    root.map["uses-fallback-paths"].value = (closure->usedFallbackPaths() ? "true" : "false");

   // add missing files array if they exist
    closure->forEachMustBeMissingFile(^(const char* path, bool& stop) {
        Node fileNode;
        fileNode.value = path;
        root.map["must-be-missing-files"].array.push_back(fileNode);
    });

    // add skipped files array if they exist
    closure->forEachSkipIfExistsFile(^(const LaunchClosure::SkippedFile &file, bool &stop) {
        Node fileNode;
        fileNode.map["path"].value = file.path;
        fileNode.map["file-mod-time"].value = hex(file.mtime);
        fileNode.map["file-inode"].value = hex(file.inode);
        root.map["skipped-existing-files"].array.push_back(fileNode);
    });

    // add interposing info, if any
    closure->forEachInterposingTuple(^(const InterposingTuple& tuple, bool& stop) {
        Node tupleNode;
        tupleNode.map["stock"].value   = printTarget(imagesArrays, tuple.stockImplementation);
        tupleNode.map["replace"].value = printTarget(imagesArrays, tuple.newImplementation);
        root.map["interposing-tuples"].array.push_back(tupleNode);
    });

    closure->forEachPatchEntry(^(const Closure::PatchEntry& patchEntry) {
        Node patchNode;
        patchNode.map["func-dyld-cache-offset"].value = hex8(patchEntry.exportCacheOffset);
        patchNode.map["func-image-num"].value         = hex8(patchEntry.overriddenDylibInCache);
        patchNode.map["replacement"].value            = printTarget(imagesArrays, patchEntry.replacement);
        root.map["dyld-cache-fixups"].array.push_back(patchNode);
    });

    root.map["initial-image-count"].value = decimal(closure->initialLoadCount());

    // add env-vars if they exist
    closure->forEachEnvVar(^(const char* keyEqualValue, bool& stop) {
        const char* equ = strchr(keyEqualValue, '=');
        if ( equ != nullptr ) {
            char key[512];
            strncpy(key, keyEqualValue, equ-keyEqualValue);
            key[equ-keyEqualValue] = '\0';
            root.map["env-vars"].map[key].value = equ+1;
        }
    });

    if (hasPreoptimizedObjCSelectors) {
        __block Node selectorsNode;
        selectorHashTable->forEachString(selectorImages,
                                         ^(uint64_t selVMOffset, ImageNum imageNum) {
            // Convert to a target we can get a real name for
            dyld3::closure::Image::ResolvedSymbolTarget target;
            target.image.kind           = dyld3::closure::Image::ResolvedSymbolTarget::kindImage;
            target.image.imageNum       = imageNum;
            target.image.offset         = selVMOffset;

            Node targetNode;
            targetNode.value = printTarget(imagesArrays, target);
            selectorsNode.array.push_back(targetNode);
        });

        root.map["objc-selectors"] = selectorsNode;
    }
    
    Array<Image::ObjCClassImage> classImages;
    const ObjCClassOpt* classHashTable      = nullptr;
    const ObjCClassOpt* protocolHashTable   = nullptr;
    if (closure->classAndProtocolHashTables(classImages, classHashTable, protocolHashTable)) {
        if ( classHashTable != nullptr ) {
            __block Node classesNode;

            classHashTable->forEachClass(classImages,
                                         ^(uint64_t classNameVMOffset, ImageNum imageNum) {
                // Convert to a target we can get a real name for
                dyld3::closure::Image::ResolvedSymbolTarget target;
                target.image.kind           = dyld3::closure::Image::ResolvedSymbolTarget::kindImage;
                target.image.imageNum       = imageNum;
                target.image.offset         = classNameVMOffset;

                Node targetNode;
                targetNode.value = printTarget(imagesArrays, target);

                Node classNode;
                classNode.map["name"] = targetNode;
                classesNode.array.push_back(classNode);
            },
            ^(uint64_t classVMOffset, ImageNum imageNum) {
                dyld3::closure::Image::ResolvedSymbolTarget implTarget;
                implTarget.image.kind           = dyld3::closure::Image::ResolvedSymbolTarget::kindImage;
                implTarget.image.imageNum       = imageNum;
                implTarget.image.offset         = classVMOffset;

                Node implNode;
                implNode.value = printTarget(imagesArrays, implTarget);
                classesNode.array.back().map["implementations"].array.push_back(implNode);
            });

            root.map["objc-classes"] = classesNode;
        }

        if ( protocolHashTable != nullptr ) {
            __block Node protocolsNode;

            protocolHashTable->forEachClass(classImages,
                                            ^(uint64_t protocolNameVMOffset, ImageNum imageNum) {
                // Convert to a target we can get a real name for
                dyld3::closure::Image::ResolvedSymbolTarget target;
                target.image.kind           = dyld3::closure::Image::ResolvedSymbolTarget::kindImage;
                target.image.imageNum       = imageNum;
                target.image.offset         = protocolNameVMOffset;

                Node targetNode;
                targetNode.value = printTarget(imagesArrays, target);

                Node protocolNode;
                protocolNode.map["name"] = targetNode;
                protocolsNode.array.push_back(protocolNode);
            },
            ^(uint64_t protocolVMOffset, ImageNum imageNum) {
                dyld3::closure::Image::ResolvedSymbolTarget implTarget;
                implTarget.image.kind           = dyld3::closure::Image::ResolvedSymbolTarget::kindImage;
                implTarget.image.imageNum       = imageNum;
                implTarget.image.offset         = protocolVMOffset;

                Node implNode;
                implNode.value = printTarget(imagesArrays, implTarget);
                protocolsNode.array.back().map["implementations"].array.push_back(implNode);
            });

            root.map["objc-protocols"] = protocolsNode;
        }
    }

    const ObjCClassDuplicatesOpt* duplicateClassesHashTable   = nullptr;
    closure->duplicateClassesHashTable(duplicateClassesHashTable);
    if ( duplicateClassesHashTable != nullptr ) {
        __block Node duplicateClassesNode;
        duplicateClassesHashTable->forEachClass(^(Image::ObjCDuplicateClass duplicateClass) {
            objc_opt::objc_clsopt_t* clsOpt = dyldCache->objcOpt()->clsopt();
            const char* className = clsOpt->getClassNameForIndex(duplicateClass.sharedCacheClassOptIndex);
            const void* classImpl = clsOpt->getClassForIndex(duplicateClass.sharedCacheClassOptIndex, duplicateClass.sharedCacheClassDuplicateIndex);

            // Convert to a target we can get a real name for
            dyld3::closure::Image::ResolvedSymbolTarget target;
            target.sharedCache.kind     = dyld3::closure::Image::ResolvedSymbolTarget::kindSharedCache;
            target.sharedCache.offset   = (uint64_t)classImpl - (uint64_t)dyldCache;

            Node targetNode;
            targetNode.value = printTarget(imagesArrays, target);

            Node duplicateClassNode;
            duplicateClassNode.map["name"].value = className;
            duplicateClassNode.map["implementation"] = targetNode;
            duplicateClassNode.array.push_back(targetNode);

            duplicateClassesNode.array.push_back(duplicateClassNode);
        });

        root.map["objc-duplicate-classes"] = duplicateClassesNode;
    }

    // add warnings for objc if they exist
    closure->forEachWarning(Closure::Warning::duplicateObjCClass, ^(const char *warning, bool &stop) {
        Node warningNode;
        warningNode.value = warning;
        root.map["objc-duplicate-class-warnings"].array.push_back(warningNode);
    });

#if 0


    // add uuid of dyld cache this closure requires
    closure.dyldCacheUUID();
    uuid_string_t cacheUuidStr;
    uuid_unparse(*closure.dyldCacheUUID(), cacheUuidStr);
    root.map["dyld-cache-uuid"].value = cacheUuidStr;

    // add top level images
    Node& rootImages = root.map["root-images"];
    uint32_t initImageCount = closure.mainExecutableImageIndex();
    rootImages.array.resize(initImageCount+1);
    for (uint32_t i=0; i <= initImageCount; ++i) {
        const Image image = closure.group().image(i);
        uuid_string_t uuidStr;
        uuid_unparse(image->uuid(), uuidStr);
        rootImages.array[i].value = uuidStr;
    }
    root.map["initial-image-count"].value = decimal(closure.initialImageCount());

    // add images
    root.map["group-num"].value = decimal(closure.group().groupNum());


    __block Node cacheOverrides;
    closure.group().forEachDyldCacheSymbolOverride(^(uint32_t patchTableIndex, uint32_t imageIndexInClosure, uint32_t imageOffset, bool& stop) {
        Node patch;
        patch.map["patch-index"].value = decimal(patchTableIndex);
        patch.map["replacement"].value = "{closure[" + decimal(imageIndexInClosure) + "]+" + hex(imageOffset) + "}";
        cacheOverrides.array.push_back(patch);
    });
    if ( !cacheOverrides.array.empty() )
        root.map["dyld-cache-overrides"].array = cacheOverrides.array;
#endif
    return root;
}

void printImageAsJSON(const Image* image, const Array<const ImageArray*>& imagesArrays,
                      bool printFixups,  bool printRaw, const DyldSharedCache* dyldCache, std::ostream& out)
{
    Node root = buildImageNode(image, imagesArrays, printFixups, false, printRaw, dyldCache, nullptr, {});
    printJSON(root, 0, out);
}

void printDyldCacheImagesAsJSON(const DyldSharedCache* dyldCache, bool printFixups, bool printRaw, std::ostream& out)
{
    const dyld3::closure::ImageArray* dylibs = dyldCache->cachedDylibsImageArray();
    STACK_ALLOC_ARRAY(const ImageArray*, imagesArrays, 2);
    imagesArrays.push_back(dylibs);

    Node root = buildImageArrayNode(dylibs, imagesArrays, printFixups, false, printRaw, dyldCache, nullptr, {});
    printJSON(root, 0, out);
}

void printClosureAsJSON(const LaunchClosure* cls, const Array<const ImageArray*>& imagesArrays,
                        bool printFixups, bool printRaw, const DyldSharedCache* dyldCache, std::ostream& out)
{
    Node root = buildClosureNode(cls, imagesArrays, printFixups, false, printRaw, dyldCache);
    printJSON(root, 0, out);
}

void printClosureAsJSON(const DlopenClosure* cls, const Array<const ImageArray*>& imagesArrays,
                        bool printFixups, bool printRaw, const DyldSharedCache* dyldCache, std::ostream& out)
{
    Node root = buildClosureNode(cls, imagesArrays, printFixups, printRaw, false);
    printJSON(root, 0, out);
}


} // namespace closure
} // namespace dyld3
