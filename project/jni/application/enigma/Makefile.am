#
# Dependencies:
#   tools -> lib-src/lua
#   src   -> tools, lib-src
#
SUBDIRS = m4 lib-src tools intl src data doc po etc
EXTRA_DIST = enigma CHANGES ACKNOWLEDGEMENTS

docdir = @datadir@/doc/enigma
doc_DATA = README CHANGES COPYING ACKNOWLEDGEMENTS

ACLOCAL_AMFLAGS = -I m4

tolua:
	cd src && $(MAKE) $(AM_MAKEFLAGS) tolua

pot:
	cd po && $(MAKE) $(AM_MAKEFLAGS) enigma.pot-update

pos:
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-po

gmo:
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

macapp:
	cd src && $(MAKE) $(AM_MAKEFLAGS) bundle && $(MAKE) $(AM_MAKEFLAGS) bundle-fw && $(MAKE) $(AM_MAKEFLAGS) bundle-doc && $(MAKE) $(AM_MAKEFLAGS) bundle-dmg

pdf:
	cd doc/manual/images && $(MAKE) pdf
	cd doc/manual && $(MAKE) pdf
	cd doc/reference/images && $(MAKE) pdf
	cd doc/reference && $(MAKE) pdf

