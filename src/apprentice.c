/*
 * apprentice - make one pass through /etc/magic, learning its secrets.
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file.h"

#ifndef	lint
static char *moduleid = 
	"@(#)$Header: /p/file/cvsroot/file/src/apprentice.c,v 1.11 1992/05/22 17:49:23 ian Exp $";
#endif	/* lint */

#define MAXSTR		500
#define	EATAB {while (isascii(*l) && isspace(*l))  ++l;}

extern char *progname;
extern char *magicfile;
extern int debug;		/* option */
extern int nmagic;		/* number of valid magic[]s */
extern long strtol();

struct magic magic[MAXMAGIS];

#if	defined(__STDC__) || defined(__cplusplus)
static int getvalue(struct magic *m, char **p);
static int hextoint(int c);
static char *getstr(char *s, char *p, int plen, int *slen);
#else
static int getvalue();
static int hextoint();
static char *getstr();
#endif

int
apprentice(fn, check)
char *fn;			/* name of magic file */
int check;			/* non-zero? checking-only run. */
{
	FILE *f;
	char line[MAXSTR+1];
	int errs = 0;

	f = fopen(fn, "r");
	if (f==NULL) {
		(void) fprintf(stderr, "%s: can't read magic file %s\n",
		progname, fn);
		if (check)
			return -1;
		else
			exit(1);
	}

	/* parse it */
	if (check)	/* print silly verbose header for USG compat. */
		(void) printf("cont\toffset\ttype\topcode\tmask\tvalue\tdesc\n");

	while (fgets(line, MAXSTR, f) != NULL) {
		if (line[0]=='#')	/* comment, do not parse */
			continue;
		if (strlen(line) <= 1)	/* null line, garbage, etc */
			continue;
		line[strlen(line)-1] = '\0'; /* delete newline */
		if (parse(line, &nmagic, check) != 0)
			++errs;
	}

	(void) fclose(f);
	return errs ? -1 : 0;
}

/*
 * parse one line from magic file, put into magic[index++] if valid
 */
int
parse(l, ndx, check)
char *l;
int *ndx, check;
{
	int i = 0, nd = *ndx;
	static int warned = 0;
	struct magic *m;
	extern int errno;

	/*
	 * TODO malloc the magic structures (linked list?) so this can't happen
	 */
	if (nd+1 >= MAXMAGIS){
		if (warned++ == 0)
			warning(
"magic table overflow - increase MAXMAGIS beyond %d in file/file.h\n",
			MAXMAGIS);
		return -1;
	}
	m = &magic[*ndx];

	if (*l == '>') {
		++l;		/* step over */
		m->contflag = 1;
	} else
		m->contflag = 0;

	/* get offset, then skip over it */
#ifdef OldWay
	m->offset = atoi(l);
	while (isascii(*l) && isdigit(*l))
		++l;
#else
	m->offset = (int) strtol(l,&l,0);
#endif
	EATAB;

#define NBYTE 4
#define NSHORT 5
#define NLONG 4
#define NSTRING 6
	/* get type, skip it */
	if (strncmp(l, "byte", NBYTE)==0) {
		m->type = BYTE;
		l += NBYTE;
	} else if (strncmp(l, "short", NSHORT)==0) {
		m->type = SHORT;
		l += NSHORT;
	} else if (strncmp(l, "long", NLONG)==0) {
		m->type = LONG;
		l += NLONG;
	} else if (strncmp(l, "string", NSTRING)==0) {
		m->type = STRING;
		l += NSTRING;
	} else {
		errno = 0;
		warning("type %s invalid", l);
		return -1;
	}
	if (*l == '&') {
		++l;
		m->mask = strtol(l, &l, 0);
	} else
		m->mask = 0L;
	EATAB;
  
	switch (*l) {
	case '>':
	case '<':
	case '&':
	case '=':
  		m->reln = *l;
  		++l;
		break;
	case '!':
	case '^':
		if (m->type != STRING) {
			m->reln = *l;
			++l;
			break;
		}
		/* FALL THROUGH */
	default:
		if (*l == 'x' && isascii(l[1]) && isspace(l[1])) {
			m->reln = *l;
			++l;
			goto GetDesc;	/* Bill The Cat */
		}
  		m->reln = '=';
		break;
	}
  	EATAB;
  
	if (getvalue(m, &l))
		return -1;
	/*
	 * TODO finish this macro and start using it!
	 * #define offsetcheck {if (offset > HOWMANY-1) 
	 *	warning("offset too big"); }
	 */

	/* 
	 * if the relation was ``&'',
	 * change it to a MASK op relation.
	 */
	EATAB;
	if (m->reln == '&') {
		if (*l == '>' || *l == '<' || *l == '=') {
			m->reln = *l | MASK;
			++l;
		} else
			m->reln = '=' | MASK;
		m->mask = m->value.l;
		EATAB;
		if (getvalue(m, &l))
			return -1;
	}

	/*
	 * now get last part - the description
	 */
GetDesc:
	EATAB;
	if (l[0] == '\b') {
		++l;
		m->nospflag = 1;
	} else if ((l[0] == '\\') && (l[1] == 'b')) {
		++l;
		++l;
		m->nospflag = 1;
	} else
		m->nospflag = 0;
	while ((m->desc[i++] = *l++) != '\0' && i<MAXDESC)
		/* NULLBODY */;

	if (check) {
		mdump(m);
	}
	++(*ndx);		/* make room for next */
	return 0;
}

/* 
 * Read a numeric value from a pointer, into the value union of a magic 
 * pointer, according to the magic type.  Update the string pointer to point 
 * just after the number read.  Return 0 for success, non-zero for failure.
 */
static int
getvalue(m, p)
struct magic *m;
char **p;
{
	int slen;

	if (m->type == STRING) {
		*p = getstr(*p, m->value.s, sizeof(m->value.s), &slen);
		m->vallen = slen;
	} else {
		if (m->reln != 'x') {
			switch(m->type) {
			/*
			 * Do not remove the casts below.  They are vital.
			 * When later compared with the data, the sign
			 * extension must have happened.
			 */
			case BYTE:
				m->value.l = (char) strtol(*p,p,0);
				break;
			case SHORT:
				m->value.l = (short) strtol(*p,p,0);
				break;
			case LONG:
				m->value.l = (long) strtol(*p,p,0);
				break;
			default:
				warning("can't happen: m->type=%d\n", m->type);
				return -1;
			}
		}
	}
	return 0;
}

/*
 * Convert a string containing C character escapes.  Stop at an unescaped
 * space or tab.
 * Copy the converted version to "p", returning its length in *slen.
 * Return updated scan pointer as function result.
 */
static char *
getstr(s, p, plen, slen)
register char	*s;
register char	*p;
int	plen, *slen;
{
	char	*origs = s, *origp = p;
	char	*pmax = p + plen - 1;
	register int	c;
	register int	val;

	while ((c = *s++) != '\0') {
		if (isspace(c))
			break;
		if (p >= pmax) {
			fprintf(stderr, "String too long: %s\n", origs);
			break;
		}
		if(c == '\\') {
			switch(c = *s++) {

			case '\0':
				goto out;

			default:
				*p++ = c;
				break;

			case 'n':
				*p++ = '\n';
				break;

			case 'r':
				*p++ = '\r';
				break;

			case 'b':
				*p++ = '\b';
				break;

			case 't':
				*p++ = '\t';
				break;

			case 'f':
				*p++ = '\f';
				break;

			case 'v':
				*p++ = '\v';
				break;

			/* \ and up to 3 octal digits */
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				val = c - '0';
				c = *s++;  /* try for 2 */
				if(c >= '0' && c <= '7') {
					val = (val<<3) | (c - '0');
					c = *s++;  /* try for 3 */
					if(c >= '0' && c <= '7')
						val = (val<<3) | (c-'0');
					else
						--s;
				}
				else
					--s;
				*p++ = val;
				break;

			/* \x and up to 3 hex digits */
			case 'x':
				val = 'x';	/* Default if no digits */
				c = hextoint(*s++);	/* Get next char */
				if (c >= 0) {
					val = c;
					c = hextoint(*s++);
					if (c >= 0) {
						val = (val << 4) + c;
						c = hextoint(*s++);
						if (c >= 0) {
							val = (val << 4) + c;
						} else
							--s;
					} else
						--s;
				} else
					--s;
				*p++ = val;
				break;
			}
		} else
			*p++ = c;
	}
out:
	*p = '\0';
	*slen = p - origp;
	return s;
}


/* Single hex char to int; -1 if not a hex char. */
static int
hextoint(c)
int c;
{
	if (!isascii(c))	return -1;
	if (isdigit(c))		return c - '0';
	if ((c>='a')&&(c<='f'))	return c + 10 - 'a';
	if ((c>='A')&&(c<='F'))	return c + 10 - 'A';
				return -1;
}


/*
 * Print a string containing C character escapes.
 */
void
showstr(s)
register char	*s;
{
	register char	c;

	while((c = *s++) != '\0') {
		if(c >= 040 && c <= 0176)	/* TODO isprint && !iscntrl */
			putchar(c);
		else {
			putchar('\\');
			switch (c) {
			
			case '\n':
				putchar('n');
				break;

			case '\r':
				putchar('r');
				break;

			case '\b':
				putchar('b');
				break;

			case '\t':
				putchar('t');
				break;

			case '\f':
				putchar('f');
				break;

			case '\v':
				putchar('v');
				break;

			default:
				printf("%.3o", c & 0377);
				break;
			}
		}
	}
	putchar('\t');
}
