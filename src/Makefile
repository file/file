# Makefile for file(1) cmd. Copyright (c) Darwin 86/09/01 - see LEGAL.NOTICE.

DEFS = # -Dvoid=int
CFLAGS = -O $(DEFS)
OTHERS = darwin!ian	# people to whom we mail updates
FILE=/usr/bin/file.sun # old version or system-provided command, for comparison

# There are no system-dependant configuration options (except maybe CFLAGS);
# you may delete some of LOCALSRCS and LOCALOBJS if they're in your C library.
LOCALSRCS = strtol.c strtok.c
SRCS = file.c apprentice.c fsmagic.c softmagic.c ascmagic.c is_tar.c \
	print.c $(LOCALSRCS)
LOCALOBJS = strtol.o strtok.o
OBJS = file.o apprentice.o fsmagic.o softmagic.o ascmagic.o is_tar.o \
	print.o $(LOCALOBJS)

ALLSRC = LEGAL.NOTICE README PORTING TODO $(SRCS) *.h \
	Makefile file.[15] magdir tst/Makefile

run:		file magic
		time $(FILE) -m ./magic * tst/* >/tmp/t1
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

dist:		$(ALLSRC)
		(echo mkdir tst; bundle $(ALLSRC)) | \
			mailx -s 'semi-public-domain file(1)' $(OTHERS)
update:		$(ALLSRC)
		(echo mkdir tst; bundle $?; echo touch $@) | mail $(OTHERS)
		touch $@
