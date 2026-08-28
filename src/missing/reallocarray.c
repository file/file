/*
 * Public domain fallback for reallocarray(3), for platforms (e.g.
 * AmigaOS/libnix) whose C library declares it but does not implement it.
 */
#include "file.h"
#include <stdlib.h>
#include <errno.h>

void *
reallocarray(void *optr, size_t nmemb, size_t size)
{
	if (size != 0 && nmemb > (size_t)-1 / size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, nmemb * size);
}
