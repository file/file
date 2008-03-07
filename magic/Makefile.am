MAGIC_FRAGMENT_BASE = Magdir
MAGIC_FRAGMENT_DIR = $(top_srcdir)/magic/$(MAGIC_FRAGMENT_BASE)

pkgdata_DATA = magic.mgc $(MAGIC_FRAGMENT_DIR)

EXTRA_DIST = Localstuff $(MAGIC_FRAGMENT_DIR)

MAGIC = magic.mgc
CLEANFILES = ${MAGIC}

# FIXME: Build file natively as well so that it can be used to compile
# the target's magic file
if IS_CROSS_COMPILE
FILE_COMPILE = file
else
FILE_COMPILE = $(top_builddir)/src/file
endif

${MAGIC}: $(MAGIC_FRAGMENT_DIR) $(FILE_COMPILE)
	$(FILE_COMPILE) -C -m $(MAGIC_FRAGMENT_DIR)
	@mv $(MAGIC_FRAGMENT_DIR).mgc $@
