/*
 * variable magic from /etc/magic
 */

#include <stdio.h>
#include "file.h"

#define USAGE		"usage: %s [-f file] [-m magicfile] file...\n"

extern char *progname;
extern char *magicfile;	/* name of current /etc/magic or clone */
extern int debug, nmagic;
extern FILE *efopen();
extern struct magic magic[];
static int magindex;

/*
 * softmagic - lookup one file in /etc/magic database.
 * Passed the name and FILE * of one file to be typed.
 */
softmagic(buf)
char *buf;
{
	magindex = 0;
	if (match(buf))
		return 1;

	return 0;
}

/*
 * go through the whole list, stopping if you find a match.
 * Be sure to process every continuation of this match.
 */
match(s)
char	*s;
{
	while (magindex < nmagic) {
		/* if main entry matches, print it... */
		if (mcheck(s, &magic[magindex])) {
			mprint(&magic[magindex],s);
			/* and any continuations that match */
			while (magic[magindex+1].contflag &&
				magindex < nmagic) {
				++magindex;
				if (mcheck(s, &magic[magindex])){
					(void) putchar(' ');
					mprint(&magic[magindex],s);
				}
			}
			return 1;		/* all through */
		} else {
			/* main entry didn't match, flush its continuations */
			while (magic[magindex+1].contflag &&
				magindex < nmagic) {
				++magindex;
			}
		}
		++magindex;			/* on to the next */
	}
	return 0;				/* no match at all */
}


mprint(m,s)
struct magic *m;
char *s;
{
	register union VALUETYPE *p = (union VALUETYPE *)(s+m->offset);

	switch (m->type) {
	case BYTE:
		(void) printf(m->desc, p->b);
		break;
	case SHORT:
		(void) printf(m->desc, p->h);
		break;
	case LONG:
		(void) printf(m->desc, p->l);
		break;
	case STRING:
		(void) printf(m->desc, p->s);
		break;
	default:
		warning("invalid m->type (%d) in mprint()", m->type);
	}
}

int
mcheck(s, m)
char	*s;
struct magic *m;
{
	register union VALUETYPE *p = (union VALUETYPE *)(s+m->offset);
	register long l = m->value.l;
	register long v;

	if (debug) {
		(void) printf("mcheck: %10.10s ", s);
		mdump(m);
	}
	switch (m->type) {
	case BYTE:
		v = p->b; break;
	case SHORT:
		v = p->h; break;
	case LONG:
		v = p->l; break;
	case STRING:
		l = 0;
		/* What we want here is:
		 * v = strncmp(m->value.s, p->s, m->vallen);
		 * but ignoring any nulls.  bcmp doesn't give -/+/0
		 * and isn't universally available anyway.
		 */
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
		warning("invalid type %d in mcheck()", m->type);
		return 0;
	}

	switch (m->reln) {
	case '=':
		return v == l;
	case '>':
		return v > l;
	case '<':
		return v < l;
	case '&':
		return v & l;
	default:
		warning("mcheck: can't happen: invalid relation %d", m->reln);
		return 0;
	}
}
