/*
 * Names.h - names and types used by ascmagic in file(1).
 * These tokens are here because they can appear anywhere in
 * the first HOWMANY bytes, while tokens in /etc/magic must
 * appear at fixed offsets into the file. Don't make HOWMANY
 * too high unless you have a very fast CPU.
 */

/* these types are used to index the table 'types': keep em in sync! */
#define L_C	0		/* first and foremost on UNIX */
#define	L_FORT	1		/* the oldest one */
#define L_MAKE	2		/* Makefiles */
#define L_PLI	3		/* PL/1 */
#define L_MACH	4		/* some kinda assembler */
#define L_ENG	5		/* English */
#define	L_PAS	6		/* Pascal */

char *types[] = {
	"c program text",
	"fortran program text",
	"makefile commands text" ,
	"pl/1 (blech) program text",
	"assembler (blech) program text",
	"english text",
	"pascal program text",
	"can't happen error on names.h/types",
	0};

struct names {
	char *name;
	short type;
} names[] = {
	/* These must be sorted by eye for optimal hit rate */
	/* Add to this list only after substantial meditation */
	{"/*",		L_C},
	{"#include",	L_C},
	{"char",	L_C},
	{"The",		L_ENG},
	{"double",	L_C},
	{"extern",	L_C},
	{"float",	L_C},
	{"real",	L_C},
	{"struct",	L_C},
	{"union",	L_C},
	{"CFLAGS",	L_MAKE},
	{"LDFLAGS",	L_MAKE},
	{"all:",	L_MAKE},
	{".PRECIOUS",	L_MAKE},
	{"subroutine",	L_FORT},
	{"function",	L_FORT},
	{"block",	L_FORT},
	{"common",	L_FORT},
	{"dimension",	L_FORT},
	{"integer",	L_FORT},
	{"data",	L_FORT},
	{".ascii",	L_MACH},
	{".asciiz",	L_MACH},
	{".byte",	L_MACH},
	{".even",	L_MACH},
	{".globl",	L_MACH},
	{"clr",		L_MACH},
	{"(input,",	L_PAS},
	{"dcl",		L_PLI},
	0};
#define NNAMES ((sizeof(names)/sizeof(struct names)) - 1)
