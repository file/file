/*
 * readelf.h 
 * @(#)$Id: readelf.h,v 1.1 1996/06/22 22:04:22 christos Exp $
 *
 * Provide elf data structures for non-elf machines, allowing file
 * non-elf hosts to determine if an elf binary is stripped.
 * Note: cobbled from the linux header file, with modifications
 */
#ifndef __fake_elf_h__
#define __fake_elf_h__

typedef unsigned int	Elf32_Addr;
typedef unsigned short	Elf32_Half;
typedef unsigned int	Elf32_Off;
typedef unsigned int	Elf32_Word;
typedef unsigned char	Elf32_Char;

#define EI_NIDENT	16

typedef struct elfhdr{
    Elf32_Char	e_ident[EI_NIDENT];
    Elf32_Half	e_type;
    Elf32_Half	e_machine;
    Elf32_Word	e_version;
    Elf32_Addr	e_entry;  /* Entry point */
    Elf32_Off	e_phoff;
    Elf32_Off	e_shoff;
    Elf32_Word	e_flags;
    Elf32_Half	e_ehsize;
    Elf32_Half	e_phentsize;
    Elf32_Half	e_phnum;
    Elf32_Half	e_shentsize;
    Elf32_Half	e_shnum;
    Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

/* sh_type */
#define SHT_SYMTAB	2

/* elf type */
#define ELFDATANONE	0		/* e_ident[EI_DATA] */
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

/* magic number */
#define	EI_MAG0		0		/* e_ident[] indexes */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define	EI_PAD		7

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"

typedef struct {
    Elf32_Word	sh_name;
    Elf32_Word	sh_type;
    Elf32_Word	sh_flags;
    Elf32_Addr	sh_addr;
    Elf32_Off	sh_offset;
    Elf32_Word	sh_size;
    Elf32_Word	sh_link;
    Elf32_Word	sh_info;
    Elf32_Word	sh_addralign;
    Elf32_Word	sh_entsize;
} Elf32_Shdr;

#endif
