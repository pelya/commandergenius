# info_TEXINFOS = enigma-ref.texi

SUBDIRS = images

HTML = enigma-ref.html

refdir=@docdir@/reference
ref_DATA = $(HTML) ant_lua.txt sounds.txt soundset.lua lua2xml xml2lua

EXTRA_DIST = enigma-ref.texi enigma-t2h.init $(ref_DATA)


enigma-ref.html: $(srcdir)/enigma-ref.texi $(srcdir)/enigma-t2h.init
	@if [ "${TEXI2HTML}" ]; then \
	  echo "Creating reference manual"; \
	  echo "... texi2html ..."; \
	  ${TEXI2HTML} -Verbose -init_file $(srcdir)/enigma-t2h.init $(srcdir)/enigma-ref.texi; \
	else \
	  echo "Warning: texi2html not found"; \
	fi

pdf: enigma-ref.pdf

%.pdf: %.texi
	texi2dvi --pdf $<
