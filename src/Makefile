# Makefile for file(1) cmd. 
# Copyright (c) Ian F. Darwin 86/09/01 - see LEGAL.NOTICE.
#
# This software is not subject to any license of the American Telephone
# and Telegraph Company or of the Regents of the University of California.
#
# Permission is granted to anyone to use this software for any purpose on
# any computer system, and to alter it and redistribute it freely, subject
# to the following restrictions:
#
# 1. The author is not responsible for the consequences of use of this
#    software, no matter how awful, even if they arise from flaws in it.
#
# 2. The origin of this software must not be misrepresented, either by
#    explicit claim or by omission.  Since few users ever read sources,
#    credits must appear in the documentation.
#
# 3. Altered versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.  Since few users
#    ever read sources, credits must appear in the documentation.
#
# 4. This notice may not be removed or altered.

MAGIC	= /etc/magic
DEFS	= -DMAGIC='"$(MAGIC)"' # -Dvoid=int
COPTS	= -O # -g
CFLAGS	= $(COPTS) $(DEFS)
SHAR	= bundle
OFILE	= /usr/bin/file.orig	# old or distributed version, for comparison
# Where new binary lives; typically /usr/local (BSD), /usr/lbin (USG).
BINDIR	= /usr/local
# For installing our man pages; typically MANDIR is /usr/man/man1 (BSD)
# or /usr/man/local (USG). MAN?EXT is [15] on BSD, l for BSD if
# installing in /usr/man/manl (for local), [14] on USG.
MANDIR	= /usr/man/local
MAN1EXT	= 1
MAN4EXT	= 4

# There are no system-dependant configuration options (except maybe CFLAGS).
# Delete some of LOCALSRCS and LOCALOBJS if they're in your C library.
LOCALSRCS = strtol.c strtok.c
SRCS = file.c apprentice.c fsmagic.c softmagic.c ascmagic.c is_tar.c \
	print.c $(LOCALSRCS)
LOCALOBJS = strtol.o strtok.o
OBJS = file.o apprentice.o fsmagic.o softmagic.o ascmagic.o is_tar.o \
	print.o $(LOCALOBJS)

ALLSRC = LEGAL.NOTICE README PORTING $(SRCS) *.h \
	Makefile file.1 magic.4 magdir/[a-z]* tst/Makefile

all:		file magic

test:		all
		cd tst; make
		time $(OFILE) -m ./magic * tst/* >/tmp/t1
		time ./file -m ./magic * tst/* >/tmp/t2
		-diff -b /tmp/t[12]
		what ./file >lastnocore

file:		$(OBJS)
		cc $(CFLAGS) $(OBJS) -o $@
lint:		$(SRCS)
		lint -ha $(DEFS) $(SRCS)
magic:		magdir
#		exclude RCS or SCCS dirs:
		cat magdir/[a-z]* >$@

ascmagic.o:	names.h

apprentice.o ascmagic.o file.o fsmagic.o print.o softmagic.o: file.h

install:	all
		cp file $(BINDIR)/file
		cp magic $(MAGIC)
		cp file.1 $(MANDIR)/$(MAN1EXT)
		cp magic.4 $(MANDIR)/$(MAN4EXT)

clean:
		rm -f *.o file magic lint.out
		(cd tst; make clean)

dist:		$(ALLSRC)
		(echo mkdir magdir tst; $(SHAR) $(ALLSRC) ) >$@
