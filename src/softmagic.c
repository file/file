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

mcheck(s, m)
char	*s;
struct magic *m;
{
	register char reln = m->reln;
	register union VALUETYPE *p = (union VALUETYPE *)(s+m->offset);
	register long l = m->value.l;

	if (debug) {
		(void) printf("mcheck: %10.10s ", s);
		mdump(m);
	}
	switch (m->type) {
	case BYTE:
		if (reln == '>')
			return p->b > l;
		if (reln == '&')
			return l & p->b;
		if (reln == '=')
			return l == p->b;
		warning("mcheck: can't happen: invalid BYTE reln %d", reln);
		return 0;
	case SHORT:
		if (reln == '=')
			return l == p->h;
		if (reln == '>')
			return p->h > l;
		if (reln == '&')
			return l & p->h;
		warning("mcheck: can't happen: invalid SHORT reln %d", reln);
		return 0;
	case LONG:
		if (reln == '=')
			return l == p->l;
		if (reln == '>')
			return p->l > l;
		if (reln == '&')
			return l & p->l;
		warning("mcheck: can't happen: invalid LONG reln %d", reln);
		return 0;
	case STRING:
		if (reln == '=')
			return strncmp(m->value.s, p->s,
				strlen(m->value.s)) == 0;
		if (reln == '>')
			if (strcmp(m->value.s, "0") == 0) /* special case! */
				return *s > '\0';
			else
				return strncmp(m->value.s, p->s,
					strlen(m->value.s)) > 0;
		warning("mcheck: can't happen: invalid STRING reln %c(0%o)",
			reln, reln);
		return 0;
	default:
		warning("invalid type %d in mcheck()", m->type);
		return 0;
	}
}


