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

#include "file.h"
#include "magic.h"
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <sys/stat.h>
/* Since major is a function on SVR4, we can't use `ifndef major'.  */
#ifdef MAJOR_IN_MKDEV
# include <sys/mkdev.h>
# define HAVE_MAJOR
#endif
#ifdef MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
# define HAVE_MAJOR
#endif
#ifdef major			/* Might be defined in sys/types.h.  */
# define HAVE_MAJOR
#endif
  
#ifndef HAVE_MAJOR
# define major(dev)  (((dev) >> 8) & 0xff)
# define minor(dev)  ((dev) & 0xff)
#endif
#undef HAVE_MAJOR

#ifndef	lint
FILE_RCSID("@(#)$Id: fsmagic.c,v 1.37 2003/03/23 04:06:05 christos Exp $")
#endif	/* lint */

protected int
file_fsmagic(struct magic_set *ms, const char *fn, struct stat *sb)
{
	int ret = 0;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() to identify symlinks.
	 */
#ifdef	S_IFLNK
	if ((ms->flags & MAGIC_SYMLINK) != 0)
		ret = lstat(fn, sb);
	else
#endif
	ret = stat(fn, sb);	/* don't merge into if; see "ret =" above */

	if (ret) {
		if (file_printf(ms, "can't stat `%s' (%s)",
		    fn, strerror(errno)) == -1)
			return -1;
		return 1;
	}

	if ((ms->flags & MAGIC_MIME) != 0) {
		if ((sb->st_mode & S_IFMT) != S_IFREG) {
			if (file_printf(ms, "application/x-not-regular-file")
			    == -1)
				return -1;
			return 1;
		}
	}
	else {
#ifdef S_ISUID
		if (sb->st_mode & S_ISUID) 
			if (file_printf(ms, "setuid ") == -1)
				return -1;
#endif
#ifdef S_ISGID
		if (sb->st_mode & S_ISGID) 
			if (file_printf(ms, "setgid ") == -1)
				return -1;
#endif
#ifdef S_ISVTX
		if (sb->st_mode & S_ISVTX) 
			if (file_printf(ms, "sticky ") == -1)
				return -1;
#endif
	}
	
	switch (sb->st_mode & S_IFMT) {
	case S_IFDIR:
		if (file_printf(ms, "directory") == -1)
			return -1;
		return 1;
#ifdef S_IFCHR
	case S_IFCHR:
		/* 
		 * If -s has been specified, treat character special files
		 * like ordinary files.  Otherwise, just report that they
		 * are block special files and go on to the next file.
		 */
		if ((ms->flags & MAGIC_DEVICES) != 0)
			break;
#ifdef HAVE_ST_RDEV
# ifdef dv_unit
		if (file_printf(ms, "character special (%d/%d/%d)",
		    major(sb->st_rdev), dv_unit(sb->st_rdev),
		    dv_subunit(sb->st_rdev)) == -1)
			return -1;
# else
		if (file_printf(ms, "character special (%ld/%ld)",
		    (long) major(sb->st_rdev), (long) minor(sb->st_rdev)) == -1)
			return -1;
# endif
#else
		if (file_printf(ms, "character special") == -1)
			return -1;
#endif
		return 1;
#endif
#ifdef S_IFBLK
	case S_IFBLK:
		/* 
		 * If -s has been specified, treat block special files
		 * like ordinary files.  Otherwise, just report that they
		 * are block special files and go on to the next file.
		 */
		if ((ms->flags & MAGIC_DEVICES) != 0)
			break;
#ifdef HAVE_ST_RDEV
# ifdef dv_unit
		if (file_printf(ms, "block special (%d/%d/%d)",
		    major(sb->st_rdev), dv_unit(sb->st_rdev),
		    dv_subunit(sb->st_rdev)) == -1)
			return -1;
# else
		if (file_printf(ms, "block special (%ld/%ld)",
		    (long)major(sb->st_rdev), (long)minor(sb->st_rdev)) == -1)
			return -1;
# endif
#else
		if (file_printf(ms, "block special") == -1)
			return -1;
#endif
		return 1;
#endif
	/* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef	S_IFIFO
	case S_IFIFO:
		if (file_printf(ms, "fifo (named pipe)") == -1)
			return -1;
		return 1;
#endif
#ifdef	S_IFDOOR
	case S_IFDOOR:
		if (file_printf(ms, "door") == -1)
			return -1;
		return 1;
#endif
#ifdef	S_IFLNK
	case S_IFLNK:
		{
			char buf[BUFSIZ+4];
			int nch;
			struct stat tstatbuf;

			if ((nch = readlink(fn, buf, BUFSIZ-1)) <= 0) {
				if (file_printf(ms,
				    "unreadable symlink `%s' (%s)", 
				    strerror(errno)) == -1)
					return -1;
				return 1;
			}
			buf[nch] = '\0';	/* readlink(2) forgets this */

			/* If broken symlink, say so and quit early. */
			if (*buf == '/') {
			    if (stat(buf, &tstatbuf) < 0) {
				if (file_printf(ms,
				    "broken symbolic link to `%s'", buf) == -1)
					return -1;
				return 1;
			    }
			}
			else {
			    char *tmp;
			    char buf2[BUFSIZ+BUFSIZ+4];

			    if ((tmp = strrchr(fn,  '/')) == NULL) {
				tmp = buf; /* in current directory anyway */
			    }
			    else {
				strcpy(buf2, fn);  /* take directory part */
				buf2[tmp-fn+1] = '\0';
				strcat(buf2, buf); /* plus (relative) symlink */
				tmp = buf2;
			    }
			    if (stat(tmp, &tstatbuf) < 0) {
				if (file_printf(ms,
				    "broken symbolic link to `%s'",
				    buf) == -1)
					return -1;
				return 1;
			    }
                        }

			/* Otherwise, handle it. */
			if ((ms->flags & MAGIC_SYMLINK) != 0) {
				const char *p;
				ms->flags &= MAGIC_SYMLINK;
				p = magic_file(ms, buf);
				ms->flags |= MAGIC_SYMLINK;
				return p != NULL ? 1 : -1;
			} else { /* just print what it points to */
				if (file_printf(ms, "symbolic link to `%s'",
				    buf) == -1)
					return -1;
			}
		}
		return 1;
#endif
#ifdef	S_IFSOCK
#ifndef __COHERENT__
	case S_IFSOCK:
		if (file_printf(ms, "socket") == -1)
			return -1;
		return 1;
#endif
#endif
	case S_IFREG:
		break;
	default:
		file_error(ms, "invalid mode 0%o", sb->st_mode);
		return -1;
		/*NOTREACHED*/
	}

	/*
	 * regular file, check next possibility
	 *
	 * If stat() tells us the file has zero length, report here that
	 * the file is empty, so we can skip all the work of opening and 
	 * reading the file.
	 * But if the -s option has been given, we skip this optimization,
	 * since on some systems, stat() reports zero size for raw disk
	 * partitions.  (If the block special device really has zero length,
	 * the fact that it is empty will be detected and reported correctly
	 * when we read the file.)
	 */
	if ((ms->flags & MAGIC_DEVICES) == 0 && sb->st_size == 0) {
		if (file_printf(ms, (ms->flags & MAGIC_MIME) ?
		    "application/x-empty" : "empty") == -1)
			return -1;
		return 1;
	}
	return 0;
}
