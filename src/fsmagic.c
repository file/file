/*
 * Copyright (c) Ian F. Darwin 1986-1995.
 * Software written by Ian F. Darwin and others;
 * maintained 1995-present by Christos Zoulas and others.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * fsmagic - magic based on filesystem info - directory, special files, etc.
 */

#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: fsmagic.c,v 1.89 2026/08/28 15:33:00 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
/* Since major is a function on SVR4, we cannot use `ifndef major'.  */
#ifdef MAJOR_IN_MKDEV
# include <sys/mkdev.h>
# define HAVE_MAJOR
#endif
#ifdef HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
# define HAVE_MAJOR
#endif
#if defined(major) && !defined(HAVE_MAJOR)
/* Might be defined in sys/types.h.  */
# define HAVE_MAJOR
#endif
#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#ifndef HAVE_MAJOR
# define major(dev)  (((dev) >> 8) & 0xff)
# define minor(dev)  ((dev) & 0xff)
#endif
#undef HAVE_MAJOR


struct vn {
	uint32_t v;
	const char *s;
};
static const struct vn sugid[] = {
#ifdef S_ISUID
	{ S_ISUID, "setuid" },
#endif
#ifdef S_ISGID
	{ S_ISGID, "setgid" },
#endif
#ifdef S_ISVTX
	{ S_ISVTX, "sticky" },
#endif
};
#if HAVE_STRUCT_STAT_ST_FLAGS
static const struct vn sflags[] = {
#ifdef UF_NODUMP
	{ UF_NODUMP, "unodump" },
#endif
#ifdef UF_IMMUTABLE
	{ UF_IMMUTABLE, "uimmutable" },
#endif
#ifdef UF_APPEND
	{ UF_APPEND, "uappend" },
#endif
#ifdef UF_OPAQUE
	{ UF_OPAQUE, "uopaque" },
#endif
#ifdef UF_NOUNLINK
	{ UF_NOUNLINK, "unounlink" },
#endif
#ifdef SF_ARCHIVED
	{ SF_ARCHIVED, "sarchived" },
#endif
#ifdef SF_IMMUTABLE
	{ SF_IMMUTABLE, "simmutable" },
#endif
#ifdef SF_APPEND
	{ SF_APPEND, "sappend" },
#endif
#ifdef SF_NOUNLINK
	{ SF_NOUNLINK, "snounlink" },
#endif
#ifdef SF_SNAPSHOT
	{ SF_SNAPSHOT, "ssnapshot" },
#endif
#ifdef SF_LOG
	{ SF_LOG, "slog" },
#endif
#ifdef SF_SNAPINVAL
	{ SF_LOG, "ssnapinval" },
#endif
};
#endif

#ifdef	S_IFLNK
file_private int
bad_link(struct magic_set *ms, int err, char *buf)
{
	int mime = ms->flags & MAGIC_MIME;
	if ((mime & MAGIC_MIME_TYPE) &&
	    file_printf(ms, "inode/symlink")
	    == -1)
		return -1;
	else if (!mime) {
		if (ms->flags & MAGIC_ERROR) {
			file_error(ms, err,
				   "broken symbolic link to %s", buf);
			return -1;
		}
		if (file_printf(ms, "broken symbolic link to %s", buf) == -1)
			return -1;
	}
	return 1;
}
#endif
file_private int
handle_mime(struct magic_set *ms, int mime, const char *str)
{
	if ((mime & MAGIC_MIME_TYPE)) {
		if (file_printf(ms, "inode/%s", str) == -1)
			return -1;
		if ((mime & MAGIC_MIME_ENCODING) && file_printf(ms,
		    "; charset=") == -1)
			return -1;
	}
	if ((mime & MAGIC_MIME_ENCODING) && file_printf(ms, "binary") == -1)
		return -1;
	return 0;
}

#define COMMA	((*did)++ ? ", " : "")

static int
pr_flags(struct magic_set *ms, uint32_t flag, const struct vn *nvlist,
    size_t nvsize, int *did)
{
	for (size_t i = 0; i < nvsize; i++)
		if (flag & nvlist[i].v)
			if (file_printf(ms, "%s%s", nvlist[i].s, COMMA) == -1)
				return -1;
	return 0;
}

file_protected int
file_fsmagic(struct magic_set *ms, const char *fn, struct stat *sb)
{
	int ret, didv = 0, *did;
	int mime = ms->flags & MAGIC_MIME;
	int silent = ms->flags & (MAGIC_APPLE|MAGIC_EXTENSION);
#ifdef	S_IFLNK
	char buf[BUFSIZ+4];
	ssize_t nch;
	struct stat tstatbuf;
#endif
	did = &didv;

	if (fn == NULL)
		return 0;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() to identify symlinks.
	 */
#ifdef	S_IFLNK
	if ((ms->flags & MAGIC_SYMLINK) == 0)
		ret = lstat(fn, sb);
	else
#endif
	ret = stat(fn, sb);	/* don't merge into if; see "ret =" above */

#ifdef WIN32
	{
		HANDLE hFile = CreateFile((LPCSTR)fn, 0, FILE_SHARE_DELETE |
		    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0,
		    NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			/*
			 * Stat failed, but we can still open it - assume it's
			 * a block device, if nothing else.
			 */
			if (ret) {
				sb->st_mode = S_IFBLK;
				ret = 0;
			}
			switch (GetFileType(hFile)) {
			case FILE_TYPE_CHAR:
				sb->st_mode |= S_IFCHR;
				sb->st_mode &= ~S_IFREG;
				break;
			case FILE_TYPE_PIPE:
				sb->st_mode |= S_IFIFO;
				sb->st_mode &= ~S_IFREG;
				break;
			}
			CloseHandle(hFile);
		}
	}
#endif

	if (ret) {
		if (ms->flags & MAGIC_ERROR) {
			file_error(ms, errno, "cannot stat `%s'", fn);
			return -1;
		}
		if (file_printf(ms, "cannot open `%s' (%s)",
		    fn, strerror(errno)) == -1)
			return -1;
		return 0;
	}

	ret = 1;
	if (!mime && !silent) {
		if (pr_flags(ms, sb->st_mode, sugid, __arraycount(sugid),
		    did) == -1)
			return -1;
#if HAVE_STRUCT_STAT_ST_FLAGS
		if (pr_flags(ms, sb->st_flags, sflags, __arraycount(sflags),
		    did) == -1)
			return -1;
#endif
	}

	switch (sb->st_mode & S_IFMT) {
	case S_IFDIR:
		if (mime) {
			if (handle_mime(ms, mime, "directory") == -1)
				return -1;
		} else if (silent) {
		} else if (file_printf(ms, "%sdirectory", COMMA) == -1)
			return -1;
		break;
#ifdef S_IFCHR
	case S_IFCHR:
		/*
		 * If -s has been specified, treat character special files
		 * like ordinary files.  Otherwise, just report that they
		 * are block special files and go on to the next file.
		 */
		if ((ms->flags & MAGIC_DEVICES) != 0) {
			ret = 0;
			break;
		}
		if (mime) {
			if (handle_mime(ms, mime, "chardevice") == -1)
				return -1;
		} else if (silent) {
		} else {
#ifdef HAVE_STRUCT_STAT_ST_RDEV
# ifdef dv_unit
			if (file_printf(ms, "%scharacter special (%d/%d/%d)",
			    COMMA, major(sb->st_rdev), dv_unit(sb->st_rdev),
					dv_subunit(sb->st_rdev)) == -1)
				return -1;
# else
			if (file_printf(ms, "%scharacter special (%ld/%ld)",
			    COMMA, (long)major(sb->st_rdev),
			    (long)minor(sb->st_rdev)) == -1)
				return -1;
# endif
#else
			if (file_printf(ms, "%scharacter special", COMMA) == -1)
				return -1;
#endif
		}
		break;
#endif
#ifdef S_IFBLK
	case S_IFBLK:
		/*
		 * If -s has been specified, treat block special files
		 * like ordinary files.  Otherwise, just report that they
		 * are block special files and go on to the next file.
		 */
		if ((ms->flags & MAGIC_DEVICES) != 0) {
			ret = 0;
			break;
		}
		if (mime) {
			if (handle_mime(ms, mime, "blockdevice") == -1)
				return -1;
		} else if (silent) {
		} else {
#ifdef HAVE_STRUCT_STAT_ST_RDEV
# ifdef dv_unit
			if (file_printf(ms, "%sblock special (%d/%d/%d)",
			    COMMA, major(sb->st_rdev), dv_unit(sb->st_rdev),
			    dv_subunit(sb->st_rdev)) == -1)
				return -1;
# else
			if (file_printf(ms, "%sblock special (%ld/%ld)",
			    COMMA, (long)major(sb->st_rdev),
			    (long)minor(sb->st_rdev)) == -1)
				return -1;
# endif
#else
			if (file_printf(ms, "%sblock special", COMMA) == -1)
				return -1;
#endif
		}
		break;
#endif
	/* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef	S_IFIFO
	case S_IFIFO:
		if((ms->flags & MAGIC_DEVICES) != 0)
			break;
		if (mime) {
			if (handle_mime(ms, mime, "fifo") == -1)
				return -1;
		} else if (silent) {
		} else if (file_printf(ms, "%sfifo (named pipe)", COMMA) == -1)
			return -1;
		break;
#endif
#ifdef	S_IFDOOR
	case S_IFDOOR:
		if (mime) {
			if (handle_mime(ms, mime, "door") == -1)
				return -1;
		} else if (silent) {
		} else if (file_printf(ms, "%sdoor", COMMA) == -1)
			return -1;
		break;
#endif
/* Resolving a symlink needs readlink(2); without it fall through to the
 * generic handling rather than failing to link. */
#if defined(S_IFLNK) && defined(HAVE_READLINK)
	case S_IFLNK:
		if ((nch = readlink(fn, buf, BUFSIZ-1)) <= 0) {
			if (ms->flags & MAGIC_ERROR) {
			    file_error(ms, errno, "unreadable symlink `%s'",
				fn);
			    return -1;
			}
			if (mime) {
				if (handle_mime(ms, mime, "symlink") == -1)
					return -1;
			} else if (silent) {
			} else if (file_printf(ms,
			    "%sunreadable symlink `%s' (%s)", COMMA, fn,
			    strerror(errno)) == -1)
				return -1;
			break;
		}
		buf[nch] = '\0';	/* readlink(2) does not do this */

		/* If broken symlink, say so and quit early. */
#ifdef __linux__
		/*
		 * linux procfs/devfs makes symlinks like pipe:[3515864880]
		 * that we can't stat their readlink output, so stat the
		 * original filename instead.
		 */
		if (stat(fn, &tstatbuf) < 0)
			return bad_link(ms, errno, buf);
#else
		if (*buf == '/') {
			if (stat(buf, &tstatbuf) < 0)
				return bad_link(ms, errno, buf);
		} else {
			char *tmp;
			char buf2[BUFSIZ+BUFSIZ+4];

			if ((tmp = CCAST(char *, strrchr(fn,  '/'))) == NULL) {
				tmp = buf; /* in current directory anyway */
			} else {
				if (tmp - fn + 1 > BUFSIZ) {
					if (ms->flags & MAGIC_ERROR) {
						file_error(ms, 0,
						    "path too long: `%s'", buf);
						return -1;
					}
					if (mime) {
						if (handle_mime(ms, mime,
						    "x-path-too-long") == -1)
							return -1;
					} else if (silent) {
					} else if (file_printf(ms,
					    "%spath too long: `%s'", COMMA,
					    fn) == -1)
						return -1;
					break;
				}
				/* take dir part */
				(void)strlcpy(buf2, fn, sizeof buf2);
				buf2[tmp - fn + 1] = '\0';
				/* plus (rel) link */
				(void)strlcat(buf2, buf, sizeof buf2);
				tmp = buf2;
			}
			if (stat(tmp, &tstatbuf) < 0)
				return bad_link(ms, errno, buf);
		}
#endif

		/* Otherwise, handle it. */
		if ((ms->flags & MAGIC_SYMLINK) != 0) {
			const char *p;
			ms->flags &= MAGIC_SYMLINK;
			p = magic_file(ms, buf);
			ms->flags |= MAGIC_SYMLINK;
			if (p == NULL)
				return -1;
		} else { /* just print what it points to */
			if (mime) {
				if (handle_mime(ms, mime, "symlink") == -1)
					return -1;
			} else if (silent) {
			} else if (file_printf(ms, "%ssymbolic link to %s",
			    COMMA, buf) == -1)
				return -1;
		}
		break;
#endif
#ifdef	S_IFSOCK
#ifndef __COHERENT__
	case S_IFSOCK:
		if (mime) {
			if (handle_mime(ms, mime, "socket") == -1)
				return -1;
		} else if (silent) {
		} else if (file_printf(ms, "%ssocket", COMMA) == -1)
			return -1;
		break;
#endif
#endif
	case S_IFREG:
		/*
		 * regular file, check next possibility
		 *
		 * If stat() tells us the file has zero length, report here that
		 * the file is empty, so we can skip all the work of opening and
		 * reading the file.
		 * But if the -s option has been given, we skip this
		 * optimization, since on some systems, stat() reports zero
		 * size for raw disk partitions. (If the block special device
		 * really has zero length, the fact that it is empty will be
		 * detected and reported correctly when we read the file.)
		 */
		if ((ms->flags & MAGIC_DEVICES) == 0 && sb->st_size == 0) {
			if (mime) {
				if (handle_mime(ms, mime, "x-empty") == -1)
					return -1;
			} else if (silent) {
			} else if (file_printf(ms, "%sempty", COMMA) == -1)
				return -1;
			break;
		}
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
#ifdef SF_SNAPSHOT
		if ((ms->flags & MAGIC_DEVICES) == 0 &&
		    (sb->st_flags & SF_SNAPSHOT) != 0) {
			char tbuf[256];

			if (mime) {
				if (handle_mime(ms, mime, "x-fs-snapshot")
				    == -1)
					return -1;
			} else if (silent) {
			} else if (file_printf(ms,
				"%sinternal file system snapshot taken at %s",
				COMMA,
				file_fmtdatetime(tbuf, sizeof(tbuf),
				    sb->st_mtime, 0))
			    == -1)
				return -1;
			break;
		}
#endif
#endif
		ret = 0;
		break;

	default:
		file_error(ms, 0, "invalid mode 0%o", sb->st_mode);
		return -1;
		/*NOTREACHED*/
	}

	if (!silent && !mime && *did && ret == 0) {
	    if (file_printf(ms, " ") == -1)
		    return -1;
	}
	/*
	 * If we were looking for extensions or apple (silent) it is not our
	 * job to print here, so don't count this as a match.
	 */
	if (ret == 1 && silent)
		return 0;
	return ret;
}
