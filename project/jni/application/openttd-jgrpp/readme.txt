Quick compilation guide:
Install liblzma-dev and libtimidity-dev, it's needed for broken configure script
sudo apt-get install liblzma-dev libtimidity-dev
Download my Git repo from https://github.com/pelya/commandergenius,
then install Android SDK, Android NDK, then launch commands
    git submodule update --init --recursive
    build.sh openttd-jgrpp
