#include "magic.h"
#include "file.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/*
 * Like printf, only we print to a buffer and advance it.
 */
protected int
file_printf(struct magic_set *ms, const char *fmt, ...)
{
	va_list ap;
	int len;
	char *buf;

	va_start(ap, fmt);

	if ((len = vsnprintf(ms->o.ptr, ms->o.len, fmt, ap)) >= ms->o.len) {
		va_end(ap);
		if ((buf = realloc(ms->o.buf, len + 1024)) == NULL) {
			file_oomem(ms);
			return -1;
		}
		ms->o.ptr = buf + (ms->o.ptr - ms->o.buf);
		ms->o.buf = buf;
		ms->o.len = ms->o.size - (ms->o.ptr - ms->o.buf);
		ms->o.size = len + 1024;

		va_start(ap, fmt);
		len = vsnprintf(ms->o.ptr, ms->o.len, fmt, ap);
	}
	ms->o.ptr += len;
	ms->o.len -= len;
	va_end(ap);
	return 0;
}

/*
 * error - print best error message possible
 */
/*VARARGS*/
protected void
#ifdef __STDC__
file_error(struct magic_set *ms, const char *f, ...)
#else
error(va_alist)
	va_dcl
#endif
{
	va_list va;
#ifdef __STDC__
	va_start(va, f);
#else
	const char *f;
	va_start(va);
	f = va_arg(va, const char *);
#endif
	/* cuz we use stdout for most, stderr here */
	(void) fflush(stdout); 

	(void) vsnprintf(ms->o.buf, ms->o.size, f, va);
	ms->haderr++;
	va_end(va);
}


protected void
file_oomem(struct magic_set *ms)
{
	file_error(ms, "%s", strerror(errno));
}

protected void
file_badseek(struct magic_set *ms)
{
	file_error(ms, "Error seeking (%s)", strerror(errno));
}

protected void
file_badread(struct magic_set *ms)
{
	file_error(ms, "Error reading (%s)", strerror(errno));
}

protected int
file_buf(struct magic_set *ms, const void *buf, size_t nb)
{
    int m;
    /* try compression stuff */
    if ((m = file_zmagic(ms, buf, nb)) == 0) {
	/* try tests in /etc/magic (or surrogate magic file) */
	if ((m = file_softmagic(ms, buf, nb)) == 0) {
	    /* try known keywords, check whether it is ASCII */
	    if ((m = file_ascmagic(ms, buf, nb)) == 0) {
		/* abandon hope, all ye who remain here */
		if (file_printf(ms, ms->flags & MAGIC_MIME ?
		    "application/octet-stream" : "data") == -1)
			return -1;
		m = 1;
	    }
	}
    }
    return m;
}

protected int
file_reset(struct magic_set *ms)
{
	if (ms->mlist == NULL) {
		file_error(ms, "No magic files loaded");
		return -1;
	}
	ms->o.ptr = ms->o.buf;
	ms->haderr = 0;
	return 0;
}
