/*-
 * Copyright (c) 2008 Christos Zoulas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: readcdf.c,v 1.2 2008/10/12 17:07:14 christos Exp $")
#endif

#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "cdf.h"
#include "magic.h"

#define NOTMIME(ms) (((ms)->flags & MAGIC_MIME) == 0)

private int
cdf_file_section_info(struct magic_set *ms, const cdf_stream_t *sst,
    uint32_t offs)
{
	const cdf_section_header_t *shp;
	cdf_section_header_t sh;
	const uint32_t *p, *q, *e;
	const char *s;
	size_t i, len;
	uint32_t u32;
	int32_t s32;
	int16_t s16;
	cdf_timestamp_t tp;
	struct timespec ts;
	char buf[64];
	const char *str = "vnd.ms-office";

	shp = (const void *)((const char *)sst->sst_tab + offs);
	sh.sh_len = CDF_TOLE4(shp->sh_len);
	sh.sh_properties = CDF_TOLE4(shp->sh_properties);
	p = (const void *)((const char *)sst->sst_tab + offs + sizeof(sh));
	q = p + (sh.sh_properties << 1);
	e = (const void *)(((const char *)shp) + sh.sh_len);
	for (i = 0; i < sh.sh_properties; i++) {
		cdf_print_property_name(buf, sizeof(buf), CDF_TOLE4(p[i << 1]));
		switch (CDF_TOLE4(q[0])) {
		case CDF_SIGNED16:
			(void)memcpy(&s16, &q[1], sizeof(s16));
			s16 = CDF_TOLE2(s16);
			if (NOTMIME(ms) && file_printf(ms, ", %s: %hd", buf,
			    s16) == -1)
				return -1;
			len = 2;
			break;
		case CDF_SIGNED32:
			(void)memcpy(&s32, &q[1], sizeof(s32));
			s32 = CDF_TOLE4(s32);
			if (NOTMIME(ms) && file_printf(ms, ", %s: %d", buf, s32)
			    == -1)
				return -1;
			len = 4;
			break;
		case CDF_UNSIGNED32:
			(void)memcpy(&u32, &q[1], sizeof(u32));
			u32 = CDF_TOLE4(u32);
			if (NOTMIME(ms) && file_printf(ms, ", %s: %u", buf, u32)
			    == -1)
				return -1;
			len = 4;
			break;
		case CDF_LENGTH32_STRING:
			if (q[1] > 1) {
				s = (const char *)(&q[2]);
				if (NOTMIME(ms)) {
					if (file_printf(ms, ", %s: %.*s", buf,
					    CDF_TOLE4(q[1]), s) == -1)
						return -1;
				} else if (CDF_TOLE4(p[i << 1]) == 
					CDF_PROPERTY_NAME_OF_APPLICATION) {
					if (strstr(s, "Word"))
						str = "msword";
					else if (strstr(s, "Excel"))
						str = "vnd.ms-excel";
					else if (strstr(s, "Powerpoint"))
						str = "vnd.ms-powerpoint";
				}
			}
			len = 4 + q[1];
			break;
		case CDF_FILETIME:
			(void)memcpy(&tp, &q[1], sizeof(tp));
			tp = CDF_TOLE8(tp);
			if (tp != 0) {
				if (tp < 1000000000000000LL) {
					char tbuf[64];
					cdf_print_elapsed_time(tbuf,
					    sizeof(tbuf), tp);
					if (NOTMIME(ms) && file_printf(ms,
					    ", %s: %s", buf, tbuf) == -1)
						return -1;
				} else {
					char *c, *ec;
					cdf_timestamp_to_timespec(&ts, tp);
					c = ctime(&ts.tv_sec);
					if ((ec = strchr(c, '\n')) != NULL)
						*ec = '\0';

					if (NOTMIME(ms) && file_printf(ms,
					    ", %s: %s", buf, c) == -1)
						return -1;
				}
			}
			len = 8;
			break;
		case CDF_CLIPBOARD:
			len = 4 + CDF_TOLE4(q[1]);
			break;
		default:
			len = 4;
			file_error(ms, 0, "Internal parsing error");
			return -1;
		}
		q++;
		q = (const void *)(((const char *)q) +
		    CDF_ROUND(len, sizeof(*q)));
		if (q > e) {
			file_error(ms, 0, "Internal parsing error");
			return -1;
		}
	}
	if (!NOTMIME(ms)) {
		if (file_printf(ms, "application/%s", str) == -1)
			return -1;
	}
	return 0;
}

private int
cdf_file_summary_info(struct magic_set *ms, const cdf_stream_t *sst)
{
	size_t i;
	const cdf_summary_info_header_t *si = sst->sst_tab;
	const cdf_section_declaration_t *sd = (const void *)
	    ((const char *)sst->sst_tab + CDF_SECTION_DECLARATION_OFFSET);

	if (CDF_TOLE2(si->si_byte_order) != 0xfffe)
		return 0;

	if (NOTMIME(ms) && file_printf(ms, "CDF V2 Document") == -1)
		return -1;

	if (NOTMIME(ms) && file_printf(ms, ", %s Endian",
	    CDF_TOLE4(si->si_byte_order) == 0xfffe ?  "Little" : "Big") == -1)
		return -1;

	if (NOTMIME(ms) && file_printf(ms, ", Os Version: %d.%d",
	    si->si_os_version & 0xff, si->si_os_version >> 8) == -1)
		return -1;

	if (NOTMIME(ms) && file_printf(ms, ", Os: %d", si->si_os) == -1)
		return -1;

	for (i = 0; i < si->si_count; i++)
		if (cdf_file_section_info(ms, sst, CDF_TOLE4(sd->sd_offset))
		    == -1)
			return -1;
	return 1;
}

protected int
file_trycdf(struct magic_set *ms, int fd, const unsigned char *buf,
    size_t nbytes)
{
	cdf_header_t h;
	cdf_sat_t sat, ssat;
	cdf_stream_t sst;
	cdf_dir_t dir;
	int i;
	(void)&nbytes;
	(void)&buf;

	if (cdf_read_header(fd, &h) == -1)
		return 0;
#ifdef CDF_DEBUG
	cdf_dump_header(&h);
#endif

	if (cdf_read_sat(fd, &h, &sat) == -1) {
		file_error(ms, errno, "Can't read SAT");
		return -1;
	}
#ifdef CDF_DEBUG
	cdf_dump_sat("SAT", &h, &sat);
#endif

	if (cdf_read_ssat(fd, &h, &sat, &ssat) == -1) {
		file_error(ms, errno, "Can't read SAT");
		free(sat.sat_tab);
		return -1;
	}
#ifdef CDF_DEBUG
	cdf_dump_sat("SSAT", &h, &ssat);
#endif

	if (cdf_read_dir(fd, &h, &sat, &dir) == -1) {
		file_error(ms, errno, "Can't read directory");
		free(sat.sat_tab);
		free(ssat.sat_tab);
		return -1;
	}
#ifdef CDF_DEBUG
	cdf_dump_dir(fd, &h, &sat, &dir);
#endif

	if (cdf_read_summary_info(fd, &h, &sat, &dir, &sst) == -1) {
		file_error(ms, errno, "Can't read summary_info");
		free(sat.sat_tab);
		free(ssat.sat_tab);
		free(dir.dir_tab);
		return -1;
	}
#ifdef CDF_DEBUG
	cdf_dump_summary_info(&h, &sst);
#endif
	i = cdf_file_summary_info(ms, &sst);
	free(sat.sat_tab);
	free(ssat.sat_tab);
	free(dir.dir_tab);
	free(sst.sst_tab);
	return i;
}
