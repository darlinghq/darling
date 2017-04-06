//
//  Manifest.mm
//  dyld
//
//  Created by Louis Gerbarg on 7/23/15.
//
//

#if BOM_SUPPORT
extern "C" {
#include <Bom/Bom.h>
#include <Metabom/MBTypes.h>
#include <Metabom/MBEntry.h>
#include <Metabom/MBMetabom.h>
#include <Metabom/MBIterator.h>
};
#endif /* BOM_SUPPORT */

#include <algorithm>

#include <Foundation/Foundation.h>
#include <rootless.h>

#include "MachOFileAbstraction.hpp"
#include "FileAbstraction.hpp"
#include "Trie.hpp"
#include "Logging.h"

#include <mach-o/loader.h>
#include <mach-o/fat.h>

#include <array>
#include <vector>

#include "Manifest.h"
#include "dsc_iterator.h"

#include "mega-dylib-utils.h"

namespace {
	//FIXME this should be in a class
	static bool rootless = true;

        static inline NSString* cppToObjStr(const std::string& str) { return [NSString stringWithUTF8String:str.c_str()]; }

#if BOM_SUPPORT
        std::string effectivePath(const std::string source, const std::string target)
        {
            if (target[0] == '/')
                return normalize_absolute_file_path(target);

            std::size_t found = source.find_last_of('/');
            return normalize_absolute_file_path(source.substr(0, found) + "/" + target);
        }

        std::string checkSymlink(const std::string path, const std::pair<std::string, std::string>& symlink, const std::set<std::string>& directories)
        {
            if (directories.count(symlink.second) == 0) {
                if (path == symlink.second)
                    return symlink.first;
            } else {
                auto res = std::mismatch(symlink.second.begin(), symlink.second.end(), path.begin());
                if (res.first == symlink.second.end()) {
                    std::string alias = normalize_absolute_file_path(symlink.first + std::string(res.second, path.end()));
                    return alias;
                }
            }
            return "";
        }
#endif
        }

#if BOM_SUPPORT

        Manifest::Manifest(const std::string& path)
            : Manifest(path, std::set<std::string>())
        {
        }

        Manifest::Manifest(const std::string& path, const std::set<std::string>& overlays)
        {
            NSMutableDictionary* manifestDict = [NSMutableDictionary dictionaryWithContentsOfFile:cppToObjStr(path)];
            std::map<std::string, std::string>           metabomTagMap;
            std::map<std::string, std::set<std::string>> metabomExcludeTagMap;
            std::map<std::string, std::set<std::string>> metabomRestrictedTagMap;
            metabomFile = [manifestDict[@"metabomFile"] UTF8String];

            for (NSString* project in manifestDict[@"projects"]) {
                for (NSString* source in manifestDict[@"projects"][project]) {
                    projects[[project UTF8String]].sources.push_back([source UTF8String]);
                }
            }

            for (NSString* configuration in manifestDict[@"configurations"]) {
                std::string configStr = [configuration UTF8String];
                std::string configTag = [manifestDict[@"configurations"][configuration][@"metabomTag"] UTF8String];
                metabomTagMap[configTag] = configStr;

                if (manifestDict[@"configurations"][configuration][@"metabomExcludeTags"]) {
                    for (NSString* excludeTag in manifestDict[@"configurations"][configuration][@"metabomExcludeTags"]) {
                        metabomExcludeTagMap[configStr].insert([excludeTag UTF8String]);
                        configurations[configStr].metabomExcludeTags.insert([excludeTag UTF8String]);
                    }
                }

                if (manifestDict[@"configurations"][configuration][@"metabomRestrictTags"]) {
                    for (NSString* restrictTag in manifestDict[@"configurations"][configuration][@"metabomRestrictTags"]) {
                        metabomRestrictedTagMap[configStr].insert([restrictTag UTF8String]);
                        configurations[configStr].metabomRestrictTags.insert([restrictTag UTF8String]);
                    }
                }

                configurations[configStr].metabomTag = configTag;
                configurations[configStr].platformName =
                    [manifestDict[@"configurations"][configuration][@"platformName"] UTF8String];
            }

            manifest_version = [manifestDict[@"manifest-version"] unsignedIntValue];
            build = [manifestDict[@"build"] UTF8String];
            if (manifestDict[@"dylibOrderFile"]) {
                dylibOrderFile = [manifestDict[@"dylibOrderFile"] UTF8String];
            }
            if (manifestDict[@"dirtyDataOrderFile"]) {
                dirtyDataOrderFile = [manifestDict[@"dirtyDataOrderFile"] UTF8String];
            }

            auto    metabom = MBMetabomOpen(metabomFile.c_str(), false);
            auto    metabomEnumerator = MBIteratorNewWithPath(metabom, ".", "");
            MBEntry entry;

            std::map<std::string, std::string> symlinks;
            std::set<std::string> directories;

            // FIXME error handling (NULL metabom)

            while ((entry = MBIteratorNext(metabomEnumerator))) {
                auto        fsObject = MBEntryGetFSObject(entry);
                std::string entryPath = BOMFSObjectPathName(fsObject);
                if (entryPath[0] == '.') {
                    entryPath.erase(0, 1);
                }
                auto entryType = BOMFSObjectType(fsObject);

                switch (entryType) {
                case BOMFileType: {
                    MBTag tag;
                    auto  tagCount = MBEntryGetNumberOfProjectTags(entry);

                    if (!BOMFSObjectIsBinaryObject(fsObject))
                        break;

                    if (tagCount == 0) {
                        break;
                    } else if (tagCount == 1) {
                        MBEntryGetProjectTags(entry, &tag);
                    } else {
                        MBTag*                tags = (MBTag*)malloc(sizeof(MBTag) * tagCount);
                        MBEntryGetProjectTags(entry, tags);

                        //Sigh, we can have duplicate entries for the same tag, so build a set to work with
                        std::set<std::string> tagStrs;
                        std::map<std::string, MBTag> tagStrMap;
                        for (auto i = 0; i < tagCount; ++i) {
                            tagStrs.insert(MBMetabomGetProjectForTag(metabom, tags[i]));
                            tagStrMap.insert(std::make_pair(MBMetabomGetProjectForTag(metabom, tags[i]), tags[i]));
                        }

                        if (tagStrs.size() > 1) {
                            std::string projects;
                            for (const auto& tagStr : tagStrs) {
                                if (!projects.empty())
                                    projects += ", ";

                                projects += "'" + tagStr + "'";
                            }
                            warning("Bom entry '%s' is claimed by multiple projects: %s, taking first entry", entryPath.c_str(), projects.c_str());
                        }
                        tag = tagStrMap[*tagStrs.begin()];
                        free(tags);
                    }

                    std::string projectName = MBMetabomGetProjectForTag(metabom, tag);

                    // FIXME we need to actually walk down the searchpaths
                    auto project = projects.find(projectName);
                    if (project == projects.end())
                        break;
                    if (project->second.sources.size() == 0)
                        break;
                    std::string projectPath = project->second.sources[0];
                    std::map<std::string, MachOProxy*> proxies;

                    for (const auto& overlay : overlays) {
                        proxies = MachOProxy::findDylibInfo(overlay + "/" + entryPath);
                        if (proxies.size() > 0)
                            break;
                    }

                    if (proxies.size() == 0) {
                        proxies = MachOProxy::findDylibInfo(projectPath + "/" + entryPath);
                    }

                    for (auto& proxy : proxies) {
                        assert(proxy.second != nullptr);
                        if (proxy.second->isExecutable()) {
                            architectureFiles[proxy.first].executables.insert(std::make_pair(entryPath, File(proxy.second)));
                            continue;
                        }
                        if (!proxy.second->isDylib())
                            continue;
                        assert(proxy.second->installName != "");
                        proxy.second->addAlias(entryPath);
                        architectureFiles[proxy.first].dylibs.insert(
                            std::make_pair(proxy.second->installName, File(proxy.second)));
                        auto   tagCount = MBEntryGetNumberOfPackageTags(entry);
                        MBTag* tags = (MBTag*)malloc(sizeof(MBTag) * tagCount);
                        MBEntryGetPackageTags(entry, tags);
                        std::set<std::string> tagStrs;

                        for (auto i = 0; i < tagCount; ++i) {
                            tagStrs.insert(MBMetabomGetPackageForTag(metabom, tags[i]));
                        }

                        for (const auto& tagStr : tagStrs) {
                            // Does the configuration exist
                            auto configuration = metabomTagMap.find(tagStr);
                            if (configuration == metabomTagMap.end())
                                continue;

                            auto restrictions = metabomRestrictedTagMap.find(configuration->second);
                            if (restrictions != metabomRestrictedTagMap.end() && !is_disjoint(restrictions->second, tagStrs)) {
                                configurations[configuration->second].restrictedInstallnames.insert(proxy.second->installName);
                            }
                            // Is the configuration excluded
                            auto exclusions = metabomExcludeTagMap.find(configuration->second);
                            if (exclusions != metabomExcludeTagMap.end() && !is_disjoint(exclusions->second, tagStrs))
                                continue;

                            if ([manifestDict[@"configurations"][cppToObjStr(configuration->second)][@"architectures"]
                                    containsObject:cppToObjStr(proxy.first)]) {
                                configurations[configuration->second.c_str()].architectures[proxy.first].anchors.push_back(
                                    proxy.second->installName);
                            }
                        }

                        free(tags);
                    }
                } break;
                case BOMSymlinkType: {
                    if (!has_prefix(entryPath, "/usr/lib/") && !has_prefix(entryPath, "/System/Library/"))
                        break;
                    const char* target = BOMFSObjectSymlinkTarget(fsObject);
                    if (target) {
                        symlinks[entryPath] = effectivePath(entryPath, target);
                    }
                } break;
                case BOMDirectoryType: {
                    if (!has_prefix(entryPath, "/usr/lib/") && !has_prefix(entryPath, "/System/Library/"))
                        break;
                    directories.insert(entryPath);
                } break;
                default:
                    break;
                }
	}

        MBIteratorFree(metabomEnumerator);
        MBMetabomFree(metabom);

        dispatch_queue_t symlinkQueue = dispatch_get_global_queue(QOS_CLASS_DEFAULT, NULL);
        dispatch_group_t symlinkGroup = dispatch_group_create();

        for (auto& fileSet : architectureFiles) {
            cacheBuilderDispatchGroupAsync(symlinkGroup, symlinkQueue, [&] {
                for (auto& file : fileSet.second.dylibs) {
                    bool aliasAdded = true;
                    auto proxy = file.second.proxy;

                    while (aliasAdded) {
                        aliasAdded = false;

                        for (auto& symlink : symlinks) {
                            std::set<std::string> newAliases;
                            auto                  alias = checkSymlink(proxy->installName, symlink, directories);
                            if (alias != "") {
                                newAliases.insert(alias);
                            }

                            for (auto& existingAlias : proxy->installNameAliases) {
                                alias = checkSymlink(existingAlias, symlink, directories);
                                if (alias != "") {
                                    newAliases.insert(alias);
                                }
                            }

                            for (auto& alias : newAliases) {
                                if (proxy->addAlias(alias)) {
                                    aliasAdded = true;
                                }
                            }
                        }
                    }
                }
            });
        }
        dispatch_group_wait(symlinkGroup, DISPATCH_TIME_FOREVER);

        for (auto& fileSet : architectureFiles) {
            for (auto& file : fileSet.second.dylibs) {
                auto proxy = file.second.proxy;

                for (const auto& dependency : proxy->dependencies) {
                    auto dependencyProxy = dylibProxy(dependency, fileSet.first);
                    if (dependencyProxy == nullptr)
                        break;

                    dependencyProxy->dependents.insert(proxy->installName);
                }
            }
        }
}
#endif

void Manifest::calculateClosure( bool enforceRootless ) {
    rootless = enforceRootless;

    for ( auto& config : configurations ) {
        for ( auto& arch : config.second.architectures ) {
            calculateClosure( config.first, arch.first );
        }
    }
}

Manifest::File* Manifest::dylibForInstallName( const std::string& installname, const std::string& arch ) {
    auto archIter = architectureFiles.find( arch );
    if ( archIter == architectureFiles.end() ) return nullptr;

    auto& files = archIter->second.dylibs;
    auto dylibIterator = files.find( installname );

    if ( dylibIterator != files.end() ) return &dylibIterator->second;

    for ( auto& candidate : files ) {
        if ( candidate.second.proxy->installNameAliases.count( installname ) > 0 ) {
            dylibIterator = files.find( candidate.first );
            return &dylibIterator->second;
        }
        }
	// Check if we can fallback to an interworkable architecture
	std::string fallbackArchStr = fallbackArchStringForArchString( arch );
	if ( !fallbackArchStr.empty() ) {
		return dylibForInstallName( installname, fallbackArchStr );
	}
	
	return nullptr;
}


MachOProxy* Manifest::dylibProxy( const std::string& installname, const std::string& arch ) {
    auto dylib = dylibForInstallName( installname, arch );

    if ( dylib != nullptr ) {
        assert( dylib->proxy != nullptr );
        return dylib->proxy;
    }

    return nullptr;
}

bool
Manifest::sameContentsAsCacheAtPath(const std::string& configuration, const std::string& architecture, const std::string& path) const {
	__block std::set<std::pair<std::string, std::array<char, 16>>> cacheDylibs;
	std::set<std::pair<std::string, std::array<char, 16>>> manifestDylibs;
	struct stat statbuf;
	if ( ::stat(path.c_str(), &statbuf) == -1 ) {
		// <rdar://problem/25912438> don't warn if there is no existing cache file
		if ( errno != ENOENT )
			warning("stat() failed for dyld shared cache at %s, errno=%d", path.c_str(), errno);
		return false;
	}

	int cache_fd = ::open(path.c_str(), O_RDONLY);
	if ( cache_fd < 0 ) {
		warning("open() failed for shared cache file at %s, errno=%d", path.c_str(), errno);
		return false;
	}

	const void *mappedCache = ::mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, cache_fd, 0);
	if (mappedCache == MAP_FAILED) {
		::close(cache_fd);
			warning("mmap() for shared cache at %s failed, errno=%d", path.c_str(), errno);
			return false;
	}
	::close(cache_fd);

	if (configurations.count(configuration) == 0
		|| configurations.find(configuration)->second.architectures.count(architecture) == 0)
		return false;

	for (auto& dylib : configurations.find(configuration)->second.architectures.find(architecture)->second.results.dylibs) {
		if ( dylib.second.included == true) {
			std::pair<std::string, std::array<char, 16>> dylibPair;
			dylibPair.first = dylib.first;
			bcopy((const void *)&dylib.second.uuid[0], &dylibPair.second[0], sizeof(uuid_t));
			manifestDylibs.insert(dylibPair);
                        auto file = architectureFiles.find(architecture)->second.dylibs.find(dylib.first);
                        if (file != architectureFiles.find(architecture)->second.dylibs.end()) {
                            for ( auto& alias : file->second.proxy->installNameAliases ) {
                                std::pair<std::string, std::array<char, 16>> aliasPair;
                                aliasPair.first = alias;
                                        bcopy((const void *)&dylib.second.uuid[0], &aliasPair.second[0], sizeof(uuid_t));
					manifestDylibs.insert(aliasPair);
				}
			}
		}
	}

	(void)dyld_shared_cache_iterate(mappedCache, (uint32_t)statbuf.st_size,
										 ^(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo){
											 std::pair<std::string, std::array<char, 16>> dylibPair;
											 dylibPair.first = dylibInfo->path;
											 bcopy((const void *)&dylibInfo->uuid[0], &dylibPair.second[0], sizeof(uuid_t));
											cacheDylibs.insert(dylibPair);
										 });

	return (manifestDylibs == cacheDylibs);
}

void Manifest::removeDylib( MachOProxy* proxy, const std::string& reason, const std::string& configuration,
                            const std::string& architecture, std::unordered_set<std::string>& processedInstallnames ) {
    auto configIter = configurations.find( configuration );
    if ( configIter == configurations.end() ) return;
    auto archIter = configIter->second.architectures.find( architecture );
    if ( archIter == configIter->second.architectures.end() ) return;
    auto& archManifest = archIter->second;

    if ( archManifest.results.dylibs.count( proxy->installName ) == 0 ) {
        bcopy( &proxy->uuid[0], &archManifest.results.dylibs[proxy->installName].uuid[0], sizeof( uuid_t ) );
        processedInstallnames.insert( proxy->installName );
    }
    archManifest.results.dylibs[proxy->installName].exclude( reason );

    processedInstallnames.insert( proxy->installName );
    for ( auto& alias : proxy->installNameAliases ) {
        processedInstallnames.insert( alias );
    }

    for ( const auto& dependent : proxy->dependents ) {
        auto dependentProxy = dylibProxy( dependent, architecture );
        auto dependentResultIter = archManifest.results.dylibs.find( dependentProxy->installName );
        if ( dependentProxy &&
             ( dependentResultIter == archManifest.results.dylibs.end() || dependentResultIter->second.included == true ) ) {
            removeDylib( dependentProxy, "Missing dependency: " + proxy->installName, configuration, architecture,
                         processedInstallnames );
        }
    }
}

MachOProxy* Manifest::removeLargestLeafDylib( const std::string& configuration, const std::string& architecture ) {
    std::set<std::string> activeInstallnames;
    std::set<MachOProxy*> leafDylibs;

    auto configIter = configurations.find( configuration );
    if ( configIter == configurations.end() ) terminate( "Internal error" );
    ;
    auto archIter = configIter->second.architectures.find( architecture );
    if ( archIter == configIter->second.architectures.end() ) terminate( "Internal error" );
    ;
    for ( const auto& dylibInfo : archIter->second.results.dylibs ) {
        if ( dylibInfo.second.included ) {
            activeInstallnames.insert( dylibInfo.first );
        }
    }
    for ( const auto& installname : activeInstallnames ) {
        auto dylib = dylibProxy( installname, architecture );
        bool dependents = false;
        for ( const auto& depedent : dylib->dependents ) {
            if ( depedent != dylib->installName && activeInstallnames.count( depedent ) ) {
                dependents = true;
                break;
            }
        }
        if ( !dependents ) {
            leafDylibs.insert( dylib );
        }
    }
    if ( leafDylibs.empty() ) {
        terminate( "No leaf dylibs to evict" );
    }
    MachOProxy* largestLeafDylib = nullptr;
    for ( const auto& dylib : leafDylibs ) {
        if ( largestLeafDylib == nullptr || dylib->fileSize > largestLeafDylib->fileSize ) {
            largestLeafDylib = dylib;
        }
    }
    std::unordered_set<std::string> empty;
    removeDylib( largestLeafDylib, "VM space overflow", configuration, architecture, empty );
    return largestLeafDylib;
}

static void recursiveInvalidate(const std::string& invalidName, std::unordered_map<std::string, std::unordered_set<std::string>>& usesOf, std::unordered_set<std::string>& unusableInstallNames)
{
	if ( unusableInstallNames.count(invalidName) )
		return;
	unusableInstallNames.insert(invalidName);
	for (const std::string& name : usesOf[invalidName] ) {
		recursiveInvalidate(name, usesOf, unusableInstallNames);
	}
}

void Manifest::pruneClosure()
{
	for (auto& config : configurations) {
		for (auto& arch : config.second.architectures) {
			pruneClosure(config.first, arch.first);
		}
    }
}

void Manifest::pruneClosure(const std::string& configuration, const std::string& architecture)
{
	auto configIter = configurations.find(configuration);
	if ( configIter == configurations.end() )
		return;
	auto archIter = configIter->second.architectures.find(architecture);
	if ( archIter == configIter->second.architectures.end() )
		return;
	auto& archManifest = archIter->second;

	// build reverse dependency map and list of excluded dylibs
	std::unordered_map<std::string, std::unordered_set<std::string>> reverseDep;
	std::unordered_set<std::string> unusableStart;
	for (const auto& dylib : archManifest.results.dylibs) {
		const std::string dylibInstallName = dylib.first;
		if ( dylib.second.included ) {
			if ( MachOProxy* proxy = dylibProxy(dylibInstallName, architecture) ) {
				for (const std::string& dependentPath : proxy->dependencies) {
					reverseDep[dependentPath].insert(dylibInstallName);
				}
			}
		}
		else {
			unusableStart.insert(dylibInstallName);
		}
	}

	// mark unusable, all dylibs depending on the initially unusable dylibs
	std::unordered_set<std::string> newUnusable;
	for (const std::string& unusable : unusableStart) {
		recursiveInvalidate(unusable, reverseDep, newUnusable);
	}

	// remove unusable dylibs from manifest
	std::unordered_set<std::string> dummy;
	for (const std::string& unusable : newUnusable) {
		if ( MachOProxy* proxy = dylibProxy(unusable, architecture) )
			removeDylib(proxy, "Missing dependency: " + unusable, configuration, architecture, dummy);
		warning("can't use: %s because dependent dylib cannot be used", unusable.c_str());
	}
}

void Manifest::calculateClosure( const std::string& configuration, const std::string& architecture ) {
    auto& archManifest = configurations[configuration].architectures[architecture];
    auto archFileIter = architectureFiles.find( architecture );
    assert( archFileIter != architectureFiles.end() );
    auto files = archFileIter->second.dylibs;

    std::unordered_set<std::string> newInstallnames;

    for ( auto& anchor : archManifest.anchors ) {
                newInstallnames.insert(anchor.installname);
	}

	std::unordered_set<std::string> processedInstallnames;

	while (!newInstallnames.empty()) {
		std::unordered_set<std::string> installnamesToProcess = newInstallnames;
		newInstallnames.clear();

		for (const std::string& installname : installnamesToProcess) {
			if (processedInstallnames.count(installname) > 0) {
				continue;
			}

            auto proxy = dylibProxy( installname, architecture );

            if ( proxy == nullptr ) {
                // No path
                archManifest.results.dylibs[installname].exclude( "Could not find file for install name" );
                warning("Could not find file for install name (%s)", installname.c_str());
                continue;
            }

            if (configurations[configuration].restrictedInstallnames.count(installname) != 0) {
                removeDylib(proxy, "Dylib '" + installname + "' removed due to explict restriction", configuration, architecture,
                    processedInstallnames);
                continue;
            }
            // Validate we have all are depedencies
            for ( const auto& dependency : proxy->dependencies ) {
                if ( !dylibProxy( dependency, architecture ) ) {
                    removeDylib( proxy, "Missing dependency: " + dependency, configuration, architecture,
                                 processedInstallnames );
                    break;
                }
            }

            // assert(info->installName == installname);
            if ( archManifest.results.dylibs.count( proxy->installName ) == 0 ) {
                bcopy( &proxy->uuid[0], &archManifest.results.dylibs[proxy->installName].uuid[0], sizeof( uuid_t ) );
                processedInstallnames.insert( proxy->installName );

                auto fileIter = files.find( proxy->installName );
                if ( fileIter != files.end() ) {
                    for ( auto& aliasName : fileIter->second.proxy->installNameAliases ) {
                        processedInstallnames.insert( aliasName );
                    }
                }
            }

            for ( const auto& dependency : proxy->dependencies ) {
                if ( processedInstallnames.count( dependency ) == 0 ) {
                    newInstallnames.insert( dependency );
                }
            }
        }
	}
}

void Manifest::write( const std::string& path ) {
    if ( path.empty() ) return;

    NSMutableDictionary* cacheDict = [[NSMutableDictionary alloc] init];
    NSMutableDictionary* projectDict = [[NSMutableDictionary alloc] init];
    NSMutableDictionary* configurationsDict = [[NSMutableDictionary alloc] init];
    NSMutableDictionary* resultsDict = [[NSMutableDictionary alloc] init];

    cacheDict[@"manifest-version"] = @( manifest_version );
    cacheDict[@"build"] = cppToObjStr( build );
    cacheDict[@"dylibOrderFile"] = cppToObjStr( dylibOrderFile );
    cacheDict[@"dirtyDataOrderFile"] = cppToObjStr( dirtyDataOrderFile );
    cacheDict[@"metabomFile"] = cppToObjStr( metabomFile );

    cacheDict[@"projects"] = projectDict;
    cacheDict[@"results"] = resultsDict;
    cacheDict[@"configurations"] = configurationsDict;

    for ( const auto& project : projects ) {
        NSMutableArray* sources = [[NSMutableArray alloc] init];

        for ( const auto& source : project.second.sources ) {
            [sources addObject:cppToObjStr( source )];
        }

        projectDict[cppToObjStr( project.first )] = sources;
    }

    for ( auto& configuration : configurations ) {
        NSMutableArray* archArray = [[NSMutableArray alloc] init];
        for ( auto& arch : configuration.second.architectures ) {
            [archArray addObject:cppToObjStr( arch.first )];
        }

        NSMutableArray* excludeTags = [[NSMutableArray alloc] init];
        for ( const auto& excludeTag : configuration.second.metabomExcludeTags ) {
            [excludeTags addObject:cppToObjStr( excludeTag )];
        }

        configurationsDict[cppToObjStr( configuration.first )] = @{
            @"platformName" : cppToObjStr( configuration.second.platformName ),
            @"metabomTag" : cppToObjStr( configuration.second.metabomTag ),
            @"metabomExcludeTags" : excludeTags,
            @"architectures" : archArray
        };
    }

    for ( auto& configuration : configurations ) {
        NSMutableDictionary* archResultsDict = [[NSMutableDictionary alloc] init];
        for ( auto& arch : configuration.second.architectures ) {
            NSMutableDictionary* dylibsDict = [[NSMutableDictionary alloc] init];
            NSMutableArray* warningsArray = [[NSMutableArray alloc] init];
            NSMutableDictionary* devRegionsDict = [[NSMutableDictionary alloc] init];
            NSMutableDictionary* prodRegionsDict = [[NSMutableDictionary alloc] init];
                        NSString *prodCDHash = cppToObjStr(arch.second.results.productionCache.cdHash);
			NSString *devCDHash = cppToObjStr(arch.second.results.developmentCache.cdHash);

                        for ( auto& dylib : arch.second.results.dylibs ) {
                            NSMutableDictionary* dylibDict = [[NSMutableDictionary alloc] init];
                            if ( dylib.second.included ) {
                                NSMutableDictionary* segments = [[NSMutableDictionary alloc] init];
                                dylibDict[@"included"] = @YES;
                                for ( auto& segment : dylib.second.segments ) {
                                    segments[cppToObjStr( segment.name )] =
                                        @{ @"startAddr" : @( segment.startAddr ),
                                           @"endAddr" : @( segment.endAddr ) };
                                }
                                dylibDict[@"segments"] = segments;
                            } else {
                                dylibDict[@"included"] = @NO;
                                dylibDict[@"exclusionInfo"] = cppToObjStr(dylib.second.exclusionInfo);
                            }
                            dylibsDict[cppToObjStr( dylib.first )] = dylibDict;
                        }

                        for ( auto& region : arch.second.results.developmentCache.regions ) {
                            devRegionsDict[cppToObjStr( region.name )] =
                                @{ @"startAddr" : @( region.startAddr ),
                                   @"endAddr" : @( region.endAddr ) };
                        }

                        for ( auto& region : arch.second.results.productionCache.regions ) {
                            prodRegionsDict[cppToObjStr( region.name )] =
                                @{ @"startAddr" : @( region.startAddr ),
                                   @"endAddr" : @( region.endAddr ) };
                        }

                        for ( auto& warning : arch.second.results.warnings ) {
                            [warningsArray addObject:cppToObjStr( warning )];
                        }

                        BOOL built = arch.second.results.failure.empty();
                        archResultsDict[cppToObjStr( arch.first )] = @{
                            @"dylibs" : dylibsDict,
                            @"built" : @( built ),
                            @"failure" : cppToObjStr( arch.second.results.failure ),
                            @"productionCache" : @{@"cdhash" : prodCDHash, @"regions" : prodRegionsDict},
                            @"developmentCache" : @{@"cdhash" : devCDHash, @"regions" : devRegionsDict},
                            @"warnings" : warningsArray
                        };
        }
        resultsDict[cppToObjStr( configuration.first )] = archResultsDict;
    }

    NSError* error = nil;
        NSData *outData = [NSPropertyListSerialization dataWithPropertyList:cacheDict
																 format:NSPropertyListBinaryFormat_v1_0
																options:0
																  error:&error];
	(void)[outData writeToFile:cppToObjStr(path) atomically:YES];
}
