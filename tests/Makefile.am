check_PROGRAMS = test
test_LDADD = $(top_builddir)/src/libmagic.la
test_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = \
	*.testfile *.result

T = $(top_srcdir)/tests
check-local:
	MAGIC=$(top_builddir)/magic/magic ./test
	for i in $T/*.testfile; do MAGIC=$(top_builddir)/magic/magic ./test $T/$$i $T/$${i%%.testfile}.result; done
