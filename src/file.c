/*
 * file - find type of a file or files
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file.h"

#define USAGE		"usage: %s [-f file] [-m magicfile] file...\n"

extern char *ckfmsg;
int check = 0,		/* check format of magic file */
	debug = 0, 	/* huh? */
	nbytes = 0,	/* number of bytes read from a datafile */
	nmagic = 0;	/* number of valid magic[]s */
FILE *efopen();
#ifdef MAGIC
char *magicfile = MAGIC;	/* where magic be found */
#else
char *magicfile = "/etc/magic";	/* where magic be found */
#endif
char *progname;
struct stat statbuf;
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
	int errflg = 0;
	extern int optind;
	extern char *optarg;

	progname = argv[0];

	while ((c = getopt(argc, argv, "cdf:m:")) != EOF)
		switch (c) {
		case 'c':
			++check;
			break;
		case 'd':
			++debug;
			break;
		case 'f':
			unwrap(optarg);
			break;
		case 'm':
			magicfile = optarg;
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

	apprentice(magicfile);

	if (optind == argc)
		(void)fprintf(stderr, USAGE, progname);
	else
		for (; optind < argc; optind++)
			process(argv[optind]);

	exit(0);
}

/*
 * unwrap -- read a file of filenames, do each one.
 */
unwrap(fn)
char *fn;
{
#define FILENAMELEN 128
	char buf[FILENAMELEN];
	FILE *f;

	if ((f = fopen(fn, "r")) == NULL)
		(void) fprintf(stderr, "%s: file %s unreadable\n",
			progname, fn);
	else {
		while (fgets(buf, FILENAMELEN, f) != NULL) {
			buf[strlen(buf)-1] = '\0';
			process(buf);
		}
		(void) fclose(f);
	}
}

/*
 * process - process input file
 */
process(inname)
char	*inname;
{
	int	fd;
	char	buf[HOWMANY];
	struct utimbuf utbuf;

	(void) printf("%s:\t", inname);

	/*
	 * first try judging the file based on its filesystem status
	 * Side effect: fsmagic updates global data `statbuf'.
	 */
	if (fsmagic(inname) != 0) {
		/*NULLBODY*/;
	} else if ((fd = open(inname, 0)) < 0) {
		warning("can't open for reading");
	} else {
		/*
		 * try looking at the first HOWMANY bytes
		 */
		if ((nbytes = read(fd, buf, HOWMANY)) == -1)
			warning("read failed");
		/*
		 * try tests in /etc/magic (or surrogate magic file
		 */
		if (softmagic(buf) == 1)
			/*NULLBODY*/;
		else if (ascmagic(buf) == 1)
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
		/*
		 * Restore access, modification times if we read the file.
		 */
		utbuf.actime = statbuf.st_atime;
		utbuf.modtime = statbuf.st_mtime;
		(void) utime(inname, &utbuf); /* don't care if we lack perms */
		(void) close(fd);
	}

	(void) putchar('\n');
}


