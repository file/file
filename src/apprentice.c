/*
 * make one pass through /etc/magic, reading it into core
 */

#include <stdio.h>
#include <ctype.h>
#include "file.h"

#define MAXSTR		500
#define	EATAB {while (isascii(*l) && *l == '\t')  ++l;}

extern char *progname;
extern char *magicfile;
extern int debug, check;	/* options */
extern int nmagic;		/* number of valid magic[]s */
extern long strtol();

struct magic magic[MAXMAGIS];

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
	static int warned = 0;
	struct magic *m;

	if (nd+1 >= MAXMAGIS){
		if (warned++ == 0)
			warning("magic tab overflow - increase MAXMAGIS beyond %d in file/apprentice.c\n", MAXMAGIS);
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

	if (*l == '>' || *l == '&' || *l == '=') {
		m->reln = *l;
		++l;
	} else
		m->reln = '=';
	EATAB;

/* TODO finish this macro and start using it! */
#define offsetcheck {if (offset > HOWMANY-1) warning("offset too big"); }
	switch(m->type) {
	case BYTE:
		m->value.l = strtol(l,&l,0);
		break;
	case SHORT:
		m->value.l = strtol(l,&l,0);
		break;
	case LONG:
		m->value.l = strtol(l,&l,0);
		break;
	case STRING:
		/* TODO check return from sscanf (system dependant?) */
		(void) sscanf(l, "%[^\t]", m->value.s);
		l += strlen(m->value.s);
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

