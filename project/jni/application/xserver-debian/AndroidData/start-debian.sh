#!/system/bin/sh

$SECURE_STORAGE_DIR/usr/bin/xloadimage -onroot -fullscreen $UNSECURE_STORAGE_DIR/logo.png

logwrapper $SECURE_STORAGE_DIR/img/proot.sh /startx.sh
