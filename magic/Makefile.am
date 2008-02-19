magic_FRAGMENTS = magic/*

pkgdata_DATA = magic.mgc $(magic_FRAGMENTS)

EXTRA_DIST = Localstuff $(magic_FRAGMENTS)

CLEANFILES = magic.mgc

# FIXME: Build file natively as well so that it can be used to compile
# the target's magic file
if IS_CROSS_COMPILE
FILE_COMPILE = file
else
FILE_COMPILE = $(top_builddir)/src/file
endif

magic.mgc: $(magic_FRAGMENTS) $(FILE_COMPILE)
	$(FILE_COMPILE) -C -m $(top_srcdir)/magic/magic
