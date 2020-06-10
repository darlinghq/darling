/*
 * Copyright (c) 1999-2015 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#if !defined(lint) && defined(SCCSIDS)
static	char sccsid[] = "@(#)getnetgrent.c	1.2 90/07/20 4.1NFSSRC; from 1.22 88/02/08 Copyr 1985 Sun Micro";
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <rpcsvc/ypclnt.h>

#define MAXGROUPLEN 1024

/* 
 * access members of a netgroup
 */

static struct grouplist {		/* also used by pwlib */
	char	*gl_machine;
	char	*gl_name;
	char	*gl_domain;
	struct	grouplist *gl_nxt;
} *grouplist;


struct list {			/* list of names to check for loops */
	char *name;
	struct list *nxt;
};

static	void doit();
static	char *fill();
static	char *match();

static	char *domain;

char	*NETGROUP = "netgroup";
/*
 * recursive function to find the members of netgroup "group". "list" is
 * the path followed through the netgroups so far, to check for cycles.
 */
static void
doit(group,list)
	char *group;
	struct list *list;
{
	register char *p, *q;
	register struct list *ls;
	struct list this_group;
	char *val;
	struct grouplist *gpls;
 
	/*
	 * check for non-existing groups
	 */
	if ((val = match(group)) == NULL)
		return;
 
	/*
	 * check for cycles
	 */
	for (ls = list; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, group) == 0) {
			(void) fprintf(stderr,
			    "Cycle detected in /etc/netgroup: %s.\n", group);
			return;
		}
 
	ls = &this_group;
	ls->name = group;
	ls->nxt = list;
	list = ls;
    
	p = val;
	while (p != NULL) {
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == 0 || *p =='#')
			break;
		if (*p == '(') {
			gpls = (struct grouplist *)
			    malloc(sizeof(struct grouplist));
			if (gpls == NULL) return;
			p++;
			if (!(p = fill(p,&gpls->gl_machine,',')))
				goto syntax_error;
			if (!(p = fill(p,&gpls->gl_name,',')))
				goto syntax_error;
			if (!(p = fill(p,&gpls->gl_domain,')')))
				goto syntax_error;
			gpls->gl_nxt = grouplist;
			grouplist = gpls;
		} else {
			q = strpbrk(p, " \t\n#");
			if (q == NULL || *q == '#')
				break;
			*q = 0;
			doit(p,list);
			*q = ' ';
		}
		p = strpbrk(p, " \t");
	}
	return;
 
syntax_error:
	free(gpls);
	(void) fprintf(stderr,"syntax error in /etc/netgroup\n");
	(void) fprintf(stderr,"--- %s\n",val);
	return;
}

/*
 * Fill a buffer "target" selectively from buffer "start".
 * "termchar" terminates the information in start, and preceding
 * or trailing white space is ignored. The location just after the
 * terminating character is returned.  
 */
static char *
fill(start,target,termchar)
	char *start, **target, termchar;
{
	register char *p, *q; 
	char *r;
	unsigned size;
 
	for (p = start; *p == ' ' || *p == '\t'; p++)
		;
	r = index(p, termchar);
	if (r == NULL)
		return (NULL);
	if (p == r)
		*target = NULL;	
	else {
		for (q = r-1; *q == ' ' || *q == '\t'; q--)
			;
		size = q - p + 1;
		*target = malloc(size+1);
		if (*target == NULL) return NULL;
		(void) strncpy(*target,p,(int) size);
		(*target)[size] = 0;
	}
	return (r+1);
}

static char *
match(group)
	char *group;
{
	char *val;
	int vallen;

	if (domain == NULL)
		(void) yp_get_default_domain(&domain );
	if (yp_match(domain, NETGROUP, group, strlen(group), &val, &vallen))
		return (NULL);
	return (val);
}
