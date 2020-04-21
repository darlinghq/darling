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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <ext/__hash>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <mach-o/dyld_priv.h>

#include "ClosureWriter.h"
#include "ClosureBuilder.h"
#include "MachOAnalyzer.h"
#include "libdyldEntryVector.h"
#include "Tracing.h"

#define CLOSURE_SELOPT_WRITE
#include "objc-shared-cache.h"

namespace dyld3 {
namespace closure {


const DlopenClosure* ClosureBuilder::sRetryDlopenClosure = (const DlopenClosure*)(-1);

ClosureBuilder::ClosureBuilder(uint32_t startImageNum, const FileSystem& fileSystem, const DyldSharedCache* dyldCache, bool dyldCacheIsLive,
                               const GradedArchs& archs, const PathOverrides& pathOverrides, AtPath atPathHandling, bool allowRelativePaths,
                               LaunchErrorInfo* errorInfo, Platform platform, const CacheDylibsBindingHandlers* handlers)
    : _fileSystem(fileSystem), _dyldCache(dyldCache), _pathOverrides(pathOverrides), _archs(archs), _platform(platform), _startImageNum(startImageNum),
      _handlers(handlers), _atPathHandling(atPathHandling), _launchErrorInfo(errorInfo), _dyldCacheIsLive(dyldCacheIsLive), _allowRelativePaths(allowRelativePaths)
{
    if ( dyldCache != nullptr ) {
        _dyldImageArray = dyldCache->cachedDylibsImageArray();
        if ( (dyldCache->header.otherImageArrayAddr != 0) && (dyldCache->header.progClosuresSize == 0) )
            _makingClosuresInCache = true;
    }
}


ClosureBuilder::~ClosureBuilder() {
    if ( _tempPaths != nullptr )
        PathPool::deallocate(_tempPaths);
    if ( _mustBeMissingPaths != nullptr )
        PathPool::deallocate(_mustBeMissingPaths);
    if ( _objcDuplicateClassWarnings != nullptr )
        PathPool::deallocate(_objcDuplicateClassWarnings);
}

bool ClosureBuilder::findImage(const char* loadPath, const LoadedImageChain& forImageChain, BuilderLoadedImage*& foundImage, LinkageType linkageType,
                                uint32_t compatVersion, bool canUseSharedCacheClosure)
{
    // There shouldn't be an error here as the callers should stop trying to find more images if they get an error for an image
    _diag.assertNoError();

    __block bool result = false;

    // record if this is a non-overridable path
    bool pathIsInDyldCacheWhichCannotBeOverridden = false;
    bool dylibsExpectedOnDisk = true;
    if ( _dyldCache != nullptr ) {
        pathIsInDyldCacheWhichCannotBeOverridden = _dyldCache->hasNonOverridablePath(loadPath);
        dylibsExpectedOnDisk = _dyldCache->header.dylibsExpectedOnDisk;
    }

    _pathOverrides.forEachPathVariant(loadPath, pathIsInDyldCacheWhichCannotBeOverridden, ^(const char* possibleVariantPath, bool isFallbackPath, bool& stopPathVariant) {

        // This check is within forEachPathVariant() to let DYLD_LIBRARY_PATH override LC_RPATH
        bool isRPath = (strncmp(possibleVariantPath, "@rpath/", 7) == 0);

        // passing a leaf name to dlopen() allows rpath searching for it
        // FIXME: Does this apply to DYLD_INSERT_LIBRARIES too?
        bool implictRPath = (linkageType == LinkageType::kDynamic) && (loadPath[0] != '/') && (loadPath == possibleVariantPath) && (_atPathHandling != AtPath::none);

        // expand @ paths
        forEachResolvedPathVar(possibleVariantPath, forImageChain, implictRPath, linkageType,
                               ^(const char* possiblePath, bool& stop) {
            if ( possibleVariantPath != possiblePath )
                _atPathUsed = true;

            // look at already loaded images
            const char* leafName = strrchr(possiblePath, '/');
            for (BuilderLoadedImage& li: _loadedImages) {
                if ( strcmp(li.path(), possiblePath) == 0 ) {
                    foundImage = &li;
                    result = true;
                    stop = true;
                    return;
                }
                else if ( isRPath ) {
                    // Special case @rpath/ because name in li.fileInfo.path is full path.
                    // Getting installName is expensive, so first see if an already loaded image
                    // has same leaf name and if so see if its installName matches request @rpath
                    if (const char* aLeaf = strrchr(li.path(), '/')) {
                        if ( strcmp(aLeaf, leafName) == 0 ) {
                            if ( li.loadAddress()->isDylib() && (strcmp(loadPath, li.loadAddress()->installName()) == 0) ) {
                                foundImage = &li;
                                result = true;
                                stop = true;
                                return;
                            }
                        }
                    }
                }
            }

            // look to see if image already loaded via a different symlink
            bool                  fileFound             = false;
            uint64_t              fileFoundINode        = 0;
            uint64_t              fileFoundMTime        = 0;
            bool                  inodesMatchRuntime    = false;
            // Note, we only do this check if we even expect to find this on-disk
            // We can also use the pathIsInDyldCacheWhichCannotBeOverridden result if we are still trying the same path
            // it was computed from
            if ( dylibsExpectedOnDisk || !pathIsInDyldCacheWhichCannotBeOverridden || (loadPath != possiblePath) ) {
                if ( _fileSystem.fileExists(possiblePath, &fileFoundINode, &fileFoundMTime, nullptr, &inodesMatchRuntime) ) {
                    fileFound = true;
                    for (BuilderLoadedImage& li: _loadedImages) {
                        if ( (li.loadedFileInfo.inode == fileFoundINode) && (li.loadedFileInfo.mtime == fileFoundMTime) )  {
                            foundImage = &li;
                            result = true;
                            stop = true;
                            return;
                        }
                    }
                }
            }

            bool                  unmapWhenDone     = false;
            bool                  contentRebased    = false;
            bool                  hasInits          = false;
            bool                  markNeverUnload   = false;
            bool                  mustBuildClosure = _dyldCacheInvalidFormatVersion;
            ImageNum              overrideImageNum  = 0;
            ImageNum              foundImageNum     = 0;
            const MachOAnalyzer*  mh                = nullptr;
            const char*           filePath          = nullptr;
            LoadedFileInfo        loadedFileInfo;

            // look in dyld cache
            filePath = possiblePath;
            char realPath[MAXPATHLEN];
            if ( _dyldImageArray != nullptr ) {
                uint32_t dyldCacheImageIndex;
                bool foundInCache = _dyldCache->hasImagePath(possiblePath, dyldCacheImageIndex);
                if ( !foundInCache && fileFound ) {
                    // see if this is an OS dylib/bundle with a pre-built dlopen closure
                    // We can only use the pre-built closure if we are dynamic linkage (a dlopen) and
                    // there are no roots
                    if ( canUseSharedCacheClosure && (linkageType == LinkageType::kDynamic) ) {
                        if (const dyld3::closure::Image* otherImage = _dyldCache->findDlopenOtherImage(possiblePath) ) {
                            uint64_t expectedInode;
                            uint64_t expectedModTime;
                            if ( !otherImage->isInvalid()  ) {
                                bool hasInodeInfo = otherImage->hasFileModTimeAndInode(expectedInode, expectedModTime);
                                // use pre-built Image if it does not have mtime/inode or it does and it has matches current file info
                                if ( !hasInodeInfo || ((expectedInode == fileFoundINode) && (expectedModTime == fileFoundMTime)) ) {
                                    loadedFileInfo = MachOAnalyzer::load(_diag, _fileSystem, possiblePath, _archs, _platform, realPath);
                                    if ( _diag.noError() ) {
                                        mh = (const MachOAnalyzer*)loadedFileInfo.fileContent;
                                        foundImageNum = otherImage->imageNum();
                                        unmapWhenDone = true;
                                        contentRebased = false;
                                        hasInits = otherImage->hasInitializers() || otherImage->mayHavePlusLoads();
                                        // Use the realpath in the case where we loaded a symlink
                                        // The closure must have recordered an alias path
                                        if (realPath[0] != '\0')
                                            filePath = realPath;
                                    }
                                }
                            }
                        }
                    }
                    // if not found in cache, may be a symlink to something in cache
                    if ( mh == nullptr ) {
                        if ( _fileSystem.getRealPath(possiblePath, realPath) ) {
                            foundInCache = _dyldCache->hasImagePath(realPath, dyldCacheImageIndex);
                            if ( foundInCache ) {
                                filePath = realPath;
#if BUILDING_LIBDYLD
                                // handle case where OS dylib was updated after this process launched
                                if ( foundInCache ) {
                                    for (BuilderLoadedImage& li: _loadedImages) {
                                        if ( strcmp(li.path(), realPath) == 0 ) {
                                            foundImage = &li;
                                            result = true;
                                            stop = true;
                                            return;
                                        }
                                    }
                                }
#endif
                            }
                        }
                    }
                }

                // if using a cached dylib, look to see if there is an override
                if ( foundInCache ) {
                    ImageNum dyldCacheImageNum = dyldCacheImageIndex + 1;
                    bool useCache = true;
                    markNeverUnload = true; // dylibs in cache, or dylibs that override cache should not be unloaded at runtime
                    const Image* image = _dyldImageArray->imageForNum(dyldCacheImageNum);
                    if ( image->overridableDylib() ) {
                        if ( fileFound ) {
                            uint64_t expectedInode;
                            uint64_t expectedModTime;
                            if ( image->hasFileModTimeAndInode(expectedInode, expectedModTime) ) {
                                // macOS where dylibs remain on disk.  only use cache if mtime and inode have not changed
                                useCache = ( (fileFoundINode == expectedInode) && (fileFoundMTime == expectedModTime) );
                            }
                            else if ( _makingClosuresInCache ) {
                                // during iOS cache build, don't look at files on disk, use ones in cache
                                useCache = true;
                            }
                            else {
                                // iOS internal build. Any disk on cache overrides cache
                                useCache = false;
                            }
                        }
                        if ( !useCache ) {
                            overrideImageNum = dyldCacheImageNum;
                            _foundDyldCacheRoots = true;
                        }
                    }
                    if ( useCache ) {
                        foundImageNum = dyldCacheImageNum;
                        mh = (MachOAnalyzer*)_dyldCache->getIndexedImageEntry(foundImageNum-1, loadedFileInfo.mtime, loadedFileInfo.inode);
                        unmapWhenDone = false;
                        // if we are building ImageArray in dyld cache, content is not rebased
                        contentRebased = !_makingDyldCacheImages && _dyldCacheIsLive;
                        hasInits = image->hasInitializers() || image->mayHavePlusLoads();
                        // If the cache format is different from dyld/libdyld then we can't use this closure.
                        if ( (_dyldCache->header.formatVersion != dyld3::closure::kFormatVersion) || !canUseSharedCacheClosure ) {
                            mustBuildClosure = true;
                            _foundDyldCacheRoots = true;
                        }
                    }
                }
            }

            // If we are building the cache, and don't find an image, then it might be weak so just return
            if (_makingDyldCacheImages) {
                addMustBeMissingPath(possiblePath);
                return;
            }

             // if not found yet, mmap file
            if ( mh == nullptr ) {
                loadedFileInfo = MachOAnalyzer::load(_diag, _fileSystem, filePath, _archs, _platform, realPath);
                mh = (const MachOAnalyzer*)loadedFileInfo.fileContent;
                if ( mh == nullptr ) {
                    // Don't add must be missing paths for dlopen as we don't cache dlopen closures
                    if (_isLaunchClosure) {
                        // If we found the file then we want to skip it as its not a valid macho for this platform/arch
                        // We can't record skipped file mtime/inode for caches built on a different machine that it runs on.
                        // In that case, we expect the file to be mastered out, as otherwise we couldn't detect if its
                        // changed or not on the device side
                        if (fileFound && inodesMatchRuntime) {
                            addSkippedFile(possiblePath, fileFoundINode, fileFoundMTime);
                        } else {
                            addMustBeMissingPath(possiblePath);
                        }
                    }
                    return;
                }
                if ( linkageType != LinkageType::kDynamic ) {
                    // LC_LOAD_DYLIB can only link with dylibs, and DYLD_INSERT_LIBRARIES can only be dylibs
                    if ( !mh->isDylib() ) {
                        _diag.error("found '%s' which is not a dylib.  Needed by '%s'", filePath, forImageChain.image.path());
                        return;
                    }
                    // verify this is compatable dylib version
                    const char* installName;
                    uint32_t    foundCompatVers;
                    uint32_t    foundCurrentVers;
                    mh->getDylibInstallName(&installName, &foundCompatVers, &foundCurrentVers);
                    if ( (foundCompatVers < compatVersion) && mh->enforceCompatVersion() ) {
                        char foundStr[32];
                        char requiredStr[32];
                        MachOFile::packedVersionToString(foundCompatVers, foundStr);
                        MachOFile::packedVersionToString(compatVersion, requiredStr);
                        _diag.error("found '%s' which has compat version (%s) which is less than required (%s).  Needed by '%s'",
                                    filePath, foundStr, requiredStr, forImageChain.image.path());
                        return;
                    }
                }
                else if ( mh->isMainExecutable() ) {
                    // when dlopen()ing a main executable, it must be dynamic Position Independent Executable
                    if ( !mh->isPIE() || !mh->isDynamicExecutable() ) {
                        _diag.error("not PIE");
                        return;
                    }
                }
                // Use the realpath in the case where we loaded a symlink
                // The closure must have recordered an alias path
                if (realPath[0] != '\0')
                    filePath = realPath;
                foundImageNum = _startImageNum + _nextIndex++;
                _foundNonCachedImage = true;
                mustBuildClosure = true;
                unmapWhenDone = true;
            } else {
                loadedFileInfo.fileContent = mh;
            }

            // if path is not original path, or its an inserted path (as forEachInColonList uses a stack temporary)
            if ( (filePath != loadPath) || (linkageType == LinkageType::kInserted) ) {
                // possiblePath may be a temporary (stack) string, since we found file at that path, make it permanent
                filePath = strdup_temp(filePath);
                // check if this overrides what would have been found in cache
                // This is the case where we didn't find the image with the path in the shared cache, perhaps as it used library paths
                // but the path we requested had pointed in to the cache
                // FIXME: What if load path is via an @rpath and we will override the cache?
                if ( overrideImageNum == 0 ) {
                    if ( _dyldImageArray != nullptr )  {
                        uint32_t dyldCacheImageIndex;
                        if ( _dyldCache->hasImagePath(loadPath, dyldCacheImageIndex) ) {
                            ImageNum possibleOverrideNum = dyldCacheImageIndex+1;
                            if ( possibleOverrideNum != foundImageNum )
                                overrideImageNum = possibleOverrideNum;
                        }
                    }
                }
            }

            if ( !markNeverUnload ) {
                switch (linkageType) {
                    case LinkageType::kStatic:
                        // Static linkages can only be unloaded if the image loading us can be unloaded
                        markNeverUnload = forImageChain.image.markNeverUnload;
                        break;
                    case LinkageType::kDynamic:
                        markNeverUnload = false;
                        break;
                    case LinkageType::kInserted:
                        // Inserted libraries must never be unloaded
                        markNeverUnload = true;
                        break;
                };
            }

            if ( !markNeverUnload ) {
                // If the parent didn't force us to be never unload, other conditions still may
                if ( mh->hasThreadLocalVariables() ) {
                    markNeverUnload = true;
                } else if ( mh->hasObjC() && mh->isDylib() ) {
                    markNeverUnload = true;
                } else {
                    // record if image has DOF sections
                    __block bool hasDOFs = false;
                    mh->forEachDOFSection(_diag, ^(uint32_t offset) {
                        hasDOFs = true;
                    });
                    if ( hasDOFs )
                        markNeverUnload = true;
                }
            }

            // Set the path again just in case it was strdup'ed.
            loadedFileInfo.path = filePath;

            // add new entry
            BuilderLoadedImage entry;
            entry.loadedFileInfo        = loadedFileInfo;
            entry.imageNum              = foundImageNum;
            entry.unmapWhenDone         = unmapWhenDone;
            entry.contentRebased        = contentRebased;
            entry.hasInits              = hasInits;
            entry.markNeverUnload       = markNeverUnload;
            entry.rtldLocal             = false;
            entry.isBadImage            = false;
            entry.mustBuildClosure      = mustBuildClosure;
            entry.hasMissingWeakImports = false;
            entry.overrideImageNum      = overrideImageNum;
            _loadedImages.push_back(entry);
            foundImage = &_loadedImages.back();
            if ( isFallbackPath )
                _fallbackPathUsed = true;
            stop = true;
            result = true;
        });
        if (result)
            stopPathVariant = true;
    }, _platform);

    // If we found a file, but also had an error, then we must have logged a diagnostic for a file we couldn't use.
    // Clear that for now.
    // FIXME: Surface this to the user in case they wanted to see the error
    if (result && _diag.hasError())
        _diag.clearError();

    return result;
}

bool ClosureBuilder::expandAtLoaderPath(const char* loadPath, bool fromLCRPATH, const BuilderLoadedImage& loadedImage, char fixedPath[])
{
    switch ( _atPathHandling ) {
        case AtPath::none:
            return false;
        case AtPath::onlyInRPaths:
            if ( !fromLCRPATH ) {
                // <rdar://42360708> allow @loader_path in LC_LOAD_DYLIB during dlopen()
                if ( _isLaunchClosure )
                    return false;
            }
            break;
        case AtPath::all:
            break;
    }
    if ( strncmp(loadPath, "@loader_path/", 13) != 0 )
        return false;

    strlcpy(fixedPath, loadedImage.path(), PATH_MAX);
    char* lastSlash = strrchr(fixedPath, '/');
    if ( lastSlash != nullptr ) {
        strcpy(lastSlash+1, &loadPath[13]);
        return true;
    }
    return false;
}

bool ClosureBuilder::expandAtExecutablePath(const char* loadPath, bool fromLCRPATH, char fixedPath[])
{
    switch ( _atPathHandling ) {
        case AtPath::none:
            return false;
        case AtPath::onlyInRPaths:
            if ( !fromLCRPATH )
                return false;
            break;
        case AtPath::all:
            break;
    }
    if ( strncmp(loadPath, "@executable_path/", 17) != 0 )
        return false;

    if ( _atPathHandling != AtPath::all )
        return false;

    strlcpy(fixedPath, _mainProgLoadPath, PATH_MAX);
    char* lastSlash = strrchr(fixedPath, '/');
    if ( lastSlash != nullptr ) {
        strcpy(lastSlash+1, &loadPath[17]);
        return true;
    }
    return false;
}

void ClosureBuilder::forEachResolvedPathVar(const char* loadPath, const LoadedImageChain& forImageChain,
                                            bool implictRPath, LinkageType linkageType,
                                            void (^handler)(const char* possiblePath, bool& stop))
{
    // don't expand @loader_path or @executable_path if disallowed
    if ( (_atPathHandling == AtPath::none) && (loadPath[0] == '@') && (loadPath[1] != 'r') ) {
        bool stop = false;
        handler(loadPath, stop);
        return;
    }

    // quick out if not @ path or not implicit rpath
    if ( !implictRPath && (loadPath[0] != '@') ) {
        bool stop = false;
        handler(loadPath, stop);
        return;
    }

    // expand @loader_path
    // Note this isn't supported for DYLD_INSERT_LIBRARIES
    BLOCK_ACCCESSIBLE_ARRAY(char, tempPath, PATH_MAX);  // read as:  char tempPath[PATH_MAX];
    if ( (linkageType != LinkageType::kInserted) && expandAtLoaderPath(loadPath, false, forImageChain.image, tempPath) ) {
        bool stop = false;
        handler(tempPath, stop);
        return;
    }

    // expand @executable_path
    // Note this is supported for DYLD_INSERT_LIBRARIES
    if ( expandAtExecutablePath(loadPath, false, tempPath) ) {
        bool stop = false;
        handler(tempPath, stop);
        return;
    }

    // expand @rpath
    // Note this isn't supported for DYLD_INSERT_LIBRARIES
    const char* rpathTail = nullptr;
    char implicitRpathBuffer[PATH_MAX];
    if ( linkageType != LinkageType::kInserted ) {
        if ( strncmp(loadPath, "@rpath/", 7) == 0 ) {
            // note: rpathTail starts with '/'
            rpathTail = &loadPath[6];
        }
        else if ( implictRPath ) {
            // make rpathTail starts with '/'
            strlcpy(implicitRpathBuffer, "/", PATH_MAX);
            strlcat(implicitRpathBuffer, loadPath, PATH_MAX);
            rpathTail = implicitRpathBuffer;
        }
    }
    if ( rpathTail != nullptr ) {
        // rpath is expansion is technically a stack of rpath dirs built starting with main executable and pushing
        // LC_RPATHS from each dylib as they are recursively loaded.  Our imageChain represents that stack.
        __block bool done = false;
        for (const LoadedImageChain* link = &forImageChain; (link != nullptr) && !done; link = link->previous) {
            link->image.loadAddress()->forEachRPath(^(const char* rPath, bool& stop) {
                // fprintf(stderr, "LC_RPATH %s from %s\n", rPath, link->image.loadedFileInfo.path);
                if ( expandAtLoaderPath(rPath, true, link->image, tempPath) || expandAtExecutablePath(rPath, true, tempPath) ) {
                    // @loader_path allowed and expended
                    strlcat(tempPath, rpathTail, PATH_MAX);
                    handler(tempPath, stop);
                }
                else if ( rPath[0] == '/' ) {
                    // LC_RPATH is an absolute path, not blocked by AtPath::none
                    strlcpy(tempPath, rPath, PATH_MAX);
                    strlcat(tempPath, rpathTail, PATH_MAX);
                    handler(tempPath, stop);
                }
                if (stop)
                    done = true;
#if 0
                if ( _fileSystem.fileExists(tempPath) ) {
                    stop = true;
                    result = strdup_temp(tempPath);
                }
                else {
                    // Don't add must be missing paths for dlopen as we don't cache dlopen closures
                    if (_isLaunchClosure) {
                        addMustBeMissingPath(tempPath);
                    }
                }
#endif
            });
        }
        if (done)
            return;
    }

    bool stop = false;
    handler(loadPath, stop);
}

const char* ClosureBuilder::strdup_temp(const char* path)
{
    if ( _tempPaths == nullptr )
        _tempPaths = PathPool::allocate();
    return _tempPaths->add(path);
}

void ClosureBuilder::addMustBeMissingPath(const char* path)
{
    //fprintf(stderr, "must be missing: %s\n", path);
    if ( _mustBeMissingPaths == nullptr )
        _mustBeMissingPaths = PathPool::allocate();
    _mustBeMissingPaths->add(path);
}

void ClosureBuilder::addSkippedFile(const char* path, uint64_t inode, uint64_t mtime)
{
    _skippedFiles.push_back({ strdup_temp(path), inode, mtime });
}

ClosureBuilder::BuilderLoadedImage& ClosureBuilder::findLoadedImage(ImageNum imageNum)
{
    for (BuilderLoadedImage& li : _loadedImages) {
        if ( li.imageNum == imageNum ) {
            return li;
        }
    }
    for (BuilderLoadedImage& li : _loadedImages) {
        if ( li.overrideImageNum == imageNum ) {
            return li;
        }
    }
    assert(0 && "LoadedImage not found");
}

ClosureBuilder::BuilderLoadedImage& ClosureBuilder::findLoadedImage(const MachOAnalyzer* mh)
{
    for (BuilderLoadedImage& li : _loadedImages) {
        if ( li.loadAddress() == mh ) {
             return li;
        }
    }
    assert(0 && "LoadedImage not found");
}

const MachOAnalyzer* ClosureBuilder::machOForImageNum(ImageNum imageNum)
{
    return findLoadedImage(imageNum).loadAddress();
}

const MachOAnalyzer* ClosureBuilder::findDependent(const MachOLoaded* mh, uint32_t depIndex)
{
    for (const BuilderLoadedImage& li : _loadedImages) {
        if ( li.loadAddress() == mh ) {
            if (li.isBadImage) {
                // Bad image duting building group 1 closures, so the dependents array
                // is potentially incomplete.
                return nullptr;
            }
            ImageNum childNum = li.dependents[depIndex].imageNum();
            // This is typically something like a missing weak-dylib we are re-exporting a weak-import symbol from
            if (childNum == kMissingWeakLinkedImage)
                return nullptr;
            return machOForImageNum(childNum);
        }
    }
    return nullptr;
}

ImageNum ClosureBuilder::imageNumForMachO(const MachOAnalyzer* mh)
{
    for (const BuilderLoadedImage& li : _loadedImages) {
        if ( li.loadAddress() == mh ) {
             return li.imageNum;
        }
    }
    assert(0 && "unknown mach-o");
    return 0;
}

void ClosureBuilder::recursiveLoadDependents(LoadedImageChain& forImageChain, bool canUseSharedCacheClosure)
{
    // if dependents is set, then we have already loaded this
    if ( forImageChain.image.dependents.begin() != nullptr )
        return;

    uintptr_t startDepIndex = _dependencies.count();
    // add dependents
    __block uint32_t depIndex = 0;
    forImageChain.image.loadAddress()->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool &stop) {
        Image::LinkKind kind = Image::LinkKind::regular;
        if ( isWeak )
            kind = Image::LinkKind::weak;
        else if ( isReExport )
            kind = Image::LinkKind::reExport;
        else if ( isUpward )
            kind = Image::LinkKind::upward;
        BuilderLoadedImage* foundImage;
        if ( findImage(loadPath, forImageChain, foundImage, LinkageType::kStatic, compatVersion, canUseSharedCacheClosure) ) {
            ImageNum foundImageNum = foundImage->imageNum;
            if ( _diag.noError() )
                _dependencies.push_back(Image::LinkedImage(kind, foundImageNum));
        }
        else if ( isWeak ) {
            _dependencies.push_back(Image::LinkedImage(Image::LinkKind::weak, kMissingWeakLinkedImage));
            // <rdar://problem/54387345> don't let an error loading weak dylib cause everything to fail
            // _diag is checked after each dependent load, so if there is an error it was with loading the current dylib.
            // Since it is a weak load, it is ok to ignore and and go on.
            _diag.clearError();
        }
        else {
            BLOCK_ACCCESSIBLE_ARRAY(char, extra, 4096);
            extra[0] = '\0';
            const char* targetLeaf = strrchr(loadPath, '/');
            if ( targetLeaf == nullptr )
                targetLeaf = loadPath;
            if ( _mustBeMissingPaths != nullptr ) {
                strcpy(extra, ", tried but didn't find: ");
                _mustBeMissingPaths->forEachPath(^(const char* aPath) {
                    const char* aLeaf = strrchr(aPath, '/');
                    if ( aLeaf == nullptr )
                        aLeaf = aPath;
                  if ( strcmp(targetLeaf, aLeaf) == 0 ) {
                        strlcat(extra, "'", 4096);
                        strlcat(extra, aPath, 4096);
                        strlcat(extra, "' ", 4096);
                    }
                });
            }
            if ( !_skippedFiles.empty() ) {
                strcpy(extra, ", tried but invalid: ");
                for (const SkippedFile& skippedFile : _skippedFiles) {
                    const char* aPath = skippedFile.path;
                    const char* aLeaf = strrchr(aPath, '/');
                    if ( aLeaf == nullptr )
                        aLeaf = aPath;
                    if ( strcmp(targetLeaf, aLeaf) == 0 ) {
                        strlcat(extra, "'", 4096);
                        strlcat(extra, aPath, 4096);
                        strlcat(extra, "' ", 4096);
                    }
                }
            }
            if ( _diag.hasError() ) {
        #if BUILDING_CACHE_BUILDER
                std::string errorMessageBuffer = _diag.errorMessage();
                const char* msg = errorMessageBuffer.c_str();
        #else
                const char* msg = _diag.errorMessage();
        #endif
                char msgCopy[strlen(msg)+4];
                strcpy(msgCopy, msg);
                _diag.error("dependent dylib '%s' not found for '%s'. %s", loadPath, forImageChain.image.path(), msgCopy);
            }
            else {
                _diag.error("dependent dylib '%s' not found for '%s'%s", loadPath, forImageChain.image.path(), extra);
            }
            if ( _launchErrorInfo != nullptr ) {
                _launchErrorInfo->kind              = DYLD_EXIT_REASON_DYLIB_MISSING;
                _launchErrorInfo->clientOfDylibPath = strdup_temp(forImageChain.image.path());
                _launchErrorInfo->targetDylibPath   = strdup_temp(loadPath);
                _launchErrorInfo->symbol            = nullptr;
           }
        }
        ++depIndex;
        if ( _diag.hasError() )
            stop = true;
    });
    if ( _diag.hasError() )
        return;
    forImageChain.image.dependents = _dependencies.subArray(startDepIndex, depIndex);

    // breadth first recurse
    for (Image::LinkedImage dep : forImageChain.image.dependents) {
        // don't recurse upwards
        if ( dep.kind() == Image::LinkKind::upward )
            continue;
        // don't recurse down missing weak links
        if ( (dep.kind() == Image::LinkKind::weak) && (dep.imageNum() == kMissingWeakLinkedImage) )
            continue;
        BuilderLoadedImage& depLoadedImage = findLoadedImage(dep.imageNum());
        LoadedImageChain chain = { &forImageChain, depLoadedImage };
        recursiveLoadDependents(chain, canUseSharedCacheClosure);
        if ( _diag.hasError() )
            break;
    }
}

void ClosureBuilder::loadDanglingUpwardLinks(bool canUseSharedCacheClosure)
{
    bool danglingFixed;
    do {
        danglingFixed = false;
        for (BuilderLoadedImage& li : _loadedImages) {
            if ( li.dependents.begin() == nullptr ) {
                // this image has not have dependents set (probably a dangling upward link or referenced by upward link)
                LoadedImageChain chain = { nullptr, li };
                recursiveLoadDependents(chain, canUseSharedCacheClosure);
                danglingFixed = true;
                break;
            }
        }
    } while (danglingFixed && _diag.noError());
}

bool ClosureBuilder::overridableDylib(const BuilderLoadedImage& forImage)
{
    // only set on dylibs in the dyld shared cache
    if ( !_makingDyldCacheImages )
        return false;

    // on macOS dylibs always override cache
    if ( _platform == Platform::macOS )
        return true;

    // on embedded platforms with Internal cache, allow overrides
    if ( !_makingCustomerCache )
        return true;

    // embedded platform customer caches, no overrides
    return false;  // FIXME, allow libdispatch.dylib to be overridden
}

void ClosureBuilder::buildImage(ImageWriter& writer, BuilderLoadedImage& forImage)
{
    const MachOAnalyzer* macho = forImage.loadAddress();
	// set ImageNum
    writer.setImageNum(forImage.imageNum);

    // set flags
    writer.setHasWeakDefs(macho->hasWeakDefs());
    writer.setIsBundle(macho->isBundle());
    writer.setIsDylib(macho->isDylib());
    writer.setIs64(macho->is64());
    writer.setIsExecutable(macho->isMainExecutable());
    writer.setUses16KPages(macho->uses16KPages());
    writer.setOverridableDylib(overridableDylib(forImage));
    writer.setInDyldCache(macho->inDyldCache());
    if ( macho->hasObjC() ) {
        writer.setHasObjC(true);
        bool hasPlusLoads = macho->hasPlusLoadMethod(_diag);
        writer.setHasPlusLoads(hasPlusLoads);
        if ( hasPlusLoads )
            forImage.hasInits = true;
    }
    else {
        writer.setHasObjC(false);
        writer.setHasPlusLoads(false);
    }

    if ( forImage.markNeverUnload ) {
        writer.setNeverUnload(true);
    }

#if BUILDING_DYLD || BUILDING_LIBDYLD
    if ( _foundDyldCacheRoots ) {
        // If we had roots, then some images are potentially on-disk while others are
        // being rebuilt for a new initializer order, but do not exist on disk
        if ( macho->inDyldCache() && !_dyldCache->header.dylibsExpectedOnDisk ) {
            // don't add file info for shared cache files mastered out of final file system
        }
        else {
            // file is either not in cache or is in cache but not mastered out
            writer.setFileInfo(forImage.loadedFileInfo.inode, forImage.loadedFileInfo.mtime);
        }
    } else {
        // shared cache not built by dyld or libdyld.dylib, so must be real file
        writer.setFileInfo(forImage.loadedFileInfo.inode, forImage.loadedFileInfo.mtime);
    }
#else
    if ( _platform == Platform::macOS || MachOFile::isSimulatorPlatform(_platform) ) {
        if ( macho->inDyldCache() && !_dyldCache->header.dylibsExpectedOnDisk ) {
            // don't add file info for shared cache files mastered out of final file system
        }
        else {
            // file is either not in cache or is in cache but not mastered out
            writer.setFileInfo(forImage.loadedFileInfo.inode, forImage.loadedFileInfo.mtime);
        }
    }
    else {
        // all other platforms, cache is built off-device, so inodes are not known
    }
#endif

    // add info on how to load image
    if ( !macho->inDyldCache() ) {
        writer.setMappingInfo(forImage.loadedFileInfo.sliceOffset, macho->mappedSize());
        // add code signature, if signed
        uint32_t codeSigFileOffset;
        uint32_t codeSigSize;
        if ( macho->hasCodeSignature(codeSigFileOffset, codeSigSize) ) {
            writer.setCodeSignatureLocation(codeSigFileOffset, codeSigSize);
            macho->forEachCDHash(^(const uint8_t *cdHash) {
                writer.addCDHash(cdHash);
            });
        }
        // add FairPlay encryption range if encrypted
        uint32_t fairPlayFileOffset;
        uint32_t fairPlaySize;
        if ( macho->isFairPlayEncrypted(fairPlayFileOffset, fairPlaySize) ) {
            writer.setFairPlayEncryptionRange(fairPlayFileOffset, fairPlaySize);
        }
    }

    // set path
    writer.addPath(forImage.path());
    if ( _aliases != nullptr ) {
        for (const CachedDylibAlias& alias : *_aliases) {
            if ( strcmp(alias.realPath, forImage.path()) == 0 )
                writer.addPath(alias.aliasPath);
        }
    }

    // set uuid, if has one
    uuid_t uuid;
    if ( macho->getUuid(uuid) )
        writer.setUUID(uuid);

    // set dependents
    writer.setDependents(forImage.dependents);

    // set segments
    addSegments(writer, macho);

    // record if this dylib overrides something in the cache
    if ( forImage.overrideImageNum != 0 ) {
        writer.setAsOverrideOf(forImage.overrideImageNum);
        const char* overridePath = _dyldImageArray->imageForNum(forImage.overrideImageNum)->path();
        writer.addPath(overridePath);
        if ( strcmp(overridePath, "/usr/lib/system/libdyld.dylib") == 0 )
            _libDyldImageNum = forImage.imageNum;
        else if ( strcmp(overridePath, "/usr/lib/libSystem.B.dylib") == 0 )
            _libSystemImageNum = forImage.imageNum;
    }

    // do fix up info for non-cached, and cached if building cache
    if ( !macho->inDyldCache() || _makingDyldCacheImages ) {
        if ( macho->hasChainedFixups() ) {
            addChainedFixupInfo(writer, forImage);
        }
        else {
            if ( _handlers != nullptr ) {
                reportRebasesAndBinds(writer, forImage);
            }
            else {
                // Note we have to do binds before rebases so that we know if we have missing lazy binds
                addBindInfo(writer, forImage);
                if ( _diag.noError() )
                    addRebaseInfo(writer, macho);
            }
        }
    }
    if ( _diag.hasError() ) {
        writer.setInvalid();
        return;
    }
    
    // Don't build iOSMac for now.  Just add an invalid placeholder
    if ( _makingDyldCacheImages && strncmp(forImage.path(), "/System/iOSSupport/", 19) == 0 ) {
        writer.setInvalid();
        return;
    }

    // add initializers
    bool contentRebased = forImage.contentRebased;
    __block unsigned initCount = 0;
    Diagnostics initializerDiag;
    macho->forEachInitializer(initializerDiag, contentRebased, ^(uint32_t offset) {
        ++initCount;
    }, _dyldCache);
    if ( initializerDiag.noError() ) {
        if ( initCount != 0 ) {
            BLOCK_ACCCESSIBLE_ARRAY(uint32_t, initOffsets, initCount);
            __block unsigned index = 0;
           macho->forEachInitializer(_diag, contentRebased, ^(uint32_t offset) {
                initOffsets[index++] = offset;
            }, _dyldCache);
            writer.setInitOffsets(initOffsets, initCount);
            forImage.hasInits = true;
        }
    }
    else {
        // mod_init_func section is malformed, might be self modifying pointers
        macho->forEachInitializerPointerSection(_diag, ^(uint32_t sectionOffset, uint32_t sectionSize, const uint8_t* content, bool& stop) {
            writer.setInitSectRange(sectionOffset, sectionSize);
            forImage.hasInits = true;
        });
    }


    // add terminators (except for dylibs in the cache because they are never unloaded)
    if ( !macho->inDyldCache() ) {
        __block unsigned termCount = 0;
        macho->forEachTerminator(_diag, contentRebased, ^(uint32_t offset) {
            ++termCount;
        });
        if ( termCount != 0 ) {
            BLOCK_ACCCESSIBLE_ARRAY(uint32_t, termOffsets, termCount);
            __block unsigned index = 0;
            macho->forEachTerminator(_diag, contentRebased, ^(uint32_t offset) {
                termOffsets[index++] = offset;
            });
            writer.setTermOffsets(termOffsets, termCount);
        }
    }

    // record if image has DOF sections
    STACK_ALLOC_ARRAY(uint32_t, dofSectionOffsets, 256);
    macho->forEachDOFSection(_diag, ^(uint32_t offset) {
        dofSectionOffsets.push_back(offset);
    });
    if ( !dofSectionOffsets.empty() ) {
        writer.setDofOffsets(dofSectionOffsets);
    }

}

void ClosureBuilder::addSegments(ImageWriter& writer, const MachOAnalyzer* mh)
{
    const uint32_t segCount = mh->segmentCount();
    if ( mh->inDyldCache() ) {
        uint64_t cacheUnslideBaseAddress = _dyldCache->unslidLoadAddress();
        BLOCK_ACCCESSIBLE_ARRAY(Image::DyldCacheSegment, segs, segCount);
        mh->forEachSegment(^(const MachOAnalyzer::SegmentInfo& info, bool& stop) {
            segs[info.segIndex] = { (uint32_t)(info.vmAddr-cacheUnslideBaseAddress), (uint32_t)info.vmSize, info.protections };
        });
        writer.setCachedSegments(segs, segCount);
    }
    else {
        const uint32_t   pageSize          = (mh->uses16KPages() ? 0x4000 : 0x1000);
        __block uint32_t diskSegIndex      = 0;
        __block uint32_t totalPageCount    = 0;
        __block uint32_t lastFileOffsetEnd = 0;
        __block uint64_t lastVmAddrEnd     = 0;
        BLOCK_ACCCESSIBLE_ARRAY(Image::DiskSegment, dsegs, segCount*3); // room for padding
        mh->forEachSegment(^(const MachOAnalyzer::SegmentInfo& info, bool& stop) {
            if ( (info.fileOffset != 0) && (info.fileOffset != lastFileOffsetEnd) ) {
                Image::DiskSegment filePadding;
                filePadding.filePageCount   = (info.fileOffset - lastFileOffsetEnd)/pageSize;
                filePadding.vmPageCount     = 0;
                filePadding.permissions     = 0;
                filePadding.paddingNotSeg   = 1;
                dsegs[diskSegIndex++] = filePadding;
            }
            if ( (lastVmAddrEnd != 0) && (info.vmAddr != lastVmAddrEnd) ) {
                Image::DiskSegment vmPadding;
                vmPadding.filePageCount   = 0;
                vmPadding.vmPageCount     = (info.vmAddr - lastVmAddrEnd)/pageSize;
                vmPadding.permissions     = 0;
                vmPadding.paddingNotSeg   = 1;
                dsegs[diskSegIndex++] = vmPadding;
                totalPageCount += vmPadding.vmPageCount;
            }
            {
                Image::DiskSegment segInfo;
                segInfo.filePageCount   = (info.fileSize+pageSize-1)/pageSize;
                segInfo.vmPageCount     = (info.vmSize+pageSize-1)/pageSize;
                segInfo.permissions     = info.protections & 7;
                segInfo.paddingNotSeg   = 0;
                if ( info.readOnlyData )
                    segInfo.permissions  = Image::DiskSegment::kReadOnlyDataPermissions;
                dsegs[diskSegIndex++] = segInfo;
                totalPageCount   += segInfo.vmPageCount;
                if ( info.fileSize != 0 )
                    lastFileOffsetEnd = (uint32_t)(info.fileOffset + info.fileSize);
                if ( info.vmSize != 0 )
                    lastVmAddrEnd     = info.vmAddr + info.vmSize;
            }
        });
        writer.setDiskSegments(dsegs, diskSegIndex);
    }
}

static bool isTupleFixup(uint64_t tupleSectVmStartOffset, uint64_t tupleSectVmEndOffset, uint64_t imageOffsetOfFixup, uint32_t entrySize, uint32_t& tupleIndex)
{
    if ( imageOffsetOfFixup < tupleSectVmStartOffset )
        return false;
    if ( imageOffsetOfFixup > tupleSectVmEndOffset )
        return false;
    uint64_t offsetIntoSection = imageOffsetOfFixup - tupleSectVmStartOffset;
    tupleIndex = (uint32_t)(offsetIntoSection/entrySize);
    return (tupleIndex*entrySize == offsetIntoSection) || ((tupleIndex*entrySize+entrySize/2) == offsetIntoSection);
}

void ClosureBuilder::addInterposingTuples(LaunchClosureWriter& writer, const Image* image, const MachOAnalyzer* mh)
{
    const unsigned pointerSize  = mh->pointerSize();
    const uint64_t baseAddress  = mh->preferredLoadAddress();
    mh->forEachInterposingSection(_diag, ^(uint64_t sectVmOffset, uint64_t sectVmSize, bool &stop) {
        const uint32_t entrySize = 2*pointerSize;
        const uint32_t tupleCount = (uint32_t)(sectVmSize/entrySize);
        const uint64_t sectVmEndOffset = sectVmOffset + sectVmSize;
        BLOCK_ACCCESSIBLE_ARRAY(InterposingTuple, resolvedTuples, tupleCount);
        for (uint32_t i=0; i < tupleCount; ++i) {
            resolvedTuples[i].stockImplementation.absolute.kind  = Image::ResolvedSymbolTarget::kindAbsolute;
            resolvedTuples[i].stockImplementation.absolute.value = 0;
            resolvedTuples[i].newImplementation.absolute.kind    = Image::ResolvedSymbolTarget::kindAbsolute;
            resolvedTuples[i].newImplementation.absolute.value   = 0;
        }
        // figure out what the replacement (rebase) and replacement (bind) of the tuple point to
        image->forEachFixup(^(uint64_t imageOffsetToRebase, bool& rebaseStop) {
            uint32_t tupleIndex;
            if ( isTupleFixup(sectVmOffset, sectVmEndOffset, imageOffsetToRebase, entrySize, tupleIndex) ) {
                const void* content = (uint8_t*)mh + imageOffsetToRebase;
                uint64_t unslidTargetAddress = mh->is64() ?  *(uint64_t*)content : *(uint32_t*)content;
                resolvedTuples[tupleIndex].newImplementation.image.kind     = Image::ResolvedSymbolTarget::kindImage;
                resolvedTuples[tupleIndex].newImplementation.image.imageNum = image->imageNum();
                resolvedTuples[tupleIndex].newImplementation.image.offset   = unslidTargetAddress - mh->preferredLoadAddress();
            }
        },
        ^(uint64_t imageOffsetToBind, Image::ResolvedSymbolTarget bindTarget, bool &bindStop) {
            uint32_t tupleIndex;
            if ( isTupleFixup(sectVmOffset, sectVmEndOffset, imageOffsetToBind, entrySize, tupleIndex) ) {
                resolvedTuples[tupleIndex].stockImplementation = bindTarget;
            }
        },
        ^(uint64_t imageOffsetToStartsInfo, const Array<Image::ResolvedSymbolTarget>& targets, bool& chainStop) {
            mh->withChainStarts(_diag, imageOffsetToStartsInfo, ^(const dyld_chained_starts_in_image* startsInfo) {
                mh->forEachFixupInAllChains(_diag, startsInfo, false, ^(MachOLoaded::ChainedFixupPointerOnDisk* fixupLoc, const dyld_chained_starts_in_segment* segInfo, bool& fixupsStop) {
                    uint64_t fixupOffset = (uint8_t*)fixupLoc - (uint8_t*)mh;
                    uint32_t tupleIndex;
                    if ( !isTupleFixup(sectVmOffset, sectVmEndOffset, fixupOffset, entrySize, tupleIndex) )
                        return;
                    uint32_t bindOrdinal;
                    uint64_t rebaseTargetOffset;
                    if ( fixupLoc->isBind(segInfo->pointer_format, bindOrdinal) ) {
                        if ( bindOrdinal < targets.count() ) {
                            resolvedTuples[tupleIndex].stockImplementation = targets[bindOrdinal];
                        }
                        else {
                            _diag.error("out of range bind ordinal %d (max %lu)", bindOrdinal, targets.count());
                            fixupsStop = true;
                        }
                    }
                    else if ( fixupLoc->isRebase(segInfo->pointer_format, baseAddress, rebaseTargetOffset) ) {
                        resolvedTuples[tupleIndex].newImplementation.image.kind     = Image::ResolvedSymbolTarget::kindImage;
                        resolvedTuples[tupleIndex].newImplementation.image.imageNum = image->imageNum();
                        resolvedTuples[tupleIndex].newImplementation.image.offset   = rebaseTargetOffset;
                    }
                });
            });
        },
        ^(uint64_t imageOffsetToFixup) {
            // objc optimisation can't be interposed so nothing to do here.
        },
        ^(uint64_t imageOffsetToBind, Image::ResolvedSymbolTarget bindTarget, bool &bindStop) {
            // objc protocol optimisation fixups can't be interposed so nothing to do here.
        },
        ^(uint64_t imageOffsetToFixup, uint32_t selectorIndex, bool inSharedCache, bool &fixupStop) {
            // objc selector optimisation fixups can't be interposed so nothing to do here.
        },
        ^(uint64_t imageOffsetToFixup, bool &fixupStop) {
            // objc stable Swift optimisation fixups can't be interposed so nothing to do here.
        },
        ^(uint64_t imageOffsetToFixup, bool &fixupStop) {
            // objc method list optimisation fixups can't be interposed so nothing to do here.
        });

        // remove any tuples in which both sides are not set (or target is weak-import NULL)
        STACK_ALLOC_ARRAY(InterposingTuple, goodTuples, tupleCount);
        for (uint32_t i=0; i < tupleCount; ++i) {
            if ( (resolvedTuples[i].stockImplementation.image.kind != Image::ResolvedSymbolTarget::kindAbsolute)
              && (resolvedTuples[i].newImplementation.image.kind != Image::ResolvedSymbolTarget::kindAbsolute) )
                goodTuples.push_back(resolvedTuples[i]);
        }
        writer.addInterposingTuples(goodTuples);

        // if the target of the interposing is in the dyld shared cache, add a PatchEntry so the cache is fixed up at launch
        STACK_ALLOC_ARRAY(Closure::PatchEntry, patches, goodTuples.count());
        for (const InterposingTuple& aTuple : goodTuples) {
            if ( aTuple.stockImplementation.sharedCache.kind == Image::ResolvedSymbolTarget::kindSharedCache ) {
                uint32_t imageIndex;
                assert(_dyldCache->addressInText((uint32_t)aTuple.stockImplementation.sharedCache.offset, &imageIndex));
                ImageNum imageInCache = imageIndex+1;
                Closure::PatchEntry patch;
                patch.exportCacheOffset      = (uint32_t)aTuple.stockImplementation.sharedCache.offset;
                patch.overriddenDylibInCache = imageInCache;
                patch.replacement            = aTuple.newImplementation;
                patches.push_back(patch);
            }
        }
        writer.addCachePatches(patches);
    });
}

void ClosureBuilder::addRebaseInfo(ImageWriter& writer, const MachOAnalyzer* mh)
{
	const uint64_t ptrSize = mh->pointerSize();
    Image::RebasePattern maxLeapPattern = { 0xFFFFF, 0, 0xF };
    const uint64_t maxLeapCount = maxLeapPattern.repeatCount * maxLeapPattern.skipCount;
    STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::RebasePattern, rebaseEntries, 1024);
    __block uint64_t lastLocation = -ptrSize;
	mh->forEachRebase(_diag, !_foundMissingLazyBinds, ^(uint64_t runtimeOffset, bool& stop) {
        const uint64_t delta   = runtimeOffset - lastLocation;
        const bool     aligned = ((delta % ptrSize) == 0);
        if ( delta == ptrSize ) {
            // this rebase location is contiguous to previous
            if ( rebaseEntries.back().contigCount < 255 ) {
                // just bump previous's contigCount
                rebaseEntries.back().contigCount++;
            }
            else {
                // previous contiguous run already has max 255, so start a new run
                rebaseEntries.push_back({ 1, 1, 0 });
            }
        }
        else if ( aligned && (delta <= (ptrSize*15)) ) {
            // this rebase is within skip distance of last rebase
            rebaseEntries.back().skipCount = (uint8_t)((delta-ptrSize)/ptrSize);
            int lastIndex = (int)(rebaseEntries.count() - 1);
            if ( lastIndex > 1 ) {
                if ( (rebaseEntries[lastIndex].contigCount == rebaseEntries[lastIndex-1].contigCount)
                  && (rebaseEntries[lastIndex].skipCount   == rebaseEntries[lastIndex-1].skipCount) ) {
                    // this entry as same contig and skip as prev, so remove it and bump repeat count of previous
                    rebaseEntries.pop_back();
                    rebaseEntries.back().repeatCount += 1;
                }
            }
            rebaseEntries.push_back({ 1, 1, 0 });
        }
        else {
            uint64_t advanceCount = (delta-ptrSize);
            if ( (runtimeOffset < lastLocation) && (lastLocation != -ptrSize) ) {
                // out of rebases! handle this be resting rebase offset to zero
                rebaseEntries.push_back({ 0, 0, 0 });
                advanceCount = runtimeOffset;
            }
            // if next rebase is too far to reach with one pattern, use series
            while ( advanceCount > maxLeapCount ) {
                rebaseEntries.push_back(maxLeapPattern);
                advanceCount -= maxLeapCount;
            }
            // if next rebase is not reachable with skipCount==1 or skipCount==15, add intermediate
            while ( advanceCount > maxLeapPattern.repeatCount ) {
                uint64_t count = advanceCount / maxLeapPattern.skipCount;
                rebaseEntries.push_back({ (uint32_t)count, 0, maxLeapPattern.skipCount });
                advanceCount -= (count*maxLeapPattern.skipCount);
            }
            if ( advanceCount != 0 )
                rebaseEntries.push_back({ (uint32_t)advanceCount, 0, 1 });
            rebaseEntries.push_back({ 1, 1, 0 });
        }
        lastLocation = runtimeOffset;
	});
    writer.setRebaseInfo(rebaseEntries);

    // i386 programs also use text relocs to rebase stubs
    if ( mh->cputype == CPU_TYPE_I386 ) {
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::TextFixupPattern, textRebases, 512);
        __block uint64_t lastOffset = -4;
        mh->forEachTextRebase(_diag, ^(uint64_t runtimeOffset, bool& stop) {
            if ( textRebases.freeCount() < 2 ) {
                _diag.error("too many text rebase locations (%ld) in %s", textRebases.maxCount(), writer.currentImage()->path());
                stop = true;
            }
            bool mergedIntoPrevious = false;
            if ( (runtimeOffset > lastOffset) && !textRebases.empty() ) {
                uint32_t skipAmount = (uint32_t)(runtimeOffset - lastOffset);
                if ( (textRebases.back().repeatCount == 1) && (textRebases.back().skipCount == 0) ) {
                    textRebases.back().repeatCount = 2;
                    textRebases.back().skipCount   = skipAmount;
                    mergedIntoPrevious             = true;
                }
                else if ( textRebases.back().skipCount == skipAmount ) {
                    textRebases.back().repeatCount += 1;
                    mergedIntoPrevious = true;
                }
            }
            if ( !mergedIntoPrevious ) {
                Image::TextFixupPattern pattern;
                pattern.target.raw    = 0;
                pattern.startVmOffset = (uint32_t)runtimeOffset;
                pattern.repeatCount   = 1;
                pattern.skipCount     = 0;
                textRebases.push_back(pattern);
            }
            lastOffset = runtimeOffset;
        });
        writer.setTextRebaseInfo(textRebases);
    }
}


void ClosureBuilder::forEachBind(BuilderLoadedImage& forImage, void (^handler)(uint64_t runtimeOffset, Image::ResolvedSymbolTarget target, const ResolvedTargetInfo& targetInfo, bool& stop),
                                                               void (^strongHandler)(const char* strongSymbolName),
                                                               void (^missingLazyBindHandler)())
{
    __block int                         lastLibOrdinal  = 256;
    __block const char*                 lastSymbolName  = nullptr;
    __block uint64_t                    lastAddend      = 0;
    __block Image::ResolvedSymbolTarget target;
    __block ResolvedTargetInfo          targetInfo;
    forImage.loadAddress()->forEachBind(_diag, ^(uint64_t runtimeOffset, int libOrdinal, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop) {
        if ( (symbolName == lastSymbolName) && (libOrdinal == lastLibOrdinal) && (addend == lastAddend) ) {
            // same symbol lookup as last location
            handler(runtimeOffset, target, targetInfo, stop);
        }
        else if ( findSymbol(forImage, libOrdinal, symbolName, weakImport, lazyBind, addend, target, targetInfo) ) {
            if ( !targetInfo.skippableWeakDef ) {
                handler(runtimeOffset, target, targetInfo, stop);
                lastSymbolName = symbolName;
                lastLibOrdinal = libOrdinal;
                lastAddend     = addend;
            }
        }
        else {
            stop = true;
        }
    }, ^(const char* symbolName) {
        strongHandler(symbolName);
    }, ^() {
        missingLazyBindHandler();
    });
}

void ClosureBuilder::addBindInfo(ImageWriter& writer, BuilderLoadedImage& forImage)
{
    const uint32_t ptrSize = forImage.loadAddress()->pointerSize();
	STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::BindPattern, binds, 512);
    __block uint64_t                    lastOffset = -ptrSize;
	__block Image::ResolvedSymbolTarget lastTarget = { {0, 0} };
    forEachBind(forImage, ^(uint64_t runtimeOffset, Image::ResolvedSymbolTarget target, const ResolvedTargetInfo& targetInfo, bool& stop) {
        if ( targetInfo.weakBindCoalese )  {
            // may be previous bind to this location
            // if so, update that rather create new BindPattern
            for (Image::BindPattern& aBind : binds) {
                if ( (aBind.startVmOffset == runtimeOffset) && (aBind.repeatCount == 1)  && (aBind.skipCount == 0) ) {
                    aBind.target = target;
                    return;
                }
            }
        }
        bool mergedIntoPrevious = false;
        if ( !mergedIntoPrevious && (target == lastTarget) && (runtimeOffset > lastOffset) && !binds.empty() ) {
            uint64_t skipAmount = (runtimeOffset - lastOffset - ptrSize)/ptrSize;
            if ( skipAmount*ptrSize != (runtimeOffset - lastOffset - ptrSize) ) {
                // misaligned pointer means we cannot optimize 
            }
            else {
                if ( (binds.back().repeatCount == 1) && (binds.back().skipCount == 0) && (skipAmount <= 255) ) {
                    binds.back().repeatCount = 2;
                    binds.back().skipCount   = skipAmount;
                    assert(binds.back().skipCount == skipAmount); // check overflow
                    mergedIntoPrevious       = true;
                }
                else if ( (binds.back().skipCount == skipAmount) && (binds.back().repeatCount < 0xfff) ) {
                    uint32_t prevRepeatCount = binds.back().repeatCount;
                    binds.back().repeatCount += 1;
                    assert(binds.back().repeatCount > prevRepeatCount); // check overflow
                    mergedIntoPrevious       = true;
                }
            }
        }
        if ( (target == lastTarget) && (runtimeOffset == lastOffset) && !binds.empty() ) {
            // duplicate bind for same location, ignore this one
            mergedIntoPrevious = true;
        }
        if ( !mergedIntoPrevious ) {
            Image::BindPattern pattern;
            pattern.target        = target;
            pattern.startVmOffset = runtimeOffset;
            pattern.repeatCount   = 1;
            pattern.skipCount     = 0;
            assert(pattern.startVmOffset == runtimeOffset);
            binds.push_back(pattern);
        }
        lastTarget = target;
        lastOffset = runtimeOffset;
	}, ^(const char* strongSymbolName) {
        if ( !_makingDyldCacheImages ) {
            // something has a strong symbol definition that may override a weak impl in the dyld cache
            Image::ResolvedSymbolTarget strongOverride;
            ResolvedTargetInfo          strongTargetInfo;
            if ( findSymbolInImage(forImage.loadAddress(), strongSymbolName, 0, false, false, strongOverride, strongTargetInfo) ) {
                for (const BuilderLoadedImage& li : _loadedImages) {
                    if ( li.loadAddress()->inDyldCache() && li.loadAddress()->hasWeakDefs() ) {
                        Image::ResolvedSymbolTarget implInCache;
                        ResolvedTargetInfo          implInCacheInfo;
                        if ( findSymbolInImage(li.loadAddress(), strongSymbolName, 0, false, false, implInCache, implInCacheInfo) ) {
                            // found another instance in some dylib in dyld cache, will need to patch it
                            Closure::PatchEntry patch;
                            patch.exportCacheOffset      = (uint32_t)implInCache.sharedCache.offset;
                            patch.overriddenDylibInCache = li.imageNum;
                            patch.replacement            = strongOverride;
                            _weakDefCacheOverrides.push_back(patch);
                        }
                    }
                }
            }
        }
    }, ^() {
        _foundMissingLazyBinds = true;
    });

    // check for __dyld section in main executable to support licenseware
    if ( forImage.loadAddress()->filetype == MH_EXECUTE ) {
        forImage.loadAddress()->forEachSection(^(const MachOAnalyzer::SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
            if ( (strcmp(sectInfo.sectName, "__dyld") == 0) && (strcmp(sectInfo.segInfo.segName, "__DATA") == 0) ) {
                // find dyld3::compatFuncLookup in libdyld.dylib
                assert(_libDyldImageNum != 0);
                Image::ResolvedSymbolTarget lookupFuncTarget;
                ResolvedTargetInfo          lookupFuncInfo;
                if ( findSymbolInImage(findLoadedImage(_libDyldImageNum).loadAddress(), "__ZN5dyld316compatFuncLookupEPKcPPv", 0, false, false, lookupFuncTarget, lookupFuncInfo) ) {
                    // add bind to set second pointer in __dyld section to be dyld3::compatFuncLookup
                    uint64_t runtimeOffset = sectInfo.sectAddr - forImage.loadAddress()->preferredLoadAddress() + forImage.loadAddress()->pointerSize();
                    Image::BindPattern compatFuncPattern;
                    compatFuncPattern.target        = lookupFuncTarget;
                    compatFuncPattern.startVmOffset = runtimeOffset;
                    compatFuncPattern.repeatCount   = 1;
                    compatFuncPattern.skipCount     = 0;
                    assert(compatFuncPattern.startVmOffset == runtimeOffset);
                    binds.push_back(compatFuncPattern);
                }
                else {
                    _diag.error("libdyld.dylib is dyld3::compatFuncLookup");
                }
            }
        });
    }

    writer.setBindInfo(binds);
}

void ClosureBuilder::reportRebasesAndBinds(ImageWriter& writer, BuilderLoadedImage& forImage)
{
    // report all rebases
    forImage.loadAddress()->forEachRebase(_diag, true, ^(uint64_t runtimeOffset, bool& stop) {
        _handlers->rebase(forImage.imageNum, forImage.loadAddress(), (uint32_t)runtimeOffset);
    });

    // report all binds
    forEachBind(forImage, ^(uint64_t runtimeOffset, Image::ResolvedSymbolTarget target, const ResolvedTargetInfo& targetInfo, bool& stop) {
        _handlers->bind(forImage.imageNum, forImage.loadAddress(), (uint32_t)runtimeOffset, target, targetInfo);
    },
    ^(const char* strongSymbolName) {},
    ^() { });

    // i386 programs also use text relocs to rebase stubs
    if ( forImage.loadAddress()->cputype == CPU_TYPE_I386 ) {
        // FIX ME
    }
}

// These are mangled symbols for all the variants of operator new and delete
// which a main executable can define (non-weak) and override the
// weak-def implementation in the OS.
static const char* const sTreatAsWeak[] = {
    "__Znwm", "__ZnwmRKSt9nothrow_t",
    "__Znam", "__ZnamRKSt9nothrow_t",
    "__ZdlPv", "__ZdlPvRKSt9nothrow_t", "__ZdlPvm",
    "__ZdaPv", "__ZdaPvRKSt9nothrow_t", "__ZdaPvm",
    "__ZnwmSt11align_val_t", "__ZnwmSt11align_val_tRKSt9nothrow_t",
    "__ZnamSt11align_val_t", "__ZnamSt11align_val_tRKSt9nothrow_t",
    "__ZdlPvSt11align_val_t", "__ZdlPvSt11align_val_tRKSt9nothrow_t", "__ZdlPvmSt11align_val_t",
    "__ZdaPvSt11align_val_t", "__ZdaPvSt11align_val_tRKSt9nothrow_t", "__ZdaPvmSt11align_val_t"
};


void ClosureBuilder::addChainedFixupInfo(ImageWriter& writer, BuilderLoadedImage& forImage)
{
    // build array of targets
    STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::ResolvedSymbolTarget, targets,     1024);
    STACK_ALLOC_OVERFLOW_SAFE_ARRAY(ResolvedTargetInfo,          targetInfos, 1024);
    forImage.loadAddress()->forEachChainedFixupTarget(_diag, ^(int libOrdinal, const char* symbolName, uint64_t addend, bool weakImport, bool& stop) {
        Image::ResolvedSymbolTarget target;
        ResolvedTargetInfo          targetInfo;
        if ( !findSymbol(forImage, libOrdinal, symbolName, weakImport, false, addend, target, targetInfo) ) {
            const char* expectedInPath = forImage.loadAddress()->dependentDylibLoadPath(libOrdinal-1);
            _diag.error("symbol '%s' not found, expected in '%s', needed by '%s'", symbolName, expectedInPath, forImage.path());
            stop = true;
            return;
        }
        if ( libOrdinal == BIND_SPECIAL_DYLIB_WEAK_LOOKUP ) {
            // add if not already in array
            bool alreadyInArray = false;
            for (const char* sym : _weakDefsFromChainedBinds) {
                if ( strcmp(sym, symbolName) == 0 ) {
                    alreadyInArray = true;
                    break;
                }
            }
            if ( !alreadyInArray )
                _weakDefsFromChainedBinds.push_back(symbolName);
        }
        targets.push_back(target);
        targetInfos.push_back(targetInfo);
    });
    if ( _diag.hasError() )
        return;

    uint64_t chainStartsOffset = forImage.loadAddress()->chainStartsOffset();
    if ( _handlers != nullptr ) {
        forImage.loadAddress()->withChainStarts(_diag, chainStartsOffset, ^(const dyld_chained_starts_in_image* starts) {
            _handlers->chainedBind(forImage.imageNum, forImage.loadAddress(), starts, targets, targetInfos);
        });
    }
    else {
        writer.setChainedFixups(chainStartsOffset, targets);
    }

    // with chained fixups, main executable may define symbol that overrides weak-defs but has no fixup
    if ( _isLaunchClosure && forImage.loadAddress()->hasWeakDefs() && forImage.loadAddress()->isMainExecutable() ) {
        for (const char* weakSymbolName : sTreatAsWeak) {
            Diagnostics exportDiag;
            dyld3::MachOAnalyzer::FoundSymbol foundInfo;
            if ( forImage.loadAddress()->findExportedSymbol(exportDiag, weakSymbolName, false, foundInfo, nullptr) ) {
                _weakDefsFromChainedBinds.push_back(weakSymbolName);
            }
        }
    }
}


bool ClosureBuilder::findSymbolInImage(const MachOAnalyzer* macho, const char* symbolName, uint64_t addend, bool followReExports,
                                       bool weakImport, Image::ResolvedSymbolTarget& target, ResolvedTargetInfo& targetInfo)
{
    targetInfo.foundInDylib        = nullptr;
    targetInfo.requestedSymbolName = symbolName;
    targetInfo.addend              = addend;
    targetInfo.weakBindCoalese     = false;
    targetInfo.weakBindSameImage   = false;
    targetInfo.isWeakDef           = false;
    targetInfo.skippableWeakDef    = false;
    MachOLoaded::DependentToMachOLoaded reexportFinder = ^(const MachOLoaded* mh, uint32_t depIndex) {
        return (const MachOLoaded*)findDependent(mh, depIndex);
    };
    MachOAnalyzer::DependentToMachOLoaded finder = nullptr;
    if ( followReExports )
        finder = reexportFinder;

    dyld3::MachOAnalyzer::FoundSymbol foundInfo;
    if ( macho->findExportedSymbol(_diag, symbolName, weakImport, foundInfo, finder) ) {
        const MachOAnalyzer* impDylib = (const MachOAnalyzer*)foundInfo.foundInDylib;
        targetInfo.foundInDylib    = foundInfo.foundInDylib;
        targetInfo.foundSymbolName = foundInfo.foundSymbolName;
        if ( foundInfo.isWeakDef )
            targetInfo.isWeakDef = true;
        if ( foundInfo.kind == MachOAnalyzer::FoundSymbol::Kind::absolute ) {
            target.absolute.kind   = Image::ResolvedSymbolTarget::kindAbsolute;
            target.absolute.value  = foundInfo.value + addend;
        }
        else if ( impDylib->inDyldCache() ) {
            uint64_t offsetValue = (uint8_t*)impDylib - (uint8_t*)_dyldCache + foundInfo.value + addend;
            target.sharedCache.kind   = Image::ResolvedSymbolTarget::kindSharedCache;
            target.sharedCache.offset = offsetValue;
            assert(target.sharedCache.offset == offsetValue);
        }
        else {
            uint64_t offsetValue = foundInfo.value + addend;
            target.image.kind     = Image::ResolvedSymbolTarget::kindImage;
            target.image.imageNum = findLoadedImage(impDylib).imageNum;
            target.image.offset   = offsetValue;
            assert(target.image.offset == offsetValue);
        }
        return true;
    }
    return false;
}

bool ClosureBuilder::findSymbol(BuilderLoadedImage& fromImage, int libOrdinal, const char* symbolName, bool weakImport, bool lazyBind,
                                uint64_t addend, Image::ResolvedSymbolTarget& target, ResolvedTargetInfo& targetInfo)
{
    target.raw                      = 0;
    targetInfo.weakBindCoalese      = false;
    targetInfo.weakBindSameImage    = false;
    targetInfo.isWeakDef            = false;
    targetInfo.skippableWeakDef     = false;
    targetInfo.requestedSymbolName  = symbolName;
    targetInfo.libOrdinal           = libOrdinal;
    if ( libOrdinal == BIND_SPECIAL_DYLIB_FLAT_LOOKUP ) {
        for (const BuilderLoadedImage& li : _loadedImages) {
            if ( !li.rtldLocal && findSymbolInImage(li.loadAddress(), symbolName, addend, true, weakImport, target, targetInfo) )
                return true;
        }
        if ( weakImport ) {
            target.absolute.kind  = Image::ResolvedSymbolTarget::kindAbsolute;
            target.absolute.value = 0;
            // Record that we found a missing weak import so that the objc optimizer doens't have to check
            fromImage.hasMissingWeakImports = true;
            return true;
        }
        // <rdar://problem/44315944> closures should bind missing lazy-bind symbols to a missing symbol handler in libdyld in flat namespace
        if ( lazyBind && _allowMissingLazies ) {
            if ( findMissingSymbolHandler(target, targetInfo) )
                return true;
        }
        _diag.error("symbol '%s' not found, expected in flat namespace by '%s'", symbolName, fromImage.path());
    }
    else if ( libOrdinal == BIND_SPECIAL_DYLIB_WEAK_LOOKUP ) {
        // to resolve weakDef coalesing, we need to search all images in order and use first definition
        // but, if first found is a weakDef, a later non-weak def overrides that
        bool foundWeakDefImpl   = false;
        bool foundStrongDefImpl = false;
        bool foundImpl          = false;

        if ( _makingDyldCacheImages ) {
            // _loadedImages is all dylibs in the dyld cache, it is not load-order, so need alterate weak-def binding algorithm
            // look first in /usr/lib/libc++, most will be here
            for (const BuilderLoadedImage& li : _loadedImages) {
                if ( li.loadAddress()->hasWeakDefs() && (strncmp(li.path(), "/usr/lib/libc++", 15) == 0) ) {
                   if ( findSymbolInImage(li.loadAddress(), symbolName, addend, false, weakImport, target, targetInfo) ) {
                        foundImpl = true;
                        break;
                    }
                }
            }
            // if not found, try looking in the images itself, most custom weak-def symbols have a copy in the image itself
            if ( !foundImpl ) {
                if ( findSymbolInImage(fromImage.loadAddress(), symbolName, addend, false, weakImport, target, targetInfo) ) {
                    foundImpl = true;
                }
            }
            // if still not found, then this is the rare case of a simple use of a weak-def symbol
            if ( !foundImpl ) {
                // look in all direct dependents
                for (Image::LinkedImage child : fromImage.dependents) {
                    if (child.imageNum() == kMissingWeakLinkedImage)
                        continue;
                    BuilderLoadedImage& childLi = findLoadedImage(child.imageNum());
                    if ( childLi.loadAddress()->hasWeakDefs() && findSymbolInImage(childLi.loadAddress(), symbolName, addend, false, weakImport, target, targetInfo) ) {
                        foundImpl = true;
                        break;
                    }
                }
            }
            targetInfo.weakBindCoalese = true;
        }
        else {
            // walk images in load-order to find first that implements this symbol
            Image::ResolvedSymbolTarget  aTarget;
            ResolvedTargetInfo           aTargetInfo;
            STACK_ALLOC_ARRAY(const BuilderLoadedImage*, cachedDylibsUsingSymbol, 1024);
            for (const BuilderLoadedImage& li : _loadedImages) {
                // only search images with weak-defs that were not loaded with RTLD_LOCAL
                if ( li.loadAddress()->hasWeakDefs() && !li.rtldLocal ) {
                    if ( findSymbolInImage(li.loadAddress(), symbolName, addend, false, weakImport, aTarget, aTargetInfo) ) {
                        foundImpl = true;
                        // with non-chained images, weak-defs first have a rebase to their local impl, and a weak-bind which allows earlier impls to override
                        if ( !li.loadAddress()->hasChainedFixups() && (aTargetInfo.foundInDylib == fromImage.loadAddress()) )
                            targetInfo.weakBindSameImage = true;
                        if ( aTargetInfo.isWeakDef ) {
                            // found a weakDef impl, if this is first found, set target to this
                            if ( !foundWeakDefImpl && !foundStrongDefImpl ) {
                                target      = aTarget;
                                targetInfo  = aTargetInfo;
                            }
                            foundWeakDefImpl = true;
                        }
                        else {
                            // found a non-weak impl, use this (unless early strong found)
                            if ( !foundStrongDefImpl ) {
                                target      = aTarget;
                                targetInfo  = aTargetInfo;
                            }
                            foundStrongDefImpl = true;
                        }
                    }
                    if ( foundImpl && li.loadAddress()->inDyldCache() )
                        cachedDylibsUsingSymbol.push_back(&li);
                }
            }

            // now that final target found, if any dylib in dyld cache uses that symbol name, redirect it to new target
            if ( !cachedDylibsUsingSymbol.empty() ) {
                for (const BuilderLoadedImage* li : cachedDylibsUsingSymbol) {
                    Image::ResolvedSymbolTarget implInCache;
                    ResolvedTargetInfo          implInCacheInfo;
                    if ( findSymbolInImage(li->loadAddress(), symbolName, addend, false, weakImport, implInCache, implInCacheInfo) ) {
                        if ( implInCache != target ) {
                            // found another instance in some dylib in dyld cache, will need to patch it
                            Closure::PatchEntry patch;
                            patch.exportCacheOffset      = (uint32_t)implInCache.sharedCache.offset;
                            patch.overriddenDylibInCache = li->imageNum;
                            patch.replacement            = target;
                            _weakDefCacheOverrides.push_back(patch);
                        }
                    }
                }
            }
            targetInfo.weakBindCoalese = true;
        }

        if ( foundImpl )
            return true;
        if ( weakImport ) {
            target.absolute.kind  = Image::ResolvedSymbolTarget::kindAbsolute;
            target.absolute.value = 0;
            return true;
        }
        if ( ! fromImage.loadAddress()->hasChainedFixups() ) {
            // support old binaries where symbols have been stripped and have weak_bind to itself
            targetInfo.skippableWeakDef = true;
            return true;
        }

        _diag.error("symbol '%s' not found, expected to be weak-def coalesced by '%s'", symbolName, fromImage.path());
    }
    else {
        const BuilderLoadedImage* targetLoadedImage = nullptr;
        if ( (libOrdinal > 0) && (libOrdinal <= (int)fromImage.dependents.count()) ) {
            ImageNum childNum = fromImage.dependents[libOrdinal - 1].imageNum();
            if ( childNum != kMissingWeakLinkedImage ) {
                targetLoadedImage = &findLoadedImage(childNum);
            }
        }
        else if ( libOrdinal == BIND_SPECIAL_DYLIB_SELF ) {
            targetLoadedImage = &fromImage;
        }
        else if ( libOrdinal == BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE ) {
            targetLoadedImage = &_loadedImages[_mainProgLoadIndex];
        }
        else {
            _diag.error("unknown special ordinal %d in %s", libOrdinal, fromImage.path());
            return false;
        }

        if ( targetLoadedImage != nullptr ) {
            if ( findSymbolInImage(targetLoadedImage->loadAddress(), symbolName, addend, true, weakImport, target, targetInfo) )
                return true;
        }

        if ( weakImport ) {
            target.absolute.kind  = Image::ResolvedSymbolTarget::kindAbsolute;
            target.absolute.value = 0;
            // Record that we found a missing weak import so that the objc optimizer doens't have to check
            fromImage.hasMissingWeakImports = true;
            return true;
        }

        // <rdar://problem/43315403> closures should bind missing lazy-bind symbols to a missing symbol handler in libdyld
        if ( lazyBind && _allowMissingLazies ) {
            if ( findMissingSymbolHandler(target, targetInfo) )
                return true;
        }

        // symbol not found and not weak or lazy so error out
        const char* expectedInPath = targetLoadedImage ? targetLoadedImage->path() : "unknown";
        _diag.error("symbol '%s' not found, expected in '%s', needed by '%s'", symbolName, expectedInPath, fromImage.path());
        if ( _launchErrorInfo != nullptr ) {
            _launchErrorInfo->kind              = DYLD_EXIT_REASON_SYMBOL_MISSING;
            _launchErrorInfo->clientOfDylibPath = strdup_temp(fromImage.path());
            _launchErrorInfo->targetDylibPath   = strdup_temp(expectedInPath);
            _launchErrorInfo->symbol            = symbolName;
        }
    }
    return false;
}


bool ClosureBuilder::findMissingSymbolHandler(Image::ResolvedSymbolTarget& target, ResolvedTargetInfo& targetInfo)
{
   for (BuilderLoadedImage& li : _loadedImages) {
         if ( li.loadAddress()->isDylib() && (strcmp(li.loadAddress()->installName(), "/usr/lib/system/libdyld.dylib") == 0) ) {
            if ( findSymbolInImage(li.loadAddress(), "__dyld_missing_symbol_abort", 0, false, false, target, targetInfo) ) {
                return true;
            }
            break;
        }
    }
    return false;
}

void ClosureBuilder::depthFirstRecurseSetInitInfo(uint32_t loadIndex, InitInfo initInfos[], uint32_t& initOrder, bool& hasError)
{
    if ( initInfos[loadIndex].visited )
        return;
    initInfos[loadIndex].visited        = true;
    initInfos[loadIndex].danglingUpward = false;

    if (_loadedImages[loadIndex].isBadImage) {
        hasError = true;
        return;
    }

    for (const Image::LinkedImage& dep : _loadedImages[loadIndex].dependents) {
        if ( dep.imageNum() == kMissingWeakLinkedImage )
            continue;
        ClosureBuilder::BuilderLoadedImage& depLi = findLoadedImage(dep.imageNum());
        uint32_t depLoadIndex = (uint32_t)_loadedImages.index(depLi);
        if ( dep.kind() == Image::LinkKind::upward ) {
            if ( !initInfos[depLoadIndex].visited )
                initInfos[depLoadIndex].danglingUpward = true;
        }
        else {
            depthFirstRecurseSetInitInfo(depLoadIndex, initInfos, initOrder, hasError);
            if (hasError)
                return;
        }
    }
    initInfos[loadIndex].initOrder = initOrder++;
}

void ClosureBuilder::computeInitOrder(ImageWriter& imageWriter, uint32_t loadIndex)
{
    // allocate array to track initializers
    InitInfo initInfos[_loadedImages.count()];
    bzero(initInfos, sizeof(initInfos));

    // recurse all images and build initializer list from bottom up
    uint32_t initOrder = 1;
    bool hasMissingDependent = false;
    depthFirstRecurseSetInitInfo(loadIndex, initInfos, initOrder, hasMissingDependent);
    if (hasMissingDependent) {
        imageWriter.setInvalid();
        return;
    }

    // any images not visited yet are are danging, force add them to end of init list
    for (uint32_t i=0; i < (uint32_t)_loadedImages.count(); ++i) {
        if ( !initInfos[i].visited && initInfos[i].danglingUpward ) {
            depthFirstRecurseSetInitInfo(i, initInfos, initOrder, hasMissingDependent);
        }
    }

    if (hasMissingDependent) {
        imageWriter.setInvalid();
        return;
    }
    
    // build array of just images with initializer
    STACK_ALLOC_ARRAY(uint32_t, indexOfImagesWithInits, _loadedImages.count());
    uint32_t index = 0;
    for (const BuilderLoadedImage& li : _loadedImages) {
        if ( initInfos[index].visited && li.hasInits ) {
            indexOfImagesWithInits.push_back(index);
        }
        ++index;
    }

    // bubble sort (FIXME)
    if ( indexOfImagesWithInits.count() > 1 ) {
        for (uint32_t i=0; i < indexOfImagesWithInits.count()-1; ++i) {
            for (uint32_t j=0; j < indexOfImagesWithInits.count()-i-1; ++j) {
                if ( initInfos[indexOfImagesWithInits[j]].initOrder > initInfos[indexOfImagesWithInits[j+1]].initOrder ) {
                    uint32_t temp               = indexOfImagesWithInits[j];
                    indexOfImagesWithInits[j]   = indexOfImagesWithInits[j+1];
                    indexOfImagesWithInits[j+1] = temp;
                }
            }
        }
    }

    // copy ImageNum of each image with initializers into array
    ImageNum initNums[indexOfImagesWithInits.count()];
    for (uint32_t i=0; i < indexOfImagesWithInits.count(); ++i) {
        initNums[i] = _loadedImages[indexOfImagesWithInits[i]].imageNum;
    }

    // add to closure info
    imageWriter.setInitsOrder(initNums, (uint32_t)indexOfImagesWithInits.count());
}

void ClosureBuilder::addClosureInfo(LaunchClosureWriter& closureWriter)
{
    // record which is libSystem
    assert(_libSystemImageNum != 0);
	closureWriter.setLibSystemImageNum(_libSystemImageNum);

    // record which is libdyld
    assert(_libDyldImageNum != 0);
    Image::ResolvedSymbolTarget entryLocation;
    ResolvedTargetInfo          entryInfo;
    if ( findSymbolInImage(findLoadedImage(_libDyldImageNum).loadAddress(), "__ZN5dyld318entryVectorForDyldE", 0, false, false, entryLocation, entryInfo) ) {
        const dyld3::LibDyldEntryVector* libDyldEntry = nullptr;
        switch ( entryLocation.image.kind ) {
            case Image::ResolvedSymbolTarget::kindSharedCache:
                libDyldEntry = (dyld3::LibDyldEntryVector*)((uint8_t*)_dyldCache + entryLocation.sharedCache.offset);
                break;
            case Image::ResolvedSymbolTarget::kindImage:
                libDyldEntry = (dyld3::LibDyldEntryVector*)((uint8_t*)findLoadedImage(entryLocation.image.imageNum).loadAddress() + entryLocation.image.offset);
                break;
        }
        if ( (libDyldEntry != nullptr) && ((libDyldEntry->binaryFormatVersion & LibDyldEntryVector::kBinaryFormatVersionMask) == dyld3::closure::kFormatVersion) )
            closureWriter.setLibDyldEntry(entryLocation);
        else
            _diag.error("libdyld.dylib entry vector is incompatible");
    }
    else {
        _diag.error("libdyld.dylib is missing entry vector");
    }

    // record which is main executable
    ImageNum mainProgImageNum = _loadedImages[_mainProgLoadIndex].imageNum;
    closureWriter.setTopImageNum(mainProgImageNum);

    // add entry
    uint32_t    entryOffset;
    bool        usesCRT;
    if ( _loadedImages[_mainProgLoadIndex].loadAddress()->getEntry(entryOffset, usesCRT) ) {
        Image::ResolvedSymbolTarget location;
        location.image.kind     = Image::ResolvedSymbolTarget::kindImage;
        location.image.imageNum = mainProgImageNum;
        location.image.offset   = entryOffset;
        if ( usesCRT )
            closureWriter.setStartEntry(location);
        else
            closureWriter.setMainEntry(location);
    }

    // add env vars that must match at launch time
    _pathOverrides.forEachEnvVar(^(const char* envVar) {
        closureWriter.addEnvVar(envVar);
    });

    // add list of files which must be missing
    STACK_ALLOC_OVERFLOW_SAFE_ARRAY(const char*, paths, 8192);
    if ( _mustBeMissingPaths != nullptr ) {
        _mustBeMissingPaths->forEachPath(^(const char* aPath) {
            paths.push_back(aPath);
        });
    }
	closureWriter.setMustBeMissingFiles(paths);

    // add list of files which must be be present with a specific inode/mtime
    if (!_skippedFiles.empty())
        closureWriter.setMustExistFiles(_skippedFiles);
}
void ClosureBuilder::invalidateInitializerRoots()
{
    while (true) {
        bool madeChange = false;
        for (uintptr_t loadedImageIndex = _alreadyInitedIndex; loadedImageIndex != _loadedImages.count(); ++loadedImageIndex) {
            BuilderLoadedImage& li = _loadedImages[loadedImageIndex];
            if ( li.mustBuildClosure ) {
                // Already invalidated
                continue;
            }
            for (Image::LinkedImage depIndex : li.dependents) {
                if ( depIndex.imageNum() == kMissingWeakLinkedImage )
                    continue;
                BuilderLoadedImage& depImage = findLoadedImage(depIndex.imageNum());
                // If a dependent is bad, or a new image num, or an override, then we need this image to get a new closure
                if ( depImage.mustBuildClosure ) {
                    li.mustBuildClosure = true;    // mark bad
                    madeChange = true;
                }
            }
        }
        if (!madeChange)
            break;
        // If we made a change, then we detected an existing image with a dependent which needed to be rebuilt.
        // This corresponds to a root of the shared cache where the existing image is a shared cache one and the root is the depImage
        _foundDyldCacheRoots = true;
    }
}

size_t ClosureBuilder::HashCString::hash(const char* v) {
    // FIXME: Use hash<string_view> when it has the correct visibility markup
    return __gnu_cxx::hash<const char*>{}(v);
}

bool ClosureBuilder::EqualCString::equal(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}


struct HashUInt64 {
    static size_t hash(const uint64_t& v) {
        return std::hash<uint64_t>{}(v);
    }
};

struct EqualUInt64 {
    static bool equal(uint64_t s1, uint64_t s2) {
        return s1 == s2;
    }
};

void ClosureBuilder::writeClassOrProtocolHashTable(bool classes, Array<ObjCOptimizerImage>& objcImages) {
    __block MultiMap<const char*, dyld3::closure::Image::ObjCClassImageOffset, HashCString, EqualCString> seenClassesMap;
    __block Map<const char*, dyld3::closure::Image::ObjCClassNameImageOffset, HashCString, EqualCString> classNameMap;
    __block OverflowSafeArray<const char*> classNames;

    // Note we walk the images backwards as we want them in load order to match the order they are registered with objc
    for (size_t imageIndex = 0, reverseIndex = (objcImages.count() - 1); imageIndex != objcImages.count(); ++imageIndex, --reverseIndex) {
        if (objcImages[reverseIndex].diag.hasError())
            continue;
        ObjCOptimizerImage& image = objcImages[reverseIndex];
        const OverflowSafeArray<ObjCOptimizerImage::SeenClass>& seenClasses = classes ? image.seenClasses : image.seenProtocols;

        for (const ObjCOptimizerImage::SeenClass& seenClass : seenClasses) {
            closure::Image::ObjCClassNameImageOffset    classNameTarget = seenClass.first;
            dyld3::closure::Image::ObjCClassImageOffset classDataTarget = seenClass.second;
            Image::ObjCClassImage                       classImage      = _objcClassesHashTableImages[classNameTarget.classNameImageIndex];

            const BuilderLoadedImage& li    = findLoadedImage(classImage.imageNum);
            const dyld3::MachOAnalyzer* ma  = li.loadAddress();

            const char* className   = ((const char*)ma) + classImage.offsetOfClassNames + classNameTarget.classNameImageOffset;
            //uint64_t nameVMAddr     = ma->preferredLoadAddress() + classImage.offsetOfClassNames + classNameTarget.classNameImageOffset;
            //printf("%s: 0x%08llx = '%s'\n", li.path(), nameVMAddr, className);
            seenClassesMap.insert({ className, classDataTarget });

            // Also track the name
            auto itAndInserted = classNameMap.insert({ className, dyld3::closure::Image::ObjCClassNameImageOffset() });
            if (itAndInserted.second) {
                // We inserted the class name so we need to add it to the strings for the closure hash table
                classNames.push_back(className);

                // We already computed a class name target in a previous loop so use that one
                itAndInserted.first->second = seenClass.first;

                // If we are processing protocols, and this is the first one we've seen, then track its ISA to be fixed up
                if ( !classes ) {
                    uint64_t protocolVMOffset = classImage.offsetOfClasses + classDataTarget.classData.imageOffset;
                    image.protocolISAFixups.push_back(protocolVMOffset);
                }
            }
        }
    }

    __block uint32_t duplicateCount = 0;
    seenClassesMap.forEachEntry(^(const char *const &key, const Image::ObjCClassImageOffset **values,
                                  uint64_t valuesCount) {
        if (valuesCount != 1)
            duplicateCount += valuesCount;
    });

    // If we have closure class names, we need to make a hash table for them.
    OverflowSafeArray<uint8_t>& hashTable = classes ? _objcClassesHashTable : _objcProtocolsHashTable;
    if (!classNames.empty()) {
        objc_opt::perfect_hash phash;
        objc_opt::make_perfect(classNames, phash);
        size_t size = ObjCClassOpt::size(phash, duplicateCount);
        hashTable.resize(size);
        //printf("Class table size: %lld\n", size);
        ObjCClassOpt* classesHashTable = (ObjCClassOpt*)hashTable.begin();
        classesHashTable->write(phash, classNameMap.array(), seenClassesMap, duplicateCount);
    }
}

bool ClosureBuilder::optimizeObjC(Array<ImageWriter>& writers) {
    if ( _dyldCache == nullptr )
        return false;

    // If we have the read only data, make sure it has a valid selector table inside.
    const objc_opt::objc_clsopt_t* objcClassOpt = nullptr;
    const objc_opt::objc_selopt_t* objcSelOpt = nullptr;
    const objc_opt::objc_protocolopt2_t* objcProtocolOpt = nullptr;
    if (const objc_opt::objc_opt_t* optObjCHeader = _dyldCache->objcOpt()) {
        objcClassOpt = optObjCHeader->clsopt();
        objcSelOpt = optObjCHeader->selopt();
        objcProtocolOpt = optObjCHeader->protocolopt2();
    }

    if ( !objcClassOpt || !objcSelOpt || !objcProtocolOpt )
        return false;

    // We have 24 bits of index in SelectorReferenceFixup so we can't handle a
    // shared cache selector table larger than that
    if ( objcSelOpt->usedCount() >= (1 << 24) )
        return false;

    // Make sure we have the pointers section with the pointer to the protocol class
    const void* objcOptPtrs = _dyldCache->objcOptPtrs();
    if ( objcOptPtrs == nullptr )
        return false;

    uint32_t pointerSize = _loadedImages.begin()->loadAddress()->pointerSize();
    uint64_t classProtocolVMAddr = (pointerSize == 8) ? *(uint64_t*)objcOptPtrs : *(uint32_t*)objcOptPtrs;

    Image::ResolvedSymbolTarget objcProtocolClassTarget;
    objcProtocolClassTarget.sharedCache.kind = Image::ResolvedSymbolTarget::kindSharedCache;
    if ( _dyldCacheIsLive ) {
        objcProtocolClassTarget.sharedCache.offset = classProtocolVMAddr - (uint64_t)_dyldCache;
    } else {
        objcProtocolClassTarget.sharedCache.offset = classProtocolVMAddr - _dyldCache->unslidLoadAddress();
    }

    STACK_ALLOC_OVERFLOW_SAFE_ARRAY(ObjCOptimizerImage, objcImages, 32);
    ArrayFinalizer<ObjCOptimizerImage> scopedCleanup(objcImages,
                                                     ^(ObjCOptimizerImage& objcImage) {
                                                         objcImage.~ObjCOptimizerImage();
                                                     });

    // Find all the images with valid objc info
    // Also add shared cache images to a map so that we can see them later for looking up classes
    Map<const dyld3::MachOAnalyzer*, bool, HashPointer, EqualPointer> sharedCacheImagesMap;
    for (size_t imageIndex = 0, writerIndex = 0; imageIndex != _loadedImages.count(); ++imageIndex) {
        BuilderLoadedImage& li = _loadedImages[imageIndex];

        // Skip shared cache images as even if they need a new closure, the objc runtime can still use
        // the optimized shared cache tables.
        if ( li.loadAddress()->inDyldCache() ) {
            sharedCacheImagesMap.insert({ li.loadAddress(), true });
            // Bump the writer index if we have a writer for this image
            if ( li.mustBuildClosure )
                ++writerIndex;
            continue;
        }
        // Images which don't need a closure can be skipped.  They are from the shared cache
        if ( !li.mustBuildClosure )
            continue;

        // If we have a root of libobjc, just give up for now
        if ( !strcmp(li.path(), "/usr/lib/libobjc.A.dylib"))
            return false;

        ImageWriter& writer = writers[writerIndex];
        ++writerIndex;

        const dyld3::MachOAnalyzer* ma = li.loadAddress();

        // Skip images with chained fixups other than arm64e legacy fixups until we can test them
        // FIXME: Handle chained fixups
        if ( ma->hasChainedFixups() ) {
            switch ( ma->chainedPointerFormat() ) {
                case DYLD_CHAINED_PTR_ARM64E:
                case DYLD_CHAINED_PTR_64:
                    // We've tested the 64-bit chained fixups.
                    break;
                case DYLD_CHAINED_PTR_32:
                case DYLD_CHAINED_PTR_32_CACHE:
                case DYLD_CHAINED_PTR_32_FIRMWARE:
                    // FIXME: Test 32-bit chained fixups then enable this.
                    continue;
            }
        }

        const MachOAnalyzer::ObjCImageInfo* objcImageInfo = ma->objcImageInfo();
        if ( objcImageInfo == nullptr )
            continue;

        // This image is good so record it for use later.
        objcImages.default_constuct_back();
        ObjCOptimizerImage& image = objcImages.back();
        image.loadedImage   = &li;
        image.writer        = &writer;

        // Find FairPlay encryption range if encrypted
        uint32_t fairPlayFileOffset;
        uint32_t fairPlaySize;
        if ( ma->isFairPlayEncrypted(fairPlayFileOffset, fairPlaySize) ) {
            image.fairplayFileOffsetStart   = fairPlayFileOffset;
            image.fairplayFileOffsetEnd     = fairPlayFileOffset;
        }

        // Set the offset to the objc image info
        image.objcImageInfoVMOffset = (uint64_t)objcImageInfo - (uint64_t)ma;
    }

    OverflowSafeArray<const char*>                                                          closureSelectorStrings;
    Map<const char*, dyld3::closure::Image::ObjCImageOffset, HashCString, EqualCString>     closureSelectorMap;
    OverflowSafeArray<const char*>                                                          closureDuplicateSharedCacheClassNames;
    Map<const char*, dyld3::closure::Image::ObjCDuplicateClass, HashCString, EqualCString>  closureDuplicateSharedCacheClassMap;
    for (ObjCOptimizerImage& image : objcImages) {
        optimizeObjCClasses(objcClassOpt, sharedCacheImagesMap, closureDuplicateSharedCacheClassMap, image);
        if (image.diag.hasError())
            continue;

        optimizeObjCProtocols(objcProtocolOpt, sharedCacheImagesMap, image);
        if (image.diag.hasError())
            continue;

        optimizeObjCSelectors(objcSelOpt, closureSelectorMap, image);
        if (image.diag.hasError())
            continue;

        // If this image is still valid, then add its intermediate results to the main tables

        // Class results
        for (auto nameAndDataVMOffset : image.classesNameAndDataVMOffsets) {
            uint64_t nameVMOffset = nameAndDataVMOffset.first;
            uint64_t dataVMOffset = nameAndDataVMOffset.second;
            _objcClassesHashTableImages.push_back({ image.loadedImage->imageNum, (uint32_t)nameVMOffset, (uint32_t)dataVMOffset });
        }
        image.classesNameAndDataVMOffsets.clear();

        for (const auto& stringAndDuplicate : image.classSharedCacheDuplicates) {
            closureDuplicateSharedCacheClassMap[stringAndDuplicate.first] = stringAndDuplicate.second;
            closureDuplicateSharedCacheClassNames.push_back(stringAndDuplicate.first);
        }

        // Selector results
        // Note we don't need to add the selector binds here.  Its easier just to process them later from each image
        for (const auto& stringAndTarget : image.selectorMap) {
            closureSelectorMap[stringAndTarget.first] = stringAndTarget.second;
            closureSelectorStrings.push_back(stringAndTarget.first);
        }
        if (image.methodNameVMOffset)
            _objcSelectorsHashTableImages.push_back({ image.loadedImage->imageNum, (uint32_t)*image.methodNameVMOffset });
    }

    // If we successfully analyzed the classes and selectors, we can now emit their data
    // Set all the writers to have optimized objc
    for (ObjCOptimizerImage& image : objcImages) {
        if (image.diag.hasError())
            continue;
        image.writer->setHasPrecomputedObjC(true);
    }

    // Write out the class table
    writeClassOrProtocolHashTable(true, objcImages);

    // Write out the protocol table
    writeClassOrProtocolHashTable(false, objcImages);

    // If we have closure duplicate classes, we need to make a hash table for them.
    closure::ObjCStringTable* duplicateClassesTable = nullptr;
    if (!closureDuplicateSharedCacheClassNames.empty()) {
        objc_opt::perfect_hash phash;
        objc_opt::make_perfect(closureDuplicateSharedCacheClassNames, phash);
        size_t size = ObjCStringTable::size(phash);
        _objcClassesDuplicatesHashTable.resize(size);
        //printf("Duplicate classes table size: %lld\n", size);
        duplicateClassesTable = (closure::ObjCClassDuplicatesOpt*)_objcClassesDuplicatesHashTable.begin();
        duplicateClassesTable->write(phash, closureDuplicateSharedCacheClassMap.array());
    }

    // If we have closure selectors, we need to make a hash table for them.
    closure::ObjCStringTable* selectorStringTable = nullptr;
    if (!closureSelectorStrings.empty()) {
        objc_opt::perfect_hash phash;
        objc_opt::make_perfect(closureSelectorStrings, phash);
        size_t size = ObjCStringTable::size(phash);
        _objcSelectorsHashTable.resize(size);
        //printf("Selector table size: %lld\n", size);
        selectorStringTable = (closure::ObjCStringTable*)_objcSelectorsHashTable.begin();
        selectorStringTable->write(phash, closureSelectorMap.array());
    }

    // Add fixups for the image info, protocol ISAs, and selector refs
    for (ObjCOptimizerImage& image : objcImages) {
        if (image.diag.hasError())
            continue;

        // Protocol ISA references
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::ProtocolISAFixup, protocolFixups, 512);
        if ( !image.protocolISAFixups.empty() ) {

            __block uint64_t lastOffset = -pointerSize;
            for (uint64_t runtimeOffset : image.protocolISAFixups) {
                bool mergedIntoPrevious = false;
                if ( (runtimeOffset > lastOffset) && !protocolFixups.empty() ) {
                    uint64_t skipAmount = (runtimeOffset - lastOffset - pointerSize)/pointerSize;
                    if ( skipAmount*pointerSize != (runtimeOffset - lastOffset - pointerSize) ) {
                        // misaligned pointer means we cannot optimize
                    }
                    else {
                        if ( (protocolFixups.back().repeatCount == 1) && (protocolFixups.back().skipCount == 0) && (skipAmount <= 255) ) {
                            protocolFixups.back().repeatCount = 2;
                            protocolFixups.back().skipCount   = skipAmount;
                            assert(protocolFixups.back().skipCount == skipAmount); // check overflow
                            mergedIntoPrevious       = true;
                        }
                        else if ( (protocolFixups.back().skipCount == skipAmount) && (protocolFixups.back().repeatCount < 0xfff) ) {
                            uint32_t prevRepeatCount = protocolFixups.back().repeatCount;
                            protocolFixups.back().repeatCount += 1;
                            assert(protocolFixups.back().repeatCount > prevRepeatCount); // check overflow
                            mergedIntoPrevious       = true;
                        }
                    }
                }
                if ( !mergedIntoPrevious ) {
                    Image::ProtocolISAFixup pattern;
                    pattern.startVmOffset = runtimeOffset;
                    pattern.repeatCount   = 1;
                    pattern.skipCount     = 0;
                    assert(pattern.startVmOffset == runtimeOffset);
                    protocolFixups.push_back(pattern);
                }
                lastOffset = runtimeOffset;
            }
        }

        // Selector references
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::SelectorReferenceFixup, selRefFixups, 512);
        if ( !image.selectorFixups.empty() ) {
            uint64_t prevVMOffset = 0;
            const uint64_t maxChainOffset = (4 * ((1 << 7) - 1));
            for (const ObjCOptimizerImage::SelectorFixup& selectorFixup : image.selectorFixups) {
                assert( (selectorFixup.fixupVMOffset & 3) == 0 );
                if ( (selectorFixup.fixupVMOffset - prevVMOffset) <= maxChainOffset ) {
                    // Add this to the previous chain
                    selRefFixups.back().chainEntry.next = (uint32_t)(selectorFixup.fixupVMOffset - prevVMOffset) / 4;
                } else {
                    // Need to start a new chain as the previous offset can't reach
                    Image::SelectorReferenceFixup fixup;
                    fixup.chainStartVMOffset = selectorFixup.fixupVMOffset;
                    selRefFixups.push_back(fixup);
                }

                if ( selectorFixup.isSharedCache ) {
                    // If the entry is in the shared cache then we already have the index for it
                    Image::SelectorReferenceFixup fixup;
                    fixup.chainEntry.index          = selectorFixup.sharedCache.selectorTableIndex;
                    fixup.chainEntry.next           = 0;
                    fixup.chainEntry.inSharedCache  = 1;
                    selRefFixups.push_back(fixup);
                } else {
                    // We had to record the string for the closure table entries as we don't know the
                    // index until now
                    uint32_t selectorTableIndex = selectorStringTable->getIndex(selectorFixup.image.selectorString);
                    assert(selectorTableIndex != ObjCSelectorOpt::indexNotFound);
                    Image::SelectorReferenceFixup fixup;
                    fixup.chainEntry.index          = selectorTableIndex;
                    fixup.chainEntry.next           = 0;
                    fixup.chainEntry.inSharedCache  = 0;
                    selRefFixups.push_back(fixup);
                }

                prevVMOffset = selectorFixup.fixupVMOffset;
            }
        }

        // Stable Swift fixups
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::ClassStableSwiftFixup, stableSwiftFixups, 512);
        if ( !image.classStableSwiftFixups.empty() ) {

            __block uint64_t lastOffset = -pointerSize;
            for (uint64_t runtimeOffset : image.classStableSwiftFixups) {
                bool mergedIntoPrevious = false;
                if ( (runtimeOffset > lastOffset) && !stableSwiftFixups.empty() ) {
                    uint64_t skipAmount = (runtimeOffset - lastOffset - pointerSize)/pointerSize;
                    if ( skipAmount*pointerSize != (runtimeOffset - lastOffset - pointerSize) ) {
                        // misaligned pointer means we cannot optimize
                    }
                    else {
                        if ( (stableSwiftFixups.back().repeatCount == 1) && (stableSwiftFixups.back().skipCount == 0) && (skipAmount <= 255) ) {
                            stableSwiftFixups.back().repeatCount = 2;
                            stableSwiftFixups.back().skipCount   = skipAmount;
                            assert(stableSwiftFixups.back().skipCount == skipAmount); // check overflow
                            mergedIntoPrevious       = true;
                        }
                        else if ( (stableSwiftFixups.back().skipCount == skipAmount) && (stableSwiftFixups.back().repeatCount < 0xfff) ) {
                            uint32_t prevRepeatCount = stableSwiftFixups.back().repeatCount;
                            stableSwiftFixups.back().repeatCount += 1;
                            assert(stableSwiftFixups.back().repeatCount > prevRepeatCount); // check overflow
                            mergedIntoPrevious       = true;
                        }
                    }
                }
                if ( !mergedIntoPrevious ) {
                    Image::ClassStableSwiftFixup pattern;
                    pattern.startVmOffset = runtimeOffset;
                    pattern.repeatCount   = 1;
                    pattern.skipCount     = 0;
                    assert(pattern.startVmOffset == runtimeOffset);
                    stableSwiftFixups.push_back(pattern);
                }
                lastOffset = runtimeOffset;
            }
        }

        // Method list fixups
        // TODO: Implement this
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(Image::MethodListFixup, methodListFixups, 512);

        image.writer->setObjCFixupInfo(objcProtocolClassTarget, image.objcImageInfoVMOffset, protocolFixups,
                                       selRefFixups, stableSwiftFixups, methodListFixups);
    }

    return true;
}

void ClosureBuilder::optimizeObjCSelectors(const objc_opt::objc_selopt_t* objcSelOpt,
                                           const Map<const char*, dyld3::closure::Image::ObjCImageOffset, HashCString, EqualCString>& closureSelectorMap,
                                           ObjCOptimizerImage& image) {

    BuilderLoadedImage& li = *image.loadedImage;

    const dyld3::MachOAnalyzer* ma = li.loadAddress();
    uint32_t pointerSize = ma->pointerSize();
    const uint64_t loadAddress = ma->preferredLoadAddress();

    // The legacy (objc1) codebase uses a bunch of sections we don't want to reason about.  If we see them just give up.
    __block bool foundBadSection = false;
    ma->forEachSection(^(const MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stop) {
        if ( strcmp(sectInfo.segInfo.segName, "__OBJC") != 0 )
            return;
        if (strcmp(sectInfo.sectName, "__module_info") == 0) {
            foundBadSection = true;
            stop = true;
            return;
        }
        if (strcmp(sectInfo.sectName, "__protocol") == 0) {
            foundBadSection = true;
            stop = true;
            return;
        }
        if (strcmp(sectInfo.sectName, "__message_refs") == 0) {
            foundBadSection = true;
            stop = true;
            return;
        }
    });
    if (foundBadSection) {
        image.diag.error("Old objc section");
        return;
    }

    __block MachOAnalyzer::SectionCache selectorStringSectionCache(ma);

    uint32_t sharedCacheSentinelIndex = objcSelOpt->getSentinelIndex();

    auto visitReferenceToObjCSelector = ^void(uint64_t selectorStringVMAddr, uint64_t selectorReferenceVMAddr) {

        uint64_t selectorUseImageOffset = selectorReferenceVMAddr - loadAddress;
        if ( (selectorUseImageOffset & 3) != 0 ) {
            image.diag.error("Unaligned selector reference fixup");
            return;
        }

        // Image::SelectorReferenceFixup only has a 32-bit reach
        if ( selectorUseImageOffset >= (1ULL << 32) ) {
            image.diag.error("Selector reference fixup exceeds supported vm offset");
            return;
        }

        // Get the section for the name
        const char* selectorString = nullptr;
        MachOAnalyzer::PrintableStringResult selectorStringResult = MachOAnalyzer::PrintableStringResult::UnknownSection;
        __block uint64_t selectorStringSectionStartVMAddr = 0;
        auto selectorStringSectionHandler = ^bool(const MachOAnalyzer::SectionInfo& sectInfo) {

            // We only have 24-bits in ObjCClassNameImageOffset to index in to the strings
            if (sectInfo.sectSize >= Image::ObjCImageOffset::maximumOffset) {
                return false;
            }

            // We use 32-bit offsets so make sure the section is no larger than that.
            uint64_t classNameVMOffset = sectInfo.sectAddr - loadAddress;
            if (classNameVMOffset >= (1ULL << 32)) {
                return false;
            }

            selectorStringSectionStartVMAddr = sectInfo.sectAddr;
            return true;
        };
        selectorString = ma->getPrintableString(selectorStringVMAddr, selectorStringResult,
                                                &selectorStringSectionCache, selectorStringSectionHandler);

        if ( selectorStringResult != MachOAnalyzer::PrintableStringResult::CanPrint ) {
            image.diag.error("Invalid selector string for objc optimisation");
            return;
        }

        uint32_t cacheSelectorIndex = objcSelOpt->getIndexForKey(selectorString);
        //printf("selector: %p -> %p %s\n", methodName, cacheSelector, selectorString);

        if ( cacheSelectorIndex != sharedCacheSentinelIndex ) {
            // We got the selector from the cache so add a fixup to point there.
            ObjCOptimizerImage::SelectorFixup fixup;
            fixup.isSharedCache                     = true;
            fixup.fixupVMOffset                     = (uint32_t)selectorUseImageOffset;
            fixup.sharedCache.selectorTableIndex    = cacheSelectorIndex;

            //printf("Overriding fixup at 0x%08llX to cache offset 0x%08llX\n", selectorUseImageOffset, (uint64_t)cacheSelector - (uint64_t)_dyldCache);
            image.selectorFixups.push_back(fixup);
            return;
        }

        // See if this selector is already in the closure map from a previous image
        auto closureSelectorIt = closureSelectorMap.find(selectorString);
        if (closureSelectorIt != closureSelectorMap.end()) {
            // This selector was found in a previous image, so use it here.
            ObjCOptimizerImage::SelectorFixup fixup;
            fixup.isSharedCache         = false;
            fixup.fixupVMOffset         = (uint32_t)selectorUseImageOffset;
            fixup.image.selectorString  = selectorString;

            //printf("Overriding fixup at 0x%08llX to '%s' offset 0x%08llX\n", selectorUseImageOffset, findLoadedImage(target.image.imageNum).path(), target.image.offset);
            image.selectorFixups.push_back(fixup);
            return;
        }

        // See if this selector is already in the map for this image
        auto itAndInserted = image.selectorMap.insert({ selectorString, dyld3::closure::Image::ObjCImageOffset() });
        if (itAndInserted.second) {
            // We added the selector so its pointing in to our own image.
            // We don't need to add a fixup to our image, but we do need to
            // populate the data for other images later to point here.
            // First put our image in the list if its not already there.
            uint64_t methodNameVMOffset = selectorStringSectionStartVMAddr - loadAddress;
            if (!image.methodNameVMOffset) {
                if ( _objcSelectorsHashTableImages.count() == Image::ObjCImageOffset::maximumImageIndex ) {
                    image.diag.error("Out of space for selector hash images");
                    return;
                }
                image.methodNameVMOffset = methodNameVMOffset;
            } else {
                // If we already set the offset to the start of the method names section, double check that
                // the section we are in right now is the same as that one.  Otherwise we don't have the code
                // to handle both right now.
                if (*image.methodNameVMOffset != methodNameVMOffset) {
                    image.diag.error("Cannot handle more than one selector strings section");
                    return;
                }
            }

            dyld3::closure::Image::ObjCImageOffset target;
            target.imageIndex   = (uint32_t)_objcSelectorsHashTableImages.count();
            target.imageOffset  = (uint32_t)(selectorStringVMAddr - selectorStringSectionStartVMAddr);
            itAndInserted.first->second = target;
            return;
        }

        // This selector was found elsewhere in our image.  If this reference already points to the same
        // selector string as we found before (and it should!) then we have nothing to do.  Otherwise we
        // need to add a fixup here to make sure we point to our chosen definition.
        uint32_t imageOffset = (uint32_t)(selectorStringVMAddr - loadAddress);
        if ( imageOffset == (*image.methodNameVMOffset + itAndInserted.first->second.imageOffset) )
            return;

        ObjCOptimizerImage::SelectorFixup fixup;
        fixup.isSharedCache         = false;
        fixup.fixupVMOffset         = (uint32_t)selectorUseImageOffset;
        fixup.image.selectorString  = selectorString;

        //printf("Overriding fixup at 0x%08llX to '%s' offset 0x%08llX\n", selectorUseImageOffset, findLoadedImage(target.image.imageNum).path(), target.image.offset);
        image.selectorFixups.push_back(fixup);
    };

    auto visitMethod = ^(uint64_t methodVMAddr, const dyld3::MachOAnalyzer::ObjCMethod& method) {
        visitReferenceToObjCSelector(method.nameVMAddr, method.nameLocationVMAddr);
    };

    auto visitClass = ^(Diagnostics& diag, uint64_t classVMAddr,
                        uint64_t classSuperclassVMAddr, uint64_t classDataVMAddr,
                        const dyld3::MachOAnalyzer::ObjCClassInfo& objcClass, bool isMetaClass) {
        ma->forEachObjCMethod(objcClass.baseMethodsVMAddr(pointerSize), li.contentRebased,
                              visitMethod);
    };

    auto visitCategory = ^(Diagnostics& diag, uint64_t categoryVMAddr,
                           const dyld3::MachOAnalyzer::ObjCCategory& objcCategory) {
        ma->forEachObjCMethod(objcCategory.instanceMethodsVMAddr, li.contentRebased,
                              visitMethod);
        ma->forEachObjCMethod(objcCategory.classMethodsVMAddr, li.contentRebased,
                              visitMethod);
    };
    auto visitProtocol = ^(Diagnostics& diag, uint64_t protocolVMAddr,
                           const dyld3::MachOAnalyzer::ObjCProtocol& objCProtocol) {
        ma->forEachObjCMethod(objCProtocol.instanceMethodsVMAddr, li.contentRebased,
                              visitMethod);
        ma->forEachObjCMethod(objCProtocol.classMethodsVMAddr, li.contentRebased,
                              visitMethod);
        ma->forEachObjCMethod(objCProtocol.optionalInstanceMethodsVMAddr, li.contentRebased,
                              visitMethod);
        ma->forEachObjCMethod(objCProtocol.optionalClassMethodsVMAddr, li.contentRebased,
                              visitMethod);
    };

    // Walk the class list
    ma->forEachObjCClass(image.diag, li.contentRebased, visitClass);
    if (image.diag.hasError())
        return;

    // Walk the category list
    ma->forEachObjCCategory(image.diag, li.contentRebased, visitCategory);
    if (image.diag.hasError())
        return;

    // Walk the protocol list
    ma->forEachObjCProtocol(image.diag, li.contentRebased, visitProtocol);
    if (image.diag.hasError())
        return;

    // Visit the selector refs
    ma->forEachObjCSelectorReference(image.diag, li.contentRebased, ^(uint64_t selRefVMAddr, uint64_t selRefTargetVMAddr) {
        visitReferenceToObjCSelector(selRefTargetVMAddr, selRefVMAddr);
    });
    if (image.diag.hasError())
        return;

    // Visit the message refs
    // Note this isn't actually supported in libobjc any more.  Its logic for deciding whether to support it is if this is true:
    // #if (defined(__x86_64__) && (TARGET_OS_OSX || TARGET_OS_SIMULATOR))
    // So to keep it simple, lets only do this walk if we are x86_64
    if ( ma->isArch("x86_64") || ma->isArch("x86_64h") ) {
        if (ma->hasObjCMessageReferences()) {
            image.diag.error("Cannot handle message refs");
            return;
        }
    }
}

static const dyld3::MachOAnalyzer* getMachHeaderFromObjCHeaderInfo(const void* opaqueHeaderInfo, uint32_t pointerSize) {
    if (pointerSize == 8) {
        typedef int64_t PtrTy;
        struct HeaderInfo {
            PtrTy mhdr_offset;     // offset to mach_header_64
            PtrTy info_offset;     // offset to objc_image_info *
        };
        const HeaderInfo* headerInfo = (const HeaderInfo*)opaqueHeaderInfo;
        return (const dyld3::MachOAnalyzer*)(((const uint8_t*)&headerInfo->mhdr_offset) + headerInfo->mhdr_offset);
    } else {
        typedef int32_t PtrTy;
        struct HeaderInfo {
            PtrTy mhdr_offset;     // offset to mach_header
            PtrTy info_offset;     // offset to objc_image_info *
        };
        const HeaderInfo* headerInfo = (const HeaderInfo*)opaqueHeaderInfo;
        return (const dyld3::MachOAnalyzer*)(((const uint8_t*)&headerInfo->mhdr_offset) + headerInfo->mhdr_offset);
    }
}

void ClosureBuilder::addDuplicateObjCClassWarning(const char* className,
                                                  const char* duplicateDefinitionPath,
                                                  const char* canonicalDefinitionPath)
{
    if ( _objcDuplicateClassWarnings == nullptr )
        _objcDuplicateClassWarnings = PathPool::allocate();
    // Use a diagnostic to give us a buffer we can safely print to
    Diagnostics diag;
    diag.error("Class %s is implemented in both %s and %s. One of the two will be used. Which one is undefined.",
               className, canonicalDefinitionPath, duplicateDefinitionPath);
#if BUILDING_CACHE_BUILDER
    _objcDuplicateClassWarnings->add(diag.errorMessage().c_str());
#else
    _objcDuplicateClassWarnings->add(diag.errorMessage());
#endif
}

void ClosureBuilder::optimizeObjCClasses(const objc_opt::objc_clsopt_t* objcClassOpt,
                                         const Map<const dyld3::MachOAnalyzer*, bool, HashPointer, EqualPointer>& sharedCacheImagesMap,
                                         const Map<const char*, dyld3::closure::Image::ObjCDuplicateClass, HashCString, EqualCString>& duplicateSharedCacheClasses,
                                         ObjCOptimizerImage& image) {

    BuilderLoadedImage& li = *image.loadedImage;
    OverflowSafeArray<ObjCOptimizerImage::SeenClass>& seenClasses = image.seenClasses;

    const dyld3::MachOAnalyzer* ma = li.loadAddress();
    const uint32_t pointerSize = ma->pointerSize();
    const uint64_t loadAddress = ma->preferredLoadAddress();

    // Keep track of any missing weak imports so that we can tell if the superclasses are nil
    // This is necessary as the shared cache will be marked with 'no missing weak superclasses'
    // and so we need to continue to satisfy that constraint
    __block Map<uint64_t, bool, HashUInt64, EqualUInt64> missingWeakImportOffets;
    if (li.hasMissingWeakImports) {
        if (ma->hasChainedFixups()) {
            const Image* closureImage = image.writer->currentImage();

            const Array<Image::ResolvedSymbolTarget> targets = closureImage->chainedTargets();
            if ( !targets.empty() ) {
                ma->withChainStarts(_diag, closureImage->chainedStartsOffset(), ^(const dyld_chained_starts_in_image* startsInfo) {
                    ma->forEachFixupInAllChains(_diag, startsInfo, false, ^(MachOLoaded::ChainedFixupPointerOnDisk* fixupLoc,
                                                                            const dyld_chained_starts_in_segment* segInfo, bool& fixupsStop) {
                        uint64_t fixupOffset = (uint8_t*)fixupLoc - (uint8_t*)ma;
                        uint32_t bindOrdinal;
                        if ( fixupLoc->isBind(segInfo->pointer_format, bindOrdinal) ) {
                            if ( bindOrdinal < targets.count() ) {
                                const Image::ResolvedSymbolTarget& target = targets[bindOrdinal];
                                if ( (target.absolute.kind == Image::ResolvedSymbolTarget::kindAbsolute) && (target.absolute.value == 0) )
                                    missingWeakImportOffets[fixupOffset] = true;
                            }
                            else {
                                image.diag.error("out of range bind ordinal %d (max %lu)", bindOrdinal, targets.count());
                                fixupsStop = true;
                            }
                        }
                    });
                });
                if (image.diag.hasError())
                    return;
            }
        } else {
            forEachBind(li, ^(uint64_t runtimeOffset, Image::ResolvedSymbolTarget target, const ResolvedTargetInfo& targetInfo, bool& stop) {
                if ( (target.absolute.kind == Image::ResolvedSymbolTarget::kindAbsolute) && (target.absolute.value == 0) )
                    missingWeakImportOffets[runtimeOffset] = true;
            }, ^(const char *strongSymbolName) {
            }, ^() { });
        }
    }

    // Class names and data may be in different sections depending on swift vs objc so handle multiple sections
    __block MachOAnalyzer::SectionCache classNameSectionCache(ma);
    __block MachOAnalyzer::SectionCache classSectionCache(ma);

    ma->forEachObjCClass(image.diag, li.contentRebased, ^(Diagnostics &diag, uint64_t classVMAddr,
                                                          uint64_t classSuperclassVMAddr, uint64_t classDataVMAddr,
                                                          const MachOAnalyzer::ObjCClassInfo &objcClass, bool isMetaClass) {
        if (isMetaClass) return;

        // Make sure the superclass pointer is not nil
        uint64_t superclassRuntimeOffset = classSuperclassVMAddr - loadAddress;
        if (missingWeakImportOffets.find(superclassRuntimeOffset) != missingWeakImportOffets.end()) {
            diag.error("Missing weak superclass");
            return;
        }

        // Does this class need to be fixed up for stable Swift ABI.
        // Note the order matches the objc runtime in that we always do this fix before checking for dupes,
        // but after excluding classes with missing weak superclasses.
        if (objcClass.isUnfixedBackwardDeployingStableSwift()) {
            // Class really is stable Swift, pretending to be pre-stable.
            // Fix its lie.  This involves fixing the FAST bits on the class data value, so record that vmaddr
            image.classStableSwiftFixups.push_back(classDataVMAddr - loadAddress);
        }

        // Get the section for the name
        const char* className = nullptr;
        MachOAnalyzer::PrintableStringResult classNameResult = MachOAnalyzer::PrintableStringResult::UnknownSection;
        __block uint64_t classNameSectionStartVMAddr = 0;
        auto classNameSectionHandler = ^bool(const MachOAnalyzer::SectionInfo& sectInfo) {
            // We only have 24-bits in ObjCClassNameImageOffset to index in to the strings
            if (sectInfo.sectSize >= Image::ObjCClassNameImageOffset::maximumOffset) {
                return false;
            }

            // We use 32-bit offsets so make sure the section is no larger than that.
            uint64_t classNameVMOffset = sectInfo.sectAddr - loadAddress;
            if (classNameVMOffset >= (1ULL << 32)) {
                return false;
            }

            classNameSectionStartVMAddr = sectInfo.sectAddr;
            return true;
        };
        uint64_t classNameVMAddr = objcClass.nameVMAddr(pointerSize);
        className = ma->getPrintableString(classNameVMAddr, classNameResult,
                                           &classNameSectionCache, classNameSectionHandler);

        if ( classNameResult != MachOAnalyzer::PrintableStringResult::CanPrint ) {
            diag.error("Invalid class name for objc optimisation");
            return;
        }

        // If the class also exists in a shared cache image which is loaded, then objc
        // would have found that one, regardless of load order.  So we can just skip this one.
        {
            void *cls;
            void *hi;
            uint32_t index;
            uint32_t count = objcClassOpt->getClassHeaderAndIndex(className, cls, hi, index);
            if (count == 1) {
                // exactly one matching class.  Check if its loaded
                const dyld3::MachOAnalyzer* sharedCacheMA = getMachHeaderFromObjCHeaderInfo(hi, pointerSize);
                if (sharedCacheImagesMap.find(sharedCacheMA) != sharedCacheImagesMap.end()) {
                    addDuplicateObjCClassWarning(className, li.path(), sharedCacheMA->installName());

                    // We have a duplicate class, so check if we've already got it in our map.
                    if ( duplicateSharedCacheClasses.find(className) == duplicateSharedCacheClasses.end() ) {
                        // We haven't seen this one yet
                        Image::ObjCDuplicateClass duplicateClass;
                        duplicateClass.sharedCacheClassOptIndex         = index;
                        duplicateClass.sharedCacheClassDuplicateIndex   = 0;
                        image.classSharedCacheDuplicates.insert({ className, duplicateClass });
                    }
                }
            }
            else if (count > 1) {
                // more than one matching class - find one that is loaded
                void *clslist[count];
                void *hilist[count];
                objcClassOpt->getClassesAndHeaders(className, clslist, hilist);
                for (uint32_t i = 0; i < count; i++) {
                    const dyld3::MachOAnalyzer* sharedCacheMA = getMachHeaderFromObjCHeaderInfo(hilist[i], pointerSize);
                    if (sharedCacheImagesMap.find(sharedCacheMA) != sharedCacheImagesMap.end()) {
                        addDuplicateObjCClassWarning(className, li.path(), sharedCacheMA->installName());

                        // We have a duplicate class, so check if we've already got it in our map.
                        if ( duplicateSharedCacheClasses.find(className) == duplicateSharedCacheClasses.end() ) {
                            // We haven't seen this one yet
                            Image::ObjCDuplicateClass duplicateClass;
                            duplicateClass.sharedCacheClassOptIndex         = index;
                            duplicateClass.sharedCacheClassDuplicateIndex   = i;
                            image.classSharedCacheDuplicates.insert({ className, duplicateClass });
                        }

                        break;
                    }
                }
            }
        }

        // Get the section for the class itself
        __block uint64_t classSectionStartVMAddr = 0;
        auto classSectionHandler = ^bool(const MachOAnalyzer::SectionInfo& sectInfo) {
            // We only have 23-bits in ObjCClassImageOffset to index in to the classes
            if (sectInfo.sectSize > Image::ObjCClassImageOffset::maximumOffset) {
                return false;
            }

            // We use 32-bit offsets so make sure the section is no larger than that.
            uint64_t classDatasVMOffset = sectInfo.sectAddr - loadAddress;
            if (classDatasVMOffset >= (1ULL << 32)) {
                return false;
            }

            classSectionStartVMAddr = sectInfo.sectAddr;
            return true;
        };
        if (!classSectionCache.findSectionForVMAddr(classVMAddr, classSectionHandler)) {
            diag.error("Invalid class for objc optimisation");
            return;
        }

        // Make sure we have an entry for our images offsets for later
        uint64_t classNameSectionVMOffset = classNameSectionStartVMAddr - loadAddress;
        uint64_t classSectionVMOffset = classSectionStartVMAddr - loadAddress;
        uint64_t hashTableVMOffsetsIndex = 0;
        for (auto nameAndDataVMOffset : image.classesNameAndDataVMOffsets) {
            if ( (nameAndDataVMOffset.first == classNameSectionVMOffset) && (nameAndDataVMOffset.second == classSectionVMOffset) )
                break;
            ++hashTableVMOffsetsIndex;
        }

        if (hashTableVMOffsetsIndex == image.classesNameAndDataVMOffsets.count()) {
            // Didn't find an image entry with this offset.  Add one if we have space
            uint64_t totalHashTableImages = image.classesNameAndDataVMOffsets.count() + _objcClassesHashTableImages.count();
            if ( totalHashTableImages == Image::ObjCClassNameImageOffset::maximumImageIndex ) {
                // No more space.  We need to give up
                diag.error("No more space for class hash table image");
                return;
            }
            image.classesNameAndDataVMOffsets.push_back({ classNameSectionVMOffset, classSectionVMOffset });
        }

        hashTableVMOffsetsIndex += _objcClassesHashTableImages.count();

        uint64_t classNameOffset = classNameVMAddr - classNameSectionStartVMAddr;
        uint64_t classDataOffset = classVMAddr - classSectionStartVMAddr;

        closure::Image::ObjCClassNameImageOffset classNameTarget;
        classNameTarget.classNameImageIndex   = (uint32_t)hashTableVMOffsetsIndex;
        classNameTarget.classNameImageOffset  = (uint32_t)classNameOffset;

        dyld3::closure::Image::ObjCClassImageOffset classDataTarget;
        classDataTarget.classData.imageIndex   = (uint32_t)hashTableVMOffsetsIndex;
        classDataTarget.classData.imageOffset  = (uint32_t)classDataOffset;
        classDataTarget.classData.isDuplicate  = 0;

        seenClasses.push_back({ classNameTarget, classDataTarget });
    });
}

void ClosureBuilder::optimizeObjCProtocols(const objc_opt::objc_protocolopt2_t* objcProtocolOpt,
                                           const Map<const dyld3::MachOAnalyzer*, bool, HashPointer, EqualPointer>& sharedCacheImagesMap,
                                           ObjCOptimizerImage& image) {

    BuilderLoadedImage& li = *image.loadedImage;
    OverflowSafeArray<ObjCOptimizerImage::SeenClass>& seenProtocols = image.seenProtocols;

    const dyld3::MachOAnalyzer* ma = li.loadAddress();
    const uint32_t pointerSize = ma->pointerSize();
    const uint64_t loadAddress = ma->preferredLoadAddress();

    // Protocol names and data may be in different sections depending on swift vs objc so handle multiple sections
    __block MachOAnalyzer::SectionCache protocolNameSectionCache(ma);
    __block MachOAnalyzer::SectionCache protocolSectionCache(ma);

    ma->forEachObjCProtocol(image.diag, li.contentRebased, ^(Diagnostics &diag, uint64_t protocolVMAddr,
                                                             const dyld3::MachOAnalyzer::ObjCProtocol &objCProtocol) {
        if ( objCProtocol.requiresObjCReallocation ) {
            // We can't optimize this protocol as the runtime needs all fields to be present
            diag.error("Protocol is too small to be optimized");
            return;
        }
        if ( objCProtocol.isaVMAddr != 0 ) {
            // We can't optimize this protocol if it has an ISA as we want to override it
            diag.error("Protocol ISA cannot be non-zero");
            return;
        }

        // Get the section for the name
        const char* protocolName = nullptr;
        MachOAnalyzer::PrintableStringResult protocolNameResult = MachOAnalyzer::PrintableStringResult::UnknownSection;
        __block uint64_t protocolNameSectionStartVMAddr = 0;
        auto protocolNameSectionHandler = ^bool(const MachOAnalyzer::SectionInfo& sectInfo) {
            // We only have 24-bits in ObjCClassNameImageOffset to index in to the strings
            if (sectInfo.sectSize >= Image::ObjCClassNameImageOffset::maximumOffset) {
                return false;
            }

            // We use 32-bit offsets so make sure the section is no larger than that.
            uint64_t protocolNameVMOffset = sectInfo.sectAddr - loadAddress;
            if (protocolNameVMOffset >= (1ULL << 32)) {
                return false;
            }

            protocolNameSectionStartVMAddr = sectInfo.sectAddr;
            return true;
        };
        uint64_t protocolNameVMAddr = objCProtocol.nameVMAddr;
        protocolName = ma->getPrintableString(protocolNameVMAddr, protocolNameResult,
                                              &protocolNameSectionCache, protocolNameSectionHandler);

        if ( protocolNameResult != MachOAnalyzer::PrintableStringResult::CanPrint ) {
            diag.error("Invalid protocol name for objc optimisation");
            return;
        }

        // If the protocol also exists in a shared cache image which is loaded, then objc
        // would have found that one, regardless of load order.  So we can just skip this one.
        {
            void *cls;
            void *hi;
            uint32_t count = objcProtocolOpt->getClassAndHeader(protocolName, cls, hi);
            if (count == 1) {
                // exactly one matching protocol.  Check if its loaded
                if (sharedCacheImagesMap.find(getMachHeaderFromObjCHeaderInfo(hi, pointerSize)) != sharedCacheImagesMap.end())
                    return;
            }
            else if (count > 1) {
                // more than one matching protocol - find one that is loaded
                void *clslist[count];
                void *hilist[count];
                objcProtocolOpt->getClassesAndHeaders(protocolName, clslist, hilist);
                for (uint32_t i = 0; i < count; i++) {
                    if (sharedCacheImagesMap.find(getMachHeaderFromObjCHeaderInfo(hilist[i], pointerSize)) != sharedCacheImagesMap.end())
                        return;
                }
            }
        }

        // Get the section for the protocol itself
        __block uint64_t protocolSectionStartVMAddr = 0;
        auto protocolSectionHandler = ^bool(const MachOAnalyzer::SectionInfo& sectInfo) {
            // We only have 23-bits in ObjCClassImageOffset to index in to the protocols
            if (sectInfo.sectSize > Image::ObjCClassImageOffset::maximumOffset) {
                return false;
            }

            // We use 32-bit offsets so make sure the section is no larger than that.
            uint64_t protocolDatasVMOffset = sectInfo.sectAddr - loadAddress;
            if (protocolDatasVMOffset >= (1ULL << 32)) {
                return false;
            }

            protocolSectionStartVMAddr = sectInfo.sectAddr;
            return true;
        };
        if (!protocolSectionCache.findSectionForVMAddr(protocolVMAddr, protocolSectionHandler)) {
            diag.error("Invalid protocol for objc optimisation");
            return;
        }

        // Make sure we have an entry for our images offsets for later
        uint64_t protocolNameSectionVMOffset = protocolNameSectionStartVMAddr - loadAddress;
        uint64_t protocolSectionVMOffset = protocolSectionStartVMAddr - loadAddress;
        uint64_t hashTableVMOffsetsIndex = 0;
        for (auto nameAndDataVMOffset : image.classesNameAndDataVMOffsets) {
            if ( (nameAndDataVMOffset.first == protocolNameSectionVMOffset) && (nameAndDataVMOffset.second == protocolSectionVMOffset) )
                break;
            ++hashTableVMOffsetsIndex;
        }

        if (hashTableVMOffsetsIndex == image.classesNameAndDataVMOffsets.count()) {
            // Didn't find an image entry with this offset.  Add one if we have space
            uint64_t totalHashTableImages = image.classesNameAndDataVMOffsets.count() + _objcClassesHashTableImages.count();
            if ( totalHashTableImages == Image::ObjCClassNameImageOffset::maximumImageIndex ) {
                // No more space.  We need to give up
                diag.error("No more space for protocol hash table image");
                return;
            }
            image.classesNameAndDataVMOffsets.push_back({ protocolNameSectionVMOffset, protocolSectionVMOffset });
        }

        hashTableVMOffsetsIndex += _objcClassesHashTableImages.count();

        uint64_t protocolNameOffset = protocolNameVMAddr - protocolNameSectionStartVMAddr;
        uint64_t protocolDataOffset = protocolVMAddr - protocolSectionStartVMAddr;

        closure::Image::ObjCClassNameImageOffset protocolNameTarget;
        protocolNameTarget.classNameImageIndex   = (uint32_t)hashTableVMOffsetsIndex;
        protocolNameTarget.classNameImageOffset  = (uint32_t)protocolNameOffset;

        dyld3::closure::Image::ObjCClassImageOffset protocolDataTarget;
        protocolDataTarget.classData.imageIndex   = (uint32_t)hashTableVMOffsetsIndex;
        protocolDataTarget.classData.imageOffset  = (uint32_t)protocolDataOffset;
        protocolDataTarget.classData.isDuplicate  = 0;

        seenProtocols.push_back({ protocolNameTarget, protocolDataTarget });
    });
}

// used at launch by dyld when kernel has already mapped main executable
const LaunchClosure* ClosureBuilder::makeLaunchClosure(const LoadedFileInfo& fileInfo, bool allowInsertFailures)
{
    dyld3::ScopedTimer timer(DBG_DYLD_TIMING_BUILD_CLOSURE, 0, 0, 0);
    const mach_header* mainMH = (const mach_header*)fileInfo.fileContent;
    // set up stack based storage for all arrays
    BuilderLoadedImage  loadImagesStorage[512];
    Image::LinkedImage  dependenciesStorage[512*8];
    InterposingTuple    tuplesStorage[64];
    Closure::PatchEntry cachePatchStorage[64];
    const char*         weakDefNameStorage[64];
    _loadedImages.setInitialStorage(loadImagesStorage, 512);
    _dependencies.setInitialStorage(dependenciesStorage, 512*8);
    _interposingTuples.setInitialStorage(tuplesStorage, 64);
    _weakDefCacheOverrides.setInitialStorage(cachePatchStorage, 64);
    _weakDefsFromChainedBinds.setInitialStorage(weakDefNameStorage, 64);
    ArrayFinalizer<BuilderLoadedImage> scopedCleanup(_loadedImages, ^(BuilderLoadedImage& li) { if (li.unmapWhenDone) {_fileSystem.unloadFile(li.loadedFileInfo); li.unmapWhenDone=false;} });

    const MachOAnalyzer* mainExecutable = MachOAnalyzer::validMainExecutable(_diag, mainMH, fileInfo.path, fileInfo.sliceLen, _archs, _platform);
    if ( mainExecutable == nullptr )
        return nullptr;
    if ( !mainExecutable->isDynamicExecutable() ) {
        _diag.error("not a main executable");
        return nullptr;
    }
    _isLaunchClosure    = true;
    _allowMissingLazies = true;

    _nextIndex = 0;

    // add main executable
    __block BuilderLoadedImage mainEntry;
    mainEntry.loadedFileInfo            = fileInfo;
    mainEntry.imageNum                  = 0; // We can't fill this in until we've done inserted dylibs
    mainEntry.unmapWhenDone             = false;
    mainEntry.contentRebased            = false;
    mainEntry.hasInits                  = false;
    mainEntry.markNeverUnload           = true;
    mainEntry.rtldLocal                 = false;
    mainEntry.isBadImage                = false;
    mainEntry.mustBuildClosure          = true;
    mainEntry.hasMissingWeakImports     = false;
    mainEntry.overrideImageNum          = 0;

    // Set the executable load path so that @executable_path can use it later
    _mainProgLoadPath = fileInfo.path;

    // add any DYLD_INSERT_LIBRARIES
    _pathOverrides.forEachInsertedDylib(^(const char* dylibPath, bool &stop) {
        LoadedImageChain chainMain = { nullptr, mainEntry };
        BuilderLoadedImage* foundTopImage;
        if ( !findImage(dylibPath, chainMain, foundTopImage, LinkageType::kInserted, 0, true) ) {
            if ( !allowInsertFailures ) {
                if ( _diag.noError() )
                    _diag.error("could not load inserted dylib %s", dylibPath);
                stop = true;
                return;
            }
            _diag.clearError(); // FIXME add way to plumb back warning
        }
    });

    if ( _diag.hasError() )
        return nullptr;

    _mainProgLoadIndex = (uint32_t)_loadedImages.count();
    mainEntry.imageNum = _startImageNum + _nextIndex++;
    _loadedImages.push_back(mainEntry);

	// get mach_headers for all images needed to launch this main executable
    LoadedImageChain chainStart = { nullptr, _loadedImages[_mainProgLoadIndex] };
    recursiveLoadDependents(chainStart);
    if ( _diag.hasError() )
        return nullptr;
    for (uint32_t i=0; i < _mainProgLoadIndex; ++i) {
        LoadedImageChain insertChainStart = { nullptr, _loadedImages[i] };
        recursiveLoadDependents(insertChainStart);
        if ( _diag.hasError() )
            return nullptr;
    }
    loadDanglingUpwardLinks();

    // If we have an on-disk image then we need all images which are dependent on the disk image to get a new
    // initializer order.  Its not enough to just do the top level image as we may dlopen while in dlopen
    invalidateInitializerRoots();

    // now that everything loaded, set _libDyldImageNum and _libSystemImageNum
    for (BuilderLoadedImage& li : _loadedImages) {
        if ( li.loadAddress()->isDylib() && (strcmp(li.loadAddress()->installName(), "/usr/lib/system/libdyld.dylib") == 0) )
            _libDyldImageNum = li.imageNum;
        else if ( strcmp(li.path(), "/usr/lib/libSystem.B.dylib") == 0 )
            _libSystemImageNum = li.imageNum;
    }

    // only some images need to go into closure (non-rooted ones from dyld cache do not)
    STACK_ALLOC_ARRAY(ImageWriter, writers, _loadedImages.count());
    for (BuilderLoadedImage& li : _loadedImages) {
        if ( li.mustBuildClosure ) {
            writers.push_back(ImageWriter());
            buildImage(writers.back(), li);
            if ( _diag.hasError() )
                return nullptr;
        }
   }

    bool optimizedObjC = optimizeObjC(writers);

    // Note we have to compute the init order after buildImage as buildImage may set hasInits to true
    for (uintptr_t imageIndex = 0, writerIndex = 0; imageIndex != _loadedImages.count(); ++imageIndex) {
        BuilderLoadedImage& li = _loadedImages[imageIndex];
        if ( li.mustBuildClosure ) {
            computeInitOrder(writers[writerIndex], (uint32_t)imageIndex);
            writerIndex++;
        }
    }

    // combine all Image objects into one ImageArray
    ImageArrayWriter imageArrayWriter(_startImageNum, (uint32_t)writers.count(), _foundDyldCacheRoots);
    for (ImageWriter& writer : writers) {
        imageArrayWriter.appendImage(writer.finalize());
        writer.deallocate();
    }
    const ImageArray* imageArray = imageArrayWriter.finalize();

    // merge ImageArray object into LaunchClosure object
    __block LaunchClosureWriter closureWriter(imageArray);

    if (optimizedObjC) {
        if (!_objcSelectorsHashTable.empty())
            closureWriter.setObjCSelectorInfo(_objcSelectorsHashTable, _objcSelectorsHashTableImages);
    
        if (!_objcClassesHashTableImages.empty()) {
            closureWriter.setObjCClassAndProtocolInfo(_objcClassesHashTable, _objcProtocolsHashTable,
                                                      _objcClassesHashTableImages);
        }

        if ( _objcDuplicateClassWarnings != nullptr ) {
            _objcDuplicateClassWarnings->forEachPath(^(const char* warning) {
                closureWriter.addWarning(Closure::Warning::duplicateObjCClass, warning);
            });
        }

        if (!_objcClassesDuplicatesHashTable.empty())
            closureWriter.setObjCDuplicateClassesInfo(_objcClassesDuplicatesHashTable);
    }

    // record shared cache info
    if ( _dyldCache != nullptr ) {
        // record cache UUID
        uuid_t cacheUUID;
        _dyldCache->getUUID(cacheUUID);
        closureWriter.setDyldCacheUUID(cacheUUID);

        // record any cache patching needed because of dylib overriding cache
        for (const BuilderLoadedImage& li : _loadedImages) {
            if ( li.overrideImageNum != 0 ) {
                uint32_t imageIndex = li.overrideImageNum - (uint32_t)_dyldImageArray->startImageNum();
                STACK_ALLOC_ARRAY(Closure::PatchEntry, patches, _dyldCache->patchableExportCount(imageIndex));
                MachOLoaded::DependentToMachOLoaded reexportFinder = ^(const MachOLoaded* mh, uint32_t depIndex) {
                    return (const MachOLoaded*)findDependent(mh, depIndex);
                };
                //fprintf(stderr, "'%s' overrides '%s'\n", li.loadedFileInfo.path, cacheImage->path());
                _dyldCache->forEachPatchableExport(imageIndex, ^(uint32_t cacheOffsetOfImpl, const char* symbolName) {
                    dyld3::MachOAnalyzer::FoundSymbol foundInfo;
                    Diagnostics                       patchDiag;
                    Closure::PatchEntry               patch;
                    patch.overriddenDylibInCache  = li.overrideImageNum;
                    patch.exportCacheOffset       = cacheOffsetOfImpl;
                    if ( li.loadAddress()->findExportedSymbol(patchDiag, symbolName, false, foundInfo, reexportFinder) ) {
                        const MachOAnalyzer* impDylib = (const MachOAnalyzer*)foundInfo.foundInDylib;
                        patch.replacement.image.kind     = Image::ResolvedSymbolTarget::kindImage;
                        patch.replacement.image.imageNum = findLoadedImage(impDylib).imageNum;
                        patch.replacement.image.offset   = foundInfo.value;
                    }
                    else {
                        // this means the symbol is missing in the cache override dylib, so set any uses to NULL
                        patch.replacement.absolute.kind    = Image::ResolvedSymbolTarget::kindAbsolute;
                        patch.replacement.absolute.value   = 0;
                    }
                    patches.push_back(patch);
                });
                closureWriter.addCachePatches(patches);
            }
        }

        // handle any extra weak-def coalescing needed by chained fixups
        if ( !_weakDefsFromChainedBinds.empty() ) {
            for (const char* symbolName : _weakDefsFromChainedBinds) {
                Image::ResolvedSymbolTarget cacheOverrideTarget;
                bool haveCacheOverride = false;
                bool foundCachOverrideIsWeakDef = false;
                for (const BuilderLoadedImage& li : _loadedImages) {
                    if ( !li.loadAddress()->hasWeakDefs() )
                        continue;
                    Image::ResolvedSymbolTarget target;
                    ResolvedTargetInfo          targetInfo;
                    if ( findSymbolInImage(li.loadAddress(), symbolName, 0, false, false, target, targetInfo) ) {
                        if ( li.loadAddress()->inDyldCache() ) {
                            if ( haveCacheOverride ) {
                                Closure::PatchEntry patch;
                                patch.exportCacheOffset      = (uint32_t)target.sharedCache.offset;
                                patch.overriddenDylibInCache = li.imageNum;
                                patch.replacement            = cacheOverrideTarget;
                                _weakDefCacheOverrides.push_back(patch);
                            }
                            else {
                                // found first in cached dylib, so no need to patch cache for this symbol
                                break;
                            }
                        }
                        else {
                            // found image that exports this symbol and is not in cache
                            if ( !haveCacheOverride || (foundCachOverrideIsWeakDef && !targetInfo.isWeakDef) ) {
                                // update cache to use this symbol if it if first found or it is first non-weak found
                                cacheOverrideTarget         = target;
                                foundCachOverrideIsWeakDef  = targetInfo.isWeakDef;
                                haveCacheOverride           = true;
                            }
                        }
                    }
                }
            }
        }

        // record any cache patching needed because weak-def C++ symbols override dyld cache
        if ( !_weakDefCacheOverrides.empty() )
            closureWriter.addCachePatches(_weakDefCacheOverrides);

   }

#if __IPHONE_OS_VERSION_MIN_REQUIRED
    // if closure is built on-device for iOS, then record boot UUID
    char bootSessionUUID[256] = { 0 };
    size_t bootSize = sizeof(bootSessionUUID);
    if ( sysctlbyname("kern.bootsessionuuid", bootSessionUUID, &bootSize, NULL, 0) == 0 )
        closureWriter.setBootUUID(bootSessionUUID);
#endif

     // record any interposing info
    imageArray->forEachImage(^(const Image* image, bool &stop) {
        if ( !image->inDyldCache() )
            addInterposingTuples(closureWriter, image, findLoadedImage(image->imageNum()).loadAddress());
    });

    // modify fixups in contained Images by applying interposing tuples
    closureWriter.applyInterposing((const LaunchClosure*)closureWriter.currentTypedBytes());

    // set flags
    closureWriter.setUsedAtPaths(_atPathUsed);
    closureWriter.setUsedFallbackPaths(_fallbackPathUsed);
    closureWriter.setHasInsertedLibraries(_mainProgLoadIndex > 0);
    closureWriter.setInitImageCount((uint32_t)_loadedImages.count());

    // add other closure attributes
    addClosureInfo(closureWriter);

    // make result
    const LaunchClosure* result = closureWriter.finalize();
    imageArrayWriter.deallocate();

    timer.setData4(dyld3::DyldTimingBuildClosure::LaunchClosure_Built);

    return result;
}

// used by libdyld for dlopen()
const DlopenClosure* ClosureBuilder::makeDlopenClosure(const char* path, const LaunchClosure* mainClosure, const Array<LoadedImage>& alreadyLoadedList,
                                                       closure::ImageNum callerImageNum, bool noLoad, bool forceBindLazies, bool canUseSharedCacheClosure, closure::ImageNum* topImageNum)
{
    dyld3::ScopedTimer timer(DBG_DYLD_TIMING_BUILD_CLOSURE, 0, 0, 0);
    // set up stack based storage for all arrays
    BuilderLoadedImage  loadImagesStorage[300];
    Image::LinkedImage  dependenciesStorage[128];
    Closure::PatchEntry cachePatchStorage[64];
    _loadedImages.setInitialStorage(loadImagesStorage, 300);
    _dependencies.setInitialStorage(dependenciesStorage, 128);
    _weakDefCacheOverrides.setInitialStorage(cachePatchStorage, 64);
    ArrayFinalizer<BuilderLoadedImage> scopedCleanup(_loadedImages, ^(BuilderLoadedImage& li) { if (li.unmapWhenDone) {_fileSystem.unloadFile(li.loadedFileInfo); li.unmapWhenDone=false;} });

    // fill in builder array from already loaded images
    bool cachedDylibsExpectedOnDisk = _dyldCache ? _dyldCache->header.dylibsExpectedOnDisk : true;
    uintptr_t callerImageIndex = UINTPTR_MAX;
    for (const LoadedImage& ali : alreadyLoadedList) {
        const Image*          image       = ali.image();
        const MachOAnalyzer*  ma          = (MachOAnalyzer*)(ali.loadedAddress());
        bool                  inDyldCache = ma->inDyldCache();
        BuilderLoadedImage entry;
        ImageNum overrideImageNum;
        entry.loadedFileInfo.path        = image->path();
        entry.loadedFileInfo.fileContent = ma;
        entry.loadedFileInfo.sliceOffset = 0;
        entry.loadedFileInfo.inode       = 0;
        entry.loadedFileInfo.mtime       = 0;
        entry.imageNum                   = image->imageNum();
        entry.dependents                 = image->dependentsArray();
        entry.unmapWhenDone              = false;
        entry.contentRebased             = inDyldCache;
        entry.hasInits                   = false;
        entry.markNeverUnload            = image->neverUnload();
        entry.rtldLocal                  = ali.hideFromFlatSearch();
        entry.isBadImage                 = false;
        entry.mustBuildClosure           = false;
        entry.hasMissingWeakImports      = false;
        entry.overrideImageNum           = 0;
        if ( !inDyldCache && image->isOverrideOfDyldCacheImage(overrideImageNum) ) {
            entry.overrideImageNum  = overrideImageNum;
            canUseSharedCacheClosure = false;
        }
        if ( !inDyldCache || cachedDylibsExpectedOnDisk )
            image->hasFileModTimeAndInode(entry.loadedFileInfo.inode, entry.loadedFileInfo.mtime);
        if ( entry.imageNum == callerImageNum )
            callerImageIndex = _loadedImages.count();
        _loadedImages.push_back(entry);
   }
    _alreadyInitedIndex = (uint32_t)_loadedImages.count();

    // find main executable (may be needed for @executable_path)
    _isLaunchClosure = false;
    for (uint32_t i=0; i < alreadyLoadedList.count(); ++i) {
        if ( _loadedImages[i].loadAddress()->isMainExecutable() )  {
            _mainProgLoadIndex = i;
            _mainProgLoadPath = _loadedImages[i].path();
            break;
        }
    }

    // We can't use an existing dlopen closure if the main closure had interposing tuples
    if (canUseSharedCacheClosure) {
        if (mainClosure->hasInterposings())
            canUseSharedCacheClosure = false;
    }

    // add top level dylib being dlopen()ed
    BuilderLoadedImage* foundTopImage;
    _nextIndex = 0;
    // @rpath has caller's LC_PRATH, then main executable's LC_RPATH
    BuilderLoadedImage& callerImage = (callerImageIndex != UINTPTR_MAX) ? _loadedImages[callerImageIndex]  : _loadedImages[_mainProgLoadIndex];
    LoadedImageChain chainCaller = { nullptr, callerImage };
    LoadedImageChain chainMain = { &chainCaller, _loadedImages[_mainProgLoadIndex] };
    if ( !findImage(path, chainMain, foundTopImage, LinkageType::kDynamic, 0, canUseSharedCacheClosure) ) {
        // If we didn't find the image, it might be a symlink to something in the dyld cache that is not on disk
        if ( (_dyldCache != nullptr) && !_dyldCache->header.dylibsExpectedOnDisk ) {
            char resolvedPath[PATH_MAX];
            if ( _fileSystem.getRealPath(path, resolvedPath) ) {
                _diag.clearError();
                if ( !findImage(resolvedPath, chainMain, foundTopImage, LinkageType::kDynamic, 0, canUseSharedCacheClosure) ) {
                    return nullptr;
                }
            } else {
                // We didn't find a new path from realpath
                return nullptr;
            }
        } else {
            // cached dylibs on disk, so don't call realpath() again, it would have been found first call to findImage()
            return nullptr;
        }
    }

    // exit early in RTLD_NOLOAD mode
    if ( noLoad ) {
        timer.setData4(dyld3::DyldTimingBuildClosure::DlopenClosure_NoLoad);
        // if no new images added to _loadedImages, then requested path was already loaded
        if ( (uint32_t)_loadedImages.count() == _alreadyInitedIndex )
            *topImageNum = foundTopImage->imageNum;
        else
            *topImageNum = 0;
        return nullptr;
    }

    // fast path if roots are not allowed and target is in dyld cache or is other
    if ( (_dyldCache != nullptr) && (_dyldCache->header.cacheType == kDyldSharedCacheTypeProduction) ) {
        if ( foundTopImage->imageNum < closure::kFirstLaunchClosureImageNum ) {
            if (foundTopImage->imageNum < closure::kLastDyldCacheImageNum)
                timer.setData4(dyld3::DyldTimingBuildClosure::DlopenClosure_UsedSharedCacheDylib);
            else
                timer.setData4(dyld3::DyldTimingBuildClosure::DlopenClosure_UsedSharedCacheOther);
            *topImageNum = foundTopImage->imageNum;
            return nullptr;
        }
    }

    // recursive load dependents
    // @rpath for stuff top dylib depends on uses LC_RPATH from caller, main exe, and dylib being dlopen()ed
    LoadedImageChain chainTopDylib = { &chainMain, *foundTopImage };
    recursiveLoadDependents(chainTopDylib, canUseSharedCacheClosure);
    if ( _diag.hasError() )
        return nullptr;
    loadDanglingUpwardLinks(canUseSharedCacheClosure);
    if ( _diag.hasError() )
        return nullptr;

    // RTLD_NOW means fail the dlopen() if a symbol cannot be bound
    _allowMissingLazies = !forceBindLazies;

    // only some images need to go into closure (ones from dyld cache do not, unless the cache format changed)
    STACK_ALLOC_ARRAY(ImageWriter, writers, _loadedImages.count());
    if ( _foundNonCachedImage || _foundDyldCacheRoots ) {
        // If we have an on-disk image then we need all images which are dependent on the disk image to get a new
        // initializer order.  Its not enough to just do the top level image as we may dlopen while in dlopen
        invalidateInitializerRoots();

        for (uintptr_t loadedImageIndex = 0; loadedImageIndex != _loadedImages.count(); ++loadedImageIndex) {
            BuilderLoadedImage& li = _loadedImages[loadedImageIndex];
            if ( li.mustBuildClosure ) {
                writers.push_back(ImageWriter());
                buildImage(writers.back(), li);
                if ( _diag.hasError() )
                    return nullptr;
            }
        }

        // Note we have to compute the init order after buildImage as buildImage may set hasInits to true
        for (uintptr_t imageIndex = 0, writerIndex = 0; imageIndex != _loadedImages.count(); ++imageIndex) {
            BuilderLoadedImage& li = _loadedImages[imageIndex];
            if ( li.mustBuildClosure ) {
                computeInitOrder(writers[writerIndex], (uint32_t)imageIndex);
                writerIndex++;
            }
        }
    }
    if ( _diag.hasError() )
        return nullptr;

    // check if top image loaded is in shared cache along with everything it depends on
    *topImageNum = foundTopImage->imageNum;
    if ( _foundNonCachedImage || _foundDyldCacheRoots ) {
        if ( canUseSharedCacheClosure && ( foundTopImage->imageNum < closure::kFirstLaunchClosureImageNum ) ) {
            // We used a shared cache built closure, but now discovered roots.  We need to try again
            topImageNum = 0;
            return sRetryDlopenClosure;
        }
    } else {
        if (foundTopImage->imageNum < closure::kLastDyldCacheImageNum)
            timer.setData4(dyld3::DyldTimingBuildClosure::DlopenClosure_UsedSharedCacheDylib);
        else
            timer.setData4(dyld3::DyldTimingBuildClosure::DlopenClosure_UsedSharedCacheOther);
        return nullptr;
    }

    // combine all Image objects into one ImageArray
    ImageArrayWriter imageArrayWriter(_startImageNum, (uint32_t)writers.count(), _foundDyldCacheRoots);
    for (ImageWriter& writer : writers) {
        imageArrayWriter.appendImage(writer.finalize());
        writer.deallocate();
    }
    const ImageArray* imageArray = imageArrayWriter.finalize();

    // merge ImageArray object into LaunchClosure object
    DlopenClosureWriter closureWriter(imageArray);

    // add other closure attributes
    closureWriter.setTopImageNum(foundTopImage->imageNum);

    // record any cache patching needed because of dylib overriding cache
    if ( _dyldCache != nullptr ) {
        for (const BuilderLoadedImage& li : _loadedImages) {
            if ( (li.overrideImageNum != 0) && (li.imageNum >= _startImageNum) ) {
                const Image* cacheImage = _dyldImageArray->imageForNum(li.overrideImageNum);
                uint32_t imageIndex = cacheImage->imageNum() - (uint32_t)_dyldCache->cachedDylibsImageArray()->startImageNum();
                STACK_ALLOC_ARRAY(Closure::PatchEntry, patches, _dyldCache->patchableExportCount(imageIndex));
                MachOLoaded::DependentToMachOLoaded reexportFinder = ^(const MachOLoaded* mh, uint32_t depIndex) {
                    return (const MachOLoaded*)findDependent(mh, depIndex);
                };
                //fprintf(stderr, "'%s' overrides '%s'\n", li.loadedFileInfo.path, cacheImage->path());
                _dyldCache->forEachPatchableExport(imageIndex,
                                                   ^(uint32_t cacheOffsetOfImpl, const char* symbolName) {
                    dyld3::MachOAnalyzer::FoundSymbol foundInfo;
                    Diagnostics                       patchDiag;
                    Closure::PatchEntry               patch;
                    patch.overriddenDylibInCache  = li.overrideImageNum;
                    patch.exportCacheOffset       = cacheOffsetOfImpl;
                    if ( li.loadAddress()->findExportedSymbol(patchDiag, symbolName, false, foundInfo, reexportFinder) ) {
                        const MachOAnalyzer* impDylib = (const MachOAnalyzer*)foundInfo.foundInDylib;
                        patch.replacement.image.kind     = Image::ResolvedSymbolTarget::kindImage;
                        patch.replacement.image.imageNum = findLoadedImage(impDylib).imageNum;
                        patch.replacement.image.offset   = foundInfo.value;
                    }
                    else {
                        patch.replacement.absolute.kind    = Image::ResolvedSymbolTarget::kindAbsolute;
                        patch.replacement.absolute.value   = 0;
                    }
                    patches.push_back(patch);
                });
                closureWriter.addCachePatches(patches);
            }
        }
    }

    // modify fixups in contained Images by applying interposing tuples
    closureWriter.applyInterposing(mainClosure);

    // Dlopen's should never keep track of missing paths as we don't cache these closures.
    assert(_mustBeMissingPaths == nullptr);

    // make final DlopenClosure object
    const DlopenClosure* result = closureWriter.finalize();
    imageArrayWriter.deallocate();
    timer.setData4(dyld3::DyldTimingBuildClosure::DlopenClosure_Built);
    return result;
}


// used by dyld_closure_util
const LaunchClosure* ClosureBuilder::makeLaunchClosure(const char* mainPath, bool allowInsertFailures)
{
    char realerPath[MAXPATHLEN];
    closure::LoadedFileInfo loadedFileInfo = MachOAnalyzer::load(_diag, _fileSystem, mainPath, _archs, _platform, realerPath);
    const MachOAnalyzer* mh = (const MachOAnalyzer*)loadedFileInfo.fileContent;
    loadedFileInfo.path = mainPath;
    if (_diag.hasError())
        return nullptr;
    if (mh == nullptr) {
        _diag.error("could not load file");
        return nullptr;
    }
    if (!mh->isDynamicExecutable()) {
        _diag.error("file is not an executable");
        return nullptr;
    }
    const_cast<PathOverrides*>(&_pathOverrides)->setMainExecutable(mh, mainPath);
    const LaunchClosure* launchClosure = makeLaunchClosure(loadedFileInfo, allowInsertFailures);
    loadedFileInfo.unload(loadedFileInfo);
    return launchClosure;
}

void ClosureBuilder::setDyldCacheInvalidFormatVersion() {
    _dyldCacheInvalidFormatVersion = true;
}


// used by dyld shared cache builder
const ImageArray* ClosureBuilder::makeDyldCacheImageArray(bool customerCache, const Array<CachedDylibInfo>& dylibs, const Array<CachedDylibAlias>& aliases)
{
    // because this is run in cache builder using dispatch_apply() there is minimal stack space
    // so set up storage for all arrays to be vm_allocated
    uintptr_t maxImageCount = dylibs.count() + 16;
    _loadedImages.reserve(maxImageCount);
    _dependencies.reserve(maxImageCount*16);

    _makingDyldCacheImages = true;
    _allowMissingLazies    = false;
    _makingCustomerCache   = customerCache;
    _aliases               = &aliases;

    // build _loadedImages[] with every dylib in cache
    __block ImageNum imageNum = _startImageNum;
    for (const CachedDylibInfo& aDylibInfo : dylibs)  {
        BuilderLoadedImage entry;
        entry.loadedFileInfo                = aDylibInfo.fileInfo;
        entry.imageNum                      = imageNum++;
        entry.unmapWhenDone                 = false;
        entry.contentRebased                = false;
        entry.hasInits                      = false;
        entry.markNeverUnload               = true;
        entry.rtldLocal                     = false;
        entry.isBadImage                    = false;
        entry.mustBuildClosure              = false;
        entry.hasMissingWeakImports         = false;
        entry.overrideImageNum              = 0;
        _loadedImages.push_back(entry);
    }

    // wire up dependencies between cached dylibs
    for (BuilderLoadedImage& li : _loadedImages) {
        LoadedImageChain chainStart = { nullptr, li };
        recursiveLoadDependents(chainStart);
        if ( _diag.hasError() )
            break;
    }
    assert(_loadedImages.count() == dylibs.count());

    // create an ImageWriter for each cached dylib
    STACK_ALLOC_ARRAY(ImageWriter, writers, _loadedImages.count());
    for (BuilderLoadedImage& li : _loadedImages) {
        writers.push_back(ImageWriter());
        buildImage(writers.back(), li);
    }

    // add initializer order into each dylib
    // Note we have to compute the init order after buildImage as buildImage may set hasInits to true
    for (const BuilderLoadedImage& li : _loadedImages) {
        uint32_t index = li.imageNum - _startImageNum;
        computeInitOrder(writers[index], index);
    }

    // combine all Image objects into one ImageArray
    ImageArrayWriter imageArrayWriter(_startImageNum, (uint32_t)writers.count(), _foundDyldCacheRoots);
    for (ImageWriter& writer : writers) {
        imageArrayWriter.appendImage(writer.finalize());
        writer.deallocate();
    }
    const ImageArray* imageArray = imageArrayWriter.finalize();

    return imageArray;
}


#if BUILDING_CACHE_BUILDER
const ImageArray* ClosureBuilder::makeOtherDylibsImageArray(const Array<LoadedFileInfo>& otherDylibs, uint32_t cachedDylibsCount)
{
    // because this is run in cache builder using dispatch_apply() there is minimal stack space
    // so set up storage for all arrays to be vm_allocated
    uintptr_t maxImageCount = otherDylibs.count() + cachedDylibsCount + 128;
    _loadedImages.reserve(maxImageCount);
    _dependencies.reserve(maxImageCount*16);

    // build _loadedImages[] with every dylib in cache, followed by others
    _nextIndex = 0;
    for (const LoadedFileInfo& aDylibInfo : otherDylibs)  {
        BuilderLoadedImage entry;
        entry.loadedFileInfo                = aDylibInfo;
        entry.imageNum                      = _startImageNum + _nextIndex++;
        entry.unmapWhenDone                 = false;
        entry.contentRebased                = false;
        entry.hasInits                      = false;
        entry.markNeverUnload               = false;
        entry.rtldLocal                     = false;
        entry.isBadImage                    = false;
        entry.mustBuildClosure              = false;
        entry.hasMissingWeakImports         = false;
        entry.overrideImageNum              = 0;
        _loadedImages.push_back(entry);
    }

    // wire up dependencies between cached dylibs
    // Note, _loadedImages can grow when we call recursiveLoadDependents so we need
    // to check the count on each iteration.
    for (uint64_t index = 0; index != _loadedImages.count(); ++index) {
        BuilderLoadedImage& li = _loadedImages[index];
        LoadedImageChain chainStart = { nullptr, li };
        recursiveLoadDependents(chainStart);
        if ( _diag.hasError() ) {
            _diag.warning("while building dlopen closure for %s: %s", li.loadedFileInfo.path, _diag.errorMessage().c_str());
            //fprintf(stderr, "while building dlopen closure for %s: %s\n", li.loadedFileInfo.path, _diag.errorMessage().c_str());
            _diag.clearError();
            li.isBadImage = true;    // mark bad
        }
    }

    auto invalidateBadImages = [&]() {
        // Invalidate images with bad dependencies
        while (true) {
            bool madeChange = false;
            for (BuilderLoadedImage& li : _loadedImages) {
                if (li.isBadImage) {
                    // Already invalidated
                    continue;
                }
                for (Image::LinkedImage depIndex : li.dependents) {
                    if ( depIndex.imageNum() == kMissingWeakLinkedImage )
                        continue;
                    if ( depIndex.imageNum() >= dyld3::closure::kLastDyldCacheImageNum ) {
                        // dlopen closures can only depend on the shared cache.  This is because if foo.dylib links bar.dylib
                        // and bar.dylib is loaded in to the launch closure, then the dlopen closure for foo.dylib wouldn't see
                        // bar.dylib at the image num in the launch closure
                        _diag.warning("while building dlopen closure for %s: dependent dylib is not from shared cache", li.loadedFileInfo.path);
                        li.isBadImage = true;    // mark bad
                        madeChange = true;
                        continue;
                    }
                    BuilderLoadedImage& depImage = findLoadedImage(depIndex.imageNum());
                    if (depImage.isBadImage) {
                        _diag.warning("while building dlopen closure for %s: dependent dylib had error", li.loadedFileInfo.path);
                        li.isBadImage = true;    // mark bad
                        madeChange = true;
                    }
                }
            }
            if (!madeChange)
                break;
        }
    };

    invalidateBadImages();

    // create an ImageWriter for each cached dylib
    STACK_ALLOC_ARRAY(ImageWriter, writers, _loadedImages.count());
    for (BuilderLoadedImage& li : _loadedImages) {
        if ( li.isBadImage )  {
            writers.push_back(ImageWriter());
            writers.back().setInvalid();
            continue;
        }
        if ( li.imageNum < dyld3::closure::kLastDyldCacheImageNum )
            continue;
        writers.push_back(ImageWriter());
        buildImage(writers.back(), li);
        if ( _diag.hasError() ) {
            _diag.warning("while building dlopen closure for %s: %s", li.loadedFileInfo.path, _diag.errorMessage().c_str());
            //fprintf(stderr, "while building dlopen closure for %s: %s\n", li.loadedFileInfo.path, _diag.errorMessage().c_str());
            _diag.clearError();
            li.isBadImage = true;    // mark bad
            writers.back().setInvalid();
        }
    }

    invalidateBadImages();

    // add initializer order into each dylib
    // Note we have to compute the init order after buildImage as buildImage may set hasInits to true
    for (const BuilderLoadedImage& li : _loadedImages) {
        if ( li.imageNum < dyld3::closure::kLastDyldCacheImageNum )
            continue;
        if (li.isBadImage)
            continue;
        uint32_t index = li.imageNum - _startImageNum;
        computeInitOrder(writers[index], index);
    }

    // combine all Image objects into one ImageArray
    ImageArrayWriter imageArrayWriter(_startImageNum, (uint32_t)writers.count(), _foundDyldCacheRoots);
    for (ImageWriter& writer : writers) {
        imageArrayWriter.appendImage(writer.finalize());
        writer.deallocate();
    }
    const ImageArray* imageArray = imageArrayWriter.finalize();

    return imageArray;
}
#endif


bool ClosureBuilder::inLoadedImageArray(const Array<LoadedImage>& loadedList, ImageNum imageNum)
{
    for (const LoadedImage& ali : loadedList) {
        if ( ali.image()->representsImageNum(imageNum) )
            return true;
    }
    return false;
}

void ClosureBuilder::buildLoadOrderRecurse(Array<LoadedImage>& loadedList, const Array<const ImageArray*>& imagesArrays, const Image* image)
{
    // breadth first load
    STACK_ALLOC_ARRAY(const Image*, needToRecurse, 256);
    image->forEachDependentImage(^(uint32_t dependentIndex, dyld3::closure::Image::LinkKind kind, ImageNum depImageNum, bool &stop) {
        if ( !inLoadedImageArray(loadedList, depImageNum) ) {
            const Image* depImage = ImageArray::findImage(imagesArrays, depImageNum);
            loadedList.push_back(LoadedImage::make(depImage));
            needToRecurse.push_back(depImage);
        }
    });

    // recurse load
    for (const Image* img : needToRecurse) {
        buildLoadOrderRecurse(loadedList, imagesArrays, img);
    }
}

void ClosureBuilder::buildLoadOrder(Array<LoadedImage>& loadedList, const Array<const ImageArray*>& imagesArrays, const Closure* toAdd)
{
    const dyld3::closure::Image* topImage = ImageArray::findImage(imagesArrays, toAdd->topImage());
	loadedList.push_back(LoadedImage::make(topImage));
	buildLoadOrderRecurse(loadedList, imagesArrays, topImage);
}



////////////////////////////  ObjCStringTable ////////////////////////////////////////

template<typename PerfectHashT, typename ImageOffsetT>
void ObjCStringTable::write(const PerfectHashT& phash, const Array<std::pair<const char*, ImageOffsetT>>& strings)
{
    ObjCSelectorOpt::StringTarget sentinel = (ObjCSelectorOpt::StringTarget)ImageOffsetT::sentinelValue;
    // Set header
    capacity = phash.capacity;
    occupied = phash.occupied;
    shift = phash.shift;
    mask = phash.mask;
    sentinelTarget = sentinel;
    roundedTabSize = std::max(phash.mask+1, 4U);
    salt = phash.salt;

    // Set hash data
    for (uint32_t i = 0; i < 256; i++) {
        scramble[i] = phash.scramble[i];
    }
    for (uint32_t i = 0; i < phash.mask+1; i++) {
        tab[i] = phash.tab[i];
    }

    dyld3::Array<StringTarget> targetsArray = targets();
    dyld3::Array<StringHashCheckByte> checkBytesArray = checkBytes();

    // Set offsets to the sentinel
    for (uint32_t i = 0; i < phash.capacity; i++) {
        targetsArray[i] = sentinel;
    }
    // Set checkbytes to 0
    for (uint32_t i = 0; i < phash.capacity; i++) {
        checkBytesArray[i] = 0;
    }

    // Set real string offsets and checkbytes
    for (const auto& s : strings) {
        assert(s.second.raw != sentinelTarget);
        uint32_t h = hash(s.first);
        targetsArray[h] = s.second.raw;
        checkBytesArray[h] = checkbyte(s.first);
    }
}

////////////////////////////  ObjCClassOpt ////////////////////////////////////////


template<typename PerfectHashT, typename ImageOffsetT, typename ClassesMapT>
void ObjCClassOpt::write(const PerfectHashT& phash, const Array<std::pair<const char*, ImageOffsetT>>& strings,
           const ClassesMapT& classes, uint32_t preCalculatedDuplicateCount)
{
    ObjCStringTable::write(phash, strings);
    
    __block dyld3::Array<ClassTarget> classOffsetsArray = classOffsets();
    __block dyld3::Array<ClassTarget> duplicateOffsetsArray = duplicateOffsets(preCalculatedDuplicateCount);

    // Set class offsets to 0
    for (uint32_t i = 0; i < capacity; i++) {
        classOffsetsArray[i].raw = dyld3::closure::Image::ObjCImageOffset::sentinelValue;
    }
    
    classes.forEachEntry(^(const char *const &key, const Image::ObjCClassImageOffset **values, uint64_t valuesCount) {
        uint32_t keyIndex = getIndex(key);
        assert(keyIndex != indexNotFound);
        assert(classOffsetsArray[keyIndex].raw == dyld3::closure::Image::ObjCImageOffset::sentinelValue);
        
        if (valuesCount == 1) {
            // Only one entry so write it in to the class offsets directly
            Image::ObjCClassImageOffset classImageOffset = *(values[0]);
            assert(classImageOffset.classData.isDuplicate == 0);
            classOffsetsArray[keyIndex] = classImageOffset;
            return;
        }
        
        // We have more than one value.  We add a placeholder to the class offsets which tells us the head
        // of the linked list of classes in the duplicates array
        uint32_t dest = duplicateCount();
        duplicateCount() += valuesCount;

        Image::ObjCClassImageOffset classImagePlaceholder;
        assert(valuesCount < (1 << 8));
        classImagePlaceholder.duplicateData.count           = (uint32_t)valuesCount;
        classImagePlaceholder.duplicateData.index           = dest;
        classImagePlaceholder.duplicateData.isDuplicate     = 1;
        classOffsetsArray[keyIndex] = classImagePlaceholder;
        
        for (uint64_t i = 0; i != valuesCount; ++i) {
            Image::ObjCClassImageOffset classImageOffset = *(values[i]);
            assert(classImageOffset.classData.isDuplicate == 0);
            duplicateOffsetsArray.push_back(classImageOffset);
        }
    });
}

} // namespace closure
} // namespace dyld3
