# if !MINGW32
# Comply with xdg icon theme specification
icondir = $(prefix)/share/icons/hicolor/48x48/apps
icon_DATA = enigma.png

# Backwards-compatible global search location for images
pixmapdir = $(prefix)/share/pixmaps
pixmap_DATA = enigma.png

# Install .desktop file in xdg standard location
desktopdir = $(prefix)/share/applications
desktop_DATA = enigma.desktop
# endif !MINGW32

EXTRA_DIST = \
	build-cross.sh \
        xerces-mingw-conf.sh \
        enigma.spec.in \
	enigma.spec \
	mingw32-configure.sh \
	mingw32-dist.sh.in \
	mingw32-dist.sh \
	nlcvt.pl \
	enigma.desktop \
	enigma.png \
	Info.plist \
	enigma.icns \
	mac-build.sh \
	README-SDL.txt \
	enigma.nsi.in \
	enigma.nsi \
	enigma-inst-lang.nsh \
	enigma-inst-opt.ini \
	enigma-inst-welcome.bmp

MOSTLYCLEANFILES = \
	enigma.dmg
