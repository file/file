# Makefile for file(1) cmd. 
# Copyright (c) Ian F. Darwin 86/09/01 - see LEGAL.NOTICE.
# @(#)$Header: /p/file/cvsroot/file/src/Attic/Makefile,v 1.17 1989/05/11 14:58:29 ian Exp $
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
# For installing our man pages; 
# MANCxxx is manual section for Commands, MANFxxx is section for file formats.
# MANxDIR is directory names; MANxEXT is the filename extention. Usual values:
# Variable	V7		4BSD		Sys V
# MANCDIR 	/usr/man/man1	/usr/man/man1	/usr/man/u_man/man1
# MANFDIR 	/usr/man/man5	/usr/man/man5	/usr/man/u_man/man4
# MANCEXT	1		1		1
# MANFEXT	5		5		4
# --- possible alternative for 4BSD ---
# MANCDIR			/usr/man/manl
# MANCEXT			l
# --- possible alternative for USG ---
# MANCDIR			/usr/man/local/man1
# MANCEXT			1

MANCDIR	= /usr/man/manl
MANFDIR	= /usr/man/man5
MANCEXT	= l
MANFEXT	= 5

# There are no system-dependant configuration options (except maybe CFLAGS).
# Delete any of LOCALSRCS and LOCALOBJS that are in your C library.
LOCALSRCS = getopt.c strtol.c strtok.c strchr.c
SRCS = file.c apprentice.c fsmagic.c softmagic.c ascmagic.c is_tar.c \
	print.c $(LOCALSRCS)
LOCALOBJS = getopt.o strtol.o strtok.o strchr.o
OBJS = file.o apprentice.o fsmagic.o softmagic.o ascmagic.o is_tar.o \
	print.o $(LOCALOBJS)

ALLSRC = LEGAL.NOTICE README PORTING $(SRCS) *.h \
	Makefile file.1 magic.4 magdir/[a-z]* tst/Makefile

all:		file magic

try:		all $(OFILE)
		cd tst; make
		time $(OFILE) -m ./magic * tst/* >/tmp/t1
		time ./file -m ./magic * tst/* >/tmp/t2
		-diff -b /tmp/t[12]
		what ./file >lastnocore

file:		$(OBJS)
		cc $(CFLAGS) $(OBJS) -o $@
lint:		$(SRCS)
		lint -ha $(DEFS) $(SRCS) | tee $@
magic:		magdir
#		magdir/Makefile's "install" mv's magic to here for testing.
		cd magdir; make install

ascmagic.o:	names.h

apprentice.o ascmagic.o file.o fsmagic.o print.o softmagic.o: file.h

install:	file magic file.1 magic.4 $(BINDIR) $(MANCDIR) $(MANCDIR)
		cp file		$(BINDIR)/file
		cp magic	$(MAGIC)
		cp file.1	$(MANCDIR)/file.$(MANCEXT)
		cp magic.4	$(MANFDIR)/magic.$(MANFEXT)

clean:
		rm -f *.o file magic lint.out
		(cd tst; make clean)

dist:		$(ALLSRC)
#		Some versions of shar can't handle a single file from
#		a subdirectory, so we manually insert mkdir as needed.
#		Put the extra "mkdir" AFTER the ": to unbundle..." line.
		$(SHAR) $(ALLSRC) | sed -e '1a\
		mkdir magdir tst' >$@

