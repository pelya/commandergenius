#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
export PATH=$HOME/src/endless_space/gdb-10/bin:$PATH

mkdir -p openttd-pc openttd-pc/baseset
cd openttd-pc
[ -e bin/baseset ] || cp -a ../src/bin ./
[ -e bin/fonts ] || {
	xz -d < ../AndroidData/openttd-fonts.zip.xz > bin/fonts.zip
	cd bin
	unzip fonts.zip
	cd ..
}

[ -e Makefile ] || cmake ../src || exit 1
make -j8 VERBOSE=1 || exit 1
cd bin
cp -f ../baseset/opntitle.dat opntitle.sav

if [ -z "$1" ]; then
	../openttd -d 0 -m null -r 854x480 # -g opntitle.sav
elif [ -n "$2" ]; then
	valgrind --track-fds=yes --log-file=../../valgrind.log --leak-check=full \
	../openttd -d 0 -m null # -g opntitle.sav
else
	#valgrind --track-fds=yes --log-file=valgrind.log --leak-check=full \
	gdb -ex run --args \
	../openttd -d 0 -m null -r 854x480 # -g opntitle.sav
fi
