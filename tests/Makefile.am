check_PROGRAMS = test
test_LDADD = $(top_builddir)/src/libmagic.la

EXTRA_DIST = \
	*.magic *.testfile *.result

check-local:
	MAGIC=$(top_builddir)/magic/magic ./test
	for i in *.testfile; do MAGIC=./$${i%%.testfile}.magic ./test ./$$i ./$${i%%.testfile}.result; done
