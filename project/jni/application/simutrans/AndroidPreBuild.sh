#!/bin/sh

if [ -e simutrans ]; then
	echo Simutrans SVN already downloaded
else
	svn checkout https://github.com/aburch/simutrans/trunk simutrans || exit 1
	cd simutrans || exit 1
	./get_lang_files.sh || exit 1
	cd ..
fi

echo Generating data.zip
mkdir -p AndroidData
cd simutrans/simutrans
rm -f ../../AndroidData/data.zip
zip -r -0 ../../AndroidData/data.zip * >/dev/null
cd ../..
echo Generating data.zip done

[ -e AndroidData/TimGM6mb.sf2 ] || wget https://sourceforge.net/p/mscore/code/HEAD/tree/trunk/mscore/share/sound/TimGM6mb.sf2?format=raw -O AndroidData/TimGM6mb.sf2
