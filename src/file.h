/*
 * definitions for file(1) program:
 */

#define HOWMANY	1024		/* how much of the file to look at */
#define MAXMAGIS 250		/* max entries in /etc/magic */
#define MAXDESC	50		/* max leng of text description */
#define MAXstring 32		/* max leng of "string" types */
#define ckfputs(str,fil) {if (fputs(str,fil)==EOF) error(ckfmsg,"");}

struct magic {
	short contflag;		/* 1 if '>0' appears */
	long offset;		/* offset to magic number */
	char reln;		/* relation (0=.eq, '>'=gt, etc) */
	short type;		/* int, short, long or string. */
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
	char desc[MAXDESC];	/* description */
};

extern void error(), exit();
