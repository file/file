check_PROGRAMS = test
test_LDADD = $(top_builddir)/src/libmagic.la

check-local:
	MAGIC=$(top_builddir)/magic/magic ./test
	for i in $(TEST_BASES); do MAGIC=./$$i.magic ./test ./$$i.testfile ./$$i.result; done

TEST_BASES = gedcom

EXTRA_DIST = \
	gedcom.magic gedcom.testfile gedcom.result
