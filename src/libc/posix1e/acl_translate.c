/*
 * Copyright (c) 2004, 2010, 2011 Apple Inc. All rights reserved.
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

#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <membership.h>
#include <membershipPriv.h>
#include <pwd.h>
#include <grp.h>

#include <libkern/OSByteOrder.h>

#include "aclvar.h"

/*
 * NOTE: the copy_int/copy_ext functions are duplicated here, one version of each for
 * each of native and portable endianity.  A more elegant solution might be called for
 * if the functions become much more complicated.
 */

/*
 * acl_t -> external representation, portable endianity
 */
ssize_t
acl_copy_ext(void *buf, acl_t acl, ssize_t size)
{
	struct kauth_filesec *ext = (struct kauth_filesec *)buf;
	ssize_t		reqsize;
	int		i;

	/* validate arguments, compute required size */
	reqsize = acl_size(acl);
	if (reqsize < 0)
		return(-1);
	if (reqsize > size) {
		errno = ERANGE;
		return(-1);
	}

	bzero(ext, reqsize);
	ext->fsec_magic = OSSwapHostToBigInt32(KAUTH_FILESEC_MAGIC);

	/* special case for _FILESEC_REMOVE_ACL */
	if (acl == (acl_t)_FILESEC_REMOVE_ACL) {
		ext->fsec_entrycount = OSSwapHostToBigInt32(KAUTH_FILESEC_NOACL);
		return(reqsize);
	}
		
	/* export the header */
	ext->fsec_entrycount = OSSwapHostToBigInt32(acl->a_entries);
	ext->fsec_flags = OSSwapHostToBigInt32(acl->a_flags);
	
	/* copy ACEs */
	for (i = 0; i < acl->a_entries; i++) {
		/* ACE contents are almost identical */
		ext->fsec_ace[i].ace_applicable = acl->a_ace[i].ae_applicable;
		ext->fsec_ace[i].ace_flags =
		    OSSwapHostToBigInt32((acl->a_ace[i].ae_tag & KAUTH_ACE_KINDMASK) | (acl->a_ace[i].ae_flags & ~KAUTH_ACE_KINDMASK));
		ext->fsec_ace[i].ace_rights = OSSwapHostToBigInt32(acl->a_ace[i].ae_perms);
	}		

	return(reqsize);
}

/*
 * acl_t -> external representation, native system endianity
 */
ssize_t
acl_copy_ext_native(void *buf, acl_t acl, ssize_t size)
{
	struct kauth_filesec *ext = (struct kauth_filesec *)buf;
	ssize_t		reqsize;
	int		i;

	/* validate arguments, compute required size */
	reqsize = acl_size(acl);
	if (reqsize < 0)
		return(-1);
	if (reqsize > size) {
		errno = ERANGE;
		return(-1);
	}

	bzero(ext, reqsize);
	ext->fsec_magic = KAUTH_FILESEC_MAGIC;

	/* special case for _FILESEC_REMOVE_ACL */
	if (acl == (acl_t)_FILESEC_REMOVE_ACL) {
		ext->fsec_entrycount = KAUTH_FILESEC_NOACL;
		return(reqsize);
	}
		
	/* export the header */
	ext->fsec_entrycount = acl->a_entries;
	ext->fsec_flags = acl->a_flags;
	
	/* copy ACEs */
	for (i = 0; i < acl->a_entries; i++) {
		/* ACE contents are almost identical */
		ext->fsec_ace[i].ace_applicable = acl->a_ace[i].ae_applicable;
		ext->fsec_ace[i].ace_flags =
		    (acl->a_ace[i].ae_tag & KAUTH_ACE_KINDMASK) |
		    (acl->a_ace[i].ae_flags & ~KAUTH_ACE_KINDMASK);
		ext->fsec_ace[i].ace_rights = acl->a_ace[i].ae_perms;
	}		

	return(reqsize);
}

/*
 * external representation, portable system endianity -> acl_t
 *
 * Unlike acl_copy_ext, we can't mung the buffer as it doesn't belong to us.
 */
acl_t
acl_copy_int(const void *buf)
{
	struct kauth_filesec *ext = (struct kauth_filesec *)buf;
	acl_t		ap;
	int		i;

	if (ext->fsec_magic != OSSwapHostToBigInt32(KAUTH_FILESEC_MAGIC)) {
		errno = EINVAL;
		return(NULL);
	}

	if ((ap = acl_init(OSSwapBigToHostInt32(ext->fsec_entrycount))) != NULL) {
		/* copy useful header fields */
		ap->a_flags = OSSwapBigToHostInt32(ext->fsec_flags);
		ap->a_entries = OSSwapBigToHostInt32(ext->fsec_entrycount);
		/* copy ACEs */
		for (i = 0; i < ap->a_entries; i++) {
			/* ACE contents are literally identical */
			ap->a_ace[i].ae_magic = _ACL_ENTRY_MAGIC;
			ap->a_ace[i].ae_applicable = ext->fsec_ace[i].ace_applicable;
			ap->a_ace[i].ae_flags = OSSwapBigToHostInt32(ext->fsec_ace[i].ace_flags) & ~KAUTH_ACE_KINDMASK;
			ap->a_ace[i].ae_tag = OSSwapBigToHostInt32(ext->fsec_ace[i].ace_flags) & KAUTH_ACE_KINDMASK;
			ap->a_ace[i].ae_perms = OSSwapBigToHostInt32(ext->fsec_ace[i].ace_rights);
		}
	}
	return(ap);
}

/*
 * external representation, native system endianity -> acl_t
 */
acl_t
acl_copy_int_native(const void *buf)
{
	struct kauth_filesec *ext = (struct kauth_filesec *)buf;
	acl_t		ap;
	int		i;

	if (ext->fsec_magic != KAUTH_FILESEC_MAGIC) {
		errno = EINVAL;
		return(NULL);
	}

	if ((ap = acl_init(ext->fsec_entrycount)) != NULL) {
		/* copy useful header fields */
		ap->a_flags = ext->fsec_flags;
		ap->a_entries = ext->fsec_entrycount;
		/* copy ACEs */
		for (i = 0; i < ap->a_entries; i++) {
			/* ACE contents are literally identical */
			ap->a_ace[i].ae_magic = _ACL_ENTRY_MAGIC;
			ap->a_ace[i].ae_applicable = ext->fsec_ace[i].ace_applicable;
			ap->a_ace[i].ae_flags = ext->fsec_ace[i].ace_flags & ~KAUTH_ACE_KINDMASK;
			ap->a_ace[i].ae_tag = ext->fsec_ace[i].ace_flags & KAUTH_ACE_KINDMASK;
			ap->a_ace[i].ae_perms = ext->fsec_ace[i].ace_rights;
		}
	}
	return(ap);
}

#define ACL_TYPE_DIR	(1<<0)
#define ACL_TYPE_FILE	(1<<1)
#define ACL_TYPE_ACL	(1<<2)

static struct {
	acl_perm_t	perm;
	char		*name;
	int		type;
} acl_perms[] = {
	{ACL_READ_DATA,		"read",		ACL_TYPE_FILE},
//	{ACL_LIST_DIRECTORY,	"list",		ACL_TYPE_DIR},
	{ACL_WRITE_DATA,	"write",	ACL_TYPE_FILE},
//	{ACL_ADD_FILE,		"add_file",	ACL_TYPE_DIR},
	{ACL_EXECUTE,		"execute",	ACL_TYPE_FILE},
//	{ACL_SEARCH,		"search",	ACL_TYPE_DIR},
	{ACL_DELETE,		"delete",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_APPEND_DATA,	"append",	ACL_TYPE_FILE},
//	{ACL_ADD_SUBDIRECTORY,	"add_subdirectory", ACL_TYPE_DIR},
	{ACL_DELETE_CHILD,	"delete_child",	ACL_TYPE_DIR},
	{ACL_READ_ATTRIBUTES,	"readattr",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_WRITE_ATTRIBUTES,	"writeattr",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_READ_EXTATTRIBUTES, "readextattr",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_WRITE_EXTATTRIBUTES, "writeextattr", ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_READ_SECURITY,	"readsecurity",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_WRITE_SECURITY,	"writesecurity", ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_CHANGE_OWNER,	"chown",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_SYNCHRONIZE,	"synchronize",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{0, NULL, 0}
};

static struct {
	acl_flag_t	flag;
	char		*name;
	int		type;
} acl_flags[] = {
	{ACL_ENTRY_INHERITED,		"inherited",		ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_FLAG_DEFER_INHERIT,	"defer_inherit",	ACL_TYPE_ACL},
	{ACL_ENTRY_FILE_INHERIT,	"file_inherit",		ACL_TYPE_DIR},
	{ACL_ENTRY_DIRECTORY_INHERIT,	"directory_inherit",	ACL_TYPE_DIR},
	{ACL_ENTRY_LIMIT_INHERIT,	"limit_inherit",	ACL_TYPE_FILE | ACL_TYPE_DIR},
	{ACL_ENTRY_ONLY_INHERIT,	"only_inherit",		ACL_TYPE_DIR},
	{ACL_FLAG_NO_INHERIT,		"no_inherit",		ACL_TYPE_ACL},
	{0, NULL, 0}
};

/*
 * reallocing snprintf with offset
 */
static int
raosnprintf(char **buf, size_t *size, ssize_t *offset, char *fmt, ...)
{
    va_list ap;
    int ret;

    do
    {
	if (*offset < *size)
	{
	    va_start(ap, fmt);
	    ret = vsnprintf(*buf + *offset, *size - *offset, fmt, ap);
	    va_end(ap);
	    if (ret < (*size - *offset))
	    {
		*offset += ret;
		return ret;
	    }
	}
	*buf = reallocf(*buf, (*size *= 2));
    } while (*buf);

    //warn("reallocf failure");
    return 0;
}

static char *
uuid_to_name(uuid_t *uu, uid_t *id, int *isgid)
{
    struct group *tgrp = NULL;
    struct passwd *tpass = NULL;

    if (0 == mbr_uuid_to_id(*uu, id, isgid))
    {
	switch (*isgid)
	{
	    case ID_TYPE_UID:
		if (!(tpass = getpwuid(*id)))
		    goto errout;
		return strdup(tpass->pw_name);
		break;
	    case ID_TYPE_GID:
		if (!(tgrp = getgrgid((gid_t) *id)))
		    goto errout;
		return strdup(tgrp->gr_name);
		break;
	    default:
errout:		;    //warn("Unable to translate qualifier on ACL\n");
	}
    }
    return NULL;
}

acl_t
acl_from_text(const char *buf_p)
{
    int i, error = 0, need_tag, ug_tag;
    char *buf, *orig_buf;
    char *entry, *field, *sub;
    uuid_t *uu = NULL;
    struct passwd *tpass = NULL;
    struct group *tgrp = NULL;
    acl_entry_t acl_entry;
    acl_flagset_t flags = NULL;
    acl_permset_t perms = NULL;
    acl_tag_t tag;
    acl_t acl_ret;

    if (buf_p == NULL)
    {
	errno = EINVAL;
	return NULL;
    }

    if ((buf = strdup(buf_p)) == NULL)
	return NULL;

    if ((acl_ret = acl_init(1)) == NULL)
	return NULL;

    orig_buf = buf;

    /* global acl flags
     * format: !#acl <version> [<flags>]
     */
    if ((entry = strsep(&buf, "\n")) != NULL && *entry)
    {
	/* field 1: !#acl */
	field = strsep(&entry, " ");
	if (*field && strncmp(field, "!#acl", strlen("!#acl")))
	{
	    error = EINVAL;
	    goto exit;
	}

	/* field 2: <version>
	 * currently only accepts 1
	 */
	field = strsep(&entry, " ");
	errno = 0;
	if (!*field || strtol(field, NULL, 0) != 1)
	{
	    error = EINVAL;
	    goto exit;
	}

	/* field 3: <flags>
	 * optional
	 */
	if((field = strsep(&entry, " ")) != NULL && *field)
	{
	    acl_get_flagset_np(acl_ret, &flags);
	    while ((sub = strsep(&field, ",")) && *sub)
	    {
		for (i = 0; acl_flags[i].name != NULL; ++i)
		{
		    if (acl_flags[i].type & ACL_TYPE_ACL
			    && !strcmp(acl_flags[i].name, sub))
		    {
			acl_add_flag_np(flags, acl_flags[i].flag);
			break;
		    }
		}
		if (acl_flags[i].name == NULL)
		{
		    /* couldn't find flag */
		    error = EINVAL;
		    goto exit;
		}
	    }
	}
    } else {
	error = EINVAL;
	goto exit;
    }

    /* parse each acl line
     * format: <user|group>:
     *	    [<uuid>]:
     *	    [<user|group>]:
     *	    [<uid|gid>]:
     *	    <allow|deny>[,<flags>]
     *	    [:<permissions>[,<permissions>]]
     *
     * only one of the user/group identifies is required
     * the first one found is used
     */
    while ((entry = strsep(&buf, "\n")) && *entry)
    {
	need_tag = 1;
	ug_tag = -1;

	/* field 1: <user|group> */
	field = strsep(&entry, ":");

	if(uu)
	    bzero(uu, sizeof(uuid_t));
	else if((uu = calloc(1, sizeof(uuid_t))) == NULL)
	{
	    error = errno;
	    goto exit;
	}

	if(acl_create_entry(&acl_ret, &acl_entry))
	{
	    error = errno;
	    goto exit;
	}

	if (-1 == acl_get_flagset_np(acl_entry, &flags)
	 || -1 == acl_get_permset(acl_entry, &perms))
	{
	    error = errno;
	    goto exit;
	}

	switch(*field)
	{
	    case 'u':
		if(!strcmp(field, "user"))
		    ug_tag = ID_TYPE_UID;
		break;
	    case 'g':
		if(!strcmp(field, "group"))
		    ug_tag = ID_TYPE_GID;
		break;
	    default:
		error = EINVAL;
		goto exit;
	}

	/* field 2: <uuid> */
	if ((field = strsep(&entry, ":")) != NULL && *field)
	{
	    uuid_parse(field, *uu);
	    need_tag = 0;
	}

	/* field 3: <username|groupname> */
	if ((field = strsep(&entry, ":")) != NULL && *field && need_tag)
	{
	    switch(ug_tag)
	    {
		case ID_TYPE_UID:
		    if((tpass = getpwnam(field)) != NULL)
			if (mbr_uid_to_uuid(tpass->pw_uid, *uu) != 0)
			{
			    error = EINVAL;
			    goto exit;
			}
		    break;
		case ID_TYPE_GID:
		    if ((tgrp = getgrnam(field)) != NULL)
			if (mbr_gid_to_uuid(tgrp->gr_gid, *uu) != 0)
			{
			    error = EINVAL;
			    goto exit;
			}
		    break;
		default:
		    error = EINVAL;
		    goto exit;
	    }
	    need_tag = 0;
	}

	/* field 4: <uid|gid> */
	if ((field = strsep(&entry, ":")) != NULL && *field && need_tag)
	{
	    uid_t id;
	    error = 0;

	    if((id = strtol(field, NULL, 10)) == 0 && error)
	    {
		error = EINVAL;
		goto exit;
	    }

	    switch(ug_tag)
	    {
		case ID_TYPE_UID:
		    if((tpass = getpwuid((uid_t)id)) != NULL)
			if (mbr_uid_to_uuid(tpass->pw_uid, *uu) != 0)
			{
			    error = EINVAL;
			    goto exit;
			}
		    break;
		case ID_TYPE_GID:
		    if ((tgrp = getgrgid((gid_t)id)) != NULL)
			if (mbr_gid_to_uuid(tgrp->gr_gid, *uu) != 0)
			{
			    error = EINVAL;
			    goto exit;
			}
		    break;
	    }
	    need_tag = 0;
	}

	/* sanity check: nothing set as qualifier */
	if (need_tag)
	{
	    error = EINVAL;
	    goto exit;
	}

	/* field 5: <flags> */
	if((field = strsep(&entry, ":")) == NULL || !*field)
	{
	    error = EINVAL;
	    goto exit;
	}

	for (tag = 0; (sub = strsep(&field, ",")) && *sub;)
	{
	    if (!tag)
	    {
		if (!strcmp(sub, "allow"))
		    tag = ACL_EXTENDED_ALLOW;
		else if (!strcmp(sub, "deny"))
		    tag = ACL_EXTENDED_DENY;
		else {
		    error = EINVAL;
		    goto exit;
		}
		continue;
	    }

	    for (i = 0; acl_flags[i].name != NULL; ++i)
	    {
		if (acl_flags[i].type & (ACL_TYPE_FILE | ACL_TYPE_DIR)
			&& !strcmp(acl_flags[i].name, sub))
		{
		    acl_add_flag_np(flags, acl_flags[i].flag);
		    break;
		}
	    }
	    if (acl_flags[i].name == NULL)
	    {
		/* couldn't find perm */
		error = EINVAL;
		goto exit;
	    }
	}

	/* field 6: <perms> (can be empty) */
	if((field = strsep(&entry, ":")) != NULL && *field)
	{
	    while ((sub = strsep(&field, ",")) && *sub)
	    {
		for (i = 0; acl_perms[i].name != NULL; i++)
		{
		    if (acl_perms[i].type & (ACL_TYPE_FILE | ACL_TYPE_DIR)
			    && !strcmp(acl_perms[i].name, sub))
		    {
			acl_add_perm(perms, acl_perms[i].perm);
			break;
		    }
		}
		if (acl_perms[i].name == NULL)
		{
		    /* couldn't find perm */
		    error = EINVAL;
		    goto exit;
		}
	    }
	}
	acl_set_tag_type(acl_entry, tag);
	acl_set_qualifier(acl_entry, *uu);
    }
exit:
    if(uu)
	free(uu);
    free(orig_buf);
    if (error)
    {
	acl_free(acl_ret);
	acl_ret = NULL;
	errno = error;
    }
    return acl_ret;
}

char *
acl_to_text(acl_t acl, ssize_t *len_p)
{
	acl_tag_t tag;
	acl_entry_t entry = NULL;
	acl_flagset_t flags;
	acl_permset_t perms;
	uid_t id;
	int i, first;
	int isgid;
	size_t bufsize = 1024;
	char *buf = NULL;

	if (!_ACL_VALID_ACL(acl)) {
		errno = EINVAL;
		return NULL;
	}

	if (len_p == NULL)
	    if ((len_p = alloca(sizeof(ssize_t))) == NULL)
		goto err_nomem;

	*len_p = 0;

	if ((buf = malloc(bufsize)) == NULL)
	    goto err_nomem;

	if (!raosnprintf(&buf, &bufsize, len_p, "!#acl %d", 1))
	    goto err_nomem;

	if (acl_get_flagset_np(acl, &flags) == 0)
	{
	    for (i = 0, first = 0; acl_flags[i].name != NULL; ++i)
	    {
		if (acl_flags[i].type & ACL_TYPE_ACL
			&& acl_get_flag_np(flags, acl_flags[i].flag) != 0)
		{
		    if(!raosnprintf(&buf, &bufsize, len_p, "%s%s",
			    first++ ? "," : " ", acl_flags[i].name))
			goto err_nomem;
		}
	    }
	}
	for (;acl_get_entry(acl,
		    entry == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY, &entry) == 0;)
	{
	    int valid;
	    uuid_t *uu;
	    char *str, uu_str[37];

	    if (((uu = (uuid_t *) acl_get_qualifier(entry)) == NULL)
		|| (acl_get_tag_type(entry, &tag) != 0)
		|| (acl_get_flagset_np(entry, &flags) != 0)
		|| (acl_get_permset(entry, &perms) != 0)) {
		if (uu != NULL) acl_free(uu);
		continue;
	    }

	    uuid_unparse_upper(*uu, uu_str);

	    if ((str = uuid_to_name(uu, &id, &isgid)) != NULL) {
		valid = raosnprintf(&buf, &bufsize, len_p, "\n%s:%s:%s:%d:%s",
		    isgid ? "group" : "user",
		    uu_str,
		    str,
		    id,
		    (tag == ACL_EXTENDED_ALLOW) ? "allow" : "deny");
	    } else {
		valid = raosnprintf(&buf, &bufsize, len_p, "\nuser:%s:::%s",
		    uu_str,
		    (tag == ACL_EXTENDED_ALLOW) ? "allow" : "deny");
	    }

	    free(str);
	    acl_free(uu);

	    if (!valid)
		goto err_nomem;

	    for (i = 0; acl_flags[i].name != NULL; ++i)
	    {
		if (acl_flags[i].type & (ACL_TYPE_DIR | ACL_TYPE_FILE))
		{
		    if(acl_get_flag_np(flags, acl_flags[i].flag) != 0)
		    {
			if(!raosnprintf(&buf, &bufsize, len_p, ",%s",
			    acl_flags[i].name))
			    goto err_nomem;
		    }
		}
	    }

	    for (i = 0, first = 0; acl_perms[i].name != NULL; ++i)
	    {
		if (acl_perms[i].type & (ACL_TYPE_DIR | ACL_TYPE_FILE))
		{
		    if(acl_get_perm_np(perms, acl_perms[i].perm) != 0)
		    {
			if(!raosnprintf(&buf, &bufsize, len_p, "%s%s",
			    first++ ? "," : ":", acl_perms[i].name))
			    goto err_nomem;
		    }
		}
	    }
	}

	if(!raosnprintf(&buf, &bufsize, len_p, "\n")) goto err_nomem;
	return buf;

err_nomem:
	if (buf != NULL)
	    free(buf);

	errno = ENOMEM;
	return NULL;
}

ssize_t
acl_size(acl_t acl)
{
	/* special case for _FILESEC_REMOVE_ACL */
	if (acl == (acl_t)_FILESEC_REMOVE_ACL)
		return KAUTH_FILESEC_SIZE(0);

	_ACL_VALIDATE_ACL(acl);

	return(KAUTH_FILESEC_SIZE(acl->a_entries));
}
