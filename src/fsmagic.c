/*
 * fsmagic - magic based on filesystem info - directory, special files, etc.
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

#ifndef	lint
static char *moduleid = 
	"@(#)$Header: /p/file/cvsroot/file/src/fsmagic.c,v 1.11 1991/01/23 12:12:20 ian Exp $";
#endif	/* lint */

extern char *progname;
extern char *ckfmsg, *magicfile;
extern int debug;
extern FILE *efopen();


fsmagic(fn)
char *fn;
{
	extern struct stat statbuf;
	extern followLinks;
	int ret = 0;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() to identify symlinks.
	 */
#ifdef	S_IFLNK
	if (!followLinks)
		ret = lstat(fn, &statbuf);
	else
#endif
	ret = stat(fn, &statbuf);

	if (ret) {
			warning("can't stat");
			return -1;
		}

	if (statbuf.st_mode & S_ISUID) ckfputs("setuid ", stdout);
	if (statbuf.st_mode & S_ISGID) ckfputs("setgid ", stdout);
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
		{
			char buf[BUFSIZ+4];
			register int nch;
			struct stat tstatbuf;

			if ((nch = readlink(fn, buf, BUFSIZ-1)) <= 0) {
				error("readlink failed");
				return 0;
			}
			buf[nch] = '\0';	/* readlink(2) forgets this */

			/* If dangling symlink, say so and quit early. */
			if (stat(buf, tstatbuf) < 0) {
				ckfputs("dangling symbolic link", stdout);
				return 1;
			}

			/* Otherwise, handle it. */
			if (followLinks) {
				process(buf, 0);
				return 1;
			} else { /* just print what it points to */
				ckfputs("symbolic link to ", stdout);
				ckfputs(buf, stdout);
			}
		}
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

