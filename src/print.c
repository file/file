/*
 * Debugging printout routines
 */

#include <stdio.h>
#include <errno.h>
#include "file.h"

#define MAXSTR		500

extern char *progname;
extern char *magicfile;
extern int debug, nmagic;	/* number of valid magic[]s */
extern void showstr();

mdump(m)
struct magic *m;
{
	(void) printf("%d\t%d\t%d\t%c\t",
		m->contflag,
		m->offset,
		m->type,
		m->reln,
		0);
	if (m->type == STRING)
		showstr(m->value.s);
	else
		(void) printf("%d",m->value.l);
	(void) printf("\t%s", m->desc);
	(void) putchar('\n');
}

/*
 * error - print best error message possible and exit
 */
void
error(s1, s2)
char *s1, *s2;
{
	warning(s1, s2);
	exit(1);
}

/*VARARGS*/
warning(f, a)
char *f, *a;
{
	extern int errno, sys_nerr;
	extern char *sys_errlist[];
	int myerrno;

	myerrno = errno;
	(void) printf(f, a);
	if (myerrno > 0 && myerrno < sys_nerr)
		(void) printf(" (%s)", sys_errlist[myerrno]);
}
