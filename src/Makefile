# Makefile for file(1) cmd. Copyright (c) Darwin 86/09/01 - see LEGAL.NOTICE.

MAGIC	= /etc/magic
DEFS	= -DMAGIC='"$(MAGIC)"' # -Dvoid=int
COPTS	= -O # -g
CFLAGS	= $(COPTS) $(DEFS)
SHAR	= bundle
OTHERS	= darwin!ian		# people to whom we mail updates
OFILE	= /usr/bin/file.orig	# old or distributed version, for comparison
BINDIR	= /usr/local		# where new binary lives
MANDIR	= /usr/man/local	# for our man pages

# There are no system-dependant configuration options (except maybe CFLAGS).
# Delete some of LOCALSRCS and LOCALOBJS if they're in your C library.
LOCALSRCS = strtol.c strtok.c
SRCS = file.c apprentice.c fsmagic.c softmagic.c ascmagic.c is_tar.c \
	print.c $(LOCALSRCS)
LOCALOBJS = strtol.o strtok.o
OBJS = file.o apprentice.o fsmagic.o softmagic.o ascmagic.o is_tar.o \
	print.o $(LOCALOBJS)

ALLSRC = LEGAL.NOTICE README PORTING TODO $(SRCS) *.h \
	Makefile file.[15] magdir tst/Makefile

all:		file magic

run:		all
		cd tst; make
		time $(OFILE) -m ./magic * tst/* >/tmp/t1
		time ./file -m ./magic * tst/* >/tmp/t2
		diff -b /tmp/t[12]

file:		$(OBJS)
		cc $(CFLAGS) $(OBJS) -o $@
lint:		$(SRCS)
		lint -ha $(DEFS) $(SRCS)
magic:		magdir
		cat magdir/[a-z]* >$@ # exclude RCS or SCCS dirs

ascmagic.o:	names.h

apprentice.o ascmagic.o file.o fsmagic.o print.o softmagic.o: file.h

install:	all
		cp file $(BINDIR)/file
		cp magic $(MAGIC)
		cp file.1 $(MANDIR)/man1
		cp file.5 $(MANDIR)/man5

clean:
		rm -f *.o file magic errs lintout
		(cd tst; make clean)

dist:		$(ALLSRC)
		$(SHAR) $(ALLSRC) >dist
		touch update	# kludge
update:		$(ALLSRC)
		(echo mkdir tst; $(SHAR) $?; echo touch $@) | mail $(OTHERS)
		touch $@
