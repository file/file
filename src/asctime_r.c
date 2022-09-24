/*	$NetBSD$	*/

/*	$File: asctime_r.c,v 1.2 2022/09/24 20:22:21 christos Exp $	*/

#include "file.h"
#ifndef	lint
#if 0
FILE_RCSID("@(#)$File: asctime_r.c,v 1.2 2022/09/24 20:22:21 christos Exp $")
#else
__RCSID("$NetBSD$");
#endif
#endif	/* lint */
#include <time.h>
#include <string.h>

/* asctime_r is not thread-safe anyway */
char *
asctime_r(const struct tm *t, char *dst)
{
	char *p = asctime(t);
	if (p == NULL)
		return NULL;
	memcpy(dst, p, 26);
	return dst;
}
