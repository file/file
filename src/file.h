/*
 * file.h - definitions for file(1) program
 * @(#)$Id: file.h,v 1.46 2003/03/23 04:06:05 christos Exp $
 *
 * Copyright (c) Ian F. Darwin, 1987.
 * Written by Ian F. Darwin.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or of the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. The author is not responsible for the consequences of use of this
 *    software, no matter how awful, even if they arise from flaws in it.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Since few users ever read sources,
 *    credits must appear in the documentation.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.  Since few users
 *    ever read sources, credits must appear in the documentation.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef __file_h__
#define __file_h__

#ifndef __linux__
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE 
#define _FILE_OFFSET_BITS 64
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>	/* Include that here, to make sure __P gets defined */
#include <errno.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
/* Do this here and now, because struct stat gets re-defined on solaris */
#include <sys/stat.h>


#define private static
#ifndef protected
#define protected
#endif
#define public

#ifndef HOWMANY
# define HOWMANY 65536		/* how much of the file to look at */
#endif
#define MAXMAGIS 4096		/* max entries in /etc/magic */
#define MAXDESC	50		/* max leng of text description */
#define MAXstring 32		/* max leng of "string" types */

#define MAGICNO		0xF11E041C
#define VERSIONNO	1

#define CHECK	1
#define COMPILE	2

#ifndef __GNUC__
#define __attribute__(a)
#endif

struct magic {
	uint16_t cont_level;	/* level of ">" */
	uint8_t nospflag;	/* supress space character */
	uint8_t flag;
#define INDIR	1		/* if '>(...)' appears,  */
#define	UNSIGNED 2		/* comparison is unsigned */
#define OFFADD	4		/* if '>&' appears,  */
	uint8_t reln;		/* relation (0=eq, '>'=gt, etc) */
	uint8_t vallen;		/* length of string value, if any */
	uint8_t type;		/* int, short, long or string. */
	uint8_t in_type;	/* type of indirrection */
#define 			BYTE	1
#define				SHORT	2
#define				LONG	4
#define				STRING	5
#define				DATE	6
#define				BESHORT	7
#define				BELONG	8
#define				BEDATE	9
#define				LESHORT	10
#define				LELONG	11
#define				LEDATE	12
#define				PSTRING	13
#define				LDATE	14
#define				BELDATE	15
#define				LELDATE	16
#define				REGEX	17
	uint8_t in_op;		/* operator for indirection */
	uint8_t mask_op;	/* operator for mask */
#define				OPAND	1
#define				OPOR	2
#define				OPXOR	3
#define				OPADD	4
#define				OPMINUS	5
#define				OPMULTIPLY	6
#define				OPDIVIDE	7
#define				OPMODULO	8
#define				OPINVERSE	0x80
	int32_t offset;		/* offset to magic number */
	int32_t in_offset;	/* offset from indirection */
	union VALUETYPE {
		uint8_t b;
		uint16_t h;
		uint32_t l;
		char s[MAXstring];
		char *buf;
		uint8_t hs[2];	/* 2 bytes of a fixed-endian "short" */
		uint8_t hl[4];	/* 4 bytes of a fixed-endian "long" */
	} value;		/* either number or string */
	uint32_t mask;	/* mask before comparison with value */
	char desc[MAXDESC];	/* description */
} __attribute__((__packed__));

#define BIT(A)   (1 << (A))
#define STRING_IGNORE_LOWERCASE		BIT(0)
#define STRING_COMPACT_BLANK		BIT(1)
#define STRING_COMPACT_OPTIONAL_BLANK	BIT(2)
#define CHAR_IGNORE_LOWERCASE		'c'
#define CHAR_COMPACT_BLANK		'B'
#define CHAR_COMPACT_OPTIONAL_BLANK	'b'


/* list of magic entries */
struct mlist {
	struct magic *magic;		/* array of magic entries */
	uint32_t nmagic;			/* number of entries in array */
	int mapped;  /* allocation type: 0 => apprentice_file
		      *                  1 => apprentice_map + malloc
		      *                  2 => apprentice_map + mmap */
	struct mlist *next, *prev;
};

struct magic_set {
    struct mlist *mlist;
    struct cont {
	size_t len;
	int32_t *off;
    } c;
    struct out {
	char *buf;
	char *ptr;
	size_t len;
	size_t size;
    } o;
    int flags;
    int haderr;
};

struct stat;
protected char *file_fmttime(long, int);
protected int file_buf(struct magic_set *, const void *buf, size_t);
protected int file_fsmagic(struct magic_set *, const char *, struct stat *);
protected int file_pipe2file(struct magic_set *, int, const void *, size_t);
protected int file_printf(struct magic_set *, const char *, ...);
protected int file_reset(struct magic_set *);
protected int file_tryelf(struct magic_set *, int, const unsigned char *, size_t);
protected int file_zmagic(struct magic_set *, const unsigned char *, size_t);
protected int file_ascmagic(struct magic_set *, const unsigned char *, size_t);
protected int file_is_tar(const unsigned char *, size_t);
protected int file_softmagic(struct magic_set *, const unsigned char *, size_t);
protected struct mlist *file_apprentice(struct magic_set *, const char *, int);
protected uint32_t file_signextend(struct magic_set *, struct magic *, uint32_t);
protected void file_badread(struct magic_set *);
protected void file_badseek(struct magic_set *);
protected void file_oomem(struct magic_set *);
protected void file_error(struct magic_set *, const char *, ...);
protected void file_magwarn(const char *, ...);
protected void file_mdump(struct magic *);
protected void file_showstr(FILE *, const char *, int);

#ifndef HAVE_STRERROR
extern int sys_nerr;
extern char *sys_errlist[];
#define strerror(e) \
	(((e) >= 0 && (e) < sys_nerr) ? sys_errlist[(e)] : "Unknown error")
#endif

#ifndef HAVE_STRTOUL
#define strtoul(a, b, c)	strtol(a, b, c)
#endif

#if defined(HAVE_MMAP) && defined(HAVE_SYS_MMAN_H) && !defined(QUICK)
#define QUICK
#endif

#define FILE_RCSID(id) \
static const char *rcsid(const char *p) { \
	return rcsid(p = id); \
}
#else

#endif /* __file_h__ */
