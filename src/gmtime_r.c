/*	$NetBSD$	*/

/*	$File: gmtime_r.c,v 1.3 2022/09/24 20:22:21 christos Exp $	*/

#include "file.h"
#ifndef	lint
#if 0
FILE_RCSID("@(#)$File: gmtime_r.c,v 1.3 2022/09/24 20:22:21 christos Exp $")
#else
__RCSID("$NetBSD$");
#endif
#endif	/* lint */
#include <time.h>
#include <string.h>

/* asctime_r is not thread-safe anyway */
struct tm *
gmtime_r(const time_t *t, struct tm *tm)
{
	struct tm *tmp = gmtime(t);
	if (tmp == NULL)
		return NULL;
	memcpy(tm, tmp, sizeof(*tm));
	return tmp;
}
