/*
 * make one pass through /etc/magic, reading it into core
 */

#include <stdio.h>
#include <ctype.h>
#include "file.h"

#define MAXSTR		500
#define	EATAB {while (isascii(*l) && isspace(*l))  ++l;}


extern char *progname;
extern char *magicfile;
extern int debug, check;	/* options */
extern int nmagic;		/* number of valid magic[]s */
extern long strtol();

struct magic magic[MAXMAGIS];

char *getstr();

apprentice(fn)
char *fn;
{
	FILE *f;
	char line[MAXSTR+1];

	f = fopen(fn, "r");
	if (f==NULL) {
		(void) fprintf(stderr, "%s: can't read magic file %s\n",
		progname, fn);
		exit(1);
	}

	/* parse it */
	if (check)	/* print silly verbose header for USG compat. */
		(void) printf("cont\toffset\ttype\topcode\tvalue\tdesc\n");
	while (fgets(line, MAXSTR, f) != NULL) {
		if (line[0]=='#')	/* comment, do not parse */
			continue;
		if (strlen(line) <= 1)	/* null line, garbage, etc */
			continue;
		line[strlen(line)-1] = '\0'; /* delete newline */
		(void) parse(line, &nmagic);
	}

	(void) fclose(f);
}

/*
 * parse one line from magic file, put into magic[index++] if valid
 */


parse(l, ndx)
char *l;
int *ndx;
{
	int i = 0, nd = *ndx;
	int slen;
	static int warned = 0;
	struct magic *m;

	if (nd+1 >= MAXMAGIS){
		if (warned++ == 0)
			warning("magic table overflow - increase MAXMAGIS beyond %d in file/apprentice.c\n", MAXMAGIS);
		return 0;
	}
	m = &magic[*ndx];

	if (*l == '>') {
		++l;		/* step over */
		m->contflag = 1;
	} else
		m->contflag = 0;

	/* get offset, then skip over it */
	m->offset = atoi(l);
	while (isascii(*l) && isdigit(*l))
		++l;
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
		warning("type %s invalid\n", l);
		return 0;
	}
	EATAB;

	if (*l == '>' || *l == '<' || *l == '&' || *l == '=') {
		m->reln = *l;
		++l;
	} else
		m->reln = '=';
	EATAB;

/* TODO finish this macro and start using it! */
#define offsetcheck {if (offset > HOWMANY-1) warning("offset too big"); }
	switch(m->type) {
	/*
	 * Do not remove the casts below.  They are vital.
	 * When later compared with the data, the sign extension must
	 * have happened.
	 */
	case BYTE:
		m->value.l = (char) strtol(l,&l,0);
		break;
	case SHORT:
		m->value.l = (short) strtol(l,&l,0);
		break;
	case LONG:
		m->value.l = (long) strtol(l,&l,0);
		break;
	case STRING:
		l = getstr(l, m->value.s, sizeof(m->value.s), &slen);
		m->vallen = slen;
		break;
	default:
		warning("can't happen: m->type=%d\n", m->type);
	}

	/*
	 * now get last part - the description
	 */
	EATAB;
	while ((m->desc[i++] = *l++) != '\0' && i<MAXDESC)
		/* NULLBODY */;

	if (check) {
		mdump(m);
	}
	++(*ndx);		/* make room for next */
	return 1;
}

/*
 * Convert a string containing C character escapes.  Stop at an unescaped
 * space or tab.
 * Copy the converted version to "p", returning its length in *slen.
 * Return updated scan pointer as function result.
 */
char *
getstr(s, p, plen, slen)
register char	*s;
register char	*p;
int	plen, *slen;
{
	char	*origs = s, *origp = p;
	char	*pmax = p + plen - 1;
	register char	c;
	register int	val;

	while((c = *s++) != '\0') {
		if (isspace(c)) break;
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
	return(s);
}


/* Single hex char to int; -1 if not a hex char. */
int
hextoint(c)
	char c;
{
	if (!isascii(c))	return -1;
	if (isdigit(c))		return c - '0';
	if ((c>='a')&(c<='f'))	return c + 10 - 'a';
	if ((c>='A')&(c<='F'))	return c + 10 - 'A';
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
		if(c >= 040 && c <= 0176)
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
