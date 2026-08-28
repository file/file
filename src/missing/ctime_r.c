/*	$File: ctime_r.c,v 1.1 2026/08/28 13:35:35 christos Exp $	*/

#include "file.h"
#ifndef	lint
FILE_RCSID("@(#)$File: ctime_r.c,v 1.1 2026/08/28 13:35:35 christos Exp $")
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
