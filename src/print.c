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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Ian F. Darwin and others.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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
 * print.c - debugging printout routines
 */

#include "file.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>

#ifndef lint
FILE_RCSID("@(#)$Id: print.c,v 1.41 2003/03/23 21:16:26 christos Exp $")
#endif  /* lint */

#define SZOF(a)	(sizeof(a) / sizeof(a[0]))

#ifndef COMPILE_ONLY
protected void
file_mdump(struct magic *m)
{
	private const char *typ[] = { "invalid", "byte", "short", "invalid",
				     "long", "string", "date", "beshort",
				     "belong", "bedate", "leshort", "lelong",
				     "ledate", "pstring", "ldate", "beldate",
				     "leldate", "regex" };
	private const char optyp[] = { '@', '&', '|', '^', '+', '-', 
				      '*', '/', '%' };
	(void) fputc('[', stderr);
	(void) fprintf(stderr, ">>>>>>>> %d" + 8 - (m->cont_level & 7),
		       m->offset);

	if (m->flag & INDIR) {
		(void) fprintf(stderr, "(%s,",
			       /* Note: type is unsigned */
			       (m->in_type < SZOF(typ)) ? 
					typ[m->in_type] : "*bad*");
		if (m->in_op & OPINVERSE)
			(void) fputc('~', stderr);
		(void) fprintf(stderr, "%c%d),",
			       ((m->in_op&0x7F) < SZOF(optyp)) ? 
					optyp[m->in_op&0x7F] : '?',
				m->in_offset);
	}
	(void) fprintf(stderr, " %s%s", (m->flag & UNSIGNED) ? "u" : "",
		       /* Note: type is unsigned */
		       (m->type < SZOF(typ)) ? typ[m->type] : "*bad*");
	if (m->mask_op & OPINVERSE)
		(void) fputc('~', stderr);
	if (m->mask) {
		((m->mask_op&0x7F) < SZOF(optyp)) ? 
			(void) fputc(optyp[m->mask_op&0x7F], stderr) :
			(void) fputc('?', stderr);
		if(STRING != m->type || PSTRING != m->type)
			(void) fprintf(stderr, "%.8x", m->mask);
		else {
			if (m->mask & STRING_IGNORE_LOWERCASE) 
				(void) fputc(CHAR_IGNORE_LOWERCASE, stderr);
			if (m->mask & STRING_COMPACT_BLANK) 
				(void) fputc(CHAR_COMPACT_BLANK, stderr);
			if (m->mask & STRING_COMPACT_OPTIONAL_BLANK) 
				(void) fputc(CHAR_COMPACT_OPTIONAL_BLANK,
				stderr);
		}
	}

	(void) fprintf(stderr, ",%c", m->reln);

	if (m->reln != 'x') {
		switch (m->type) {
		case BYTE:
		case SHORT:
		case LONG:
		case LESHORT:
		case LELONG:
		case BESHORT:
		case BELONG:
			(void) fprintf(stderr, "%d", m->value.l);
			break;
		case STRING:
		case PSTRING:
		case REGEX:
			file_showstr(stderr, m->value.s, -1);
			break;
		case DATE:
		case LEDATE:
		case BEDATE:
			(void)fprintf(stderr, "%s,",
			    file_fmttime(m->value.l, 1));
			break;
		case LDATE:
		case LELDATE:
		case BELDATE:
			(void)fprintf(stderr, "%s,",
			    file_fmttime(m->value.l, 0));
			break;
		default:
			(void) fputs("*bad*", stderr);
			break;
		}
	}
	(void) fprintf(stderr, ",\"%s\"]\n", m->desc);
}
#endif

/*VARARGS*/
protected void
file_magwarn(const char *f, ...)
{
	va_list va;
	va_start(va, f);

	/* cuz we use stdout for most, stderr here */
	(void) fflush(stdout); 

	(void) vfprintf(stderr, f, va);
	va_end(va);
	fputc('\n', stderr);
}

protected char *
file_fmttime(long v, int local)
{
	char *pp, *rt;
	time_t t = (time_t)v;
	struct tm *tm;

	if (local) {
		pp = ctime(&t);
	} else {
#ifndef HAVE_DAYLIGHT
		private int daylight = 0;
#ifdef HAVE_TM_ISDST
		private time_t now = (time_t)0;

		if (now == (time_t)0) {
			struct tm *tm1;
			(void)time(&now);
			tm1 = localtime(&now);
			daylight = tm1->tm_isdst;
		}
#endif /* HAVE_TM_ISDST */
#endif /* HAVE_DAYLIGHT */
		if (daylight)
			t += 3600;
		tm = gmtime(&t);
		pp = asctime(tm);
	}

	if ((rt = strchr(pp, '\n')) != NULL)
		*rt = '\0';
	return pp;
}
