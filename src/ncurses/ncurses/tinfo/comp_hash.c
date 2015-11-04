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
 *	comp_hash.c --- Routines to deal with the hashtable of capability
 *			names.
 *
 */

#define USE_TERMLIB 1
#include <curses.priv.h>

#include <tic.h>
#include <hashsize.h>

#ifdef MAIN_PROGRAM
#include <ctype.h>
#undef  DEBUG
#define DEBUG(level, params)	/*nothing */
#endif

MODULE_ID("$Id: comp_hash.c,v 1.36 2008/08/16 17:06:53 tom Exp $")

static int hash_function(const char *);

/*
 *	_nc_make_hash_table()
 *
 *	Takes the entries in table[] and hashes them into hash_table[]
 *	by name.  There are CAPTABSIZE entries in table[] and HASHTABSIZE
 *	slots in hash_table[].
 *
 */

#ifdef MAIN_PROGRAM

#undef MODULE_ID
#define MODULE_ID(id)		/*nothing */
#include <tinfo/doalloc.c>

static void
_nc_make_hash_table(struct name_table_entry *table,
		    short *hash_table)
{
    short i;
    int hashvalue;
    int collisions = 0;

    for (i = 0; i < HASHTABSIZE; i++) {
	hash_table[i] = -1;
    }
    for (i = 0; i < CAPTABSIZE; i++) {
	hashvalue = hash_function(table[i].nte_name);

	if (hash_table[hashvalue] >= 0)
	    collisions++;

	if (hash_table[hashvalue] != 0)
	    table[i].nte_link = hash_table[hashvalue];
	hash_table[hashvalue] = i;
    }

    DEBUG(4, ("Hash table complete: %d collisions out of %d entries",
	      collisions, CAPTABSIZE));
}
#endif

/*
 *	int hash_function(string)
 *
 *	Computes the hashing function on the given string.
 *
 *	The current hash function is the sum of each consectutive pair
 *	of characters, taken as two-byte integers, mod HASHTABSIZE.
 *
 */

static int
hash_function(const char *string)
{
    long sum = 0;

    DEBUG(9, ("hashing %s", string));
    while (*string) {
	sum += (long) (*string + (*(string + 1) << 8));
	string++;
    }

    DEBUG(9, ("sum is %ld", sum));
    return (int) (sum % HASHTABSIZE);
}

/*
 *	struct name_table_entry *
 *	find_entry(string)
 *
 *	Finds the entry for the given string in the hash table if present.
 *	Returns a pointer to the entry in the table or 0 if not found.
 *
 */

#ifndef MAIN_PROGRAM
NCURSES_EXPORT(struct name_table_entry const *)
_nc_find_entry(const char *string,
	       const short *hash_table)
{
    int hashvalue;
    struct name_table_entry const *ptr = 0;
    struct name_table_entry const *real_table;

    hashvalue = hash_function(string);

    if (hash_table[hashvalue] >= 0) {
	real_table = _nc_get_table(hash_table != _nc_get_hash_table(FALSE));
	ptr = real_table + hash_table[hashvalue];
	while (strcmp(ptr->nte_name, string) != 0) {
	    if (ptr->nte_link < 0)
		return 0;
	    ptr = real_table + (ptr->nte_link + hash_table[HASHTABSIZE]);
	}
    }

    return (ptr);
}

/*
 *	struct name_table_entry *
 *	find_type_entry(string, type, table)
 *
 *	Finds the first entry for the given name with the given type in the
 *	given table if present (as distinct from find_entry, which finds the
 *	the last entry regardless of type).  You can use this if you detect
 *	a name clash.  It's slower, though.  Returns a pointer to the entry
 *	in the table or 0 if not found.
 */

NCURSES_EXPORT(struct name_table_entry const *)
_nc_find_type_entry(const char *string,
		    int type,
		    const struct name_table_entry *table)
{
    struct name_table_entry const *ptr;

    for (ptr = table; ptr < table + CAPTABSIZE; ptr++) {
	if (ptr->nte_type == type && strcmp(string, ptr->nte_name) == 0)
	    return (ptr);
    }

    return ((struct name_table_entry *) NULL);
}
#endif

#ifdef MAIN_PROGRAM
/*
 * This filter reads from standard input a list of tab-delimited columns,
 * (e.g., from Caps.filtered) computes the hash-value of a specified column and
 * writes the hashed tables to standard output.
 *
 * By compiling the hash table at build time, we're able to make the entire
 * set of terminfo and termcap tables readonly (and also provide some runtime
 * performance enhancement).
 */

#define MAX_COLUMNS BUFSIZ	/* this _has_ to be worst-case */

static char **
parse_columns(char *buffer)
{
    static char **list;

    int col = 0;

    if (list == 0 && (list = typeCalloc(char *, MAX_COLUMNS)) == 0)
	  return (0);

    if (*buffer != '#') {
	while (*buffer != '\0') {
	    char *s;
	    for (s = buffer; (*s != '\0') && !isspace(UChar(*s)); s++)
		/*EMPTY */ ;
	    if (s != buffer) {
		char mark = *s;
		*s = '\0';
		if ((s - buffer) > 1
		    && (*buffer == '"')
		    && (s[-1] == '"')) {	/* strip the quotes */
		    assert(s > buffer + 1);
		    s[-1] = '\0';
		    buffer++;
		}
		list[col] = buffer;
		col++;
		if (mark == '\0')
		    break;
		while (*++s && isspace(UChar(*s)))
		    /*EMPTY */ ;
		buffer = s;
	    } else
		break;
	}
    }
    return col ? list : 0;
}

int
main(int argc, char **argv)
{
    struct name_table_entry *name_table = typeCalloc(struct
						     name_table_entry, CAPTABSIZE);
    short *hash_table = typeCalloc(short, HASHTABSIZE);
    const char *root_name = "";
    int column = 0;
    int bigstring = 0;
    int n;
    char buffer[BUFSIZ];

    static const char *typenames[] =
    {"BOOLEAN", "NUMBER", "STRING"};

    short BoolCount = 0;
    short NumCount = 0;
    short StrCount = 0;

    /* The first argument is the column-number (starting with 0).
     * The second is the root name of the tables to generate.
     */
    if (argc <= 3
	|| (column = atoi(argv[1])) <= 0
	|| (column >= MAX_COLUMNS)
	|| *(root_name = argv[2]) == 0
	|| (bigstring = atoi(argv[3])) < 0
	|| name_table == 0
	|| hash_table == 0) {
	fprintf(stderr, "usage: make_hash column root_name bigstring\n");
	exit(EXIT_FAILURE);
    }

    /*
     * Read the table into our arrays.
     */
    for (n = 0; (n < CAPTABSIZE) && fgets(buffer, BUFSIZ, stdin);) {
	char **list, *nlp = strchr(buffer, '\n');
	if (nlp)
	    *nlp = '\0';
	list = parse_columns(buffer);
	if (list == 0)		/* blank or comment */
	    continue;
	name_table[n].nte_link = -1;	/* end-of-hash */
	name_table[n].nte_name = strdup(list[column]);
	if (!strcmp(list[2], "bool")) {
	    name_table[n].nte_type = BOOLEAN;
	    name_table[n].nte_index = BoolCount++;
	} else if (!strcmp(list[2], "num")) {
	    name_table[n].nte_type = NUMBER;
	    name_table[n].nte_index = NumCount++;
	} else if (!strcmp(list[2], "str")) {
	    name_table[n].nte_type = STRING;
	    name_table[n].nte_index = StrCount++;
	} else {
	    fprintf(stderr, "Unknown type: %s\n", list[2]);
	    exit(EXIT_FAILURE);
	}
	n++;
    }
    _nc_make_hash_table(name_table, hash_table);

    /*
     * Write the compiled tables to standard output
     */
    if (bigstring) {
	int len = 0;
	int nxt;

	printf("static const char %s_names_text[] = \\\n", root_name);
	for (n = 0; n < CAPTABSIZE; n++) {
	    nxt = (int) strlen(name_table[n].nte_name) + 5;
	    if (nxt + len > 72) {
		printf("\\\n");
		len = 0;
	    }
	    printf("\"%s\\0\" ", name_table[n].nte_name);
	    len += nxt;
	}
	printf(";\n\n");

	len = 0;
	printf("static name_table_data const %s_names_data[] =\n",
	       root_name);
	printf("{\n");
	for (n = 0; n < CAPTABSIZE; n++) {
	    printf("\t{ %15d,\t%10s,\t%3d, %3d }%c\n",
		   len,
		   typenames[name_table[n].nte_type],
		   name_table[n].nte_index,
		   name_table[n].nte_link,
		   n < CAPTABSIZE - 1 ? ',' : ' ');
	    len += (int) strlen(name_table[n].nte_name) + 1;
	}
	printf("};\n\n");
	printf("static struct name_table_entry *_nc_%s_table = 0;\n\n", root_name);
    } else {

	printf("static struct name_table_entry %s _nc_%s_table[] =\n",
	       bigstring ? "" : "const",
	       root_name);
	printf("{\n");
	for (n = 0; n < CAPTABSIZE; n++) {
	    sprintf(buffer, "\"%s\"",
		    name_table[n].nte_name);
	    printf("\t{ %15s,\t%10s,\t%3d, %3d }%c\n",
		   buffer,
		   typenames[name_table[n].nte_type],
		   name_table[n].nte_index,
		   name_table[n].nte_link,
		   n < CAPTABSIZE - 1 ? ',' : ' ');
	}
	printf("};\n\n");
    }

    printf("static const short _nc_%s_hash_table[%d] =\n",
	   root_name,
	   HASHTABSIZE + 1);
    printf("{\n");
    for (n = 0; n < HASHTABSIZE; n++) {
	printf("\t%3d,\n", hash_table[n]);
    }
    printf("\t0\t/* base-of-table */\n");
    printf("};\n\n");

    printf("#if (BOOLCOUNT!=%d)||(NUMCOUNT!=%d)||(STRCOUNT!=%d)\n",
	   BoolCount, NumCount, StrCount);
    printf("#error\t--> term.h and comp_captab.c disagree about the <--\n");
    printf("#error\t--> numbers of booleans, numbers and/or strings <--\n");
    printf("#endif\n\n");

    free(hash_table);
    return EXIT_SUCCESS;
}
#endif
