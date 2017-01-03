/*
 * Copyright (c) 2005, 2008 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wordexp.h>
#include <pthread.h>
#include <regex.h>
#include <assert.h>
#include <unistd.h>
#include <paths.h>
#include <strings.h>
#include <spawn.h>
#include <sys/errno.h>

// For _NSGetEnviron() -- which gives us a pointer to environ
#include <crt_externs.h>

extern size_t malloc_good_size(size_t size);
extern int errno;

pthread_once_t re_init_c = PTHREAD_ONCE_INIT;
static regex_t re_cmd, re_goodchars, re_subcmd_syntax_err_kludge, re_quoted_string;

/* Similar to popen, but captures stderr for you.  Doesn't interoperate
  with pclose.  Call wait4 on your own */
static pid_t popen_oe(char *cmd, FILE **out, FILE **err) {
    int out_pipe[2], err_pipe[2];
    char *argv[4];
    pid_t pid;
    posix_spawn_file_actions_t file_actions;
    int errrtn;

    if ((errrtn = posix_spawn_file_actions_init(&file_actions)) != 0) {
	errno = errrtn;
	return 0;
    }
    if (pipe(out_pipe) < 0) {
	posix_spawn_file_actions_destroy(&file_actions);
	return 0;
    }
    if (pipe(err_pipe) < 0) {
	posix_spawn_file_actions_destroy(&file_actions);
	close(out_pipe[0]);
	close(out_pipe[1]);
	return 0;
    }

    if (out_pipe[1] != STDOUT_FILENO) {
	posix_spawn_file_actions_adddup2(&file_actions, out_pipe[1], STDOUT_FILENO);
	posix_spawn_file_actions_addclose(&file_actions, out_pipe[1]);
    }
    posix_spawn_file_actions_addclose(&file_actions, out_pipe[0]);
    if (err_pipe[1] != STDERR_FILENO) {
	posix_spawn_file_actions_adddup2(&file_actions, err_pipe[1], STDERR_FILENO);
	posix_spawn_file_actions_addclose(&file_actions, err_pipe[1]);
    }
    posix_spawn_file_actions_addclose(&file_actions, err_pipe[0]);

    argv[0] = "sh";
    argv[1] = "-c";
    argv[2] = cmd;
    argv[3] = NULL;

    errrtn = posix_spawn(&pid, _PATH_BSHELL, &file_actions, NULL, argv, *_NSGetEnviron());
    posix_spawn_file_actions_destroy(&file_actions);

    if (errrtn != 0) {
	close(out_pipe[0]);
	close(out_pipe[1]);
	close(err_pipe[0]);
	close(err_pipe[1]);
	errno = errrtn;
	return 0;
    }

    *out = fdopen(out_pipe[0], "r");
    assert(*out);
    close(out_pipe[1]);
    *err = fdopen(err_pipe[0], "r");
    assert(*err);
    close(err_pipe[1]);

    return pid;
}

static void re_init(void) {
    int rc = regcomp(&re_cmd, "(^|[^\\])(`|\\$\\([^(])", REG_EXTENDED|REG_NOSUB);
    /* XXX I'm not sure the { } stuff is correct,
      it may be overly restrictave */
    char *rx = "^([^\\\"'|&;<>(){}\n]"
      "|\\\\."
      "|'(\\\\\\\\|\\\\'|[^'])*'"
      "|\"(\\\\\\\\|\\\\\"|[^\"])*\""
      "|`(\\\\\\\\|\\\\`|[^`])*`"
      "|\\$\\(\\(([^)]|\\\\)*\\)\\)"  /* can't do nesting in a regex */
      "|\\$\\(([^)]|\\\\)*\\)"  /* can't do nesting in a regex */
      "|\\$\\{[^}]*\\}"
      /* XXX: { } ? */
      ")*$";
    rc = regcomp(&re_goodchars, rx,
      REG_EXTENDED|REG_NOSUB);

    rc = regcomp(&re_subcmd_syntax_err_kludge, 
      "command substitution.*syntax error", REG_EXTENDED|REG_NOSUB);

    rc = regcomp(&re_quoted_string, 
      "(^|[^\\])'(\\\\\\\\|\\\\'|[^'])*'", REG_EXTENDED|REG_NOSUB);
}

/* Returns zero if it can't realloc */
static int word_alloc(size_t want, wordexp_t *__restrict__ pwe, size_t *have) {
    if (want < *have) {
	return 1;
    }
    size_t bytes = malloc_good_size(sizeof(char *) * want * 2);
    pwe->we_wordv = reallocf(pwe->we_wordv, bytes);
    if (pwe->we_wordv) {
	*have = bytes / sizeof(char *);
	return 1;
    }
    return 0;
}

static int
cmd_search(const char *str) {
    regoff_t first = 0;
    regoff_t last = strlen(str);
    regmatch_t m = {first, last};
    int flags;

    if (last == 0) return REG_NOMATCH; /* empty string */

    flags = REG_STARTEND;
    while(regexec(&re_quoted_string, str, 1, &m, flags) == 0) {
	/*
	 * We have matched a single quoted string, from m.rm_so to m.rm_eo.
	 * So the (non-quote string) from first to m.rm_so needs to be
	 * checked for command substitution.  Then we use REG_STARTEND to
	 * look for any other single quote strings after this one.
	 */
	 regmatch_t head = {first, m.rm_so};
	 if (regexec(&re_cmd, str, 1, &head, flags) == 0) {
	     return 0; /* found a command substitution */
	 }
	 flags = REG_NOTBOL | REG_STARTEND;
	 m.rm_so = first = m.rm_eo;
	 m.rm_eo = last;
    }
    /* Check the remaining string */
     flags = REG_STARTEND;
     if (m.rm_so > 0) flags |= REG_NOTBOL;
     return regexec(&re_cmd, str, 1, &m, flags);
}

/* XXX this is _not_ designed to be fast */
/* wordexp is also rife with security "challenges", unless you pass it
  WRDE_NOCMD it *must* support subshell expansion, and even if you
  don't beause it has to support so much of the standard shell (all
  the odd little variable expansion options for example) it is hard
  to do without a subshell).  It is probbably just plan a Bad Idea
  to call in anything setuid, or executing remotely. */

int wordexp(const char *__restrict__ words,
  wordexp_t *__restrict__ pwe, int flags) {
    /* cbuf_l's inital value needs to be big enough for 'cmd' plus
      about 20 chars */
    size_t cbuf_l = 1024;
    char *cbuf = NULL;
    /* Put a NUL byte between each word, and at the end */
    char *cmd = "/usr/bin/perl -e 'print join(chr(0), @ARGV), chr(0)' -- ";
    size_t wordv_l = 0, wordv_i = 0;
    int rc;
    wordexp_t save;

    /* Some errors require us to leave pwe unchanged, so we save it here */
    save = *pwe;
    pthread_once(&re_init_c, re_init);

    if (flags & WRDE_NOCMD) {
	/* This attempts to match any backticks or $(...)'s, but there may be
	  other ways to do subshell expansion that the standard doesn't
	  cover, but I don't know of any -- failures here are a potential
	  security risk */
	rc = cmd_search(words);
	if (rc != REG_NOMATCH) {
	    /* Technically ==0 is WRDE_CMDSUB, and != REG_NOMATCH is
	      "some internal error", but failing to catch those here
	      could allow a subshell */
	    return WRDE_CMDSUB;
	}
    }
    rc = regexec(&re_goodchars, words, 0, NULL, 0);
    if (rc != 0) {
	/* Technically ==REG_NOMATCH is WRDE_BADCHAR, and != is
	  some internal error", but again failure to notice the
	  internal error could allow unexpected shell commands
	  (allowing an unexcaped ;), or file clobbering (unescaped
	  >) */
	return WRDE_BADCHAR;
    }

    if (flags & WRDE_APPEND) {
	wordv_i = wordv_l = pwe->we_wordc;
	if (flags & WRDE_DOOFFS) {
	    wordv_l = wordv_i += pwe->we_offs;
	}
    } else {
	if (flags & WRDE_REUSE) {
	    wordfree(pwe);
	}
	pwe->we_wordc = 0;
	pwe->we_wordv = NULL;

	if (flags & WRDE_DOOFFS) {
	    size_t wend = wordv_i + pwe->we_offs;
	    word_alloc(wend, pwe, &wordv_l);
	    if (!pwe->we_wordv) {
		return WRDE_NOSPACE;
	    }
	    bzero(pwe->we_wordv + wordv_i, pwe->we_offs * sizeof(char *));
	    wordv_i = wend;
	} else {
	    pwe->we_offs = 0;
	}
    }

    size_t need = 0;
    while(!cbuf || need > cbuf_l) { 
	if (need > cbuf_l) {
	    cbuf_l = malloc_good_size(need +1);
	}
	cbuf = reallocf(cbuf, cbuf_l);
	if (cbuf == NULL) {
	    wordfree(pwe);
	    return WRDE_NOSPACE;
	}
	cbuf[0] = '\0';
	if (flags & WRDE_UNDEF) {
	    strlcat(cbuf, "set -u; ", cbuf_l);
	}
	/* This kludge is needed because /bin/sh seems to set IFS to the
	  defualt even if you have set it;  We also can't just ignore it
	  because it is hard/unplesent to code around or even a potential
	  security problem because the test suiete explicitly checks
	  to make sure setting IFS "works" */
	if (getenv("IFS")) {
	    setenv("_IFS", getenv("IFS"), 1);
	    strlcat(cbuf, "export IFS=${_IFS}; ", cbuf_l);
	}
	strlcat(cbuf, cmd, cbuf_l);
	need = strlcat(cbuf, words, cbuf_l);
    }

    FILE *out, *err;
    pid_t pid = popen_oe(cbuf, &out, &err);
    if (pid == 0) {
	wordfree(pwe);
	return WRDE_NOSPACE;
    }
    
    char *word = NULL;
    int word_l = 0;
    int word_i = 0;
    int ch;

    while(EOF != (ch = fgetc(out))) {
	if (word_l <= word_i) {
	    word_l = malloc_good_size(word_l * 2 + 1);
	    word = reallocf(word, word_l);
	    if (!word) {
		fclose(err);
		fclose(out);
		wordfree(pwe);
		return WRDE_NOSPACE;
	    }
	}
	word[word_i++] = ch;

	if (ch == '\0') {
	    word_alloc(wordv_i + 1, pwe, &wordv_l);
	    char *tmp = strdup(word);
	    if (pwe->we_wordv == NULL || tmp == NULL) {
		fclose(err);
		fclose(out);
		wordfree(pwe);
		free(word);
		free(tmp);
		int status;
		wait4(pid, &status, 0, NULL);
		return WRDE_NOSPACE;
	    }
	    pwe->we_wordv[wordv_i++] = tmp;
	    pwe->we_wordc++;
	    word_i = 0;
	}
    }

    assert(word_i == 0);
    free(word);

    char err_buf[1024];
    size_t err_sz = fread(err_buf, 1, sizeof(err_buf) -1, err);
    err_buf[(err_sz >= 0) ? err_sz : 0] = '\0';
    if (flags & WRDE_SHOWERR) {
	fputs(err_buf, stderr);
    }

    pid_t got_pid = 0;
    int status;
    do {
	pid = wait4(pid, &status, 0, NULL);
    } while(got_pid == -1 && errno == EINTR);

    fclose(out);
    fclose(err);

    /* the exit status isn't set for some command syntax errors */
    if (regexec(&re_subcmd_syntax_err_kludge, err_buf, 0, NULL, 0) == 0
      || got_pid == -1 || (WIFEXITED(status) && WEXITSTATUS(status))) {
	if (!(flags & (WRDE_APPEND|WRDE_REUSE))) {
	    /* Restore pwe if possiable, can't really do it in the append
	      case, and isn't easy in the reuse case */
	    *pwe = save;
	}
	if (strstr(err_buf, " unbound variable")) {
	    return WRDE_BADVAL;
	}
	return WRDE_SYNTAX;
    }

    if (!word_alloc(wordv_i + 1, pwe, &wordv_l)) {
	return WRDE_NOSPACE;
    }
    pwe->we_wordv[wordv_i] = NULL;

    return 0;
}

void wordfree(wordexp_t *pwe) {
    if (pwe == NULL || pwe->we_wordv == NULL) {
	return;
    }

    int i = 0, e = pwe->we_wordc + pwe->we_offs;
    for(i = pwe->we_offs; i < e; i++) {
	free(pwe->we_wordv[i]);
    }
    free(pwe->we_wordv);
    pwe->we_wordv = NULL;
}
