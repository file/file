
#include "magic.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>	/* for MAXPATHLEN */
#include <sys/stat.h>
#include <fcntl.h>	/* for open() */
#ifdef QUICK
#include <sys/mman.h>
#endif
#ifdef RESTORE_TIME
# if (__COHERENT__ >= 0x420)
#  include <sys/utime.h>
# else
#  ifdef USE_UTIMES
#   include <sys/time.h>
#  else
#   include <utime.h>
#  endif
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* for read() */
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <netinet/in.h>		/* for byte swapping */

#include "patchlevel.h"

#ifndef	lint
FILE_RCSID("@(#)$Id: magic.c,v 1.4 2003/03/24 01:16:28 christos Exp $")
#endif	/* lint */


#ifndef MAGIC
# define MAGIC "/etc/magic"
#endif

private void free_mlist(struct mlist *);

public struct magic_set *
magic_open(int flags)
{
	struct magic_set *ms;

	if ((ms = malloc(sizeof(struct magic_set))) == NULL)
		return NULL;
	ms->o.ptr = ms->o.buf = malloc(ms->o.size = 1024);
	ms->o.len = 0;
	if (ms->o.buf == NULL) {
		free(ms);
		return NULL;
	}
	ms->c.off = malloc((ms->c.len = 10) * sizeof(*ms->c.off));
	if (ms->c.off == NULL) {
		free(ms->o.buf);
		free(ms);
		return NULL;
	}
	ms->flags = flags;
	ms->haderr = 0;
	return ms;
}

/*
 * load a magic file
 */
public int
magic_load(struct magic_set *ms, const char *magicfile)
{
	struct mlist *ml;

	if (magicfile == NULL)
		magicfile = (ms->flags & MAGIC_MIME) ? MAGIC ".mime" : MAGIC;

	ml = file_apprentice(ms, magicfile, 0);
	if (ml == NULL) 
		return -1;
	ms->mlist = ml;
	return 0;
}

private void
free_mlist(struct mlist *mlist)
{
	struct mlist *ml;

	if (mlist == NULL)
		return;

	for (ml = mlist->next; ml != mlist;) {
		struct mlist *next = ml->next;
		struct magic *mg = ml->magic;
		switch (ml->mapped) {
		case 0:
			free(mg);
			break;
		case 1:
			mg--;
			free(mg);
			break;
		case 2:
			mg--;
			(void)munmap(mg, sizeof(*mg) * (ml->nmagic + 1));
			break;
		}
		free(ml);
		ml = next;
	}
	free(ml);
}

public void
magic_close(ms)
    struct magic_set *ms;
{
	free_mlist(ms->mlist);
	free(ms->o.buf);
	free(ms->c.off);
	free(ms);
}

public int
magic_compile(struct magic_set *ms, const char *magicfile)
{
	struct mlist *ml = file_apprentice(ms, magicfile, FILE_COMPILE);
	if(ml == NULL)
		return -1;
	free_mlist(ml);
	return ml ? 0 : -1;
}

public int
magic_check(struct magic_set *ms, const char *magicfile)
{
	struct mlist *ml = file_apprentice(ms, magicfile, FILE_CHECK);
	if(ml == NULL)
		return -1;
	free_mlist(ml);
	return ml ? 0 : -1;
}

/*
 * find type of named file
 */
public const char *
magic_file(struct magic_set *ms, const char *inname)
{
	int	fd = 0;
	unsigned char buf[HOWMANY+1];	/* one extra for terminating '\0' */
	struct stat	sb;
	int nbytes = 0;	/* number of bytes read from a datafile */

	if (file_reset(ms) == -1)
		return NULL;

	switch (file_fsmagic(ms, inname, &sb)) {
	case -1:
		return NULL;
	case 0:
		break;
	default:
		return ms->o.buf;
	}

	if ((fd = open(inname, O_RDONLY)) < 0) {
		/* We can't open it, but we were able to stat it. */
		if (sb.st_mode & 0002)
			if (file_printf(ms, "writeable, ") == -1)
				return NULL;
		if (sb.st_mode & 0111)
			if (file_printf(ms, "executable, ") == -1)
				return NULL;
		return ms->o.buf;
	}

	/*
	 * try looking at the first HOWMANY bytes
	 */
	if ((nbytes = read(fd, (char *)buf, HOWMANY)) == -1) {
		file_error(ms, "Cannot read `%s' %s", inname, strerror(errno));
		return NULL;
	}

	if (nbytes == 0){
		if (file_printf(ms, (ms->flags & MAGIC_MIME) ?
		    "application/x-empty" : "empty") == -1)
			return NULL;
	} else {
		buf[nbytes++] = '\0';	/* null-terminate it */
		if (file_buffer(ms, buf, nbytes) == -1)
			return NULL;
#ifdef BUILTIN_ELF
		if (nbytes > 5) {
			/*
			 * We matched something in the file, so this *might*
			 * be an ELF file, and the file is at least 5 bytes
			 * long, so if it's an ELF file it has at least one
			 * byte past the ELF magic number - try extracting
			 * information from the ELF headers that can't easily
			 * be extracted with rules in the magic file.
			 */
			file_tryelf(ms, fd, buf, nbytes);
		}
#endif
	}

	return ms->haderr ? NULL : ms->o.buf;
}


public const char *
magic_buf(struct magic_set *ms, const void *buf, size_t nb)
{
	if (file_reset(ms) == -1)
		return NULL;
	/*
	 * The main work is done here!
	 * We have the file name and/or the data buffer to be identified. 
	 */
	if (file_buffer(ms, buf, nb) == -1) {
		return NULL;
	}
	return ms->haderr ? NULL : ms->o.buf;
}

public const char *
magic_error(struct magic_set *ms)
{
	return ms->haderr ? ms->o.buf : NULL;
}

public void
magic_setflags(struct magic_set *ms, int flags)
{
	ms->flags = flags;
}
