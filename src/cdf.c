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

#include <assert.h>
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

#ifndef __arraycount
#define __arraycount(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifdef CDF_DEBUG
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

static union {
	char s[4];
	uint32_t u;
} cdf_bo;

#define NEED_SWAP	(cdf_bo.u == (uint32_t)0x01020304)

#undef CDF_TOLE8
#define CDF_TOLE8(x)	(NEED_SWAP ? cdf_tole8(x) : (uint64_t)(x))
#undef CDF_TOLE4
#define CDF_TOLE4(x)	(NEED_SWAP ? cdf_tole4(x) : (uint32_t)(x))
#undef CDF_TOLE2
#define CDF_TOLE2(x)	(NEED_SWAP ? cdf_tole2(x) : (uint16_t)(x))

/*
 * swap a short
 */
uint16_t
cdf_tole2(uint16_t sv)
{
	uint16_t rv;
	uint8_t *s = (uint8_t *)(void *)&sv; 
	uint8_t *d = (uint8_t *)(void *)&rv; 
	d[0] = s[1];
	d[1] = s[0];
	return rv;
}

/*
 * swap an int
 */
uint32_t
cdf_tole4(uint32_t sv)
{
	uint32_t rv;
	uint8_t *s = (uint8_t *)(void *)&sv; 
	uint8_t *d = (uint8_t *)(void *)&rv; 
	d[0] = s[3];
	d[1] = s[2];
	d[2] = s[1];
	d[3] = s[0];
	return rv;
}

/*
 * swap a quad
 */
uint64_t
cdf_tole8(uint64_t sv)
{
	uint64_t rv;
	uint8_t *s = (uint8_t *)(void *)&sv; 
	uint8_t *d = (uint8_t *)(void *)&rv; 
	d[0] = s[7];
	d[1] = s[6];
	d[2] = s[5];
	d[3] = s[4];
	d[4] = s[3];
	d[5] = s[2];
	d[6] = s[1];
	d[7] = s[0];
	return rv;
}

int
cdf_need_swap(void)
{
	return NEED_SWAP;
}

#define CDF_UNPACK(a)	\
    (void)memcpy(&(a), &buf[len], sizeof(a)), len += sizeof(a)
#define CDF_UNPACKA(a)	\
    (void)memcpy((a), &buf[len], sizeof(a)), len += sizeof(a)

void
cdf_swap_header(cdf_header_t *h)
{
	size_t i;

	h->h_magic = CDF_TOLE8(h->h_magic);
	h->h_uuid[0] = CDF_TOLE8(h->h_uuid[0]);
	h->h_uuid[1] = CDF_TOLE8(h->h_uuid[1]);
	h->h_revision = CDF_TOLE2(h->h_revision);
	h->h_version = CDF_TOLE2(h->h_version);
	h->h_byte_order = CDF_TOLE2(h->h_byte_order);
	h->h_sec_size_p2 = CDF_TOLE2(h->h_sec_size_p2);
	h->h_short_sec_size_p2 = CDF_TOLE2(h->h_short_sec_size_p2);
	h->h_num_sectors_in_sat = CDF_TOLE4(h->h_num_sectors_in_sat);
	h->h_secid_first_directory = CDF_TOLE4(h->h_secid_first_directory);
	h->h_min_size_standard_stream = CDF_TOLE4(h->h_min_size_standard_stream);
	h->h_secid_first_sector_in_short_sat =
	    CDF_TOLE4(h->h_secid_first_sector_in_short_sat);
	h->h_num_sectors_in_short_sat = CDF_TOLE4(h->h_num_sectors_in_short_sat);
	h->h_secid_first_sector_in_master_sat =
	    CDF_TOLE4(h->h_secid_first_sector_in_master_sat);
	h->h_num_sectors_in_master_sat =
	    CDF_TOLE4(h->h_num_sectors_in_master_sat);
	for (i = 0; i < __arraycount(h->h_master_sat); i++)
		h->h_master_sat[i] = CDF_TOLE4(h->h_master_sat[i]);
}

void
cdf_unpack_header(cdf_header_t *h, char *buf)
{
	size_t i;
	size_t len = 0;

	CDF_UNPACK(h->h_magic);
	CDF_UNPACKA(h->h_uuid);
	CDF_UNPACK(h->h_revision);
	CDF_UNPACK(h->h_version);
	CDF_UNPACK(h->h_byte_order);
	CDF_UNPACK(h->h_sec_size_p2);
	CDF_UNPACK(h->h_short_sec_size_p2);
	CDF_UNPACKA(h->h_unused0);
	CDF_UNPACK(h->h_num_sectors_in_sat);
	CDF_UNPACK(h->h_secid_first_directory);
	CDF_UNPACKA(h->h_unused1);
	CDF_UNPACK(h->h_min_size_standard_stream);
	CDF_UNPACK(h->h_secid_first_sector_in_short_sat);
	CDF_UNPACK(h->h_num_sectors_in_short_sat);
	CDF_UNPACK(h->h_secid_first_sector_in_master_sat);
	CDF_UNPACK(h->h_num_sectors_in_master_sat);
	for (i = 0; i < __arraycount(h->h_master_sat); i++)
		CDF_UNPACK(h->h_master_sat[i]);
}

void
cdf_swap_dir(cdf_directory_t *d)
{
	d->d_namelen = CDF_TOLE2(d->d_namelen);
	d->d_left_child = CDF_TOLE4(d->d_left_child);
	d->d_right_child = CDF_TOLE4(d->d_right_child);
	d->d_storage = CDF_TOLE4(d->d_storage);
	d->d_storage_uuid[0] = CDF_TOLE8(d->d_storage_uuid[0]);
	d->d_storage_uuid[1] = CDF_TOLE8(d->d_storage_uuid[1]);
	d->d_flags = CDF_TOLE4(d->d_flags);
	d->d_created = CDF_TOLE8(d->d_created);
	d->d_modified = CDF_TOLE8(d->d_modified);
	d->d_stream_first_sector = CDF_TOLE4(d->d_stream_first_sector);
	d->d_size = CDF_TOLE4(d->d_size);
}

void
cdf_unpack_dir(cdf_directory_t *d, char *buf)
{
	size_t len = 0;

	CDF_UNPACKA(d->d_name);
	CDF_UNPACK(d->d_namelen);
	CDF_UNPACK(d->d_type);
	CDF_UNPACK(d->d_color);
	CDF_UNPACK(d->d_left_child);
	CDF_UNPACK(d->d_right_child);
	CDF_UNPACK(d->d_storage);
	CDF_UNPACKA(d->d_storage_uuid);
	CDF_UNPACK(d->d_flags);
	CDF_UNPACK(d->d_created);
	CDF_UNPACK(d->d_modified);
	CDF_UNPACK(d->d_stream_first_sector);
	CDF_UNPACK(d->d_size);
	CDF_UNPACK(d->d_unused0);
}
int
cdf_read_header(int fd, cdf_header_t *h)
{
	(void)memcpy(cdf_bo.s, "\01\02\03\04", 4);
	char buf[512];
	if (lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1)
		return -1;
	if (read(fd, buf, sizeof(buf)) != sizeof(buf))
		return -1;
	cdf_unpack_header(h, buf);
	cdf_swap_header(h);
	if (h->h_magic != CDF_MAGIC) {
		errno = EFTYPE;
		return -1;
	}
	return 0;
}


ssize_t
cdf_read_sector(int fd, void *buf, size_t offs, size_t len,
    const cdf_header_t *h, cdf_secid_t id)
{
	assert((size_t)CDF_SEC_SIZE(h) == len);
	if (lseek(fd, (off_t)CDF_SEC_POS(h, id), SEEK_SET) == (off_t)-1)
		return -1;
	if (read(fd, ((char *)buf) + offs, len) != (ssize_t)len)
		return -1;
	return len;
}

/*
 * Read the sector allocation table.
 */
int
cdf_read_sat(int fd, cdf_header_t *h, cdf_sat_t *sat)
{
	size_t i, j, k;
	size_t ss = CDF_SEC_SIZE(h);
	cdf_secid_t *msa, mid;

	for (i = 0; i < __arraycount(h->h_master_sat); i++)
		if (h->h_master_sat[i] == CDF_SECID_FREE)
			break;

	sat->sat_len = (h->h_num_sectors_in_master_sat + i);
	if ((sat->sat_tab = calloc(sat->sat_len, ss)) == NULL)
		return -1;

	for (i = 0; i < __arraycount(h->h_master_sat); i++) {
		if (h->h_master_sat[i] < 0)
			break;
		if (cdf_read_sector(fd, sat->sat_tab, ss * i, ss, h,
		    h->h_master_sat[i]) != (ssize_t)ss) {
			warnx("Reading sector %d", h->h_master_sat[i]);
			free(sat->sat_tab);
			return -1;
		}
	}

	if ((msa = calloc(1, ss)) == NULL)
		return -1;

	mid = h->h_secid_first_sector_in_master_sat;
	for (j = 0; j < h->h_num_sectors_in_master_sat; j++) {
		if (cdf_read_sector(fd, msa, 0, ss, h, mid) != (ssize_t)ss) {
			warnx("Reading master sector %d", mid);
			free(sat->sat_tab);
			free(msa);
			return -1;
		}
		for (k = 0; k < (ss / sizeof(mid)) - 1; k++, i++)
			if (cdf_read_sector(fd, sat->sat_tab, ss * i, ss, h,
			    msa[k]) != (ssize_t)ss) {
				warnx("Reading sector %d", msa[k]);
				free(sat->sat_tab);
				free(msa);
				return -1;
			}
		mid = CDF_TOLE4(msa[(ss / sizeof(mid)) - 1]);
	}
	free(msa);
	return 0;
}

size_t
cdf_count_chain(const cdf_header_t *h, const cdf_sat_t *sat, cdf_secid_t sid)
{
	size_t i, s = CDF_SEC_SIZE(h) / sizeof(cdf_secid_t);

	DPRINTF(("Chain:"));
	for (i = 0; sid >= 0; i++) {
		DPRINTF((" %d", sid));
		if (sid > (cdf_secid_t)(sat->sat_len * s)) {
			errno = EFTYPE;
			return (size_t)-1;
		}
		sid = CDF_TOLE4(sat->sat_tab[sid]);
	}
	DPRINTF(("\n"));
	return i;
}

int
cdf_read_stream(int fd, const cdf_header_t *h, const cdf_sat_t *sat,
    cdf_secid_t sid, size_t len, cdf_stream_t *sst)
{
	size_t ss = CDF_SEC_SIZE(h), i;

	sst->sst_len = cdf_count_chain(h, sat, sid);
	sst->sst_dirlen = len;

	if (sst->sst_len == (size_t)-1)
		return -1;

	sst->sst_tab = calloc(sst->sst_len, ss);
	if (sst->sst_tab == NULL)
		return -1;

	for (i = 0; sid >= 0; i++) {
		if (cdf_read_sector(fd, sst->sst_tab, i * ss, ss, h, sid)
		    != (ssize_t)ss) {
			warnx("Reading short stream sector %d", sid);
			free(sst->sst_tab);
			return -1;
		}
		sid = CDF_TOLE4(sat->sat_tab[sid]);
	}
	return 0;
}


int
cdf_read_dir(int fd, const cdf_header_t *h, const cdf_sat_t *sat,
    cdf_dir_t *dir)
{
	size_t i, j;
	size_t ss = CDF_SEC_SIZE(h), ns;
	char *buf;
	cdf_secid_t sid = h->h_secid_first_directory;

	ns = cdf_count_chain(h, sat, sid);
	if (ns == (size_t)-1)
		return -1;

	dir->dir_tab = calloc(ns, sizeof(dir->dir_tab[0]));
	if (dir->dir_tab == NULL)
		return -1;
	dir->dir_len = ns * ss / CDF_DIRECTORY_SIZE;

	if ((buf = malloc(ss)) == NULL) {
		free(dir->dir_tab);
		return -1;
	}

	for (i = 0; i < ns; i++) {
		if (cdf_read_sector(fd, buf, 0, ss, h, sid) != (ssize_t)ss) {
			warnx("Reading directory sector %d", sid);
			free(dir->dir_tab);
			free(buf);
			return -1;
		}
		for (j = 0; j < ss / CDF_DIRECTORY_SIZE; j++) {
			cdf_unpack_dir(&dir->dir_tab[j],
			    &buf[j * CDF_DIRECTORY_SIZE]);

		}
		sid = CDF_TOLE4(sat->sat_tab[sid]);
	}
	if (NEED_SWAP)
		for (i = 0; i < dir->dir_len; i++)
			cdf_swap_dir(&dir->dir_tab[i]);
	return 0;
}


int
cdf_read_ssat(int fd, const cdf_header_t *h, const cdf_sat_t *sat,
    cdf_sat_t *ssat)
{
	size_t i;
	size_t ss = CDF_SEC_SIZE(h);
	cdf_secid_t sid = h->h_secid_first_sector_in_short_sat;

	ssat->sat_len = cdf_count_chain(h, sat, sid);
	if (sat->sat_len == (size_t)-1)
		return -1;

	ssat->sat_tab = calloc(ssat->sat_len, ss);
	if (sat->sat_tab == NULL)
		return -1;

	for (i = 0; sid >= 0; i++) {
		if (cdf_read_sector(fd, ssat->sat_tab, i * ss, ss, h, sid) !=
		    (ssize_t)ss) {
			warnx("Reading short sat sector %d", sid);
			free(sat->sat_tab);
			return -1;
		}
		sid = CDF_TOLE4(sat->sat_tab[sid]);
	}
	return 0;
}

int
cdf_read_short_stream(int fd, const cdf_header_t *h, const cdf_sat_t *sat,
    const cdf_dir_t *dir, cdf_stream_t *sst)
{
	size_t i;
	const cdf_directory_t *d;

	for (i = 0; i < dir->dir_len; i++)
		if (dir->dir_tab[i].d_type == CDF_DIR_TYPE_ROOT_STORAGE)
			break;

	if (i == dir->dir_len) {
		errno = EFTYPE;
		return -1;
	}
	d = &dir->dir_tab[i];

	return cdf_read_stream(fd, h, sat, d->d_stream_first_sector, d->d_size,
	    sst);
}

static int
cdf_namecmp(const char *d, const uint16_t *s, size_t l)
{
	for (; l--; d++, s++)
		if (*d != *s)
			return (unsigned char)*d - *s;
	return 0;
}

int
cdf_read_summary_info(int fd, const cdf_header_t *h,
    const cdf_sat_t *sat, const cdf_dir_t *dir, cdf_stream_t *sst)
{
	size_t i;
	const cdf_directory_t *d;
	static const char name[] = "\05SummaryInformation";

	for (i = 0; i < dir->dir_len; i++)
		if (dir->dir_tab[i].d_type == CDF_DIR_TYPE_USER_STREAM &&
		    cdf_namecmp(name, dir->dir_tab[i].d_name, sizeof(name))
		    == 0)
			break;

	if (i == dir->dir_len) {
		errno = EFTYPE;
		return -1;
	}
	d = &dir->dir_tab[i];
	return cdf_read_stream(fd, h, sat, d->d_stream_first_sector, d->d_size,
	    sst);
}


int
cdf_print_classid(char *buf, size_t buflen, const cdf_classid_t *id)
{
	return snprintf(buf, buflen, "%.8x-%.4x-%.4x-%.2x%.2x-"
	    "%.2x%.2x%.2x%.2x%.2x%.2x", id->cl_dword, id->cl_word[0],
	    id->cl_word[1], id->cl_two[0], id->cl_two[1], id->cl_six[0],
	    id->cl_six[1], id->cl_six[2], id->cl_six[3], id->cl_six[4],
	    id->cl_six[5]);
}

static const struct {
	uint32_t v;
	const char *n;
} vn[] = {
	{ CDF_PROPERTY_CODE_PAGE, "Code page" },
	{ CDF_PROPERTY_TITLE, "Title" },
	{ CDF_PROPERTY_SUBJECT, "Subject" },
	{ CDF_PROPERTY_AUTHOR, "Author" },
	{ CDF_PROPERTY_KEYWORDS, "Keywords" },
	{ CDF_PROPERTY_COMMENTS, "Comments" },
	{ CDF_PROPERTY_TEMPLATE, "Template" },
	{ CDF_PROPERTY_LAST_SAVED_BY, "Last Saved By" },
	{ CDF_PROPERTY_REVISION_NUMBER, "Revision Number" },
	{ CDF_PROPERTY_TOTAL_EDITING_TIME, "Total Editing Time" },
	{ CDF_PROPERTY_LAST_PRINTED, "Last Printed" },
	{ CDF_PROPERTY_CREATE_TIME, "Create Time/Date" },
	{ CDF_PROPERTY_LAST_SAVED_TIME, "Last Saved Time/Date" },
	{ CDF_PROPERTY_NUMBER_OF_PAGES, "Number of Pages" },
	{ CDF_PROPERTY_NUMBER_OF_WORDS, "Number of Words" },
	{ CDF_PROPERTY_NUMBER_OF_CHARACTERS, "Number of Characters" },
	{ CDF_PROPERTY_THUMBNAIL, "Thumbnail" },
	{ CDF_PROPERTY_NAME_OF_APPLICATION, "Name of Creating Application" },
	{ CDF_PROPERTY_SECURITY, "Security" },
	{ CDF_PROPERTY_LOCALE_ID, "Locale ID" },
};

int
cdf_print_property_name(char *buf, size_t bufsiz, uint32_t p)
{
	size_t i;

	for (i = 0; i < __arraycount(vn); i++)
		if (vn[i].v == p)
			return snprintf(buf, bufsiz, "%s", vn[i].n);
	return snprintf(buf, bufsiz, "0x%x", p);
}

int
cdf_print_elapsed_time(char *buf, size_t bufsiz, cdf_timestamp_t ts)
{
	size_t len = 0;
	int days, hours, mins, secs;

	ts /= CDF_TIME_PREC;
	secs = ts % 60;
	ts /= 60;
	mins = ts % 60;
	ts /= 60;
	hours = ts % 24;
	ts /= 24;
	days = ts;

	if (days) {
		len += snprintf(buf + len, bufsiz - len, "%dd+", days);
		if (len >= bufsiz)
			return len;
	}

	if (days || hours) {
		len += snprintf(buf + len, bufsiz - len, "%.2d:", hours);
		if (len >= bufsiz)
			return len;
	}

	len += snprintf(buf + len, bufsiz - len, "%.2d:", mins);
	if (len >= bufsiz)
		return len;

	len += snprintf(buf + len, bufsiz - len, "%.2d", secs);
	return len;
}


#ifdef CDF_DEBUG
void
cdf_dump_header(const cdf_header_t *h)
{
	size_t i;

#define DUMP(a, b) printf("%40.40s = " a "\n", # b, h->h_ ## b)
	DUMP("%d", revision);
	DUMP("%d", version);
	DUMP("0x%x", byte_order);
	DUMP("%d", sec_size_p2);
	DUMP("%d", short_sec_size_p2);
	DUMP("%d", num_sectors_in_sat);
	DUMP("%d", secid_first_directory);
	DUMP("%d", min_size_standard_stream);
	DUMP("%d", secid_first_sector_in_short_sat);
	DUMP("%d", num_sectors_in_short_sat);
	DUMP("%d", secid_first_sector_in_master_sat);
	DUMP("%d", num_sectors_in_master_sat);
	for (i = 0; i < __arraycount(h->h_master_sat); i++) {
		if (h->h_master_sat[i] == CDF_SECID_FREE)
			break;
		printf("%35.35s[%.3zu] = %d\n",
		    "master_sat", i, h->h_master_sat[i]);
	}
}

void
cdf_dump_sat(const char *prefix, const cdf_header_t *h, const cdf_sat_t *sat)
{
	size_t i, j, s = CDF_SEC_SIZE(h) / sizeof(cdf_secid_t);

	for (i = 0; i < sat->sat_len; i++) {
		printf("%s[%zu]:\n", prefix, i);
		for (j = 0; j < s; j++) {
			printf("%5d, ", CDF_TOLE4(sat->sat_tab[s * i + j]));
			if ((j + 1) % 10 == 0)
				printf("\n");
		}
		printf("\n");
	}
}

void
cdf_dump(void *v, size_t len)
{
	size_t i;
	unsigned char *p = v;
	for (i = 0; i < len; i++, p++) {
		if (0 && *p == 0)
			continue;
		if (isprint(*p))
			printf("%c ", *p);
		else
			printf("0x%x ", *p);
	}
}

void
cdf_dump_stream(const cdf_header_t *h, const cdf_stream_t *sst)
{
	cdf_dump(sst->sst_tab, CDF_SEC_SIZE(h) * sst->sst_len);
}

void
cdf_dump_dir(int fd, const cdf_header_t *h, const cdf_sat_t *sat,
    const cdf_dir_t *dir)
{
	size_t i, j;
	cdf_directory_t *d;
	char name[__arraycount(d->d_name)];
	cdf_stream_t sst;
	struct timespec ts;

	static const char *types[] = { "empty", "user storage",
	    "user stream", "lockbytes", "property", "root storage" };

	for (i = 0; i < dir->dir_len; i++) {
		d = &dir->dir_tab[i];
		for (j = 0; j < sizeof(name); j++)
			name[j] = (char)d->d_name[j];
		printf("Directory %zu: %s\n", i, name);
		if (d->d_type < __arraycount(types))
			printf("Type: %s\n", types[d->d_type]);
		else
			printf("Type: %d\n", d->d_type);
		printf("Color: %s\n", d->d_color ? "black" : "red");
		printf("Left child: %d\n", d->d_left_child);
		printf("Right child: %d\n", d->d_right_child);
		switch (d->d_type) {
		case CDF_DIR_TYPE_USER_STORAGE:
			printf("Storage: %d\n", d->d_storage);
			break;
		case CDF_DIR_TYPE_USER_STREAM:
			if (cdf_read_stream(fd, h, sat,
			    d->d_stream_first_sector, d->d_size, &sst) == -1) {
				warn("Can't read stream");
				break;
			}
			cdf_dump_stream(h, &sst);
			free(sst.sst_tab);
			break;
		default:
			break;
		}
			
		printf("Flags: 0x%x\n", d->d_flags);
		cdf_timestamp_to_timespec(&ts, d->d_created);
		printf("Created %s", ctime(&ts.tv_sec));
		cdf_timestamp_to_timespec(&ts, d->d_modified);
		printf("Modified %s", ctime(&ts.tv_sec));
		printf("Stream %d\n", d->d_stream_first_sector);
		printf("Size %d\n", d->d_size);
	}
}

void
cdf_dump_section_info(const cdf_stream_t *sst, uint32_t offs)
{
	const cdf_section_header_t *shp;
	cdf_section_header_t sh;
	const uint32_t *p, *q, *e;
	size_t i, len;
	uint32_t u32;
	int32_t s32;
	int16_t s16;
	cdf_timestamp_t tp;
	struct timespec ts;
	char buf[64];

	shp = (const void *)((const char *)sst->sst_tab + offs);
	sh.sh_len = CDF_TOLE4(shp->sh_len);
	sh.sh_properties = CDF_TOLE4(shp->sh_properties);
	printf("Length %d, Properties %d\n", sh.sh_len, sh.sh_properties);
	p = (const void *)((const char *)sst->sst_tab + offs + sizeof(sh));
	q = p + (sh.sh_properties << 1);
	e = (void *)(((char *)shp) + sh.sh_len);
	for (i = 0; i < sh.sh_properties; i++) {
		cdf_print_property_name(buf, sizeof(buf), CDF_TOLE4(p[i << 1]));
		printf("%zu) %s: ", i, buf); 
		switch (CDF_TOLE4(q[0])) {
		case CDF_SIGNED16:
			(void)memcpy(&s16, &q[1], sizeof(s16));
			printf("signed 16 [%hd]\n", CDF_TOLE2(s16));
			len = 2;
			break;
		case CDF_SIGNED32:
			(void)memcpy(&s32, &q[1], sizeof(s32));
			printf("signed 32 [%d]\n", CDF_TOLE4(s32));
			len = 4;
			break;
		case CDF_UNSIGNED32:
			(void)memcpy(&u32, &q[1], sizeof(u32));
			printf("unsigned 32 [%u]\n", CDF_TOLE4(u32));
			len = 4;
			break;
		case CDF_LENGTH32_STRING:
			printf("string %u [%.*s]\n", CDF_TOLE4(q[1]), CDF_TOLE4(q[1]),
			    (const char *)(&q[2]));
			len = 4 + CDF_TOLE4(q[1]);
			break;
		case CDF_FILETIME:
			(void)memcpy(&tp, &q[1], sizeof(tp));
			tp = CDF_TOLE8(tp);
			if (tp < 1000000000000000LL) {
				cdf_print_elapsed_time(buf, sizeof(buf), tp);
				printf("timestamp %s\n", buf);
			} else {
				cdf_timestamp_to_timespec(&ts, tp);
				printf("timestamp %s", ctime(&ts.tv_sec));
			}
			len = 8;
			break;
		case CDF_CLIPBOARD:
			printf("\n");
			len = 4 + CDF_TOLE4(q[1]);
			break;
		default:
			len = 4;
			DPRINTF(("Don't know how to deal with %x\n",
			    CDF_TOLE4(q[0])));
			break;
		}
		q++;
		q = (void *)(((char *)q) + CDF_ROUND(len, sizeof(*q)));
		if (q > e) {
			DPRINTF(("Ran of the end %p > %p\n", q, e));
			return;
		}
	}
}



void
cdf_dump_summary_info(const cdf_header_t *h, const cdf_stream_t *sst)
{
	char buf[128];
	size_t i;
	const cdf_summary_info_header_t *si = sst->sst_tab;
	const cdf_section_declaration_t *sd = (const void *)
	    ((const char *)sst->sst_tab + CDF_SECTION_DECLARATION_OFFSET);

	printf("Endian: %x\n", si->si_byte_order);
	printf("Os Version %d.%d\n", si->si_os_version & 0xff,
		si->si_os_version >> 8);
	printf("Os %d\n", si->si_os);
	cdf_print_classid(buf, sizeof(buf), &si->si_class);
	printf("Class %s\n", buf);
	printf("Count %d\n", CDF_TOLE4(si->si_count));
	for (i = 0; i < CDF_TOLE4(si->si_count); i++) {
		cdf_print_classid(buf, sizeof(buf), &sd->sd_class);
		printf("Section %zu: %s %x\n", i, buf,
		    CDF_TOLE4(sd->sd_offset));
		cdf_dump_section_info(sst, CDF_TOLE4(sd->sd_offset));
	}
}
#endif

#ifdef TEST
int
main(int argc, char *argv[])
{
	int fd, i;
	cdf_header_t h;
	cdf_sat_t sat, ssat;
	cdf_stream_t sst;
	cdf_dir_t dir;

	if (argc < 2) {
		(void)fprintf(stderr, "Usage: %s <filename>\n", getprogname());
		return -1;
	}

	for (i = 1; i < argc; i++) {
		if ((fd = open(argv[1], O_RDONLY)) == -1)
			err(1, "Cannot open `%s'", argv[1]);

		if (cdf_read_header(fd, &h) == -1)
			err(1, "Cannot read header");
#ifdef CDF_DEBUG
		cdf_dump_header(&h);
#endif

		if (cdf_read_sat(fd, &h, &sat) == -1)
			err(1, "Cannot read sat");
#ifdef CDF_DEBUG
		cdf_dump_sat("SAT", &h, &sat);
#endif

		if (cdf_read_ssat(fd, &h, &sat, &ssat) == -1)
			err(1, "Cannot read ssat");
#ifdef CDF_DEBUG
		cdf_dump_sat("SSAT", &h, &ssat);
#endif

		if (cdf_read_dir(fd, &h, &sat, &dir) == -1)
			err(1, "Cannot read dir");
#ifdef CDF_DEBUG
		cdf_dump_dir(fd, &h, &sat, &dir);
#endif

		if (cdf_read_short_stream(fd, &h, &sat, &dir, &sst) == -1)
			err(1, "Cannot read short stream");
#ifdef CDF_DEBUG
		cdf_dump_stream(&h, &sst);
#endif

		if (cdf_read_summary_info(fd, &h, &sat, &dir, &sst)
		    == -1)
			err(1, "Cannot read summary info");
#ifdef CDF_DEBUG
		cdf_dump_summary_info(&h, &sst);
#endif

		(void)close(fd);
	}

	return 0;
}
#endif
