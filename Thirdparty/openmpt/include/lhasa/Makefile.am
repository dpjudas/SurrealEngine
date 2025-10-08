AUX_DIST_GEN = $(ac_aux_dir)

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = \
	$(AUX_DIST_GEN) \
	COPYING.md \
	README.md \
	NEWS.md \
	SECURITY.md \
	gencov \
	rpm.spec
MAINTAINERCLEANFILES = $(AUX_DIST_GEN)

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = liblhasa.pc

SUBDIRS=doc lib pkg src test
