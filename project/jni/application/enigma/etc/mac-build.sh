export CPPFLAGS="-I`sdl-config --prefix`/include -DMACOSX"
export LDFLAGS=-L`sdl-config --prefix`/lib
./configure
make
cd src
make bundle-fw
