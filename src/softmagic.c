/*
 * softmagic - interpret variable magic from /etc/magic
 *
 * Copyright (c) Ian F. Darwin, 1987.
 * Written by Ian F. Darwin.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or of the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. The author is not responsible for the consequences of use of this
 *    software, no matter how awful, even if they arise from flaws in it.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Since few users ever read sources,
 *    credits must appear in the documentation.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.  Since few users
 *    ever read sources, credits must appear in the documentation.
 *
 * 4. This notice may not be removed or altered.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "file.h"

#ifndef	lint
static char *moduleid = 
	"@(#)$Id: softmagic.c,v 1.14 1992/09/09 15:04:16 ian Exp $";
#endif	/* lint */

static int match	__P((unsigned char *));
static int mcheck	__P((unsigned char	*, struct magic *));
static void mprint	__P((struct magic *, unsigned char *));

/*
 * softmagic - lookup one file in database 
 * (already read from /etc/magic by apprentice.c).
 * Passed the name and FILE * of one file to be typed.
 */
/*ARGSUSED1*/		/* nbytes passed for regularity, maybe need later */
int
softmagic(buf, nbytes)
unsigned char *buf;
int nbytes;
{
	if (match(buf))
		return 1;

	return 0;
}

/*
 * go through the whole list, stopping if you find a match.
 * Be sure to process every continuation of this match.
 */
static int
match(s)
unsigned char	*s;
{
	int magindex = 0;
	while (magindex < nmagic) {
		/* if main entry matches, print it... */
		if (mcheck(s, &magic[magindex])) {
			mprint(&magic[magindex],s);
			/* and any continuations that match */
			while (magic[magindex+1].flag &&
				magindex < nmagic) {
				++magindex;
				if (mcheck(s, &magic[magindex])) {
					/* space if previous printed */
					if (magic[magindex-1].desc[0]
					   && (magic[magindex].nospflag == 0)
					   )
						(void) putchar(' ');
					mprint(&magic[magindex],s);
				}
			}
			return 1;		/* all through */
		} else {
			/* main entry didn't match, flush its continuation */
			while (magic[magindex+1].flag &&
				magindex < nmagic) {
				++magindex;
			}
		}
		++magindex;			/* on to the next */
	}
	return 0;				/* no match at all */
}

static void
mprint(m, s)
struct magic *m;
unsigned char *s;
{
	register union VALUETYPE *p = (union VALUETYPE *)(s+m->offset);
	char *pp, *rt;

  	switch (m->type) {
  	case BYTE:
 		(void) printf(m->desc,
 			      (m->reln & MASK) ? p->b & m->mask : p->b);
  		break;
  	case SHORT:
 		(void) printf(m->desc,
 			      (m->reln & MASK) ? p->h & m->mask : p->h);
  		break;
  	case LONG:
 		(void) printf(m->desc,
 			      (m->reln & MASK) ? p->l & m->mask : p->l);
  		break;
  	case STRING:
		if ((pp=strchr(p->s, '\n')) != NULL)
			*pp = '\0';
		(void) printf(m->desc, p->s);
		break;
	case DATE:
		pp = ctime((time_t*) &p->l);
		if ((rt = strchr(pp, '\n')) != NULL)
			*rt = '\0';
		(void) printf(m->desc, pp);
		break;
	default:
		error("invalid m->type (%d) in mprint().\n", m->type);
		/*NOTREACHED*/
	}
}

static int
mcheck(s, m)
unsigned char	*s;
struct magic *m;
{
	register union VALUETYPE *p = (union VALUETYPE *)(s+m->offset);
	register long l = m->value.l;
	register long mask = m->mask;
	register long v;

	if (debug) {
		(void) printf("mcheck: %10.10s ", s);
		mdump(m);
	}

	if ( (m->value.s[0] == 'x') && (m->value.s[1] == '\0') ) {
		printf("BOINK");
		return 1;
	}

	switch (m->type) {
	case BYTE:
		v = p->b; break;
	case SHORT:
		v = p->h; break;
	case LONG:
	case DATE:
		v = p->l; break;
	case STRING:
		l = 0;
		/* What we want here is:
		 * v = strncmp(m->value.s, p->s, m->vallen);
		 * but ignoring any nulls.  bcmp doesn't give -/+/0
		 * and isn't universally available anyway.
		 */
		v = 0;
		{
			register unsigned char *a = (unsigned char*)m->value.s;
			register unsigned char *b = (unsigned char*)p->s;
			register int len = m->vallen;

			while (--len >= 0)
				if ((v = *b++ - *a++) != 0)
					break;
		}
		break;
	default:
		error("invalid type %d in mcheck().\n", m->type);
		return -1;/*NOTREACHED*/
	}

	if (m->mask != 0L)
		v &= m->mask;

	switch (m->reln) {
	case 'x':
		return 1;
	case '!':
	case '^':
		return v != l;
	case '=':
		return v == l;
	case '>':
		return v > l;
	case '<':
		return v < l;
	case '&':
		return (v & l) == l;
	case MASK | '=':
		return (v & mask) == l;
	case MASK | '>':
		return (v & mask) > l;
	case MASK | '<':
		return (v & mask) < l;
	default:
		error("mcheck: can't happen: invalid relation %d.\n", m->reln);
		return -1;/*NOTREACHED*/
	}
}
