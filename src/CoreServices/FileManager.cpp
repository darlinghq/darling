#include "FileManager.h"
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
#include <libgen.h>
#include "DateTimeUtils.h"
#include "../util/stlutils.h"

// Doesn't resolve the last symlink
// Mallocates a new buffer
static char* realpath_ns(const char* path);
static bool FSRefMakePath(const FSRef* ref, std::string& out);
// Is the current user member of the specified group?
static bool hasgid(gid_t gid);

OSStatus FSPathMakeRef(const uint8_t* path, FSRef* fsref, Boolean* isDirectory)
{
	return FSPathMakeRefWithOptions(path, kFSPathMakeRefDoNotFollowLeafSymlink, fsref, isDirectory); 
}

OSStatus FSPathMakeRefWithOptions(const uint8_t* path, long options, FSRef* fsref, Boolean* isDirectory)
{
	if (!path || !fsref)
		return paramErr;

	std::string fullPath;
	char* rpath;
	
	if (options & kFSPathMakeRefDoNotFollowLeafSymlink)
		rpath = realpath_ns(reinterpret_cast<const char*>(path));
	else
		rpath = realpath(reinterpret_cast<const char*>(path), nullptr);

	if (!rpath)
		return fnfErr;
	if (std::count(rpath, rpath+strlen(rpath), '/') > FSRef_MAX_DEPTH)
	{
		free(rpath);
		return unimpErr;
	}

	fullPath = rpath;
	free(rpath);

	memset(fsref, 0, sizeof(*fsref));

	if (fullPath == "/")
	{
		if (isDirectory)
			*isDirectory = true;
		return noErr;
	}

	size_t pos = 1;
	struct stat st;
	int componentPos = 0;
	do
	{
		std::string component;

		pos = fullPath.find('/', pos);
		component = fullPath.substr(0, pos);
		
		if (::lstat(component.c_str(), &st) == -1)
			return makeOSStatus(errno);
		
		fsref->inodes[componentPos++] = st.st_ino;
	}
	while (pos != std::string::npos);

	if (isDirectory)
		*isDirectory = S_ISDIR(st.st_mode);

	return noErr; 
}

char* realpath_ns(const char* path)
{
	char *dup1, *dup2;
	char *dname, *bname;
	char *real, *complete;

	dup1 = strdup(path);
	dup2 = strdup(path);
	dname = dirname(dup1);
	bname = basename(dup2);

	real = realpath(dname, nullptr);
	complete = (char*) malloc(strlen(real) + strlen(bname) + 2);

	strcpy(complete, real);
	strcat(complete, "/");
	strcat(complete, bname);

	free(real);
	free(dup1);
	free(dup2);

	return complete;
}

bool FSRefMakePath(const FSRef* fsref, std::string& out)
{
	out = '/';
	for (int i = 0; i < FSRef_MAX_DEPTH && fsref->inodes[i] != 0; i++)
	{
		ino_t inode = fsref->inodes[i];
		DIR* dir = opendir(out.c_str());
		struct dirent* ent;
		bool found = false;

		while ((ent = readdir(dir)))
		{
			if (ent->d_ino == inode)
			{
				found = true;
				if (!string_endsWith(out, "/"))
					out += '/';
				out += ent->d_name;
			}
		}

		closedir(dir);

		if (!found)
			return false;
	}
	return true;
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

OSStatus FSGetCatalogInfo(const FSRef* ref, uint32_t infoBits, FSCatalogInfo* infoOut, HFSUniStr255* nameOut, FSSpecPtr fsspec, FSRef* parentDir)
{
	std::string path;

	if (!FSRefMakePath(ref, path))
		return fnfErr;

	if (nameOut)
	{
		iconv_t ic = iconv_open("UTF-16", "UTF-8");
		size_t s;
		const char* inbuf = path.c_str();
		char* outbuf = reinterpret_cast<char*>(nameOut->unicode);
		size_t inbytesleft = path.size(), outbytesleft = sizeof(nameOut->unicode);

		if (ic == iconv_t(-1))
			return -1;

		memset(nameOut->unicode, 0, outbytesleft);
		s = iconv(ic, (char**) &inbuf, &inbytesleft, &outbuf, &outbytesleft);

		iconv_close(ic);
		if (s == size_t(-1))
			return -1;
	}

	if (parentDir)
	{
		memcpy(parentDir, ref, sizeof(FSRef));
		ino_t* last = std::find(parentDir->inodes, parentDir->inodes+FSRef_MAX_DEPTH, 0);

		if (last != parentDir->inodes)
			*(last-1) = 0;
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
				infoOut->nodeFlags = 4;
		}
	
		if (infoBits & (kFSCatInfoParentDirID|kFSCatInfoNodeID))
		{
			if (infoBits & kFSCatInfoNodeID)
				infoOut->nodeID = ref->inodes[0];
			for (int i = FSRef_MAX_DEPTH-1; i > 0; i--)
			{
				if (ref->inodes[i] == 0)
					continue;
				
				if (infoBits & kFSCatInfoParentDirID)
					infoOut->parentDirID = ref->inodes[i-1];
				if (infoBits & kFSCatInfoNodeID)
					infoOut->nodeID = ref->inodes[i];
			}
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

			infoOut->fsPermissionInfo.userID = st.st_uid;
			infoOut->fsPermissionInfo.groupID = st.st_gid;
			infoOut->fsPermissionInfo.mode = st.st_mode & 07777;
			infoOut->fsPermissionInfo.userAccess = uaccess;
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

	return noErr;
}

bool hasgid(gid_t gid)
{
	gid_t* gids;
	int count;

	if (getegid() == gid)
		return true;

	while (true)
	{
		count = getgroups(0, nullptr);
		if (count == -1 && errno == EINVAL)
			continue;

		gids = (gid_t*) alloca(sizeof(gid_t)*count);
		if (getgroups(count, gids) != count)
		{
			count = 0;
			break;
		}
	}

	return std::find(gids, gids+count, gid) != (gids+count);
}


Boolean CFURLGetFSRef(CFURLRef urlref, FSRef* fsref)
{
	char* buf;
	CFIndex len;
	CFStringRef sref = CFURLCopyFileSystemPath(urlref, kCFURLPOSIXPathStyle);

	if (!sref)
		return false;

	len = CFStringGetLength(sref)*4+1;
	buf = new char[len];

	if (!CFStringGetCString(sref, buf, len, kCFStringEncodingUTF8))
	{
		delete [] buf;
		return false;
	}

	CFRelease(sref);

	OSErr err = FSPathMakeRef((uint8_t*) buf, fsref, nullptr);
	delete [] buf;

	return err == noErr;
}

