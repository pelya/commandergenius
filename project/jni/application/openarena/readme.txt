Quick compilation guide for Debian/Ubuntu (Windows is not supported, MacOsX should be okay though):
Download SDL Git repo from https://github.com/pelya/commandergenius,
install latest Android SDK, latest Android NDK, then launch commands:

    git submodule update --init project/jni/application/openarena/engine
    git submodule update --init project/jni/application/openarena/vm
    cd project/jni/application/openarena
    ./BuildVM.sh
    cd ../../../..
    ./build.sh openarena

That should do it.

To view OpenArena logs, run command

    adb logcat -s DEBUG SDL libSDL OpenArena
