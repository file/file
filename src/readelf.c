
#ifdef BUILTIN_ELF
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#ifdef __SVR4
#include <sys/procfs.h>
#endif

#include "readelf.h"
#include "file.h"

static void
doshn(fd, off, num, size, buf)
	int fd;
	off_t off;
	int num;
	size_t size;
	char *buf;
{
	/* This works for both 32 and 64 bit elf formats */
	Elf32_Shdr *sh = (Elf32_Shdr *) buf;

	if (lseek(fd, off, SEEK_SET) == -1)
		error("lseek failed (%s).\n", strerror(errno));

	for ( ; num; num--) {
		if (read(fd, buf, size) == -1)
			error("read failed (%s).\n", strerror(errno));
		if (sh->sh_type == SHT_SYMTAB)
			return;
	}
	(void) printf (", stripped");
}

/*
 * From: Ken Pizzini <ken@spry.com>
 */
static void
dophn(fd, off, num, size, buf)
	int fd;
	off_t off;
	int num;
	size_t size;
	char *buf;
{
	/* I am not sure if this works for 64 bit elf formats */
	Elf32_Phdr ph;
	Elf32_Nhdr nh;
	off_t off_sv;
	off_t fname_off;

	if (lseek(fd, off, SEEK_SET) == -1)
		error("lseek failed (%s).\n", strerror(errno));

	for ( ; num; num--) {
		if (read(fd, &ph, sizeof ph) != sizeof ph)
			error("read failed (%s).\n", strerror(errno));
		if (ph.p_type != PT_NOTE)
			continue;
		off_sv = lseek(fd, ph.p_offset, SEEK_SET);
		if (off_sv == -1)
			error("lseek failed (%s).\n", strerror(errno));
		if (read(fd, &nh, sizeof nh) != sizeof nh)
			error("read failed (%s).\n", strerror(errno));
		if (nh.n_type != NT_PRPSINFO) {
			if (lseek(fd, off_sv, SEEK_SET) == -1)
				error("lseek failed (%s).\n", strerror(errno));
			continue;
		}

#ifdef __SVR4

#define ALIGN(n)	(((n)+3) & ~3)	/* round up to nearest multiple of 4 */
#define offsetof(a, b)	(int) &((a *) 0)->b

		fname_off = offsetof(struct prpsinfo, pr_fname) +
			ALIGN(nh.n_namesz);
		if (lseek(fd, fname_off, SEEK_CUR) == -1)
			error("lseek failed (%s).\n", strerror(errno));
		if (read(fd, buf, size) == -1)
			error("read failed (%s).\n", strerror(errno));
		(void) printf ("; from `%s'", buf);
#endif
		return;
	}
}


void
tryelf(fd, buf, nbytes)
	int fd;
	char *buf;
	int nbytes;
{
	union {
		long l;
		char c[sizeof (long)];
	} u;

	/*
	 * ELF executables have multiple section headers in arbitrary
	 * file locations and thus file(1) cannot determine it from easily.
	 * Instead we traverse thru all section headers until a symbol table
	 * one is found or else the binary is stripped.
	 */
	if (buf[EI_MAG0] != ELFMAG0 || buf[EI_MAG1] != ELFMAG1
	    && buf[EI_MAG2] != ELFMAG2 || buf[EI_MAG3] != ELFMAG3)
	    return;


	if (buf[4] == ELFCLASS32) {
		Elf32_Ehdr elfhdr;
		if (nbytes <= sizeof (Elf32_Ehdr))
			return;


		u.l = 1;
		(void) memcpy(&elfhdr, buf, sizeof elfhdr);

		if (elfhdr.e_type == ET_CORE) 
			dophn(fd, elfhdr.e_phoff, elfhdr.e_phnum, 
			      elfhdr.e_phentsize, buf);
		else 
		{
			/*
			 * If the system byteorder does not equal the
			 * object byteorder then don't test.
			 */
			if ((u.c[sizeof(long) - 1] + 1) == elfhdr.e_ident[5])
				doshn(fd, elfhdr.e_shoff, elfhdr.e_shnum,
				      elfhdr.e_shentsize, buf);
		}

		return;
	}

        if (buf[4] == ELFCLASS64) {
		Elf64_Ehdr elfhdr;
		if (nbytes <= sizeof (Elf64_Ehdr))
			return;


		u.l = 1;
		(void) memcpy(&elfhdr, buf, sizeof elfhdr);

		if (elfhdr.e_type == ET_CORE) 
			dophn(fd, elfhdr.e_phoff, elfhdr.e_phnum, 
			      elfhdr.e_phentsize, buf);
		else
		{
			/*
			 * If the system byteorder does not equal the
			 * object byteorder then don't test.
			 */
			if ((u.c[sizeof(long) - 1] + 1) == elfhdr.e_ident[5])
				doshn(fd, elfhdr.e_shoff, elfhdr.e_shnum,
				      elfhdr.e_shentsize, buf);
		}

		return;
	}
}
#endif
