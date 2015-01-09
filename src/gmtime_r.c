/*	$File: asctime_r.c,v 1.1 2012/05/15 17:14:36 christos Exp $	*/

#include "file.h"
#ifndef	lint
FILE_RCSID("@(#)$File: asctime_r.c,v 1.1 2012/05/15 17:14:36 christos Exp $")
#endif	/* lint */
#include <time.h>
#include <string.h>

/* asctime_r is not thread-safe anyway */
struct tm *
gmtime_r(const time_t t, struct tm *tm)
{
	struct tm *tmp = gmtime(t);
	if (tmp == NULL)
		return NULL;
	memcpy(tm, tmp, sizeof(*tm));
	return tmp;
}
