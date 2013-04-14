function help() {
cat <<EOF
Invoke ". build/envsetup.sh" from your shell to add the following functions to your environment:
- build:   build {sdl_app_name} [-s] [-i] [-r] [release]
- croot:   Changes directory to the top of the tree.
- cgrep:   Greps on all local C/C++ files.
- jgrep:   Greps on all local Java files.
- resgrep: Greps on all local res/*.xml files.
- godir:   Go to the directory containing a file.

Look at the source to view more functions. The complete list is:
EOF
    T=$(gettop)
    local A
    A=""
    for i in `cat $T/build/envsetup.sh | sed -n "/^function /s/function \([a-z_]*\).*/\1/p" | sort`; do
      A="$A $i"
    done
    echo $A
}

# Get the value of a build variable as an absolute path.
function get_abs_build_var()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi
    (cd $T; CALLED_FROM_SETUP=true BUILD_SYSTEM=build/core \
      make --no-print-directory -C "$T" -f build/core/config.mk dumpvar-abs-$1)
}

# Get the exact value of a build variable.
function get_build_var()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi
    CALLED_FROM_SETUP=true BUILD_SYSTEM=build/core \
      make --no-print-directory -C "$T" -f build/core/config.mk dumpvar-$1
}

# check to see if the supplied product is one we can build
function check_sdl_app()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi
    CALLED_FROM_SETUP=true BUILD_SYSTEM=build/core \
        TARGET_PRODUCT=$1 \
        TARGET_BUILD_VARIANT= \
        TARGET_BUILD_TYPE= \
        TARGET_BUILD_APPS= \
        get_build_var TARGET_DEVICE > /dev/null
    # hide successful answers, but allow the errors to show
}

VARIANT_CHOICES=(user userdebug eng)

# check to see if the supplied variant is valid
function check_variant()
{
    for v in ${VARIANT_CHOICES[@]}
    do
        if [ "$v" = "$1" ]
        then
            return 0
        fi
    done
    return 1
}

function setpaths()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP."
        return
    fi

    ##################################################################
    #                                                                #
    #              Read me before you modify this code               #
    #                                                                #
    #   This function sets ANDROID_BUILD_PATHS to what it is adding  #
    #   to PATH, and the next time it is run, it removes that from   #
    #   PATH.  This is required so lunch can be run more than once   #
    #   and still have working paths.                                #
    #                                                                #
    ##################################################################

    # Note: on windows/cygwin, ANDROID_BUILD_PATHS will contain spaces
    # due to "C:\Program Files" being in the path.

    # out with the old
    if [ -n "$ANDROID_BUILD_PATHS" ] ; then
        export PATH=${PATH/$ANDROID_BUILD_PATHS/}
    fi
    if [ -n "$ANDROID_PRE_BUILD_PATHS" ] ; then
        export PATH=${PATH/$ANDROID_PRE_BUILD_PATHS/}
        # strip leading ':', if any
        export PATH=${PATH/:%/}
    fi

    # and in with the new
    CODE_REVIEWS=
    prebuiltdir=$(getprebuilt)
    gccprebuiltdir=$(get_abs_build_var ANDROID_GCC_PREBUILTS)

    # The gcc toolchain does not exists for windows/cygwin. In this case, do not reference it.
    export ANDROID_EABI_TOOLCHAIN=
    local ARCH=$(get_build_var TARGET_ARCH)
    case $ARCH in
        x86) toolchaindir=x86/i686-linux-android-4.6/bin
            ;;
        arm) toolchaindir=arm/arm-linux-androideabi-4.6/bin
            ;;
        mips) toolchaindir=mips/mipsel-linux-android-4.6/bin
            ;;
        *)
            echo "Can't find toolchain for unknown architecture: $ARCH"
            toolchaindir=xxxxxxxxx
            ;;
    esac
    if [ -d "$gccprebuiltdir/$toolchaindir" ]; then
        export ANDROID_EABI_TOOLCHAIN=$gccprebuiltdir/$toolchaindir
    fi

    unset ARM_EABI_TOOLCHAIN ARM_EABI_TOOLCHAIN_PATH
    case $ARCH in
        arm)
            toolchaindir=arm/arm-eabi-4.6/bin
            if [ -d "$gccprebuiltdir/$toolchaindir" ]; then
                 export ARM_EABI_TOOLCHAIN="$gccprebuiltdir/$toolchaindir"
                 ARM_EABI_TOOLCHAIN_PATH=":$gccprebuiltdir/$toolchaindir"
            fi
            ;;
        mips) toolchaindir=mips/mips-eabi-4.4.3/bin
            ;;
        *)
            # No need to set ARM_EABI_TOOLCHAIN for other ARCHs
            ;;
    esac

    export ANDROID_TOOLCHAIN=$ANDROID_EABI_TOOLCHAIN
    export ANDROID_QTOOLS=$T/development/emulator/qtools
    export ANDROID_DEV_SCRIPTS=$T/development/scripts
    export ANDROID_BUILD_PATHS=$(get_build_var ANDROID_BUILD_PATHS):$ANDROID_QTOOLS:$ANDROID_TOOLCHAIN$ARM_EABI_TOOLCHAIN_PATH$CODE_REVIEWS:$ANDROID_DEV_SCRIPTS:
    export PATH=$ANDROID_BUILD_PATHS$PATH

    unset ANDROID_JAVA_TOOLCHAIN
    unset ANDROID_PRE_BUILD_PATHS
    if [ -n "$JAVA_HOME" ]; then
        export ANDROID_JAVA_TOOLCHAIN=$JAVA_HOME/bin
        export ANDROID_PRE_BUILD_PATHS=$ANDROID_JAVA_TOOLCHAIN:
        export PATH=$ANDROID_PRE_BUILD_PATHS$PATH
    fi

    unset ANDROID_PRODUCT_OUT
    export ANDROID_PRODUCT_OUT=$(get_abs_build_var PRODUCT_OUT)
    export OUT=$ANDROID_PRODUCT_OUT

    unset ANDROID_HOST_OUT
    export ANDROID_HOST_OUT=$(get_abs_build_var HOST_OUT)

    # needed for processing samples collected by perf counters
    unset OPROFILE_EVENTS_DIR
    export OPROFILE_EVENTS_DIR=$T/external/oprofile/events

    # needed for building linux on MacOS
    # TODO: fix the path
    #export HOST_EXTRACFLAGS="-I "$T/system/kernel_headers/host_include
}

function printconfig()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi
    get_build_var report_config
}

function set_stuff_for_environment()
{
    settitle
    set_java_home
    setpaths
    set_sequence_number

    export ANDROID_BUILD_TOP=$(gettop)
}

function set_sequence_number()
{
    export BUILD_ENV_SEQUENCE_NUMBER=10
}

function settitle()
{
    if [ "$STAY_OFF_MY_LAWN" = "" ]; then
        local arch=$(gettargetarch)
        local product=$TARGET_PRODUCT
        local variant=$TARGET_BUILD_VARIANT
        local apps=$TARGET_BUILD_APPS
        if [ -z "$apps" ]; then
            export PROMPT_COMMAND="echo -ne \"\033]0;[${arch}-${product}-${variant}] ${USER}@${HOSTNAME}: ${PWD}\007\""
        else
            export PROMPT_COMMAND="echo -ne \"\033]0;[$arch $apps $variant] ${USER}@${HOSTNAME}: ${PWD}\007\""
        fi
    fi
}

function addcompletions()
{
    local T dir f

    # Keep us from trying to run in something that isn't bash.
    if [ -z "${BASH_VERSION}" ]; then
        return
    fi

    # Keep us from trying to run in bash that's too old.
    if [ ${BASH_VERSINFO[0]} -lt 3 ]; then
        return
    fi

    dir="sdk/bash_completion"
    if [ -d ${dir} ]; then
        for f in `/bin/ls ${dir}/[a-z]*.bash 2> /dev/null`; do
            echo "including $f"
            . $f
        done
    fi
}

function choosetype()
{
    echo "Build type choices are:"
    echo "     1. release"
    echo "     2. debug"
    echo

    local DEFAULT_NUM DEFAULT_VALUE
    DEFAULT_NUM=1
    DEFAULT_VALUE=release

    export TARGET_BUILD_TYPE=
    local ANSWER
    while [ -z $TARGET_BUILD_TYPE ]
    do
        echo -n "Which would you like? ["$DEFAULT_NUM"] "
        if [ -z "$1" ] ; then
            read ANSWER
        else
            echo $1
            ANSWER=$1
        fi
        case $ANSWER in
        "")
            export TARGET_BUILD_TYPE=$DEFAULT_VALUE
            ;;
        1)
            export TARGET_BUILD_TYPE=release
            ;;
        release)
            export TARGET_BUILD_TYPE=release
            ;;
        2)
            export TARGET_BUILD_TYPE=debug
            ;;
        debug)
            export TARGET_BUILD_TYPE=debug
            ;;
        *)
            echo
            echo "I didn't understand your response.  Please try again."
            echo
            ;;
        esac
        if [ -n "$1" ] ; then
            break
        fi
    done

    set_stuff_for_environment
}

#
# This function isn't really right:  It chooses a TARGET_PRODUCT
# based on the list of boards.  Usually, that gets you something
# that kinda works with a generic product, but really, you should
# pick a product by name.
#
function chooseproduct()
{
    if [ "x$TARGET_PRODUCT" != x ] ; then
        default_value=$TARGET_PRODUCT
    else
        default_value=full
    fi

    export TARGET_PRODUCT=
    local ANSWER
    while [ -z "$TARGET_PRODUCT" ]
    do
        echo -n "Which product would you like? [$default_value] "
        if [ -z "$1" ] ; then
            read ANSWER
        else
            echo $1
            ANSWER=$1
        fi

        if [ -z "$ANSWER" ] ; then
            export TARGET_PRODUCT=$default_value
        else
            if check_sdl_app $ANSWER
            then
                export TARGET_PRODUCT=$ANSWER
            else
                echo "** Not a valid product: $ANSWER"
            fi
        fi
        if [ -n "$1" ] ; then
            break
        fi
    done

    set_stuff_for_environment
}

function choosevariant()
{
    echo "Variant choices are:"
    local index=1
    local v
    for v in ${VARIANT_CHOICES[@]}
    do
        # The product name is the name of the directory containing
        # the makefile we found, above.
        echo "     $index. $v"
        index=$(($index+1))
    done

    local default_value=eng
    local ANSWER

    export TARGET_BUILD_VARIANT=
    while [ -z "$TARGET_BUILD_VARIANT" ]
    do
        echo -n "Which would you like? [$default_value] "
        if [ -z "$1" ] ; then
            read ANSWER
        else
            echo $1
            ANSWER=$1
        fi

        if [ -z "$ANSWER" ] ; then
            export TARGET_BUILD_VARIANT=$default_value
        elif (echo -n $ANSWER | grep -q -e "^[0-9][0-9]*$") ; then
            if [ "$ANSWER" -le "${#VARIANT_CHOICES[@]}" ] ; then
                export TARGET_BUILD_VARIANT=${VARIANT_CHOICES[$(($ANSWER-1))]}
            fi
        else
            if check_variant $ANSWER
            then
                export TARGET_BUILD_VARIANT=$ANSWER
            else
                echo "** Not a valid variant: $ANSWER"
            fi
        fi
        if [ -n "$1" ] ; then
            break
        fi
    done
}

function choosecombo()
{
    choosetype $1

    echo
    echo
    chooseproduct $2

    echo
    echo
    choosevariant $3

    echo
    set_stuff_for_environment
    printconfig
}

# Clear this variable.  It will be built up again when the vendorsetup.sh
# files are included at the end of this file.
unset LUNCH_MENU_CHOICES
function add_lunch_combo()
{
    local new_combo=$1
    local c
    for c in ${LUNCH_MENU_CHOICES[@]} ; do
        if [ "$new_combo" = "$c" ] ; then
            return
        fi
    done
    LUNCH_MENU_CHOICES=(${LUNCH_MENU_CHOICES[@]} $new_combo)
}

# clean the build environment
clean_build_env() {
    T=$(gettop)
    APP_SRC_LINK=$T/project/jni/application/src
    if [ -L $APP_SRC_LINK ]; then
        unlink $APP_SRC_LINK && echo -n "src link: $APP_SRC_LINK removed..."
    fi
}

# add all the target native application to lunch combo
function add_sdl_applications() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP."
        return
    fi
    APP_SRC_ROOT=$T/project/jni/application
    count=0
    for app in `find $APP_SRC_ROOT -maxdepth 1 -mindepth 1 -type d`; do
        if [ X$app != Xsrc ]; then
            add_lunch_combo $(basename $app)
        fi
    done
}

function print_lunch_menu()
{
    local uname=$(uname)
    echo
    echo "You're building on" $uname
    echo
    echo "Lunch menu... pick a SDL application:"

    local i=1
    local choice
    for choice in ${LUNCH_MENU_CHOICES[@]}
    do
        echo "     $i. $choice"
        i=$(($i+1))
    done

    echo
}

function clean_build_env()
{
    T=$(gettop)
    APP_SRC_LINK=$T/project/jni/application/src
    if [ -L $APP_SRC_LINK ]; then
    unlink $APP_SRC_LINK && echo "src link: $APP_SRC_LINK removed..."
    fi
}

function check_cmd_exist()
{
    if [ "$#" -eq 1 ]; then
        if [ -z `which $1` ]; then
            echo -n "CMD: $1 is not contained in your PATH env, return"
            exit 1;
        fi
    fi
}

function check_aapt()
{
    check_cmd_exist lsb_release
    RELEASE_VERSION=`lsb_release -s -d`
    case $RELEASE_VERSION in
    "Ubuntu 12.10")
# on Ubuntu 12.10 & latest android sdk tools, the aapt is LSB 32bit binary
        UNAME=`uname -a |grep -i "x86_64"`
        if [ ! -z $UNAME ]; then
            echo -n "On your current system, maybe you need to install some dependency to aapt"
            apt-get --no-install-recommends install -y ia32-libs-multiarch
        fi
    ;;
    esac
}

function build()
{
    local OLDPWD=`pwd`
    clean_build_env
    add_sdl_applications

    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP."
        return
    fi
    cd $T
    local answer

    if [ "$1" ] ; then
        answer=$1
    else
        print_lunch_menu
        echo -n "Which would you like? [ballfield] "
        read answer
    fi

    local selection=

    if [ -z "$answer" ]
    then
        selection=ballfield
    elif (echo -n $answer | grep -q -e "^[0-9][0-9]*$")
    then
        if [ $answer -le ${#LUNCH_MENU_CHOICES[@]} ]
        then
            selection=${LUNCH_MENU_CHOICES[$(($answer-1))]}
        fi
    elif (echo -n $answer | grep -q -e "^[^\-][^\-]*-[^\-][^\-]*$")
    then
        selection=$answer
    fi

    if [ -z "$selection" ]
    then
        echo
        echo "Invalid lunch combo: $answer"
        return 1
    fi

    local target_sdl_app=$selection

#echo -n "target sdl app selected:$target_sdl_app"

    ln -s $target_sdl_app $T/project/jni/application/src
    if [ -x $T/changeAppSettings.sh ]; then
        $T/changeAppSettings.sh -a
    fi

# check PATH setting & build utils
    check_cmd_exist android
    check_cmd_exist ndk-build
    check_cmd_exist ant
    check_cmd_exist aapt

#check_aapt

    local sdk_version
    echo -n "Available SDK/API levels:"
    echo
    android list target | grep "API level:"

    echo -n "Which android sdk version/API level do you want to apply?"
    read sdk_version
    android update project -p $T/project -t android-$sdk_version
    if [ $? -ne 0 ]; then
        echo -n "Applying sdk version $sdk_version failed"
        echo -n "Try to apply the default level 15"
        android update project -p project -t android-15 -s
    fi

    install_apk=false
    run_apk=false
    sign_apk=false
    build_release=false

    if [ "$#" -gt 1 -a "$2" = "-s" ]; then
            shift
            sign_apk=true
    fi

    if [ "$#" -gt 1 -a "$2" = "-i" ]; then
            shift
            install_apk=true
    fi

    if [ "$#" -gt 1 -a "$2" = "-r" ]; then
            shift
            install_apk=true
            run_apk=true
    fi

    if [ "$#" -gt 1 -a "$2" = "release" ]; then
            shift
            build_release=true
    fi

    [ -e $T/project/local.properties ] || {
            android update project -p project || exit 1
            rm -f project/src/Globals.java
    }
# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r7
    NDKBUILDPATH=$PATH
    export `grep "AppFullName=" AndroidAppSettings.cfg`
    if ( grep "package $AppFullName;" $T/project/src/Globals.java > /dev/null 2>&1 && \
                    [ "`readlink $T/AndroidAppSettings.cfg`" -ot "$T/project/src/Globals.java" ] && \
                    [ -z "`find $T/project/java/* $T/project/AndroidManifestTemplate.xml -cnewer $T/project/src/Globals.java`" ] ) ; then true ; else
            $T/changeAppSettings.sh -a
            sleep 1
            touch $T/project/src/Globals.java
    fi
    if $build_release ; then
            sed -i 's/android:debuggable="true"/android:debuggable="false"/g' project/AndroidManifest.xml
    fi

    MYARCH=linux-x86
    NCPU=4
    if uname -s | grep -i "linux" > /dev/null ; then
            MYARCH=linux-x86
            NCPU=`cat /proc/cpuinfo | grep -c -i processor`
    fi
    if uname -s | grep -i "darwin" > /dev/null ; then
            MYARCH=darwin-x86
    fi
    if uname -s | grep -i "windows" > /dev/null ; then
            MYARCH=windows-x86
    fi
    grep "64.bit" "`which ndk-build | sed 's@/ndk-build@@'`/RELEASE.TXT" >/dev/null 2>&1 && MYARCH="${MYARCH}_64"

    rm -r -f $T/project/bin/* # New Android SDK introduced some lame-ass optimizations to the build system which we should take care about
    [ -x $T/project/jni/application/src/AndroidPreBuild.sh ] && {
            cd $T/project/jni/application/src
            ./AndroidPreBuild.sh || { echo "AndroidPreBuild.sh returned with error" ; exit 1 ; }
            cd $T
    }

cd $T/project && env PATH=$NDKBUILDPATH BUILD_NUM_CPUS=$NCPU nice -n19 ndk-build -j$NCPU V=1 && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		echo Stripping libapplication.so by hand && \
		rm obj/local/armeabi/libapplication.so && \
		cp jni/application/src/libapplication.so obj/local/armeabi/ && \
		cp jni/application/src/libapplication.so libs/armeabi/ && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-4.6/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi/libapplication.so \
		|| true ; } && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=y" ../AndroidAppSettings.cfg > /dev/null && \
		echo Stripping libapplication-armeabi-v7a.so by hand && \
		rm obj/local/armeabi-v7a/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a.so obj/local/armeabi-v7a/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a.so libs/armeabi-v7a/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-4.6/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi-v7a/libapplication.so \
		|| true ; } && \
	cd .. && ./copyAssets.sh && cd $T/project && \
	{	if $build_release ; then \
			ant release || exit 1 ; \
			jarsigner -verbose -keystore ~/.android/debug.keystore -storepass android -sigalg MD5withRSA -digestalg SHA1 bin/MainActivity-release-unsigned.apk androiddebugkey || exit 1 ; \
			zipalign 4 bin/MainActivity-release-unsigned.apk bin/MainActivity-debug.apk ; \
		else \
			ant debug ; \
		fi ; } && \
	{	if $sign_apk; then cd .. && ./sign.sh && cd $T/project ; else true ; fi ; } && \
	$install_apk && [ -n "`adb devices | tail -n +2`" ] && \
	{	cd bin && adb install -r MainActivity-debug.apk | grep 'Failure' && \
		adb uninstall `grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'` && adb install -r MainActivity-debug.apk ; true ; } && \
	$run_apk && { \
		ActivityName="`grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'`/.MainActivity" ; \
		RUN_APK="adb shell am start -n $ActivityName" ; \
		echo "Running $ActivityName on the USB-connected device:" ; \
		echo "$RUN_APK" ; \
		eval $RUN_APK ; \
	}
#set_stuff_for_environment
#printconfig
    cd $OLDPWD
}

# Tab completion for lunch.
function _lunch()
{
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    COMPREPLY=( $(compgen -W "${LUNCH_MENU_CHOICES[*]}" -- ${cur}) )
    return 0
}
complete -F _lunch lunch

# Configures the build to build unbundled apps.
# Run tapas with one ore more app names (from LOCAL_PACKAGE_NAME)
function tapas()
{
    local arch=$(echo -n $(echo $* | xargs -n 1 echo | \grep -E '^(arm|x86|mips)$'))
    local variant=$(echo -n $(echo $* | xargs -n 1 echo | \grep -E '^(user|userdebug|eng)$'))
    local apps=$(echo -n $(echo $* | xargs -n 1 echo | \grep -E -v '^(user|userdebug|eng|arm|x86|mips)$'))

    if [ $(echo $arch | wc -w) -gt 1 ]; then
        echo "tapas: Error: Multiple build archs supplied: $arch"
        return
    fi
    if [ $(echo $variant | wc -w) -gt 1 ]; then
        echo "tapas: Error: Multiple build variants supplied: $variant"
        return
    fi

    local product=full
    case $arch in
      x86)   product=full_x86;;
      mips)  product=full_mips;;
    esac
    if [ -z "$variant" ]; then
        variant=eng
    fi
    if [ -z "$apps" ]; then
        apps=all
    fi

    export TARGET_PRODUCT=$product
    export TARGET_BUILD_VARIANT=$variant
    export TARGET_BUILD_TYPE=release
    export TARGET_BUILD_APPS=$apps

    set_stuff_for_environment
    printconfig
}

function gettop
{
    local TOPFILE=build/envsetup.sh
    if [ -n "$TOP" -a -f "$TOP/$TOPFILE" ] ; then
        echo $TOP
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            # We redirect cd to /dev/null in case it's aliased to
            # a command that prints something as a side-effect
            # (like pushd)
            local HERE=$PWD
            T=
            while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
                cd .. > /dev/null
                T=`PWD= /bin/pwd`
            done
            cd $HERE > /dev/null
            if [ -f "$T/$TOPFILE" ]; then
                echo $T
            fi
        fi
    fi
}

#function m()
#{
#    T=$(gettop)
#    if [ "$T" ]; then
#        make -C $T $@
#    else
#        echo "Couldn't locate the top of the tree.  Try setting TOP."
#    fi
#}

function findmakefile()
{
    TOPFILE=build/core/envsetup.mk
    # We redirect cd to /dev/null in case it's aliased to
    # a command that prints something as a side-effect
    # (like pushd)
    local HERE=$PWD
    T=
    while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
        T=`PWD= /bin/pwd`
        if [ -f "$T/Android.mk" ]; then
            echo $T/Android.mk
            cd $HERE > /dev/null
            return
        fi
        cd .. > /dev/null
    done
    cd $HERE > /dev/null
}

#function mm()
#{
#    # If we're sitting in the root of the build tree, just do a
#    # normal make.
#    if [ -f build/core/envsetup.mk -a -f Makefile ]; then
#        make $@
#    else
#        # Find the closest Android.mk file.
#        T=$(gettop)
#        local M=$(findmakefile)
#        # Remove the path to top as the makefilepath needs to be relative
#        local M=`echo $M|sed 's:'$T'/::'`
#        if [ ! "$T" ]; then
#            echo "Couldn't locate the top of the tree.  Try setting TOP."
#        elif [ ! "$M" ]; then
#            echo "Couldn't locate a makefile from the current directory."
#        else
#            ONE_SHOT_MAKEFILE=$M make -C $T all_modules $@
#        fi
#    fi
#}
#
#function mmm()
#{
#    T=$(gettop)
#    if [ "$T" ]; then
#        local MAKEFILE=
#        local MODULES=
#        local ARGS=
#        local DIR TO_CHOP
#        local DASH_ARGS=$(echo "$@" | awk -v RS=" " -v ORS=" " '/^-.*$/')
#        local DIRS=$(echo "$@" | awk -v RS=" " -v ORS=" " '/^[^-].*$/')
#        for DIR in $DIRS ; do
#            MODULES=`echo $DIR | sed -n -e 's/.*:\(.*$\)/\1/p' | sed 's/,/ /'`
#            if [ "$MODULES" = "" ]; then
#                MODULES=all_modules
#            fi
#            DIR=`echo $DIR | sed -e 's/:.*//' -e 's:/$::'`
#            if [ -f $DIR/Android.mk ]; then
#                TO_CHOP=`(cd -P -- $T && pwd -P) | wc -c | tr -d ' '`
#                TO_CHOP=`expr $TO_CHOP + 1`
#                START=`PWD= /bin/pwd`
#                MFILE=`echo $START | cut -c${TO_CHOP}-`
#                if [ "$MFILE" = "" ] ; then
#                    MFILE=$DIR/Android.mk
#                else
#                    MFILE=$MFILE/$DIR/Android.mk
#                fi
#                MAKEFILE="$MAKEFILE $MFILE"
#            else
#                if [ "$DIR" = snod ]; then
#                    ARGS="$ARGS snod"
#                elif [ "$DIR" = showcommands ]; then
#                    ARGS="$ARGS showcommands"
#                elif [ "$DIR" = dist ]; then
#                    ARGS="$ARGS dist"
#                elif [ "$DIR" = incrementaljavac ]; then
#                    ARGS="$ARGS incrementaljavac"
#                else
#                    echo "No Android.mk in $DIR."
#                    return 1
#                fi
#            fi
#        done
#        ONE_SHOT_MAKEFILE="$MAKEFILE" make -C $T $DASH_ARGS $MODULES $ARGS
#    else
#        echo "Couldn't locate the top of the tree.  Try setting TOP."
#    fi
#}

function croot()
{
    T=$(gettop)
    if [ "$T" ]; then
        cd $(gettop)
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function cproj()
{
    TOPFILE=build/core/envsetup.mk
    # We redirect cd to /dev/null in case it's aliased to
    # a command that prints something as a side-effect
    # (like pushd)
    local HERE=$PWD
    T=
    while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
        T=$PWD
        if [ -f "$T/Android.mk" ]; then
            cd $T
            return
        fi
        cd .. > /dev/null
    done
    cd $HERE > /dev/null
    echo "can't find Android.mk"
}

function pid()
{
   local EXE="$1"
   if [ "$EXE" ] ; then
       local PID=`adb shell ps | fgrep $1 | sed -e 's/[^ ]* *\([0-9]*\).*/\1/'`
       echo "$PID"
   else
       echo "usage: pid name"
   fi
}

# systemstack - dump the current stack trace of all threads in the system process
# to the usual ANR traces file
function systemstack()
{
    adb shell echo '""' '>>' /data/anr/traces.txt && adb shell chmod 776 /data/anr/traces.txt && adb shell kill -3 $(pid system_server)
}

function gdbclient()
{
   local OUT_ROOT=$(get_abs_build_var PRODUCT_OUT)
   local OUT_SYMBOLS=$(get_abs_build_var TARGET_OUT_UNSTRIPPED)
   local OUT_SO_SYMBOLS=$(get_abs_build_var TARGET_OUT_SHARED_LIBRARIES_UNSTRIPPED)
   local OUT_EXE_SYMBOLS=$(get_abs_build_var TARGET_OUT_EXECUTABLES_UNSTRIPPED)
   local PREBUILTS=$(get_abs_build_var ANDROID_PREBUILTS)
   local ARCH=$(get_build_var TARGET_ARCH)
   local GDB
   case "$ARCH" in
       x86) GDB=i686-linux-android-gdb;;
       arm) GDB=arm-linux-androideabi-gdb;;
       mips) GDB=mipsel-linux-android-gdb;;
       *) echo "Unknown arch $ARCH"; return 1;;
   esac

   if [ "$OUT_ROOT" -a "$PREBUILTS" ]; then
       local EXE="$1"
       if [ "$EXE" ] ; then
           EXE=$1
       else
           EXE="app_process"
       fi

       local PORT="$2"
       if [ "$PORT" ] ; then
           PORT=$2
       else
           PORT=":5039"
       fi

       local PID
       local PROG="$3"
       if [ "$PROG" ] ; then
           if [[ "$PROG" =~ ^[0-9]+$ ]] ; then
               PID="$3"
           else
               PID=`pid $3`
           fi
           adb forward "tcp$PORT" "tcp$PORT"
           adb shell gdbserver $PORT --attach $PID &
           sleep 2
       else
               echo ""
               echo "If you haven't done so already, do this first on the device:"
               echo "    gdbserver $PORT /system/bin/$EXE"
                   echo " or"
               echo "    gdbserver $PORT --attach $PID"
               echo ""
       fi

       echo >|"$OUT_ROOT/gdbclient.cmds" "set solib-absolute-prefix $OUT_SYMBOLS"
       echo >>"$OUT_ROOT/gdbclient.cmds" "set solib-search-path $OUT_SO_SYMBOLS:$OUT_SO_SYMBOLS/hw:$OUT_SO_SYMBOLS/ssl/engines:$OUT_SO_SYMBOLS/drm:$OUT_SO_SYMBOLS/egl:$OUT_SO_SYMBOLS/soundfx"
       echo >>"$OUT_ROOT/gdbclient.cmds" "target remote $PORT"
       echo >>"$OUT_ROOT/gdbclient.cmds" ""

       $ANDROID_TOOLCHAIN/$GDB -x "$OUT_ROOT/gdbclient.cmds" "$OUT_EXE_SYMBOLS/$EXE"
  else
       echo "Unable to determine build system output dir."
   fi

}

case `uname -s` in
    Darwin)
        function sgrep()
        {
            find -E . -name .repo -prune -o -name .git -prune -o  -type f -iregex '.*\.(c|h|cpp|S|java|xml|sh|mk)' -print0 | xargs -0 grep --color -n "$@"
        }

        ;;
    *)
        function sgrep()
        {
            find . -name .repo -prune -o -name .git -prune -o  -type f -iregex '.*\.\(c\|h\|cpp\|S\|java\|xml\|sh\|mk\)' -print0 | xargs -0 grep --color -n "$@"
        }
        ;;
esac

function gettargetarch
{
    get_build_var TARGET_ARCH
}

function jgrep()
{
    find . -name .repo -prune -o -name .git -prune -o  -type f -name "*\.java" -print0 | xargs -0 grep --color -n "$@"
}

function cgrep()
{
    find . -name .repo -prune -o -name .git -prune -o -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 grep --color -n "$@"
}

function resgrep()
{
    for dir in `find . -name .repo -prune -o -name .git -prune -o -name res -type d`; do find $dir -type f -name '*\.xml' -print0 | xargs -0 grep --color -n "$@"; done;
}

case `uname -s` in
    Darwin)
        function mgrep()
        {
            find -E . -name .repo -prune -o -name .git -prune -o -path ./out -prune -o -type f -iregex '.*/(Makefile|Makefile\..*|.*\.make|.*\.mak|.*\.mk)' -print0 | xargs -0 grep --color -n "$@"
        }

        function treegrep()
        {
            find -E . -name .repo -prune -o -name .git -prune -o -type f -iregex '.*\.(c|h|cpp|S|java|xml)' -print0 | xargs -0 grep --color -n -i "$@"
        }

        ;;
    *)
        function mgrep()
        {
            find . -name .repo -prune -o -name .git -prune -o -path ./out -prune -o -regextype posix-egrep -iregex '(.*\/Makefile|.*\/Makefile\..*|.*\.make|.*\.mak|.*\.mk)' -type f -print0 | xargs -0 grep --color -n "$@"
        }

        function treegrep()
        {
            find . -name .repo -prune -o -name .git -prune -o -regextype posix-egrep -iregex '.*\.(c|h|cpp|S|java|xml)' -type f -print0 | xargs -0 grep --color -n -i "$@"
        }

        ;;
esac

function getprebuilt
{
    get_abs_build_var ANDROID_PREBUILTS
}

function tracedmdump()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP."
        return
    fi
    local prebuiltdir=$(getprebuilt)
    local arch=$(gettargetarch)
    local KERNEL=$T/prebuilts/qemu-kernel/$arch/vmlinux-qemu

    local TRACE=$1
    if [ ! "$TRACE" ] ; then
        echo "usage:  tracedmdump  tracename"
        return
    fi

    if [ ! -r "$KERNEL" ] ; then
        echo "Error: cannot find kernel: '$KERNEL'"
        return
    fi

    local BASETRACE=$(basename $TRACE)
    if [ "$BASETRACE" = "$TRACE" ] ; then
        TRACE=$ANDROID_PRODUCT_OUT/traces/$TRACE
    fi

    echo "post-processing traces..."
    rm -f $TRACE/qtrace.dexlist
    post_trace $TRACE
    if [ $? -ne 0 ]; then
        echo "***"
        echo "*** Error: malformed trace.  Did you remember to exit the emulator?"
        echo "***"
        return
    fi
    echo "generating dexlist output..."
    /bin/ls $ANDROID_PRODUCT_OUT/system/framework/*.jar $ANDROID_PRODUCT_OUT/system/app/*.apk $ANDROID_PRODUCT_OUT/data/app/*.apk 2>/dev/null | xargs dexlist > $TRACE/qtrace.dexlist
    echo "generating dmtrace data..."
    q2dm -r $ANDROID_PRODUCT_OUT/symbols $TRACE $KERNEL $TRACE/dmtrace || return
    echo "generating html file..."
    dmtracedump -h $TRACE/dmtrace >| $TRACE/dmtrace.html || return
    echo "done, see $TRACE/dmtrace.html for details"
    echo "or run:"
    echo "    traceview $TRACE/dmtrace"
}

# communicate with a running device or emulator, set up necessary state,
# and run the hat command.
function runhat()
{
    # process standard adb options
    local adbTarget=""
    if [ "$1" = "-d" -o "$1" = "-e" ]; then
        adbTarget=$1
        shift 1
    elif [ "$1" = "-s" ]; then
        adbTarget="$1 $2"
        shift 2
    fi
    local adbOptions=${adbTarget}
    #echo adbOptions = ${adbOptions}

    # runhat options
    local targetPid=$1

    if [ "$targetPid" = "" ]; then
        echo "Usage: runhat [ -d | -e | -s serial ] target-pid"
        return
    fi

    # confirm hat is available
    if [ -z $(which hat) ]; then
        echo "hat is not available in this configuration."
        return
    fi

    # issue "am" command to cause the hprof dump
    local sdcard=$(adb shell echo -n '$EXTERNAL_STORAGE')
    local devFile=$sdcard/hprof-$targetPid
    #local devFile=/data/local/hprof-$targetPid
    echo "Poking $targetPid and waiting for data..."
    echo "Storing data at $devFile"
    adb ${adbOptions} shell am dumpheap $targetPid $devFile
    echo "Press enter when logcat shows \"hprof: heap dump completed\""
    echo -n "> "
    read

    local localFile=/tmp/$$-hprof

    echo "Retrieving file $devFile..."
    adb ${adbOptions} pull $devFile $localFile

    adb ${adbOptions} shell rm $devFile

    echo "Running hat on $localFile"
    echo "View the output by pointing your browser at http://localhost:7000/"
    echo ""
    hat -JXmx512m $localFile
}

function getbugreports()
{
    local reports=(`adb shell ls /sdcard/bugreports | tr -d '\r'`)

    if [ ! "$reports" ]; then
        echo "Could not locate any bugreports."
        return
    fi

    local report
    for report in ${reports[@]}
    do
        echo "/sdcard/bugreports/${report}"
        adb pull /sdcard/bugreports/${report} ${report}
        gunzip ${report}
    done
}

function getsdcardpath()
{
    adb ${adbOptions} shell echo -n \$\{EXTERNAL_STORAGE\}
}

function getscreenshotpath()
{
    echo "$(getsdcardpath)/Pictures/Screenshots"
}

function getlastscreenshot()
{
    local screenshot_path=$(getscreenshotpath)
    local screenshot=`adb ${adbOptions} ls ${screenshot_path} | grep Screenshot_[0-9-]*.*\.png | sort -rk 3 | cut -d " " -f 4 | head -n 1`
    if [ "$screenshot" = "" ]; then
        echo "No screenshots found."
        return
    fi
    echo "${screenshot}"
    adb ${adbOptions} pull ${screenshot_path}/${screenshot}
}

function startviewserver()
{
    local port=4939
    if [ $# -gt 0 ]; then
            port=$1
    fi
    adb shell service call window 1 i32 $port
}

function stopviewserver()
{
    adb shell service call window 2
}

function isviewserverstarted()
{
    adb shell service call window 3
}

function key_home()
{
    adb shell input keyevent 3
}

function key_back()
{
    adb shell input keyevent 4
}

function key_menu()
{
    adb shell input keyevent 82
}

function smoketest()
{
    if [ ! "$ANDROID_PRODUCT_OUT" ]; then
        echo "Couldn't locate output files.  Try running 'lunch' first." >&2
        return
    fi
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    (cd "$T" && mmm tests/SmokeTest) &&
      adb uninstall com.android.smoketest > /dev/null &&
      adb uninstall com.android.smoketest.tests > /dev/null &&
      adb install $ANDROID_PRODUCT_OUT/data/app/SmokeTestApp.apk &&
      adb install $ANDROID_PRODUCT_OUT/data/app/SmokeTest.apk &&
      adb shell am instrument -w com.android.smoketest.tests/android.test.InstrumentationTestRunner
}

# simple shortcut to the runtest command
function runtest()
{
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi
    ("$T"/development/testrunner/runtest.py $@)
}

function godir () {
    if [[ -z "$1" ]]; then
        echo "Usage: godir <regex>"
        return
    fi
    T=$(gettop)
    if [[ ! -f $T/filelist ]]; then
        echo -n "Creating index..."
        (cd $T; find . -wholename ./out -prune -o -wholename ./.repo -prune -o -type f > filelist)
        echo " Done"
        echo ""
    fi
    local lines
    lines=($(\grep "$1" $T/filelist | sed -e 's/\/[^/]*$//' | sort | uniq))
    if [[ ${#lines[@]} = 0 ]]; then
        echo "Not found"
        return
    fi
    local pathname
    local choice
    if [[ ${#lines[@]} > 1 ]]; then
        while [[ -z "$pathname" ]]; do
            local index=1
            local line
            for line in ${lines[@]}; do
                printf "%6s %s\n" "[$index]" $line
                index=$(($index + 1))
            done
            echo
            echo -n "Select one: "
            unset choice
            read choice
            if [[ $choice -gt ${#lines[@]} || $choice -lt 1 ]]; then
                echo "Invalid choice"
                continue
            fi
            pathname=${lines[$(($choice-1))]}
        done
    else
        pathname=${lines[0]}
    fi
    cd $T/$pathname
}

# Force JAVA_HOME to point to java 1.6 if it isn't already set
function set_java_home() {
    if [ ! "$JAVA_HOME" ]; then
        case `uname -s` in
            Darwin)
                export JAVA_HOME=/System/Library/Frameworks/JavaVM.framework/Versions/1.6/Home
                ;;
            *)
                export JAVA_HOME=/usr/lib/jvm/java-6-sun
                ;;
        esac
    fi
}

if [ "x$SHELL" != "x/bin/bash" ]; then
    case `ps -o command -p $$` in
        *bash*)
            ;;
        *)
            echo "WARNING: Only bash is supported, use of other shell would lead to erroneous results"
            ;;
    esac
fi

# Execute the contents of any vendorsetup.sh files we can find.
for f in `/bin/ls vendor/*/vendorsetup.sh vendor/*/*/vendorsetup.sh device/*/*/vendorsetup.sh 2> /dev/null`
do
    echo "including $f"
    . $f
done
unset f

addcompletions
