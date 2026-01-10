#!/bin/sh

mkdir -p build-tools
if [ ! -e build-tools/Makefile ]; then
	if $(c++ --version | grep -qi clang); then
		${CMAKE_BIN_LOC}cmake -DCMAKE_CXX_FLAGS=-stdlib=libc++ -DOPTION_TOOLS_ONLY=ON -B build-tools src
	else
		${CMAKE_BIN_LOC}cmake -DOPTION_TOOLS_ONLY=ON -B build-tools src
	fi
fi

make -C build-tools -j8 VERBOSE=1 || exit 1
./download-data.sh
