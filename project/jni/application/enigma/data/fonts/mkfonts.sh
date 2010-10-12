#!/bin/bash
#
# to run this script you need to have ImageMagick installed!
# to compile ttf2bmf you need the SDL_ttf and SDL_ttf-devel libraries >= 1.2.2
#

# TTF2BMF=../ttf2bmf
TTF2BMF=../../tools/ttf2bmf
if [ -x $TTF2BMF ]; then

    $TTF2BMF -as 24 dreamorp.ttf
    $TTF2BMF -as 17 -c 0x909090 -o menufont helmetr.ttf
    $TTF2BMF -as 17 -c 0xcccc00 -o menufontsel helmetr.ttf
    $TTF2BMF -as 28 -c 0xaaaaaa -g "0123456789:-" -o timefont helmetb.ttf
    $TTF2BMF -as 14 -c 0x99aadd -g "0123456789" -o smallfont helmetb.ttf
    $TTF2BMF -as 17 -c 0x909090 -o labelfont helmetr.ttf

    # arial not included due to copyright restrictions
    #$TTF2BMF -as 14 -o levelmenu arial.ttf

    for f in *.bmp; do
        png=`basename $f .bmp`.png

        echo "converting $f to $png"
        # convert -colors 256 $f $png;
        convert $f $png
        mogrify -transparent "#ff00ff" $png
        #mogrify -colors 256 $png
    done

else
    echo "Can't execute $TTF2BMF"
fi
