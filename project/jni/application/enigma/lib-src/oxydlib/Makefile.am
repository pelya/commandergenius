EXTRA_DIST = README README.enigma

noinst_LIBRARIES = liboxyd.a

liboxyd_a_SOURCES =   	    		\
	DatFile.cpp DatFile.h	 	\
	VecUtils.cpp VecUtils.h	 	\
	FileUtils.cpp FileUtils.h	\
	Level.cpp Level.h		\
	Bitmap.cpp Bitmap.h		\
	OxydVersion.h

if BUILDTOOLS
noinst_PROGRAMS = extractbitmaps extractchunks printlevels printstats

extractbitmaps_SOURCES = extractbitmaps.cpp
extractbitmaps_LDADD = liboxyd.a

extractchunks_SOURCES = extractchunks.cpp
extractchunks_LDADD = liboxyd.a

printlevels_SOURCES = printlevels.cpp
printlevels_LDADD =liboxyd.a

printstats_SOURCES = printstats.cpp
printstats_LDADD = liboxyd.a
endif
