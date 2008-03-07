MAGIC_FRAGMENT_DIR = $(top_srcdir)/magic/magic

MAGIC_FRAGMENTS = $(MAGIC_FRAGMENT_DIR)

pkgdata_DATA = magic.mgc $(MAGIC_FRAGMENTS)

EXTRA_DIST = Localstuff $(MAGIC_FRAGMENTS)

CLEANFILES = magic.mgc

# FIXME: Build file natively as well so that it can be used to compile
# the target's magic file
if IS_CROSS_COMPILE
FILE_COMPILE = file
else
FILE_COMPILE = $(top_builddir)/src/file
endif

magic.mgc: $(MAGIC_FRAGMENTS) $(FILE_COMPILE)
	$(FILE_COMPILE) -C -m $(MAGIC_FRAGMENT_DIR)
