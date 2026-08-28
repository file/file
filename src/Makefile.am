MAGIC = $(pkgdatadir)/magic

lib_LTLIBRARIES = libmagic.la
nodist_include_HEADERS = magic.h

bin_PROGRAMS = file

# LIBHACK disables the GNU regex extensions (REG_GNU et al.) in the
# bundled regcomp.c/regexec.c (see REGEX-COPYRIGHT), which are only used
# as a regcomp(3)/regexec(3) fallback on platforms without their own
# (e.g. AmigaOS/libnix) and whose regex.h does not define REG_GNU.
AM_CPPFLAGS = -DMAGIC='"$(MAGIC)"' -DLIBHACK=1
AM_CFLAGS = $(CFLAG_VISIBILITY) @WARNINGS@

libmagic_la_SOURCES = buffer.c magic.c apprentice.c softmagic.c ascmagic.c \
	encoding.c compress.c is_csv.c is_json.c is_simh.c is_tar.c readelf.c \
	print.c fsmagic.c funcs.c file.h readelf.h tar.h apptype.c der.c der.h \
	file_opts.h elfclass.h mygetopt.h cdf.c cdf_time.c readcdf.c cdf.h \
	swap.c swap.h
libmagic_la_LDFLAGS = -no-undefined -version-info 1:0:0
if MINGW
MINGWLIBS = -lgnurx -lshlwapi
else
MINGWLIBS =
endif
libmagic_la_LIBADD = -lm $(LTLIBOBJS) $(MINGWLIBS)

file_SOURCES = file.c seccomp.c landlock.c
file_LDADD = libmagic.la -lm
CLEANFILES = magic.h
EXTRA_DIST = magic.h.in cdf.mk BNF memtest.c \
	missing/engine.c missing/regex2.h regex-bsd.h missing/cname.h \
	missing/utils.h missing/namespace.h missing/REGEX-COPYRIGHT
HDR= $(top_srcdir)/src/magic.h.in
BUILT_SOURCES = magic.h

magic.h:	${HDR}
	sed -e "s/X.YY/$$(echo @VERSION@ | tr -d .)/" < ${HDR} > $@

clean-local:
	-rm -rf missing/.libs missing/*.o missing/*.lo
