//
//  Manifest.h
//  dyld
//
//  Created by Louis Gerbarg on 7/23/15.
//
//

#ifndef Manifest_h
#define Manifest_h

#include <map>
#include <set>
#include <string>
#include <vector>

#include <unordered_map>
#include <unordered_set>


struct SharedCache;
struct MachOProxy;
struct Manifest;

struct Manifest {
    struct Project {
        std::vector<std::string> sources;
    };

    struct File {
        MachOProxy* proxy;

        File( MachOProxy* P ) : proxy( P ) {}
    };

    struct FileSet {
        std::map<std::string, File> dylibs;
        std::map<std::string, File> executables;
    };

    struct Anchor {
                std::string installname;
		bool required;

                Anchor( const std::string& IN ) : installname( IN ) {}
    };

    struct SegmentInfo {
                std::string name;
		uint64_t startAddr;
		uint64_t endAddr;
	};

	struct SegmentInfoHasher {
		std::size_t operator()(const SegmentInfo &x) const {
			return std::hash<std::string>()(x.name) ^ std::hash<uint64_t>()(x.startAddr) ^ std::hash<uint64_t>()(x.endAddr);
		}
	};

	struct CacheInfo {
		std::vector<SegmentInfo> regions;
		std::string cdHash;
	};

	struct DylibInfo {
		bool included;
		std::string exclusionInfo;
		uuid_t uuid;
		std::vector<SegmentInfo> segments;
		DylibInfo(void) : included(true) {}
		void exclude(const std::string& reason) {
			included = false;
			exclusionInfo = reason;
		}
	};

	struct Results {
            std::string failure;
            std::map<std::string, DylibInfo> dylibs;
            std::vector<std::string> warnings;
                CacheInfo developmentCache;
		CacheInfo productionCache;
	};

	struct Architecture {
		std::vector<Anchor> anchors;
		mutable Results results;
		//FIXME: Gross
		std::unordered_map<std::string, std::unordered_set<std::string>> dependents;

		uint64_t hash(void) const {
			if (!_hash) {
				for (auto& dylib : results.dylibs) {
					if (dylib.second.included) {
						_hash ^= std::hash<std::string>()(dylib.first);
						//HACK to get some of the UUID into the hash
						_hash ^= std::hash<uint64_t>()(*(uint64_t *)(&dylib.second.uuid[0]));
					}
				};
			}

			return _hash;
		}

		bool equivalent(const Architecture& O) const {
			if (hash() != O.hash()) {
				return false;
			}
			for (auto& dylib : results.dylibs) {
				if (dylib.second.included) {
					auto Odylib = O.results.dylibs.find(dylib.first);
					if (Odylib == O.results.dylibs.end()
						|| Odylib->second.included == false
						|| memcmp(&Odylib->second.uuid[0], &dylib.second.uuid[0], sizeof(uuid_t)) != 0)
					return false;
				}
			}
			//Iterate over O.results to make sure we included all the same things
			for (auto Odylib : O.results.dylibs) {
				if (Odylib.second.included) {
					auto dylib = results.dylibs.find(Odylib.first);
					if (dylib == results.dylibs.end()
						|| dylib->second.included == false)
						return false;
				}
			}
			return true;
		}
	private:
		mutable uint64_t _hash = 0;
	};

	struct Configuration {
            std::string platformName;
            std::string metabomTag;
            std::set<std::string> metabomExcludeTags;
            std::set<std::string> metabomRestrictTags;
            std::set<std::string> restrictedInstallnames;
            std::map<std::string, Architecture> architectures;

            uint64_t hash( void ) const {
                        if (!_hash) {
				_hash ^= std::hash<size_t>()(architectures.size());
				// We want the preliminary info here to make dedupe decisions
				for (auto& arch : architectures) {
					_hash ^= arch.second.hash();
				};
			}
			return _hash;
		}

		//Used for dedupe
		bool equivalent(const Configuration& O) const {
			if (hash() != O.hash())
				return false;
			for (const auto& arch : architectures) {
				if (O.architectures.count(arch.first) == 0)
					return false;
				if (!arch.second.equivalent(O.architectures.find(arch.first)->second))
					return false;
			}

			return true;
		}
	private:
		mutable uint64_t _hash = 0;
	};

        std::map<std::string, FileSet> architectureFiles;
        std::map<std::string, Project> projects;
        std::map<std::string, Configuration> configurations;
        std::string dylibOrderFile;
        std::string dirtyDataOrderFile;
        std::string metabomFile;
        std::string build;
        // FIXME every needs to adopt platform string for v5
        std::string platform;
        uint32_t manifest_version;
        bool normalized;

        Manifest( void ) {}
#if BOM_SUPPORT
        Manifest( const std::string& path );
        Manifest( const std::string& path, const std::set<std::string>& overlays );
#endif
        void write( const std::string& path );
        void canonicalize( void );
        void calculateClosure( bool enforeceRootless );
		void pruneClosure();
        bool sameContentsAsCacheAtPath( const std::string& configuration, const std::string& architecture,
                                        const std::string& path ) const;
        MachOProxy* dylibProxy( const std::string& installname, const std::string& arch );
        MachOProxy* removeLargestLeafDylib( const std::string& configuration, const std::string& architecture );

private:
 void removeDylib( MachOProxy* proxy, const std::string& reason, const std::string& configuration, const std::string& architecture,
                   std::unordered_set<std::string>& processedInstallnames );
 File* dylibForInstallName( const std::string& installname, const std::string& arch );
 void calculateClosure( const std::string& configuration, const std::string& architecture);
 void pruneClosure(const std::string& configuration, const std::string& architecture);
 void canonicalizeDylib( const std::string& installname );
 template <typename P>
 void canonicalizeDylib( const std::string& installname, const uint8_t* p );
 void addImplicitAliases( void );
};


#endif /* Manifest_h */
