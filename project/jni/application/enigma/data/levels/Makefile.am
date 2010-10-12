SUBDIRS = enigma_tutorial enigma_i enigma_ii enigma_iii enigma_iv enigma_v \
	enigma_vi enigma_vii \
	enigma_esprit enigma_oxyd enigma_peroxyd enigma_oxydmagnum \
	enigma_oxydextra \
	enigma_sokoban enigma_microban enigma_mas_microban \
	enigma_experimental enigma_stable \
	enigma_cross \
    patches lib

pkgdatadir = $(datadir)/@PACKAGE@/levels
pkgdata_DATA = $(wildcard $(srcdir)/*.lua $(srcdir)/*.xml)
EXTRA_DIST = 00makepreviews.sh $(pkgdata_DATA)
