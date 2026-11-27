/*
This file is part of Darling.

Copyright (C) 2012-2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <CoreServices/FileManager.h>
#include <CarbonCore/Folders.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <memory>
#include <errno.h>
#include <iconv.h>
#include <alloca.h>
#include <limits.h>
#include <libgen.h>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <regex.h>
#include <fcntl.h>
#include <vector>
#include <CarbonCore/DateTimeUtils.h>
#include <CarbonCore/Resources.h>
#include <sys/xattr.h>
#include <sys/time.h>
#include <pwd.h>
#include <sys/mount.h>
#include <mutex>
#include <errno.h>
#include <darling/emulation/linux_premigration/ext/file_handle.h>

#define STUB() printf("STUB %s\n", __PRETTY_FUNCTION__);

// Is the current user member of the specified group?
static bool hasgid(gid_t gid);

#if !__LP64__
// Fills in the FSSpec of an item (the volume, the directory that holds it and its name)
static OSErr fsSpecForPath(const std::string& path, FSSpec* spec);
#endif

OSStatus FSPathMakeRef(const uint8_t* path, FSRef* fsref, Boolean* isDirectory)
{
	return FSPathMakeRefWithOptions(path, kFSPathMakeRefDoNotFollowLeafSymlink, fsref, isDirectory); 
}

OSStatus FSPathMakeRefWithOptions(const uint8_t* path, long options, FSRef* fsref, Boolean* isDirectory)
{
	if (!path || !fsref)
		return paramErr;

	int flags = 1;
	if (options & kFSPathMakeRefDoNotFollowLeafSymlink)
		flags = 0;

	int err = sys_name_to_handle((const char*) path, (RefData*) fsref, flags);
	if (err != 0)
		return fnfErr;

	if (isDirectory)
	{
		struct stat st;

		if (options & kFSPathMakeRefDoNotFollowLeafSymlink)
			err = lstat((const char*) path, &st);
		else
			err = stat((const char*) path, &st);

		if (err == 0)
			*isDirectory = S_ISDIR(st.st_mode);
		else
			*isDirectory = 0;
	}

	return noErr; 
}

bool FSRefMakePath(const FSRef* fsref, std::string& out)
{
	char name[4096];
	int ret = sys_handle_to_name((RefData*) fsref, name);
	if (ret != 0)
		return false;

	out = name;
	return true;
}

OSStatus FSCompareFSRefs(const FSRef* ref1, const FSRef* ref2)
{
	if (!ref1 || !ref2)
		return paramErr;

	std::string path1, path2;
	struct stat st1, st2;

	if (!FSRefMakePath(ref1, path1) || !FSRefMakePath(ref2, path2))
		return fnfErr;

	if (lstat(path1.c_str(), &st1) != 0 || lstat(path2.c_str(), &st2) != 0)
		return fnfErr;

	if (st1.st_dev != st2.st_dev)
		return diffVolErr;

	return (st1.st_ino == st2.st_ino) ? noErr : errFSRefsDifferent;
}

bool FSRefParamMakePath(const FSRefParam* param, std::string& out)
{
	std::string dir;
	CFStringRef str;
	std::unique_ptr<char[]> buf;
	size_t bufsize;
	Boolean success;
	
	if (!FSRefMakePath(param->ref, dir))
		return false;
	
	out = dir;
	if (out.back() != '/')
		out += '/';
	
	str = CFStringCreateWithCharacters(NULL, param->name, param->nameLength);
	bufsize = CFStringGetLength(str)*4+1;
	buf.reset(new char[bufsize]);
	
	success = CFStringGetCString(str, buf.get(), bufsize, kCFStringEncodingUTF8);
	CFRelease(str);
	
	if (success)
	{
		out += buf.get();
		return true;
	}
	else
		return false;
}

OSStatus FSDeleteObject(const FSRef* fsref)
{
	std::string path;
	if (FSRefMakePath(fsref, path))
	{
		if (::unlink(path.c_str()) == -1)
			return makeOSStatus(errno);
		return noErr;
	}
	return fnfErr;
}

OSStatus FSRefMakePath(const FSRef* fsref, uint8_t* path, uint32_t maxSize)
{
	std::string rpath;

	if (!fsref || !path || !maxSize)
		return paramErr;

	if (!FSRefMakePath(fsref, rpath))
		return fnfErr;

	strncpy((char*) path, rpath.c_str(), maxSize);
	path[maxSize-1] = 0;

	return noErr;
}

// Darling presents the root file system as the one volume. It carries the reference number the
// startup volume has on macOS and the volume name below.
static const FSVolumeRefNum kBootVolumeRefNum = -100;
static const char kBootVolumeName[] = "Macintosh HD";

// Node IDs of directories handed out by FSGetCatalogInfo, so that a directory ID can be turned
// back into a path (PBMakeFSRefSync). Only directories this process has looked at can be found.
static std::map<uint32_t, std::string> g_dirNodeIDs;
static std::mutex g_dirNodeIDsLock;

// The root directory has node ID fsRtDirID and parent ID fsRtParID; other items use their inode
// number, the closest thing to a catalog node ID.
static void nodeIDsForPath(const std::string& path, const struct stat& st, uint32_t* nodeID, uint32_t* parentID)
{
	std::lock_guard<std::mutex> lock(g_dirNodeIDsLock);

	if (path == "/")
	{
		*nodeID = fsRtDirID;
		*parentID = fsRtParID;
		g_dirNodeIDs[fsRtDirID] = path;
		return;
	}

	*nodeID = (uint32_t) st.st_ino;
	if (S_ISDIR(st.st_mode))
		g_dirNodeIDs[*nodeID] = path;

	size_t slash = path.find_last_of('/');
	std::string parent = (slash == std::string::npos || slash == 0) ? std::string("/") : path.substr(0, slash);
	struct stat pst;
	if (parent == "/")
	{
		*parentID = fsRtDirID;
		g_dirNodeIDs[fsRtDirID] = parent;
	}
	else if (::stat(parent.c_str(), &pst) == 0)
	{
		*parentID = (uint32_t) pst.st_ino;
		g_dirNodeIDs[*parentID] = parent;
	}
	else
		*parentID = 0;
}

static bool dirNodeIDToPath(uint32_t nodeID, std::string& path)
{
	std::lock_guard<std::mutex> lock(g_dirNodeIDsLock);
	auto it = g_dirNodeIDs.find(nodeID);
	if (it == g_dirNodeIDs.end())
		return false;
	path = it->second;
	return true;
}

OSStatus FSGetCatalogInfo(const FSRef* ref, uint32_t infoBits, FSCatalogInfo* infoOut, HFSUniStr255* nameOut, FSSpecPtr fsspec, FSRef* parentDir)
{
	std::string path;

	if (!FSRefMakePath(ref, path))
		return fnfErr;

	if (nameOut)
	{
		// The name is the item's own name (the last path component), counted in UTF-16 units.
		size_t slash = path.find_last_of('/');
		std::string leaf = (slash == std::string::npos || slash + 1 == path.length()) ? path : path.substr(slash + 1);
		CFStringRef cfstr = CFStringCreateWithCString(NULL, leaf.c_str(), kCFStringEncodingUTF8);

		nameOut->length = 0;
		if (cfstr)
		{
			nameOut->length = std::min<CFIndex>(CFStringGetLength(cfstr), 255);
			CFStringGetCharacters(cfstr, CFRangeMake(0, nameOut->length), nameOut->unicode);
			CFRelease(cfstr);
		}
	}

	if (parentDir)
	{
		/*
		memcpy(parentDir, ref, sizeof(FSRef));
		ino_t* last = std::find(parentDir->inodes, parentDir->inodes+FSRef_MAX_DEPTH, 0);

		if (last != parentDir->inodes)
			*(last-1) = 0;
		*/
		// TODO
	}

	if (infoOut && infoBits != kFSCatInfoNone)
	{
		struct stat st;

		memset(infoOut, 0, sizeof(*infoOut));

		if (::stat(path.c_str(), &st) != 0)
			return makeOSStatus(errno);

		if (infoBits & kFSCatInfoNodeFlags)
		{
			if (S_ISDIR(st.st_mode))
				infoOut->nodeFlags = kFSNodeIsDirectoryMask;
		}

		if (infoBits & kFSCatInfoVolume)
			infoOut->volume = kBootVolumeRefNum;

		if (infoBits & (kFSCatInfoParentDirID|kFSCatInfoNodeID))
		{
			uint32_t nodeID, parentID;
			nodeIDsForPath(path, st, &nodeID, &parentID);
			if (infoBits & kFSCatInfoNodeID)
				infoOut->nodeID = nodeID;
			if (infoBits & kFSCatInfoParentDirID)
				infoOut->parentDirID = parentID;
		}

		if (infoBits & kFSCatInfoDataSizes)
		{
			infoOut->dataLogicalSize = st.st_size;
			infoOut->dataPhysicalSize = st.st_blocks*512;
		}
		
		int uaccess;
		
		if (st.st_uid == getuid())
			uaccess = st.st_mode & 0700;
		else if (hasgid(st.st_gid))
			uaccess = st.st_mode & 070;
		else
			uaccess = st.st_mode & 07;

		if (infoBits & kFSCatInfoPermissions)
		{
			const uid_t uid = getuid();

			infoOut->permissions.userID = st.st_uid;
			infoOut->permissions.groupID = st.st_gid;
			infoOut->permissions.mode = st.st_mode & 07777;
			infoOut->permissions.userAccess = uaccess;
		}

		if (infoBits & kFSCatInfoUserPrivs)
		{
			if (!(uaccess & 2))
				infoOut->userPrivileges |= 0x4; // kioACUserNoMakeChangesMask
			if (getuid() != st.st_uid)
				infoOut->userPrivileges |= 0x80; // kioACUserNotOwnerMask
		}
		if (infoBits & kFSCatInfoCreateDate)
			infoOut->createDate = Darling::time_tToUTC(st.st_ctime);
		if (infoBits & kFSCatInfoContentMod)
			infoOut->attributeModDate = infoOut->contentModDate = Darling::time_tToUTC(st.st_mtime);
		if (infoBits & kFSCatInfoAccessDate)
			infoOut->accessDate = Darling::time_tToUTC(st.st_atime);
	}

#if !__LP64__
	if (fsspec)
		return fsSpecForPath(path, fsspec);
#endif
	return noErr;
}

OSErr FSSetCatalogInfo(const FSRef* ref, FSCatalogInfoBitmap whichInfo, const FSCatalogInfo* catalogInfo)
{
	std::string path;

	// QUIRK: a NULL catalogInfo with a non-empty whichInfo fails with errFSMissingCatInfo.
	// Not documented. Evidence: black-box observation on macOS 27.0.1 (tier 2),
	// vd-upstream/mac-probes/v122-rescreate-probe.c.
	// Test: tests/src/file_manager_create_res_file.c
	if (!catalogInfo && whichInfo != 0)
		return errFSMissingCatInfo;
	if (!FSRefMakePath(ref, path))
		return fnfErr;

	// Only the POSIX permissions are applied; other catalog fields have no Linux equivalent here.
	if (catalogInfo && (whichInfo & kFSCatInfoPermissions))
	{
		if (::chmod(path.c_str(), catalogInfo->permissions.mode & 07777) != 0)
			return makeOSStatus(errno);
	}

	return noErr;
}

bool hasgid(gid_t gid)
{
	gid_t* gids;
	int count;

	if (getegid() == gid)
		return true;

	count = getgroups(0, nullptr);
	if (count <= 0)
		return false;

	gids = (gid_t*) alloca(sizeof(gid_t)*count);
	if (getgroups(count, gids) != count)
		return false;

	return std::find(gids, gids+count, gid) != (gids+count);
}

static std::string getUserDirsConfigPath()
{
	std::stringstream ss;
	const char *home, *config;
	
	config = getenv("XDG_CONFIG_HOME");
	
	if (!config)
	{
		home = getenv("HOME");
		if (!home)
			return std::string();
	
		ss << home << "/.config";
	}
	else
		ss << config;
	ss << "user-dirs.dirs";
	
	return ss.str();
}

static std::string extractXDGValue(const char* home, const std::string& line)
{
	regmatch_t matches[2];
	static regex_t regexp = []() -> regex_t {
		regex_t re;
		regcomp(&re, "=\"([^\"]+)\"", REG_EXTENDED);
		return re;
	}();
	
	if (regexec(&regexp, line.c_str(), 2, matches, 0) == 0)
	{
		std::string value = line.substr(matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
		size_t pos = value.find("$HOME");
		
		if (pos != std::string::npos)
			value.replace(pos, 5, home);
		return value;
	}
	
	return std::string();
}

static OSErr fsSpecError(int err)
{
	switch (err)
	{
		case ENOENT: return fnfErr;
		case EACCES: case EPERM: return permErr;
		case EROFS: return wPrErr;
		case EEXIST: return dupFNErr;
		default: return makeOSStatus(err);
	}
}

// Makes the FSRef of a folder path, creating the folder when asked and it is missing.
static OSStatus folderRefAtPath(
    const char* path,Boolean createFolder,FSRef* location)
{
    if (!path || !*path || !location)
        return paramErr;

    struct stat st;

    if (stat(path,&st)!=0) {
        int error=errno;

        if (!createFolder || error!=ENOENT)
            return fsSpecError(error);

        if (mkdir(path,0777)!=0 && errno!=EEXIST)
            return fsSpecError(errno);

        if (stat(path,&st)!=0)
            return fsSpecError(errno);
    }

    if (!S_ISDIR(st.st_mode))
        return dirNFErr;

    FSRef candidate={};
    Boolean directory=false;

    OSStatus err=FSPathMakeRefWithOptions(
        (const uint8_t*)path,0,&candidate,&directory);

    if (err!=noErr)
        return err;

    if (!directory)
        return dirNFErr;

    *location=candidate;

    return noErr;
}

OSStatus FSFindFolder(long vRefNum, OSType folderType, Boolean createFolder, FSRef* location)
{
	if (folderType == kTemporaryFolderType)
	{
		const char* tmpdir = getenv("TMPDIR");
		
		if (!tmpdir)
			tmpdir = "/tmp";
		
		return folderRefAtPath(tmpdir,createFolder,location);
	}
	else if (folderType == kDesktopFolderType || folderType == kSystemDesktopFolderType)
	{
		std::string xdgConfig = getUserDirsConfigPath();
		std::ifstream cfg(xdgConfig.c_str());
		std::string line, desktop;
		const char* home = getenv("HOME");
		
		if (!home || !*home)
			return fnfErr;
		desktop = home;
		desktop += "/Desktop";
	
		if (cfg.is_open())
		{
			while (std::getline(cfg, line))
			{
				if (line.compare(0, 17, "XDG_DESKTOP_DIR=\"") == 0)
				{
					std::string d = extractXDGValue(home, line);
					if (!d.empty())
						desktop = d;
					
					break;
				}
			}
		}
		
		return folderRefAtPath(desktop.c_str(),createFolder,location);
	}
	else
		return unimpErr;
}

OSErr PBCreateDirectoryUnicodeSync(FSRefParam* paramBlock)
{
	std::string path;
	
	if (!FSRefParamMakePath(paramBlock, path))
		return fnfErr;
	if (mkdir(path.c_str(), 0777) == -1)
		return makeOSStatus(errno);
	
	if (paramBlock->newRef)
		FSPathMakeRef((uint8_t*) path.c_str(), paramBlock->newRef, nullptr);
	
	return noErr;
}

OSErr PBCreateFileUnicodeSync(FSRefParam* paramBlock)
{
	std::string path;
	
	if (!FSRefParamMakePath(paramBlock, path))
		return fnfErr;
	int fd = open(path.c_str(), O_CREAT|O_EXCL, 0666);
	if (fd == -1)
		return makeOSStatus(errno);
	close(fd);
	
	if (paramBlock->newRef)
		FSPathMakeRef((uint8_t*) path.c_str(), paramBlock->newRef, nullptr);
	
	return noErr;
}

OSErr PBGetCatalogInfoSync(FSRefParam *paramBlock)
{
	STUB();
	return unimpErr;
}

OSErr PBMakeFSRefUnicodeSync(FSRefParam *paramBlock)
{
	std::string path;
	
	if (!paramBlock->newRef)
		return paramErr;
	if (!FSRefParamMakePath(paramBlock, path))
		return fnfErr;
	FSPathMakeRef((uint8_t*) path.c_str(), paramBlock->newRef, nullptr);
	
	return noErr;
}

// Forks opened with FSOpenFork. The data fork is the file itself; the resource fork is the
// com.apple.ResourceFork extended attribute (as used by the Resource Manager), read into memory.
// Refnums start high so they don't collide with Resource Manager refnums, which FSCloseFork also accepts.
struct OpenFork
{
	int fd = -1;
	std::vector<uint8_t> data;
	SInt64 position = 0; // current mark of an in-memory (resource) fork; a data fork uses the fd offset
};

static std::map<FSIORefNum, OpenFork> g_openForks;
static std::mutex g_openForksLock;
static FSIORefNum g_nextForkRefNum = 0x4000;

// QUIRK: opening the resource fork of a file that has none fails with eofErr, a fork name other than
// the data and resource fork names with errFSBadForkName, and closing a reference number that is not
// open with fnOpnErr (FSGetForkSize reports rfNumErr for it).
// Not documented. Evidence: black-box observation on macOS 27.0.1, arm64 and x86_64 (tier 2),
// vd-upstream/mac-probes/v123-fork-probe.c.
// Test: tests/src/file_manager_forks.c
OSErr FSOpenFork(const FSRef* ref, UniCharCount forkNameLength, const UniChar* forkName, SInt8 permissions, FSIORefNum* forkRefNum)
{
	std::string path;
	OpenFork fork;

	if (!ref || !forkRefNum || (forkNameLength != 0 && !forkName))
		return paramErr;
	if (!FSRefMakePath(ref, path))
		return fnfErr;

	if (forkNameLength == 0)
	{
		int flags;
		switch (permissions & 3)
		{
			case fsWrPerm: flags = O_WRONLY; break;
			case fsRdWrPerm: flags = O_RDWR; break;
			default: flags = O_RDONLY; break;
		}
		// QUIRK: FSReadFork succeeds on a fork opened with write-only permission, so such a fork is
		// opened for reading as well when the file allows it.
		// Not documented. Evidence: black-box observation on macOS 10.14.4, i386 and x86_64 (tier 2),
		// vd-upstream/mac-probes/v665-fs1.c.
		// Test: tests/src/file_manager_iterators.c
		if (flags == O_WRONLY)
			fork.fd = ::open(path.c_str(), O_RDWR);
		if (fork.fd == -1)
			fork.fd = ::open(path.c_str(), flags);
		if (fork.fd == -1)
			return makeOSStatus(errno);

		// A directory has no data fork.
		struct stat st;
		if (::fstat(fork.fd, &st) == 0 && S_ISDIR(st.st_mode))
		{
			::close(fork.fd);
			return errFSForkNotFound;
		}
	}
	else
	{
		HFSUniStr255 rsrcForkName;
		FSGetResourceForkName(&rsrcForkName);
		if (rsrcForkName.length != forkNameLength || memcmp(forkName, rsrcForkName.unicode, 2 * forkNameLength) != 0)
			return errFSBadForkName;

		ssize_t size = ::getxattr(path.c_str(), "com.apple.ResourceFork", nullptr, 0, 0, 0);
		if (size < 0)
			return (errno == ENOATTR) ? eofErr : makeOSStatus(errno);
		fork.data.resize(size);
		if (size > 0 && ::getxattr(path.c_str(), "com.apple.ResourceFork", fork.data.data(), size, 0, 0) != size)
			return ioErr;
	}

	std::lock_guard<std::mutex> lock(g_openForksLock);
	while (g_openForks.count(g_nextForkRefNum))
		g_nextForkRefNum = (g_nextForkRefNum == 0x7fff) ? 0x4000 : g_nextForkRefNum + 1;
	*forkRefNum = g_nextForkRefNum;
	g_openForks.emplace(g_nextForkRefNum, std::move(fork));
	g_nextForkRefNum = (g_nextForkRefNum == 0x7fff) ? 0x4000 : g_nextForkRefNum + 1;
	return noErr;
}

OSErr FSGetForkSize(FSIORefNum forkRefNum, SInt64* forkSize)
{
	if (!forkSize)
		return paramErr;

	std::lock_guard<std::mutex> lock(g_openForksLock);
	auto it = g_openForks.find(forkRefNum);
	if (it == g_openForks.end())
		return rfNumErr;

	if (it->second.fd == -1)
	{
		*forkSize = it->second.data.size();
		return noErr;
	}

	struct stat st;
	if (::fstat(it->second.fd, &st) != 0)
		return makeOSStatus(errno);
	*forkSize = st.st_size;
	return noErr;
}

OSErr FSCloseFork(FSIORefNum forkRefNum)
{
	{
		std::lock_guard<std::mutex> lock(g_openForksLock);
		auto it = g_openForks.find(forkRefNum);
		if (it != g_openForks.end())
		{
			if (it->second.fd != -1)
				::close(it->second.fd);
			g_openForks.erase(it);
			return noErr;
		}
	}

	// Resource files opened with FSOpenResourceFile(Mapped) are closed with FSCloseFork too.
	CloseResFile(forkRefNum);
	return (ResError() == noErr) ? noErr : fnOpnErr;
}

OSErr PBOpenForkSync(FSForkIOParam *paramBlock)
{
	STUB();
	return unimpErr;
}

OSErr PBReadForkSync(FSForkIOParam *paramBlock)
{
	STUB();
	return unimpErr;
}

OSErr PBWriteForkSync(FSForkIOParam *paramBlock)
{
	STUB();
	return unimpErr;
}

OSErr PBIterateForksSync(FSForkIOParam *paramBlock)
{
	STUB();
	return unimpErr;
}

OSErr PBCloseForkSync(FSForkIOParam *paramBlock)
{
	STUB();
	return unimpErr;
}


// ---- Fork position and reading --------------------------------------------------------------

static SInt64 forkSize(const OpenFork& fork)
{
	if (fork.fd == -1)
		return (SInt64) fork.data.size();
	struct stat st;
	return (::fstat(fork.fd, &st) == 0) ? (SInt64) st.st_size : -1;
}

static SInt64 forkMark(const OpenFork& fork)
{
	if (fork.fd == -1)
		return fork.position;
	return (SInt64) ::lseek(fork.fd, 0, SEEK_CUR);
}

static void forkSetMark(OpenFork& fork, SInt64 position)
{
	if (fork.fd == -1)
		fork.position = position;
	else
		::lseek(fork.fd, (off_t) position, SEEK_SET);
}

// Resolves a positioning mode and offset to a new mark. `size` is the logical end of fork.
// Only the low two bits of the mode select the base; the caching and newline flags and any other
// bits do not matter.
enum ForkPositionResult { kForkPosOK, kForkPosBeforeStart, kForkPosPastEnd };

static ForkPositionResult resolveForkPosition(SInt64 mark, SInt64 size, UInt16 positionMode, SInt64 offset, SInt64* target)
{
	switch (positionMode & 3)
	{
		case fsFromStart: *target = offset; break;
		case fsFromLEOF: *target = size + offset; break;
		case fsFromMark: *target = mark + offset; break;
		default: *target = mark; break; // fsAtMark: the offset is ignored
	}

	if (*target < 0)
		return kForkPosBeforeStart;
	if (*target > size)
		return kForkPosPastEnd;
	return kForkPosOK;
}

OSErr FSGetForkPosition(FSIORefNum forkRefNum, SInt64* position)
{
	if (!position)
		return paramErr;

	std::lock_guard<std::mutex> lock(g_openForksLock);
	auto it = g_openForks.find(forkRefNum);
	if (it == g_openForks.end())
		return fnOpnErr;

	*position = forkMark(it->second);
	return noErr;
}

// QUIRK: moving the mark of a fork with a positioning mode that lands before the start of the
// fork fails with posErr and leaves the mark at 0, except fsFromMark, which fails with eofErr and
// leaves the mark at the end of the fork; landing after the end fails with eofErr and leaves the
// mark at the end. Not documented. Evidence: black-box observation on macOS 10.14.4, i386 and
// x86_64 (tier 2), vd-upstream/mac-probes/v665-fs1.c.
// Test: tests/src/file_manager_iterators.c (fork position tests)
OSErr FSSetForkPosition(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset)
{
	std::lock_guard<std::mutex> lock(g_openForksLock);
	auto it = g_openForks.find(forkRefNum);
	if (it == g_openForks.end())
		return fnOpnErr;

	OpenFork& fork = it->second;
	const SInt64 size = forkSize(fork);
	if (size < 0)
		return ioErr;

	SInt64 target;
	switch (resolveForkPosition(forkMark(fork), size, positionMode, positionOffset, &target))
	{
		case kForkPosBeforeStart:
			if ((positionMode & 3) == fsFromMark)
			{
				forkSetMark(fork, size);
				return eofErr;
			}
			forkSetMark(fork, 0);
			return posErr;
		case kForkPosPastEnd:
			forkSetMark(fork, size);
			return eofErr;
		default:
			forkSetMark(fork, target);
			return noErr;
	}
}

#if !__LP64__
// SetFPos moves the same fork mark as FSSetForkPosition, with the same positioning rules and
// QUIRKs. Inside Macintosh: Files, p. 2-95.
// Test: tests/src/file_manager_setfpos.c
OSErr SetFPos(FSIORefNum refNum, SInt16 posMode, SInt32 posOff)
{
	return FSSetForkPosition(refNum, (UInt16) posMode, posOff);
}
#endif

// Writing to forks .
// Data forks are written through their descriptor; writing at or past the end extends the fork.
// In-memory resource forks are not written back, so writes to them fail with wrPermErr.
// A target before the start fails with posErr and leaves the mark unchanged.

OSErr FSWriteFork(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset, ByteCount requestCount, const void* buffer, ByteCount* actualCount)
{
	if (actualCount)
		*actualCount = 0;

	std::lock_guard<std::mutex> lock(g_openForksLock);
	auto it = g_openForks.find(forkRefNum);
	if (it == g_openForks.end())
		return fnOpnErr;
	if (!buffer && requestCount > 0)
		return paramErr;

	OpenFork& fork = it->second;
	if (fork.fd == -1)
		return wrPermErr;
	const SInt64 size = forkSize(fork);
	if (size < 0)
		return ioErr;

	SInt64 start;
	if (resolveForkPosition(forkMark(fork), size, positionMode, positionOffset, &start) == kForkPosBeforeStart)
		return posErr;

	ssize_t done = requestCount ? ::pwrite(fork.fd, buffer, requestCount, (off_t) start) : 0;
	if (done < 0)
		return errno == EBADF ? wrPermErr : (errno == ENOSPC ? dskFulErr : ioErr);
	forkSetMark(fork, start + done);
	if (actualCount)
		*actualCount = (ByteCount) done;
	return (ByteCount) done == requestCount ? noErr : dskFulErr;
}

// Sets the logical end of a data fork; a mark beyond the new end moves to it.
OSErr FSSetForkSize(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset)
{
	std::lock_guard<std::mutex> lock(g_openForksLock);
	auto it = g_openForks.find(forkRefNum);
	if (it == g_openForks.end())
		return fnOpnErr;

	OpenFork& fork = it->second;
	if (fork.fd == -1)
		return wrPermErr;
	const SInt64 size = forkSize(fork);
	if (size < 0)
		return ioErr;

	SInt64 target;
	if (resolveForkPosition(forkMark(fork), size, positionMode, positionOffset, &target) == kForkPosBeforeStart)
		return posErr;
	const SInt64 mark = forkMark(fork);
	if (::ftruncate(fork.fd, (off_t) target) != 0)
		return errno == EBADF || errno == EINVAL ? wrPermErr : ioErr;
	if (mark > target)
		forkSetMark(fork, target);
	return noErr;
}

// QUIRK: a read that begins before the start of the fork fails with posErr and moves the mark to
// 0 (also for fsFromMark), one that begins after the end fails with eofErr and moves the mark to the
// end; a read that reaches the end before requestCount bytes were read returns what was available
// with eofErr, while one that ends exactly at the end succeeds. The newline mode stops after the
// first byte equal to the newline character, which is included. Not documented. Evidence:
// black-box observation on macOS 10.14.4, i386 and x86_64 (tier 2),
// vd-upstream/mac-probes/v665-fs1.c.
// Test: tests/src/file_manager_iterators.c (fork read tests)
OSErr FSReadFork(FSIORefNum forkRefNum, UInt16 positionMode, SInt64 positionOffset, ByteCount requestCount, void* buffer, ByteCount* actualCount)
{
	if (actualCount)
		*actualCount = 0;

	std::lock_guard<std::mutex> lock(g_openForksLock);
	auto it = g_openForks.find(forkRefNum);
	if (it == g_openForks.end())
		return fnOpnErr;
	if (!buffer && requestCount > 0)
		return paramErr;

	OpenFork& fork = it->second;
	const SInt64 size = forkSize(fork);
	if (size < 0)
		return ioErr;

	SInt64 start;
	switch (resolveForkPosition(forkMark(fork), size, positionMode, positionOffset, &start))
	{
		case kForkPosBeforeStart:
			forkSetMark(fork, 0);
			return posErr;
		case kForkPosPastEnd:
			forkSetMark(fork, size);
			return eofErr;
		default:
			break;
	}

	const ByteCount available = (ByteCount) (size - start);
	const ByteCount wanted = std::min(requestCount, available);
	ByteCount done = 0;

	if (fork.fd == -1)
	{
		if (wanted > 0)
			memcpy(buffer, fork.data.data() + start, wanted);
		done = wanted;
	}
	else
	{
		while (done < wanted)
		{
			ssize_t n = ::pread(fork.fd, (uint8_t*) buffer + done, wanted - done, (off_t) (start + done));
			if (n < 0)
			{
				if (errno == EINTR)
					continue;
				return makeOSStatus(errno);
			}
			if (n == 0)
				break;
			done += n;
		}
	}

	bool foundNewline = false;
	if ((positionMode & kFSNewLineMask) && done > 0)
	{
		const uint8_t newline = (positionMode & kFSNewLineCharMask) >> 8;
		const void* hit = memchr(buffer, newline, done);
		if (hit)
		{
			done = (const uint8_t*) hit - (const uint8_t*) buffer + 1;
			foundNewline = true;
		}
	}

	forkSetMark(fork, start + done);
	if (actualCount)
		*actualCount = done;
	return (done < requestCount && !foundNewline) ? eofErr : noErr;
}

// ---- Catalog iterators -----------------------------------------------------------------------

// An iterator lists the names in its container as they were when it was opened.
struct OpaqueFSIterator
{
	std::string path;
	std::vector<std::string> names;
	size_t next = 0;
};

static std::set<FSIterator> g_iterators;
static std::mutex g_iteratorsLock;

// QUIRK: only kFSIterateFlat (0) and kFSIterateDelete (2) are accepted; kFSIterateSubtree and every
// other flag fail with errFSBadIteratorFlags. Not documented (the header says that for now only
// kFSIterateFlat works with FSGetCatalogInfoBulk). Evidence: black-box observation on macOS 10.14.4,
// i386 and x86_64 (tier 2), vd-upstream/mac-probes/v665-fs1.c.
// Test: tests/src/file_manager_iterators.c (iterator flag tests)
OSErr FSOpenIterator(const FSRef* container, FSIteratorFlags iteratorFlags, FSIterator* iterator)
{
	if (!container || !iterator)
		return paramErr;
	if (iteratorFlags & ~(OptionBits) kFSIterateDelete)
		return errFSBadIteratorFlags;

	std::string path;
	if (!FSRefMakePath(container, path))
		return errFSBadFSRef;

	struct stat st;
	if (::stat(path.c_str(), &st) != 0)
		return (errno == ENOENT) ? fnfErr : makeOSStatus(errno);
	if (!S_ISDIR(st.st_mode))
		return errFSNotAFolder;

	std::unique_ptr<OpaqueFSIterator> it(new OpaqueFSIterator);
	it->path = path;

	DIR* dir = ::opendir(path.c_str());
	if (!dir)
		return makeOSStatus(errno);
	while (struct dirent* entry = ::readdir(dir))
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			it->names.push_back(entry->d_name);
	}
	::closedir(dir);

	std::lock_guard<std::mutex> lock(g_iteratorsLock);
	*iterator = it.get();
	g_iterators.insert(it.release());
	return noErr;
}

OSErr FSCloseIterator(FSIterator iterator)
{
	if (!iterator)
		return paramErr;

	std::lock_guard<std::mutex> lock(g_iteratorsLock);
	auto found = g_iterators.find(iterator);
	if (found == g_iterators.end())
		return errFSIteratorNotFound;

	g_iterators.erase(found);
	delete iterator;
	return noErr;
}

// QUIRK: when the iterator runs out before maximumObjects items were returned, the items found are
// returned together with errFSNoMoreItems; a call that returns exactly maximumObjects succeeds and
// the next call returns errFSNoMoreItems with no items. containerChanged stays false, also when
// items were added after the iterator was opened. Not documented. Evidence: black-box observation
// on macOS 10.14.4, i386 and x86_64 (tier 2), vd-upstream/mac-probes/v665-fs1.c.
// Test: tests/src/file_manager_iterators.c (iterator tests)
OSErr FSGetCatalogInfoBulk(FSIterator iterator, ItemCount maximumObjects, ItemCount* actualObjects, Boolean* containerChanged, FSCatalogInfoBitmap whichInfo, FSCatalogInfo* catalogInfos, FSRef* refs, FSSpecPtr specs, HFSUniStr255* names)
{
	if (actualObjects)
		*actualObjects = 0;
	if (containerChanged)
		*containerChanged = false;
	if (!iterator || !actualObjects)
		return paramErr;
	if (whichInfo != kFSCatInfoNone && !catalogInfos)
		return errFSMissingCatInfo;

	std::lock_guard<std::mutex> lock(g_iteratorsLock);
	if (g_iterators.find(iterator) == g_iterators.end())
		return errFSIteratorNotFound;

	ItemCount count = 0;
	while (count < maximumObjects && iterator->next < iterator->names.size())
	{
		const std::string path = (iterator->path == "/" ? std::string() : iterator->path) + "/" + iterator->names[iterator->next++];
		FSRef ref;
		if (FSPathMakeRefWithOptions((const uint8_t*) path.c_str(), kFSPathMakeRefDoNotFollowLeafSymlink, &ref, nullptr) != noErr)
			continue; // removed since the iterator was opened

		if (catalogInfos || names)
		{
			FSCatalogInfo scratch;
			OSStatus err = FSGetCatalogInfo(&ref, whichInfo, catalogInfos ? &catalogInfos[count] : &scratch, names ? &names[count] : nullptr, nullptr, nullptr);
			if (err != noErr)
				continue;
		}
		if (refs)
			refs[count] = ref;
		count++;
	}

	*actualObjects = count;
	return (count < maximumObjects) ? errFSNoMoreItems : noErr;
}

// ---- Volumes ---------------------------------------------------------------------------------

// Darling has one volume, the root file system. With no volume index it is found by the reference
// number -100 only (0, kFSInvalidVolumeRefNum, is nsvErr); with an index it is volume number 1.
//
// QUIRK: the fields not named by whichInfo are left alone. The values a local volume gets on macOS
// 10.14.4 (tier 2, black-box, vd-upstream/mac-probes/v665-vi2.c and v665-vi.c): sizes and block
// counts from statfs (total and free bytes are blocks times the block size), the file count the
// used inodes, a folder count of 0, resource and data clump size 4096, next allocation 0, next
// catalog ID 2, flags default-volume plus journaling-active (0x4020), signature 0x4244, file system
// ID 25461, drive number 1, driver reference number 0, backup and checked dates 0. The Mac listed a
// second volume at index 2; Darling has one.
// Test: tests/src/file_manager_iterators.c (volume info test)
OSErr FSGetVolumeInfo(FSVolumeRefNum volume, ItemCount volumeIndex, FSVolumeRefNum* actualVolume, FSVolumeInfoBitmap whichInfo, FSVolumeInfo* info, HFSUniStr255* volumeName, FSRef* rootDirectory)
{
	if (volumeIndex == 0 ? volume != kBootVolumeRefNum : volumeIndex != 1)
		return nsvErr;

	if (info && whichInfo != kFSVolInfoNone)
	{
		struct statfs sf;
		struct stat st;
		if (::statfs("/", &sf) != 0 || ::stat("/", &st) != 0)
			return makeOSStatus(errno);

		if (whichInfo & kFSVolInfoCreateDate)
			info->createDate = Darling::time_tToUTC(st.st_ctime);
		if (whichInfo & kFSVolInfoModDate)
			info->modifyDate = Darling::time_tToUTC(st.st_mtime);
		if (whichInfo & kFSVolInfoBackupDate)
			memset(&info->backupDate, 0, sizeof(info->backupDate));
		if (whichInfo & kFSVolInfoCheckedDate)
			memset(&info->checkedDate, 0, sizeof(info->checkedDate));
		if (whichInfo & kFSVolInfoFileCount)
			info->fileCount = (UInt32) (sf.f_files - sf.f_ffree);
		if (whichInfo & kFSVolInfoDirCount)
			info->folderCount = 0;
		if (whichInfo & kFSVolInfoSizes)
		{
			info->totalBytes = (UInt64) sf.f_blocks * sf.f_bsize;
			info->freeBytes = (UInt64) sf.f_bfree * sf.f_bsize;
		}
		if (whichInfo & kFSVolInfoBlocks)
		{
			info->blockSize = (UInt32) sf.f_bsize;
			info->totalBlocks = (UInt32) sf.f_blocks;
			info->freeBlocks = (UInt32) sf.f_bfree;
		}
		if (whichInfo & kFSVolInfoNextAlloc)
			info->nextAllocation = 0;
		if (whichInfo & kFSVolInfoRsrcClump)
			info->rsrcClumpSize = 4096;
		if (whichInfo & kFSVolInfoDataClump)
			info->dataClumpSize = 4096;
		if (whichInfo & kFSVolInfoNextID)
			info->nextCatalogID = 2;
		if (whichInfo & kFSVolInfoFinderInfo)
			memset(info->finderInfo, 0, sizeof(info->finderInfo));
		if (whichInfo & kFSVolInfoFlags)
			info->flags = kFSVolFlagDefaultVolumeMask | kFSVolFlagJournalingActiveMask;
		if (whichInfo & kFSVolInfoFSInfo)
		{
			info->filesystemID = 25461;
			info->signature = 0x4244;
		}
		if (whichInfo & kFSVolInfoDriveInfo)
		{
			info->driveNumber = 1;
			info->driverRefNum = 0;
		}
	}

	if (actualVolume)
		*actualVolume = kBootVolumeRefNum;
	if (volumeName)
	{
		const size_t length = strlen(kBootVolumeName);
		volumeName->length = (uint16_t) length;
		for (size_t i = 0; i < length; i++)
			volumeName->unicode[i] = (uint8_t) kBootVolumeName[i];
	}
	if (rootDirectory)
		return FSPathMakeRef((const uint8_t*) "/", rootDirectory, nullptr);
	return noErr;
}

// QUIRK: the volume reference numbers FSEjectVolumeSync and PBHGetVolParmsSync accept: the startup
// volume (-100), 0 and positive numbers for PBHGetVolParmsSync, and anything else fails with
// nsvErr. Ejecting never succeeds here: the only volume is the root file system, which cannot be
// ejected, and the call reports errFSOperationNotSupported for it. Evidence for the nsvErr cases:
// black-box observation on macOS 10.14.4, i386 and x86_64 (tier 2),
// vd-upstream/mac-probes/v665-fs1.c; the startup-volume result is not observed (ejecting a startup
// volume was not tried) and follows the error's documented meaning.
// Test: tests/src/file_manager_iterators.c (volume tests)
OSStatus FSEjectVolumeSync(FSVolumeRefNum vRefNum, OptionBits flags, pid_t* dissenter)
{
	if (vRefNum != kBootVolumeRefNum)
		return nsvErr;
	return errFSOperationNotSupported;
}

// Values reported for the volume by version 5 of the volume parameters buffer, as observed for a
// local startup volume on macOS 10.14.4 (tier 2), vd-upstream/mac-probes/v665-fs1.c.
static const SInt32 kVolParmsAttrib = 0x28084408;
static const SInt32 kVolParmsExtendedAttributes = 0x10ac2432;
static const UniCharCount kVolParmsMaxNameLength = 85;
static const char kVolParmsDeviceID[] = "";

// Copies the leading bytes of the version 5 volume parameters that fit in a buffer of the given
// size, and returns how many were copied.
static size_t copyVolParms(void* buffer, size_t size)
{
	GetVolParmsInfoBuffer parms;
	memset(&parms, 0, sizeof(parms));
	parms.vMVersion = 5;
	parms.vMAttrib = kVolParmsAttrib;
	parms.vMExtendedAttributes = kVolParmsExtendedAttributes;
	parms.vMDeviceID = (void*) kVolParmsDeviceID;
	parms.vMMaxNameLength = kVolParmsMaxNameLength;

	const size_t n = std::min<size_t>(size, sizeof(parms));
	memcpy(buffer, &parms, n);
	return n;
}

OSErr PBHGetVolParmsSync(HParmBlkPtr paramBlock)
{
	if (!paramBlock)
		return paramErr;

	HIOParam& io = paramBlock->ioParam;
	io.ioActCount = 0;

	OSErr err = noErr;
	if (io.ioVRefNum < 0 && io.ioVRefNum != kBootVolumeRefNum)
		err = nsvErr;
	else if (!io.ioBuffer || io.ioReqCount <= 0)
		err = paramErr;

	if (err == noErr)
		io.ioActCount = (SInt32) copyVolParms(io.ioBuffer, io.ioReqCount);

	io.ioResult = err;
	return err;
}

// QUIRK: fills as many leading bytes of the version 5 buffer as bufferSize allows, and succeeds
// with nothing written when bufferSize is 0 (no error for a short buffer). Only the startup
// volume (-100) is accepted: 0, other negative numbers and positive numbers (which
// PBHGetVolParmsSync accepts) fail with nsvErr and leave the buffer alone. Not documented.
// Evidence: black-box observation on macOS 26.7.3 x86_64 and macOS 10.14.4 i386 (tier 2).
// Test: tests/src/file_manager_volume_parms.c
OSStatus FSGetVolumeParms(FSVolumeRefNum volume, GetVolParmsInfoBuffer* buffer, ByteCount bufferSize)
{
	if (volume != kBootVolumeRefNum)
		return nsvErr;
	if (!buffer)
		return paramErr;

	copyVolParms(buffer, bufferSize);
	return noErr;
}

// ---- PBMakeFSRefSync -------------------------------------------------------------------------

// Resolves the Pascal pathname of a legacy parameter block. A name with a colon that does not begin
// with one starts with a volume name and is a full path; otherwise it is relative to the directory
// ioDirID names, or to the current directory for ID 0. A leading colon only marks a relative path,
// and each further empty component goes up one directory.
static OSErr makeFSRefFromHFSPath(const FSRefParam* param, std::string& out)
{
	std::string name;
	if (param->ioNamePtr)
	{
		const unsigned char* pstr = (const unsigned char*) param->ioNamePtr;
		CFStringRef str = CFStringCreateWithBytes(nullptr, pstr + 1, pstr[0], kCFStringEncodingMacRoman, false);
		if (!str)
			return paramErr;
		char buf[1024];
		bool ok = CFStringGetCString(str, buf, sizeof(buf), kCFStringEncodingUTF8);
		CFRelease(str);
		if (!ok)
			return paramErr;
		name = buf;
	}

	const bool fullPath = !name.empty() && name[0] != ':' && name.find(':') != std::string::npos;
	std::string path;
	size_t pos = 0;

	if (fullPath)
	{
		size_t colon = name.find(':');
		if (name.compare(0, colon, kBootVolumeName) != 0)
			return nsvErr;
		path = "/";
		pos = colon + 1;
	}
	else
	{
		if (param->ioDirID == 0)
		{
			char cwd[PATH_MAX];
			if (!::getcwd(cwd, sizeof(cwd)))
				return fnfErr;
			path = cwd;
		}
		else if (param->ioDirID == fsRtParID)
			return nsvErr;
		else if (!dirNodeIDToPath(param->ioDirID, path))
			return fnfErr;
		pos = (!name.empty() && name[0] == ':') ? 1 : 0;
	}

	while (pos < name.size())
	{
		size_t colon = name.find(':', pos);
		if (colon == std::string::npos)
			colon = name.size();
		std::string component = name.substr(pos, colon - pos);
		pos = colon + 1;

		if (component.empty())
		{
			size_t slash = path.find_last_of('/');
			path = (slash == std::string::npos || slash == 0) ? std::string("/") : path.substr(0, slash);
		}
		else if (component.find('/') != std::string::npos)
			return fnfErr;
		else
		{
			if (path.empty() || path.back() != '/')
				path += '/';
			path += component;
		}
	}

	out = path;
	return noErr;
}

// QUIRK: PBMakeFSRefSync with a NULL newRef fails with errFSBadFSRef; ioDirID 1, a name that has a
// colon in the middle but does not name this volume, and a volume name that does not exist fail
// with nsvErr, and an unknown directory ID or a missing item with fnfErr; the volume reference
// number is ignored. Not documented. Evidence: black-box observation on macOS 10.14.4, i386 (tier
// 2; the call does not exist in 64-bit processes), vd-upstream/mac-probes/v665-fs2.c.
// Test: tests/src/file_manager_iterators.c (PBMakeFSRefSync tests)
OSErr PBMakeFSRefSync(FSRefParam* paramBlock)
{
	if (!paramBlock)
		return paramErr;

	OSErr err;
	std::string path;
	if (!paramBlock->newRef)
		err = errFSBadFSRef;
	else if ((err = makeFSRefFromHFSPath(paramBlock, path)) == noErr)
	{
		if (FSPathMakeRefWithOptions((const uint8_t*) path.c_str(), kFSPathMakeRefDoNotFollowLeafSymlink, paramBlock->newRef, nullptr) != noErr)
			err = fnfErr;
	}

	paramBlock->ioResult = err;
	return err;
}

static bool copyFileContents(const char* src, const char* dst, mode_t mode)
{
	int inFd = open(src, O_RDONLY);
	if (inFd < 0)
		return false;

	int outFd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (outFd < 0)
	{
		close(inFd);
		return false;
	}

	char buf[65536];
	ssize_t bytes;
	bool ok = true;
	while (true)
	{
		bytes = read(inFd, buf, sizeof(buf));
		if (bytes < 0)
		{
			if (errno == EINTR) continue;
			ok = false;
			break;
		}
		if (bytes == 0) break;

		ssize_t written = 0;
		while (written < bytes)
		{
			ssize_t w = write(outFd, buf + written, bytes - written);
			if (w <= 0)
			{
				if (w < 0 && errno == EINTR) continue;
				ok = false;
				break;
			}
			written += w;
		}
		if (!ok)
			break;
	}

	close(inFd);
	close(outFd);
	return ok;
}

static bool copyRecursive(const std::string& src, const std::string& dst)
{
	struct stat st;
	if (lstat(src.c_str(), &st) != 0)
		return false;

	if (S_ISDIR(st.st_mode))
	{
		mode_t dirMode = (st.st_mode & 0777) | S_IWUSR;
		if (mkdir(dst.c_str(), dirMode) != 0 && errno != EEXIST)
			return false;

		DIR* dir = opendir(src.c_str());
		if (!dir)
			return false;

		struct dirent* entry;
		bool ok = true;
		while ((entry = readdir(dir)) != nullptr)
		{
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			std::string subSrc = src + "/" + entry->d_name;
			std::string subDst = dst + "/" + entry->d_name;
			if (!copyRecursive(subSrc, subDst))
			{
				ok = false;
				break;
			}
		}
		closedir(dir);
		chmod(dst.c_str(), st.st_mode & 0777);
		return ok;
	}
	else if (S_ISLNK(st.st_mode))
	{
		char target[PATH_MAX];
		ssize_t len = readlink(src.c_str(), target, sizeof(target) - 1);
		if (len < 0)
			return false;
		target[len] = '\0';
		unlink(dst.c_str());
		return symlink(target, dst.c_str()) == 0;
	}
	else if (S_ISREG(st.st_mode))
	{
		return copyFileContents(src.c_str(), dst.c_str(), st.st_mode & 0777);
	}
	return false;
}

extern "C" OSStatus FSCopyObjectSync(const FSRef *source, const FSRef *destinationDir, CFStringRef copyName, FSRef *newRef, OptionBits whichInfo)
{
	std::string srcPath;
	std::string dstDirPath;

	if (!source || !destinationDir)
		return paramErr;

	if (!FSRefMakePath(source, srcPath))
		return fnfErr;

	if (!FSRefMakePath(destinationDir, dstDirPath))
		return fnfErr;

	std::string targetPath = dstDirPath;
	if (targetPath.back() != '/')
		targetPath += '/';

	if (copyName)
	{
		char nameBuf[4096];
		if (CFStringGetCString(copyName, nameBuf, sizeof(nameBuf), kCFStringEncodingUTF8))
			targetPath += nameBuf;
		else
			return paramErr;
	}
	else
	{
		size_t slash = srcPath.rfind('/');
		if (slash != std::string::npos)
			targetPath += srcPath.substr(slash + 1);
		else
			targetPath += srcPath;
	}

	struct stat targetStat;
	if (lstat(targetPath.c_str(), &targetStat) == 0)
		return dupFNErr;

	// A directory cannot be copied into itself or one of its descendants:
	// the walk would keep finding the copy it is creating.
	if (targetPath.compare(0, srcPath.size(), srcPath) == 0 && targetPath.size() > srcPath.size()
			&& targetPath[srcPath.size()] == '/')
	{
		return paramErr;
	}

	if (!copyRecursive(srcPath, targetPath))
		return ioErr;

	if (newRef)
	{
		Boolean isDir = false;
		FSPathMakeRef((const uint8_t*)targetPath.c_str(), newRef, &isDir);
	}

	return noErr;
}

extern "C" OSStatus FSMoveObjectSync(const FSRef *source, const FSRef *destinationDir, CFStringRef moveName, FSRef *newRef, OptionBits whichInfo)
{
	std::string srcPath;
	std::string dstDirPath;

	if (!source || !destinationDir)
		return paramErr;

	if (!FSRefMakePath(source, srcPath))
		return fnfErr;

	if (!FSRefMakePath(destinationDir, dstDirPath))
		return fnfErr;

	std::string targetPath = dstDirPath;
	if (targetPath.back() != '/')
		targetPath += '/';

	if (moveName)
	{
		char nameBuf[4096];
		if (CFStringGetCString(moveName, nameBuf, sizeof(nameBuf), kCFStringEncodingUTF8))
			targetPath += nameBuf;
		else
			return paramErr;
	}
	else
	{
		size_t slash = srcPath.rfind('/');
		if (slash != std::string::npos)
			targetPath += srcPath.substr(slash + 1);
		else
			targetPath += srcPath;
	}

	struct stat targetStat;
	if (lstat(targetPath.c_str(), &targetStat) == 0)
		return dupFNErr;

	if (::rename(srcPath.c_str(), targetPath.c_str()) != 0)
	{
		if (errno == EXDEV)
			return diffVolErr;
		return makeOSStatus(errno);
	}

	if (newRef)
	{
		Boolean isDir = false;
		FSPathMakeRef((const uint8_t*)targetPath.c_str(), newRef, &isDir);
	}

	return noErr;
}


// Converts a UTF-16 name as taken by the FS*Unicode calls to the UTF-8 name of a directory entry.
// A slash cannot appear in a Linux name, so (as HFS+ names are) a slash in the name stands for a colon when
// creating; see FSCreateFileUnicode.
static bool unicodeNameToUTF8(UniCharCount length, const UniChar* name, std::string& out)
{
	CFStringRef str = CFStringCreateWithCharacters(NULL, name, length);
	if (!str)
		return false;

	CFIndex bufsize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	std::unique_ptr<char[]> buf(new char[bufsize]);
	bool ok = CFStringGetCString(str, buf.get(), bufsize, kCFStringEncodingUTF8);
	CFRelease(str);

	if (ok)
		out = buf.get();
	return ok;
}

// QUIRK: FSCreateFileUnicode stores a slash in the requested name as a colon, fails with errFSMissingName for an
// empty name and with dupFNErr for an existing one.
// Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2).
// Test: tests/src/file_manager_unicode.c (test_quirk_create_file_slash_becomes_colon)
extern "C" OSErr FSCreateFileUnicode(const FSRef* parentRef, UniCharCount nameLength, const UniChar* name,
	FSCatalogInfoBitmap whichInfo, const FSCatalogInfo* catalogInfo, FSRef* newRef, FSSpecPtr newSpec)
{
	std::string dir, leaf;

	if (!parentRef)
		return errFSBadFSRef;
	if (nameLength == 0)
		return errFSMissingName;
	if (!name)
		return bdNamErr;
	if (nameLength > 255)
		return errFSNameTooLong;
	if (!FSRefMakePath(parentRef, dir))
		return fnfErr;
	if (!unicodeNameToUTF8(nameLength, name, leaf))
		return bdNamErr;

	std::replace(leaf.begin(), leaf.end(), '/', ':');

	struct stat st;
	if (stat(dir.c_str(), &st) != 0)
		return fnfErr;
	if (!S_ISDIR(st.st_mode))
		return errFSNotAFolder;

	std::string path = dir;
	if (path.back() != '/')
		path += '/';
	path += leaf;

	int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (fd == -1)
		return errno == EEXIST ? dupFNErr : makeOSStatus(errno);
	close(fd);

	if (newSpec)
	{
		memset(newSpec, 0, sizeof(*newSpec));
#if !__LP64__
		fsSpecForPath(path, newSpec);
#endif
	}

	FSRef created;
	OSStatus status = FSPathMakeRef((const uint8_t*) path.c_str(), &created, nullptr);
	if (status != noErr)
		return status;

	if (whichInfo != kFSCatInfoNone && catalogInfo)
	{
		status = FSSetCatalogInfo(&created, whichInfo, catalogInfo);
		if (status != noErr)
			return status;
	}

	if (newRef)
		*newRef = created;
	return noErr;
}

// QUIRK: FSMakeFSRefUnicode answers the parent itself for an empty name, bdNamErr for a name containing a colon
// or equal to "..", and fnfErr for a name containing a slash.
// Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2).
// Test: tests/src/file_manager_unicode.c (test_quirk_make_fsref_special_names)
extern "C" OSStatus FSMakeFSRefUnicode(const FSRef* parentRef, UniCharCount nameLength, const UniChar* name,
	TextEncoding textEncodingHint, FSRef* newRef)
{
	std::string dir, leaf;
	(void) textEncodingHint;

	if (!parentRef || !newRef)
		return errFSBadFSRef;
	if (nameLength > 0 && !name)
		return bdNamErr;
	if (!FSRefMakePath(parentRef, dir))
		return fnfErr;

	struct stat st;
	if (lstat(dir.c_str(), &st) != 0)
		return fnfErr;
	if (!S_ISDIR(st.st_mode))
		return errFSNotAFolder;

	if (nameLength == 0)
	{
		*newRef = *parentRef;
		return noErr;
	}
	if (nameLength > 255)
		return errFSNameTooLong;
	if (!unicodeNameToUTF8(nameLength, name, leaf))
		return bdNamErr;
	if (leaf.find(':') != std::string::npos || leaf == "..")
		return bdNamErr;
	if (leaf.find('/') != std::string::npos)
		return fnfErr;

	std::string path = dir;
	if (path.back() != '/')
		path += '/';
	path += leaf;

	if (lstat(path.c_str(), &st) != 0)
		return errno == ENOTDIR ? errFSNotAFolder : fnfErr;

	return FSPathMakeRef((const uint8_t*) path.c_str(), newRef, nullptr);
}

extern "C" CFStringRef FSCreateStringFromHFSUniStr(CFAllocatorRef alloc, const HFSUniStr255* uniStr)
{
	if (!uniStr)
		return NULL;

	UniCharCount length = std::min<UniCharCount>(uniStr->length, 255);
	return CFStringCreateWithCharacters(alloc, uniStr->unicode, length);
}

extern "C" OSStatus FSGetHFSUniStrFromString(CFStringRef theString, HFSUniStr255* theOutString)
{
	if (!theString || !theOutString)
		return paramErr;

	CFIndex length = CFStringGetLength(theString);
	if (length > 255)
		return errFSNameTooLong;

	theOutString->length = length;
	CFStringGetCharacters(theString, CFRangeMake(0, length), theOutString->unicode);
	return noErr;
}

static std::string trashDirectoryPath()
{
	const char* home = getenv("HOME");
	if (!home || !*home)
	{
		struct passwd* pw = getpwuid(getuid());
		home = pw ? pw->pw_dir : nullptr;
	}
	if (!home || !*home)
		return std::string();
	return std::string(home) + "/.Trash";
}

// QUIRK: when the trash already holds an item of that name, the moved item is renamed to the name followed by
// a space and the local time as HH-MM-SS-mmm, with the original extension (if any) after it.
// Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2).
// Test: tests/src/file_manager_unicode.c (test_quirk_trash_name_collision)
extern "C" OSStatus FSMoveObjectToTrashSync(const FSRef* source, FSRef* target, OptionBits options)
{
	(void) options;

	std::string srcPath, trash;
	if (!source)
		return errFSBadFSRef;
	if (!FSRefMakePath(source, srcPath))
		return fnfErr;

	struct stat st;
	if (lstat(srcPath.c_str(), &st) != 0)
		return fnfErr;

	trash = trashDirectoryPath();
	if (trash.empty())
		return fnfErr;
	if (mkdir(trash.c_str(), 0700) != 0 && errno != EEXIST)
		return makeOSStatus(errno);

	std::string name = srcPath.substr(srcPath.rfind('/') + 1);
	if (name.empty())
		return paramErr;

	if (lstat((trash + "/" + name).c_str(), &st) == 0)
	{
		struct timeval tv;
		struct tm tmv;
		char stamp[32];

		gettimeofday(&tv, nullptr);
		localtime_r(&tv.tv_sec, &tmv);
		snprintf(stamp, sizeof(stamp), " %02d-%02d-%02d-%03d", tmv.tm_hour, tmv.tm_min, tmv.tm_sec, (int) (tv.tv_usec / 1000));

		size_t dot = name.rfind('.');
		std::string ext = (dot != std::string::npos && dot > 0) ? name.substr(dot) : std::string();
		name = name + stamp + ext;
	}

	FSRef trashRef;
	OSStatus status = FSPathMakeRef((const uint8_t*) trash.c_str(), &trashRef, nullptr);
	if (status != noErr)
		return status;

	CFStringRef cfName = CFStringCreateWithCString(NULL, name.c_str(), kCFStringEncodingUTF8);
	if (!cfName)
		return paramErr;
	status = FSMoveObjectSync(source, &trashRef, cfName, target, 0);
	CFRelease(cfName);
	return status;
}

#if !__LP64__
// FSSpec: the 32-bit File Manager's name for an item, the volume, the ID of the parent directory
// and the name of the item. The name is the item's name in the file system, not an HFS name.

static std::string fsSpecDirectory(SInt16 vRefNum, SInt32 dirID, bool* ok)
{
	std::string path;

	*ok = true;
	if (dirID == 0)
	{
		// The default directory
		char cwd[PATH_MAX];
		if (!getcwd(cwd, sizeof(cwd)))
			*ok = false;
		return cwd;
	}
	if (dirID == fsRtDirID)
		return "/";
	if (!dirNodeIDToPath(dirID, path))
		*ok = false;
	return path;
}

static bool fsSpecIsRootName(ConstStr255Param name)
{
	const size_t length = strlen(kBootVolumeName);
	return name[0] == length && memcmp(name + 1, kBootVolumeName, length) == 0;
}

// The root directory of a volume is described by the volume's name, with fsRtParID as its parent.
static void fillRootFSSpec(FSSpec* spec)
{
	const size_t length = strlen(kBootVolumeName);

	memset(spec, 0, sizeof(*spec));
	spec->vRefNum = kBootVolumeRefNum;
	spec->parID = fsRtParID;
	spec->name[0] = (unsigned char) length;
	memcpy(spec->name + 1, kBootVolumeName, length);
}

static bool fsSpecPath(const FSSpec* spec, std::string& path)
{
	bool ok;

	if (!spec || spec->name[0] == 0 || spec->name[0] > sizeof(spec->name) - 1)
		return false;
	if (spec->parID == fsRtParID)
	{
		// The volume's name under fsRtParID names the root directory of the volume
		path = "/";
		return fsSpecIsRootName(spec->name);
	}
	path = fsSpecDirectory(spec->vRefNum, spec->parID, &ok);
	if (!ok)
		return false;
	if (path.empty() || path.back() != '/')
		path += '/';
	path.append((const char*) spec->name + 1, spec->name[0]);
	return true;
}

static OSErr fsSpecForPath(const std::string& path, FSSpec* spec)
{
	size_t slash = path.find_last_of('/');
	std::string parent = (slash == std::string::npos || slash == 0) ? std::string("/") : path.substr(0, slash);
	std::string leaf = path.substr(slash == std::string::npos ? 0 : slash + 1);
	struct stat st;
	uint32_t nodeID, parentID;

	memset(spec, 0, sizeof(*spec));
	if (path == "/")
	{
		fillRootFSSpec(spec);
		return noErr;
	}
	if (leaf.empty() || leaf.size() > sizeof(spec->name) - 1)
		return bdNamErr;
	if (::stat(parent.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
		return dirNFErr;
	nodeIDsForPath(parent, st, &nodeID, &parentID);
	spec->vRefNum = kBootVolumeRefNum;
	spec->parID = parent == "/" ? fsRtDirID : (SInt32) nodeID;
	spec->name[0] = (unsigned char) leaf.size();
	memcpy(spec->name + 1, leaf.data(), leaf.size());
	return noErr;
}

OSErr FSMakeFSSpec(FSVolumeRefNum vRefNum, SInt32 dirID, ConstStr255Param fileName, FSSpec* spec)
{
	bool ok;
	std::string dir, path;
	struct stat st;
	size_t nameLength = fileName ? fileName[0] : 0;

	if (!spec)
		return paramErr;
	memset(spec, 0, sizeof(*spec));
	if (dirID == fsRtParID)
	{
		// The parent of the root directory is no directory: the only item it holds, by the volume's
		// name, is the root directory. Any other name is a volume that does not exist (the same error
		// PBMakeFSRefSync gives for this ID).
		if (!fileName || !fsSpecIsRootName(fileName))
			return nsvErr;
		fillRootFSSpec(spec);
		return noErr;
	}
	dir = fsSpecDirectory(vRefNum, dirID, &ok);
	// QUIRK: a directory ID that names no directory gives fnfErr, not dirNFErr. Not documented. Evidence:
	// black-box observation on macOS 10.14.4 i386 (tier 2).
	// Test: tests/src/fsspec_calls.c (the call with directory ID 0x7fffff00)
	if (!ok)
		return fnfErr;
	if (nameLength > 0 && fileName[1] == '/')
		path.assign((const char*) fileName + 1, nameLength);	// a full path
	else
	{
		path = dir;
		if (nameLength > 0)
		{
			if (path.empty() || path.back() != '/')
				path += '/';
			path.append((const char*) fileName + 1, nameLength);
		}
	}
	while (path.size() > 1 && path.back() == '/')
		path.pop_back();

	// The spec of an item names the item and the directory that holds it. The item may not exist
	// yet, if its directory does (the spec is filled in and the result is fnfErr).
	size_t slash = path.find_last_of('/');
	std::string parent = (slash == std::string::npos || slash == 0) ? std::string("/") : path.substr(0, slash);
	std::string leaf = path == "/" ? std::string() : path.substr(slash + 1);

	if (path == "/")
	{
		// The root directory itself (fsRtDirID with an empty name)
		fillRootFSSpec(spec);
		return noErr;
	}
	if (::stat(parent.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
		return dirNFErr;
	if (leaf.size() > sizeof(spec->name) - 1)
		return bdNamErr;

	// Registers the node ID of the directory
	uint32_t nodeID, parentID;
	nodeIDsForPath(parent, st, &nodeID, &parentID);
	spec->vRefNum = kBootVolumeRefNum;
	spec->parID = parent == "/" ? fsRtDirID : (SInt32) nodeID;
	spec->name[0] = (unsigned char) leaf.size();
	memcpy(spec->name + 1, leaf.data(), leaf.size());
	return ::lstat(path.c_str(), &st) == 0 ? noErr : fnfErr;
}

OSStatus FSpMakeFSRef(const FSSpec* source, FSRef* newRef)
{
	std::string path;

	if (!newRef)
		return paramErr;
	if (!fsSpecPath(source, path))
		return fnfErr;
	return FSPathMakeRef((const uint8_t*) path.c_str(), newRef, nullptr);
}

OSErr FSpDelete(const FSSpec* spec)
{
	std::string path;
	struct stat st;

	if (!fsSpecPath(spec, path))
		return fnfErr;
	if (::lstat(path.c_str(), &st) != 0)
		return fsSpecError(errno);
	if (S_ISDIR(st.st_mode))
	{
		if (::rmdir(path.c_str()) != 0)
			return errno == ENOTEMPTY ? fBsyErr : fsSpecError(errno);
		return noErr;
	}
	return ::unlink(path.c_str()) == 0 ? noErr : fsSpecError(errno);
}

// The Finder information is kept in the extended attribute macOS keeps it in
static const char kFinderInfoAttribute[] = "com.apple.FinderInfo";

OSErr FSpGetFInfo(const FSSpec* spec, FInfo* fndrInfo)
{
	std::string path;
	struct stat st;
	uint8_t raw[32];

	if (!fndrInfo)
		return paramErr;
	if (!fsSpecPath(spec, path))
		return fnfErr;
	if (::stat(path.c_str(), &st) != 0)
		return fsSpecError(errno);
	memset(raw, 0, sizeof(raw));
	if (getxattr(path.c_str(), kFinderInfoAttribute, raw, sizeof(raw), 0, 0) < 0 && errno != ENOATTR)
		return fsSpecError(errno);
	memset(fndrInfo, 0, sizeof(*fndrInfo));
	// The first 16 bytes of the Finder information of a file, big-endian: type, creator, flags, location, folder
	fndrInfo->fdType = (raw[0] << 24) | (raw[1] << 16) | (raw[2] << 8) | raw[3];
	fndrInfo->fdCreator = (raw[4] << 24) | (raw[5] << 16) | (raw[6] << 8) | raw[7];
	fndrInfo->fdFlags = (raw[8] << 8) | raw[9];
	fndrInfo->fdLocation.v = (SInt16) ((raw[10] << 8) | raw[11]);
	fndrInfo->fdLocation.h = (SInt16) ((raw[12] << 8) | raw[13]);
	fndrInfo->fdFldr = (SInt16) ((raw[14] << 8) | raw[15]);
	return noErr;
}

OSErr FSpSetFInfo(const FSSpec* spec, const FInfo* fndrInfo)
{
	std::string path;
	struct stat st;
	uint8_t raw[32];

	if (!fndrInfo)
		return paramErr;
	if (!fsSpecPath(spec, path))
		return fnfErr;
	if (::stat(path.c_str(), &st) != 0)
		return fsSpecError(errno);
	// The rest of the Finder information stays as it is
	memset(raw, 0, sizeof(raw));
	if (getxattr(path.c_str(), kFinderInfoAttribute, raw, sizeof(raw), 0, 0) < 0 && errno != ENOATTR)
		return fsSpecError(errno);
	raw[0] = fndrInfo->fdType >> 24; raw[1] = fndrInfo->fdType >> 16; raw[2] = fndrInfo->fdType >> 8; raw[3] = fndrInfo->fdType;
	raw[4] = fndrInfo->fdCreator >> 24; raw[5] = fndrInfo->fdCreator >> 16; raw[6] = fndrInfo->fdCreator >> 8; raw[7] = fndrInfo->fdCreator;
	raw[8] = fndrInfo->fdFlags >> 8; raw[9] = fndrInfo->fdFlags;
	raw[10] = fndrInfo->fdLocation.v >> 8; raw[11] = fndrInfo->fdLocation.v;
	raw[12] = fndrInfo->fdLocation.h >> 8; raw[13] = fndrInfo->fdLocation.h;
	raw[14] = fndrInfo->fdFldr >> 8; raw[15] = fndrInfo->fdFldr;
	if (setxattr(path.c_str(), kFinderInfoAttribute, raw, sizeof(raw), 0, 0) != 0)
		return fsSpecError(errno);
	return noErr;
}

OSErr FSpCreate(const FSSpec* spec, OSType creator, OSType fileType, ScriptCode scriptTag)
{
	std::string path;

	if (!fsSpecPath(spec, path))
		return dirNFErr;
	int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (fd < 0)
		return errno == EEXIST ? dupFNErr : fsSpecError(errno);
	::close(fd);

	FInfo info;
	memset(&info, 0, sizeof(info));
	info.fdType = fileType;
	info.fdCreator = creator;
	return FSpSetFInfo(spec, &info);
}

// The classic file access calls: they open a file by FSSpec and read and write at its mark, on the same fork
// references as FSOpenFork. The permission is that of the File Manager: fsCurPerm opens the fork for reading and
// writing if the file allows it, and for reading otherwise; fsRdWrShPerm is read and write.
// Reference: File Manager Reference (Inside Macintosh: Files).
static OSErr openForkOfSpec(const FSSpec* spec, bool resourceFork, SInt8 permission, FSIORefNum* refNum)
{
	std::string path;
	FSRef ref;
	HFSUniStr255 forkName;

	if (!refNum)
		return paramErr;
	*refNum = 0;
	if (!fsSpecPath(spec, path))
		return fnfErr;
	OSStatus status = FSPathMakeRef((const uint8_t*) path.c_str(), &ref, nullptr);
	if (status != noErr)
		return (OSErr) status;
	if (resourceFork)
		FSGetResourceForkName(&forkName);
	else
		forkName.length = 0;
	if (resourceFork)
		permission = fsRdPerm; // the resource fork is held in memory and cannot be written back
	if (permission == fsRdWrShPerm)
		permission = fsRdWrPerm;
	if (permission == fsCurPerm)
	{
		OSErr err = FSOpenFork(&ref, forkName.length, forkName.unicode, fsRdWrPerm, refNum);
		if (err != noErr)
			err = FSOpenFork(&ref, forkName.length, forkName.unicode, fsRdPerm, refNum);
		return err;
	}
	return FSOpenFork(&ref, forkName.length, forkName.unicode, permission, refNum);
}

OSErr FSpOpenDF(const FSSpec* spec, SInt8 permission, FSIORefNum* refNum)
{
	return openForkOfSpec(spec, false, permission, refNum);
}

OSErr FSpOpenRF(const FSSpec* spec, SInt8 permission, FSIORefNum* refNum)
{
	return openForkOfSpec(spec, true, permission, refNum);
}

// Reads *count bytes at the mark and sets *count to the number read; reaching the end first is eofErr
OSErr FSRead(FSIORefNum refNum, long* count, void* buffPtr)
{
	ByteCount actual = 0;

	if (!count || *count < 0)
		return paramErr;
	OSErr err = FSReadFork(refNum, fsAtMark, 0, (ByteCount) *count, buffPtr, &actual);
	*count = (long) actual;
	return err;
}

OSErr FSWrite(FSIORefNum refNum, long* count, const void* buffPtr)
{
	ByteCount actual = 0;

	if (!count || *count < 0)
		return paramErr;
	OSErr err = FSWriteFork(refNum, fsAtMark, 0, (ByteCount) *count, buffPtr, &actual);
	*count = (long) actual;
	return err;
}

OSErr GetEOF(FSIORefNum refNum, long* logEOF)
{
	SInt64 size;

	if (!logEOF)
		return paramErr;
	OSErr err = FSGetForkSize(refNum, &size);
	if (err == noErr)
		*logEOF = (long) size;
	return err;
}

OSErr SetEOF(FSIORefNum refNum, long logEOF)
{
	return FSSetForkSize(refNum, fsFromStart, logEOF);
}

OSErr GetFPos(FSIORefNum refNum, long* filePos)
{
	SInt64 position;

	if (!filePos)
		return paramErr;
	OSErr err = FSGetForkPosition(refNum, &position);
	if (err == noErr)
		*filePos = (long) position;
	return err;
}

OSErr FSClose(FSIORefNum refNum)
{
	return FSCloseFork(refNum);
}
#endif

#ifndef __LP64__
#include <stdint.h>
#include <CoreServices/FileManager.h>

#ifdef __cplusplus
extern "C"
#endif
OSErr FindFolder(
    short volume,OSType folderType,
    Boolean createFolder,
    short* foundVolume,long* foundDirectory)
{
    if (!foundVolume || !foundDirectory)
        return paramErr;

    FSRef ref;

    OSStatus err=FSFindFolder(
        volume,folderType,createFolder,&ref);

    if (err!=noErr)
        return err>=-32768 && err<=32767
            ? (OSErr)err : ioErr;

    FSCatalogInfo info={0};

    err=FSGetCatalogInfo(
        &ref,
        kFSCatInfoVolume |
        kFSCatInfoNodeID |
        kFSCatInfoNodeFlags,
        &info,0,0,0);

    if (err!=noErr)
        return err>=-32768 && err<=32767
            ? (OSErr)err : ioErr;

    if (!(info.nodeFlags & kFSNodeIsDirectoryMask))
        return dirNFErr;

    *foundVolume=info.volume;

    // Preserve the legacy 32-bit catalog identifier.
    *foundDirectory=(long)(int32_t)info.nodeID;

    return noErr;
}

#endif
