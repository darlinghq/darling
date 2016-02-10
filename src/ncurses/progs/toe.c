/****************************************************************************
 * Copyright (c) 1998-2007,2008 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	toe.c --- table of entries report generator
 */

#include <progs.priv.h>

#include <sys/stat.h>

#if USE_HASHED_DB
#include <hashed_db.h>
#endif

MODULE_ID("$Id: toe.c,v 1.51 2008/08/16 21:53:25 tom Exp $")

#define isDotname(name) (!strcmp(name, ".") || !strcmp(name, ".."))

const char *_nc_progname;

#if NO_LEAKS
#undef ExitProgram
static void ExitProgram(int code) GCC_NORETURN;
static void
ExitProgram(int code)
{
    _nc_free_entries(_nc_head);
    _nc_free_tic(code);
}
#endif

#if USE_HASHED_DB
static bool
make_db_name(char *dst, const char *src, unsigned limit)
{
    static const char suffix[] = DBM_SUFFIX;

    bool result = FALSE;
    unsigned lens = sizeof(suffix) - 1;
    unsigned size = strlen(src);
    unsigned need = lens + size;

    if (need <= limit) {
	if (size >= lens
	    && !strcmp(src + size - lens, suffix))
	    (void) strcpy(dst, src);
	else
	    (void) sprintf(dst, "%s%s", src, suffix);
	result = TRUE;
    }
    return result;
}
#endif

static bool
is_database(const char *path)
{
    bool result = FALSE;
#if USE_DATABASE
    if (_nc_is_dir_path(path) && access(path, R_OK | X_OK) == 0) {
	result = TRUE;
    }
#endif
#if USE_TERMCAP
    if (_nc_is_file_path(path) && access(path, R_OK) == 0) {
	result = TRUE;
    }
#endif
#if USE_HASHED_DB
    if (!result) {
	char filename[PATH_MAX];
	if (_nc_is_file_path(path) && access(path, R_OK) == 0) {
	    result = TRUE;
	} else if (make_db_name(filename, path, sizeof(filename))) {
	    if (_nc_is_file_path(filename) && access(filename, R_OK) == 0) {
		result = TRUE;
	    }
	}
    }
#endif
    return result;
}

static void
deschook(const char *cn, TERMTYPE *tp)
/* display a description for the type */
{
    const char *desc;

    if ((desc = strrchr(tp->term_names, '|')) == 0 || *++desc == '\0')
	desc = "(No description)";

    (void) printf("%-10s\t%s\n", cn, desc);
}

#if USE_TERMCAP
static void
show_termcap(char *buffer,
	     void (*hook) (const char *, TERMTYPE *tp))
{
    TERMTYPE data;
    char *next = strchr(buffer, ':');
    char *last;
    char *list = buffer;

    if (next)
	*next = '\0';

    last = strrchr(buffer, '|');
    if (last)
	++last;

    data.term_names = strdup(buffer);
    while ((next = strtok(list, "|")) != 0) {
	if (next != last)
	    hook(next, &data);
	list = 0;
    }
    free(data.term_names);
}
#endif

static int
typelist(int eargc, char *eargv[],
	 bool verbosity,
	 void (*hook) (const char *, TERMTYPE *tp))
/* apply a function to each entry in given terminfo directories */
{
    int i;

    for (i = 0; i < eargc; i++) {
#if USE_DATABASE
	if (_nc_is_dir_path(eargv[i])) {
	    char *cwd_buf = 0;
	    DIR *termdir;
	    DIRENT *subdir;

	    if ((termdir = opendir(eargv[i])) == 0) {
		(void) fflush(stdout);
		(void) fprintf(stderr,
			       "%s: can't open terminfo directory %s\n",
			       _nc_progname, eargv[i]);
		return (EXIT_FAILURE);
	    } else if (verbosity)
		(void) printf("#\n#%s:\n#\n", eargv[i]);

	    while ((subdir = readdir(termdir)) != 0) {
		size_t len = NAMLEN(subdir);
		size_t cwd_len = len + strlen(eargv[i]) + 3;
		char name_1[PATH_MAX];
		DIR *entrydir;
		DIRENT *entry;

		cwd_buf = typeRealloc(char, cwd_len, cwd_buf);
		if (cwd_buf == 0) {
		    perror("realloc cwd_buf");
		    continue;
		}

		strncpy(name_1, subdir->d_name, len)[len] = '\0';
		if (isDotname(name_1))
		    continue;

		(void) sprintf(cwd_buf, "%s/%.*s/", eargv[i], (int) len, name_1);
		if (chdir(cwd_buf) != 0)
		    continue;

		entrydir = opendir(".");
		if (entrydir == 0) {
		    perror(cwd_buf);
		    continue;
		}
		while ((entry = readdir(entrydir)) != 0) {
		    char name_2[PATH_MAX];
		    TERMTYPE lterm;
		    char *cn;
		    int status;

		    len = NAMLEN(entry);
		    strncpy(name_2, entry->d_name, len)[len] = '\0';
		    if (isDotname(name_2) || !_nc_is_file_path(name_2))
			continue;

		    status = _nc_read_file_entry(name_2, &lterm);
		    if (status <= 0) {
			(void) fflush(stdout);
			(void) fprintf(stderr,
				       "%s: couldn't open terminfo file %s.\n",
				       _nc_progname, name_2);
			return (EXIT_FAILURE);
		    }

		    /* only visit things once, by primary name */
		    cn = _nc_first_name(lterm.term_names);
		    if (!strcmp(cn, name_2)) {
			/* apply the selected hook function */
			(*hook) (cn, &lterm);
		    }
		    _nc_free_termtype(&lterm);
		}
		closedir(entrydir);
	    }
	    closedir(termdir);
	    if (cwd_buf != 0)
		free(cwd_buf);
	}
#if USE_HASHED_DB
	else {
	    DB *capdbp;
	    char filename[PATH_MAX];

	    if (make_db_name(filename, eargv[i], sizeof(filename))) {
		if ((capdbp = _nc_db_open(filename, FALSE)) != 0) {
		    DBT key, data;
		    int code;

		    code = _nc_db_first(capdbp, &key, &data);
		    while (code == 0) {
			TERMTYPE lterm;
			int used;
			char *have;
			char *cn;

			if (_nc_db_have_data(&key, &data, &have, &used)) {
			    if (_nc_read_termtype(&lterm, have, used) > 0) {
				/* only visit things once, by primary name */
				cn = _nc_first_name(lterm.term_names);
				/* apply the selected hook function */
				(*hook) (cn, &lterm);
				_nc_free_termtype(&lterm);
			    }
			}
			code = _nc_db_next(capdbp, &key, &data);
		    }

		    _nc_db_close(capdbp);
		}
	    }
	}
#endif
#endif
#if USE_TERMCAP
#if HAVE_BSD_CGETENT
	char *db_array[2];
	char *buffer = 0;

	if (verbosity)
	    (void) printf("#\n#%s:\n#\n", eargv[i]);

	db_array[0] = eargv[i];
	db_array[1] = 0;

	if (cgetfirst(&buffer, db_array)) {
	    show_termcap(buffer, hook);
	    free(buffer);
	    while (cgetnext(&buffer, db_array)) {
		show_termcap(buffer, hook);
		free(buffer);
	    }
	}
	cgetclose();
#else
	/* scan termcap text-file only */
	if (_nc_is_file_path(eargv[i])) {
	    char buffer[2048];
	    FILE *fp;

	    if ((fp = fopen(eargv[i], "r")) != 0) {
		while (fgets(buffer, sizeof(buffer), fp) != 0) {
		    if (*buffer == '#')
			continue;
		    if (isspace(*buffer))
			continue;
		    show_termcap(buffer, hook);
		}
		fclose(fp);
	    }
	}
#endif
#endif
    }

    return (EXIT_SUCCESS);
}

static void
usage(void)
{
    (void) fprintf(stderr, "usage: %s [-ahuUV] [-v n] [file...]\n", _nc_progname);
    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    bool all_dirs = FALSE;
    bool direct_dependencies = FALSE;
    bool invert_dependencies = FALSE;
    bool header = FALSE;
    char *report_file = 0;
    unsigned i;
    int code;
    int this_opt, last_opt = '?';
    int v_opt = 0;

    _nc_progname = _nc_rootname(argv[0]);

    while ((this_opt = getopt(argc, argv, "0123456789ahu:vU:V")) != -1) {
	/* handle optional parameter */
	if (isdigit(this_opt)) {
	    switch (last_opt) {
	    case 'v':
		v_opt = (this_opt - '0');
		break;
	    default:
		if (isdigit(last_opt))
		    v_opt *= 10;
		else
		    v_opt = 0;
		v_opt += (this_opt - '0');
		last_opt = this_opt;
	    }
	    continue;
	}
	switch (this_opt) {
	case 'a':
	    all_dirs = TRUE;
	    break;
	case 'h':
	    header = TRUE;
	    break;
	case 'u':
	    direct_dependencies = TRUE;
	    report_file = optarg;
	    break;
	case 'v':
	    v_opt = 1;
	    break;
	case 'U':
	    invert_dependencies = TRUE;
	    report_file = optarg;
	    break;
	case 'V':
	    puts(curses_version());
	    ExitProgram(EXIT_SUCCESS);
	default:
	    usage();
	}
    }
    set_trace_level(v_opt);

    if (report_file != 0) {
	if (freopen(report_file, "r", stdin) == 0) {
	    (void) fflush(stdout);
	    fprintf(stderr, "%s: can't open %s\n", _nc_progname, report_file);
	    ExitProgram(EXIT_FAILURE);
	}

	/* parse entries out of the source file */
	_nc_set_source(report_file);
	_nc_read_entry_source(stdin, 0, FALSE, FALSE, NULLHOOK);
    }

    /* maybe we want a direct-dependency listing? */
    if (direct_dependencies) {
	ENTRY *qp;

	for_entry_list(qp) {
	    if (qp->nuses) {
		unsigned j;

		(void) printf("%s:", _nc_first_name(qp->tterm.term_names));
		for (j = 0; j < qp->nuses; j++)
		    (void) printf(" %s", qp->uses[j].name);
		putchar('\n');
	    }
	}

	ExitProgram(EXIT_SUCCESS);
    }

    /* maybe we want a reverse-dependency listing? */
    if (invert_dependencies) {
	ENTRY *qp, *rp;
	int matchcount;

	for_entry_list(qp) {
	    matchcount = 0;
	    for_entry_list(rp) {
		if (rp->nuses == 0)
		    continue;

		for (i = 0; i < rp->nuses; i++)
		    if (_nc_name_match(qp->tterm.term_names,
				       rp->uses[i].name, "|")) {
			if (matchcount++ == 0)
			    (void) printf("%s:",
					  _nc_first_name(qp->tterm.term_names));
			(void) printf(" %s",
				      _nc_first_name(rp->tterm.term_names));
		    }
	    }
	    if (matchcount)
		putchar('\n');
	}

	ExitProgram(EXIT_SUCCESS);
    }

    /*
     * If we get this far, user wants a simple terminal type listing.
     */
    if (optind < argc) {
	code = typelist(argc - optind, argv + optind, header, deschook);
    } else if (all_dirs) {
	DBDIRS state;
	int offset;
	int pass;
	const char *path;
	char **eargv = 0;

	code = EXIT_FAILURE;
	for (pass = 0; pass < 2; ++pass) {
	    unsigned count = 0;

	    _nc_first_db(&state, &offset);
	    while ((path = _nc_next_db(&state, &offset)) != 0) {
		if (!is_database(path)) {
		    ;
		} else if (eargv != 0) {
		    unsigned n;
		    int found = FALSE;

		    /* eliminate duplicates */
		    for (n = 0; n < count; ++n) {
			if (!strcmp(path, eargv[n])) {
			    found = TRUE;
			    break;
			}
		    }
		    if (!found) {
			eargv[count] = strdup(path);
			++count;
		    }
		} else {
		    ++count;
		}
	    }
	    if (!pass) {
		eargv = typeCalloc(char *, count + 1);
	    } else {
		code = typelist((int) count, eargv, header, deschook);
		while (count-- > 0)
		    free(eargv[count]);
		free(eargv);
	    }
	}
    } else {
	DBDIRS state;
	int offset;
	const char *path;
	char *eargv[3];
	int count = 0;

	_nc_first_db(&state, &offset);
	while ((path = _nc_next_db(&state, &offset)) != 0) {
	    if (is_database(path)) {
		eargv[count++] = strdup(path);
		break;
	    }
	}
	eargv[count] = 0;

	code = typelist(count, eargv, header, deschook);

	while (count-- > 0)
	    free(eargv[count]);
    }
    _nc_last_db();

    ExitProgram(code);
}
