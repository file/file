/*
 * file - find type of a file or files - main program.
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
#include <sys/types.h>
#include <sys/stat.h>
#include "file.h"

#define USAGE		"usage: %s [-c] [-f namefile] [-m magicfile] file...\n"

#ifndef	lint
static char *moduleid = 
	"@(#)$Header: /p/file/cvsroot/file/src/file.c,v 1.19 1991/02/25 10:02:28 ian Exp $";
#endif	/* lint */

extern char *ckfmsg;

int 			/* Global command-line options */
	debug = 0, 	/* debugging */
	followLinks = 0, /* follow Symlinks (BSD only) */
	zflag = 0;	/* follow (uncompress) compressed files */

int			/* Misc globals */
	nmagic = 0;	/* number of valid magic[]s */

static int nbytes = 0;	/* number of bytes read from a datafile */

FILE *efopen();

char *				/* global, read in apprentice.c */
#ifdef MAGIC
	magicfile = MAGIC;	/* where magic be found */
#else
	magicfile = "/etc/magic";	/* where magic be found */
#endif
char *progname;		/* used throughout */
struct stat statbuf;	/* global, used in a few places */
struct utimbuf {	/* for utime(2), belongs in a .h file */
	time_t actime;	/* access time */
	time_t modtime;	/* modification time */
};

/*
 * main - parse arguments and handle options
 */
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int check = 0, didsomefiles = 0, errflg = 0, ret = 0;
	extern int optind;
	extern char *optarg;

	progname = argv[0];

	while ((c = getopt(argc, argv, "cdf:Lm:z")) != EOF)
		switch (c) {
		case 'c':
			++check;
			break;
		case 'd':
			++debug;
			break;
		case 'f':
			unwrap(optarg);
			++didsomefiles;
			break;
		case 'L':
			++followLinks;
			break;
		case 'm':
			magicfile = optarg;
			break;
		case 'z':
			zflag++;
			break;
		case '?':
		default:
			errflg++;
			break;
		}
	if (errflg) {
		(void) fprintf(stderr, USAGE, progname);
		exit(2);
	}

	ret = apprentice(magicfile, check);
	if (check)
		exit(ret);

	if (optind == argc) {
		if (!didsomefiles) {
			(void)fprintf(stderr, USAGE, progname);
			exit(2);
		}
	}
	else
		for (; optind < argc; optind++)
			process(argv[optind], 1);

	return 0;
}

/*
 * unwrap -- read a file of filenames, do each one.
 */
unwrap(fn)
char *fn;
{
#define FILENAMELEN 1024
	char buf[FILENAMELEN];
	FILE *f;

	if ((f = fopen(fn, "r")) == NULL)
		(void) fprintf(stderr, "%s: file %s unreadable\n",
			progname, fn);
	else {
		while (fgets(buf, FILENAMELEN, f) != NULL) {
			buf[strlen(buf)-1] = '\0';
			process(buf, 1);
		}
		(void) fclose(f);
	}
}

/*
 * process - process input file
 */
/*ARGSUSED1*/		/* why is top no longer used? */
process(inname, top)
char	*inname;
int top;		/* true if called from top level */
{
	int	fd;
	unsigned char	buf[HOWMANY];
	struct utimbuf utbuf;

	if (strcmp("-", inname) == 0) {
		(void) printf("standard input:\t");
		if (fstat(0, &statbuf)<0)
			warning("cannot fstat; ");
		fd = 0;
		goto readit;
	}
		
	/* Try to make everything line up... This behaviour is not
	 * perfect, but was copied from the SunOS4.1 (and other vendors,
	 * but not Lotus Development) distributed file(1) commands.
	 */
	if (strlen(inname)<7)
		(void) printf("%s:\t\t", inname);
	else
		(void) printf("%s:\t", inname);

	/*
	 * first try judging the file based on its filesystem status
	 * Side effect: fsmagic updates global data `statbuf'.
	 */
	if (fsmagic(inname) != 0) {
		/*NULLBODY*/;
	} else if ((fd = open(inname, 0)) < 0) {
		/* We can't open it, but we were able to stat it. */
		if (statbuf.st_mode & 0002) ckfputs("writeable, ", stdout);
		if (statbuf.st_mode & 0111) ckfputs("executable, ", stdout);
		warning("can't read");
	} else {
readit:
		/*
		 * try looking at the first HOWMANY bytes
		 */
		if ((nbytes = read(fd, (char *)buf, HOWMANY)) == -1)
			warning("read failed");
		if (nbytes == 0) {
			ckfputs("empty", stdout);
		} else
			try(buf, nbytes);
		if (strcmp("-", inname) != 0) {
			/*
			 * Try to restore access, modification times if read it.
			 */
			utbuf.actime = statbuf.st_atime;
			utbuf.modtime = statbuf.st_mtime;
			(void) utime(inname, &utbuf); /* don't care if loses */
			(void) close(fd);
		}
	}

	(void) putchar('\n');
}

try(buf, nb)
unsigned char *buf;
int nb;
{
	/*
	 * try tests in /etc/magic (or surrogate magic file)
	 */
	if (softmagic(buf, nb) == 1)
		/*NULLBODY*/;
	else if (ascmagic(buf, nb) == 1)
		/*
		 * try known keywords, check for ascii-ness too.
		 */
		/*NULLBODY*/;
	else {
		/*
		 * abandon hope, all ye who remain here
		 */
		ckfputs("data", stdout);
	}
}
