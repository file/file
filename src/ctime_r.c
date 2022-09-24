/*	$NetBSD$	*/

/*	$File: ctime_r.c,v 1.2 2022/09/24 20:22:21 christos Exp $	*/

#include "file.h"
#ifndef	lint
#if 0
FILE_RCSID("@(#)$File: ctime_r.c,v 1.2 2022/09/24 20:22:21 christos Exp $")
#else
__RCSID("$NetBSD$");
#endif
#endif	/* lint */
#include <time.h>
#include <string.h>

/* ctime_r is not thread-safe anyway */
char *
ctime_r(const time_t *t, char *dst)
{
	char *p = ctime(t);
	if (p == NULL)
		return NULL;
	memcpy(dst, p, 26);
	return dst;
}
