#!/bin/bash

SCREENSHOTS=../../screenshots

update_if_newer()  {
    if [ -f $1 ]; then
        if [ $1 -nt $2 ]; then
            echo "creating $2"
            convert $1 -crop 640x416+0+0 -resize 120x78 $2
        fi
    fi
}

update_for_subdir() {
    if [ -d $1 ]; then
        if [ -d $SCREENSHOTS/$1 ]; then
            for shot in $SCREENSHOTS/$1/*.png; do
                base=`basename $shot .png`
                png="$1/$base.png"
                update_if_newer $shot $png
            done
        fi
    else
        echo "Error: no such directory '$1'"
        exit 1
    fi
}

if [ -d $SCREENSHOTS ]; then
    for shot in $SCREENSHOTS/*.png; do
        base=`basename $shot .png`
        png="$base.png"
        update_if_newer $shot $png
    done

    update_for_subdir Sokoban
    update_for_subdir m_tutor
else
    echo "Error: no such directory ($SCREENSHOTS)"
    exit 1
fi

NEWPREVIEWS=`find . -name "*.png" -cmin -5`
COUNT=`echo $NEWPREVIEWS | wc -w`

if [ $COUNT = 0 ]; then
    echo "All existing previews were up-to-date."
else
    # if pngcrush is installed crush previews
    if [ -x /usr/local/bin/pngcrush ] ; then
        ./04crushprevievs.sh $NEWPREVIEWS
    elif which optipng ; then
        optipng $NEWPREVIEWS
    fi

    echo "$COUNT preview(s) updated."
fi
