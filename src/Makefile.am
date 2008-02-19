MAGIC = $(pkgdatadir)/magic
lib_LTLIBRARIES = libmagic.la
include_HEADERS = magic.h
EXTRA_DIST = getopt_long.c

bin_PROGRAMS = file

AM_CPPFLAGS = -DMAGIC='"$(MAGIC)"'

# FIXME: Actually run a test on a magic file
check_PROGRAMS = test
test_LDADD = libmagic.la

check-local:
	MAGIC=$(top_builddir)/magic/magic ./test

libmagic_la_SOURCES = magic.c apprentice.c softmagic.c ascmagic.c \
	compress.c is_tar.c readelf.c print.c fsmagic.c \
	funcs.c file.h names.h patchlevel.h readelf.h tar.h apptype.c \
	file_opts.h elfclass.h
libmagic_la_LDFLAGS = -no-undefined -version-info 1:0:0

file_SOURCES = file.c
file_LDADD = libmagic.la
