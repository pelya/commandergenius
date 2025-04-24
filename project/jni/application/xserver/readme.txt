You will need to install some packages to your Debian/Ubuntu first.

Install following packages onto Debian 10 for x86_64 architecture:

sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install bison make zip git locales pkg-config \
curl autoconf automake autopoint libtool libtool-bin help2man texinfo intltool \
xfonts-utils xutils-dev libfontenc-dev libxkbfile-dev libxmuu-dev \
libjpeg-dev libpng-dev libpixman-1-dev libssl-dev libpciaccess-dev \
python3-xcbgen

Install Android NDK r21 and Android SDK with Android 10.0 framework, they must be in your $PATH.

Anything other than Debian 10 is not guaranteed to compile XSDL,
because autoconf scripts search for specific package versions in system directories.

Download SDL repo, select xserver project, and build it:

git clone git@github.com:pelya/commandergenius.git sdl-android
cd sdl-android
git submodule update --init --recursive
./build.sh xserver

Busybox is precompiled, extracted from here:
https://bintray.com/termux/termux-packages-24/busybox
