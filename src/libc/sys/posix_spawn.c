/*
 * Copyright (c) 2006-2012 Apple Inc. All rights reserved.
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

/*
 * [SPN] Support for _POSIX_SPAWN
 */

#include <spawn.h>
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>	/* for OPEN_MAX, PATH_MAX */
#include <string.h>	/* for strlcpy() */
#include <paths.h>	/* for _PATH_DEFPATH */
#include <sys/stat.h>	/* for struct stat */

/*
 * posix_spawnp
 *
 * Description:	Create a new process from the process image corresponding to
 *		the supplied 'file' argument and the parent processes path
 *		environment.
 *
 * Parameters:	pid				Pointer to pid_t to receive the
 *						PID of the spawned process, if
 *						successful and 'pid' != NULL
 *		file				Name of image file to spawn
 *		file_actions			spawn file actions object which
 *						describes file actions to be
 *						performed during the spawn
 *		attrp				spawn attributes object which
 *						describes attributes to be
 *						applied during the spawn
 *		argv				argument vector array; NULL
 *						terminated
 *		envp				environment vector array; NULL
 *						terminated
 *
 * Returns:	0				Success
 *		!0				An errno value indicating the
 *						cause of the failure to spawn
 *
 * Notes:	Much of this function is derived from code from execvP() from
 *		exec.c in libc; this common code should be factored out at
 *		some point to prevent code duplication or desynchronization vs.
 *		bug fixes applied to one set of code but not the other.
 */
int
posix_spawnp(pid_t * __restrict pid, const char * __restrict file,
		const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t * __restrict attrp,
		char *const argv[ __restrict], char *const envp[ __restrict])
{
	const char *env_path;
	char *bp;
	char *cur;
	char *p;
	char **memp;
	int lp;
	int ln;
	int cnt;
	int err = 0;
	int eacces = 0;
	struct stat sb;
	char path_buf[PATH_MAX];

	if ((env_path = getenv("PATH")) == NULL)
		env_path = _PATH_DEFPATH;

	/* If it's an absolute or relative path name, it's easy. */
	if (index(file, '/')) {
		bp = (char *)file;
		cur = NULL;
		goto retry;
	}
	bp = path_buf;

	/* If it's an empty path name, fail in the usual POSIX way. */
	if (*file == '\0')
		return (ENOENT);

	if ((cur = alloca(strlen(env_path) + 1)) == NULL)
		return ENOMEM;
	strcpy(cur, env_path);
	while ((p = strsep(&cur, ":")) != NULL) {
		/*
		 * It's a SHELL path -- double, leading and trailing colons
		 * mean the current directory.
		 */
		if (*p == '\0') {
			p = ".";
			lp = 1;
		} else {
			lp = strlen(p);
		}
		ln = strlen(file);

		/*
		 * If the path is too long complain.  This is a possible
		 * security issue; given a way to make the path too long
		 * the user may spawn the wrong program.
		 */
		if (lp + ln + 2 > sizeof(path_buf)) {
			err = ENAMETOOLONG;
			goto done;
		}
		bcopy(p, path_buf, lp);
		path_buf[lp] = '/';
		bcopy(file, path_buf + lp + 1, ln);
		path_buf[lp + ln + 1] = '\0';

retry:		err = posix_spawn(pid, bp, file_actions, attrp, argv, envp);
		switch (err) {
		case E2BIG:
		case ENOMEM:
		case ETXTBSY:
			goto done;
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			break;
		case ENOEXEC:
			for (cnt = 0; argv[cnt]; ++cnt)
				;
			memp = alloca((cnt + 2) * sizeof(char *));
			if (memp == NULL) {
				/* errno = ENOMEM; XXX override ENOEXEC? */
				goto done;
			}
			memp[0] = "sh";
			memp[1] = bp;
			bcopy(argv + 1, memp + 2, cnt * sizeof(char *));
			err = posix_spawn(pid, _PATH_BSHELL, file_actions, attrp, memp, envp);
			goto done;
		default:
			/*
			 * EACCES may be for an inaccessible directory or
			 * a non-executable file.  Call stat() to decide
			 * which.  This also handles ambiguities for EFAULT
			 * and EIO, and undocumented errors like ESTALE.
			 * We hope that the race for a stat() is unimportant.
			 */
			if (stat(bp, &sb) != 0)
				break;
			if (err == EACCES) {
				eacces = 1;
				continue;
			}
			goto done;
		}
	}
	if (eacces)
		err = EACCES;
	else
		err = ENOENT;
done:
	return (err);
}
