/*
 * file.h - definitions for file(1) program
 * @(#)$Ident$
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

#define HOWMANY	1024		/* how much of the file to look at */
#define MAXMAGIS 1000		/* max entries in /etc/magic */
#define MAXDESC	50		/* max leng of text description */
#define MAXstring 32		/* max leng of "string" types */
#define ckfputs(str,fil) {if (fputs(str,fil)==EOF) error(ckfmsg,"");}

struct magic {
	short contflag;		
#define CONT	1		/* if '>0' appears,  */
#define INDIR	2		/* if '>(...)' appears,  */
	struct {
		char type;	/* byte short long */
		long offset;	/* offset from indirection */
	} in;
	long offset;		/* offset to magic number */
#define	MASK	0200		/* this is a masked op, like & v1 = v2 */
	unsigned char reln;	/* relation (0=eq, '>'=gt, etc) */
	char type;		/* int, short, long or string. */
	char vallen;		/* length of string value, if any */
#define 			BYTE	1
#define				SHORT	2
#define				LONG	4
#define				STRING	5
	union VALUETYPE {
		char b;
		short h;
		long l;
		char s[MAXstring];
	} value;		/* either number or string */
	long mask;		/* mask before comparison with value */
	char nospflag;		/* supress space character */
	char desc[MAXDESC];	/* description */
};

#if	defined(__STDC__) || defined(__cplusplus)
int apprentice(char *fn, int check);
int ascmagic(unsigned char *buf, int nbytes);
void error(char *fmt, char *d);
FILE *efopen(char *fn, char *mode);
int fsmagic(char *fn);
int is_compress(unsigned char *p, int *b);
int is_tar(unsigned char *buf);
void mdump(struct magic *m);
int parse(char *l, int *ndx, int check);
void process(char *inname);
void showstr(char *s);
int softmagic(unsigned char *buf, int nbytes);
void tryit(unsigned char *buf, int nb);
int uncompress(unsigned char *old, unsigned char **newch, int n);
void warning(char *f, char *d);
#else
int apprentice();
int ascmagic();
void error();
FILE *efopen();
int fsmagic();
int is_compress();
int is_tar();
void mdump();
int parse();
void process();
void showstr();
int softmagic();
void tryit();
int uncompress();
void warning();
#endif
