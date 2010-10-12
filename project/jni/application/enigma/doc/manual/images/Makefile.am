PNG = $(wildcard $(srcdir)/*.png)
PDF = $(addprefix $(if $(builddir), $(builddir)/,), $(addsuffix .pdf, $(basename $(notdir $(PNG)))))

pngdir=@docdir@/manual/images
png_DATA=$(PNG)

SUFFIXES = .png .pdf

pdf: $(PDF)

.png.pdf:
	convert -density 112x112 $< $@

EXTRA_DIST = $(PNG)
