/*-
 * Copyright (c) 2002 Tim J. Robbins.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
/* <rdar://problem/13875458> */

#include <wordexp.h>
int wordexp(const char *restrict words __unused, wordexp_t *restrict pwordexp __unused, int flags __unused) {
    return WRDE_NOSPACE;
}

void wordfree(wordexp_t *pwordexp __unused) {
}

#else

#include "namespace.h"
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>
#include "un-namespace.h"
#ifdef __APPLE__
// For _NSGetEnviron() -- which gives us a pointer to environ
#include <crt_externs.h>
#include <spawn.h>
#endif /* __APPLE__ */

__FBSDID("$FreeBSD$");

#ifdef __APPLE__
/*
 * To maintain backwards compatibility with wordexp_t, we can't put
 * we_strings and we_nbytes in wordexp_t.  So we create a new structure,
 * we_int_t, that has wi_strings and wi_nbytes, as well as the we_wordv
 * storage.
 */
typedef struct {
	char	*wi_strings;
	size_t	wi_nbytes;
	char	*wi_wordv[0];
} we_int_t;
/*
 * Normally, we_wordv will point to wi_wordv, so we need macros to convert
 * back and forth between wi_wordv and the we_int_t structure.
 */
#define WE_INT_T(x)	((we_int_t *)((char *)(x) - sizeof(we_int_t)))
#define WE_STRINGS(we)	(WE_INT_T((we)->we_wordv)->wi_strings)
#define WE_WORDV(x)	((x)->wi_wordv)

/*
 * bash will return success, yet print a command substitution/syntax error
 * to stderr.  So we need to capture stderr, and see if it contains this error.
 */
static const char command_substitution_str[] = "command substitution";
static const char syntax_error_str[] = "syntax error";
static const char unbound_variable_str[] = " unbound variable";
#endif /* __APPLE__ */

static int	we_askshell(const char *, wordexp_t *, int);
static int	we_check(const char *, int);

/*
 * wordexp --
 *	Perform shell word expansion on `words' and place the resulting list
 *	of words in `we'. See wordexp(3).
 *
 *	Specified by IEEE Std. 1003.1-2001.
 */
int
#ifdef __APPLE__
wordexp(const char * __restrict words, wordexp_t * __restrict we0, int flags)
#else /* !__APPLE__ */
wordexp(const char * __restrict words, wordexp_t * __restrict we, int flags)
#endif /* !__APPLE__ */
{
	int error;
#ifdef __APPLE__
	wordexp_t temp;
	wordexp_t *we = &temp;

	if ((error = we_check(words, flags)) != 0) return (error);
	we->we_offs = we0->we_offs;
	if (flags & WRDE_APPEND) {
		size_t i;
		size_t vofs = we0->we_wordc + (flags & WRDE_DOOFFS ? we0->we_offs : 0);
		we_int_t *wi0 = WE_INT_T(we0->we_wordv);
		we_int_t *wi = malloc((vofs + 1) * sizeof(char *) + sizeof(we_int_t));

		if (!wi) return (WRDE_NOSPACE);
		memcpy(wi, wi0, (vofs + 1) * sizeof(char *) + sizeof(we_int_t));
		wi->wi_strings = malloc(wi->wi_nbytes);
		if (!wi->wi_strings) {
			free(wi);
			return (WRDE_NOSPACE);
		}
		memcpy(wi->wi_strings, wi0->wi_strings, wi->wi_nbytes);
		for (i = 0; i < vofs; i++)
			if (wi->wi_wordv[i] != NULL)
				wi->wi_wordv[i] += wi->wi_strings - wi0->wi_strings;
		we->we_wordc = we0->we_wordc;
		we->we_wordv = WE_WORDV(wi);
	} else {
		we->we_wordc = 0;
		we->we_wordv = NULL;
	}
#else /* !__APPLE__ */
	if (flags & WRDE_REUSE)
		wordfree(we);
	if ((flags & WRDE_APPEND) == 0) {
		we->we_wordc = 0;
		we->we_wordv = NULL;
		we->we_strings = NULL;
		we->we_nbytes = 0;
	}
	if ((error = we_check(words, flags)) != 0) {
		wordfree(we);
		return (error);
	}
#endif /* !__APPLE__ */
	if ((error = we_askshell(words, we, flags)) != 0) {
#ifdef __APPLE__
		if (error == WRDE_NOSPACE) {
			if (flags & WRDE_REUSE)
				wordfree(we0);
			*we0 = *we;
		} else {
			wordfree(we);
		}
#else /* !__APPLE__ */
		wordfree(we);
#endif /* !__APPLE__ */
		return (error);
	}
#ifdef __APPLE__
	if (flags & WRDE_REUSE)
		wordfree(we0);
	*we0 = *we;
#endif /* __APPLE__ */
	return (0);
}

static size_t
we_read_fully(int fd, char *buffer, size_t len)
{
	size_t done;
	ssize_t nread;

	done = 0;
	do {
		nread = _read(fd, buffer + done, len - done);
		if (nread == -1 && errno == EINTR)
			continue;
		if (nread <= 0)
			break;
		done += nread;
	} while (done != len);
	return done;
}

/*
 * we_askshell --
 *	Use the `wordexp' /bin/sh builtin function to do most of the work
 *	in expanding the word string. This function is complicated by
 *	memory management.
 */
static int
we_askshell(const char *words, wordexp_t *we, int flags)
{
	int pdes[2];			/* Pipe to child */
	char bbuf[9];			/* Buffer for byte count */
	char wbuf[9];			/* Buffer for word count */
	long nwords, nbytes;		/* Number of words, bytes from child */
	long i;				/* Handy integer */
	size_t sofs;			/* Offset into we->we_strings */
	size_t vofs;			/* Offset into we->we_wordv */
	pid_t pid;			/* Process ID of child */
	pid_t wpid;			/* waitpid return value */
	int status;			/* Child exit status */
	int error;			/* Our return value */
	int serrno;			/* errno to return */
	char *np, *p;			/* Handy pointers */
	char *nstrings;			/* Temporary for realloc() */
#ifdef __APPLE__
	int perr[2];			/* Pipe to child error */
	we_int_t *nwv;			/* Temporary for realloc() */
	int spawnerr = 0;
	posix_spawn_file_actions_t file_actions;
	posix_spawnattr_t attr;
#else /* !__APPLE__ */
	char **nwv;			/* Temporary for realloc() */
#endif /* !__APPLE__ */
	sigset_t newsigblock, oldsigblock;

	serrno = errno;

#ifdef __APPLE__
	if (pipe(pdes) < 0)
#else /* !__APPLE__ */
	if (pipe2(pdes, O_CLOEXEC) < 0)
#endif /* !__APPLE__ */
		return (WRDE_NOSPACE);	/* XXX */
#ifdef __APPLE__
	if (pipe(perr) < 0) {
		close(pdes[0]);
		close(pdes[1]);
		return (WRDE_NOSPACE);	/* XXX */
	}
#endif /* __APPLE__ */
	(void)sigemptyset(&newsigblock);
	(void)sigaddset(&newsigblock, SIGCHLD);
	(void)_sigprocmask(SIG_BLOCK, &newsigblock, &oldsigblock);
#ifdef __APPLE__
	if ((spawnerr = posix_spawnattr_init(&attr)) != 0) goto spawnerrexit;
	do {
		sigset_t spawnsig;
		if ((spawnerr = posix_spawnattr_setflags(&attr, POSIX_SPAWN_CLOEXEC_DEFAULT)) != 0) break;
		(void)sigfillset(&spawnsig);
		if ((spawnerr = posix_spawnattr_setsigdefault(&attr, &spawnsig)) != 0) break;
		(void)sigemptyset(&spawnsig);
		if ((spawnerr = posix_spawnattr_setsigmask(&attr, &spawnsig)) != 0) break;
		if ((spawnerr = posix_spawn_file_actions_init(&file_actions)) != 0) break;
		do {
			char *argv[7] = {"sh"};
			const char cmd[] = "[ $# -gt 0 ] && export IFS=\"$1\";/usr/lib/system/wordexp-helper ";
			int a = 1;
			char *buf;
			int buflen;
			char *IFS;
			if (pdes[1] == STDOUT_FILENO) {
				if ((spawnerr = posix_spawn_file_actions_addinherit_np(&file_actions, STDOUT_FILENO)) != 0) break;
			} else {
				if ((spawnerr = posix_spawn_file_actions_adddup2(&file_actions, pdes[1], STDOUT_FILENO)) != 0) break;
			}
			if (perr[1] == STDERR_FILENO) {
				if ((spawnerr = posix_spawn_file_actions_addinherit_np(&file_actions, STDERR_FILENO)) != 0) break;
			} else {
				if ((spawnerr = posix_spawn_file_actions_adddup2(&file_actions, perr[1], STDERR_FILENO)) != 0) break;
			}
			if (flags & WRDE_UNDEF) argv[a++] = "-u";
			argv[a++] = "-c";
			buflen = (sizeof(cmd) - 1) + strlen(words) + 1;
			if ((buf = malloc(buflen)) == NULL) {
				spawnerr = errno;
				break;
			}
			strcpy(buf, cmd);
			strcat(buf, words);
			argv[a++] = buf;
			if ((IFS = getenv("IFS")) != NULL) {
				argv[a++] = "--";
				argv[a++] = IFS;
			}
			argv[a] = NULL;
			spawnerr = posix_spawn(&pid, _PATH_BSHELL, &file_actions, &attr, argv, *_NSGetEnviron());
			free(buf);
		} while(0);
		posix_spawn_file_actions_destroy(&file_actions);
	} while(0);
	posix_spawnattr_destroy(&attr);
	if (spawnerr) {
spawnerrexit:
		close(pdes[0]);
		close(pdes[1]);
		close(perr[0]);
		close(perr[1]);
		errno = spawnerr;
		return (WRDE_NOSPACE);	/* XXX */
	}
#else /* !__APPLE__ */
	if ((pid = fork()) < 0) {
		serrno = errno;
		_close(pdes[0]);
		_close(pdes[1]);
		(void)_sigprocmask(SIG_SETMASK, &oldsigblock, NULL);
		errno = serrno;
		return (WRDE_NOSPACE);	/* XXX */
	}
	else if (pid == 0) {
		/*
		 * We are the child; just get /bin/sh to run the wordexp
		 * builtin on `words'.
		 */
		(void)_sigprocmask(SIG_SETMASK, &oldsigblock, NULL);
		if ((pdes[1] != STDOUT_FILENO ?
		    _dup2(pdes[1], STDOUT_FILENO) :
		    _fcntl(pdes[1], F_SETFD, 0)) < 0)
			_exit(1);
		execl(_PATH_BSHELL, "sh", flags & WRDE_UNDEF ? "-u" : "+u",
		    "-c", "eval \"$1\";eval \"wordexp $2\"", "",
		    flags & WRDE_SHOWERR ? "" : "exec 2>/dev/null", words,
		    (char *)NULL);
		_exit(1);
	}
#endif /* !__APPLE__ */

	/*
	 * We are the parent; read the output of the shell wordexp function,
	 * which is a 32-bit hexadecimal word count, a 32-bit hexadecimal
	 * byte count (not including terminating null bytes), followed by
	 * the expanded words separated by nulls.
	 */
	_close(pdes[1]);
#ifdef __APPLE__
	close(perr[1]);
#endif /* __APPLE__ */
	if (we_read_fully(pdes[0], wbuf, 8) != 8 ||
			we_read_fully(pdes[0], bbuf, 8) != 8) {
		error = flags & WRDE_UNDEF ? WRDE_BADVAL : WRDE_SYNTAX;
		serrno = errno;
		goto cleanup;
	}
	wbuf[8] = bbuf[8] = '\0';
	nwords = strtol(wbuf, NULL, 16);
	nbytes = strtol(bbuf, NULL, 16) + nwords;

	/*
	 * Allocate or reallocate (when flags & WRDE_APPEND) the word vector
	 * and string storage buffers for the expanded words we're about to
	 * read from the child.
	 */
#ifndef __APPLE__
	sofs = we->we_nbytes;
#endif /* !__APPLE__ */
	vofs = we->we_wordc;
	if ((flags & (WRDE_DOOFFS|WRDE_APPEND)) == (WRDE_DOOFFS|WRDE_APPEND))
		vofs += we->we_offs;
	we->we_wordc += nwords;
#ifndef __APPLE__
	we->we_nbytes += nbytes;
#endif /* !__APPLE__ */
#ifdef __APPLE__
	if ((nwv = realloc(we->we_wordv ? WE_INT_T(we->we_wordv) : NULL, (we->we_wordc + 1 +
	    (flags & WRDE_DOOFFS ?  we->we_offs : 0)) *
	    sizeof(char *) + sizeof(we_int_t))) == NULL)
#else /* !__APPLE__ */
	if ((nwv = realloc(we->we_wordv, (we->we_wordc + 1 +
	    (flags & WRDE_DOOFFS ?  we->we_offs : 0)) *
	    sizeof(char *))) == NULL)
#endif /* !__APPLE__ */
	{
		error = WRDE_NOSPACE;
		goto cleanup;
	}
#ifdef __APPLE__
	if (!we->we_wordv) {
		nwv->wi_strings = NULL;
		nwv->wi_nbytes = 0;
	}
	sofs = nwv->wi_nbytes;
	nwv->wi_nbytes += nbytes;
	we->we_wordv = WE_WORDV(nwv);
#else /* !__APPLE__ */
	we->we_wordv = nwv;
#endif /* !__APPLE__ */
#ifdef __APPLE__
	if ((nstrings = realloc(nwv->wi_strings, nwv->wi_nbytes)) == NULL)
#else /* !__APPLE__ */
	if ((nstrings = realloc(we->we_strings, we->we_nbytes)) == NULL)
#endif /* !__APPLE__ */
	{
		error = WRDE_NOSPACE;
		goto cleanup;
	}
	for (i = 0; i < vofs; i++)
		if (we->we_wordv[i] != NULL)
#ifdef __APPLE__
			we->we_wordv[i] += nstrings - nwv->wi_strings;
#else /* !__APPLE__ */
			we->we_wordv[i] += nstrings - we->we_strings;
#endif /* !__APPLE__ */
#ifdef __APPLE__
	nwv->wi_strings = nstrings;
#else /* !__APPLE__ */
	we->we_strings = nstrings;
#endif /* !__APPLE__ */

#ifdef __APPLE__
	if (we_read_fully(pdes[0], nwv->wi_strings + sofs, nbytes) != nbytes)
#else /* !__APPLE__ */
	if (we_read_fully(pdes[0], we->we_strings + sofs, nbytes) != nbytes)
#endif /* !__APPLE__ */
	{
		error = flags & WRDE_UNDEF ? WRDE_BADVAL : WRDE_SYNTAX;
		serrno = errno;
		goto cleanup;
	}
#ifdef __APPLE__
	char err_buf[1024];
	ssize_t err_sz = read(perr[0], err_buf, sizeof(err_buf) - 1);
	if (err_sz > 0) {
		err_buf[err_sz] = '\0';
		if (flags & WRDE_SHOWERR) {
			fputs(err_buf, stderr);
		}
	} else if (err_sz < 0) {
		serrno = errno;
		error = WRDE_NOSPACE;
		goto cleanup;
	}
#endif /* __APPLE__ */

	error = 0;
cleanup:
	_close(pdes[0]);
#ifdef __APPLE__
	close(perr[0]);
#endif /* __APPLE__ */
	do
		wpid = _waitpid(pid, &status, 0);
	while (wpid < 0 && errno == EINTR);
	(void)_sigprocmask(SIG_SETMASK, &oldsigblock, NULL);
	if (error != 0) {
		errno = serrno;
		return (error);
	}
	if (wpid < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return (flags & WRDE_UNDEF ? WRDE_BADVAL : WRDE_SYNTAX);
#ifdef __APPLE__
	const char *cs = strstr(err_buf, command_substitution_str);
	if (cs && strstr(cs + (sizeof(command_substitution_str) - 1), syntax_error_str)) {
		return (strstr(err_buf, unbound_variable_str) ? WRDE_BADVAL : WRDE_SYNTAX);
	}
#endif /* __APPLE__ */

	/*
	 * Break the null-terminated expanded word strings out into
	 * the vector.
	 */
	if (vofs == 0 && flags & WRDE_DOOFFS)
		while (vofs < we->we_offs)
			we->we_wordv[vofs++] = NULL;
#ifdef __APPLE__
	p = nwv->wi_strings + sofs;
#else /* !__APPLE__ */
	p = we->we_strings + sofs;
#endif /* !__APPLE__ */
	while (nwords-- != 0) {
		we->we_wordv[vofs++] = p;
		if ((np = memchr(p, '\0', nbytes)) == NULL)
			return (WRDE_NOSPACE);	/* XXX */
		nbytes -= np - p + 1;
		p = np + 1;
	}
	we->we_wordv[vofs] = NULL;

	return (0);
}

/*
 * we_check --
 *	Check that the string contains none of the following unquoted
 *	special characters: <newline> |&;<>(){}
 *	or command substitutions when WRDE_NOCMD is set in flags.
 */
static int
we_check(const char *words, int flags)
{
	char c;
	int dquote, level, quote, squote;

	quote = squote = dquote = 0;
	while ((c = *words++) != '\0') {
		switch (c) {
		case '\\':
			if (squote == 0)
				quote ^= 1;
			continue;
		case '\'':
			if (quote + dquote == 0)
				squote ^= 1;
			break;
		case '"':
			if (quote + squote == 0)
				dquote ^= 1;
			break;
		case '`':
			if (quote + squote == 0 && flags & WRDE_NOCMD)
				return (WRDE_CMDSUB);
			while ((c = *words++) != '\0' && c != '`')
				if (c == '\\' && (c = *words++) == '\0')
					break;
			if (c == '\0')
				return (WRDE_SYNTAX);
			break;
		case '|': case '&': case ';': case '<': case '>':
		case '{': case '}': case '(': case ')': case '\n':
			if (quote + squote + dquote == 0)
				return (WRDE_BADCHAR);
			break;
		case '$':
			if ((c = *words++) == '\0')
				break;
			else if (quote + squote == 0 && c == '(') {
				if (flags & WRDE_NOCMD && *words != '(')
					return (WRDE_CMDSUB);
				level = 1;
				while ((c = *words++) != '\0') {
					if (c == '\\') {
						if ((c = *words++) == '\0')
							break;
					} else if (c == '(')
						level++;
					else if (c == ')' && --level == 0)
						break;
				}
				if (c == '\0' || level != 0)
					return (WRDE_SYNTAX);
			} else if (quote + squote == 0 && c == '{') {
				level = 1;
				while ((c = *words++) != '\0') {
					if (c == '\\') {
						if ((c = *words++) == '\0')
							break;
					} else if (c == '{')
						level++;
					else if (c == '}' && --level == 0)
						break;
				}
				if (c == '\0' || level != 0)
					return (WRDE_SYNTAX);
			} else
				--words;
			break;
		default:
			break;
		}
		quote = 0;
	}
	if (quote + squote + dquote != 0)
		return (WRDE_SYNTAX);

	return (0);
}

/*
 * wordfree --
 *	Free the result of wordexp(). See wordexp(3).
 *
 *	Specified by IEEE Std. 1003.1-2001.
 */
void
wordfree(wordexp_t *we)
{

	if (we == NULL)
		return;
#ifdef __APPLE__
	if (we->we_wordv) {
		free(WE_STRINGS(we));
		free(WE_INT_T(we->we_wordv));
	}
#else /* !__APPLE__ */
	free(we->we_wordv);
#endif /* !__APPLE__ */
#ifndef __APPLE__
	free(we->we_strings);
#endif /* !__APPLE__ */
	we->we_wordv = NULL;
#ifndef __APPLE__
	we->we_strings = NULL;
	we->we_nbytes = 0;
#endif /* !__APPLE__ */
	we->we_wordc = 0;
}

#endif /* TARGET_OS_IPHONE */
