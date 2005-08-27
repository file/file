MAGIC	 = $(pkgdatadir)/magic
lib_LTLIBRARIES = libmagic.la
include_HEADERS = magic.h
EXTRA_DIST = test.c

bin_PROGRAMS = file

AM_CPPFLAGS = -DMAGIC='"$(MAGIC)"'

libmagic_la_SOURCES = magic.c apprentice.c softmagic.c ascmagic.c \
	compress.c is_tar.c readelf.c print.c fsmagic.c \
	funcs.c file.h names.h patchlevel.h readelf.h tar.h apptype.c
libmagic_la_LDFLAGS = -no-undefined -version-info 1:0:0

file_SOURCES = file.c
file_LDADD = libmagic.la
