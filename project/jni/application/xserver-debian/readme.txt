To build system image, download repo:

https://github.com/pelya/debian-noroot

Follow it's readme to build system image.

Then follow instructions here:
https://github.com/pelya/commandergenius/tree/sdl_android/project/jni/application/xserver

If you are releasing the app to Play Store, upload dist-debian-buster-arm64-v8a.tar.xz as 'main' OBB asset.

If you are releasing standalone APK, copy dist-debian-buster-arm64-v8a.tar.xz to assets directory inside .apk zip archive:

```
mkdir assets
cp src/debian-image/img/dist-debian-buster-arm64-v8a.tar.xz assets/
zip project/app/build/outputs/apk/release/app-release.apk assets/dist-debian-buster-arm64-v8a.tar.xz
apksigner sign --ks ~/.android/debug.keystore --ks-key-alias androiddebugkey --ks-pass pass:android project/app/build/outputs/apk/release/app-release.apk
```
