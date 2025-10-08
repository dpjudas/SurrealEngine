man_MANS=lha.1
EXTRA_DIST=$(man_MANS) html Doxyfile intro.h

html:
	DOXYGEN_DOC_DIR=$(srcdir)/ doxygen $(srcdir)/Doxyfile

clean-local:
	rm -rf html
