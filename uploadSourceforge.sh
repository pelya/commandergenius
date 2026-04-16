#!/bin/bash

rsync -avP -e ssh OpenLiero*.{apk,tar.xz} "pelya@frs.sourceforge.net:/home/frs/project/libsdl-android/apk/OpenLiero/"

# List the server directory
#rsync -avP -e ssh "pelya@frs.sourceforge.net:/home/frs/project/libsdl-android/apk/OpenLiero/"
