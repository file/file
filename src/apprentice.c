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

#include "file.h"
#include "magic.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>
#ifdef QUICK
#include <sys/mman.h>
#endif

#ifndef	lint
FILE_RCSID("@(#)$Id: apprentice.c,v 1.52 2003/03/23 04:06:04 christos Exp $")
#endif	/* lint */

#define	EATAB {while (isascii((unsigned char) *l) && \
		      isspace((unsigned char) *l))  ++l;}
#define LOWCASE(l) (isupper((unsigned char) (l)) ? \
			tolower((unsigned char) (l)) : (l))
/*
 * Work around a bug in headers on Digital Unix.
 * At least confirmed for: OSF1 V4.0 878
 */
#if defined(__osf__) && defined(__DECC)
#ifdef MAP_FAILED
#undef MAP_FAILED
#endif
#endif

#ifndef MAP_FAILED
#define MAP_FAILED (void *) -1
#endif

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifdef __EMX__
  char PATHSEP=';';
#else
  char PATHSEP=':';
#endif


private int getvalue(struct magic_set *ms, struct magic *, char **);
private int hextoint(int);
private char *getstr(struct magic_set *, char *, char *, int, int *);
private int parse(struct magic_set *, struct magic **, uint32_t *, char *, int);
private void eatsize(char **);
private int apprentice_1(struct magic_set *, const char *, int, struct mlist *);
private int apprentice_file(struct magic_set *, struct magic **, uint32_t *,
    const char *, int);
private void byteswap(struct magic *, uint32_t);
private void bs1(struct magic *);
private uint16_t swap2(uint16_t);
private uint32_t swap4(uint32_t);
private char *mkdbname(struct magic_set *, const char *, char *, size_t);
private int apprentice_map(struct magic_set *, struct magic **, uint32_t *,
    const char *, int);
private int apprentice_compile(struct magic_set *, struct magic **, uint32_t *,
    const char *, int);

private int maxmagic = 0;

#ifdef COMPILE_ONLY
const char *magicfile;
char *progname;
int lineno;

int main(int, char *[]);

int
main(int argc, char *argv[])
{
	int ret;

	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	if (argc != 2) {
		(void)fprintf(stderr, "usage: %s file\n", progname);
		exit(1);
	}
	magicfile = argv[1];

	exit(apprentice(magicfile, COMPILE, MAGIC_CHECK));
}
#endif /* COMPILE_ONLY */


/*
 * Handle one file.
 */
private int
apprentice_1(struct magic_set *ms, const char *fn, int action,
    struct mlist *mlist)
{
	struct magic *magic = NULL;
	uint32_t nmagic = 0;
	struct mlist *ml;
	int rv = -1;
	int mapped;

	if (action == COMPILE) {
		rv = apprentice_file(ms, &magic, &nmagic, fn, action);
		if (rv == 0) {
			rv = apprentice_compile(ms, &magic, &nmagic, fn,
			    action);
			free(magic);
		}
		return rv;
	}
#ifndef COMPILE_ONLY
	if ((rv = apprentice_map(ms, &magic, &nmagic, fn, action)) == -1) {
		if (ms->flags & MAGIC_CHECK)
			file_magwarn("Using regular magic file `%s'", fn);
		rv = apprentice_file(ms, &magic, &nmagic, fn, action);
		mapped = 0;
	}

	if (rv == -1)
		return rv;
	mapped = rv;
	     
	if ((ml = malloc(sizeof(*ml))) == NULL) {
		file_oomem(ms);
		return -1;
	}

	if (magic == NULL || nmagic == 0)
		return -1;

	ml->magic = magic;
	ml->nmagic = nmagic;
	ml->mapped = mapped;

	mlist->prev->next = ml;
	ml->prev = mlist->prev;
	ml->next = mlist;
	mlist->prev = ml;

	return 0;
#endif /* COMPILE_ONLY */
}


/* const char *fn: list of magic files */
protected struct mlist *
file_apprentice(struct magic_set *ms, const char *fn, int action)
{
	char *p, *mfn;
	int file_err, errs = -1;
	struct mlist *mlist;

	if ((fn = mfn = strdup(fn)) == NULL) {
		file_oomem(ms);
		return NULL;
	}
	if ((mlist = malloc(sizeof(*mlist))) == NULL) {
		free(mfn);
		file_oomem(ms);
		return NULL;
	}
	mlist->next = mlist->prev = mlist;

	while (fn) {
		p = strchr(fn, PATHSEP);
		if (p)
			*p++ = '\0';
		file_err = apprentice_1(ms, fn, action, mlist);
		if (file_err > errs)
			errs = file_err;
		fn = p;
	}
	if (errs == -1) {
		free(mfn);
		free(mlist);
		mlist = NULL;
		file_error(ms, "Couldn't find any magic files!");
		return NULL;
	}
	free(mfn);
	return mlist;
}

/*
 * parse from a file
 * const char *fn: name of magic file
 */
private int
apprentice_file(struct magic_set *ms, struct magic **magicp, uint32_t *nmagicp,
    const char *fn, int action)
{
	private const char hdr[] =
		"cont\toffset\ttype\topcode\tmask\tvalue\tdesc";
	FILE *f;
	char line[BUFSIZ+1];
	int lineno;
	int errs = 0;

	f = fopen(fn, "r");
	if (f == NULL) {
		if (errno != ENOENT)
			file_error(ms, "Can't read magic file %s (%s)",
			    fn, strerror(errno));
		return -1;
	}

        maxmagic = MAXMAGIS;
	*magicp = (struct magic *) calloc(maxmagic, sizeof(struct magic));
	if (*magicp == NULL) {
		(void)fclose(f);
		file_oomem(ms);
		return -1;
	}

	/* parse it */
	if (action == CHECK)	/* print silly verbose header for USG compat. */
		(void) printf("%s\n", hdr);

	for (lineno = 1; fgets(line, BUFSIZ, f) != NULL; lineno++) {
		if (line[0]=='#')	/* comment, do not parse */
			continue;
		if (strlen(line) <= (unsigned)1) /* null line, garbage, etc */
			continue;
		line[strlen(line)-1] = '\0'; /* delete newline */
		if (parse(ms, magicp, nmagicp, line, action) != 0)
			errs = 1;
	}

	(void)fclose(f);
	if (errs) {
		free(*magicp);
		*magicp = NULL;
		*nmagicp = 0;
	}
	return errs;
}

/*
 * extend the sign bit if the comparison is to be signed
 */
protected uint32_t
file_signextend(struct magic_set *ms, struct magic *m, uint32_t v)
{
	if (!(m->flag & UNSIGNED))
		switch(m->type) {
		/*
		 * Do not remove the casts below.  They are
		 * vital.  When later compared with the data,
		 * the sign extension must have happened.
		 */
		case BYTE:
			v = (char) v;
			break;
		case SHORT:
		case BESHORT:
		case LESHORT:
			v = (short) v;
			break;
		case DATE:
		case BEDATE:
		case LEDATE:
		case LDATE:
		case BELDATE:
		case LELDATE:
		case LONG:
		case BELONG:
		case LELONG:
			v = (int32_t) v;
			break;
		case STRING:
		case PSTRING:
			break;
		case REGEX:
			break;
		default:
			if (ms->flags & MAGIC_CHECK)
			    file_magwarn("can't happen: m->type=%d\n",
				    m->type);
			return -1;
		}
	return v;
}

/*
 * parse one line from magic file, put into magic[index++] if valid
 */
private int
parse(struct magic_set *ms, struct magic **magicp, uint32_t *nmagicp, char *l,
    int action)
{
	int i = 0;
	struct magic *m;
	char *t;

#define ALLOC_INCR	200
	if (*nmagicp + 1 >= maxmagic){
		maxmagic += ALLOC_INCR;
		if ((m = (struct magic *) realloc(*magicp,
		    sizeof(struct magic) * maxmagic)) == NULL) {
			file_oomem(ms);
			if (*magicp)
				free(*magicp);
			return -1;
		}
		*magicp = m;
		memset(&(*magicp)[*nmagicp], 0, sizeof(struct magic)
		    * ALLOC_INCR);
	}
	m = &(*magicp)[*nmagicp];
	m->flag = 0;
	m->cont_level = 0;

	while (*l == '>') {
		++l;		/* step over */
		m->cont_level++; 
	}

	if (m->cont_level != 0 && *l == '(') {
		++l;		/* step over */
		m->flag |= INDIR;
	}
	if (m->cont_level != 0 && *l == '&') {
                ++l;            /* step over */
                m->flag |= OFFADD;
        }

	/* get offset, then skip over it */
	m->offset = (int) strtoul(l,&t,0);
        if (l == t)
		if (ms->flags & MAGIC_CHECK)
			file_magwarn("offset %s invalid", l);
        l = t;

	if (m->flag & INDIR) {
		m->in_type = LONG;
		m->in_offset = 0;
		/*
		 * read [.lbs][+-]nnnnn)
		 */
		if (*l == '.') {
			l++;
			switch (*l) {
			case 'l':
				m->in_type = LELONG;
				break;
			case 'L':
				m->in_type = BELONG;
				break;
			case 'h':
			case 's':
				m->in_type = LESHORT;
				break;
			case 'H':
			case 'S':
				m->in_type = BESHORT;
				break;
			case 'c':
			case 'b':
			case 'C':
			case 'B':
				m->in_type = BYTE;
				break;
			default:
				if (ms->flags & MAGIC_CHECK)
					file_magwarn(
					    "indirect offset type %c invalid",
					    *l);
				break;
			}
			l++;
		}
		if (*l == '~') {
			m->in_op = OPINVERSE;
			l++;
		}
		switch (*l) {
		case '&':
			m->in_op |= OPAND;
			l++;
			break;
		case '|':
			m->in_op |= OPOR;
			l++;
			break;
		case '^':
			m->in_op |= OPXOR;
			l++;
			break;
		case '+':
			m->in_op |= OPADD;
			l++;
			break;
		case '-':
			m->in_op |= OPMINUS;
			l++;
			break;
		case '*':
			m->in_op |= OPMULTIPLY;
			l++;
			break;
		case '/':
			m->in_op |= OPDIVIDE;
			l++;
			break;
		case '%':
			m->in_op |= OPMODULO;
			l++;
			break;
		}
		if (isdigit((unsigned char)*l)) 
			m->in_offset = strtoul(l, &t, 0);
		else
			t = l;
		if (*t++ != ')') 
			if (ms->flags & MAGIC_CHECK)
				file_magwarn("missing ')' in indirect offset");
		l = t;
	}


	while (isascii((unsigned char)*l) && isdigit((unsigned char)*l))
		++l;
	EATAB;

#define NBYTE		4
#define NSHORT		5
#define NLONG		4
#define NSTRING 	6
#define NDATE		4
#define NBESHORT	7
#define NBELONG		6
#define NBEDATE		6
#define NLESHORT	7
#define NLELONG		6
#define NLEDATE		6
#define NPSTRING	7
#define NLDATE		5
#define NBELDATE	7
#define NLELDATE	7
#define NREGEX		5

	if (*l == 'u') {
		++l;
		m->flag |= UNSIGNED;
	}

	/* get type, skip it */
	if (strncmp(l, "char", NBYTE)==0) {	/* HP/UX compat */
		m->type = BYTE;
		l += NBYTE;
	} else if (strncmp(l, "byte", NBYTE)==0) {
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
	} else if (strncmp(l, "date", NDATE)==0) {
		m->type = DATE;
		l += NDATE;
	} else if (strncmp(l, "beshort", NBESHORT)==0) {
		m->type = BESHORT;
		l += NBESHORT;
	} else if (strncmp(l, "belong", NBELONG)==0) {
		m->type = BELONG;
		l += NBELONG;
	} else if (strncmp(l, "bedate", NBEDATE)==0) {
		m->type = BEDATE;
		l += NBEDATE;
	} else if (strncmp(l, "leshort", NLESHORT)==0) {
		m->type = LESHORT;
		l += NLESHORT;
	} else if (strncmp(l, "lelong", NLELONG)==0) {
		m->type = LELONG;
		l += NLELONG;
	} else if (strncmp(l, "ledate", NLEDATE)==0) {
		m->type = LEDATE;
		l += NLEDATE;
	} else if (strncmp(l, "pstring", NPSTRING)==0) {
		m->type = PSTRING;
		l += NPSTRING;
	} else if (strncmp(l, "ldate", NLDATE)==0) {
		m->type = LDATE;
		l += NLDATE;
	} else if (strncmp(l, "beldate", NBELDATE)==0) {
		m->type = BELDATE;
		l += NBELDATE;
	} else if (strncmp(l, "leldate", NLELDATE)==0) {
		m->type = LELDATE;
		l += NLELDATE;
	} else if (strncmp(l, "regex", NREGEX)==0) {
		m->type = REGEX;
		l += sizeof("regex");
	} else {
		if (ms->flags & MAGIC_CHECK)
			file_magwarn("type %s invalid", l);
		return -1;
	}
	/* New-style anding: "0 byte&0x80 =0x80 dynamically linked" */
	/* New and improved: ~ & | ^ + - * / % -- exciting, isn't it? */
	if (*l == '~') {
		if (STRING != m->type && PSTRING != m->type)
			m->mask_op = OPINVERSE;
		++l;
	}
	switch (*l) {
	case '&':
		m->mask_op |= OPAND;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '|':
		m->mask_op |= OPOR;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '^':
		m->mask_op |= OPXOR;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '+':
		m->mask_op |= OPADD;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '-':
		m->mask_op |= OPMINUS;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '*':
		m->mask_op |= OPMULTIPLY;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '%':
		m->mask_op |= OPMODULO;
		++l;
		m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
		eatsize(&l);
		break;
	case '/':
		if (STRING != m->type && PSTRING != m->type) {
			m->mask_op |= OPDIVIDE;
			++l;
			m->mask = file_signextend(ms, m, strtoul(l, &l, 0));
			eatsize(&l);
		} else {
			m->mask = 0L;
			while (!isspace(*++l)) {
				switch (*l) {
				case CHAR_IGNORE_LOWERCASE:
					m->mask |= STRING_IGNORE_LOWERCASE;
					break;
				case CHAR_COMPACT_BLANK:
					m->mask |= STRING_COMPACT_BLANK;
					break;
				case CHAR_COMPACT_OPTIONAL_BLANK:
					m->mask |=
					    STRING_COMPACT_OPTIONAL_BLANK;
					break;
				default:
					if (ms->flags & MAGIC_CHECK)
						file_magwarn(
						"string extension %c invalid",
						*l);
					return -1;
				}
			}
		}
		break;
	}
	/* We used to set mask to all 1's here, instead let's just not do anything 
	   if mask = 0 (unless you have a better idea) */
	EATAB;
  
	switch (*l) {
	case '>':
	case '<':
	/* Old-style anding: "0 byte &0x80 dynamically linked" */
	case '&':
	case '^':
	case '=':
  		m->reln = *l;
  		++l;
		if (*l == '=') {
		   /* HP compat: ignore &= etc. */
		   ++l;
		}
		break;
	case '!':
		if (m->type != STRING && m->type != PSTRING) {
			m->reln = *l;
			++l;
			break;
		}
		/* FALL THROUGH */
	default:
		if (*l == 'x' && isascii((unsigned char)l[1]) && 
		    isspace((unsigned char)l[1])) {
			m->reln = *l;
			++l;
			goto GetDesc;	/* Bill The Cat */
		}
  		m->reln = '=';
		break;
	}
  	EATAB;
  
	if (getvalue(ms, m, &l))
		return -1;
	/*
	 * TODO finish this macro and start using it!
	 * #define offsetcheck {if (offset > HOWMANY-1) 
	 *	magwarn("offset too big"); }
	 */

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

#ifndef COMPILE_ONLY
	if (action == CHECK) {
		file_mdump(m);
	}
#endif
	++(*nmagicp);		/* make room for next */
	return 0;
}

/* 
 * Read a numeric value from a pointer, into the value union of a magic 
 * pointer, according to the magic type.  Update the string pointer to point 
 * just after the number read.  Return 0 for success, non-zero for failure.
 */
private int
getvalue(struct magic_set *ms, struct magic *m, char **p)
{
	int slen;

	if (m->type == STRING || m->type == PSTRING || m->type == REGEX) {
		*p = getstr(ms, *p, m->value.s, sizeof(m->value.s), &slen);
		if (*p == NULL)
			return -1;
		m->vallen = slen;
	} else
		if (m->reln != 'x') {
			m->value.l = file_signextend(ms, m, strtoul(*p, p, 0));
			eatsize(p);
		}
	return 0;
}

/*
 * Convert a string containing C character escapes.  Stop at an unescaped
 * space or tab.
 * Copy the converted version to "p", returning its length in *slen.
 * Return updated scan pointer as function result.
 */
private char *
getstr(struct magic_set *ms, char *s, char *p, int plen, int *slen)
{
	char	*origs = s, *origp = p;
	char	*pmax = p + plen - 1;
	int	c;
	int	val;

	while ((c = *s++) != '\0') {
		if (isspace((unsigned char) c))
			break;
		if (p >= pmax) {
			file_error(ms, "String too long: `%s'", origs);
			return NULL;
		}
		if(c == '\\') {
			switch(c = *s++) {

			case '\0':
				goto out;

			default:
				*p++ = (char) c;
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
				*p++ = (char)val;
				break;

			/* \x and up to 2 hex digits */
			case 'x':
				val = 'x';	/* Default if no digits */
				c = hextoint(*s++);	/* Get next char */
				if (c >= 0) {
					val = c;
					c = hextoint(*s++);
					if (c >= 0)
						val = (val << 4) + c;
					else
						--s;
				} else
					--s;
				*p++ = (char)val;
				break;
			}
		} else
			*p++ = (char)c;
	}
out:
	*p = '\0';
	*slen = p - origp;
	return s;
}


/* Single hex char to int; -1 if not a hex char. */
private int
hextoint(int c)
{
	if (!isascii((unsigned char) c))
		return -1;
	if (isdigit((unsigned char) c))
		return c - '0';
	if ((c >= 'a')&&(c <= 'f'))
		return c + 10 - 'a';
	if (( c>= 'A')&&(c <= 'F'))
		return c + 10 - 'A';
	return -1;
}


/*
 * Print a string containing C character escapes.
 */
protected void
file_showstr(FILE *fp, const char *s, int len)
{
	char	c;

	for (;;) {
		c = *s++;
		if (len == -1) {
			if (c == '\0')
				break;
		}
		else  {
			if (len-- == 0)
				break;
		}
		if(c >= 040 && c <= 0176)	/* TODO isprint && !iscntrl */
			(void) fputc(c, fp);
		else {
			(void) fputc('\\', fp);
			switch (c) {
			
			case '\n':
				(void) fputc('n', fp);
				break;

			case '\r':
				(void) fputc('r', fp);
				break;

			case '\b':
				(void) fputc('b', fp);
				break;

			case '\t':
				(void) fputc('t', fp);
				break;

			case '\f':
				(void) fputc('f', fp);
				break;

			case '\v':
				(void) fputc('v', fp);
				break;

			default:
				(void) fprintf(fp, "%.3o", c & 0377);
				break;
			}
		}
	}
}

/*
 * eatsize(): Eat the size spec from a number [eg. 10UL]
 */
private void
eatsize(char **p)
{
	char *l = *p;

	if (LOWCASE(*l) == 'u') 
		l++;

	switch (LOWCASE(*l)) {
	case 'l':    /* long */
	case 's':    /* short */
	case 'h':    /* short */
	case 'b':    /* char/byte */
	case 'c':    /* char/byte */
		l++;
		/*FALLTHROUGH*/
	default:
		break;
	}

	*p = l;
}

/*
 * handle a compiled file.
 */
private int
apprentice_map(struct magic_set *ms, struct magic **magicp, uint32_t *nmagicp,
    const char *fn, int action)
{
	int fd;
	struct stat st;
	uint32_t *ptr;
	uint32_t version;
	int needsbyteswap;
	char buf[MAXPATHLEN];
	char *dbname = mkdbname(ms, fn, buf, sizeof(buf));
	void *mm;

	if (dbname == NULL)
		return -1;

	if ((fd = open(dbname, O_RDONLY)) == -1)
		return -1;

	if (fstat(fd, &st) == -1) {
		file_error(ms, "Cannot stat `%s' (%s)", dbname,
		    strerror(errno));
		goto error;
	}

#ifdef QUICK
	if ((mm = mmap(0, (size_t)st.st_size, PROT_READ|PROT_WRITE,
	    MAP_PRIVATE|MAP_FILE, fd, (off_t)0)) == MAP_FAILED) {
		file_error(ms, "Cannot map `%s' (%s)", dbname, strerror(errno));
		goto error;
	}
#else
	if ((mm = malloc((size_t)st.st_size)) == NULL) {
		file_oomem(ms);
		goto error;
	}
	if (read(fd, mm, (size_t)st.st_size) != (size_t)st.st_size) {
		file_error(ms, "Read failed (%s)", strerror(errno));
		goto error;
	}
#endif
	*magicp = mm;
	(void)close(fd);
	fd = -1;
	ptr = (uint32_t *) *magicp;
	if (*ptr != MAGICNO) {
		if (swap4(*ptr) != MAGICNO) {
			file_error(ms, "Bad magic in `%s'", dbname);
			goto error;
		}
		needsbyteswap = 1;
	} else
		needsbyteswap = 0;
	if (needsbyteswap)
		version = swap4(ptr[1]);
	else
		version = ptr[1];
	if (version != VERSIONNO) {
		file_error(ms, "version mismatch (%d != %d) in `%s'",
		    version, VERSIONNO, dbname);
		goto error;
	}
	*nmagicp = (st.st_size / sizeof(struct magic)) - 1;
	(*magicp)++;
	if (needsbyteswap)
		byteswap(*magicp, *nmagicp);
	return 0;

error:
	if (fd != -1)
		(void)close(fd);
	if (mm) {
#ifdef QUICK
		(void)munmap(mm, (size_t)st.st_size);
#else
		free(mm);
#endif
	} else {
		*magicp = NULL;
		*nmagicp = 0;
	}
	return -1;
}

private const uint32_t ar[] = {
    MAGICNO, VERSIONNO
};
/*
 * handle an mmaped file.
 */
private int
apprentice_compile(struct magic_set *ms, struct magic **magicp,
    uint32_t *nmagicp, const char *fn, int action)
{
	int fd;
	char buf[MAXPATHLEN];
	char *dbname = mkdbname(ms, fn, buf, sizeof(buf));

	if (dbname == NULL) 
		return -1;

	if ((fd = open(dbname, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1) {
		file_error(ms, "Cannot open `%s' (%s)", dbname, strerror(errno));
		return -1;
	}

	if (write(fd, ar, sizeof(ar)) != sizeof(ar)) {
		file_error(ms, "Error writing `%s' (%s)", dbname,
		    strerror(errno));
		return -1;
	}

	if (lseek(fd, sizeof(struct magic), SEEK_SET) != sizeof(struct magic)) {
		file_error(ms, "Error seeking `%s' (%s)", dbname,
		    strerror(errno));
		return -1;
	}

	if (write(fd, *magicp,  sizeof(struct magic) * *nmagicp) 
	    != sizeof(struct magic) * *nmagicp) {
		file_error(ms, "Error writing `%s' (%s)", dbname,
		    strerror(errno));
		return -1;
	}

	(void)close(fd);
	return 0;
}

private const char ext[] = ".mgc";
/*
 * make a dbname
 */
private char *
mkdbname(struct magic_set *ms, const char *fn, char *buf, size_t bufsiz)
{
	const char *p;
	if ((p = strrchr(fn, '/')) != NULL)
		p++;
	else
		p = fn;
	snprintf(buf, bufsiz, "%s%s", p, ext);
	return buf;
}

/*
 * Byteswap an mmap'ed file if needed
 */
private void
byteswap(struct magic *magic, uint32_t nmagic)
{
	uint32_t i;
	for (i = 0; i < nmagic; i++)
		bs1(&magic[i]);
}

/*
 * swap a short
 */
private uint16_t
swap2(uint16_t sv)
{
	uint16_t rv;
	uint8_t *s = (uint8_t *) &sv; 
	uint8_t *d = (uint8_t *) &rv; 
	d[0] = s[1];
	d[1] = s[0];
	return rv;
}

/*
 * swap an int
 */
private uint32_t
swap4(uint32_t sv)
{
	uint32_t rv;
	uint8_t *s = (uint8_t *) &sv; 
	uint8_t *d = (uint8_t *) &rv; 
	d[0] = s[3];
	d[1] = s[2];
	d[2] = s[1];
	d[3] = s[0];
	return rv;
}

/*
 * byteswap a single magic entry
 */
private void
bs1(struct magic *m)
{
	m->cont_level = swap2(m->cont_level);
	m->offset = swap4(m->offset);
	m->in_offset = swap4(m->in_offset);
	if (m->type != STRING)
		m->value.l = swap4(m->value.l);
	m->mask = swap4(m->mask);
}
