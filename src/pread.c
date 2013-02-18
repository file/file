#include "file.h"
#ifndef lint
FILE_RCSID("@(#)$File: ctime_r.c,v 1.1 2012/05/15 17:14:36 christos Exp $")
#endif  /* lint */
#include <fcntl.h>
#include <unistd.h>

ssize_t
pread(int fd, void *buf, ssize_t len, off_t off) {
	if (lseek(fd, off, SEEK_SET) == (off_t)-1)
		return -1;

	return read(fd, buf, len);
}
