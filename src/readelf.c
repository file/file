
#ifdef BUILTIN_ELF
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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

#ifdef notyet
static void
dophn(fd, off, num, size, buf)
	int fd;
	off_t off;
	int num;
	size_t size;
	char *buf;
{
	Elf32_Phdr *ph = (Elf32_Phdr *) buf;

	if (lseek(fd, off, SEEK_SET) == -1)
		error("lseek failed (%s).\n", strerror(errno));

	for ( ; num; num--) {
		if (read(fd, buf, size) == -1)
			error("read failed (%s).\n", strerror(errno));
		printf("type:%d\n", ph->p_type);
		if (ph->p_type != PT_NOTE)
			continue;
		if (lseek(fd, ph->p_offset, SEEK_SET) == -1)
			error("lseek failed (%s).\n", strerror(errno));
		if (size > BUFSIZ)
			size = BUFSIZ;
		if (read(fd, buf, size) == -1)
			error("read failed (%s).\n", strerror(errno));
		for (size = 0; size < BUFSIZ; size++)
			if (isalnum(buf[size]))
				printf("%d %c\n", size, buf[size]);
		return;
	}
}
#endif

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

#ifdef notyet
		if (elfhdr.e_type == ET_CORE) 
			dophn(fd, elfhdr.e_phoff, elfhdr.e_phnum, 
			      elfhdr.e_phentsize, buf);
		else 
#endif
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

#ifdef notyet
		if (elfhdr.e_type == ET_CORE) 
			dophn(fd, elfhdr.e_phoff, elfhdr.e_phnum, 
			      elfhdr.e_phentsize, buf);
		else
#endif
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
