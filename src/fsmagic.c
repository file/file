/*
 * magic based on mode of file - directory, special files, etc.
 */

#include <stdio.h>
#include <sys/types.h>
#ifndef	major			/* if `major' not defined in types.h, */
#include <sys/sysmacros.h>	/* try this one. */
#endif
#ifndef	major	/* still not defined? give up, manual intervention needed */
		/* If cc tries to compile this, read and act on it. */
		/* On most systems cpp will discard it automatically */
		Congratulations, you have found a portability bug.
		Please grep /usr/include/sys and edit the above #include 
		to point at the file that defines the `major' macro.
#endif	/*major*/
#include <sys/stat.h>
#include "file.h"

#define USAGE		"usage: %s [-f file] [-m magicfile] file...\n"

extern char *progname;
extern char *ckfmsg, *magicfile;
extern int debug;
extern FILE *efopen();

fsmagic(fn)
char *fn;
{
	extern struct stat statbuf;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() so identify symlinks.
	 */
#ifdef	S_IFLNK
	if (lstat(fn, &statbuf) <0)
#else
	if (stat(fn, &statbuf) <0)
#endif
		{
			warning("can't stat");
			return -1;
		}

	if (statbuf.st_mode & S_ISUID) ckfputs("suid ", stdout);
	if (statbuf.st_mode & S_ISGID) ckfputs("sgid ", stdout);
	if (statbuf.st_mode & S_ISVTX) ckfputs("sticky ", stdout);
	
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFDIR:
		ckfputs("directory", stdout);
		return 1;
	case S_IFCHR:
		(void) printf("character special (%d/%d)",
			major(statbuf.st_rdev), minor(statbuf.st_rdev));
		return 1;
	case S_IFBLK:
		(void) printf("block special (%d/%d)",
			major(statbuf.st_rdev), minor(statbuf.st_rdev));
		return 1;
	/* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef	S_IFIFO
	case S_IFIFO:
		ckfputs("fifo (named pipe)", stdout);
		return 1;
#endif
#ifdef	S_IFLNK
	case S_IFLNK:
		ckfputs("symbolic link", stdout);
		return 1;
#endif
#ifdef	S_IFSOCK
	case S_IFSOCK:
		ckfputs("socket", stdout);
		return 1;
#endif
	case S_IFREG:
		break;
	default:
		warning("invalid st_mode %d in statbuf!", statbuf.st_mode);
	}

	/*
	 * regular file, check next possibility
	 */
	if (statbuf.st_size == 0) {
		ckfputs("empty", stdout);
		return 1;
	}
	return 0;
}

